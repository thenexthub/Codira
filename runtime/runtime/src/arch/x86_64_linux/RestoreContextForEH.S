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


// context include all callee saved register %rbx %r12 %r13 %r14 %r15 %rbp and %rsp
// stack need to be 16-byte aligned, so we should use (8 * even numbers)
#define ContextSize (8 * 8)

// This assembly function is only used in exception handling.
// It is easier to maintain the stack structure and register status by using assembly.
// Used to restore the callee saved register and frame pointer layer by layer.
// Finally, jump to the landingpad of catch pointer.
    .text
    .align 2
    .global   RestoreContextForEH
    .type     RestoreContextForEH, @function
RestoreContextForEH:
    .cfi_startproc
    // Current rsp point to the top managed method.
    pushq   %rsp
    pushq   %rbp
    .cfi_def_cfa_register %rsp
    .cfi_def_cfa_offset 16
    callq   MRT_GetTopManagedPC
    movq    0x08(%rsp), %rdi
    movq    %rax, 0x08(%rsp)

    subq    $ContextSize, %rsp
    .cfi_def_cfa_offset 80
    // Save all callee saved register to stack as the layout of context struct.
    movq    %rbx, (%rsp)
    movq    %r12, 0x08(%rsp)
    movq    %r13, 0x10(%rsp)
    movq    %r14, 0x18(%rsp)
    movq    %r15, 0x20(%rsp)
    movq    %rbp, 0x28(%rsp)
    movq    %rdi, 0x30(%rsp)

    // Current rsp point to the context struct address.
    movq    %rsp, %rdi
    callq   MRT_RestoreContext

    // Restore all callee saved register.
    movq    (%rsp), %rbx
    movq    0x08(%rsp), %r12
    movq    0x10(%rsp), %r13
    movq    0x18(%rsp), %r14
    movq    0x20(%rsp), %r15
    movq    0x28(%rsp), %rbp
    movq    0x30(%rsp), %rsp

    // jump to landingpad.
    jmpq   *%rax
    .cfi_endproc
    .size RestoreContextForEH, .-RestoreContextForEH
