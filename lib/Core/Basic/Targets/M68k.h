//===--- M68k.h - Declare M68k target feature support -------*- C++ -*-===//
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
// This file declares M68k TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_BASIC_TARGETS_M68K_H
#define LANGUAGE_CORE_LIB_BASIC_TARGETS_M68K_H

#include "OSTargets.h"
#include "language/Core/Basic/TargetInfo.h"
#include "language/Core/Basic/TargetOptions.h"
#include "toolchain/Support/Compiler.h"
#include "toolchain/TargetParser/Triple.h"
#include <optional>

namespace language::Core {
namespace targets {

class LLVM_LIBRARY_VISIBILITY M68kTargetInfo : public TargetInfo {
  static const char *const GCCRegNames[];
  static const TargetInfo::GCCRegAlias GCCRegAliases[];

  enum CPUKind {
    CK_Unknown,
    CK_68000,
    CK_68010,
    CK_68020,
    CK_68030,
    CK_68040,
    CK_68060
  } CPU = CK_Unknown;

  const TargetOptions &TargetOpts;

public:
  M68kTargetInfo(const toolchain::Triple &Triple, const TargetOptions &);

  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;
  toolchain::SmallVector<Builtin::InfosShard> getTargetBuiltins() const override;
  bool hasFeature(StringRef Feature) const override;
  ArrayRef<const char *> getGCCRegNames() const override;
  ArrayRef<TargetInfo::GCCRegAlias> getGCCRegAliases() const override;
  std::string convertConstraint(const char *&Constraint) const override;
  bool validateAsmConstraint(const char *&Name,
                             TargetInfo::ConstraintInfo &info) const override;
  std::optional<std::string> handleAsmEscapedChar(char EscChar) const override;
  std::string_view getClobbers() const override;
  BuiltinVaListKind getBuiltinVaListKind() const override;
  bool setCPU(const std::string &Name) override;
  CallingConvCheckResult checkCallingConvention(CallingConv CC) const override;

  std::pair<unsigned, unsigned> hardwareInterferenceSizes() const override {
    return std::make_pair(32, 32);
  }
};

} // namespace targets
} // namespace language::Core

#endif
