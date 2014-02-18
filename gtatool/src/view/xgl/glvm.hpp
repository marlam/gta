/*
 * C++ vector and matrix classes that resemble GLSL style.
 *
 * Copyright (C) 2008, 2009, 2010, 2011, 2012, 2013
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
 * is fully supported, including assignments! Just remember that the swizzlers
 * are member functions (use parentheses).
 * Example: "v4.wz() = v3.rg() + v2;"
 *
 * All data elements are accessible via a two-dimensional array v or a linear
 * array vl. Both v and vl are column-major, like OpenGL.
 * Use transpose() to exchange data with row-major libraries.
 *
 * Everything that is specified by GLSL 1.30 should work, unless it is
 * impossible to implement in C++.
 */


#ifndef GLVM_H
#define GLVM_H

#include <cmath>
#include <climits>
#include <cstdlib>
#if __cplusplus >= 201103L
# include <cstdint>
#else
# include <stdint.h>
#endif

#include <limits>
#include <string>
#include <iostream>
#include <sstream>

/* Values copied from /usr/include/math.h on a recent GNU/Linux system. */
#ifndef M_El
# define M_El           2.7182818284590452353602874713526625L  /* e */
#endif
#ifndef M_LOG2El
# define M_LOG2El       1.4426950408889634073599246810018921L  /* log_2 e */
#endif
#ifndef M_LOG10El
# define M_LOG10El      0.4342944819032518276511289189166051L  /* log_10 e */
#endif
#ifndef M_LN2l
# define M_LN2l         0.6931471805599453094172321214581766L  /* log_e 2 */
#endif
#ifndef M_LN10l
# define M_LN10l        2.3025850929940456840179914546843642L  /* log_e 10 */
#endif
#ifndef M_PIl
# define M_PIl          3.1415926535897932384626433832795029L  /* pi */
#endif
#ifndef M_PI_2l
# define M_PI_2l        1.5707963267948966192313216916397514L  /* pi/2 */
#endif
#ifndef M_PI_4l
# define M_PI_4l        0.7853981633974483096156608458198757L  /* pi/4 */
#endif
#ifndef M_1_PIl
# define M_1_PIl        0.3183098861837906715377675267450287L  /* 1/pi */
#endif
#ifndef M_2_PIl
# define M_2_PIl        0.6366197723675813430755350534900574L  /* 2/pi */
#endif
#ifndef M_2_SQRTPIl
# define M_2_SQRTPIl    1.1283791670955125738961589031215452L  /* 2/sqrt(pi) */
#endif
#ifndef M_SQRT2l
# define M_SQRT2l       1.4142135623730950488016887242096981L  /* sqrt(2) */
#endif
#ifndef M_SQRT1_2l
# define M_SQRT1_2l     0.7071067811865475244008443621048490L  /* 1/sqrt(2) */
#endif


namespace glvm
{
    /* First: basic numeric constants.
     * Why are these still not properly available in C++???? */
    template<typename T> inline T const_e();
    template<> inline float const_e<float>() { return M_El; }
    template<> inline double const_e<double>() { return M_El; }
    template<> inline long double const_e<long double>() { return M_El; }
    template<typename T> inline T const_log2e();
    template<> inline float const_log2e<float>() { return M_LOG2El; }
    template<> inline double const_log2e<double>() { return M_LOG2El; }
    template<> inline long double const_log2e<long double>() { return M_LOG2El; }
    template<typename T> inline T const_log10e();
    template<> inline float const_log10e<float>() { return M_LOG10El; }
    template<> inline double const_log10e<double>() { return M_LOG10El; }
    template<> inline long double const_log10e<long double>() { return M_LOG10El; }
    template<typename T> inline T const_ln2();
    template<> inline float const_ln2<float>() { return M_LN2l; }
    template<> inline double const_ln2<double>() { return M_LN2l; }
    template<> inline long double const_ln2<long double>() { return M_LN2l; }
    template<typename T> inline T const_ln10();
    template<> inline float const_ln10<float>() { return M_LN10l; }
    template<> inline double const_ln10<double>() { return M_LN10l; }
    template<> inline long double const_ln10<long double>() { return M_LN10l; }
    template<typename T> inline T const_pi();
    template<> inline float const_pi<float>() { return M_PIl; }
    template<> inline double const_pi<double>() { return M_PIl; }
    template<> inline long double const_pi<long double>() { return M_PIl; }
    template<typename T> inline T const_pi_2();
    template<> inline float const_pi_2<float>() { return M_PI_2l; }
    template<> inline double const_pi_2<double>() { return M_PI_2l; }
    template<> inline long double const_pi_2<long double>() { return M_PI_2l; }
    template<typename T> inline T const_pi_4();
    template<> inline float const_pi_4<float>() { return M_PI_4l; }
    template<> inline double const_pi_4<double>() { return M_PI_4l; }
    template<> inline long double const_pi_4<long double>() { return M_PI_4l; }
    template<typename T> inline T const_1_pi();
    template<> inline float const_1_pi<float>() { return M_1_PIl; }
    template<> inline double const_1_pi<double>() { return M_1_PIl; }
    template<> inline long double const_1_pi<long double>() { return M_1_PIl; }
    template<typename T> inline T const_2_pi();
    template<> inline float const_2_pi<float>() { return M_2_PIl; }
    template<> inline double const_2_pi<double>() { return M_2_PIl; }
    template<> inline long double const_2_pi<long double>() { return M_2_PIl; }
    template<typename T> inline T const_2_sqrtpi();
    template<> inline float const_2_sqrtpi<float>() { return M_2_SQRTPIl; }
    template<> inline double const_2_sqrtpi<double>() { return M_2_SQRTPIl; }
    template<> inline long double const_2_sqrtpi<long double>() { return M_2_SQRTPIl; }
    template<typename T> inline T const_sqrt2();
    template<> inline float const_sqrt2<float>() { return M_SQRT2l; }
    template<> inline double const_sqrt2<double>() { return M_SQRT2l; }
    template<> inline long double const_sqrt2<long double>() { return M_SQRT2l; }
    template<typename T> inline T const_sqrt1_2();
    template<> inline float const_sqrt1_2<float>() { return M_SQRT1_2l; }
    template<> inline double const_sqrt1_2<double>() { return M_SQRT1_2l; }
    template<> inline long double const_sqrt1_2<long double>() { return M_SQRT1_2l; }

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
     *
     * We cannot use templates here because then the compiler would try to apply
     * these functions to vec and mat. We use templates to define a different
     * behaviour for vec and mat below.
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
        return glvm::min(maxval, glvm::max(minval, x));
    }

    inline unsigned char clamp(unsigned char x, unsigned char minval, unsigned char maxval)
    {
        return glvm::min(maxval, glvm::max(minval, x));
    }

    inline short clamp(short x, short minval, short maxval)
    {
        return glvm::min(maxval, glvm::max(minval, x));
    }

    inline unsigned short clamp(unsigned short x, unsigned short minval, unsigned short maxval)
    {
        return glvm::min(maxval, glvm::max(minval, x));
    }

    inline int clamp(int x, int minval, int maxval)
    {
        return glvm::min(maxval, glvm::max(minval, x));
    }

    inline unsigned int clamp(unsigned int x, unsigned int minval, unsigned int maxval)
    {
        return glvm::min(maxval, glvm::max(minval, x));
    }

    inline long clamp(long x, long minval, long maxval)
    {
        return glvm::min(maxval, glvm::max(minval, x));
    }

    inline unsigned long clamp(unsigned long x, unsigned long minval, unsigned long maxval)
    {
        return glvm::min(maxval, glvm::max(minval, x));
    }

    inline long long clamp(long long x, long long minval, long long maxval)
    {
        return glvm::min(maxval, glvm::max(minval, x));
    }

    inline unsigned long long clamp(unsigned long long x, unsigned long long minval, unsigned long long maxval)
    {
        return glvm::min(maxval, glvm::max(minval, x));
    }

    inline float clamp(float x, float minval, float maxval)
    {
        return glvm::min(maxval, glvm::max(minval, x));
    }

    inline double clamp(double x, double minval, double maxval)
    {
        return glvm::min(maxval, glvm::max(minval, x));
    }

    inline long double clamp(long double x, long double minval, long double maxval)
    {
        return glvm::min(maxval, glvm::max(minval, x));
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

#if __cplusplus >= 201103L
    using std::exp2;
#elif (defined _MSC_VER && _MSC_VER <= 1600)
    inline float exp2(float x) { return std::pow(2.0f, x); }
    inline double exp2(double x) { return std::pow(2.0, x); }
    inline long double exp2(long double x) { return std::pow(2.0L, x); }
#else
    using ::exp2;
    inline float exp2(const float x)
    {
        return ::exp2f(x);
    }
    inline long double exp2(const long double x)
    {
        return ::exp2l(x);
    }
#endif

    using std::log;

#if __cplusplus >= 201103L
    using std::log2;
#elif (defined _MSC_VER && _MSC_VER <= 1600)
    inline float log2(float x) { return std::log(x) / std::log(2.0f); }
    inline double log2(double x) { return std::log(x) / std::log(2.0); }
    inline long double log2(long double x) { return std::log(x) / std::log(2.0L); }
#else
    using ::log2;
    inline float log2(const float x)
    {
        return ::log2f(x);
    }
    inline long double log2(const long double x)
    {
        return ::log2l(x);
    }
#endif

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

#if __cplusplus >= 201103L
    using std::cbrt;
#elif (defined _MSC_VER && _MSC_VER <= 1600)
    inline float cbrt(float x) { return x < 0.0f ? -std::pow(-x, 1.0f / 3.0f) : std::pow(x, 1.0f / 3.0f); }
    inline double cbrt(double x) { return x < 0.0 ? -std::pow(-x, 1.0 / 3.0) : std::pow(x, 1.0 / 3.0); }
    inline long double cbrt(long double x) { return x < 0.0L ? -std::pow(-x, 1.0L / 3.0L) : std::pow(x, 1.0L / 3.0L); }
#else
    using ::cbrt;
    inline float cbrt(const float x)
    {
        return ::cbrtf(x);
    }
    inline long double cbrt(const long double x)
    {
        return ::cbrtl(x);
    }
#endif

#if __cplusplus >= 201103L
    using std::round;
#elif (defined _MSC_VER && _MSC_VER <= 1600)
    inline float round(float x) { return x < 0.0f ? std::ceil(x - 0.5f) : std::floor(x + 0.5f); }
    inline double round(double x) { return x < 0.0 ? std::ceil(x - 0.5) : std::floor(x + 0.5); }
    inline long double round(long double x) { return x < 0.0L ? std::ceil(x - 0.5L) : std::floor(x + 0.5L); }
#else
    using ::round;
    inline float round(const float x)
    {
        return ::roundf(x);
    }
    inline long double round(const long double x)
    {
        return ::roundl(x);
    }
#endif

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

#if (defined _MSC_VER && _MSC_VER <= 1600)
    // XXX: MSVC only has the 'double' version; it is unclear if conversion works as expected
    inline bool isfinite(float x) { return ::_finite(x); }
    inline bool isfinite(double x) { return ::_finite(x); }
    inline bool isfinite(long double x) { return ::_finite(x); }
#else
    using std::isfinite;
#endif

#if (defined _MSC_VER && _MSC_VER <= 1600)
    // XXX: MSVC only has the 'double' version; it is unclear if conversion works as expected
    inline bool isnan(float x) { return ::_isnan(x); }
    inline bool isnan(double x) { return ::_isnan(x); }
    inline bool isnan(long double x) { return ::_isnan(x); }
#else
    using std::isnan;
#endif

#if (defined _MSC_VER && _MSC_VER <= 1600)
    inline bool isinf(float x) { return !isnan(x) && !isfinite(x); }
    inline bool isinf(double x) { return !isnan(x) && !isfinite(x); }
    inline bool isinf(long double x) { return !isnan(x) && !isfinite(x); }
#else
    using std::isinf;
#endif

#if (defined _MSC_VER && _MSC_VER <= 1600)
    // XXX: MSVC only has the 'double' version; it is unclear if conversion works as expected
    inline bool isnormal(float x) { return ::_fpclass(x) == _FPCLASS_NN || ::_fpclass(x) == _FPCLASS_PN; }
    inline bool isnormal(double x) { return ::_fpclass(x) == _FPCLASS_NN || ::_fpclass(x) == _FPCLASS_PN; }
    inline bool isnormal(long double x) { return ::_fpclass(x) == _FPCLASS_NN || ::_fpclass(x) == _FPCLASS_PN; }
#else
    using std::isnormal;
#endif

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
        float t = glvm::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
        return t * t * (3.0f - t * 2.0f);
    }

    inline double smoothstep(double x, double edge0, double edge1)
    {
        double t = glvm::clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0);
        return t * t * (3.0 - t * 2.0);
    }

    inline long double smoothstep(long double x, long double edge0, long double edge1)
    {
        long double t = glvm::clamp((x - edge0) / (edge1 - edge0), 0.0L, 1.0L);
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
        return a > b;
    }

    inline bool greaterThan(double a, double b)
    {
        return a > b;
    }

    inline bool greaterThan(long double a, long double b)
    {
        return a > b;
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
        return a >= b;
    }

    inline bool greaterThanEqual(double a, double b)
    {
        return a >= b;
    }

    inline bool greaterThanEqual(long double a, long double b)
    {
        return a >= b;
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
        return a < b;
    }

    inline bool lessThan(double a, double b)
    {
        return a < b;
    }

    inline bool lessThan(long double a, long double b)
    {
        return a < b;
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
        return a <= b;
    }

    inline bool lessThanEqual(double a, double b)
    {
        return a <= b;
    }

    inline bool lessThanEqual(long double a, long double b)
    {
        return a <= b;
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

    /* The 'equal' function for floating point numbers is adapted from
     * http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm
     *
     * The max_ulps parameter measures the maximum number of floating point
     * numbers that lie inbetween a and b.
     *
     * These functions relies on IEEE 754 floating point numbers and
     * two-complement integers of the same size. The second requirement is the
     * reason why there is no version for long doubles.
     */

    inline bool equal(float a, float b, int max_ulps = 0)
    {
        bool result;

        if (glvm::isinf(a) || glvm::isinf(b))
        {
            result = (glvm::isinf(a) && glvm::isinf(b)
                    && static_cast<int>(glvm::sign(a)) == static_cast<int>(glvm::sign(b)));
        }
        else if (glvm::isnan(a) || glvm::isnan(b))
        {
            result = false;
        }
        else
        {
            //assert(sizeof(float) == sizeof(int32_t));
            //assert(max_ulps > 0);
            union { float f; int32_t i; } x;
            union { float f; int32_t i; } y;
            x.f = a;
            y.f = b;
            if ((x.i < 0 && y.i > 0) || (x.i > 0 && y.i < 0))
            {
                result = false;
            }
            else
            {
                if (x.i < 0)
                {
                    x.i = 0x80000000 - x.i;
                }
                if (y.i < 0)
                {
                    y.i = 0x80000000 - y.i;
                }
                int32_t intdiff = glvm::abs(x.i - y.i);
                result = (intdiff <= max_ulps);
            }
        }
        return result;
    }

    inline bool equal(double a, double b, int max_ulps = 0)
    {
        bool result;

        if (glvm::isinf(a) || glvm::isinf(b))
        {
            result = (glvm::isinf(a) && glvm::isinf(b)
                    && static_cast<int>(glvm::sign(a)) == static_cast<int>(glvm::sign(b)));
        }
        else if (glvm::isnan(a) || glvm::isnan(b))
        {
            result = false;
        }
        else
        {
            //assert(sizeof(double) == sizeof(int64_t));
            //assert(max_ulps > 0);
            union { double d; int64_t i; } x;
            union { double d; int64_t i; } y;
            x.d = a;
            y.d = b;
            if ((x.i < 0 && y.i > 0) || (x.i > 0 && y.i < 0))
            {
                result = false;
            }
            else
            {
                if (x.i < 0)
                {
                    x.i = 0x8000000000000000LL - x.i;
                }
                if (y.i < 0)
                {
                    y.i = 0x8000000000000000LL - y.i;
                }
                int64_t intdiff = glvm::abs(x.i - y.i);
                result = (intdiff <= max_ulps);
            }
        }
        return result;
    }

    inline bool equal(long double a, long double b, int max_ulps = 0)
    {
        bool result;

        if (glvm::isinf(a) || glvm::isinf(b))
        {
            result = (glvm::isinf(a) && glvm::isinf(b)
                    && static_cast<int>(glvm::sign(a)) == static_cast<int>(glvm::sign(b)));
        }
        else if (glvm::isnan(a) || glvm::isnan(b))
        {
            result = false;
        }
        else
        {
            abort();
            //assert(max_ulps > 0);
            /* FIXME: Implement this for long double.
             * Problem: There is no int128_t on most platforms. */
            result = (max_ulps < 1);
        }
        return result;
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

    inline bool notEqual(float a, float b, int max_ulps = 0)
    {
        return !glvm::equal(a, b, max_ulps);
    }

    inline bool notEqual(double a, double b, int max_ulps = 0)
    {
        return !glvm::equal(a, b, max_ulps);
    }

    inline bool notEqual(long double a, long double b, int max_ulps = 0)
    {
        return !glvm::equal(a, b, max_ulps);
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
        return x < one ? zero : (sizeof(unsigned int) * CHAR_BIT - 1 - __builtin_clz(x));
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
        return x < one ? zero : (sizeof(unsigned long) * CHAR_BIT - 1 - __builtin_clzl(x));
#else
        return _log2(x);
#endif
    }
    template<typename T> inline T _log2ll(T x)
    {
#ifdef __GNUC__
        const T zero = static_cast<T>(0);
        const T one = static_cast<T>(1);
        return x < one ? zero : (sizeof(unsigned long long) * CHAR_BIT - 1 - __builtin_clzll(x));
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


    /* The array class is the base for both the vec and the mat class. */

    template<typename T, int cols, int rows>
    class array_data
    {
    public:
        union {
            T v[cols][rows];
            T vl[cols * rows];
        };
    };

    template<typename T>
    class array_data<T, 1, 2>
    {
    public:
        union {
            T v[1][2];
            T vl[2];
            struct {
                union {
                    T x, s, r;
                };
                union {
                    T y, t, g;
                };
            };
        };
    };

    template<typename T>
    class array_data<T, 1, 3>
    {
    public:
        union {
            T v[1][3];
            T vl[3];
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
        };
    };

    template<typename T>
    class array_data<T, 1, 4>
    {
    public:
        union {
            T v[1][4];
            T vl[4];
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
        };
    };

    template<typename T, int cols, int rows>
    class array : public array_data<T, cols, rows>
    {
    public:
        using array_data<T, cols, rows>::v;
        using array_data<T, cols, rows>::vl;

        /* Constructors, Destructor */

        array() {}

        // The following constructor are needed to construct vec and mat
        // objects from a series of values. We need constructors for 1, 2, 3,
        // 4, 6, 8, 9, 12, and 16 elements.

        explicit array(T x)
        {
            for (int i = 0; i < cols * rows; i++)
                vl[i] = x;
        }

        explicit array(T x, T y)
        {
            vl[0] = x;
            vl[1] = y;
        }

        explicit array(T x, T y, T z)
        {
            vl[0] = x;
            vl[1] = y;
            vl[2] = z;
        }

        explicit array(T x, T y, T z, T w)
        {
            vl[0] = x;
            vl[1] = y;
            vl[2] = z;
            vl[3] = w;
        }

        explicit array(T v0, T v1, T v2, T v3, T v4, T v5)
        {
            vl[0] = v0;
            vl[1] = v1;
            vl[2] = v2;
            vl[3] = v3;
            vl[4] = v4;
            vl[5] = v5;
        }

        explicit array(T v0, T v1, T v2, T v3, T v4, T v5, T v6, T v7)
        {
            vl[0] = v0;
            vl[1] = v1;
            vl[2] = v2;
            vl[3] = v3;
            vl[4] = v4;
            vl[5] = v5;
            vl[6] = v6;
            vl[7] = v7;
        }

        explicit array(T v0, T v1, T v2, T v3, T v4, T v5, T v6, T v7, T v8)
        {
            vl[0] = v0;
            vl[1] = v1;
            vl[2] = v2;
            vl[3] = v3;
            vl[4] = v4;
            vl[5] = v5;
            vl[6] = v6;
            vl[7] = v7;
            vl[8] = v8;
        }

        explicit array(T v0, T v1, T v2, T v3, T v4, T v5, T v6, T v7, T v8, T v9, T v10, T v11)
        {
            vl[0] = v0;
            vl[1] = v1;
            vl[2] = v2;
            vl[3] = v3;
            vl[4] = v4;
            vl[5] = v5;
            vl[6] = v6;
            vl[7] = v7;
            vl[8] = v8;
            vl[9] = v9;
            vl[10] = v10;
            vl[11] = v11;
        }

        explicit array(T v0, T v1, T v2, T v3, T v4, T v5, T v6, T v7, T v8, T v9, T v10, T v11, T v12, T v13, T v14, T v15)
        {
            vl[0] = v0;
            vl[1] = v1;
            vl[2] = v2;
            vl[3] = v3;
            vl[4] = v4;
            vl[5] = v5;
            vl[6] = v6;
            vl[7] = v7;
            vl[8] = v8;
            vl[9] = v9;
            vl[10] = v10;
            vl[11] = v11;
            vl[12] = v12;
            vl[13] = v13;
            vl[14] = v14;
            vl[15] = v15;
        }

        explicit array(const T* a)
        {
            for (int i = 0; i < cols * rows; i++)
                vl[i] = a[i];
        }

        template<typename U> explicit
        array(const array<U, cols, rows>& a)
        {
            for (int i = 0; i < cols * rows; i++)
                vl[i] = a.vl[i];
        }

        ~array()
        {
        }

        array<T, cols - 1, rows - 1> _strike(int col, int row) const
        {
            array<T, cols - 1, rows - 1> r;
            int ii = 0;
            for (int i = 0; i < cols; i++) {
                if (i == col)
                    continue;
                int jj = 0;
                for (int j = 0; j < rows; j++) {
                    if (j == row)
                        continue;
                    r.v[ii][jj] = v[i][j];
                    jj++;
                }
                ii++;
            }
            return r;
        }

        template<int subcols, int subrows>
        void _set_sub_array(const array<T, subcols, subrows>& m, int col = 0, int row = 0)
        {
            for (int i = 0; i < subcols; i++)
                for (int j = 0; j < subrows; j++)
                    v[col + i][row + j] = m.v[i][j];
        }

        /* Operators */

        const array& _op_assign(const array& a)
        {
            for (int i = 0; i < cols * rows; i++)
                vl[i] = a.vl[i];
            return *this;
        }

        array _op_plus(const array& a) const
        {
            array r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = vl[i] + a.vl[i];
            return r;
        }

        const array& _op_plus_assign(const array& a)
        {
            for (int i = 0; i < cols * rows; i++)
                vl[i] += a.vl[i];
            return *this;
        }

        array _op_unary_plus() const
        {
            return *this;
        }

        array _op_minus(const array& a) const
        {
            array r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = vl[i] - a.vl[i];
            return r;
        }

        const array& _op_minus_assign(const array& a)
        {
            for (int i = 0; i < cols * rows; i++)
                vl[i] -= a.vl[i];
            return *this;
        }

        array _op_unary_minus() const
        {
            array r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = -vl[i];
            return r;
        }

        array _op_scal_mult(const T s) const
        {
            array r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = vl[i] * s;
            return r;
        }

        const array& _op_scal_mult_assign(const T s)
        {
            for (int i = 0; i < cols * rows; i++)
                vl[i] *= s;
            return *this;
        }

        array _op_comp_mult(const array& a) const
        {
            array r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = vl[i] * a.vl[i];
            return r;
        }

        const array& _op_comp_mult_assign(const array& a)
        {
            for (int i = 0; i < cols * rows; i++)
                vl[i] *= a.vl[i];
            return *this;
        }

        array _op_scal_div(const T s) const
        {
            array r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = vl[i] / s;
            return r;
        }

        const array& _op_scal_div_assign(const T s)
        {
            for (int i = 0; i < cols * rows; i++)
                vl[i] /= s;
            return *this;
        }

        array _op_comp_div(const array& a) const
        {
            array r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = vl[i] / a.vl[i];
            return r;
        }

        const array& _op_comp_div_assign(const array& a)
        {
            for (int i = 0; i < cols * rows; i++)
                vl[i] /= a.vl[i];
            return *this;
        }

        array _op_scal_mod(const T s) const
        {
            array r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = vl[i] % s;
            return r;
        }

        const array& _op_scal_mod_assign(const T s)
        {
            for (int i = 0; i < cols * rows; i++)
                vl[i] %= s;
            return *this;
        }

        array _op_comp_mod(const array& a) const
        {
            array r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = vl[i] % a.vl[i];
            return r;
        }

        const array& _op_comp_mod_assign(const array& a)
        {
            for (int i = 0; i < cols * rows; i++)
                vl[i] %= a.vl[i];
            return *this;
        }

        bool _op_equal(const array& a) const
        {
            bool ret = true;
            for (int i = 0; i < cols * rows; i++) {
                if (!equal(vl[i], a.vl[i])) {
                    ret = false;
                    break;
                }
            }
            return ret;
        }

        bool _op_notequal(const array& a) const
        {
            return !this->_op_equal(a);
        }

        /* Trigonometric functions */

        array _sin() const
        {
            array r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = glvm::sin(vl[i]);
            return r;
        }

        array _cos() const
        {
            array r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = glvm::cos(vl[i]);
            return r;
        }

        array _tan() const
        {
            array r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = glvm::tan(vl[i]);
            return r;
        }

        array _asin() const
        {
            array r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = glvm::asin(vl[i]);
            return r;
        }

        array _acos() const
        {
            array r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = glvm::acos(vl[i]);
            return r;
        }

        array _atan() const
        {
            array r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = glvm::atan(vl[i]);
            return r;
        }

        array _atan(const array& a) const
        {
            array r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = glvm::atan(vl[i], a.vl[i]);
            return r;
        }

        array _radians() const
        {
            array r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = glvm::radians(vl[i]);
            return r;
        }

        array _degrees() const
        {
            array r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = glvm::degrees(vl[i]);
            return r;
        }

        /* Exponential functions */

        array _pow(const T p) const
        {
            array r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = glvm::pow(vl[i], p);
            return r;
        }

        array _exp() const
        {
            array r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = glvm::exp(vl[i]);
            return r;
        }

        array _exp2() const
        {
            array r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = glvm::exp2(vl[i]);
            return r;
        }

        array _log() const
        {
            array r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = glvm::log(vl[i]);
            return r;
        }

        array _log2() const
        {
            array r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = glvm::log2(vl[i]);
            return r;
        }

        array _log10() const
        {
            array r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = glvm::log10(vl[i]);
            return r;
        }

        array _sqrt() const
        {
            array r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = glvm::sqrt(vl[i]);
            return r;
        }

        array _inversesqrt() const
        {
            array r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = glvm::inversesqrt(vl[i]);
            return r;
        }

        array _cbrt() const
        {
            array r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = glvm::cbrt(vl[i]);
            return r;
        }

        /* Common functions */

        array<bool, cols, rows> _isfinite() const
        {
            array<bool, cols, rows> r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = glvm::isfinite(vl[i]);
            return r;
        }

        array<bool, cols, rows> _isinf() const
        {
            array<bool, cols, rows> r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = glvm::isinf(vl[i]);
            return r;
        }

        array<bool, cols, rows> _isnan() const
        {
            array<bool, cols, rows> r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = glvm::isnan(vl[i]);
            return r;
        }

        array<bool, cols, rows> _isnormal() const
        {
            array<bool, cols, rows> r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = glvm::isnormal(vl[i]);
            return r;
        }

        array _sign() const
        {
            array r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = glvm::sign(vl[i]);
            return r;
        }

        array _abs() const
        {
            array r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = glvm::abs(vl[i]);
            return r;
        }

        array _floor() const
        {
            array r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = glvm::floor(vl[i]);
            return r;
        }

        array _ceil() const
        {
            array r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = glvm::ceil(vl[i]);
            return r;
        }

        array _round() const
        {
            array r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = glvm::round(vl[i]);
            return r;
        }

        array _fract() const
        {
            array r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = glvm::fract(vl[i]);
            return r;
        }

        array _min(const T x) const
        {
            array r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = glvm::min(x, vl[i]);
            return r;
        }

        array _min(const array& a) const
        {
            array r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = glvm::min(a.vl[i], vl[i]);
            return r;
        }

        array _max(const T x) const
        {
            array r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = glvm::max(x, vl[i]);
            return r;
        }

        array _max(const array& a) const
        {
            array r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = glvm::max(a.vl[i], vl[i]);
            return r;
        }

        array _clamp(const T minval, const T maxval) const
        {
            array r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = glvm::clamp(vl[i], minval, maxval);
            return r;
        }

        array _clamp(const array& minval, const array& maxval) const
        {
            array r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = glvm::clamp(vl[i], minval.vl[i], maxval.vl[i]);
            return r;
        }

        array _mix(const array& a, const T alpha) const
        {
            array r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = glvm::mix(vl[i], a.vl[i], alpha);
            return r;
        }

        array _mix(const array& a, const array& alpha) const
        {
            array r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = glvm::mix(vl[i], a.vl[i], alpha.vl[i]);
            return r;
        }

        array _step(const T edge) const
        {
            array r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = glvm::step(vl[i], edge);
            return r;
        }

        array _step(const array& edge) const
        {
            array r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = glvm::step(vl[i], edge.vl[i]);
            return r;
        }

        array _smoothstep(const T edge0, const T edge1) const
        {
            array r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = glvm::smoothstep(vl[i], edge0, edge1);
            return r;
        }

        array _smoothstep(const array& edge0, const array& edge1) const
        {
            array r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = glvm::smoothstep(vl[i], edge0.vl[i], edge1.vl[i]);
            return r;
        }

        array _mod(const T y) const
        {
            array r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = glvm::mod(vl[i], y);
            return r;
        }

        array _mod(const array& y) const
        {
            array r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = glvm::mod(vl[i], y.vl[i]);
            return r;
        }

        /* Comparison functions */

        array<bool, cols, rows> _greaterThan(const array& a) const
        {
            array<bool, cols, rows> r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = glvm::greaterThan(vl[i], a.vl[i]);
            return r;
        }

        array<bool, cols, rows> _greaterThanEqual(const array& a) const
        {
            array<bool, cols, rows> r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = glvm::greaterThanEqual(vl[i], a.vl[i]);
            return r;
        }

        array<bool, cols, rows> _lessThan(const array& a) const
        {
            array<bool, cols, rows> r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = glvm::lessThan(vl[i], a.vl[i]);
            return r;
        }

        array<bool, cols, rows> _lessThanEqual(const array& a) const
        {
            array<bool, cols, rows> r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = glvm::lessThanEqual(vl[i], a.vl[i]);
            return r;
        }

        array<bool, cols, rows> _equal(const array& a) const
        {
            array<bool, cols, rows> r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = glvm::equal(vl[i], a.vl[i]);
            return r;
        }

        array<bool, cols, rows> _equal(const array& a, int max_ulps) const
        {
            array<bool, cols, rows> r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = glvm::equal(vl[i], a.vl[i], max_ulps);
            return r;
        }

        array<bool, cols, rows> _notEqual(const array& a) const
        {
            array<bool, cols, rows> r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = glvm::notEqual(vl[i], a.vl[i]);
            return r;
        }

        array<bool, cols, rows> _notEqual(const array& a, int max_ulps) const
        {
            array<bool, cols, rows> r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = glvm::notEqual(vl[i], a.vl[i], max_ulps);
            return r;
        }

        bool _any() const
        {
            bool ret = false;
            for (int i = 0; i < cols * rows; i++) {
                if (vl[i]) {
                    ret = true;
                    break;
                }
            }
            return ret;
        }

        bool _all() const
        {
            bool ret = true;
            for (int i = 0; i < cols * rows; i++) {
                if (!vl[i]) {
                    ret = false;
                    break;
                }
            }
            return ret;
        }

        array<bool, cols, rows> _negate() const
        {
            array<bool, cols, rows> r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = !vl[i];
            return r;
        }

        /* Geometric functions */

        T _length() const
        {
            T l = static_cast<T>(0);
            for (int i = 0; i < cols * rows; i++)
                l += vl[i] * vl[i];
            return glvm::sqrt(l);
        }

        T _distance(const array& a) const
        {
            return (*this - a)._length();
        }

        T _dot(const array& a) const
        {
            T d = static_cast<T>(0);
            for (int i = 0; i < cols * rows; i++)
                d += vl[i] * a.vl[i];
            return d;
        }

        array _normalize() const
        {
            return this->_op_scal_div(this->_length());
        }

        array _faceforward(const array& I, const array& Nref) const
        {
            return Nref._dot(I) < static_cast<T>(0) ? *this : - *this;
        }

        array _reflect(const array& N) const
        {
            return *this - static_cast<T>(2) * N._dot(*this) * N;
        }

        array _refract(const array& N, T eta) const
        {
            const T d = N._dot(*this);
            const T k = static_cast<T>(1) - eta * eta * (static_cast<T>(1) - d * d);
            return k < static_cast<T>(0) ? array<T, cols, 1>(static_cast<T>(0)) : *this * eta - N * (eta * d + glvm::sqrt(k));
        }

        array<T, rows, cols> _transpose() const
        {
            array<T, rows, cols> r;
            for (int i = 0; i < rows; i++)
                for (int j = 0; j < cols; j++)
                    r.v[i][j] = array<T, cols, rows>::v[j][i];
            return r;
        }

        // Bonus functions. Note that log2 is already covered.

        array<bool, cols, rows> _is_pow2() const
        {
            array<bool, cols, rows> r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = glvm::is_pow2(vl[i]);
            return r;
        }

        array _next_pow2() const
        {
            array r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = glvm::next_pow2(vl[i]);
            return r;
        }

        array _next_multiple(T y) const
        {
            array r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = glvm::next_multiple(vl[i], y);
            return r;
        }

        array _next_multiple(const array& y) const
        {
            array r;
            for (int i = 0; i < cols * rows; i++)
                r.vl[i] = glvm::next_multiple(vl[i], y.vl[i]);
            return r;
        }
    };


    /* Swizzlers */

    template<typename T, int rows>
    class swizzler : public array<T, 1, rows>
    {
    };

    template<typename T>
    class swizzler<T, 2> : public array<T, 1, 2>
    {
    public:
        T& x;
        T& y;

        swizzler(T& s0, T& s1) : array<T, 1, 2>(s0, s1), x(s0), y(s1)
        {
        }

        swizzler(T s[2]) : array<T, 1, 2>(s[0], s[1]), x(s[0]), y(s[1])
        {
        }

        const swizzler& operator=(const swizzler& s)
        {
            T a = s.x;
            T b = s.y;
            x = a;
            y = b;
            array<T, 1, 2>::vl[0] = a;
            array<T, 1, 2>::vl[1] = b;
            return *this;
        }

        const swizzler& operator=(const array<T, 1, 2>& v)
        {
            x = v.vl[0];
            y = v.vl[1];
            array<T, 1, 2>::vl[0] = v.vl[0];
            array<T, 1, 2>::vl[1] = v.vl[1];
            return *this;
        }

        const swizzler& operator+=(const array<T, 1, 2>& a)
        {
            x = array<T, 1, 2>::vl[0] + a.vl[0];
            y = array<T, 1, 2>::vl[1] + a.vl[1];
            array<T, 1, 2>::vl[0] = x;
            array<T, 1, 2>::vl[1] = y;
            return *this;
        }

        const swizzler& operator-=(const array<T, 1, 2>& a)
        {
            x = array<T, 1, 2>::vl[0] - a.vl[0];
            y = array<T, 1, 2>::vl[1] - a.vl[1];
            array<T, 1, 2>::vl[0] = x;
            array<T, 1, 2>::vl[1] = y;
            return *this;
        }

        const swizzler& operator*=(const array<T, 1, 2>& a)
        {
            x = array<T, 1, 2>::vl[0] * a.vl[0];
            y = array<T, 1, 2>::vl[1] * a.vl[1];
            array<T, 1, 2>::vl[0] = x;
            array<T, 1, 2>::vl[1] = y;
            return *this;
        }

        const swizzler& operator*=(T s)
        {
            x = array<T, 1, 2>::vl[0] * s;
            y = array<T, 1, 2>::vl[1] * s;
            array<T, 1, 2>::vl[0] = x;
            array<T, 1, 2>::vl[1] = y;
            return *this;
        }

        const swizzler& operator/=(const array<T, 1, 2>& a)
        {
            x = array<T, 1, 2>::vl[0] / a.vl[0];
            y = array<T, 1, 2>::vl[1] / a.vl[1];
            array<T, 1, 2>::vl[0] = x;
            array<T, 1, 2>::vl[1] = y;
            return *this;
        }

        const swizzler& operator/=(T s)
        {
            x = array<T, 1, 2>::vl[0] / s;
            y = array<T, 1, 2>::vl[1] / s;
            array<T, 1, 2>::vl[0] = x;
            array<T, 1, 2>::vl[1] = y;
            return *this;
        }

        const swizzler& operator%=(const array<T, 1, 2>& a)
        {
            x = array<T, 1, 2>::vl[0] % a.vl[0];
            y = array<T, 1, 2>::vl[1] % a.vl[1];
            array<T, 1, 2>::vl[0] = x;
            array<T, 1, 2>::vl[1] = y;
            return *this;
        }

        const swizzler& operator%=(T s)
        {
            x = array<T, 1, 2>::vl[0] % s;
            y = array<T, 1, 2>::vl[1] % s;
            array<T, 1, 2>::vl[0] = x;
            array<T, 1, 2>::vl[1] = y;
            return *this;
        }

        T& operator[](unsigned int i)
        {
            return (i == 0 ? x : y);
        }

        const T& operator[](unsigned int i) const
        {
            return (i == 0 ? x : y);
        }
    };

    template<typename T>
    class swizzler<T, 3> : public array<T, 1, 3>
    {
    public:
        T& x;
        T& y;
        T& z;

        swizzler(T& s0, T& s1, T& s2) : array<T, 1, 3>(s0, s1, s2), x(s0), y(s1), z(s2)
        {
        }

        swizzler(T s[3]) : array<T, 1, 3>(s[0], s[1], s[2]), x(s[0]), y(s[1]), z(s[2])
        {
        }

        const swizzler& operator=(const swizzler& s)
        {
            T a = s.x;
            T b = s.y;
            T c = s.z;
            x = a;
            y = b;
            z = c;
            array<T, 1, 3>::vl[0] = a;
            array<T, 1, 3>::vl[1] = b;
            array<T, 1, 3>::vl[2] = c;
            return *this;
        }

        const swizzler& operator=(const array<T, 1, 3>& v)
        {
            x = v.vl[0];
            y = v.vl[1];
            z = v.vl[2];
            array<T, 1, 3>::vl[0] = v.vl[0];
            array<T, 1, 3>::vl[1] = v.vl[1];
            array<T, 1, 3>::vl[2] = v.vl[2];
            return *this;
        }

        const swizzler& operator+=(const array<T, 1, 3>& a)
        {
            x = array<T, 1, 3>::vl[0] + a.vl[0];
            y = array<T, 1, 3>::vl[1] + a.vl[1];
            z = array<T, 1, 3>::vl[2] + a.vl[2];
            array<T, 1, 3>::vl[0] = x;
            array<T, 1, 3>::vl[1] = y;
            array<T, 1, 3>::vl[2] = z;
            return *this;
        }

        const swizzler& operator-=(const array<T, 1, 3>& a)
        {
            x = array<T, 1, 3>::vl[0] - a.vl[0];
            y = array<T, 1, 3>::vl[1] - a.vl[1];
            z = array<T, 1, 3>::vl[2] - a.vl[2];
            array<T, 1, 3>::vl[0] = x;
            array<T, 1, 3>::vl[1] = y;
            array<T, 1, 3>::vl[2] = z;
            return *this;
        }

        const swizzler& operator*=(const array<T, 1, 3>& a)
        {
            x = array<T, 1, 3>::vl[0] * a.vl[0];
            y = array<T, 1, 3>::vl[1] * a.vl[1];
            z = array<T, 1, 3>::vl[2] * a.vl[2];
            array<T, 1, 3>::vl[0] = x;
            array<T, 1, 3>::vl[1] = y;
            array<T, 1, 3>::vl[2] = z;
            return *this;
        }

        const swizzler& operator*=(T s)
        {
            x = array<T, 1, 3>::vl[0] * s;
            y = array<T, 1, 3>::vl[1] * s;
            z = array<T, 1, 3>::vl[2] * s;
            array<T, 1, 3>::vl[0] = x;
            array<T, 1, 3>::vl[1] = y;
            array<T, 1, 3>::vl[2] = z;
            return *this;
        }

        const swizzler& operator/=(const array<T, 1, 3>& a)
        {
            x = array<T, 1, 3>::vl[0] / a.vl[0];
            y = array<T, 1, 3>::vl[1] / a.vl[1];
            z = array<T, 1, 3>::vl[2] / a.vl[2];
            array<T, 1, 3>::vl[0] = x;
            array<T, 1, 3>::vl[1] = y;
            array<T, 1, 3>::vl[2] = z;
            return *this;
        }

        const swizzler& operator/=(T s)
        {
            x = array<T, 1, 3>::vl[0] / s;
            y = array<T, 1, 3>::vl[1] / s;
            z = array<T, 1, 3>::vl[2] / s;
            array<T, 1, 3>::vl[0] = x;
            array<T, 1, 3>::vl[1] = y;
            array<T, 1, 3>::vl[2] = z;
            return *this;
        }

        const swizzler& operator%=(const array<T, 1, 3>& a)
        {
            x = array<T, 1, 3>::vl[0] % a.vl[0];
            y = array<T, 1, 3>::vl[1] % a.vl[1];
            z = array<T, 1, 3>::vl[2] % a.vl[2];
            array<T, 1, 3>::vl[0] = x;
            array<T, 1, 3>::vl[1] = y;
            array<T, 1, 3>::vl[2] = z;
            return *this;
        }

        const swizzler& operator%=(T s)
        {
            x = array<T, 1, 3>::vl[0] % s;
            y = array<T, 1, 3>::vl[1] % s;
            z = array<T, 1, 3>::vl[2] % s;
            array<T, 1, 3>::vl[0] = x;
            array<T, 1, 3>::vl[1] = y;
            array<T, 1, 3>::vl[2] = z;
            return *this;
        }

        T& operator[](unsigned int i)
        {
            return (i == 0 ? x : i == 1 ? y : z);
        }

        const T& operator[](unsigned int i) const
        {
            return (i == 0 ? x : i == 1 ? y : z);
        }
    };

    template<typename T>
    class swizzler<T, 4> : public array<T, 1, 4>
    {
    public:
        T& x;
        T& y;
        T& z;
        T& w;

        swizzler(T& s0, T& s1, T& s2, T& s3) : array<T, 1, 4>(s0, s1, s2, s3), x(s0), y(s1), z(s2), w(s3)
        {
        }

        swizzler(T s[4]) : array<T, 1, 4>(s[0], s[1], s[2], s[3]), x(s[0]), y(s[1]), z(s[2]), w(s[3])
        {
        }

        const swizzler& operator=(const swizzler& s)
        {
            T a = s.x;
            T b = s.y;
            T c = s.z;
            T d = s.w;
            x = a;
            y = b;
            z = c;
            w = d;
            array<T, 1, 4>::vl[0] = a;
            array<T, 1, 4>::vl[1] = b;
            array<T, 1, 4>::vl[2] = c;
            array<T, 1, 4>::vl[3] = d;
            return *this;
        }

        const swizzler& operator=(const array<T, 1, 4>& v)
        {
            x = v.vl[0];
            y = v.vl[1];
            z = v.vl[2];
            w = v.vl[3];
            array<T, 1, 4>::vl[0] = v.vl[0];
            array<T, 1, 4>::vl[1] = v.vl[1];
            array<T, 1, 4>::vl[2] = v.vl[2];
            array<T, 1, 4>::vl[3] = v.vl[3];
            return *this;
        }

        const swizzler& operator+=(const array<T, 1, 4>& a)
        {
            x = array<T, 1, 4>::vl[0] + a.vl[0];
            y = array<T, 1, 4>::vl[1] + a.vl[1];
            z = array<T, 1, 4>::vl[2] + a.vl[2];
            w = array<T, 1, 4>::vl[3] + a.vl[3];
            array<T, 1, 4>::vl[0] = x;
            array<T, 1, 4>::vl[1] = y;
            array<T, 1, 4>::vl[2] = z;
            array<T, 1, 4>::vl[3] = w;
            return *this;
        }

        const swizzler& operator-=(const array<T, 1, 4>& a)
        {
            x = array<T, 1, 4>::vl[0] - a.vl[0];
            y = array<T, 1, 4>::vl[1] - a.vl[1];
            z = array<T, 1, 4>::vl[2] - a.vl[2];
            w = array<T, 1, 4>::vl[3] - a.vl[3];
            array<T, 1, 4>::vl[0] = x;
            array<T, 1, 4>::vl[1] = y;
            array<T, 1, 4>::vl[2] = z;
            array<T, 1, 4>::vl[3] = w;
            return *this;
        }

        const swizzler& operator*=(const array<T, 1, 4>& a)
        {
            x = array<T, 1, 4>::vl[0] * a.vl[0];
            y = array<T, 1, 4>::vl[1] * a.vl[1];
            z = array<T, 1, 4>::vl[2] * a.vl[2];
            w = array<T, 1, 4>::vl[3] * a.vl[3];
            array<T, 1, 4>::vl[0] = x;
            array<T, 1, 4>::vl[1] = y;
            array<T, 1, 4>::vl[2] = z;
            array<T, 1, 4>::vl[3] = w;
            return *this;
        }

        const swizzler& operator*=(T s)
        {
            x = array<T, 1, 4>::vl[0] * s;
            y = array<T, 1, 4>::vl[1] * s;
            z = array<T, 1, 4>::vl[2] * s;
            w = array<T, 1, 4>::vl[3] * s;
            array<T, 1, 4>::vl[0] = x;
            array<T, 1, 4>::vl[1] = y;
            array<T, 1, 4>::vl[2] = z;
            array<T, 1, 4>::vl[3] = w;
            return *this;
        }

        const swizzler& operator/=(const array<T, 1, 4>& a)
        {
            x = array<T, 1, 4>::vl[0] / a.vl[0];
            y = array<T, 1, 4>::vl[1] / a.vl[1];
            z = array<T, 1, 4>::vl[2] / a.vl[2];
            w = array<T, 1, 4>::vl[3] / a.vl[3];
            array<T, 1, 4>::vl[0] = x;
            array<T, 1, 4>::vl[1] = y;
            array<T, 1, 4>::vl[2] = z;
            array<T, 1, 4>::vl[3] = w;
            return *this;
        }

        const swizzler& operator/=(T s)
        {
            x = array<T, 1, 4>::vl[0] / s;
            y = array<T, 1, 4>::vl[1] / s;
            z = array<T, 1, 4>::vl[2] / s;
            w = array<T, 1, 4>::vl[3] / s;
            array<T, 1, 4>::vl[0] = x;
            array<T, 1, 4>::vl[1] = y;
            array<T, 1, 4>::vl[2] = z;
            array<T, 1, 4>::vl[3] = w;
            return *this;
        }

        const swizzler& operator%=(const array<T, 1, 4>& a)
        {
            x = array<T, 1, 4>::vl[0] % a.vl[0];
            y = array<T, 1, 4>::vl[1] % a.vl[1];
            z = array<T, 1, 4>::vl[2] % a.vl[2];
            w = array<T, 1, 4>::vl[3] % a.vl[3];
            array<T, 1, 4>::vl[0] = x;
            array<T, 1, 4>::vl[1] = y;
            array<T, 1, 4>::vl[2] = z;
            array<T, 1, 4>::vl[3] = w;
            return *this;
        }

        const swizzler& operator%=(T s)
        {
            x = array<T, 1, 4>::vl[0] % s;
            y = array<T, 1, 4>::vl[1] % s;
            z = array<T, 1, 4>::vl[2] % s;
            w = array<T, 1, 4>::vl[3] % s;
            array<T, 1, 4>::vl[0] = x;
            array<T, 1, 4>::vl[1] = y;
            array<T, 1, 4>::vl[2] = z;
            array<T, 1, 4>::vl[3] = w;
            return *this;
        }

        T& operator[](unsigned int i)
        {
            return (i == 0 ? x : i == 1 ? y : i == 2 ? z : w);
        }

        const T& operator[](unsigned int i) const
        {
            return (i == 0 ? x : i == 1 ? y : i == 2 ? z : w);
        }
    };


    /* Vectors */

    template<typename T, int rows>
    class vector : public array<T, 1, rows>
    {
    public:
        using array<T, 1, rows>::vl;

        /* Constructors, Destructor */

        vector() {}
        vector(const array<T, 1, rows>& v) : array<T, 1, rows>(v) {}
        vector(const swizzler<T, rows>& v) : array<T, 1, rows>(v) {}
        explicit vector(T x) : array<T, 1, rows>(x) {}
        explicit vector(T x, T y) : array<T, 1, rows>(x, y) {}
        explicit vector(const vector<T, 2>& xy) : array<T, 1, rows>(xy.vl[0], xy.vl[1]) {}
        explicit vector(T x, T y, T z) : array<T, 1, rows>(x, y, z) {}
        explicit vector(const vector<T, 3>& xyz) : array<T, 1, rows>(xyz.vl[0], xyz.vl[1], xyz.vl[2]) {}
        explicit vector(const vector<T, 2>& xy, T z) : array<T, 1, rows>(xy.vl[0], xy.vl[1], z) {}
        explicit vector(T x, const vector<T, 2>& yz) : array<T, 1, rows>(x, yz.vl[0], yz.vl[1]) {}
        explicit vector(T x, T y, T z, T w) : array<T, 1, rows>(x, y, z, w) {}
        explicit vector(const vector<T, 4>& xyzw) : array<T, 1, rows>(xyzw.vl[0], xyzw.vl[1], xyzw.vl[2], xyzw.vl[3]) {}
        explicit vector(const vector<T, 2> xy, T z, T w) : array<T, 1, rows>(xy.vl[0], xy.vl[1], z, w) {}
        explicit vector(const vector<T, 2> xy, const vector<T, 2> zw) : array<T, 1, rows>(xy.vl[0], xy.vl[1], zw.vl[0], zw.vl[1]) {}
        explicit vector(T x, const vector<T, 2> yz, T w) : array<T, 1, rows>(x, yz.vl[0], yz.vl[1], w) {}
        explicit vector(T x, T y, const vector<T, 2> zw) : array<T, 1, rows>(x, y, zw.vl[0], zw.vl[1]) {}
        explicit vector(const vector<T, 3> xyz, T w) : array<T, 1, rows>(xyz.vl[0], xyz.vl[1], xyz.vl[2], w) {}
        explicit vector(T x, const vector<T, 3> yzw) : array<T, 1, rows>(x, yzw.vl[0], yzw.vl[1], yzw.vl[2]) {}
        explicit vector(const T* v) : array<T, 1, rows>(v) {}
        template<typename U> explicit vector(const vector<U, rows>& v) : array<T, 1, rows>(v) {}

        /* Operators */

        T& operator[](unsigned int i)
        {
            return vl[i];
        }

        const T& operator[](unsigned int i) const
        {
            return vl[i];
        }

        const vector& operator=(const array<T, 1, rows>& a) { this->_op_assign(a); return *this; }
        const vector& operator+=(const array<T, 1, rows>& a) { this->_op_plus_assign(a); return *this; }
        const vector& operator-=(const array<T, 1, rows>& a) { this->_op_minus_assign(a); return *this; }
        const vector& operator*=(const array<T, 1, rows>& a) { this->_op_comp_mult_assign(a); return *this; }
        const vector& operator*=(T s) { this->_op_scal_mult_assign(s); return *this; }
        const vector& operator/=(const array<T, 1, rows>& a) { this->_op_comp_div_assign(a); return *this; }
        const vector& operator/=(T s) { this->_op_scal_div_assign(s); return *this; }
        const vector& operator%=(const array<T, 1, rows>& a) { this->_op_comp_mod_assign(a); return *this; }
        const vector& operator%=(T s) { this->_op_scal_mod_assign(s); return *this; }

        /* Swizzling */

        vector<T, 2> xx() const { return vector<T, 2>(vl[0], vl[0]); }
        vector<T, 2> rr() const { return vector<T, 2>(vl[0], vl[0]); }
        vector<T, 2> ss() const { return vector<T, 2>(vl[0], vl[0]); }
        vector<T, 2> xy() const { return vector<T, 2>(vl[0], vl[1]); }
        vector<T, 2> rg() const { return vector<T, 2>(vl[0], vl[1]); }
        vector<T, 2> st() const { return vector<T, 2>(vl[0], vl[1]); }
        vector<T, 2> xz() const { return vector<T, 2>(vl[0], vl[2]); }
        vector<T, 2> rb() const { return vector<T, 2>(vl[0], vl[2]); }
        vector<T, 2> sp() const { return vector<T, 2>(vl[0], vl[2]); }
        vector<T, 2> xw() const { return vector<T, 2>(vl[0], vl[3]); }
        vector<T, 2> ra() const { return vector<T, 2>(vl[0], vl[3]); }
        vector<T, 2> sq() const { return vector<T, 2>(vl[0], vl[3]); }
        vector<T, 2> yx() const { return vector<T, 2>(vl[1], vl[0]); }
        vector<T, 2> gr() const { return vector<T, 2>(vl[1], vl[0]); }
        vector<T, 2> ts() const { return vector<T, 2>(vl[1], vl[0]); }
        vector<T, 2> yy() const { return vector<T, 2>(vl[1], vl[1]); }
        vector<T, 2> gg() const { return vector<T, 2>(vl[1], vl[1]); }
        vector<T, 2> tt() const { return vector<T, 2>(vl[1], vl[1]); }
        vector<T, 2> yz() const { return vector<T, 2>(vl[1], vl[2]); }
        vector<T, 2> gb() const { return vector<T, 2>(vl[1], vl[2]); }
        vector<T, 2> tp() const { return vector<T, 2>(vl[1], vl[2]); }
        vector<T, 2> yw() const { return vector<T, 2>(vl[1], vl[3]); }
        vector<T, 2> ga() const { return vector<T, 2>(vl[1], vl[3]); }
        vector<T, 2> tq() const { return vector<T, 2>(vl[1], vl[3]); }
        vector<T, 2> zx() const { return vector<T, 2>(vl[2], vl[0]); }
        vector<T, 2> br() const { return vector<T, 2>(vl[2], vl[0]); }
        vector<T, 2> ps() const { return vector<T, 2>(vl[2], vl[0]); }
        vector<T, 2> zy() const { return vector<T, 2>(vl[2], vl[1]); }
        vector<T, 2> bg() const { return vector<T, 2>(vl[2], vl[1]); }
        vector<T, 2> pt() const { return vector<T, 2>(vl[2], vl[1]); }
        vector<T, 2> zz() const { return vector<T, 2>(vl[2], vl[2]); }
        vector<T, 2> bb() const { return vector<T, 2>(vl[2], vl[2]); }
        vector<T, 2> pp() const { return vector<T, 2>(vl[2], vl[2]); }
        vector<T, 2> zw() const { return vector<T, 2>(vl[2], vl[3]); }
        vector<T, 2> ba() const { return vector<T, 2>(vl[2], vl[3]); }
        vector<T, 2> pq() const { return vector<T, 2>(vl[2], vl[3]); }
        vector<T, 2> wx() const { return vector<T, 2>(vl[3], vl[0]); }
        vector<T, 2> ar() const { return vector<T, 2>(vl[3], vl[0]); }
        vector<T, 2> qs() const { return vector<T, 2>(vl[3], vl[0]); }
        vector<T, 2> wy() const { return vector<T, 2>(vl[3], vl[1]); }
        vector<T, 2> ag() const { return vector<T, 2>(vl[3], vl[1]); }
        vector<T, 2> qt() const { return vector<T, 2>(vl[3], vl[1]); }
        vector<T, 2> wz() const { return vector<T, 2>(vl[3], vl[2]); }
        vector<T, 2> ab() const { return vector<T, 2>(vl[3], vl[2]); }
        vector<T, 2> qp() const { return vector<T, 2>(vl[3], vl[2]); }
        vector<T, 2> ww() const { return vector<T, 2>(vl[3], vl[3]); }
        vector<T, 2> aa() const { return vector<T, 2>(vl[3], vl[3]); }
        vector<T, 2> qq() const { return vector<T, 2>(vl[3], vl[3]); }
        vector<T, 3> xxx() const { return vector<T, 3>(vl[0], vl[0], vl[0]); }
        vector<T, 3> rrr() const { return vector<T, 3>(vl[0], vl[0], vl[0]); }
        vector<T, 3> sss() const { return vector<T, 3>(vl[0], vl[0], vl[0]); }
        vector<T, 3> xxy() const { return vector<T, 3>(vl[0], vl[0], vl[1]); }
        vector<T, 3> rrg() const { return vector<T, 3>(vl[0], vl[0], vl[1]); }
        vector<T, 3> sst() const { return vector<T, 3>(vl[0], vl[0], vl[1]); }
        vector<T, 3> xxz() const { return vector<T, 3>(vl[0], vl[0], vl[2]); }
        vector<T, 3> rrb() const { return vector<T, 3>(vl[0], vl[0], vl[2]); }
        vector<T, 3> ssp() const { return vector<T, 3>(vl[0], vl[0], vl[2]); }
        vector<T, 3> xxw() const { return vector<T, 3>(vl[0], vl[0], vl[3]); }
        vector<T, 3> rra() const { return vector<T, 3>(vl[0], vl[0], vl[3]); }
        vector<T, 3> ssq() const { return vector<T, 3>(vl[0], vl[0], vl[3]); }
        vector<T, 3> xyx() const { return vector<T, 3>(vl[0], vl[1], vl[0]); }
        vector<T, 3> rgr() const { return vector<T, 3>(vl[0], vl[1], vl[0]); }
        vector<T, 3> sts() const { return vector<T, 3>(vl[0], vl[1], vl[0]); }
        vector<T, 3> xyy() const { return vector<T, 3>(vl[0], vl[1], vl[1]); }
        vector<T, 3> rgg() const { return vector<T, 3>(vl[0], vl[1], vl[1]); }
        vector<T, 3> stt() const { return vector<T, 3>(vl[0], vl[1], vl[1]); }
        vector<T, 3> xyz() const { return vector<T, 3>(vl[0], vl[1], vl[2]); }
        vector<T, 3> rgb() const { return vector<T, 3>(vl[0], vl[1], vl[2]); }
        vector<T, 3> stp() const { return vector<T, 3>(vl[0], vl[1], vl[2]); }
        vector<T, 3> xyw() const { return vector<T, 3>(vl[0], vl[1], vl[3]); }
        vector<T, 3> rga() const { return vector<T, 3>(vl[0], vl[1], vl[3]); }
        vector<T, 3> stq() const { return vector<T, 3>(vl[0], vl[1], vl[3]); }
        vector<T, 3> xzx() const { return vector<T, 3>(vl[0], vl[2], vl[0]); }
        vector<T, 3> rbr() const { return vector<T, 3>(vl[0], vl[2], vl[0]); }
        vector<T, 3> sps() const { return vector<T, 3>(vl[0], vl[2], vl[0]); }
        vector<T, 3> xzy() const { return vector<T, 3>(vl[0], vl[2], vl[1]); }
        vector<T, 3> rbg() const { return vector<T, 3>(vl[0], vl[2], vl[1]); }
        vector<T, 3> spt() const { return vector<T, 3>(vl[0], vl[2], vl[1]); }
        vector<T, 3> xzz() const { return vector<T, 3>(vl[0], vl[2], vl[2]); }
        vector<T, 3> rbb() const { return vector<T, 3>(vl[0], vl[2], vl[2]); }
        vector<T, 3> spp() const { return vector<T, 3>(vl[0], vl[2], vl[2]); }
        vector<T, 3> xzw() const { return vector<T, 3>(vl[0], vl[2], vl[3]); }
        vector<T, 3> rba() const { return vector<T, 3>(vl[0], vl[2], vl[3]); }
        vector<T, 3> spq() const { return vector<T, 3>(vl[0], vl[2], vl[3]); }
        vector<T, 3> xwx() const { return vector<T, 3>(vl[0], vl[3], vl[0]); }
        vector<T, 3> rar() const { return vector<T, 3>(vl[0], vl[3], vl[0]); }
        vector<T, 3> sqs() const { return vector<T, 3>(vl[0], vl[3], vl[0]); }
        vector<T, 3> xwy() const { return vector<T, 3>(vl[0], vl[3], vl[1]); }
        vector<T, 3> rag() const { return vector<T, 3>(vl[0], vl[3], vl[1]); }
        vector<T, 3> sqt() const { return vector<T, 3>(vl[0], vl[3], vl[1]); }
        vector<T, 3> xwz() const { return vector<T, 3>(vl[0], vl[3], vl[2]); }
        vector<T, 3> rab() const { return vector<T, 3>(vl[0], vl[3], vl[2]); }
        vector<T, 3> sqp() const { return vector<T, 3>(vl[0], vl[3], vl[2]); }
        vector<T, 3> xww() const { return vector<T, 3>(vl[0], vl[3], vl[3]); }
        vector<T, 3> raa() const { return vector<T, 3>(vl[0], vl[3], vl[3]); }
        vector<T, 3> sqq() const { return vector<T, 3>(vl[0], vl[3], vl[3]); }
        vector<T, 3> yxx() const { return vector<T, 3>(vl[1], vl[0], vl[0]); }
        vector<T, 3> grr() const { return vector<T, 3>(vl[1], vl[0], vl[0]); }
        vector<T, 3> tss() const { return vector<T, 3>(vl[1], vl[0], vl[0]); }
        vector<T, 3> yxy() const { return vector<T, 3>(vl[1], vl[0], vl[1]); }
        vector<T, 3> grg() const { return vector<T, 3>(vl[1], vl[0], vl[1]); }
        vector<T, 3> tst() const { return vector<T, 3>(vl[1], vl[0], vl[1]); }
        vector<T, 3> yxz() const { return vector<T, 3>(vl[1], vl[0], vl[2]); }
        vector<T, 3> grb() const { return vector<T, 3>(vl[1], vl[0], vl[2]); }
        vector<T, 3> tsp() const { return vector<T, 3>(vl[1], vl[0], vl[2]); }
        vector<T, 3> yxw() const { return vector<T, 3>(vl[1], vl[0], vl[3]); }
        vector<T, 3> gra() const { return vector<T, 3>(vl[1], vl[0], vl[3]); }
        vector<T, 3> tsq() const { return vector<T, 3>(vl[1], vl[0], vl[3]); }
        vector<T, 3> yyx() const { return vector<T, 3>(vl[1], vl[1], vl[0]); }
        vector<T, 3> ggr() const { return vector<T, 3>(vl[1], vl[1], vl[0]); }
        vector<T, 3> tts() const { return vector<T, 3>(vl[1], vl[1], vl[0]); }
        vector<T, 3> yyy() const { return vector<T, 3>(vl[1], vl[1], vl[1]); }
        vector<T, 3> ggg() const { return vector<T, 3>(vl[1], vl[1], vl[1]); }
        vector<T, 3> ttt() const { return vector<T, 3>(vl[1], vl[1], vl[1]); }
        vector<T, 3> yyz() const { return vector<T, 3>(vl[1], vl[1], vl[2]); }
        vector<T, 3> ggb() const { return vector<T, 3>(vl[1], vl[1], vl[2]); }
        vector<T, 3> ttp() const { return vector<T, 3>(vl[1], vl[1], vl[2]); }
        vector<T, 3> yyw() const { return vector<T, 3>(vl[1], vl[1], vl[3]); }
        vector<T, 3> gga() const { return vector<T, 3>(vl[1], vl[1], vl[3]); }
        vector<T, 3> ttq() const { return vector<T, 3>(vl[1], vl[1], vl[3]); }
        vector<T, 3> yzx() const { return vector<T, 3>(vl[1], vl[2], vl[0]); }
        vector<T, 3> gbr() const { return vector<T, 3>(vl[1], vl[2], vl[0]); }
        vector<T, 3> tps() const { return vector<T, 3>(vl[1], vl[2], vl[0]); }
        vector<T, 3> yzy() const { return vector<T, 3>(vl[1], vl[2], vl[1]); }
        vector<T, 3> gbg() const { return vector<T, 3>(vl[1], vl[2], vl[1]); }
        vector<T, 3> tpt() const { return vector<T, 3>(vl[1], vl[2], vl[1]); }
        vector<T, 3> yzz() const { return vector<T, 3>(vl[1], vl[2], vl[2]); }
        vector<T, 3> gbb() const { return vector<T, 3>(vl[1], vl[2], vl[2]); }
        vector<T, 3> tpp() const { return vector<T, 3>(vl[1], vl[2], vl[2]); }
        vector<T, 3> yzw() const { return vector<T, 3>(vl[1], vl[2], vl[3]); }
        vector<T, 3> gba() const { return vector<T, 3>(vl[1], vl[2], vl[3]); }
        vector<T, 3> tpq() const { return vector<T, 3>(vl[1], vl[2], vl[3]); }
        vector<T, 3> ywx() const { return vector<T, 3>(vl[1], vl[3], vl[0]); }
        vector<T, 3> gar() const { return vector<T, 3>(vl[1], vl[3], vl[0]); }
        vector<T, 3> tqs() const { return vector<T, 3>(vl[1], vl[3], vl[0]); }
        vector<T, 3> ywy() const { return vector<T, 3>(vl[1], vl[3], vl[1]); }
        vector<T, 3> gag() const { return vector<T, 3>(vl[1], vl[3], vl[1]); }
        vector<T, 3> tqt() const { return vector<T, 3>(vl[1], vl[3], vl[1]); }
        vector<T, 3> ywz() const { return vector<T, 3>(vl[1], vl[3], vl[2]); }
        vector<T, 3> gab() const { return vector<T, 3>(vl[1], vl[3], vl[2]); }
        vector<T, 3> tqp() const { return vector<T, 3>(vl[1], vl[3], vl[2]); }
        vector<T, 3> yww() const { return vector<T, 3>(vl[1], vl[3], vl[3]); }
        vector<T, 3> gaa() const { return vector<T, 3>(vl[1], vl[3], vl[3]); }
        vector<T, 3> tqq() const { return vector<T, 3>(vl[1], vl[3], vl[3]); }
        vector<T, 3> zxx() const { return vector<T, 3>(vl[2], vl[0], vl[0]); }
        vector<T, 3> brr() const { return vector<T, 3>(vl[2], vl[0], vl[0]); }
        vector<T, 3> pss() const { return vector<T, 3>(vl[2], vl[0], vl[0]); }
        vector<T, 3> zxy() const { return vector<T, 3>(vl[2], vl[0], vl[1]); }
        vector<T, 3> brg() const { return vector<T, 3>(vl[2], vl[0], vl[1]); }
        vector<T, 3> pst() const { return vector<T, 3>(vl[2], vl[0], vl[1]); }
        vector<T, 3> zxz() const { return vector<T, 3>(vl[2], vl[0], vl[2]); }
        vector<T, 3> brb() const { return vector<T, 3>(vl[2], vl[0], vl[2]); }
        vector<T, 3> psp() const { return vector<T, 3>(vl[2], vl[0], vl[2]); }
        vector<T, 3> zxw() const { return vector<T, 3>(vl[2], vl[0], vl[3]); }
        vector<T, 3> bra() const { return vector<T, 3>(vl[2], vl[0], vl[3]); }
        vector<T, 3> psq() const { return vector<T, 3>(vl[2], vl[0], vl[3]); }
        vector<T, 3> zyx() const { return vector<T, 3>(vl[2], vl[1], vl[0]); }
        vector<T, 3> bgr() const { return vector<T, 3>(vl[2], vl[1], vl[0]); }
        vector<T, 3> pts() const { return vector<T, 3>(vl[2], vl[1], vl[0]); }
        vector<T, 3> zyy() const { return vector<T, 3>(vl[2], vl[1], vl[1]); }
        vector<T, 3> bgg() const { return vector<T, 3>(vl[2], vl[1], vl[1]); }
        vector<T, 3> ptt() const { return vector<T, 3>(vl[2], vl[1], vl[1]); }
        vector<T, 3> zyz() const { return vector<T, 3>(vl[2], vl[1], vl[2]); }
        vector<T, 3> bgb() const { return vector<T, 3>(vl[2], vl[1], vl[2]); }
        vector<T, 3> ptp() const { return vector<T, 3>(vl[2], vl[1], vl[2]); }
        vector<T, 3> zyw() const { return vector<T, 3>(vl[2], vl[1], vl[3]); }
        vector<T, 3> bga() const { return vector<T, 3>(vl[2], vl[1], vl[3]); }
        vector<T, 3> ptq() const { return vector<T, 3>(vl[2], vl[1], vl[3]); }
        vector<T, 3> zzx() const { return vector<T, 3>(vl[2], vl[2], vl[0]); }
        vector<T, 3> bbr() const { return vector<T, 3>(vl[2], vl[2], vl[0]); }
        vector<T, 3> pps() const { return vector<T, 3>(vl[2], vl[2], vl[0]); }
        vector<T, 3> zzy() const { return vector<T, 3>(vl[2], vl[2], vl[1]); }
        vector<T, 3> bbg() const { return vector<T, 3>(vl[2], vl[2], vl[1]); }
        vector<T, 3> ppt() const { return vector<T, 3>(vl[2], vl[2], vl[1]); }
        vector<T, 3> zzz() const { return vector<T, 3>(vl[2], vl[2], vl[2]); }
        vector<T, 3> bbb() const { return vector<T, 3>(vl[2], vl[2], vl[2]); }
        vector<T, 3> ppp() const { return vector<T, 3>(vl[2], vl[2], vl[2]); }
        vector<T, 3> zzw() const { return vector<T, 3>(vl[2], vl[2], vl[3]); }
        vector<T, 3> bba() const { return vector<T, 3>(vl[2], vl[2], vl[3]); }
        vector<T, 3> ppq() const { return vector<T, 3>(vl[2], vl[2], vl[3]); }
        vector<T, 3> zwx() const { return vector<T, 3>(vl[2], vl[3], vl[0]); }
        vector<T, 3> bar() const { return vector<T, 3>(vl[2], vl[3], vl[0]); }
        vector<T, 3> pqs() const { return vector<T, 3>(vl[2], vl[3], vl[0]); }
        vector<T, 3> zwy() const { return vector<T, 3>(vl[2], vl[3], vl[1]); }
        vector<T, 3> bag() const { return vector<T, 3>(vl[2], vl[3], vl[1]); }
        vector<T, 3> pqt() const { return vector<T, 3>(vl[2], vl[3], vl[1]); }
        vector<T, 3> zwz() const { return vector<T, 3>(vl[2], vl[3], vl[2]); }
        vector<T, 3> bab() const { return vector<T, 3>(vl[2], vl[3], vl[2]); }
        vector<T, 3> pqp() const { return vector<T, 3>(vl[2], vl[3], vl[2]); }
        vector<T, 3> zww() const { return vector<T, 3>(vl[2], vl[3], vl[3]); }
        vector<T, 3> baa() const { return vector<T, 3>(vl[2], vl[3], vl[3]); }
        vector<T, 3> pqq() const { return vector<T, 3>(vl[2], vl[3], vl[3]); }
        vector<T, 3> wxx() const { return vector<T, 3>(vl[3], vl[0], vl[0]); }
        vector<T, 3> arr() const { return vector<T, 3>(vl[3], vl[0], vl[0]); }
        vector<T, 3> qss() const { return vector<T, 3>(vl[3], vl[0], vl[0]); }
        vector<T, 3> wxy() const { return vector<T, 3>(vl[3], vl[0], vl[1]); }
        vector<T, 3> arg() const { return vector<T, 3>(vl[3], vl[0], vl[1]); }
        vector<T, 3> qst() const { return vector<T, 3>(vl[3], vl[0], vl[1]); }
        vector<T, 3> wxz() const { return vector<T, 3>(vl[3], vl[0], vl[2]); }
        vector<T, 3> arb() const { return vector<T, 3>(vl[3], vl[0], vl[2]); }
        vector<T, 3> qsp() const { return vector<T, 3>(vl[3], vl[0], vl[2]); }
        vector<T, 3> wxw() const { return vector<T, 3>(vl[3], vl[0], vl[3]); }
        vector<T, 3> ara() const { return vector<T, 3>(vl[3], vl[0], vl[3]); }
        vector<T, 3> qsq() const { return vector<T, 3>(vl[3], vl[0], vl[3]); }
        vector<T, 3> wyx() const { return vector<T, 3>(vl[3], vl[1], vl[0]); }
        vector<T, 3> agr() const { return vector<T, 3>(vl[3], vl[1], vl[0]); }
        vector<T, 3> qts() const { return vector<T, 3>(vl[3], vl[1], vl[0]); }
        vector<T, 3> wyy() const { return vector<T, 3>(vl[3], vl[1], vl[1]); }
        vector<T, 3> agg() const { return vector<T, 3>(vl[3], vl[1], vl[1]); }
        vector<T, 3> qtt() const { return vector<T, 3>(vl[3], vl[1], vl[1]); }
        vector<T, 3> wyz() const { return vector<T, 3>(vl[3], vl[1], vl[2]); }
        vector<T, 3> agb() const { return vector<T, 3>(vl[3], vl[1], vl[2]); }
        vector<T, 3> qtp() const { return vector<T, 3>(vl[3], vl[1], vl[2]); }
        vector<T, 3> wyw() const { return vector<T, 3>(vl[3], vl[1], vl[3]); }
        vector<T, 3> aga() const { return vector<T, 3>(vl[3], vl[1], vl[3]); }
        vector<T, 3> qtq() const { return vector<T, 3>(vl[3], vl[1], vl[3]); }
        vector<T, 3> wzx() const { return vector<T, 3>(vl[3], vl[2], vl[0]); }
        vector<T, 3> abr() const { return vector<T, 3>(vl[3], vl[2], vl[0]); }
        vector<T, 3> qps() const { return vector<T, 3>(vl[3], vl[2], vl[0]); }
        vector<T, 3> wzy() const { return vector<T, 3>(vl[3], vl[2], vl[1]); }
        vector<T, 3> abg() const { return vector<T, 3>(vl[3], vl[2], vl[1]); }
        vector<T, 3> qpt() const { return vector<T, 3>(vl[3], vl[2], vl[1]); }
        vector<T, 3> wzz() const { return vector<T, 3>(vl[3], vl[2], vl[2]); }
        vector<T, 3> abb() const { return vector<T, 3>(vl[3], vl[2], vl[2]); }
        vector<T, 3> qpp() const { return vector<T, 3>(vl[3], vl[2], vl[2]); }
        vector<T, 3> wzw() const { return vector<T, 3>(vl[3], vl[2], vl[3]); }
        vector<T, 3> aba() const { return vector<T, 3>(vl[3], vl[2], vl[3]); }
        vector<T, 3> qpq() const { return vector<T, 3>(vl[3], vl[2], vl[3]); }
        vector<T, 3> wwx() const { return vector<T, 3>(vl[3], vl[3], vl[0]); }
        vector<T, 3> aar() const { return vector<T, 3>(vl[3], vl[3], vl[0]); }
        vector<T, 3> qqs() const { return vector<T, 3>(vl[3], vl[3], vl[0]); }
        vector<T, 3> wwy() const { return vector<T, 3>(vl[3], vl[3], vl[1]); }
        vector<T, 3> aag() const { return vector<T, 3>(vl[3], vl[3], vl[1]); }
        vector<T, 3> qqt() const { return vector<T, 3>(vl[3], vl[3], vl[1]); }
        vector<T, 3> wwz() const { return vector<T, 3>(vl[3], vl[3], vl[2]); }
        vector<T, 3> aab() const { return vector<T, 3>(vl[3], vl[3], vl[2]); }
        vector<T, 3> qqp() const { return vector<T, 3>(vl[3], vl[3], vl[2]); }
        vector<T, 3> www() const { return vector<T, 3>(vl[3], vl[3], vl[3]); }
        vector<T, 3> aaa() const { return vector<T, 3>(vl[3], vl[3], vl[3]); }
        vector<T, 3> qqq() const { return vector<T, 3>(vl[3], vl[3], vl[3]); }
        vector<T, 4> xxxx() const { return vector<T, 4>(vl[0], vl[0], vl[0], vl[0]); }
        vector<T, 4> rrrr() const { return vector<T, 4>(vl[0], vl[0], vl[0], vl[0]); }
        vector<T, 4> ssss() const { return vector<T, 4>(vl[0], vl[0], vl[0], vl[0]); }
        vector<T, 4> xxxy() const { return vector<T, 4>(vl[0], vl[0], vl[0], vl[1]); }
        vector<T, 4> rrrg() const { return vector<T, 4>(vl[0], vl[0], vl[0], vl[1]); }
        vector<T, 4> ssst() const { return vector<T, 4>(vl[0], vl[0], vl[0], vl[1]); }
        vector<T, 4> xxxz() const { return vector<T, 4>(vl[0], vl[0], vl[0], vl[2]); }
        vector<T, 4> rrrb() const { return vector<T, 4>(vl[0], vl[0], vl[0], vl[2]); }
        vector<T, 4> sssp() const { return vector<T, 4>(vl[0], vl[0], vl[0], vl[2]); }
        vector<T, 4> xxxw() const { return vector<T, 4>(vl[0], vl[0], vl[0], vl[3]); }
        vector<T, 4> rrra() const { return vector<T, 4>(vl[0], vl[0], vl[0], vl[3]); }
        vector<T, 4> sssq() const { return vector<T, 4>(vl[0], vl[0], vl[0], vl[3]); }
        vector<T, 4> xxyx() const { return vector<T, 4>(vl[0], vl[0], vl[1], vl[0]); }
        vector<T, 4> rrgr() const { return vector<T, 4>(vl[0], vl[0], vl[1], vl[0]); }
        vector<T, 4> ssts() const { return vector<T, 4>(vl[0], vl[0], vl[1], vl[0]); }
        vector<T, 4> xxyy() const { return vector<T, 4>(vl[0], vl[0], vl[1], vl[1]); }
        vector<T, 4> rrgg() const { return vector<T, 4>(vl[0], vl[0], vl[1], vl[1]); }
        vector<T, 4> sstt() const { return vector<T, 4>(vl[0], vl[0], vl[1], vl[1]); }
        vector<T, 4> xxyz() const { return vector<T, 4>(vl[0], vl[0], vl[1], vl[2]); }
        vector<T, 4> rrgb() const { return vector<T, 4>(vl[0], vl[0], vl[1], vl[2]); }
        vector<T, 4> sstp() const { return vector<T, 4>(vl[0], vl[0], vl[1], vl[2]); }
        vector<T, 4> xxyw() const { return vector<T, 4>(vl[0], vl[0], vl[1], vl[3]); }
        vector<T, 4> rrga() const { return vector<T, 4>(vl[0], vl[0], vl[1], vl[3]); }
        vector<T, 4> sstq() const { return vector<T, 4>(vl[0], vl[0], vl[1], vl[3]); }
        vector<T, 4> xxzx() const { return vector<T, 4>(vl[0], vl[0], vl[2], vl[0]); }
        vector<T, 4> rrbr() const { return vector<T, 4>(vl[0], vl[0], vl[2], vl[0]); }
        vector<T, 4> ssps() const { return vector<T, 4>(vl[0], vl[0], vl[2], vl[0]); }
        vector<T, 4> xxzy() const { return vector<T, 4>(vl[0], vl[0], vl[2], vl[1]); }
        vector<T, 4> rrbg() const { return vector<T, 4>(vl[0], vl[0], vl[2], vl[1]); }
        vector<T, 4> sspt() const { return vector<T, 4>(vl[0], vl[0], vl[2], vl[1]); }
        vector<T, 4> xxzz() const { return vector<T, 4>(vl[0], vl[0], vl[2], vl[2]); }
        vector<T, 4> rrbb() const { return vector<T, 4>(vl[0], vl[0], vl[2], vl[2]); }
        vector<T, 4> sspp() const { return vector<T, 4>(vl[0], vl[0], vl[2], vl[2]); }
        vector<T, 4> xxzw() const { return vector<T, 4>(vl[0], vl[0], vl[2], vl[3]); }
        vector<T, 4> rrba() const { return vector<T, 4>(vl[0], vl[0], vl[2], vl[3]); }
        vector<T, 4> sspq() const { return vector<T, 4>(vl[0], vl[0], vl[2], vl[3]); }
        vector<T, 4> xxwx() const { return vector<T, 4>(vl[0], vl[0], vl[3], vl[0]); }
        vector<T, 4> rrar() const { return vector<T, 4>(vl[0], vl[0], vl[3], vl[0]); }
        vector<T, 4> ssqs() const { return vector<T, 4>(vl[0], vl[0], vl[3], vl[0]); }
        vector<T, 4> xxwy() const { return vector<T, 4>(vl[0], vl[0], vl[3], vl[1]); }
        vector<T, 4> rrag() const { return vector<T, 4>(vl[0], vl[0], vl[3], vl[1]); }
        vector<T, 4> ssqt() const { return vector<T, 4>(vl[0], vl[0], vl[3], vl[1]); }
        vector<T, 4> xxwz() const { return vector<T, 4>(vl[0], vl[0], vl[3], vl[2]); }
        vector<T, 4> rrab() const { return vector<T, 4>(vl[0], vl[0], vl[3], vl[2]); }
        vector<T, 4> ssqp() const { return vector<T, 4>(vl[0], vl[0], vl[3], vl[2]); }
        vector<T, 4> xxww() const { return vector<T, 4>(vl[0], vl[0], vl[3], vl[3]); }
        vector<T, 4> rraa() const { return vector<T, 4>(vl[0], vl[0], vl[3], vl[3]); }
        vector<T, 4> ssqq() const { return vector<T, 4>(vl[0], vl[0], vl[3], vl[3]); }
        vector<T, 4> xyxx() const { return vector<T, 4>(vl[0], vl[1], vl[0], vl[0]); }
        vector<T, 4> rgrr() const { return vector<T, 4>(vl[0], vl[1], vl[0], vl[0]); }
        vector<T, 4> stss() const { return vector<T, 4>(vl[0], vl[1], vl[0], vl[0]); }
        vector<T, 4> xyxy() const { return vector<T, 4>(vl[0], vl[1], vl[0], vl[1]); }
        vector<T, 4> rgrg() const { return vector<T, 4>(vl[0], vl[1], vl[0], vl[1]); }
        vector<T, 4> stst() const { return vector<T, 4>(vl[0], vl[1], vl[0], vl[1]); }
        vector<T, 4> xyxz() const { return vector<T, 4>(vl[0], vl[1], vl[0], vl[2]); }
        vector<T, 4> rgrb() const { return vector<T, 4>(vl[0], vl[1], vl[0], vl[2]); }
        vector<T, 4> stsp() const { return vector<T, 4>(vl[0], vl[1], vl[0], vl[2]); }
        vector<T, 4> xyxw() const { return vector<T, 4>(vl[0], vl[1], vl[0], vl[3]); }
        vector<T, 4> rgra() const { return vector<T, 4>(vl[0], vl[1], vl[0], vl[3]); }
        vector<T, 4> stsq() const { return vector<T, 4>(vl[0], vl[1], vl[0], vl[3]); }
        vector<T, 4> xyyx() const { return vector<T, 4>(vl[0], vl[1], vl[1], vl[0]); }
        vector<T, 4> rggr() const { return vector<T, 4>(vl[0], vl[1], vl[1], vl[0]); }
        vector<T, 4> stts() const { return vector<T, 4>(vl[0], vl[1], vl[1], vl[0]); }
        vector<T, 4> xyyy() const { return vector<T, 4>(vl[0], vl[1], vl[1], vl[1]); }
        vector<T, 4> rggg() const { return vector<T, 4>(vl[0], vl[1], vl[1], vl[1]); }
        vector<T, 4> sttt() const { return vector<T, 4>(vl[0], vl[1], vl[1], vl[1]); }
        vector<T, 4> xyyz() const { return vector<T, 4>(vl[0], vl[1], vl[1], vl[2]); }
        vector<T, 4> rggb() const { return vector<T, 4>(vl[0], vl[1], vl[1], vl[2]); }
        vector<T, 4> sttp() const { return vector<T, 4>(vl[0], vl[1], vl[1], vl[2]); }
        vector<T, 4> xyyw() const { return vector<T, 4>(vl[0], vl[1], vl[1], vl[3]); }
        vector<T, 4> rgga() const { return vector<T, 4>(vl[0], vl[1], vl[1], vl[3]); }
        vector<T, 4> sttq() const { return vector<T, 4>(vl[0], vl[1], vl[1], vl[3]); }
        vector<T, 4> xyzx() const { return vector<T, 4>(vl[0], vl[1], vl[2], vl[0]); }
        vector<T, 4> rgbr() const { return vector<T, 4>(vl[0], vl[1], vl[2], vl[0]); }
        vector<T, 4> stps() const { return vector<T, 4>(vl[0], vl[1], vl[2], vl[0]); }
        vector<T, 4> xyzy() const { return vector<T, 4>(vl[0], vl[1], vl[2], vl[1]); }
        vector<T, 4> rgbg() const { return vector<T, 4>(vl[0], vl[1], vl[2], vl[1]); }
        vector<T, 4> stpt() const { return vector<T, 4>(vl[0], vl[1], vl[2], vl[1]); }
        vector<T, 4> xyzz() const { return vector<T, 4>(vl[0], vl[1], vl[2], vl[2]); }
        vector<T, 4> rgbb() const { return vector<T, 4>(vl[0], vl[1], vl[2], vl[2]); }
        vector<T, 4> stpp() const { return vector<T, 4>(vl[0], vl[1], vl[2], vl[2]); }
        vector<T, 4> xyzw() const { return vector<T, 4>(vl[0], vl[1], vl[2], vl[3]); }
        vector<T, 4> rgba() const { return vector<T, 4>(vl[0], vl[1], vl[2], vl[3]); }
        vector<T, 4> stpq() const { return vector<T, 4>(vl[0], vl[1], vl[2], vl[3]); }
        vector<T, 4> xywx() const { return vector<T, 4>(vl[0], vl[1], vl[3], vl[0]); }
        vector<T, 4> rgar() const { return vector<T, 4>(vl[0], vl[1], vl[3], vl[0]); }
        vector<T, 4> stqs() const { return vector<T, 4>(vl[0], vl[1], vl[3], vl[0]); }
        vector<T, 4> xywy() const { return vector<T, 4>(vl[0], vl[1], vl[3], vl[1]); }
        vector<T, 4> rgag() const { return vector<T, 4>(vl[0], vl[1], vl[3], vl[1]); }
        vector<T, 4> stqt() const { return vector<T, 4>(vl[0], vl[1], vl[3], vl[1]); }
        vector<T, 4> xywz() const { return vector<T, 4>(vl[0], vl[1], vl[3], vl[2]); }
        vector<T, 4> rgab() const { return vector<T, 4>(vl[0], vl[1], vl[3], vl[2]); }
        vector<T, 4> stqp() const { return vector<T, 4>(vl[0], vl[1], vl[3], vl[2]); }
        vector<T, 4> xyww() const { return vector<T, 4>(vl[0], vl[1], vl[3], vl[3]); }
        vector<T, 4> rgaa() const { return vector<T, 4>(vl[0], vl[1], vl[3], vl[3]); }
        vector<T, 4> stqq() const { return vector<T, 4>(vl[0], vl[1], vl[3], vl[3]); }
        vector<T, 4> xzxx() const { return vector<T, 4>(vl[0], vl[2], vl[0], vl[0]); }
        vector<T, 4> rbrr() const { return vector<T, 4>(vl[0], vl[2], vl[0], vl[0]); }
        vector<T, 4> spss() const { return vector<T, 4>(vl[0], vl[2], vl[0], vl[0]); }
        vector<T, 4> xzxy() const { return vector<T, 4>(vl[0], vl[2], vl[0], vl[1]); }
        vector<T, 4> rbrg() const { return vector<T, 4>(vl[0], vl[2], vl[0], vl[1]); }
        vector<T, 4> spst() const { return vector<T, 4>(vl[0], vl[2], vl[0], vl[1]); }
        vector<T, 4> xzxz() const { return vector<T, 4>(vl[0], vl[2], vl[0], vl[2]); }
        vector<T, 4> rbrb() const { return vector<T, 4>(vl[0], vl[2], vl[0], vl[2]); }
        vector<T, 4> spsp() const { return vector<T, 4>(vl[0], vl[2], vl[0], vl[2]); }
        vector<T, 4> xzxw() const { return vector<T, 4>(vl[0], vl[2], vl[0], vl[3]); }
        vector<T, 4> rbra() const { return vector<T, 4>(vl[0], vl[2], vl[0], vl[3]); }
        vector<T, 4> spsq() const { return vector<T, 4>(vl[0], vl[2], vl[0], vl[3]); }
        vector<T, 4> xzyx() const { return vector<T, 4>(vl[0], vl[2], vl[1], vl[0]); }
        vector<T, 4> rbgr() const { return vector<T, 4>(vl[0], vl[2], vl[1], vl[0]); }
        vector<T, 4> spts() const { return vector<T, 4>(vl[0], vl[2], vl[1], vl[0]); }
        vector<T, 4> xzyy() const { return vector<T, 4>(vl[0], vl[2], vl[1], vl[1]); }
        vector<T, 4> rbgg() const { return vector<T, 4>(vl[0], vl[2], vl[1], vl[1]); }
        vector<T, 4> sptt() const { return vector<T, 4>(vl[0], vl[2], vl[1], vl[1]); }
        vector<T, 4> xzyz() const { return vector<T, 4>(vl[0], vl[2], vl[1], vl[2]); }
        vector<T, 4> rbgb() const { return vector<T, 4>(vl[0], vl[2], vl[1], vl[2]); }
        vector<T, 4> sptp() const { return vector<T, 4>(vl[0], vl[2], vl[1], vl[2]); }
        vector<T, 4> xzyw() const { return vector<T, 4>(vl[0], vl[2], vl[1], vl[3]); }
        vector<T, 4> rbga() const { return vector<T, 4>(vl[0], vl[2], vl[1], vl[3]); }
        vector<T, 4> sptq() const { return vector<T, 4>(vl[0], vl[2], vl[1], vl[3]); }
        vector<T, 4> xzzx() const { return vector<T, 4>(vl[0], vl[2], vl[2], vl[0]); }
        vector<T, 4> rbbr() const { return vector<T, 4>(vl[0], vl[2], vl[2], vl[0]); }
        vector<T, 4> spps() const { return vector<T, 4>(vl[0], vl[2], vl[2], vl[0]); }
        vector<T, 4> xzzy() const { return vector<T, 4>(vl[0], vl[2], vl[2], vl[1]); }
        vector<T, 4> rbbg() const { return vector<T, 4>(vl[0], vl[2], vl[2], vl[1]); }
        vector<T, 4> sppt() const { return vector<T, 4>(vl[0], vl[2], vl[2], vl[1]); }
        vector<T, 4> xzzz() const { return vector<T, 4>(vl[0], vl[2], vl[2], vl[2]); }
        vector<T, 4> rbbb() const { return vector<T, 4>(vl[0], vl[2], vl[2], vl[2]); }
        vector<T, 4> sppp() const { return vector<T, 4>(vl[0], vl[2], vl[2], vl[2]); }
        vector<T, 4> xzzw() const { return vector<T, 4>(vl[0], vl[2], vl[2], vl[3]); }
        vector<T, 4> rbba() const { return vector<T, 4>(vl[0], vl[2], vl[2], vl[3]); }
        vector<T, 4> sppq() const { return vector<T, 4>(vl[0], vl[2], vl[2], vl[3]); }
        vector<T, 4> xzwx() const { return vector<T, 4>(vl[0], vl[2], vl[3], vl[0]); }
        vector<T, 4> rbar() const { return vector<T, 4>(vl[0], vl[2], vl[3], vl[0]); }
        vector<T, 4> spqs() const { return vector<T, 4>(vl[0], vl[2], vl[3], vl[0]); }
        vector<T, 4> xzwy() const { return vector<T, 4>(vl[0], vl[2], vl[3], vl[1]); }
        vector<T, 4> rbag() const { return vector<T, 4>(vl[0], vl[2], vl[3], vl[1]); }
        vector<T, 4> spqt() const { return vector<T, 4>(vl[0], vl[2], vl[3], vl[1]); }
        vector<T, 4> xzwz() const { return vector<T, 4>(vl[0], vl[2], vl[3], vl[2]); }
        vector<T, 4> rbab() const { return vector<T, 4>(vl[0], vl[2], vl[3], vl[2]); }
        vector<T, 4> spqp() const { return vector<T, 4>(vl[0], vl[2], vl[3], vl[2]); }
        vector<T, 4> xzww() const { return vector<T, 4>(vl[0], vl[2], vl[3], vl[3]); }
        vector<T, 4> rbaa() const { return vector<T, 4>(vl[0], vl[2], vl[3], vl[3]); }
        vector<T, 4> spqq() const { return vector<T, 4>(vl[0], vl[2], vl[3], vl[3]); }
        vector<T, 4> xwxx() const { return vector<T, 4>(vl[0], vl[3], vl[0], vl[0]); }
        vector<T, 4> rarr() const { return vector<T, 4>(vl[0], vl[3], vl[0], vl[0]); }
        vector<T, 4> sqss() const { return vector<T, 4>(vl[0], vl[3], vl[0], vl[0]); }
        vector<T, 4> xwxy() const { return vector<T, 4>(vl[0], vl[3], vl[0], vl[1]); }
        vector<T, 4> rarg() const { return vector<T, 4>(vl[0], vl[3], vl[0], vl[1]); }
        vector<T, 4> sqst() const { return vector<T, 4>(vl[0], vl[3], vl[0], vl[1]); }
        vector<T, 4> xwxz() const { return vector<T, 4>(vl[0], vl[3], vl[0], vl[2]); }
        vector<T, 4> rarb() const { return vector<T, 4>(vl[0], vl[3], vl[0], vl[2]); }
        vector<T, 4> sqsp() const { return vector<T, 4>(vl[0], vl[3], vl[0], vl[2]); }
        vector<T, 4> xwxw() const { return vector<T, 4>(vl[0], vl[3], vl[0], vl[3]); }
        vector<T, 4> rara() const { return vector<T, 4>(vl[0], vl[3], vl[0], vl[3]); }
        vector<T, 4> sqsq() const { return vector<T, 4>(vl[0], vl[3], vl[0], vl[3]); }
        vector<T, 4> xwyx() const { return vector<T, 4>(vl[0], vl[3], vl[1], vl[0]); }
        vector<T, 4> ragr() const { return vector<T, 4>(vl[0], vl[3], vl[1], vl[0]); }
        vector<T, 4> sqts() const { return vector<T, 4>(vl[0], vl[3], vl[1], vl[0]); }
        vector<T, 4> xwyy() const { return vector<T, 4>(vl[0], vl[3], vl[1], vl[1]); }
        vector<T, 4> ragg() const { return vector<T, 4>(vl[0], vl[3], vl[1], vl[1]); }
        vector<T, 4> sqtt() const { return vector<T, 4>(vl[0], vl[3], vl[1], vl[1]); }
        vector<T, 4> xwyz() const { return vector<T, 4>(vl[0], vl[3], vl[1], vl[2]); }
        vector<T, 4> ragb() const { return vector<T, 4>(vl[0], vl[3], vl[1], vl[2]); }
        vector<T, 4> sqtp() const { return vector<T, 4>(vl[0], vl[3], vl[1], vl[2]); }
        vector<T, 4> xwyw() const { return vector<T, 4>(vl[0], vl[3], vl[1], vl[3]); }
        vector<T, 4> raga() const { return vector<T, 4>(vl[0], vl[3], vl[1], vl[3]); }
        vector<T, 4> sqtq() const { return vector<T, 4>(vl[0], vl[3], vl[1], vl[3]); }
        vector<T, 4> xwzx() const { return vector<T, 4>(vl[0], vl[3], vl[2], vl[0]); }
        vector<T, 4> rabr() const { return vector<T, 4>(vl[0], vl[3], vl[2], vl[0]); }
        vector<T, 4> sqps() const { return vector<T, 4>(vl[0], vl[3], vl[2], vl[0]); }
        vector<T, 4> xwzy() const { return vector<T, 4>(vl[0], vl[3], vl[2], vl[1]); }
        vector<T, 4> rabg() const { return vector<T, 4>(vl[0], vl[3], vl[2], vl[1]); }
        vector<T, 4> sqpt() const { return vector<T, 4>(vl[0], vl[3], vl[2], vl[1]); }
        vector<T, 4> xwzz() const { return vector<T, 4>(vl[0], vl[3], vl[2], vl[2]); }
        vector<T, 4> rabb() const { return vector<T, 4>(vl[0], vl[3], vl[2], vl[2]); }
        vector<T, 4> sqpp() const { return vector<T, 4>(vl[0], vl[3], vl[2], vl[2]); }
        vector<T, 4> xwzw() const { return vector<T, 4>(vl[0], vl[3], vl[2], vl[3]); }
        vector<T, 4> raba() const { return vector<T, 4>(vl[0], vl[3], vl[2], vl[3]); }
        vector<T, 4> sqpq() const { return vector<T, 4>(vl[0], vl[3], vl[2], vl[3]); }
        vector<T, 4> xwwx() const { return vector<T, 4>(vl[0], vl[3], vl[3], vl[0]); }
        vector<T, 4> raar() const { return vector<T, 4>(vl[0], vl[3], vl[3], vl[0]); }
        vector<T, 4> sqqs() const { return vector<T, 4>(vl[0], vl[3], vl[3], vl[0]); }
        vector<T, 4> xwwy() const { return vector<T, 4>(vl[0], vl[3], vl[3], vl[1]); }
        vector<T, 4> raag() const { return vector<T, 4>(vl[0], vl[3], vl[3], vl[1]); }
        vector<T, 4> sqqt() const { return vector<T, 4>(vl[0], vl[3], vl[3], vl[1]); }
        vector<T, 4> xwwz() const { return vector<T, 4>(vl[0], vl[3], vl[3], vl[2]); }
        vector<T, 4> raab() const { return vector<T, 4>(vl[0], vl[3], vl[3], vl[2]); }
        vector<T, 4> sqqp() const { return vector<T, 4>(vl[0], vl[3], vl[3], vl[2]); }
        vector<T, 4> xwww() const { return vector<T, 4>(vl[0], vl[3], vl[3], vl[3]); }
        vector<T, 4> raaa() const { return vector<T, 4>(vl[0], vl[3], vl[3], vl[3]); }
        vector<T, 4> sqqq() const { return vector<T, 4>(vl[0], vl[3], vl[3], vl[3]); }
        vector<T, 4> yxxx() const { return vector<T, 4>(vl[1], vl[0], vl[0], vl[0]); }
        vector<T, 4> grrr() const { return vector<T, 4>(vl[1], vl[0], vl[0], vl[0]); }
        vector<T, 4> tsss() const { return vector<T, 4>(vl[1], vl[0], vl[0], vl[0]); }
        vector<T, 4> yxxy() const { return vector<T, 4>(vl[1], vl[0], vl[0], vl[1]); }
        vector<T, 4> grrg() const { return vector<T, 4>(vl[1], vl[0], vl[0], vl[1]); }
        vector<T, 4> tsst() const { return vector<T, 4>(vl[1], vl[0], vl[0], vl[1]); }
        vector<T, 4> yxxz() const { return vector<T, 4>(vl[1], vl[0], vl[0], vl[2]); }
        vector<T, 4> grrb() const { return vector<T, 4>(vl[1], vl[0], vl[0], vl[2]); }
        vector<T, 4> tssp() const { return vector<T, 4>(vl[1], vl[0], vl[0], vl[2]); }
        vector<T, 4> yxxw() const { return vector<T, 4>(vl[1], vl[0], vl[0], vl[3]); }
        vector<T, 4> grra() const { return vector<T, 4>(vl[1], vl[0], vl[0], vl[3]); }
        vector<T, 4> tssq() const { return vector<T, 4>(vl[1], vl[0], vl[0], vl[3]); }
        vector<T, 4> yxyx() const { return vector<T, 4>(vl[1], vl[0], vl[1], vl[0]); }
        vector<T, 4> grgr() const { return vector<T, 4>(vl[1], vl[0], vl[1], vl[0]); }
        vector<T, 4> tsts() const { return vector<T, 4>(vl[1], vl[0], vl[1], vl[0]); }
        vector<T, 4> yxyy() const { return vector<T, 4>(vl[1], vl[0], vl[1], vl[1]); }
        vector<T, 4> grgg() const { return vector<T, 4>(vl[1], vl[0], vl[1], vl[1]); }
        vector<T, 4> tstt() const { return vector<T, 4>(vl[1], vl[0], vl[1], vl[1]); }
        vector<T, 4> yxyz() const { return vector<T, 4>(vl[1], vl[0], vl[1], vl[2]); }
        vector<T, 4> grgb() const { return vector<T, 4>(vl[1], vl[0], vl[1], vl[2]); }
        vector<T, 4> tstp() const { return vector<T, 4>(vl[1], vl[0], vl[1], vl[2]); }
        vector<T, 4> yxyw() const { return vector<T, 4>(vl[1], vl[0], vl[1], vl[3]); }
        vector<T, 4> grga() const { return vector<T, 4>(vl[1], vl[0], vl[1], vl[3]); }
        vector<T, 4> tstq() const { return vector<T, 4>(vl[1], vl[0], vl[1], vl[3]); }
        vector<T, 4> yxzx() const { return vector<T, 4>(vl[1], vl[0], vl[2], vl[0]); }
        vector<T, 4> grbr() const { return vector<T, 4>(vl[1], vl[0], vl[2], vl[0]); }
        vector<T, 4> tsps() const { return vector<T, 4>(vl[1], vl[0], vl[2], vl[0]); }
        vector<T, 4> yxzy() const { return vector<T, 4>(vl[1], vl[0], vl[2], vl[1]); }
        vector<T, 4> grbg() const { return vector<T, 4>(vl[1], vl[0], vl[2], vl[1]); }
        vector<T, 4> tspt() const { return vector<T, 4>(vl[1], vl[0], vl[2], vl[1]); }
        vector<T, 4> yxzz() const { return vector<T, 4>(vl[1], vl[0], vl[2], vl[2]); }
        vector<T, 4> grbb() const { return vector<T, 4>(vl[1], vl[0], vl[2], vl[2]); }
        vector<T, 4> tspp() const { return vector<T, 4>(vl[1], vl[0], vl[2], vl[2]); }
        vector<T, 4> yxzw() const { return vector<T, 4>(vl[1], vl[0], vl[2], vl[3]); }
        vector<T, 4> grba() const { return vector<T, 4>(vl[1], vl[0], vl[2], vl[3]); }
        vector<T, 4> tspq() const { return vector<T, 4>(vl[1], vl[0], vl[2], vl[3]); }
        vector<T, 4> yxwx() const { return vector<T, 4>(vl[1], vl[0], vl[3], vl[0]); }
        vector<T, 4> grar() const { return vector<T, 4>(vl[1], vl[0], vl[3], vl[0]); }
        vector<T, 4> tsqs() const { return vector<T, 4>(vl[1], vl[0], vl[3], vl[0]); }
        vector<T, 4> yxwy() const { return vector<T, 4>(vl[1], vl[0], vl[3], vl[1]); }
        vector<T, 4> grag() const { return vector<T, 4>(vl[1], vl[0], vl[3], vl[1]); }
        vector<T, 4> tsqt() const { return vector<T, 4>(vl[1], vl[0], vl[3], vl[1]); }
        vector<T, 4> yxwz() const { return vector<T, 4>(vl[1], vl[0], vl[3], vl[2]); }
        vector<T, 4> grab() const { return vector<T, 4>(vl[1], vl[0], vl[3], vl[2]); }
        vector<T, 4> tsqp() const { return vector<T, 4>(vl[1], vl[0], vl[3], vl[2]); }
        vector<T, 4> yxww() const { return vector<T, 4>(vl[1], vl[0], vl[3], vl[3]); }
        vector<T, 4> graa() const { return vector<T, 4>(vl[1], vl[0], vl[3], vl[3]); }
        vector<T, 4> tsqq() const { return vector<T, 4>(vl[1], vl[0], vl[3], vl[3]); }
        vector<T, 4> yyxx() const { return vector<T, 4>(vl[1], vl[1], vl[0], vl[0]); }
        vector<T, 4> ggrr() const { return vector<T, 4>(vl[1], vl[1], vl[0], vl[0]); }
        vector<T, 4> ttss() const { return vector<T, 4>(vl[1], vl[1], vl[0], vl[0]); }
        vector<T, 4> yyxy() const { return vector<T, 4>(vl[1], vl[1], vl[0], vl[1]); }
        vector<T, 4> ggrg() const { return vector<T, 4>(vl[1], vl[1], vl[0], vl[1]); }
        vector<T, 4> ttst() const { return vector<T, 4>(vl[1], vl[1], vl[0], vl[1]); }
        vector<T, 4> yyxz() const { return vector<T, 4>(vl[1], vl[1], vl[0], vl[2]); }
        vector<T, 4> ggrb() const { return vector<T, 4>(vl[1], vl[1], vl[0], vl[2]); }
        vector<T, 4> ttsp() const { return vector<T, 4>(vl[1], vl[1], vl[0], vl[2]); }
        vector<T, 4> yyxw() const { return vector<T, 4>(vl[1], vl[1], vl[0], vl[3]); }
        vector<T, 4> ggra() const { return vector<T, 4>(vl[1], vl[1], vl[0], vl[3]); }
        vector<T, 4> ttsq() const { return vector<T, 4>(vl[1], vl[1], vl[0], vl[3]); }
        vector<T, 4> yyyx() const { return vector<T, 4>(vl[1], vl[1], vl[1], vl[0]); }
        vector<T, 4> gggr() const { return vector<T, 4>(vl[1], vl[1], vl[1], vl[0]); }
        vector<T, 4> ttts() const { return vector<T, 4>(vl[1], vl[1], vl[1], vl[0]); }
        vector<T, 4> yyyy() const { return vector<T, 4>(vl[1], vl[1], vl[1], vl[1]); }
        vector<T, 4> gggg() const { return vector<T, 4>(vl[1], vl[1], vl[1], vl[1]); }
        vector<T, 4> tttt() const { return vector<T, 4>(vl[1], vl[1], vl[1], vl[1]); }
        vector<T, 4> yyyz() const { return vector<T, 4>(vl[1], vl[1], vl[1], vl[2]); }
        vector<T, 4> gggb() const { return vector<T, 4>(vl[1], vl[1], vl[1], vl[2]); }
        vector<T, 4> tttp() const { return vector<T, 4>(vl[1], vl[1], vl[1], vl[2]); }
        vector<T, 4> yyyw() const { return vector<T, 4>(vl[1], vl[1], vl[1], vl[3]); }
        vector<T, 4> ggga() const { return vector<T, 4>(vl[1], vl[1], vl[1], vl[3]); }
        vector<T, 4> tttq() const { return vector<T, 4>(vl[1], vl[1], vl[1], vl[3]); }
        vector<T, 4> yyzx() const { return vector<T, 4>(vl[1], vl[1], vl[2], vl[0]); }
        vector<T, 4> ggbr() const { return vector<T, 4>(vl[1], vl[1], vl[2], vl[0]); }
        vector<T, 4> ttps() const { return vector<T, 4>(vl[1], vl[1], vl[2], vl[0]); }
        vector<T, 4> yyzy() const { return vector<T, 4>(vl[1], vl[1], vl[2], vl[1]); }
        vector<T, 4> ggbg() const { return vector<T, 4>(vl[1], vl[1], vl[2], vl[1]); }
        vector<T, 4> ttpt() const { return vector<T, 4>(vl[1], vl[1], vl[2], vl[1]); }
        vector<T, 4> yyzz() const { return vector<T, 4>(vl[1], vl[1], vl[2], vl[2]); }
        vector<T, 4> ggbb() const { return vector<T, 4>(vl[1], vl[1], vl[2], vl[2]); }
        vector<T, 4> ttpp() const { return vector<T, 4>(vl[1], vl[1], vl[2], vl[2]); }
        vector<T, 4> yyzw() const { return vector<T, 4>(vl[1], vl[1], vl[2], vl[3]); }
        vector<T, 4> ggba() const { return vector<T, 4>(vl[1], vl[1], vl[2], vl[3]); }
        vector<T, 4> ttpq() const { return vector<T, 4>(vl[1], vl[1], vl[2], vl[3]); }
        vector<T, 4> yywx() const { return vector<T, 4>(vl[1], vl[1], vl[3], vl[0]); }
        vector<T, 4> ggar() const { return vector<T, 4>(vl[1], vl[1], vl[3], vl[0]); }
        vector<T, 4> ttqs() const { return vector<T, 4>(vl[1], vl[1], vl[3], vl[0]); }
        vector<T, 4> yywy() const { return vector<T, 4>(vl[1], vl[1], vl[3], vl[1]); }
        vector<T, 4> ggag() const { return vector<T, 4>(vl[1], vl[1], vl[3], vl[1]); }
        vector<T, 4> ttqt() const { return vector<T, 4>(vl[1], vl[1], vl[3], vl[1]); }
        vector<T, 4> yywz() const { return vector<T, 4>(vl[1], vl[1], vl[3], vl[2]); }
        vector<T, 4> ggab() const { return vector<T, 4>(vl[1], vl[1], vl[3], vl[2]); }
        vector<T, 4> ttqp() const { return vector<T, 4>(vl[1], vl[1], vl[3], vl[2]); }
        vector<T, 4> yyww() const { return vector<T, 4>(vl[1], vl[1], vl[3], vl[3]); }
        vector<T, 4> ggaa() const { return vector<T, 4>(vl[1], vl[1], vl[3], vl[3]); }
        vector<T, 4> ttqq() const { return vector<T, 4>(vl[1], vl[1], vl[3], vl[3]); }
        vector<T, 4> yzxx() const { return vector<T, 4>(vl[1], vl[2], vl[0], vl[0]); }
        vector<T, 4> gbrr() const { return vector<T, 4>(vl[1], vl[2], vl[0], vl[0]); }
        vector<T, 4> tpss() const { return vector<T, 4>(vl[1], vl[2], vl[0], vl[0]); }
        vector<T, 4> yzxy() const { return vector<T, 4>(vl[1], vl[2], vl[0], vl[1]); }
        vector<T, 4> gbrg() const { return vector<T, 4>(vl[1], vl[2], vl[0], vl[1]); }
        vector<T, 4> tpst() const { return vector<T, 4>(vl[1], vl[2], vl[0], vl[1]); }
        vector<T, 4> yzxz() const { return vector<T, 4>(vl[1], vl[2], vl[0], vl[2]); }
        vector<T, 4> gbrb() const { return vector<T, 4>(vl[1], vl[2], vl[0], vl[2]); }
        vector<T, 4> tpsp() const { return vector<T, 4>(vl[1], vl[2], vl[0], vl[2]); }
        vector<T, 4> yzxw() const { return vector<T, 4>(vl[1], vl[2], vl[0], vl[3]); }
        vector<T, 4> gbra() const { return vector<T, 4>(vl[1], vl[2], vl[0], vl[3]); }
        vector<T, 4> tpsq() const { return vector<T, 4>(vl[1], vl[2], vl[0], vl[3]); }
        vector<T, 4> yzyx() const { return vector<T, 4>(vl[1], vl[2], vl[1], vl[0]); }
        vector<T, 4> gbgr() const { return vector<T, 4>(vl[1], vl[2], vl[1], vl[0]); }
        vector<T, 4> tpts() const { return vector<T, 4>(vl[1], vl[2], vl[1], vl[0]); }
        vector<T, 4> yzyy() const { return vector<T, 4>(vl[1], vl[2], vl[1], vl[1]); }
        vector<T, 4> gbgg() const { return vector<T, 4>(vl[1], vl[2], vl[1], vl[1]); }
        vector<T, 4> tptt() const { return vector<T, 4>(vl[1], vl[2], vl[1], vl[1]); }
        vector<T, 4> yzyz() const { return vector<T, 4>(vl[1], vl[2], vl[1], vl[2]); }
        vector<T, 4> gbgb() const { return vector<T, 4>(vl[1], vl[2], vl[1], vl[2]); }
        vector<T, 4> tptp() const { return vector<T, 4>(vl[1], vl[2], vl[1], vl[2]); }
        vector<T, 4> yzyw() const { return vector<T, 4>(vl[1], vl[2], vl[1], vl[3]); }
        vector<T, 4> gbga() const { return vector<T, 4>(vl[1], vl[2], vl[1], vl[3]); }
        vector<T, 4> tptq() const { return vector<T, 4>(vl[1], vl[2], vl[1], vl[3]); }
        vector<T, 4> yzzx() const { return vector<T, 4>(vl[1], vl[2], vl[2], vl[0]); }
        vector<T, 4> gbbr() const { return vector<T, 4>(vl[1], vl[2], vl[2], vl[0]); }
        vector<T, 4> tpps() const { return vector<T, 4>(vl[1], vl[2], vl[2], vl[0]); }
        vector<T, 4> yzzy() const { return vector<T, 4>(vl[1], vl[2], vl[2], vl[1]); }
        vector<T, 4> gbbg() const { return vector<T, 4>(vl[1], vl[2], vl[2], vl[1]); }
        vector<T, 4> tppt() const { return vector<T, 4>(vl[1], vl[2], vl[2], vl[1]); }
        vector<T, 4> yzzz() const { return vector<T, 4>(vl[1], vl[2], vl[2], vl[2]); }
        vector<T, 4> gbbb() const { return vector<T, 4>(vl[1], vl[2], vl[2], vl[2]); }
        vector<T, 4> tppp() const { return vector<T, 4>(vl[1], vl[2], vl[2], vl[2]); }
        vector<T, 4> yzzw() const { return vector<T, 4>(vl[1], vl[2], vl[2], vl[3]); }
        vector<T, 4> gbba() const { return vector<T, 4>(vl[1], vl[2], vl[2], vl[3]); }
        vector<T, 4> tppq() const { return vector<T, 4>(vl[1], vl[2], vl[2], vl[3]); }
        vector<T, 4> yzwx() const { return vector<T, 4>(vl[1], vl[2], vl[3], vl[0]); }
        vector<T, 4> gbar() const { return vector<T, 4>(vl[1], vl[2], vl[3], vl[0]); }
        vector<T, 4> tpqs() const { return vector<T, 4>(vl[1], vl[2], vl[3], vl[0]); }
        vector<T, 4> yzwy() const { return vector<T, 4>(vl[1], vl[2], vl[3], vl[1]); }
        vector<T, 4> gbag() const { return vector<T, 4>(vl[1], vl[2], vl[3], vl[1]); }
        vector<T, 4> tpqt() const { return vector<T, 4>(vl[1], vl[2], vl[3], vl[1]); }
        vector<T, 4> yzwz() const { return vector<T, 4>(vl[1], vl[2], vl[3], vl[2]); }
        vector<T, 4> gbab() const { return vector<T, 4>(vl[1], vl[2], vl[3], vl[2]); }
        vector<T, 4> tpqp() const { return vector<T, 4>(vl[1], vl[2], vl[3], vl[2]); }
        vector<T, 4> yzww() const { return vector<T, 4>(vl[1], vl[2], vl[3], vl[3]); }
        vector<T, 4> gbaa() const { return vector<T, 4>(vl[1], vl[2], vl[3], vl[3]); }
        vector<T, 4> tpqq() const { return vector<T, 4>(vl[1], vl[2], vl[3], vl[3]); }
        vector<T, 4> ywxx() const { return vector<T, 4>(vl[1], vl[3], vl[0], vl[0]); }
        vector<T, 4> garr() const { return vector<T, 4>(vl[1], vl[3], vl[0], vl[0]); }
        vector<T, 4> tqss() const { return vector<T, 4>(vl[1], vl[3], vl[0], vl[0]); }
        vector<T, 4> ywxy() const { return vector<T, 4>(vl[1], vl[3], vl[0], vl[1]); }
        vector<T, 4> garg() const { return vector<T, 4>(vl[1], vl[3], vl[0], vl[1]); }
        vector<T, 4> tqst() const { return vector<T, 4>(vl[1], vl[3], vl[0], vl[1]); }
        vector<T, 4> ywxz() const { return vector<T, 4>(vl[1], vl[3], vl[0], vl[2]); }
        vector<T, 4> garb() const { return vector<T, 4>(vl[1], vl[3], vl[0], vl[2]); }
        vector<T, 4> tqsp() const { return vector<T, 4>(vl[1], vl[3], vl[0], vl[2]); }
        vector<T, 4> ywxw() const { return vector<T, 4>(vl[1], vl[3], vl[0], vl[3]); }
        vector<T, 4> gara() const { return vector<T, 4>(vl[1], vl[3], vl[0], vl[3]); }
        vector<T, 4> tqsq() const { return vector<T, 4>(vl[1], vl[3], vl[0], vl[3]); }
        vector<T, 4> ywyx() const { return vector<T, 4>(vl[1], vl[3], vl[1], vl[0]); }
        vector<T, 4> gagr() const { return vector<T, 4>(vl[1], vl[3], vl[1], vl[0]); }
        vector<T, 4> tqts() const { return vector<T, 4>(vl[1], vl[3], vl[1], vl[0]); }
        vector<T, 4> ywyy() const { return vector<T, 4>(vl[1], vl[3], vl[1], vl[1]); }
        vector<T, 4> gagg() const { return vector<T, 4>(vl[1], vl[3], vl[1], vl[1]); }
        vector<T, 4> tqtt() const { return vector<T, 4>(vl[1], vl[3], vl[1], vl[1]); }
        vector<T, 4> ywyz() const { return vector<T, 4>(vl[1], vl[3], vl[1], vl[2]); }
        vector<T, 4> gagb() const { return vector<T, 4>(vl[1], vl[3], vl[1], vl[2]); }
        vector<T, 4> tqtp() const { return vector<T, 4>(vl[1], vl[3], vl[1], vl[2]); }
        vector<T, 4> ywyw() const { return vector<T, 4>(vl[1], vl[3], vl[1], vl[3]); }
        vector<T, 4> gaga() const { return vector<T, 4>(vl[1], vl[3], vl[1], vl[3]); }
        vector<T, 4> tqtq() const { return vector<T, 4>(vl[1], vl[3], vl[1], vl[3]); }
        vector<T, 4> ywzx() const { return vector<T, 4>(vl[1], vl[3], vl[2], vl[0]); }
        vector<T, 4> gabr() const { return vector<T, 4>(vl[1], vl[3], vl[2], vl[0]); }
        vector<T, 4> tqps() const { return vector<T, 4>(vl[1], vl[3], vl[2], vl[0]); }
        vector<T, 4> ywzy() const { return vector<T, 4>(vl[1], vl[3], vl[2], vl[1]); }
        vector<T, 4> gabg() const { return vector<T, 4>(vl[1], vl[3], vl[2], vl[1]); }
        vector<T, 4> tqpt() const { return vector<T, 4>(vl[1], vl[3], vl[2], vl[1]); }
        vector<T, 4> ywzz() const { return vector<T, 4>(vl[1], vl[3], vl[2], vl[2]); }
        vector<T, 4> gabb() const { return vector<T, 4>(vl[1], vl[3], vl[2], vl[2]); }
        vector<T, 4> tqpp() const { return vector<T, 4>(vl[1], vl[3], vl[2], vl[2]); }
        vector<T, 4> ywzw() const { return vector<T, 4>(vl[1], vl[3], vl[2], vl[3]); }
        vector<T, 4> gaba() const { return vector<T, 4>(vl[1], vl[3], vl[2], vl[3]); }
        vector<T, 4> tqpq() const { return vector<T, 4>(vl[1], vl[3], vl[2], vl[3]); }
        vector<T, 4> ywwx() const { return vector<T, 4>(vl[1], vl[3], vl[3], vl[0]); }
        vector<T, 4> gaar() const { return vector<T, 4>(vl[1], vl[3], vl[3], vl[0]); }
        vector<T, 4> tqqs() const { return vector<T, 4>(vl[1], vl[3], vl[3], vl[0]); }
        vector<T, 4> ywwy() const { return vector<T, 4>(vl[1], vl[3], vl[3], vl[1]); }
        vector<T, 4> gaag() const { return vector<T, 4>(vl[1], vl[3], vl[3], vl[1]); }
        vector<T, 4> tqqt() const { return vector<T, 4>(vl[1], vl[3], vl[3], vl[1]); }
        vector<T, 4> ywwz() const { return vector<T, 4>(vl[1], vl[3], vl[3], vl[2]); }
        vector<T, 4> gaab() const { return vector<T, 4>(vl[1], vl[3], vl[3], vl[2]); }
        vector<T, 4> tqqp() const { return vector<T, 4>(vl[1], vl[3], vl[3], vl[2]); }
        vector<T, 4> ywww() const { return vector<T, 4>(vl[1], vl[3], vl[3], vl[3]); }
        vector<T, 4> gaaa() const { return vector<T, 4>(vl[1], vl[3], vl[3], vl[3]); }
        vector<T, 4> tqqq() const { return vector<T, 4>(vl[1], vl[3], vl[3], vl[3]); }
        vector<T, 4> zxxx() const { return vector<T, 4>(vl[2], vl[0], vl[0], vl[0]); }
        vector<T, 4> brrr() const { return vector<T, 4>(vl[2], vl[0], vl[0], vl[0]); }
        vector<T, 4> psss() const { return vector<T, 4>(vl[2], vl[0], vl[0], vl[0]); }
        vector<T, 4> zxxy() const { return vector<T, 4>(vl[2], vl[0], vl[0], vl[1]); }
        vector<T, 4> brrg() const { return vector<T, 4>(vl[2], vl[0], vl[0], vl[1]); }
        vector<T, 4> psst() const { return vector<T, 4>(vl[2], vl[0], vl[0], vl[1]); }
        vector<T, 4> zxxz() const { return vector<T, 4>(vl[2], vl[0], vl[0], vl[2]); }
        vector<T, 4> brrb() const { return vector<T, 4>(vl[2], vl[0], vl[0], vl[2]); }
        vector<T, 4> pssp() const { return vector<T, 4>(vl[2], vl[0], vl[0], vl[2]); }
        vector<T, 4> zxxw() const { return vector<T, 4>(vl[2], vl[0], vl[0], vl[3]); }
        vector<T, 4> brra() const { return vector<T, 4>(vl[2], vl[0], vl[0], vl[3]); }
        vector<T, 4> pssq() const { return vector<T, 4>(vl[2], vl[0], vl[0], vl[3]); }
        vector<T, 4> zxyx() const { return vector<T, 4>(vl[2], vl[0], vl[1], vl[0]); }
        vector<T, 4> brgr() const { return vector<T, 4>(vl[2], vl[0], vl[1], vl[0]); }
        vector<T, 4> psts() const { return vector<T, 4>(vl[2], vl[0], vl[1], vl[0]); }
        vector<T, 4> zxyy() const { return vector<T, 4>(vl[2], vl[0], vl[1], vl[1]); }
        vector<T, 4> brgg() const { return vector<T, 4>(vl[2], vl[0], vl[1], vl[1]); }
        vector<T, 4> pstt() const { return vector<T, 4>(vl[2], vl[0], vl[1], vl[1]); }
        vector<T, 4> zxyz() const { return vector<T, 4>(vl[2], vl[0], vl[1], vl[2]); }
        vector<T, 4> brgb() const { return vector<T, 4>(vl[2], vl[0], vl[1], vl[2]); }
        vector<T, 4> pstp() const { return vector<T, 4>(vl[2], vl[0], vl[1], vl[2]); }
        vector<T, 4> zxyw() const { return vector<T, 4>(vl[2], vl[0], vl[1], vl[3]); }
        vector<T, 4> brga() const { return vector<T, 4>(vl[2], vl[0], vl[1], vl[3]); }
        vector<T, 4> pstq() const { return vector<T, 4>(vl[2], vl[0], vl[1], vl[3]); }
        vector<T, 4> zxzx() const { return vector<T, 4>(vl[2], vl[0], vl[2], vl[0]); }
        vector<T, 4> brbr() const { return vector<T, 4>(vl[2], vl[0], vl[2], vl[0]); }
        vector<T, 4> psps() const { return vector<T, 4>(vl[2], vl[0], vl[2], vl[0]); }
        vector<T, 4> zxzy() const { return vector<T, 4>(vl[2], vl[0], vl[2], vl[1]); }
        vector<T, 4> brbg() const { return vector<T, 4>(vl[2], vl[0], vl[2], vl[1]); }
        vector<T, 4> pspt() const { return vector<T, 4>(vl[2], vl[0], vl[2], vl[1]); }
        vector<T, 4> zxzz() const { return vector<T, 4>(vl[2], vl[0], vl[2], vl[2]); }
        vector<T, 4> brbb() const { return vector<T, 4>(vl[2], vl[0], vl[2], vl[2]); }
        vector<T, 4> pspp() const { return vector<T, 4>(vl[2], vl[0], vl[2], vl[2]); }
        vector<T, 4> zxzw() const { return vector<T, 4>(vl[2], vl[0], vl[2], vl[3]); }
        vector<T, 4> brba() const { return vector<T, 4>(vl[2], vl[0], vl[2], vl[3]); }
        vector<T, 4> pspq() const { return vector<T, 4>(vl[2], vl[0], vl[2], vl[3]); }
        vector<T, 4> zxwx() const { return vector<T, 4>(vl[2], vl[0], vl[3], vl[0]); }
        vector<T, 4> brar() const { return vector<T, 4>(vl[2], vl[0], vl[3], vl[0]); }
        vector<T, 4> psqs() const { return vector<T, 4>(vl[2], vl[0], vl[3], vl[0]); }
        vector<T, 4> zxwy() const { return vector<T, 4>(vl[2], vl[0], vl[3], vl[1]); }
        vector<T, 4> brag() const { return vector<T, 4>(vl[2], vl[0], vl[3], vl[1]); }
        vector<T, 4> psqt() const { return vector<T, 4>(vl[2], vl[0], vl[3], vl[1]); }
        vector<T, 4> zxwz() const { return vector<T, 4>(vl[2], vl[0], vl[3], vl[2]); }
        vector<T, 4> brab() const { return vector<T, 4>(vl[2], vl[0], vl[3], vl[2]); }
        vector<T, 4> psqp() const { return vector<T, 4>(vl[2], vl[0], vl[3], vl[2]); }
        vector<T, 4> zxww() const { return vector<T, 4>(vl[2], vl[0], vl[3], vl[3]); }
        vector<T, 4> braa() const { return vector<T, 4>(vl[2], vl[0], vl[3], vl[3]); }
        vector<T, 4> psqq() const { return vector<T, 4>(vl[2], vl[0], vl[3], vl[3]); }
        vector<T, 4> zyxx() const { return vector<T, 4>(vl[2], vl[1], vl[0], vl[0]); }
        vector<T, 4> bgrr() const { return vector<T, 4>(vl[2], vl[1], vl[0], vl[0]); }
        vector<T, 4> ptss() const { return vector<T, 4>(vl[2], vl[1], vl[0], vl[0]); }
        vector<T, 4> zyxy() const { return vector<T, 4>(vl[2], vl[1], vl[0], vl[1]); }
        vector<T, 4> bgrg() const { return vector<T, 4>(vl[2], vl[1], vl[0], vl[1]); }
        vector<T, 4> ptst() const { return vector<T, 4>(vl[2], vl[1], vl[0], vl[1]); }
        vector<T, 4> zyxz() const { return vector<T, 4>(vl[2], vl[1], vl[0], vl[2]); }
        vector<T, 4> bgrb() const { return vector<T, 4>(vl[2], vl[1], vl[0], vl[2]); }
        vector<T, 4> ptsp() const { return vector<T, 4>(vl[2], vl[1], vl[0], vl[2]); }
        vector<T, 4> zyxw() const { return vector<T, 4>(vl[2], vl[1], vl[0], vl[3]); }
        vector<T, 4> bgra() const { return vector<T, 4>(vl[2], vl[1], vl[0], vl[3]); }
        vector<T, 4> ptsq() const { return vector<T, 4>(vl[2], vl[1], vl[0], vl[3]); }
        vector<T, 4> zyyx() const { return vector<T, 4>(vl[2], vl[1], vl[1], vl[0]); }
        vector<T, 4> bggr() const { return vector<T, 4>(vl[2], vl[1], vl[1], vl[0]); }
        vector<T, 4> ptts() const { return vector<T, 4>(vl[2], vl[1], vl[1], vl[0]); }
        vector<T, 4> zyyy() const { return vector<T, 4>(vl[2], vl[1], vl[1], vl[1]); }
        vector<T, 4> bggg() const { return vector<T, 4>(vl[2], vl[1], vl[1], vl[1]); }
        vector<T, 4> pttt() const { return vector<T, 4>(vl[2], vl[1], vl[1], vl[1]); }
        vector<T, 4> zyyz() const { return vector<T, 4>(vl[2], vl[1], vl[1], vl[2]); }
        vector<T, 4> bggb() const { return vector<T, 4>(vl[2], vl[1], vl[1], vl[2]); }
        vector<T, 4> pttp() const { return vector<T, 4>(vl[2], vl[1], vl[1], vl[2]); }
        vector<T, 4> zyyw() const { return vector<T, 4>(vl[2], vl[1], vl[1], vl[3]); }
        vector<T, 4> bgga() const { return vector<T, 4>(vl[2], vl[1], vl[1], vl[3]); }
        vector<T, 4> pttq() const { return vector<T, 4>(vl[2], vl[1], vl[1], vl[3]); }
        vector<T, 4> zyzx() const { return vector<T, 4>(vl[2], vl[1], vl[2], vl[0]); }
        vector<T, 4> bgbr() const { return vector<T, 4>(vl[2], vl[1], vl[2], vl[0]); }
        vector<T, 4> ptps() const { return vector<T, 4>(vl[2], vl[1], vl[2], vl[0]); }
        vector<T, 4> zyzy() const { return vector<T, 4>(vl[2], vl[1], vl[2], vl[1]); }
        vector<T, 4> bgbg() const { return vector<T, 4>(vl[2], vl[1], vl[2], vl[1]); }
        vector<T, 4> ptpt() const { return vector<T, 4>(vl[2], vl[1], vl[2], vl[1]); }
        vector<T, 4> zyzz() const { return vector<T, 4>(vl[2], vl[1], vl[2], vl[2]); }
        vector<T, 4> bgbb() const { return vector<T, 4>(vl[2], vl[1], vl[2], vl[2]); }
        vector<T, 4> ptpp() const { return vector<T, 4>(vl[2], vl[1], vl[2], vl[2]); }
        vector<T, 4> zyzw() const { return vector<T, 4>(vl[2], vl[1], vl[2], vl[3]); }
        vector<T, 4> bgba() const { return vector<T, 4>(vl[2], vl[1], vl[2], vl[3]); }
        vector<T, 4> ptpq() const { return vector<T, 4>(vl[2], vl[1], vl[2], vl[3]); }
        vector<T, 4> zywx() const { return vector<T, 4>(vl[2], vl[1], vl[3], vl[0]); }
        vector<T, 4> bgar() const { return vector<T, 4>(vl[2], vl[1], vl[3], vl[0]); }
        vector<T, 4> ptqs() const { return vector<T, 4>(vl[2], vl[1], vl[3], vl[0]); }
        vector<T, 4> zywy() const { return vector<T, 4>(vl[2], vl[1], vl[3], vl[1]); }
        vector<T, 4> bgag() const { return vector<T, 4>(vl[2], vl[1], vl[3], vl[1]); }
        vector<T, 4> ptqt() const { return vector<T, 4>(vl[2], vl[1], vl[3], vl[1]); }
        vector<T, 4> zywz() const { return vector<T, 4>(vl[2], vl[1], vl[3], vl[2]); }
        vector<T, 4> bgab() const { return vector<T, 4>(vl[2], vl[1], vl[3], vl[2]); }
        vector<T, 4> ptqp() const { return vector<T, 4>(vl[2], vl[1], vl[3], vl[2]); }
        vector<T, 4> zyww() const { return vector<T, 4>(vl[2], vl[1], vl[3], vl[3]); }
        vector<T, 4> bgaa() const { return vector<T, 4>(vl[2], vl[1], vl[3], vl[3]); }
        vector<T, 4> ptqq() const { return vector<T, 4>(vl[2], vl[1], vl[3], vl[3]); }
        vector<T, 4> zzxx() const { return vector<T, 4>(vl[2], vl[2], vl[0], vl[0]); }
        vector<T, 4> bbrr() const { return vector<T, 4>(vl[2], vl[2], vl[0], vl[0]); }
        vector<T, 4> ppss() const { return vector<T, 4>(vl[2], vl[2], vl[0], vl[0]); }
        vector<T, 4> zzxy() const { return vector<T, 4>(vl[2], vl[2], vl[0], vl[1]); }
        vector<T, 4> bbrg() const { return vector<T, 4>(vl[2], vl[2], vl[0], vl[1]); }
        vector<T, 4> ppst() const { return vector<T, 4>(vl[2], vl[2], vl[0], vl[1]); }
        vector<T, 4> zzxz() const { return vector<T, 4>(vl[2], vl[2], vl[0], vl[2]); }
        vector<T, 4> bbrb() const { return vector<T, 4>(vl[2], vl[2], vl[0], vl[2]); }
        vector<T, 4> ppsp() const { return vector<T, 4>(vl[2], vl[2], vl[0], vl[2]); }
        vector<T, 4> zzxw() const { return vector<T, 4>(vl[2], vl[2], vl[0], vl[3]); }
        vector<T, 4> bbra() const { return vector<T, 4>(vl[2], vl[2], vl[0], vl[3]); }
        vector<T, 4> ppsq() const { return vector<T, 4>(vl[2], vl[2], vl[0], vl[3]); }
        vector<T, 4> zzyx() const { return vector<T, 4>(vl[2], vl[2], vl[1], vl[0]); }
        vector<T, 4> bbgr() const { return vector<T, 4>(vl[2], vl[2], vl[1], vl[0]); }
        vector<T, 4> ppts() const { return vector<T, 4>(vl[2], vl[2], vl[1], vl[0]); }
        vector<T, 4> zzyy() const { return vector<T, 4>(vl[2], vl[2], vl[1], vl[1]); }
        vector<T, 4> bbgg() const { return vector<T, 4>(vl[2], vl[2], vl[1], vl[1]); }
        vector<T, 4> pptt() const { return vector<T, 4>(vl[2], vl[2], vl[1], vl[1]); }
        vector<T, 4> zzyz() const { return vector<T, 4>(vl[2], vl[2], vl[1], vl[2]); }
        vector<T, 4> bbgb() const { return vector<T, 4>(vl[2], vl[2], vl[1], vl[2]); }
        vector<T, 4> pptp() const { return vector<T, 4>(vl[2], vl[2], vl[1], vl[2]); }
        vector<T, 4> zzyw() const { return vector<T, 4>(vl[2], vl[2], vl[1], vl[3]); }
        vector<T, 4> bbga() const { return vector<T, 4>(vl[2], vl[2], vl[1], vl[3]); }
        vector<T, 4> pptq() const { return vector<T, 4>(vl[2], vl[2], vl[1], vl[3]); }
        vector<T, 4> zzzx() const { return vector<T, 4>(vl[2], vl[2], vl[2], vl[0]); }
        vector<T, 4> bbbr() const { return vector<T, 4>(vl[2], vl[2], vl[2], vl[0]); }
        vector<T, 4> ppps() const { return vector<T, 4>(vl[2], vl[2], vl[2], vl[0]); }
        vector<T, 4> zzzy() const { return vector<T, 4>(vl[2], vl[2], vl[2], vl[1]); }
        vector<T, 4> bbbg() const { return vector<T, 4>(vl[2], vl[2], vl[2], vl[1]); }
        vector<T, 4> pppt() const { return vector<T, 4>(vl[2], vl[2], vl[2], vl[1]); }
        vector<T, 4> zzzz() const { return vector<T, 4>(vl[2], vl[2], vl[2], vl[2]); }
        vector<T, 4> bbbb() const { return vector<T, 4>(vl[2], vl[2], vl[2], vl[2]); }
        vector<T, 4> pppp() const { return vector<T, 4>(vl[2], vl[2], vl[2], vl[2]); }
        vector<T, 4> zzzw() const { return vector<T, 4>(vl[2], vl[2], vl[2], vl[3]); }
        vector<T, 4> bbba() const { return vector<T, 4>(vl[2], vl[2], vl[2], vl[3]); }
        vector<T, 4> pppq() const { return vector<T, 4>(vl[2], vl[2], vl[2], vl[3]); }
        vector<T, 4> zzwx() const { return vector<T, 4>(vl[2], vl[2], vl[3], vl[0]); }
        vector<T, 4> bbar() const { return vector<T, 4>(vl[2], vl[2], vl[3], vl[0]); }
        vector<T, 4> ppqs() const { return vector<T, 4>(vl[2], vl[2], vl[3], vl[0]); }
        vector<T, 4> zzwy() const { return vector<T, 4>(vl[2], vl[2], vl[3], vl[1]); }
        vector<T, 4> bbag() const { return vector<T, 4>(vl[2], vl[2], vl[3], vl[1]); }
        vector<T, 4> ppqt() const { return vector<T, 4>(vl[2], vl[2], vl[3], vl[1]); }
        vector<T, 4> zzwz() const { return vector<T, 4>(vl[2], vl[2], vl[3], vl[2]); }
        vector<T, 4> bbab() const { return vector<T, 4>(vl[2], vl[2], vl[3], vl[2]); }
        vector<T, 4> ppqp() const { return vector<T, 4>(vl[2], vl[2], vl[3], vl[2]); }
        vector<T, 4> zzww() const { return vector<T, 4>(vl[2], vl[2], vl[3], vl[3]); }
        vector<T, 4> bbaa() const { return vector<T, 4>(vl[2], vl[2], vl[3], vl[3]); }
        vector<T, 4> ppqq() const { return vector<T, 4>(vl[2], vl[2], vl[3], vl[3]); }
        vector<T, 4> zwxx() const { return vector<T, 4>(vl[2], vl[3], vl[0], vl[0]); }
        vector<T, 4> barr() const { return vector<T, 4>(vl[2], vl[3], vl[0], vl[0]); }
        vector<T, 4> pqss() const { return vector<T, 4>(vl[2], vl[3], vl[0], vl[0]); }
        vector<T, 4> zwxy() const { return vector<T, 4>(vl[2], vl[3], vl[0], vl[1]); }
        vector<T, 4> barg() const { return vector<T, 4>(vl[2], vl[3], vl[0], vl[1]); }
        vector<T, 4> pqst() const { return vector<T, 4>(vl[2], vl[3], vl[0], vl[1]); }
        vector<T, 4> zwxz() const { return vector<T, 4>(vl[2], vl[3], vl[0], vl[2]); }
        vector<T, 4> barb() const { return vector<T, 4>(vl[2], vl[3], vl[0], vl[2]); }
        vector<T, 4> pqsp() const { return vector<T, 4>(vl[2], vl[3], vl[0], vl[2]); }
        vector<T, 4> zwxw() const { return vector<T, 4>(vl[2], vl[3], vl[0], vl[3]); }
        vector<T, 4> bara() const { return vector<T, 4>(vl[2], vl[3], vl[0], vl[3]); }
        vector<T, 4> pqsq() const { return vector<T, 4>(vl[2], vl[3], vl[0], vl[3]); }
        vector<T, 4> zwyx() const { return vector<T, 4>(vl[2], vl[3], vl[1], vl[0]); }
        vector<T, 4> bagr() const { return vector<T, 4>(vl[2], vl[3], vl[1], vl[0]); }
        vector<T, 4> pqts() const { return vector<T, 4>(vl[2], vl[3], vl[1], vl[0]); }
        vector<T, 4> zwyy() const { return vector<T, 4>(vl[2], vl[3], vl[1], vl[1]); }
        vector<T, 4> bagg() const { return vector<T, 4>(vl[2], vl[3], vl[1], vl[1]); }
        vector<T, 4> pqtt() const { return vector<T, 4>(vl[2], vl[3], vl[1], vl[1]); }
        vector<T, 4> zwyz() const { return vector<T, 4>(vl[2], vl[3], vl[1], vl[2]); }
        vector<T, 4> bagb() const { return vector<T, 4>(vl[2], vl[3], vl[1], vl[2]); }
        vector<T, 4> pqtp() const { return vector<T, 4>(vl[2], vl[3], vl[1], vl[2]); }
        vector<T, 4> zwyw() const { return vector<T, 4>(vl[2], vl[3], vl[1], vl[3]); }
        vector<T, 4> baga() const { return vector<T, 4>(vl[2], vl[3], vl[1], vl[3]); }
        vector<T, 4> pqtq() const { return vector<T, 4>(vl[2], vl[3], vl[1], vl[3]); }
        vector<T, 4> zwzx() const { return vector<T, 4>(vl[2], vl[3], vl[2], vl[0]); }
        vector<T, 4> babr() const { return vector<T, 4>(vl[2], vl[3], vl[2], vl[0]); }
        vector<T, 4> pqps() const { return vector<T, 4>(vl[2], vl[3], vl[2], vl[0]); }
        vector<T, 4> zwzy() const { return vector<T, 4>(vl[2], vl[3], vl[2], vl[1]); }
        vector<T, 4> babg() const { return vector<T, 4>(vl[2], vl[3], vl[2], vl[1]); }
        vector<T, 4> pqpt() const { return vector<T, 4>(vl[2], vl[3], vl[2], vl[1]); }
        vector<T, 4> zwzz() const { return vector<T, 4>(vl[2], vl[3], vl[2], vl[2]); }
        vector<T, 4> babb() const { return vector<T, 4>(vl[2], vl[3], vl[2], vl[2]); }
        vector<T, 4> pqpp() const { return vector<T, 4>(vl[2], vl[3], vl[2], vl[2]); }
        vector<T, 4> zwzw() const { return vector<T, 4>(vl[2], vl[3], vl[2], vl[3]); }
        vector<T, 4> baba() const { return vector<T, 4>(vl[2], vl[3], vl[2], vl[3]); }
        vector<T, 4> pqpq() const { return vector<T, 4>(vl[2], vl[3], vl[2], vl[3]); }
        vector<T, 4> zwwx() const { return vector<T, 4>(vl[2], vl[3], vl[3], vl[0]); }
        vector<T, 4> baar() const { return vector<T, 4>(vl[2], vl[3], vl[3], vl[0]); }
        vector<T, 4> pqqs() const { return vector<T, 4>(vl[2], vl[3], vl[3], vl[0]); }
        vector<T, 4> zwwy() const { return vector<T, 4>(vl[2], vl[3], vl[3], vl[1]); }
        vector<T, 4> baag() const { return vector<T, 4>(vl[2], vl[3], vl[3], vl[1]); }
        vector<T, 4> pqqt() const { return vector<T, 4>(vl[2], vl[3], vl[3], vl[1]); }
        vector<T, 4> zwwz() const { return vector<T, 4>(vl[2], vl[3], vl[3], vl[2]); }
        vector<T, 4> baab() const { return vector<T, 4>(vl[2], vl[3], vl[3], vl[2]); }
        vector<T, 4> pqqp() const { return vector<T, 4>(vl[2], vl[3], vl[3], vl[2]); }
        vector<T, 4> zwww() const { return vector<T, 4>(vl[2], vl[3], vl[3], vl[3]); }
        vector<T, 4> baaa() const { return vector<T, 4>(vl[2], vl[3], vl[3], vl[3]); }
        vector<T, 4> pqqq() const { return vector<T, 4>(vl[2], vl[3], vl[3], vl[3]); }
        vector<T, 4> wxxx() const { return vector<T, 4>(vl[3], vl[0], vl[0], vl[0]); }
        vector<T, 4> arrr() const { return vector<T, 4>(vl[3], vl[0], vl[0], vl[0]); }
        vector<T, 4> qsss() const { return vector<T, 4>(vl[3], vl[0], vl[0], vl[0]); }
        vector<T, 4> wxxy() const { return vector<T, 4>(vl[3], vl[0], vl[0], vl[1]); }
        vector<T, 4> arrg() const { return vector<T, 4>(vl[3], vl[0], vl[0], vl[1]); }
        vector<T, 4> qsst() const { return vector<T, 4>(vl[3], vl[0], vl[0], vl[1]); }
        vector<T, 4> wxxz() const { return vector<T, 4>(vl[3], vl[0], vl[0], vl[2]); }
        vector<T, 4> arrb() const { return vector<T, 4>(vl[3], vl[0], vl[0], vl[2]); }
        vector<T, 4> qssp() const { return vector<T, 4>(vl[3], vl[0], vl[0], vl[2]); }
        vector<T, 4> wxxw() const { return vector<T, 4>(vl[3], vl[0], vl[0], vl[3]); }
        vector<T, 4> arra() const { return vector<T, 4>(vl[3], vl[0], vl[0], vl[3]); }
        vector<T, 4> qssq() const { return vector<T, 4>(vl[3], vl[0], vl[0], vl[3]); }
        vector<T, 4> wxyx() const { return vector<T, 4>(vl[3], vl[0], vl[1], vl[0]); }
        vector<T, 4> argr() const { return vector<T, 4>(vl[3], vl[0], vl[1], vl[0]); }
        vector<T, 4> qsts() const { return vector<T, 4>(vl[3], vl[0], vl[1], vl[0]); }
        vector<T, 4> wxyy() const { return vector<T, 4>(vl[3], vl[0], vl[1], vl[1]); }
        vector<T, 4> argg() const { return vector<T, 4>(vl[3], vl[0], vl[1], vl[1]); }
        vector<T, 4> qstt() const { return vector<T, 4>(vl[3], vl[0], vl[1], vl[1]); }
        vector<T, 4> wxyz() const { return vector<T, 4>(vl[3], vl[0], vl[1], vl[2]); }
        vector<T, 4> argb() const { return vector<T, 4>(vl[3], vl[0], vl[1], vl[2]); }
        vector<T, 4> qstp() const { return vector<T, 4>(vl[3], vl[0], vl[1], vl[2]); }
        vector<T, 4> wxyw() const { return vector<T, 4>(vl[3], vl[0], vl[1], vl[3]); }
        vector<T, 4> arga() const { return vector<T, 4>(vl[3], vl[0], vl[1], vl[3]); }
        vector<T, 4> qstq() const { return vector<T, 4>(vl[3], vl[0], vl[1], vl[3]); }
        vector<T, 4> wxzx() const { return vector<T, 4>(vl[3], vl[0], vl[2], vl[0]); }
        vector<T, 4> arbr() const { return vector<T, 4>(vl[3], vl[0], vl[2], vl[0]); }
        vector<T, 4> qsps() const { return vector<T, 4>(vl[3], vl[0], vl[2], vl[0]); }
        vector<T, 4> wxzy() const { return vector<T, 4>(vl[3], vl[0], vl[2], vl[1]); }
        vector<T, 4> arbg() const { return vector<T, 4>(vl[3], vl[0], vl[2], vl[1]); }
        vector<T, 4> qspt() const { return vector<T, 4>(vl[3], vl[0], vl[2], vl[1]); }
        vector<T, 4> wxzz() const { return vector<T, 4>(vl[3], vl[0], vl[2], vl[2]); }
        vector<T, 4> arbb() const { return vector<T, 4>(vl[3], vl[0], vl[2], vl[2]); }
        vector<T, 4> qspp() const { return vector<T, 4>(vl[3], vl[0], vl[2], vl[2]); }
        vector<T, 4> wxzw() const { return vector<T, 4>(vl[3], vl[0], vl[2], vl[3]); }
        vector<T, 4> arba() const { return vector<T, 4>(vl[3], vl[0], vl[2], vl[3]); }
        vector<T, 4> qspq() const { return vector<T, 4>(vl[3], vl[0], vl[2], vl[3]); }
        vector<T, 4> wxwx() const { return vector<T, 4>(vl[3], vl[0], vl[3], vl[0]); }
        vector<T, 4> arar() const { return vector<T, 4>(vl[3], vl[0], vl[3], vl[0]); }
        vector<T, 4> qsqs() const { return vector<T, 4>(vl[3], vl[0], vl[3], vl[0]); }
        vector<T, 4> wxwy() const { return vector<T, 4>(vl[3], vl[0], vl[3], vl[1]); }
        vector<T, 4> arag() const { return vector<T, 4>(vl[3], vl[0], vl[3], vl[1]); }
        vector<T, 4> qsqt() const { return vector<T, 4>(vl[3], vl[0], vl[3], vl[1]); }
        vector<T, 4> wxwz() const { return vector<T, 4>(vl[3], vl[0], vl[3], vl[2]); }
        vector<T, 4> arab() const { return vector<T, 4>(vl[3], vl[0], vl[3], vl[2]); }
        vector<T, 4> qsqp() const { return vector<T, 4>(vl[3], vl[0], vl[3], vl[2]); }
        vector<T, 4> wxww() const { return vector<T, 4>(vl[3], vl[0], vl[3], vl[3]); }
        vector<T, 4> araa() const { return vector<T, 4>(vl[3], vl[0], vl[3], vl[3]); }
        vector<T, 4> qsqq() const { return vector<T, 4>(vl[3], vl[0], vl[3], vl[3]); }
        vector<T, 4> wyxx() const { return vector<T, 4>(vl[3], vl[1], vl[0], vl[0]); }
        vector<T, 4> agrr() const { return vector<T, 4>(vl[3], vl[1], vl[0], vl[0]); }
        vector<T, 4> qtss() const { return vector<T, 4>(vl[3], vl[1], vl[0], vl[0]); }
        vector<T, 4> wyxy() const { return vector<T, 4>(vl[3], vl[1], vl[0], vl[1]); }
        vector<T, 4> agrg() const { return vector<T, 4>(vl[3], vl[1], vl[0], vl[1]); }
        vector<T, 4> qtst() const { return vector<T, 4>(vl[3], vl[1], vl[0], vl[1]); }
        vector<T, 4> wyxz() const { return vector<T, 4>(vl[3], vl[1], vl[0], vl[2]); }
        vector<T, 4> agrb() const { return vector<T, 4>(vl[3], vl[1], vl[0], vl[2]); }
        vector<T, 4> qtsp() const { return vector<T, 4>(vl[3], vl[1], vl[0], vl[2]); }
        vector<T, 4> wyxw() const { return vector<T, 4>(vl[3], vl[1], vl[0], vl[3]); }
        vector<T, 4> agra() const { return vector<T, 4>(vl[3], vl[1], vl[0], vl[3]); }
        vector<T, 4> qtsq() const { return vector<T, 4>(vl[3], vl[1], vl[0], vl[3]); }
        vector<T, 4> wyyx() const { return vector<T, 4>(vl[3], vl[1], vl[1], vl[0]); }
        vector<T, 4> aggr() const { return vector<T, 4>(vl[3], vl[1], vl[1], vl[0]); }
        vector<T, 4> qtts() const { return vector<T, 4>(vl[3], vl[1], vl[1], vl[0]); }
        vector<T, 4> wyyy() const { return vector<T, 4>(vl[3], vl[1], vl[1], vl[1]); }
        vector<T, 4> aggg() const { return vector<T, 4>(vl[3], vl[1], vl[1], vl[1]); }
        vector<T, 4> qttt() const { return vector<T, 4>(vl[3], vl[1], vl[1], vl[1]); }
        vector<T, 4> wyyz() const { return vector<T, 4>(vl[3], vl[1], vl[1], vl[2]); }
        vector<T, 4> aggb() const { return vector<T, 4>(vl[3], vl[1], vl[1], vl[2]); }
        vector<T, 4> qttp() const { return vector<T, 4>(vl[3], vl[1], vl[1], vl[2]); }
        vector<T, 4> wyyw() const { return vector<T, 4>(vl[3], vl[1], vl[1], vl[3]); }
        vector<T, 4> agga() const { return vector<T, 4>(vl[3], vl[1], vl[1], vl[3]); }
        vector<T, 4> qttq() const { return vector<T, 4>(vl[3], vl[1], vl[1], vl[3]); }
        vector<T, 4> wyzx() const { return vector<T, 4>(vl[3], vl[1], vl[2], vl[0]); }
        vector<T, 4> agbr() const { return vector<T, 4>(vl[3], vl[1], vl[2], vl[0]); }
        vector<T, 4> qtps() const { return vector<T, 4>(vl[3], vl[1], vl[2], vl[0]); }
        vector<T, 4> wyzy() const { return vector<T, 4>(vl[3], vl[1], vl[2], vl[1]); }
        vector<T, 4> agbg() const { return vector<T, 4>(vl[3], vl[1], vl[2], vl[1]); }
        vector<T, 4> qtpt() const { return vector<T, 4>(vl[3], vl[1], vl[2], vl[1]); }
        vector<T, 4> wyzz() const { return vector<T, 4>(vl[3], vl[1], vl[2], vl[2]); }
        vector<T, 4> agbb() const { return vector<T, 4>(vl[3], vl[1], vl[2], vl[2]); }
        vector<T, 4> qtpp() const { return vector<T, 4>(vl[3], vl[1], vl[2], vl[2]); }
        vector<T, 4> wyzw() const { return vector<T, 4>(vl[3], vl[1], vl[2], vl[3]); }
        vector<T, 4> agba() const { return vector<T, 4>(vl[3], vl[1], vl[2], vl[3]); }
        vector<T, 4> qtpq() const { return vector<T, 4>(vl[3], vl[1], vl[2], vl[3]); }
        vector<T, 4> wywx() const { return vector<T, 4>(vl[3], vl[1], vl[3], vl[0]); }
        vector<T, 4> agar() const { return vector<T, 4>(vl[3], vl[1], vl[3], vl[0]); }
        vector<T, 4> qtqs() const { return vector<T, 4>(vl[3], vl[1], vl[3], vl[0]); }
        vector<T, 4> wywy() const { return vector<T, 4>(vl[3], vl[1], vl[3], vl[1]); }
        vector<T, 4> agag() const { return vector<T, 4>(vl[3], vl[1], vl[3], vl[1]); }
        vector<T, 4> qtqt() const { return vector<T, 4>(vl[3], vl[1], vl[3], vl[1]); }
        vector<T, 4> wywz() const { return vector<T, 4>(vl[3], vl[1], vl[3], vl[2]); }
        vector<T, 4> agab() const { return vector<T, 4>(vl[3], vl[1], vl[3], vl[2]); }
        vector<T, 4> qtqp() const { return vector<T, 4>(vl[3], vl[1], vl[3], vl[2]); }
        vector<T, 4> wyww() const { return vector<T, 4>(vl[3], vl[1], vl[3], vl[3]); }
        vector<T, 4> agaa() const { return vector<T, 4>(vl[3], vl[1], vl[3], vl[3]); }
        vector<T, 4> qtqq() const { return vector<T, 4>(vl[3], vl[1], vl[3], vl[3]); }
        vector<T, 4> wzxx() const { return vector<T, 4>(vl[3], vl[2], vl[0], vl[0]); }
        vector<T, 4> abrr() const { return vector<T, 4>(vl[3], vl[2], vl[0], vl[0]); }
        vector<T, 4> qpss() const { return vector<T, 4>(vl[3], vl[2], vl[0], vl[0]); }
        vector<T, 4> wzxy() const { return vector<T, 4>(vl[3], vl[2], vl[0], vl[1]); }
        vector<T, 4> abrg() const { return vector<T, 4>(vl[3], vl[2], vl[0], vl[1]); }
        vector<T, 4> qpst() const { return vector<T, 4>(vl[3], vl[2], vl[0], vl[1]); }
        vector<T, 4> wzxz() const { return vector<T, 4>(vl[3], vl[2], vl[0], vl[2]); }
        vector<T, 4> abrb() const { return vector<T, 4>(vl[3], vl[2], vl[0], vl[2]); }
        vector<T, 4> qpsp() const { return vector<T, 4>(vl[3], vl[2], vl[0], vl[2]); }
        vector<T, 4> wzxw() const { return vector<T, 4>(vl[3], vl[2], vl[0], vl[3]); }
        vector<T, 4> abra() const { return vector<T, 4>(vl[3], vl[2], vl[0], vl[3]); }
        vector<T, 4> qpsq() const { return vector<T, 4>(vl[3], vl[2], vl[0], vl[3]); }
        vector<T, 4> wzyx() const { return vector<T, 4>(vl[3], vl[2], vl[1], vl[0]); }
        vector<T, 4> abgr() const { return vector<T, 4>(vl[3], vl[2], vl[1], vl[0]); }
        vector<T, 4> qpts() const { return vector<T, 4>(vl[3], vl[2], vl[1], vl[0]); }
        vector<T, 4> wzyy() const { return vector<T, 4>(vl[3], vl[2], vl[1], vl[1]); }
        vector<T, 4> abgg() const { return vector<T, 4>(vl[3], vl[2], vl[1], vl[1]); }
        vector<T, 4> qptt() const { return vector<T, 4>(vl[3], vl[2], vl[1], vl[1]); }
        vector<T, 4> wzyz() const { return vector<T, 4>(vl[3], vl[2], vl[1], vl[2]); }
        vector<T, 4> abgb() const { return vector<T, 4>(vl[3], vl[2], vl[1], vl[2]); }
        vector<T, 4> qptp() const { return vector<T, 4>(vl[3], vl[2], vl[1], vl[2]); }
        vector<T, 4> wzyw() const { return vector<T, 4>(vl[3], vl[2], vl[1], vl[3]); }
        vector<T, 4> abga() const { return vector<T, 4>(vl[3], vl[2], vl[1], vl[3]); }
        vector<T, 4> qptq() const { return vector<T, 4>(vl[3], vl[2], vl[1], vl[3]); }
        vector<T, 4> wzzx() const { return vector<T, 4>(vl[3], vl[2], vl[2], vl[0]); }
        vector<T, 4> abbr() const { return vector<T, 4>(vl[3], vl[2], vl[2], vl[0]); }
        vector<T, 4> qpps() const { return vector<T, 4>(vl[3], vl[2], vl[2], vl[0]); }
        vector<T, 4> wzzy() const { return vector<T, 4>(vl[3], vl[2], vl[2], vl[1]); }
        vector<T, 4> abbg() const { return vector<T, 4>(vl[3], vl[2], vl[2], vl[1]); }
        vector<T, 4> qppt() const { return vector<T, 4>(vl[3], vl[2], vl[2], vl[1]); }
        vector<T, 4> wzzz() const { return vector<T, 4>(vl[3], vl[2], vl[2], vl[2]); }
        vector<T, 4> abbb() const { return vector<T, 4>(vl[3], vl[2], vl[2], vl[2]); }
        vector<T, 4> qppp() const { return vector<T, 4>(vl[3], vl[2], vl[2], vl[2]); }
        vector<T, 4> wzzw() const { return vector<T, 4>(vl[3], vl[2], vl[2], vl[3]); }
        vector<T, 4> abba() const { return vector<T, 4>(vl[3], vl[2], vl[2], vl[3]); }
        vector<T, 4> qppq() const { return vector<T, 4>(vl[3], vl[2], vl[2], vl[3]); }
        vector<T, 4> wzwx() const { return vector<T, 4>(vl[3], vl[2], vl[3], vl[0]); }
        vector<T, 4> abar() const { return vector<T, 4>(vl[3], vl[2], vl[3], vl[0]); }
        vector<T, 4> qpqs() const { return vector<T, 4>(vl[3], vl[2], vl[3], vl[0]); }
        vector<T, 4> wzwy() const { return vector<T, 4>(vl[3], vl[2], vl[3], vl[1]); }
        vector<T, 4> abag() const { return vector<T, 4>(vl[3], vl[2], vl[3], vl[1]); }
        vector<T, 4> qpqt() const { return vector<T, 4>(vl[3], vl[2], vl[3], vl[1]); }
        vector<T, 4> wzwz() const { return vector<T, 4>(vl[3], vl[2], vl[3], vl[2]); }
        vector<T, 4> abab() const { return vector<T, 4>(vl[3], vl[2], vl[3], vl[2]); }
        vector<T, 4> qpqp() const { return vector<T, 4>(vl[3], vl[2], vl[3], vl[2]); }
        vector<T, 4> wzww() const { return vector<T, 4>(vl[3], vl[2], vl[3], vl[3]); }
        vector<T, 4> abaa() const { return vector<T, 4>(vl[3], vl[2], vl[3], vl[3]); }
        vector<T, 4> qpqq() const { return vector<T, 4>(vl[3], vl[2], vl[3], vl[3]); }
        vector<T, 4> wwxx() const { return vector<T, 4>(vl[3], vl[3], vl[0], vl[0]); }
        vector<T, 4> aarr() const { return vector<T, 4>(vl[3], vl[3], vl[0], vl[0]); }
        vector<T, 4> qqss() const { return vector<T, 4>(vl[3], vl[3], vl[0], vl[0]); }
        vector<T, 4> wwxy() const { return vector<T, 4>(vl[3], vl[3], vl[0], vl[1]); }
        vector<T, 4> aarg() const { return vector<T, 4>(vl[3], vl[3], vl[0], vl[1]); }
        vector<T, 4> qqst() const { return vector<T, 4>(vl[3], vl[3], vl[0], vl[1]); }
        vector<T, 4> wwxz() const { return vector<T, 4>(vl[3], vl[3], vl[0], vl[2]); }
        vector<T, 4> aarb() const { return vector<T, 4>(vl[3], vl[3], vl[0], vl[2]); }
        vector<T, 4> qqsp() const { return vector<T, 4>(vl[3], vl[3], vl[0], vl[2]); }
        vector<T, 4> wwxw() const { return vector<T, 4>(vl[3], vl[3], vl[0], vl[3]); }
        vector<T, 4> aara() const { return vector<T, 4>(vl[3], vl[3], vl[0], vl[3]); }
        vector<T, 4> qqsq() const { return vector<T, 4>(vl[3], vl[3], vl[0], vl[3]); }
        vector<T, 4> wwyx() const { return vector<T, 4>(vl[3], vl[3], vl[1], vl[0]); }
        vector<T, 4> aagr() const { return vector<T, 4>(vl[3], vl[3], vl[1], vl[0]); }
        vector<T, 4> qqts() const { return vector<T, 4>(vl[3], vl[3], vl[1], vl[0]); }
        vector<T, 4> wwyy() const { return vector<T, 4>(vl[3], vl[3], vl[1], vl[1]); }
        vector<T, 4> aagg() const { return vector<T, 4>(vl[3], vl[3], vl[1], vl[1]); }
        vector<T, 4> qqtt() const { return vector<T, 4>(vl[3], vl[3], vl[1], vl[1]); }
        vector<T, 4> wwyz() const { return vector<T, 4>(vl[3], vl[3], vl[1], vl[2]); }
        vector<T, 4> aagb() const { return vector<T, 4>(vl[3], vl[3], vl[1], vl[2]); }
        vector<T, 4> qqtp() const { return vector<T, 4>(vl[3], vl[3], vl[1], vl[2]); }
        vector<T, 4> wwyw() const { return vector<T, 4>(vl[3], vl[3], vl[1], vl[3]); }
        vector<T, 4> aaga() const { return vector<T, 4>(vl[3], vl[3], vl[1], vl[3]); }
        vector<T, 4> qqtq() const { return vector<T, 4>(vl[3], vl[3], vl[1], vl[3]); }
        vector<T, 4> wwzx() const { return vector<T, 4>(vl[3], vl[3], vl[2], vl[0]); }
        vector<T, 4> aabr() const { return vector<T, 4>(vl[3], vl[3], vl[2], vl[0]); }
        vector<T, 4> qqps() const { return vector<T, 4>(vl[3], vl[3], vl[2], vl[0]); }
        vector<T, 4> wwzy() const { return vector<T, 4>(vl[3], vl[3], vl[2], vl[1]); }
        vector<T, 4> aabg() const { return vector<T, 4>(vl[3], vl[3], vl[2], vl[1]); }
        vector<T, 4> qqpt() const { return vector<T, 4>(vl[3], vl[3], vl[2], vl[1]); }
        vector<T, 4> wwzz() const { return vector<T, 4>(vl[3], vl[3], vl[2], vl[2]); }
        vector<T, 4> aabb() const { return vector<T, 4>(vl[3], vl[3], vl[2], vl[2]); }
        vector<T, 4> qqpp() const { return vector<T, 4>(vl[3], vl[3], vl[2], vl[2]); }
        vector<T, 4> wwzw() const { return vector<T, 4>(vl[3], vl[3], vl[2], vl[3]); }
        vector<T, 4> aaba() const { return vector<T, 4>(vl[3], vl[3], vl[2], vl[3]); }
        vector<T, 4> qqpq() const { return vector<T, 4>(vl[3], vl[3], vl[2], vl[3]); }
        vector<T, 4> wwwx() const { return vector<T, 4>(vl[3], vl[3], vl[3], vl[0]); }
        vector<T, 4> aaar() const { return vector<T, 4>(vl[3], vl[3], vl[3], vl[0]); }
        vector<T, 4> qqqs() const { return vector<T, 4>(vl[3], vl[3], vl[3], vl[0]); }
        vector<T, 4> wwwy() const { return vector<T, 4>(vl[3], vl[3], vl[3], vl[1]); }
        vector<T, 4> aaag() const { return vector<T, 4>(vl[3], vl[3], vl[3], vl[1]); }
        vector<T, 4> qqqt() const { return vector<T, 4>(vl[3], vl[3], vl[3], vl[1]); }
        vector<T, 4> wwwz() const { return vector<T, 4>(vl[3], vl[3], vl[3], vl[2]); }
        vector<T, 4> aaab() const { return vector<T, 4>(vl[3], vl[3], vl[3], vl[2]); }
        vector<T, 4> qqqp() const { return vector<T, 4>(vl[3], vl[3], vl[3], vl[2]); }
        vector<T, 4> wwww() const { return vector<T, 4>(vl[3], vl[3], vl[3], vl[3]); }
        vector<T, 4> aaaa() const { return vector<T, 4>(vl[3], vl[3], vl[3], vl[3]); }
        vector<T, 4> qqqq() const { return vector<T, 4>(vl[3], vl[3], vl[3], vl[3]); }
        swizzler<T, 2> xy() { return swizzler<T, 2>(vl[0], vl[1]); }
        swizzler<T, 2> rg() { return swizzler<T, 2>(vl[0], vl[1]); }
        swizzler<T, 2> st() { return swizzler<T, 2>(vl[0], vl[1]); }
        swizzler<T, 2> xz() { return swizzler<T, 2>(vl[0], vl[2]); }
        swizzler<T, 2> rb() { return swizzler<T, 2>(vl[0], vl[2]); }
        swizzler<T, 2> sp() { return swizzler<T, 2>(vl[0], vl[2]); }
        swizzler<T, 2> xw() { return swizzler<T, 2>(vl[0], vl[3]); }
        swizzler<T, 2> ra() { return swizzler<T, 2>(vl[0], vl[3]); }
        swizzler<T, 2> sq() { return swizzler<T, 2>(vl[0], vl[3]); }
        swizzler<T, 2> yx() { return swizzler<T, 2>(vl[1], vl[0]); }
        swizzler<T, 2> gr() { return swizzler<T, 2>(vl[1], vl[0]); }
        swizzler<T, 2> ts() { return swizzler<T, 2>(vl[1], vl[0]); }
        swizzler<T, 2> yz() { return swizzler<T, 2>(vl[1], vl[2]); }
        swizzler<T, 2> gb() { return swizzler<T, 2>(vl[1], vl[2]); }
        swizzler<T, 2> tp() { return swizzler<T, 2>(vl[1], vl[2]); }
        swizzler<T, 2> yw() { return swizzler<T, 2>(vl[1], vl[3]); }
        swizzler<T, 2> ga() { return swizzler<T, 2>(vl[1], vl[3]); }
        swizzler<T, 2> tq() { return swizzler<T, 2>(vl[1], vl[3]); }
        swizzler<T, 2> zx() { return swizzler<T, 2>(vl[2], vl[0]); }
        swizzler<T, 2> br() { return swizzler<T, 2>(vl[2], vl[0]); }
        swizzler<T, 2> ps() { return swizzler<T, 2>(vl[2], vl[0]); }
        swizzler<T, 2> zy() { return swizzler<T, 2>(vl[2], vl[1]); }
        swizzler<T, 2> bg() { return swizzler<T, 2>(vl[2], vl[1]); }
        swizzler<T, 2> pt() { return swizzler<T, 2>(vl[2], vl[1]); }
        swizzler<T, 2> zw() { return swizzler<T, 2>(vl[2], vl[3]); }
        swizzler<T, 2> ba() { return swizzler<T, 2>(vl[2], vl[3]); }
        swizzler<T, 2> pq() { return swizzler<T, 2>(vl[2], vl[3]); }
        swizzler<T, 2> wx() { return swizzler<T, 2>(vl[3], vl[0]); }
        swizzler<T, 2> ar() { return swizzler<T, 2>(vl[3], vl[0]); }
        swizzler<T, 2> qs() { return swizzler<T, 2>(vl[3], vl[0]); }
        swizzler<T, 2> wy() { return swizzler<T, 2>(vl[3], vl[1]); }
        swizzler<T, 2> ag() { return swizzler<T, 2>(vl[3], vl[1]); }
        swizzler<T, 2> qt() { return swizzler<T, 2>(vl[3], vl[1]); }
        swizzler<T, 2> wz() { return swizzler<T, 2>(vl[3], vl[2]); }
        swizzler<T, 2> ab() { return swizzler<T, 2>(vl[3], vl[2]); }
        swizzler<T, 2> qp() { return swizzler<T, 2>(vl[3], vl[2]); }
        swizzler<T, 3> xyz() { return swizzler<T, 3>(vl[0], vl[1], vl[2]); }
        swizzler<T, 3> rgb() { return swizzler<T, 3>(vl[0], vl[1], vl[2]); }
        swizzler<T, 3> stp() { return swizzler<T, 3>(vl[0], vl[1], vl[2]); }
        swizzler<T, 3> xyw() { return swizzler<T, 3>(vl[0], vl[1], vl[3]); }
        swizzler<T, 3> rga() { return swizzler<T, 3>(vl[0], vl[1], vl[3]); }
        swizzler<T, 3> stq() { return swizzler<T, 3>(vl[0], vl[1], vl[3]); }
        swizzler<T, 3> xzy() { return swizzler<T, 3>(vl[0], vl[2], vl[1]); }
        swizzler<T, 3> rbg() { return swizzler<T, 3>(vl[0], vl[2], vl[1]); }
        swizzler<T, 3> spt() { return swizzler<T, 3>(vl[0], vl[2], vl[1]); }
        swizzler<T, 3> xzw() { return swizzler<T, 3>(vl[0], vl[2], vl[3]); }
        swizzler<T, 3> rba() { return swizzler<T, 3>(vl[0], vl[2], vl[3]); }
        swizzler<T, 3> spq() { return swizzler<T, 3>(vl[0], vl[2], vl[3]); }
        swizzler<T, 3> xwy() { return swizzler<T, 3>(vl[0], vl[3], vl[1]); }
        swizzler<T, 3> rag() { return swizzler<T, 3>(vl[0], vl[3], vl[1]); }
        swizzler<T, 3> sqt() { return swizzler<T, 3>(vl[0], vl[3], vl[1]); }
        swizzler<T, 3> xwz() { return swizzler<T, 3>(vl[0], vl[3], vl[2]); }
        swizzler<T, 3> rab() { return swizzler<T, 3>(vl[0], vl[3], vl[2]); }
        swizzler<T, 3> sqp() { return swizzler<T, 3>(vl[0], vl[3], vl[2]); }
        swizzler<T, 3> yxz() { return swizzler<T, 3>(vl[1], vl[0], vl[2]); }
        swizzler<T, 3> grb() { return swizzler<T, 3>(vl[1], vl[0], vl[2]); }
        swizzler<T, 3> tsp() { return swizzler<T, 3>(vl[1], vl[0], vl[2]); }
        swizzler<T, 3> yxw() { return swizzler<T, 3>(vl[1], vl[0], vl[3]); }
        swizzler<T, 3> gra() { return swizzler<T, 3>(vl[1], vl[0], vl[3]); }
        swizzler<T, 3> tsq() { return swizzler<T, 3>(vl[1], vl[0], vl[3]); }
        swizzler<T, 3> yzx() { return swizzler<T, 3>(vl[1], vl[2], vl[0]); }
        swizzler<T, 3> gbr() { return swizzler<T, 3>(vl[1], vl[2], vl[0]); }
        swizzler<T, 3> tps() { return swizzler<T, 3>(vl[1], vl[2], vl[0]); }
        swizzler<T, 3> yzw() { return swizzler<T, 3>(vl[1], vl[2], vl[3]); }
        swizzler<T, 3> gba() { return swizzler<T, 3>(vl[1], vl[2], vl[3]); }
        swizzler<T, 3> tpq() { return swizzler<T, 3>(vl[1], vl[2], vl[3]); }
        swizzler<T, 3> ywx() { return swizzler<T, 3>(vl[1], vl[3], vl[0]); }
        swizzler<T, 3> gar() { return swizzler<T, 3>(vl[1], vl[3], vl[0]); }
        swizzler<T, 3> tqs() { return swizzler<T, 3>(vl[1], vl[3], vl[0]); }
        swizzler<T, 3> ywz() { return swizzler<T, 3>(vl[1], vl[3], vl[2]); }
        swizzler<T, 3> gab() { return swizzler<T, 3>(vl[1], vl[3], vl[2]); }
        swizzler<T, 3> tqp() { return swizzler<T, 3>(vl[1], vl[3], vl[2]); }
        swizzler<T, 3> zxy() { return swizzler<T, 3>(vl[2], vl[0], vl[1]); }
        swizzler<T, 3> brg() { return swizzler<T, 3>(vl[2], vl[0], vl[1]); }
        swizzler<T, 3> pst() { return swizzler<T, 3>(vl[2], vl[0], vl[1]); }
        swizzler<T, 3> zxw() { return swizzler<T, 3>(vl[2], vl[0], vl[3]); }
        swizzler<T, 3> bra() { return swizzler<T, 3>(vl[2], vl[0], vl[3]); }
        swizzler<T, 3> psq() { return swizzler<T, 3>(vl[2], vl[0], vl[3]); }
        swizzler<T, 3> zyx() { return swizzler<T, 3>(vl[2], vl[1], vl[0]); }
        swizzler<T, 3> bgr() { return swizzler<T, 3>(vl[2], vl[1], vl[0]); }
        swizzler<T, 3> pts() { return swizzler<T, 3>(vl[2], vl[1], vl[0]); }
        swizzler<T, 3> zyw() { return swizzler<T, 3>(vl[2], vl[1], vl[3]); }
        swizzler<T, 3> bga() { return swizzler<T, 3>(vl[2], vl[1], vl[3]); }
        swizzler<T, 3> ptq() { return swizzler<T, 3>(vl[2], vl[1], vl[3]); }
        swizzler<T, 3> zwx() { return swizzler<T, 3>(vl[2], vl[3], vl[0]); }
        swizzler<T, 3> bar() { return swizzler<T, 3>(vl[2], vl[3], vl[0]); }
        swizzler<T, 3> pqs() { return swizzler<T, 3>(vl[2], vl[3], vl[0]); }
        swizzler<T, 3> zwy() { return swizzler<T, 3>(vl[2], vl[3], vl[1]); }
        swizzler<T, 3> bag() { return swizzler<T, 3>(vl[2], vl[3], vl[1]); }
        swizzler<T, 3> pqt() { return swizzler<T, 3>(vl[2], vl[3], vl[1]); }
        swizzler<T, 3> wxy() { return swizzler<T, 3>(vl[3], vl[0], vl[1]); }
        swizzler<T, 3> arg() { return swizzler<T, 3>(vl[3], vl[0], vl[1]); }
        swizzler<T, 3> qst() { return swizzler<T, 3>(vl[3], vl[0], vl[1]); }
        swizzler<T, 3> wxz() { return swizzler<T, 3>(vl[3], vl[0], vl[2]); }
        swizzler<T, 3> arb() { return swizzler<T, 3>(vl[3], vl[0], vl[2]); }
        swizzler<T, 3> qsp() { return swizzler<T, 3>(vl[3], vl[0], vl[2]); }
        swizzler<T, 3> wyx() { return swizzler<T, 3>(vl[3], vl[1], vl[0]); }
        swizzler<T, 3> agr() { return swizzler<T, 3>(vl[3], vl[1], vl[0]); }
        swizzler<T, 3> qts() { return swizzler<T, 3>(vl[3], vl[1], vl[0]); }
        swizzler<T, 3> wyz() { return swizzler<T, 3>(vl[3], vl[1], vl[2]); }
        swizzler<T, 3> agb() { return swizzler<T, 3>(vl[3], vl[1], vl[2]); }
        swizzler<T, 3> qtp() { return swizzler<T, 3>(vl[3], vl[1], vl[2]); }
        swizzler<T, 3> wzx() { return swizzler<T, 3>(vl[3], vl[2], vl[0]); }
        swizzler<T, 3> abr() { return swizzler<T, 3>(vl[3], vl[2], vl[0]); }
        swizzler<T, 3> qps() { return swizzler<T, 3>(vl[3], vl[2], vl[0]); }
        swizzler<T, 3> wzy() { return swizzler<T, 3>(vl[3], vl[2], vl[1]); }
        swizzler<T, 3> abg() { return swizzler<T, 3>(vl[3], vl[2], vl[1]); }
        swizzler<T, 3> qpt() { return swizzler<T, 3>(vl[3], vl[2], vl[1]); }
        swizzler<T, 4> xyzw() { return swizzler<T, 4>(vl[0], vl[1], vl[2], vl[3]); }
        swizzler<T, 4> rgba() { return swizzler<T, 4>(vl[0], vl[1], vl[2], vl[3]); }
        swizzler<T, 4> stpq() { return swizzler<T, 4>(vl[0], vl[1], vl[2], vl[3]); }
        swizzler<T, 4> xywz() { return swizzler<T, 4>(vl[0], vl[1], vl[3], vl[2]); }
        swizzler<T, 4> rgab() { return swizzler<T, 4>(vl[0], vl[1], vl[3], vl[2]); }
        swizzler<T, 4> stqp() { return swizzler<T, 4>(vl[0], vl[1], vl[3], vl[2]); }
        swizzler<T, 4> xzyw() { return swizzler<T, 4>(vl[0], vl[2], vl[1], vl[3]); }
        swizzler<T, 4> rbga() { return swizzler<T, 4>(vl[0], vl[2], vl[1], vl[3]); }
        swizzler<T, 4> sptq() { return swizzler<T, 4>(vl[0], vl[2], vl[1], vl[3]); }
        swizzler<T, 4> xzwy() { return swizzler<T, 4>(vl[0], vl[2], vl[3], vl[1]); }
        swizzler<T, 4> rbag() { return swizzler<T, 4>(vl[0], vl[2], vl[3], vl[1]); }
        swizzler<T, 4> spqt() { return swizzler<T, 4>(vl[0], vl[2], vl[3], vl[1]); }
        swizzler<T, 4> xwyz() { return swizzler<T, 4>(vl[0], vl[3], vl[1], vl[2]); }
        swizzler<T, 4> ragb() { return swizzler<T, 4>(vl[0], vl[3], vl[1], vl[2]); }
        swizzler<T, 4> sqtp() { return swizzler<T, 4>(vl[0], vl[3], vl[1], vl[2]); }
        swizzler<T, 4> xwzy() { return swizzler<T, 4>(vl[0], vl[3], vl[2], vl[1]); }
        swizzler<T, 4> rabg() { return swizzler<T, 4>(vl[0], vl[3], vl[2], vl[1]); }
        swizzler<T, 4> sqpt() { return swizzler<T, 4>(vl[0], vl[3], vl[2], vl[1]); }
        swizzler<T, 4> yxzw() { return swizzler<T, 4>(vl[1], vl[0], vl[2], vl[3]); }
        swizzler<T, 4> grba() { return swizzler<T, 4>(vl[1], vl[0], vl[2], vl[3]); }
        swizzler<T, 4> tspq() { return swizzler<T, 4>(vl[1], vl[0], vl[2], vl[3]); }
        swizzler<T, 4> yxwz() { return swizzler<T, 4>(vl[1], vl[0], vl[3], vl[2]); }
        swizzler<T, 4> grab() { return swizzler<T, 4>(vl[1], vl[0], vl[3], vl[2]); }
        swizzler<T, 4> tsqp() { return swizzler<T, 4>(vl[1], vl[0], vl[3], vl[2]); }
        swizzler<T, 4> yzxw() { return swizzler<T, 4>(vl[1], vl[2], vl[0], vl[3]); }
        swizzler<T, 4> gbra() { return swizzler<T, 4>(vl[1], vl[2], vl[0], vl[3]); }
        swizzler<T, 4> tpsq() { return swizzler<T, 4>(vl[1], vl[2], vl[0], vl[3]); }
        swizzler<T, 4> yzwx() { return swizzler<T, 4>(vl[1], vl[2], vl[3], vl[0]); }
        swizzler<T, 4> gbar() { return swizzler<T, 4>(vl[1], vl[2], vl[3], vl[0]); }
        swizzler<T, 4> tpqs() { return swizzler<T, 4>(vl[1], vl[2], vl[3], vl[0]); }
        swizzler<T, 4> ywxz() { return swizzler<T, 4>(vl[1], vl[3], vl[0], vl[2]); }
        swizzler<T, 4> garb() { return swizzler<T, 4>(vl[1], vl[3], vl[0], vl[2]); }
        swizzler<T, 4> tqsp() { return swizzler<T, 4>(vl[1], vl[3], vl[0], vl[2]); }
        swizzler<T, 4> ywzx() { return swizzler<T, 4>(vl[1], vl[3], vl[2], vl[0]); }
        swizzler<T, 4> gabr() { return swizzler<T, 4>(vl[1], vl[3], vl[2], vl[0]); }
        swizzler<T, 4> tqps() { return swizzler<T, 4>(vl[1], vl[3], vl[2], vl[0]); }
        swizzler<T, 4> zxyw() { return swizzler<T, 4>(vl[2], vl[0], vl[1], vl[3]); }
        swizzler<T, 4> brga() { return swizzler<T, 4>(vl[2], vl[0], vl[1], vl[3]); }
        swizzler<T, 4> pstq() { return swizzler<T, 4>(vl[2], vl[0], vl[1], vl[3]); }
        swizzler<T, 4> zxwy() { return swizzler<T, 4>(vl[2], vl[0], vl[3], vl[1]); }
        swizzler<T, 4> brag() { return swizzler<T, 4>(vl[2], vl[0], vl[3], vl[1]); }
        swizzler<T, 4> psqt() { return swizzler<T, 4>(vl[2], vl[0], vl[3], vl[1]); }
        swizzler<T, 4> zyxw() { return swizzler<T, 4>(vl[2], vl[1], vl[0], vl[3]); }
        swizzler<T, 4> bgra() { return swizzler<T, 4>(vl[2], vl[1], vl[0], vl[3]); }
        swizzler<T, 4> ptsq() { return swizzler<T, 4>(vl[2], vl[1], vl[0], vl[3]); }
        swizzler<T, 4> zywx() { return swizzler<T, 4>(vl[2], vl[1], vl[3], vl[0]); }
        swizzler<T, 4> bgar() { return swizzler<T, 4>(vl[2], vl[1], vl[3], vl[0]); }
        swizzler<T, 4> ptqs() { return swizzler<T, 4>(vl[2], vl[1], vl[3], vl[0]); }
        swizzler<T, 4> zwxy() { return swizzler<T, 4>(vl[2], vl[3], vl[0], vl[1]); }
        swizzler<T, 4> barg() { return swizzler<T, 4>(vl[2], vl[3], vl[0], vl[1]); }
        swizzler<T, 4> pqst() { return swizzler<T, 4>(vl[2], vl[3], vl[0], vl[1]); }
        swizzler<T, 4> zwyx() { return swizzler<T, 4>(vl[2], vl[3], vl[1], vl[0]); }
        swizzler<T, 4> bagr() { return swizzler<T, 4>(vl[2], vl[3], vl[1], vl[0]); }
        swizzler<T, 4> pqts() { return swizzler<T, 4>(vl[2], vl[3], vl[1], vl[0]); }
        swizzler<T, 4> wxyz() { return swizzler<T, 4>(vl[3], vl[0], vl[1], vl[2]); }
        swizzler<T, 4> argb() { return swizzler<T, 4>(vl[3], vl[0], vl[1], vl[2]); }
        swizzler<T, 4> qstp() { return swizzler<T, 4>(vl[3], vl[0], vl[1], vl[2]); }
        swizzler<T, 4> wxzy() { return swizzler<T, 4>(vl[3], vl[0], vl[2], vl[1]); }
        swizzler<T, 4> arbg() { return swizzler<T, 4>(vl[3], vl[0], vl[2], vl[1]); }
        swizzler<T, 4> qspt() { return swizzler<T, 4>(vl[3], vl[0], vl[2], vl[1]); }
        swizzler<T, 4> wyxz() { return swizzler<T, 4>(vl[3], vl[1], vl[0], vl[2]); }
        swizzler<T, 4> agrb() { return swizzler<T, 4>(vl[3], vl[1], vl[0], vl[2]); }
        swizzler<T, 4> qtsp() { return swizzler<T, 4>(vl[3], vl[1], vl[0], vl[2]); }
        swizzler<T, 4> wyzx() { return swizzler<T, 4>(vl[3], vl[1], vl[2], vl[0]); }
        swizzler<T, 4> agbr() { return swizzler<T, 4>(vl[3], vl[1], vl[2], vl[0]); }
        swizzler<T, 4> qtps() { return swizzler<T, 4>(vl[3], vl[1], vl[2], vl[0]); }
        swizzler<T, 4> wzxy() { return swizzler<T, 4>(vl[3], vl[2], vl[0], vl[1]); }
        swizzler<T, 4> abrg() { return swizzler<T, 4>(vl[3], vl[2], vl[0], vl[1]); }
        swizzler<T, 4> qpst() { return swizzler<T, 4>(vl[3], vl[2], vl[0], vl[1]); }
        swizzler<T, 4> wzyx() { return swizzler<T, 4>(vl[3], vl[2], vl[1], vl[0]); }
        swizzler<T, 4> abgr() { return swizzler<T, 4>(vl[3], vl[2], vl[1], vl[0]); }
        swizzler<T, 4> qpts() { return swizzler<T, 4>(vl[3], vl[2], vl[1], vl[0]); }
    };

    template<typename T, int r> vector<T, r> operator+(const array<T, 1, r>& a, const array<T, 1, r>& b) { return a._op_plus(b); }
    template<typename T, int r> vector<T, r> operator+(const array<T, 1, r>& a) { return a._op_unary_plus(); }
    template<typename T, int r> vector<T, r> operator-(const array<T, 1, r>& a, const array<T, 1, r>& b) { return a._op_minus(b); }
    template<typename T, int r> vector<T, r> operator-(const array<T, 1, r>& a) { return a._op_unary_minus(); }
    template<typename T, int r> vector<T, r> operator*(const array<T, 1, r>& a, const array<T, 1, r>& b) { return a._op_comp_mult(b); }
    template<typename T, int r> vector<T, r> operator*(T s, const array<T, 1, r>& a) { return a._op_scal_mult(s); }
    template<typename T, int r> vector<T, r> operator*(const array<T, 1, r>& a, T s) { return a._op_scal_mult(s); }
    template<typename T, int r> vector<T, r> operator/(const array<T, 1, r>& a, const array<T, 1, r>& b) { return a._op_comp_div(b); }
    template<typename T, int r> vector<T, r> operator/(const array<T, 1, r>& a, T s) { return a._op_scal_div(s); }
    template<typename T, int r> vector<T, r> operator%(const array<T, 1, r>& a, const array<T, 1, r>& b) { return a._op_comp_mod(b); }
    template<typename T, int r> vector<T, r> operator%(const array<T, 1, r>& a, T s) { return a._op_scal_mod(s); }
    template<typename T, int r> bool operator==(const array<T, 1, r>& a, const array<T, 1, r>& b) { return a._op_equal(b); }
    template<typename T, int r> bool operator!=(const array<T, 1, r>& a, const array<T, 1, r>& b) { return a._op_notequal(b); }

    template<typename T, int r> vector<T, r> sin(const array<T, 1, r>& v) { return v._sin(); }
    template<typename T, int r> vector<T, r> cos(const array<T, 1, r>& v) { return v._cos(); }
    template<typename T, int r> vector<T, r> tan(const array<T, 1, r>& v) { return v._tan(); }
    template<typename T, int r> vector<T, r> asin(const array<T, 1, r>& v) { return v._asin(); }
    template<typename T, int r> vector<T, r> acos(const array<T, 1, r>& v) { return v._acos(); }
    template<typename T, int r> vector<T, r> atan(const array<T, 1, r>& v) { return v._atan(); }
    template<typename T, int r> vector<T, r> atan(const array<T, 1, r>& v, const array<T, 1, r>& w) { return v._atan(w); }
    template<typename T, int r> vector<T, r> radians(const array<T, 1, r>& v) { return v._radians(); }
    template<typename T, int r> vector<T, r> degrees(const array<T, 1, r>& v) { return v._degrees(); }

    template<typename T, int r> vector<T, r> pow(const array<T, 1, r>& v, T p) { return v._pow(p); }
    template<typename T, int r> vector<T, r> exp(const array<T, 1, r>& v) { return v._exp(); }
    template<typename T, int r> vector<T, r> exp2(const array<T, 1, r>& v) { return v._exp2(); }
    template<typename T, int r> vector<T, r> log(const array<T, 1, r>& v) { return v._log(); }
    template<typename T, int r> vector<T, r> log2(const array<T, 1, r>& v) { return v._log2(); }
    template<typename T, int r> vector<T, r> log10(const array<T, 1, r>& v) { return v._log10(); }
    template<typename T, int r> vector<T, r> sqrt(const array<T, 1, r>& v) { return v._sqrt(); }
    template<typename T, int r> vector<T, r> inversesqrt(const array<T, 1, r>& v) { return v._inversesqrt(); }
    template<typename T, int r> vector<T, r> cbrt(const array<T, 1, r>& v) { return v._cbrt(); }

    template<typename T, int r> vector<bool, r> isfinite(const array<T, 1, r>& v) { return v._isfinite(); }
    template<typename T, int r> vector<bool, r> isinf(const array<T, 1, r>& v) { return v._isinf(); }
    template<typename T, int r> vector<bool, r> isnan(const array<T, 1, r>& v) { return v._isnan(); }
    template<typename T, int r> vector<bool, r> isnormal(const array<T, 1, r>& v) { return v._isnormal(); }

    template<typename T, int r> vector<T, r> abs(const array<T, 1, r>& v) { return v._abs(); }
    template<typename T, int r> vector<T, r> sign(const array<T, 1, r>& v) { return v._sign(); }
    template<typename T, int r> vector<T, r> floor(const array<T, 1, r>& v) { return v._floor(); }
    template<typename T, int r> vector<T, r> ceil(const array<T, 1, r>& v) { return v._ceil(); }
    template<typename T, int r> vector<T, r> round(const array<T, 1, r>& v) { return v._round(); }
    template<typename T, int r> vector<T, r> fract(const array<T, 1, r>& v) { return v._fract(); }

    template<typename T, int r> vector<T, r> min(const array<T, 1, r>& v, T x) { return v._min(x); }
    template<typename T, int r> vector<T, r> min(const array<T, 1, r>& v, const array<T, 1, r>& w) { return v._min(w); }
    template<typename T, int r> vector<T, r> max(const array<T, 1, r>& v, T x) { return v._max(x); }
    template<typename T, int r> vector<T, r> max(const array<T, 1, r>& v, const array<T, 1, r>& w) { return v._max(w); }

    template<typename T, int r> vector<T, r> clamp(const array<T, 1, r>& v, T a, T b) { return v._clamp(a, b); }
    template<typename T, int r> vector<T, r> clamp(const array<T, 1, r>& v, const array<T, 1, r>& a, const array<T, 1, r>& b) { return v._clamp(a, b); }
    template<typename T, int r> vector<T, r> mix(const array<T, 1, r>& v, const array<T, 1, r>& w, T a) { return v._mix(w, a); }
    template<typename T, int r> vector<T, r> mix(const array<T, 1, r>& v, const array<T, 1, r>& w, const array<T, 1, r>& a) { return v._mix(w, a); }
    template<typename T, int r> vector<T, r> step(const array<T, 1, r>& v, T a) { return v._step(a); }
    template<typename T, int r> vector<T, r> step(const array<T, 1, r>& v, const array<T, 1, r>& a) { return v._step(a); }
    template<typename T, int r> vector<T, r> smoothstep(const array<T, 1, r>& v, T e0, T e1) { return v._smoothstep(e0, e1); }
    template<typename T, int r> vector<T, r> smoothstep(const array<T, 1, r>& v, const array<T, 1, r>& e0, const array<T, 1, r>& e1) { return v._smoothstep(e0, e1); }
    template<typename T, int r> vector<T, r> mod(const array<T, 1, r>& v, T x) { return v._mod(x); }
    template<typename T, int r> vector<T, r> mod(const array<T, 1, r>& v, const array<T, 1, r>& w) { return v._mod(w); }

    template<typename T, int r> vector<bool, r> greaterThan(const array<T, 1, r>& v, const array<T, 1, r>& w) { return v._greaterThan(w); }
    template<typename T, int r> vector<bool, r> greaterThanEqual(const array<T, 1, r>& v, const array<T, 1, r>& w) { return v._greaterThanEqual(w); }
    template<typename T, int r> vector<bool, r> lessThan(const array<T, 1, r>& v, const array<T, 1, r>& w) { return v._lessThan(w); }
    template<typename T, int r> vector<bool, r> lessThanEqual(const array<T, 1, r>& v, const array<T, 1, r>& w) { return v._lessThanEqual(w); }
    template<typename T, int r> vector<bool, r> equal(const array<T, 1, r>& v, const array<T, 1, r>& w) { return v._equal(w); }
    template<typename T, int r> vector<bool, r> equal(const array<T, 1, r>& v, const array<T, 1, r>& w, int max_ulps) { return v._equal(w, max_ulps); }
    template<typename T, int r> vector<bool, r> notEqual(const array<T, 1, r>& v, const array<T, 1, r>& w) { return v._notEqual(w); }
    template<typename T, int r> vector<bool, r> notEqual(const array<T, 1, r>& v, const array<T, 1, r>& w, int max_ulps) { return v._notEqual(w, max_ulps); }
    template<int r> bool any(const array<bool, 1, r>& v) { return v._any(); }
    template<int r> bool all(const array<bool, 1, r>& v) { return v._all(); }
    template<int r> bool negate(const array<bool, 1, r>& v) { return v._negate(); }

    template<typename T, int r> T length(const array<T, 1, r>& v) { return v._length(); }
    template<typename T, int r> T distance(const array<T, 1, r>& v, const array<T, 1, r>& w) { return v._distance(w); }
    template<typename T, int r> T dot(const array<T, 1, r>& v, const array<T, 1, r>& w) { return v._dot(w); }
    template<typename T, int r> vector<T, r> normalize(const array<T, 1, r>& v) { return v._normalize(); }

    template<typename T, int r> vector<T, r> faceforward(const array<T, 1, r>& v, const array<T, 1, r>& I, const array<T, 1, r>& Nref) { return v._faceforward(I, Nref); }
    template<typename T, int r> vector<T, r> reflect(const array<T, 1, r>& I, const array<T, 1, r>& N) { return I._reflect(N); }
    template<typename T, int r> vector<T, r> refract(const array<T, 1, r>& I, const array<T, 1, r>& N, T eta) { return I._refract(N, eta); }

    template<typename T> vector<T, 3> cross(const array<T, 1, 3>& v, const array<T, 1, 3>& w)
    {
        return vector<T, 3>(
                v.vl[1] * w.vl[2] - v.vl[2] * w.vl[1],
                v.vl[2] * w.vl[0] - v.vl[0] * w.vl[2],
                v.vl[0] * w.vl[1] - v.vl[1] * w.vl[0]);
    }

    template<typename T, int r> vector<bool, r> is_pow2(const array<T, 1, r>& v) { return v._is_pow2(); }
    template<typename T, int r> vector<T, r> next_pow2(const array<T, 1, r>& v) { return v._next_pow2(); }
    template<typename T, int r> vector<T, r> next_multiple(const array<T, 1, r>& v, const array<T, 1, r>& w) { return v._next_multiple(w); }
    template<typename T, int r> vector<T, r> next_multiple(const array<T, 1, r>& v, T x) { return v._next_multiple(x); }

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

    template<typename T, int cols, int rows>
    class matrix : public array<T, cols, rows>
    {
    public:
        using array<T, cols, rows>::v;
        using array<T, cols, rows>::vl;

        /* Constructors, Destructor */

        matrix() {}
        matrix(const array<T, cols, rows>& a) : array<T, cols, rows>(a) {}
        matrix(T x)
        {
            for (int i = 0; i < cols; i++)
                for (int j = 0; j < rows; j++)
                    array<T, cols, rows>::v[i][j] = (i == j ? x : static_cast<T>(0));
        }
        matrix(T v0, T v1, T v2, T v3)
            : array<T, cols, rows>(v0, v1, v2, v3) {}
        matrix(T v0, T v1, T v2, T v3, T v4, T v5)
            : array<T, cols, rows>(v0, v1, v2, v3, v4, v5) {}
        matrix(T v0, T v1, T v2, T v3, T v4, T v5, T v6, T v7)
            : array<T, cols, rows>(v0, v1, v2, v3, v4, v5, v6, v7) {}
        matrix(T v0, T v1, T v2, T v3, T v4, T v5, T v6, T v7, T v8)
            : array<T, cols, rows>(v0, v1, v2, v3, v4, v5, v6, v7, v8) {}
        matrix(T v0, T v1, T v2, T v3, T v4, T v5, T v6, T v7, T v8, T v9, T v10, T v11)
            : array<T, cols, rows>(v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11) {}
        matrix(T v0, T v1, T v2, T v3, T v4, T v5, T v6, T v7, T v8, T v9, T v10, T v11, T v12, T v13, T v14, T v15)
            : array<T, cols, rows>(v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15) {}

        matrix(const vector<T, rows>& col0, const vector<T, rows>& col1)
        {
            for (int i = 0; i < rows; i++) {
                v[0][i] = col0.vl[i];
                v[1][i] = col1.vl[i];
            }
        }
        matrix(const vector<T, rows>& col0, const vector<T, rows>& col1, const vector<T, rows>& col2)
        {
            for (int i = 0; i < rows; i++) {
                v[0][i] = col0.vl[i];
                v[1][i] = col1.vl[i];
                v[2][i] = col2.vl[i];
            }
        }
        matrix(const vector<T, rows>& col0, const vector<T, rows>& col1, const vector<T, rows>& col2, const vector<T, rows>& col3)
        {
            for (int i = 0; i < rows; i++) {
                v[0][i] = col0.vl[i];
                v[1][i] = col1.vl[i];
                v[2][i] = col2.vl[i];
                v[3][i] = col3.vl[i];
            }
        }

        matrix(const T* a) : array<T, cols, rows>(a) {}
        template<typename U> matrix(const matrix<U, cols, rows>& a) : array<T, cols, rows>(a) {}

        /* Operators */

        swizzler<T, rows> operator[](unsigned int i)
        {
            return swizzler<T, rows>(v[i]);
        }

        vector<T, rows> operator[](unsigned int i) const
        {
            return vector<T, rows>(v[i]);
        }

        const matrix& operator=(const matrix& a)      { this->_op_assign(a); return *this; }
        matrix operator+(const matrix& a) const       { return this->_op_plus(a); }
        const matrix& operator+=(const matrix& a)     { this->_op_plus_assign(a); return *this; }
        matrix operator+() const                      { return array<T, cols, rows>::_op_unary_plus(); }
        matrix operator-(const matrix& a) const       { return this->_op_minus(a); }
        const matrix& operator-=(const matrix& a)     { this->_op_minus_assign(a); return *this; }
        matrix operator-() const                      { return array<T, cols, rows>::_op_unary_minus(); }
        matrix operator*(T s) const                   { return this->_op_scal_mult(s); }
        friend matrix operator*(T s, const matrix& a) { return a._op_scal_mult(s); }
        const matrix& operator*=(T s)                 { this->_op_scal_mult_assign(s); return *this; }
        matrix operator/(T s) const                   { return this->_op_scal_div(s); }
        const matrix& operator/=(T s)                 { this->_op_scal_div_assign(s); return *this; }
        matrix operator%(T s) const                   { return this->_op_scal_mod(s); }
        const matrix& operator%=(T s)                 { this->_op_scal_mod_assign(s); return *this; }
        bool operator==(const matrix& a) const        { return this->_op_equal(a); }
        bool operator!=(const matrix& a) const        { return this->_op_notequal(a); }

        vector<T, rows> operator*(const vector<T, cols>& w) const
        {
            vector<T, rows> r;
            for (int i = 0; i < rows; i++) {
                r.vl[i] = static_cast<T>(0);
                for (int j = 0; j < cols; j++) {
                    r.vl[i] += array<T, cols, rows>::v[j][i] * w.vl[j];
                }
            }
            return r;
        }

        friend vector<T, cols> operator*(const vector<T, rows>& w, const matrix& m)
        {
            vector<T, cols> r;
            for (int i = 0; i < cols; i++) {
                r.vl[i] = static_cast<T>(0);
                for (int j = 0; j < rows; j++) {
                    r.vl[i] += m.v[i][j] * w.vl[j];
                }
            }
            return r;
        }

        matrix<T, rows, rows> operator*(const matrix<T, rows, cols>& n) const
        {
            matrix<T, rows, rows> r;
            for (int i = 0; i < rows; i++) {
                for (int j = 0; j < rows; j++) {
                    r.v[i][j] = static_cast<T>(0);
                    for (int k = 0; k < cols; k++) {
                        r.v[i][j] += array<T, cols, rows>::v[k][j] * n.v[i][k];
                    }
                }
            }
            return r;
        }

        const matrix<T, cols, rows>& operator*=(const matrix<T, cols, rows>& n)
        {
            matrix<T, cols, rows> r = *this * n;
            *this = r;
            return *this;
        }
    };

    template<typename T, int r> vector<T, 2> row(const matrix<T, 2, r>& m, int row) { return vector<T, 2>(m.v[0][row], m.v[1][row]); }
    template<typename T, int r> vector<T, 3> row(const matrix<T, 3, r>& m, int row) { return vector<T, 3>(m.v[0][row], m.v[1][row], m.v[2][row]); }
    template<typename T, int r> vector<T, 4> row(const matrix<T, 4, r>& m, int row) { return vector<T, 4>(m.v[0][row], m.v[1][row], m.v[2][row], m.v[3][row]); }
    template<typename T, int r> swizzler<T, 2> row(matrix<T, 2, r>& m, int row) { return swizzler<T, 2>(m.v[0][row], m.v[1][row]); }
    template<typename T, int r> swizzler<T, 3> row(matrix<T, 3, r>& m, int row) { return swizzler<T, 3>(m.v[0][row], m.v[1][row], m.v[2][row]); }
    template<typename T, int r> swizzler<T, 4> row(matrix<T, 4, r>& m, int row) { return swizzler<T, 4>(m.v[0][row], m.v[1][row], m.v[2][row], m.v[3][row]); }
    template<typename T, int c> vector<T, 2> col(const matrix<T, c, 2>& m, int col) { return vector<T, 2>(m.v[col][0], m.v[col][1]); }
    template<typename T, int c> vector<T, 3> col(const matrix<T, c, 3>& m, int col) { return vector<T, 3>(m.v[col][0], m.v[col][1], m.v[col][2]); }
    template<typename T, int c> vector<T, 4> col(const matrix<T, c, 4>& m, int col) { return vector<T, 4>(m.v[col][0], m.v[col][1], m.v[col][2], m.v[col][3]); }
    template<typename T, int c> swizzler<T, 2> col(matrix<T, c, 2>& m, int col) { return swizzler<T, 2>(m.v[col][0], m.v[col][1]); }
    template<typename T, int c> swizzler<T, 3> col(matrix<T, c, 3>& m, int col) { return swizzler<T, 3>(m.v[col][0], m.v[col][1], m.v[col][2]); }
    template<typename T, int c> swizzler<T, 4> col(matrix<T, c, 4>& m, int col) { return swizzler<T, 4>(m.v[col][0], m.v[col][1], m.v[col][2], m.v[col][3]); }

    template<typename T, int c, int r> matrix<T, c - 1, r - 1> strike(const matrix<T, c, r>& m, int col, int row) { return m._strike(col, row); }

    template<typename T, int c, int r, int rs, int cs> matrix<T, c, r> set(
            const matrix<T, c, r>& m, const matrix<T, cs, rs>& s, int col = 0, int row = 0)
    {
        matrix<T, c, r> result(m);
        result._set_sub_array(s, col, row);
        return result;
    }

    template<typename T, int c, int r> matrix<T, c, r> sin(const matrix<T, c, r>& v) { return v._sin(); }
    template<typename T, int c, int r> matrix<T, c, r> cos(const matrix<T, c, r>& v) { return v._cos(); }
    template<typename T, int c, int r> matrix<T, c, r> tan(const matrix<T, c, r>& v) { return v._tan(); }
    template<typename T, int c, int r> matrix<T, c, r> asin(const matrix<T, c, r>& v) { return v._asin(); }
    template<typename T, int c, int r> matrix<T, c, r> acos(const matrix<T, c, r>& v) { return v._acos(); }
    template<typename T, int c, int r> matrix<T, c, r> atan(const matrix<T, c, r>& v) { return v._atan(); }
    template<typename T, int c, int r> matrix<T, c, r> atan(const matrix<T, c, r>& v, const matrix<T, c, r>& w) { return v._atan(w); }
    template<typename T, int c, int r> matrix<T, c, r> radians(const matrix<T, c, r>& v) { return v._radians(); }
    template<typename T, int c, int r> matrix<T, c, r> degrees(const matrix<T, c, r>& v) { return v._degrees(); }

    template<typename T, int c, int r> matrix<T, c, r> pow(const matrix<T, c, r>& v, T p) { return v._pow(p); }
    template<typename T, int c, int r> matrix<T, c, r> exp(const matrix<T, c, r>& v) { return v._exp(); }
    template<typename T, int c, int r> matrix<T, c, r> exp2(const matrix<T, c, r>& v) { return v._exp2(); }
    template<typename T, int c, int r> matrix<T, c, r> log(const matrix<T, c, r>& v) { return v._log(); }
    template<typename T, int c, int r> matrix<T, c, r> log2(const matrix<T, c, r>& v) { return v._log2(); }
    template<typename T, int c, int r> matrix<T, c, r> log10(const matrix<T, c, r>& v) { return v._log10(); }
    template<typename T, int c, int r> matrix<T, c, r> sqrt(const matrix<T, c, r>& v) { return v._sqrt(); }
    template<typename T, int c, int r> matrix<T, c, r> inversesqrt(const matrix<T, c, r>& v) { return v._inversesqrt(); }
    template<typename T, int c, int r> matrix<T, c, r> cbrt(const matrix<T, c, r>& v) { return v._cbrt(); }

    template<typename T, int c, int r> matrix<bool, c, r> isfinite(const matrix<T, c, r>& v) { return v._isfinite(); }
    template<typename T, int c, int r> matrix<bool, c, r> isinf(const matrix<T, c, r>& v) { return v._isinf(); }
    template<typename T, int c, int r> matrix<bool, c, r> isnan(const matrix<T, c, r>& v) { return v._isnan(); }
    template<typename T, int c, int r> matrix<bool, c, r> isnormal(const matrix<T, c, r>& v) { return v._isnormal(); }

    template<typename T, int c, int r> matrix<T, c, r> abs(const matrix<T, c, r>& v) { return v._abs(); }
    template<typename T, int c, int r> matrix<T, c, r> sign(const matrix<T, c, r>& v) { return v._sign(); }
    template<typename T, int c, int r> matrix<T, c, r> floor(const matrix<T, c, r>& v) { return v._floor(); }
    template<typename T, int c, int r> matrix<T, c, r> ceil(const matrix<T, c, r>& v) { return v._ceil(); }
    template<typename T, int c, int r> matrix<T, c, r> round(const matrix<T, c, r>& v) { return v._round(); }
    template<typename T, int c, int r> matrix<T, c, r> fract(const matrix<T, c, r>& v) { return v._fract(); }

    template<typename T, int c, int r> matrix<T, c, r> min(const matrix<T, c, r>& v, T x) { return v._min(x); }
    template<typename T, int c, int r> matrix<T, c, r> min(const matrix<T, c, r>& v, const matrix<T, c, r>& w) { return v._min(w); }
    template<typename T, int c, int r> matrix<T, c, r> max(const matrix<T, c, r>& v, T x) { return v._max(x); }
    template<typename T, int c, int r> matrix<T, c, r> max(const matrix<T, c, r>& v, const matrix<T, c, r>& w) { return v._max(w); }

    template<typename T, int c, int r> matrix<T, c, r> clamp(const matrix<T, c, r>& v, T a, T b) { return v._clamp(a, b); }
    template<typename T, int c, int r> matrix<T, c, r> clamp(const matrix<T, c, r>& v, const matrix<T, c, r>& a, const matrix<T, c, r>& b) { return v._clamp(a, b); }
    template<typename T, int c, int r> matrix<T, c, r> mix(const matrix<T, c, r>& v, const matrix<T, c, r>& w, T a) { return v._mix(w, a); }
    template<typename T, int c, int r> matrix<T, c, r> mix(const matrix<T, c, r>& v, const matrix<T, c, r>& w, const matrix<T, c, r>& a) { return v._mix(w, a); }
    template<typename T, int c, int r> matrix<T, c, r> step(const matrix<T, c, r>& v, T a) { return v._step(a); }
    template<typename T, int c, int r> matrix<T, c, r> step(const matrix<T, c, r>& v, const matrix<T, c, r>& a) { return v._step(a); }
    template<typename T, int c, int r> matrix<T, c, r> smoothstep(const matrix<T, c, r>& v, T e0, T e1) { return v._smoothstep(e0, e1); }
    template<typename T, int c, int r> matrix<T, c, r> smoothstep(const matrix<T, c, r>& v, const vector<T, 1>& e0, const vector<T, 1>& e1) { return v._smoothstep(e0, e1); }
    template<typename T, int c, int r> matrix<T, c, r> mod(const matrix<T, c, r>& v, T x) { return v._mod(x); }
    template<typename T, int c, int r> matrix<T, c, r> mod(const matrix<T, c, r>& v, const matrix<T, c, r>& w) { return v._mod(w); }

    template<typename T, int c, int r> matrix<bool, c, r> greaterThan(const matrix<T, c, r>& v, const matrix<T, c, r>& w) { return v._greaterThan(w); }
    template<typename T, int c, int r> matrix<bool, c, r> greaterThanEqual(const matrix<T, c, r>& v, const matrix<T, c, r>& w) { return v._greaterThanEqual(w); }
    template<typename T, int c, int r> matrix<bool, c, r> lessThan(const matrix<T, c, r>& v, const matrix<T, c, r>& w) { return v._lessThan(w); }
    template<typename T, int c, int r> matrix<bool, c, r> lessThanEqual(const matrix<T, c, r>& v, const matrix<T, c, r>& w) { return v._lessThanEqual(w); }
    template<typename T, int c, int r> matrix<bool, c, r> equal(const matrix<T, c, r>& v, const matrix<T, c, r>& w) { return v._equal(w); }
    template<typename T, int c, int r> matrix<bool, c, r> equal(const matrix<T, c, r>& v, const matrix<T, c, r>& w, int max_ulps) { return v._equal(w, max_ulps); }
    template<typename T, int c, int r> matrix<bool, c, r> notEqual(const matrix<T, c, r>& v, const matrix<T, c, r>& w) { return v._notEqual(w); }
    template<typename T, int c, int r> matrix<bool, c, r> notEqual(const matrix<T, c, r>& v, const matrix<T, c, r>& w, int max_ulps) { return v._notEqual(w, max_ulps); }
    template<int c, int r> bool any(const matrix<bool, c, r>& v) { return v._any(); }
    template<int c, int r> bool all(const matrix<bool, c, r>& v) { return v._all(); }
    template<int c, int r> bool negate(const matrix<bool, c, r>& v) { return v._negate(); }

    template<typename T, int c, int r> matrix<T, c, r> transpose(const matrix<T, r, c>& m) { return m._transpose(); }

    template<typename T, int c, int r> matrix<T, c, r> matrixCompMult(const matrix<T, c, r>& m, const matrix<T, c, r>& n) { return m._op_comp_mult(n); }

    template<typename T, int c, int r> matrix<T, c, r> outerProduct(const vector<T, r>& v, const vector<T, c>& w)
    {
        matrix<T, c, r> m;
        for (int i = 0; i < c; i++)
            for (int j = 0; j < r; j++)
                m.v[i][j] = v[j] * w[i];
        return m;
    }

    template<typename T> T det(const matrix<T, 2, 2>& m)
    {
        return m.v[0][0] * m.v[1][1] - m.v[1][0] * m.v[0][1];
    }

    template<typename T> bool invertible(const matrix<T, 2, 2>& m, T epsilon = std::numeric_limits<T>::epsilon())
    {
        T d = det(m);
        return (d > epsilon || d < -epsilon);
    }

    template<typename T> matrix<T, 2, 2> inverse(const matrix<T, 2, 2>& m)
    {
        return matrix<T, 2, 2>(m.v[1][1], -m.v[1][0], -m.v[0][1], m.v[0][0]) / det(m);
    }

    template<typename T> T det(const matrix<T, 3, 3>& m)
    {
        return m.v[0][0] * (m.v[1][1] * m.v[2][2] - m.v[1][2] * m.v[2][1])
            + m.v[0][1] * (m.v[1][2] * m.v[2][0] - m.v[1][0] * m.v[2][2])
            + m.v[0][2] * (m.v[1][0] * m.v[2][1] - m.v[1][1] * m.v[2][0]);
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
        I.v[0][0] = m.v[1][1] * m.v[2][2] - m.v[2][1] * m.v[1][2];
        I.v[0][1] = m.v[2][1] * m.v[0][2] - m.v[0][1] * m.v[2][2];
        I.v[0][2] = m.v[0][1] * m.v[1][2] - m.v[1][1] * m.v[0][2];
        I.v[1][0] = m.v[2][0] * m.v[1][2] - m.v[1][0] * m.v[2][2];
        I.v[1][1] = m.v[0][0] * m.v[2][2] - m.v[2][0] * m.v[0][2];
        I.v[1][2] = m.v[1][0] * m.v[0][2] - m.v[0][0] * m.v[1][2];
        I.v[2][0] = m.v[1][0] * m.v[2][1] - m.v[2][0] * m.v[1][1];
        I.v[2][1] = m.v[2][0] * m.v[0][1] - m.v[0][0] * m.v[2][1];
        I.v[2][2] = m.v[0][0] * m.v[1][1] - m.v[1][0] * m.v[0][1];
        T det = m.v[0][0] * I.v[0][0] + m.v[1][0] * I.v[0][1] + m.v[2][0] * I.v[0][2];
        return I / det;
    }

    template<typename T> T det(const matrix<T, 4, 4>& m)
    {
        T d0 = m.v[1][1] * (m.v[2][2] * m.v[3][3] - m.v[3][2] * m.v[2][3])
            + m.v[2][1] * (m.v[3][2] * m.v[1][3] - m.v[1][2] * m.v[3][3])
            + m.v[3][1] * (m.v[1][2] * m.v[2][3] - m.v[2][2] * m.v[1][3]);
        T d1 = m.v[0][1] * (m.v[2][2] * m.v[3][3] - m.v[3][2] * m.v[2][3])
            + m.v[2][1] * (m.v[3][2] * m.v[0][3] - m.v[0][2] * m.v[3][3])
            + m.v[3][1] * (m.v[0][2] * m.v[2][3] - m.v[2][2] * m.v[0][3]);
        T d2 = m.v[0][1] * (m.v[1][2] * m.v[3][3] - m.v[3][2] * m.v[1][3])
            + m.v[1][1] * (m.v[3][2] * m.v[0][3] - m.v[0][2] * m.v[3][3])
            + m.v[3][1] * (m.v[0][2] * m.v[1][3] - m.v[1][2] * m.v[0][3]);
        T d3 = m.v[0][1] * (m.v[1][2] * m.v[2][3] - m.v[2][2] * m.v[1][3])
            + m.v[1][1] * (m.v[2][2] * m.v[0][3] - m.v[0][2] * m.v[2][3])
            + m.v[2][1] * (m.v[0][2] * m.v[1][3] - m.v[1][2] * m.v[0][3]);
        return m.v[0][0] * d0 - m.v[1][0] * d1 + m.v[2][0] * d2 - m.v[3][0] * d3;
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
            m.v[2][0] * m.v[3][1] - m.v[2][1] * m.v[3][0],
            m.v[2][0] * m.v[3][2] - m.v[2][2] * m.v[3][0],
            m.v[2][0] * m.v[3][3] - m.v[2][3] * m.v[3][0],
            m.v[2][1] * m.v[3][2] - m.v[2][2] * m.v[3][1],
            m.v[2][1] * m.v[3][3] - m.v[2][3] * m.v[3][1],
            m.v[2][2] * m.v[3][3] - m.v[2][3] * m.v[3][2]
        };

        /* first half of co_matrix: 24 multiplications, 16 additions */
        result.v[0][0] = m.v[1][1] * t1[5] - m.v[1][2] * t1[4] + m.v[1][3] * t1[3];
        result.v[1][0] = m.v[1][2] * t1[2] - m.v[1][3] * t1[1] - m.v[1][0] * t1[5];
        result.v[2][0] = m.v[1][3] * t1[0] - m.v[1][1] * t1[2] + m.v[1][0] * t1[4];
        result.v[3][0] = m.v[1][1] * t1[1] - m.v[1][0] * t1[3] - m.v[1][2] * t1[0];
        result.v[0][1] = m.v[0][2] * t1[4] - m.v[0][1] * t1[5] - m.v[0][3] * t1[3];
        result.v[1][1] = m.v[0][0] * t1[5] - m.v[0][2] * t1[2] + m.v[0][3] * t1[1];
        result.v[2][1] = m.v[0][1] * t1[2] - m.v[0][3] * t1[0] - m.v[0][0] * t1[4];
        result.v[3][1] = m.v[0][0] * t1[3] - m.v[0][1] * t1[1] + m.v[0][2] * t1[0];

        /* second set of 2x2 determinants: 12 multiplications, 6 additions */
        T t2[6] =
        {
            m.v[0][0] * m.v[1][1] - m.v[0][1] * m.v[1][0],
            m.v[0][0] * m.v[1][2] - m.v[0][2] * m.v[1][0],
            m.v[0][0] * m.v[1][3] - m.v[0][3] * m.v[1][0],
            m.v[0][1] * m.v[1][2] - m.v[0][2] * m.v[1][1],
            m.v[0][1] * m.v[1][3] - m.v[0][3] * m.v[1][1],
            m.v[0][2] * m.v[1][3] - m.v[0][3] * m.v[1][2]
        };

        /* second half of co_matrix: 24 multiplications, 16 additions */
        result.v[0][2] = m.v[3][1] * t2[5] - m.v[3][2] * t2[4] + m.v[3][3] * t2[3];
        result.v[1][2] = m.v[3][2] * t2[2] - m.v[3][3] * t2[1] - m.v[3][0] * t2[5];
        result.v[2][2] = m.v[3][3] * t2[0] - m.v[3][1] * t2[2] + m.v[3][0] * t2[4];
        result.v[3][2] = m.v[3][1] * t2[1] - m.v[3][0] * t2[3] - m.v[3][2] * t2[0];
        result.v[0][3] = m.v[2][2] * t2[4] - m.v[2][1] * t2[5] - m.v[2][3] * t2[3];
        result.v[1][3] = m.v[2][0] * t2[5] - m.v[2][2] * t2[2] + m.v[2][3] * t2[1];
        result.v[2][3] = m.v[2][1] * t2[2] - m.v[2][3] * t2[0] - m.v[2][0] * t2[4];
        result.v[3][3] = m.v[2][0] * t2[3] - m.v[2][1] * t2[1] + m.v[2][2] * t2[0];

        /* determinant: 4 multiplications, 3 additions */
        T determinant =
            m.v[0][0] * result.v[0][0] + m.v[0][1] * result.v[1][0] +
            m.v[0][2] * result.v[2][0] + m.v[0][3] * result.v[3][0];

        /* division: 16 multiplications, 1 division */
        return result / determinant;
    }

    template<typename T> vector<T, 3> translation(const matrix<T, 4, 4>& m)
    {
        return vector<T, 3>(m.v[3][0], m.v[3][1], m.v[3][2]);
    }

    template<typename T> swizzler<T, 3> translation(matrix<T, 4, 4>& m)
    {
        return swizzler<T, 3>(m.v[3][0], m.v[3][1], m.v[3][2]);
    }

    template<typename T> matrix<T, 4, 4> translate(const matrix<T, 4, 4>& m, const vector<T, 3>& v)
    {
        vector<T, 4> t(v, static_cast<T>(1));
        matrix<T, 4, 4> r;
        r.v[0][0] = m.v[0][0];
        r.v[0][1] = m.v[0][1];
        r.v[0][2] = m.v[0][2];
        r.v[0][3] = m.v[0][3];
        r.v[1][0] = m.v[1][0];
        r.v[1][1] = m.v[1][1];
        r.v[1][2] = m.v[1][2];
        r.v[1][3] = m.v[1][3];
        r.v[2][0] = m.v[2][0];
        r.v[2][1] = m.v[2][1];
        r.v[2][2] = m.v[2][2];
        r.v[2][3] = m.v[2][3];
        r.v[3][0] = dot(row(m, 0), t);
        r.v[3][1] = dot(row(m, 1), t);
        r.v[3][2] = dot(row(m, 2), t);
        r.v[3][3] = dot(row(m, 3), t);
        return r;
    }

    template<typename T> matrix<T, 4, 4> scale(const matrix<T, 4, 4>& m, const vector<T, 3>& v)
    {
        matrix<T, 4, 4> r;
        col(r, 0) = col(m, 0) * v.x;
        col(r, 1) = col(m, 1) * v.y;
        col(r, 2) = col(m, 2) * v.z;
        col(r, 3) = col(m, 3);
        return r;
    }

    template<typename T> matrix<T, 4, 4> rotate(const matrix<T, 4, 4>& m, T angle, const vector<T, 3>& axis)
    {
        return m * toMat4(angle, axis);
    }

    template<typename T, int c, int r> matrix<bool, c, r> is_pow2(const matrix<T, c, r>& v) { return v._ispow2(); }
    template<typename T, int c, int r> matrix<T, c, r> next_pow2(const matrix<T, c, r>& v) { return v._next_pow2(); }
    template<typename T, int c, int r> matrix<T, c, r> next_multiple(const matrix<T, c, r>& v, const matrix<T, c, r>& w) { return v._next_multiple(w); }
    template<typename T, int c, int r> matrix<T, c, r> next_multiple(const matrix<T, c, r>& v, T x) { return v._next_multiple(x); }

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
    class quaternion : public array<T, 1, 4>
    {
    public:
        using array<T, 1, 4>::vl;
        using array<T, 1, 4>::x;
        using array<T, 1, 4>::y;
        using array<T, 1, 4>::z;
        using array<T, 1, 4>::w;

        /* Constructors, Destructor */

        quaternion() {}
        explicit quaternion(const array<T, 1, 4>& a) : array<T, 1, 4>(a) {}
        explicit quaternion(const vector<T, 4>& a) : array<T, 1, 4>(a) {}
        explicit quaternion(const T* v) : array<T, 1, 4>(v) {}
        template<typename U> explicit quaternion(const quaternion<U>& q) : array<T, 1, 4>(q) {}
        explicit quaternion(T x, T y, T z, T w) : array<T, 1, 4>(x, y, z, w) {}

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
            return vec3(t.x, t.y, t.z);
        }

        vector<T, 4> operator*(const vector<T, 4>& v) const
        {
            //return toMat4(*this) * v;
            quaternion<T> t = *this * quaternion<T>(v.x, v.y, v.z, static_cast<T>(0)) * conjugate(*this);
            return vec4(t.x, t.y, t.z, t.w);
        }
    };

    template<typename T> T magnitude(const quaternion<T>& q) { return q._length(); }
    template<typename T> quaternion<T> conjugate(const quaternion<T>& q) { return quaternion<T>(-q.x, -q.y, -q.z, q.w); }
    template<typename T> quaternion<T> inverse(const quaternion<T>& q) { return quaternion<T>(conjugate(q)._op_scal_div(magnitude(q))); }
    template<typename T> quaternion<T> normalize(const quaternion<T>& q) { return quaternion<T>(q._op_scal_div(magnitude(q))); }

    typedef quaternion<float> quat;
    typedef quaternion<double> dquat;


    template<typename T>
    class frustum : public array<T, 1, 6>
    {
    public:
        using array<T, 1, 6>::vl;

        /* Constructors, Destructor */

        frustum() {}

        explicit frustum(const array<T, 1, 6>& a) : array<T, 1, 6>(a) {}
        explicit frustum(const vector<T, 6>& a) : array<T, 1, 6>(a) {}
        explicit frustum(const T* v) : array<T, 1, 6>(v) {}
        explicit frustum(const std::string &s) : array<T, 1, 6>(s) {}
        template<typename U> explicit frustum(const frustum<U>& q) : array<T, 1, 6>(q) {}
        explicit frustum(T l, T r, T b, T t, T n, T f) : array<T, 1, 6>(l, r, b, t, n ,f) {}

        /* Get / set components */

        const T& l() const { return vl[0]; }
        const T& r() const { return vl[1]; }
        const T& b() const { return vl[2]; }
        const T& t() const { return vl[3]; }
        const T& n() const { return vl[4]; }
        const T& f() const { return vl[5]; }
        T& l() { return vl[0]; }
        T& r() { return vl[1]; }
        T& b() { return vl[2]; }
        T& t() { return vl[3]; }
        T& n() { return vl[4]; }
        T& f() { return vl[5]; }

        /* Frustum operations */

        void adjust_near(T new_near)
        {
            T q = new_near / n();
            l() *= q;
            r() *= q;
            b() *= q;
            t() *= q;
            n() = new_near;
        }
    };

    template<typename T> matrix<T, 4, 4> toMat4(const frustum<T>& f)
    {
        matrix<T, 4, 4> m;
        m.v[0][0] = static_cast<T>(2) * f.n() / (f.r() - f.l());
        m.v[0][1] = static_cast<T>(0);
        m.v[0][2] = static_cast<T>(0);
        m.v[0][3] = static_cast<T>(0);
        m.v[1][0] = static_cast<T>(0);
        m.v[1][1] = static_cast<T>(2) * f.n() / (f.t() - f.b());
        m.v[1][2] = static_cast<T>(0);
        m.v[1][3] = static_cast<T>(0);
        m.v[2][0] = (f.r() + f.l()) / (f.r() - f.l());
        m.v[2][1] = (f.t() + f.b()) / (f.t() - f.b());
        m.v[2][2] = - (f.f() + f.n()) / (f.f() - f.n());
        m.v[2][3] = static_cast<T>(-1);
        m.v[3][0] = static_cast<T>(0);
        m.v[3][1] = static_cast<T>(0);
        m.v[3][2] = - static_cast<T>(2) * f.f() * f.n() / (f.f() - f.n());
        m.v[3][3] = static_cast<T>(0);
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
        T t = static_cast<T>(1) + rot_matrix.v[0][0] + rot_matrix.v[1][1] + rot_matrix.v[2][2];
        if (t > static_cast<T>(1e-8))
        {
            T s = sqrt(t) * static_cast<T>(2);
            q.x = (rot_matrix.v[1][2] - rot_matrix.v[2][1]) / s;
            q.y = (rot_matrix.v[2][0] - rot_matrix.v[0][2]) / s;
            q.z = (rot_matrix.v[0][1] - rot_matrix.v[1][0]) / s;
            q.w = s / static_cast<T>(4);
        }
        else if (rot_matrix.v[0][0] > rot_matrix.v[1][1] && rot_matrix.v[0][0] > rot_matrix.v[2][2])
        {
            t = static_cast<T>(1) + rot_matrix.v[0][0] - rot_matrix.v[1][1] - rot_matrix.v[2][2];
            T s = sqrt(t) * static_cast<T>(2);
            q.x = s / static_cast<T>(4);
            q.y = (rot_matrix.v[0][1] + rot_matrix.v[1][0]) / s;
            q.z = (rot_matrix.v[2][0] + rot_matrix.v[0][2]) / s;
            q.w = (rot_matrix.v[1][2] - rot_matrix.v[2][1]) / s;
        }
        else if (rot_matrix.v[1][1] > rot_matrix.v[2][2])
        {
            t = static_cast<T>(1) + rot_matrix.v[1][1] - rot_matrix.v[0][0] - rot_matrix.v[2][2];
            T s = sqrt(t) * static_cast<T>(2);
            q.x = (rot_matrix.v[0][1] + rot_matrix.v[1][0]) / s;
            q.y = s / static_cast<T>(4);
            q.z = (rot_matrix.v[1][2] + rot_matrix.v[2][1]) / s;
            q.w = (rot_matrix.v[2][0] - rot_matrix.v[0][2]) / s;
        }
        else
        {
            t = static_cast<T>(1) + rot_matrix.v[2][2] - rot_matrix.v[0][0] - rot_matrix.v[1][1];
            T s = sqrt(t) * static_cast<T>(2);
            q.x = (rot_matrix.v[2][0] + rot_matrix.v[0][2]) / s;
            q.y = (rot_matrix.v[1][2] + rot_matrix.v[2][1]) / s;
            q.z = s / static_cast<T>(4);
            q.w = (rot_matrix.v[0][1] - rot_matrix.v[1][0]) / s;
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
        m.v[0][0] = n.x * n.x * mc + c;
        m.v[0][1] = n.y * n.x * mc + n.z * s;
        m.v[0][2] = n.x * n.z * mc - n.y * s;
        m.v[1][0] = n.x * n.y * mc - n.z * s;
        m.v[1][1] = n.y * n.y * mc + c;
        m.v[1][2] = n.y * n.z * mc + n.x * s;
        m.v[2][0] = n.x * n.z * mc + n.y * s;
        m.v[2][1] = n.y * n.z * mc - n.x * s;
        m.v[2][2] = n.z * n.z * mc + c;
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
        m.v[0][0] = static_cast<T>(1) - static_cast<T>(2) * (yy + zz);
        m.v[0][1] = static_cast<T>(2) * (xy + zw);
        m.v[0][2] = static_cast<T>(2) * (xz - yw);
        m.v[1][0] = static_cast<T>(2) * (xy - zw);
        m.v[1][1] = static_cast<T>(1) - static_cast<T>(2) * (xx + zz);
        m.v[1][2] = static_cast<T>(2) * (yz + xw);
        m.v[2][0] = static_cast<T>(2) * (xz + yw);
        m.v[2][1] = static_cast<T>(2) * (yz - xw);
        m.v[2][2] = static_cast<T>(1) - static_cast<T>(2) * (xx + yy);
        return m;
    }

    template<typename T> matrix<T, 4, 4> toMat4(T angle, const vector<T, 3>& axis)
    {
        matrix<T, 4, 4> m;
        m._set_sub_array(toMat3(angle, axis));
        m.v[0][3] = static_cast<T>(0);
        m.v[1][3] = static_cast<T>(0);
        m.v[2][3] = static_cast<T>(0);
        m.v[3][0] = static_cast<T>(0);
        m.v[3][1] = static_cast<T>(0);
        m.v[3][2] = static_cast<T>(0);
        m.v[3][3] = static_cast<T>(1);
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
        matrix<T, 4, 4> m;
        m._set_sub_array(toMat3(q));
        m.v[0][3] = static_cast<T>(0);
        m.v[1][3] = static_cast<T>(0);
        m.v[2][3] = static_cast<T>(0);
        m.v[3][0] = static_cast<T>(0);
        m.v[3][1] = static_cast<T>(0);
        m.v[3][2] = static_cast<T>(0);
        m.v[3][3] = static_cast<T>(1);
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
                    0.0);
        }
        else if (singularity_test < static_cast<T>(-0.4999))
        {
            // south pole
            result = vector<T, 3>(
                    -static_cast<T>(2) * glvm::atan(q.x, q.w),
                    -const_pi_2<T>(),
                    0.0);
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
        T t = tan(fovy / 2.0f);
        T top = zNear * t;
        T bottom = -top;
        T right = top * aspect;
        T left = -right;
        return frustum<T>(left, right, bottom, top, zNear, zFar);
    }

    // gluLookAt()
    template<typename T> matrix<T, 4, 4> lookat(const vector<T, 3>& eye, const vector<T, 3>& center, const vector<T, 3>& up)
    {
        vector<T, 3> v = normalize(center - eye);
        vector<T, 3> s = normalize(cross(v, up));
        vector<T, 3> u = cross(s, v);
        return translate(matrix<T, 4, 4>(
                     s.x,  u.x, -v.x, 0.0f,
                     s.y,  u.y, -v.y, 0.0f,
                     s.z,  u.z, -v.z, 0.0f,
                    0.0f, 0.0f, 0.0f, 1.0f), -eye);
    }
}

#endif
