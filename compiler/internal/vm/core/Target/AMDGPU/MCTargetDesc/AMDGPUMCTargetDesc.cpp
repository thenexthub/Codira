//===-- AMDGPUMCTargetDesc.cpp - AMDGPU Target Descriptions ---------------===//
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
/// \file
/// This file provides AMDGPU specific target descriptions.
//
//===----------------------------------------------------------------------===//

#include "AMDGPUMCTargetDesc.h"
#include "AMDGPUELFStreamer.h"
#include "AMDGPUInstPrinter.h"
#include "AMDGPUMCAsmInfo.h"
#include "AMDGPUTargetStreamer.h"
#include "R600InstPrinter.h"
#include "R600MCTargetDesc.h"
#include "TargetInfo/AMDGPUTargetInfo.h"
#include "vm/core/MC/MCAsmBackend.h"
#include "vm/core/MC/MCCodeEmitter.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/MC/MCELFStreamer.h"
#include "vm/core/MC/MCInstPrinter.h"
#include "vm/core/MC/MCInstrDesc.h"
#include "vm/core/MC/MCInstrInfo.h"
#include "vm/core/MC/MCObjectWriter.h"
#include "vm/core/MC/MCRegisterInfo.h"
#include "vm/core/MC/MCStreamer.h"
#include "vm/core/MC/MCSubtargetInfo.h"
#include "vm/core/MC/TargetRegistry.h"
#include "vm/core/Support/Compiler.h"

using namespace vm::core;

#define GET_INSTRINFO_MC_DESC
#define ENABLE_INSTR_PREDICATE_VERIFIER
#include "AMDGPUGenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "AMDGPUGenSubtargetInfo.inc"

#define NoSchedModel NoSchedModelR600
#define GET_SUBTARGETINFO_MC_DESC
#include "R600GenSubtargetInfo.inc"
#undef NoSchedModelR600

#define GET_REGINFO_MC_DESC
#include "AMDGPUGenRegisterInfo.inc"

#define GET_REGINFO_MC_DESC
#include "R600GenRegisterInfo.inc"

static MCInstrInfo *createAMDGPUMCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitAMDGPUMCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createAMDGPUMCRegisterInfo(const Triple &TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  if (TT.getArch() == Triple::r600)
    InitR600MCRegisterInfo(X, 0);
  else
    InitAMDGPUMCRegisterInfo(X, AMDGPU::PC_REG);
  return X;
}

MCRegisterInfo *toolchain::createGCNMCRegisterInfo(AMDGPUDwarfFlavour DwarfFlavour) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitAMDGPUMCRegisterInfo(X, AMDGPU::PC_REG, DwarfFlavour, DwarfFlavour);
  return X;
}

static MCSubtargetInfo *
createAMDGPUMCSubtargetInfo(const Triple &TT, StringRef CPU, StringRef FS) {
  if (TT.getArch() == Triple::r600)
    return createR600MCSubtargetInfoImpl(TT, CPU, /*TuneCPU*/ CPU, FS);

  MCSubtargetInfo *STI =
      createAMDGPUMCSubtargetInfoImpl(TT, CPU, /*TuneCPU*/ CPU, FS);

  bool IsWave64 = STI->hasFeature(AMDGPU::FeatureWavefrontSize64);
  bool IsWave32 = STI->hasFeature(AMDGPU::FeatureWavefrontSize32);

  // FIXME: We should error for the default target.
  if (STI->getFeatureBits().none())
    STI->ToggleFeature(AMDGPU::FeatureSouthernIslands);

  if (!IsWave64 && !IsWave32) {
    // If there is no default wave size it must be a generation before gfx10,
    // these have FeatureWavefrontSize64 in their definition already. For gfx10+
    // set wave32 as a default.
    STI->ToggleFeature(AMDGPU::isGFX10Plus(*STI)
                           ? AMDGPU::FeatureWavefrontSize32
                           : AMDGPU::FeatureWavefrontSize64);
  } else if (IsWave64 && IsWave32) {
    // The wave size is mutually exclusive. If both somehow end up set, wave32
    // wins if supported.
    STI->ToggleFeature(AMDGPU::supportsWave32(*STI)
                           ? AMDGPU::FeatureWavefrontSize64
                           : AMDGPU::FeatureWavefrontSize32);

    // If both wavesizes were manually requested, hack in a feature to permit
    // assembling modules with mixed wavesizes.
    STI->ToggleFeature(AMDGPU::FeatureAssemblerPermissiveWavesize);
  }

  assert((STI->hasFeature(AMDGPU::FeatureWavefrontSize64) !=
          STI->hasFeature(AMDGPU::FeatureWavefrontSize32)) &&
         "wavesize features are mutually exclusive");

  return STI;
}

static MCInstPrinter *createAMDGPUMCInstPrinter(const Triple &T,
                                                unsigned SyntaxVariant,
                                                const MCAsmInfo &MAI,
                                                const MCInstrInfo &MII,
                                                const MCRegisterInfo &MRI) {
  if (T.getArch() == Triple::r600)
    return new R600InstPrinter(MAI, MII, MRI);
  return new AMDGPUInstPrinter(MAI, MII, MRI);
}

static MCTargetStreamer *
createAMDGPUAsmTargetStreamer(MCStreamer &S, formatted_raw_ostream &OS,
                              MCInstPrinter *InstPrint) {
  return new AMDGPUTargetAsmStreamer(S, OS);
}

static MCTargetStreamer * createAMDGPUObjectTargetStreamer(
                                                   MCStreamer &S,
                                                   const MCSubtargetInfo &STI) {
  return new AMDGPUTargetELFStreamer(S, STI);
}

static MCTargetStreamer *createAMDGPUNullTargetStreamer(MCStreamer &S) {
  return new AMDGPUTargetStreamer(S);
}

static MCStreamer *createMCStreamer(const Triple &T, MCContext &Context,
                                    std::unique_ptr<MCAsmBackend> &&MAB,
                                    std::unique_ptr<MCObjectWriter> &&OW,
                                    std::unique_ptr<MCCodeEmitter> &&Emitter) {
  return createAMDGPUELFStreamer(T, Context, std::move(MAB), std::move(OW),
                                 std::move(Emitter));
}

namespace vm::core {
namespace AMDGPU {

bool AMDGPUMCInstrAnalysis::evaluateBranch(const MCInst &Inst, uint64_t Addr,
                                           uint64_t Size,
                                           uint64_t &Target) const {
  if (Inst.getNumOperands() == 0 || !Inst.getOperand(0).isImm() ||
      Info->get(Inst.getOpcode()).operands()[0].OperandType !=
          MCOI::OPERAND_PCREL)
    return false;

  int64_t Imm = Inst.getOperand(0).getImm();
  // Our branches take a simm16.
  Target = SignExtend64<16>(Imm) * 4 + Addr + Size;
  return true;
}

void AMDGPUMCInstrAnalysis::updateState(const MCInst &Inst, uint64_t Addr) {
  if (Inst.getOpcode() == AMDGPU::S_SET_VGPR_MSB_gfx12)
    VgprMSBs = Inst.getOperand(0).getImm() & 0xff;
  else if (isTerminator(Inst))
    VgprMSBs = 0;
}

} // end namespace AMDGPU
} // end namespace vm::core

static MCInstrAnalysis *createAMDGPUMCInstrAnalysis(const MCInstrInfo *Info) {
  return new AMDGPU::AMDGPUMCInstrAnalysis(Info);
}

extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void
LLVMInitializeAMDGPUTargetMC() {

  TargetRegistry::RegisterMCInstrInfo(getTheGCNTarget(), createAMDGPUMCInstrInfo);
  TargetRegistry::RegisterMCInstrInfo(getTheR600Target(),
                                      createR600MCInstrInfo);
  for (Target *T : {&getTheR600Target(), &getTheGCNTarget()}) {
    RegisterMCAsmInfo<AMDGPUMCAsmInfo> X(*T);

    TargetRegistry::RegisterMCRegInfo(*T, createAMDGPUMCRegisterInfo);
    TargetRegistry::RegisterMCSubtargetInfo(*T, createAMDGPUMCSubtargetInfo);
    TargetRegistry::RegisterMCInstPrinter(*T, createAMDGPUMCInstPrinter);
    TargetRegistry::RegisterMCInstrAnalysis(*T, createAMDGPUMCInstrAnalysis);
    TargetRegistry::RegisterMCAsmBackend(*T, createAMDGPUAsmBackend);
    TargetRegistry::RegisterELFStreamer(*T, createMCStreamer);
  }

  // R600 specific registration
  TargetRegistry::RegisterMCCodeEmitter(getTheR600Target(),
                                        createR600MCCodeEmitter);
  TargetRegistry::RegisterObjectTargetStreamer(
      getTheR600Target(), createAMDGPUObjectTargetStreamer);

  // GCN specific registration
  TargetRegistry::RegisterMCCodeEmitter(getTheGCNTarget(),
                                        createAMDGPUMCCodeEmitter);

  TargetRegistry::RegisterAsmTargetStreamer(getTheGCNTarget(),
                                            createAMDGPUAsmTargetStreamer);
  TargetRegistry::RegisterObjectTargetStreamer(
      getTheGCNTarget(), createAMDGPUObjectTargetStreamer);
  TargetRegistry::RegisterNullTargetStreamer(getTheGCNTarget(),
                                             createAMDGPUNullTargetStreamer);
}
