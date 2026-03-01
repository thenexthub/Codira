//===- Xtensa.td - Describe the Xtensa Target Machine ------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
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
// Target-independent interfaces
//===----------------------------------------------------------------------===//

include "toolchain/Target/Target.td"

//===----------------------------------------------------------------------===//
// Subtarget Features.
//===----------------------------------------------------------------------===//

include "XtensaFeatures.td"

//===----------------------------------------------------------------------===//
// Xtensa supported processors.
//===----------------------------------------------------------------------===//

include "XtensaProcessors.td"

//===----------------------------------------------------------------------===//
// Register File Description
//===----------------------------------------------------------------------===//

include "XtensaRegisterInfo.td"

//===----------------------------------------------------------------------===//
// Calling Convention Description
//===----------------------------------------------------------------------===//

include "XtensaCallingConv.td"

//===----------------------------------------------------------------------===//
// Instruction Descriptions
//===----------------------------------------------------------------------===//

include "XtensaInstrInfo.td"

defm : RemapAllTargetPseudoPointerOperands<AR>;

def XtensaInstrInfo : InstrInfo;

//===----------------------------------------------------------------------===//
// Target Declaration
//===----------------------------------------------------------------------===//

def XtensaAsmParser : AsmParser {
  let ShouldEmitMatchRegisterAltName = 1;
}

def XtensaInstPrinter : AsmWriter {
  string AsmWriterClassName  = "InstPrinter";
}

def Xtensa : Target {
  let InstructionSet = XtensaInstrInfo;
  let AssemblyWriters = [XtensaInstPrinter];
  let AssemblyParsers = [XtensaAsmParser];
}

