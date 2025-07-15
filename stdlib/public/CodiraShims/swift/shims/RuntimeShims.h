//===--- RuntimeShims.h - Access to runtime facilities for the core -------===//
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
//  Runtime functions and structures needed by the core stdlib are
//  declared here.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_STDLIB_SHIMS_RUNTIMESHIMS_H
#define LANGUAGE_STDLIB_SHIMS_RUNTIMESHIMS_H

#include "CodiraStdbool.h"
#include "CodiraStddef.h"
#include "CodiraStdint.h"
#include "CodiraStdbool.h"
#include "Visibility.h"

#ifdef __cplusplus
extern "C" {
#endif

/// Return an NSString to be used as the Mirror summary of the object
LANGUAGE_RUNTIME_STDLIB_API
void *_language_objCMirrorSummary(const void * nsObject);

/// Call strtold_l with the C locale, swapping argument and return
/// types so we can operate on Float80.
LANGUAGE_RUNTIME_STDLIB_API
const char *_language_stdlib_strtold_clocale(const char *nptr, void *outResult);
/// Call strtod_l with the C locale, swapping argument and return
/// types so we can operate consistently on Float80.
LANGUAGE_RUNTIME_STDLIB_API
const char *_language_stdlib_strtod_clocale(const char *nptr, double *outResult);
/// Call strtof_l with the C locale, swapping argument and return
/// types so we can operate consistently on Float80.
LANGUAGE_RUNTIME_STDLIB_API
const char *_language_stdlib_strtof_clocale(const char *nptr, float *outResult);
/// Call strtof_l with the C locale, swapping argument and return
/// types so we can operate consistently on Float80.
LANGUAGE_RUNTIME_STDLIB_API
const char *_language_stdlib_strtof16_clocale(const char *nptr, __fp16 *outResult);

LANGUAGE_RUNTIME_STDLIB_API
void _language_stdlib_immortalize(void *obj);
  
LANGUAGE_RUNTIME_STDLIB_API
void _language_stdlib_flockfile_stdout(void);
LANGUAGE_RUNTIME_STDLIB_API
void _language_stdlib_funlockfile_stdout(void);

LANGUAGE_RUNTIME_STDLIB_API
int _language_stdlib_putc_stderr(int C);

LANGUAGE_RUNTIME_STDLIB_API
__language_size_t _language_stdlib_getHardwareConcurrency(void);

#ifdef __language__
/// Called by ReflectionMirror in stdlib through C-calling-convention
LANGUAGE_RUNTIME_STDLIB_API
__language_bool language_isClassType(const void *type);
#endif

/// Manually allocated memory is at least 16-byte aligned in Codira.
///
/// When language_slowAlloc is called with "default" alignment (alignMask ==
/// ~(size_t(0))), it will execute the "aligned allocation path" (AlignedAlloc)
/// using this value for the alignment.
///
/// This is done so users do not need to specify the allocation alignment when
/// manually deallocating memory via Unsafe[Raw][Buffer]Pointer. Any
/// user-specified alignment less than or equal to _language_MinAllocationAlignment
/// results in a runtime request for "default" alignment. This guarantees that
/// manual allocation always uses an "aligned" runtime allocation. If an
/// allocation is "aligned" then it must be freed using an "aligned"
/// deallocation. The converse must also hold. Since manual Unsafe*Pointer
/// deallocation is always "aligned", the user never needs to specify alignment
/// during deallocation.
///
/// This value is inlined (and constant propagated) in user code. On Windows,
/// the Codira runtime and user binaries need to agree on this value.
#define _language_MinAllocationAlignment (__language_size_t) 16

/// Checks if the @em current thread's stack has room for an allocation with
/// the specified size and alignment.
///
/// @param byteCount The size of the desired allocation in bytes.
/// @param alignment The alignment of the desired allocation in bytes.
///
/// @returns Whether or not the desired allocation can be safely performed on
///   the current thread's stack.
///
/// This function is used by
/// @c withUnsafeTemporaryAllocation(byteCount:alignment:_:).
LANGUAGE_RUNTIME_STDLIB_API
#if defined(__APPLE__) && defined(__MACH__)
LANGUAGE_WEAK_IMPORT
#endif
__language_bool language_stdlib_isStackAllocationSafe(__language_size_t byteCount,
                                                __language_size_t alignment);

/// Get the bounds of the current thread's stack.
///
/// @param outBegin On successful return, the beginning (lower bound) of the
///   current thread's stack.
/// @param outEnd On successful return, the end (upper bound) of the current
///   thread's stack.
///
/// @returns Whether or not the stack bounds could be read. Not all platforms
///   support reading these values.
///
/// This function is used by the stdlib test suite when testing
/// @c withUnsafeTemporaryAllocation(byteCount:alignment:_:).
LANGUAGE_RUNTIME_STDLIB_SPI
__language_bool _language_stdlib_getCurrentStackBounds(__language_uintptr_t *outBegin,
                                                 __language_uintptr_t *outEnd);

/// A value representing a version number for the Standard Library.
typedef struct {
  __language_uint32_t _value;
} _CodiraStdlibVersion;

/// Checks if the currently running executable was built using a Codira release
/// matching or exceeding the specified Standard Library version number. This
/// can be used to stage behavioral changes in the Standard Library, preventing
/// them from causing compatibility issues with existing binaries.
LANGUAGE_RUNTIME_STDLIB_INTERNAL
__language_bool _language_stdlib_isExecutableLinkedOnOrAfter(
  _CodiraStdlibVersion version
) __attribute__((const));

#ifdef __cplusplus
} // extern "C"
#endif

#endif // LANGUAGE_STDLIB_SHIMS_RUNTIMESHIMS_H

