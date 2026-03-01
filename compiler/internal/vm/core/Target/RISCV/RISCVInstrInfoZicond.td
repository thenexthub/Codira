//===-- RISCVInstrInfoZicond.td ----------------------------*- tablegen -*-===//
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
// This file describes the RISC-V instructions from the standard Integer
// Conditional operations extension (Zicond).
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// RISC-V specific DAG Nodes.
//===----------------------------------------------------------------------===//

// Branchless select operations, matching the semantics of the instructions
// defined in Zicond or XVentanaCondOps
def riscv_czero_eqz : RVSDNode<"CZERO_EQZ", SDTIntBinOp>; // vt.maskc for XVentanaCondOps.
def riscv_czero_nez : RVSDNode<"CZERO_NEZ", SDTIntBinOp>; // vt.maskcn for XVentanaCondOps.

//===----------------------------------------------------------------------===//
// Instructions
//===----------------------------------------------------------------------===//

let Predicates = [HasStdExtZicond] in {
def CZERO_EQZ : ALU_rr<0b0000111, 0b101, "czero.eqz">,
                Sched<[WriteIALU, ReadIALU, ReadIALU]>;
def CZERO_NEZ : ALU_rr<0b0000111, 0b111, "czero.nez">,
                Sched<[WriteIALU, ReadIALU, ReadIALU]>;
} // Predicates = [HasStdExtZicond]

//===----------------------------------------------------------------------===//
// Pseudo-instructions and codegen patterns
//===----------------------------------------------------------------------===//

let Predicates = [HasStdExtZicond] in {
def : Pat<(XLenVT (riscv_czero_eqz GPR:$rs1, GPR:$rc)),
          (CZERO_EQZ GPR:$rs1, GPR:$rc)>;
def : Pat<(XLenVT (riscv_czero_nez GPR:$rs1, GPR:$rc)),
          (CZERO_NEZ GPR:$rs1, GPR:$rc)>;

def : Pat<(XLenVT (riscv_czero_eqz GPR:$rs1, (riscv_setne (XLenVT GPR:$rc)))),
          (CZERO_EQZ GPR:$rs1, GPR:$rc)>;
def : Pat<(XLenVT (riscv_czero_eqz GPR:$rs1, (riscv_seteq (XLenVT GPR:$rc)))),
          (CZERO_NEZ GPR:$rs1, GPR:$rc)>;
def : Pat<(XLenVT (riscv_czero_nez GPR:$rs1, (riscv_setne (XLenVT GPR:$rc)))),
          (CZERO_NEZ GPR:$rs1, GPR:$rc)>;
def : Pat<(XLenVT (riscv_czero_nez GPR:$rs1, (riscv_seteq (XLenVT GPR:$rc)))),
          (CZERO_EQZ GPR:$rs1, GPR:$rc)>;
} // Predicates = [HasStdExtZicond]
