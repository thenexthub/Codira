//===-- RISCVInstrInfoZimop.td -----------------------------*- tablegen -*-===//
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
// This file describes the RISC-V instructions from the standard
// May-Be-Operations Extension (Zimop).
//
//===----------------------------------------------------------------------===//

class RVInstIMopr<bits<7> imm7, bits<5> imm5, bits<3> funct3, RISCVOpcode opcode,
                   dag outs, dag ins, string opcodestr, string argstr>
    : RVInstIBase<funct3, opcode, outs, ins, opcodestr, argstr> {
  let Inst{31} = imm7{6};
  let Inst{30} = imm5{4};
  let Inst{29-28} = imm7{5-4};
  let Inst{27-26} = imm5{3-2};
  let Inst{25-22} = imm7{3-0};
  let Inst{21-20} = imm5{1-0};
}

class RVInstRMoprr<bits<4> imm4, bits<3> imm3, bits<3> funct3, RISCVOpcode opcode,
                   dag outs, dag ins, string opcodestr, string argstr>
    : RVInstRBase<funct3, opcode, outs, ins, opcodestr, argstr> {
  let Inst{31} = imm4{3};
  let Inst{30} = imm3{2};
  let Inst{29-28} = imm4{2-1};
  let Inst{27-26} = imm3{1-0};
  let Inst{25} = imm4{0};
}

// May-Be-Operations
def riscv_mop_r  : RVSDNode<"MOP_R",
                            SDTypeProfile<1, 2, [SDTCisInt<0>, SDTCisSameAs<0, 1>,
                                                 SDTCisSameAs<0, 2>]>>;
def riscv_mop_rr : RVSDNode<"MOP_RR",
                            SDTypeProfile<1, 3, [SDTCisInt<0>, SDTCisSameAs<0, 1>,
                                                 SDTCisSameAs<0, 2>,
                                                 SDTCisSameAs<0, 3>]>>;

let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
class RVMopr<bits<7> imm7, bits<5> imm5, bits<3> funct3,
             RISCVOpcode opcode, string opcodestr>
    : RVInstIMopr<imm7, imm5, funct3, opcode, (outs GPR:$rd), (ins GPR:$rs1),
                   opcodestr, "$rd, $rs1">;

let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
class RVMoprr<bits<4> imm4, bits<3> imm3, bits<3> funct3,
             RISCVOpcode opcode, string opcodestr>
    : RVInstRMoprr<imm4, imm3, funct3, opcode, (outs GPR:$rd),
                   (ins GPR:$rs1, GPR:$rs2),
                   opcodestr, "$rd, $rs1, $rs2">;

foreach i = 0...31 in {
  let Predicates = [HasStdExtZimop] in
  def MOP_R_#i : RVMopr<0b1000111, i, 0b100, OPC_SYSTEM, "mop.r."#i>,
                 Sched<[]>;
}

foreach i = 0...7 in {
  let Predicates = [HasStdExtZimop] in
  def MOP_RR_#i : RVMoprr<0b1001, i, 0b100, OPC_SYSTEM, "mop.rr."#i>,
                Sched<[]>;
}

let Predicates = [HasStdExtZimop] in {
// Zimop instructions
foreach i = 0...31 in {
  def : Pat<(XLenVT (riscv_mop_r GPR:$rs1, (XLenVT i))),
            (!cast<Instruction>("MOP_R_"#i) GPR:$rs1)>;
}

foreach i = 0...7 in {
  def : Pat<(XLenVT (riscv_mop_rr GPR:$rs1, GPR:$rs2, (XLenVT i))),
            (!cast<Instruction>("MOP_RR_"#i) GPR:$rs1, GPR:$rs2)>;
}

} // Predicates = [HasStdExtZimop]
