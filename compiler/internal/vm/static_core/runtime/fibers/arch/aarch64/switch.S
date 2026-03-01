/*
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

#include "runtime/fibers/arch/asm_macros.h"
#include "runtime/fibers/arch/aarch64/context_layout.h"

#define GPR_O(reg) FCTX_GPR_OFFSET_BYTES_ ## reg
#define FP_O(reg) FCTX_FP_OFFSET_BYTES_ ## reg
#include "runtime/fibers/arch/aarch64/helpers.S"

/**
 * arguments:
 * x0: uint8_t* ctx_memory_from
 * x1: uint8_t* ctx_memory_to
 */
// NOTE(korobeinikovevgeny, #314 (new)): the macro body has been manually expanded here to speed up
// the MacOS target support. Should be optimized in future.)
.globl MAKE_ASM_NAME(SwitchContext)
TYPE_FUNCTION(SwitchContext)
.balign FUNC_ALIGNMENT_BYTES
MAKE_ASM_NAME(SwitchContext):
    /*** save current context ***/
    STORE_CONTEXT ctx_baseaddr_regid=0, tmp_regid=8

    /*** restore next context ***/
    /* restore GPR */
    // gpr (incl. fp): assuming that X18..X29 reside in a contiguous block
    ldp x18, x19, [x1, GPR_O(R18)]
    ldp x20, x21, [x1, GPR_O(R20)]
    ldp x22, x23, [x1, GPR_O(R22)]
    ldp x24, x25, [x1, GPR_O(R24)]
    ldp x26, x27, [x1, GPR_O(R26)]
    ldp x28, x29, [x1, GPR_O(R28)]
    // arg register
    ldr x0, [x1, GPR_O(R0)]
    // lr
    ldr lr, [x1, GPR_O(LR)]
    // sp
    ldr x8, [x1, GPR_O(SP)]
    mov sp, x8

    /* restore FP */
    // regs: assuming that Q8..Q15 reside in a contiguous block
    mov x8, x1
    add x8, x8, # FP_O(Q8)
    ldp q8, q9, [x8], # 2 * FCTX_FP_SIZE_BYTES
    ldp q10, q11, [x8], # 2 * FCTX_FP_SIZE_BYTES
    ldp q12, q13, [x8], # 2 * FCTX_FP_SIZE_BYTES
    ldp q14, q15, [x8], # 2 * FCTX_FP_SIZE_BYTES
    // control/status
    ldr w8, [x1, FP_O(FPSR)]
    msr fpsr, x8
    ldr w8, [x1, FP_O(FPCR)]
    msr fpcr, x8

    /* branch to the restored PC */
    ldr x8, [x1, GPR_O(PC)]
    br  x8
FUNCTION_END(SwitchContext)

#undef GPR_O
#undef FP_O

// we don't need executable stack.
#ifndef PANDA_TARGET_MACOS
.section .note.GNU-stack,"",%progbits
#endif
