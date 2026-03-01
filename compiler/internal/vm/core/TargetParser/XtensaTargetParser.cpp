//==-- XtensaTargetParser - Parser for Xtensa features ------------*- C++ -*-=//
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
// This file implements a target parser to recognise Xtensa hardware features
//
//===----------------------------------------------------------------------===//

#include "vm/core/TargetParser/XtensaTargetParser.h"
#include "vm/core/ADT/STLExtras.h"
#include "vm/core/ADT/StringSwitch.h"
#include <vector>

namespace vm::core {

namespace Xtensa {
struct CPUInfo {
  StringLiteral Name;
  CPUKind Kind;
  uint64_t Features;
};

struct FeatureName {
  uint64_t ID;
  const char *NameCStr;
  size_t NameLength;

  StringRef getName() const { return StringRef(NameCStr, NameLength); }
};

const FeatureName XtensaFeatureNames[] = {
#define XTENSA_FEATURE(ID, NAME) {ID, "+" NAME, sizeof(NAME)},
#include "vm/core/TargetParser/XtensaTargetParser.def"
};

constexpr CPUInfo XtensaCPUInfo[] = {
#define XTENSA_CPU(ENUM, NAME, FEATURES) {NAME, CK_##ENUM, FEATURES},
#include "vm/core/TargetParser/XtensaTargetParser.def"
};

StringRef getBaseName(StringRef CPU) {
  return toolchain::StringSwitch<StringRef>(CPU)
#define XTENSA_CPU_ALIAS(NAME, ANAME) .Case(ANAME, NAME)
#include "vm/core/TargetParser/XtensaTargetParser.def"
      .Default(CPU);
}

StringRef getAliasName(StringRef CPU) {
  return toolchain::StringSwitch<StringRef>(CPU)
#define XTENSA_CPU_ALIAS(NAME, ANAME) .Case(NAME, ANAME)
#include "vm/core/TargetParser/XtensaTargetParser.def"
      .Default(CPU);
}

CPUKind parseCPUKind(StringRef CPU) {
  CPU = getBaseName(CPU);
  return toolchain::StringSwitch<CPUKind>(CPU)
#define XTENSA_CPU(ENUM, NAME, FEATURES) .Case(NAME, CK_##ENUM)
#include "vm/core/TargetParser/XtensaTargetParser.def"
      .Default(CK_INVALID);
}

// Get all features for the CPU
void getCPUFeatures(StringRef CPU, std::vector<StringRef> &Features) {
  CPU = getBaseName(CPU);
  auto I = toolchain::find_if(XtensaCPUInfo,
                         [&](const CPUInfo &CI) { return CI.Name == CPU; });
  assert(I != std::end(XtensaCPUInfo) && "CPU not found!");
  uint64_t Bits = I->Features;

  for (const auto &F : XtensaFeatureNames) {
    if ((Bits & F.ID) == F.ID)
      Features.push_back(F.getName());
  }
}

// Find all valid CPUs
void fillValidCPUList(std::vector<StringRef> &Values) {
  for (const auto &C : XtensaCPUInfo) {
    if (C.Kind != CK_INVALID) {
      Values.emplace_back(C.Name);
      StringRef Name = getAliasName(C.Name);
      if (Name != C.Name)
        Values.emplace_back(Name);
    }
  }
}

} // namespace Xtensa
} // namespace vm::core
