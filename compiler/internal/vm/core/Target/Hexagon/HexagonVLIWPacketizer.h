//===- HexagonPacketizer.h - VLIW packetizer --------------------*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_HEXAGON_HEXAGONVLIWPACKETIZER_H
#define LLVM_LIB_TARGET_HEXAGON_HEXAGONVLIWPACKETIZER_H

#include "vm/core/CodeGen/DFAPacketizer.h"
#include "vm/core/CodeGen/MachineBasicBlock.h"
#include "vm/core/CodeGen/ScheduleDAG.h"
#include <vector>

namespace vm::core {

class HexagonInstrInfo;
class HexagonRegisterInfo;
class MachineBranchProbabilityInfo;
class MachineFunction;
class MachineInstr;
class MachineLoopInfo;
class TargetRegisterClass;

class HexagonPacketizerList : public VLIWPacketizerList {
  // Vector of instructions assigned to the packet that has just been created.
  std::vector<MachineInstr *> OldPacketMIs;

  // Has the instruction been promoted to a dot-new instruction.
  bool PromotedToDotNew;

  // Has the instruction been glued to allocframe.
  bool GlueAllocframeStore;

  // Has the feeder instruction been glued to new value jump.
  bool GlueToNewValueJump;

  // This holds the offset value, when pruning the dependences.
  int64_t ChangedOffset;

  // Check if there is a dependence between some instruction already in this
  // packet and this instruction.
  bool Dependence;

  // Only check for dependence if there are resources available to
  // schedule this instruction.
  bool FoundSequentialDependence;

  bool MemShufDisabled = false;

  // Track MIs with ignored dependence.
  std::vector<MachineInstr*> IgnoreDepMIs;

  // Set to true if the packet contains an instruction that stalls with an
  // instruction from the previous packet.
  bool PacketStalls = false;
  // Set to the number of cycles of stall a given instruction will incur
  // because of dependence on instruction in previous packet.
  unsigned int PacketStallCycles = 0;

  // Set to true if the packet has a duplex pair of sub-instructions.
  bool PacketHasDuplex = false;

  // Set to true if the packet has a instruction that can only be executed
  // in SLOT0.
  bool PacketHasSLOT0OnlyInsn = false;

protected:
  /// A handle to the branch probability pass.
  const MachineBranchProbabilityInfo *MBPI;
  const MachineLoopInfo *MLI;

private:
  const HexagonInstrInfo *HII;
  const HexagonRegisterInfo *HRI;
  const bool Minimal;

public:
  HexagonPacketizerList(MachineFunction &MF, MachineLoopInfo &MLI,
                        AAResults *AA, const MachineBranchProbabilityInfo *MBPI,
                        bool Minimal);

  // initPacketizerState - initialize some internal flags.
  void initPacketizerState() override;

  // ignorePseudoInstruction - Ignore bundling of pseudo instructions.
  bool ignorePseudoInstruction(const MachineInstr &MI,
                               const MachineBasicBlock *MBB) override;

  // isSoloInstruction - return true if instruction MI can not be packetized
  // with any other instruction, which means that MI itself is a packet.
  bool isSoloInstruction(const MachineInstr &MI) override;

  // isLegalToPacketizeTogether - Is it legal to packetize SUI and SUJ
  // together.
  bool isLegalToPacketizeTogether(SUnit *SUI, SUnit *SUJ) override;

  // isLegalToPruneDependencies - Is it legal to prune dependence between SUI
  // and SUJ.
  bool isLegalToPruneDependencies(SUnit *SUI, SUnit *SUJ) override;

  bool foundLSInPacket();
  MachineBasicBlock::iterator addToPacket(MachineInstr &MI) override;
  void endPacket(MachineBasicBlock *MBB,
                 MachineBasicBlock::iterator MI) override;
  bool shouldAddToPacket(const MachineInstr &MI) override;

  void unpacketizeSoloInstrs(MachineFunction &MF);

protected:
  bool getmemShufDisabled() {
    return MemShufDisabled;
  };
  void setmemShufDisabled(bool val) {
    MemShufDisabled = val;
  };
  bool isCallDependent(const MachineInstr &MI, SDep::Kind DepType,
                       unsigned DepReg);
  bool promoteToDotCur(MachineInstr &MI, SDep::Kind DepType,
                       MachineBasicBlock::iterator &MII,
                       const TargetRegisterClass *RC);
  bool canPromoteToDotCur(const MachineInstr &MI, const SUnit *PacketSU,
                          unsigned DepReg, MachineBasicBlock::iterator &MII,
                          const TargetRegisterClass *RC);
  void cleanUpDotCur();

  bool promoteToDotNew(MachineInstr &MI, SDep::Kind DepType,
                       MachineBasicBlock::iterator &MII,
                       const TargetRegisterClass *RC);
  bool canPromoteToDotNew(const MachineInstr &MI, const SUnit *PacketSU,
                          unsigned DepReg, MachineBasicBlock::iterator &MII,
                          const TargetRegisterClass *RC);
  bool canPromoteToNewValue(const MachineInstr &MI, const SUnit *PacketSU,
                            unsigned DepReg, MachineBasicBlock::iterator &MII);
  bool canPromoteToNewValueStore(const MachineInstr &MI,
                                 const MachineInstr &PacketMI, unsigned DepReg);
  bool demoteToDotOld(MachineInstr &MI);
  bool useCallersSP(MachineInstr &MI);
  void useCalleesSP(MachineInstr &MI);
  bool updateOffset(SUnit *SUI, SUnit *SUJ);
  void undoChangedOffset(MachineInstr &MI);
  bool arePredicatesComplements(MachineInstr &MI1, MachineInstr &MI2);
  bool restrictingDepExistInPacket(MachineInstr&, unsigned);
  bool isNewifiable(const MachineInstr &MI, const TargetRegisterClass *NewRC);
  bool isCurifiable(MachineInstr &MI);
  bool cannotCoexist(const MachineInstr &MI, const MachineInstr &MJ);

  bool isPromotedToDotNew() const {
    return PromotedToDotNew;
  }

  bool tryAllocateResourcesForConstExt(bool Reserve);
  bool canReserveResourcesForConstExt();
  void reserveResourcesForConstExt();
  bool hasDeadDependence(const MachineInstr &I, const MachineInstr &J);
  bool hasControlDependence(const MachineInstr &I, const MachineInstr &J);
  bool hasRegMaskDependence(const MachineInstr &I, const MachineInstr &J);
  bool hasDualStoreDependence(const MachineInstr &I, const MachineInstr &J);
  bool producesStall(const MachineInstr &MI);
  unsigned int calcStall(const MachineInstr &MI);
};

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_HEXAGON_HEXAGONVLIWPACKETIZER_H
