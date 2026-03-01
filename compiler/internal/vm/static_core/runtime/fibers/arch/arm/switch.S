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
#include "runtime/fibers/arch/arm/context_layout.h"

#define GPR_O(reg) FCTX_GPR_OFFSET_BYTES_ ## reg
#define FP_O(reg) FCTX_FP_OFFSET_BYTES_ ## reg
#include "runtime/fibers/arch/arm/helpers.S"

/**
 * arguments:
 * r0: uint8_t* ctx_memory_from
 * r1: uint8_t* ctx_memory_to
 */
FUNCTION_START(SwitchContext)
    /*** save current context ***/
    STORE_CONTEXT tmp_regid=3
    /*** restore next context ***/
    /* restore GPR */
    // gpr (incl. fp): assuming that R4..R15 reside in a contiguous block
    add r3, r1, # GPR_O(R4)
    ldm r3, {r4-r12}
    ldr sp, [r1, # GPR_O(SP)]
    ldr lr, [r1, # GPR_O(LR)]
    // arg register
    ldr r0, [r1, # GPR_O(R0)]

#ifndef PANDA_TARGET_ARM32_ABI_SOFT
    /* restore FP */
    // regs: assuming that D8..D15 reside in a contiguous block
    add r3, r1, # FP_O(D8)
    vldm r3, {d8-d15}
    // control/status
    ldr r3, [r1, # FP_O(FPSCR)]      // Get saved FPSCR value
    fmxr fpscr, r3                   // and restore it
#endif

    /* branch to the restored PC */
    ldr r3, [r1, # GPR_O(PC)]
    bx r3
FUNCTION_END(SwitchContext)

#undef GPR_O
#undef FP_O

// we don't need executable stack.
.section .note.GNU-stack,"",%progbits