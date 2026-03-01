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

#ifndef PANDA_RUNTIME_FIBERS_ARCH_AARCH64_HELPERS_S
#define PANDA_RUNTIME_FIBERS_ARCH_AARCH64_HELPERS_S

/// stores GPR and FP context to the buffer that starts at the address stored in the register #ctx_baseaddr_regid using
/// the register #tmp_regid as a temporary
.macro STORE_CONTEXT ctx_baseaddr_regid, tmp_regid
    /* save GPR */
    // gpr (incl. fp): assuming that X18..X29 reside in a contiguous block
    stp x18, x19, [x\ctx_baseaddr_regid, GPR_O(R18)]
    stp x20, x21, [x\ctx_baseaddr_regid, GPR_O(R20)]
    stp x22, x23, [x\ctx_baseaddr_regid, GPR_O(R22)]
    stp x24, x25, [x\ctx_baseaddr_regid, GPR_O(R24)]
    stp x26, x27, [x\ctx_baseaddr_regid, GPR_O(R26)]
    stp x28, x29, [x\ctx_baseaddr_regid, GPR_O(R28)]
    // arg register: store 0
    str xzr, [x\ctx_baseaddr_regid, GPR_O(R0)]
    // lr
    str lr, [x\ctx_baseaddr_regid, GPR_O(LR)]
    // pc = lr (return to the caller)
    str lr, [x\ctx_baseaddr_regid, GPR_O(PC)]
    // sp
    mov x\tmp_regid, sp
    str x\tmp_regid, [x\ctx_baseaddr_regid, GPR_O(SP)]

    /* save FP */
    // regs: assuming that Q8..Q15 reside in a contiguous block
    mov x\tmp_regid, x\ctx_baseaddr_regid
    add x\tmp_regid, x\tmp_regid, # FP_O(Q8)
    stp q8,  q9, [x\tmp_regid], # 2 * FCTX_FP_SIZE_BYTES
    stp q10, q11, [x\tmp_regid], # 2 * FCTX_FP_SIZE_BYTES
    stp q12, q13, [x\tmp_regid], # 2 * FCTX_FP_SIZE_BYTES
    stp q14, q15, [x\tmp_regid], # 2 * FCTX_FP_SIZE_BYTES
    // control/status
    mrs x\tmp_regid, fpsr
    str w\tmp_regid, [x\ctx_baseaddr_regid, FP_O(FPSR)]
    mrs x\tmp_regid, fpcr
    str w\tmp_regid, [x\ctx_baseaddr_regid, FP_O(FPCR)]
.endm

#endif /* PANDA_RUNTIME_FIBERS_ARCH_AARCH64_HELPERS_S */