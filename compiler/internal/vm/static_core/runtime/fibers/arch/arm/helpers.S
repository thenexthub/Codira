/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef PANDA_RUNTIME_FIBERS_ARCH_ARM_HELPERS_S
#define PANDA_RUNTIME_FIBERS_ARCH_ARM_HELPERS_S

/// stores GPR and FP context to the buffer that starts at the address stored in the register r0 using
/// the register #tmp_regid as a temporary
.macro STORE_CONTEXT tmp_regid
    /* save GPR */
    // gpr (incl. fp): assuming that R4..R15 reside in a contiguous block
    add r\tmp_regid, r0, # GPR_O(R4)
    stm r\tmp_regid, {r4-r12}
    str sp, [r0, # GPR_O(SP)]
    str lr, [r0, # GPR_O(LR)]
    // pc = lr (return to the caller)
    str lr, [r0, # GPR_O(PC)]
    // arg register: store 0
    mov r\tmp_regid, #0
    str r\tmp_regid, [r0, # GPR_O(R0)]

#ifndef PANDA_TARGET_ARM32_ABI_SOFT
    /* save FP */
    // regs: assuming that D8..D15 reside in a contiguous block
    add r\tmp_regid, r0, # FP_O(D8)
    vstm r\tmp_regid, {d8-d15}
    // control/status
    fmrx r\tmp_regid, fpscr                 // Get FPSCR value
    str r\tmp_regid, [r0, # FP_O(FPSCR)]    // and store it
#endif
.endm

#endif /* PANDA_RUNTIME_FIBERS_ARCH_ARM_HELPERS_S */