//===- toolchain/Support/ErrorHandling.h - Fatal error handling ------*- C++ -*-===//
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
// This file defines an API used to indicate fatal error conditions.  Non-fatal
// errors (most of them) should be handled through LLVMContext.
//
//===----------------------------------------------------------------------===//

#ifndef TOOLCHAIN_SUPPORT_ERRORHANDLING_H
#define TOOLCHAIN_SUPPORT_ERRORHANDLING_H

#include "toolchain/Support/Compiler.h"
#include <string>

inline namespace __language { inline namespace __runtime {
namespace toolchain {
class StringRef;
/// An error handler callback.
typedef void (*fatal_error_handler_t)(void *user_data,
                                      const std::string& reason,
                                      bool gen_crash_diag);

/// Reports a serious error, calling any installed error handler. These
/// functions are intended to be used for error conditions which are outside
/// the control of the compiler (I/O errors, invalid user input, etc.)
///
/// If no error handler is installed the default is to print the message to
/// standard error, followed by a newline.
/// After the error handler is called this function will call abort(), it
/// does not return.
TOOLCHAIN_ATTRIBUTE_NORETURN void report_fatal_error(const char *reason,
                                                bool gen_crash_diag = true);
TOOLCHAIN_ATTRIBUTE_NORETURN void report_fatal_error(const std::string &reason,
                                                bool gen_crash_diag = true);
TOOLCHAIN_ATTRIBUTE_NORETURN void report_fatal_error(StringRef reason,
                                                bool gen_crash_diag = true);

/// Reports a bad alloc error, calling any user defined bad alloc
/// error handler. In contrast to the generic 'report_fatal_error'
/// functions, this function might not terminate, e.g. the user
/// defined error handler throws an exception, but it won't return.
///
/// Note: When throwing an exception in the bad alloc handler, make sure that
/// the following unwind succeeds, e.g. do not trigger additional allocations
/// in the unwind chain.
///
/// If no error handler is installed (default), throws a bad_alloc exception
/// if LLVM is compiled with exception support. Otherwise prints the error
/// to standard error and calls abort().
TOOLCHAIN_ATTRIBUTE_NORETURN void report_bad_alloc_error(const char *Reason,
                                                    bool GenCrashDiag = true);

/// This function calls abort(), and prints the optional message to stderr.
/// Use the toolchain_unreachable macro (that adds location info), instead of
/// calling this function directly.
TOOLCHAIN_ATTRIBUTE_NORETURN void
toolchain_unreachable_internal(const char *msg = nullptr, const char *file = nullptr,
                          unsigned line = 0);
}
}} // namespace language::runtime

/// Marks that the current location is not supposed to be reachable.
/// In !NDEBUG builds, prints the message and location info to stderr.
/// In NDEBUG builds, becomes an optimizer hint that the current location
/// is not supposed to be reachable.  On compilers that don't support
/// such hints, prints a reduced message instead and aborts the program.
///
/// Use this instead of assert(0).  It conveys intent more clearly and
/// allows compilers to omit some unnecessary code.
#ifndef NDEBUG
#define toolchain_unreachable(msg) \
  __language::__runtime::toolchain::toolchain_unreachable_internal(msg, __FILE__, __LINE__)
#elif defined(TOOLCHAIN_BUILTIN_UNREACHABLE)
#define toolchain_unreachable(msg) TOOLCHAIN_BUILTIN_UNREACHABLE
#else
#define toolchain_unreachable(msg) __language::__runtime::toolchain::toolchain_unreachable_internal()
#endif

#endif
