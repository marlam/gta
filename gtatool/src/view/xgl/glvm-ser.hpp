/*
 * C++ vector and matrix classes that resemble GLSL style.
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

#ifndef GLVM_S11N_H
#define GLVM_S11N_H

#include "base/ser.h"

#include "xgl/glvm.hpp"

namespace s11n
{
    template<typename T, int rows, int cols>
    void save(std::ostream& os, const glvm::array<T, rows, cols>& x)
    {
        for (int i = 0; i < rows * cols; i++) {
            s11n::save(os, x.vl[i]);
        }
    }

    template<typename T, int rows, int cols>
    void save(std::ostream& os, const char* name, const glvm::array<T, rows, cols>& x)
    {
        s11n::startgroup(os, name);
        for (int i = 0; i < rows * cols; i++) {
            s11n::save(os, "", x.vl[i]);
        }
        s11n::endgroup(os);
    }

    template<typename T, int rows, int cols>
    void load(std::istream& is, glvm::array<T, rows, cols>& x)
    {
        for (int i = 0; i < rows * cols; i++) {
            s11n::load(is, x.vl[i]);
        }
    }

    template<typename T, int rows, int cols>
    void load(const std::string& s, glvm::array<T, rows, cols>& x)
    {
        std::stringstream ss(s);
        std::string name, value;
        for (int i = 0; i < rows * cols; i++) {
            s11n::load(ss, name, value);
            load(value, x.vl[i]);
        }
    }
}

#endif
