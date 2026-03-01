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
#ifndef PANDA_INTERPRETER_ARCH_AARCH64_MACROS_H_
#define PANDA_INTERPRETER_ARCH_AARCH64_MACROS_H_

#ifndef __clang_analyzer__
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define RESTORE_GLOBAL_REGS() asm volatile("" ::: "x20", "x21", "x22", "x23", "x24", "x25", "x28")
#else
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define RESTORE_GLOBAL_REGS()
#endif

// clang-format off
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DISPATCH(DISPATCH_TABLE, OPCODE)                                             \
    do {                                                                             \
        void const *_label;                                                          \
        asm("ldr %[label], [%[dispatch_table], %w[opcode], uxtw #3]"                 \
            : [label] "=r"(_label)                                                   \
            /* CC-OFFNXT(G.PRE.02) code generation */                                \
            : [dispatch_table] "r"(DISPATCH_TABLE), [opcode] "r"((uint32_t)OPCODE)); \
        goto *_label; /* CC-OFF(G.PRE.05) code generation */                         \
    } while (0)
// clang-format on

#endif  // PANDA_INTERPRETER_ARCH_AARCH64_MACROS_H_
