//===-- VE.td - Describe the VE Target Machine -------------*- tablegen -*-===//
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
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Target-independent interfaces which we are implementing
//===----------------------------------------------------------------------===//

include "toolchain/Target/Target.td"

//===----------------------------------------------------------------------===//
// VE Subtarget features.
//
def FeatureEnableVPU
  : SubtargetFeature<"vpu", "EnableVPU", "true",
                     "Enable the VPU">;

//===----------------------------------------------------------------------===//
// Register File, Calling Conv, Instruction Descriptions
//===----------------------------------------------------------------------===//

include "VERegisterInfo.td"
include "VECallingConv.td"
include "VEInstrInfo.td"

defm : RemapAllTargetPseudoPointerOperands<ve_ptr_rc>;
def VEInstrInfo : InstrInfo {}

def VEAsmParser : AsmParser {
  // Use both VE register name matcher to accept "S0~S63" register names
  // and default register matcher to accept other registeres.
  let AllowDuplicateRegisterNames = 1;
  let ShouldEmitMatchRegisterAltName = 1;
}

//===----------------------------------------------------------------------===//
// VE processors supported.
//===----------------------------------------------------------------------===//

class Proc<string Name, list<SubtargetFeature> Features>
 : Processor<Name, NoItineraries, Features>;

def : Proc<"generic", [FeatureEnableVPU]>;

//===----------------------------------------------------------------------===//
// Declare the target which we are implementing
//===----------------------------------------------------------------------===//

def VEAsmWriter : AsmWriter {
  string AsmWriterClassName  = "InstPrinter";
  int PassSubtarget = 1;
  int Variant = 0;
}

def VE : Target {
  // Pull in Instruction Info:
  let InstructionSet = VEInstrInfo;
  let AssemblyParsers = [VEAsmParser];
  let AssemblyWriters = [VEAsmWriter];
  let AllowRegisterRenaming = 1;
}
