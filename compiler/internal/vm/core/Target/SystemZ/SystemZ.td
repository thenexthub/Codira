//===-- SystemZ.td - Describe the SystemZ target machine -----*- tblgen -*-===//
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
// SystemZ subtarget features
//===----------------------------------------------------------------------===//

include "SystemZFeatures.td"

//===----------------------------------------------------------------------===//
// SystemZ subtarget scheduling models
//===----------------------------------------------------------------------===//

include "SystemZSchedule.td"

//===----------------------------------------------------------------------===//
// SystemZ supported processors
//===----------------------------------------------------------------------===//

include "SystemZProcessors.td"

//===----------------------------------------------------------------------===//
// Register file description
//===----------------------------------------------------------------------===//

include "SystemZRegisterInfo.td"

//===----------------------------------------------------------------------===//
// Calling convention description
//===----------------------------------------------------------------------===//

include "SystemZCallingConv.td"

//===----------------------------------------------------------------------===//
// Instruction descriptions
//===----------------------------------------------------------------------===//

include "SystemZOperators.td"
include "SystemZOperands.td"
include "SystemZPatterns.td"
include "SystemZInstrFormats.td"
include "SystemZInstrInfo.td"
include "SystemZInstrVector.td"
include "SystemZInstrFP.td"
include "SystemZInstrHFP.td"
include "SystemZInstrDFP.td"
include "SystemZInstrSystem.td"


defm : RemapAllTargetPseudoPointerOperands<ADDR64Bit>;

def SystemZInstrInfo : InstrInfo { let guessInstructionProperties = 0; }

//===----------------------------------------------------------------------===//
// Assembly parser
//===----------------------------------------------------------------------===//

def SystemZAsmParser : AsmParser {
  let ShouldEmitMatchRegisterName = 0;
}

def GNUAsmParserVariant : AsmParserVariant {
  int Variant = 0;

  // Variant name.
  string Name = "gnu";
}

def HLASMAsmParserVariant : AsmParserVariant {
  int Variant = 1;

  // Variant name.
  string Name = "hlasm";
}

//===----------------------------------------------------------------------===//
// Assembly writer
//===----------------------------------------------------------------------===//

// The SystemZ target supports two different syntaxes for emitting machine code.
def GNUAsmWriter : AsmWriter {
  string AsmWriterClassName  = "GNUInstPrinter";
  int Variant = 0;
}
def HLASMAsmWriter : AsmWriter {
  string AsmWriterClassName  = "HLASMInstPrinter";
  int Variant = 1;
}

//===----------------------------------------------------------------------===//
// Top-level target declaration
//===----------------------------------------------------------------------===//

def SystemZ : Target {
  let InstructionSet = SystemZInstrInfo;
  let AssemblyParsers = [SystemZAsmParser];
  let AssemblyParserVariants = [GNUAsmParserVariant, HLASMAsmParserVariant];
  let AssemblyWriters = [GNUAsmWriter, HLASMAsmWriter];
  let AllowRegisterRenaming = 1;
}
