/*
 * This file is part of gtatool, a tool to manipulate Generic Tagged Arrays
 * (GTAs).
 *
 * Copyright (C) 2011, 2012, 2013
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

#include "base/msg.h"
#include "base/blb.h"
#include "base/fio.h"
#include "base/opt.h"
#include "base/str.h"
#include "base/chk.h"

#include "lib.h"

extern "C" void gtatool_from_pcd_help(void)
{
    msg::req_txt("from-pcd <input-file> [<output-file>]\n"
            "\n"
            "Converts PCD files to GTAs.\n"
            "Currently only combinations of XYZ, normal, intensity, RGB/RGBA are supported.");
}

bool have_field(const sensor_msgs::PointCloud2& cloud_blob, const char* name)
{
    for (size_t i = 0; i < cloud_blob.fields.size(); i++)
        if (cloud_blob.fields[i].name == std::string(name))
            return true;
    return false;
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
        bool have_xyz = have_field(cloud_blob, "x") && have_field(cloud_blob, "y") && have_field(cloud_blob, "z");
        bool have_intensity = have_field(cloud_blob, "intensity");
        bool have_rgb = have_field(cloud_blob, "rgb");
        bool have_rgba = have_field(cloud_blob, "rgba");
        bool have_normal = have_field(cloud_blob, "normal_x") && have_field(cloud_blob, "normal_y") && have_field(cloud_blob, "normal_z");
        gta::header hdr;
        std::string nameo;
        blob element;
        if (have_xyz && !have_intensity && !have_rgb && !have_rgba && !have_normal)
        {
            // pcl::PointXYZ
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
        else if (have_xyz && have_intensity && !have_rgb && !have_rgba && !have_normal)
        {
            // pcl::PointXYZI
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
        else if (have_xyz && !have_intensity && have_rgb && !have_rgba && !have_normal)
        {
            // pcl::PointXYZRGB
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
        else if (have_xyz && !have_intensity && !have_rgb && have_rgba && !have_normal)
        {
            // pcl::PointXYZRGBA
            pcl::PointCloud<pcl::PointXYZRGBA> cloud;
            pcl::fromROSMsg(cloud_blob, cloud);
            hdr.set_dimensions(cloud.points.size());
            std::vector<gta::type> types(3, gta::float32);
            types.resize(7, gta::uint8);
            hdr.set_components(7, &(types[0]));
            hdr.component_taglist(0).set("INTERPRETATION", "X");
            hdr.component_taglist(1).set("INTERPRETATION", "Y");
            hdr.component_taglist(2).set("INTERPRETATION", "Z");
            hdr.component_taglist(3).set("INTERPRETATION", "RED");
            hdr.component_taglist(4).set("INTERPRETATION", "GREEN");
            hdr.component_taglist(5).set("INTERPRETATION", "BLUE");
            hdr.component_taglist(6).set("INTERPRETATION", "ALPHA");
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
                std::memcpy(&color, &(cloud.points[i].rgba), sizeof(uint32_t));
                uint8_t r = (color >> 16) & 0xff;
                uint8_t g = (color >> 8) & 0xff;
                uint8_t b = color & 0xff;
                uint8_t a = (color >> 24) & 0xff;
                std::memcpy(element.ptr<uint8_t>(12), &r, sizeof(uint8_t));
                std::memcpy(element.ptr<uint8_t>(13), &g, sizeof(uint8_t));
                std::memcpy(element.ptr<uint8_t>(14), &b, sizeof(uint8_t));
                std::memcpy(element.ptr<uint8_t>(15), &a, sizeof(uint8_t));
                element_loop.write(element.ptr());
            }
        }
        else if (have_xyz && !have_intensity && !have_rgb && !have_rgba && have_normal)
        {
            // pcl::PointNormal
            pcl::PointCloud<pcl::PointNormal> cloud;
            pcl::fromROSMsg(cloud_blob, cloud);
            hdr.set_dimensions(cloud.points.size());
            std::vector<gta::type> types(6, gta::float32);
            hdr.set_components(6, &(types[0]));
            hdr.component_taglist(0).set("INTERPRETATION", "X");
            hdr.component_taglist(1).set("INTERPRETATION", "Y");
            hdr.component_taglist(2).set("INTERPRETATION", "Z");
            hdr.component_taglist(3).set("INTERPRETATION", "X-NORMAL-X");
            hdr.component_taglist(4).set("INTERPRETATION", "X-NORMAL-Y");
            hdr.component_taglist(5).set("INTERPRETATION", "X-NORMAL-Z");
            array_loop.write(hdr, nameo);
            element.resize(hdr.element_size());
            element_loop_t element_loop;
            array_loop.start_element_loop(element_loop, gta::header(), hdr);
            for (size_t i = 0; i < cloud.points.size(); i++)
            {
                element.ptr<float>()[0] = cloud.points[i].x;
                element.ptr<float>()[1] = cloud.points[i].y;
                element.ptr<float>()[2] = cloud.points[i].z;
                element.ptr<float>()[3] = cloud.points[i].normal_x;
                element.ptr<float>()[4] = cloud.points[i].normal_y;
                element.ptr<float>()[5] = cloud.points[i].normal_z;
                element_loop.write(element.ptr());
            }
        }
        else if (have_xyz && have_intensity && !have_rgb && !have_rgba && have_normal)
        {
            // pcl::PointXYZINormal
            pcl::PointCloud<pcl::PointXYZINormal> cloud;
            pcl::fromROSMsg(cloud_blob, cloud);
            hdr.set_dimensions(cloud.points.size());
            std::vector<gta::type> types(7, gta::float32);
            hdr.set_components(7, &(types[0]));
            hdr.component_taglist(0).set("INTERPRETATION", "X");
            hdr.component_taglist(1).set("INTERPRETATION", "Y");
            hdr.component_taglist(2).set("INTERPRETATION", "Z");
            hdr.component_taglist(3).set("INTERPRETATION", "X-NORMAL-X");
            hdr.component_taglist(4).set("INTERPRETATION", "X-NORMAL-Y");
            hdr.component_taglist(5).set("INTERPRETATION", "X-NORMAL-Z");
            array_loop.write(hdr, nameo);
            element.resize(hdr.element_size());
            element_loop_t element_loop;
            array_loop.start_element_loop(element_loop, gta::header(), hdr);
            for (size_t i = 0; i < cloud.points.size(); i++)
            {
                element.ptr<float>()[0] = cloud.points[i].x;
                element.ptr<float>()[1] = cloud.points[i].y;
                element.ptr<float>()[2] = cloud.points[i].z;
                element.ptr<float>()[3] = cloud.points[i].normal_x;
                element.ptr<float>()[4] = cloud.points[i].normal_y;
                element.ptr<float>()[5] = cloud.points[i].normal_z;
                element.ptr<float>()[6] = cloud.points[i].intensity;
                element_loop.write(element.ptr());
            }
        }
        else if (have_xyz && !have_intensity && have_rgb && !have_rgba && have_normal)
        {
            // pcl::PointXYZRGBNormal
            pcl::PointCloud<pcl::PointXYZRGBNormal> cloud;
            pcl::fromROSMsg(cloud_blob, cloud);
            hdr.set_dimensions(cloud.points.size());
            std::vector<gta::type> types(6, gta::float32);
            types.resize(9, gta::uint8);
            hdr.set_components(9, &(types[0]));
            hdr.component_taglist(0).set("INTERPRETATION", "X");
            hdr.component_taglist(1).set("INTERPRETATION", "Y");
            hdr.component_taglist(2).set("INTERPRETATION", "Z");
            hdr.component_taglist(3).set("INTERPRETATION", "X-NORMAL-X");
            hdr.component_taglist(4).set("INTERPRETATION", "X-NORMAL-Y");
            hdr.component_taglist(5).set("INTERPRETATION", "X-NORMAL-Z");
            hdr.component_taglist(6).set("INTERPRETATION", "RED");
            hdr.component_taglist(7).set("INTERPRETATION", "GREEN");
            hdr.component_taglist(8).set("INTERPRETATION", "BLUE");
            array_loop.write(hdr, nameo);
            element.resize(hdr.element_size());
            element_loop_t element_loop;
            array_loop.start_element_loop(element_loop, gta::header(), hdr);
            for (size_t i = 0; i < cloud.points.size(); i++)
            {
                element.ptr<float>()[0] = cloud.points[i].x;
                element.ptr<float>()[1] = cloud.points[i].y;
                element.ptr<float>()[2] = cloud.points[i].z;
                element.ptr<float>()[3] = cloud.points[i].normal_x;
                element.ptr<float>()[4] = cloud.points[i].normal_y;
                element.ptr<float>()[5] = cloud.points[i].normal_z;
                assert(sizeof(float) == sizeof(uint32_t));
                uint32_t color;
                std::memcpy(&color, &(cloud.points[i].rgb), sizeof(uint32_t));
                uint8_t r = (color >> 16) & 0xff;
                uint8_t g = (color >> 8) & 0xff;
                uint8_t b = color & 0xff;
                std::memcpy(element.ptr<uint8_t>(24), &r, sizeof(uint8_t));
                std::memcpy(element.ptr<uint8_t>(25), &g, sizeof(uint8_t));
                std::memcpy(element.ptr<uint8_t>(26), &b, sizeof(uint8_t));
                element_loop.write(element.ptr());
            }
        }
        else
        {
            throw exc(namei + ": unsupported point type or attributes.");
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
