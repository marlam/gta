/*
 * This file is part of gtatool, a tool to manipulate Generic Tagged Arrays
 * (GTAs).
 *
 * Copyright (C) 2010, 2011
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
#include <cstring>

#include <gdal.h>
#include <cpl_conv.h>
#include <cpl_string.h>

#include <gta/gta.hpp>

#include "msg.h"
#include "blob.h"
#include "cio.h"
#include "opt.h"
#include "debug.h"
#include "intcheck.h"

#include "lib.h"


extern "C" void gtatool_to_gdal_help(void)
{
    msg::req_txt("to-gdal [--format=<format>] [<input-file>] <output-file>\n"
            "\n"
            "Converts GTAs to a format supported by GDAL. The default format is GTiff.");
}

extern "C" int gtatool_to_gdal(int argc, char *argv[])
{
    std::vector<opt::option *> options;
    opt::info help("help", '\0', opt::optional);
    options.push_back(&help);
    opt::string format("format", '\0', opt::optional, "GTiff");
    options.push_back(&format);
    std::vector<std::string> arguments;
    if (!opt::parse(argc, argv, options, 1, 2, arguments))
    {
        return 1;
    }
    if (help.value())
    {
        gtatool_to_gdal_help();
        return 0;
    }

    FILE *fi = gtatool_stdin;
    std::string ifilename("standard input");
    std::string ofilename(arguments[0]);
    try
    {
        if (arguments.size() == 2)
        {
            ifilename = arguments[0];
            fi = cio::open(ifilename, "r");
            ofilename = arguments[1];
        }
    }
    catch (std::exception &e)
    {
        msg::err_txt("%s", e.what());
        return 1;
    }

    try
    {
        // GTA
        gta::header hdr;
        gta::type type;
        blob dataline;
        // GDAL
        GDALDataType gdal_type;
        GDALDriverH driver; 
        char **driver_metadata;
        char **driver_options = NULL;   // TODO: Allow to give driver options on the command line?
        GDALDatasetH dataset;
        GDALRasterBandH band;
        blob scanlines;

        GDALAllRegister();
        hdr.read_from(fi);
        if (hdr.dimensions() != 2)
        {
            throw exc("cannot export " + ifilename + ": only two-dimensional arrays can be exported to images");
        }
        if (hdr.dimension_size(0) > static_cast<uintmax_t>(std::numeric_limits<int>::max())
                || hdr.dimension_size(1) > static_cast<uintmax_t>(std::numeric_limits<int>::max()))
        {
            throw exc("cannot export " + ifilename + ": array too large");
        }
        if (hdr.components() < 1 || hdr.components() >= static_cast<uintmax_t>(std::numeric_limits<int>::max()))
        {
            throw exc("cannot export " + ifilename + ": unsupported number of components");
        }
        type = hdr.component_type(0);
        for (uintmax_t i = 1; i < hdr.components(); i++)
        {
            if (hdr.component_type(i) != type)
            {
                throw exc("cannot export " + ifilename + ": array element components differ in type");
            }
        }
        switch (type)
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
            throw exc("cannot export " + ifilename + ": array element component type not supported by GDAL");
            break;
        }
        if (hdr.compression() != gta::none)
        {
            throw exc("cannot export " + ifilename + ": currently only uncompressed GTAs can be exported");
        }
        driver = GDALGetDriverByName(format.value().c_str());
        if (!driver)
        {
            throw exc("cannot export " + ifilename + ": GDAL does not know the format " + format.value());
        }
        driver_metadata = GDALGetMetadata(driver, NULL);
        if (!CSLFetchBoolean(driver_metadata, GDAL_DCAP_CREATE, FALSE))
        {
            throw exc("cannot export " + ifilename + ": the GDAL format driver "
                    + format.value() + " does not support the creation of files with the Create() method");
        }
        dataset = GDALCreate(driver, ofilename.c_str(), hdr.dimension_size(0), hdr.dimension_size(1), 
                hdr.components(), gdal_type, driver_options);
        if (!dataset)
        {
            throw exc("cannot export " + ifilename + ": GDAL failed to create a data set");
        }
        if (hdr.global_taglist().get("DESCRIPTION"))
        {
            GDALSetDescription(dataset, hdr.global_taglist().get("DESCRIPTION"));
        }
        if (hdr.global_taglist().get("GDAL/GEO_TRANSFORM"))
        {
            double geo_transform[6];
            if (sscanf(hdr.global_taglist().get("GDAL/GEO_TRANSFORM"), "%lf %lf %lf %lf %lf %lf",
                        geo_transform + 0, geo_transform + 1, geo_transform + 2,
                        geo_transform + 3, geo_transform + 4, geo_transform + 5) != 6
                    || GDALSetGeoTransform(dataset, geo_transform) != CE_None)
            {
                msg::wrn_txt("GTA contains invalid GDAL/GEO_TRANSFORM information");
            }
        }
        if (hdr.global_taglist().get("GDAL/PROJECTION"))
        {
            if (GDALSetProjection(dataset, hdr.global_taglist().get("GDAL/PROJECTION")) != CE_None)
            {
                msg::wrn_txt("GTA contains invalid GDAL/PROJECTION information");
            }
        }
        for (uintmax_t i = 0; i < hdr.global_taglist().tags(); i++)
        {
            const char *tag_name = hdr.global_taglist().name(i);
            if (strncmp(tag_name, "GDAL/META/", 10 ) == 0)
            {
                const char *domain_end = strchr(tag_name + 10, '/' );
                if (domain_end && domain_end - (tag_name + 10) > 0)
                {
                    std::string domain = std::string(tag_name + 10).substr(0, domain_end - (tag_name + 10));
                    const char *name = tag_name + 10 + domain.length() + 1;
                    const char *value = hdr.global_taglist().value(i);
                    GDALSetMetadataItem(dataset, name, value,
                            domain == "DEFAULT" ? NULL : domain.c_str());
                }
            }
        }
        if (hdr.global_taglist().get("GDAL/GCP_COUNT"))
        {
            int gcp_count = str::to<int>(hdr.global_taglist().get("GDAL/GCP_COUNT"));
            if (gcp_count > 0)
            {
                std::vector<GDAL_GCP> gcps;
                gcps.resize(gcp_count);
                for( int i = 0; i < gcp_count; i++ )
                {
                    gcps[i].dfGCPPixel = 0.0;
                    gcps[i].dfGCPLine = 0.0;
                    gcps[i].dfGCPX = 0.0;
                    gcps[i].dfGCPY = 0.0;
                    gcps[i].dfGCPZ = 0.0;
                    gcps[i].pszId = strdup(str::from(i).c_str());
                    const char *info = hdr.global_taglist().get((std::string("GDAL/GCP") + str::from(i) + "_INFO").c_str());
                    gcps[i].pszInfo = strdup(info ? info : "");
                    const char *gcp = hdr.global_taglist().get((std::string("GDAL/GCP") + str::from(i)).c_str());
                    if (gcp)
                    {
                        if (sscanf(gcp, "%lf %lf %lf %lf %lf",
                                    &gcps[i].dfGCPPixel, &gcps[i].dfGCPLine, &gcps[i].dfGCPX, &gcps[i].dfGCPY, &gcps[i].dfGCPZ) != 5)
                        {
                            msg::wrn_txt("GTA contains invalid GDAL/GCP%d information", i);
                        }
                    }
                    if (GDALSetGCPs(dataset, gcp_count, &(gcps[0]),
                                hdr.global_taglist().get("GDAL/GCP_PROJECTION") ?
                                hdr.global_taglist().get("GDAL/GCP_PROJECTION") : "") != CE_None)
                    {
                        msg::wrn_txt("GTA contains invalid GCP information");
                    }
                }
            }
        }
        for (uintmax_t i = 0; i < hdr.components(); i++)
        {
            band = GDALGetRasterBand(dataset, i + 1);
            if (hdr.component_taglist(i).get("DESCRIPTION"))
            {
                GDALSetDescription(band, hdr.component_taglist(i).get("DESCRIPTION"));
            }
            double value;
            if (hdr.component_taglist(i).get("GDAL/OFFSET"))
            {
                if (sscanf(hdr.component_taglist(i).get("GDAL/OFFSET"), "%lf", &value) != 1
                        || GDALSetRasterOffset(band, value) != CE_None)
                {
                    msg::wrn_txt(std::string("GTA component ") + str::from(i) + " contains invalid GDAL/OFFSET information");
                }
            }
            if (hdr.component_taglist(i).get("GDAL/SCALE"))
            {
                if (sscanf(hdr.component_taglist(i).get("GDAL/SCALE"), "%lf", &value) != 1
                        || GDALSetRasterScale(band, value) != CE_None)
                {
                    msg::wrn_txt(std::string("GTA component ") + str::from(i) + " contains invalid GDAL/SCALE information");
                }
            }
            if (hdr.component_taglist(i).get("NO_DATA_VALUE"))
            {
                if (sscanf(hdr.component_taglist(i).get("NO_DATA_VALUE"), "%lf", &value) != 1
                        || GDALSetRasterNoDataValue(band, value) != CE_None)
                {
                    msg::wrn_txt(std::string("GTA component ") + str::from(i) + " contains invalid NO_DATA_VALUE information");
                }
            }
#if GDAL_VERSION_NUM >= 1800
            /* GDALSetRasterUnitType() is not available before GDAL 1.8.0 */
            if (hdr.component_taglist(i).get("UNIT"))
            {
                if (GDALSetRasterUnitType(band, hdr.component_taglist(i).get("UNIT")) != CE_None)
                {
                    msg::wrn_txt(std::string("GTA component ") + str::from(i) + " contains invalid UNIT information");
                }
            }
#endif
            int category_count = 0;
            if (hdr.component_taglist(i).get("GDAL/CATEGORY_COUNT")
                    && (category_count = str::to<int>(hdr.component_taglist(i).get("GDAL/CATEGORY_COUNT"))) > 0)
            {
                std::vector<char *> category_names(category_count + 1);
                for (int j = 0; j < category_count; j++)
                {
                    const char *name = hdr.component_taglist(i).get((std::string("GDAL/CATEGORY") + str::from(j)).c_str());
                    category_names[j] = strdup(name ? name : "");
                }
                category_names[category_names.size() - 1] = NULL;
                if (GDALSetRasterCategoryNames(band, &(category_names[0])) != CE_None)
                {
                    msg::wrn_txt(std::string("Cannot set category names for GTA component ") + str::from(i));
                }
            }
            const char *interpretation = hdr.component_taglist(i).get("INTERPRETATION");
            if (interpretation)
            {
                if (strcmp(interpretation, "GRAY") == 0)
                    GDALSetRasterColorInterpretation(band, GCI_GrayIndex);
                else if (strcmp(interpretation, "RED") == 0)
                    GDALSetRasterColorInterpretation(band, GCI_RedBand);
                else if (strcmp(interpretation, "GREEN") == 0)
                    GDALSetRasterColorInterpretation(band, GCI_GreenBand);
                else if (strcmp(interpretation, "BLUE") == 0)
                    GDALSetRasterColorInterpretation(band, GCI_BlueBand);
                else if (strcmp(interpretation, "ALPHA") == 0)
                    GDALSetRasterColorInterpretation(band, GCI_AlphaBand);
                else if (strcmp(interpretation, "HSL/H") == 0)
                    GDALSetRasterColorInterpretation(band, GCI_HueBand);
                else if (strcmp(interpretation, "HSL/S") == 0)
                    GDALSetRasterColorInterpretation(band, GCI_SaturationBand);
                else if (strcmp(interpretation, "HSL/L") == 0)
                    GDALSetRasterColorInterpretation(band, GCI_LightnessBand);
                else if (strcmp(interpretation, "CMYK/C") == 0)
                    GDALSetRasterColorInterpretation(band, GCI_CyanBand);
                else if (strcmp(interpretation, "CMYK/M") == 0)
                    GDALSetRasterColorInterpretation(band, GCI_MagentaBand);
                else if (strcmp(interpretation, "CMYK/Y") == 0)
                    GDALSetRasterColorInterpretation(band, GCI_YellowBand);
                else if (strcmp(interpretation, "CMYK/K") == 0)
                    GDALSetRasterColorInterpretation(band, GCI_BlackBand);
                else if (strcmp(interpretation, "YCBCR/Y") == 0)
                    GDALSetRasterColorInterpretation(band, GCI_YCbCr_YBand);
                else if (strcmp(interpretation, "YCBCR/CB") == 0)
                    GDALSetRasterColorInterpretation(band, GCI_YCbCr_CbBand);
                else if (strcmp(interpretation, "YCBCR/CR") == 0)
                    GDALSetRasterColorInterpretation(band, GCI_YCbCr_CrBand);
            }
        }
        dataline.resize(checked_cast<size_t>(hdr.element_size()), checked_cast<size_t>(hdr.dimension_size(0)));
        scanlines.resize(sizeof(void *), checked_cast<size_t>(hdr.components()));
        for (uintmax_t i = 0; i < hdr.components(); i++)
        {
            *scanlines.ptr<void *>(i) = CPLMalloc(hdr.component_size(i) * hdr.dimension_size(0));
            if (!*scanlines.ptr<void *>(i))
            {
                throw exc("Cannot export " + ifilename, ENOMEM);
            }
        }
        gta::io_state si;
        for (uintmax_t y = 0; y < hdr.dimension_size(1); y++)
        {
            hdr.read_elements(si, fi, hdr.dimension_size(0), dataline.ptr());
            for (uintmax_t x = 0; x < hdr.dimension_size(0); x++)
            {
                const void *element = hdr.element(dataline.ptr(), x, 0);
                for (uintmax_t i = 0; i < hdr.components(); i++)
                {
                    const void *component = hdr.component(element, i);
                    memcpy(static_cast<char *>(*scanlines.ptr<void *>(i)) + x * hdr.component_size(i),
                            component, hdr.component_size(i));
                }
            }
            for (uintmax_t i = 0; i < hdr.components(); i++)
            {
                band = GDALGetRasterBand(dataset, i + 1);
                if (GDALRasterIO(band, GF_Write, 0, y, hdr.dimension_size(0), 1,
                            *scanlines.ptr<void *>(i), hdr.dimension_size(0), 1, gdal_type, 0, 0) != CE_None)
                {
                    throw exc("Cannot export " + ifilename, EIO);
                }
            }
        }
        if (fi != gtatool_stdin)
        {
            cio::close(fi);
        }
        for (uintmax_t i = 0; i < hdr.components(); i++)
        {
            CPLFree(*scanlines.ptr<void *>(i));
        }
        GDALClose(dataset);
    }
    catch (std::exception &e)
    {
        msg::err_txt("%s", e.what());
        return 1;
    }

    return 0;
}
