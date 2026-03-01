//===--- HexagonOperands.td -----------------------------------------------===//
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

def f32ImmOperand : AsmOperandClass { let Name = "f32Imm"; }
def f32Imm : Operand<f32> { let ParserMatchClass = f32ImmOperand; }
def f64ImmOperand : AsmOperandClass { let Name = "f64Imm"; }
def f64Imm : Operand<f64> { let ParserMatchClass = f64ImmOperand; }
def s8_0Imm64Pred  : PatLeaf<(i64 imm), [{ return isInt<8>(N->getSExtValue()); }]>;
def s9_0ImmOperand : AsmOperandClass { let Name = "s9_0Imm"; }
def s9_0Imm : Operand<i32> { let ParserMatchClass = s9_0ImmOperand; }
def s27_2ImmOperand : AsmOperandClass { let Name = "s27_2Imm"; let RenderMethod = "addSignedImmOperands"; }
def s27_2Imm : Operand<i32> { let ParserMatchClass = s27_2ImmOperand; }
def r32_0ImmPred  : PatLeaf<(i32 imm), [{
  int64_t v = (int64_t)N->getSExtValue();
  return isInt<32>(v);
}]>;
def u9_0ImmPred  : PatLeaf<(i32 imm), [{
  int64_t v = (int64_t)N->getSExtValue();
  return isUInt<9>(v);
}]>;

def u64_0ImmOperand : AsmOperandClass { let Name = "u64_0Imm"; let RenderMethod = "addImmOperands"; }
def u64_0Imm : Operand<i64> { let ParserMatchClass = u64_0ImmOperand; }
def n1ConstOperand : AsmOperandClass { let Name = "n1Const"; }
def n1Const : Operand<i32> {
  let ParserMatchClass = n1ConstOperand;
  let DecoderMethod = "n1ConstDecoder";
}
def sgp10ConstOperand : AsmOperandClass { let Name = "sgp10Const"; }
def sgp10Const : Operand<i32> {
  let ParserMatchClass = sgp10ConstOperand;
  let DecoderMethod = "sgp10ConstDecoder";
}

def bblabel : Operand<i32>;
def bbl     : SDNode<"ISD::BasicBlock", SDTPtrLeaf, [], "BasicBlockSDNode">;
