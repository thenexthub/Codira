//===-- RISCVInstrInfoZibi.td - 'Zibi' instructions --------*- tablegen -*-===//
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
/// This file describes the RISC-V instructions for 'Zibi' (branch with imm).
///
//===----------------------------------------------------------------------===//

// A 5-bit unsigned immediate representing 1-31 and -1. 00000 represents -1.
def imm5_zibi : RISCVOp<XLenVT>, ImmLeaf<XLenVT, [{
    return (Imm != 0 && isUInt<5>(Imm)) || Imm == -1;
}]> {
  let ParserMatchClass = ImmAsmOperand<"", 5, "Zibi">;
  let EncoderMethod = "getImmOpValueZibi";
  let DecoderMethod = "decodeImmZibiOperand";
  let MCOperandPredicate = [{
    int64_t Imm;
    if (!MCOp.evaluateAsConstantImm(Imm))
      return false;
    return (Imm >= 1 && Imm <= 31) || Imm == -1;
  }];
  let OperandType = "OPERAND_IMM5_ZIBI";
}

class Branch_imm<bits<3> funct3, string opcodestr>
    : RVInstBIMM<funct3, OPC_BRANCH, (outs),
                 (ins GPR:$rs1, imm5_zibi:$cimm, bare_simm13_lsb0:$imm12),
                 opcodestr, "$rs1, $cimm, $imm12">,
      Sched<[WriteJmp, ReadJmp]> {
  let isBranch = 1;
  let isTerminator = 1;
  let hasSideEffects = 0;
  let mayLoad = 0;
  let mayStore = 0;
}

let Predicates = [HasStdExtZibi] in {
  def BEQI : Branch_imm<0b010, "beqi">;
  def BNEI : Branch_imm<0b011, "bnei">;
} // Predicates = [HasStdExtZibi]

multiclass BccImmPat<CondCode Cond, Branch_imm Inst> {
  def : Pat<(riscv_brcc (XLenVT GPR:$rs1), imm5_zibi:$cimm, Cond, bb:$imm12),
            (Inst GPR:$rs1, imm5_zibi:$cimm, bare_simm13_lsb0_bb:$imm12)>;
}

defm CC_Imm5_Zibi : SelectCC_GPR_riirr<GPR, imm5_zibi>;

class SelectZibi<CondCode Cond>
    : Pat<(riscv_selectcc (XLenVT GPR:$lhs), imm5_zibi:$cimm, Cond:$cc,
                          (XLenVT GPR:$truev), GPR:$falsev),
          (Select_GPR_Using_CC_Imm5_Zibi GPR:$lhs, imm5_zibi:$cimm,
              (CCtoRISCVCC $cc), GPR:$truev, GPR:$falsev)>;

let Predicates = [HasStdExtZibi] in {
  def : SelectZibi<SETEQ>;
  def : SelectZibi<SETNE>;

  defm : BccImmPat<SETEQ, BEQI>;
  defm : BccImmPat<SETNE, BNEI>;
} // Predicates = [HasStdExtZibi]
