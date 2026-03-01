//===-- HexagonMCTargetDesc.h - Hexagon Target Descriptions -----*- C++ -*-===//
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
// This file provides Hexagon specific target descriptions.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_HEXAGON_MCTARGETDESC_HEXAGONMCTARGETDESC_H
#define LLVM_LIB_TARGET_HEXAGON_MCTARGETDESC_HEXAGONMCTARGETDESC_H

#include "vm/core/MC/MCRegisterInfo.h"
#include "vm/core/Support/CommandLine.h"
#include <cstdint>

#define Hexagon_POINTER_SIZE 4

#define Hexagon_PointerSize (Hexagon_POINTER_SIZE)
#define Hexagon_PointerSize_Bits (Hexagon_POINTER_SIZE * 8)
#define Hexagon_WordSize Hexagon_PointerSize
#define Hexagon_WordSize_Bits Hexagon_PointerSize_Bits

// allocframe saves LR and FP on stack before allocating
// a new stack frame. This takes 8 bytes.
#define HEXAGON_LRFP_SIZE 8

// Normal instruction size (in bytes).
#define HEXAGON_INSTR_SIZE 4

// Maximum number of words and instructions in a packet.
#define HEXAGON_PACKET_SIZE 4
#define HEXAGON_MAX_PACKET_SIZE (HEXAGON_PACKET_SIZE * HEXAGON_INSTR_SIZE)
// Minimum number of instructions in an end-loop packet.
#define HEXAGON_PACKET_INNER_SIZE 2
#define HEXAGON_PACKET_OUTER_SIZE 3
// Maximum number of instructions in a packet before shuffling,
// including a compound one or a duplex or an extender.
#define HEXAGON_PRESHUFFLE_PACKET_SIZE (HEXAGON_PACKET_SIZE + 3)

// Name of the global offset table as defined by the Hexagon ABI
#define HEXAGON_GOT_SYM_NAME "_GLOBAL_OFFSET_TABLE_"

namespace vm::core {

struct InstrStage;
class FeatureBitset;
class MCAsmBackend;
class MCCodeEmitter;
class MCContext;
class MCInstrInfo;
class MCObjectTargetWriter;
class MCRegisterInfo;
class MCSubtargetInfo;
class MCTargetOptions;
class Target;
class Triple;
class StringRef;

extern cl::opt<bool> HexagonDisableCompound;
extern cl::opt<bool> HexagonDisableDuplex;
extern const InstrStage HexagonStages[];

MCInstrInfo *createHexagonMCInstrInfo();
MCRegisterInfo *createHexagonMCRegisterInfo(StringRef TT);

namespace Hexagon_MC {
  StringRef selectHexagonCPU(StringRef CPU);

  FeatureBitset completeHVXFeatures(const FeatureBitset &FB);
  /// Create a Hexagon MCSubtargetInfo instance. This is exposed so Asm parser,
  /// etc. do not need to go through TargetRegistry.
  MCSubtargetInfo *createHexagonMCSubtargetInfo(const Triple &TT, StringRef CPU,
                                                StringRef FS);
  MCSubtargetInfo const *getArchSubtarget(MCSubtargetInfo const *STI);
  void addArchSubtarget(MCSubtargetInfo const *STI,
                        StringRef FS);
  unsigned GetELFFlags(const MCSubtargetInfo &STI);

  toolchain::ArrayRef<MCPhysReg> GetVectRegRev();

  std::optional<unsigned> getHVXVersion(const FeatureBitset &Features);

  unsigned getArchVersion(const FeatureBitset &Features);
  } // namespace Hexagon_MC

MCCodeEmitter *createHexagonMCCodeEmitter(const MCInstrInfo &MCII,
                                          MCContext &MCT);

MCAsmBackend *createHexagonAsmBackend(const Target &T,
                                      const MCSubtargetInfo &STI,
                                      const MCRegisterInfo &MRI,
                                      const MCTargetOptions &Options);

std::unique_ptr<MCObjectTargetWriter>
createHexagonELFObjectWriter(uint8_t OSABI, StringRef CPU);

unsigned HexagonGetLastSlot();
unsigned HexagonConvertUnits(unsigned ItinUnits, unsigned *Lanes);

} // End toolchain namespace

// Define symbolic names for Hexagon registers.  This defines a mapping from
// register name to register number.
//
#define GET_REGINFO_ENUM
#include "HexagonGenRegisterInfo.inc"

// Defines symbolic names for the Hexagon instructions.
//
#define GET_INSTRINFO_ENUM
#define GET_INSTRINFO_SCHED_ENUM
#define GET_INSTRINFO_MC_HELPER_DECLS
#include "HexagonGenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "HexagonGenSubtargetInfo.inc"

#endif // LLVM_LIB_TARGET_HEXAGON_MCTARGETDESC_HEXAGONMCTARGETDESC_H
