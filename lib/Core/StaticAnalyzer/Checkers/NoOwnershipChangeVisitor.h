//===--------------------------------------------------------------*- C++ -*--//
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

#include "language/Core/StaticAnalyzer/Core/BugReporter/BugReporterVisitors.h"
#include "language/Core/StaticAnalyzer/Core/Checker.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/ProgramState_Fwd.h"

namespace language::Core {
namespace ento {

class NoOwnershipChangeVisitor : public NoStateChangeFuncVisitor {
protected:
  // The symbol whose (lack of) ownership change we are interested in.
  SymbolRef Sym;
  const CheckerBackend &Checker;

  LLVM_DUMP_METHOD static std::string
  getFunctionName(const ExplodedNode *CallEnterN);

  /// Heuristically guess whether the callee intended to free the resource. This
  /// is done syntactically, because we are trying to argue about alternative
  /// paths of execution, and as a consequence we don't have path-sensitive
  /// information.
  virtual bool doesFnIntendToHandleOwnership(const Decl *Callee,
                                             ASTContext &ACtx) = 0;

  virtual bool hasResourceStateChanged(ProgramStateRef CallEnterState,
                                       ProgramStateRef CallExitEndState) = 0;

  bool wasModifiedInFunction(const ExplodedNode *CallEnterN,
                             const ExplodedNode *CallExitEndN) final;

  virtual PathDiagnosticPieceRef emitNote(const ExplodedNode *N) = 0;

  PathDiagnosticPieceRef maybeEmitNoteForObjCSelf(PathSensitiveBugReport &R,
                                                  const ObjCMethodCall &Call,
                                                  const ExplodedNode *N) final {
    // TODO: Implement.
    return nullptr;
  }

  PathDiagnosticPieceRef maybeEmitNoteForCXXThis(PathSensitiveBugReport &R,
                                                 const CXXConstructorCall &Call,
                                                 const ExplodedNode *N) final {
    // TODO: Implement.
    return nullptr;
  }

  // Set this to final, effectively dispatch to emitNote.
  PathDiagnosticPieceRef
  maybeEmitNoteForParameters(PathSensitiveBugReport &R, const CallEvent &Call,
                             const ExplodedNode *N) final;

public:
  using OwnerSet = toolchain::SmallPtrSet<const MemRegion *, 8>;

private:
  OwnerSet getOwnersAtNode(const ExplodedNode *N);

public:
  NoOwnershipChangeVisitor(SymbolRef Sym, const CheckerBackend *Checker)
      : NoStateChangeFuncVisitor(bugreporter::TrackingKind::Thorough), Sym(Sym),
        Checker(*Checker) {}

  void Profile(toolchain::FoldingSetNodeID &ID) const override {
    static int Tag = 0;
    ID.AddPointer(&Tag);
    ID.AddPointer(Sym);
  }
};
} // namespace ento
} // namespace language::Core
