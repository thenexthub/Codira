//===-- MPIBugReporter.h - bug reporter -----------------------*- C++ -*-===//
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
/// This file defines prefabricated reports which are emitted in
/// case of MPI related bugs, detected by path-sensitive analysis.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_STATICANALYZER_CHECKERS_MPICHECKER_MPIBUGREPORTER_H
#define LANGUAGE_CORE_LIB_STATICANALYZER_CHECKERS_MPICHECKER_MPIBUGREPORTER_H

#include "MPITypes.h"
#include "language/Core/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "toolchain/ADT/StringRef.h"

namespace language::Core {
namespace ento {
namespace mpi {

class MPIBugReporter {
public:
  MPIBugReporter(const CheckerBase &CB)
      : UnmatchedWaitBugType(&CB, "Unmatched wait", MPIError),
        MissingWaitBugType(&CB, "Missing wait", MPIError),
        DoubleNonblockingBugType(&CB, "Double nonblocking", MPIError) {}

  /// Report duplicate request use by nonblocking calls without intermediate
  /// wait.
  ///
  /// \param MPICallEvent MPI call that caused the double nonblocking
  /// \param Req request that was used by two nonblocking calls in sequence
  /// \param RequestRegion memory region of the request
  /// \param ExplNode node in the graph the bug appeared at
  /// \param BReporter bug reporter for current context
  void reportDoubleNonblocking(const CallEvent &MPICallEvent,
                               const Request &Req,
                               const MemRegion *const RequestRegion,
                               const ExplodedNode *const ExplNode,
                              BugReporter &BReporter) const;

  /// Report a missing wait for a nonblocking call.
  ///
  /// \param Req request that is not matched by a wait
  /// \param RequestRegion memory region of the request
  /// \param ExplNode node in the graph the bug appeared at
  /// \param BReporter bug reporter for current context
  void reportMissingWait(const Request &Req,
                         const MemRegion *const RequestRegion,
                         const ExplodedNode *const ExplNode,
                         BugReporter &BReporter) const;

  /// Report a wait on a request that has not been used at all before.
  ///
  /// \param CE wait call that uses the request
  /// \param RequestRegion memory region of the request
  /// \param ExplNode node in the graph the bug appeared at
  /// \param BReporter bug reporter for current context
  void reportUnmatchedWait(const CallEvent &CE,
                           const MemRegion *const RequestRegion,
                           const ExplodedNode *const ExplNode,
                           BugReporter &BReporter) const;

private:
  const toolchain::StringLiteral MPIError = "MPI Error";
  const BugType UnmatchedWaitBugType;
  const BugType MissingWaitBugType;
  const BugType DoubleNonblockingBugType;

  /// Bug visitor class to find the node where the request region was previously
  /// used in order to include it into the BugReport path.
  class RequestNodeVisitor : public BugReporterVisitor {
  public:
    RequestNodeVisitor(const MemRegion *const MemoryRegion,
                       const std::string &ErrText)
        : RequestRegion(MemoryRegion), ErrorText(ErrText) {}

    void Profile(toolchain::FoldingSetNodeID &ID) const override {
      static int X = 0;
      ID.AddPointer(&X);
      ID.AddPointer(RequestRegion);
    }

    PathDiagnosticPieceRef VisitNode(const ExplodedNode *N,
                                     BugReporterContext &BRC,
                                     PathSensitiveBugReport &BR) override;

  private:
    const MemRegion *const RequestRegion;
    bool IsNodeFound = false;
    std::string ErrorText;
  };
};

} // end of namespace: mpi
} // end of namespace: ento
} // end of namespace: clang

#endif
