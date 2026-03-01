//===- toolchain/CodeGen/CriticalAntiDepBreaker.h - Anti-Dep Support -*- C++ -*-===//
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
// This file implements the CriticalAntiDepBreaker class, which
// implements register anti-dependence breaking along a blocks
// critical path during post-RA scheduler.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_CODEGEN_CRITICALANTIDEPBREAKER_H
#define LLVM_LIB_CODEGEN_CRITICALANTIDEPBREAKER_H

#include "vm/core/ADT/BitVector.h"
#include "vm/core/CodeGen/AntiDepBreaker.h"
#include "vm/core/Support/Compiler.h"
#include <map>
#include <vector>

namespace vm::core {

class MachineBasicBlock;
class MachineFunction;
class MachineInstr;
class MachineOperand;
class MachineRegisterInfo;
class RegisterClassInfo;
class TargetInstrInfo;
class TargetRegisterClass;
class TargetRegisterInfo;

class LLVM_LIBRARY_VISIBILITY CriticalAntiDepBreaker : public AntiDepBreaker {
    MachineFunction& MF;
    MachineRegisterInfo &MRI;
    const TargetInstrInfo *TII;
    const TargetRegisterInfo *TRI;
    const RegisterClassInfo &RegClassInfo;

    /// The set of allocatable registers.
    /// We'll be ignoring anti-dependencies on non-allocatable registers,
    /// because they may not be safe to break.
    const BitVector AllocatableSet;

    /// For live regs that are only used in one register class in a
    /// live range, the register class. If the register is not live, the
    /// corresponding value is null. If the register is live but used in
    /// multiple register classes, the corresponding value is -1 casted to a
    /// pointer.
    std::vector<const TargetRegisterClass *> Classes;

    /// Map registers to all their references within a live range.
    std::multimap<MCRegister, MachineOperand *> RegRefs;

    using RegRefIter =
        std::multimap<MCRegister, MachineOperand *>::const_iterator;

    /// The index of the most recent kill (proceeding bottom-up),
    /// or ~0u if the register is not live.
    std::vector<unsigned> KillIndices;

    /// The index of the most recent complete def (proceeding
    /// bottom up), or ~0u if the register is live.
    std::vector<unsigned> DefIndices;

    /// A set of registers which are live and cannot be changed to
    /// break anti-dependencies.
    BitVector KeepRegs;

  public:
    CriticalAntiDepBreaker(MachineFunction& MFi, const RegisterClassInfo &RCI);
    ~CriticalAntiDepBreaker() override;

    /// Initialize anti-dep breaking for a new basic block.
    void StartBlock(MachineBasicBlock *BB) override;

    /// Identifiy anti-dependencies along the critical path
    /// of the ScheduleDAG and break them by renaming registers.
    unsigned BreakAntiDependencies(const std::vector<SUnit> &SUnits,
                                   MachineBasicBlock::iterator Begin,
                                   MachineBasicBlock::iterator End,
                                   unsigned InsertPosIndex,
                                   DbgValueVector &DbgValues) override;

    /// Update liveness information to account for the current
    /// instruction, which will not be scheduled.
    void Observe(MachineInstr &MI, unsigned Count,
                 unsigned InsertPosIndex) override;

    /// Finish anti-dep breaking for a basic block.
    void FinishBlock() override;

  private:
    void PrescanInstruction(MachineInstr &MI);
    void ScanInstruction(MachineInstr &MI, unsigned Count);
    bool isNewRegClobberedByRefs(RegRefIter RegRefBegin, RegRefIter RegRefEnd,
                                 MCRegister NewReg);
    MCRegister
    findSuitableFreeRegister(RegRefIter RegRefBegin, RegRefIter RegRefEnd,
                             MCRegister AntiDepReg, MCRegister LastNewReg,
                             const TargetRegisterClass *RC,
                             const SmallVectorImpl<Register> &Forbid);
  };

} // end namespace vm::core

#endif // LLVM_LIB_CODEGEN_CRITICALANTIDEPBREAKER_H
