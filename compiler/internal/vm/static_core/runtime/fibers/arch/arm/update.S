/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

#include "runtime/fibers/arch/asm_macros.h"
#include "runtime/fibers/arch/arm/context_layout.h"

.extern abort

LOCAL_FUNCTION_START(FiberExitBridge)
    // indicate abnormal exit by calling exit(-1)
    // because fibers should not return from FiberProc
    // mov r0, #-1
    // bl exit

    // let's make the exit more violent than exit(-1) for now
    // TODO(konstanting, #IAD5MH): revert to exit() when everything is tested and works
    bl abort
FUNCTION_END(FiberExitBridge)

#define GPR_O(reg) FCTX_GPR_OFFSET_BYTES_ ## reg
#define FP_O(reg) FCTX_FP_OFFSET_BYTES_ ## reg

/**
 * arguments:
 * r0: uint8_t* ctx_memory
 * r1: void (*func)(void*)
 * r2: void* argument
 * r3: uint8_t* stack
 * [sp]: size_t stack_size_bytes
 */
FUNCTION_START(UpdateContext)
    // context->PC = func
    str r1, [r0, # GPR_O(PC)]
    // now we can reuse r1
    // context->LR = FiberExitBridge (for the case when Fiber returns)
    adr r1, FiberExitBridge
    str r1, [r0, # GPR_O(LR)]
    // context->R0 = argument
    str r2, [r0, # GPR_O(R0)]

    /**
     * r1 will hold the new SP value:
     * NEW_SP = align((stack + stack_size), 16B)
     */

    // r1 = stack + stack_size_bytes
    ldr r1, [sp]
    add r1, r3, r1
    // r1 = align(r1, 16B)
    and r1, r1, #-16
    // context->SP = r1
    str r1, [r0, # GPR_O(SP)]
    // context->FP = 0
    mov r1, #0
    str r1, [r0, # GPR_O(R11)]

    // return success
    mov r0, #0
    blx lr
FUNCTION_END(UpdateContext)

/**
 * arguments:
 * r0: uint8_t* ctx_memory
 * r1: void (*func)(void*)
 * r2: void* argument
 */
FUNCTION_START(UpdateContextKeepStack)
    // r4 = context->PC
    ldr r4, [r0, # GPR_O(PC)]
    // context->PC = func
    str r1, [r0, # GPR_O(PC)]
    // context->R0 = argument
    str r2, [r0, # GPR_O(R0)]

    // pretend that we called the func(), i.e. put context's PC to LR
    // context->LR = r4
    str r4, [r0, # GPR_O(LR)]

    // return success
    mov r0, #0
    blx lr
FUNCTION_END(UpdateContextKeepStack)

#undef GPR_O
#undef FP_O

// we don't need executable stack.
.section .note.GNU-stack,"",%progbits