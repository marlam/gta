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

#ifndef MINMAXHIST_H
#define MINMAXHIST_H

#include <vector>

#include <gta/gta.hpp>

#include "base/ser.h"


class MinMaxHist : public serializable
{
public:
    virtual ~MinMaxHist() {}

    // Minval, maxval, and histogram for each GTA component
    std::vector<float> minvals;
    std::vector<float> maxvals;
    std::vector<std::vector<int> > histograms;
    std::vector<int> histogram_maxvals;

    // Has the information been computed yet?
    bool valid() const { return histograms.size() > 0; }

    // Compute the information.
    void compute(const gta::header& hdr, const void* data);

    // Serialization
    virtual void save(std::ostream& os) const;
    virtual void load(std::istream& is);
};

#endif
