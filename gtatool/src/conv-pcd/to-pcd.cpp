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

extern "C" void gtatool_to_pcd_help(void)
{
    msg::req_txt("to-pcd [<input-file>] <output-file>\n"
            "\n"
            "Converts GTAs to the PCD format used by the Point Cloud Library.\n"
            "The input GTA must be one-dimensional (which means currently only "
            "unstructured point clouds are supported).\n"
            "Furthermore, only combinations of XYZ, normal, intensity, RGB/RGBA are supported.\n"
            "RGB/RGBA must use the uint8 type, everything else must use float32.\n"
            "The order of point attributes must be XYZ [NORMAL] [I|RGB|RGBA].");
}

extern "C" int gtatool_to_pcd(int argc, char *argv[])
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
        gtatool_to_pcd_help();
        return 0;
    }

    try
    {
        std::string nameo = arguments.size() == 1 ? arguments[0] : arguments[1];
        array_loop_t array_loop;
        gta::header hdr;
        std::string name;

        array_loop.start(arguments.size() == 1 ? std::vector<std::string>() : std::vector<std::string>(1, arguments[0]), nameo);
        while (array_loop.read(hdr, name))
        {
            if (hdr.dimensions() != 1)
            {
                throw exc(name + ": only one-dimensional arrays can be converted to PCD.");
            }
            if (hdr.components() == 3
                    && hdr.component_type(0) == gta::float32
                    && hdr.component_type(1) == gta::float32
                    && hdr.component_type(2) == gta::float32)
            {
                // pcl::PointXYZ
                pcl::PointCloud<pcl::PointXYZ> cloud;
                cloud.width = checked_cast<uint32_t>(hdr.elements());
                cloud.height = 1;
                cloud.is_dense = false;
                cloud.points.resize(cloud.width * cloud.height);
                element_loop_t element_loop;
                array_loop.start_element_loop(element_loop, hdr, gta::header());
                for (uintmax_t e = 0; e < hdr.elements(); e++)
                {
                    const float *element = static_cast<const float *>(element_loop.read());
                    cloud.points[e].x = element[0];
                    cloud.points[e].y = element[1];
                    cloud.points[e].z = element[2];
                }
                pcl::io::savePCDFileBinary(nameo.c_str(), cloud);
            }
            else if (hdr.components() == 4
                    && hdr.component_type(0) == gta::float32
                    && hdr.component_type(1) == gta::float32
                    && hdr.component_type(2) == gta::float32
                    && hdr.component_type(3) == gta::float32)
            {
                // pcl::PointXYZI
                pcl::PointCloud<pcl::PointXYZI> cloud;
                cloud.width = checked_cast<uint32_t>(hdr.elements());
                cloud.height = 1;
                cloud.is_dense = false;
                cloud.points.resize(cloud.width * cloud.height);
                element_loop_t element_loop;
                array_loop.start_element_loop(element_loop, hdr, gta::header());
                for (uintmax_t e = 0; e < hdr.elements(); e++)
                {
                    const float *element = static_cast<const float *>(element_loop.read());
                    cloud.points[e].x = element[0];
                    cloud.points[e].y = element[1];
                    cloud.points[e].z = element[2];
                    cloud.points[e].intensity = element[3];
                }
                pcl::io::savePCDFileBinary(nameo.c_str(), cloud);
            }
            else if (hdr.components() == 6
                    && hdr.component_type(0) == gta::float32
                    && hdr.component_type(1) == gta::float32
                    && hdr.component_type(2) == gta::float32
                    && hdr.component_type(3) == gta::uint8
                    && hdr.component_type(4) == gta::uint8
                    && hdr.component_type(5) == gta::uint8)
            {
                // pcl::PointXYZRGB
                pcl::PointCloud<pcl::PointXYZRGB> cloud;
                cloud.width = checked_cast<uint32_t>(hdr.elements());
                cloud.height = 1;
                cloud.is_dense = false;
                cloud.points.resize(cloud.width * cloud.height);
                element_loop_t element_loop;
                array_loop.start_element_loop(element_loop, hdr, gta::header());
                for (uintmax_t e = 0; e < hdr.elements(); e++)
                {
                    const void *element = element_loop.read();
                    cloud.points[e].x = static_cast<const float *>(element)[0];
                    cloud.points[e].y = static_cast<const float *>(element)[1];
                    cloud.points[e].z = static_cast<const float *>(element)[2];
                    uint8_t r, g, b;
                    std::memcpy(&r, static_cast<const uint8_t *>(element) + 12, sizeof(uint8_t));
                    std::memcpy(&g, static_cast<const uint8_t *>(element) + 13, sizeof(uint8_t));
                    std::memcpy(&b, static_cast<const uint8_t *>(element) + 14, sizeof(uint8_t));
                    assert(sizeof(float) == sizeof(uint32_t));
                    uint32_t color = (static_cast<int>(r) << 16) | (static_cast<int>(g) << 8) | b;
                    std::memcpy(&cloud.points[e].rgb, &color, sizeof(uint32_t));
                }
                pcl::io::savePCDFileBinary(nameo.c_str(), cloud);
            }
            else if (hdr.components() == 6
                    && hdr.component_type(0) == gta::float32
                    && hdr.component_type(1) == gta::float32
                    && hdr.component_type(2) == gta::float32
                    && hdr.component_type(3) == gta::float32
                    && hdr.component_type(4) == gta::float32
                    && hdr.component_type(5) == gta::float32)
            {
                // pcl::PointNormal
                pcl::PointCloud<pcl::PointNormal> cloud;
                cloud.width = checked_cast<uint32_t>(hdr.elements());
                cloud.height = 1;
                cloud.is_dense = false;
                cloud.points.resize(cloud.width * cloud.height);
                element_loop_t element_loop;
                array_loop.start_element_loop(element_loop, hdr, gta::header());
                for (uintmax_t e = 0; e < hdr.elements(); e++)
                {
                    const float *element = static_cast<const float *>(element_loop.read());
                    cloud.points[e].x = element[0];
                    cloud.points[e].y = element[1];
                    cloud.points[e].z = element[2];
                    cloud.points[e].normal_x = element[3];
                    cloud.points[e].normal_y = element[4];
                    cloud.points[e].normal_z = element[5];
                }
                pcl::io::savePCDFileBinary(nameo.c_str(), cloud);
            }
            else if (hdr.components() == 7
                    && hdr.component_type(0) == gta::float32
                    && hdr.component_type(1) == gta::float32
                    && hdr.component_type(2) == gta::float32
                    && hdr.component_type(3) == gta::uint8
                    && hdr.component_type(4) == gta::uint8
                    && hdr.component_type(5) == gta::uint8
                    && hdr.component_type(6) == gta::uint8)
            {
                // pcl::PointXYZRGBA
                pcl::PointCloud<pcl::PointXYZRGBA> cloud;
                cloud.width = checked_cast<uint32_t>(hdr.elements());
                cloud.height = 1;
                cloud.is_dense = false;
                cloud.points.resize(cloud.width * cloud.height);
                element_loop_t element_loop;
                array_loop.start_element_loop(element_loop, hdr, gta::header());
                for (uintmax_t e = 0; e < hdr.elements(); e++)
                {
                    const void *element = element_loop.read();
                    cloud.points[e].x = static_cast<const float *>(element)[0];
                    cloud.points[e].y = static_cast<const float *>(element)[1];
                    cloud.points[e].z = static_cast<const float *>(element)[2];
                    uint8_t r, g, b, a;
                    std::memcpy(&r, static_cast<const uint8_t *>(element) + 12, sizeof(uint8_t));
                    std::memcpy(&g, static_cast<const uint8_t *>(element) + 13, sizeof(uint8_t));
                    std::memcpy(&b, static_cast<const uint8_t *>(element) + 14, sizeof(uint8_t));
                    std::memcpy(&a, static_cast<const uint8_t *>(element) + 15, sizeof(uint8_t));
                    assert(sizeof(float) == sizeof(uint32_t));
                    uint32_t color = (static_cast<int>(a) << 24) | (static_cast<int>(r) << 16) | (static_cast<int>(g) << 8) | b;
                    std::memcpy(&cloud.points[e].rgba, &color, sizeof(uint32_t));
                }
                pcl::io::savePCDFileBinary(nameo.c_str(), cloud);
            }
            else if (hdr.components() == 7
                    && hdr.component_type(0) == gta::float32
                    && hdr.component_type(1) == gta::float32
                    && hdr.component_type(2) == gta::float32
                    && hdr.component_type(3) == gta::float32
                    && hdr.component_type(4) == gta::float32
                    && hdr.component_type(5) == gta::float32
                    && hdr.component_type(6) == gta::float32)
            {
                // pcl::PointXYZINormal
                pcl::PointCloud<pcl::PointXYZINormal> cloud;
                cloud.width = checked_cast<uint32_t>(hdr.elements());
                cloud.height = 1;
                cloud.is_dense = false;
                cloud.points.resize(cloud.width * cloud.height);
                element_loop_t element_loop;
                array_loop.start_element_loop(element_loop, hdr, gta::header());
                for (uintmax_t e = 0; e < hdr.elements(); e++)
                {
                    const float *element = static_cast<const float *>(element_loop.read());
                    cloud.points[e].x = element[0];
                    cloud.points[e].y = element[1];
                    cloud.points[e].z = element[2];
                    cloud.points[e].normal_x = element[3];
                    cloud.points[e].normal_y = element[4];
                    cloud.points[e].normal_z = element[5];
                    cloud.points[e].intensity = element[6];
                }
                pcl::io::savePCDFileBinary(nameo.c_str(), cloud);
            }
            else if (hdr.components() == 9
                    && hdr.component_type(0) == gta::float32
                    && hdr.component_type(1) == gta::float32
                    && hdr.component_type(2) == gta::float32
                    && hdr.component_type(3) == gta::float32
                    && hdr.component_type(4) == gta::float32
                    && hdr.component_type(5) == gta::float32
                    && hdr.component_type(6) == gta::uint8
                    && hdr.component_type(7) == gta::uint8
                    && hdr.component_type(8) == gta::uint8)
            {
                // pcl::PointXYZRGBNormal
                pcl::PointCloud<pcl::PointXYZRGBNormal> cloud;
                cloud.width = checked_cast<uint32_t>(hdr.elements());
                cloud.height = 1;
                cloud.is_dense = false;
                cloud.points.resize(cloud.width * cloud.height);
                element_loop_t element_loop;
                array_loop.start_element_loop(element_loop, hdr, gta::header());
                for (uintmax_t e = 0; e < hdr.elements(); e++)
                {
                    const void *element = element_loop.read();
                    cloud.points[e].x = static_cast<const float *>(element)[0];
                    cloud.points[e].y = static_cast<const float *>(element)[1];
                    cloud.points[e].z = static_cast<const float *>(element)[2];
                    cloud.points[e].normal_x = static_cast<const float *>(element)[3];
                    cloud.points[e].normal_y = static_cast<const float *>(element)[4];
                    cloud.points[e].normal_z = static_cast<const float *>(element)[5];
                    uint8_t r, g, b;
                    std::memcpy(&r, static_cast<const uint8_t *>(element) + 24, sizeof(uint8_t));
                    std::memcpy(&g, static_cast<const uint8_t *>(element) + 25, sizeof(uint8_t));
                    std::memcpy(&b, static_cast<const uint8_t *>(element) + 26, sizeof(uint8_t));
                    assert(sizeof(float) == sizeof(uint32_t));
                    uint32_t color = (static_cast<int>(r) << 16) | (static_cast<int>(g) << 8) | b;
                    std::memcpy(&cloud.points[e].rgb, &color, sizeof(uint32_t));
                }
                pcl::io::savePCDFileBinary(nameo.c_str(), cloud);
            }
            else
            {
                throw exc(name + ": unsupported point type or attributes.");
            }
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
