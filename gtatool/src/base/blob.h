/*
 * blob.h
 *
 * This file is part of gtatool, a tool to manipulate Generic Tagged Arrays
 * (GTAs).
 *
 * Copyright (C) 2010  Martin Lambers <marlam@marlam.de>
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

/**
 * \file blob.h
 *
 * This class provides an opaque memory block of a given size which can
 * store any kind of data. Such memory blocks are a pain to manage with
 * new/delete or new[]/delete[] or malloc()/free() because of the necessary
 * type casting. This class provides easy access pointers and a destructor.
 */

#ifndef BLOB_H
#define BLOB_H

#include <limits>
#include <cstdlib>
#include <cerrno>

#include "tools.h"
#include "exc.h"


class blob
{
private:
    
    size_t _size;
    void *_ptr;

    static size_t to_size_t(int a) throw (exc)
    {
        if (a < 0 || static_cast<unsigned int>(a) > std::numeric_limits<size_t>::max())
        {
#ifndef EOVERFLOW
            throw exc(ENOMEM);
#else
            throw exc(EOVERFLOW);
#endif
        }
        return static_cast<size_t>(a);
    }

    static size_t to_size_t(long a) throw (exc)
    {
        if (a < 0 || static_cast<unsigned long>(a) > std::numeric_limits<size_t>::max())
        {
#ifndef EOVERFLOW
            throw exc(ENOMEM);
#else
            throw exc(EOVERFLOW);
#endif
        }
        return static_cast<size_t>(a);
    }

    static size_t to_size_t(long long a) throw (exc)
    {
        if (a < 0 || static_cast<unsigned long long>(a) > std::numeric_limits<size_t>::max())
        {
#ifndef EOVERFLOW
            throw exc(ENOMEM);
#else
            throw exc(EOVERFLOW);
#endif
        }
        return static_cast<size_t>(a);
    }

    static size_t to_size_t(unsigned int a) throw (exc)
    {
        if (a > std::numeric_limits<size_t>::max())
        {
#ifndef EOVERFLOW
            throw exc(ENOMEM);
#else
            throw exc(EOVERFLOW);
#endif
        }
        return static_cast<size_t>(a);
    }

    static size_t to_size_t(unsigned long a) throw (exc)
    {
        if (a > std::numeric_limits<size_t>::max())
        {
#ifndef EOVERFLOW
            throw exc(ENOMEM);
#else
            throw exc(EOVERFLOW);
#endif
        }
        return static_cast<size_t>(a);
    }

    static size_t to_size_t(unsigned long long a) throw (exc)
    {
        if (a > std::numeric_limits<size_t>::max())
        {
#ifndef EOVERFLOW
            throw exc(ENOMEM);
#else
            throw exc(EOVERFLOW);
#endif
        }
        return static_cast<size_t>(a);
    }

    static size_t smul(size_t a, size_t b) throw (exc)
    {
        if (tools::product_overflows(a, b))
        {
#ifndef EOVERFLOW
            throw exc(ENOMEM);
#else
            throw exc(EOVERFLOW);
#endif
        }
        return a * b;
    }

    static size_t smul(size_t a, size_t b, size_t c) throw (exc)
    {
        return smul(smul(a, b), c);
    }

    static size_t smul(size_t a, size_t b, size_t c, size_t d) throw (exc)
    {
        return smul(smul(a, b), smul(c, d));
    }

    static size_t mul(int a, int b) throw (exc)
    {
        return smul(to_size_t(a), to_size_t(b));
    }

    static size_t mul(int a, int b, int c) throw (exc)
    {
        return smul(to_size_t(a), to_size_t(b), to_size_t(c));
    }

    static size_t mul(int a, int b, int c, int d) throw (exc)
    {
        return smul(to_size_t(a), to_size_t(b), to_size_t(c), to_size_t(d));
    }

    static size_t mul(long a, long b) throw (exc)
    {
        return smul(to_size_t(a), to_size_t(b));
    }

    static size_t mul(long a, long b, long c) throw (exc)
    {
        return smul(to_size_t(a), to_size_t(b), to_size_t(c));
    }

    static size_t mul(long a, long b, long c, long d) throw (exc)
    {
        return smul(to_size_t(a), to_size_t(b), to_size_t(c), to_size_t(d));
    }

    static size_t mul(long long a, long long b) throw (exc)
    {
        return smul(to_size_t(a), to_size_t(b));
    }

    static size_t mul(long long a, long long b, long long c) throw (exc)
    {
        return smul(to_size_t(a), to_size_t(b), to_size_t(c));
    }

    static size_t mul(long long a, long long b, long long c, long long d) throw (exc)
    {
        return smul(to_size_t(a), to_size_t(b), to_size_t(c), to_size_t(d));
    }

    static size_t mul(unsigned int a, unsigned int b) throw (exc)
    {
        return smul(to_size_t(a), to_size_t(b));
    }

    static size_t mul(unsigned int a, unsigned int b, unsigned int c) throw (exc)
    {
        return smul(to_size_t(a), to_size_t(b), to_size_t(c));
    }

    static size_t mul(unsigned int a, unsigned int b, unsigned int c, unsigned int d) throw (exc)
    {
        return smul(to_size_t(a), to_size_t(b), to_size_t(c), to_size_t(d));
    }

    static size_t mul(unsigned long a, unsigned long b) throw (exc)
    {
        return smul(to_size_t(a), to_size_t(b));
    }

    static size_t mul(unsigned long a, unsigned long b, unsigned long c) throw (exc)
    {
        return smul(to_size_t(a), to_size_t(b), to_size_t(c));
    }

    static size_t mul(unsigned long a, unsigned long b, unsigned long c, unsigned long d) throw (exc)
    {
        return smul(to_size_t(a), to_size_t(b), to_size_t(c), to_size_t(d));
    }

    static size_t mul(unsigned long long a, unsigned long long b) throw (exc)
    {
        return smul(to_size_t(a), to_size_t(b));
    }

    static size_t mul(unsigned long long a, unsigned long long b, unsigned long long c) throw (exc)
    {
        return smul(to_size_t(a), to_size_t(b), to_size_t(c));
    }

    static size_t mul(unsigned long long a, unsigned long long b, unsigned long long c, unsigned long long d) throw (exc)
    {
        return smul(to_size_t(a), to_size_t(b), to_size_t(c), to_size_t(d));
    }

    static void *alloc(size_t s) throw (exc)
    {
        void *ptr = ::malloc(s);
        if (s != 0 && !ptr)
        {
            throw exc(ENOMEM);
        }
        return ptr;
    }

    static void *realloc(void *p, size_t s) throw (exc)
    {
        void *ptr = ::realloc(p, s);
        if (s != 0 && !ptr)
        {
            throw exc(ENOMEM);
        }
        return ptr;
    }

public:
    
    blob() throw ()
        : _size(0), _ptr(NULL)
    {
    }

    blob(int s) throw (exc)
        : _size(to_size_t(s)), _ptr(alloc(_size))
    {
    }

    blob(int s, int n) throw (exc)
        : _size(mul(s, n)), _ptr(alloc(_size))
    {
    }

    blob(int s, int n0, int n1) throw (exc)
        : _size(mul(s, n0, n1)), _ptr(alloc(_size))
    {
    }

    blob(int s, int n0, int n1, int n2) throw (exc)
        : _size(mul(s, n0, n1, n2)), _ptr(alloc(_size))
    {
    }

    blob(long s) throw (exc)
        : _size(to_size_t(s)), _ptr(alloc(_size))
    {
    }

    blob(long s, long n) throw (exc)
        : _size(mul(s, n)), _ptr(alloc(_size))
    {
    }

    blob(long s, long n0, long n1) throw (exc)
        : _size(mul(s, n0, n1)), _ptr(alloc(_size))
    {
    }

    blob(long s, long n0, long n1, long n2) throw (exc)
        : _size(mul(s, n0, n1, n2)), _ptr(alloc(_size))
    {
    }

    blob(long long s) throw (exc)
        : _size(to_size_t(s)), _ptr(alloc(_size))
    {
    }

    blob(long long s, long long n) throw (exc)
        : _size(mul(s, n)), _ptr(alloc(_size))
    {
    }

    blob(long long s, long long n0, long long n1) throw (exc)
        : _size(mul(s, n0, n1)), _ptr(alloc(_size))
    {
    }

    blob(long long s, long long n0, long long n1, long long n2) throw (exc)
        : _size(mul(s, n0, n1, n2)), _ptr(alloc(_size))
    {
    }

    blob(unsigned int s) throw (exc)
        : _size(to_size_t(s)), _ptr(alloc(_size))
    {
    }

    blob(unsigned int s, unsigned int n) throw (exc)
        : _size(mul(s, n)), _ptr(alloc(_size))
    {
    }

    blob(unsigned int s, unsigned int n0, unsigned int n1) throw (exc)
        : _size(mul(s, n0, n1)), _ptr(alloc(_size))
    {
    }

    blob(unsigned int s, unsigned int n0, unsigned int n1, unsigned int n2) throw (exc)
        : _size(mul(s, n0, n1, n2)), _ptr(alloc(_size))
    {
    }

    blob(unsigned long s) throw (exc)
        : _size(to_size_t(s)), _ptr(alloc(_size))
    {
    }

    blob(unsigned long s, unsigned long n) throw (exc)
        : _size(mul(s, n)), _ptr(alloc(_size))
    {
    }

    blob(unsigned long s, unsigned long n0, unsigned long n1) throw (exc)
        : _size(mul(s, n0, n1)), _ptr(alloc(_size))
    {
    }

    blob(unsigned long s, unsigned long n0, unsigned long n1, unsigned long n2) throw (exc)
        : _size(mul(s, n0, n1, n2)), _ptr(alloc(_size))
    {
    }

    blob(unsigned long long s) throw (exc)
        : _size(to_size_t(s)), _ptr(alloc(_size))
    {
    }

    blob(unsigned long long s, unsigned long long n) throw (exc)
        : _size(mul(s, n)), _ptr(alloc(_size))
    {
    }

    blob(unsigned long long s, unsigned long long n0, unsigned long long n1) throw (exc)
        : _size(mul(s, n0, n1)), _ptr(alloc(_size))
    {
    }

    blob(unsigned long long s, unsigned long long n0, unsigned long long n1, unsigned long long n2) throw (exc)
        : _size(mul(s, n0, n1, n2)), _ptr(alloc(_size))
    {
    }

    blob(const blob &b) throw (exc)
        : _size(b._size), _ptr(alloc(_size))
    {
        memcpy(_ptr, b._ptr, _size);
    }

    const blob &operator=(const blob &b) throw (exc)
    {
        void *ptr = alloc(b.size());
        memcpy(ptr, b.ptr(), b.size());
        ::free(_ptr);
        _ptr = ptr;
        _size = b.size();
        return *this;
    }

    ~blob() throw ()
    {
        ::free(_ptr);
    }

    void resize(int s) throw (exc)
    {
        _ptr = realloc(_ptr, to_size_t(s));
        _size = s;
    }

    void resize(int s, int n) throw (exc)
    {
        _ptr = realloc(_ptr, mul(s, n));
        _size = s;
    }

    void resize(int s, int n0, int n1) throw (exc)
    {
        _ptr = realloc(_ptr, mul(s, n0, n1));
        _size = s;
    }

    void resize(int s, int n0, int n1, int n2) throw (exc)
    {
        _ptr = realloc(_ptr, mul(s, n0, n1, n2));
        _size = s;
    }

    void resize(long s) throw (exc)
    {
        _ptr = realloc(_ptr, to_size_t(s));
        _size = s;
    }

    void resize(long s, long n) throw (exc)
    {
        _ptr = realloc(_ptr, mul(s, n));
        _size = s;
    }

    void resize(long s, long n0, long n1) throw (exc)
    {
        _ptr = realloc(_ptr, mul(s, n0, n1));
        _size = s;
    }

    void resize(long s, long n0, long n1, long n2) throw (exc)
    {
        _ptr = realloc(_ptr, mul(s, n0, n1, n2));
        _size = s;
    }

    void resize(long long s) throw (exc)
    {
        _ptr = realloc(_ptr, to_size_t(s));
        _size = s;
    }

    void resize(long long s, long long n) throw (exc)
    {
        _ptr = realloc(_ptr, mul(s, n));
        _size = s;
    }

    void resize(long long s, long long n0, long long n1) throw (exc)
    {
        _ptr = realloc(_ptr, mul(s, n0, n1));
        _size = s;
    }

    void resize(long long s, long long n0, long long n1, long long n2) throw (exc)
    {
        _ptr = realloc(_ptr, mul(s, n0, n1, n2));
        _size = s;
    }

    void resize(unsigned int s) throw (exc)
    {
        _ptr = realloc(_ptr, to_size_t(s));
        _size = s;
    }

    void resize(unsigned int s, unsigned int n) throw (exc)
    {
        _ptr = realloc(_ptr, mul(s, n));
        _size = s;
    }

    void resize(unsigned int s, unsigned int n0, unsigned int n1) throw (exc)
    {
        _ptr = realloc(_ptr, mul(s, n0, n1));
        _size = s;
    }

    void resize(unsigned int s, unsigned int n0, unsigned int n1, unsigned int n2) throw (exc)
    {
        _ptr = realloc(_ptr, mul(s, n0, n1, n2));
        _size = s;
    }

    void resize(unsigned long s) throw (exc)
    {
        _ptr = realloc(_ptr, to_size_t(s));
        _size = s;
    }

    void resize(unsigned long s, unsigned long n) throw (exc)
    {
        _ptr = realloc(_ptr, mul(s, n));
        _size = s;
    }

    void resize(unsigned long s, unsigned long n0, unsigned long n1) throw (exc)
    {
        _ptr = realloc(_ptr, mul(s, n0, n1));
        _size = s;
    }

    void resize(unsigned long s, unsigned long n0, unsigned long n1, unsigned long n2) throw (exc)
    {
        _ptr = realloc(_ptr, mul(s, n0, n1, n2));
        _size = s;
    }

    void resize(unsigned long long s) throw (exc)
    {
        _ptr = realloc(_ptr, to_size_t(s));
        _size = s;
    }

    void resize(unsigned long long s, unsigned long long n) throw (exc)
    {
        _ptr = realloc(_ptr, mul(s, n));
        _size = s;
    }

    void resize(unsigned long long s, unsigned long long n0, unsigned long long n1) throw (exc)
    {
        _ptr = realloc(_ptr, mul(s, n0, n1));
        _size = s;
    }

    void resize(unsigned long long s, unsigned long long n0, unsigned long long n1, unsigned long long n2) throw (exc)
    {
        _ptr = realloc(_ptr, mul(s, n0, n1, n2));
        _size = s;
    }

    size_t size() const throw ()
    {
        return _size;
    }

    const void *ptr() const throw ()
    {
        return _ptr;
    }

    const void *ptr(int offset) const throw (exc)
    {
        return static_cast<const void *>(static_cast<const char *>(_ptr) + to_size_t(offset));
    }

    const void *ptr(long offset) const throw (exc)
    {
        return static_cast<const void *>(static_cast<const char *>(_ptr) + to_size_t(offset));
    }

    const void *ptr(long long offset) const throw (exc)
    {
        return static_cast<const void *>(static_cast<const char *>(_ptr) + to_size_t(offset));
    }

    const void *ptr(unsigned int offset) const throw (exc)
    {
        return static_cast<const void *>(static_cast<const char *>(_ptr) + to_size_t(offset));
    }

    const void *ptr(unsigned long offset) const throw (exc)
    {
        return static_cast<const void *>(static_cast<const char *>(_ptr) + to_size_t(offset));
    }

    const void *ptr(unsigned long long offset) const throw (exc)
    {
        return static_cast<const void *>(static_cast<const char *>(_ptr) + to_size_t(offset));
    }

    void *ptr() throw ()
    {
        return _ptr;
    }

    void *ptr(int offset) throw (exc)
    {
        return static_cast<void *>(static_cast<char *>(_ptr) + to_size_t(offset));
    }

    void *ptr(long offset) throw (exc)
    {
        return static_cast<void *>(static_cast<char *>(_ptr) + to_size_t(offset));
    }

    void *ptr(long long offset) throw (exc)
    {
        return static_cast<void *>(static_cast<char *>(_ptr) + to_size_t(offset));
    }

    void *ptr(unsigned int offset) throw (exc)
    {
        return static_cast<void *>(static_cast<char *>(_ptr) + to_size_t(offset));
    }

    void *ptr(unsigned long offset) throw (exc)
    {
        return static_cast<void *>(static_cast<char *>(_ptr) + to_size_t(offset));
    }

    void *ptr(unsigned long long offset) throw (exc)
    {
        return static_cast<void *>(static_cast<char *>(_ptr) + to_size_t(offset));
    }

    template<typename T>
    const T *ptr() const throw ()
    {
        return static_cast<const T *>(_ptr);
    }

    template<typename T>
    const T *ptr(int offset) const throw (exc)
    {
        return static_cast<const T *>(ptr(smul(to_size_t(offset), sizeof(T))));
    }

    template<typename T>
    const T *ptr(long offset) const throw (exc)
    {
        return static_cast<const T *>(ptr(smul(to_size_t(offset), sizeof(T))));
    }

    template<typename T>
    const T *ptr(long long offset) const throw (exc)
    {
        return static_cast<const T *>(ptr(smul(to_size_t(offset), sizeof(T))));
    }

    template<typename T>
    const T *ptr(unsigned int offset) const throw (exc)
    {
        return static_cast<const T *>(ptr(smul(to_size_t(offset), sizeof(T))));
    }

    template<typename T>
    const T *ptr(unsigned long offset) const throw (exc)
    {
        return static_cast<const T *>(ptr(smul(to_size_t(offset), sizeof(T))));
    }

    template<typename T>
    const T *ptr(unsigned long long offset) const throw (exc)
    {
        return static_cast<const T *>(ptr(smul(to_size_t(offset), sizeof(T))));
    }

    template<typename T>
    T *ptr() throw ()
    {
        return static_cast<T *>(_ptr);
    }

    template<typename T>
    T *ptr(int offset)
    {
        return static_cast<T *>(ptr(smul(to_size_t(offset), sizeof(T))));
    }

    template<typename T>
    T *ptr(long offset)
    {
        return static_cast<T *>(ptr(smul(to_size_t(offset), sizeof(T))));
    }

    template<typename T>
    T *ptr(long long offset)
    {
        return static_cast<T *>(ptr(smul(to_size_t(offset), sizeof(T))));
    }

    template<typename T>
    T *ptr(unsigned int offset)
    {
        return static_cast<T *>(ptr(smul(to_size_t(offset), sizeof(T))));
    }

    template<typename T>
    T *ptr(unsigned long offset)
    {
        return static_cast<T *>(ptr(smul(to_size_t(offset), sizeof(T))));
    }

    template<typename T>
    T *ptr(unsigned long long offset)
    {
        return static_cast<T *>(ptr(smul(to_size_t(offset), sizeof(T))));
    }
};

#endif
