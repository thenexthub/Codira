//===- Sanitizers.cpp - C Language Family Language Options ----------------===//
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
//  This file defines the classes from Sanitizers.h
//
//===----------------------------------------------------------------------===//

#include "language/Core/Basic/Sanitizers.h"
#include "toolchain/ADT/Hashing.h"
#include "toolchain/ADT/StringSwitch.h"
#include "toolchain/Support/Format.h"
#include "toolchain/Support/raw_ostream.h"
#include <algorithm>
#include <cmath>
#include <optional>

using namespace language::Core;

static const double SanitizerMaskCutoffsEps = 0.000000001f;

void SanitizerMaskCutoffs::set(SanitizerMask K, double V) {
  if (V < SanitizerMaskCutoffsEps && Cutoffs.empty())
    return;
  for (unsigned int i = 0; i < SanitizerKind::SO_Count; ++i)
    if (K & SanitizerMask::bitPosToMask(i)) {
      Cutoffs.resize(SanitizerKind::SO_Count);
      Cutoffs[i] = V;
    }
}

std::optional<double> SanitizerMaskCutoffs::operator[](unsigned Kind) const {
  if (Cutoffs.empty() || Cutoffs[Kind] < SanitizerMaskCutoffsEps)
    return std::nullopt;

  return Cutoffs[Kind];
}

void SanitizerMaskCutoffs::clear(SanitizerMask K) { set(K, 0); }

std::optional<std::vector<unsigned>>
SanitizerMaskCutoffs::getAllScaled(unsigned ScalingFactor) const {
  std::vector<unsigned> ScaledCutoffs;

  bool AnyCutoff = false;
  for (unsigned int i = 0; i < SanitizerKind::SO_Count; ++i) {
    auto C = (*this)[i];
    if (C.has_value()) {
      ScaledCutoffs.push_back(lround(std::clamp(*C, 0.0, 1.0) * ScalingFactor));
      AnyCutoff = true;
    } else {
      ScaledCutoffs.push_back(0);
    }
  }

  if (AnyCutoff)
    return ScaledCutoffs;

  return std::nullopt;
}

// Once LLVM switches to C++17, the constexpr variables can be inline and we
// won't need this.
#define SANITIZER(NAME, ID) constexpr SanitizerMask SanitizerKind::ID;
#define SANITIZER_GROUP(NAME, ID, ALIAS)                                       \
  constexpr SanitizerMask SanitizerKind::ID;                                   \
  constexpr SanitizerMask SanitizerKind::ID##Group;
#include "language/Core/Basic/Sanitizers.def"

SanitizerMask language::Core::parseSanitizerValue(StringRef Value, bool AllowGroups) {
  SanitizerMask ParsedKind = toolchain::StringSwitch<SanitizerMask>(Value)
#define SANITIZER(NAME, ID) .Case(NAME, SanitizerKind::ID)
#define SANITIZER_GROUP(NAME, ID, ALIAS)                                       \
  .Case(NAME, AllowGroups ? SanitizerKind::ID##Group : SanitizerMask())
#include "language/Core/Basic/Sanitizers.def"
    .Default(SanitizerMask());
  return ParsedKind;
}

bool language::Core::parseSanitizerWeightedValue(StringRef Value, bool AllowGroups,
                                        SanitizerMaskCutoffs &Cutoffs) {
  SanitizerMask ParsedKind = toolchain::StringSwitch<SanitizerMask>(Value)
#define SANITIZER(NAME, ID) .StartsWith(NAME "=", SanitizerKind::ID)
#define SANITIZER_GROUP(NAME, ID, ALIAS)                                       \
  .StartsWith(NAME "=",                                                        \
              AllowGroups ? SanitizerKind::ID##Group : SanitizerMask())
#include "language/Core/Basic/Sanitizers.def"
                                 .Default(SanitizerMask());

  if (!ParsedKind)
    return false;
  auto [N, W] = Value.split('=');
  double A;
  if (W.getAsDouble(A) || A < 0.0 || A > 1.0)
    return false;
  // AllowGroups is already taken into account for ParsedKind,
  // hence we unconditionally expandSanitizerGroups.
  Cutoffs.set(expandSanitizerGroups(ParsedKind), A);
  return true;
}

void language::Core::serializeSanitizerSet(SanitizerSet Set,
                                  SmallVectorImpl<StringRef> &Values) {
#define SANITIZER(NAME, ID)                                                    \
  if (Set.has(SanitizerKind::ID))                                              \
    Values.push_back(NAME);
#include "language/Core/Basic/Sanitizers.def"
}

void language::Core::serializeSanitizerMaskCutoffs(
    const SanitizerMaskCutoffs &Cutoffs, SmallVectorImpl<std::string> &Values) {
#define SANITIZER(NAME, ID)                                                    \
  if (auto C = Cutoffs[SanitizerKind::SO_##ID]) {                              \
    std::string Str;                                                           \
    toolchain::raw_string_ostream OS(Str);                                          \
    OS << NAME "=" << toolchain::format("%.8f", *C);                                \
    Values.emplace_back(StringRef(Str).rtrim('0'));                            \
  }
#include "language/Core/Basic/Sanitizers.def"
}

SanitizerMask language::Core::expandSanitizerGroups(SanitizerMask Kinds) {
#define SANITIZER(NAME, ID)
#define SANITIZER_GROUP(NAME, ID, ALIAS)                                       \
  if (Kinds & SanitizerKind::ID##Group)                                        \
    Kinds |= SanitizerKind::ID;
#include "language/Core/Basic/Sanitizers.def"
  return Kinds;
}

toolchain::hash_code SanitizerMask::hash_value() const {
  return toolchain::hash_combine_range(&maskLoToHigh[0], &maskLoToHigh[kNumElem]);
}

namespace language::Core {
unsigned SanitizerMask::countPopulation() const {
  unsigned total = 0;
  for (const auto &Val : maskLoToHigh)
    total += toolchain::popcount(Val);
  return total;
}

toolchain::hash_code hash_value(const language::Core::SanitizerMask &Arg) {
  return Arg.hash_value();
}

StringRef AsanDtorKindToString(toolchain::AsanDtorKind kind) {
  switch (kind) {
  case toolchain::AsanDtorKind::None:
    return "none";
  case toolchain::AsanDtorKind::Global:
    return "global";
  case toolchain::AsanDtorKind::Invalid:
    return "invalid";
  }
  return "invalid";
}

toolchain::AsanDtorKind AsanDtorKindFromString(StringRef kindStr) {
  return toolchain::StringSwitch<toolchain::AsanDtorKind>(kindStr)
      .Case("none", toolchain::AsanDtorKind::None)
      .Case("global", toolchain::AsanDtorKind::Global)
      .Default(toolchain::AsanDtorKind::Invalid);
}

StringRef AsanDetectStackUseAfterReturnModeToString(
    toolchain::AsanDetectStackUseAfterReturnMode mode) {
  switch (mode) {
  case toolchain::AsanDetectStackUseAfterReturnMode::Always:
    return "always";
  case toolchain::AsanDetectStackUseAfterReturnMode::Runtime:
    return "runtime";
  case toolchain::AsanDetectStackUseAfterReturnMode::Never:
    return "never";
  case toolchain::AsanDetectStackUseAfterReturnMode::Invalid:
    return "invalid";
  }
  return "invalid";
}

toolchain::AsanDetectStackUseAfterReturnMode
AsanDetectStackUseAfterReturnModeFromString(StringRef modeStr) {
  return toolchain::StringSwitch<toolchain::AsanDetectStackUseAfterReturnMode>(modeStr)
      .Case("always", toolchain::AsanDetectStackUseAfterReturnMode::Always)
      .Case("runtime", toolchain::AsanDetectStackUseAfterReturnMode::Runtime)
      .Case("never", toolchain::AsanDetectStackUseAfterReturnMode::Never)
      .Default(toolchain::AsanDetectStackUseAfterReturnMode::Invalid);
}

} // namespace language::Core
