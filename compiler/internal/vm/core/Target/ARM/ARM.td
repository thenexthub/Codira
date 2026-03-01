//===-- ARM.td - Describe the ARM Target Machine -----------*- tablegen -*-===//
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

include "ARMFeatures.td"
include "ARMArchitectures.td"

//===----------------------------------------------------------------------===//
// Register File Description
//===----------------------------------------------------------------------===//

include "ARMRegisterInfo.td"
include "ARMRegisterBanks.td"
include "ARMCallingConv.td"

//===----------------------------------------------------------------------===//
// ARM schedules.
//===----------------------------------------------------------------------===//
//
include "ARMPredicates.td"
include "ARMSchedule.td"

//===----------------------------------------------------------------------===//
// Instruction Descriptions
//===----------------------------------------------------------------------===//

include "ARMInstrInfo.td"

def Thumb1OnlyMode : HwMode<[IsThumb1Only]>;
def arm_ptr_rc : RegClassByHwMode<
  [DefaultMode, Thumb1OnlyMode],
  [GPR, tGPR]>;

defm : RemapAllTargetPseudoPointerOperands<arm_ptr_rc>;

def ARMInstrInfo : InstrInfo;

//===----------------------------------------------------------------------===//
// ARM schedules
//
include "ARMScheduleV6.td"
include "ARMScheduleA8.td"
include "ARMScheduleA9.td"
include "ARMScheduleSwift.td"
include "ARMScheduleR52.td"
include "ARMScheduleA57.td"
include "ARMScheduleM4.td"
include "ARMScheduleM55.td"
include "ARMScheduleM7.td"
include "ARMScheduleM85.td"

include "ARMProcessors.td"

//===----------------------------------------------------------------------===//
// Declare the target which we are implementing
//===----------------------------------------------------------------------===//

def ARMAsmWriter : AsmWriter {
  string AsmWriterClassName  = "InstPrinter";
  int PassSubtarget = 1;
  int Variant = 0;
  bit isMCAsmWriter = 1;
}

def ARMAsmParser : AsmParser {
  bit ReportMultipleNearMisses = 1;
  let PreferSmallerInstructions = true;
}

def ARMAsmParserVariant : AsmParserVariant {
  int Variant = 0;
  string Name = "ARM";
  string BreakCharacters = ".";
}

def ARM : Target {
  // Pull in Instruction Info.
  let InstructionSet = ARMInstrInfo;
  let AssemblyWriters = [ARMAsmWriter];
  let AssemblyParsers = [ARMAsmParser];
  let AssemblyParserVariants = [ARMAsmParserVariant];
  let AllowRegisterRenaming = 1;
}
