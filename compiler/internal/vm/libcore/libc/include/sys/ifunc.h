/*
 * Copyright (C) 2019 The Android Open Source Project
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

#include <sys/cdefs.h>

#include <stddef.h>
#include <stdint.h>

/**
 * @file sys/ifunc.h
 * @brief Declarations used for ifunc resolvers. Currently only meaningful for arm64.
 */

__BEGIN_DECLS

#if defined(__aarch64__)

/**
 * Helper function that returns the given AT_HWCAP if available, 0 otherwise.
 *
 * The first argument is which hwcap: 1 for AT_HWCAP, 2 for AT_HWCAP2, and so on.
 * The second argument is the first argument passed to your ifunc resolver;
 * it's unused, but there for glibc source compatibility.
 * The third argument is the second argument passed to your ifunc resolver.
 *
 * TODO: should this call getauxval() for < api level 30?
 */
static __inline unsigned long __ifunc_hwcap(size_t __which, uint64_t __unused __ifunc_arg0, uint64_t* __ifunc_arg1) {
  return (__which > 0) && (__which < __ifunc_arg1[0] / sizeof(uint64_t)) ? __ifunc_arg1[__which] : 0;
}

/**
 * Provides information about hardware capabilities to arm64 ifunc resolvers.
 *
 * New code should use __ifunc_hwcap() rather than interpreting ifunc arguments
 * directly.
 *
 * See https://github.com/ARM-software/abi-aa/blob/main/sysvabi64/sysvabi64.rst#gnu-indirect-functions
 * for more information.
 *
 * Prior to API level 30, arm64 ifunc resolvers are passed no arguments.
 *
 * Starting with API level 30, arm64 ifunc resolvers are passed two arguments.
 * The first is a uint64_t whose value is equal to `getauxval(AT_HWCAP) | _IFUNC_ARG_HWCAP`.
 * The second is a pointer to an `__ifunc_arg_t`.
 *
 * Code that wishes to be compatible with API levels before 30 must call getauxval() itself.
 */
typedef struct __ifunc_arg_t {
  /**
   * Used to determine which subset of fields are actually available
   * on the device the ifunc is being run on.
   *
   * New code should use the __ifunc_hwcap() helper which takes care of this.
   */
  unsigned long _size;

  /** Set to getauxval(AT_HWCAP). */
  unsigned long _hwcap;

  /** Set to getauxval(AT_HWCAP2). */
  unsigned long _hwcap2;

  /** Unsafe to access without checking `_size`; use __ifunc_hwcap(). */
  unsigned long _hwcap3;

  /** Unsafe to access without checking `_size`; use __ifunc_hwcap(). */
  unsigned long _hwcap4;
} __ifunc_arg_t;

/**
 * If this bit is set in the first argument to an ifunc resolver, the second argument
 * is a pointer to a data structure of type __ifunc_arg_t.
 *
 * This bit is always set on Android starting with API level 30.
 * This bit is meaningless before API level 30 because ifunc resolvers are not passed any arguments.
 * This bit has no real use on Android, but is included for glibc source compatibility;
 * glibc used this bit to distinguish the case where the ifunc resolver received a single argument,
 * which was an evolutionary stage Android never went through.
 */
#define _IFUNC_ARG_HWCAP (1ULL << 62)

#endif

__END_DECLS
