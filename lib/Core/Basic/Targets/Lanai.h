//===--- Lanai.h - Declare Lanai target feature support ---------*- C++ -*-===//
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
// This file declares Lanai TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_BASIC_TARGETS_LANAI_H
#define LANGUAGE_CORE_LIB_BASIC_TARGETS_LANAI_H

#include "language/Core/Basic/TargetInfo.h"
#include "language/Core/Basic/TargetOptions.h"
#include "toolchain/Support/Compiler.h"
#include "toolchain/TargetParser/Triple.h"

namespace language::Core {
namespace targets {

class LLVM_LIBRARY_VISIBILITY LanaiTargetInfo : public TargetInfo {
  // Class for Lanai (32-bit).
  // The CPU profiles supported by the Lanai backend
  enum CPUKind {
    CK_NONE,
    CK_V11,
  } CPU;

  static const TargetInfo::GCCRegAlias GCCRegAliases[];
  static const char *const GCCRegNames[];

public:
  LanaiTargetInfo(const toolchain::Triple &Triple, const TargetOptions &)
      : TargetInfo(Triple) {
    // Description string has to be kept in sync with backend.
    resetDataLayout("E"        // Big endian
                    "-m:e"     // ELF name manging
                    "-p:32:32" // 32 bit pointers, 32 bit aligned
                    "-i64:64"  // 64 bit integers, 64 bit aligned
                    "-a:0:32"  // 32 bit alignment of objects of aggregate type
                    "-n32"     // 32 bit native integer width
                    "-S64"     // 64 bit natural stack alignment
    );

    // Setting RegParmMax equal to what mregparm was set to in the old
    // toolchain
    RegParmMax = 4;

    // Set the default CPU to V11
    CPU = CK_V11;

    // Temporary approach to make everything at least word-aligned and allow for
    // safely casting between pointers with different alignment requirements.
    // TODO: Remove this when there are no more cast align warnings on the
    // firmware.
    MinGlobalAlign = 32;
  }

  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;

  bool isValidCPUName(StringRef Name) const override;

  void fillValidCPUList(SmallVectorImpl<StringRef> &Values) const override;

  bool setCPU(const std::string &Name) override;

  bool hasFeature(StringRef Feature) const override;

  ArrayRef<const char *> getGCCRegNames() const override;

  ArrayRef<TargetInfo::GCCRegAlias> getGCCRegAliases() const override;

  BuiltinVaListKind getBuiltinVaListKind() const override {
    return TargetInfo::VoidPtrBuiltinVaList;
  }

  toolchain::SmallVector<Builtin::InfosShard> getTargetBuiltins() const override {
    return {};
  }

  bool validateAsmConstraint(const char *&Name,
                             TargetInfo::ConstraintInfo &info) const override {
    return false;
  }

  std::string_view getClobbers() const override { return ""; }

  bool hasBitIntType() const override { return true; }
};
} // namespace targets
} // namespace language::Core

#endif // LANGUAGE_CORE_LIB_BASIC_TARGETS_LANAI_H
