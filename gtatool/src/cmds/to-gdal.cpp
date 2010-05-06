/*
 * to-gdal.cpp
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
#include <cpl_string.h>

#include <gta/gta.hpp>

#include "msg.h"
#include "blob.h"
#include "cio.h"
#include "opt.h"
#include "debug.h"
#include "intcheck.h"


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

    FILE *fi = stdin;
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
        msg::err("%s", e.what());
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
            throw exc("Cannot export " + ifilename, "Only two-dimensional arrays can be exported to images");
        }
        if (hdr.dimension_size(0) > static_cast<uintmax_t>(std::numeric_limits<int>::max())
                || hdr.dimension_size(1) > static_cast<uintmax_t>(std::numeric_limits<int>::max()))
        {
            throw exc("Cannot export " + ifilename, "Array too large");
        }
        if (hdr.components() < 1 || hdr.components() >= static_cast<uintmax_t>(std::numeric_limits<int>::max()))
        {
            throw exc("Cannot export " + ifilename, "Unsupported number of components");
        }
        type = hdr.component_type(0);
        for (uintmax_t i = 1; i < hdr.components(); i++)
        {
            if (hdr.component_type(i) != type)
            {
                throw exc("Cannot export " + ifilename, "Array element components differ in type");
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
            throw exc("Cannot export " + ifilename, "Array element component type not supported by GDAL");
            break;
        }
        if (hdr.data_is_chunked())
        {
            throw exc("Cannot export " + ifilename, "Currently only uncompressed GTAs can be exported");
        }
        driver = GDALGetDriverByName(format.value().c_str());
        if (!driver)
        {
            throw exc("Cannot export " + ifilename, std::string("GDAL does not know the format ") + format.value());
        }
        driver_metadata = GDALGetMetadata(driver, NULL);
        if (!CSLFetchBoolean(driver_metadata, GDAL_DCAP_CREATE, FALSE))
        {
            throw exc("Cannot export " + ifilename, std::string("The GDAL format driver ") 
                    + format.value() + std::string(" does not support the creation of files with the Create() method"));
        }
        dataset = GDALCreate(driver, ofilename.c_str(), hdr.dimension_size(0), hdr.dimension_size(1), 
                hdr.components(), gdal_type, driver_options);
        if (!dataset)
        {
            throw exc("Cannot export " + ifilename, "GDAL failed to create a data set");
        }
        if (hdr.global_taglist().get("GDAL/GEO_TRANSFORM"))
        {
            double geo_transform[6];
            if (sscanf(hdr.global_taglist().get("GDAL/GEO_TRANSFORM"), "%lf %lf %lf %lf %lf %lf",
                        geo_transform + 0, geo_transform + 1, geo_transform + 2,
                        geo_transform + 3, geo_transform + 4, geo_transform + 5) != 6
                    || GDALSetGeoTransform(dataset, geo_transform) != CE_None)
            {
                msg::wrn("GTA contains invalid GDAL/GEO_TRANSFORM information");
            }
        }
        if (hdr.global_taglist().get("GDAL/PROJECTION"))
        {
            if (GDALSetProjection(dataset, hdr.global_taglist().get("GDAL/PROJECTION")) != CE_None)
            {
                msg::wrn("GTA contains invalid GDAL/PROJECTION information");
            }
        }
        for (uintmax_t t = 0; t < hdr.global_taglist().tags(); t++)
        {
            std::string name(hdr.global_taglist().name(t));
            if (name.length() > 14 && name.compare(0, 14, "GDAL/METADATA/") == 0)
            {
                if (GDALSetMetadataItem(dataset, name.substr(14).c_str(), hdr.global_taglist().value(t), NULL) != CE_None)
                {
                    msg::wrn("GTA contains invalid GDAL/METADATA/* information");
                }
            }
        }
        for (uintmax_t i = 0; i < hdr.components(); i++)
        {
            band = GDALGetRasterBand(dataset, i + 1);
            double value;
            if (hdr.component_taglist(i).get("GDAL/OFFSET"))
            {
                if (sscanf(hdr.component_taglist(i).get("GDAL/OFFSET"), "%lf", &value) != 1
                        || GDALSetRasterOffset(band, value) != CE_None)
                {
                    msg::wrn(std::string("GTA component ") + str::str(i) + " contains invalid GDAL/OFFSET information");
                }
            }
            if (hdr.component_taglist(i).get("GDAL/SCALE"))
            {
                if (sscanf(hdr.component_taglist(i).get("GDAL/SCALE"), "%lf", &value) != 1
                        || GDALSetRasterScale(band, value) != CE_None)
                {
                    msg::wrn(std::string("GTA component ") + str::str(i) + " contains invalid GDAL/SCALE information");
                }
            }
            if (hdr.component_taglist(i).get("GDAL/NO_DATA_VALUE"))
            {
                if (sscanf(hdr.component_taglist(i).get("GDAL/NO_DATA_VALUE"), "%lf", &value) != 1
                        || GDALSetRasterNoDataValue(band, value) != CE_None)
                {
                    msg::wrn(std::string("GTA component ") + str::str(i) + " contains invalid GDAL/NO_DATA_VALUE information");
                }
            }
            /* FIXME: There is no GDALSetRasterUnitType() function?!
            if (hdr.component_taglist(i).get("GDAL/UNIT_TYPE"))
            {
                if (GDALSetRasterUnitType(band, hdr.component_taglist(i).get("GDAL/UNIT_TYPE")) != CE_None)
                {
                    msg::wrn(std::string("GTA component ") + str::str(i) + " contains invalid GDAL/UNIT_TYPE information");
                }
            }
            */
            GDALColorInterp ci;
            bool have_ci = false;
            if (hdr.component_taglist(i).get("GDAL/COLOR_INTERPRETATION"))
            {
                const char *interpretation = hdr.component_taglist(i).get("GDAL/COLOR_INTERPRETATION");
                if (strcmp(interpretation, "Gray") == 0)
                {
                    ci = GCI_GrayIndex;
                    have_ci = true;
                }
                else if (strcmp(interpretation, "Red") == 0)
                {
                    ci = GCI_RedBand;
                    have_ci = true;
                }
                else if (strcmp(interpretation, "Green") == 0)
                {
                    ci = GCI_GreenBand;
                    have_ci = true;
                }
                else if (strcmp(interpretation, "Blue") == 0)
                {
                    ci = GCI_BlueBand;
                    have_ci = true;
                }
                else if (strcmp(interpretation, "Alpha") == 0)
                {
                    ci = GCI_AlphaBand;
                    have_ci = true;
                }
                else if (strcmp(interpretation, "Hue") == 0)
                {
                    ci = GCI_HueBand;
                    have_ci = true;
                }
                else if (strcmp(interpretation, "Saturation") == 0)
                {
                    ci = GCI_SaturationBand;
                    have_ci = true;
                }
                else if (strcmp(interpretation, "Lightness") == 0)
                {
                    ci = GCI_LightnessBand;
                    have_ci = true;
                }
                else if (strcmp(interpretation, "Cyan") == 0)
                {
                    ci = GCI_CyanBand;
                    have_ci = true;
                }
                else if (strcmp(interpretation, "Magenta") == 0)
                {
                    ci = GCI_MagentaBand;
                    have_ci = true;
                }
                else if (strcmp(interpretation, "Yellow") == 0)
                {
                    ci = GCI_YellowBand;
                    have_ci = true;
                }
                else if (strcmp(interpretation, "Black") == 0)
                {
                    ci = GCI_BlackBand;
                    have_ci = true;
                }
                else if (strcmp(interpretation, "YCbCr_Y") == 0)
                {
                    ci = GCI_YCbCr_YBand;
                    have_ci = true;
                }
                else if (strcmp(interpretation, "YCbCr_Cb") == 0)
                {
                    ci = GCI_YCbCr_CbBand;
                    have_ci = true;
                }
                else if (strcmp(interpretation, "YCbCr_Cr") == 0)
                {
                    ci = GCI_YCbCr_CrBand;
                    have_ci = true;
                }
                else
                {
                    msg::wrn(std::string("GTA component ") + str::str(i) + " contains invalid GDAL/COLOR_INTERPRETATION information");
                }
            }
            else if (hdr.component_taglist(i).get("INTERPRETATION"))
            {
                const char *interpretation = hdr.component_taglist(i).get("INTERPRETATION");
                if (strcmp(interpretation, "GRAY") == 0)
                {
                    ci = GCI_GrayIndex;
                    have_ci = true;
                }
                else if (strcmp(interpretation, "RED") == 0)
                {
                    ci = GCI_RedBand;
                    have_ci = true;
                }
                else if (strcmp(interpretation, "GREEN") == 0)
                {
                    ci = GCI_GreenBand;
                    have_ci = true;
                }
                else if (strcmp(interpretation, "BLUE") == 0)
                {
                    ci = GCI_BlueBand;
                    have_ci = true;
                }
                else if (strcmp(interpretation, "ALPHA") == 0)
                {
                    ci = GCI_AlphaBand;
                    have_ci = true;
                }
                else if (strcmp(interpretation, "HSL/H") == 0)
                {
                    ci = GCI_HueBand;
                    have_ci = true;
                }
                else if (strcmp(interpretation, "HSL/S") == 0)
                {
                    ci = GCI_SaturationBand;
                    have_ci = true;
                }
                else if (strcmp(interpretation, "HSL/L") == 0)
                {
                    ci = GCI_LightnessBand;
                    have_ci = true;
                }
                else if (strcmp(interpretation, "CMYK/C") == 0)
                {
                    ci = GCI_CyanBand;
                    have_ci = true;
                }
                else if (strcmp(interpretation, "CMYK/M") == 0)
                {
                    ci = GCI_MagentaBand;
                    have_ci = true;
                }
                else if (strcmp(interpretation, "CMYK/Y") == 0)
                {
                    ci = GCI_YellowBand;
                    have_ci = true;
                }
                else if (strcmp(interpretation, "CMYK/K") == 0)
                {
                    ci = GCI_BlackBand;
                    have_ci = true;
                }
                else if (strcmp(interpretation, "YCBCR/Y") == 0)
                {
                    ci = GCI_YCbCr_YBand;
                    have_ci = true;
                }
                else if (strcmp(interpretation, "YCBCR/CB") == 0)
                {
                    ci = GCI_YCbCr_CbBand;
                    have_ci = true;
                }
                else if (strcmp(interpretation, "YCBCR/CR") == 0)
                {
                    ci = GCI_YCbCr_CrBand;
                    have_ci = true;
                }
            }
            if (have_ci)
            {
                GDALSetRasterColorInterpretation(band, ci);
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
        if (fi != stdin)
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
        msg::err("%s", e.what());
        return 1;
    }

    return 0;
}
