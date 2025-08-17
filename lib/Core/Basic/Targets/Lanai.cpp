//===--- Lanai.cpp - Implement Lanai target feature support ---------------===//
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
// This file implements Lanai TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#include "Lanai.h"
#include "language/Core/Basic/MacroBuilder.h"
#include "toolchain/ADT/StringSwitch.h"

using namespace language::Core;
using namespace language::Core::targets;

const char *const LanaiTargetInfo::GCCRegNames[] = {
    "r0",  "r1",  "r2",  "r3",  "r4",  "r5",  "r6",  "r7",  "r8",  "r9",  "r10",
    "r11", "r12", "r13", "r14", "r15", "r16", "r17", "r18", "r19", "r20", "r21",
    "r22", "r23", "r24", "r25", "r26", "r27", "r28", "r29", "r30", "r31"
};

ArrayRef<const char *> LanaiTargetInfo::getGCCRegNames() const {
  return toolchain::ArrayRef(GCCRegNames);
}

const TargetInfo::GCCRegAlias LanaiTargetInfo::GCCRegAliases[] = {
    {{"pc"}, "r2"},   {{"sp"}, "r4"},   {{"fp"}, "r5"},   {{"rv"}, "r8"},
    {{"rr1"}, "r10"}, {{"rr2"}, "r11"}, {{"rca"}, "r15"},
};

ArrayRef<TargetInfo::GCCRegAlias> LanaiTargetInfo::getGCCRegAliases() const {
  return toolchain::ArrayRef(GCCRegAliases);
}

bool LanaiTargetInfo::isValidCPUName(StringRef Name) const {
  return toolchain::StringSwitch<bool>(Name).Case("v11", true).Default(false);
}
void LanaiTargetInfo::fillValidCPUList(
    SmallVectorImpl<StringRef> &Values) const {
  Values.emplace_back("v11");
}

bool LanaiTargetInfo::setCPU(const std::string &Name) {
  CPU = toolchain::StringSwitch<CPUKind>(Name).Case("v11", CK_V11).Default(CK_NONE);

  return CPU != CK_NONE;
}

bool LanaiTargetInfo::hasFeature(StringRef Feature) const {
  return toolchain::StringSwitch<bool>(Feature).Case("lanai", true).Default(false);
}

void LanaiTargetInfo::getTargetDefines(const LangOptions &Opts,
                                       MacroBuilder &Builder) const {
  // Define __lanai__ when building for target lanai.
  Builder.defineMacro("__lanai__");

  // Set define for the CPU specified.
  switch (CPU) {
  case CK_V11:
    Builder.defineMacro("__LANAI_V11__");
    break;
  case CK_NONE:
    toolchain_unreachable("Unhandled target CPU");
  }
}
