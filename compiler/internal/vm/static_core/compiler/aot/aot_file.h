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

#ifndef COMPILER_AOT_AOT_FILE_H
#define COMPILER_AOT_AOT_FILE_H

#include "aot_headers.h"
#include "compiler/code_info/code_info.h"
#include "libarkbase/os/library_loader.h"
#include "libarkbase/utils/span.h"
#include "libarkfile/file.h"

#include <string>
#include <array>
#include <memory>
#include <algorithm>

namespace ark::compiler {
class RuntimeInterface;

class AotFile {
public:
    static constexpr std::array MAGIC = {'.', 'a', 'n', '\0'};
    static constexpr std::array VERSION = {'0', '0', '6', '\0'};

    enum AotSlotType {
        PLT_SLOT = 1,
        VTABLE_INDEX = 2,
        CLASS_SLOT = 3,
        STRING_SLOT = 4,
        INLINECACHE_SLOT = 5,
        COMMON_SLOT = 6,
        COUNT
    };

    AotFile(ark::os::library_loader::LibraryHandle &&handle, Span<const uint8_t> aotData, Span<const uint8_t> code)
        : handle_(std::move(handle)), aotData_(aotData), code_(code)
    {
    }

    NO_MOVE_SEMANTIC(AotFile);
    NO_COPY_SEMANTIC(AotFile);
    ~AotFile() = default;

public:
    static Expected<std::unique_ptr<AotFile>, std::string> Open(const std::string &fileName, uint32_t gcType,
                                                                bool forDump = false);

    const void *GetCode() const
    {
        return code_.data();
    }

    size_t GetCodeSize() const
    {
        return code_.size();
    }

    auto FileHeaders() const
    {
        return aotData_.SubSpan<const PandaFileHeader>(GetAotHeader()->filesOffset, GetFilesCount());
    }

    auto GetMethodHeader(size_t index) const
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return reinterpret_cast<const MethodHeader *>(&aotData_[GetAotHeader()->methodsOffset]) + index;
    }

    const MethodHeader *GetMethodHeadersPtr() const
    {
        return reinterpret_cast<const MethodHeader *>(&aotData_[GetAotHeader()->methodsOffset]);
    }

    auto GetClassHeaders(const PandaFileHeader &fileHeader) const
    {
        return aotData_.SubSpan<const ClassHeader>(
            GetAotHeader()->classesOffset + fileHeader.classesOffset * sizeof(ClassHeader), fileHeader.classesCount);
    }

    auto GetClassHashTable(const PandaFileHeader &fileHeader) const
    {
        return aotData_.SubSpan<const ark::panda_file::EntityPairHeader>(
            GetAotHeader()->classHashTablesOffset + fileHeader.classHashTableOffset, fileHeader.classHashTableSize);
    }

    const uint8_t *GetMethodsBitmap() const
    {
        return &aotData_[GetAotHeader()->bitmapOffset];
    }

    size_t GetFilesCount() const
    {
        return GetAotHeader()->filesCount;
    }

    const PandaFileHeader *FindPandaFile(const std::string &fileName) const
    {
        auto fileHeaders = FileHeaders();
        auto res = std::find_if(fileHeaders.begin(), fileHeaders.end(),
                                [this, &fileName](auto &header) { return fileName == GetString(header.fileNameStr); });
        return res == fileHeaders.end() ? nullptr : res;
    }

    const uint8_t *GetMethodCode(const MethodHeader *methodHeader) const
    {
        return code_.data() + methodHeader->codeOffset;
    }

    const char *GetString(size_t offset) const
    {
        return reinterpret_cast<const char *>(aotData_.data() + GetAotHeader()->strtabOffset + offset);
    }

    const AotHeader *GetAotHeader() const
    {
        return reinterpret_cast<const AotHeader *>(aotData_.data());
    }

    const char *GetFileName() const
    {
        return GetString(GetAotHeader()->fileNameStr);
    }

    const char *GetCommandLine() const
    {
        return GetString(GetAotHeader()->cmdlineStr);
    }

    const char *GetClassContext() const
    {
        return GetString(GetAotHeader()->classCtxStr);
    }

    bool IsCompiledWithCha() const
    {
        return GetAotHeader()->withCha != 0U;
    }

    bool IsBootPandaFile() const
    {
        return GetAotHeader()->bootAot != 0U;
    }

    void InitializeGot(RuntimeInterface *runtime);

    void PatchTable(RuntimeInterface *runtime);

private:
    ark::os::library_loader::LibraryHandle handle_ {nullptr};
    Span<const uint8_t> aotData_;
    Span<const uint8_t> code_;
};

class AotClass final {
public:
    AotClass() = default;
    AotClass(const AotFile *file, const ClassHeader *header) : aotFile_(file), header_(header) {}
    ~AotClass() = default;
    DEFAULT_COPY_SEMANTIC(AotClass);
    DEFAULT_MOVE_SEMANTIC(AotClass);

    const void *FindMethodCodeEntry(size_t index) const;
    Span<const uint8_t> FindMethodCodeSpan(size_t index) const;
    const MethodHeader *FindMethodHeader(size_t index) const;

    BitVectorSpan GetBitmap() const;

    auto GetMethodHeaders() const
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return Span(aotFile_->GetMethodHeadersPtr() + header_->methodsOffset, header_->methodsCount);
    }

    static AotClass Invalid()
    {
        return AotClass();
    }

    bool IsValid() const
    {
        return header_ != nullptr;
    }

private:
    const AotFile *aotFile_ {nullptr};
    const ClassHeader *header_ {nullptr};
};

class AotPandaFile {
public:
    AotPandaFile() = default;
    AotPandaFile(AotFile *file, const PandaFileHeader *header) : aotFile_(file), header_(header)
    {
        LoadClassHashTable();
    }

    DEFAULT_MOVE_SEMANTIC(AotPandaFile);
    DEFAULT_COPY_SEMANTIC(AotPandaFile);
    ~AotPandaFile() = default;

    const AotFile *GetAotFile() const
    {
        return aotFile_;
    }
    const PandaFileHeader *GetHeader()
    {
        return header_;
    }
    const PandaFileHeader *GetHeader() const
    {
        return header_;
    }
    std::string GetFileName() const
    {
        return GetAotFile()->GetString(GetHeader()->fileNameStr);
    }
    AotClass GetClass(uint32_t classId) const;

    Span<const ClassHeader> GetClassHeaders() const
    {
        return aotFile_->GetClassHeaders(*header_);
    }

    CodeInfo GetMethodCodeInfo(const MethodHeader *methodHeader) const
    {
        return CodeInfo(GetAotFile()->GetMethodCode(methodHeader), methodHeader->codeSize);
    }

    void LoadClassHashTable()
    {
        ASSERT(header_ != nullptr);
        classHashTable_ = GetAotFile()->GetClassHashTable(*header_);
    }

    ark::Span<const ark::panda_file::EntityPairHeader> GetClassHashTable() const
    {
        return classHashTable_;
    }

private:
    AotFile *aotFile_ {nullptr};
    const PandaFileHeader *header_ {nullptr};
    ark::Span<const ark::panda_file::EntityPairHeader> classHashTable_;
};
}  // namespace ark::compiler

#endif  // COMPILER_AOT_AOT_FILE_H
