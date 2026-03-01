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

#ifndef PANDA_RUNTIME_FIBERS_ARCH_ARM_CONTEXT_LAYOUT_H
#define PANDA_RUNTIME_FIBERS_ARCH_ARM_CONTEXT_LAYOUT_H

// NOLINTBEGIN(cppcoreguidelines-macro-usage)

/**
 * Memory layout of the saved context:
 *
 * GPRs: 9 x 4 = 36 bytes (r0, r4-r11)
 * Special registers: 4 x 4 = 16 bytes (r12-r15)
 * if PANDA_TARGET_ARM32_ABI_SOFT:
 * == TOTAL: 52 bytes ==
 *
 * else:
 * FP status and control: 4 bytes (FPSCR)
 * FP regs: 8 x 8 = 64 bytes (D8-D15)
 * == TOTAL: 120 bytes ==
 *
 * OFFSET HEX | OFFSET DEC | SIZE | CONTENTS
 * -----------------------------------------
 * 0x0        | 0          | 4    | R0
 * 0x4        | 4          | 4    | R4
 * 0x8        | 8          | 4    | R5
 * 0xc        | 12         | 4    | R6
 * 0x10       | 16         | 4    | R7
 * 0x14       | 20         | 4    | R8
 * 0x18       | 24         | 4    | R9
 * 0x1c       | 28         | 4    | R10
 * 0x20       | 32         | 4    | R11 (FP)
 * 0x24       | 36         | 4    | R12 (IP)
 * 0x28       | 40         | 4    | R13 (SP)
 * 0x2c       | 44         | 4    | R14 (LR)
 * 0x30       | 48         | 4    | R15 (PC)
 * -----------------------------------------
 * 0x34       | 52         | 4    | FPSCR
 * 0x38       | 56         | 8    | D8
 * 0x40       | 64         | 8    | D9
 * 0x48       | 72         | 8    | D10
 * 0x50       | 80         | 8    | D11
 * 0x58       | 88         | 8    | D12
 * 0x60       | 96         | 8    | D13
 * 0x68       | 104        | 8    | D14
 * 0x70       | 112        | 8    | D15
 *
 * according to the SYSV ABI (AAPCS):
 * (saving)
 * CALLEE-SAVED: r4-r11, d8-d15
 * SPECIAL: IP(r12), SP(r13), LR(r14), PC(r15)
 * SYSTEM FP: FPSCR
 * ARG: r0 (so we are able to set r0 for the target func with UpdateContext())
 *
 * (skipping, because we emulate function call by the context switch)
 * ARGS/SCRATCH: r0-r3
 */

#ifndef PANDA_TARGET_ARM32_ABI_SOFT
#define FCTX_LEN_BYTES 120
#else
#define FCTX_LEN_BYTES 52
#endif

// gpr
#define FCTX_GPR_OFFSET_BYTES 0
#define FCTX_GPR_SIZE_BYTES 4
#define FCTX_GPR_OFFSET_BYTES_BY_INDEX(i) (FCTX_GPR_OFFSET_BYTES + FCTX_GPR_SIZE_BYTES * (i))
#define FCTX_GPR_OFFSET_BYTES_R0 FCTX_GPR_OFFSET_BYTES_BY_INDEX(0)
#define FCTX_GPR_OFFSET_BYTES_R4 FCTX_GPR_OFFSET_BYTES_BY_INDEX(1)
#define FCTX_GPR_OFFSET_BYTES_R5 FCTX_GPR_OFFSET_BYTES_BY_INDEX(2)
#define FCTX_GPR_OFFSET_BYTES_R6 FCTX_GPR_OFFSET_BYTES_BY_INDEX(3)
#define FCTX_GPR_OFFSET_BYTES_R7 FCTX_GPR_OFFSET_BYTES_BY_INDEX(4)
#define FCTX_GPR_OFFSET_BYTES_R8 FCTX_GPR_OFFSET_BYTES_BY_INDEX(5)
#define FCTX_GPR_OFFSET_BYTES_R9 FCTX_GPR_OFFSET_BYTES_BY_INDEX(6)
#define FCTX_GPR_OFFSET_BYTES_R10 FCTX_GPR_OFFSET_BYTES_BY_INDEX(7)
#define FCTX_GPR_OFFSET_BYTES_R11 FCTX_GPR_OFFSET_BYTES_BY_INDEX(8)
#define FCTX_GPR_OFFSET_BYTES_R12 FCTX_GPR_OFFSET_BYTES_BY_INDEX(9)
#define FCTX_GPR_OFFSET_BYTES_SP FCTX_GPR_OFFSET_BYTES_BY_INDEX(10)
#define FCTX_GPR_OFFSET_BYTES_LR FCTX_GPR_OFFSET_BYTES_BY_INDEX(11)
#define FCTX_GPR_OFFSET_BYTES_PC FCTX_GPR_OFFSET_BYTES_BY_INDEX(12)
// fp
#define FCTX_FP_OFFSET_BYTES_FPSCR 52
#define FCTX_FP_OFFSET_BYTES 56
#define FCTX_FP_SIZE_BYTES 8
#define FCTX_FP_OFFSET_BYTES_BY_INDEX(i) (FCTX_FP_OFFSET_BYTES + FCTX_FP_SIZE_BYTES * (i))
#define FCTX_FP_OFFSET_BYTES_D8 FCTX_FP_OFFSET_BYTES_BY_INDEX(0)
#define FCTX_FP_OFFSET_BYTES_D9 FCTX_FP_OFFSET_BYTES_BY_INDEX(1)
#define FCTX_FP_OFFSET_BYTES_D10 FCTX_FP_OFFSET_BYTES_BY_INDEX(2)
#define FCTX_FP_OFFSET_BYTES_D11 FCTX_FP_OFFSET_BYTES_BY_INDEX(3)
#define FCTX_FP_OFFSET_BYTES_D12 FCTX_FP_OFFSET_BYTES_BY_INDEX(4)
#define FCTX_FP_OFFSET_BYTES_D13 FCTX_FP_OFFSET_BYTES_BY_INDEX(5)
#define FCTX_FP_OFFSET_BYTES_D14 FCTX_FP_OFFSET_BYTES_BY_INDEX(6)
#define FCTX_FP_OFFSET_BYTES_D15 FCTX_FP_OFFSET_BYTES_BY_INDEX(7)

// NOLINTEND(cppcoreguidelines-macro-usage)

#endif /* PANDA_RUNTIME_FIBERS_ARCH_ARM_CONTEXT_LAYOUT_H */