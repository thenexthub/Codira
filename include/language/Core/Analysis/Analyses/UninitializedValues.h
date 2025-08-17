//=- UninitializedValues.h - Finding uses of uninitialized values -*- C++ -*-=//
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
// This file defines APIs for invoking and reported uninitialized values
// warnings.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_ANALYSIS_ANALYSES_UNINITIALIZEDVALUES_H
#define LANGUAGE_CORE_ANALYSIS_ANALYSES_UNINITIALIZEDVALUES_H

#include "language/Core/Basic/LLVM.h"
#include "toolchain/ADT/SmallVector.h"

namespace language::Core {

class AnalysisDeclContext;
class CFG;
class DeclContext;
class Expr;
class Stmt;
class VarDecl;

/// A use of a variable, which might be uninitialized.
class UninitUse {
public:
  struct Branch {
    const Stmt *Terminator;
    unsigned Output;
  };

private:
  /// The expression which uses this variable.
  const Expr *User;

  /// Is this use uninitialized whenever the function is called?
  bool UninitAfterCall = false;

  /// Is this use uninitialized whenever the variable declaration is reached?
  bool UninitAfterDecl = false;

  /// Does this use always see an uninitialized value?
  bool AlwaysUninit;

  /// Is this use a const reference to this variable?
  bool ConstRefUse = false;

  /// Is this use a const pointer to this variable?
  bool ConstPtrUse = false;

  /// This use is always uninitialized if it occurs after any of these branches
  /// is taken.
  SmallVector<Branch, 2> UninitBranches;

public:
  UninitUse(const Expr *User, bool AlwaysUninit)
      : User(User), AlwaysUninit(AlwaysUninit) {}

  void addUninitBranch(Branch B) {
    UninitBranches.push_back(B);
  }

  void setUninitAfterCall() { UninitAfterCall = true; }
  void setUninitAfterDecl() { UninitAfterDecl = true; }
  void setConstRefUse() { ConstRefUse = true; }
  void setConstPtrUse() { ConstPtrUse = true; }

  /// Get the expression containing the uninitialized use.
  const Expr *getUser() const { return User; }

  bool isConstRefUse() const { return ConstRefUse; }
  bool isConstPtrUse() const { return ConstPtrUse; }
  bool isConstRefOrPtrUse() const { return ConstRefUse || ConstPtrUse; }

  /// The kind of uninitialized use.
  enum Kind {
    /// The use might be uninitialized.
    Maybe,

    /// The use is uninitialized whenever a certain branch is taken.
    Sometimes,

    /// The use is uninitialized the first time it is reached after we reach
    /// the variable's declaration.
    AfterDecl,

    /// The use is uninitialized the first time it is reached after the function
    /// is called.
    AfterCall,

    /// The use is always uninitialized.
    Always
  };

  /// Get the kind of uninitialized use.
  Kind getKind() const {
    return AlwaysUninit ? Always :
           UninitAfterCall ? AfterCall :
           UninitAfterDecl ? AfterDecl :
           !branch_empty() ? Sometimes : Maybe;
  }

  using branch_iterator = SmallVectorImpl<Branch>::const_iterator;

  /// Branches which inevitably result in the variable being used uninitialized.
  branch_iterator branch_begin() const { return UninitBranches.begin(); }
  branch_iterator branch_end() const { return UninitBranches.end(); }
  bool branch_empty() const { return UninitBranches.empty(); }
};

class UninitVariablesHandler {
public:
  UninitVariablesHandler() = default;
  virtual ~UninitVariablesHandler();

  /// Called when the uninitialized variable is used at the given expression.
  virtual void handleUseOfUninitVariable(const VarDecl *vd,
                                         const UninitUse &use) {}

  /// Called when the uninitialized variable analysis detects the
  /// idiom 'int x = x'.  All other uses of 'x' within the initializer
  /// are handled by handleUseOfUninitVariable.
  virtual void handleSelfInit(const VarDecl *vd) {}
};

struct UninitVariablesAnalysisStats {
  unsigned NumVariablesAnalyzed;
  unsigned NumBlockVisits;
};

void runUninitializedVariablesAnalysis(const DeclContext &dc, const CFG &cfg,
                                       AnalysisDeclContext &ac,
                                       UninitVariablesHandler &handler,
                                       UninitVariablesAnalysisStats &stats);

} // namespace language::Core

#endif // LANGUAGE_CORE_ANALYSIS_ANALYSES_UNINITIALIZEDVALUES_H
