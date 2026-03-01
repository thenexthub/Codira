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

    .def    CODE_MCC_ThrowStackOverflowError;
    .scl    2;
    .type   32;
    .endef
    .globl  CODE_MCC_ThrowStackOverflowError            # -- Begin function CODE_MCC_ThrowStackOverflowError
    .p2align    4, 0x90
CODE_MCC_ThrowStackOverflowError:                   # @CODE_MCC_ThrowStackOverflowError
.seh_proc CODE_MCC_ThrowStackOverflowError
# %bb.0:
    pushq   %rbp
    .seh_pushreg %rbp
    movq    %rsp, %rbp
    .seh_setframe %rbp, 0
    .seh_endprologue
    callq  MCC_ThrowStackOverflowError
    popq    %rbp
    retq
    .seh_endproc
