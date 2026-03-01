/**
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PANDA_RUNTIME_FIBERS_ARCH_AARCH64_CONTEXT_LAYOUT_H
#define PANDA_RUNTIME_FIBERS_ARCH_AARCH64_CONTEXT_LAYOUT_H

// NOLINTBEGIN(cppcoreguidelines-macro-usage)

/**
 * Memory layout of the saved context:
 *
 * GPRs: 14 x 8 = 112 bytes (r0, r18-r30)
 * Special GPRs: 2 x 8 = 16 bytes (PC, SP)
 * FP status: 4 bytes (FPSR, low 32 bits)
 * FP control: 4 bytes (FPCR, low 32 bits)
 * FP regs: 8 x 16 = 128 bytes (Q8-Q15)
 * == TOTAL: 264 bytes ==
 *
 * OFFSET HEX | OFFSET DEC | SIZE | CONTENTS
 * -----------------------------------------
 * 0x0        | 0          | 8    | R18
 * 0x8        | 8          | 8    | R19
 * 0x10       | 16         | 8    | R20
 * 0x18       | 24         | 8    | R21
 * 0x20       | 32         | 8    | R22
 * 0x28       | 40         | 8    | R23
 * 0x30       | 48         | 8    | R24
 * 0x38       | 56         | 8    | R25
 * 0x40       | 64         | 8    | R26
 * 0x48       | 72         | 8    | R27
 * 0x50       | 80         | 8    | R28
 * 0x58       | 88         | 8    | R29 (FP)
 * 0x60       | 96         | 8    | R30 (LR)
 * 0x68       | 104        | 8    | PC
 * 0x70       | 112        | 8    | SP
 * 0x78       | 120        | 4    | FPSR
 * 0x7c       | 124        | 4    | FPCR
 * 0x80       | 128        | 16   | Q8
 * 0x90       | 144        | 16   | Q9
 * 0xa0       | 160        | 16   | Q10
 * 0xb0       | 176        | 16   | Q11
 * 0xc0       | 192        | 16   | Q12
 * 0xd0       | 208        | 16   | Q13
 * 0xe0       | 224        | 16   | Q14
 * 0xf0       | 240        | 16   | Q15
 * 0x100      | 256        | 8    | R0
 *
 * according to the SYSV ABI (AAPCS):
 * (saving)
 * CALLEE-SAVED: r19-r28, q8-q15 (we save the 128 bit Q form instead of 64 bit V for simplification)
 * SPECIAL: FP(r29), LR(r30), PC, SP, platform reg(r18)
 * SYSTEM FP: FPCR, FPSR
 * ARG: r0 (so we are able to set r0 for the target func with UpdateContext())
 *
 * (skipping, because we emulate function call by the context switch)
 * ARGS: r0-r7, v0-v7
 * TEMPS: r8-r17, v16-v31
 */

#define FCTX_LEN_BYTES 264

// gpr
#define FCTX_GPR_OFFSET_BYTES 0
#define FCTX_GPR_SIZE_BYTES 8
#define FCTX_GPR_OFFSET_BYTES_BY_INDEX(i) (FCTX_GPR_OFFSET_BYTES + FCTX_GPR_SIZE_BYTES * (i))
#define FCTX_GPR_OFFSET_BYTES_R18 FCTX_GPR_OFFSET_BYTES_BY_INDEX(0)
#define FCTX_GPR_OFFSET_BYTES_R19 FCTX_GPR_OFFSET_BYTES_BY_INDEX(1)
#define FCTX_GPR_OFFSET_BYTES_R20 FCTX_GPR_OFFSET_BYTES_BY_INDEX(2)
#define FCTX_GPR_OFFSET_BYTES_R21 FCTX_GPR_OFFSET_BYTES_BY_INDEX(3)
#define FCTX_GPR_OFFSET_BYTES_R22 FCTX_GPR_OFFSET_BYTES_BY_INDEX(4)
#define FCTX_GPR_OFFSET_BYTES_R23 FCTX_GPR_OFFSET_BYTES_BY_INDEX(5)
#define FCTX_GPR_OFFSET_BYTES_R24 FCTX_GPR_OFFSET_BYTES_BY_INDEX(6)
#define FCTX_GPR_OFFSET_BYTES_R25 FCTX_GPR_OFFSET_BYTES_BY_INDEX(7)
#define FCTX_GPR_OFFSET_BYTES_R26 FCTX_GPR_OFFSET_BYTES_BY_INDEX(8)
#define FCTX_GPR_OFFSET_BYTES_R27 FCTX_GPR_OFFSET_BYTES_BY_INDEX(9)
#define FCTX_GPR_OFFSET_BYTES_R28 FCTX_GPR_OFFSET_BYTES_BY_INDEX(10)
#define FCTX_GPR_OFFSET_BYTES_FP FCTX_GPR_OFFSET_BYTES_BY_INDEX(11)
#define FCTX_GPR_OFFSET_BYTES_LR FCTX_GPR_OFFSET_BYTES_BY_INDEX(12)
#define FCTX_GPR_OFFSET_BYTES_PC FCTX_GPR_OFFSET_BYTES_BY_INDEX(13)
#define FCTX_GPR_OFFSET_BYTES_SP FCTX_GPR_OFFSET_BYTES_BY_INDEX(14)
// fp
#define FCTX_FPSR_OFFSET_BYTES 120
#define FCTX_FP_OFFSET_BYTES_FPSR FCTX_FPSR_OFFSET_BYTES
#define FCTX_FPCR_OFFSET_BYTES 124
#define FCTX_FP_OFFSET_BYTES_FPCR FCTX_FPCR_OFFSET_BYTES
#define FCTX_FP_OFFSET_BYTES 128
#define FCTX_FP_SIZE_BYTES 16
#define FCTX_FP_OFFSET_BYTES_BY_INDEX(i) (FCTX_FP_OFFSET_BYTES + FCTX_FP_SIZE_BYTES * (i))
#define FCTX_FP_OFFSET_BYTES_Q8 FCTX_FP_OFFSET_BYTES_BY_INDEX(0)
#define FCTX_FP_OFFSET_BYTES_Q9 FCTX_FP_OFFSET_BYTES_BY_INDEX(1)
#define FCTX_FP_OFFSET_BYTES_Q10 FCTX_FP_OFFSET_BYTES_BY_INDEX(2)
#define FCTX_FP_OFFSET_BYTES_Q11 FCTX_FP_OFFSET_BYTES_BY_INDEX(3)
#define FCTX_FP_OFFSET_BYTES_Q12 FCTX_FP_OFFSET_BYTES_BY_INDEX(4)
#define FCTX_FP_OFFSET_BYTES_Q13 FCTX_FP_OFFSET_BYTES_BY_INDEX(5)
#define FCTX_FP_OFFSET_BYTES_Q14 FCTX_FP_OFFSET_BYTES_BY_INDEX(6)
#define FCTX_FP_OFFSET_BYTES_Q15 FCTX_FP_OFFSET_BYTES_BY_INDEX(7)
// extra: the arg register
#define FCTX_GPR_OFFSET_BYTES_R0 256

// NOLINTEND(cppcoreguidelines-macro-usage)

#endif /* PANDA_RUNTIME_FIBERS_ARCH_AARCH64_CONTEXT_LAYOUT_H */