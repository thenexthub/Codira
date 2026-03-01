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

#ifndef PANDA_DWARF_BUILDER_H
#define PANDA_DWARF_BUILDER_H

#include "libarkbase/utils/arch.h"
#include "libarkbase/utils/span.h"

#ifdef PANDA_COMPILER_DEBUG_INFO
#include <libdwarf/libdwarf.h>
#endif

#include "elfio/elfio.hpp"

#include <string>
#include <unordered_map>

namespace ark::irtoc {
class Function;

class DwarfBuilder {
public:
    struct Section {
        std::string name;
        unsigned type;
        unsigned flags;
        unsigned link;
        unsigned info;
        std::vector<uint8_t> data;
    };

    enum class SectionKind { DEBUG_LINE = 1, REL_DEBUG_LINE };

    DwarfBuilder(Arch arch, ELFIO::elfio *elf);

    bool BuildGraphNestedFunction(unsigned symbol, const Function *func, Dwarf_Error &error, Dwarf_P_Die &die);

    bool BuildGraph(const Function *func, uint32_t codeOffset, unsigned symbol);

    bool FinalizeNestedFunction(Dwarf_Error &error);

    bool Finalize(uint32_t codeSize);

    Dwarf_Unsigned AddFile(const std::string &fname, Dwarf_Unsigned dirIndex);

    Dwarf_Unsigned AddDir(const std::string &dname);

    Arch GetArch()
    {
        return arch_;
    }

    ELFIO::elfio *GetElfBuilder()
    {
        return elfBuilder_;
    }

    // CC-OFFNXT(G.FUN.01-CPP) depend on Dwarf_Callback_Func by DWARF Producer Interface. Should be suppress
    static int CreateSectionCallback([[maybe_unused]] char *name, [[maybe_unused]] int size,
                                     [[maybe_unused]] Dwarf_Unsigned type, [[maybe_unused]] Dwarf_Unsigned flags,
                                     [[maybe_unused]] Dwarf_Unsigned link, [[maybe_unused]] Dwarf_Unsigned info,
                                     [[maybe_unused]] Dwarf_Unsigned *sectNameIndex, [[maybe_unused]] void *userData,
                                     [[maybe_unused]] int *error);

private:
    std::vector<ELFIO::section *> sections_;
    std::unordered_map<std::string, Dwarf_Unsigned> sourceFilesMap_;
    std::unordered_map<std::string, Dwarf_Unsigned> dirsMap_;
    std::unordered_map<unsigned, unsigned> indexMap_;
    std::unordered_map<unsigned, ELFIO::relocation_section_accessor> relMap_;

    ELFIO::elfio *elfBuilder_ {nullptr};
    Dwarf_P_Debug dwarf_ {nullptr};
    Dwarf_P_Die compileUnitDie_ {nullptr};
    Dwarf_P_Die prevProgramDie_ {nullptr};
    uint32_t codeSize_ {0};
    Arch arch_;
};
}  // namespace ark::irtoc

#endif  // PANDA_DWARF_BUILDER_H
