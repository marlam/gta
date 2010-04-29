/*
 * from-gdal.cpp
 *
 * This file is part of gtatool, a tool to manipulate Generic Tagged Arrays
 * (GTAs).
 *
 * Copyright (C) 2010  Martin Lambers <marlam@marlam.de>
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

#include <gdal.h>
#include <cpl_conv.h>

#include <gta/gta.hpp>

#include "msg.h"
#include "blob.h"
#include "cio.h"
#include "opt.h"
#include "debug.h"
#include "intcheck.h"


extern "C" void gtatool_from_gdal_help(void)
{
    msg::req_txt("from-gdal <input-file> [<output-file>]\n"
            "\n"
            "Converts GDAL-readable files to GTAs.");
}

extern "C" int gtatool_from_gdal(int argc, char *argv[])
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
        gtatool_from_gdal_help();
        return 0;
    }

    FILE *fo = stdout;
    std::string ifilename(arguments[0]);
    std::string ofilename("standard output");
    try
    {
        if (arguments.size() == 2)
        {
            ofilename = arguments[1];
            fo = cio::open(ofilename, "w");
        }
        if (cio::isatty(fo))
        {
            throw exc("refusing to write to a tty");
        }
    }
    catch (std::exception &e)
    {
        msg::err("%s", e.what());
        return 1;
    }

    try
    {
        // GTA
        gta::header hdr;
        uintmax_t components;
        blob types;
        blob dataline;
        // GDAL
        GDALDatasetH dataset;
        GDALDriverH driver;
        GDALRasterBandH band;
        double geo_transform[6];
        char **metadata;
        blob scanlines;

        GDALAllRegister();
        if (!(dataset = GDALOpen(ifilename.c_str(), GA_ReadOnly)))
        {
            throw exc("Cannot import " + ifilename, "file does not seem to be in a format supported by GDAL");
        }
        driver = GDALGetDatasetDriver(dataset);
        hdr.set_dimensions(GDALGetRasterXSize(dataset), GDALGetRasterYSize(dataset));
        hdr.dimension_taglist(0).set("INTERPRETATION", "X");
        hdr.dimension_taglist(1).set("INTERPRETATION", "Y");
        if (GDALGetProjectionRef(dataset))
        {
            hdr.global_taglist().set("GDAL/PROJECTION", GDALGetProjectionRef(dataset));
        }
        if (GDALGetGeoTransform(dataset, geo_transform) == CE_None)
        {
            hdr.global_taglist().set("GDAL/ORIGIN", (str::str(geo_transform[0]) + " " + str::str(geo_transform[3])).c_str());
            hdr.global_taglist().set("GDAL/PIXEL_SIZE", (str::str(geo_transform[1]) + " " + str::str(geo_transform[5])).c_str());
            hdr.global_taglist().set("GDAL/GEO_TRANSFORM",
                    (str::str(geo_transform[0]) + " "
                     + str::str(geo_transform[1]) + " "
                     + str::str(geo_transform[2]) + " "
                     + str::str(geo_transform[3]) + " "
                     + str::str(geo_transform[4]) + " "
                     + str::str(geo_transform[5])).c_str());
        }
        metadata = GDALGetMetadata(dataset, NULL);
        if (metadata)
        {
            for (int i = 0; metadata[i] != NULL; i++)
            {
                std::string s(metadata[i]);
                size_t e = s.find('=');
                if (e != std::string::npos && e != 0)
                {
                    hdr.global_taglist().set(
                            (std::string("GDAL/METADATA/") + s.substr(0, e)).c_str(),
                        s.substr(e + 1).c_str());
                }
            }
        }
        components = GDALGetRasterCount(dataset);
        types.resize(sizeof(gta::type), checked_cast<size_t>(components));
        for (uintmax_t i = 0; i < components; i++)
        {
            band = GDALGetRasterBand(dataset, i + 1);
            gta::type type;
            switch (GDALGetRasterDataType(band))
            {
            case GDT_Byte:
                type = gta::uint8;
                break;
            case GDT_UInt16:
                type = gta::uint16;
                break;
            case GDT_Int16:
                type = gta::int16;
                break;
            case GDT_UInt32:
                type = gta::uint32;
                break;
            case GDT_Int32:
                type = gta::int32;
                break;
            case GDT_Float32:
                type = gta::float32;
                break;
            case GDT_Float64:
                type = gta::float64;
                break;
            case GDT_CInt16:
                type = gta::cfloat32;
                msg::inf("Band %s: Converting GDT_CInt16 to gta::cfloat32", str::str(i + 1).c_str());
                break;
            case GDT_CInt32:
                type = gta::cfloat32;
                msg::inf("Band %s: Converting GDT_CInt16 to gta::cfloat32", str::str(i + 1).c_str());
                break;
            case GDT_CFloat32:
                type = gta::cfloat32;
                break;
            case GDT_CFloat64:
                type = gta::cfloat64;
                break;
            case GDT_Unknown:
            default:
                throw exc("Cannot import " + ifilename, "file contains data of unknown type");
                break;
            }
            *types.ptr<gta::type>(i) = type;
        }
        hdr.set_components(components, types.ptr<gta::type>());
        scanlines.resize(sizeof(void *), checked_cast<size_t>(components));
        for (uintmax_t i = 0; i < components; i++)
        {
            band = GDALGetRasterBand(dataset, i + 1);
            int success;
            double value;
            value = GDALGetRasterOffset(band, &success);
            if (success)
            {
                hdr.component_taglist(i).set("GDAL/OFFSET", str::str(value).c_str());
            }
            value = GDALGetRasterScale(band, &success);
            if (success)
            {
                hdr.component_taglist(i).set("GDAL/SCALE", str::str(value).c_str());
            }
            value = GDALGetRasterNoDataValue(band, &success);
            if (success)
            {
                hdr.component_taglist(i).set("GDAL/NO_DATA_VALUE", str::str(value).c_str());
            }
            const char *unit = GDALGetRasterUnitType(band);
            if (unit && unit[0])
            {
                hdr.component_taglist(i).set("GDAL/UNIT_TYPE", unit);
            }
            hdr.component_taglist(i).set("GDAL/COLOR_INTERPRETATION",
                    GDALGetColorInterpretationName(GDALGetRasterColorInterpretation(band)));
            switch (GDALGetRasterColorInterpretation(band))
            {
            case GCI_GrayIndex:
                hdr.component_taglist(i).set("INTERPRETATION", "GRAY");
                break;
            case GCI_RedBand:
                hdr.component_taglist(i).set("INTERPRETATION", "RED");
                break;
            case GCI_GreenBand:
                hdr.component_taglist(i).set("INTERPRETATION", "GREEN");
                break;
            case GCI_BlueBand:
                hdr.component_taglist(i).set("INTERPRETATION", "BLUE");
                break;
            case GCI_AlphaBand:
                hdr.component_taglist(i).set("INTERPRETATION", "ALPHA");
                break;
            case GCI_HueBand:
                hdr.component_taglist(i).set("INTERPRETATION", "HSL/H");
                break;
            case GCI_SaturationBand:
                hdr.component_taglist(i).set("INTERPRETATION", "HSL/S");
                break;
            case GCI_LightnessBand:
                hdr.component_taglist(i).set("INTERPRETATION", "HSL/L");
                break;
            case GCI_CyanBand:
                hdr.component_taglist(i).set("INTERPRETATION", "CMYK/C");
                break;
            case GCI_MagentaBand:
                hdr.component_taglist(i).set("INTERPRETATION", "CMYK/M");
                break;
            case GCI_YellowBand:
                hdr.component_taglist(i).set("INTERPRETATION", "CMYK/Y");
                break;
            case GCI_BlackBand:
                hdr.component_taglist(i).set("INTERPRETATION", "CMYK/K");
                break;
            case GCI_YCbCr_YBand:
                hdr.component_taglist(i).set("INTERPRETATION", "YCBCR/Y");
                break;
            case GCI_YCbCr_CbBand:
                hdr.component_taglist(i).set("INTERPRETATION", "YCBCR/CB");
                break;
            case GCI_YCbCr_CrBand:
                hdr.component_taglist(i).set("INTERPRETATION", "YCBCR/CR");
                break;
            case GCI_PaletteIndex:
            default:
                // no tag fits
                break;
            }
            *scanlines.ptr<void *>(i) = CPLMalloc(hdr.component_size(i) * hdr.dimension_size(0));
            if (!*scanlines.ptr<void *>(i))
            {
                throw exc("Cannot import " + ifilename, ENOMEM);
            }
        }
        hdr.write_to(fo);
        gta::io_state so;
        dataline.resize(checked_cast<size_t>(hdr.element_size()), checked_cast<size_t>(hdr.dimension_size(0)));
        for (uintmax_t y = 0; y < hdr.dimension_size(1); y++)
        {
            for (uintmax_t i = 0; i < components; i++)
            {
                GDALDataType gdal_type;
                switch (hdr.component_type(i))
                {
                case gta::uint8:
                    gdal_type = GDT_Byte;
                    break;
                case gta::uint16:
                    gdal_type = GDT_UInt16;
                    break;
                case gta::int16:
                    gdal_type = GDT_Int16;
                    break;
                case gta::uint32:
                    gdal_type = GDT_UInt32;
                    break;
                case gta::int32:
                    gdal_type = GDT_Int32;
                    break;
                case gta::float32:
                    gdal_type = GDT_Float32;
                    break;
                case gta::float64:
                    gdal_type = GDT_Float64;
                    break;
                case gta::cfloat32:
                    gdal_type = GDT_CFloat32;
                    break;
                case gta::cfloat64:
                    gdal_type = GDT_CFloat64;
                    break;
                default:
                    throw exc("Cannot import " + ifilename, "Bug: impossible component type");
                    break;
                }
                band = GDALGetRasterBand(dataset, i + 1);
                if (GDALRasterIO(band, GF_Read, 0, y, hdr.dimension_size(0), 1,
                            *scanlines.ptr<void *>(i), hdr.dimension_size(0), 1, gdal_type, 0, 0) != CE_None)
                {
                    throw exc("Cannot import " + ifilename, EIO);
                }
            }
            for (uintmax_t x = 0; x < hdr.dimension_size(0); x++)
            {
                void *element = hdr.element(dataline.ptr(), x, 0);
                for (uintmax_t i = 0; i < components; i++)
                {
                    void *component = hdr.component(element, i);
                    memcpy(component,
                            static_cast<char *>(*scanlines.ptr<void *>(i)) + x * hdr.component_size(i),
                            hdr.component_size(i));
                }
            }
            hdr.write_elements(so, fo, hdr.dimension_size(0), dataline.ptr());
        }
        if (fo != stdout)
        {
            cio::close(fo);
        }
        for (uintmax_t i = 0; i < components; i++)
        {
            CPLFree(*scanlines.ptr<void *>(i));
        }
        GDALClose(dataset);
    }
    catch (std::exception &e)
    {
        msg::err("%s", e.what());
        return 1;
    }

    return 0;
}
