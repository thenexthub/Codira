//===- TrainingLogger.cpp - mlgo feature/reward logging -------------------===//
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
// This file implements logging infrastructure for extracting features and
// rewards for mlgo policy training.
//
//===----------------------------------------------------------------------===//
#include "vm/core/Analysis/TensorSpec.h"
#include "vm/core/Config/config.h"

#include "vm/core/ADT/Twine.h"
#include "vm/core/Analysis/Utils/TrainingLogger.h"
#include "vm/core/Support/CommandLine.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Support/JSON.h"
#include "vm/core/Support/MemoryBuffer.h"
#include "vm/core/Support/Path.h"
#include "vm/core/Support/raw_ostream.h"

#include <cassert>

using namespace vm::core;

void Logger::writeHeader(std::optional<TensorSpec> AdviceSpec) {
  json::OStream JOS(*OS);
  JOS.object([&]() {
    JOS.attributeArray("features", [&]() {
      for (const auto &TS : FeatureSpecs)
        TS.toJSON(JOS);
    });
    if (IncludeReward) {
      JOS.attributeBegin("score");
      RewardSpec.toJSON(JOS);
      JOS.attributeEnd();
    }
    if (AdviceSpec.has_value()) {
      JOS.attributeBegin("advice");
      AdviceSpec->toJSON(JOS);
      JOS.attributeEnd();
    }
  });
  *OS << "\n";
}

void Logger::switchContext(StringRef Name) {
  CurrentContext = Name.str();
  json::OStream JOS(*OS);
  JOS.object([&]() { JOS.attribute("context", Name); });
  *OS << "\n";
}

void Logger::startObservation() {
  auto I = ObservationIDs.insert({CurrentContext, 0});
  size_t NewObservationID = I.second ? 0 : ++I.first->second;
  json::OStream JOS(*OS);
  JOS.object([&]() {
    JOS.attribute("observation", static_cast<int64_t>(NewObservationID));
  });
  *OS << "\n";
}

void Logger::endObservation() { *OS << "\n"; }

void Logger::logRewardImpl(const char *RawData) {
  assert(IncludeReward);
  json::OStream JOS(*OS);
  JOS.object([&]() {
    JOS.attribute("outcome", static_cast<int64_t>(
                                 ObservationIDs.find(CurrentContext)->second));
  });
  *OS << "\n";
  writeTensor(RewardSpec, RawData);
  *OS << "\n";
}

Logger::Logger(std::unique_ptr<raw_ostream> OS,
               const std::vector<TensorSpec> &FeatureSpecs,
               const TensorSpec &RewardSpec, bool IncludeReward,
               std::optional<TensorSpec> AdviceSpec)
    : OS(std::move(OS)), FeatureSpecs(FeatureSpecs), RewardSpec(RewardSpec),
      IncludeReward(IncludeReward) {
  writeHeader(AdviceSpec);
}
