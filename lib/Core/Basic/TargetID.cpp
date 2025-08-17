//===--- TargetID.cpp - Utilities for parsing target ID -------------------===//
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

#include "language/Core/Basic/TargetID.h"
#include "toolchain/ADT/STLExtras.h"
#include "toolchain/ADT/SmallSet.h"
#include "toolchain/TargetParser/TargetParser.h"
#include "toolchain/TargetParser/Triple.h"
#include <map>
#include <optional>

namespace language::Core {

static toolchain::SmallVector<toolchain::StringRef, 4>
getAllPossibleAMDGPUTargetIDFeatures(const toolchain::Triple &T,
                                     toolchain::StringRef Proc) {
  // Entries in returned vector should be in alphabetical order.
  toolchain::SmallVector<toolchain::StringRef, 4> Ret;
  auto ProcKind = T.isAMDGCN() ? toolchain::AMDGPU::parseArchAMDGCN(Proc)
                               : toolchain::AMDGPU::parseArchR600(Proc);
  if (ProcKind == toolchain::AMDGPU::GK_NONE)
    return Ret;
  auto Features = T.isAMDGCN() ? toolchain::AMDGPU::getArchAttrAMDGCN(ProcKind)
                               : toolchain::AMDGPU::getArchAttrR600(ProcKind);
  if (Features & toolchain::AMDGPU::FEATURE_SRAMECC)
    Ret.push_back("sramecc");
  if (Features & toolchain::AMDGPU::FEATURE_XNACK)
    Ret.push_back("xnack");
  return Ret;
}

toolchain::SmallVector<toolchain::StringRef, 4>
getAllPossibleTargetIDFeatures(const toolchain::Triple &T,
                               toolchain::StringRef Processor) {
  toolchain::SmallVector<toolchain::StringRef, 4> Ret;
  if (T.isAMDGPU())
    return getAllPossibleAMDGPUTargetIDFeatures(T, Processor);
  return Ret;
}

/// Returns canonical processor name or empty string if \p Processor is invalid.
static toolchain::StringRef getCanonicalProcessorName(const toolchain::Triple &T,
                                                 toolchain::StringRef Processor) {
  if (T.isAMDGPU())
    return toolchain::AMDGPU::getCanonicalArchName(T, Processor);
  return Processor;
}

toolchain::StringRef getProcessorFromTargetID(const toolchain::Triple &T,
                                         toolchain::StringRef TargetID) {
  auto Split = TargetID.split(':');
  return getCanonicalProcessorName(T, Split.first);
}

// Parse a target ID with format checking only. Do not check whether processor
// name or features are valid for the processor.
//
// A target ID is a processor name followed by a list of target features
// delimited by colon. Each target feature is a string post-fixed by a plus
// or minus sign, e.g. gfx908:sramecc+:xnack-.
static std::optional<toolchain::StringRef>
parseTargetIDWithFormatCheckingOnly(toolchain::StringRef TargetID,
                                    toolchain::StringMap<bool> *FeatureMap) {
  toolchain::StringRef Processor;

  if (TargetID.empty())
    return toolchain::StringRef();

  auto Split = TargetID.split(':');
  Processor = Split.first;
  if (Processor.empty())
    return std::nullopt;

  auto Features = Split.second;
  if (Features.empty())
    return Processor;

  toolchain::StringMap<bool> LocalFeatureMap;
  if (!FeatureMap)
    FeatureMap = &LocalFeatureMap;

  while (!Features.empty()) {
    auto Splits = Features.split(':');
    auto Sign = Splits.first.back();
    auto Feature = Splits.first.drop_back();
    if (Sign != '+' && Sign != '-')
      return std::nullopt;
    bool IsOn = Sign == '+';
    // Each feature can only show up at most once in target ID.
    if (!FeatureMap->try_emplace(Feature, IsOn).second)
      return std::nullopt;
    Features = Splits.second;
  }
  return Processor;
}

std::optional<toolchain::StringRef>
parseTargetID(const toolchain::Triple &T, toolchain::StringRef TargetID,
              toolchain::StringMap<bool> *FeatureMap) {
  auto OptionalProcessor =
      parseTargetIDWithFormatCheckingOnly(TargetID, FeatureMap);

  if (!OptionalProcessor)
    return std::nullopt;

  toolchain::StringRef Processor = getCanonicalProcessorName(T, *OptionalProcessor);
  if (Processor.empty())
    return std::nullopt;

  toolchain::SmallSet<toolchain::StringRef, 4> AllFeatures(
      toolchain::from_range, getAllPossibleTargetIDFeatures(T, Processor));

  for (auto &&F : *FeatureMap)
    if (!AllFeatures.count(F.first()))
      return std::nullopt;

  return Processor;
}

// A canonical target ID is a target ID containing a canonical processor name
// and features in alphabetical order.
std::string getCanonicalTargetID(toolchain::StringRef Processor,
                                 const toolchain::StringMap<bool> &Features) {
  std::string TargetID = Processor.str();
  std::map<const toolchain::StringRef, bool> OrderedMap;
  for (const auto &F : Features)
    OrderedMap[F.first()] = F.second;
  for (const auto &F : OrderedMap)
    TargetID = TargetID + ':' + F.first.str() + (F.second ? "+" : "-");
  return TargetID;
}

// For a specific processor, a feature either shows up in all target IDs, or
// does not show up in any target IDs. Otherwise the target ID combination
// is invalid.
std::optional<std::pair<toolchain::StringRef, toolchain::StringRef>>
getConflictTargetIDCombination(const std::set<toolchain::StringRef> &TargetIDs) {
  struct Info {
    toolchain::StringRef TargetID;
    toolchain::StringMap<bool> Features;
    Info(toolchain::StringRef TargetID, const toolchain::StringMap<bool> &Features)
        : TargetID(TargetID), Features(Features) {}
  };
  toolchain::StringMap<Info> FeatureMap;
  for (auto &&ID : TargetIDs) {
    toolchain::StringMap<bool> Features;
    toolchain::StringRef Proc = *parseTargetIDWithFormatCheckingOnly(ID, &Features);
    auto [Loc, Inserted] = FeatureMap.try_emplace(Proc, ID, Features);
    if (!Inserted) {
      auto &ExistingFeatures = Loc->second.Features;
      if (toolchain::any_of(Features, [&](auto &F) {
            return ExistingFeatures.count(F.first()) == 0;
          }))
        return std::make_pair(Loc->second.TargetID, ID);
    }
  }
  return std::nullopt;
}

bool isCompatibleTargetID(toolchain::StringRef Provided, toolchain::StringRef Requested) {
  toolchain::StringMap<bool> ProvidedFeatures, RequestedFeatures;
  toolchain::StringRef ProvidedProc =
      *parseTargetIDWithFormatCheckingOnly(Provided, &ProvidedFeatures);
  toolchain::StringRef RequestedProc =
      *parseTargetIDWithFormatCheckingOnly(Requested, &RequestedFeatures);
  if (ProvidedProc != RequestedProc)
    return false;
  for (const auto &F : ProvidedFeatures) {
    auto Loc = RequestedFeatures.find(F.first());
    // The default (unspecified) value of a feature is 'All', which can match
    // either 'On' or 'Off'.
    if (Loc == RequestedFeatures.end())
      return false;
    // If a feature is specified, it must have exact match.
    if (Loc->second != F.second)
      return false;
  }
  return true;
}

} // namespace language::Core
