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

#version 120

/*
 * Input data
 */

uniform sampler2D components[3];
uniform float denorm_factor;

/*
 * Parameters
 */

uniform bool is_complex; // implies colorspace == COLORSPACE_NULL

#define COLORSPACE_NULL 0 // these
#define COLORSPACE_RGB  1 // must
#define COLORSPACE_SRGB 2 // match
#define COLORSPACE_LUM  3 // colorspace_t
#define COLORSPACE_XYZ  4 // from
#define COLORSPACE_HSL  5 // viewparams.h
uniform int colorspace;

uniform float minval;
uniform float maxval;

uniform bool do_gamma;
uniform float gamma;
uniform bool do_urq;
uniform float urq;

uniform bool do_jetcolor;
uniform bool jetcolor_cyclic;
uniform bool do_gradient;
uniform sampler2D gradient_tex;
uniform bool coloring_inverse;
uniform float coloring_start;
uniform float coloring_lightvar;

/*
 * Color space conversions
 */

// We use the D65 reference white for the RGB values.
// The computation is exactly the same that is used by pfstools-1.6.2.
vec3 rgb_to_xyz(vec3 rgb)
{
    mat3 M = mat3(0.412424, 0.357579, 0.180464,
                  0.212656, 0.715158, 0.072186,
                  0.019332, 0.119193, 0.950444);
    return rgb * M;
}
vec3 xyz_to_rgb(vec3 xyz)
{
    mat3 M = mat3( 3.240708, -1.537259, -0.498570,
                  -0.969257,  1.875995,  0.041555,
                   0.055636, -0.203996,  1.057069);
    return xyz * M;
}

vec3 srgb_to_rgb(vec3 srgb)
{
    // See GL_ARB_framebuffer_sRGB extension
    srgb.r = (srgb.r <= 0.04045 ? (srgb.r / 12.92) : (pow((srgb.r + 0.055) / 1.055, 2.4)));
    srgb.g = (srgb.g <= 0.04045 ? (srgb.g / 12.92) : (pow((srgb.g + 0.055) / 1.055, 2.4)));
    srgb.b = (srgb.b <= 0.04045 ? (srgb.b / 12.92) : (pow((srgb.b + 0.055) / 1.055, 2.4)));
    return srgb;
}
vec3 srgb_to_xyz(vec3 srgb)
{
    return rgb_to_xyz(srgb_to_rgb(srgb));
}

vec3 lum_to_xyz(float lum)
{
    const float d65_x = 0.31271;
    const float d65_y = 0.32902;
    float Y = lum;
    float X = Y * (d65_x / d65_y);
    float Z = min(1.0, Y * (1.0 - d65_x - d65_y) / d65_y);
    return vec3(X, Y, Z);
}

float hsl_to_rgb_helper(float tmp2, float tmp1, float H)
{
    float ret;

    if (H < 0.0)
        H += 1.0;
    else if (H > 1.0)
        H -= 1.0;

    if (H < 1.0 / 6.0)
        ret = (tmp2 + (tmp1 - tmp2) * (360.0 / 60.0) * H);
    else if (H < 1.0 / 2.0)
        ret = tmp1;
    else if (H < 2.0 / 3.0)
        ret = (tmp2 + (tmp1 - tmp2) * ((2.0 / 3.0) - H) * (360.0 / 60.0));
    else
        ret = tmp2;
    return ret;
}
vec3 hsl_to_rgb(vec3 hsl)
{
    vec3 rgb;
    float tmp1, tmp2;

    if (hsl.z < 0.5)
        tmp1 = hsl.z * (1.0 + hsl.y);
    else
        tmp1 = (hsl.z + hsl.y) - (hsl.z * hsl.y);
    tmp2 = 2.0 * hsl.z - tmp1;
    rgb.r = hsl_to_rgb_helper(tmp2, tmp1, hsl.x + (1.0 / 3.0));
    rgb.g = hsl_to_rgb_helper(tmp2, tmp1, hsl.x);
    rgb.b = hsl_to_rgb_helper(tmp2, tmp1, hsl.x - (1.0 / 3.0));
    return rgb;
}
vec3 hsl_to_xyz(vec3 hsl)
{
    return rgb_to_xyz(hsl_to_rgb(hsl));
}

/*
 * Main function
 */

void main()
{
    float v = 0.0;
    vec3 color = vec3(0.0);

    // Read value, transform [minval,maxval] to [0,1], and apply value adaptation
    if (colorspace == COLORSPACE_NULL) {
        if (is_complex) {
            // Complex types use floating point which does not need denormalization
            v = length(texture2D(components[0], gl_TexCoord[0].xy).rg);
        } else {
            v = texture2D(components[0], gl_TexCoord[0].xy).r;
            // Internal formats with integer representation need denormalization.
            // note that for signed integers, we lose the lowest representable
            // number in the process (see GL_EXT_texture_snorm).
            v *= denorm_factor;
        }
        v = (clamp(v, minval, maxval) - minval) / (maxval - minval);
        if (do_gamma)
            v = pow(v, gamma);
        if (do_urq)
            v = (urq * v) / ((urq - 1.0) * v + 1.0);
    } else {
        color = vec3(texture2D(components[0], gl_TexCoord[0].xy).r,
                texture2D(components[1], gl_TexCoord[0].xy).r,
                texture2D(components[2], gl_TexCoord[0].xy).r);
        if (colorspace == COLORSPACE_RGB)
            color = rgb_to_xyz(color);
        else if (colorspace == COLORSPACE_SRGB)
            color = srgb_to_xyz(color);
        else if (colorspace == COLORSPACE_LUM)
            color = lum_to_xyz(color.y);
        else if (colorspace == COLORSPACE_HSL)
            color = hsl_to_xyz(color);
        float new_y = color.y;
        if (minval != 0.0 || maxval != 1.0)
            new_y = (clamp(color.y, minval + 0.00001, maxval) - minval) / (maxval - minval);
        if (do_gamma)
            new_y = pow(new_y, gamma);
        if (do_urq)
            new_y = (urq * new_y) / ((urq - 1.0) * new_y + 1.0);
        if (new_y != color.y) {
            vec3 tmpval = color / (color.x + color.y + color.z);
            color = vec3(
                    new_y / tmpval.y * tmpval.x,
                    new_y,
                    new_y / tmpval.y * tmpval.z);
        }
        v = color.y;
    }

    // Get linear RGB color
    if (do_jetcolor) {
        if (coloring_inverse)
            v = 1.0 - v;
        float jetcolor_factor = (jetcolor_cyclic ? 1.0 : (2.0 / 3.0));
        float H = (2.0 / 3.0) - coloring_start - jetcolor_factor * v;
        if (H < -1.0)
            H += 2.0;
        else if (H < 0.0)
            H += 1.0;
        float S = 1.0;
        float L = clamp(((1.0 - coloring_lightvar) * 0.5) + coloring_lightvar * v * 0.5, 0.0, 0.5);
        color = hsl_to_rgb(vec3(H, S, L));
    } else if (do_gradient) {
        if (coloring_inverse)
            v = 1.0 - v;
        float g = v + coloring_start;
        if (g > 1.0)
            g -= 1.0;
        color = texture2D(gradient_tex, vec2(g, 0.5)).rgb;
        color -= coloring_lightvar * (1.0 - v) * color;
    } else if (colorspace == COLORSPACE_NULL) {
        color = vec3(v);
    } else {
        color = xyz_to_rgb(color);
    }

    gl_FragColor = vec4(color, 1.0);
}
