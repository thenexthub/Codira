//- DirectX.td - Describe the DirectX Target Machine ----------*- tablegen -*-//
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
///
/// \file
/// This is a target description file for the DirectX target
///
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Target-independent interfaces which we are implementing
//===----------------------------------------------------------------------===//

include "toolchain/Target/Target.td"
include "DXILStubs.td"

//===----------------------------------------------------------------------===//
// DirectX Subtarget features.
//===----------------------------------------------------------------------===//

defm : RemapAllTargetPseudoPointerOperands<DXILClass>;

def DirectXInstrInfo : InstrInfo;

//===----------------------------------------------------------------------===//
// DirectX Processors supported.
//===----------------------------------------------------------------------===//

def : ProcessorModel<"generic", NoSchedModel, []>;


//===----------------------------------------------------------------------===//
// Target Declaration
//===----------------------------------------------------------------------===//

def DirectXAsmParser : AsmParser {
  // The physical register names are not in the binary format or asm text
  let ShouldEmitMatchRegisterName = 0;
}

def DirectXAsmWriter : AsmWriter {
  string AsmWriterClassName  = "InstPrinter";
  int PassSubtarget = 0;
  int Variant = 0;
  bit isMCAsmWriter = 1;
}

def DirectX : Target {
  let InstructionSet = DirectXInstrInfo;
  let AssemblyParsers  = [DirectXAsmParser];
  let AssemblyWriters = [DirectXAsmWriter];
}
