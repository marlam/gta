/*
 * This file is part of gtatool, a tool to manipulate Generic Tagged Arrays
 * (GTAs).
 *
 * Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012, 2013
 * Martin Lambers <marlam@marlam.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <string>
#include <limits>

#include <gta/gta.hpp>

#include "base/msg.h"
#include "base/blb.h"
#include "base/fio.h"
#include "base/opt.h"
#include "base/str.h"
#include "base/chk.h"
#include "base/end.h"

#include "lib.h"


extern "C" void gtatool_from_rat_help(void)
{
    msg::req_txt("from-rat <input-file> [<output-file>]\n"
            "\n"
            "Converts RAT RadarTools files to GTAs.");
}

static void reorder_rat_data(gta::header &dsthdr, void *dst, const gta::header &srchdr, const void *src)
{
    dsthdr = srchdr;
    std::vector<uintmax_t> dstindices(checked_cast<size_t>(dsthdr.dimensions()));
    std::vector<uintmax_t> srcindices(checked_cast<size_t>(srchdr.dimensions()));
    for (uintmax_t i = 0; i < dsthdr.elements(); i++)
    {
        // For arrays with 2 dimensions, we have to mirror the y component.
        // This code always mirrors the last component; I'm not sure that that's correct.
        dsthdr.linear_index_to_indices(i, &(dstindices[0]));
        for (uintmax_t j = 0; j < dsthdr.dimensions(); j++)
        {
            if (j == dsthdr.dimensions() - 1)
            {
                srcindices[j] = dsthdr.dimension_size(j) - 1 - dstindices[j];
            }
            else
            {
                srcindices[j] = dstindices[j];
            }
        }
        uintmax_t k = srchdr.indices_to_linear_index(&(srcindices[0]));
        memcpy(dsthdr.element(dst, i), srchdr.element(src, k), dsthdr.element_size());
        if (endianness::endianness == endianness::little)
        {
            swap_element_endianness(dsthdr, dsthdr.element(dst, i));
        }
    }
}

extern "C" int gtatool_from_rat(int argc, char *argv[])
{
    std::vector<opt::option *> options;
    opt::info help("help", '\0', opt::optional);
    options.push_back(&help);
    std::vector<std::string> arguments;
    if (!opt::parse(argc, argv, options, 1, 2, arguments))
    {
        return 1;
    }
    if (help.value())
    {
        gtatool_from_rat_help();
        return 0;
    }

    FILE *fo = gtatool_stdout;
    std::string ifilename(arguments[0]);
    std::string ofilename("standard output");
    try
    {
        if (arguments.size() == 2)
        {
            ofilename = arguments[1];
            fo = fio::open(ofilename, "w");
        }
        if (fio::isatty(fo))
        {
            throw exc("refusing to write to a tty");
        }
    }
    catch (std::exception &e)
    {
        msg::err_txt("%s", e.what());
        return 1;
    }

    try
    {
        FILE *fi = fio::open(ifilename, "r");
	int32_t rat_dim;
        fio::read(&rat_dim, sizeof(int32_t), 1, fi, ifilename);
        if (endianness::endianness == endianness::little)
        {
            endianness::swap32(&rat_dim);
        }
        if (rat_dim < 1)
        {
            throw exc(ifilename + ": cannot read RAT data with " + str::from(rat_dim) + " dimensions");
        }
        std::vector<int32_t> rat_sizes(rat_dim);
        fio::read(&(rat_sizes[0]), sizeof(int32_t), rat_dim, fi, ifilename);
        if (endianness::endianness == endianness::little)
        {
            for (int i = 0; i < rat_dim; i++)
            {
                endianness::swap32(&(rat_sizes[i]));
            }
        }
        for (int i = 0; i < rat_dim; i++)
        {
            if (rat_sizes[i] < 1)
            {
                throw exc(ifilename + ": RAT data has invalid dimensions");
            }
        }
	int32_t rat_var;
        fio::read(&rat_var, sizeof(int32_t), 1, fi, ifilename);
        if (endianness::endianness == endianness::little)
        {
            endianness::swap32(&rat_var);
        }
	int32_t rat_type;
        fio::read(&rat_type, sizeof(int32_t), 1, fi, ifilename);
        if (endianness::endianness == endianness::little)
        {
            endianness::swap32(&rat_type);
        }
        int32_t rat_dummy[4];   // I have no idea what these fields mean, but it seems they can be ignored.
        fio::read(rat_dummy, sizeof(int32_t), 4, fi, ifilename);
	char rat_info[80];
        fio::read(rat_info, sizeof(char), 80, fi, ifilename);

        gta::type type;
        // See rat_v0.20/definitions.pro
        switch (rat_var)
        {
        case 1:
            type = gta::uint8;
            break;
        case 2:
        case 3: // this assumes "long" = "int"
            type = gta::int32;
            break;
        case 12:
        case 13: // this assumes "long" = "int"
            type = gta::uint32;
            break;
        case 14:
            type = gta::int64;
            break;
        case 15:
            type = gta::uint64;
            break;
        case 4:
            type = gta::float32;
            break;
        case 5:
            type = gta::float64;
            break;
        case 6:
            type = gta::cfloat32;
            break;
        case 9:
            type = gta::cfloat64;
            break;
        default:
            throw exc(ifilename + ": RAT data has unknown type");
        }
        
        gta::header thdr;
        std::vector<uintmax_t> tdim_sizes(rat_dim);
        for (int i = 0; i < rat_dim; i++)
        {
            tdim_sizes[i] = rat_sizes[i];
        }
        thdr.set_dimensions(rat_dim, &(tdim_sizes[0]));
        thdr.set_components(1, &type, NULL);
        blob tdata(checked_cast<size_t>(thdr.data_size()));
        fio::read(tdata.ptr(), thdr.data_size(), 1, fi, ifilename);
        
        gta::header hdr;
        blob data(checked_cast<size_t>(thdr.data_size()));
        reorder_rat_data(hdr, data.ptr(), thdr, tdata.ptr());

        try
        {
            if (rat_info[0])
            {
                hdr.global_taglist().set("RAT/INFO", std::string(rat_info, 80).c_str());
            }
        }
        catch (...)
        {
        }
        hdr.global_taglist().set("RAT/TYPE", str::from(rat_type).c_str());
        // See rat_v0.20/definitions.pro
        switch (rat_type)
        {
        case 50:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "generic amplitude");
            break;
        case 51:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "generic amplitude (mean scaled)");
            break;
        case 52:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "generic phase");
            break;
        case 53:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "generic complex amplitude");
            break;
        case 54:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "generic complex amplitude (mean scaled)");
            break;
        case 55:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "generic complex phase");
            break;
        case 56:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "generic correlation");
            break;
        case 57:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "generic complex correlation");
            break;
        case 58:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "generic amplitude (histogram scaled)");
            break;
        case 100:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "SAR amplitude image");
            break;
        case 101:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "SAR complex image");
            break;
        case 102:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "SAR phase image");
            break;
        case 103:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "SAR intensity image");
            break;
        case 110:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "SAR image after edge detection");
            break;
        case 120:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "Co-occurance texture features");
            break;
        case 121:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "Variation coefficient");
            break;
        case 122:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "Band ratio");
            break;
        case 123:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "Band difference");
            break;
        case 124:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "Propability of change");
            break;
        case 125:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "Band entropy");
            break;
        case 200:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "scattering vector, lexicographic basis");
            break;
        case 209:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "scattering vector, lexicographic arbitrary basis");
            break;
        case 210:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "Pauli decomposition");
            break;
        case 211:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "Freeman-Durden decomposition");
            break;
        case 212:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "Unknown decomposition");
            break;
        case 213:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "Sphere-Diplane-Helix decomposition");
            break;
        case 214:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "Eigenvector decomposition");
            break;
        case 216:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "Moriyama decomposition");
            break;
        case 220:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "covariance matrix [C]");
            break;
        case 221:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "coherency matrix [T]");
            break;
        case 222:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "covariance matrix [C], arbitrary basis");
            break;
        case 230:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "polarimetric entropy");
            break;
        case 231:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "polarimetric alpha angle");
            break;
        case 232:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "polarimetric anisotropy");
            break;
        case 233:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "Entropy / Alpha / Anisotropy");
            break;
        case 234:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "Alpha / Beta / Gamma / Delta angles");
            break;
        case 250:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "polarimetric span image");
            break;
        case 280:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "ENVISAT partial polarimetry scattering vector");
            break;
        case 300:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "interferometric image pair");
            break;
        case 301:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "complex interferogram");
            break;
        case 302:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "interferometric phase");
            break;
        case 303:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "unwrapped phase");
            break;
        case 310:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "interferometric coherence");
            break;
        case 311:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "complex interferometric coherence");
            break;
        case 320:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "shaded relief");
            break;
        case 390:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "Flat-earth phase");
            break;
        case 391:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "Flat-earth phase (multiple tracks)");
            break;
        case 392:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "Wavenumber");
            break;
        case 393:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "Wavenumber (multiple tracks)");
            break;
        case 394:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "Baseline");
            break;
        case 395:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "Baseline (multiple tracks)");
            break;
        case 400:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "Entropy / Alpha classification");
            break;
        case 401:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "Entropy / Alpha / Anisotropy classification");
            break;
        case 402:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "Wishart Entropy / Alpha classification");
            break;
        case 403:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "Wishart Entropy / Alpha / Anisotropy classification");
            break;
        case 404:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "Physical classification");
            break;
        case 405:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "Forest classification");
            break;
        case 406:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "Surface classification");
            break;
        case 407:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "Double bounce classification");
            break;
        case 408:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "Number of scattering mechanisms");
            break;
        case 409:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "Lee category preserving classification");
            break;
        case 410:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "Cameron classification");
            break;
        case 411:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "Wishart EM classification");
            break;
        case 444:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "General classification");
            break;
        case 450:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "PolInSAR Wishart classification");
            break;
        case 451:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "PolInSAR A1/A2 coherence classification");
            break;
        case 499:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "Colour palette file");
            break;
        case 500:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "PolInSAR scattering vector, lexicographic basis");
            break;
        case 501:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "PolInSAR scattering vector, Pauli basis");
            break;
        case 502:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "PolInSAR scattering vector, lexicographic arbitrary basis");
            break;
        case 503:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "PolInSAR scattering vector, Pauli arbitrary basis");
            break;
        case 510:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "PolInSAR covariance matrix");
            break;
        case 511:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "PolInSAR coherency matrix");
            break;
        case 512:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "PolInSAR covariance matrix, arbitrary basis");
            break;
        case 513:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "PolInSAR coherency matrix, arbitrary basis");
            break;
        case 514:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "PolInSAR normalized cov/coh matrix");
            break;
        case 530:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "PolInSAR coherence");
            break;
        case 532:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "POLInSAR optimized coherence");
            break;
        case 535:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "POLInSAR scattering mechanims vectors");
            break;
        case 540:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "PolInSAR LFF coherence parameters (A1,A2,Hint,Aint)");
            break;
        case 600:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "Subaperture decomposition");
            break;
        case 601:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "Multi-channel subapertures");
            break;
        case 610:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "Covariance matrices for every subaperture");
            break;
        case 615:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "Subapertures covariance matrix");
            break;
        case 630:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "Subapertures stationarity [log(L)]");
            break;
        case 700:
            hdr.global_taglist().set("RAT/TYPE_DESCRIPTION", "Multitemporal data");
            break;
        default:
            break;
        }

        hdr.write_to(fo);
        hdr.write_data(fo, data.ptr());
        fio::close(fi);
        if (fo != gtatool_stdout)
        {
            fio::close(fo);
        }
    }
    catch (std::exception &e)
    {
        msg::err_txt("%s", e.what());
        return 1;
    }

    return 0;
}
