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

#ifndef PANDA_RUNTIME_FIBERS_ARCH_AMD64_HELPERS_S
#define PANDA_RUNTIME_FIBERS_ARCH_AMD64_HELPERS_S

/// stores GPR and FP context to the buffer that starts at the address stored in the register named ctx_baseaddr_reg using
/// the register named tmp_reg as a temporary
.macro STORE_CONTEXT ctx_baseaddr_reg, tmp_reg
    // put zero to the call arg register
    movq    $0, GPR_O(RDI)(\ctx_baseaddr_reg)
    // gp: callee-saved
    movq    %rbx, GPR_O(RBX)(\ctx_baseaddr_reg)
    movq    %rbp, GPR_O(RBP)(\ctx_baseaddr_reg)
    movq    %r12, GPR_O(R12)(\ctx_baseaddr_reg)
    movq    %r13, GPR_O(R13)(\ctx_baseaddr_reg)
    movq    %r14, GPR_O(R14)(\ctx_baseaddr_reg)
    movq    %r15, GPR_O(R15)(\ctx_baseaddr_reg)
    // context->RIP = return address from stack, i.e. *%rsp
    movq    (%rsp), \tmp_reg
    movq    \tmp_reg, GPR_O(RIP)(\ctx_baseaddr_reg)
    // context->RSP = %rsp + sizeof(void*), i.e. like we just returned from GetCurrentContext
    leaq     8(%rsp), \tmp_reg
    movq    \tmp_reg, GPR_O(RSP)(\ctx_baseaddr_reg)
    // fp: env
    leaq    FP_O(FPENV)(\ctx_baseaddr_reg), \tmp_reg
    // fp: csr
    stmxcsr FP_O(MXCSR)(\ctx_baseaddr_reg)
    // do load-store because fnstenv blocks FP exceptions
    fnstenv (\tmp_reg)
    fldenv  (\tmp_reg)
.endm

#endif /* PANDA_RUNTIME_FIBERS_ARCH_AMD64_HELPERS_S */