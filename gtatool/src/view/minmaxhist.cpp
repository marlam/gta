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

#include <cmath>

#include "xgl/glvm.hpp"

#include "base/dbg.h"
#include "base/msg.h"
#include "base/str.h"

#include "minmaxhist.hpp"


template<typename T>
static void get_gta_nodata(const gta::header& hdr, size_t component, T* nodata_value, bool* have_nodata_value)
{
    *have_nodata_value = false;
    const char* tagval = hdr.component_taglist(component).get("NO_DATA_VALUE");
    if (tagval && str::to(tagval, nodata_value))
        *have_nodata_value = true;
}

template<typename T>
static void get_gta_minmax_helper(const gta::header& hdr, const void* data, size_t component,
        float* minval, float* maxval)
{
    T nodata_value;
    bool have_nodata_value;
    get_gta_nodata(hdr, component, &nodata_value, &have_nodata_value);

    float tmp_minval, tmp_maxval;
    tmp_minval = std::numeric_limits<T>::max();
    if (std::numeric_limits<T>::is_integer)
        tmp_maxval = std::numeric_limits<T>::min();
    else
        tmp_maxval = -std::numeric_limits<T>::max();

    size_t component_offset = static_cast<const unsigned char*>(
            hdr.component(static_cast<const void*>(0), component))
        - static_cast<const unsigned char*>(0);
    size_t element_size = hdr.element_size();
    bool have_valid_values = false;
    for (size_t e = 0; e < hdr.elements(); e++) {
        const char* ptr = static_cast<const char*>(data) + e * element_size + component_offset;
        if (have_nodata_value && std::memcmp(ptr, &nodata_value, sizeof(T)) == 0)
            continue;
        have_valid_values = true;
        float v;
        T vt;
        std::memcpy(&vt, ptr, sizeof(T));
        v = vt;
        if (v < tmp_minval)
            tmp_minval = v;
        if (v > tmp_maxval)
            tmp_maxval = v;
    }

    if (have_valid_values) {
        *minval = tmp_minval;
        *maxval = tmp_maxval;
    } else {
        *minval = nodata_value;
        *maxval = nodata_value;
    }
}

template<typename T>
static void get_gta_histogram_helper(const gta::header& hdr, const void* data, size_t component,
        float minval, float maxval, std::vector<int>* histogram, int* hist_maxval)
{
    T nodata_value;
    bool have_nodata_value;
    get_gta_nodata(hdr, component, &nodata_value, &have_nodata_value);

    size_t component_offset = static_cast<const unsigned char*>(
            hdr.component(static_cast<const void*>(0), component))
        - static_cast<const unsigned char*>(0);
    size_t element_size = hdr.element_size();
    for (size_t e = 0; e < hdr.elements(); e++) {
        const char* ptr = static_cast<const char*>(data) + e * element_size + component_offset;
        if (have_nodata_value && std::memcmp(ptr, &nodata_value, sizeof(T)) == 0)
            continue;
        float v;
        T vt;
        std::memcpy(&vt, ptr, sizeof(T));
        v = vt;
        if (std::numeric_limits<T>::is_integer || std::isfinite(v)) {
            int bin = (v - minval) / (maxval - minval) * (histogram->size() - 1);
            bin = glvm::clamp(bin, 0, static_cast<int>(histogram->size() - 1));
            (*histogram)[bin]++;
        }
    }
    *hist_maxval = 0;
    for (size_t i = 0; i < histogram->size(); i++) {
        if ((*histogram)[i] > *hist_maxval)
            *hist_maxval = (*histogram)[i];
    }
}


void MinMaxHist::compute(const gta::header& hdr, const void* data)
{
    minvals.resize(hdr.components());
    maxvals.resize(hdr.components());
    histograms.resize(hdr.components());
    histogram_maxvals.resize(hdr.components());
    for (uintmax_t c = 0; c < hdr.components(); c++) {
        msg::dbg("getting min, max, and histogram for gta data component %lu...", static_cast<unsigned long>(c));
        switch (hdr.component_type(c)) {
        case gta::uint8:
            get_gta_minmax_helper<uint8_t>(hdr, data, c, &minvals[c], &maxvals[c]);
            // use special parameters for the histogram avoid both superfluous precision and holes in the histogram
            histograms[c].resize(256, 0);
            get_gta_histogram_helper<uint8_t>(hdr, data, c, 0, 255, &histograms[c], &histogram_maxvals[c]);
            break;
        case gta::int8:
            get_gta_minmax_helper<int8_t>(hdr, data, c, &minvals[c], &maxvals[c]);
            // use special parameters for the histogram avoid both superfluous precision and holes in the histogram
            histograms[c].resize(256, 0);
            get_gta_histogram_helper<int8_t>(hdr, data, c, -128, 127, &histograms[c], &histogram_maxvals[c]);
            break;
        case gta::uint16:
            get_gta_minmax_helper<uint16_t>(hdr, data, c, &minvals[c], &maxvals[c]);
            histograms[c].resize(1024, 0);
            get_gta_histogram_helper<uint16_t>(hdr, data, c, minvals[c], maxvals[c], &histograms[c], &histogram_maxvals[c]);
            break;
        case gta::int16:
            get_gta_minmax_helper<int16_t>(hdr, data, c, &minvals[c], &maxvals[c]);
            histograms[c].resize(1024, 0);
            get_gta_histogram_helper<int16_t>(hdr, data, c, minvals[c], maxvals[c], &histograms[c], &histogram_maxvals[c]);
            break;
        case gta::uint32:
            get_gta_minmax_helper<uint32_t>(hdr, data, c, &minvals[c], &maxvals[c]);
            histograms[c].resize(1024, 0);
            get_gta_histogram_helper<uint32_t>(hdr, data, c, minvals[c], maxvals[c], &histograms[c], &histogram_maxvals[c]);
            break;
        case gta::int32:
            get_gta_minmax_helper<int32_t>(hdr, data, c, &minvals[c], &maxvals[c]);
            histograms[c].resize(1024, 0);
            get_gta_histogram_helper<int32_t>(hdr, data, c, minvals[c], maxvals[c], &histograms[c], &histogram_maxvals[c]);
            break;
        case gta::uint64:
            get_gta_minmax_helper<uint64_t>(hdr, data, c, &minvals[c], &maxvals[c]);
            histograms[c].resize(1024, 0);
            get_gta_histogram_helper<uint64_t>(hdr, data, c, minvals[c], maxvals[c], &histograms[c], &histogram_maxvals[c]);
            break;
        case gta::int64:
            get_gta_minmax_helper<int64_t>(hdr, data, c, &minvals[c], &maxvals[c]);
            histograms[c].resize(1024, 0);
            get_gta_histogram_helper<int64_t>(hdr, data, c, minvals[c], maxvals[c], &histograms[c], &histogram_maxvals[c]);
            break;
        case gta::float32:
            get_gta_minmax_helper<float>(hdr, data, c, &minvals[c], &maxvals[c]);
            histograms[c].resize(1024, 0);
            get_gta_histogram_helper<float>(hdr, data, c, minvals[c], maxvals[c], &histograms[c], &histogram_maxvals[c]);
            break;
        case gta::float64:
            get_gta_minmax_helper<double>(hdr, data, c, &minvals[c], &maxvals[c]);
            histograms[c].resize(1024, 0);
            get_gta_histogram_helper<double>(hdr, data, c, minvals[c], maxvals[c], &histograms[c], &histogram_maxvals[c]);
            break;
        case gta::cfloat32:
            get_gta_minmax_helper<float>(hdr, data, c, &minvals[c], &maxvals[c]);
            histograms[c].resize(1024, 0);
            get_gta_histogram_helper<float>(hdr, data, c, minvals[c], maxvals[c], &histograms[c], &histogram_maxvals[c]);
            break;
        case gta::cfloat64:
            get_gta_minmax_helper<double>(hdr, data, c, &minvals[c], &maxvals[c]);
            histograms[c].resize(1024, 0);
            get_gta_histogram_helper<double>(hdr, data, c, minvals[c], maxvals[c], &histograms[c], &histogram_maxvals[c]);
            break;
        default:
            // cannot happen
            assert(false);
        }
        msg::dbg("... done: min=%g, max=%g", minvals[c], maxvals[c]);
    }
}

void MinMaxHist::save(std::ostream& os) const
{
    s11n::save(os, minvals);
    s11n::save(os, maxvals);
    s11n::save(os, histograms);
    s11n::save(os, histogram_maxvals);
}

void MinMaxHist::load(std::istream& is)
{
    s11n::load(is, minvals);
    s11n::load(is, maxvals);
    s11n::load(is, histograms);
    s11n::load(is, histogram_maxvals);
}
