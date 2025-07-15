//===--- Visibility.h - Visibility macros for runtime exports ---*- C++ -*-===//
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
//  These macros are used to declare symbols that should be exported from the
//  Codira runtime.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_STDLIB_SHIMS_VISIBILITY_H
#define LANGUAGE_STDLIB_SHIMS_VISIBILITY_H

#if !defined(__has_feature)
#define __has_feature(x) 0
#endif

#if !defined(__has_attribute)
#define __has_attribute(x) 0
#endif

#if !defined(__has_builtin)
#define __has_builtin(builtin) 0
#endif

#if !defined(__has_cpp_attribute)
#define __has_cpp_attribute(attribute) 0
#endif

// TODO: These macro definitions are duplicated in BridgedCodiraObject.h. Move
// them to a single file if we find a location that both Visibility.h and
// BridgedCodiraObject.h can import.
#if __has_feature(nullability)
// Provide macros to temporarily suppress warning about the use of
// _Nullable and _Nonnull.
# define LANGUAGE_BEGIN_NULLABILITY_ANNOTATIONS                        \
  _Pragma("clang diagnostic push")                                  \
  _Pragma("clang diagnostic ignored \"-Wnullability-extension\"")
# define LANGUAGE_END_NULLABILITY_ANNOTATIONS                          \
  _Pragma("clang diagnostic pop")

#else
// #define _Nullable and _Nonnull to nothing if we're not being built
// with a compiler that supports them.
# define _Nullable
# define _Nonnull
# define LANGUAGE_BEGIN_NULLABILITY_ANNOTATIONS
# define LANGUAGE_END_NULLABILITY_ANNOTATIONS
#endif

#define LANGUAGE_MACRO_CONCAT(A, B) A ## B
#define LANGUAGE_MACRO_IF_0(IF_TRUE, IF_FALSE) IF_FALSE
#define LANGUAGE_MACRO_IF_1(IF_TRUE, IF_FALSE) IF_TRUE
#define LANGUAGE_MACRO_IF(COND, IF_TRUE, IF_FALSE) \
  LANGUAGE_MACRO_CONCAT(LANGUAGE_MACRO_IF_, COND)(IF_TRUE, IF_FALSE)

#if __has_attribute(pure)
#define LANGUAGE_READONLY __attribute__((__pure__))
#else
#define LANGUAGE_READONLY
#endif

#if __has_attribute(const)
#define LANGUAGE_READNONE __attribute__((__const__))
#else
#define LANGUAGE_READNONE
#endif

#if __has_attribute(always_inline)
#define LANGUAGE_ALWAYS_INLINE __attribute__((always_inline))
#else
#define LANGUAGE_ALWAYS_INLINE
#endif

#if __has_attribute(noinline)
#define LANGUAGE_NOINLINE __attribute__((__noinline__))
#else
#define LANGUAGE_NOINLINE
#endif

#if __has_attribute(noreturn)
#define LANGUAGE_NORETURN __attribute__((__noreturn__))
#else
#define LANGUAGE_NORETURN
#endif

#if __has_attribute(used)
#define LANGUAGE_USED __attribute__((__used__))
#else
#define LANGUAGE_USED
#endif

#if __has_attribute(unavailable)
#define LANGUAGE_ATTRIBUTE_UNAVAILABLE __attribute__((__unavailable__))
#else
#define LANGUAGE_ATTRIBUTE_UNAVAILABLE
#endif

#if (__has_attribute(weak_import))
#define LANGUAGE_WEAK_IMPORT __attribute__((weak_import))
#else
#define LANGUAGE_WEAK_IMPORT
#endif

#if __has_attribute(musttail)
#define LANGUAGE_MUSTTAIL [[clang::musttail]]
#else
#define LANGUAGE_MUSTTAIL
#endif

// Define the appropriate attributes for sharing symbols across
// image (executable / shared-library) boundaries.
//
// LANGUAGE_ATTRIBUTE_FOR_EXPORTS will be placed on declarations that
// are known to be exported from the current image.  Typically, they
// are placed on header declarations and then inherited by the actual
// definitions.
//
// LANGUAGE_ATTRIBUTE_FOR_IMPORTS will be placed on declarations that
// are known to be exported from a different image.  This never
// includes a definition.
//
// Getting the right attribute on a declaration can be pretty awkward,
// but it's necessary under the C translation model.  All of this
// ceremony is familiar to Windows programmers; C/C++ programmers
// everywhere else usually don't bother, but since we have to get it
// right for Windows, we have everything set up to get it right on
// other targets as well, and doing so lets the compiler use more
// efficient symbol access patterns.
#if defined(__MACH__) || defined(__wasm__)

// On Mach-O and WebAssembly, we use non-hidden visibility.  We just use
// default visibility on both imports and exports, both because these
// targets don't support protected visibility but because they don't
// need it: symbols are not interposable outside the current image
// by default.
# define LANGUAGE_ATTRIBUTE_FOR_EXPORTS __attribute__((__visibility__("default")))
# define LANGUAGE_ATTRIBUTE_FOR_IMPORTS __attribute__((__visibility__("default")))

#elif defined(__ELF__)

// On ELF, we use non-hidden visibility.  For exports, we must use
// protected visibility to tell the compiler and linker that the symbols
// can't be interposed outside the current image.  For imports, we must
// use default visibility because protected visibility guarantees that
// the symbol is defined in the current library, which isn't true for
// an import.
//
// The compiler does assume that the runtime and standard library can
// refer to each other's symbols as DSO-local, so it's important that
// we get this right or we can get linker errors.
# define LANGUAGE_ATTRIBUTE_FOR_EXPORTS __attribute__((__visibility__("protected")))
# define LANGUAGE_ATTRIBUTE_FOR_IMPORTS __attribute__((__visibility__("default")))

#elif defined(__CYGWIN__)

// For now, we ignore all this on Cygwin.
# define LANGUAGE_ATTRIBUTE_FOR_EXPORTS
# define LANGUAGE_ATTRIBUTE_FOR_IMPORTS

// FIXME: this #else should be some sort of #elif Windows
#else // !__MACH__ && !__ELF__

# if defined(LANGUAGE_STATIC_STDLIB)
#   define LANGUAGE_ATTRIBUTE_FOR_EXPORTS /**/
#   define LANGUAGE_ATTRIBUTE_FOR_IMPORTS /**/
# else
#   define LANGUAGE_ATTRIBUTE_FOR_EXPORTS __declspec(dllexport)
#   define LANGUAGE_ATTRIBUTE_FOR_IMPORTS __declspec(dllimport)
# endif

#endif

// CMake conventionally passes -DlibraryName_EXPORTS when building
// code that goes into libraryName.  This isn't the best macro name,
// but it's conventional.  We do have to pass it explicitly in a few
// places in the build system for a variety of reasons.
//
// Unfortunately, defined(D) is a special function you can use in
// preprocessor conditions, not a macro you can use anywhere, so we
// need to manually check for all the libraries we know about so that
// we can use them in our condition below.s
#if defined(languageCore_EXPORTS)
#define LANGUAGE_IMAGE_EXPORTS_languageCore 1
#else
#define LANGUAGE_IMAGE_EXPORTS_languageCore 0
#endif
#if defined(language_Concurrency_EXPORTS)
#define LANGUAGE_IMAGE_EXPORTS_language_Concurrency 1
#else
#define LANGUAGE_IMAGE_EXPORTS_language_Concurrency 0
#endif
#if defined(languageDistributed_EXPORTS)
#define LANGUAGE_IMAGE_EXPORTS_languageDistributed 1
#else
#define LANGUAGE_IMAGE_EXPORTS_languageDistributed 0
#endif
#if defined(language_Differentiation_EXPORTS)
#define LANGUAGE_IMAGE_EXPORTS_language_Differentiation 1
#else
#define LANGUAGE_IMAGE_EXPORTS_language_Differentiation 0
#endif

#define LANGUAGE_EXPORT_FROM_ATTRIBUTE(LIBRARY)                          \
  LANGUAGE_MACRO_IF(LANGUAGE_IMAGE_EXPORTS_##LIBRARY,                       \
                 LANGUAGE_ATTRIBUTE_FOR_EXPORTS,                         \
                 LANGUAGE_ATTRIBUTE_FOR_IMPORTS)

// LANGUAGE_EXPORT_FROM(LIBRARY) declares something to be a C-linkage
// entity exported by the given library.
//
// LANGUAGE_RUNTIME_EXPORT is just LANGUAGE_EXPORT_FROM(languageCore).
//
// TODO: use this in shims headers in overlays.
#if defined(__cplusplus)
#define LANGUAGE_EXPORT_FROM(LIBRARY) extern "C" LANGUAGE_EXPORT_FROM_ATTRIBUTE(LIBRARY)
#define LANGUAGE_EXTERN_C extern "C" 
#else
#define LANGUAGE_EXPORT_FROM(LIBRARY) LANGUAGE_EXPORT_FROM_ATTRIBUTE(LIBRARY)
#define LANGUAGE_EXTERN_C
#endif
#define LANGUAGE_RUNTIME_EXPORT LANGUAGE_EXPORT_FROM(languageCore)
#define LANGUAGE_RUNTIME_EXPORT_ATTRIBUTE LANGUAGE_EXPORT_FROM_ATTRIBUTE(languageCore)

#if __cplusplus > 201402l && __has_cpp_attribute(fallthrough)
#define LANGUAGE_FALLTHROUGH [[fallthrough]]
#elif __has_cpp_attribute(gnu::fallthrough)
#define LANGUAGE_FALLTHROUGH [[gnu::fallthrough]]
#elif __has_cpp_attribute(clang::fallthrough)
#define LANGUAGE_FALLTHROUGH [[clang::fallthrough]]
#elif __has_attribute(fallthrough)
#define LANGUAGE_FALLTHROUGH __attribute__((__fallthrough__))
#else
#define LANGUAGE_FALLTHROUGH
#endif

#if __cplusplus > 201402l && __has_cpp_attribute(nodiscard)
#define LANGUAGE_NODISCARD [[nodiscard]]
#elif __has_cpp_attribute(clang::warn_unused_result)
#define LANGUAGE_NODISCARD [[clang::warn_unused_result]]
#else
#define LANGUAGE_NODISCARD
#endif

#if __has_cpp_attribute(gnu::returns_nonnull)
#define LANGUAGE_RETURNS_NONNULL [[gnu::returns_nonnull]]
#elif defined(_MSC_VER) && defined(_Ret_notnull_)
#define LANGUAGE_RETURNS_NONNULL _Ret_notnull_
#else
#define LANGUAGE_RETURNS_NONNULL
#endif

/// Attributes for runtime-stdlib interfaces.
/// Use these for C implementations that are imported into Codira via CodiraShims
/// and for C implementations of Codira @_silgen_name declarations
/// Note that @_silgen_name implementations must also be marked LANGUAGE_CC(language).
///
/// LANGUAGE_RUNTIME_STDLIB_API functions are called by compiler-generated code
/// or by @inlinable Codira code.
/// Such functions must be exported and must be supported forever as API.
/// The function name should be prefixed with `language_`.
///
/// LANGUAGE_RUNTIME_STDLIB_SPI functions are called by overlay code.
/// Such functions must be exported, but are still SPI
/// and may be changed at any time.
/// The function name should be prefixed with `_language_`.
///
/// LANGUAGE_RUNTIME_STDLIB_INTERNAL functions are called only by the stdlib.
/// Such functions are internal and are not exported.
#define LANGUAGE_RUNTIME_STDLIB_API       LANGUAGE_RUNTIME_EXPORT
#define LANGUAGE_RUNTIME_STDLIB_SPI       LANGUAGE_RUNTIME_EXPORT

// Match the definition of TOOLCHAIN_LIBRARY_VISIBILITY from LLVM's
// Compiler.h. That header requires C++ and this needs to work in C.
#if __has_attribute(visibility) && (defined(__ELF__) || defined(__MACH__))
#define LANGUAGE_LIBRARY_VISIBILITY __attribute__ ((__visibility__("hidden")))
#else
#define LANGUAGE_LIBRARY_VISIBILITY
#endif

#if defined(__cplusplus)
#define LANGUAGE_RUNTIME_STDLIB_INTERNAL extern "C" LANGUAGE_LIBRARY_VISIBILITY
#else
#define LANGUAGE_RUNTIME_STDLIB_INTERNAL LANGUAGE_LIBRARY_VISIBILITY
#endif

#if __has_builtin(__builtin_expect)
#define LANGUAGE_LIKELY(expression) (__builtin_expect(!!(expression), 1))
#define LANGUAGE_UNLIKELY(expression) (__builtin_expect(!!(expression), 0))
#else
#define LANGUAGE_LIKELY(expression) ((expression))
#define LANGUAGE_UNLIKELY(expression) ((expression))
#endif

// LANGUAGE_STDLIB_SHIMS_VISIBILITY_H
#endif
