//===-- SimplifyConstraints.h -----------------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_ANALYSIS_FLOWSENSITIVE_SIMPLIFYCONSTRAINTS_H
#define LANGUAGE_CORE_ANALYSIS_FLOWSENSITIVE_SIMPLIFYCONSTRAINTS_H

#include "language/Core/Analysis/FlowSensitive/Arena.h"
#include "language/Core/Analysis/FlowSensitive/Formula.h"
#include "toolchain/ADT/SetVector.h"

namespace language::Core {
namespace dataflow {

/// Information on the way a set of constraints was simplified.
struct SimplifyConstraintsInfo {
  /// List of equivalence classes of atoms. For each equivalence class, the
  /// original constraints imply that all atoms in it must be equivalent.
  /// Simplification replaces all occurrences of atoms in an equivalence class
  /// with a single representative atom from the class.
  /// Does not contain equivalence classes with just one member or atoms
  /// contained in `TrueAtoms` or `FalseAtoms`.
  toolchain::SmallVector<toolchain::SmallVector<Atom>> EquivalentAtoms;
  /// Atoms that the original constraints imply must be true.
  /// Simplification replaces all occurrences of these atoms by a true literal
  /// (which may enable additional simplifications).
  toolchain::SmallVector<Atom> TrueAtoms;
  /// Atoms that the original constraints imply must be false.
  /// Simplification replaces all occurrences of these atoms by a false literal
  /// (which may enable additional simplifications).
  toolchain::SmallVector<Atom> FalseAtoms;
};

/// Simplifies a set of constraints (implicitly connected by "and") in a way
/// that does not change satisfiability of the constraints. This does _not_ mean
/// that the set of solutions is the same before and after simplification.
/// `Info`, if non-null, will be populated with information about the
/// simplifications that were made to the formula (e.g. to display to the user).
void simplifyConstraints(toolchain::SetVector<const Formula *> &Constraints,
                         Arena &arena, SimplifyConstraintsInfo *Info = nullptr);

} // namespace dataflow
} // namespace language::Core

#endif // LANGUAGE_CORE_ANALYSIS_FLOWSENSITIVE_SIMPLIFYCONSTRAINTS_H
