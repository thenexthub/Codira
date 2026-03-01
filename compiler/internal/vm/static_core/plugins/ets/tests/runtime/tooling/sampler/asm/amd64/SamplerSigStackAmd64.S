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

//  int SumEightElements(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) {
//      int arr[64];   // add it to avoid red zone on x86-64
//      arr[0] = 1;
//      arr[63] = 2;
//      
//      int sum = 0;
//      sum += a1;
//      sum += a2;
//      sum += a3;
//      sum += a4;
//      sum += a5;
//      sum += a6;
//      sum += a7;
//      sum += a8;
//      return sum;
//  }

.global SumEightElements
.type SumEightElements, %function
SumEightElements:         // @SumFoo(int, int, int, int, int, int, int, int)
    pushq   %rbp
    movq    %rsp, %rbp
    // subq    $184, %rsp       // Move stack upper than is should be
    movl    %edi, -276(%rbp)    // to enable writing under red zone (128 bytes)
    movl    %esi, -280(%rbp)    // It is needed to enable signal stack frame override 
    movl    %edx, -284(%rbp)    // memory of previos frame and corrupt it
    movl    %ecx, -288(%rbp)    // To avoid it we use alternate signal stack
    movl    %r8d, -292(%rbp)    // If memory under red zone wasn't corrupted
    movl    %r9d, -296(%rbp)    // alternate signal stack works fine
    movl    $1, -272(%rbp)
    movl    $2, -20(%rbp)
    movl    $0, -4(%rbp)
    movl    -276(%rbp), %eax
    addl    %eax, -4(%rbp)
    movl    -280(%rbp), %eax
    addl    %eax, -4(%rbp)
    movl    -284(%rbp), %eax
    addl    %eax, -4(%rbp)
    movl    -288(%rbp), %eax
    addl    %eax, -4(%rbp)
    movl    -292(%rbp), %eax
    addl    %eax, -4(%rbp)
    movl    -296(%rbp), %eax
    addl    %eax, -4(%rbp)
    movl    16(%rbp), %eax
    addl    %eax, -4(%rbp)
    movl    24(%rbp), %eax
    addl    %eax, -4(%rbp)
    movl    -4(%rbp), %eax
    leave
    ret

//            Stack
//        +-----------+
//        |           |
//        |           | <- fp, sp (moved upper)
//        |           |
//        |           |
//        |           | <- sp (moved upper) - 128, end of red zone
//        |           |                                        ^
//        |           | <- sp original and correct position    |
//        |           |                                        | Memory under threat
//        | edi value |                                        | of corruption by
//        | esi value |                                        | next frame
//        |    ...    |                                        |
//        |           | <- end of original red zone            v
//        |           |
//        +-----------+
