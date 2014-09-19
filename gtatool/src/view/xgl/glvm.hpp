/*
 * C++ vector and matrix classes that resemble GLSL style.
 *
 * Copyright (C) 2008, 2009, 2010, 2011, 2012, 2013, 2014
 * Martin Lambers <marlam@marlam.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of the <ORGANIZATION> nor the names of its contributors
 *   may be used to endorse or promote products derived from this software
 *   without specific prior written permission. *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * These are vector and matrix classes that resemble the GLSL types vec2, vec3,
 * vec4, mat2, mat3, mat4, mat2x3, mat3x2, mat2x4, mat4x2, mat3x4, mat4x3 (and
 * the variants bvec, ivec, dvec, dmat).
 * Additionally, there is a quaternion class (quat and dquat) and a frustum class
 * (frust and dfrust).
 *
 * Vector elements are called (x,y,z,w) and (r,g,b,a) and (s,t,p,q). Swizzling
 * is fully supported, including assignments!
 *
 * All data elements are also accessible via a member array called 'data'. This
 * array is column-major, like OpenGL.
 * Use transpose() to exchange data with row-major libraries.
 */


#ifndef GLVM_H
#define GLVM_H

#include <cmath>
#include <cstdint>
#include <limits>

namespace glvm
{
    /* Basic numeric constants. Values copied from /usr/include/math.h on a recent GNU/Linux system. */
    template<typename T> inline T const_e();       /* e */
    template<> inline long double const_e<long double>() { return 2.7182818284590452353602874713526625L; }
    template<> inline double const_e<double>() { return static_cast<double>(const_e<long double>()); }
    template<> inline float const_e<float>() { return static_cast<float>(const_e<long double>()); }
    template<typename T> inline T const_log2e();   /* log_2 e */
    template<> inline long double const_log2e<long double>() { return 1.4426950408889634073599246810018921L; }
    template<> inline double const_log2e<double>() { return const_log2e<long double>(); }
    template<> inline float const_log2e<float>() { return const_log2e<long double>(); }
    template<typename T> inline T const_log10e();  /* log_10 e */
    template<> inline long double const_log10e<long double>() { return 0.4342944819032518276511289189166051L; }
    template<> inline double const_log10e<double>() { return const_log10e<long double>(); }
    template<> inline float const_log10e<float>() { return const_log10e<long double>(); }
    template<typename T> inline T const_ln2();     /* log_e 2 */
    template<> inline long double const_ln2<long double>() { return 0.6931471805599453094172321214581766L; }
    template<> inline double const_ln2<double>() { return const_ln2<long double>(); }
    template<> inline float const_ln2<float>() { return const_ln2<long double>(); }
    template<typename T> inline T const_ln10();    /* log_e 10 */
    template<> inline long double const_ln10<long double>() { return 2.3025850929940456840179914546843642L; }
    template<> inline double const_ln10<double>() { return const_ln10<long double>(); }
    template<> inline float const_ln10<float>() { return const_ln10<long double>(); }
    template<typename T> inline T const_pi();      /* pi */
    template<> inline long double const_pi<long double>() { return 3.1415926535897932384626433832795029L; }
    template<> inline double const_pi<double>() { return const_pi<long double>(); }
    template<> inline float const_pi<float>() { return const_pi<long double>(); }
    template<typename T> inline T const_pi_2();    /* pi / 2 */
    template<> inline long double const_pi_2<long double>() { return 1.5707963267948966192313216916397514L; }
    template<> inline double const_pi_2<double>() { return const_pi_2<long double>(); }
    template<> inline float const_pi_2<float>() { return const_pi_2<long double>(); }
    template<typename T> inline T const_pi_4();    /* pi / 4 */
    template<> inline long double const_pi_4<long double>() { return 0.7853981633974483096156608458198757L; }
    template<> inline double const_pi_4<double>() { return const_pi_4<long double>(); }
    template<> inline float const_pi_4<float>() { return const_pi_4<long double>(); }
    template<typename T> inline T const_1_pi();    /* 1 / pi */
    template<> inline long double const_1_pi<long double>() { return 0.3183098861837906715377675267450287L; }
    template<> inline double const_1_pi<double>() { return const_1_pi<long double>(); }
    template<> inline float const_1_pi<float>() { return const_1_pi<long double>(); }
    template<typename T> inline T const_2_pi();    /* 2 / pi */
    template<> inline long double const_2_pi<long double>() { return 0.6366197723675813430755350534900574L; }
    template<> inline double const_2_pi<double>() { return const_2_pi<long double>(); }
    template<> inline float const_2_pi<float>() { return const_2_pi<long double>(); }
    template<typename T> inline T const_2_sqrtpi(); /* 2 / sqrt(pi) */
    template<> inline long double const_2_sqrtpi<long double>() { return 1.1283791670955125738961589031215452L; }
    template<> inline double const_2_sqrtpi<double>() { return const_2_sqrtpi<long double>(); }
    template<> inline float const_2_sqrtpi<float>() { return const_2_sqrtpi<long double>(); }
    template<typename T> inline T const_sqrt2();    /* sqrt(2) */
    template<> inline long double const_sqrt2<long double>() { return 1.4142135623730950488016887242096981L; }
    template<> inline double const_sqrt2<double>() { return const_sqrt2<long double>(); }
    template<> inline float const_sqrt2<float>() { return const_sqrt2<long double>(); }
    template<typename T> inline T const_sqrt1_2();  /* 1 / sqrt(2) */
    template<> inline long double const_sqrt1_2<long double>() { return 0.7071067811865475244008443621048490L; }
    template<> inline double const_sqrt1_2<double>() { return const_sqrt1_2<long double>(); }
    template<> inline float const_sqrt1_2<float>() { return const_sqrt1_2<long double>(); }

    /* Define the GLSL functions for the subset of the base data types
     * for which the function makes sense. The base data types are:
     *
     * bool
     * signed char, unsigned char
     * short, unsigned short
     * int, unsigned int
     * long, unsigned long
     * long long, unsigned long long
     * float, double, long double
     */

    inline signed char min(signed char x, signed char y)
    {
        return (x < y ? x : y);
    }

    inline unsigned char min(unsigned char x, unsigned char y)
    {
        return (x < y ? x : y);
    }

    inline short min(short x, short y)
    {
        return (x < y ? x : y);
    }

    inline unsigned short min(unsigned short x, unsigned short y)
    {
        return (x < y ? x : y);
    }

    inline int min(int x, int y)
    {
        return (x < y ? x : y);
    }

    inline unsigned int min(unsigned int x, unsigned int y)
    {
        return (x < y ? x : y);
    }

    inline long min(long x, long y)
    {
        return (x < y ? x : y);
    }

    inline unsigned long min(unsigned long x, unsigned long y)
    {
        return (x < y ? x : y);
    }

    inline long long min(long long x, long long y)
    {
        return (x < y ? x : y);
    }

    inline unsigned long long min(unsigned long long x, unsigned long long y)
    {
        return (x < y ? x : y);
    }

    inline float min(float x, float y)
    {
        return (x < y ? x : y);
    }

    inline double min(double x, double y)
    {
        return (x < y ? x : y);
    }

    inline long double min(long double x, long double y)
    {
        return (x < y ? x : y);
    }

    // Bonus: min() for 3 and 4 arguments
    template<typename T> inline T min(T x, T y, T z) { return min(min(x, y), z); }
    template<typename T> inline T min(T x, T y, T z, T w) { return min(min(min(x, y), z), w); }

    inline signed char max(signed char x, signed char y)
    {
        return (x > y ? x : y);
    }

    inline unsigned char max(unsigned char x, unsigned char y)
    {
        return (x > y ? x : y);
    }

    inline short max(short x, short y)
    {
        return (x > y ? x : y);
    }

    inline unsigned short max(unsigned short x, unsigned short y)
    {
        return (x > y ? x : y);
    }

    inline int max(int x, int y)
    {
        return (x > y ? x : y);
    }

    inline unsigned int max(unsigned int x, unsigned int y)
    {
        return (x > y ? x : y);
    }

    inline long max(long x, long y)
    {
        return (x > y ? x : y);
    }

    inline unsigned long max(unsigned long x, unsigned long y)
    {
        return (x > y ? x : y);
    }

    inline long long max(long long x, long long y)
    {
        return (x > y ? x : y);
    }

    inline unsigned long long max(unsigned long long x, unsigned long long y)
    {
        return (x > y ? x : y);
    }

    inline float max(float x, float y)
    {
        return (x > y ? x : y);
    }

    inline double max(double x, double y)
    {
        return (x > y ? x : y);
    }

    inline long double max(long double x, long double y)
    {
        return (x > y ? x : y);
    }

    // Bonus: max() for 3 and 4 arguments
    template<typename T> inline T max(T x, T y, T z) { return max(max(x, y), z); }
    template<typename T> inline T max(T x, T y, T z, T w) { return max(max(max(x, y), z), w); }

    inline signed char clamp(signed char x, signed char minval, signed char maxval)
    {
        return min(maxval, max(minval, x));
    }

    inline unsigned char clamp(unsigned char x, unsigned char minval, unsigned char maxval)
    {
        return min(maxval, max(minval, x));
    }

    inline short clamp(short x, short minval, short maxval)
    {
        return min(maxval, max(minval, x));
    }

    inline unsigned short clamp(unsigned short x, unsigned short minval, unsigned short maxval)
    {
        return min(maxval, max(minval, x));
    }

    inline int clamp(int x, int minval, int maxval)
    {
        return min(maxval, max(minval, x));
    }

    inline unsigned int clamp(unsigned int x, unsigned int minval, unsigned int maxval)
    {
        return min(maxval, max(minval, x));
    }

    inline long clamp(long x, long minval, long maxval)
    {
        return min(maxval, max(minval, x));
    }

    inline unsigned long clamp(unsigned long x, unsigned long minval, unsigned long maxval)
    {
        return min(maxval, max(minval, x));
    }

    inline long long clamp(long long x, long long minval, long long maxval)
    {
        return min(maxval, max(minval, x));
    }

    inline unsigned long long clamp(unsigned long long x, unsigned long long minval, unsigned long long maxval)
    {
        return min(maxval, max(minval, x));
    }

    inline float clamp(float x, float minval, float maxval)
    {
        return min(maxval, max(minval, x));
    }

    inline double clamp(double x, double minval, double maxval)
    {
        return min(maxval, max(minval, x));
    }

    inline long double clamp(long double x, long double minval, long double maxval)
    {
        return min(maxval, max(minval, x));
    }

    inline signed char step(signed char x, signed char edge)
    {
        return (x < edge ? 0 : 1);
    }

    inline unsigned char step(unsigned char x, unsigned char edge)
    {
        return (x < edge ? 0 : 1);
    }

    inline short step(short x, short edge)
    {
        return (x < edge ? 0 : 1);
    }

    inline unsigned short step(unsigned short x, unsigned short edge)
    {
        return (x < edge ? 0 : 1);
    }

    inline int step(int x, int edge)
    {
        return (x < edge ? 0 : 1);
    }

    inline unsigned int step(unsigned int x, unsigned int edge)
    {
        return (x < edge ? 0 : 1);
    }

    inline long step(long x, long edge)
    {
        return (x < edge ? 0 : 1);
    }

    inline unsigned long step(unsigned long x, unsigned long edge)
    {
        return (x < edge ? 0 : 1);
    }

    inline long long step(long long x, long long edge)
    {
        return (x < edge ? 0 : 1);
    }

    inline unsigned long long step(unsigned long long x, unsigned long long edge)
    {
        return (x < edge ? 0 : 1);
    }

    inline float step(float x, float edge)
    {
        return (x < edge ? 0.0f : 1.0f);
    }

    inline double step(double x, double edge)
    {
        return (x < edge ? 0.0 : 1.0);
    }

    inline long double step(long double x, long double edge)
    {
        return (x < edge ? 0.0L : 1.0L);
    }

    inline signed char mod(signed char x, signed char y)
    {
        return x - (x / y) * y;
    }

    inline unsigned char mod(unsigned char x, unsigned char y)
    {
        return x - (x / y) * y;
    }

    inline short mod(short x, short y)
    {
        return x - (x / y) * y;
    }

    inline unsigned short mod(unsigned short x, unsigned short y)
    {
        return x - (x / y) * y;
    }

    inline int mod(int x, int y)
    {
        return x - (x / y) * y;
    }

    inline unsigned int mod(unsigned int x, unsigned int y)
    {
        return x - (x / y) * y;
    }

    inline long mod(long x, long y)
    {
        return x - (x / y) * y;
    }

    inline unsigned long mod(unsigned long x, unsigned long y)
    {
        return x - (x / y) * y;
    }

    inline long long mod(long long x, long long y)
    {
        return x - (x / y) * y;
    }

    inline unsigned long long mod(unsigned long long x, unsigned long long y)
    {
        return x - (x / y) * y;
    }

    inline float mod(float x, float y)
    {
        return x - std::floor(x / y) * y;
    }

    inline double mod(double x, double y)
    {
        return x - std::floor(x / y) * y;
    }

    inline long double mod(long double x, long double y)
    {
        return x - std::floor(x / y) * y;
    }

    inline signed char sign(signed char x)
    {
        return (x < 0 ? -1 : x > 0 ? +1 : 0);
    }

    inline unsigned char sign(unsigned char x)
    {
        return (x > 0 ? +1 : 0);
    }

    inline short sign(short x)
    {
        return (x < 0 ? -1 : x > 0 ? +1 : 0);
    }

    inline unsigned short sign(unsigned short x)
    {
        return (x > 0 ? +1 : 0);
    }

    inline int sign(int x)
    {
        return (x < 0 ? -1 : x > 0 ? +1 : 0);
    }

    inline unsigned int sign(unsigned int x)
    {
        return (x > 0 ? +1 : 0);
    }

    inline long sign(long x)
    {
        return (x < 0 ? -1 : x > 0 ? +1 : 0);
    }

    inline unsigned long sign(unsigned long x)
    {
        return (x > 0 ? +1 : 0);
    }

    inline long long sign(long long x)
    {
        return (x < 0 ? -1 : x > 0 ? +1 : 0);
    }

    inline unsigned long long sign(unsigned long long x)
    {
        return (x > 0 ? +1 : 0);
    }

    inline float sign(float x)
    {
        return (x < 0.0f ? -1.0f : x > 0.0f ? +1.0f : 0.0f);
    }

    inline double sign(double x)
    {
        return (x < 0.0 ? -1.0 : x > 0.0 ? +1.0 : 0.0);
    }

    inline long double sign(long double x)
    {
        return (x < 0.0L ? -1.0L : x > 0.0L ? +1.0L : 0.0L);
    }

    using std::abs;

    inline bool abs(bool x)
    {
        return x;
    }

    inline signed char abs(signed char x)
    {
        return (x < 0 ? -x : x);
    }

    inline unsigned char abs(unsigned char x)
    {
        return x;
    }

    inline short abs(short x)
    {
        return (x < 0 ? -x : x);
    }

    inline unsigned short abs(unsigned short x)
    {
        return x;
    }

    inline unsigned int abs(unsigned int x)
    {
        return x;
    }

    inline unsigned long abs(unsigned long x)
    {
        return x;
    }

    inline unsigned long long abs(unsigned long long x)
    {
        return x;
    }

    inline float radians(float x)
    {
        return x * (const_pi<float>() / 180.0f);
    }

    inline double radians(double x)
    {
        return x * (const_pi<double>() / 180.0);
    }

    inline long double radians(long double x)
    {
        return x * (const_pi<long double>() / 180.0L);
    }

    inline float degrees(float x)
    {
        return x * (180.0f / const_pi<float>());
    }

    inline double degrees(double x)
    {
        return x * (180.0 / const_pi<double>());
    }

    inline long double degrees(long double x)
    {
        return x * (180.0L / const_pi<long double>());
    }

    using std::sin;

    using std::cos;

    using std::tan;

    using std::asin;

    using std::acos;

    using std::atan;

    inline float atan(float x, float y)
    {
        return std::atan2(x, y);
    }

    inline double atan(double x, double y)
    {
        return std::atan2(x, y);
    }

    inline long double atan(long double x, long double y)
    {
        return std::atan2(x, y);
    }

    using std::pow;

    using std::exp;

    using std::exp2;

    using std::log;

    using std::log2;

    using std::log10;

    using std::sqrt;

    inline float inversesqrt(float x)
    {
        return 1.0f / std::sqrt(x);
    }

    inline double inversesqrt(double x)
    {
        return 1.0 / std::sqrt(x);
    }

    inline long double inversesqrt(long double x)
    {
        return 1.0L / std::sqrt(x);
    }

    using std::cbrt;

    using std::round;

    using std::floor;

    using std::ceil;

    inline float fract(float x)
    {
        return x - std::floor(x);
    }

    inline double fract(double x)
    {
        return x - std::floor(x);
    }

    inline long double fract(long double x)
    {
        return x - std::floor(x);
    }

    using std::isfinite;

    using std::isnan;

    using std::isinf;

    using std::isnormal;

    inline float mix(float x, float y, float alpha)
    {
        return x * (1.0f - alpha) + y * alpha;
    }

    inline double mix(double x, double y, double alpha)
    {
        return x * (1.0 - alpha) + y * alpha;
    }

    inline long double mix(long double x, long double y, long double alpha)
    {
        return x * (1.0L - alpha) + y * alpha;
    }

    inline float smoothstep(float x, float edge0, float edge1)
    {
        float t = clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
        return t * t * (3.0f - t * 2.0f);
    }

    inline double smoothstep(double x, double edge0, double edge1)
    {
        double t = clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0);
        return t * t * (3.0 - t * 2.0);
    }

    inline long double smoothstep(long double x, long double edge0, long double edge1)
    {
        long double t = clamp((x - edge0) / (edge1 - edge0), 0.0L, 1.0L);
        return t * t * (3.0L - t * 2.0L);
    }

    inline bool greaterThan(bool a, bool b)
    {
        return a > b;
    }

    inline bool greaterThan(signed char a, signed char b)
    {
        return a > b;
    }

    inline bool greaterThan(unsigned char a, unsigned char b)
    {
        return a > b;
    }

    inline bool greaterThan(short a, short b)
    {
        return a > b;
    }

    inline bool greaterThan(unsigned short a, unsigned short b)
    {
        return a > b;
    }

    inline bool greaterThan(int a, int b)
    {
        return a > b;
    }

    inline bool greaterThan(unsigned int a, unsigned int b)
    {
        return a > b;
    }

    inline bool greaterThan(long a, long b)
    {
        return a > b;
    }

    inline bool greaterThan(unsigned long a, unsigned long b)
    {
        return a > b;
    }

    inline bool greaterThan(long long a, long long b)
    {
        return a > b;
    }

    inline bool greaterThan(unsigned long long a, unsigned long long b)
    {
        return a > b;
    }

    inline bool greaterThan(float a, float b)
    {
        return std::isgreater(a, b);
    }

    inline bool greaterThan(double a, double b)
    {
        return std::isgreater(a, b);
    }

    inline bool greaterThan(long double a, long double b)
    {
        return std::isgreater(a, b);
    }

    inline bool greaterThanEqual(bool a, bool b)
    {
        return a >= b;
    }

    inline bool greaterThanEqual(signed char a, signed char b)
    {
        return a >= b;
    }

    inline bool greaterThanEqual(unsigned char a, unsigned char b)
    {
        return a >= b;
    }

    inline bool greaterThanEqual(short a, short b)
    {
        return a >= b;
    }

    inline bool greaterThanEqual(unsigned short a, unsigned short b)
    {
        return a >= b;
    }

    inline bool greaterThanEqual(int a, int b)
    {
        return a >= b;
    }

    inline bool greaterThanEqual(unsigned int a, unsigned int b)
    {
        return a >= b;
    }

    inline bool greaterThanEqual(long a, long b)
    {
        return a >= b;
    }

    inline bool greaterThanEqual(unsigned long a, unsigned long b)
    {
        return a >= b;
    }

    inline bool greaterThanEqual(long long a, long long b)
    {
        return a >= b;
    }

    inline bool greaterThanEqual(unsigned long long a, unsigned long long b)
    {
        return a >= b;
    }

    inline bool greaterThanEqual(float a, float b)
    {
        return std::isgreaterequal(a, b);
    }

    inline bool greaterThanEqual(double a, double b)
    {
        return std::isgreaterequal(a, b);
    }

    inline bool greaterThanEqual(long double a, long double b)
    {
        return std::isgreaterequal(a, b);
    }

    inline bool lessThan(bool a, bool b)
    {
        return a < b;
    }

    inline bool lessThan(signed char a, signed char b)
    {
        return a < b;
    }

    inline bool lessThan(unsigned char a, unsigned char b)
    {
        return a < b;
    }

    inline bool lessThan(short a, short b)
    {
        return a < b;
    }

    inline bool lessThan(unsigned short a, unsigned short b)
    {
        return a < b;
    }

    inline bool lessThan(int a, int b)
    {
        return a < b;
    }

    inline bool lessThan(unsigned int a, unsigned int b)
    {
        return a < b;
    }

    inline bool lessThan(long a, long b)
    {
        return a < b;
    }

    inline bool lessThan(unsigned long a, unsigned long b)
    {
        return a < b;
    }

    inline bool lessThan(long long a, long long b)
    {
        return a < b;
    }

    inline bool lessThan(unsigned long long a, unsigned long long b)
    {
        return a < b;
    }

    inline bool lessThan(float a, float b)
    {
        return std::isless(a, b);
    }

    inline bool lessThan(double a, double b)
    {
        return std::isless(a, b);
    }

    inline bool lessThan(long double a, long double b)
    {
        return std::isless(a, b);
    }

    inline bool lessThanEqual(bool a, bool b)
    {
        return a <= b;
    }

    inline bool lessThanEqual(signed char a, signed char b)
    {
        return a <= b;
    }

    inline bool lessThanEqual(unsigned char a, unsigned char b)
    {
        return a <= b;
    }

    inline bool lessThanEqual(short a, short b)
    {
        return a <= b;
    }

    inline bool lessThanEqual(unsigned short a, unsigned short b)
    {
        return a <= b;
    }

    inline bool lessThanEqual(int a, int b)
    {
        return a <= b;
    }

    inline bool lessThanEqual(unsigned int a, unsigned int b)
    {
        return a <= b;
    }

    inline bool lessThanEqual(long a, long b)
    {
        return a <= b;
    }

    inline bool lessThanEqual(unsigned long a, unsigned long b)
    {
        return a <= b;
    }

    inline bool lessThanEqual(long long a, long long b)
    {
        return a <= b;
    }

    inline bool lessThanEqual(unsigned long long a, unsigned long long b)
    {
        return a <= b;
    }

    inline bool lessThanEqual(float a, float b)
    {
        return std::islessequal(a, b);
    }

    inline bool lessThanEqual(double a, double b)
    {
        return std::islessequal(a, b);
    }

    inline bool lessThanEqual(long double a, long double b)
    {
        return std::islessequal(a, b);
    }

    inline bool equal(bool a, bool b)
    {
        return a == b;
    }

    inline bool equal(signed char a, signed char b)
    {
        return a == b;
    }

    inline bool equal(unsigned char a, unsigned char b)
    {
        return a == b;
    }

    inline bool equal(short a, short b)
    {
        return a == b;
    }

    inline bool equal(unsigned short a, unsigned short b)
    {
        return a == b;
    }

    inline bool equal(int a, int b)
    {
        return a == b;
    }

    inline bool equal(unsigned int a, unsigned int b)
    {
        return a == b;
    }

    inline bool equal(long a, long b)
    {
        return a == b;
    }

    inline bool equal(unsigned long a, unsigned long b)
    {
        return a == b;
    }

    inline bool equal(long long a, long long b)
    {
        return a == b;
    }

    inline bool equal(unsigned long long a, unsigned long long b)
    {
        return a == b;
    }

    inline bool equal(float a, float b)
    {
        return greaterThanEqual(a, b) && lessThanEqual(a, b);
    }

    inline bool equal(double a, double b)
    {
        return greaterThanEqual(a, b) && lessThanEqual(a, b);
    }

    inline bool equal(long double a, long double b)
    {
        return greaterThanEqual(a, b) && lessThanEqual(a, b);
    }

    inline bool notEqual(bool a, bool b)
    {
        return a != b;
    }

    inline bool notEqual(signed char a, signed char b)
    {
        return a != b;
    }

    inline bool notEqual(unsigned char a, unsigned char b)
    {
        return a != b;
    }

    inline bool notEqual(short a, short b)
    {
        return a != b;
    }

    inline bool notEqual(unsigned short a, unsigned short b)
    {
        return a != b;
    }

    inline bool notEqual(int a, int b)
    {
        return a != b;
    }

    inline bool notEqual(unsigned int a, unsigned int b)
    {
        return a != b;
    }

    inline bool notEqual(long a, long b)
    {
        return a != b;
    }

    inline bool notEqual(unsigned long a, unsigned long b)
    {
        return a != b;
    }

    inline bool notEqual(long long a, long long b)
    {
        return a != b;
    }

    inline bool notEqual(unsigned long long a, unsigned long long b)
    {
        return a != b;
    }

    inline bool notEqual(float a, float b)
    {
        return !equal(a, b);
    }

    inline bool notEqual(double a, double b)
    {
        return !equal(a, b);
    }

    inline bool notEqual(long double a, long double b)
    {
        return !equal(a, b);
    }

    inline bool any(bool a)
    {
        return a;
    }

    inline bool all(bool a)
    {
        return a;
    }

    inline bool negate(bool a)
    {
        return !a;
    }

    // Bonus: log2 for positive integers
    template<typename T> inline T _log2(T x)
    {
        const T zero = static_cast<T>(0);
        const T one = static_cast<T>(1);
#ifdef __GNUC__
        return x < one ? zero : (std::numeric_limits<unsigned int>::digits - 1 - __builtin_clz(x));
#else
        T r = zero;
        while (x > one) {
            x >>= one;
            r++;
        }
        return r;
#endif
    }
    template<typename T> inline T _log2l(T x)
    {
#ifdef __GNUC__
        const T zero = static_cast<T>(0);
        const T one = static_cast<T>(1);
        return x < one ? zero : (std::numeric_limits<unsigned long>::digits - 1 - __builtin_clzl(x));
#else
        return _log2(x);
#endif
    }
    template<typename T> inline T _log2ll(T x)
    {
#ifdef __GNUC__
        const T zero = static_cast<T>(0);
        const T one = static_cast<T>(1);
        return x < one ? zero : (std::numeric_limits<unsigned long long>::digits - 1 - __builtin_clzll(x));
#else
        return _log2(x);
#endif
    }
    inline signed char log2(signed char x) { return _log2(x); }
    inline unsigned char log2(unsigned char x) { return _log2(x); }
    inline short log2(short x) { return _log2(x); }
    inline unsigned short log2(unsigned short x) { return _log2(x); }
    inline int log2(int x) { return _log2(x); }
    inline unsigned int log2(unsigned int x) { return _log2(x); }
    inline long log2(long x) { return _log2l(x); }
    inline unsigned long log2(unsigned long x) { return _log2l(x); }
    inline long long log2(long long x) { return _log2ll(x); }
    inline unsigned long long log2(unsigned long long x) { return _log2ll(x); }

    // Bonus: power-of-two check for positive integers
    template<typename T> inline bool _is_pow2(T x)
    {
        const T zero = static_cast<T>(0);
        const T one = static_cast<T>(1);
        return (x > zero && (x & (x - one)) == zero);
    }
    inline bool is_pow2(signed char x) { return _is_pow2(x); }
    inline bool is_pow2(unsigned char x) { return _is_pow2(x); }
    inline bool is_pow2(short x) { return _is_pow2(x); }
    inline bool is_pow2(unsigned short x) { return _is_pow2(x); }
    inline bool is_pow2(int x) { return _is_pow2(x); }
    inline bool is_pow2(unsigned int x) { return _is_pow2(x); }
    inline bool is_pow2(long x) { return _is_pow2(x); }
    inline bool is_pow2(unsigned long x) { return _is_pow2(x); }
    inline bool is_pow2(long long x) { return _is_pow2(x); }
    inline bool is_pow2(unsigned long long x) { return _is_pow2(x); }

    // Bonus: return the next power of two, or x itself if it already is a power of two
    template<typename T> inline T _next_pow2(T x)
    {
        const T zero = static_cast<T>(0);
        const T one = static_cast<T>(1);
        return (x < one ? one : (x & (x - one)) == zero ? x : one << (log2(x) + one));
    }
    inline signed char next_pow2(signed char x) { return _next_pow2(x); }
    inline unsigned char next_pow2(unsigned char x) { return _next_pow2(x); }
    inline short next_pow2(short x) { return _next_pow2(x); }
    inline unsigned short next_pow2(unsigned short x) { return _next_pow2(x); }
    inline int next_pow2(int x) { return _next_pow2(x); }
    inline unsigned int next_pow2(unsigned int x) { return _next_pow2(x); }
    inline long next_pow2(long x) { return _next_pow2(x); }
    inline unsigned long next_pow2(unsigned long x) { return _next_pow2(x); }
    inline long long next_pow2(long long x) { return _next_pow2(x); }
    inline unsigned long long next_pow2(unsigned long long x) { return _next_pow2(x); }

    // Bonus: return the next multiple of b (> 0) that is greater than or equal to a (>= 0).
    template<typename T> inline T _next_multiple(T a, T b)
    {
        const T zero = static_cast<T>(0);
        const T one = static_cast<T>(1);
        return ((a / b) + (a % b == zero ? zero : one)) * b;
    }
    inline signed char next_multiple(signed char a, signed char b) { return _next_multiple(a, b); }
    inline unsigned char next_multiple(unsigned char a, unsigned char b) { return _next_multiple(a, b); }
    inline short next_multiple(short a, short b) { return _next_multiple(a, b); }
    inline unsigned short next_multiple(unsigned short a, unsigned short b) { return _next_multiple(a, b); }
    inline int next_multiple(int a, int b) { return _next_multiple(a, b); }
    inline unsigned int next_multiple(unsigned int a, unsigned int b) { return _next_multiple(a, b); }
    inline long next_multiple(long a, long b) { return _next_multiple(a, b); }
    inline unsigned long next_multiple(unsigned long a, unsigned long b) { return _next_multiple(a, b); }
    inline long long next_multiple(long long a, long long b) { return _next_multiple(a, b); }
    inline unsigned long long next_multiple(unsigned long long a, unsigned long long b) { return _next_multiple(a, b); }



    /* Base class for vector and matrix data. Provides component-wise operations. */

    template<typename T, int S> class Data
    {
    public:
        T data[S];

        /* Operators */

        T& operator[](unsigned int i)
        {
            return data[i];
        }

        T operator[](unsigned int i) const
        {
            return data[i];
        }

        const Data& _op_assign(const Data& a)
        {
            for (int i = 0; i < S; i++)
                data[i] = a.data[i];
            return *this;
        }

        Data _op_plus(const Data& a) const
        {
            Data r;
            for (int i = 0; i < S; i++)
                r.data[i] = data[i] + a.data[i];
            return r;
        }

        const Data& _op_plus_assign(const Data& a)
        {
            for (int i = 0; i < S; i++)
                data[i] += a.data[i];
            return *this;
        }

        Data _op_unary_plus() const
        {
            return *this;
        }

        Data _op_minus(const Data& a) const
        {
            Data r;
            for (int i = 0; i < S; i++)
                r.data[i] = data[i] - a.data[i];
            return r;
        }

        const Data& _op_minus_assign(const Data& a)
        {
            for (int i = 0; i < S; i++)
                data[i] -= a.data[i];
            return *this;
        }

        Data _op_unary_minus() const
        {
            Data r;
            for (int i = 0; i < S; i++)
                r.data[i] = -data[i];
            return r;
        }

        Data _op_scal_mult(const T s) const
        {
            Data r;
            for (int i = 0; i < S; i++)
                r.data[i] = data[i] * s;
            return r;
        }

        const Data& _op_scal_mult_assign(const T s)
        {
            for (int i = 0; i < S; i++)
                data[i] *= s;
            return *this;
        }

        Data _op_comp_mult(const Data& a) const
        {
            Data r;
            for (int i = 0; i < S; i++)
                r.data[i] = data[i] * a.data[i];
            return r;
        }

        const Data& _op_comp_mult_assign(const Data& a)
        {
            for (int i = 0; i < S; i++)
                data[i] *= a.data[i];
            return *this;
        }

        Data _op_scal_div(const T s) const
        {
            Data r;
            for (int i = 0; i < S; i++)
                r.data[i] = data[i] / s;
            return r;
        }

        const Data& _op_scal_div_assign(const T s)
        {
            for (int i = 0; i < S; i++)
                data[i] /= s;
            return *this;
        }

        Data _op_comp_div(const Data& a) const
        {
            Data r;
            for (int i = 0; i < S; i++)
                r.data[i] = data[i] / a.data[i];
            return r;
        }

        const Data& _op_comp_div_assign(const Data& a)
        {
            for (int i = 0; i < S; i++)
                data[i] /= a.data[i];
            return *this;
        }

        Data _op_scal_mod(const T s) const
        {
            Data r;
            for (int i = 0; i < S; i++)
                r.data[i] = data[i] % s;
            return r;
        }

        const Data& _op_scal_mod_assign(const T s)
        {
            for (int i = 0; i < S; i++)
                data[i] %= s;
            return *this;
        }

        Data _op_comp_mod(const Data& a) const
        {
            Data r;
            for (int i = 0; i < S; i++)
                r.data[i] = data[i] % a.data[i];
            return r;
        }

        const Data& _op_comp_mod_assign(const Data& a)
        {
            for (int i = 0; i < S; i++)
                data[i] %= a.data[i];
            return *this;
        }

        bool _op_equal(const Data& a) const
        {
            bool ret = true;
            for (int i = 0; i < S; i++) {
                if (!equal(data[i], a.data[i])) {
                    ret = false;
                    break;
                }
            }
            return ret;
        }

        bool _op_notequal(const Data& a) const
        {
            return !this->data._op_equal(a);
        }

        /* Trigonometric functions */

        Data _sin() const
        {
            Data r;
            for (int i = 0; i < S; i++)
                r.data[i] = sin(data[i]);
            return r;
        }

        Data _cos() const
        {
            Data r;
            for (int i = 0; i < S; i++)
                r.data[i] = cos(data[i]);
            return r;
        }

        Data _tan() const
        {
            Data r;
            for (int i = 0; i < S; i++)
                r.data[i] = tan(data[i]);
            return r;
        }

        Data _asin() const
        {
            Data r;
            for (int i = 0; i < S; i++)
                r.data[i] = asin(data[i]);
            return r;
        }

        Data _acos() const
        {
            Data r;
            for (int i = 0; i < S; i++)
                r.data[i] = acos(data[i]);
            return r;
        }

        Data _atan() const
        {
            Data r;
            for (int i = 0; i < S; i++)
                r.data[i] = atan(data[i]);
            return r;
        }

        Data _atan(const Data& a) const
        {
            Data r;
            for (int i = 0; i < S; i++)
                r.data[i] = atan(data[i], a.data[i]);
            return r;
        }

        Data _radians() const
        {
            Data r;
            for (int i = 0; i < S; i++)
                r.data[i] = radians(data[i]);
            return r;
        }

        Data _degrees() const
        {
            Data r;
            for (int i = 0; i < S; i++)
                r.data[i] = degrees(data[i]);
            return r;
        }

        /* Exponential functions */

        Data _pow(const T p) const
        {
            Data r;
            for (int i = 0; i < S; i++)
                r.data[i] = pow(data[i], p);
            return r;
        }

        Data _exp() const
        {
            Data r;
            for (int i = 0; i < S; i++)
                r.data[i] = exp(data[i]);
            return r;
        }

        Data _exp2() const
        {
            Data r;
            for (int i = 0; i < S; i++)
                r.data[i] = exp2(data[i]);
            return r;
        }

        Data _log() const
        {
            Data r;
            for (int i = 0; i < S; i++)
                r.data[i] = log(data[i]);
            return r;
        }

        Data _log2() const
        {
            Data r;
            for (int i = 0; i < S; i++)
                r.data[i] = log2(data[i]);
            return r;
        }

        Data _log10() const
        {
            Data r;
            for (int i = 0; i < S; i++)
                r.data[i] = log10(data[i]);
            return r;
        }

        Data _sqrt() const
        {
            Data r;
            for (int i = 0; i < S; i++)
                r.data[i] = sqrt(data[i]);
            return r;
        }

        Data _inversesqrt() const
        {
            Data r;
            for (int i = 0; i < S; i++)
                r.data[i] = inversesqrt(data[i]);
            return r;
        }

        Data _cbrt() const
        {
            Data r;
            for (int i = 0; i < S; i++)
                r.data[i] = cbrt(data[i]);
            return r;
        }

        /* Common functions */

        Data<bool, S> _isfinite() const
        {
            Data<bool, S> r;
            for (int i = 0; i < S; i++)
                r.data[i] = isfinite(data[i]);
            return r;
        }

        Data<bool, S> _isinf() const
        {
            Data<bool, S> r;
            for (int i = 0; i < S; i++)
                r.data[i] = isinf(data[i]);
            return r;
        }

        Data<bool, S> _isnan() const
        {
            Data<bool, S> r;
            for (int i = 0; i < S; i++)
                r.data[i] = isnan(data[i]);
            return r;
        }

        Data<bool, S> _isnormal() const
        {
            Data<bool, S> r;
            for (int i = 0; i < S; i++)
                r.data[i] = isnormal(data[i]);
            return r;
        }

        Data _sign() const
        {
            Data r;
            for (int i = 0; i < S; i++)
                r.data[i] = sign(data[i]);
            return r;
        }

        Data _abs() const
        {
            Data r;
            for (int i = 0; i < S; i++)
                r.data[i] = abs(data[i]);
            return r;
        }

        Data _floor() const
        {
            Data r;
            for (int i = 0; i < S; i++)
                r.data[i] = floor(data[i]);
            return r;
        }

        Data _ceil() const
        {
            Data r;
            for (int i = 0; i < S; i++)
                r.data[i] = ceil(data[i]);
            return r;
        }

        Data _round() const
        {
            Data r;
            for (int i = 0; i < S; i++)
                r.data[i] = round(data[i]);
            return r;
        }

        Data _fract() const
        {
            Data r;
            for (int i = 0; i < S; i++)
                r.data[i] = fract(data[i]);
            return r;
        }

        Data _min(const T x) const
        {
            Data r;
            for (int i = 0; i < S; i++)
                r.data[i] = min(x, data[i]);
            return r;
        }

        Data _min(const Data& a) const
        {
            Data r;
            for (int i = 0; i < S; i++)
                r.data[i] = min(a.data[i], data[i]);
            return r;
        }

        Data _max(const T x) const
        {
            Data r;
            for (int i = 0; i < S; i++)
                r.data[i] = max(x, data[i]);
            return r;
        }

        Data _max(const Data& a) const
        {
            Data r;
            for (int i = 0; i < S; i++)
                r.data[i] = max(a.data[i], data[i]);
            return r;
        }

        Data _clamp(const T minval, const T maxval) const
        {
            Data r;
            for (int i = 0; i < S; i++)
                r.data[i] = clamp(data[i], minval, maxval);
            return r;
        }

        Data _clamp(const Data& minval, const Data& maxval) const
        {
            Data r;
            for (int i = 0; i < S; i++)
                r.data[i] = clamp(data[i], minval.data[i], maxval.data[i]);
            return r;
        }

        Data _mix(const Data& a, const T alpha) const
        {
            Data r;
            for (int i = 0; i < S; i++)
                r.data[i] = mix(data[i], a.data[i], alpha);
            return r;
        }

        Data _mix(const Data& a, const Data& alpha) const
        {
            Data r;
            for (int i = 0; i < S; i++)
                r.data[i] = mix(data[i], a.data[i], alpha.data[i]);
            return r;
        }

        Data _step(const T edge) const
        {
            Data r;
            for (int i = 0; i < S; i++)
                r.data[i] = step(data[i], edge);
            return r;
        }

        Data _step(const Data& edge) const
        {
            Data r;
            for (int i = 0; i < S; i++)
                r.data[i] = step(data[i], edge.data[i]);
            return r;
        }

        Data _smoothstep(const T edge0, const T edge1) const
        {
            Data r;
            for (int i = 0; i < S; i++)
                r.data[i] = smoothstep(data[i], edge0, edge1);
            return r;
        }

        Data _smoothstep(const Data& edge0, const Data& edge1) const
        {
            Data r;
            for (int i = 0; i < S; i++)
                r.data[i] = smoothstep(data[i], edge0.data[i], edge1.data[i]);
            return r;
        }

        Data _mod(const T y) const
        {
            Data r;
            for (int i = 0; i < S; i++)
                r.data[i] = mod(data[i], y);
            return r;
        }

        Data _mod(const Data& y) const
        {
            Data r;
            for (int i = 0; i < S; i++)
                r.data[i] = mod(data[i], y.data[i]);
            return r;
        }

        /* Comparison functions */

        Data<bool, S> _greaterThan(const Data& a) const
        {
            Data<bool, S> r;
            for (int i = 0; i < S; i++)
                r.data[i] = greaterThan(data[i], a.data[i]);
            return r;
        }

        Data<bool, S> _greaterThanEqual(const Data& a) const
        {
            Data<bool, S> r;
            for (int i = 0; i < S; i++)
                r.data[i] = greaterThanEqual(data[i], a.data[i]);
            return r;
        }

        Data<bool, S> _lessThan(const Data& a) const
        {
            Data<bool, S> r;
            for (int i = 0; i < S; i++)
                r.data[i] = lessThan(data[i], a.data[i]);
            return r;
        }

        Data<bool, S> _lessThanEqual(const Data& a) const
        {
            Data<bool, S> r;
            for (int i = 0; i < S; i++)
                r.data[i] = lessThanEqual(data[i], a.data[i]);
            return r;
        }

        Data<bool, S> _equal(const Data& a) const
        {
            Data<bool, S> r;
            for (int i = 0; i < S; i++)
                r.data[i] = equal(data[i], a.data[i]);
            return r;
        }

        Data<bool, S> _notEqual(const Data& a) const
        {
            Data<bool, S> r;
            for (int i = 0; i < S; i++)
                r.data[i] = notEqual(data[i], a.data[i]);
            return r;
        }

        bool _any() const
        {
            bool ret = false;
            for (int i = 0; i < S; i++) {
                if (data[i]) {
                    ret = true;
                    break;
                }
            }
            return ret;
        }

        bool _all() const
        {
            bool ret = true;
            for (int i = 0; i < S; i++) {
                if (!data[i]) {
                    ret = false;
                    break;
                }
            }
            return ret;
        }

        Data<bool, S> _negate() const
        {
            Data<bool, S> r;
            for (int i = 0; i < S; i++)
                r.data[i] = !data[i];
            return r;
        }

        /* Geometric functions */

        T _length() const
        {
            T l = static_cast<T>(0);
            for (int i = 0; i < S; i++)
                l += data[i] * data[i];
            return sqrt(l);
        }

        T _distance(const Data& a) const
        {
            return (*this - a).data._length();
        }

        T _dot(const Data& a) const
        {
            T d = static_cast<T>(0);
            for (int i = 0; i < S; i++)
                d += data[i] * a.data[i];
            return d;
        }

        Data _normalize() const
        {
            return this->_op_scal_div(this->_length());
        }

        // Bonus functions. Note that log2 is already covered.

        Data<bool, S> _is_pow2() const
        {
            Data<bool, S> r;
            for (int i = 0; i < S; i++)
                r.data[i] = is_pow2(data[i]);
            return r;
        }

        Data _next_pow2() const
        {
            Data r;
            for (int i = 0; i < S; i++)
                r.data[i] = next_pow2(data[i]);
            return r;
        }

        Data _next_multiple(T y) const
        {
            Data r;
            for (int i = 0; i < S; i++)
                r.data[i] = next_multiple(data[i], y);
            return r;
        }

        Data _next_multiple(const Data& y) const
        {
            Data r;
            for (int i = 0; i < S; i++)
                r.data[i] = next_multiple(data[i], y.data[i]);
            return r;
        }
    };


    /* Vectors */

    template<typename T, int S> class vector
    {
    public:
        Data<T, S> data;
    };

    template<typename T,
        int C,  // number of components of this swizzler
        int S,  // number of components of underlying data
        int I, int J, int K, int L // indices of data components to read when creating a swizzler
            > class swizzler
    {
    public:
        T data[S];

        swizzler(const vector<T, C>& d)
        {
            static_assert(C >= 2 && C <= 4, "swizzlers must have 2, 3, or 4 components");
            static_assert(I != J && I != K && I != L
                    && J != K && J != L
                    && (K == 9 || K != L)
                    , "cannot assign to ambiguous swizzler");
            data[0] = d.data[I];
            data[1] = d.data[J];
            if (C > 2) data[2] = d.data[K];
            if (C > 3) data[3] = d.data[L];
        }

        template<int SS, int SI, int SJ, int SK, int SL>
        swizzler(const swizzler<T, C, SS, SI, SJ, SK, SL>& s) : swizzler(vector<T, C>(s)) {}
    };

    template<typename T> class vector<T, 2>
    {
    public:
        union {
            Data<T, 2> data;
            struct {
                union {
                    T x, s, r;
                };
                union {
                    T y, t, g;
                };
            };
            swizzler<T, 2, 2, 0, 0, 9, 9> xx, rr, ss;
            swizzler<T, 2, 2, 0, 1, 9, 9> xy, rg, st;
            swizzler<T, 2, 2, 1, 0, 9, 9> yx, gr, ts;
            swizzler<T, 2, 2, 1, 1, 9, 9> yy, gg, tt;
            swizzler<T, 3, 2, 0, 0, 0, 9> xxx, rrr, sss;
            swizzler<T, 3, 2, 0, 0, 1, 9> xxy, rrg, sst;
            swizzler<T, 3, 2, 0, 1, 0, 9> xyx, rgr, sts;
            swizzler<T, 3, 2, 0, 1, 1, 9> xyy, rgg, stt;
            swizzler<T, 3, 2, 1, 0, 0, 9> yxx, grr, tss;
            swizzler<T, 3, 2, 1, 0, 1, 9> yxy, grg, tst;
            swizzler<T, 3, 2, 1, 1, 0, 9> yyx, ggr, tts;
            swizzler<T, 3, 2, 1, 1, 1, 9> yyy, ggg, ttt;
            swizzler<T, 4, 2, 0, 0, 0, 0> xxxx, rrrr, ssss;
            swizzler<T, 4, 2, 0, 0, 0, 1> xxxy, rrrg, ssst;
            swizzler<T, 4, 2, 0, 0, 1, 0> xxyx, rrgr, ssts;
            swizzler<T, 4, 2, 0, 0, 1, 1> xxyy, rrgg, sstt;
            swizzler<T, 4, 2, 0, 1, 0, 0> xyxx, rgrr, stss;
            swizzler<T, 4, 2, 0, 1, 0, 1> xyxy, rgrg, stst;
            swizzler<T, 4, 2, 0, 1, 1, 0> xyyx, rggr, stts;
            swizzler<T, 4, 2, 0, 1, 1, 1> xyyy, rggg, sttt;
            swizzler<T, 4, 2, 1, 0, 0, 0> yxxx, grrr, tsss;
            swizzler<T, 4, 2, 1, 0, 0, 1> yxxy, grrg, tsst;
            swizzler<T, 4, 2, 1, 0, 1, 0> yxyx, grgr, tsts;
            swizzler<T, 4, 2, 1, 0, 1, 1> yxyy, grgg, tstt;
            swizzler<T, 4, 2, 1, 1, 0, 0> yyxx, ggrr, ttss;
            swizzler<T, 4, 2, 1, 1, 0, 1> yyxy, ggrg, ttst;
            swizzler<T, 4, 2, 1, 1, 1, 0> yyyx, gggr, ttts;
            swizzler<T, 4, 2, 1, 1, 1, 1> yyyy, gggg, tttt;
        };

        vector() {}

        vector(T x, T y)
        {
            data[0] = x;
            data[1] = y;
        }

        vector(T x) : vector(x, x) {}
        vector(const vector<T, 2>& xy) : vector(xy.data[0], xy.data[1]) {}
        vector(const Data<T, 2>& xy) : vector(xy.data[0], xy.data[1]) {}
        vector(const T* xy) : vector(xy[0], xy[1]) {}
        template<typename U> vector(const vector<U, 2>& xy) : vector(xy.data[0], xy.data[1]) {}

        template<int S, int I, int J, int K, int L>
        vector(const swizzler<T, 2, S, I, J, K, L>& s)
        : vector(s.data[I], s.data[J]) {}

        operator const T*() const { return data; }
        T& operator[](unsigned int i) { return data[i]; }
        T operator[](unsigned int i) const { return data[i]; }
    };

    template<typename T> class vector<T, 3>
    {
    public:
        union {
            Data<T, 3> data;
            struct {
                union {
                    T x, s, r;
                };
                union {
                    T y, t, g;
                };
                union {
                    T z, p, b;
                };
            };
            swizzler<T, 2, 3, 0, 0, 9, 9> xx, rr, ss;
            swizzler<T, 2, 3, 0, 1, 9, 9> xy, rg, st;
            swizzler<T, 2, 3, 0, 2, 9, 9> xz, rb, sp;
            swizzler<T, 2, 3, 1, 0, 9, 9> yx, gr, ts;
            swizzler<T, 2, 3, 1, 1, 9, 9> yy, gg, tt;
            swizzler<T, 2, 3, 1, 2, 9, 9> yz, gb, tp;
            swizzler<T, 2, 3, 2, 0, 9, 9> zx, br, ps;
            swizzler<T, 2, 3, 2, 1, 9, 9> zy, bg, pt;
            swizzler<T, 2, 3, 2, 2, 9, 9> zz, bb, pp;
            swizzler<T, 3, 3, 0, 0, 0, 9> xxx, rrr, sss;
            swizzler<T, 3, 3, 0, 0, 1, 9> xxy, rrg, sst;
            swizzler<T, 3, 3, 0, 0, 2, 9> xxz, rrb, ssp;
            swizzler<T, 3, 3, 0, 1, 0, 9> xyx, rgr, sts;
            swizzler<T, 3, 3, 0, 1, 1, 9> xyy, rgg, stt;
            swizzler<T, 3, 3, 0, 1, 2, 9> xyz, rgb, stp;
            swizzler<T, 3, 3, 0, 2, 0, 9> xzx, rbr, sps;
            swizzler<T, 3, 3, 0, 2, 1, 9> xzy, rbg, spt;
            swizzler<T, 3, 3, 0, 2, 2, 9> xzz, rbb, spp;
            swizzler<T, 3, 3, 1, 0, 0, 9> yxx, grr, tss;
            swizzler<T, 3, 3, 1, 0, 1, 9> yxy, grg, tst;
            swizzler<T, 3, 3, 1, 0, 2, 9> yxz, grb, tsp;
            swizzler<T, 3, 3, 1, 1, 0, 9> yyx, ggr, tts;
            swizzler<T, 3, 3, 1, 1, 1, 9> yyy, ggg, ttt;
            swizzler<T, 3, 3, 1, 1, 2, 9> yyz, ggb, ttp;
            swizzler<T, 3, 3, 1, 2, 0, 9> yzx, gbr, tps;
            swizzler<T, 3, 3, 1, 2, 1, 9> yzy, gbg, tpt;
            swizzler<T, 3, 3, 1, 2, 2, 9> yzz, gbb, tpp;
            swizzler<T, 3, 3, 2, 0, 0, 9> zxx, brr, pss;
            swizzler<T, 3, 3, 2, 0, 1, 9> zxy, brg, pst;
            swizzler<T, 3, 3, 2, 0, 2, 9> zxz, brb, psp;
            swizzler<T, 3, 3, 2, 1, 0, 9> zyx, bgr, pts;
            swizzler<T, 3, 3, 2, 1, 1, 9> zyy, bgg, ptt;
            swizzler<T, 3, 3, 2, 1, 2, 9> zyz, bgb, ptp;
            swizzler<T, 3, 3, 2, 2, 0, 9> zzx, bbr, pps;
            swizzler<T, 3, 3, 2, 2, 1, 9> zzy, bbg, ppt;
            swizzler<T, 3, 3, 2, 2, 2, 9> zzz, bbb, ppp;
            swizzler<T, 4, 3, 0, 0, 0, 0> xxxx, rrrr, ssss;
            swizzler<T, 4, 3, 0, 0, 0, 1> xxxy, rrrg, ssst;
            swizzler<T, 4, 3, 0, 0, 0, 2> xxxz, rrrb, sssp;
            swizzler<T, 4, 3, 0, 0, 1, 0> xxyx, rrgr, ssts;
            swizzler<T, 4, 3, 0, 0, 1, 1> xxyy, rrgg, sstt;
            swizzler<T, 4, 3, 0, 0, 1, 2> xxyz, rrgb, sstp;
            swizzler<T, 4, 3, 0, 0, 2, 0> xxzx, rrbr, ssps;
            swizzler<T, 4, 3, 0, 0, 2, 1> xxzy, rrbg, sspt;
            swizzler<T, 4, 3, 0, 0, 2, 2> xxzz, rrbb, sspp;
            swizzler<T, 4, 3, 0, 1, 0, 0> xyxx, rgrr, stss;
            swizzler<T, 4, 3, 0, 1, 0, 1> xyxy, rgrg, stst;
            swizzler<T, 4, 3, 0, 1, 0, 2> xyxz, rgrb, stsp;
            swizzler<T, 4, 3, 0, 1, 1, 0> xyyx, rggr, stts;
            swizzler<T, 4, 3, 0, 1, 1, 1> xyyy, rggg, sttt;
            swizzler<T, 4, 3, 0, 1, 1, 2> xyyz, rggb, sttp;
            swizzler<T, 4, 3, 0, 1, 2, 0> xyzx, rgbr, stps;
            swizzler<T, 4, 3, 0, 1, 2, 1> xyzy, rgbg, stpt;
            swizzler<T, 4, 3, 0, 1, 2, 2> xyzz, rgbb, stpp;
            swizzler<T, 4, 3, 0, 2, 0, 0> xzxx, rbrr, spss;
            swizzler<T, 4, 3, 0, 2, 0, 1> xzxy, rbrg, spst;
            swizzler<T, 4, 3, 0, 2, 0, 2> xzxz, rbrb, spsp;
            swizzler<T, 4, 3, 0, 2, 1, 0> xzyx, rbgr, spts;
            swizzler<T, 4, 3, 0, 2, 1, 1> xzyy, rbgg, sptt;
            swizzler<T, 4, 3, 0, 2, 1, 2> xzyz, rbgb, sptp;
            swizzler<T, 4, 3, 0, 2, 2, 0> xzzx, rbbr, spps;
            swizzler<T, 4, 3, 0, 2, 2, 1> xzzy, rbbg, sppt;
            swizzler<T, 4, 3, 0, 2, 2, 2> xzzz, rbbb, sppp;
            swizzler<T, 4, 3, 1, 0, 0, 0> yxxx, grrr, tsss;
            swizzler<T, 4, 3, 1, 0, 0, 1> yxxy, grrg, tsst;
            swizzler<T, 4, 3, 1, 0, 0, 2> yxxz, grrb, tssp;
            swizzler<T, 4, 3, 1, 0, 1, 0> yxyx, grgr, tsts;
            swizzler<T, 4, 3, 1, 0, 1, 1> yxyy, grgg, tstt;
            swizzler<T, 4, 3, 1, 0, 1, 2> yxyz, grgb, tstp;
            swizzler<T, 4, 3, 1, 0, 2, 0> yxzx, grbr, tsps;
            swizzler<T, 4, 3, 1, 0, 2, 1> yxzy, grbg, tspt;
            swizzler<T, 4, 3, 1, 0, 2, 2> yxzz, grbb, tspp;
            swizzler<T, 4, 3, 1, 1, 0, 0> yyxx, ggrr, ttss;
            swizzler<T, 4, 3, 1, 1, 0, 1> yyxy, ggrg, ttst;
            swizzler<T, 4, 3, 1, 1, 0, 2> yyxz, ggrb, ttsp;
            swizzler<T, 4, 3, 1, 1, 1, 0> yyyx, gggr, ttts;
            swizzler<T, 4, 3, 1, 1, 1, 1> yyyy, gggg, tttt;
            swizzler<T, 4, 3, 1, 1, 1, 2> yyyz, gggb, tttp;
            swizzler<T, 4, 3, 1, 1, 2, 0> yyzx, ggbr, ttps;
            swizzler<T, 4, 3, 1, 1, 2, 1> yyzy, ggbg, ttpt;
            swizzler<T, 4, 3, 1, 1, 2, 2> yyzz, ggbb, ttpp;
            swizzler<T, 4, 3, 1, 2, 0, 0> yzxx, gbrr, tpss;
            swizzler<T, 4, 3, 1, 2, 0, 1> yzxy, gbrg, tpst;
            swizzler<T, 4, 3, 1, 2, 0, 2> yzxz, gbrb, tpsp;
            swizzler<T, 4, 3, 1, 2, 1, 0> yzyx, gbgr, tpts;
            swizzler<T, 4, 3, 1, 2, 1, 1> yzyy, gbgg, tptt;
            swizzler<T, 4, 3, 1, 2, 1, 2> yzyz, gbgb, tptp;
            swizzler<T, 4, 3, 1, 2, 2, 0> yzzx, gbbr, tpps;
            swizzler<T, 4, 3, 1, 2, 2, 1> yzzy, gbbg, tppt;
            swizzler<T, 4, 3, 1, 2, 2, 2> yzzz, gbbb, tppp;
            swizzler<T, 4, 3, 2, 0, 0, 0> zxxx, brrr, psss;
            swizzler<T, 4, 3, 2, 0, 0, 1> zxxy, brrg, psst;
            swizzler<T, 4, 3, 2, 0, 0, 2> zxxz, brrb, pssp;
            swizzler<T, 4, 3, 2, 0, 1, 0> zxyx, brgr, psts;
            swizzler<T, 4, 3, 2, 0, 1, 1> zxyy, brgg, pstt;
            swizzler<T, 4, 3, 2, 0, 1, 2> zxyz, brgb, pstp;
            swizzler<T, 4, 3, 2, 0, 2, 0> zxzx, brbr, psps;
            swizzler<T, 4, 3, 2, 0, 2, 1> zxzy, brbg, pspt;
            swizzler<T, 4, 3, 2, 0, 2, 2> zxzz, brbb, pspp;
            swizzler<T, 4, 3, 2, 1, 0, 0> zyxx, bgrr, ptss;
            swizzler<T, 4, 3, 2, 1, 0, 1> zyxy, bgrg, ptst;
            swizzler<T, 4, 3, 2, 1, 0, 2> zyxz, bgrb, ptsp;
            swizzler<T, 4, 3, 2, 1, 1, 0> zyyx, bggr, ptts;
            swizzler<T, 4, 3, 2, 1, 1, 1> zyyy, bggg, pttt;
            swizzler<T, 4, 3, 2, 1, 1, 2> zyyz, bggb, pttp;
            swizzler<T, 4, 3, 2, 1, 2, 0> zyzx, bgbr, ptps;
            swizzler<T, 4, 3, 2, 1, 2, 1> zyzy, bgbg, ptpt;
            swizzler<T, 4, 3, 2, 1, 2, 2> zyzz, bgbb, ptpp;
            swizzler<T, 4, 3, 2, 2, 0, 0> zzxx, bbrr, ppss;
            swizzler<T, 4, 3, 2, 2, 0, 1> zzxy, bbrg, ppst;
            swizzler<T, 4, 3, 2, 2, 0, 2> zzxz, bbrb, ppsp;
            swizzler<T, 4, 3, 2, 2, 1, 0> zzyx, bbgr, ppts;
            swizzler<T, 4, 3, 2, 2, 1, 1> zzyy, bbgg, pptt;
            swizzler<T, 4, 3, 2, 2, 1, 2> zzyz, bbgb, pptp;
            swizzler<T, 4, 3, 2, 2, 2, 0> zzzx, bbbr, ppps;
            swizzler<T, 4, 3, 2, 2, 2, 1> zzzy, bbbg, pppt;
            swizzler<T, 4, 3, 2, 2, 2, 2> zzzz, bbbb, pppp;
        };

        vector() {}

        vector(T x, T y, T z)
        {
            data[0] = x;
            data[1] = y;
            data[2] = z;
        }

        vector(T x) : vector(x, x, x) {}
        vector(const vector<T, 2>& xy, T z) : vector(xy.data[0], xy.data[1], z) {}
        vector(T x, const vector<T, 2>& yz) : vector(x, yz.data[0], yz.data[1]) {}
        vector(const vector<T, 3>& xyz) : vector(xyz.data[0], xyz.data[1], xyz.data[2]) {}
        vector(const Data<T, 3>& xyz) : vector(xyz.data[0], xyz.data[1], xyz.data[2]) {}
        vector(const T* xyz) : vector(xyz[0], xyz[1], xyz[2]) {}
        template<typename U> vector(const vector<U, 2>& xyz) : vector(xyz.data[0], xyz.data[1], xyz.data[2]) {}

        template<int S, int I, int J, int K, int L>
        vector(const swizzler<T, 3, S, I, J, K, L>& s)
        : vector(s.data[I], s.data[J], s.data[K]) {}

        operator const T*() const { return data; }
        T& operator[](unsigned int i) { return data[i]; }
        T operator[](unsigned int i) const { return data[i]; }
    };

    template<typename T> class vector<T, 4>
    {
    public:
        union {
            Data<T, 4> data;
            struct {
                union {
                    T x, s, r;
                };
                union {
                    T y, t, g;
                };
                union {
                    T z, p, b;
                };
                union {
                    T w, q, a;
                };
            };
            swizzler<T, 2, 4, 0, 0, 9, 9> xx, rr, ss;
            swizzler<T, 2, 4, 0, 1, 9, 9> xy, rg, st;
            swizzler<T, 2, 4, 0, 2, 9, 9> xz, rb, sp;
            swizzler<T, 2, 4, 0, 3, 9, 9> xw, ra, sq;
            swizzler<T, 2, 4, 1, 0, 9, 9> yx, gr, ts;
            swizzler<T, 2, 4, 1, 1, 9, 9> yy, gg, tt;
            swizzler<T, 2, 4, 1, 2, 9, 9> yz, gb, tp;
            swizzler<T, 2, 4, 1, 3, 9, 9> yw, ga, tq;
            swizzler<T, 2, 4, 2, 0, 9, 9> zx, br, ps;
            swizzler<T, 2, 4, 2, 1, 9, 9> zy, bg, pt;
            swizzler<T, 2, 4, 2, 2, 9, 9> zz, bb, pp;
            swizzler<T, 2, 4, 2, 3, 9, 9> zw, ba, pq;
            swizzler<T, 2, 4, 3, 0, 9, 9> wx, ar, qs;
            swizzler<T, 2, 4, 3, 1, 9, 9> wy, ag, qt;
            swizzler<T, 2, 4, 3, 2, 9, 9> wz, ab, qp;
            swizzler<T, 2, 4, 3, 3, 9, 9> ww, aa, qq;
            swizzler<T, 3, 4, 0, 0, 0, 9> xxx, rrr, sss;
            swizzler<T, 3, 4, 0, 0, 1, 9> xxy, rrg, sst;
            swizzler<T, 3, 4, 0, 0, 2, 9> xxz, rrb, ssp;
            swizzler<T, 3, 4, 0, 0, 3, 9> xxw, rra, ssq;
            swizzler<T, 3, 4, 0, 1, 0, 9> xyx, rgr, sts;
            swizzler<T, 3, 4, 0, 1, 1, 9> xyy, rgg, stt;
            swizzler<T, 3, 4, 0, 1, 2, 9> xyz, rgb, stp;
            swizzler<T, 3, 4, 0, 1, 3, 9> xyw, rga, stq;
            swizzler<T, 3, 4, 0, 2, 0, 9> xzx, rbr, sps;
            swizzler<T, 3, 4, 0, 2, 1, 9> xzy, rbg, spt;
            swizzler<T, 3, 4, 0, 2, 2, 9> xzz, rbb, spp;
            swizzler<T, 3, 4, 0, 2, 3, 9> xzw, rba, spq;
            swizzler<T, 3, 4, 0, 3, 0, 9> xwx, rar, sqs;
            swizzler<T, 3, 4, 0, 3, 1, 9> xwy, rag, sqt;
            swizzler<T, 3, 4, 0, 3, 2, 9> xwz, rab, sqp;
            swizzler<T, 3, 4, 0, 3, 3, 9> xww, raa, sqq;
            swizzler<T, 3, 4, 1, 0, 0, 9> yxx, grr, tss;
            swizzler<T, 3, 4, 1, 0, 1, 9> yxy, grg, tst;
            swizzler<T, 3, 4, 1, 0, 2, 9> yxz, grb, tsp;
            swizzler<T, 3, 4, 1, 0, 3, 9> yxw, gra, tsq;
            swizzler<T, 3, 4, 1, 1, 0, 9> yyx, ggr, tts;
            swizzler<T, 3, 4, 1, 1, 1, 9> yyy, ggg, ttt;
            swizzler<T, 3, 4, 1, 1, 2, 9> yyz, ggb, ttp;
            swizzler<T, 3, 4, 1, 1, 3, 9> yyw, gga, ttq;
            swizzler<T, 3, 4, 1, 2, 0, 9> yzx, gbr, tps;
            swizzler<T, 3, 4, 1, 2, 1, 9> yzy, gbg, tpt;
            swizzler<T, 3, 4, 1, 2, 2, 9> yzz, gbb, tpp;
            swizzler<T, 3, 4, 1, 2, 3, 9> yzw, gba, tpq;
            swizzler<T, 3, 4, 1, 3, 0, 9> ywx, gar, tqs;
            swizzler<T, 3, 4, 1, 3, 1, 9> ywy, gag, tqt;
            swizzler<T, 3, 4, 1, 3, 2, 9> ywz, gab, tqp;
            swizzler<T, 3, 4, 1, 3, 3, 9> yww, gaa, tqq;
            swizzler<T, 3, 4, 2, 0, 0, 9> zxx, brr, pss;
            swizzler<T, 3, 4, 2, 0, 1, 9> zxy, brg, pst;
            swizzler<T, 3, 4, 2, 0, 2, 9> zxz, brb, psp;
            swizzler<T, 3, 4, 2, 0, 3, 9> zxw, bra, psq;
            swizzler<T, 3, 4, 2, 1, 0, 9> zyx, bgr, pts;
            swizzler<T, 3, 4, 2, 1, 1, 9> zyy, bgg, ptt;
            swizzler<T, 3, 4, 2, 1, 2, 9> zyz, bgb, ptp;
            swizzler<T, 3, 4, 2, 1, 3, 9> zyw, bga, ptq;
            swizzler<T, 3, 4, 2, 2, 0, 9> zzx, bbr, pps;
            swizzler<T, 3, 4, 2, 2, 1, 9> zzy, bbg, ppt;
            swizzler<T, 3, 4, 2, 2, 2, 9> zzz, bbb, ppp;
            swizzler<T, 3, 4, 2, 2, 3, 9> zzw, bba, ppq;
            swizzler<T, 3, 4, 2, 3, 0, 9> zwx, bar, pqs;
            swizzler<T, 3, 4, 2, 3, 1, 9> zwy, bag, pqt;
            swizzler<T, 3, 4, 2, 3, 2, 9> zwz, bab, pqp;
            swizzler<T, 3, 4, 2, 3, 3, 9> zww, baa, pqq;
            swizzler<T, 3, 4, 3, 0, 0, 9> wxx, arr, qss;
            swizzler<T, 3, 4, 3, 0, 1, 9> wxy, arg, qst;
            swizzler<T, 3, 4, 3, 0, 2, 9> wxz, arb, qsp;
            swizzler<T, 3, 4, 3, 0, 3, 9> wxw, ara, qsq;
            swizzler<T, 3, 4, 3, 1, 0, 9> wyx, agr, qts;
            swizzler<T, 3, 4, 3, 1, 1, 9> wyy, agg, qtt;
            swizzler<T, 3, 4, 3, 1, 2, 9> wyz, agb, qtp;
            swizzler<T, 3, 4, 3, 1, 3, 9> wyw, aga, qtq;
            swizzler<T, 3, 4, 3, 2, 0, 9> wzx, abr, qps;
            swizzler<T, 3, 4, 3, 2, 1, 9> wzy, abg, qpt;
            swizzler<T, 3, 4, 3, 2, 2, 9> wzz, abb, qpp;
            swizzler<T, 3, 4, 3, 2, 3, 9> wzw, aba, qpq;
            swizzler<T, 3, 4, 3, 3, 0, 9> wwx, aar, qqs;
            swizzler<T, 3, 4, 3, 3, 1, 9> wwy, aag, qqt;
            swizzler<T, 3, 4, 3, 3, 2, 9> wwz, aab, qqp;
            swizzler<T, 3, 4, 3, 3, 3, 9> www, aaa, qqq;
            swizzler<T, 4, 4, 0, 0, 0, 0> xxxx, rrrr, ssss;
            swizzler<T, 4, 4, 0, 0, 0, 1> xxxy, rrrg, ssst;
            swizzler<T, 4, 4, 0, 0, 0, 2> xxxz, rrrb, sssp;
            swizzler<T, 4, 4, 0, 0, 0, 3> xxxw, rrra, sssq;
            swizzler<T, 4, 4, 0, 0, 1, 0> xxyx, rrgr, ssts;
            swizzler<T, 4, 4, 0, 0, 1, 1> xxyy, rrgg, sstt;
            swizzler<T, 4, 4, 0, 0, 1, 2> xxyz, rrgb, sstp;
            swizzler<T, 4, 4, 0, 0, 1, 3> xxyw, rrga, sstq;
            swizzler<T, 4, 4, 0, 0, 2, 0> xxzx, rrbr, ssps;
            swizzler<T, 4, 4, 0, 0, 2, 1> xxzy, rrbg, sspt;
            swizzler<T, 4, 4, 0, 0, 2, 2> xxzz, rrbb, sspp;
            swizzler<T, 4, 4, 0, 0, 2, 3> xxzw, rrba, sspq;
            swizzler<T, 4, 4, 0, 0, 3, 0> xxwx, rrar, ssqs;
            swizzler<T, 4, 4, 0, 0, 3, 1> xxwy, rrag, ssqt;
            swizzler<T, 4, 4, 0, 0, 3, 2> xxwz, rrab, ssqp;
            swizzler<T, 4, 4, 0, 0, 3, 3> xxww, rraa, ssqq;
            swizzler<T, 4, 4, 0, 1, 0, 0> xyxx, rgrr, stss;
            swizzler<T, 4, 4, 0, 1, 0, 1> xyxy, rgrg, stst;
            swizzler<T, 4, 4, 0, 1, 0, 2> xyxz, rgrb, stsp;
            swizzler<T, 4, 4, 0, 1, 0, 3> xyxw, rgra, stsq;
            swizzler<T, 4, 4, 0, 1, 1, 0> xyyx, rggr, stts;
            swizzler<T, 4, 4, 0, 1, 1, 1> xyyy, rggg, sttt;
            swizzler<T, 4, 4, 0, 1, 1, 2> xyyz, rggb, sttp;
            swizzler<T, 4, 4, 0, 1, 1, 3> xyyw, rgga, sttq;
            swizzler<T, 4, 4, 0, 1, 2, 0> xyzx, rgbr, stps;
            swizzler<T, 4, 4, 0, 1, 2, 1> xyzy, rgbg, stpt;
            swizzler<T, 4, 4, 0, 1, 2, 2> xyzz, rgbb, stpp;
            swizzler<T, 4, 4, 0, 1, 2, 3> xyzw, rgba, stpq;
            swizzler<T, 4, 4, 0, 1, 3, 0> xywx, rgar, stqs;
            swizzler<T, 4, 4, 0, 1, 3, 1> xywy, rgag, stqt;
            swizzler<T, 4, 4, 0, 1, 3, 2> xywz, rgab, stqp;
            swizzler<T, 4, 4, 0, 1, 3, 3> xyww, rgaa, stqq;
            swizzler<T, 4, 4, 0, 2, 0, 0> xzxx, rbrr, spss;
            swizzler<T, 4, 4, 0, 2, 0, 1> xzxy, rbrg, spst;
            swizzler<T, 4, 4, 0, 2, 0, 2> xzxz, rbrb, spsp;
            swizzler<T, 4, 4, 0, 2, 0, 3> xzxw, rbra, spsq;
            swizzler<T, 4, 4, 0, 2, 1, 0> xzyx, rbgr, spts;
            swizzler<T, 4, 4, 0, 2, 1, 1> xzyy, rbgg, sptt;
            swizzler<T, 4, 4, 0, 2, 1, 2> xzyz, rbgb, sptp;
            swizzler<T, 4, 4, 0, 2, 1, 3> xzyw, rbga, sptq;
            swizzler<T, 4, 4, 0, 2, 2, 0> xzzx, rbbr, spps;
            swizzler<T, 4, 4, 0, 2, 2, 1> xzzy, rbbg, sppt;
            swizzler<T, 4, 4, 0, 2, 2, 2> xzzz, rbbb, sppp;
            swizzler<T, 4, 4, 0, 2, 2, 3> xzzw, rbba, sppq;
            swizzler<T, 4, 4, 0, 2, 3, 0> xzwx, rbar, spqs;
            swizzler<T, 4, 4, 0, 2, 3, 1> xzwy, rbag, spqt;
            swizzler<T, 4, 4, 0, 2, 3, 2> xzwz, rbab, spqp;
            swizzler<T, 4, 4, 0, 2, 3, 3> xzww, rbaa, spqq;
            swizzler<T, 4, 4, 0, 3, 0, 0> xwxx, rarr, sqss;
            swizzler<T, 4, 4, 0, 3, 0, 1> xwxy, rarg, sqst;
            swizzler<T, 4, 4, 0, 3, 0, 2> xwxz, rarb, sqsp;
            swizzler<T, 4, 4, 0, 3, 0, 3> xwxw, rara, sqsq;
            swizzler<T, 4, 4, 0, 3, 1, 0> xwyx, ragr, sqts;
            swizzler<T, 4, 4, 0, 3, 1, 1> xwyy, ragg, sqtt;
            swizzler<T, 4, 4, 0, 3, 1, 2> xwyz, ragb, sqtp;
            swizzler<T, 4, 4, 0, 3, 1, 3> xwyw, raga, sqtq;
            swizzler<T, 4, 4, 0, 3, 2, 0> xwzx, rabr, sqps;
            swizzler<T, 4, 4, 0, 3, 2, 1> xwzy, rabg, sqpt;
            swizzler<T, 4, 4, 0, 3, 2, 2> xwzz, rabb, sqpp;
            swizzler<T, 4, 4, 0, 3, 2, 3> xwzw, raba, sqpq;
            swizzler<T, 4, 4, 0, 3, 3, 0> xwwx, raar, sqqs;
            swizzler<T, 4, 4, 0, 3, 3, 1> xwwy, raag, sqqt;
            swizzler<T, 4, 4, 0, 3, 3, 2> xwwz, raab, sqqp;
            swizzler<T, 4, 4, 0, 3, 3, 3> xwww, raaa, sqqq;
            swizzler<T, 4, 4, 1, 0, 0, 0> yxxx, grrr, tsss;
            swizzler<T, 4, 4, 1, 0, 0, 1> yxxy, grrg, tsst;
            swizzler<T, 4, 4, 1, 0, 0, 2> yxxz, grrb, tssp;
            swizzler<T, 4, 4, 1, 0, 0, 3> yxxw, grra, tssq;
            swizzler<T, 4, 4, 1, 0, 1, 0> yxyx, grgr, tsts;
            swizzler<T, 4, 4, 1, 0, 1, 1> yxyy, grgg, tstt;
            swizzler<T, 4, 4, 1, 0, 1, 2> yxyz, grgb, tstp;
            swizzler<T, 4, 4, 1, 0, 1, 3> yxyw, grga, tstq;
            swizzler<T, 4, 4, 1, 0, 2, 0> yxzx, grbr, tsps;
            swizzler<T, 4, 4, 1, 0, 2, 1> yxzy, grbg, tspt;
            swizzler<T, 4, 4, 1, 0, 2, 2> yxzz, grbb, tspp;
            swizzler<T, 4, 4, 1, 0, 2, 3> yxzw, grba, tspq;
            swizzler<T, 4, 4, 1, 0, 3, 0> yxwx, grar, tsqs;
            swizzler<T, 4, 4, 1, 0, 3, 1> yxwy, grag, tsqt;
            swizzler<T, 4, 4, 1, 0, 3, 2> yxwz, grab, tsqp;
            swizzler<T, 4, 4, 1, 0, 3, 3> yxww, graa, tsqq;
            swizzler<T, 4, 4, 1, 1, 0, 0> yyxx, ggrr, ttss;
            swizzler<T, 4, 4, 1, 1, 0, 1> yyxy, ggrg, ttst;
            swizzler<T, 4, 4, 1, 1, 0, 2> yyxz, ggrb, ttsp;
            swizzler<T, 4, 4, 1, 1, 0, 3> yyxw, ggra, ttsq;
            swizzler<T, 4, 4, 1, 1, 1, 0> yyyx, gggr, ttts;
            swizzler<T, 4, 4, 1, 1, 1, 1> yyyy, gggg, tttt;
            swizzler<T, 4, 4, 1, 1, 1, 2> yyyz, gggb, tttp;
            swizzler<T, 4, 4, 1, 1, 1, 3> yyyw, ggga, tttq;
            swizzler<T, 4, 4, 1, 1, 2, 0> yyzx, ggbr, ttps;
            swizzler<T, 4, 4, 1, 1, 2, 1> yyzy, ggbg, ttpt;
            swizzler<T, 4, 4, 1, 1, 2, 2> yyzz, ggbb, ttpp;
            swizzler<T, 4, 4, 1, 1, 2, 3> yyzw, ggba, ttpq;
            swizzler<T, 4, 4, 1, 1, 3, 0> yywx, ggar, ttqs;
            swizzler<T, 4, 4, 1, 1, 3, 1> yywy, ggag, ttqt;
            swizzler<T, 4, 4, 1, 1, 3, 2> yywz, ggab, ttqp;
            swizzler<T, 4, 4, 1, 1, 3, 3> yyww, ggaa, ttqq;
            swizzler<T, 4, 4, 1, 2, 0, 0> yzxx, gbrr, tpss;
            swizzler<T, 4, 4, 1, 2, 0, 1> yzxy, gbrg, tpst;
            swizzler<T, 4, 4, 1, 2, 0, 2> yzxz, gbrb, tpsp;
            swizzler<T, 4, 4, 1, 2, 0, 3> yzxw, gbra, tpsq;
            swizzler<T, 4, 4, 1, 2, 1, 0> yzyx, gbgr, tpts;
            swizzler<T, 4, 4, 1, 2, 1, 1> yzyy, gbgg, tptt;
            swizzler<T, 4, 4, 1, 2, 1, 2> yzyz, gbgb, tptp;
            swizzler<T, 4, 4, 1, 2, 1, 3> yzyw, gbga, tptq;
            swizzler<T, 4, 4, 1, 2, 2, 0> yzzx, gbbr, tpps;
            swizzler<T, 4, 4, 1, 2, 2, 1> yzzy, gbbg, tppt;
            swizzler<T, 4, 4, 1, 2, 2, 2> yzzz, gbbb, tppp;
            swizzler<T, 4, 4, 1, 2, 2, 3> yzzw, gbba, tppq;
            swizzler<T, 4, 4, 1, 2, 3, 0> yzwx, gbar, tpqs;
            swizzler<T, 4, 4, 1, 2, 3, 1> yzwy, gbag, tpqt;
            swizzler<T, 4, 4, 1, 2, 3, 2> yzwz, gbab, tpqp;
            swizzler<T, 4, 4, 1, 2, 3, 3> yzww, gbaa, tpqq;
            swizzler<T, 4, 4, 1, 3, 0, 0> ywxx, garr, tqss;
            swizzler<T, 4, 4, 1, 3, 0, 1> ywxy, garg, tqst;
            swizzler<T, 4, 4, 1, 3, 0, 2> ywxz, garb, tqsp;
            swizzler<T, 4, 4, 1, 3, 0, 3> ywxw, gara, tqsq;
            swizzler<T, 4, 4, 1, 3, 1, 0> ywyx, gagr, tqts;
            swizzler<T, 4, 4, 1, 3, 1, 1> ywyy, gagg, tqtt;
            swizzler<T, 4, 4, 1, 3, 1, 2> ywyz, gagb, tqtp;
            swizzler<T, 4, 4, 1, 3, 1, 3> ywyw, gaga, tqtq;
            swizzler<T, 4, 4, 1, 3, 2, 0> ywzx, gabr, tqps;
            swizzler<T, 4, 4, 1, 3, 2, 1> ywzy, gabg, tqpt;
            swizzler<T, 4, 4, 1, 3, 2, 2> ywzz, gabb, tqpp;
            swizzler<T, 4, 4, 1, 3, 2, 3> ywzw, gaba, tqpq;
            swizzler<T, 4, 4, 1, 3, 3, 0> ywwx, gaar, tqqs;
            swizzler<T, 4, 4, 1, 3, 3, 1> ywwy, gaag, tqqt;
            swizzler<T, 4, 4, 1, 3, 3, 2> ywwz, gaab, tqqp;
            swizzler<T, 4, 4, 1, 3, 3, 3> ywww, gaaa, tqqq;
            swizzler<T, 4, 4, 2, 0, 0, 0> zxxx, brrr, psss;
            swizzler<T, 4, 4, 2, 0, 0, 1> zxxy, brrg, psst;
            swizzler<T, 4, 4, 2, 0, 0, 2> zxxz, brrb, pssp;
            swizzler<T, 4, 4, 2, 0, 0, 3> zxxw, brra, pssq;
            swizzler<T, 4, 4, 2, 0, 1, 0> zxyx, brgr, psts;
            swizzler<T, 4, 4, 2, 0, 1, 1> zxyy, brgg, pstt;
            swizzler<T, 4, 4, 2, 0, 1, 2> zxyz, brgb, pstp;
            swizzler<T, 4, 4, 2, 0, 1, 3> zxyw, brga, pstq;
            swizzler<T, 4, 4, 2, 0, 2, 0> zxzx, brbr, psps;
            swizzler<T, 4, 4, 2, 0, 2, 1> zxzy, brbg, pspt;
            swizzler<T, 4, 4, 2, 0, 2, 2> zxzz, brbb, pspp;
            swizzler<T, 4, 4, 2, 0, 2, 3> zxzw, brba, pspq;
            swizzler<T, 4, 4, 2, 0, 3, 0> zxwx, brar, psqs;
            swizzler<T, 4, 4, 2, 0, 3, 1> zxwy, brag, psqt;
            swizzler<T, 4, 4, 2, 0, 3, 2> zxwz, brab, psqp;
            swizzler<T, 4, 4, 2, 0, 3, 3> zxww, braa, psqq;
            swizzler<T, 4, 4, 2, 1, 0, 0> zyxx, bgrr, ptss;
            swizzler<T, 4, 4, 2, 1, 0, 1> zyxy, bgrg, ptst;
            swizzler<T, 4, 4, 2, 1, 0, 2> zyxz, bgrb, ptsp;
            swizzler<T, 4, 4, 2, 1, 0, 3> zyxw, bgra, ptsq;
            swizzler<T, 4, 4, 2, 1, 1, 0> zyyx, bggr, ptts;
            swizzler<T, 4, 4, 2, 1, 1, 1> zyyy, bggg, pttt;
            swizzler<T, 4, 4, 2, 1, 1, 2> zyyz, bggb, pttp;
            swizzler<T, 4, 4, 2, 1, 1, 3> zyyw, bgga, pttq;
            swizzler<T, 4, 4, 2, 1, 2, 0> zyzx, bgbr, ptps;
            swizzler<T, 4, 4, 2, 1, 2, 1> zyzy, bgbg, ptpt;
            swizzler<T, 4, 4, 2, 1, 2, 2> zyzz, bgbb, ptpp;
            swizzler<T, 4, 4, 2, 1, 2, 3> zyzw, bgba, ptpq;
            swizzler<T, 4, 4, 2, 1, 3, 0> zywx, bgar, ptqs;
            swizzler<T, 4, 4, 2, 1, 3, 1> zywy, bgag, ptqt;
            swizzler<T, 4, 4, 2, 1, 3, 2> zywz, bgab, ptqp;
            swizzler<T, 4, 4, 2, 1, 3, 3> zyww, bgaa, ptqq;
            swizzler<T, 4, 4, 2, 2, 0, 0> zzxx, bbrr, ppss;
            swizzler<T, 4, 4, 2, 2, 0, 1> zzxy, bbrg, ppst;
            swizzler<T, 4, 4, 2, 2, 0, 2> zzxz, bbrb, ppsp;
            swizzler<T, 4, 4, 2, 2, 0, 3> zzxw, bbra, ppsq;
            swizzler<T, 4, 4, 2, 2, 1, 0> zzyx, bbgr, ppts;
            swizzler<T, 4, 4, 2, 2, 1, 1> zzyy, bbgg, pptt;
            swizzler<T, 4, 4, 2, 2, 1, 2> zzyz, bbgb, pptp;
            swizzler<T, 4, 4, 2, 2, 1, 3> zzyw, bbga, pptq;
            swizzler<T, 4, 4, 2, 2, 2, 0> zzzx, bbbr, ppps;
            swizzler<T, 4, 4, 2, 2, 2, 1> zzzy, bbbg, pppt;
            swizzler<T, 4, 4, 2, 2, 2, 2> zzzz, bbbb, pppp;
            swizzler<T, 4, 4, 2, 2, 2, 3> zzzw, bbba, pppq;
            swizzler<T, 4, 4, 2, 2, 3, 0> zzwx, bbar, ppqs;
            swizzler<T, 4, 4, 2, 2, 3, 1> zzwy, bbag, ppqt;
            swizzler<T, 4, 4, 2, 2, 3, 2> zzwz, bbab, ppqp;
            swizzler<T, 4, 4, 2, 2, 3, 3> zzww, bbaa, ppqq;
            swizzler<T, 4, 4, 2, 3, 0, 0> zwxx, barr, pqss;
            swizzler<T, 4, 4, 2, 3, 0, 1> zwxy, barg, pqst;
            swizzler<T, 4, 4, 2, 3, 0, 2> zwxz, barb, pqsp;
            swizzler<T, 4, 4, 2, 3, 0, 3> zwxw, bara, pqsq;
            swizzler<T, 4, 4, 2, 3, 1, 0> zwyx, bagr, pqts;
            swizzler<T, 4, 4, 2, 3, 1, 1> zwyy, bagg, pqtt;
            swizzler<T, 4, 4, 2, 3, 1, 2> zwyz, bagb, pqtp;
            swizzler<T, 4, 4, 2, 3, 1, 3> zwyw, baga, pqtq;
            swizzler<T, 4, 4, 2, 3, 2, 0> zwzx, babr, pqps;
            swizzler<T, 4, 4, 2, 3, 2, 1> zwzy, babg, pqpt;
            swizzler<T, 4, 4, 2, 3, 2, 2> zwzz, babb, pqpp;
            swizzler<T, 4, 4, 2, 3, 2, 3> zwzw, baba, pqpq;
            swizzler<T, 4, 4, 2, 3, 3, 0> zwwx, baar, pqqs;
            swizzler<T, 4, 4, 2, 3, 3, 1> zwwy, baag, pqqt;
            swizzler<T, 4, 4, 2, 3, 3, 2> zwwz, baab, pqqp;
            swizzler<T, 4, 4, 2, 3, 3, 3> zwww, baaa, pqqq;
            swizzler<T, 4, 4, 3, 0, 0, 0> wxxx, arrr, qsss;
            swizzler<T, 4, 4, 3, 0, 0, 1> wxxy, arrg, qsst;
            swizzler<T, 4, 4, 3, 0, 0, 2> wxxz, arrb, qssp;
            swizzler<T, 4, 4, 3, 0, 0, 3> wxxw, arra, qssq;
            swizzler<T, 4, 4, 3, 0, 1, 0> wxyx, argr, qsts;
            swizzler<T, 4, 4, 3, 0, 1, 1> wxyy, argg, qstt;
            swizzler<T, 4, 4, 3, 0, 1, 2> wxyz, argb, qstp;
            swizzler<T, 4, 4, 3, 0, 1, 3> wxyw, arga, qstq;
            swizzler<T, 4, 4, 3, 0, 2, 0> wxzx, arbr, qsps;
            swizzler<T, 4, 4, 3, 0, 2, 1> wxzy, arbg, qspt;
            swizzler<T, 4, 4, 3, 0, 2, 2> wxzz, arbb, qspp;
            swizzler<T, 4, 4, 3, 0, 2, 3> wxzw, arba, qspq;
            swizzler<T, 4, 4, 3, 0, 3, 0> wxwx, arar, qsqs;
            swizzler<T, 4, 4, 3, 0, 3, 1> wxwy, arag, qsqt;
            swizzler<T, 4, 4, 3, 0, 3, 2> wxwz, arab, qsqp;
            swizzler<T, 4, 4, 3, 0, 3, 3> wxww, araa, qsqq;
            swizzler<T, 4, 4, 3, 1, 0, 0> wyxx, agrr, qtss;
            swizzler<T, 4, 4, 3, 1, 0, 1> wyxy, agrg, qtst;
            swizzler<T, 4, 4, 3, 1, 0, 2> wyxz, agrb, qtsp;
            swizzler<T, 4, 4, 3, 1, 0, 3> wyxw, agra, qtsq;
            swizzler<T, 4, 4, 3, 1, 1, 0> wyyx, aggr, qtts;
            swizzler<T, 4, 4, 3, 1, 1, 1> wyyy, aggg, qttt;
            swizzler<T, 4, 4, 3, 1, 1, 2> wyyz, aggb, qttp;
            swizzler<T, 4, 4, 3, 1, 1, 3> wyyw, agga, qttq;
            swizzler<T, 4, 4, 3, 1, 2, 0> wyzx, agbr, qtps;
            swizzler<T, 4, 4, 3, 1, 2, 1> wyzy, agbg, qtpt;
            swizzler<T, 4, 4, 3, 1, 2, 2> wyzz, agbb, qtpp;
            swizzler<T, 4, 4, 3, 1, 2, 3> wyzw, agba, qtpq;
            swizzler<T, 4, 4, 3, 1, 3, 0> wywx, agar, qtqs;
            swizzler<T, 4, 4, 3, 1, 3, 1> wywy, agag, qtqt;
            swizzler<T, 4, 4, 3, 1, 3, 2> wywz, agab, qtqp;
            swizzler<T, 4, 4, 3, 1, 3, 3> wyww, agaa, qtqq;
            swizzler<T, 4, 4, 3, 2, 0, 0> wzxx, abrr, qpss;
            swizzler<T, 4, 4, 3, 2, 0, 1> wzxy, abrg, qpst;
            swizzler<T, 4, 4, 3, 2, 0, 2> wzxz, abrb, qpsp;
            swizzler<T, 4, 4, 3, 2, 0, 3> wzxw, abra, qpsq;
            swizzler<T, 4, 4, 3, 2, 1, 0> wzyx, abgr, qpts;
            swizzler<T, 4, 4, 3, 2, 1, 1> wzyy, abgg, qptt;
            swizzler<T, 4, 4, 3, 2, 1, 2> wzyz, abgb, qptp;
            swizzler<T, 4, 4, 3, 2, 1, 3> wzyw, abga, qptq;
            swizzler<T, 4, 4, 3, 2, 2, 0> wzzx, abbr, qpps;
            swizzler<T, 4, 4, 3, 2, 2, 1> wzzy, abbg, qppt;
            swizzler<T, 4, 4, 3, 2, 2, 2> wzzz, abbb, qppp;
            swizzler<T, 4, 4, 3, 2, 2, 3> wzzw, abba, qppq;
            swizzler<T, 4, 4, 3, 2, 3, 0> wzwx, abar, qpqs;
            swizzler<T, 4, 4, 3, 2, 3, 1> wzwy, abag, qpqt;
            swizzler<T, 4, 4, 3, 2, 3, 2> wzwz, abab, qpqp;
            swizzler<T, 4, 4, 3, 2, 3, 3> wzww, abaa, qpqq;
            swizzler<T, 4, 4, 3, 3, 0, 0> wwxx, aarr, qqss;
            swizzler<T, 4, 4, 3, 3, 0, 1> wwxy, aarg, qqst;
            swizzler<T, 4, 4, 3, 3, 0, 2> wwxz, aarb, qqsp;
            swizzler<T, 4, 4, 3, 3, 0, 3> wwxw, aara, qqsq;
            swizzler<T, 4, 4, 3, 3, 1, 0> wwyx, aagr, qqts;
            swizzler<T, 4, 4, 3, 3, 1, 1> wwyy, aagg, qqtt;
            swizzler<T, 4, 4, 3, 3, 1, 2> wwyz, aagb, qqtp;
            swizzler<T, 4, 4, 3, 3, 1, 3> wwyw, aaga, qqtq;
            swizzler<T, 4, 4, 3, 3, 2, 0> wwzx, aabr, qqps;
            swizzler<T, 4, 4, 3, 3, 2, 1> wwzy, aabg, qqpt;
            swizzler<T, 4, 4, 3, 3, 2, 2> wwzz, aabb, qqpp;
            swizzler<T, 4, 4, 3, 3, 2, 3> wwzw, aaba, qqpq;
            swizzler<T, 4, 4, 3, 3, 3, 0> wwwx, aaar, qqqs;
            swizzler<T, 4, 4, 3, 3, 3, 1> wwwy, aaag, qqqt;
            swizzler<T, 4, 4, 3, 3, 3, 2> wwwz, aaab, qqqp;
            swizzler<T, 4, 4, 3, 3, 3, 3> wwww, aaaa, qqqq;
        };

        vector() {}

        vector(T x, T y, T z, T w)
        {
            data[0] = x;
            data[1] = y;
            data[2] = z;
            data[3] = w;
        }

        vector(T x) : vector(x, x, x, x) {}
        vector(const vector<T, 2>& xy, T z, T w) : vector(xy.data[0], xy.data[1], z, w) {}
        vector(const vector<T, 2>& xy, const vector<T, 2>& zw) : vector(xy.data[0], xy.data[1], zw.data[0], zw.data[1]) {}
        vector(T x, const vector<T, 2>& yz, T w) : vector(x, yz.data[0], yz.data[1], w) {}
        vector(T x, T y, const vector<T, 2>& zw) : vector(x, y, zw.data[0], zw.data[1]) {}
        vector(const vector<T, 3>& xyz, T w) : vector(xyz.data[0], xyz.data[1], xyz.data[2], w) {}
        vector(T x, const vector<T, 3>& yzw) : vector(x, yzw.data[0], yzw.data[1], yzw.data[2]) {}
        vector(const vector<T, 4>& xyzw) : vector(xyzw.data[0], xyzw.data[1], xyzw.data[2], xyzw.data[3]) {}
        vector(const Data<T, 4>& xyzw) : vector(xyzw.data[0], xyzw.data[1], xyzw.data[2], xyzw.data[3]) {}
        vector(const T* xyzw) : vector(xyzw[0], xyzw[1], xyzw[2], xyzw[3]) {}
        template<typename U> vector(const vector<U, 2>& xyzw) : vector(xyzw.data[0], xyzw.data[1], xyzw.data[2], xyzw.data[3]) {}

        template<int S, int I, int J, int K, int L>
        vector(const swizzler<T, 4, S, I, J, K, L>& s)
        : vector(s.data[I], s.data[J], s.data[K], s.data[L]) {}

        operator const T*() const { return data; }
        T& operator[](unsigned int i) { return data[i]; }
        T operator[](unsigned int i) const { return data[i]; }
    };

    template<typename T, int S> vector<T, S>& operator+=(vector<T, S>& a, const vector<T, S>& b) { a.data._op_plus_assign(b.data); return a; }
    template<typename T, int S> vector<T, S>& operator-=(vector<T, S>& a, const vector<T, S>& b) { a.data._op_minus_assign(b.data); return a; }
    template<typename T, int S> vector<T, S>& operator*=(vector<T, S>& a, const vector<T, S>& b) { a.data._op_comp_mult_assign(b.data); return a; }
    template<typename T, int S> vector<T, S>& operator/=(vector<T, S>& a, const vector<T, S>& b) { a.data._op_comp_div_assign(b.data); return a; }
    template<typename T, int S> vector<T, S>& operator%=(vector<T, S>& a, const vector<T, S>& b) { a.data._op_comp_mod_assign(b.data); return a; }
    template<typename T, int S> vector<T, S>& operator*=(vector<T, S>& a, T s) { a.data._op_scal_mult_assign(s); return a; }
    template<typename T, int S> vector<T, S>& operator/=(vector<T, S>& a, T s) { a.data._op_scal_div_assign(s); return a; }
    template<typename T, int S> vector<T, S>& operator%=(vector<T, S>& a, T s) { a.data._op_scal_mod_assign(s); return a; }

    template<typename T, int S> vector<T, S> operator+(const vector<T, S>& a, const vector<T, S>& b) { return a.data._op_plus(b.data); }
    template<typename T, int S> vector<T, S> operator+(const vector<T, S>& a) { return a.data._op_unary_plus(); }
    template<typename T, int S> vector<T, S> operator-(const vector<T, S>& a, const vector<T, S>& b) { return a.data._op_minus(b.data); }
    template<typename T, int S> vector<T, S> operator-(const vector<T, S>& a) { return a.data._op_unary_minus(); }
    template<typename T, int S> vector<T, S> operator*(const vector<T, S>& a, const vector<T, S>& b) { return a.data._op_comp_mult(b.data); }
    template<typename T, int S> vector<T, S> operator*(T s, const vector<T, S>& a) { return a.data._op_scal_mult(s); }
    template<typename T, int S> vector<T, S> operator*(const vector<T, S>& a, T s) { return a.data._op_scal_mult(s); }
    template<typename T, int S> vector<T, S> operator/(const vector<T, S>& a, const vector<T, S>& b) { return a.data._op_comp_div(b.data); }
    template<typename T, int S> vector<T, S> operator/(const vector<T, S>& a, T s) { return a.data._op_scal_div(s); }
    template<typename T, int S> vector<T, S> operator%(const vector<T, S>& a, const vector<T, S>& b) { return a.data._op_comp_mod(b.data); }
    template<typename T, int S> vector<T, S> operator%(const vector<T, S>& a, T s) { return a.data._op_scal_mod(s); }
    template<typename T, int S> bool operator==(const vector<T, S>& a, const vector<T, S>& b) { return a.data._op_equal(b.data); }
    template<typename T, int S> bool operator!=(const vector<T, S>& a, const vector<T, S>& b) { return a.data._op_notequal(b.data); }

    template<typename T, int S> vector<T, S> sin(const vector<T, S>& v) { return v.data._sin(); }
    template<typename T, int S> vector<T, S> cos(const vector<T, S>& v) { return v.data._cos(); }
    template<typename T, int S> vector<T, S> tan(const vector<T, S>& v) { return v.data._tan(); }
    template<typename T, int S> vector<T, S> asin(const vector<T, S>& v) { return v.data._asin(); }
    template<typename T, int S> vector<T, S> acos(const vector<T, S>& v) { return v.data._acos(); }
    template<typename T, int S> vector<T, S> atan(const vector<T, S>& v) { return v.data._atan(); }
    template<typename T, int S> vector<T, S> atan(const vector<T, S>& v, const vector<T, S>& w) { return v.data._atan(w.data); }
    template<typename T, int S> vector<T, S> radians(const vector<T, S>& v) { return v.data._radians(); }
    template<typename T, int S> vector<T, S> degrees(const vector<T, S>& v) { return v.data._degrees(); }

    template<typename T, int S> vector<T, S> pow(const vector<T, S>& v, T p) { return v.data._pow(p); }
    template<typename T, int S> vector<T, S> exp(const vector<T, S>& v) { return v.data._exp(); }
    template<typename T, int S> vector<T, S> exp2(const vector<T, S>& v) { return v.data._exp2(); }
    template<typename T, int S> vector<T, S> log(const vector<T, S>& v) { return v.data._log(); }
    template<typename T, int S> vector<T, S> log2(const vector<T, S>& v) { return v.data._log2(); }
    template<typename T, int S> vector<T, S> log10(const vector<T, S>& v) { return v.data._log10(); }
    template<typename T, int S> vector<T, S> sqrt(const vector<T, S>& v) { return v.data._sqrt(); }
    template<typename T, int S> vector<T, S> inversesqrt(const vector<T, S>& v) { return v.data._inversesqrt(); }
    template<typename T, int S> vector<T, S> cbrt(const vector<T, S>& v) { return v.data._cbrt(); }

    template<typename T, int S> vector<bool, S> isfinite(const vector<T, S>& v) { return v.data._isfinite(); }
    template<typename T, int S> vector<bool, S> isinf(const vector<T, S>& v) { return v.data._isinf(); }
    template<typename T, int S> vector<bool, S> isnan(const vector<T, S>& v) { return v.data._isnan(); }
    template<typename T, int S> vector<bool, S> isnormal(const vector<T, S>& v) { return v.data._isnormal(); }

    template<typename T, int S> vector<T, S> abs(const vector<T, S>& v) { return v.data._abs(); }
    template<typename T, int S> vector<T, S> sign(const vector<T, S>& v) { return v.data._sign(); }
    template<typename T, int S> vector<T, S> floor(const vector<T, S>& v) { return v.data._floor(); }
    template<typename T, int S> vector<T, S> ceil(const vector<T, S>& v) { return v.data._ceil(); }
    template<typename T, int S> vector<T, S> round(const vector<T, S>& v) { return v.data._round(); }
    template<typename T, int S> vector<T, S> fract(const vector<T, S>& v) { return v.data._fract(); }

    template<typename T, int S> vector<T, S> min(const vector<T, S>& v, T x) { return v.data._min(x); }
    template<typename T, int S> vector<T, S> min(const vector<T, S>& v, const vector<T, S>& w) { return v.data._min(w.data); }
    template<typename T, int S> vector<T, S> max(const vector<T, S>& v, T x) { return v.data._max(x); }
    template<typename T, int S> vector<T, S> max(const vector<T, S>& v, const vector<T, S>& w) { return v.data._max(w.data); }

    template<typename T, int S> vector<T, S> clamp(const vector<T, S>& v, T a, T b) { return v.data._clamp(a, b); }
    template<typename T, int S> vector<T, S> clamp(const vector<T, S>& v, const vector<T, S>& a, const vector<T, S>& b) { return v.data._clamp(a.data, b.data); }
    template<typename T, int S> vector<T, S> mix(const vector<T, S>& v, const vector<T, S>& w, T a) { return v.data._mix(w.data, a); }
    template<typename T, int S> vector<T, S> mix(const vector<T, S>& v, const vector<T, S>& w, const vector<T, S>& a) { return v.data._mix(w.data, a); }
    template<typename T, int S> vector<T, S> step(const vector<T, S>& v, T a) { return v.data._step(a); }
    template<typename T, int S> vector<T, S> step(const vector<T, S>& v, const vector<T, S>& a) { return v.data._step(a.data); }
    template<typename T, int S> vector<T, S> smoothstep(const vector<T, S>& v, T e0, T e1) { return v.data._smoothstep(e0, e1); }
    template<typename T, int S> vector<T, S> smoothstep(const vector<T, S>& v, const vector<T, S>& e0, const vector<T, S>& e1) { return v.data._smoothstep(e0.data, e1.data); }
    template<typename T, int S> vector<T, S> mod(const vector<T, S>& v, T x) { return v.data._mod(x); }
    template<typename T, int S> vector<T, S> mod(const vector<T, S>& v, const vector<T, S>& w) { return v.data._mod(w.data); }

    template<typename T, int S> vector<bool, S> greaterThan(const vector<T, S>& v, const vector<T, S>& w) { return v.data._greaterThan(w.data); }
    template<typename T, int S> vector<bool, S> greaterThanEqual(const vector<T, S>& v, const vector<T, S>& w) { return v.data._greaterThanEqual(w.data); }
    template<typename T, int S> vector<bool, S> lessThan(const vector<T, S>& v, const vector<T, S>& w) { return v.data._lessThan(w.data); }
    template<typename T, int S> vector<bool, S> lessThanEqual(const vector<T, S>& v, const vector<T, S>& w) { return v.data._lessThanEqual(w.data); }
    template<typename T, int S> vector<bool, S> equal(const vector<T, S>& v, const vector<T, S>& w) { return v.data._equal(w.data); }
    template<typename T, int S> vector<bool, S> notEqual(const vector<T, S>& v, const vector<T, S>& w) { return v.data._notEqual(w.data); }
    template<int S> bool any(const vector<bool, S>& v) { return v.data._any(); }
    template<int S> bool all(const vector<bool, S>& v) { return v.data._all(); }
    template<int S> bool negate(const vector<bool, S>& v) { return v.data._negate(); }

    template<typename T, int S> T length(const vector<T, S>& v) { return v.data._length(); }
    template<typename T, int S> T distance(const vector<T, S>& v, const vector<T, S>& w) { return v.data._distance(w.data); }
    template<typename T, int S> T dot(const vector<T, S>& v, const vector<T, S>& w) { return v.data._dot(w.data); }
    template<typename T, int S> vector<T, S> normalize(const vector<T, S>& v) { return v.data._normalize(); }

    template<typename T, int S> vector<T, S> faceforward(const vector<T, S>& v, const vector<T, S>& I, const vector<T, S>& Nref)
    {
        return Nref.data._dot(I.data) < static_cast<T>(0) ? v : - v;
    }

    template<typename T, int S> vector<T, S> reflect(const vector<T, S>& I, const vector<T, S>& N)
    {
        return I - static_cast<T>(2) * N.data._dot(I.data) * N;
    }

    template<typename T, int S> vector<T, S> refract(const vector<T, S>& I, const vector<T, S>& N, T eta)
    {
        const T d = N.data._dot(I.data);
        const T k = static_cast<T>(1) - eta * eta * (static_cast<T>(1) - d * d);
        return k < static_cast<T>(0) ? vector<T, S>(static_cast<T>(0)) : I * eta - N * (eta * d + sqrt(k));
    }

    template<typename T> vector<T, 3> cross(const vector<T, 3>& v, const vector<T, 3>& w)
    {
        return vector<T, 3>(
                v.y * w.z - v.z * w.y,
                v.z * w.x - v.x * w.z,
                v.x * w.y - v.y * w.x);
    }

    template<typename T, int S> vector<bool, S> is_pow2(const vector<T, S>& v) { return v.data._is_pow2(); }
    template<typename T, int S> vector<T, S> next_pow2(const vector<T, S>& v) { return v.data._next_pow2(); }
    template<typename T, int S> vector<T, S> next_multiple(const vector<T, S>& v, const vector<T, S>& w) { return v.data._next_multiple(w.data); }
    template<typename T, int S> vector<T, S> next_multiple(const vector<T, S>& v, T x) { return v.data._next_multiple(x); }

    typedef vector<bool, 2> bvec2;
    typedef vector<bool, 3> bvec3;
    typedef vector<bool, 4> bvec4;
    typedef vector<int8_t, 2> ibvec2;
    typedef vector<int8_t, 3> ibvec3;
    typedef vector<int8_t, 4> ibvec4;
    typedef vector<uint8_t, 2> ubvec2;
    typedef vector<uint8_t, 3> ubvec3;
    typedef vector<uint8_t, 4> ubvec4;
    typedef vector<int16_t, 2> svec2;
    typedef vector<int16_t, 3> svec3;
    typedef vector<int16_t, 4> svec4;
    typedef vector<uint16_t, 2> usvec2;
    typedef vector<uint16_t, 3> usvec3;
    typedef vector<uint16_t, 4> usvec4;
    typedef vector<int32_t, 2> ivec2;
    typedef vector<int32_t, 3> ivec3;
    typedef vector<int32_t, 4> ivec4;
    typedef vector<uint32_t, 2> uvec2;
    typedef vector<uint32_t, 3> uvec3;
    typedef vector<uint32_t, 4> uvec4;
    typedef vector<int64_t, 2> i64vec2;
    typedef vector<int64_t, 3> i64vec3;
    typedef vector<int64_t, 4> i64vec4;
    typedef vector<uint64_t, 2> u64vec2;
    typedef vector<uint64_t, 3> u64vec3;
    typedef vector<uint64_t, 4> u64vec4;
    typedef vector<float, 2> vec2;
    typedef vector<float, 3> vec3;
    typedef vector<float, 4> vec4;
    typedef vector<double, 2> dvec2;
    typedef vector<double, 3> dvec3;
    typedef vector<double, 4> dvec4;


    /* Matrices */

    template<typename T, int C, int R> class matrix
    {
    public:
        union {
            Data<T, C * R> data;
            vector<T, R> columns[C];
        };

        /* Constructors, Destructor */

        matrix() {}

        matrix(const matrix<T, C, R>& a)
        {
            for (int i = 0; i < C * R; i++)
                data[i] = a.data[i];
        }

        matrix(const Data<T, C * R>& a)
        {
            for (int i = 0; i < C * R; i++)
                data[i] = a.data[i];
        }

        matrix(const T* a)
        {
            for (int i = 0; i < C * R; i++)
                data[i] = a[i];
        }

        matrix(T x)
        {
            for (int i = 0; i < C; i++)
                for (int j = 0; j < R; j++)
                    data[i * R + j] = (i == j ? x : static_cast<T>(0));
        }

        matrix(T v0, T v1, T v2, T v3)
        {
            static_assert(C * R == 4, "only a matrix with 4 elements can be initialized this way");
            data[0] = v0; data[1] = v1; data[2] = v2; data[3] = v3;
        }

        matrix(T v0, T v1, T v2, T v3, T v4, T v5)
        {
            static_assert(C * R == 6, "only a matrix with 6 elements can be initialized this way");
            data[0] = v0; data[1] = v1; data[2] = v2; data[3] = v3;
            data[4] = v4; data[5] = v5;
        }

        matrix(T v0, T v1, T v2, T v3, T v4, T v5, T v6, T v7)
        {
            static_assert(C * R == 6, "only a matrix with 8 elements can be initialized this way");
            data[0] = v0; data[1] = v1; data[2] = v2; data[3] = v3;
            data[4] = v4; data[5] = v5; data[6] = v6; data[7] = v7;
        }

        matrix(T v0, T v1, T v2, T v3, T v4, T v5, T v6, T v7, T v8)
        {
            static_assert(C * R == 9, "only a matrix with 9 elements can be initialized this way");
            data[0] = v0; data[1] = v1; data[2] = v2; data[3] = v3;
            data[4] = v4; data[5] = v5; data[6] = v6; data[7] = v7;
            data[8] = v8;
        }

        matrix(T v0, T v1, T v2, T v3, T v4, T v5, T v6, T v7, T v8, T v9, T v10, T v11)
        {
            static_assert(C * R == 12, "only a matrix with 12 elements can be initialized this way");
            data[0] = v0; data[1] = v1; data[2] = v2; data[3] = v3;
            data[4] = v4; data[5] = v5; data[6] = v6; data[7] = v7;
            data[8] = v8; data[9] = v9; data[10] = v10; data[11] = v11;
        }

        matrix(T v0, T v1, T v2, T v3, T v4, T v5, T v6, T v7, T v8, T v9, T v10, T v11, T v12, T v13, T v14, T v15)
        {
            static_assert(C * R == 16, "only a matrix with 16 elements can be initialized this way");
            data[0] = v0; data[1] = v1; data[2] = v2; data[3] = v3;
            data[4] = v4; data[5] = v5; data[6] = v6; data[7] = v7;
            data[8] = v8; data[9] = v9; data[10] = v10; data[11] = v11;
            data[12] = v12; data[13] = v13; data[14] = v14; data[15] = v15;
        }

        matrix(const vector<T, R>& col0, const vector<T, R>& col1)
        {
            static_assert(C == 2, "only a matrix with 2 columns can be initialized this way");
            columns[0] = col0;
            columns[1] = col1;
        }
        matrix(const vector<T, R>& col0, const vector<T, R>& col1, const vector<T, R>& col2)
        {
            static_assert(C == 3, "only a matrix with 3 columns can be initialized this way");
            columns[0] = col0;
            columns[1] = col1;
            columns[2] = col2;
        }
        matrix(const vector<T, R>& col0, const vector<T, R>& col1, const vector<T, R>& col2, const vector<T, R>& col3)
        {
            static_assert(C == 4, "only a matrix with 4 columns can be initialized this way");
            columns[0] = col0;
            columns[1] = col1;
            columns[2] = col2;
            columns[3] = col3;
        }

        /* Operators */

        operator const T*() const { return data.data; }

        vector<T, R>& operator[](unsigned int i)
        {
            return columns[i];
        }

        vector<T, R> operator[](unsigned int i) const
        {
            return columns[i];
        }

        const matrix& operator=(const matrix& a)      { this->data._op_assign(a.data); return *this; }
        matrix operator+(const matrix& a) const       { return this->data._op_plus(a.data); }
        const matrix& operator+=(const matrix& a)     { this->data._op_plus_assign(a.data); return *this; }
        matrix operator+() const                      { return this->data._op_unary_plus(); }
        matrix operator-(const matrix& a) const       { return this->data._op_minus(a.data); }
        const matrix& operator-=(const matrix& a)     { this->data._op_minus_assign(a.data); return *this; }
        matrix operator-() const                      { return this->data._op_unary_minus(); }
        matrix operator*(T s) const                   { return this->data._op_scal_mult(s); }
        friend matrix operator*(T s, const matrix& a) { return a.data._op_scal_mult(s); }
        const matrix& operator*=(T s)                 { this->data._op_scal_mult_assign(s); return *this; }
        matrix operator/(T s) const                   { return this->data._op_scal_div(s); }
        const matrix& operator/=(T s)                 { this->data._op_scal_div_assign(s); return *this; }
        matrix operator%(T s) const                   { return this->data._op_scal_mod(s); }
        const matrix& operator%=(T s)                 { this->data._op_scal_mod_assign(s); return *this; }
        bool operator==(const matrix& a) const        { return this->data._op_equal(a.data); }
        bool operator!=(const matrix& a) const        { return this->data._op_notequal(a.data); }

        vector<T, R> operator*(const vector<T, C>& w) const
        {
            vector<T, R> r;
            for (int i = 0; i < R; i++) {
                r.data[i] = static_cast<T>(0);
                for (int j = 0; j < C; j++) {
                    r.data[i] += columns[j][i] * w.data[j];
                }
            }
            return r;
        }

        friend vector<T, C> operator*(const vector<T, R>& w, const matrix& m)
        {
            vector<T, C> r;
            for (int i = 0; i < C; i++) {
                r.data[i] = static_cast<T>(0);
                for (int j = 0; j < R; j++) {
                    r.data[i] += m.columns[i][j] * w.data[j];
                }
            }
            return r;
        }

        matrix<T, R, R> operator*(const matrix<T, R, C>& n) const
        {
            matrix<T, R, R> r;
            for (int i = 0; i < R; i++) {
                for (int j = 0; j < R; j++) {
                    r.columns[i][j] = static_cast<T>(0);
                    for (int k = 0; k < C; k++) {
                        r.columns[i][j] += columns[k][j] * n.columns[i][k];
                    }
                }
            }
            return r;
        }

        const matrix<T, C, R>& operator*=(const matrix<T, C, R>& n)
        {
            matrix<T, C, R> r = *this * n;
            *this = r;
            return *this;
        }
    };

    template<typename T, int r> vector<T, 2> row(const matrix<T, 2, r>& m, int row) { return vector<T, 2>(m[0][row], m[1][row]); }
    template<typename T, int r> vector<T, 3> row(const matrix<T, 3, r>& m, int row) { return vector<T, 3>(m[0][row], m[1][row], m[2][row]); }
    template<typename T, int r> vector<T, 4> row(const matrix<T, 4, r>& m, int row) { return vector<T, 4>(m[0][row], m[1][row], m[2][row], m[3][row]); }
    template<typename T, int c> vector<T, 2> col(const matrix<T, c, 2>& m, int col) { return vector<T, 2>(m[col][0], m[col][1]); }
    template<typename T, int c> vector<T, 3> col(const matrix<T, c, 3>& m, int col) { return vector<T, 3>(m[col][0], m[col][1], m[col][2]); }
    template<typename T, int c> vector<T, 4> col(const matrix<T, c, 4>& m, int col) { return vector<T, 4>(m[col][0], m[col][1], m[col][2], m[col][3]); }

    template<typename T, int c, int r> matrix<T, c - 1, r - 1> strike(const matrix<T, c, r>& m, int col, int row) { return m.data._strike(col, row); }

    template<typename T, int c, int r, int rs, int cs> matrix<T, c, r> set(
            const matrix<T, c, r>& m, const matrix<T, cs, rs>& s, int col = 0, int row = 0)
    {
        matrix<T, c, r> result(m);
        for (int i = 0; i < c; i++) {
            for (int j = 0; j < r; j++) {
                result[i][j] = (i >= col && i <= col + cs && j >= row && j <= row + rs)
                    ? s[i - col][j - row] : m[i][j];
            }
        }
        return result;
    }

    template<typename T, int c, int r> matrix<T, c, r> sin(const matrix<T, c, r>& v) { return v.data._sin(); }
    template<typename T, int c, int r> matrix<T, c, r> cos(const matrix<T, c, r>& v) { return v.data._cos(); }
    template<typename T, int c, int r> matrix<T, c, r> tan(const matrix<T, c, r>& v) { return v.data._tan(); }
    template<typename T, int c, int r> matrix<T, c, r> asin(const matrix<T, c, r>& v) { return v.data._asin(); }
    template<typename T, int c, int r> matrix<T, c, r> acos(const matrix<T, c, r>& v) { return v.data._acos(); }
    template<typename T, int c, int r> matrix<T, c, r> atan(const matrix<T, c, r>& v) { return v.data._atan(); }
    template<typename T, int c, int r> matrix<T, c, r> atan(const matrix<T, c, r>& v, const matrix<T, c, r>& w) { return v.data._atan(w.data); }
    template<typename T, int c, int r> matrix<T, c, r> radians(const matrix<T, c, r>& v) { return v.data._radians(); }
    template<typename T, int c, int r> matrix<T, c, r> degrees(const matrix<T, c, r>& v) { return v.data._degrees(); }

    template<typename T, int c, int r> matrix<T, c, r> pow(const matrix<T, c, r>& v, T p) { return v.data._pow(p); }
    template<typename T, int c, int r> matrix<T, c, r> exp(const matrix<T, c, r>& v) { return v.data._exp(); }
    template<typename T, int c, int r> matrix<T, c, r> exp2(const matrix<T, c, r>& v) { return v.data._exp2(); }
    template<typename T, int c, int r> matrix<T, c, r> log(const matrix<T, c, r>& v) { return v.data._log(); }
    template<typename T, int c, int r> matrix<T, c, r> log2(const matrix<T, c, r>& v) { return v.data._log2(); }
    template<typename T, int c, int r> matrix<T, c, r> log10(const matrix<T, c, r>& v) { return v.data._log10(); }
    template<typename T, int c, int r> matrix<T, c, r> sqrt(const matrix<T, c, r>& v) { return v.data._sqrt(); }
    template<typename T, int c, int r> matrix<T, c, r> inversesqrt(const matrix<T, c, r>& v) { return v.data._inversesqrt(); }
    template<typename T, int c, int r> matrix<T, c, r> cbrt(const matrix<T, c, r>& v) { return v.data._cbrt(); }

    template<typename T, int c, int r> matrix<bool, c, r> isfinite(const matrix<T, c, r>& v) { return v.data._isfinite(); }
    template<typename T, int c, int r> matrix<bool, c, r> isinf(const matrix<T, c, r>& v) { return v.data._isinf(); }
    template<typename T, int c, int r> matrix<bool, c, r> isnan(const matrix<T, c, r>& v) { return v.data._isnan(); }
    template<typename T, int c, int r> matrix<bool, c, r> isnormal(const matrix<T, c, r>& v) { return v.data._isnormal(); }

    template<typename T, int c, int r> matrix<T, c, r> abs(const matrix<T, c, r>& v) { return v.data._abs(); }
    template<typename T, int c, int r> matrix<T, c, r> sign(const matrix<T, c, r>& v) { return v.data._sign(); }
    template<typename T, int c, int r> matrix<T, c, r> floor(const matrix<T, c, r>& v) { return v.data._floor(); }
    template<typename T, int c, int r> matrix<T, c, r> ceil(const matrix<T, c, r>& v) { return v.data._ceil(); }
    template<typename T, int c, int r> matrix<T, c, r> round(const matrix<T, c, r>& v) { return v.data._round(); }
    template<typename T, int c, int r> matrix<T, c, r> fract(const matrix<T, c, r>& v) { return v.data._fract(); }

    template<typename T, int c, int r> matrix<T, c, r> min(const matrix<T, c, r>& v, T x) { return v.data._min(x); }
    template<typename T, int c, int r> matrix<T, c, r> min(const matrix<T, c, r>& v, const matrix<T, c, r>& w) { return v.data._min(w.data); }
    template<typename T, int c, int r> matrix<T, c, r> max(const matrix<T, c, r>& v, T x) { return v.data._max(x); }
    template<typename T, int c, int r> matrix<T, c, r> max(const matrix<T, c, r>& v, const matrix<T, c, r>& w) { return v.data._max(w.data); }

    template<typename T, int c, int r> matrix<T, c, r> clamp(const matrix<T, c, r>& v, T a, T b) { return v.data._clamp(a, b); }
    template<typename T, int c, int r> matrix<T, c, r> clamp(const matrix<T, c, r>& v, const matrix<T, c, r>& a, const matrix<T, c, r>& b) { return v.data._clamp(a.data, b.data); }
    template<typename T, int c, int r> matrix<T, c, r> mix(const matrix<T, c, r>& v, const matrix<T, c, r>& w, T a) { return v.data._mix(w.data, a); }
    template<typename T, int c, int r> matrix<T, c, r> mix(const matrix<T, c, r>& v, const matrix<T, c, r>& w, const matrix<T, c, r>& a) { return v.data._mix(w.data, a); }
    template<typename T, int c, int r> matrix<T, c, r> step(const matrix<T, c, r>& v, T a) { return v.data._step(a); }
    template<typename T, int c, int r> matrix<T, c, r> step(const matrix<T, c, r>& v, const matrix<T, c, r>& a) { return v.data._step(a.data); }
    template<typename T, int c, int r> matrix<T, c, r> smoothstep(const matrix<T, c, r>& v, T e0, T e1) { return v.data._smoothstep(e0, e1); }
    template<typename T, int c, int r> matrix<T, c, r> smoothstep(const matrix<T, c, r>& v, const vector<T, 1>& e0, const vector<T, 1>& e1) { return v.data._smoothstep(e0.data, e1.data); }
    template<typename T, int c, int r> matrix<T, c, r> mod(const matrix<T, c, r>& v, T x) { return v.data._mod(x); }
    template<typename T, int c, int r> matrix<T, c, r> mod(const matrix<T, c, r>& v, const matrix<T, c, r>& w) { return v.data._mod(w.data); }

    template<typename T, int c, int r> matrix<bool, c, r> greaterThan(const matrix<T, c, r>& v, const matrix<T, c, r>& w) { return v.data._greaterThan(w.data); }
    template<typename T, int c, int r> matrix<bool, c, r> greaterThanEqual(const matrix<T, c, r>& v, const matrix<T, c, r>& w) { return v.data._greaterThanEqual(w.data); }
    template<typename T, int c, int r> matrix<bool, c, r> lessThan(const matrix<T, c, r>& v, const matrix<T, c, r>& w) { return v.data._lessThan(w.data); }
    template<typename T, int c, int r> matrix<bool, c, r> lessThanEqual(const matrix<T, c, r>& v, const matrix<T, c, r>& w) { return v.data._lessThanEqual(w.data); }
    template<typename T, int c, int r> matrix<bool, c, r> equal(const matrix<T, c, r>& v, const matrix<T, c, r>& w) { return v.data._equal(w.data); }
    template<typename T, int c, int r> matrix<bool, c, r> notEqual(const matrix<T, c, r>& v, const matrix<T, c, r>& w) { return v.data._notEqual(w.data); }
    template<int c, int r> bool any(const matrix<bool, c, r>& v) { return v.data._any(); }
    template<int c, int r> bool all(const matrix<bool, c, r>& v) { return v.data._all(); }
    template<int c, int r> bool negate(const matrix<bool, c, r>& v) { return v.data._negate(); }

    template<typename T, int c, int r> matrix<T, r, c> transpose(const matrix<T, c, r>& m)
    {
        matrix<T, r, c> t;
        for (int i = 0; i < r; i++) {
            for (int j = 0; j < c; j++) {
                t[i][j] = m[j][i];
            }
        }
        return t;
    }

    template<typename T, int c, int r> matrix<T, c, r> matrixCompMult(const matrix<T, c, r>& m, const matrix<T, c, r>& n) { return m.data._op_comp_mult(n.data); }

    template<typename T, int c, int r> matrix<T, c, r> outerProduct(const vector<T, r>& v, const vector<T, c>& w)
    {
        matrix<T, c, r> m;
        for (int i = 0; i < c; i++)
            for (int j = 0; j < r; j++)
                m[i][j] = v[j] * w[i];
        return m;
    }

    template<typename T> T det(const matrix<T, 2, 2>& m)
    {
        return m[0][0] * m[1][1] - m[1][0] * m[0][1];
    }

    template<typename T> bool invertible(const matrix<T, 2, 2>& m, T epsilon = std::numeric_limits<T>::epsilon())
    {
        T d = det(m);
        return (d > epsilon || d < -epsilon);
    }

    template<typename T> matrix<T, 2, 2> inverse(const matrix<T, 2, 2>& m)
    {
        return matrix<T, 2, 2>(m[1][1], -m[1][0], -m[0][1], m[0][0]) / det(m);
    }

    template<typename T> T det(const matrix<T, 3, 3>& m)
    {
        return m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1])
            + m[0][1] * (m[1][2] * m[2][0] - m[1][0] * m[2][2])
            + m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
    }

    template<typename T> bool invertible(const matrix<T, 3, 3>& m, T epsilon = std::numeric_limits<T>::epsilon())
    {
        T d = det(m);
        return (d > epsilon || d < -epsilon);
    }

    template<typename T> matrix<T, 3, 3> inverse(const matrix<T, 3, 3>& m)
    {
        // Using cofactors; see
        // http://en.wikipedia.org/wiki/Invertible_matrix
        // http://en.wikipedia.org/wiki/Minor_(linear_algebra)#Cofactors
        matrix<T, 3, 3> I;
        I[0][0] = m[1][1] * m[2][2] - m[2][1] * m[1][2];
        I[0][1] = m[2][1] * m[0][2] - m[0][1] * m[2][2];
        I[0][2] = m[0][1] * m[1][2] - m[1][1] * m[0][2];
        I[1][0] = m[2][0] * m[1][2] - m[1][0] * m[2][2];
        I[1][1] = m[0][0] * m[2][2] - m[2][0] * m[0][2];
        I[1][2] = m[1][0] * m[0][2] - m[0][0] * m[1][2];
        I[2][0] = m[1][0] * m[2][1] - m[2][0] * m[1][1];
        I[2][1] = m[2][0] * m[0][1] - m[0][0] * m[2][1];
        I[2][2] = m[0][0] * m[1][1] - m[1][0] * m[0][1];
        T det = m[0][0] * I[0][0] + m[1][0] * I[0][1] + m[2][0] * I[0][2];
        return I / det;
    }

    template<typename T> T det(const matrix<T, 4, 4>& m)
    {
        T d0 = m[1][1] * (m[2][2] * m[3][3] - m[3][2] * m[2][3])
            + m[2][1] * (m[3][2] * m[1][3] - m[1][2] * m[3][3])
            + m[3][1] * (m[1][2] * m[2][3] - m[2][2] * m[1][3]);
        T d1 = m[0][1] * (m[2][2] * m[3][3] - m[3][2] * m[2][3])
            + m[2][1] * (m[3][2] * m[0][3] - m[0][2] * m[3][3])
            + m[3][1] * (m[0][2] * m[2][3] - m[2][2] * m[0][3]);
        T d2 = m[0][1] * (m[1][2] * m[3][3] - m[3][2] * m[1][3])
            + m[1][1] * (m[3][2] * m[0][3] - m[0][2] * m[3][3])
            + m[3][1] * (m[0][2] * m[1][3] - m[1][2] * m[0][3]);
        T d3 = m[0][1] * (m[1][2] * m[2][3] - m[2][2] * m[1][3])
            + m[1][1] * (m[2][2] * m[0][3] - m[0][2] * m[2][3])
            + m[2][1] * (m[0][2] * m[1][3] - m[1][2] * m[0][3]);
        return m[0][0] * d0 - m[1][0] * d1 + m[2][0] * d2 - m[3][0] * d3;
    }

    template<typename T> bool invertible(const matrix<T, 4, 4>& m, T epsilon = std::numeric_limits<T>::epsilon())
    {
        T d = det(m);
        return (d > epsilon || d < -epsilon);
    }

    template<typename T> matrix<T, 4, 4> inverse(const matrix<T, 4, 4>& m)
    {
        // Using cofactors; see
        // http://en.wikipedia.org/wiki/Invertible_matrix
        // http://en.wikipedia.org/wiki/Minor_(linear_algebra)#Cofactors

        /* This code was adapted from Equalizer-0.5.0,
         * src/externals/vmmlib/matrix4.h:
         *
         * VMMLib - vector & matrix Math Lib
         *
         * @author Jonas Boesch
         * @author Stefan Eilemann
         * @author Renato Pajarola
         * @author David H. Eberly ( Wild Magic )
         * @author Andrew Willmott ( VL )
         *
         * @license revised BSD license, check LICENSE
         *
         * parts of the source code of VMMLib were inspired by David Eberly's
         * Wild Magic and Andrew Willmott's VL.
         *
         * tuned version from Claude Knaus
         */

        matrix<T, 4, 4> result;

        /* first set of 2x2 determinants: 12 multiplications, 6 additions */
        T t1[6] =
        {
            m[2][0] * m[3][1] - m[2][1] * m[3][0],
            m[2][0] * m[3][2] - m[2][2] * m[3][0],
            m[2][0] * m[3][3] - m[2][3] * m[3][0],
            m[2][1] * m[3][2] - m[2][2] * m[3][1],
            m[2][1] * m[3][3] - m[2][3] * m[3][1],
            m[2][2] * m[3][3] - m[2][3] * m[3][2]
        };

        /* first half of co_matrix: 24 multiplications, 16 additions */
        result[0][0] = m[1][1] * t1[5] - m[1][2] * t1[4] + m[1][3] * t1[3];
        result[1][0] = m[1][2] * t1[2] - m[1][3] * t1[1] - m[1][0] * t1[5];
        result[2][0] = m[1][3] * t1[0] - m[1][1] * t1[2] + m[1][0] * t1[4];
        result[3][0] = m[1][1] * t1[1] - m[1][0] * t1[3] - m[1][2] * t1[0];
        result[0][1] = m[0][2] * t1[4] - m[0][1] * t1[5] - m[0][3] * t1[3];
        result[1][1] = m[0][0] * t1[5] - m[0][2] * t1[2] + m[0][3] * t1[1];
        result[2][1] = m[0][1] * t1[2] - m[0][3] * t1[0] - m[0][0] * t1[4];
        result[3][1] = m[0][0] * t1[3] - m[0][1] * t1[1] + m[0][2] * t1[0];

        /* second set of 2x2 determinants: 12 multiplications, 6 additions */
        T t2[6] =
        {
            m[0][0] * m[1][1] - m[0][1] * m[1][0],
            m[0][0] * m[1][2] - m[0][2] * m[1][0],
            m[0][0] * m[1][3] - m[0][3] * m[1][0],
            m[0][1] * m[1][2] - m[0][2] * m[1][1],
            m[0][1] * m[1][3] - m[0][3] * m[1][1],
            m[0][2] * m[1][3] - m[0][3] * m[1][2]
        };

        /* second half of co_matrix: 24 multiplications, 16 additions */
        result[0][2] = m[3][1] * t2[5] - m[3][2] * t2[4] + m[3][3] * t2[3];
        result[1][2] = m[3][2] * t2[2] - m[3][3] * t2[1] - m[3][0] * t2[5];
        result[2][2] = m[3][3] * t2[0] - m[3][1] * t2[2] + m[3][0] * t2[4];
        result[3][2] = m[3][1] * t2[1] - m[3][0] * t2[3] - m[3][2] * t2[0];
        result[0][3] = m[2][2] * t2[4] - m[2][1] * t2[5] - m[2][3] * t2[3];
        result[1][3] = m[2][0] * t2[5] - m[2][2] * t2[2] + m[2][3] * t2[1];
        result[2][3] = m[2][1] * t2[2] - m[2][3] * t2[0] - m[2][0] * t2[4];
        result[3][3] = m[2][0] * t2[3] - m[2][1] * t2[1] + m[2][2] * t2[0];

        /* determinant: 4 multiplications, 3 additions */
        T determinant =
            m[0][0] * result[0][0] + m[0][1] * result[1][0] +
            m[0][2] * result[2][0] + m[0][3] * result[3][0];

        /* division: 16 multiplications, 1 division */
        return result / determinant;
    }

    template<typename T> vector<T, 3> translation(const matrix<T, 4, 4>& m)
    {
        return vector<T, 3>(m[3][0], m[3][1], m[3][2]);
    }

    template<typename T> matrix<T, 4, 4> translate(const matrix<T, 4, 4>& m, const vector<T, 3>& v)
    {
        vector<T, 4> t(v, static_cast<T>(1));
        matrix<T, 4, 4> r;
        r[0][0] = m[0][0];
        r[0][1] = m[0][1];
        r[0][2] = m[0][2];
        r[0][3] = m[0][3];
        r[1][0] = m[1][0];
        r[1][1] = m[1][1];
        r[1][2] = m[1][2];
        r[1][3] = m[1][3];
        r[2][0] = m[2][0];
        r[2][1] = m[2][1];
        r[2][2] = m[2][2];
        r[2][3] = m[2][3];
        r[3][0] = dot(row(m, 0), t);
        r[3][1] = dot(row(m, 1), t);
        r[3][2] = dot(row(m, 2), t);
        r[3][3] = dot(row(m, 3), t);
        return r;
    }

    template<typename T> matrix<T, 4, 4> scale(const matrix<T, 4, 4>& m, const vector<T, 3>& v)
    {
        matrix<T, 4, 4> r;
        r[0] = m[0] * v.x;
        r[1] = m[1] * v.y;
        r[2] = m[2] * v.z;
        r[3] = m[3];
        return r;
    }

    template<typename T> matrix<T, 4, 4> rotate(const matrix<T, 4, 4>& m, T angle, const vector<T, 3>& axis)
    {
        return m * toMat4(angle, axis);
    }

    template<typename T, int c, int r> matrix<bool, c, r> is_pow2(const matrix<T, c, r>& v) { return v.data._ispow2(); }
    template<typename T, int c, int r> matrix<T, c, r> next_pow2(const matrix<T, c, r>& v) { return v.data._next_pow2(); }
    template<typename T, int c, int r> matrix<T, c, r> next_multiple(const matrix<T, c, r>& v, const matrix<T, c, r>& w) { return v.data._next_multiple(w); }
    template<typename T, int c, int r> matrix<T, c, r> next_multiple(const matrix<T, c, r>& v, T x) { return v.data._next_multiple(x); }

    typedef matrix<float, 2, 2> mat2;
    typedef matrix<double, 2, 2> dmat2;
    typedef matrix<float, 3, 3> mat3;
    typedef matrix<double, 3, 3> dmat3;
    typedef matrix<float, 4, 4> mat4;
    typedef matrix<double, 4, 4> dmat4;
    typedef matrix<float, 2, 3> mat2x3;
    typedef matrix<double, 2, 3> dmat2x3;
    typedef matrix<float, 3, 2> mat3x2;
    typedef matrix<double, 3, 2> dmat3x2;
    typedef matrix<float, 2, 4> mat2x4;
    typedef matrix<double, 2, 4> dmat2x4;
    typedef matrix<float, 4, 2> mat4x2;
    typedef matrix<double, 4, 2> dmat4x2;
    typedef matrix<float, 3, 4> mat3x4;
    typedef matrix<double, 4, 4> dmat3x4;
    typedef matrix<float, 4, 3> mat4x3;
    typedef matrix<double, 4, 3> dmat4x3;

    template<typename T>
    class quaternion
    {
    public:
        union {
            Data<T, 4> data;
            struct {
                T x, y, z, w;
            };
        };

        /* Constructors, Destructor */

        quaternion() { x = 0; y = 0; z = 0; w = 1; }
        quaternion(T x, T y, T z, T w) { this->x = x; this->y = y; this->z = z; this->w = w; }

        /* Get / set components */

        vector<T, 3> axis() const
        {
            T cos_a = w;
            T sin_a = glvm::sqrt(static_cast<T>(1) - cos_a * cos_a);
            if (glvm::abs(sin_a) < static_cast<T>(0.0005))
            {
                sin_a = static_cast<T>(1);
            }
            return vector<T, 3>(x / sin_a, y / sin_a, z / sin_a);
        }

        T angle() const
        {
            return glvm::acos(w) * static_cast<T>(2);
        }

        quaternion operator+() const
        {
            return *this;
        }

        quaternion operator-() const
        {
            return conjugate(*this);
        }

        quaternion operator*(const quaternion& q) const
        {
            quaternion p;
            p.x = w * q.x + x * q.w + y * q.z - z * q.y;
            p.y = w * q.y + y * q.w + z * q.x - x * q.z;
            p.z = w * q.z + z * q.w + x * q.y - y * q.x;
            p.w = w * q.w - x * q.x - y * q.y - z * q.z;
            return p;
        }

        const quaternion& operator*=(const quaternion& q)
        {
            *this = *this * q;
            return *this;
        }

        vector<T, 3> operator*(const vector<T, 3>& v) const
        {
            // return toMat3(*this) * v;
            quaternion<T> t = *this * quaternion<T>(v.x, v.y, v.z, static_cast<T>(0)) * conjugate(*this);
            return vector<T, 3>(t.x, t.y, t.z);
        }

        vector<T, 4> operator*(const vector<T, 4>& v) const
        {
            //return toMat4(*this) * v;
            quaternion<T> t = *this * quaternion<T>(v.x, v.y, v.z, static_cast<T>(0)) * conjugate(*this);
            return vector<T, 4>(t.x, t.y, t.z, t.w);
        }

        operator const T*() const { return &x; }
    };

    template<typename T> T magnitude(const quaternion<T>& q) { return q.data._length(); }
    template<typename T> quaternion<T> conjugate(const quaternion<T>& q) { return quaternion<T>(-q.x, -q.y, -q.z, q.w); }
    template<typename T> quaternion<T> inverse(const quaternion<T>& q) { return quaternion<T>(conjugate(q).data._op_scal_div(magnitude(q))); }
    template<typename T> quaternion<T> normalize(const quaternion<T>& q) { return quaternion<T>(q.data._op_scal_div(magnitude(q))); }

    typedef quaternion<float> quat;
    typedef quaternion<double> dquat;


    template<typename T>
    class frustum
    {
    public:
        union {
            Data<T, 6> data;
            struct {
                T l, r, b, t, n, f;
            };
        };

        /* Constructors, Destructor */

        frustum() {}

        frustum(T l, T r, T b, T t, T n, T f)
        {
            this->l = l;
            this->r = r;
            this->b = b;
            this->t = t;
            this->n = n;
            this->f = f;
        }

        /* Frustum operations */

        void adjust_near(T new_near)
        {
            T q = new_near / n;
            l *= q;
            r *= q;
            b *= q;
            t *= q;
            n = new_near;
        }

        operator const T*() const { return &l; }
    };

    template<typename T> matrix<T, 4, 4> toMat4(const frustum<T>& f)
    {
        matrix<T, 4, 4> m;
        m[0][0] = static_cast<T>(2) * f.n / (f.r - f.l);
        m[0][1] = static_cast<T>(0);
        m[0][2] = static_cast<T>(0);
        m[0][3] = static_cast<T>(0);
        m[1][0] = static_cast<T>(0);
        m[1][1] = static_cast<T>(2) * f.n / (f.t - f.b);
        m[1][2] = static_cast<T>(0);
        m[1][3] = static_cast<T>(0);
        m[2][0] = (f.r + f.l) / (f.r - f.l);
        m[2][1] = (f.t + f.b) / (f.t - f.b);
        m[2][2] = - (f.f + f.n) / (f.f - f.n);
        m[2][3] = static_cast<T>(-1);
        m[3][0] = static_cast<T>(0);
        m[3][1] = static_cast<T>(0);
        m[3][2] = - static_cast<T>(2) * f.f * f.n / (f.f - f.n);
        m[3][3] = static_cast<T>(0);
        return m;
    }

    typedef frustum<float> frust;
    typedef frustum<double> dfrust;


    /* Conversions between different representations of rotations:
     * from angle/axis, oldpoint/newpoint, euler angles, matrix3 to quat
     * from angle/axis, oldpoint/newpoint, euler angles, quat to matrix3
     * from angle/axis, oldpoint/newpoint, euler angles, quat to matrix4
     * from angle/axis, oldpoint/newpoint, matrix3, quat to euler angles
     * TODO: Conversions from euler angles, matrix3, quat to angle/axis.
     */

    template<typename T> quaternion<T> toQuat(T angle, const vector<T, 3>& axis)
    {
        if (all(equal(axis, vector<T, 3>(static_cast<T>(0))))) {
            return quaternion<T>(static_cast<T>(0), static_cast<T>(0), static_cast<T>(0), static_cast<T>(1));
        } else {
            vector<T, 3> naxis = normalize(axis);
            T sin_a = sin(angle / static_cast<T>(2));
            T cos_a = cos(angle / static_cast<T>(2));
            return quaternion<T>(naxis.x * sin_a, naxis.y * sin_a, naxis.z * sin_a, cos_a);
        }
    }

    template<typename T> quaternion<T> toQuat(const vector<T, 3>& oldpoint, const vector<T, 3>& newpoint)
    {
        vector<T, 3> axis = cross(oldpoint, newpoint);
        T angle = acos(dot(oldpoint, newpoint) / sqrt(dot(oldpoint, oldpoint) * dot(newpoint, newpoint)));
        return toQuat(angle, axis);
    }

    template<typename T> quaternion<T> toQuat(const vector<T, 3>& euler_rot)
    {
        T x2 = euler_rot.x / static_cast<T>(2);
        T y2 = euler_rot.y / static_cast<T>(2);
        T z2 = euler_rot.z / static_cast<T>(2);
        T cx2 = cos(x2);
        T sx2 = sin(x2);
        T cy2 = cos(y2);
        T sy2 = sin(y2);
        T cz2 = cos(z2);
        T sz2 = sin(z2);
        quaternion<T> q;
        q.x = sx2 * cy2 * cz2 - cx2 * sy2 * sz2;
        q.y = cx2 * sy2 * cz2 + sx2 * cy2 * sz2;
        q.z = cx2 * cy2 * sz2 - sx2 * sy2 * cz2;
        q.w = cx2 * cy2 * cz2 + sx2 * sy2 * sz2;
        return q;
    }

    template<typename T> quaternion<T> toQuat(const matrix<T, 3, 3>& rot_matrix)
    {
        quaternion<T> q;
        // From "Matrix and Quaternion FAQ", Q55
        T t = static_cast<T>(1) + rot_matrix[0][0] + rot_matrix[1][1] + rot_matrix[2][2];
        if (t > static_cast<T>(1e-8))
        {
            T s = sqrt(t) * static_cast<T>(2);
            q.x = (rot_matrix[1][2] - rot_matrix[2][1]) / s;
            q.y = (rot_matrix[2][0] - rot_matrix[0][2]) / s;
            q.z = (rot_matrix[0][1] - rot_matrix[1][0]) / s;
            q.w = s / static_cast<T>(4);
        }
        else if (rot_matrix[0][0] > rot_matrix[1][1] && rot_matrix[0][0] > rot_matrix[2][2])
        {
            t = static_cast<T>(1) + rot_matrix[0][0] - rot_matrix[1][1] - rot_matrix[2][2];
            T s = sqrt(t) * static_cast<T>(2);
            q.x = s / static_cast<T>(4);
            q.y = (rot_matrix[0][1] + rot_matrix[1][0]) / s;
            q.z = (rot_matrix[2][0] + rot_matrix[0][2]) / s;
            q.w = (rot_matrix[1][2] - rot_matrix[2][1]) / s;
        }
        else if (rot_matrix[1][1] > rot_matrix[2][2])
        {
            t = static_cast<T>(1) + rot_matrix[1][1] - rot_matrix[0][0] - rot_matrix[2][2];
            T s = sqrt(t) * static_cast<T>(2);
            q.x = (rot_matrix[0][1] + rot_matrix[1][0]) / s;
            q.y = s / static_cast<T>(4);
            q.z = (rot_matrix[1][2] + rot_matrix[2][1]) / s;
            q.w = (rot_matrix[2][0] - rot_matrix[0][2]) / s;
        }
        else
        {
            t = static_cast<T>(1) + rot_matrix[2][2] - rot_matrix[0][0] - rot_matrix[1][1];
            T s = sqrt(t) * static_cast<T>(2);
            q.x = (rot_matrix[2][0] + rot_matrix[0][2]) / s;
            q.y = (rot_matrix[1][2] + rot_matrix[2][1]) / s;
            q.z = s / static_cast<T>(4);
            q.w = (rot_matrix[0][1] - rot_matrix[1][0]) / s;
        }
        return q;
    }

    template<typename T> matrix<T, 3, 3> toMat3(T angle, const vector<T, 3>& axis)
    {
        const vector<T, 3> n = normalize(axis);
        T c = glvm::cos(angle);
        T s = glvm::sin(angle);
        T mc = static_cast<T>(1) - c;
        matrix<T, 3, 3> m;
        m[0][0] = n.x * n.x * mc + c;
        m[0][1] = n.y * n.x * mc + n.z * s;
        m[0][2] = n.x * n.z * mc - n.y * s;
        m[1][0] = n.x * n.y * mc - n.z * s;
        m[1][1] = n.y * n.y * mc + c;
        m[1][2] = n.y * n.z * mc + n.x * s;
        m[2][0] = n.x * n.z * mc + n.y * s;
        m[2][1] = n.y * n.z * mc - n.x * s;
        m[2][2] = n.z * n.z * mc + c;
        return m;
    }

    template<typename T> matrix<T, 3, 3> toMat3(const vector<T, 3>& oldpoint, const vector<T, 3>& newpoint)
    {
        vector<T, 3> axis = cross(oldpoint, newpoint);
        T angle = acos(dot(oldpoint, newpoint) / sqrt(dot(oldpoint, oldpoint) * dot(newpoint, newpoint)));
        return toMat3(angle, axis);
    }

    template<typename T> matrix<T, 3, 3> toMat3(const vector<T, 3>& euler_rot)
    {
        return toMat3(toQuat(euler_rot));
    }

    template<typename T> matrix<T, 3, 3> toMat3(const quaternion<T>& q)
    {
        matrix<T, 3, 3> m;
        T xx = q.x * q.x;
        T xy = q.x * q.y;
        T xz = q.x * q.z;
        T xw = q.x * q.w;
        T yy = q.y * q.y;
        T yz = q.y * q.z;
        T yw = q.y * q.w;
        T zz = q.z * q.z;
        T zw = q.z * q.w;
        m[0][0] = static_cast<T>(1) - static_cast<T>(2) * (yy + zz);
        m[0][1] = static_cast<T>(2) * (xy + zw);
        m[0][2] = static_cast<T>(2) * (xz - yw);
        m[1][0] = static_cast<T>(2) * (xy - zw);
        m[1][1] = static_cast<T>(1) - static_cast<T>(2) * (xx + zz);
        m[1][2] = static_cast<T>(2) * (yz + xw);
        m[2][0] = static_cast<T>(2) * (xz + yw);
        m[2][1] = static_cast<T>(2) * (yz - xw);
        m[2][2] = static_cast<T>(1) - static_cast<T>(2) * (xx + yy);
        return m;
    }

    template<typename T> matrix<T, 4, 4> toMat4(T angle, const vector<T, 3>& axis)
    {
        matrix<T, 3, 3> r = toMat3(angle, axis);
        matrix<T, 4, 4> m;
        m[0][0] = r[0][0];
        m[0][1] = r[0][1];
        m[0][2] = r[0][2];
        m[0][3] = static_cast<T>(0);
        m[1][0] = r[1][0];
        m[1][1] = r[1][1];
        m[1][2] = r[1][2];
        m[1][3] = static_cast<T>(0);
        m[2][0] = r[2][0];
        m[2][1] = r[2][1];
        m[2][2] = r[2][2];
        m[2][3] = static_cast<T>(0);
        m[3][0] = static_cast<T>(0);
        m[3][1] = static_cast<T>(0);
        m[3][2] = static_cast<T>(0);
        m[3][3] = static_cast<T>(1);
        return m;
    }

    template<typename T> matrix<T, 4, 4> toMat4(const vector<T, 3>& oldpoint, const vector<T, 3>& newpoint)
    {
        vector<T, 3> axis = cross(oldpoint, newpoint);
        T angle = acos(dot(oldpoint, newpoint) / sqrt(dot(oldpoint, oldpoint) * dot(newpoint, newpoint)));
        return toMat4(angle, axis);
    }

    template<typename T> matrix<T, 4, 4> toMat4(const vector<T, 3>& euler_rot)
    {
        return toMat4(toQuat(euler_rot));
    }

    template<typename T> matrix<T, 4, 4> toMat4(const quaternion<T>& q)
    {
        matrix<T, 3, 3> r = toMat3(q);
        matrix<T, 4, 4> m;
        m[0][0] = r[0][0];
        m[0][1] = r[0][1];
        m[0][2] = r[0][2];
        m[0][3] = static_cast<T>(0);
        m[1][0] = r[1][0];
        m[1][1] = r[1][1];
        m[1][2] = r[1][2];
        m[1][3] = static_cast<T>(0);
        m[2][0] = r[2][0];
        m[2][1] = r[2][1];
        m[2][2] = r[2][2];
        m[2][3] = static_cast<T>(0);
        m[3][0] = static_cast<T>(0);
        m[3][1] = static_cast<T>(0);
        m[3][2] = static_cast<T>(0);
        m[3][3] = static_cast<T>(1);
        return m;
    }

    template<typename T> vector<T, 3> toEuler(T angle, const vector<T, 3>& axis)
    {
        return toEuler(toQuat(angle, axis));
    }

    template<typename T> vector<T, 3> toEuler(const vector<T, 3>& oldpoint, const vector<T, 3>& newpoint)
    {
        return toEuler(toQuat(oldpoint, newpoint));
    }

    template<typename T> matrix<T, 3, 3> toEuler(const matrix<T, 3, 3>& rot_matrix)
    {
        return toEuler(toQuat(rot_matrix));
    }

    template<typename T> vector<T, 3> toEuler(const quaternion<T>& q)
    {
        T singularity_test = q.x * q.y + q.z * q.w;
        vector<T, 3> result;
        if (singularity_test > static_cast<T>(+0.4999))
        {
            // north pole
            result = vector<T, 3>(
                    static_cast<T>(2) * glvm::atan(q.x, q.w),
                    const_pi_2<T>(),
                    static_cast<T>(0));
        }
        else if (singularity_test < static_cast<T>(-0.4999))
        {
            // south pole
            result = vector<T, 3>(
                    -static_cast<T>(2) * glvm::atan(q.x, q.w),
                    -const_pi_2<T>(),
                    static_cast<T>(0));
        }
        else
        {
            result = vector<T, 3>(
                    glvm::atan(static_cast<T>(2) * (q.w * q.x + q.y * q.z),
                        (static_cast<T>(1) - static_cast<T>(2) * (q.x * q.x + q.y * q.y))),
                    glvm::asin(static_cast<T>(2) * (q.w * q.y - q.x * q.z)),
                    glvm::atan(static_cast<T>(2) * (q.w * q.z + q.x * q.y),
                        (static_cast<T>(1) - static_cast<T>(2) * (q.y * q.y + q.z * q.z))));
        }
        return result;
    }

    /* Replacements for some useful old GLU functions */

    // gluPerspective()
    template<typename T> frustum<T> perspective(T fovy, T aspect, T zNear, T zFar)
    {
        T t = tan(fovy / static_cast<T>(2));
        T top = zNear * t;
        T bottom = -top;
        T right = top * aspect;
        T left = -right;
        return frustum<T>(left, right, bottom, top, zNear, zFar);
    }

    // gluLookAt()
    template<typename T> matrix<T, 4, 4> lookAt(const vector<T, 3>& eye, const vector<T, 3>& center, const vector<T, 3>& up)
    {
        vector<T, 3> v = normalize(center - eye);
        vector<T, 3> s = normalize(cross(v, up));
        vector<T, 3> u = cross(s, v);
        return translate(matrix<T, 4, 4>(
                     s.x,  u.x, -v.x, static_cast<T>(0),
                     s.y,  u.y, -v.y, static_cast<T>(0),
                     s.z,  u.z, -v.z, static_cast<T>(0),
                    static_cast<T>(0), static_cast<T>(0), static_cast<T>(0), static_cast<T>(1)),
                -eye);
    }
}

#endif
