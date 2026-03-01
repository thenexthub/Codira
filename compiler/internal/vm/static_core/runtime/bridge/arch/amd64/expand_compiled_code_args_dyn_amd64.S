/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "arch/asm_support.h"
#include "arch/amd64/helpers_amd64.S"

.global ExpandCompiledCodeArgsDyn
TYPE_FUNCTION(ExpandCompiledCodeArgsDyn)
ExpandCompiledCodeArgsDyn:
    // %edi         - method, preserved
    // %esi         - num_actual, preserved
    // %edx         - num_expected, tmp1
    // %rcx         - tmp2
    // %r8          - tmp3
    // %r9          - tmp4

    // %rdx         - 8 * (aligned(2) num_expected - num_actual)
    // %rcx         - num_actual_args + 3 (for fp, lr, lr)
    // %r8          - %rsp before moving
    subl %esi, %edx
    negq %rdx
    andq $-2, %rdx
    shlq $3, %rdx
    leal 3(%esi), %ecx
    movq %rsp, %r8

    // Move sp and fp
    addq %rdx, %rsp
    addq %rdx, %rbp

.Lloop_move:
    movq (%r8), %r9
    movq %r9, (%r8, %rdx)
    addq $8, %r8
    subl $1, %ecx
    ja .Lloop_move

    movl $TAGGED_VALUE_UNDEFINED, %r9d
    // Use loop counter as index
    leaq -8(%r8, %rdx), %r8
    negl %edx
.Lloop_init:
    movq %r9, (%r8, %rdx)
    subl $8, %edx
    ja .Lloop_init

    ret
