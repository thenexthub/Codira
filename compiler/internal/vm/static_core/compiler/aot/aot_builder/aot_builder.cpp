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

#include "aot_builder.h"
#include "aot/aot_file.h"
#include "elf_builder.h"
#include "include/class.h"
#include "include/method.h"
#include "optimizer/code_generator/encode.h"
#include "code_info/code_info.h"

#include <numeric>

namespace ark::compiler {

/// Fills text section in the ELF builder by the code from the methods in AotBuilder.
class CodeDataProvider : public ElfSectionDataProvider {
public:
    explicit CodeDataProvider(AotBuilder *aotBuilder) : aotBuilder_(aotBuilder) {}

    void FillData(Span<uint8_t> stream, size_t streamBegin) const override
    {
        const size_t codeOffset = CodeInfo::GetCodeOffset(aotBuilder_->GetArch());
        CodePrefix prefix;
        size_t currPos;
        for (size_t i = 0; i < aotBuilder_->methods_.size(); i++) {
            auto &method = aotBuilder_->methods_[i];
            auto &methodHeader = aotBuilder_->methodHeaders_[i];
            prefix.codeSize = method.GetCode().size();
            prefix.codeInfoOffset = codeOffset + RoundUp(method.GetCode().size(), CodeInfo::ALIGNMENT);
            prefix.codeInfoSize = method.GetCodeInfo().size();
            // Prefix
            currPos = streamBegin + methodHeader.codeOffset;
            const char *data = reinterpret_cast<char *>(&prefix);
            CopyToSpan(stream, data, sizeof(prefix), currPos);
            currPos += sizeof(prefix);

            // Code
            currPos += codeOffset - sizeof(prefix);
            data = reinterpret_cast<const char *>(method.GetCode().data());
            CopyToSpan(stream, data, method.GetCode().size(), currPos);
            currPos += method.GetCode().size();

            // CodeInfo
            currPos += RoundUp(method.GetCode().size(), CodeInfo::ALIGNMENT) - method.GetCode().size();
            data = reinterpret_cast<const char *>(method.GetCodeInfo().data());
            CopyToSpan(stream, data, method.GetCodeInfo().size(), currPos);
        }
    }

    size_t GetDataSize() const override
    {
        return aotBuilder_->currentCodeSize_;
    }

private:
    AotBuilder *aotBuilder_;
};

void AotBuilder::StartFile(const std::string &name, uint32_t checksum)
{
    auto &fileHeader = fileHeaders_.emplace_back();
    fileHeader.classesOffset = classHeaders_.size();
    fileHeader.fileChecksum = checksum;
    fileHeader.fileOffset = 0;
    fileHeader.fileNameStr = AddString(name);
    fileHeader.methodsOffset = methodHeaders_.size();
}

void AotBuilder::EndFile()
{
    ASSERT(!fileHeaders_.empty());
    auto &fileHeader = fileHeaders_.back();
    fileHeader.classesCount = classHeaders_.size() - fileHeader.classesOffset;
    if (fileHeader.classesCount == 0 && (classHashTablesSize_.empty() || classHashTablesSize_.back() == 0)) {
        /* Just return, if there is nothing compiled in the file */
        CHECK_EQ(fileHeader.methodsCount, 0U);
        fileHeaders_.pop_back();
        return;
    }
    ASSERT(!classHashTablesSize_.empty());
    fileHeader.classHashTableOffset =
        (entityPairHeaders_.size() - classHashTablesSize_.back()) * sizeof(panda_file::EntityPairHeader);
    fileHeader.classHashTableSize = classHashTablesSize_.back();
    fileHeader.methodsCount = methodHeaders_.size() - fileHeader.methodsOffset;
    // We should keep class headers sorted, since AOT manager uses binary search to find classes.
    std::sort(classHeaders_.begin() + fileHeader.classesOffset, classHeaders_.end(),
              [](const auto &a, const auto &b) { return a.classId < b.classId; });
}

int AotBuilder::Write(const std::string &cmdline, const std::string &fileName)
{
    switch (arch_) {
        case Arch::AARCH32:
            return WriteImpl<Arch::AARCH32>(cmdline, fileName);
        case Arch::AARCH64:
            return WriteImpl<Arch::AARCH64>(cmdline, fileName);
        case Arch::X86:
            return WriteImpl<Arch::X86>(cmdline, fileName);
        case Arch::X86_64:
            return WriteImpl<Arch::X86_64>(cmdline, fileName);
        default:
            LOG(ERROR, COMPILER) << "AotBuilder: Unsupported arch";
            return 1;
    }
}

void AotBuilder::FillHeader(const std::string &cmdline, const std::string &fileName)
{
    aotHeader_.magic = compiler::AotFile::MAGIC;
    aotHeader_.version = compiler::AotFile::VERSION;
    aotHeader_.checksum = 0;  // NOTE(msherstennikov)
    ASSERT(GetRuntime() != nullptr);
    aotHeader_.environmentChecksum = GetRuntime()->GetEnvironmentChecksum(arch_);
    aotHeader_.arch = static_cast<uint32_t>(arch_);
    aotHeader_.gcType = gcType_;
    aotHeader_.filesOffset = sizeof(aotHeader_);
    aotHeader_.filesCount = fileHeaders_.size();
    aotHeader_.classHashTablesOffset =
        aotHeader_.filesOffset + aotHeader_.filesCount * sizeof(compiler::PandaFileHeader);
    size_t classHashTablesSize = entityPairHeaders_.size() * sizeof(panda_file::EntityPairHeader);
    aotHeader_.classesOffset = aotHeader_.classHashTablesOffset + classHashTablesSize;
    aotHeader_.methodsOffset = aotHeader_.classesOffset + classHeaders_.size() * sizeof(compiler::ClassHeader);
    aotHeader_.bitmapOffset = aotHeader_.methodsOffset + methods_.size() * sizeof(compiler::MethodHeader);
    size_t bitmapsSize =
        std::accumulate(classMethodsBitmaps_.begin(), classMethodsBitmaps_.end(), 0U,
                        [](size_t sum, const auto &vec) { return vec.GetContainerSizeInBytes() + sum; });
    aotHeader_.strtabOffset = aotHeader_.bitmapOffset + bitmapsSize;
    aotHeader_.fileNameStr = AddString(fileName);
    aotHeader_.cmdlineStr = AddString(cmdline);
    aotHeader_.bootAot = static_cast<uint32_t>(bootAot_);
    aotHeader_.withCha = static_cast<uint32_t>(withCha_);
    aotHeader_.classCtxStr = AddString(classCtx_);
}

template <Arch ARCH>
int AotBuilder::WriteImpl(const std::string &cmdline, const std::string &fileName)
{
    ElfBuilder<ARCH> builder;

    PrepareElfBuilder(builder, cmdline, fileName);
    builder.Build(fileName);
    builder.Write(fileName);

    return 0;
}

template <Arch ARCH>
int AotBuilder::PrepareElfBuilder(ElfBuilder<ARCH> &builder, const std::string &cmdline, const std::string &fileName)
{
    constexpr size_t PAGE_SIZE_BYTES = 0x1000;
    constexpr size_t CALL_STATIC_SLOT_SIZE = 3;
    constexpr size_t CALL_VIRTUAL_SLOT_SIZE = 2;
    constexpr size_t STRING_SLOT_SIZE = 2;
    constexpr size_t INLINE_CACHE_SLOT_SIZE = 1;
    constexpr size_t COMMON_SLOT_SIZE = 1;

    auto codeProvider = std::make_unique<CodeDataProvider>(this);
    builder.GetTextSection()->SetDataProvider(std::move(codeProvider));

    builder.PreSizeRoDataSections(roDatas_.size());
    for (const auto &roData : roDatas_) {
        builder.AddRoDataSection(roData.name, roData.alignment);
    }
    auto roDataSections = builder.GetRoDataSections();
    auto aotSection = builder.GetAotSection();
    auto gotSection = builder.GetGotSection();
    std::vector<uint8_t> &gotData = gotSection->GetVector();
    // +1 is the extra slot that indicates the end of the aot table
    auto gotDataSize = static_cast<size_t>(RuntimeInterface::IntrinsicId::COUNT) + 1 +
                       CALL_STATIC_SLOT_SIZE * (gotPlt_.size() + gotClass_.size()) +
                       CALL_VIRTUAL_SLOT_SIZE * gotVirtIndexes_.size() + STRING_SLOT_SIZE * gotString_.size() +
                       INLINE_CACHE_SLOT_SIZE * gotIntfInlineCache_.size() + COMMON_SLOT_SIZE * gotCommon_.size();
    // We need to fill the whole segment with aot_got section because it is filled from the end.
    gotData.resize(RoundUp(PointerSize(ARCH) * gotDataSize, PAGE_SIZE_BYTES), 0);

    GenerateSymbols(builder);

    FillHeader(cmdline, fileName);

    aotSection->AppendData(&aotHeader_, sizeof(aotHeader_));
    aotSection->AppendData(fileHeaders_.data(), fileHeaders_.size() * sizeof(compiler::PandaFileHeader));
    aotSection->AppendData(entityPairHeaders_.data(), entityPairHeaders_.size() * sizeof(panda_file::EntityPairHeader));
    aotSection->AppendData(classHeaders_.data(), classHeaders_.size() * sizeof(compiler::ClassHeader));
    aotSection->AppendData(methodHeaders_.data(), methodHeaders_.size() * sizeof(compiler::MethodHeader));

    for (auto &bitmap : classMethodsBitmaps_) {
        aotSection->AppendData(bitmap.data(), bitmap.GetContainerSizeInBytes());
    }
    aotSection->AppendData(stringTable_.data(), stringTable_.size());

    for (size_t i = 0; i < roDatas_.size(); i++) {
        auto &section = roDatas_.at(i);
        auto dataSection = roDataSections->at(i);
        ASSERT(section.name == dataSection->GetName());
        dataSection->AppendData(section.content.data(), section.content.size());
    }

    using PtrType = typename ArchTraits<ARCH>::WordType;
    auto ptrView = Span(gotData).template SubSpan<PtrType>(0, gotData.size() / sizeof(PtrType));
    EmitPlt<ARCH>(ptrView, gotDataSize);

#ifdef PANDA_COMPILER_DEBUG_INFO
    builder.SetFrameData(&frameData_);
#endif

    return 0;
}
template int AotBuilder::PrepareElfBuilder<Arch::X86>(ElfBuilder<Arch::X86> &builder, const std::string &cmdline,
                                                      const std::string &file_name);
template int AotBuilder::PrepareElfBuilder<Arch::X86_64>(ElfBuilder<Arch::X86_64> &builder, const std::string &cmdline,
                                                         const std::string &file_name);
template int AotBuilder::PrepareElfBuilder<Arch::AARCH32>(ElfBuilder<Arch::AARCH32> &builder,
                                                          const std::string &cmdline, const std::string &file_name);
template int AotBuilder::PrepareElfBuilder<Arch::AARCH64>(ElfBuilder<Arch::AARCH64> &builder,
                                                          const std::string &cmdline, const std::string &file_name);

static inline uint64_t PackFileEntityId(RuntimeInterface *runtime,
                                        const std::pair<const panda_file::File *, uint32_t> &gotItem, bool isExternal)
{
    auto [pf, entityIdOffset] = gotItem;
    if (!isExternal || !g_options.IsCompilerInlineExternalMethods() ||
        !g_options.IsCompilerInlineExternalMethodsAot()) {
        return entityIdOffset;
    }
    auto pandaFileIndex = runtime->GetAOTBinaryFileSnapshotIndex(
        const_cast<RuntimeInterface::BinaryFilePtr>(reinterpret_cast<const void *>(pf)));
    return panda_file::File::PackFileEntityIdWithIndex(entityIdOffset, pandaFileIndex);
}

template <Arch ARCH>
void AotBuilder::EmitPlt(Span<typename ArchTraits<ARCH>::WordType> ptrView, size_t gotDataSize)
{
    if (!gotPlt_.empty() || !gotVirtIndexes_.empty() || !gotClass_.empty() || !gotString_.empty() ||
        !gotIntfInlineCache_.empty() || !gotCommon_.empty()) {
        ASSERT(PointerSize(ARCH) >= sizeof(uint32_t));

        auto ptrCnt = ptrView.Size();
        auto end = static_cast<size_t>(RuntimeInterface::IntrinsicId::COUNT);
        auto diff = static_cast<ssize_t>(ptrCnt - end);
        ptrView[ptrCnt - gotDataSize] = 0;
        constexpr size_t IMM_2 = 2;
        for (auto [method, value] : gotPlt_) {
            auto [idx, isExternal] = value;
            ASSERT(idx <= 0);
            ptrView[diff + idx] = AotFile::AotSlotType::PLT_SLOT;
            ptrView[diff + idx - IMM_2] = PackFileEntityId(runtime_, method, isExternal);
        }
        for (auto [method, value] : gotVirtIndexes_) {
            auto [idx, isExternal] = value;
            ASSERT(idx <= 0);
            ptrView[diff + idx] = AotFile::AotSlotType::VTABLE_INDEX;
            ptrView[diff + idx - 1] = PackFileEntityId(runtime_, method, isExternal);
        }
        for (auto [klass, value] : gotClass_) {
            auto [idx, isExternal] = value;
            ASSERT(idx <= 0);
            ptrView[diff + idx] = AotFile::AotSlotType::CLASS_SLOT;
            ptrView[diff + idx - IMM_2] = PackFileEntityId(runtime_, klass, isExternal);
        }
        for (auto [string_id, value] : gotString_) {
            auto [idx, isExternal] = value;
            ASSERT(idx <= 0);
            ptrView[diff + idx] = AotFile::AotSlotType::STRING_SLOT;
            ptrView[diff + idx - 1] = PackFileEntityId(runtime_, string_id, isExternal);
        }
        for (auto [cache, idx] : gotIntfInlineCache_) {
            (void)cache;
            ASSERT(idx < 0);
            ptrView[diff + idx] = AotFile::AotSlotType::INLINECACHE_SLOT;
        }
        for (auto [cache, idx] : gotCommon_) {
            (void)cache;
            ASSERT(idx < 0);
            ptrView[diff + idx] = AotFile::AotSlotType::COMMON_SLOT;
        }
    }
}

/// Add method names to the symbol table
template <Arch ARCH>
void AotBuilder::GenerateSymbols(ElfBuilder<ARCH> &builder)
{
    if (generateSymbols_) {
        auto textSection = builder.GetTextSection();
        std::string name;
        ASSERT(methods_.size() == methodHeaders_.size());
        for (size_t i = 0; i < methods_.size(); i++) {
            auto method = methods_.at(i).GetMethod();
            if (method->GetPandaFile() == nullptr) {
                name = "Error: method doesn't belong to any panda file";
            } else {
                auto methodCasted = reinterpret_cast<RuntimeInterface::MethodPtr>(method);
                name = runtime_->GetMethodFullName(methodCasted, true);
            }
            size_t offset = methodHeaders_[i].codeOffset;
            builder.template AddSymbol<true>(name, methodHeaders_[i].codeSize, *textSection, [offset, textSection]() {
                return textSection->GetAddress() + offset + CodeInfo::GetCodeOffset(ARCH);
            });
        }
    }
}

void AotBuilder::AddClassHashTable(const panda_file::File &pandaFile)
{
    const panda_file::File::Header *header = pandaFile.GetHeader();
    uint32_t numClasses = header->numClasses;
    if (numClasses == 0) {
        return;
    }

    size_t hashTableSize = ark::helpers::math::GetPowerOfTwoValue32(numClasses);
    std::vector<panda_file::EntityPairHeader> entityPairs;
    std::vector<unsigned int> conflictEntityTable;
    entityPairs.resize(hashTableSize);
    conflictEntityTable.resize(hashTableSize);
    size_t conflictNum = 0;

    auto classes = pandaFile.GetClasses();
    for (size_t i = 0; i < numClasses; ++i) {
        auto entityId = panda_file::File::EntityId(classes[i]);
        auto name = pandaFile.GetStringData(entityId).data;
        uint32_t hash = GetHash32String(name);
        uint32_t pos = hash & (hashTableSize - 1);
        auto &entityPair = entityPairs[pos];
        if (entityPair.descriptorHash == 0) {
            entityPair.descriptorHash = hash;
            entityPair.entityIdOffset = entityId.GetOffset();
        } else {
            conflictEntityTable[conflictNum] = i;
            conflictNum++;
        }
    }
    if (conflictNum == 0) {
        entityPairHeaders_.insert(entityPairHeaders_.end(), entityPairs.begin(), entityPairs.end());
        classHashTablesSize_.emplace_back(entityPairs.size());
    } else {
        ResolveConflictClassHashTable(pandaFile, std::move(conflictEntityTable), conflictNum, entityPairs);
    }
}

void AotBuilder::ResolveConflictClassHashTable(const panda_file::File &pandaFile,
                                               std::vector<unsigned int> conflictEntityTable, size_t conflictNum,
                                               std::vector<panda_file::EntityPairHeader> &entityPairs)
{
    auto classes = pandaFile.GetClasses();
    auto hashTableSize = entityPairs.size();
    for (size_t j = 0; j < conflictNum; ++j) {
        if (j > 0 && conflictEntityTable[j - 1] == conflictEntityTable[j]) {
            break;  // Exit for loop if there is no conlict elements anymore
        }
        auto i = conflictEntityTable[j];
        auto entityId = panda_file::File::EntityId(classes[i]);
        auto name = pandaFile.GetStringData(entityId).data;
        uint32_t hash = GetHash32String(name);
        uint32_t theoryPos = hash & (hashTableSize - 1);
        ASSERT(entityPairs[theoryPos].descriptorHash != 0);

        uint32_t actualPos = theoryPos;
        while (actualPos < (hashTableSize - 1) && entityPairs[actualPos].descriptorHash != 0) {
            actualPos++;
        }
        if (actualPos == (hashTableSize - 1) && entityPairs[actualPos].descriptorHash != 0) {
            actualPos = 0;
            while (actualPos < theoryPos && entityPairs[actualPos].descriptorHash != 0) {
                actualPos++;
            }
        }
        ASSERT(entityPairs[actualPos].descriptorHash == 0);
        auto &entityPair = entityPairs[actualPos];
        entityPair.descriptorHash = hash;
        entityPair.entityIdOffset = entityId.GetOffset();
        while (entityPairs[theoryPos].nextPos != 0) {
            theoryPos = entityPairs[theoryPos].nextPos - 1;
        }
        // add 1 is to distinguish the initial value 0 of next_pos and the situation that the next pos is really 0
        entityPairs[theoryPos].nextPos = actualPos + 1;
    }
    entityPairHeaders_.insert(entityPairHeaders_.end(), entityPairs.begin(), entityPairs.end());
    classHashTablesSize_.emplace_back(entityPairs.size());
}

}  // namespace ark::compiler
