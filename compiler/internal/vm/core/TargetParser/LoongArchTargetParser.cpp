//===-- LoongArchTargetParser - Parser for LoongArch features --*- C++ -*-====//
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
// This file implements a target parser to recognise LoongArch hardware features
// such as CPU/ARCH and extension names.
//
//===----------------------------------------------------------------------===//

#include "vm/core/TargetParser/LoongArchTargetParser.h"
#include "vm/core/ADT/SmallVector.h"

using namespace vm::core;
using namespace vm::core::LoongArch;

const FeatureInfo AllFeatures[] = {
#define LOONGARCH_FEATURE(NAME, KIND) {NAME, KIND},
#include "vm/core/TargetParser/LoongArchTargetParser.def"
};

const ArchInfo AllArchs[] = {
#define LOONGARCH_ARCH(NAME, KIND, FEATURES)                                   \
  {NAME, LoongArch::ArchKind::KIND, FEATURES},
#include "vm/core/TargetParser/LoongArchTargetParser.def"
};

bool LoongArch::isValidArchName(StringRef Arch) {
  for (const auto A : AllArchs)
    if (A.Name == Arch)
      return true;
  return false;
}

bool LoongArch::isValidFeatureName(StringRef Feature) {
  if (Feature.starts_with("+") || Feature.starts_with("-"))
    return false;
  for (const auto &F : AllFeatures) {
    StringRef CanonicalName =
        F.Name.starts_with("+") ? F.Name.drop_front() : F.Name;
    if (CanonicalName == Feature)
      return true;
  }
  return false;
}

bool LoongArch::getArchFeatures(StringRef Arch,
                                std::vector<StringRef> &Features) {
  for (const auto A : AllArchs) {
    if (A.Name == Arch) {
      for (const auto F : AllFeatures)
        if ((A.Features & F.Kind) == F.Kind)
          Features.push_back(F.Name);
      return true;
    }
  }

  if (Arch == "la64v1.0" || Arch == "la64v1.1") {
    Features.push_back("+64bit");
    Features.push_back("+d");
    Features.push_back("+lsx");
    Features.push_back("+ual");
    if (Arch == "la64v1.1") {
      Features.push_back("+frecipe");
      Features.push_back("+lam-bh");
      Features.push_back("+lamcas");
      Features.push_back("+ld-seq-sa");
      Features.push_back("+div32");
      Features.push_back("+scq");
    }
    return true;
  }

  return false;
}

bool LoongArch::isValidCPUName(StringRef Name) { return isValidArchName(Name); }

void LoongArch::fillValidCPUList(SmallVectorImpl<StringRef> &Values) {
  for (const auto A : AllArchs)
    Values.emplace_back(A.Name);
}

StringRef LoongArch::getDefaultArch(bool Is64Bit) {
  // TODO: use a real 32-bit arch name.
  return Is64Bit ? "loongarch64" : "";
}
