//===--- LLVM.h - Import various common LLVM datatypes ----------*- C++ -*-===//
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
/// \file
/// Forward-declares and imports various common LLVM datatypes that
/// clang wants to use unqualified.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_BASIC_LLVM_H
#define LANGUAGE_CORE_BASIC_LLVM_H

// Do not proliferate #includes here, require clients to #include their
// dependencies.
// Casting.h has complex templates that cannot be easily forward declared.
#include "toolchain/Support/Casting.h"
// Add this header as a workaround to prevent `too few template arguments for
// class template 'SmallVector'` building error with build compilers like XL.
#include "toolchain/ADT/SmallVector.h"

namespace toolchain {
  // ADT's.
  class StringRef;
  class Twine;
  class VersionTuple;
  template<typename T> class ArrayRef;
  template<typename T> class MutableArrayRef;
  template<typename T> class OwningArrayRef;
  template<unsigned InternalLen> class SmallString;
  template<typename T, unsigned N> class SmallVector;
  template<typename T> class SmallVectorImpl;
  template <class T> class Expected;

  template<typename T>
  struct SaveAndRestore;

  // Reference counting.
  template <typename T> class IntrusiveRefCntPtr;
  template <typename T> struct IntrusiveRefCntPtrInfo;
  template <class Derived> class RefCountedBase;

  class raw_ostream;
  class raw_pwrite_stream;
  // TODO: DenseMap, ...
}


namespace language::Core {
  // Casting operators.
  using toolchain::isa;
  using toolchain::isa_and_nonnull;
  using toolchain::isa_and_present;
  using toolchain::cast;
  using toolchain::dyn_cast;
  using toolchain::dyn_cast_or_null;
  using toolchain::dyn_cast_if_present;
  using toolchain::cast_or_null;
  using toolchain::cast_if_present;

  // ADT's.
  using toolchain::ArrayRef;
  using toolchain::MutableArrayRef;
  using toolchain::OwningArrayRef;
  using toolchain::SaveAndRestore;
  using toolchain::SmallString;
  using toolchain::SmallVector;
  using toolchain::SmallVectorImpl;
  using toolchain::StringRef;
  using toolchain::Twine;
  using toolchain::VersionTuple;

  // Error handling.
  using toolchain::Expected;

  // Reference counting.
  using toolchain::IntrusiveRefCntPtr;
  using toolchain::IntrusiveRefCntPtrInfo;
  using toolchain::RefCountedBase;

  using toolchain::raw_ostream;
  using toolchain::raw_pwrite_stream;
} // end namespace language::Core.

#endif
