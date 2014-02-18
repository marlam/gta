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

#ifndef VIEWPARAMS_H
#define VIEWPARAMS_H

#include <vector>

#include <gta/gta.hpp>

#include "base/ser.h"

#include "minmaxhist.hpp"


class ViewParameters : public serializable
{
public:
    static const int max_gradient_length = 512;

    typedef enum {
        mode_null,
        mode_2d,
    } mode_t;

    typedef enum {
        colorspace_null = 0, // no color
        colorspace_rgb  = 1, // linear RGB
        colorspace_srgb = 2, // SRGB
        colorspace_lum  = 3, // linear graylevel
        colorspace_xyz  = 4, // CIE XYZ
        colorspace_hsl  = 5  // HSL
    } colorspace_t;

    mode_t mode;

    ViewParameters() : mode(mode_null)
    {
    }

    virtual ~ViewParameters() {}

    static mode_t suggest_mode(const gta::header& hdr, std::string* failure_msg);

    void set_mode(mode_t mode, const gta::header& hdr, const MinMaxHist& minmaxhist);

    bool mode_is_valid() const
    {
        return mode != mode_null;
    }

    bool mode_is_2d() const
    {
        return mode == mode_2d && !mode_2d_global.show_3d;
    }

    virtual void save(std::ostream& os) const;
    virtual void load(std::istream& is);

    // mode_2d
    typedef struct {
        float minval;             // assume this minimum value in the data
        float maxval;             // assume this maximum value in the data
        float default_minval;     // default value for minval, usually derived from the data
        float default_maxval;     // default value for maxval, usually derived from the data
        float range_min;          // clamp to this minimum value
        float range_max;          // clamp to this maximum value
        bool gamma;               // apply gamma correction
        float gamma_value;        // use this gamma value for correction
        bool urq;                 // apply uniform rational quantization
        float urq_value;          // use this value for urq
        bool jetcolor;            // apply jet color pseudo coloring
        bool jetcolor_cyclic;     // use full hue range
        bool gradient;            // apply color gradient
        int gradient_length;      // length of color gradient
        uint8_t gradient_colors[max_gradient_length * 3]; // gradient colors
        bool coloring_inverse;    // inverse color direction
        float coloring_start;     // use this color as start [0,1]
        float coloring_lightvar;  // lightness variability, from 0 to 1
    } mode_2d_component_t;
    typedef struct {
        int component;            // component to view, or last_component_index+1 for color mode
        float array_aspect;       // aspect ratio of the array (width in samples / height in samples)
        float sample_aspect;      // aspect ratio of an individual sample (width / height)
        colorspace_t colorspace;  // color space, null if there are no color components
        int color_components[3];  // components that contain the color values
        float lum_minval;         // minimum luminance value, for color data
        float lum_maxval;         // maximum luminance value, for color data
        int lum_histogram[1024];  // histogram of luminances, for color data
        int lum_histogram_maxval; // maximum entry in luminance histogram, for color data
        int z_component;          // component that contains distance or height, or -1
        bool z_is_height;         // whether the z component represents distance or height
        bool z_is_radial;         // whether the z component is radial or planar
        float z_factor;           // scale factor for distance/height
        bool show_grid;           // show raster grid lines
        bool show_3d;             // only if z component is != -1
        bool show_3d_cuboid;      // show cuboid that contains the data
    } mode_2d_global_t;
    mode_2d_global_t mode_2d_global;
    std::vector<mode_2d_component_t> mode_2d_components; // the last entry is for color mode!
};

#endif
