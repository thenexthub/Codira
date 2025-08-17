//===-- ComponentPath.h -----------------------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_LOWER_COMPONENTPATH_H
#define LANGUAGE_COMPABILITY_LOWER_COMPONENTPATH_H

#include "language/Compability/Lower/IterationSpace.h"
#include "toolchain/ADT/SmallVector.h"
#include <optional>

namespace fir {
class ArrayLoadOp;
}
namespace language::Compability::evaluate {
class ArrayRef;
}

namespace language::Compability::lower {

namespace details {
class ImplicitSubscripts {};
} // namespace details

using PathComponent =
    std::variant<const evaluate::ArrayRef *, const evaluate::Component *,
                 const evaluate::ComplexPart *, details::ImplicitSubscripts>;

/// Collection of components.
///
/// This class is used both to collect front-end post-order functional Expr
/// trees and their translations to Values to be used in a pre-order list of
/// arguments.
class ComponentPath {
public:
  using ExtendRefFunc = std::function<mlir::Value(const mlir::Value &)>;

  ComponentPath(bool isImplicit) { setPC(isImplicit); }
  ComponentPath(bool isImplicit, const evaluate::Substring *ss)
      : substring(ss) {
    setPC(isImplicit);
  }
  ComponentPath() = delete;

  bool isSlice() const { return !trips.empty() || hasComponents(); }
  bool hasComponents() const { return !suffixComponents.empty(); }
  void clear();

  bool hasExtendCoorRef() const { return extendCoorRef.has_value(); }
  ExtendRefFunc getExtendCoorRef() const;
  void resetExtendCoorRef() { extendCoorRef = std::nullopt; }
  void resetPC();

  toolchain::SmallVector<PathComponent> reversePath;
  const evaluate::Substring *substring = nullptr;
  bool applied = false;

  toolchain::SmallVector<mlir::Value> prefixComponents;
  toolchain::SmallVector<mlir::Value> trips;
  toolchain::SmallVector<mlir::Value> suffixComponents;
  std::function<IterationSpace(const IterationSpace &)> pc;

  /// In the case where a path of components involves members that are POINTER
  /// or ALLOCATABLE, a dereference is required in FIR for semantic correctness.
  /// This optional continuation allows the generation of those dereferences.
  /// These accesses are always on Fortran entities of record types, which are
  /// implicitly in-memory objects.
  std::optional<ExtendRefFunc> extendCoorRef;

private:
  void setPC(bool isImplicit);
};

/// Examine each subscript expression of \p x and return true if and only if any
/// of the subscripts is a vector or has a rank greater than 0.
bool isRankedArrayAccess(const language::Compability::evaluate::ArrayRef &x);

} // namespace language::Compability::lower

#endif // FORTRAN_LOWER_COMPONENTPATH_H
