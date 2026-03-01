//===-- RISCVFixupKinds.h - RISC-V Specific Fixup Entries -------*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_RISCV_MCTARGETDESC_RISCVFIXUPKINDS_H
#define LLVM_LIB_TARGET_RISCV_MCTARGETDESC_RISCVFIXUPKINDS_H

#include "vm/core/BinaryFormat/ELF.h"
#include "vm/core/MC/MCFixup.h"

#undef RISCV

namespace vm::core::RISCV {
enum Fixups {
  // 20-bit fixup corresponding to %hi(foo) for instructions like lui
  fixup_riscv_hi20 = FirstTargetFixupKind,
  // 12-bit fixup corresponding to %lo(foo) for instructions like addi
  fixup_riscv_lo12_i,
  // 12-bit fixup corresponding to foo-bar for instructions like addi
  fixup_riscv_12_i,
  // 12-bit fixup corresponding to %lo(foo) for the S-type store instructions
  fixup_riscv_lo12_s,
  // 20-bit fixup corresponding to %pcrel_hi(foo) for instructions like auipc
  fixup_riscv_pcrel_hi20,
  // 12-bit fixup corresponding to %pcrel_lo(foo) for instructions like addi
  fixup_riscv_pcrel_lo12_i,
  // 12-bit fixup corresponding to %pcrel_lo(foo) for the S-type store
  // instructions
  fixup_riscv_pcrel_lo12_s,
  // 20-bit fixup for symbol references in the jal instruction
  fixup_riscv_jal,
  // 12-bit fixup for symbol references in the branch instructions
  fixup_riscv_branch,
  // 11-bit fixup for symbol references in the compressed jump instruction
  fixup_riscv_rvc_jump,
  // 8-bit fixup for symbol references in the compressed branch instruction
  fixup_riscv_rvc_branch,
  // 6-bit fixup for symbol references in instructions like c.li
  fixup_riscv_rvc_imm,
  // Fixup representing a legacy no-pic function call attached to the auipc
  // instruction in a pair composed of adjacent auipc+jalr instructions.
  fixup_riscv_call,
  // Fixup representing a function call attached to the auipc instruction in a
  // pair composed of adjacent auipc+jalr instructions.
  fixup_riscv_call_plt,

  // Qualcomm specific fixups
  // 12-bit fixup for symbol references in the 48-bit Xqcibi branch immediate
  // instructions
  fixup_riscv_qc_e_branch,
  // 32-bit fixup for symbol references in the 48-bit qc.e.li instruction
  fixup_riscv_qc_e_32,
  // 20-bit fixup for symbol references in the 32-bit qc.li instruction
  fixup_riscv_qc_abs20_u,
  // 32-bit fixup for symbol references in the 48-bit qc.j/qc.jal instructions
  fixup_riscv_qc_e_call_plt,

  // Andes specific fixups
  // 10-bit fixup for symbol references in the xandesperf branch instruction
  fixup_riscv_nds_branch_10,

  // Used as a sentinel, must be the last
  fixup_riscv_invalid,
  NumTargetFixupKinds = fixup_riscv_invalid - FirstTargetFixupKind
};
} // end namespace vm::core::RISCV

#endif
