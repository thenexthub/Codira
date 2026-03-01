//===-- BPF.td - Describe the BPF Target Machine -----------*- tablegen -*-===//
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

include "BPFRegisterInfo.td"
include "BPFCallingConv.td"
include "BPFInstrInfo.td"
include "GISel/BPFRegisterBanks.td"


defm : RemapAllTargetPseudoPointerOperands<GPR>;

def BPFInstrInfo : InstrInfo;

class Proc<string Name, list<SubtargetFeature> Features>
 : Processor<Name, NoItineraries, Features>;

def DummyFeature : SubtargetFeature<"dummy", "isDummyMode",
                                    "true", "unused feature">;

def ALU32 : SubtargetFeature<"alu32", "HasAlu32", "true",
                             "Enable ALU32 instructions">;

def DwarfRIS: SubtargetFeature<"dwarfris", "UseDwarfRIS", "true",
                               "Disable MCAsmInfo DwarfUsesRelocationsAcrossSections">;

def MisalignedMemAccess : SubtargetFeature<"allows-misaligned-mem-access",
                                           "AllowsMisalignedMemAccess", "true",
                                           "Allows misaligned memory access">;

def : Proc<"generic", []>;
def : Proc<"v1", []>;
def : Proc<"v2", []>;
def : Proc<"v3", [ALU32]>;
def : Proc<"v4", [ALU32]>;
def : Proc<"probe", []>;

def BPFInstPrinter : AsmWriter {
  string AsmWriterClassName  = "InstPrinter";
  bit isMCAsmWriter = 1;
}

def BPFAsmParser : AsmParser {
  bit HasMnemonicFirst = 0;
}

def BPFAsmParserVariant : AsmParserVariant {
  int Variant = 0;
  string Name = "BPF";
  string BreakCharacters = ".";
  string TokenizingCharacters = "#()[]=:.<>!+*%/";
}

def BPF : Target {
  let InstructionSet = BPFInstrInfo;
  let AssemblyWriters = [BPFInstPrinter];
  let AssemblyParsers = [BPFAsmParser];
  let AssemblyParserVariants = [BPFAsmParserVariant];
}
