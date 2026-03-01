//===-- MSP430.td - Describe the MSP430 Target Machine -----*- tablegen -*-===//
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
// This is the top level entry point for the MSP430 target.
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Target-independent interfaces
//===----------------------------------------------------------------------===//

include "toolchain/Target/Target.td"

//===----------------------------------------------------------------------===//
// Subtarget Features. 
//===----------------------------------------------------------------------===//
def FeatureX
 : SubtargetFeature<"ext", "ExtendedInsts", "true",
                    "Enable MSP430-X extensions">;

def FeatureHWMult16
 : SubtargetFeature<"hwmult16", "HWMultMode", "HWMult16",
                    "Enable 16-bit hardware multiplier">;

def FeatureHWMult32
 : SubtargetFeature<"hwmult32", "HWMultMode", "HWMult32",
                    "Enable 32-bit hardware multiplier">;

def FeatureHWMultF5
 : SubtargetFeature<"hwmultf5", "HWMultMode", "HWMultF5",
                    "Enable F5 series hardware multiplier">;

//===----------------------------------------------------------------------===//
// MSP430 supported processors.
//===----------------------------------------------------------------------===//
class Proc<string Name, list<SubtargetFeature> Features>
 : Processor<Name, NoItineraries, Features>;

def : Proc<"generic",         []>;
def : Proc<"msp430",          []>;
def : Proc<"msp430x",         [FeatureX]>;

//===----------------------------------------------------------------------===//
// Register File Description
//===----------------------------------------------------------------------===//

include "MSP430RegisterInfo.td"

//===----------------------------------------------------------------------===//
// Calling Convention Description
//===----------------------------------------------------------------------===//

include "MSP430CallingConv.td"

//===----------------------------------------------------------------------===//
// Instruction Descriptions
//===----------------------------------------------------------------------===//

include "MSP430InstrInfo.td"

defm : RemapAllTargetPseudoPointerOperands<GR16>;

def MSP430InstrInfo : InstrInfo;

//===---------------------------------------------------------------------===//
// Assembly Printers
//===---------------------------------------------------------------------===//

def MSP430AsmWriter : AsmWriter {
  string AsmWriterClassName = "InstPrinter";
}

//===---------------------------------------------------------------------===//
// Assembly Parsers
//===---------------------------------------------------------------------===//

def MSP430AsmParser : AsmParser {
  let AllowDuplicateRegisterNames = 1;
  let ShouldEmitMatchRegisterAltName = 1;
}

//===----------------------------------------------------------------------===//
// Target Declaration
//===----------------------------------------------------------------------===//

def MSP430 : Target {
  let InstructionSet = MSP430InstrInfo;
  let AssemblyParsers = [MSP430AsmParser];
}

