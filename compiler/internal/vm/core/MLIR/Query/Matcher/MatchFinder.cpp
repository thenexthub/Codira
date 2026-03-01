//===- MatchFinder.cpp - --------------------------------------------------===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
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
// This file contains the method definitions for the `MatchFinder` class
//
//===----------------------------------------------------------------------===//

#include "mlir/Query/Matcher/MatchFinder.h"
namespace mlir::query::matcher {

MatchFinder::MatchResult::MatchResult(Operation *rootOp,
                                      std::vector<Operation *> matchedOps)
    : rootOp(rootOp), matchedOps(std::move(matchedOps)) {}

std::vector<MatchFinder::MatchResult>
MatchFinder::collectMatches(Operation *root, DynMatcher matcher) const {
  std::vector<MatchResult> results;
  toolchain::SetVector<Operation *> tempStorage;
  root->walk([&](Operation *subOp) {
    if (matcher.match(subOp)) {
      MatchResult match;
      match.rootOp = subOp;
      match.matchedOps.push_back(subOp);
      results.push_back(std::move(match));
    } else if (matcher.match(subOp, tempStorage)) {
      results.emplace_back(subOp, std::vector<Operation *>(tempStorage.begin(),
                                                           tempStorage.end()));
    }
    tempStorage.clear();
  });
  return results;
}

void MatchFinder::printMatch(toolchain::raw_ostream &os, QuerySession &qs,
                             Operation *op) const {
  if (auto fileLoc = op->getLoc()->findInstanceOf<FileLineColLoc>()) {
    SMLoc smloc = qs.getSourceManager().FindLocForLineAndColumn(
        qs.getBufferId(), fileLoc.getLine(), fileLoc.getColumn());
    toolchain::SMDiagnostic diag =
        qs.getSourceManager().GetMessage(smloc, toolchain::SourceMgr::DK_Note, "");
    diag.print("", os, true, false, true);
  }
}

void MatchFinder::printMatch(toolchain::raw_ostream &os, QuerySession &qs,
                             Operation *op, const std::string &binding) const {
  if (auto fileLoc = op->getLoc()->findInstanceOf<FileLineColLoc>()) {
    auto smloc = qs.getSourceManager().FindLocForLineAndColumn(
        qs.getBufferId(), fileLoc.getLine(), fileLoc.getColumn());
    qs.getSourceManager().PrintMessage(os, smloc, toolchain::SourceMgr::DK_Note,
                                       "\"" + binding + "\" binds here");
  }
}

std::vector<Operation *>
MatchFinder::flattenMatchedOps(std::vector<MatchResult> &matches) const {
  std::vector<Operation *> newVector;
  for (auto &result : matches) {
    newVector.insert(newVector.end(), result.matchedOps.begin(),
                     result.matchedOps.end());
  }
  return newVector;
}

} // namespace mlir::query::matcher
