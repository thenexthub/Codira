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

#ifndef COMPILER_AOT_AOT_BULDER_AOT_FILE_BUILDER_H
#define COMPILER_AOT_AOT_BULDER_AOT_FILE_BUILDER_H

#include "aot/compiled_method.h"
#include "aot/aot_file.h"
#include "elf_builder.h"
#include "libarkbase/utils/arch.h"
#include "libarkbase/utils/arena_containers.h"
#include "libarkbase/utils/bit_vector.h"
#include "optimizer/ir/runtime_interface.h"
#include <string>
#include <vector>
#include "mem/gc/gc_types.h"

namespace ark {
class Class;
}  // namespace ark

namespace ark::compiler {

template <Arch ARCH, bool IS_JIT_MODE>
class ElfBuilder;

struct RoData {
    std::vector<uint8_t> content;
    std::string name;
    size_t alignment;
};

class AotBuilder : public ElfWriter {
public:
    void SetGcType(uint32_t gcType)
    {
        gcType_ = gcType;
    }
    uint32_t GetGcType() const
    {
        return gcType_;
    }

    uint64_t *GetIntfInlineCacheIndex()
    {
        return &intfInlineCacheIndex_;
    }

    int Write(const std::string &cmdline, const std::string &fileName);

    void StartFile(const std::string &name, uint32_t checksum);
    void EndFile();

    void SetRoDataSections(std::vector<RoData> roDatas)
    {
        roDatas_ = std::move(roDatas);
    }

    auto *GetGotPlt()
    {
        return &gotPlt_;
    }

    auto *GetGotVirtIndexes()
    {
        return &gotVirtIndexes_;
    }

    auto *GetGotClass()
    {
        return &gotClass_;
    }

    auto *GetGotString()
    {
        return &gotString_;
    }

    auto *GetGotIntfInlineCache()
    {
        return &gotIntfInlineCache_;
    }

    auto *GetGotCommon()
    {
        return &gotCommon_;
    }

    void SetBootAot(bool bootAot)
    {
        bootAot_ = bootAot;
    }

    void SetWithCha(bool withCha)
    {
        withCha_ = withCha;
    }

    void SetGenerateSymbols(bool generateSymbols)
    {
        generateSymbols_ = generateSymbols;
    }

    void AddClassHashTable(const panda_file::File &pandaFile);

    void InsertEntityPairHeader(uint32_t classHash, uint32_t classId)
    {
        entityPairHeaders_.emplace_back();
        auto &entityPair = entityPairHeaders_.back();
        entityPair.descriptorHash = classHash;
        entityPair.entityIdOffset = classId;
    }

    auto *GetEntityPairHeaders() const
    {
        return &entityPairHeaders_;
    }

    void InsertClassHashTableSize(uint32_t size)
    {
        classHashTablesSize_.emplace_back(size);
    }

    auto *GetClassHashTableSize() const
    {
        return &classHashTablesSize_;
    }

protected:
    template <Arch ARCH>
    int PrepareElfBuilder(ElfBuilder<ARCH> &builder, const std::string &cmdline, const std::string &fileName);

private:
    template <Arch ARCH>
    int WriteImpl(const std::string &cmdline, const std::string &fileName);

    template <Arch ARCH>
    void GenerateSymbols(ElfBuilder<ARCH> &builder);

    template <Arch ARCH>
    void EmitPlt(Span<typename ArchTraits<ARCH>::WordType> ptrView, size_t gotDataSize);

    void FillHeader(const std::string &cmdline, const std::string &fileName);

    void ResolveConflictClassHashTable(const panda_file::File &pandaFile, std::vector<unsigned int> conflictEntityTable,
                                       size_t conflictNum, std::vector<panda_file::EntityPairHeader> &entityPairs);

private:
    std::string fileName_;
    compiler::AotHeader aotHeader_ {};
    uint32_t gcType_ {static_cast<uint32_t>(mem::GCType::INVALID_GC)};
    uint64_t intfInlineCacheIndex_ {0};
    std::vector<compiler::RoData> roDatas_;
    std::map<std::pair<const panda_file::File *, uint32_t>, std::pair<int32_t, bool>> gotPlt_;
    std::map<std::pair<const panda_file::File *, uint32_t>, std::pair<int32_t, bool>> gotVirtIndexes_;
    std::map<std::pair<const panda_file::File *, uint32_t>, std::pair<int32_t, bool>> gotClass_;
    std::map<std::pair<const panda_file::File *, uint32_t>, std::pair<int32_t, bool>> gotString_;
    std::map<std::pair<const panda_file::File *, uint64_t>, int32_t> gotIntfInlineCache_;
    std::map<std::pair<const panda_file::File *, uint64_t>, int32_t> gotCommon_;
    bool bootAot_ {false};
    bool withCha_ {true};
    bool generateSymbols_ {false};

    std::vector<panda_file::EntityPairHeader> entityPairHeaders_;
    std::vector<uint32_t> classHashTablesSize_;
    friend class CodeDataProvider;
    friend class JitCodeDataProvider;
};

}  // namespace ark::compiler

#endif  // COMPILER_AOT_AOT_BULDER_AOT_FILE_BUILDER_H
