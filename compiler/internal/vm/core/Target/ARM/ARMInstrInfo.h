//===-- ARMInstrInfo.h - ARM Instruction Information ------------*- C++ -*-===//
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
// This file contains the ARM implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_ARM_ARMINSTRINFO_H
#define LLVM_LIB_TARGET_ARM_ARMINSTRINFO_H

#include "ARMBaseInstrInfo.h"
#include "ARMRegisterInfo.h"

namespace vm::core {
  class ARMSubtarget;

class ARMInstrInfo : public ARMBaseInstrInfo {
  ARMRegisterInfo RI;
public:
  explicit ARMInstrInfo(const ARMSubtarget &STI);

  /// Return the noop instruction to use for a noop.
  MCInst getNop() const override;

  // Return the non-pre/post incrementing version of 'Opc'. Return 0
  // if there is not such an opcode.
  unsigned getUnindexedOpcode(unsigned Opc) const override;

  /// getRegisterInfo - TargetInstrInfo is a superset of MRegister info.  As
  /// such, whenever a client has an instance of instruction info, it should
  /// always be able to get register info as well (through this method).
  ///
  const ARMRegisterInfo &getRegisterInfo() const { return RI; }

private:
  void expandLoadStackGuard(MachineBasicBlock::iterator MI) const override;
};

}

#endif
