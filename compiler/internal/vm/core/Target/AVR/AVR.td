//===-- AVR.td - Describe the AVR Target Machine ----------*- tablegen -*-===//
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
//===---------------------------------------------------------------------===//
// This is the top level entry point for the AVR target.
//===---------------------------------------------------------------------===//

//===---------------------------------------------------------------------===//
// Target-independent interfaces which we are implementing
//===---------------------------------------------------------------------===//

include "toolchain/Target/Target.td"

//===---------------------------------------------------------------------===//
// AVR Device Definitions
//===---------------------------------------------------------------------===//

include "AVRDevices.td"

//===---------------------------------------------------------------------===//
// Register File Description
//===---------------------------------------------------------------------===//

include "AVRRegisterInfo.td"

//===---------------------------------------------------------------------===//
// Instruction Descriptions
//===---------------------------------------------------------------------===//

include "AVRInstrInfo.td"

defm : RemapAllTargetPseudoPointerOperands<PTRDISPREGS>;

def AVRInstrInfo : InstrInfo;

//===---------------------------------------------------------------------===//
// Calling Conventions
//===---------------------------------------------------------------------===//

include "AVRCallingConv.td"

//===---------------------------------------------------------------------===//
// Assembly Printers
//===---------------------------------------------------------------------===//

def AVRAsmWriter : AsmWriter {
  string AsmWriterClassName = "InstPrinter";
  bit isMCAsmWriter = 1;
}

//===---------------------------------------------------------------------===//
// Assembly Parsers
//===---------------------------------------------------------------------===//

def AVRAsmParser : AsmParser {
  let ShouldEmitMatchRegisterName = 1;
  let ShouldEmitMatchRegisterAltName = 1;
}

def AVRAsmParserVariant : AsmParserVariant {
  int Variant = 0;

  // Recognize hard coded registers.
  string RegisterPrefix = "$";
  string TokenizingCharacters = "+";
}

//===---------------------------------------------------------------------===//
// Target Declaration
//===---------------------------------------------------------------------===//

def AVR : Target {
  let InstructionSet = AVRInstrInfo;
  let AssemblyWriters = [AVRAsmWriter];

  let AssemblyParsers = [AVRAsmParser];
  let AssemblyParserVariants = [AVRAsmParserVariant];
}
