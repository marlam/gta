/*
 * This file is part of gtatool, a tool to manipulate Generic Tagged Arrays
 * (GTAs).
 *
 * Copyright (C) 2013
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

#include "filters.h"

std::vector<std::string> find_filters(const std::string& extension, bool import)
{
    std::vector<std::string> filters;
    if (extension == "au") {
        filters.push_back("sndfile");
        if (import) filters.push_back("ffmpeg");
    } else if (extension == "avi") {
        if (import) filters.push_back("ffmpeg");
    } else if (extension == "bmp") {
        filters.push_back("magick");
    } else if (extension == "csv") {
        filters.push_back("csv");
    } else if (extension == "dcm" || extension == "dicom") {
        if (import) filters.push_back("dcmtk");
    } else if (extension == "exr") {
        filters.push_back("exr");
    } else if (extension == "gif") {
        filters.push_back("magick");
    } else if (extension == "h4" || extension == "he4" || extension == "hdf4") {
        filters.push_back("netcdf");
    } else if (extension == "h5" || extension == "he5" || extension == "hdf5") {
        filters.push_back("netcdf");
    } else if (extension == "hdf") {
        filters.push_back("netcdf");
    } else if (extension == "jpg" || extension == "jpeg") {
        filters.push_back("jpeg");
        filters.push_back("magick");
    } else if (extension == "mat") {
        filters.push_back("mat");
    } else if (extension == "mkv") {
        if (import) filters.push_back("ffmpeg");
    } else if (extension == "mov") {
        if (import) filters.push_back("ffmpeg");
    } else if (extension == "mp4") {
        if (import) filters.push_back("ffmpeg");
    } else if (extension == "mpg") {
        if (import) filters.push_back("ffmpeg");
    } else if (extension == "nc" || extension == "cdf") {
        filters.push_back("netcdf");
    } else if (extension == "nrrd") {
        filters.push_back("teem");
    } else if (extension == "ogg") {
        filters.push_back("sndfile");
        if (import) filters.push_back("ffmpeg");
    } else if (extension == "pbm"
            || extension == "pgm"
            || extension == "ppm"
            || extension == "pnm"
            || extension == "pfm"
            || extension == "pam") {
        filters.push_back("netpbm");
    } else if (extension == "pcd") {
        filters.push_back("pcd");
    } else if (extension == "pfs") {
        filters.push_back("pfs");
    } else if (extension == "ply") {
        filters.push_back("ply");
    } else if (extension == "png") {
        filters.push_back("png");
        filters.push_back("magick");
    } else if (extension == "pvm") {
        filters.push_back("pvm");
    } else if (extension == "rat") {
        filters.push_back("rat");
    } else if (extension == "raw" && !import) {
        filters.push_back("raw");
    } else if (extension == "snd") {
        filters.push_back("sndfile");
        if (import) filters.push_back("ffmpeg");
    } else if (extension == "tga" || extension == "tpic") {
        filters.push_back("magick");
    } else if (extension == "tif" || extension == "tiff") {
        filters.push_back("gdal");
        filters.push_back("magick");
    } else if (extension == "wav") {
        filters.push_back("sndfile");
        if (import) filters.push_back("ffmpeg");
    } else if (extension == "webm") {
        if (import) filters.push_back("ffmpeg");
    } else if (extension == "webp") {
        filters.push_back("magick");
    }
    return filters;
}
