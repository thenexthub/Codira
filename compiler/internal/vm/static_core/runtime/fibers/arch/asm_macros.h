/**
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

#ifndef PANDA_RUNTIME_FIBERS_ARCH_ASM_MACROS_H
#define PANDA_RUNTIME_FIBERS_ARCH_ASM_MACROS_H

#include "runtime/arch/asm_support.h"

#if defined(PANDA_TARGET_ARM64)
#define FUNC_ALIGNMENT_BYTES 32
#elif defined(PANDA_TARGET_ARM32)
#define FUNC_ALIGNMENT_BYTES 16
#elif defined(PANDA_TARGET_X86)
#error "Unsupported target"
#elif defined(PANDA_TARGET_AMD64)
#define FUNC_ALIGNMENT_BYTES 16
#else
#error "Unsupported target"
#endif

#define FUNCTION_HEADER(name)     \
    .globl name;                  \
    TYPE_FUNCTION(name);          \
    .balign FUNC_ALIGNMENT_BYTES; \
    name##:

// This part needs to be rewritten for MacOS to call a multiline NASM macro
// NOTE(korobeinikovevgeny, #314 (new)): the macro body has been manually expanded here to speed up
// the MacOS target support. Should be optimized in future.)
#define FUNCTION_START(name) FUNCTION_HEADER(name)
#define LOCAL_FUNCTION_START(name) \
    .hidden name;                  \
    FUNCTION_HEADER(name)

// CC-OFFNXT(G.PRE.09) code generation
// CC-OFFNXT(G.PRE.02) list generation
#define FUNCTION_END(name) .size name, .- name;

#endif /* PANDA_RUNTIME_FIBERS_ARCH_ASM_MACROS_H */