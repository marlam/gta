/*
 * tools.h
 *
 * This file is part of gtatool, a tool to manipulate Generic Tagged Arrays
 * (GTAs).
 *
 * Copyright (C) 2009, 2010  Martin Lambers <marlam@marlam.de>
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
 * \file tools.h
 * \brief Miscellaneous tools.
 *
 * Miscellaneous tools.
 */


#ifndef TOOLS_H
#define TOOLS_H

#include <string>
#include <cstddef>
#include <cstdlib>
#include <climits>
#include <cstring>
#include <limits>
#include <cmath>
#include <stdint.h>

#include "debug.h"


namespace tools
{
    /**
     * \return          HOME directory.
     *
     * Returns the home directory (or a good match thereof).
     */
    std::string homedir();

    /**
     * \param appname   Name of the application (optional).
     * \return          Application ID.
     *
     * Generates a string that identifies an application. The generated
     * ID is unique and safe to be used as part of filenames.
     */
    std::string app_id(const std::string &appname = "");

    /**
     * \param milliseconds      Number of milliseconds to sleep.
     *
     * Sleeps for the given number of milliseconds.
     */
    void msleep(const unsigned int milliseconds);

    /**
     * \param x         An integer.
     * \param y         An integer greater than or equal to zero.
     * \return          x raised to the power of y.
     *
     * Returns \a x raised to the power of \a y.
     */
    inline int pow(int x, int y)
    {
        assert(y >= 0);

        int result = 1;
        if (x == 2 && y > 0)
        {
            result = x << (y - 1);
        }
        else
        {
            for (int i = 0; i < y; i++)
                result *= x;
        }
        return result;
    }

    /**
     * \param x         A size_t.
     * \param y         A size_t.
     * \return          x raised to the power of y.
     *
     * Returns \a x raised to the power of \a y.
     */
    inline size_t pow(size_t x, size_t y)
    {
        size_t result = 1;
        if (x == 2 && y > 0)
        {
            result = x << (y - 1);
        }
        else
        {
            for (size_t i = 0; i < y; i++)
                result *= x;
        }
        return result;
    }

    /**
     * \param a         A value.
     * \param b         A value.
     * \return          The minimum of the given values.
     *
     * Returns the minimum of the given values.
     */
    template<typename T>
    inline T min(const T &a, const T &b)
    {
        return (a < b ? a : b);
    }

    /**
     * \param a         A value.
     * \param b         A value.
     * \param c         A value.
     * \return          The minimum of the given values.
     *
     * Returns the minimum of the given values.
     */
    template<typename T>
    inline T min(const T &a, const T &b, const T &c)
    {
        return min(min(a, b), c);
    }

    /**
     * \param a         A value.
     * \param b         A value.
     * \param c         A value.
     * \param d         A value.
     * \return          The minimum of the given values.
     *
     * Returns the minimum of the given values.
     */
    template<typename T>
    inline T min(const T &a, const T &b, const T &c, const T &d)
    {
        return min(min(a, b), min(c, d));
    }

    /**
     * \param a         A value.
     * \param b         A value.
     * \return          The maximum of the given values.
     *
     * Returns the maximum of two values.
     */
    template<typename T>
    inline T max(const T &a, const T &b)
    {
        return (a > b ? a : b);
    }

    /**
     * \param a         A value.
     * \param b         A value.
     * \param c         A value.
     * \return          The maximum of the given values.
     *
     * Returns the maximum of the given values.
     */
    template<typename T>
    inline T max(const T &a, const T &b, const T &c)
    {
        return max(max(a, b), c);
    }

    /**
     * \param a         A value.
     * \param b         A value.
     * \param c         A value.
     * \param d         A value.
     * \return          The maximum of the given values.
     *
     * Returns the maximum of the given values.
     */
    template<typename T>
    inline T max(const T &a, const T &b, const T &c, const T &d)
    {
        return max(max(a, b), max(c, d));
    }

    /**
     * \param x         A value.
     * \param lo        Lower bound.
     * \param hi        Upper bound.
     *
     * Returns \a x clamped to [\a lo, \a hi].
     */
    template<typename T>
    inline T clamp(const T &x, const T &lo, const T &hi)
    {
        return min(hi, max(lo, x));
    }

    /**
     * \param c         The coordinate that should be reflected if necessary.
     * \param bound     The upper bound of the coordinate + 1 (in most cases width or height).
     * \return          The coordinate with reflective indexing applied.
     *
     * Returns the coordinate with reflective indexing applied. Every value of \a c
     * can be mapped into [0,bound-1] in this way. \a bound must be greater than
     * zero.
     */
    inline int coord_reflect(int c, int bound)
    {
        assert(bound > 0);

        int r;

        if (c < 0)
        {
            c = - (c + 1);
        }
        r = c % bound;
        if ((c / bound) % 2 == 1)
        {
            r = (bound - 1) - r;
        }
        return r;
    }

    /**
     * \param c         The coordinate that should be wrapped if necessary.
     * \param bound     The upper bound of the coordinate + 1 (in most cases width or height).
     * \return          The coordinate with wrapped indexing applied.
     *
     * Returns the coordinate with wrapped indexing applied. Every value of \a c
     * can be mapped into [0,bound-1] in this way. \a bound must be greater than
     * zero.
     */
    inline int coord_wrap(int c, int bound)
    {
        assert(bound > 0);

        int w = c % bound;
        if (c < 0 && w != 0)
        {
            w += bound;
        }
        return w;
    }

    /**
     * \param c         The coordinate that should be clamped if necessary.
     * \param bound     The upper bound of the coordinate + 1 (in most cases width or height).
     * \return          The coordinate with clamped indexing applied.
     *
     * Returns the coordinate with clamped indexing applied. Every value of \a c
     * can be mapped into [0,bound-1] in this way. \a bound must be greater than
     * zero.
     */
    inline int coord_clamp(int c, int bound)
    {
        assert(bound > 0);

        if (c < 0)
        {
            c = 0;
        }
        else if (c >= bound)
        {
            c = bound - 1;
        }
        return c;
    }


    /**
     * \param a         A number.
     * \param b         A number.
     * \return          The greatest common divisor of a and b.
     *
     * Returns the greatest common divisor of a and b.
     */
    inline int gcd(int a, int b)
    {
        int t;

        while (b != 0)
        {
            t = b;
            b = a % b;
            a = t;
        }
        return a;
    }

    /**
     * \param a         A number.
     * \param b         A number.
     * \return          The least common multiple of a and b.
     *
     * Returns the least common multiple of a and b.
     */
    inline int lcm(int a, int b)
    {
        int c = gcd(a,b);
        return (c == 0 ? 0 : (a / c) * b);
    }


    /*
     * \param a         The first factor.
     * \param b         The second factor.
     * \return          Whether the product overflows the data type.
     *
     * Checks if the product overflows the data type.
     * This is an internal function suitable for signed integer types.
     */
    template<typename T>
    inline bool _signed_product_overflows(T a, T b)
    {
        /* Adapted from the comp.lang.c FAQ, see http://c-faq.com/misc/sd26.html */
        if (a == std::numeric_limits<T>::min())
        {
            return (b != static_cast<T>(0) && b != static_cast<T>(1));
        }
        else if (b == std::numeric_limits<T>::min())
        {
            return (a != static_cast<T>(0) && a != static_cast<T>(1));
        }
        else
        {
            if (a < 0)
                a = -a;
            if (b < 0)
                b = -b;
            return !(b == 0 || !(std::numeric_limits<T>::max() / b < a));
        }
    }

    /*
     * \param a         The first factor.
     * \param b         The second factor.
     * \return          Whether the product overflows the data type.
     *
     * Checks if the product overflows the data type.
     * This is an internal function suitable for unsigned integer types.
     */
    template<typename T>
    inline bool _unsigned_product_overflows(T a, T b)
    {
        return !(b == 0 || !(std::numeric_limits<T>::max() / b < a));
    }

    inline bool product_overflows(signed char a, signed char b)
    {
        return _signed_product_overflows(a, b);
    }
    inline bool product_overflows(unsigned char a, unsigned char b)
    {
        return _unsigned_product_overflows(a, b);
    }
    inline bool product_overflows(short a, short b)
    {
        return _signed_product_overflows(a, b);
    }
    inline bool product_overflows(unsigned short a, unsigned short b)
    {
        return _unsigned_product_overflows(a, b);
    }
    inline bool product_overflows(int a, int b)
    {
        return _signed_product_overflows(a, b);
    }
    inline bool product_overflows(unsigned int a, unsigned int b)
    {
        return _unsigned_product_overflows(a, b);
    }
    inline bool product_overflows(long a, long b)
    {
        return _signed_product_overflows(a, b);
    }
    inline bool product_overflows(unsigned long a, unsigned long b)
    {
        return _unsigned_product_overflows(a, b);
    }
    inline bool product_overflows(long long a, long long b)
    {
        return _signed_product_overflows(a, b);
    }
    inline bool product_overflows(unsigned long long a, unsigned long long b)
    {
        return _unsigned_product_overflows(a, b);
    }


    /**
     * \param k         Length parameter. The length is 2k+1.
     * \param s         Standard deviation.
     * \param mask      An array with at least 2k+1 elements.
     * \param weight_sum        Storage space for the weight sum or NULL.
     *
     * Generates a 1D Gauss mask with standard deviation \a s in the (2k+1) first
     * elements of \a mask. The sum of weights in the mask is stored in \a
     * weight_sum if \a weight_sum is not NULL.
     */
    template<typename T>
    inline void gauss_mask(int k, T s, T *mask, T *weight_sum)
    {
        assert(k >= 0 && k < INT_MAX / 2 - 1);
        assert(s >= static_cast<T>(0));
        assert(mask != NULL);

        T *gauss = new T[k + 1];

        T gauss_sum = static_cast<T>(0);
        for (int i = 0; i <= k; i++)
        {
            gauss[i] = expf(- static_cast<T>(i * i) / (static_cast<T>(2) * s * s))
                / (std::sqrt(static_cast<T>(2) * static_cast<T>(M_PI)) * s);
            gauss_sum += gauss[i];
        }

        for (int i = 0; i <= k; i++)
            mask[i] = gauss[k - i];
        for (int i = k + 1; i < 2 * k + 1; i++)
            mask[i] = gauss[i - k];
        if (weight_sum)
            *weight_sum = static_cast<T>(2) * gauss_sum - gauss[0];

        delete[] gauss;
    }

    /**
     * \param n         Number of intervalls.
     * \param X         n+1 sample points.
     * \param Y         n+1 sample values.
     * \param K         Storage space for n+1 second derivatives.
     *
     * Computes a cubic spline for the \a n+1 samples given by \a X and \a Y. This is
     * done by computing the \a n+1 second derivates (curvature values) at the \a n+1
     * sample points \a X and storing them in \a K. The spline can be evaluated with
     * cspline_eval() using these values. The sample points \a X must be sorted
     * in ascending order.
     */
    template<typename T>
    inline void cspline_prep(int n, const T *X, const T *Y, T *K)
    {
        assert(n > 0);
        assert(X != NULL);
        assert(Y != NULL);
        assert(K != NULL);

        T *h = new T[n + 1];
        T *e = new T[n + 1];
        T *u = new T[n + 1];
        T *r = new T[n + 1];

        for (int i = 0; i < n; i++)
        {
            h[i] = X[i + 1] - X[i];
            e[i] = (6.0f / h[i]) * (Y[i + 1] - Y[i]);
        }
        u[1] = 2.0f * (h[0] + h[1]);
        r[1] = e[1] - e[0];
        for (int i = 2; i < n; i++)
        {
            u[i] = 2.0f * (h[i] + h[i - 1]) - (h[i - 1] * h[i - 1] / u[i - 1]);
            r[i] = (e[i] - e[i - 1]) - (r[i - 1] * h[i - 1] / u[i - 1]);
        }
        K[n] = 0.0f;
        for (int i = n - 1; i > 0; i--)
        {
            K[i] = (r[i] - h[i] * K[i + 1]) / u[i];
        }
        K[0] = 0.0f;

        delete[] h;
        delete[] e;
        delete[] u;
        delete[] r;
    }

    /**
     * \param n         Number of intervalls.
     * \param X         n+1 sample points.
     * \param Y         n+1 sample values.
     * \param K         n+1 second derivatives.
     * \param x         Evaluation point.
     * \return          The evaluation result.
     *
     * Evaluates a cubic spline given by the \a n+1 samples \a X (sorted in
     * ascending order), \a Y and the curvature values \a K that were previously
     * computed by cspline_prep(). The point \a x at which the spline is
     * evaluated must be inside the sample points (\a X[0] <= \a x <= \a X[n]).
     */
    template<typename T>
    inline T cspline_eval(int n, const T *X, const T *Y, const T *K, T x)
    {
        assert(n > 0);
        assert(X != NULL);
        assert(Y != NULL);
        assert(K != NULL);
        assert(x >= X[0]);
        assert(x <= X[n]);

        int r = 0;
        int s = n - 1;
        int i = -1;
        for (;;)
        {
            i = (r + s) / 2;
            if (x < X[i])
                s = i - 1;
            else if (x > X[i + 1])
                r = i + 1;
            else
                break;
        }

        T h = X[i + 1] - X[i];
        T a = (K[i + 1] - K[i]) / (static_cast<T>(6) * h);
        T b = K[i] / static_cast<T>(2);
        T c = (Y[i + 1] - Y[i]) / h - (h / static_cast<T>(6)) * (static_cast<T>(2) * K[i] + K[i + 1]);
        T d = Y[i];
        T xd = x - X[i];
        return a * xd * xd * xd + b * xd * xd + c * xd + d;
    }

    /**
     * \param n         The dimension n.
     * \param A         The nxn matrix A.
     * \param b         The vector b with n components.
     * \param x         The result x with n components.
     * \returns         True for success or false for error.
     *
     * Solves the linear system of equations \a A * \a x = \a b. Systems with
     * exactly one solution never lead to an error condition. The result is stored
     * in \a x. This function may modify \a A and \a b!
     */
    template<typename T>
    inline bool lse_solver(int n, T *A, T *b, T *x, T epsilon = std::numeric_limits<T>::epsilon())
    {
        assert(n >= 1);
        assert(A != NULL);
        assert(b != NULL);
        assert(x != NULL);

        /* Elimination method with pivot search */
        int i, j, k;    /* counter */
        T l;            /* factor */
        int piv_index;  /* row of the pivot element */
        T piv;          /* used for pivot search */
        T tmp;

        /* Loop over all rows */
        for (j = 0; j < n - 1; j++)
        {
            /* Pivot search */
            piv = std::abs(A[j * n + j]);
            piv_index = j;
            for (i = j + 1; i < n; i++)
            {
                if (std::abs(A[i * n + j]) > piv)
                {
                    piv = std::abs(A[i * n + j]);
                    piv_index = i;
                }
            }
            if (std::abs(piv) < epsilon)
            {
                /* No pivot element found!? */
                //fprintf(stderr, "LSE solver: no pivot found!\n");
                return false;
            }
            /* Swap rows */
            for (k = j; k < n; k++)
            {
                tmp = A[piv_index * n + k];
                A[piv_index * n + k] = A[j * n + k];
                A[j * n + k] = tmp;
            }
            tmp = b[piv_index];
            b[piv_index] = b[j];
            b[j] = tmp;

            /* Forward computation */
            for (i = j + 1; i < n; i++)
            {
                l = A[i * n + j] / A[j * n + j];
                for (k = j + 1; k < n; k++)
                {
                    A[i * n + k] -= l * A[j * n + k];
                }
                b[i] -= l * b[j];
            }
        }

        /* Backward computation */
        for (j = n - 1; j >= 0; j--)
        {
            x[j] = b[j];
            for (k = j + 1; k < n; k++)
            {
                x[j] -= A[j * n + k] * x[k];
            }
            x[j] /= A[j * n + j];
        }

        return true;
    }

    /**
     * \param a         An array.
     * \param s         The size of one array element.
     * \param n         The number of array elements.
     *
     * Shuffles the array elements using the rand(3) random number generator.
     * The caller may want to seed this RNG using srand(3) before calling this
     * function.
     */
    inline void shuffle(void *a, size_t s, size_t n)
    {
        assert(a != NULL);
        assert(s > 0);

        char *ac = static_cast<char *>(a);
        char tmp[s];
        for (size_t i = n - 1; i > 0; i--)
        {
            size_t j = rand() % (i + 1);
            memcpy(tmp, ac + i * s, s);
            memcpy(ac + i * s, ac + j * s, s);
            memcpy(ac + j * s, tmp, s);
        }
    }

    /**
     * \param x         The angle in degrees.
     * \return          The angle in radians.
     *
     * Converts degrees to radians.
     */
    inline float radians(float x)
    {
        return (x * (static_cast<float>(M_PI) / 180.0f));
    }

    /**
     * \param x         The angle in radians.
     * \return          The angle in degrees.
     *
     * Converts radians to degrees.
     */
    inline float degrees(float x)
    {
        return (x * (180.0f / static_cast<float>(M_PI)));
    }

    /**
     * \param x         The angle.
     * \return          The angle normalized to [0,2PI].
     *
     * Normalizes an angle to [0,2PI].
     */
    inline float angle_0_to_2pi(float x)
    {
        x = x / (2.0f * static_cast<float>(M_PI));
        x -= floorf(x);
        return (x * 2.0f * static_cast<float>(M_PI));
    }

    /**
     * \param x         The angle.
     * \return          The angle normalized to [-PI,+PI].
     *
     * Normalizes an angle to [-PI,+PI].
     */
    inline float angle_mpi_to_ppi(float x)
    {
        x = (x + static_cast<float>(M_PI)) / (2.0f * static_cast<float>(M_PI));
        x -= floorf(x);
        return (x * 2.0f * static_cast<float>(M_PI) - static_cast<float>(M_PI));
    }

    /**
     * \param x         An integer.
     * \return          Whether the integer is a power of two.
     *
     * Returns true if \a x is a power of two, and false otherwise.
     */
    inline bool is_power_of_two(int x)
    {
        return (x > 0 && (x & (x - 1)) == 0);
    }

    /**
     * \param x         An integer.
     * \return          The next power of two.
     *
     * Returns the next power of two that is greater than or equal to \a x.
     */
    inline int next_power_of_two(int x)
    {
        int p = 1;
        while (p < x)
            p *= 2;
        return p;
    }

    /**
     * \param a         A non-negative integer.
     * \param b         A positive integer.
     * \return          The next multiple of b relative to a.
     *
     * Returns the smallest multiple of \a b that is greater than or equal to a.
     */
    inline int next_multiple(int a, int b)
    {
        assert(a >= 0);
        assert(b > 0);

        return ((a / b) + (a % b == 0 ? 0 : 1)) * b;
    }

    /**
     * \param x         A positive integer.
     * \return          The logarithm of x with base 2.
     *
     * Returns the logarithm of x with base 2, rounded towards zero.
     */
    inline int log2(int x)
    {
        assert(x > 0);

        int log2 = 0;
        while (x > 1)
        {
            x /= 2;
            log2++;
        }
        return log2;
    }

    /**
     * \param ptr       Pointer to a 16 bit value.
     *
     * Swaps the endianness of a 16 bit value.
     */
    inline void swap_endianness_16(void *ptr)
    {
        uint16_t x;
        memcpy(&x, ptr, sizeof(uint16_t));
        x = (x << uint16_t(8)) | (x >> uint16_t(8));
        memcpy(ptr, &x, sizeof(uint16_t));
    }

    /**
     * \param ptr       Pointer to a 32 bit value.
     *
     * Swaps the endianness of a 32 bit value.
     */
    inline void swap_endianness_32(void *ptr)
    {
        uint32_t x;
        memcpy(&x, ptr, sizeof(uint32_t));
        x =   ((x                        ) << uint32_t(24))
            | ((x & uint32_t(0x0000ff00U)) << uint32_t(8))
            | ((x & uint32_t(0x00ff0000U)) >> uint32_t(8))
            | ((x                        ) >> uint32_t(24));
        memcpy(ptr, &x, sizeof(uint32_t));
    }

    /**
     * \param ptr       Pointer to a 64 bit value.
     *
     * Swaps the endianness of a 64 bit value.
     */
    inline void swap_endianness_64(void *ptr)
    {
        uint64_t x;
        memcpy(&x, ptr, sizeof(uint64_t));
        x =   ((x                                  ) << uint64_t(56))
            | ((x & uint64_t(0x000000000000ff00ULL)) << uint64_t(40))
            | ((x & uint64_t(0x0000000000ff0000ULL)) << uint64_t(24))
            | ((x & uint64_t(0x00000000ff000000ULL)) << uint64_t(8))
            | ((x & uint64_t(0x000000ff00000000ULL)) >> uint64_t(8))
            | ((x & uint64_t(0x0000ff0000000000ULL)) >> uint64_t(24))
            | ((x & uint64_t(0x00ff000000000000ULL)) >> uint64_t(40))
            | ((x                                  ) >> uint64_t(56));
        memcpy(ptr, &x, sizeof(uint64_t));
    }

    /**
     * \param ptr       Pointer to a 128 bit value.
     *
     * Swaps the endianness of a 128 bit value.
     */
    inline void swap_endianness_128(void *ptr)
    {
        /* We cannot expect uint128_t to be available. */
        uint64_t x[2];
        memcpy(x, ptr, 2 * sizeof(uint64_t));
        swap_endianness_64(x + 0);
        swap_endianness_64(x + 1);
        uint64_t tmp = x[0];
        x[0] = x[1];
        x[1] = tmp;
        memcpy(ptr, x, 2 * sizeof(uint64_t));
    }
}

#endif
