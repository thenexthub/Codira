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

#include "runtime/arch/asm_support.h"

// int SumEightElements(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) {    
//     int sum = 0;
//     sum += a1;
//     sum += a2;
//     sum += a3;
//     sum += a4;
//     sum += a5;
//     sum += a6;
//     sum += a7;
//     sum += a8;
//     return sum;
// }

.global MAKE_ASM_NAME(SumEightElements)
.type SumEightElements, %function
MAKE_ASM_NAME(SumEightElements):
    sub     sp, sp, #16     // Move stack upper than is should be (originally sub 48)
    str     w0, [sp, -4]    // to enable writing under stack pointer.
    str     w1, [sp, -8]    // It is needed to enable signal stack frame override 
    str     w2, [sp, -12]   // memory of previos frame and corrupt it
    str     w3, [sp, -16]   // To avoid it we use alternate signal stack
    str     w4, [sp, -20]   // If memory under sp wasn't corrupted
    str     w5, [sp, -24]   // alternate signal stack works fine
    str     w6, [sp, -28]
    str     w7, [sp, -32]
    str     wzr, [sp, 12]
    ldr     w1, [sp, 12]
    ldr     w0, [sp, -4]
    add     w0, w1, w0
    str     w0, [sp, 12]
    ldr     w1, [sp, 12]
    ldr     w0, [sp, -8]
    add     w0, w1, w0
    str     w0, [sp, 12]
    ldr     w1, [sp, 12]
    ldr     w0, [sp, -12]
    add     w0, w1, w0
    str     w0, [sp, 12]
    ldr     w1, [sp, 12]
    ldr     w0, [sp, -16]
    add     w0, w1, w0
    str     w0, [sp, 12]
    ldr     w1, [sp, 12]
    ldr     w0, [sp, -20]
    add     w0, w1, w0
    str     w0, [sp, 12]
    ldr     w1, [sp, 12]
    ldr     w0, [sp, -24]
    add     w0, w1, w0
    str     w0, [sp, 12]
    ldr     w1, [sp, 12]
    ldr     w0, [sp, -28]
    add     w0, w1, w0
    str     w0, [sp, 12]
    ldr     w1, [sp, 12]
    ldr     w0, [sp, -32]
    add     w0, w1, w0
    str     w0, [sp, 12]
    ldr     w0, [sp, 12]
    add     sp, sp, 16
    ret

//            Stack
//        +-----------+
//        |           |
//        |           | <- fp
//        |           |
//        |           |
//        |           | <- sp moved upper
//        | w0 value  | ^
//        | w1 value  | |
//        | w2 value  | | Memory area under threat of corruption
//        |   ...     | | by next frame
//        |   ...     | |
//        |           | v
//        |           | <- sp original and correct position
//        |           |
//        +-----------+
