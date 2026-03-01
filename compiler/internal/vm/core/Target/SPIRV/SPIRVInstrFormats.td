//===-- SPIRVInstrFormats.td - SPIR-V Instruction Formats --*- tablegen -*-===//
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

def StringImm: Operand<i32>{
  let PrintMethod="printStringImm";
}

class Op<bits<16> Opcode, dag outs, dag ins, string asmstr, list<dag> pattern = []>
  : Instruction {
  field bits<16> Inst;

  let Inst = Opcode;

  let Namespace = "SPIRV";
  let DecoderNamespace = "SPIRV";

  dag OutOperandList = outs;
  dag InOperandList = ins;
  let AsmString = asmstr;
  let Pattern = pattern;
}

class UnknownOp<dag outs, dag ins, string asmstr, list<dag> pattern = []>
    : Op<0, outs, ins, asmstr, pattern> {
  let isPseudo = 1;
}

// Pseudo instructions
class Pseudo<dag outs, dag ins> : Op<0, outs, ins, ""> {
  let isPseudo = 1;
  let hasSideEffects = 0;
}
