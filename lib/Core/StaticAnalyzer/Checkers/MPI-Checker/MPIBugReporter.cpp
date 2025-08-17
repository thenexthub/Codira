//===-- MPIBugReporter.cpp - bug reporter -----------------------*- C++ -*-===//
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

#include "MPIBugReporter.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/CallEvent.h"

namespace language::Core {
namespace ento {
namespace mpi {

void MPIBugReporter::reportDoubleNonblocking(
    const CallEvent &MPICallEvent, const ento::mpi::Request &Req,
    const MemRegion *const RequestRegion,
    const ExplodedNode *const ExplNode,
    BugReporter &BReporter) const {

  std::string ErrorText;
  ErrorText = "Double nonblocking on request " +
              RequestRegion->getDescriptiveName() + ". ";

  auto Report = std::make_unique<PathSensitiveBugReport>(
      DoubleNonblockingBugType, ErrorText, ExplNode);

  Report->addRange(MPICallEvent.getSourceRange());
  SourceRange Range = RequestRegion->sourceRange();

  if (Range.isValid())
    Report->addRange(Range);

  Report->addVisitor(std::make_unique<RequestNodeVisitor>(
      RequestRegion, "Request is previously used by nonblocking call here. "));
  Report->markInteresting(RequestRegion);

  BReporter.emitReport(std::move(Report));
}

void MPIBugReporter::reportMissingWait(
    const ento::mpi::Request &Req, const MemRegion *const RequestRegion,
    const ExplodedNode *const ExplNode,
    BugReporter &BReporter) const {
  std::string ErrorText{"Request " + RequestRegion->getDescriptiveName() +
                        " has no matching wait. "};

  auto Report = std::make_unique<PathSensitiveBugReport>(MissingWaitBugType,
                                                         ErrorText, ExplNode);

  SourceRange Range = RequestRegion->sourceRange();
  if (Range.isValid())
    Report->addRange(Range);
  Report->addVisitor(std::make_unique<RequestNodeVisitor>(
      RequestRegion, "Request is previously used by nonblocking call here. "));
  Report->markInteresting(RequestRegion);

  BReporter.emitReport(std::move(Report));
}

void MPIBugReporter::reportUnmatchedWait(
    const CallEvent &CE, const language::Core::ento::MemRegion *const RequestRegion,
    const ExplodedNode *const ExplNode,
    BugReporter &BReporter) const {
  std::string ErrorText{"Request " + RequestRegion->getDescriptiveName() +
                        " has no matching nonblocking call. "};

  auto Report = std::make_unique<PathSensitiveBugReport>(UnmatchedWaitBugType,
                                                         ErrorText, ExplNode);

  Report->addRange(CE.getSourceRange());
  SourceRange Range = RequestRegion->sourceRange();
  if (Range.isValid())
    Report->addRange(Range);

  BReporter.emitReport(std::move(Report));
}

PathDiagnosticPieceRef
MPIBugReporter::RequestNodeVisitor::VisitNode(const ExplodedNode *N,
                                              BugReporterContext &BRC,
                                              PathSensitiveBugReport &BR) {

  if (IsNodeFound)
    return nullptr;

  const Request *const Req = N->getState()->get<RequestMap>(RequestRegion);
  assert(Req && "The region must be tracked and alive, given that we've "
                "just emitted a report against it");
  const Request *const PrevReq =
      N->getFirstPred()->getState()->get<RequestMap>(RequestRegion);

  // Check if request was previously unused or in a different state.
  if (!PrevReq || (Req->CurrentState != PrevReq->CurrentState)) {
    IsNodeFound = true;

    ProgramPoint P = N->getFirstPred()->getLocation();
    PathDiagnosticLocation L =
        PathDiagnosticLocation::create(P, BRC.getSourceManager());

    return std::make_shared<PathDiagnosticEventPiece>(L, ErrorText);
  }

  return nullptr;
}

} // end of namespace: mpi
} // end of namespace: ento
} // end of namespace: clang
