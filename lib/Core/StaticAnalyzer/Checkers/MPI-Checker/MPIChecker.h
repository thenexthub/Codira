//===-- MPIChecker.h - Verify MPI API usage- --------------------*- C++ -*-===//
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
///
/// \file
/// This file defines the main class of MPI-Checker which serves as an entry
/// point. It is created once for each translation unit analysed.
/// The checker defines path-sensitive checks, to verify correct usage of the
/// MPI API.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_STATICANALYZER_CHECKERS_MPICHECKER_MPICHECKER_H
#define LANGUAGE_CORE_LIB_STATICANALYZER_CHECKERS_MPICHECKER_MPICHECKER_H

#include "MPIBugReporter.h"
#include "MPITypes.h"
#include "language/Core/StaticAnalyzer/Checkers/MPIFunctionClassifier.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"

namespace language::Core {
namespace ento {
namespace mpi {

class MPIChecker : public Checker<check::PreCall, check::DeadSymbols> {
public:
  MPIChecker() : BReporter(*this) {}

  // path-sensitive callbacks
  void checkPreCall(const CallEvent &CE, CheckerContext &Ctx) const {
    dynamicInit(Ctx);
    checkUnmatchedWaits(CE, Ctx);
    checkDoubleNonblocking(CE, Ctx);
  }

  void checkDeadSymbols(SymbolReaper &SymReaper, CheckerContext &Ctx) const {
    dynamicInit(Ctx);
    checkMissingWaits(SymReaper, Ctx);
  }

  void dynamicInit(CheckerContext &Ctx) const {
    if (FuncClassifier)
      return;
    const_cast<std::unique_ptr<MPIFunctionClassifier> &>(FuncClassifier)
        .reset(new MPIFunctionClassifier{Ctx.getASTContext()});
  }

  /// Checks if a request is used by nonblocking calls multiple times
  /// in sequence without intermediate wait. The check contains a guard,
  /// in order to only inspect nonblocking functions.
  ///
  /// \param PreCallEvent MPI call to verify
  void checkDoubleNonblocking(const language::Core::ento::CallEvent &PreCallEvent,
                              language::Core::ento::CheckerContext &Ctx) const;

  /// Checks if the request used by the wait function was not used at all
  /// before. The check contains a guard, in order to only inspect wait
  /// functions.
  ///
  /// \param PreCallEvent MPI call to verify
  void checkUnmatchedWaits(const language::Core::ento::CallEvent &PreCallEvent,
                           language::Core::ento::CheckerContext &Ctx) const;

  /// Check if a nonblocking call is not matched by a wait.
  /// If a memory region is not alive and the last function using the
  /// request was a nonblocking call, this is rated as a missing wait.
  void checkMissingWaits(language::Core::ento::SymbolReaper &SymReaper,
                         language::Core::ento::CheckerContext &Ctx) const;

private:
  /// Collects all memory regions of a request(array) used by a wait
  /// function. If the wait function uses a single request, this is a single
  /// region. For wait functions using multiple requests, multiple regions
  /// representing elements in the array are collected.
  ///
  /// \param ReqRegions vector the regions get pushed into
  /// \param MR top most region to iterate
  /// \param CE MPI wait call using the request(s)
  void allRegionsUsedByWait(
      toolchain::SmallVector<const language::Core::ento::MemRegion *, 2> &ReqRegions,
      const language::Core::ento::MemRegion *const MR, const language::Core::ento::CallEvent &CE,
      language::Core::ento::CheckerContext &Ctx) const;

  /// Returns the memory region used by a wait function.
  /// Distinguishes between MPI_Wait and MPI_Waitall.
  ///
  /// \param CE MPI wait call
  const language::Core::ento::MemRegion *
  topRegionUsedByWait(const language::Core::ento::CallEvent &CE) const;

  const std::unique_ptr<MPIFunctionClassifier> FuncClassifier;
  MPIBugReporter BReporter;
};

} // end of namespace: mpi
} // end of namespace: ento
} // end of namespace: clang

#endif
