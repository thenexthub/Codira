//===--- Toolchain.h - Import various common LLVM datatypes ----------*- C++ -*-===//
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
// This file forward declares and imports various common LLVM datatypes that
// language wants to use unqualified.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_BASIC_TOOLCHAIN_H
#define LANGUAGE_BASIC_TOOLCHAIN_H

// Do not proliferate #includes here, require clients to #include their
// dependencies.
// Casting.h has complex templates that cannot be easily forward declared.
#include "toolchain/Support/Casting.h"

#if defined(__clang_major__) && __clang_major__ < 6
// Add this header as a workaround to prevent `too few template arguments for
// class template 'SmallVector'` on the buggy Clang 5 compiler (it doesn't
// merge template arguments correctly). Remove once the CentOS 7 job is
// replaced.
// rdar://98218902
#include "toolchain/ADT/SmallVector.h"
#endif

// Don't pre-declare certain LLVM types in the runtime, which must
// not put things in namespace toolchain for ODR reasons.
#if defined(LANGUAGE_RUNTIME)
#define LANGUAGE_TOOLCHAIN_ODR_SAFE 0
#else
#define LANGUAGE_TOOLCHAIN_ODR_SAFE 1
#endif

// Forward declarations.
namespace toolchain {
  // Containers.
  class StringRef;
  class StringLiteral;
  class Twine;
  template <typename T> class SmallPtrSetImpl;
  template <typename T, unsigned N> class SmallPtrSet;
#if LANGUAGE_TOOLCHAIN_ODR_SAFE
  template <typename T> class SmallVectorImpl;
  template <typename T, unsigned N> class SmallVector;
#endif
  template <unsigned N> class SmallString;
#if LANGUAGE_TOOLCHAIN_ODR_SAFE
  template<typename T> class ArrayRef;
  template<typename T> class MutableArrayRef;
#endif
  template <typename T>
  class TinyPtrVector;
  template <typename ...PTs> class PointerUnion;
  template <typename IteratorT> class iterator_range;
  class SmallBitVector;

  // Other common classes.
  class raw_ostream;
  class APInt;
  class APFloat;
#if LANGUAGE_TOOLCHAIN_ODR_SAFE
  template <typename Fn> class function_ref;
#endif
} // end namespace toolchain


namespace language {
  // Casting operators.
  using toolchain::isa;
  using toolchain::isa_and_nonnull;
  using toolchain::cast;
  using toolchain::dyn_cast;
  using toolchain::dyn_cast_or_null;
  using toolchain::cast_or_null;

  // Containers.
#if LANGUAGE_TOOLCHAIN_ODR_SAFE
  using toolchain::ArrayRef;
  using toolchain::MutableArrayRef;
#endif
  using toolchain::iterator_range;
  using toolchain::PointerUnion;
  using toolchain::SmallBitVector;
  using toolchain::SmallPtrSet;
  using toolchain::SmallPtrSetImpl;
  using toolchain::SmallString;
#if LANGUAGE_TOOLCHAIN_ODR_SAFE
  using toolchain::SmallVector;
  using toolchain::SmallVectorImpl;
#endif
  using toolchain::StringLiteral;
  using toolchain::StringRef;
  using toolchain::TinyPtrVector;
  using toolchain::Twine;

  // Other common classes.
  using toolchain::APFloat;
  using toolchain::APInt;
#if LANGUAGE_TOOLCHAIN_ODR_SAFE
  using toolchain::function_ref;
#endif
  using toolchain::raw_ostream;
} // end namespace language

#endif // LANGUAGE_BASIC_TOOLCHAIN_H
