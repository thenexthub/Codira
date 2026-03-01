//===-- RISCV.td - Describe the RISC-V Target Machine ------*- tablegen -*-===//
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

include "toolchain/Target/Target.td"

//===----------------------------------------------------------------------===//
// RISC-V subtarget features and instruction predicates.
//===----------------------------------------------------------------------===//

include "RISCVFeatures.td"

//===----------------------------------------------------------------------===//
// RISC-V profiles supported.
//===----------------------------------------------------------------------===//

include "RISCVProfiles.td"

//===----------------------------------------------------------------------===//
// Named operands for CSR instructions.
//===----------------------------------------------------------------------===//

include "RISCVSystemOperands.td"

//===----------------------------------------------------------------------===//
// Registers, calling conventions, instruction descriptions.
//===----------------------------------------------------------------------===//

include "RISCVRegisterInfo.td"
include "RISCVSchedule.td"
include "RISCVCallingConv.td"
include "RISCVInstrInfo.td"
include "GISel/RISCVRegisterBanks.td"

//===----------------------------------------------------------------------===//
// Instruction predicates
//===----------------------------------------------------------------------===//

include "RISCVInstrPredicates.td"

//===----------------------------------------------------------------------===//
// RISC-V macro fusions.
//===----------------------------------------------------------------------===//

include "RISCVMacroFusion.td"

//===----------------------------------------------------------------------===//
// RISC-V Scheduling Models
//===----------------------------------------------------------------------===//
include "RISCVSchedAndes45.td"
include "RISCVSchedGenericOOO.td"
include "RISCVSchedMIPSP8700.td"
include "RISCVSchedRocket.td"
include "RISCVSchedSiFive7.td"
include "RISCVSchedSiFiveP400.td"
include "RISCVSchedSiFiveP500.td"
include "RISCVSchedSiFiveP600.td"
include "RISCVSchedSiFiveP800.td"
include "RISCVSchedSpacemitX60.td"
include "RISCVSchedSyntacoreSCR1.td"
include "RISCVSchedSyntacoreSCR345.td"
include "RISCVSchedSyntacoreSCR7.td"
include "RISCVSchedTTAscalonD8.td"
include "RISCVSchedXiangShanNanHu.td"

//===----------------------------------------------------------------------===//
// RISC-V processors supported.
//===----------------------------------------------------------------------===//

include "RISCVProcessors.td"

//===----------------------------------------------------------------------===//
// Pfm Counters
//===----------------------------------------------------------------------===//

include "RISCVPfmCounters.td"

//===----------------------------------------------------------------------===//
// Define the RISC-V target.
//===----------------------------------------------------------------------===//

def RISCVInstrInfo : InstrInfo {
  let guessInstructionProperties = 0;
}

def RISCVAsmParser : AsmParser {
  let ShouldEmitMatchRegisterAltName = 1;
  let AllowDuplicateRegisterNames = 1;
}

def RISCVAsmWriter : AsmWriter {
  int PassSubtarget = 1;
}

defm : RemapAllTargetPseudoPointerOperands<GPR>;

def RISCV : Target {
  let InstructionSet = RISCVInstrInfo;
  let AssemblyParsers = [RISCVAsmParser];
  let AssemblyWriters = [RISCVAsmWriter];
  let AllowRegisterRenaming = 1;
}
