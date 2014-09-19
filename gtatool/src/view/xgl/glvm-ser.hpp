/*
 * C++ vector and matrix classes that resemble GLSL style.
 *
 * Copyright (C) 2011, 2012, 2013, 2014
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
    template<typename T>
    void _save(std::ostream& os, const T* x, int S)
    {
        for (int i = 0; i < S; i++) {
            s11n::save(os, x[i]);
        }
    }

    template<typename T>
    void _save(std::ostream& os, const char* name, const T* x, int S)
    {
        s11n::startgroup(os, name);
        for (int i = 0; i < S; i++) {
            s11n::save(os, "", x[i]);
        }
        s11n::endgroup(os);
    }

    template<typename T>
    void _load(std::istream& is, T* x, int S)
    {
        for (int i = 0; i < S; i++) {
            s11n::load(is, x[i]);
        }
    }

    template<typename T>
    void _load(const std::string& s, T* x, int S)
    {
        std::stringstream ss(s);
        std::string name, value;
        for (int i = 0; i < S; i++) {
            s11n::load(ss, name, value);
            load(value, x[i]);
        }
    }

    template<typename T, int S>
    void save(std::ostream& os, const glvm::vector<T, S>& x)
    {
        _save(os, &x.x, S);
    }

    template<typename T, int S>
    void save(std::ostream& os, const char* name, const glvm::vector<T, S>& x)
    {
        _save(os, name, &x.x, S);
    }

    template<typename T, int S>
    void load(std::istream& is, glvm::vector<T, S>& x)
    {
        _load(is, &x.x, S);
    }

    template<typename T, int S>
    void load(const std::string& s, glvm::vector<T, S>& x)
    {
        _load(s, &x.x, S);
    }

    template<typename T, int C, int R>
    void save(std::ostream& os, const glvm::matrix<T, R, C>& x)
    {
        _save(os, &(x[0][0]), C * R);
    }

    template<typename T, int C, int R>
    void save(std::ostream& os, const char* name, const glvm::matrix<T, R, C>& x)
    {
        _save(os, name, &(x[0][0]), C * R);
    }

    template<typename T, int C, int R>
    void load(std::istream& is, glvm::matrix<T, R, C>& x)
    {
        _load(is, &(x[0][0]), C * R);
    }

    template<typename T, int C, int R>
    void load(const std::string& s, glvm::matrix<T, R, C>& x)
    {
        _load(s, &(x[0][0]), C * R);
    }

    template<typename T>
    void save(std::ostream& os, const glvm::quaternion<T>& x)
    {
        _save(os, &x.x, 4);
    }

    template<typename T>
    void save(std::ostream& os, const char* name, const glvm::quaternion<T>& x)
    {
        _save(os, name, &x.x, 4);
    }

    template<typename T>
    void load(std::istream& is, glvm::quaternion<T>& x)
    {
        _load(is, &x.x, 4);
    }

    template<typename T>
    void load(const std::string& s, glvm::quaternion<T>& x)
    {
        _load(s, &x.x, 4);
    }

    template<typename T>
    void save(std::ostream& os, const glvm::frustum<T>& x)
    {
        _save(os, &x.l, 6);
    }

    template<typename T>
    void save(std::ostream& os, const char* name, const glvm::frustum<T>& x)
    {
        _save(os, name, &x.l, 6);
    }

    template<typename T>
    void load(std::istream& is, glvm::frustum<T>& x)
    {
        _load(is, &x.l, 6);
    }

    template<typename T>
    void load(const std::string& s, glvm::frustum<T>& x)
    {
        _load(s, &x.l, 6);
    }
}

#endif
