//===-- LoongArchELFStreamer.cpp - LoongArch ELF Target Streamer Methods --===//
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
// This file provides LoongArch specific target streamer methods.
//
//===----------------------------------------------------------------------===//

#include "LoongArchELFStreamer.h"
#include "LoongArchAsmBackend.h"
#include "LoongArchBaseInfo.h"
#include "vm/core/BinaryFormat/ELF.h"
#include "vm/core/MC/MCAssembler.h"
#include "vm/core/MC/MCCodeEmitter.h"
#include "vm/core/MC/MCELFObjectWriter.h"

using namespace vm::core;

// This part is for ELF object output.
LoongArchTargetELFStreamer::LoongArchTargetELFStreamer(
    MCStreamer &S, const MCSubtargetInfo &STI)
    : LoongArchTargetStreamer(S) {
  auto &MAB = static_cast<LoongArchAsmBackend &>(
      getStreamer().getAssembler().getBackend());
  setTargetABI(LoongArchABI::computeTargetABI(
      STI.getTargetTriple(), STI.getFeatureBits(),
      MAB.getTargetOptions().getABIName()));
}

MCELFStreamer &LoongArchTargetELFStreamer::getStreamer() {
  return static_cast<MCELFStreamer &>(Streamer);
}

void LoongArchTargetELFStreamer::emitDirectiveOptionPush() {}
void LoongArchTargetELFStreamer::emitDirectiveOptionPop() {}
void LoongArchTargetELFStreamer::emitDirectiveOptionRelax() {}
void LoongArchTargetELFStreamer::emitDirectiveOptionNoRelax() {}

void LoongArchTargetELFStreamer::finish() {
  LoongArchTargetStreamer::finish();
  ELFObjectWriter &W = getStreamer().getWriter();
  LoongArchABI::ABI ABI = getTargetABI();

  // Figure out the e_flags.
  //
  // Bitness is already represented with the EI_CLASS byte in the current spec,
  // so here we only record the base ABI modifier. Also set the object file ABI
  // version to v1, as upstream LLVM cannot handle the previous stack-machine-
  // based relocs from day one.
  //
  // Refer to LoongArch ELF psABI v2.01 for details.
  unsigned EFlags = W.getELFHeaderEFlags();
  EFlags |= ELF::EF_LOONGARCH_OBJABI_V1;
  switch (ABI) {
  case LoongArchABI::ABI_ILP32S:
  case LoongArchABI::ABI_LP64S:
    EFlags |= ELF::EF_LOONGARCH_ABI_SOFT_FLOAT;
    break;
  case LoongArchABI::ABI_ILP32F:
  case LoongArchABI::ABI_LP64F:
    EFlags |= ELF::EF_LOONGARCH_ABI_SINGLE_FLOAT;
    break;
  case LoongArchABI::ABI_ILP32D:
  case LoongArchABI::ABI_LP64D:
    EFlags |= ELF::EF_LOONGARCH_ABI_DOUBLE_FLOAT;
    break;
  case LoongArchABI::ABI_Unknown:
    llvm_unreachable("Improperly initialized target ABI");
  }
  W.setELFHeaderEFlags(EFlags);
}

namespace {
class LoongArchELFStreamer : public MCELFStreamer {
public:
  LoongArchELFStreamer(MCContext &C, std::unique_ptr<MCAsmBackend> MAB,
                       std::unique_ptr<MCObjectWriter> MOW,
                       std::unique_ptr<MCCodeEmitter> MCE)
      : MCELFStreamer(C, std::move(MAB), std::move(MOW), std::move(MCE)) {}
};
} // end namespace

namespace vm::core {
MCELFStreamer *createLoongArchELFStreamer(MCContext &C,
                                          std::unique_ptr<MCAsmBackend> MAB,
                                          std::unique_ptr<MCObjectWriter> MOW,
                                          std::unique_ptr<MCCodeEmitter> MCE) {
  LoongArchELFStreamer *S = new LoongArchELFStreamer(
      C, std::move(MAB), std::move(MOW), std::move(MCE));
  return S;
}
} // end namespace vm::core
