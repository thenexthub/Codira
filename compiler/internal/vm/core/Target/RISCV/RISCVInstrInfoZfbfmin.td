//===-- RISCVInstrInfoZfbfmin.td - 'Zfbfmin' instructions --*- tablegen -*-===//
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
// This file describes the RISC-V instructions from the standard 'Zfbfmin'
// extension, providing scalar conversion instructions for BFloat16.
// This version is still experimental as the 'Zfbfmin' extension hasn't been
// ratified yet.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Instructions
//===----------------------------------------------------------------------===//

let Predicates = [HasStdExtZfbfmin] in {
def FCVT_BF16_S : FPUnaryOp_r_frm<0b0100010, 0b01000, FPR16, FPR32, "fcvt.bf16.s">,
                  Sched<[WriteFCvtF32ToF16, ReadFCvtF32ToF16]>;
def FCVT_S_BF16 : FPUnaryOp_r_frmlegacy<0b0100000, 0b00110, FPR32, FPR16, "fcvt.s.bf16">,
                  Sched<[WriteFCvtF16ToF32, ReadFCvtF16ToF32]>;
} // Predicates = [HasStdExtZfbfmin]

//===----------------------------------------------------------------------===//
// Pseudo-instructions and codegen patterns
//===----------------------------------------------------------------------===//

let Predicates = [HasStdExtZfbfmin] in {
def : Pat<(riscv_selectcc (XLenVT GPR:$lhs), GPR:$rhs, cond:$cc,
                          (bf16 FPR16:$truev), FPR16:$falsev),
          (Select_FPR16_Using_CC_GPR GPR:$lhs, GPR:$rhs,
           (CCtoRISCVCC $cc), FPR16:$truev, FPR16:$falsev)>;

// Explicitly select 0 in the condition to X0. The register coalescer doesn't
// always do it.
def : Pat<(riscv_selectcc (XLenVT GPR:$lhs), 0, cond:$cc,
                          (bf16 FPR16:$truev), FPR16:$falsev),
          (Select_FPR16_Using_CC_GPR GPR:$lhs, (XLenVT X0),
           (CCtoRISCVCC $cc), FPR16:$truev, FPR16:$falsev)>;

/// Loads
def : LdPat<load, FLH, bf16>;

/// Stores
def : StPat<store, FSH, FPR16, bf16>;

/// Float conversion operations
// f32 -> bf16, bf16 -> f32
def : Pat<(bf16 (fpround FPR32:$rs1)),
          (FCVT_BF16_S FPR32:$rs1, FRM_DYN)>;
def : Pat<(fpextend (bf16 FPR16:$rs1)),
          (FCVT_S_BF16 FPR16:$rs1, FRM_RNE)>;

// Moves (no conversion)
def : Pat<(bf16 (riscv_fmv_h_x GPR:$src)), (FMV_H_X GPR:$src)>;
def : Pat<(riscv_fmv_x_anyexth (bf16 FPR16:$src)), (FMV_X_H FPR16:$src)>;
def : Pat<(riscv_fmv_x_signexth (bf16 FPR16:$src)), (FMV_X_H FPR16:$src)>;
} // Predicates = [HasStdExtZfbfmin]

let Predicates = [HasStdExtZfbfmin, HasStdExtD] in {
// f64 -> bf16
// FIXME: This pattern double rounds.
def : Pat<(bf16 (fpround FPR64:$rs1)),
          (FCVT_BF16_S (FCVT_S_D FPR64:$rs1, FRM_DYN), FRM_DYN)>;
// bf16 -> f64
def : Pat<(fpextend (bf16 FPR16:$rs1)),
          (FCVT_D_S (FCVT_S_BF16 FPR16:$rs1, FRM_DYN), FRM_RNE)>;
}

let Predicates = [HasStdExtZfhmin, HasStdExtZfbfmin] in {
// bf16 -> f16
def : Pat<(f16 (bitconvert (bf16 FPR16:$src))), (f16 FPR16:$src)>;
// f16 -> bf16
def : Pat<(bf16 (bitconvert (f16 FPR16:$src))), (bf16 FPR16:$src)>;
}
