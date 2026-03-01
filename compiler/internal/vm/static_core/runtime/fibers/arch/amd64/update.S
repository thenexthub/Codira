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
#include "runtime/fibers/arch/amd64/context_layout.h"

.extern abort

LOCAL_FUNCTION_START(FiberExitBridge)
    // indicate abnormal exit by calling exit(-1)
    // because fibers should not return from FiberProc
    // movq $-1, %rax
    // call exit

    // let's make the exit more violent than exit(-1) for now
    // TODO(konstanting, #IAD5MH): revert to exit() when everything is tested and works
    call    abort@plt
FUNCTION_END(FiberExitBridge)


#define GPR_O(reg) FCTX_GPR_OFFSET_BYTES_ ## reg
#define FP_O(reg) FCTX_FP_OFFSET_BYTES_ ## reg

/**
 * arguments:
 * rdi: uint8_t* ctx_memory
 * rsi: void (*func)(void*)
 * rdx: void* argument
 * rcx: uint8_t* stack
 * r8:  size_t stack_size_bytes
 */
FUNCTION_START(UpdateContext)
    // context->RIP = func
    movq    %rsi, GPR_O(RIP)(%rdi)
    // context->RDI = argument
    movq    %rdx, GPR_O(RDI)(%rdi)

    /**
     * %rax will hold the new RSP value:
     * NEW_RSP = align((stack + stack_size), 16B) - sizeof(trampoline_ptr)
     */

    // %rax = stack + stack_size_bytes
    movq    %rcx, %rax
    addq    %r8, %rax
    // %rax = align(%rax, 16B) - 8B
    andq    $-16, %rax
    subq    $8, %rax
    // context->RSP = %rax
    movq    %rax, GPR_O(RSP)(%rdi)
    // put FiberExitBridge as the return address for the FiberProc
    leaq    FiberExitBridge(%rip), %r10
    movq    %r10, (%rax)

    // return success
    xorq    %rax, %rax
    ret
FUNCTION_END(UpdateContext)

/**
 * arguments:
 * rdi: uint8_t* ctx_memory
 * rsi: void (*func)(void*)
 * rdx: void* argument
 */
FUNCTION_START(UpdateContextKeepStack)
    // (use %rax, %r10 as temporaries)
    // %r10 = context->RIP
    movq    GPR_O(RIP)(%rdi), %r10
    // context->RIP = func
    movq    %rsi, GPR_O(RIP)(%rdi)
    // context->RDI = argument
    movq    %rdx, GPR_O(RDI)(%rdi)

    // pretend that we called the func(), i.e. push context's RIP to context's stack:
    // %rax = context->RSP
    movq    GPR_O(RSP)(%rdi), %rax
    // %rax -= 8
    subq    $8, %rax
    // M[%rax] = %r10
    movq    (%rax), %r10
    // context->RSP = %rax
    movq    %rax, GPR_O(RSP)(%rdi)

    // return success
    xorq    %rax, %rax
    ret
FUNCTION_END(UpdateContextKeepStack)

#undef GPR_O
#undef FP_O

// we don't need executable stack.
.section .note.GNU-stack,"",%progbits