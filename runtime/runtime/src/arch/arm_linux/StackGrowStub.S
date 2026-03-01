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

#define cfi_adjust_cfa_offset(off)	.cfi_adjust_cfa_offset off
#define cfi_rel_offset(reg, off)	.cfi_rel_offset reg, off
#define cfi_restore(reg)		.cfi_restore reg
#define cfi_def_cfa_register(reg)	.cfi_def_cfa_register reg

// This signal handler stub relies on MRT_PrepareToHandleSignal to put continuation pc in x17.
// Continuation pc describes the control flow after signal handling completes.

    .text
    .align 2
    .global CODE_MCC_StackGrowStub
    .type CODE_MCC_StackGrowStub, %function
CODE_MCC_StackGrowStub:
    .cfi_startproc
    sub  sp, sp, #8
    str  r11, [sp]
    str  lr, [sp, #4]
    cfi_adjust_cfa_offset (8)
    cfi_rel_offset (r11, 0)
    cfi_rel_offset (lr, 4)

    // unsupport for arm32
    bl  MRT_StackGrow
    .global unwindPCForStackGrowStub
unwindPCForStackGrowStub:
    ldr  r11, [sp]
    ldr  lr, [sp, #4]
    cfi_restore(r11)
    cfi_restore(lr)
    bx  lr
    .cfi_endproc
    .size CODE_MCC_StackGrowStub, .-CODE_MCC_StackGrowStub
