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

#ifndef LIBPANDAFILE_FILE_H_
#define LIBPANDAFILE_FILE_H_

#include "libarkbase/os/mem.h"
#include "libarkbase/utils/span.h"
#include "libarkbase/utils/utf.h"
#include <cstdint>

#include <array>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>

namespace ark {
struct EntryFileStat;
}  // namespace ark

namespace ark::panda_file {

class PandaCache;

/*
 * EntityPairHeader Describes pair for hash value of class's descriptor and its entity id offset,
 * used to quickly find class by its descriptor.
 */
struct EntityPairHeader {
    uint32_t descriptorHash;
    uint32_t entityIdOffset;
    uint32_t nextPos;
};

class File {
public:
    using Index = uint16_t;
    using Index32 = uint32_t;

    static constexpr size_t MAGIC_SIZE = 8;
    static constexpr size_t VERSION_SIZE = 4;
    static const std::array<uint8_t, MAGIC_SIZE> MAGIC;

    struct Header {
        std::array<uint8_t, MAGIC_SIZE> magic;
        uint32_t checksum;
        std::array<uint8_t, VERSION_SIZE> version;
        uint32_t fileSize;
        uint32_t foreignOff;
        uint32_t foreignSize;
        uint32_t quickenedFlag;
        uint32_t numClasses;
        uint32_t classIdxOff;
        uint32_t numExportTable;
        uint32_t exportTableOff;
        uint32_t numLnps;
        uint32_t lnpIdxOff;
        uint32_t numLiteralarrays;
        uint32_t literalarrayIdxOff;
        uint32_t numIndexes;
        uint32_t indexSectionOff;
    };

    struct RegionHeader {
        uint32_t start;
        uint32_t end;
        uint32_t classIdxSize;
        uint32_t classIdxOff;
        uint32_t methodIdxSize;
        uint32_t methodIdxOff;
        uint32_t fieldIdxSize;
        uint32_t fieldIdxOff;
        uint32_t protoIdxSize;
        uint32_t protoIdxOff;
    };

    struct StringData {
        StringData(uint32_t len, const uint8_t *d) : utf16Length(len), isAscii(false), data(d) {}
        StringData() = default;
        uint32_t utf16Length;  // NOLINT(misc-non-private-member-variables-in-classes)
        bool isAscii;          // NOLINT(misc-non-private-member-variables-in-classes)
        const uint8_t *data;   // NOLINT(misc-non-private-member-variables-in-classes)
    };

    // NOLINTNEXTLINE(cppcoreguidelines-special-member-functions, hicpp-special-member-functions)
    class EntityId {
    public:
        explicit constexpr EntityId(uint32_t offset) : offset_(offset) {}

        EntityId() = default;

        ~EntityId() = default;

        bool IsValid() const
        {
            return offset_ > sizeof(Header);
        }

        uint32_t GetOffset() const
        {
            return offset_;
        }

        static constexpr size_t GetSize()
        {
            return sizeof(uint32_t);
        }

        friend bool operator<(const EntityId &l, const EntityId &r)
        {
            return l.offset_ < r.offset_;
        }

        friend bool operator==(const EntityId &l, const EntityId &r)
        {
            return l.offset_ == r.offset_;
        }

        friend bool operator!=(const EntityId &l, const EntityId &r)
        {
            return l.offset_ != r.offset_;
        }

        friend std::ostream &operator<<(std::ostream &stream, const EntityId &id)
        {
            return stream << id.offset_;
        }

    private:
        uint32_t offset_ {0};
    };

    enum OpenMode { READ_ONLY, READ_WRITE, WRITE_ONLY };

    StringData GetStringData(EntityId id) const;
    PANDA_PUBLIC_API EntityId GetLiteralArraysId() const;

    PANDA_PUBLIC_API EntityId GetClassId(const uint8_t *mutf8Name) const;

    EntityId GetClassIdFromClassHashTable(const uint8_t *mutf8Name) const;

    const Header *GetHeader() const
    {
        return reinterpret_cast<const Header *>(GetBase());
    }

    const uint8_t *GetBase() const
    {
        return reinterpret_cast<const uint8_t *>(base_.Get());
    }

    const os::mem::ConstBytePtr &GetPtr() const
    {
        return base_;
    }

    bool IsExternal(EntityId id) const
    {
        const Header *header = GetHeader();
        uint32_t foreignBegin = header->foreignOff;
        uint32_t foreignEnd = foreignBegin + header->foreignSize;
        return id.GetOffset() >= foreignBegin && id.GetOffset() < foreignEnd;
    }

    EntityId GetIdFromPointer(const uint8_t *ptr) const
    {
        return EntityId(ptr - GetBase());
    }

    Span<const uint8_t> GetSpanFromId(EntityId id) const
    {
        const Header *header = GetHeader();
        Span file(GetBase(), header->fileSize);
        return file.Last(file.size() - id.GetOffset());
    }

    Span<const uint32_t> GetClasses() const
    {
        const Header *header = GetHeader();
        Span file(GetBase(), header->fileSize);
        Span classIdxData = file.SubSpan(header->classIdxOff, header->numClasses * sizeof(uint32_t));
        return Span(reinterpret_cast<const uint32_t *>(classIdxData.data()), header->numClasses);
    }

    Span<const uint32_t> GetExported() const
    {
        const Header *header = GetHeader();
        Span file(GetBase(), header->fileSize);
        Span exportedIdxData = file.SubSpan(header->exportTableOff, header->numExportTable * sizeof(uint32_t));
        return Span(reinterpret_cast<const uint32_t *>(exportedIdxData.data()), header->numExportTable);
    }

    Span<const uint32_t> GetLiteralArrays() const
    {
        const Header *header = GetHeader();
        Span file(GetBase(), header->fileSize);
        Span litarrIdxData = file.SubSpan(header->literalarrayIdxOff, header->numLiteralarrays * sizeof(uint32_t));
        return Span(reinterpret_cast<const uint32_t *>(litarrIdxData.data()), header->numLiteralarrays);
    }

    Span<const RegionHeader> GetRegionHeaders() const
    {
        const Header *header = GetHeader();
        Span file(GetBase(), header->fileSize);
        auto sp = file.SubSpan(header->indexSectionOff, header->numIndexes * sizeof(RegionHeader));
        return Span(reinterpret_cast<const RegionHeader *>(sp.data()), header->numIndexes);
    }

    const RegionHeader *GetRegionHeader(EntityId id) const
    {
        auto headers = GetRegionHeaders();
        auto offset = id.GetOffset();
        for (const auto &header : headers) {
            if (header.start <= offset && offset < header.end) {
                return &header;
            }
        }
        return nullptr;
    }

    Span<const EntityId> GetClassIndex(const RegionHeader *regionHeader) const
    {
        auto *header = GetHeader();
        Span file(GetBase(), header->fileSize);
        ASSERT(regionHeader != nullptr);
        auto sp = file.SubSpan(regionHeader->classIdxOff, regionHeader->classIdxSize * EntityId::GetSize());
        return Span(reinterpret_cast<const EntityId *>(sp.data()), regionHeader->classIdxSize);
    }

    Span<const EntityId> GetClassIndex(EntityId id) const
    {
        auto *regionHeader = GetRegionHeader(id);
        ASSERT(regionHeader != nullptr);
        return GetClassIndex(regionHeader);
    }

    Span<const EntityId> GetMethodIndex(const RegionHeader *regionHeader) const
    {
        auto *header = GetHeader();
        Span file(GetBase(), header->fileSize);
        ASSERT(regionHeader != nullptr);
        auto sp = file.SubSpan(regionHeader->methodIdxOff, regionHeader->methodIdxSize * EntityId::GetSize());
        return Span(reinterpret_cast<const EntityId *>(sp.data()), regionHeader->methodIdxSize);
    }

    Span<const EntityId> GetMethodIndex(EntityId id) const
    {
        auto *regionHeader = GetRegionHeader(id);
        ASSERT(regionHeader != nullptr);
        return GetMethodIndex(regionHeader);
    }

    Span<const EntityId> GetFieldIndex(const RegionHeader *regionHeader) const
    {
        auto *header = GetHeader();
        Span file(GetBase(), header->fileSize);
        ASSERT(regionHeader != nullptr);
        auto sp = file.SubSpan(regionHeader->fieldIdxOff, regionHeader->fieldIdxSize * EntityId::GetSize());
        return Span(reinterpret_cast<const EntityId *>(sp.data()), regionHeader->fieldIdxSize);
    }

    Span<const EntityId> GetFieldIndex(EntityId id) const
    {
        auto *regionHeader = GetRegionHeader(id);
        ASSERT(regionHeader != nullptr);
        return GetFieldIndex(regionHeader);
    }

    Span<const EntityId> GetProtoIndex(const RegionHeader *regionHeader) const
    {
        auto *header = GetHeader();
        Span file(GetBase(), header->fileSize);
        ASSERT(regionHeader != nullptr);
        auto sp = file.SubSpan(regionHeader->protoIdxOff, regionHeader->protoIdxSize * EntityId::GetSize());
        return Span(reinterpret_cast<const EntityId *>(sp.data()), regionHeader->protoIdxSize);
    }

    Span<const EntityId> GetProtoIndex(EntityId id) const
    {
        auto *regionHeader = GetRegionHeader(id);
        ASSERT(regionHeader != nullptr);
        return GetProtoIndex(regionHeader);
    }

    Span<const EntityId> GetLineNumberProgramIndex() const
    {
        const Header *header = GetHeader();
        Span file(GetBase(), header->fileSize);
        Span lnpIdxData = file.SubSpan(header->lnpIdxOff, header->numLnps * EntityId::GetSize());
        return Span(reinterpret_cast<const EntityId *>(lnpIdxData.data()), header->numLnps);
    }

    EntityId ResolveClassIndex(EntityId id, Index idx) const
    {
        auto index = GetClassIndex(id);
        return index[idx];
    }

    EntityId ResolveMethodIndex(EntityId id, Index idx) const
    {
        auto index = GetMethodIndex(id);
        return index[idx];
    }

    EntityId ResolveFieldIndex(EntityId id, Index idx) const
    {
        auto index = GetFieldIndex(id);
        return index[idx];
    }

    EntityId ResolveProtoIndex(EntityId id, Index idx) const
    {
        auto index = GetProtoIndex(id);
        return index[idx];
    }

    EntityId ResolveLineNumberProgramIndex(Index32 idx) const
    {
        auto index = GetLineNumberProgramIndex();
        return index[idx];
    }

    const std::string &GetFilename() const
    {
        return filename_;
    }

    PandaCache *GetPandaCache() const
    {
        return pandaCache_.get();
    }

    uint32_t GetFilenameHash() const
    {
        return filenameHash_;
    }

    // note: intentionally returns uint64_t instead of the field type due to usage
    uint64_t GetUniqId() const
    {
        return uniqId_;
    }

    const std::string &GetFullFileName() const
    {
        return fullFilename_;
    }

    static constexpr uint32_t GetFileBaseOffset()
    {
        return MEMBER_OFFSET(File, base_);
    }

    Span<const ark::panda_file::EntityPairHeader> GetClassHashTable() const
    {
        return classHashTable_;
    }

    std::string GetPaddedChecksum() const
    {
        std::stringstream paddedChecksum;
        // Length of hexed maximum unit32_t value of checksum (0xFFFFFFFF) is equal to 8
        constexpr size_t CHECKSUM_LENGTH = 8;
        paddedChecksum << std::setfill('0') << std::setw(CHECKSUM_LENGTH) << std::hex << GetHeader()->checksum;
        return paddedChecksum.str();
    }

    static uint32_t CalcFilenameHash(const std::string &filename);

    PANDA_PUBLIC_API static std::unique_ptr<const File> Open(std::string_view filename, OpenMode openMode = READ_ONLY);

    PANDA_PUBLIC_API static std::unique_ptr<const File> OpenFromMemory(os::mem::ConstBytePtr &&ptr);

    static std::unique_ptr<const File> OpenFromMemory(os::mem::ConstBytePtr &&ptr, std::string_view filename);

    static std::unique_ptr<const File> OpenUncompressedArchive(int fd, const std::string_view &filename, size_t size,
                                                               uint32_t offset, OpenMode openMode = READ_ONLY);

    void SetClassHashTable(ark::Span<const ark::panda_file::EntityPairHeader> classHashTable) const
    {
        classHashTable_ = classHashTable;
    }

    static constexpr uint32_t FILE_INDEX_MASK = UINT32_MAX;
    static constexpr uint32_t FILE_INDEX_SHIFT = 32;
    static constexpr uint32_t INVALID_FILE_INDEX = 0;
    static constexpr uint32_t FILE_INDEX_BASE_OFFSET = 1;

    static constexpr uint64_t PackFileEntityIdWithIndex(uint32_t entityIdOffset, uint32_t index)
    {
        return entityIdOffset | (static_cast<uint64_t>(index) & FILE_INDEX_MASK) << FILE_INDEX_SHIFT;
    }

    static constexpr std::pair<File::EntityId, uint32_t> UnpackFileEntityIdWithIndex(
        uint64_t packedFileEntityIdWithIndex)
    {
        return {EntityId(static_cast<uint32_t>(packedFileEntityIdWithIndex)),
                static_cast<uint32_t>((packedFileEntityIdWithIndex >> FILE_INDEX_SHIFT) & FILE_INDEX_MASK)};
    }

    static constexpr bool IsFileEntityIdWithIndex(uint64_t value)
    {
        return ((value >> FILE_INDEX_SHIFT) & FILE_INDEX_MASK) != INVALID_FILE_INDEX;
    }

    PANDA_PUBLIC_API ~File();

    NO_COPY_SEMANTIC(File);
    NO_MOVE_SEMANTIC(File);

private:
    File(std::string filename, os::mem::ConstBytePtr &&base);

    os::mem::ConstBytePtr base_;
    const std::string filename_;
    const uint32_t filenameHash_;
    const std::string fullFilename_;
    std::unique_ptr<PandaCache> pandaCache_;
    const uint32_t uniqId_;
    mutable ark::Span<const ark::panda_file::EntityPairHeader> classHashTable_;
};

static_assert(File::GetFileBaseOffset() == 0);

inline bool operator==(const File::StringData &stringData1, const File::StringData &stringData2)
{
    if (stringData1.utf16Length != stringData2.utf16Length) {
        return false;
    }

    return utf::IsEqual(stringData1.data, stringData2.data);
}

inline bool operator!=(const File::StringData &stringData1, const File::StringData &stringData2)
{
    return !(stringData1 == stringData2);
}

inline bool operator<(const File::StringData &stringData1, const File::StringData &stringData2)
{
    if (stringData1.utf16Length == stringData2.utf16Length) {
        return utf::CompareMUtf8ToMUtf8(stringData1.data, stringData2.data) < 0;
    }

    return stringData1.utf16Length < stringData2.utf16Length;
}

/*
 * OpenPandaFileOrZip from location which specicify the name.
 */
std::unique_ptr<const File> OpenPandaFileOrZip(std::string_view location,
                                               panda_file::File::OpenMode openMode = panda_file::File::READ_ONLY);

/*
 * OpenPandaFileFromMemory from file buffer.
 */
std::unique_ptr<const File> OpenPandaFileFromMemory(const void *buffer, size_t size, std::string tag = "");

/*
 * OpenPandaFileFromMemory from secure buffer.
 */
std::unique_ptr<const File> OpenPandaFileFromSecureMemory(uint8_t *buffer, size_t size, std::string filename = "");

/*
 * OpenZipPandaFile from the location zip.
 */
std::unique_ptr<const File> OpenZipPandaFile(FILE *fp, std::string_view location, std::string_view archiveFilename,
                                             panda_file::File::OpenMode openMode = panda_file::File::READ_ONLY);

/*
 * OpenPandaFile from location which specicify the name.
 */
PANDA_PUBLIC_API std::unique_ptr<const File> OpenPandaFile(
    std::string_view location, std::string_view archiveFilename = "",
    panda_file::File::OpenMode openMode = panda_file::File::READ_ONLY);

/*
 * Check ptr point valid panda file: checksum
 */
bool ValidateChecksum(const os::mem::ConstBytePtr &ptr, const std::string_view &filename = "");

/*
 * Check ptr point valid panda file: magic
 */
bool CheckHeader(const os::mem::ConstBytePtr &ptr, const std::string_view &filename = "",
                 const size_t &expectedLength = 0);

// NOLINTNEXTLINE(readability-identifier-naming)
extern const char *ARCHIVE_FILENAME;
}  // namespace ark::panda_file

namespace std {
template <>
struct hash<ark::panda_file::File::EntityId> {
    std::size_t operator()(ark::panda_file::File::EntityId id) const
    {
        return std::hash<uint32_t> {}(id.GetOffset());
    }
};
}  // namespace std

#endif
