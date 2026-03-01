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

.global MAKE_ASM_NAME(ExpandCompiledCodeArgsDyn)
MAKE_ASM_NAME(ExpandCompiledCodeArgsDyn):
    //x0   - method, preserved
    //x1   - num_actual, preserved
    //x2   - num_expected, tmp1
    //x3   - tmp2
    //x4   - tmp3
    //x5   - tmp4

    // x2   - 8 * (aligned(2) num_expected - num_actual)
    // w3   - num_actual_args + 2 (for fp, lr)
    // x4   - sp before moving
    sub x2, x1, x2
    and x2, x2, -2
    lsl x2, x2, 3
    add w3, w1, 2
    mov x4, sp

    // Move sp and fp
    add sp, sp, x2
    add fp, fp, x2

.Lloop_move:
    ldr x5, [x4]
    str x5, [x4, x2]
    add x4, x4, 8
    subs w3, w3, 1
    bhi .Lloop_move

    mov w5, TAGGED_VALUE_UNDEFINED
    // Use loop counter as index
    add x4, x4, x2
    sub x4, x4, 8
    neg x2, x2
.Lloop_init:
    str x5, [x4, x2]
    subs w2, w2, 8
    bhi .Lloop_init

    ret
