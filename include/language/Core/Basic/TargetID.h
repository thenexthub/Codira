//===--- TargetID.h - Utilities for target ID -------------------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_BASIC_TARGETID_H
#define LANGUAGE_CORE_BASIC_TARGETID_H

#include "toolchain/ADT/SmallVector.h"
#include "toolchain/ADT/StringMap.h"
#include "toolchain/TargetParser/Triple.h"
#include <optional>
#include <set>

namespace language::Core {

/// Get all feature strings that can be used in target ID for \p Processor.
/// Target ID is a processor name with optional feature strings
/// postfixed by a plus or minus sign delimited by colons, e.g.
/// gfx908:xnack+:sramecc-. Each processor have a limited
/// number of predefined features when showing up in a target ID.
toolchain::SmallVector<toolchain::StringRef, 4>
getAllPossibleTargetIDFeatures(const toolchain::Triple &T,
                               toolchain::StringRef Processor);

/// Get processor name from target ID.
/// Returns canonical processor name or empty if the processor name is invalid.
toolchain::StringRef getProcessorFromTargetID(const toolchain::Triple &T,
                                         toolchain::StringRef OffloadArch);

/// Parse a target ID to get processor and feature map.
/// Returns canonicalized processor name or std::nullopt if the target ID is
/// invalid.  Returns target ID features in \p FeatureMap if it is not null
/// pointer. This function assumes \p OffloadArch is a valid target ID.
/// If the target ID contains feature+, map it to true.
/// If the target ID contains feature-, map it to false.
/// If the target ID does not contain a feature (default), do not map it.
std::optional<toolchain::StringRef> parseTargetID(const toolchain::Triple &T,
                                             toolchain::StringRef OffloadArch,
                                             toolchain::StringMap<bool> *FeatureMap);

/// Returns canonical target ID, assuming \p Processor is canonical and all
/// entries in \p Features are valid.
std::string getCanonicalTargetID(toolchain::StringRef Processor,
                                 const toolchain::StringMap<bool> &Features);

/// Get the conflicted pair of target IDs for a compilation or a bundled code
/// object, assuming \p TargetIDs are canonicalized. If there is no conflicts,
/// returns std::nullopt.
std::optional<std::pair<toolchain::StringRef, toolchain::StringRef>>
getConflictTargetIDCombination(const std::set<toolchain::StringRef> &TargetIDs);

/// Check whether the provided target ID is compatible with the requested
/// target ID.
bool isCompatibleTargetID(toolchain::StringRef Provided, toolchain::StringRef Requested);
} // namespace language::Core

#endif
