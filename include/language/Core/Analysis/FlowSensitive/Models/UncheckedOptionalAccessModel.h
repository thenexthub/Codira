//===-- UncheckedOptionalAccessModel.h --------------------------*- C++ -*-===//
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
//  This file defines a dataflow analysis that detects unsafe uses of optional
//  values.
//
//===----------------------------------------------------------------------===//

#ifndef CLANG_ANALYSIS_FLOWSENSITIVE_MODELS_UNCHECKEDOPTIONALACCESSMODEL_H
#define CLANG_ANALYSIS_FLOWSENSITIVE_MODELS_UNCHECKEDOPTIONALACCESSMODEL_H

#include "language/Core/AST/ASTContext.h"
#include "language/Core/Analysis/CFG.h"
#include "language/Core/Analysis/FlowSensitive/CFGMatchSwitch.h"
#include "language/Core/Analysis/FlowSensitive/CachedConstAccessorsLattice.h"
#include "language/Core/Analysis/FlowSensitive/DataflowAnalysis.h"
#include "language/Core/Analysis/FlowSensitive/DataflowEnvironment.h"
#include "language/Core/Analysis/FlowSensitive/MatchSwitch.h"
#include "language/Core/Analysis/FlowSensitive/NoopLattice.h"
#include "language/Core/Basic/SourceLocation.h"
#include "toolchain/ADT/SmallVector.h"

namespace language::Core {
namespace dataflow {

// FIXME: Explore using an allowlist-approach, where constructs supported by the
// analysis are always enabled and additional constructs are enabled through the
// `Options`.
struct UncheckedOptionalAccessModelOptions {
  /// In generating diagnostics, ignore optionals reachable through overloaded
  /// `operator*` or `operator->` (other than those of the optional type
  /// itself). The analysis does not equate the results of such calls, so it
  /// can't identify when their results are used safely (across calls),
  /// resulting in false positives in all such cases. Note: this option does not
  /// cover access through `operator[]`.
  ///
  /// FIXME: we now cache and equate the result of const accessors
  /// that look like unique_ptr, have both `->` (returning a pointer type) and
  /// `*` (returning a reference type). This includes mixing `->` and
  /// `*` in a sequence of calls as long as the object is not modified. Once we
  /// are confident in this const accessor caching, we shouldn't need the
  /// IgnoreSmartPointerDereference option anymore.
  bool IgnoreSmartPointerDereference = false;
};

using UncheckedOptionalAccessLattice = CachedConstAccessorsLattice<NoopLattice>;

/// Dataflow analysis that models whether optionals hold values or not.
///
/// Models the `std::optional`, `absl::optional`, and `base::Optional` types.
class UncheckedOptionalAccessModel
    : public DataflowAnalysis<UncheckedOptionalAccessModel,
                              UncheckedOptionalAccessLattice> {
public:
  UncheckedOptionalAccessModel(ASTContext &Ctx, dataflow::Environment &Env);

  /// Returns a matcher for the optional classes covered by this model.
  static ast_matchers::DeclarationMatcher optionalClassDecl();

  static UncheckedOptionalAccessLattice initialElement() { return {}; }

  void transfer(const CFGElement &Elt, UncheckedOptionalAccessLattice &L,
                Environment &Env);

private:
  CFGMatchSwitch<TransferState<UncheckedOptionalAccessLattice>>
      TransferMatchSwitch;
};

/// Diagnostic information for an unchecked optional access.
struct UncheckedOptionalAccessDiagnostic {
  CharSourceRange Range;
};

class UncheckedOptionalAccessDiagnoser {
public:
  UncheckedOptionalAccessDiagnoser(
      UncheckedOptionalAccessModelOptions Options = {});

  toolchain::SmallVector<UncheckedOptionalAccessDiagnostic>
  operator()(const CFGElement &Elt, ASTContext &Ctx,
             const TransferStateForDiagnostics<UncheckedOptionalAccessLattice>
                 &State) {
    return DiagnoseMatchSwitch(Elt, Ctx, State.Env);
  }

private:
  CFGMatchSwitch<const Environment,
                 toolchain::SmallVector<UncheckedOptionalAccessDiagnostic>>
      DiagnoseMatchSwitch;
};

} // namespace dataflow
} // namespace language::Core

#endif // CLANG_ANALYSIS_FLOWSENSITIVE_MODELS_UNCHECKEDOPTIONALACCESSMODEL_H
