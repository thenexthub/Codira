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

#include "runtime/fibers/arch/asm_macros.h"
#include "runtime/fibers/arch/amd64/context_layout.h"

#define GPR_O(reg) FCTX_GPR_OFFSET_BYTES_ ## reg
#define FP_O(reg) FCTX_FP_OFFSET_BYTES_ ## reg
#include "runtime/fibers/arch/amd64/helpers.S"

/**
 * arguments:
 * rdi: uint8_t* ctx_memory
 */
FUNCTION_START(GetCurrentContext)
    STORE_CONTEXT ctx_baseaddr_reg=%rdi, tmp_reg=%rcx

    xorq    %rax, %rax
    ret
FUNCTION_END(GetCurrentContext)

#undef GPR_O
#undef FP_O

// we don't need executable stack.
.section .note.GNU-stack,"",%progbits