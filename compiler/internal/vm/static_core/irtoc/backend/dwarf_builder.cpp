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

#include "dwarf_builder.h"
#include "compiler/optimizer/ir/graph.h"
#include "compiler/optimizer/ir/basicblock.h"
#include "function.h"

#ifdef PANDA_COMPILER_DEBUG_INFO
#include <libdwarf/dwarf.h>
#endif

#include <numeric>

namespace ark::irtoc {
template <typename T>
static constexpr bool IsDwarfBadAddr(T v)
{
    return reinterpret_cast<Dwarf_Addr>(v) == DW_DLV_BADADDR;
}

// CC-OFFNXT(G.FUN.01-CPP) depend on Dwarf_Callback_Func by DWARF Producer Interface. Should be suppress
int DwarfBuilder::CreateSectionCallback([[maybe_unused]] char *name, [[maybe_unused]] int size,
                                        [[maybe_unused]] Dwarf_Unsigned type, [[maybe_unused]] Dwarf_Unsigned flags,
                                        [[maybe_unused]] Dwarf_Unsigned link, [[maybe_unused]] Dwarf_Unsigned info,
                                        [[maybe_unused]] Dwarf_Unsigned *sectNameIndex, [[maybe_unused]] void *userData,
                                        [[maybe_unused]] int *error)
{
    auto self = reinterpret_cast<DwarfBuilder *>(userData);
    ELFIO::section *section = self->GetElfBuilder()->sections.add(name);
    if (strncmp(name, ".rel", 4U) == 0) {
        self->relMap_.insert(
            {section->get_index(), ELFIO::relocation_section_accessor(*self->GetElfBuilder(), section)});
        section->set_entry_size(self->GetElfBuilder()->get_default_entry_size(ELFIO::SHT_REL));
    }
    self->indexMap_[section->get_index()] = self->sections_.size();
    self->sections_.push_back(section);

    section->set_type(type);
    section->set_flags(flags);
    section->set_link(3U);
    section->set_info(info);

    return section->get_index();
}

DwarfBuilder::DwarfBuilder(Arch arch, ELFIO::elfio *elf) : elfBuilder_(elf), arch_(arch)
{
    auto toU {[](int n) { return static_cast<Dwarf_Unsigned>(n); }};

    Dwarf_Error error {nullptr};
    dwarf_producer_init(toU(DW_DLC_WRITE) | toU(DW_DLC_POINTER64) | toU(DW_DLC_OFFSET32) | toU(DW_DLC_SIZE_64) |
                            toU(DW_DLC_SYMBOLIC_RELOCATIONS) | toU(DW_DLC_TARGET_LITTLEENDIAN),
                        reinterpret_cast<Dwarf_Callback_Func>(DwarfBuilder::CreateSectionCallback), nullptr, nullptr,
                        this, GetIsaName(GetArch()), "V4", nullptr, &dwarf_, &error);
    if (error != DW_DLV_OK) {
        LOG(FATAL, COMPILER) << "Dwarf initialization failed: " << dwarf_errmsg(error);
    }

    int res = dwarf_pro_set_default_string_form(dwarf_, DW_FORM_string, &error);
    if (res != DW_DLV_OK) {
        LOG(FATAL, COMPILER) << "dwarf_pro_set_default_string_form failed: " << dwarf_errmsg(error);
    }

    compileUnitDie_ = dwarf_new_die(dwarf_, DW_TAG_compile_unit, nullptr, nullptr, nullptr, nullptr, &error);
    if (error != DW_DLV_OK || IsDwarfBadAddr(compileUnitDie_)) {
        LOG(FATAL, COMPILER) << "dwarf_new_die failed: " << dwarf_errmsg(error);
    }

    dwarf_add_AT_producer(compileUnitDie_, const_cast<char *>("ASD"), &error);
    if (error != DW_DLV_OK) {
        LOG(FATAL, COMPILER) << "dwarf_add_AT_producer failed: " << dwarf_errmsg(error);
    }

    dwarf_add_die_to_debug(dwarf_, compileUnitDie_, &error);
    if (error != DW_DLV_OK) {
        LOG(FATAL, COMPILER) << "dwarf_add_die_to_debug_a failed: " << dwarf_errmsg(error);
    }

    dwarf_pro_set_default_string_form(dwarf_, DW_FORM_strp, &error);

    dwarf_add_AT_unsigned_const(dwarf_, compileUnitDie_, DW_AT_language, DW_LANG_C, &error);
    if (error != DW_DLV_OK) {
        LOG(FATAL, COMPILER) << "DW_AT_language failed: " << dwarf_errmsg(error);
    }
}

bool DwarfBuilder::BuildGraphNestedFunction(unsigned symbol, const Function *func, Dwarf_Error &error, Dwarf_P_Die &die)
{
    auto graph = func->GetGraph();

    auto sourceDirs {func->GetSourceDirs()};
    auto sourceFiles {func->GetSourceFiles()};

    if (dwarf_lne_set_address(dwarf_, 0, symbol, &error) != DW_DLV_OK) {
        LOG(FATAL, COMPILER) << "dwarf_lne_set_address failed: " << dwarf_errmsg(error);
    }

    for (auto bb : *graph) {
        if (bb == nullptr) {
            continue;
        }
        for (auto inst : bb->Insts()) {
            auto debugInfo = inst->GetDebugInfo();
            if (debugInfo == nullptr) {
                continue;
            }

            auto dirIndex {AddDir(sourceDirs[debugInfo->GetDirIndex()])};
            auto fileIndex {AddFile(sourceFiles[debugInfo->GetFileIndex()], dirIndex)};
            if (dwarf_add_line_entry(dwarf_, fileIndex, debugInfo->GetOffset(), debugInfo->GetLineNumber(), 0, 1, 0,
                                     &error) != DW_DLV_OK) {
                LOG(ERROR, COMPILER) << "dwarf_add_line_entry failed: " << dwarf_errmsg(error);
                return false;
            }
        }
    }

    if (dwarf_lne_end_sequence(dwarf_, graph->GetCode().size(), &error) != DW_DLV_OK) {
        LOG(ERROR, COMPILER) << "dwarf_lne_end_sequence failed: " << dwarf_errmsg(error);
        return false;
    }

    auto attr = dwarf_add_AT_targ_address_b(dwarf_, die, DW_AT_low_pc, 0, symbol, &error);
    if (error != DW_DLV_OK || IsDwarfBadAddr(attr)) {
        LOG(ERROR, COMPILER) << "dwarf_add_AT_targ_address_b(DW_AT_low_pc) failed: " << dwarf_errmsg(error);
        return false;
    }

    attr = dwarf_add_AT_targ_address_b(dwarf_, die, DW_AT_high_pc, graph->GetCode().size(), symbol, &error);
    if (error != DW_DLV_OK || IsDwarfBadAddr(attr)) {
        LOG(ERROR, COMPILER) << "dwarf_add_AT_targ_address_b(DW_AT_high_pc) failed: " << dwarf_errmsg(error);
        return false;
    }

    return true;
}

bool DwarfBuilder::BuildGraph(const Function *func, uint32_t codeOffset, unsigned symbol)
{
    auto graph = func->GetGraph();
    if (!graph->IsLineDebugInfoEnabled()) {
        return true;
    }

    Dwarf_Error error = nullptr;
    auto die = dwarf_new_die(dwarf_, DW_TAG_subprogram, nullptr, nullptr, nullptr, nullptr, &error);
    if (error != DW_DLV_OK || IsDwarfBadAddr(die)) {
        LOG(ERROR, COMPILER) << "dwarf_new_die failed: " << dwarf_errmsg(error);
        return false;
    }

    Dwarf_P_Die res;
    if (prevProgramDie_ != nullptr) {
        res = dwarf_die_link(die, nullptr, nullptr, prevProgramDie_, nullptr, &error);
    } else {
        res = dwarf_die_link(die, compileUnitDie_, nullptr, nullptr, nullptr, &error);
    }
    if (error != DW_DLV_OK || IsDwarfBadAddr(res)) {
        LOG(ERROR, COMPILER) << "dwarf_die_link failed: " << dwarf_errmsg(error);
    }
    prevProgramDie_ = die;

    Dwarf_P_Attribute a = dwarf_add_AT_name(die, const_cast<char *>(func->GetName()), &error);
    if (error != DW_DLV_OK || IsDwarfBadAddr(a)) {
        LOG(ERROR, COMPILER) << "dwarf_add_AT_targ_address_b failed: " << dwarf_errmsg(error);
    }

    a = dwarf_add_AT_flag(dwarf_, die, DW_AT_external, static_cast<Dwarf_Small>(true), &error);
    if (error != DW_DLV_OK || IsDwarfBadAddr(a)) {
        LOG(ERROR, COMPILER) << "dwarf_add_AT_flag failed: " << dwarf_errmsg(error);
    }

    if (!BuildGraphNestedFunction(symbol, func, error, die)) {
        return false;
    }

    codeSize_ = codeOffset + graph->GetCode().size();
    return true;
}

bool DwarfBuilder::FinalizeNestedFunction(Dwarf_Error &error)
{
    Dwarf_Unsigned relSectionsCount;
    int bufVersion;
    auto res = dwarf_get_relocation_info_count(dwarf_, &relSectionsCount, &bufVersion, &error);
    if (res != DW_DLV_OK) {
        LOG(ERROR, COMPILER) << "dwarf_get_relocation_info_count failed: " << dwarf_errmsg(error);
        return false;
    }
    for (Dwarf_Unsigned i = 0; i < relSectionsCount; ++i) {
        Dwarf_Signed elfIndex = 0;
        Dwarf_Signed linkIndex = 0;
        Dwarf_Unsigned bufCount;
        Dwarf_Relocation_Data data;
        if (dwarf_get_relocation_info(dwarf_, &elfIndex, &linkIndex, &bufCount, &data, &error) != DW_DLV_OK) {
            LOG(ERROR, COMPILER) << "dwarf_get_relocation_info failed: " << dwarf_errmsg(error);
            return false;
        }
        auto &relWriter = relMap_.at(elfIndex);
        for (Dwarf_Unsigned j = 0; j < bufCount; j++, data++) {
            if (data->drd_symbol_index == 0) {
                continue;
            }
            relWriter.add_entry(data->drd_offset,
                                // NOLINTNEXTLINE (hicpp-signed-bitwise)
                                static_cast<ELFIO::Elf_Xword>(ELF64_R_INFO(data->drd_symbol_index, data->drd_type)));
        }
    }

    if (dwarf_producer_finish(dwarf_, &error) != DW_DLV_OK) {
        LOG(ERROR, COMPILER) << "dwarf_producer_finish failed: " << dwarf_errmsg(error);
        return false;
    }

    return true;
}

bool DwarfBuilder::Finalize(uint32_t codeSize)
{
    Dwarf_Error error {nullptr};

    auto attr = dwarf_add_AT_targ_address_b(dwarf_, compileUnitDie_, DW_AT_low_pc, 0, 0, &error);
    if (error != DW_DLV_OK || IsDwarfBadAddr(attr)) {
        LOG(ERROR, COMPILER) << "dwarf_add_AT_targ_address_b(DW_AT_low_pc) failed: " << dwarf_errmsg(error);
        return false;
    }

    attr = dwarf_add_AT_targ_address_b(dwarf_, compileUnitDie_, DW_AT_high_pc, codeSize, 0, &error);
    if (error != DW_DLV_OK || IsDwarfBadAddr(attr)) {
        LOG(ERROR, COMPILER) << "dwarf_add_AT_targ_address_b(DW_AT_high_pc) failed: " << dwarf_errmsg(error);
        return false;
    }

    auto sections = dwarf_transform_to_disk_form(dwarf_, &error);
    if (error != DW_DLV_OK || sections == DW_DLV_NOCOUNT) {
        LOG(ERROR, COMPILER) << "dwarf_transform_to_disk_form failed: " << dwarf_errmsg(error);
        return false;
    }

    for (decltype(sections) i = 0; i < sections; ++i) {
        Dwarf_Unsigned len = 0;
        Dwarf_Signed elfIdx = 0;
        auto bytes = reinterpret_cast<const char *>(dwarf_get_section_bytes(dwarf_, i, &elfIdx, &len, &error));
        ASSERT(error == DW_DLV_OK);
        (void)bytes;
        sections_[indexMap_[elfIdx]]->append_data(bytes, len);
    }

    return FinalizeNestedFunction(error);
}

Dwarf_Unsigned DwarfBuilder::AddFile(const std::string &fname, Dwarf_Unsigned dirIndex)
{
    if (auto it = sourceFilesMap_.find(fname); it != sourceFilesMap_.end()) {
        return it->second;
    }
    Dwarf_Error error {nullptr};
    Dwarf_Unsigned index = dwarf_add_file_decl(dwarf_, const_cast<char *>(fname.data()), dirIndex, 0U, 0U, &error);
    if (index == static_cast<Dwarf_Unsigned>(DW_DLV_NOCOUNT)) {
        LOG(FATAL, COMPILER) << "dwarf_add_file_decl failed: " << dwarf_errmsg(error);
    }
    sourceFilesMap_[fname] = index;
    return index;
}

Dwarf_Unsigned DwarfBuilder::AddDir(const std::string &dname)
{
    if (auto it = dirsMap_.find(dname); it != dirsMap_.end()) {
        return it->second;
    }
    Dwarf_Error error {nullptr};
    Dwarf_Unsigned index = dwarf_add_directory_decl(dwarf_, const_cast<char *>(dname.data()), &error);
    if (index == static_cast<Dwarf_Unsigned>(DW_DLV_NOCOUNT)) {
        LOG(FATAL, COMPILER) << "dwarf_add_file_decl failed: " << dwarf_errmsg(error);
    }
    dirsMap_[dname] = index;
    return index;
}
}  // namespace ark::irtoc
