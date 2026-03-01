//===- MipsABIFlagsSection.cpp - Mips ELF ABI Flags Section ---------------===//
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

#include "MCTargetDesc/MipsABIFlagsSection.h"
#include "vm/core/ADT/StringRef.h"
#include "vm/core/MC/MCStreamer.h"
#include "vm/core/Support/ErrorHandling.h"
#include "vm/core/Support/MipsABIFlags.h"

using namespace vm::core;

uint8_t MipsABIFlagsSection::getFpABIValue() {
  switch (FpABI) {
  case FpABIKind::ANY:
    return Mips::Val_GNU_MIPS_ABI_FP_ANY;
  case FpABIKind::SOFT:
    return Mips::Val_GNU_MIPS_ABI_FP_SOFT;
  case FpABIKind::XX:
    return Mips::Val_GNU_MIPS_ABI_FP_XX;
  case FpABIKind::S32:
    return Mips::Val_GNU_MIPS_ABI_FP_DOUBLE;
  case FpABIKind::S64:
    if (Is32BitABI)
      return OddSPReg ? Mips::Val_GNU_MIPS_ABI_FP_64
                      : Mips::Val_GNU_MIPS_ABI_FP_64A;
    return Mips::Val_GNU_MIPS_ABI_FP_DOUBLE;
  }

  llvm_unreachable("unexpected fp abi value");
}

StringRef MipsABIFlagsSection::getFpABIString(FpABIKind Value) {
  switch (Value) {
  case FpABIKind::XX:
    return "xx";
  case FpABIKind::S32:
    return "32";
  case FpABIKind::S64:
    return "64";
  default:
    llvm_unreachable("unsupported fp abi value");
  }
}

uint8_t MipsABIFlagsSection::getCPR1SizeValue() {
  if (FpABI == FpABIKind::XX)
    return (uint8_t)Mips::AFL_REG_32;
  return (uint8_t)CPR1Size;
}

namespace vm::core {

MCStreamer &operator<<(MCStreamer &OS, MipsABIFlagsSection &ABIFlagsSection) {
  // Write out a Elf_Internal_ABIFlags_v0 struct
  OS.emitIntValue(ABIFlagsSection.getVersionValue(), 2);      // version
  OS.emitIntValue(ABIFlagsSection.getISALevelValue(), 1);     // isa_level
  OS.emitIntValue(ABIFlagsSection.getISARevisionValue(), 1);  // isa_rev
  OS.emitIntValue(ABIFlagsSection.getGPRSizeValue(), 1);      // gpr_size
  OS.emitIntValue(ABIFlagsSection.getCPR1SizeValue(), 1);     // cpr1_size
  OS.emitIntValue(ABIFlagsSection.getCPR2SizeValue(), 1);     // cpr2_size
  OS.emitIntValue(ABIFlagsSection.getFpABIValue(), 1);        // fp_abi
  OS.emitIntValue(ABIFlagsSection.getISAExtensionValue(), 4); // isa_ext
  OS.emitIntValue(ABIFlagsSection.getASESetValue(), 4);       // ases
  OS.emitIntValue(ABIFlagsSection.getFlags1Value(), 4);       // flags1
  OS.emitIntValue(ABIFlagsSection.getFlags2Value(), 4);       // flags2
  return OS;
}

} // end namespace vm::core
