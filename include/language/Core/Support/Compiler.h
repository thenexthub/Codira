//===-- clang/Support/Compiler.h - Compiler abstraction support -*- C++ -*-===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// 
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//
//
// This file defines explicit visibility macros used to export symbols from
// clang-cpp
//
//===----------------------------------------------------------------------===//

#ifndef CLANG_SUPPORT_COMPILER_H
#define CLANG_SUPPORT_COMPILER_H

#include "toolchain/Support/Compiler.h"

/// CLANG_ABI is the main export/visibility macro to mark something as
/// explicitly exported when clang is built as a shared library with everything
/// else that is unannotated having hidden visibility.
///
/// CLANG_EXPORT_TEMPLATE is used on explicit template instantiations in source
/// files that were declared extern in a header. This macro is only set as a
/// compiler export attribute on windows, on other platforms it does nothing.
///
/// CLANG_TEMPLATE_ABI is for annotating extern template declarations in headers
/// for both functions and classes. On windows its turned in to dllimport for
/// library consumers, for other platforms its a default visibility attribute.
#ifndef CLANG_ABI_GENERATING_ANNOTATIONS
// Marker to add to classes or functions in public headers that should not have
// export macros added to them by the clang tool
#define CLANG_ABI_NOT_EXPORTED
// Some libraries like those for tablegen are linked in to tools that used
// in the build so can't depend on the toolchain shared library. If export macros
// were left enabled when building these we would get duplicate or
// missing symbol linker errors on windows.
#if defined(CLANG_BUILD_STATIC)
#define CLANG_ABI
#define CLANG_TEMPLATE_ABI
#define CLANG_EXPORT_TEMPLATE
#elif defined(_WIN32) && !defined(__MINGW32__)
#if defined(CLANG_EXPORTS)
#define CLANG_ABI __declspec(dllexport)
#define CLANG_TEMPLATE_ABI
#define CLANG_EXPORT_TEMPLATE __declspec(dllexport)
#else
#define CLANG_ABI __declspec(dllimport)
#define CLANG_TEMPLATE_ABI __declspec(dllimport)
#define CLANG_EXPORT_TEMPLATE
#endif
#elif defined(__ELF__) || defined(__MINGW32__) || defined(_AIX) ||             \
    defined(__MVS__) || defined(__CYGWIN__)
#define CLANG_ABI LLVM_ATTRIBUTE_VISIBILITY_DEFAULT
#define CLANG_TEMPLATE_ABI LLVM_ATTRIBUTE_VISIBILITY_DEFAULT
#define CLANG_EXPORT_TEMPLATE
#elif defined(__MACH__) || defined(__WASM__) || defined(__EMSCRIPTEN__)
#define CLANG_ABI LLVM_ATTRIBUTE_VISIBILITY_DEFAULT
#define CLANG_TEMPLATE_ABI
#define CLANG_EXPORT_TEMPLATE
#endif
#endif

#endif
