//===- DirectXMCTargetDesc.cpp - DirectX Target Implementation --*- C++ -*-===//
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
///
/// \file
/// This file contains DirectX target initializer.
///
//===----------------------------------------------------------------------===//

#include "DirectXMCTargetDesc.h"
#include "DirectXContainerObjectWriter.h"
#include "TargetInfo/DirectXTargetInfo.h"
#include "vm/core/MC/LaneBitmask.h"
#include "vm/core/MC/MCAsmBackend.h"
#include "vm/core/MC/MCAsmInfo.h"
#include "vm/core/MC/MCCodeEmitter.h"
#include "vm/core/MC/MCDXContainerWriter.h"
#include "vm/core/MC/MCInstPrinter.h"
#include "vm/core/MC/MCInstrInfo.h"
#include "vm/core/MC/MCRegisterInfo.h"
#include "vm/core/MC/MCSchedule.h"
#include "vm/core/MC/MCSubtargetInfo.h"
#include "vm/core/MC/TargetRegistry.h"
#include "vm/core/Support/Compiler.h"
#include "vm/core/TargetParser/Triple.h"
#include <memory>

using namespace vm::core;

#define GET_INSTRINFO_MC_DESC
#define GET_INSTRINFO_MC_HELPERS
#include "DirectXGenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "DirectXGenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "DirectXGenRegisterInfo.inc"

namespace {

// DXILInstPrinter is a null stub because DXIL instructions aren't printed.
class DXILInstPrinter : public MCInstPrinter {
public:
  DXILInstPrinter(const MCAsmInfo &MAI, const MCInstrInfo &MII,
                  const MCRegisterInfo &MRI)
      : MCInstPrinter(MAI, MII, MRI) {}

  void printInst(const MCInst *MI, uint64_t Address, StringRef Annot,
                 const MCSubtargetInfo &STI, raw_ostream &O) override {}

  std::pair<const char *, uint64_t>
  getMnemonic(const MCInst &MI) const override {
    return std::make_pair<const char *, uint64_t>("", 0ull);
  }

private:
};

class DXILMCCodeEmitter : public MCCodeEmitter {
public:
  DXILMCCodeEmitter() {}

  void encodeInstruction(const MCInst &Inst, SmallVectorImpl<char> &CB,
                         SmallVectorImpl<MCFixup> &Fixups,
                         const MCSubtargetInfo &STI) const override {}
};

class DXILAsmBackend : public MCAsmBackend {

public:
  DXILAsmBackend(const MCSubtargetInfo &STI)
      : MCAsmBackend(toolchain::endianness::little) {}
  ~DXILAsmBackend() override = default;

  void applyFixup(const MCFragment &, const MCFixup &, const MCValue &Target,
                  uint8_t *Data, uint64_t Value, bool IsResolved) override {}

  std::unique_ptr<MCObjectTargetWriter>
  createObjectTargetWriter() const override {
    return createDXContainerTargetObjectWriter();
  }

  bool writeNopData(raw_ostream &OS, uint64_t Count,
                    const MCSubtargetInfo *STI) const override {
    return true;
  }
};

class DirectXMCAsmInfo : public MCAsmInfo {
public:
  explicit DirectXMCAsmInfo(const Triple &TT, const MCTargetOptions &Options)
      : MCAsmInfo() {}
};

} // namespace

static MCInstPrinter *createDXILMCInstPrinter(const Triple &T,
                                              unsigned SyntaxVariant,
                                              const MCAsmInfo &MAI,
                                              const MCInstrInfo &MII,
                                              const MCRegisterInfo &MRI) {
  if (SyntaxVariant == 0)
    return new DXILInstPrinter(MAI, MII, MRI);
  return nullptr;
}

MCCodeEmitter *createDXILMCCodeEmitter(const MCInstrInfo &MCII,
                                       MCContext &Ctx) {
  return new DXILMCCodeEmitter();
}

MCAsmBackend *createDXILMCAsmBackend(const Target &T,
                                     const MCSubtargetInfo &STI,
                                     const MCRegisterInfo &MRI,
                                     const MCTargetOptions &Options) {
  return new DXILAsmBackend(STI);
}

static MCSubtargetInfo *
createDirectXMCSubtargetInfo(const Triple &TT, StringRef CPU, StringRef FS) {
  return createDirectXMCSubtargetInfoImpl(TT, CPU, /*TuneCPU*/ CPU, FS);
}

static MCRegisterInfo *createDirectXMCRegisterInfo(const Triple &Triple) {
  return new MCRegisterInfo();
}

static MCInstrInfo *createDirectXMCInstrInfo() { return new MCInstrInfo(); }

extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void
LLVMInitializeDirectXTargetMC() {
  Target &T = getTheDirectXTarget();
  RegisterMCAsmInfo<DirectXMCAsmInfo> X(T);
  TargetRegistry::RegisterMCInstrInfo(T, createDirectXMCInstrInfo);
  TargetRegistry::RegisterMCInstPrinter(T, createDXILMCInstPrinter);
  TargetRegistry::RegisterMCRegInfo(T, createDirectXMCRegisterInfo);
  TargetRegistry::RegisterMCSubtargetInfo(T, createDirectXMCSubtargetInfo);
  TargetRegistry::RegisterMCCodeEmitter(T, createDXILMCCodeEmitter);
  TargetRegistry::RegisterMCAsmBackend(T, createDXILMCAsmBackend);
}
