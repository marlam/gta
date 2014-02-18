/*
 * Copyright (C) 2014
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

#include <limits>
#include <cstdlib>
#include <cmath>

#include "viewparams.hpp"

#include "base/dbg.h"
#include "base/sys.h"


void ViewParameters::set_mode(mode_t m, const gta::header& hdr, const MinMaxHist& minmaxhist)
{
    mode = m;
    assert(mode == mode_2d);
    const char* tagval;

    if (mode == mode_2d) {
        assert(hdr.dimensions() == 2);
        mode_2d_global.component = 0;
        // Determine array aspect ratio
        mode_2d_global.array_aspect = static_cast<float>(hdr.dimension_size(0))
            / static_cast<float>(hdr.dimension_size(1));
        // Determine sample aspect ratio
        mode_2d_global.sample_aspect = 1.0f;
        if ((tagval = hdr.dimension_taglist(0).get("SAMPLE-DISTANCE")))
            mode_2d_global.sample_aspect *= std::atof(tagval);
        if ((tagval = hdr.dimension_taglist(1).get("SAMPLE-DISTANCE")))
            mode_2d_global.sample_aspect /= std::atof(tagval);
        if (!std::isfinite(mode_2d_global.sample_aspect))
            mode_2d_global.sample_aspect = 1.0f;
        // Find color components
        int ch_rgb[3] = { -1, -1, -1 };
        int ch_srgb[3] = { -1, -1, -1 };
        int ch_lum = -1;
        int ch_xyz[3] = { -1, -1, -1 };
        int ch_hsl[3] = { -1, -1, -1 };
        for (size_t i = 0; i < hdr.components(); i++) {
            const char* tagval = hdr.component_taglist(i).get("INTERPRETATION");
            if (!tagval)
                continue;
            if (std::strcmp(tagval, "RED") == 0)
                ch_rgb[0] = i;
            else if (std::strcmp(tagval, "GREEN") == 0)
                ch_rgb[1] = i;
            else if (std::strcmp(tagval, "BLUE") == 0)
                ch_rgb[2] = i;
            else if (std::strcmp(tagval, "SRGB/RED") == 0)
                ch_srgb[0] = i;
            else if (std::strcmp(tagval, "SRGB/GREEN") == 0)
                ch_srgb[1] = i;
            else if (std::strcmp(tagval, "SRGB/BLUE") == 0)
                ch_srgb[2] = i;
            else if (std::strcmp(tagval, "GRAY") == 0)
                ch_lum = i;
            else if (std::strcmp(tagval, "XYZ/X") == 0)
                ch_xyz[0] = i;
            else if (std::strcmp(tagval, "XYZ/Y") == 0)
                ch_xyz[1] = i;
            else if (std::strcmp(tagval, "XYZ/Z") == 0)
                ch_xyz[2] = i;
            else if (std::strcmp(tagval, "HSL/X") == 0)
                ch_hsl[0] = i;
            else if (std::strcmp(tagval, "HSL/Y") == 0)
                ch_hsl[1] = i;
            else if (std::strcmp(tagval, "HSL/Z") == 0)
                ch_hsl[2] = i;
        }
        mode_2d_global.colorspace = colorspace_null;
        mode_2d_global.color_components[0] = -1;
        mode_2d_global.color_components[1] = -1;
        mode_2d_global.color_components[2] = -1;
        mode_2d_global.lum_minval = 0.0f;
        mode_2d_global.lum_maxval = 0.0f;
        size_t histsize = sizeof(mode_2d_global.lum_histogram) / sizeof(mode_2d_global.lum_histogram[0]);
        for (size_t i = 0; i < histsize; i++)
            mode_2d_global.lum_histogram[i] = 0;
        mode_2d_global.lum_histogram_maxval = 0;
        if (ch_rgb[0] >= 0 && ch_rgb[1] >= 0 && ch_rgb[2] >= 0) {
            mode_2d_global.component = hdr.components();
            mode_2d_global.colorspace = colorspace_rgb;
            mode_2d_global.color_components[0] = ch_rgb[0];
            mode_2d_global.color_components[1] = ch_rgb[1];
            mode_2d_global.color_components[2] = ch_rgb[2];
            mode_2d_global.lum_minval = 0.0f;
            mode_2d_global.lum_maxval = 1.0f;
            // Luminance histogram cannot be computed without re-examining the data.
            // We don't do that here.
        } else if (ch_srgb[0] >= 0 && ch_srgb[1] >= 0 && ch_srgb[2] >= 0) {
            mode_2d_global.component = hdr.components();
            mode_2d_global.colorspace = colorspace_srgb;
            mode_2d_global.color_components[0] = ch_srgb[0];
            mode_2d_global.color_components[1] = ch_srgb[1];
            mode_2d_global.color_components[2] = ch_srgb[2];
            mode_2d_global.lum_minval = 0.0f;
            mode_2d_global.lum_maxval = 1.0f;
            // Luminance histogram cannot be computed without re-examining the data.
            // We don't do that here.
        } else if (ch_lum >= 0) {
            mode_2d_global.component = hdr.components();
            mode_2d_global.colorspace = colorspace_lum;
            mode_2d_global.color_components[0] = ch_lum;
            mode_2d_global.lum_minval = minmaxhist.minvals[ch_lum];
            mode_2d_global.lum_maxval = minmaxhist.maxvals[ch_lum];
            for (size_t i = 0; i < histsize; i++)
                mode_2d_global.lum_histogram[i] = minmaxhist.histograms[ch_lum][i];
            mode_2d_global.lum_histogram_maxval = minmaxhist.histogram_maxvals[ch_lum];
        } else if (ch_xyz[0] >= 0 && ch_xyz[1] >= 0 && ch_xyz[2] >= 0) {
            mode_2d_global.component = hdr.components();
            mode_2d_global.colorspace = colorspace_xyz;
            mode_2d_global.color_components[0] = ch_xyz[0];
            mode_2d_global.color_components[1] = ch_xyz[1];
            mode_2d_global.color_components[2] = ch_xyz[2];
            mode_2d_global.lum_minval = minmaxhist.minvals[ch_xyz[1]];
            mode_2d_global.lum_maxval = minmaxhist.maxvals[ch_xyz[1]];
            for (size_t i = 0; i < histsize; i++)
                mode_2d_global.lum_histogram[i] = minmaxhist.histograms[ch_xyz[1]][i];
            mode_2d_global.lum_histogram_maxval = minmaxhist.histogram_maxvals[ch_xyz[1]];
        } else if (ch_hsl[0] >= 0 && ch_hsl[1] >= 0 && ch_hsl[2] >= 0) {
            mode_2d_global.component = hdr.components();
            mode_2d_global.colorspace = colorspace_hsl;
            mode_2d_global.color_components[0] = ch_hsl[0];
            mode_2d_global.color_components[1] = ch_hsl[1];
            mode_2d_global.color_components[2] = ch_hsl[2];
            mode_2d_global.lum_minval = minmaxhist.minvals[ch_hsl[1]];
            mode_2d_global.lum_maxval = minmaxhist.maxvals[ch_hsl[2]];
            for (size_t i = 0; i < histsize; i++)
                mode_2d_global.lum_histogram[i] = minmaxhist.histograms[ch_hsl[2]][i];
            mode_2d_global.lum_histogram_maxval = minmaxhist.histogram_maxvals[ch_hsl[2]];
        }
        // Find z component
        mode_2d_global.z_component = -1;
        for (size_t i = 0; i < hdr.components(); i++) {
            const char* tagval = hdr.component_taglist(i).get("INTERPRETATION");
            if (!tagval)
                continue;
            if (strcmp(tagval, "HEIGHT") == 0 || strcmp(tagval, "HEIGHT/PLANAR") == 0) {
                mode_2d_global.z_component = i;
                mode_2d_global.z_is_height = true;
                mode_2d_global.z_is_radial = false;
            } else if (strcmp(tagval, "DEPTH") == 0 || strcmp(tagval, "DEPTH/PLANAR") == 0) {
                mode_2d_global.z_component = i;
                mode_2d_global.z_is_height = false;
                mode_2d_global.z_is_radial = false;
            } else if (strcmp(tagval, "DEPTH/RADIAL") == 0) {
                mode_2d_global.z_component = i;
                mode_2d_global.z_is_height = false;
                mode_2d_global.z_is_radial = true;
            }
        }
        mode_2d_global.z_factor = 1.0f;
        // Global options
        mode_2d_global.show_grid = false;
        mode_2d_global.show_3d = false;
        mode_2d_global.show_3d_cuboid = false;
        // Channel options
        mode_2d_components.resize(hdr.components() + (mode_2d_global.colorspace != colorspace_null ? 1 : 0));
        for (size_t i = 0; i < mode_2d_components.size(); i++) {
            if (i < hdr.components()) {
                mode_2d_components[i].minval = minmaxhist.minvals[i];
                mode_2d_components[i].maxval = minmaxhist.maxvals[i];
                mode_2d_components[i].default_minval = minmaxhist.minvals[i];
                mode_2d_components[i].default_maxval = minmaxhist.maxvals[i];
                mode_2d_components[i].range_min = minmaxhist.minvals[i];
                mode_2d_components[i].range_max = minmaxhist.maxvals[i];
            } else { // color mode
                assert(mode_2d_global.colorspace != colorspace_null);
                mode_2d_components[i].minval = mode_2d_global.lum_minval;
                mode_2d_components[i].maxval = mode_2d_global.lum_maxval;
                mode_2d_components[i].default_minval = mode_2d_global.lum_minval;
                mode_2d_components[i].default_maxval = mode_2d_global.lum_maxval;
                mode_2d_components[i].range_min = mode_2d_global.lum_minval;
                mode_2d_components[i].range_max = mode_2d_global.lum_maxval;
            }
            mode_2d_components[i].gamma = false;
            mode_2d_components[i].gamma_value = 1.0f;
            mode_2d_components[i].urq = false;
            mode_2d_components[i].urq_value = 1.0f;
            mode_2d_components[i].jetcolor = false;
            mode_2d_components[i].jetcolor_cyclic = false;
            mode_2d_components[i].gradient = false;
            assert(max_gradient_length >= 256);
            for (int j = 0; j < 256; j++) {
                mode_2d_components[i].gradient_colors[3 * j + 0] = j;
                mode_2d_components[i].gradient_colors[3 * j + 1] = j;
                mode_2d_components[i].gradient_colors[3 * j + 2] = j;
            }
            mode_2d_components[i].gradient_length = 256;
            mode_2d_components[i].coloring_inverse = false;
            mode_2d_components[i].coloring_start = 0.0f;
            mode_2d_components[i].coloring_lightvar = 0.0f;
        }
    }
}

ViewParameters::mode_t ViewParameters::suggest_mode(const gta::header& hdr, std::string* failure_reason)
{
    /* General checks */

    // Check if we have any data
    if (hdr.data_size() == 0) {
        if (failure_reason)
            *failure_reason = "No data";
        return mode_null;
    }
    // Check if the data is small enough
    bool too_big = false;
    if (hdr.data_size() > sys::total_ram() / 3
            || hdr.element_size() >= static_cast<unsigned int>(std::numeric_limits<int>::max())
            || hdr.dimensions() >= static_cast<unsigned int>(std::numeric_limits<int>::max())
            || hdr.components() >= static_cast<unsigned int>(std::numeric_limits<int>::max())) {
        too_big = true;
    }
    for (uintmax_t d = 0; d < hdr.dimensions() && !too_big; d++) {
        if (hdr.dimension_size(d) >= static_cast<unsigned int>(std::numeric_limits<int>::max()))
            too_big = true;
    }
    if (too_big) {
        if (failure_reason)
            *failure_reason = "Data too big";
        return mode_null;
    }
    // Check if we can work with the data types
    for (uintmax_t c = 0; c < hdr.components(); c++) {
        gta::type t = hdr.component_type(c);
        if (t != gta::int8 && t != gta::uint8 && t != gta::int16 && t != gta::uint16
                && t != gta::int32 && t != gta::uint32 && t != gta::int64 && t != gta::uint64
                && t != gta::float32 && t != gta::float64 && t != gta::cfloat32 && t != gta::cfloat64) {
            if (failure_reason)
                *failure_reason = "Unsupported component type";
            return mode_null;
        }
    }

    /* Find a suitable mode */
    if (hdr.dimensions() == 2) {
        return mode_2d;
    }

    /* Fallback */
    if (failure_reason)
        *failure_reason = "Don't know how to visualize this";
    return mode_null;
}

void ViewParameters::save(std::ostream& os) const
{
    s11n::save(os, static_cast<int>(mode));
    if (mode == mode_2d) {
        s11n::save(os, &mode_2d_global, sizeof(mode_2d_global_t));
        s11n::save(os, mode_2d_components.size());
        for (size_t i = 0; i < mode_2d_components.size(); i++)
            s11n::save(os, &(mode_2d_components[i]), sizeof(mode_2d_component_t));
    }
}

void ViewParameters::load(std::istream& is)
{
    int x;
    s11n::load(is, x); mode = static_cast<mode_t>(x);
    if (mode == mode_2d) {
        s11n::load(is, &mode_2d_global, sizeof(mode_2d_global_t));
        size_t components_size;
        s11n::load(is, components_size);
        mode_2d_components.resize(components_size);
        for (size_t i = 0; i < mode_2d_components.size(); i++)
            s11n::load(is, &(mode_2d_components[i]), sizeof(mode_2d_component_t));
    }
}
