//===--- Compiler.h - Compiler specific definitions -------------*- C++ -*-===//
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

#ifndef LANGUAGE_BASIC_COMPILER_H
#define LANGUAGE_BASIC_COMPILER_H

#include <stddef.h>

#if defined(_MSC_VER) && !defined(__clang__)
#define LANGUAGE_COMPILER_IS_MSVC 1
#else
#define LANGUAGE_COMPILER_IS_MSVC 0
#endif

// Workaround non-clang compilers
#ifndef __has_builtin
#define __has_builtin(x) 0
#endif
#ifndef __has_attribute
#define __has_attribute(x) 0
#endif

// __builtin_assume() is an optimization hint.
#if __has_builtin(__builtin_assume)
#define LANGUAGE_ASSUME(x) __builtin_assume(x)
#else
#define LANGUAGE_ASSUME(x)
#endif

/// Attributes.

#if __has_attribute(constructor)
#define LANGUAGE_CONSTRUCTOR __attribute__((constructor))
#else
#define LANGUAGE_CONSTRUCTOR
#endif

/// \macro LANGUAGE_GNUC_PREREQ
/// Extend the default __GNUC_PREREQ even if glibc's features.h isn't
/// available.
#ifndef LANGUAGE_GNUC_PREREQ
# if defined(__GNUC__) && defined(__GNUC_MINOR__) && defined(__GNUC_PATCHLEVEL__)
#  define LANGUAGE_GNUC_PREREQ(maj, min, patch) \
    ((__GNUC__ << 20) + (__GNUC_MINOR__ << 10) + __GNUC_PATCHLEVEL__ >= \
     ((maj) << 20) + ((min) << 10) + (patch))
# elif defined(__GNUC__) && defined(__GNUC_MINOR__)
#  define LANGUAGE_GNUC_PREREQ(maj, min, patch) \
    ((__GNUC__ << 20) + (__GNUC_MINOR__ << 10) >= ((maj) << 20) + ((min) << 10))
# else
#  define LANGUAGE_GNUC_PREREQ(maj, min, patch) 0
# endif
#endif


/// LANGUAGE_ATTRIBUTE_NOINLINE - On compilers where we have a directive to do so,
/// mark a method "not for inlining".
#if __has_attribute(noinline) || LANGUAGE_GNUC_PREREQ(3, 4, 0)
#define LANGUAGE_ATTRIBUTE_NOINLINE __attribute__((noinline))
#elif defined(_MSC_VER)
#define LANGUAGE_ATTRIBUTE_NOINLINE __declspec(noinline)
#else
#define LANGUAGE_ATTRIBUTE_NOINLINE
#endif

/// LANGUAGE_ATTRIBUTE_ALWAYS_INLINE - On compilers where we have a directive to do
/// so, mark a method "always inline" because it is performance sensitive. GCC
/// 3.4 supported this but is buggy in various cases and produces unimplemented
/// errors, just use it in GCC 4.0 and later.
#if __has_attribute(always_inline) || LANGUAGE_GNUC_PREREQ(4, 0, 0)
#define LANGUAGE_ATTRIBUTE_ALWAYS_INLINE __attribute__((always_inline))
#elif defined(_MSC_VER)
#define LANGUAGE_ATTRIBUTE_ALWAYS_INLINE __forceinline
#else
#define LANGUAGE_ATTRIBUTE_ALWAYS_INLINE
#endif

// Needed for C++ bridging functions which return types with pointers.
#if __has_attribute(language_attr)
#define LANGUAGE_IMPORT_UNSAFE __attribute__((language_attr("import_unsafe")))
#else
#define LANGUAGE_IMPORT_UNSAFE
#endif

#ifdef __GNUC__
#define LANGUAGE_ATTRIBUTE_NORETURN __attribute__((noreturn))
#elif defined(_MSC_VER)
#define LANGUAGE_ATTRIBUTE_NORETURN __declspec(noreturn)
#else
#define LANGUAGE_ATTRIBUTE_NORETURN
#endif

#if __has_attribute(unused)
#define LANGUAGE_ATTRIBUTE_UNUSED __attribute__((__unused__))
#else
#define LANGUAGE_ATTRIBUTE_UNUSED
#endif

#ifndef LANGUAGE_BUG_REPORT_URL
#define LANGUAGE_BUG_REPORT_URL "https://language.org/contributing/#reporting-bugs"
#endif

#define LANGUAGE_BUG_REPORT_MESSAGE_BASE \
  "submit a bug report (" LANGUAGE_BUG_REPORT_URL ")"

#define LANGUAGE_BUG_REPORT_MESSAGE \
  "please " LANGUAGE_BUG_REPORT_MESSAGE_BASE

#define LANGUAGE_CRASH_BUG_REPORT_MESSAGE \
  "Please " LANGUAGE_BUG_REPORT_MESSAGE_BASE " and include the crash backtrace."

#if defined(__LP64__) || defined(_WIN64)
#define LANGUAGE_POINTER_IS_8_BYTES 1
#define LANGUAGE_POINTER_IS_4_BYTES 0
#else
// TODO: consider supporting 16-bit targets
#define LANGUAGE_POINTER_IS_8_BYTES 0
#define LANGUAGE_POINTER_IS_4_BYTES 1
#endif

// Produce a string literal for the raw argument tokens.
#define LANGUAGE_STRINGIZE_RAW(TOK) #TOK

// Produce a string literal for the macro-expanded argument tokens.
#define LANGUAGE_STRINGIZE_EXPANDED(TOK) LANGUAGE_STRINGIZE_RAW(TOK)

#if defined(__USER_LABEL_PREFIX__)
#define LANGUAGE_SYMBOL_PREFIX_STRING \
  LANGUAGE_STRINGIZE_EXPANDED(__USER_LABEL_PREFIX__)
#else
// Clang and GCC always define __USER_LABEL_PREFIX__, so this should
// only come up with MSVC, and Windows doesn't use a prefix.
#define LANGUAGE_SYMBOL_PREFIX_STRING ""
#endif

// An attribute to override the symbol name of a declaration.
// This does not compensate for platform symbol prefixes; for that,
// use LANGUAGE_ASM_LABEL_WITH_PREFIX.
//
// This only actually works on Clang or GCC; MSVC does not provide
// an attribute to change the asm label.
#define LANGUAGE_ASM_LABEL_RAW(STRING) __asm__(STRING)
#define LANGUAGE_ASM_LABEL_WITH_PREFIX(STRING) \
  LANGUAGE_ASM_LABEL_RAW(LANGUAGE_SYMBOL_PREFIX_STRING STRING)

// LANGUAGE_FORMAT(fmt,first) marks a function as taking a format string argument
// at argument `fmt`, with the first argument for the format string as `first`.
#if __has_attribute(format)
#define LANGUAGE_FORMAT(fmt, first) __attribute__((format(printf, fmt, first)))
#else
#define LANGUAGE_FORMAT(fmt, first)
#endif

// LANGUAGE_VFORMAT(fmt) marks a function as taking a format string argument at
// argument `fmt`, with the arguments in a `va_list`.
#if __has_attribute(format)
#define LANGUAGE_VFORMAT(fmt) __attribute__((format(printf, fmt, 0)))
#else
#define LANGUAGE_VFORMAT(fmt)
#endif

#if __has_attribute(enum_extensibility)
#define ENUM_EXTENSIBILITY_ATTR(arg) __attribute__((enum_extensibility(arg)))
#else
#define ENUM_EXTENSIBILITY_ATTR(arg)
#endif

// The 'u8' string literal prefix creates `char` types on C++14/17 but
// `char8_t` types on C++20. To support compiling in both modes
// simultaneously, wrap Unicode literals in `LANGUAGE_UTF8("...")` to ensure
// that they are interpreted by the compiler as UTF-8 but always return
// `char` types.
#if defined(__cplusplus)
#if defined(__cpp_char8_t)
inline constexpr char operator""_language_u8(char8_t c) { return c; }
inline const char *operator""_language_u8(const char8_t *p, size_t) {
  return reinterpret_cast<const char *>(p);
}
#define LANGUAGE_UTF8(literal) u8##literal##_language_u8
#else  // !defined(__cpp_char8_t)
#define LANGUAGE_UTF8(literal) u8##literal
#endif // defined(__cpp_char8_t)
#endif // defined(__cplusplus)

#endif // LANGUAGE_BASIC_COMPILER_H
