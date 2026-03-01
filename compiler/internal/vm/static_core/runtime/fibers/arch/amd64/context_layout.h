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

#ifndef PANDA_RUNTIME_FIBERS_ARCH_AMD64_CONTEXT_LAYOUT_H
#define PANDA_RUNTIME_FIBERS_ARCH_AMD64_CONTEXT_LAYOUT_H

// NOLINTBEGIN(cppcoreguidelines-macro-usage)

/**
 * Memory layout of the saved context:
 *
 * GPRs: 9 x 8 = 72 bytes (RBX, RBP, 12, 13, 14, 15, RDI, RIP, RSP)
 * Padding: 8 bytes
 * FPENV: 28 bytes
 * Padding: 4 bytes
 * MXCSR: 4 bytes
 * Padding: 12 bytes
 * == TOTAL: 128 bytes ==
 *
 * OFFSET HEX | OFFSET DEC | SIZE | CONTENTS
 * -----------------------------------------
 * 0x0        | 0          | 8    | RBX
 * 0x8        | 8          | 8    | RBP
 * 0x10       | 16         | 8    | R12
 * 0x18       | 24         | 8    | R13
 * 0x20       | 32         | 8    | R14
 * 0x28       | 40         | 8    | R15
 * 0x30       | 48         | 8    | RDI
 * 0x38       | 56         | 8    | RIP
 * 0x40       | 64         | 8    | RSP
 * 0x48       | 72         | 8    | (padding)
 * 0x50       | 80         | 28   | FPENV
 * 0x6c       | 108        | 4    | (padding)
 * 0x70       | 112        | 4    | MXCSR*
 * 0x74       | 116        | 12   | (padding)
 *
 * according to the SYSV ABI:
 * (saving)
 * ARGS: rdi (holds data pointer for the fiber function)
 * CALLEE-SAVED: rsp, rbp, rbx, r12, r13, r14, r15
 * SPECIAL: rip
 *
 * (skipping, because we emulate function call by the context switch)
 * CALLER SAVED (temps): R10, R11
 * ARGS: rsi, rdx, rcx, r8, r9
 * ZERO: RAX
 */

#define FCTX_LEN_BYTES 128

// gpr
#define FCTX_GPR_OFFSET_BYTES 0
#define FCTX_GPR_SIZE_BYTES 8
#define FCTX_GPR_OFFSET_BYTES_BY_INDEX(i) (FCTX_GPR_OFFSET_BYTES + FCTX_GPR_SIZE_BYTES * (i))
#define FCTX_GPR_OFFSET_BYTES_RBX FCTX_GPR_OFFSET_BYTES_BY_INDEX(0)
#define FCTX_GPR_OFFSET_BYTES_RBP FCTX_GPR_OFFSET_BYTES_BY_INDEX(1)
#define FCTX_GPR_OFFSET_BYTES_R12 FCTX_GPR_OFFSET_BYTES_BY_INDEX(2)
#define FCTX_GPR_OFFSET_BYTES_R13 FCTX_GPR_OFFSET_BYTES_BY_INDEX(3)
#define FCTX_GPR_OFFSET_BYTES_R14 FCTX_GPR_OFFSET_BYTES_BY_INDEX(4)
#define FCTX_GPR_OFFSET_BYTES_R15 FCTX_GPR_OFFSET_BYTES_BY_INDEX(5)
#define FCTX_GPR_OFFSET_BYTES_RDI FCTX_GPR_OFFSET_BYTES_BY_INDEX(6)
#define FCTX_GPR_OFFSET_BYTES_RIP FCTX_GPR_OFFSET_BYTES_BY_INDEX(7)
#define FCTX_GPR_OFFSET_BYTES_RSP FCTX_GPR_OFFSET_BYTES_BY_INDEX(8)
// fp
#define FCTX_FPENV_OFFSET_BYTES 80
#define FCTX_FP_OFFSET_BYTES_FPENV FCTX_FPENV_OFFSET_BYTES
#define FCTX_MXCSR_OFFSET_BYTES 112
#define FCTX_FP_OFFSET_BYTES_MXCSR FCTX_MXCSR_OFFSET_BYTES

// NOLINTEND(cppcoreguidelines-macro-usage)

#endif /* PANDA_RUNTIME_FIBERS_ARCH_AMD64_CONTEXT_LAYOUT_H */