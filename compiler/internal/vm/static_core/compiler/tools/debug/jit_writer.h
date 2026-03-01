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

#ifndef COMPILER_TOOLS_DEBUG_JIT_WRITER_H
#define COMPILER_TOOLS_DEBUG_JIT_WRITER_H

#ifdef PANDA_COMPILER_DEBUG_INFO

#include "aot/compiled_method.h"
#include "aot/aot_file.h"
#include "aot/aot_builder/elf_builder.h"
#include "libarkbase/utils/arch.h"
#include "libarkbase/utils/arena_containers.h"
#include "libarkbase/utils/bit_vector.h"
#include "optimizer/ir/runtime_interface.h"
#include <string>
#include <vector>
#include "mem/gc/gc_types.h"

namespace ark::panda_file {
class File;
}  // namespace ark::panda_file

namespace ark {
class Class;
}  // namespace ark

namespace ark::compiler {
template <Arch ARCH, bool IS_JIT_MODE>
class ElfBuilder;

class JitDebugWriter : public ElfWriter {
public:
    // NOLINTNEXTLINE(modernize-pass-by-value)
    JitDebugWriter(Arch arch, RuntimeInterface *runtime, CodeAllocator *codeAllocator, const std::string &methodName)
        : codeAllocator_(codeAllocator), methodName_(methodName)
    {
        SetArch(arch);
        SetRuntime(runtime);
        SetEmitDebugInfo(true);
    }

    JitDebugWriter() = delete;

    bool Write();

    void Start();
    void End();

    Span<uint8_t> GetElf()
    {
        return elf_;
    }

    Span<uint8_t> GetCode()
    {
        return code_;
    }

private:
    template <Arch ARCH>
    bool WriteImpl();

private:
    Span<uint8_t> elf_;
    Span<uint8_t> code_;
    CodeAllocator *codeAllocator_ {nullptr};

    const std::string &methodName_;
    friend class CodeDataProvider;
    friend class JitCodeDataProvider;
};

}  // namespace ark::compiler

// Next "C"-code need for enable interaction with gdb
// Please read "JIT Compilation Interface" from gdb-documentation for more information
extern "C" {
// NOLINTNEXTLINE(modernize-use-using)
typedef enum { JIT_NOACTION = 0, JIT_REGISTER_FN, JIT_UNREGISTER_FN } JitActionsT;

// NOLINTNEXTLINE(modernize-use-using)
typedef struct jit_code_entry JitCodeEntry;
struct jit_code_entry {
    jit_code_entry *nextEntry;
    jit_code_entry *prevEntry;
    const char *symfileAddr;
    uint64_t symfileSize;
};

// NOLINTNEXTLINE(modernize-use-using, readability-identifier-naming)
typedef struct jit_descriptor {
    uint32_t version;
    uint32_t actionFlag;
    jit_code_entry *relevantEntry;
    jit_code_entry *firstEntry;
} jit_descriptor;  // NOLINT(readability-identifier-naming)
}

#endif  // PANDA_COMPILER_DEBUG_INFO
#endif  // COMPILER_TOOLS_DEBUG_JIT_WRITER_H
