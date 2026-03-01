//===- Lanai.td - Describe the Lanai Target Machine --------*- tablegen -*-===//
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

//===----------------------------------------------------------------------===//
// Target-independent interfaces which we are implementing
//===----------------------------------------------------------------------===//

include "toolchain/Target/Target.td"

//===----------------------------------------------------------------------===//
// Register File, Calling Conv, Instruction Descriptions
//===----------------------------------------------------------------------===//

include "LanaiSchedule.td"
include "LanaiRegisterInfo.td"
include "LanaiCallingConv.td"
include "LanaiInstrInfo.td"

defm : RemapAllTargetPseudoPointerOperands<GPR>;

def LanaiInstrInfo : InstrInfo;

//===----------------------------------------------------------------------===//
// Lanai processors supported.
//===----------------------------------------------------------------------===//

def : ProcessorModel<"generic", LanaiSchedModel, []>;
def : ProcessorModel<"v11", LanaiSchedModel, []>;

def LanaiInstPrinter : AsmWriter {
  string AsmWriterClassName  = "InstPrinter";
  bit isMCAsmWriter = 1;
}

//===----------------------------------------------------------------------===//
// Declare the target which we are implementing
//===----------------------------------------------------------------------===//

def Lanai : Target {
  // Pull in Instruction Info:
  let InstructionSet = LanaiInstrInfo;
  let AssemblyWriters = [LanaiInstPrinter];
}
