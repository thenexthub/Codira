//===--- VarBypassDetector.h - Bypass jumps detector --------------*- C++ -*-=//
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
// This file contains VarBypassDetector class, which is used to detect
// local variable declarations which can be bypassed by jumps.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_CODEGEN_VARBYPASSDETECTOR_H
#define LANGUAGE_CORE_LIB_CODEGEN_VARBYPASSDETECTOR_H

#include "CodeGenModule.h"
#include "language/Core/AST/Decl.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/ADT/DenseSet.h"
#include "toolchain/ADT/SmallVector.h"

namespace language::Core {

class Decl;
class Stmt;
class VarDecl;

namespace CodeGen {

/// The class detects jumps which bypass local variables declaration:
///    goto L;
///    int a;
///  L:
///
/// This is simplified version of JumpScopeChecker. Primary differences:
///  * Detects only jumps into the scope local variables.
///  * Does not detect jumps out of the scope of local variables.
///  * Not limited to variables with initializers, JumpScopeChecker is limited.
class VarBypassDetector {
  // Scope information. Contains a parent scope and related variable
  // declaration.
  toolchain::SmallVector<std::pair<unsigned, const VarDecl *>, 48> Scopes;
  // List of jumps with scopes.
  toolchain::SmallVector<std::pair<const Stmt *, unsigned>, 16> FromScopes;
  // Lookup map to find scope for destinations.
  toolchain::DenseMap<const Stmt *, unsigned> ToScopes;
  // Set of variables which were bypassed by some jump.
  toolchain::DenseSet<const VarDecl *> Bypasses;
  // If true assume that all variables are being bypassed.
  bool AlwaysBypassed = false;

public:
  void Init(CodeGenModule &CGM, const Stmt *Body);

  /// Returns true if the variable declaration was by bypassed by any goto or
  /// switch statement.
  bool IsBypassed(const VarDecl *D) const {
    return AlwaysBypassed || Bypasses.contains(D);
  }

private:
  bool BuildScopeInformation(CodeGenModule &CGM, const Decl *D,
                             unsigned &ParentScope);
  bool BuildScopeInformation(CodeGenModule &CGM, const Stmt *S,
                             unsigned &origParentScope);
  void Detect();
  void Detect(unsigned From, unsigned To);
};
}
}

#endif
