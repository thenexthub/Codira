//===- AnalyzerOptions.cpp - Analysis Engine Options ----------------------===//
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
// This file contains special accessors for analyzer configuration options
// with string representations.
//
//===----------------------------------------------------------------------===//

#include "language/Core/StaticAnalyzer/Core/AnalyzerOptions.h"
#include "language/Core/StaticAnalyzer/Core/Checker.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/ADT/StringSwitch.h"
#include "toolchain/ADT/Twine.h"
#include "toolchain/Support/ErrorHandling.h"
#include "toolchain/Support/FormattedStream.h"
#include "toolchain/Support/raw_ostream.h"
#include <cassert>
#include <cstddef>
#include <optional>
#include <utility>

using namespace language::Core;
using namespace ento;
using namespace toolchain;

void AnalyzerOptions::printFormattedEntry(
    toolchain::raw_ostream &Out,
    std::pair<StringRef, StringRef> EntryDescPair,
    size_t InitialPad, size_t EntryWidth, size_t MinLineWidth) {

  toolchain::formatted_raw_ostream FOut(Out);

  const size_t PadForDesc = InitialPad + EntryWidth;

  FOut.PadToColumn(InitialPad) << EntryDescPair.first;
  // If the buffer's length is greater than PadForDesc, print a newline.
  if (FOut.getColumn() > PadForDesc)
    FOut << '\n';

  FOut.PadToColumn(PadForDesc);

  if (MinLineWidth == 0) {
    FOut << EntryDescPair.second;
    return;
  }

  for (char C : EntryDescPair.second) {
    if (FOut.getColumn() > MinLineWidth && C == ' ') {
      FOut << '\n';
      FOut.PadToColumn(PadForDesc);
      continue;
    }
    FOut << C;
  }
}

ExplorationStrategyKind
AnalyzerOptions::getExplorationStrategy() const {
  auto K =
      toolchain::StringSwitch<std::optional<ExplorationStrategyKind>>(
          ExplorationStrategy)
          .Case("dfs", ExplorationStrategyKind::DFS)
          .Case("bfs", ExplorationStrategyKind::BFS)
          .Case("unexplored_first", ExplorationStrategyKind::UnexploredFirst)
          .Case("unexplored_first_queue",
                ExplorationStrategyKind::UnexploredFirstQueue)
          .Case("unexplored_first_location_queue",
                ExplorationStrategyKind::UnexploredFirstLocationQueue)
          .Case("bfs_block_dfs_contents",
                ExplorationStrategyKind::BFSBlockDFSContents)
          .Default(std::nullopt);
  assert(K && "User mode is invalid.");
  return *K;
}

CTUPhase1InliningKind AnalyzerOptions::getCTUPhase1Inlining() const {
  auto K = toolchain::StringSwitch<std::optional<CTUPhase1InliningKind>>(
               CTUPhase1InliningMode)
               .Case("none", CTUPhase1InliningKind::None)
               .Case("small", CTUPhase1InliningKind::Small)
               .Case("all", CTUPhase1InliningKind::All)
               .Default(std::nullopt);
  assert(K && "CTU inlining mode is invalid.");
  return *K;
}

IPAKind AnalyzerOptions::getIPAMode() const {
  auto K = toolchain::StringSwitch<std::optional<IPAKind>>(IPAMode)
               .Case("none", IPAK_None)
               .Case("basic-inlining", IPAK_BasicInlining)
               .Case("inlining", IPAK_Inlining)
               .Case("dynamic", IPAK_DynamicDispatch)
               .Case("dynamic-bifurcate", IPAK_DynamicDispatchBifurcate)
               .Default(std::nullopt);
  assert(K && "IPA Mode is invalid.");

  return *K;
}

bool
AnalyzerOptions::mayInlineCXXMemberFunction(
                                          CXXInlineableMemberKind Param) const {
  if (getIPAMode() < IPAK_Inlining)
    return false;

  auto K = toolchain::StringSwitch<std::optional<CXXInlineableMemberKind>>(
               CXXMemberInliningMode)
               .Case("constructors", CIMK_Constructors)
               .Case("destructors", CIMK_Destructors)
               .Case("methods", CIMK_MemberFunctions)
               .Case("none", CIMK_None)
               .Default(std::nullopt);

  assert(K && "Invalid c++ member function inlining mode.");

  return *K >= Param;
}

StringRef AnalyzerOptions::getCheckerStringOption(StringRef CheckerName,
                                                  StringRef OptionName,
                                                  bool SearchInParents) const {
  assert(!CheckerName.empty() &&
         "Empty checker name! Make sure the checker object (including it's "
         "bases!) if fully initialized before calling this function!");

  ConfigTable::const_iterator E = Config.end();
  do {
    ConfigTable::const_iterator I =
        Config.find((Twine(CheckerName) + ":" + OptionName).str());
    if (I != E)
      return StringRef(I->getValue());
    size_t Pos = CheckerName.rfind('.');
    if (Pos == StringRef::npos)
      break;

    CheckerName = CheckerName.substr(0, Pos);
  } while (!CheckerName.empty() && SearchInParents);

  toolchain_unreachable("Unknown checker option! Did you call getChecker*Option "
                   "with incorrect parameters? User input must've been "
                   "verified by CheckerRegistry.");

  return "";
}

StringRef AnalyzerOptions::getCheckerStringOption(const ento::CheckerBase *C,
                                                  StringRef OptionName,
                                                  bool SearchInParents) const {
  return getCheckerStringOption(C->getName(), OptionName, SearchInParents);
}

bool AnalyzerOptions::getCheckerBooleanOption(StringRef CheckerName,
                                              StringRef OptionName,
                                              bool SearchInParents) const {
  auto Ret =
      toolchain::StringSwitch<std::optional<bool>>(
          getCheckerStringOption(CheckerName, OptionName, SearchInParents))
          .Case("true", true)
          .Case("false", false)
          .Default(std::nullopt);

  assert(Ret &&
         "This option should be either 'true' or 'false', and should've been "
         "validated by CheckerRegistry!");

  return *Ret;
}

bool AnalyzerOptions::getCheckerBooleanOption(const ento::CheckerBase *C,
                                              StringRef OptionName,
                                              bool SearchInParents) const {
  return getCheckerBooleanOption(C->getName(), OptionName, SearchInParents);
}

int AnalyzerOptions::getCheckerIntegerOption(StringRef CheckerName,
                                             StringRef OptionName,
                                             bool SearchInParents) const {
  int Ret = 0;
  bool HasFailed = getCheckerStringOption(CheckerName, OptionName,
                                          SearchInParents)
                     .getAsInteger(0, Ret);
  assert(!HasFailed &&
         "This option should be numeric, and should've been validated by "
         "CheckerRegistry!");
  (void)HasFailed;
  return Ret;
}

int AnalyzerOptions::getCheckerIntegerOption(const ento::CheckerBase *C,
                                             StringRef OptionName,
                                             bool SearchInParents) const {
  return getCheckerIntegerOption(C->getName(), OptionName, SearchInParents);
}
