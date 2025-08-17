// MmapWriteExecChecker.cpp - Check for the prot argument -----------------===//
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
// This checker tests the 3rd argument of mmap's calls to check if
// it is writable and executable in the same time. It's somehow
// an optional checker since for example in JIT libraries it is pretty common.
//
//===----------------------------------------------------------------------===//

#include "language/Core/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"

#include "language/Core/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "language/Core/StaticAnalyzer/Core/Checker.h"
#include "language/Core/StaticAnalyzer/Core/CheckerManager.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/CallDescription.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/CheckerHelpers.h"

using namespace language::Core;
using namespace ento;

namespace {
class MmapWriteExecChecker
    : public Checker<check::ASTDecl<TranslationUnitDecl>, check::PreCall> {
  CallDescription MmapFn{CDM::CLibrary, {"mmap"}, 6};
  CallDescription MprotectFn{CDM::CLibrary, {"mprotect"}, 3};
  const BugType BT{this, "W^X check fails, Write Exec prot flags set",
                   "Security"};

  // Default values are used if definition of the flags is not found.
  mutable int ProtRead = 0x01;
  mutable int ProtWrite = 0x02;
  mutable int ProtExec = 0x04;

public:
  void checkASTDecl(const TranslationUnitDecl *TU, AnalysisManager &Mgr,
                    BugReporter &BR) const;
  void checkPreCall(const CallEvent &Call, CheckerContext &C) const;
};
}

void MmapWriteExecChecker::checkASTDecl(const TranslationUnitDecl *TU,
                                        AnalysisManager &Mgr,
                                        BugReporter &BR) const {
  Preprocessor &PP = Mgr.getPreprocessor();
  const std::optional<int> FoundProtRead = tryExpandAsInteger("PROT_READ", PP);
  const std::optional<int> FoundProtWrite =
      tryExpandAsInteger("PROT_WRITE", PP);
  const std::optional<int> FoundProtExec = tryExpandAsInteger("PROT_EXEC", PP);
  if (FoundProtRead && FoundProtWrite && FoundProtExec) {
    ProtRead = *FoundProtRead;
    ProtWrite = *FoundProtWrite;
    ProtExec = *FoundProtExec;
  }
}

void MmapWriteExecChecker::checkPreCall(const CallEvent &Call,
                                        CheckerContext &C) const {
  if (matchesAny(Call, MmapFn, MprotectFn)) {
    SVal ProtVal = Call.getArgSVal(2);
    auto ProtLoc = ProtVal.getAs<nonloc::ConcreteInt>();
    if (!ProtLoc)
      return;
    int64_t Prot = ProtLoc->getValue()->getSExtValue();

    if ((Prot & ProtWrite) && (Prot & ProtExec)) {
      ExplodedNode *N = C.generateNonFatalErrorNode();
      if (!N)
        return;

      auto Report = std::make_unique<PathSensitiveBugReport>(
          BT,
          "Both PROT_WRITE and PROT_EXEC flags are set. This can "
          "lead to exploitable memory regions, which could be overwritten "
          "with malicious code",
          N);
      Report->addRange(Call.getArgSourceRange(2));
      C.emitReport(std::move(Report));
    }
  }
}

void ento::registerMmapWriteExecChecker(CheckerManager &Mgr) {
  Mgr.registerChecker<MmapWriteExecChecker>();
}

bool ento::shouldRegisterMmapWriteExecChecker(const CheckerManager &) {
  return true;
}
