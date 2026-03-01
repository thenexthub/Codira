//===- lib/CodeGen/MachineStableHash.cpp ----------------------------------===//
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
// Stable hashing for MachineInstr and MachineOperand. Useful or getting a
// hash across runs, modules, etc.
//
//===----------------------------------------------------------------------===//

#include "vm/core/CodeGen/MachineStableHash.h"
#include "vm/core/ADT/APFloat.h"
#include "vm/core/ADT/APInt.h"
#include "vm/core/ADT/STLExtras.h"
#include "vm/core/ADT/SmallVector.h"
#include "vm/core/ADT/StableHashing.h"
#include "vm/core/ADT/Statistic.h"
#include "vm/core/CodeGen/MachineBasicBlock.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/CodeGen/MachineInstr.h"
#include "vm/core/CodeGen/MachineMemOperand.h"
#include "vm/core/CodeGen/MachineOperand.h"
#include "vm/core/CodeGen/MachineRegisterInfo.h"
#include "vm/core/CodeGen/Register.h"
#include "vm/core/Config/toolchain-config.h"
#include "vm/core/IR/Constants.h"
#include "vm/core/IR/GlobalVariable.h"
#include "vm/core/IR/StructuralHash.h"
#include "vm/core/MC/MCSymbol.h"
#include "vm/core/Support/Alignment.h"
#include "vm/core/Support/ErrorHandling.h"

#define DEBUG_TYPE "machine-stable-hash"

using namespace vm::core;

STATISTIC(StableHashBailingMachineBasicBlock,
          "Number of encountered unsupported MachineOperands that were "
          "MachineBasicBlocks while computing stable hashes");
STATISTIC(StableHashBailingConstantPoolIndex,
          "Number of encountered unsupported MachineOperands that were "
          "ConstantPoolIndex while computing stable hashes");
STATISTIC(StableHashBailingTargetIndexNoName,
          "Number of encountered unsupported MachineOperands that were "
          "TargetIndex with no name");
STATISTIC(StableHashBailingGlobalAddress,
          "Number of encountered unsupported MachineOperands that were "
          "GlobalAddress while computing stable hashes");
STATISTIC(StableHashBailingBlockAddress,
          "Number of encountered unsupported MachineOperands that were "
          "BlockAddress while computing stable hashes");
STATISTIC(StableHashBailingMetadataUnsupported,
          "Number of encountered unsupported MachineOperands that were "
          "Metadata of an unsupported kind while computing stable hashes");

stable_hash toolchain::stableHashValue(const MachineOperand &MO) {
  switch (MO.getType()) {
  case MachineOperand::MO_Register:
    if (MO.getReg().isVirtual()) {
      const MachineRegisterInfo &MRI = MO.getParent()->getMF()->getRegInfo();
      SmallVector<stable_hash> DefOpcodes;
      for (auto &Def : MRI.def_instructions(MO.getReg()))
        DefOpcodes.push_back(Def.getOpcode());
      return stable_hash_combine(DefOpcodes);
    }

    // Register operands don't have target flags.
    return stable_hash_combine(MO.getType(), MO.getReg().id(), MO.getSubReg(),
                               MO.isDef());
  case MachineOperand::MO_Immediate:
    return stable_hash_combine(MO.getType(), MO.getTargetFlags(), MO.getImm());
  case MachineOperand::MO_CImmediate:
  case MachineOperand::MO_FPImmediate: {
    auto Val = MO.isCImm() ? MO.getCImm()->getValue()
                           : MO.getFPImm()->getValueAPF().bitcastToAPInt();
    auto ValHash = stable_hash_combine(
        ArrayRef<stable_hash>(Val.getRawData(), Val.getNumWords()));
    return stable_hash_combine(MO.getType(), MO.getTargetFlags(), ValHash);
  }

  case MachineOperand::MO_MachineBasicBlock:
    ++StableHashBailingMachineBasicBlock;
    return 0;
  case MachineOperand::MO_ConstantPoolIndex:
    ++StableHashBailingConstantPoolIndex;
    return 0;
  case MachineOperand::MO_BlockAddress:
    ++StableHashBailingBlockAddress;
    return 0;
  case MachineOperand::MO_Metadata:
    ++StableHashBailingMetadataUnsupported;
    return 0;
  case MachineOperand::MO_GlobalAddress: {
    const GlobalValue *GV = MO.getGlobal();
    stable_hash GVHash = 0;
    if (auto *GVar = dyn_cast<GlobalVariable>(GV))
      GVHash = StructuralHash(*GVar);
    if (!GVHash) {
      if (!GV->hasName()) {
        ++StableHashBailingGlobalAddress;
        return 0;
      }
      GVHash = stable_hash_name(GV->getName());
    }

    return stable_hash_combine(MO.getType(), MO.getTargetFlags(), GVHash,
                               MO.getOffset());
  }

  case MachineOperand::MO_TargetIndex: {
    if (const char *Name = MO.getTargetIndexName())
      return stable_hash_combine(MO.getType(), MO.getTargetFlags(),
                                 stable_hash_name(Name), MO.getOffset());
    ++StableHashBailingTargetIndexNoName;
    return 0;
  }

  case MachineOperand::MO_FrameIndex:
  case MachineOperand::MO_JumpTableIndex:
    return stable_hash_combine(MO.getType(), MO.getTargetFlags(),
                               MO.getIndex());

  case MachineOperand::MO_ExternalSymbol:
    return stable_hash_combine(MO.getType(), MO.getTargetFlags(),
                               MO.getOffset(),
                               stable_hash_name(MO.getSymbolName()));

  case MachineOperand::MO_RegisterMask:
  case MachineOperand::MO_RegisterLiveOut: {
    if (const MachineInstr *MI = MO.getParent()) {
      if (const MachineBasicBlock *MBB = MI->getParent()) {
        if (const MachineFunction *MF = MBB->getParent()) {
          const TargetRegisterInfo *TRI = MF->getSubtarget().getRegisterInfo();
          unsigned RegMaskSize =
              MachineOperand::getRegMaskSize(TRI->getNumRegs());
          const uint32_t *RegMask =
              MO.isRegMask() ? MO.getRegMask() : MO.getRegLiveOut();
          std::vector<toolchain::stable_hash> RegMaskHashes(RegMask,
                                                       RegMask + RegMaskSize);
          return stable_hash_combine(MO.getType(), MO.getTargetFlags(),
                                     stable_hash_combine(RegMaskHashes));
        }
      }
    }

    assert(0 && "MachineOperand not associated with any MachineFunction");
    return stable_hash_combine(MO.getType(), MO.getTargetFlags());
  }

  case MachineOperand::MO_ShuffleMask: {
    std::vector<toolchain::stable_hash> ShuffleMaskHashes;

    toolchain::transform(
        MO.getShuffleMask(), std::back_inserter(ShuffleMaskHashes),
        [](int S) -> toolchain::stable_hash { return toolchain::stable_hash(S); });

    return stable_hash_combine(MO.getType(), MO.getTargetFlags(),
                               stable_hash_combine(ShuffleMaskHashes));
  }
  case MachineOperand::MO_MCSymbol: {
    auto SymbolName = MO.getMCSymbol()->getName();
    return stable_hash_combine(MO.getType(), MO.getTargetFlags(),
                               stable_hash_name(SymbolName));
  }
  case MachineOperand::MO_LaneMask: {
    return stable_hash_combine(MO.getType(), MO.getTargetFlags(),
                               MO.getLaneMask().getAsInteger());
  }
  case MachineOperand::MO_CFIIndex:
    return stable_hash_combine(MO.getType(), MO.getTargetFlags(),
                               MO.getCFIIndex());
  case MachineOperand::MO_IntrinsicID:
    return stable_hash_combine(MO.getType(), MO.getTargetFlags(),
                               MO.getIntrinsicID());
  case MachineOperand::MO_Predicate:
    return stable_hash_combine(MO.getType(), MO.getTargetFlags(),
                               MO.getPredicate());
  case MachineOperand::MO_DbgInstrRef:
    return stable_hash_combine(MO.getType(), MO.getInstrRefInstrIndex(),
                               MO.getInstrRefOpIndex());
  }
  llvm_unreachable("Invalid machine operand type");
}

/// A stable hash value for machine instructions.
/// Returns 0 if no stable hash could be computed.
/// The hashing and equality testing functions ignore definitions so this is
/// useful for CSE, etc.
stable_hash toolchain::stableHashValue(const MachineInstr &MI, bool HashVRegs,
                                  bool HashConstantPoolIndices,
                                  bool HashMemOperands) {
  // Build up a buffer of hash code components.
  SmallVector<stable_hash, 16> HashComponents;
  HashComponents.reserve(MI.getNumOperands() + MI.getNumMemOperands() + 2);
  HashComponents.push_back(MI.getOpcode());
  HashComponents.push_back(MI.getFlags());
  for (const MachineOperand &MO : MI.operands()) {
    if (!HashVRegs && MO.isReg() && MO.isDef() && MO.getReg().isVirtual())
      continue; // Skip virtual register defs.

    if (MO.isCPI()) {
      HashComponents.push_back(stable_hash_combine(
          MO.getType(), MO.getTargetFlags(), MO.getIndex()));
      continue;
    }

    stable_hash StableHash = stableHashValue(MO);
    if (!StableHash)
      return 0;
    HashComponents.push_back(StableHash);
  }

  for (const auto *Op : MI.memoperands()) {
    if (!HashMemOperands)
      break;
    HashComponents.push_back(static_cast<unsigned>(Op->getSize().getValue()));
    HashComponents.push_back(static_cast<unsigned>(Op->getFlags()));
    HashComponents.push_back(static_cast<unsigned>(Op->getOffset()));
    HashComponents.push_back(static_cast<unsigned>(Op->getSuccessOrdering()));
    HashComponents.push_back(static_cast<unsigned>(Op->getAddrSpace()));
    HashComponents.push_back(static_cast<unsigned>(Op->getSyncScopeID()));
    HashComponents.push_back(static_cast<unsigned>(Op->getBaseAlign().value()));
    HashComponents.push_back(static_cast<unsigned>(Op->getFailureOrdering()));
  }

  return stable_hash_combine(HashComponents);
}

stable_hash toolchain::stableHashValue(const MachineBasicBlock &MBB) {
  SmallVector<stable_hash> HashComponents;
  // TODO: Hash more stuff like block alignment and branch probabilities.
  for (const auto &MI : MBB)
    HashComponents.push_back(stableHashValue(MI));
  return stable_hash_combine(HashComponents);
}

stable_hash toolchain::stableHashValue(const MachineFunction &MF) {
  SmallVector<stable_hash> HashComponents;
  // TODO: Hash lots more stuff like function alignment and stack objects.
  for (const auto &MBB : MF)
    HashComponents.push_back(stableHashValue(MBB));
  return stable_hash_combine(HashComponents);
}
