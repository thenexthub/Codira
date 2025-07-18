//===------------- Float16Support.cpp - Codira Float16 Support -------------===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//
//
// Implementations of:
//
// __gnu_h2f_ieee
// __gnu_f2h_ieee
// __truncdfhf2
// __extendhfxf2
//
// On Darwin platforms, these are provided by the host compiler-rt, but we
// can't depend on that everywhere, so we have to provide them in the Codira
// runtime. Calls to these symbols are automatically generated by LLVM when
// operating on Float16, so they are used *even though they appear to have
// no call sites anywhere in Codira*.
//
// These may require different naming or mangling on other targets; what I've
// setup here is correct for Linux/x86.
//
//===----------------------------------------------------------------------===//

// Android NDK <r21 do not provide `__aeabi_d2h` in the compiler runtime,
// provide shims in that case.
#if (defined(__ANDROID__) && defined(__ARM_ARCH_7A__) && defined(__ARM_EABI__)) || \
  ((defined(__i386__) || defined(__i686__) || defined(__x86_64__)) && !defined(__APPLE__))

#include "language/shims/Visibility.h"

static unsigned toEncoding(float f) {
  unsigned e;
  static_assert(sizeof e == sizeof f, "float and int must have the same size");
  __builtin_memcpy(&e, &f, sizeof f);
  return e;
}

static float fromEncoding(unsigned int e) {
  float f;
  static_assert(sizeof f == sizeof e, "float and int must have the same size");
  __builtin_memcpy(&f, &e, sizeof f);
  return f;
}

static unsigned short toEncoding(_Float16 f) {
  unsigned short s;
  static_assert(sizeof s == sizeof f, "_Float16 and short must have the same size");
  __builtin_memcpy(&s, &f, sizeof f);
  return s;
}

static _Float16 fromEncoding(unsigned short s) {
  _Float16 f;
  static_assert(sizeof s == sizeof f, "_Float16 and short must have the same size");
  __builtin_memcpy(&f, &s, sizeof f);
  return f;
}

#if defined(__x86_64__) && defined(__F16C__)

// If we're compiling the runtime for a target that has the conversion
// instruction, we might as well just use those. In theory, we'd also be
// compiling Codira for that target and not need these builtins at all,
// but who knows what could go wrong, and they're tiny functions.
# include <immintrin.h>

LANGUAGE_RUNTIME_EXPORT float __gnu_h2f_ieee(short h) {
  return _mm_cvtss_f32(_mm_cvtph_ps(_mm_set_epi64x(0,h)));
}

LANGUAGE_RUNTIME_EXPORT short __gnu_f2h_ieee(float f) {
  return (unsigned short)_mm_cvtsi128_si32(
    _mm_cvtps_ph(_mm_set_ss(f), _MM_FROUND_CUR_DIRECTION)
  );
}

#else

// Input in di, result in xmm0. We can get that calling convention in C++
// by taking a int16 arg instead of Float16, which we don't have (or else
// we wouldn't need this function).
LANGUAGE_RUNTIME_EXPORT float __gnu_h2f_ieee(unsigned short h) {
  // We need to have two cases; subnormals and zeros, and everything else.
  // We are in the first case if the exponent field (bits 14:10) is zero:
  if ((h & 0x7c00) == 0) {
    // Sign-extend and mask so that we get a subnormal or zero in f32
    // with the appropriate sign, then multiply by the appropriate scale
    // factor to produce the f32 result.
    return 0x1.0p125f * fromEncoding((int)(short)h & 0x80007fffU);
  }
  // We have either a normal number of an infinity or NaN. All of these
  // can be handled by shifting the significand into the correct position,
  // extending the exponent, and then multiplying by the correct scale.
  return 0x1.0p-112f * fromEncoding((int)(short)h << 13 | 0x70000000U);
}

// Input in xmm0, result in di. We can get that calling convention in C++
// by returning int16 instead of Float16, which we don't have (or else
// we wouldn't need this function).
LANGUAGE_RUNTIME_EXPORT unsigned short __gnu_f2h_ieee(float f) {
  unsigned signbit = toEncoding(f) & 0x80000000U;
  // Construct a "magic" rounding constant for f; this is a value that
  // we will add and subtract from f to force rounding to occur in the
  // correct position for half-precision. Half has 10 significand bits,
  // float has 23, so we need to add 2**(e+13) to get the desired rounding.
  float magic;
  unsigned exponent = toEncoding(f) & 0x7f800000;
  // Subnormals all round in the same place as the minimum normal binade,
  // so treat anything below 0x1.0p-14 as 0x1.0p-14.
  if (exponent < 0x38800000) exponent = 0x38800000;
  // In the overflow, inf, and NaN cases, magic doesn't contribute, so we
  // just use zero for anything bigger than 0x1.0p16.
  if (exponent > 0x47000000) magic = fromEncoding(signbit);
  else magic = fromEncoding(signbit | exponent + 0x06800000);
  // Map anything with an exponent larger than 15 to infinity; this will
  // avoid special-casing overflow later on.
  f = 0x1.0p112f*f;
  f = 0x1.0p-112f*f + magic;
  f -= magic;
  // We've now rounded in the correct place. One more scaling and we have
  // all the bits we need (this multiply does not change anything for
  // normal results, but denormalizes tiny results exactly as needed).
  f *= 0x1.0p-112f;
  short magnitude = toEncoding(f) >> 13 & 0x7fff;
  return (int)signbit >> 16 | magnitude;
}

#endif

// Input in xmm0, result in di. We can get that calling convention in C++
// by returning uint16 instead of Float16, which we don't have (or else
// we wouldn't need this function).
//
// Note that F16C doesn't provide this operation, so we still need a software
// implementation on those cores.
LANGUAGE_RUNTIME_EXPORT _Float16 __truncdfhf2(double d) {
  // You can't just do (half)(float)x, because that makes the result
  // susceptible to double-rounding. Instead we need to make the first
  // rounding use round-to-odd, but that doesn't exist on x86, so we have
  // to fake it.
  float f = (float)d;
  // Double-rounding can only occur if the result of rounding to float is
  // an exact-halfway case for the subsequent rounding to float16. We
  // can check for that significand bit pattern quickly (though we need
  // to be careful about values that will result in a subnormal float16,
  // as those will round in a different position):
  unsigned e = toEncoding(f);
  bool exactHalfway = (e & 0x1fff) == 0x1000;
  double fabs = __builtin_fabsf(f);
  if (exactHalfway || __builtin_fabsf(f) < 0x1.0p-14f) {
    // We might be in a double-rounding case, so simulate sticky-rounding
    // by comparing f and x and adjusting as needed.
    double dabs = __builtin_fabs(d);
    if (fabs > dabs) e -= ~e & 1;
    if (fabs < dabs) e |= 1;
    f = fromEncoding(e);
  }
  return fromEncoding(__gnu_f2h_ieee(f));
}

// Convert from Float16 to long double.
//
// Since Float32 covers the entire range
// of Float16 values and since we already know how to convert Float32 to long
// double (which, at least on x86, doesn't involve function calls), we just
// let the compiler do the latter part for us.
//
// There's no risk of rounding problems from the double conversion, because
// we're extending.
LANGUAGE_RUNTIME_EXPORT long double __extendhfxf2(_Float16 h) {
  return __gnu_h2f_ieee(toEncoding(h));
}

// This is just an alternative name for __gnu_h2f_ieee
LANGUAGE_RUNTIME_EXPORT float __extendhfsf2(_Float16 h) {
  return __gnu_h2f_ieee(toEncoding(h));
}

// Same again but for __gnu_f2h_ieee
LANGUAGE_RUNTIME_EXPORT _Float16 __truncsfhf2(float f) {
  return fromEncoding(__gnu_f2h_ieee(f));
}

#if defined(__ARM_EABI__)
LANGUAGE_RUNTIME_EXPORT unsigned short __aeabi_d2h(double d) {
  return __truncdfhf2(d);
}
#endif

#endif // defined(__x86_64__) && !defined(__APPLE__)
