//===-- AMDGPUMCTargetDesc.h - AMDGPU Target Descriptions -----*- C++ -*-===//
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
/// Provides AMDGPU specific target descriptions.
//
//===----------------------------------------------------------------------===//
//

#ifndef LLVM_LIB_TARGET_AMDGPU_MCTARGETDESC_AMDGPUMCTARGETDESC_H
#define LLVM_LIB_TARGET_AMDGPU_MCTARGETDESC_AMDGPUMCTARGETDESC_H

#include "vm/core/MC/MCInstrAnalysis.h"
#include <cstdint>
#include <memory>

namespace vm::core {
class Target;
class MCAsmBackend;
class MCCodeEmitter;
class MCContext;
class MCInstrInfo;
class MCObjectTargetWriter;
class MCRegisterInfo;
class MCSubtargetInfo;
class MCTargetOptions;

enum AMDGPUDwarfFlavour : unsigned { Wave64 = 0, Wave32 = 1 };

MCRegisterInfo *createGCNMCRegisterInfo(AMDGPUDwarfFlavour DwarfFlavour);

MCCodeEmitter *createAMDGPUMCCodeEmitter(const MCInstrInfo &MCII,
                                         MCContext &Ctx);

MCAsmBackend *createAMDGPUAsmBackend(const Target &T,
                                     const MCSubtargetInfo &STI,
                                     const MCRegisterInfo &MRI,
                                     const MCTargetOptions &Options);

std::unique_ptr<MCObjectTargetWriter>
createAMDGPUELFObjectWriter(bool Is64Bit, uint8_t OSABI,
                            bool HasRelocationAddend);

namespace AMDGPU {
class AMDGPUMCInstrAnalysis : public MCInstrAnalysis {
private:
  unsigned VgprMSBs = 0;

public:
  explicit AMDGPUMCInstrAnalysis(const MCInstrInfo *Info)
      : MCInstrAnalysis(Info) {}

  bool evaluateBranch(const MCInst &Inst, uint64_t Addr, uint64_t Size,
                      uint64_t &Target) const override;

  void resetState() override { VgprMSBs = 0; }

  void updateState(const MCInst &Inst, uint64_t Addr) override;

  unsigned getVgprMSBs() const { return VgprMSBs; }
};

} // namespace AMDGPU

} // namespace vm::core

#define GET_REGINFO_ENUM
#include "AMDGPUGenRegisterInfo.inc"

#define GET_INSTRINFO_ENUM
#define GET_INSTRINFO_MC_HELPER_DECLS
#include "AMDGPUGenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "AMDGPUGenSubtargetInfo.inc"

#endif
