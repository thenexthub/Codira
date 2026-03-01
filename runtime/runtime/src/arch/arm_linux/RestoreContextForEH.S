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

// context include all callee saved register r4 r5 r6 r7 r8 r9 r10
// and d8 d9 d10 d11 d12 d13 d14 d15 fa sp.
// stack need to be 16-byte aligned, so we should use (8 * even numbers)
#define ContextSize (4 * 14 + 8 * 8)

// This assembly function is only used in exception handling.
// It is easier to maintain the stack structure and register status by using assembly.
// Used to restore the callee saved register and frame pointer layer by layer.
// Finally, jump to the landingpad of catch pointer.
    .text
    .align 2
    .global   RestoreContextForEH
    .type     RestoreContextForEH, %function
RestoreContextForEH:
    .cfi_startproc
    // Current sp point to the top managed method.
    mov  r1, sp
    sub  sp, sp, #ContextSize
    str  r11, [sp, #112]
    str  r1, [sp, #116]
    .cfi_def_cfa sp, 120
    .cfi_offset  lr, -4

    bl  MRT_GetTopManagedPC
    ldr  r1, [sp, #116]
    str  r0, [sp, #116]

    // Save all callee saved register to stack as the layout of context struct
    str  r4, [sp]
    str  r5, [sp, #4]
    str  r6, [sp, #8]
    str  r7, [sp, #12]
    str  r8, [sp, #16]
    str  r9, [sp, #20]
    str  r10, [sp, #24]
    str  r11, [sp, #28]
    str  lr, [sp, #32]
    
    vstr  d8, [sp, #40]
    vstr  d9, [sp, #48]
    vstr  d10, [sp, #56]
    vstr  d11, [sp, #64]
    vstr  d12, [sp, #72]
    vstr  d13, [sp, #80]
    vstr  d14, [sp, #88]
    vstr  d15, [sp, #96]
    str  r1, [sp, #104]

    mov  r0, sp
    bl  MRT_RestoreContext

    // Restore all callee saved register.
    ldr  r4, [sp]
    ldr  r5, [sp, #4]
    ldr  r6, [sp, #8]
    ldr  r7, [sp, #12]
    ldr  r8, [sp, #16]
    ldr  r9, [sp, #20]
    ldr  r10, [sp, #24]
    ldr  r11, [sp, #28]
    ldr  lr, [sp, #32]
    
    vldr  d8, [sp, #40]
    vldr  d9, [sp, #48]
    vldr  d10, [sp, #56]
    vldr  d11, [sp, #64]
    vldr  d12, [sp, #72]
    vldr  d13, [sp, #80]
    vldr  d14, [sp, #88]
    vldr  d15, [sp, #96]
    ldr  r1, [sp, #104]
    mov  sp, r1

    // jump to landingpad.
    bx  r0
    .cfi_endproc
    .size RestoreContextForEH, .-RestoreContextForEH
