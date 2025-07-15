//===--- CoreFoundationShims.h - Access to CF for the core stdlib ---------===//
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
//  Using the CoreFoundation module in the core stdlib would create a
//  circular dependency, so instead we import these declarations as
//  part of CodiraShims.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_STDLIB_SHIMS_COREFOUNDATIONSHIMS_H
#define LANGUAGE_STDLIB_SHIMS_COREFOUNDATIONSHIMS_H

#include "CodiraStdint.h"
#include "Visibility.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __OBJC2__
#if __LLP64__
typedef unsigned long long _language_shims_CFHashCode;
typedef signed long long _language_shims_CFIndex;
#else
typedef unsigned long _language_shims_CFHashCode;
typedef signed long _language_shims_CFIndex;
#endif

typedef unsigned long _language_shims_NSUInteger;

// Consider creating CodiraMacTypes.h for these
typedef unsigned char _language_shims_Boolean;
typedef __language_uint8_t _language_shims_UInt8;
typedef __language_uint32_t _language_shims_CFStringEncoding;

/* This is layout-compatible with constant CFStringRefs on Darwin */
typedef struct __language_shims_builtin_CFString {
  const void * _Nonnull isa; // point to __CFConstantStringClassReference
  unsigned long flags;
  const __language_uint8_t * _Nonnull str;
  unsigned long length;
} _language_shims_builtin_CFString;

LANGUAGE_RUNTIME_STDLIB_API
__language_uint8_t _language_stdlib_isNSString(id _Nonnull obj);

LANGUAGE_RUNTIME_STDLIB_API
_language_shims_CFHashCode _language_stdlib_CFStringHashNSString(id _Nonnull obj);

LANGUAGE_RUNTIME_STDLIB_API
_language_shims_CFHashCode
_language_stdlib_CFStringHashCString(const _language_shims_UInt8 * _Nonnull bytes,
                                  _language_shims_CFIndex length);

LANGUAGE_RUNTIME_STDLIB_API
const __language_uint8_t * _Nullable
_language_stdlib_NSStringCStringUsingEncodingTrampoline(id _Nonnull obj,
                                                     unsigned long encoding);

LANGUAGE_RUNTIME_STDLIB_API
__language_uint8_t
_language_stdlib_NSStringGetCStringTrampoline(id _Nonnull obj,
                                           _language_shims_UInt8 *_Nonnull buffer,
                                           _language_shims_CFIndex maxLength,
                                           unsigned long encoding);

LANGUAGE_RUNTIME_STDLIB_API
__language_uint8_t
_language_stdlib_dyld_is_objc_constant_string(const void * _Nonnull addr);

LANGUAGE_RUNTIME_STDLIB_API
const void * _Nullable
_language_stdlib_CreateIndirectTaggedPointerString(const __language_uint8_t * _Nonnull bytes,
                                                _language_shims_CFIndex len);

LANGUAGE_RUNTIME_STDLIB_API
const _language_shims_NSUInteger
_language_stdlib_NSStringLengthOfBytesInEncodingTrampoline(id _Nonnull obj,
                                                        unsigned long encoding);

#endif // __OBJC2__

#ifdef __cplusplus
} // extern "C"
#endif

#endif // LANGUAGE_STDLIB_SHIMS_COREFOUNDATIONSHIMS_H

