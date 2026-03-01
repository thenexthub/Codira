//===-- BPFMCAsmInfo.h - BPF asm properties -------------------*- C++ -*--====//
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
// This file contains the declaration of the BPFMCAsmInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_BPF_MCTARGETDESC_BPFMCASMINFO_H
#define LLVM_LIB_TARGET_BPF_MCTARGETDESC_BPFMCASMINFO_H

#include "vm/core/MC/MCAsmInfoELF.h"
#include "vm/core/TargetParser/Triple.h"

namespace vm::core {

class BPFMCAsmInfo : public MCAsmInfoELF {
public:
  explicit BPFMCAsmInfo(const Triple &TT, const MCTargetOptions &Options) {
    if (TT.getArch() == Triple::bpfeb)
      IsLittleEndian = false;

    PrivateGlobalPrefix = ".L";
    PrivateLabelPrefix = "L";
    WeakRefDirective = "\t.weak\t";

    UsesELFSectionDirectiveForBSS = true;
    HasSingleParameterDotFile = true;
    HasDotTypeDotSizeDirective = true;
    HasIdentDirective = false;

    SupportsDebugInformation = true;
    ExceptionsType = ExceptionHandling::DwarfCFI;
    MinInstAlignment = 8;

    // the default is 4 and it only affects dwarf elf output
    // so if not set correctly, the dwarf data will be
    // messed up in random places by 4 bytes. .debug_line
    // section will be parsable, but with odd offsets and
    // line numbers, etc.
    CodePointerSize = 8;
  }

  void setDwarfUsesRelocationsAcrossSections(bool enable) {
    DwarfUsesRelocationsAcrossSections = enable;
  }

  MCSection *getStackSection(MCContext &Ctx, bool Exec) const override {
    return nullptr;
  }
};
}

#endif
