/*
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

.global StackWalkerHookBridge
.type StackWalkerHookBridge, %function
StackWalkerHookBridge:
    CFI_STARTPROC
    CFI_DEF_CFA(sp, 0)
    push {fp, lr}
    CFI_ADJUST_CFA_OFFSET(8)
    CFI_REL_OFFSET(lr, 4)
    CFI_REL_OFFSET(fp, 0)
    mov r0, lr
    mov r1, fp
    blx PandaRunnerHook
    pop {fp, lr}
    CFI_RESTORE(fp)
    CFI_RESTORE(lr)
    CFI_ADJUST_CFA_OFFSET(-8)
    bx lr
    CFI_ENDPROC
