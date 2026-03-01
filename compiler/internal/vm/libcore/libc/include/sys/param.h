/*
 * Copyright (C) 2008 The Android Open Source Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#pragma once

/**
 * @file sys/param.h
 * @brief Various macros.
 */

#include <sys/cdefs.h>

#include <endian.h>
#include <limits.h>
#include <linux/param.h>

__BEGIN_DECLS

/** The unit of `st_blocks` in `struct stat`. */
#define DEV_BSIZE 512

/** A historical name for PATH_MAX. Use PATH_MAX in new code. */
#define MAXPATHLEN PATH_MAX

/** A historical name for NGROUPS_MAX. Use NGROUPS_MAX in new code. */
#define NGROUPS NGROUPS_MAX

/**
 * The maximum number of symbolic links the kernel will traverse in one path
 * lookup before failing with ELOOP.
 *
 * This is hard-coded to 40 in Linux in "include/linux/namei.h" but not
 * exported in any uapi header.
 */
#define MAXSYMLINKS 40

#ifndef howmany
/**
 * howmany(3) returns the number of elements of size y needed to hold x.
 *
 * Note that this macro may evaluate its arguments more than once.
 */
#define howmany(x, y)   (((x)+((y)-1))/(y))
#endif

/**
 * [roundup(3)](https://man7.org/linux/man-pages/man3/roundup.3.html)
 * returns x rounded up to the smallest multiple of y not less than x.
 *
 * Note that this macro may evaluate its arguments more than once.
 */
#define roundup(x, y)   ((((x)+((y)-1))/(y))*(y))

/**
 * [powerof2(3)](https://man7.org/linux/man-pages/man3/roundup.3.html)
 * returns true if x is zero or has exactly one bit set.
 *
 * Contrary to the macro name, this macro DOES NOT determine if the provided
 * value is a power of 2. In particular, this function falsely returns true for
 * powerof2(0) and some negative numbers.
 *
 * See also stdc_has_single_bit() in <stdbit.h>.
 */
#define powerof2(x)                                               \
  ({                                                              \
    __auto_type _x = (x);                                         \
    __auto_type _x2 = _x;                                         \
    __builtin_add_overflow(_x, -1, &_x2) ? 1 : ((_x2 & _x) == 0); \
  })

/**
 * [MIN(3)](https://man7.org/linux/man-pages/man3/MIN.3.html)
 * returns the lesser of its two arguments.
 *
 * Note that this macro may evaluate its arguments more than once,
 * and cannot be fixed because statement expressions are not allowed at file
 * scope but this macro is.
 *
 * See also fmin()/fminf()/fminl() for floating point types.
 */
#define MIN(a, b) (((a)<(b))?(a):(b))

/**
 * [MAX(3)](https://man7.org/linux/man-pages/man3/MAX.3.html)
 * returns the greater of its two arguments.
 *
 * Note that this macro may evaluate its arguments more than once,
 * and cannot be fixed because statement expressions are not allowed at file
 * scope but this macro is.
 *
 * See also fmax()/fmaxf()/fmaxl() for floating point types.
 */
#define MAX(a, b) (((a)>(b))?(a):(b))

__END_DECLS
