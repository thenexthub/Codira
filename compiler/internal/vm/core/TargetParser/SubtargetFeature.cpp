//===- SubtargetFeature.cpp - CPU characteristics Implementation ----------===//
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
/// \file Implements the SubtargetFeature interface.
//
//===----------------------------------------------------------------------===//

#include "vm/core/TargetParser/SubtargetFeature.h"
#include "vm/core/ADT/SmallVector.h"
#include "vm/core/ADT/StringExtras.h"
#include "vm/core/ADT/StringRef.h"
#include "vm/core/Config/toolchain-config.h"
#include "vm/core/Support/Compiler.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Support/raw_ostream.h"
#include "vm/core/TargetParser/Triple.h"
#include <string>
#include <vector>

using namespace vm::core;

/// Splits a string of comma separated items in to a vector of strings.
void SubtargetFeatures::Split(std::vector<std::string> &V, StringRef S) {
  SmallVector<StringRef, 3> Tmp;
  S.split(Tmp, ',', -1, false /* KeepEmpty */);
  V.reserve(Tmp.size());
  for (StringRef T : Tmp)
    V.push_back(std::string(T));
}

void SubtargetFeatures::AddFeature(StringRef String, bool Enable) {
  // Don't add empty features.
  if (!String.empty())
    // Convert to lowercase, prepend flag if we don't already have a flag.
    Features.push_back(hasFlag(String) ? String.lower()
                                       : (Enable ? "+" : "-") + String.lower());
}

void SubtargetFeatures::addFeaturesVector(
    const ArrayRef<std::string> OtherFeatures) {
  toolchain::append_range(Features, OtherFeatures);
}

SubtargetFeatures::SubtargetFeatures(StringRef Initial) {
  // Break up string into separate features
  Split(Features, Initial);
}

std::string SubtargetFeatures::getString() const {
  return join(Features.begin(), Features.end(), ",");
}

void SubtargetFeatures::print(raw_ostream &OS) const {
  for (const auto &F : Features)
    OS << F << " ";
  OS << "\n";
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD void SubtargetFeatures::dump() const {
  print(dbgs());
}
#endif

void SubtargetFeatures::getDefaultSubtargetFeatures(const Triple& Triple) {
  // FIXME: This is an inelegant way of specifying the features of a
  // subtarget. It would be better if we could encode this information
  // into the IR.
  if (Triple.getVendor() == Triple::Apple) {
    if (Triple.getArch() == Triple::ppc) {
      // powerpc-apple-*
      AddFeature("altivec");
    } else if (Triple.getArch() == Triple::ppc64) {
      // powerpc64-apple-*
      AddFeature("64bit");
      AddFeature("altivec");
    }
  }
}
