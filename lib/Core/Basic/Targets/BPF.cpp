//===--- BPF.cpp - Implement BPF target feature support -------------------===//
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
// This file implements BPF TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#include "BPF.h"
#include "language/Core/Basic/MacroBuilder.h"
#include "language/Core/Basic/TargetBuiltins.h"
#include "toolchain/ADT/StringRef.h"

using namespace language::Core;
using namespace language::Core::targets;

static constexpr int NumBuiltins =
    language::Core::BPF::LastTSBuiltin - Builtin::FirstTSBuiltin;

#define GET_BUILTIN_STR_TABLE
#include "language/Core/Basic/BuiltinsBPF.inc"
#undef GET_BUILTIN_STR_TABLE

static constexpr Builtin::Info BuiltinInfos[] = {
#define GET_BUILTIN_INFOS
#include "language/Core/Basic/BuiltinsBPF.inc"
#undef GET_BUILTIN_INFOS
};
static_assert(std::size(BuiltinInfos) == NumBuiltins);

void BPFTargetInfo::getTargetDefines(const LangOptions &Opts,
                                     MacroBuilder &Builder) const {
  Builder.defineMacro("__bpf__");
  Builder.defineMacro("__BPF__");

  std::string CPU = getTargetOpts().CPU;
  if (CPU == "probe") {
    Builder.defineMacro("__BPF_CPU_VERSION__", "0");
    return;
  }

  Builder.defineMacro("__BPF_FEATURE_ADDR_SPACE_CAST");
  Builder.defineMacro("__BPF_FEATURE_MAY_GOTO");
  Builder.defineMacro("__BPF_FEATURE_ATOMIC_MEM_ORDERING");

  if (CPU.empty())
    CPU = "v3";

  if (CPU == "generic" || CPU == "v1") {
    Builder.defineMacro("__BPF_CPU_VERSION__", "1");
    return;
  }

  std::string CpuVerNumStr = CPU.substr(1);
  Builder.defineMacro("__BPF_CPU_VERSION__", CpuVerNumStr);

  int CpuVerNum = std::stoi(CpuVerNumStr);
  if (CpuVerNum >= 2)
    Builder.defineMacro("__BPF_FEATURE_JMP_EXT");

  if (CpuVerNum >= 3) {
    Builder.defineMacro("__BPF_FEATURE_JMP32");
    Builder.defineMacro("__BPF_FEATURE_ALU32");
  }

  if (CpuVerNum >= 4) {
    Builder.defineMacro("__BPF_FEATURE_LDSX");
    Builder.defineMacro("__BPF_FEATURE_MOVSX");
    Builder.defineMacro("__BPF_FEATURE_BSWAP");
    Builder.defineMacro("__BPF_FEATURE_SDIV_SMOD");
    Builder.defineMacro("__BPF_FEATURE_GOTOL");
    Builder.defineMacro("__BPF_FEATURE_ST");
    Builder.defineMacro("__BPF_FEATURE_LOAD_ACQ_STORE_REL");
  }
}

static constexpr toolchain::StringLiteral ValidCPUNames[] = {"generic", "v1", "v2",
                                                        "v3", "v4", "probe"};

bool BPFTargetInfo::isValidCPUName(StringRef Name) const {
  return toolchain::is_contained(ValidCPUNames, Name);
}

void BPFTargetInfo::fillValidCPUList(SmallVectorImpl<StringRef> &Values) const {
  Values.append(std::begin(ValidCPUNames), std::end(ValidCPUNames));
}

toolchain::SmallVector<Builtin::InfosShard>
BPFTargetInfo::getTargetBuiltins() const {
  return {{&BuiltinStrings, BuiltinInfos}};
}

bool BPFTargetInfo::handleTargetFeatures(std::vector<std::string> &Features,
                                         DiagnosticsEngine &Diags) {
  for (const auto &Feature : Features) {
    if (Feature == "+alu32") {
      HasAlu32 = true;
    }
  }

  return true;
}
