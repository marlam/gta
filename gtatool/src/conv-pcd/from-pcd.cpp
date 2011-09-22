/*
 * This file is part of gtatool, a tool to manipulate Generic Tagged Arrays
 * (GTAs).
 *
 * Copyright (C) 2011
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

#include <pcl/io/pcd_io.h>
#include <pcl/point_types.h>

#include "msg.h"
#include "blob.h"
#include "cio.h"
#include "opt.h"
#include "str.h"
#include "intcheck.h"

#include "lib.h"

extern "C" void gtatool_from_pcd_help(void)
{
    msg::req_txt("from-pcd <input-file> [<output-file>]\n"
            "\n"
            "Converts PCD files to GTAs.\n"
            "Currently only XYZ, XYZI, and XYZRGB data is supported.");
}

extern "C" int gtatool_from_pcd(int argc, char *argv[])
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
        gtatool_from_pcd_help();
        return 0;
    }

    try
    {
        std::string namei = arguments[0];
        array_loop_t array_loop;
        array_loop.start(std::vector<std::string>(1, namei), arguments.size() == 2 ? arguments[1] : "");

        sensor_msgs::PointCloud2 cloud_blob;
        if (pcl::io::loadPCDFile(namei.c_str(), cloud_blob) == -1)
        {
            throw exc(namei + ": cannot read file.");
        }
        // Find out what type of PCD we have.
        // XXX: There *must* be a better way to do this, but I cannot find one in the PCL documentation.
        bool have_x = false, have_y = false, have_z = false, have_intensity = false, have_rgb = false;
        for (size_t i = 0; i < cloud_blob.fields.size(); i++)
        {
            have_x = have_x || (cloud_blob.fields[i].name == "x");
            have_y = have_y || (cloud_blob.fields[i].name == "y");
            have_z = have_z || (cloud_blob.fields[i].name == "z");
            have_intensity = have_intensity || (cloud_blob.fields[i].name == "intensity");
            have_rgb = have_rgb || (cloud_blob.fields[i].name == "rgb");
        }
        int components = 
            (have_x && have_y && have_z && have_rgb) ? 6
            : (have_x && have_y && have_z && have_intensity) ? 4
            : (have_x && have_y && have_z) ? 3 : -1;
        if (components == -1)
        {
            throw exc(namei + ": unsupported components.");
        }

        gta::header hdr;
        std::string nameo;
        blob element;
        if (components == 3)
        {
            pcl::PointCloud<pcl::PointXYZ> cloud;
            pcl::fromROSMsg(cloud_blob, cloud);
            hdr.set_dimensions(cloud.points.size());
            hdr.set_components(gta::float32, gta::float32, gta::float32);
            hdr.component_taglist(0).set("INTERPRETATION", "X");
            hdr.component_taglist(1).set("INTERPRETATION", "Y");
            hdr.component_taglist(2).set("INTERPRETATION", "Z");
            array_loop.write(hdr, nameo);
            element.resize(hdr.element_size());
            element_loop_t element_loop;
            array_loop.start_element_loop(element_loop, gta::header(), hdr);
            for (size_t i = 0; i < cloud.points.size(); i++)
            {
                element.ptr<float>()[0] = cloud.points[i].x;
                element.ptr<float>()[1] = cloud.points[i].y;
                element.ptr<float>()[2] = cloud.points[i].z;
                element_loop.write(element.ptr());
            }
        }
        else if (components == 4)
        {
            pcl::PointCloud<pcl::PointXYZI> cloud;
            pcl::fromROSMsg(cloud_blob, cloud);
            hdr.set_dimensions(cloud.points.size());
            hdr.set_components(gta::float32, gta::float32, gta::float32, gta::float32);
            hdr.component_taglist(0).set("INTERPRETATION", "X");
            hdr.component_taglist(1).set("INTERPRETATION", "Y");
            hdr.component_taglist(2).set("INTERPRETATION", "Z");
            array_loop.write(hdr, nameo);
            element.resize(hdr.element_size());
            element_loop_t element_loop;
            array_loop.start_element_loop(element_loop, gta::header(), hdr);
            for (size_t i = 0; i < cloud.points.size(); i++)
            {
                element.ptr<float>()[0] = cloud.points[i].x;
                element.ptr<float>()[1] = cloud.points[i].y;
                element.ptr<float>()[2] = cloud.points[i].z;
                element.ptr<float>()[3] = cloud.points[i].intensity;
                element_loop.write(element.ptr());
            }
        }
        else if (components == 6)
        {
            pcl::PointCloud<pcl::PointXYZRGB> cloud;
            pcl::fromROSMsg(cloud_blob, cloud);
            hdr.set_dimensions(cloud.points.size());
            std::vector<gta::type> types(3, gta::float32);
            types.resize(6, gta::uint8);
            hdr.set_components(6, &(types[0]));
            hdr.component_taglist(0).set("INTERPRETATION", "X");
            hdr.component_taglist(1).set("INTERPRETATION", "Y");
            hdr.component_taglist(2).set("INTERPRETATION", "Z");
            hdr.component_taglist(3).set("INTERPRETATION", "RED");
            hdr.component_taglist(4).set("INTERPRETATION", "GREEN");
            hdr.component_taglist(5).set("INTERPRETATION", "BLUE");
            array_loop.write(hdr, nameo);
            element.resize(hdr.element_size());
            element_loop_t element_loop;
            array_loop.start_element_loop(element_loop, gta::header(), hdr);
            for (size_t i = 0; i < cloud.points.size(); i++)
            {
                element.ptr<float>()[0] = cloud.points[i].x;
                element.ptr<float>()[1] = cloud.points[i].y;
                element.ptr<float>()[2] = cloud.points[i].z;
                assert(sizeof(float) == sizeof(uint32_t));
                uint32_t color;
                std::memcpy(&color, &(cloud.points[i].rgb), sizeof(uint32_t));
                uint8_t r = (color >> 16) & 0xff;
                uint8_t g = (color >> 8) & 0xff;
                uint8_t b = color & 0xff;
                std::memcpy(element.ptr<uint8_t>(12), &r, sizeof(uint8_t));
                std::memcpy(element.ptr<uint8_t>(13), &g, sizeof(uint8_t));
                std::memcpy(element.ptr<uint8_t>(14), &b, sizeof(uint8_t));
                element_loop.write(element.ptr());
            }
        }
        else
        {
            throw exc(namei + ": unsupported number of point attributes.");
        }
        array_loop.finish();
    }
    catch (std::exception &e)
    {
        msg::err_txt("%s", e.what());
        return 1;
    }

    return 0;
}
