//===-- ChromiumCheckModel.cpp ----------------------------------*- C++ -*-===//
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

#include "language/Core/Analysis/FlowSensitive/Models/ChromiumCheckModel.h"
#include "language/Core/AST/Decl.h"
#include "language/Core/AST/DeclCXX.h"
#include "toolchain/ADT/DenseSet.h"

namespace language::Core {
namespace dataflow {

/// Determines whether `D` is one of the methods used to implement Chromium's
/// `CHECK` macros. Populates `CheckDecls`, if empty.
static bool
isCheckLikeMethod(toolchain::SmallDenseSet<const CXXMethodDecl *> &CheckDecls,
                  const CXXMethodDecl &D) {
  // All of the methods of interest are static, so avoid any lookup for
  // non-static methods (the common case).
  if (!D.isStatic())
    return false;

  if (CheckDecls.empty()) {
    // Attempt to initialize `CheckDecls` with the methods in class
    // `CheckError`.
    const CXXRecordDecl *ParentClass = D.getParent();
    if (ParentClass == nullptr || !ParentClass->getDeclName().isIdentifier() ||
        ParentClass->getName() != "CheckError")
      return false;

    // Check whether namespace is "logging".
    const auto *N =
        dyn_cast_or_null<NamespaceDecl>(ParentClass->getDeclContext());
    if (N == nullptr || !N->getDeclName().isIdentifier() ||
        N->getName() != "logging")
      return false;

    // Check whether "logging" is a top-level namespace.
    if (N->getParent() == nullptr || !N->getParent()->isTranslationUnit())
      return false;

    for (const CXXMethodDecl *M : ParentClass->methods())
      if (M->getDeclName().isIdentifier() && M->getName().ends_with("Check"))
        CheckDecls.insert(M);
  }

  return CheckDecls.contains(&D);
}

bool ChromiumCheckModel::transfer(const CFGElement &Element, Environment &Env) {
  auto CS = Element.getAs<CFGStmt>();
  if (!CS)
    return false;
  auto Stmt = CS->getStmt();
  if (const auto *Call = dyn_cast<CallExpr>(Stmt)) {
    if (const auto *M =
            dyn_cast_or_null<CXXMethodDecl>(Call->getDirectCallee())) {
      if (isCheckLikeMethod(CheckDecls, *M)) {
        // Mark this branch as unreachable.
        Env.assume(Env.arena().makeLiteral(false));
        return true;
      }
    }
  }
  return false;
}

} // namespace dataflow
} // namespace language::Core
