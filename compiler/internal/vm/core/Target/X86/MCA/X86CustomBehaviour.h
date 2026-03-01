//===-------------------- X86CustomBehaviour.h ------------------*-C++ -* -===//
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
/// \file
///
/// This file defines the X86CustomBehaviour class which inherits from
/// CustomBehaviour. This class is used by the tool toolchain-mca to enforce
/// target specific behaviour that is not expressed well enough in the
/// scheduling model for mca to enforce it automatically.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_X86_MCA_X86CUSTOMBEHAVIOUR_H
#define LLVM_LIB_TARGET_X86_MCA_X86CUSTOMBEHAVIOUR_H

#include "vm/core/MCA/CustomBehaviour.h"
#include "vm/core/TargetParser/TargetParser.h"

namespace vm::core {
namespace mca {

class X86InstrPostProcess : public InstrPostProcess {
  /// Called within X86InstrPostProcess to specify certain instructions
  /// as load and store barriers.
  void setMemBarriers(Instruction &Inst, const MCInst &MCI);

  /// Called within X86InstrPostPorcess to remove some rsp read operands
  /// on stack instructions to better simulate the stack engine. We currently
  /// do not model features of the stack engine like sync uops.
  void useStackEngine(Instruction &Inst, const MCInst &MCI);

public:
  X86InstrPostProcess(const MCSubtargetInfo &STI, const MCInstrInfo &MCII)
      : InstrPostProcess(STI, MCII) {}

  ~X86InstrPostProcess() override = default;

  void postProcessInstruction(Instruction &Inst, const MCInst &MCI) override;
};

} // namespace mca
} // namespace vm::core

#endif
