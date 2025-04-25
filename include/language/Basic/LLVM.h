//===--- LLVM.h - Import various common LLVM datatypes ----------*- C++ -*-===//
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
// swift wants to use unqualified.
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_BASIC_LLVM_H
#define SWIFT_BASIC_LLVM_H

// Do not proliferate #includes here, require clients to #include their
// dependencies.
// Casting.h has complex templates that cannot be easily forward declared.
#include "llvm/Support/Casting.h"

#if defined(__clang_major__) && __clang_major__ < 6
// Add this header as a workaround to prevent `too few template arguments for
// class template 'SmallVector'` on the buggy Clang 5 compiler (it doesn't
// merge template arguments correctly). Remove once the CentOS 7 job is
// replaced.
// rdar://98218902
#include "llvm/ADT/SmallVector.h"
#endif

// Don't pre-declare certain LLVM types in the runtime, which must
// not put things in namespace llvm for ODR reasons.
#if defined(SWIFT_RUNTIME)
#define SWIFT_LLVM_ODR_SAFE 0
#else
#define SWIFT_LLVM_ODR_SAFE 1
#endif

// Forward declarations.
namespace llvm {
  // Containers.
  class StringRef;
  class StringLiteral;
  class Twine;
  template <typename T> class SmallPtrSetImpl;
  template <typename T, unsigned N> class SmallPtrSet;
#if SWIFT_LLVM_ODR_SAFE
  template <typename T> class SmallVectorImpl;
  template <typename T, unsigned N> class SmallVector;
#endif
  template <unsigned N> class SmallString;
#if SWIFT_LLVM_ODR_SAFE
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
#if SWIFT_LLVM_ODR_SAFE
  template <typename Fn> class function_ref;
#endif
} // end namespace llvm


namespace language {
  // Casting operators.
  using llvm::isa;
  using llvm::isa_and_nonnull;
  using llvm::cast;
  using llvm::dyn_cast;
  using llvm::dyn_cast_or_null;
  using llvm::cast_or_null;

  // Containers.
#if SWIFT_LLVM_ODR_SAFE
  using llvm::ArrayRef;
  using llvm::MutableArrayRef;
#endif
  using llvm::iterator_range;
  using llvm::PointerUnion;
  using llvm::SmallBitVector;
  using llvm::SmallPtrSet;
  using llvm::SmallPtrSetImpl;
  using llvm::SmallString;
#if SWIFT_LLVM_ODR_SAFE
  using llvm::SmallVector;
  using llvm::SmallVectorImpl;
#endif
  using llvm::StringLiteral;
  using llvm::StringRef;
  using llvm::TinyPtrVector;
  using llvm::Twine;

  // Other common classes.
  using llvm::APFloat;
  using llvm::APInt;
#if SWIFT_LLVM_ODR_SAFE
  using llvm::function_ref;
#endif
  using llvm::raw_ostream;
} // end namespace language

#endif // SWIFT_BASIC_LLVM_H
