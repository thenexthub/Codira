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

#ifndef LIBPANDAFILE_FILE_H
#define LIBPANDAFILE_FILE_H

#include <array>
#include <cstdint>
#include <fcntl.h>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>

#include "helpers.h"
#include "os/mem.h"
#include "os/filesystem.h"
#include "utils/span.h"
#include "utils/utf.h"
#include "utils/logger.h"

namespace panda {
struct EntryFileStat;
}  // namespace panda

namespace panda::panda_file {

class PandaCache;

#define XPM_PROC_LENGTH 50
#define PROC_SELF_XPM_REGION_PATH "/proc/self/xpm_region"
/*
 * EntityPairHeader Describes pair for hash value of class's descriptor and its entity id offset,
 * used to quickly find class by its descriptor.
 */
struct EntityPairHeader {
    uint32_t descriptor_hash;
    uint32_t entity_id_offset;
    uint32_t next_pos;
};

class File {
public:
    using Index = uint16_t;
    using Index32 = uint32_t;

    static constexpr size_t MAGIC_SIZE = 8;
    static constexpr size_t VERSION_SIZE = 4;
    static const std::array<uint8_t, MAGIC_SIZE> MAGIC;
    static constexpr std::array<uint8_t, VERSION_SIZE> STATIC_VERSION = {0, 0, 0, 5};

    struct Header {
        std::array<uint8_t, MAGIC_SIZE> magic;
        uint32_t checksum;
        std::array<uint8_t, VERSION_SIZE> version;
        uint32_t file_size;
        uint32_t foreign_off;
        uint32_t foreign_size;
        uint32_t num_classes;
        uint32_t class_idx_off;
        uint32_t num_lnps;
        uint32_t lnp_idx_off;
        uint32_t num_literalarrays;
        uint32_t literalarray_idx_off;
        uint32_t num_indexes;
        uint32_t index_section_off;
    };

    struct IndexHeader {
        uint32_t start;
        uint32_t end;
        uint32_t class_idx_size;
        uint32_t class_idx_off;
        uint32_t method_idx_size;
        uint32_t method_idx_off;
        uint32_t field_idx_size;
        uint32_t field_idx_off;
        uint32_t proto_idx_size;
        uint32_t proto_idx_off;
    };

    struct StringData {
        StringData(uint32_t len, const uint8_t *d) : utf16_length(len), is_ascii(false), data(d) {}
        StringData() = default;
        uint32_t utf16_length;  // NOLINT(misc-non-private-member-variables-in-classes)
        bool is_ascii;          // NOLINT(misc-non-private-member-variables-in-classes)
        const uint8_t *data;    // NOLINT(misc-non-private-member-variables-in-classes)
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

    EntityId GetClassId(const uint8_t *mutf8_name) const;

    EntityId GetClassIdFromClassHashTable(const uint8_t *mutf8_name) const;

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
        uint32_t foreign_begin = header->foreign_off;
        uint32_t foreign_end = foreign_begin + header->foreign_size;
        return id.GetOffset() >= foreign_begin && id.GetOffset() < foreign_end;
    }

    EntityId GetIdFromPointer(const uint8_t *ptr) const
    {
        return EntityId(ptr - GetBase());
    }

    Span<const uint8_t> GetSpanFromId(EntityId id) const
    {
        const Header *header = GetHeader();
        Span file(GetBase(), header->file_size);
        ThrowIfWithCheck(!id.IsValid() || id.GetOffset() >= file.size(), File::INVALID_FILE_OFFSET,
                         File::GET_SPAN_FROM_ID);
        return file.Last(file.size() - id.GetOffset());
    }

    Span<const uint32_t> GetClasses() const
    {
        const Header *header = GetHeader();
        Span file(GetBase(), header->file_size);
        Span class_idx_data = file.SubSpan(header->class_idx_off, header->num_classes * sizeof(uint32_t));
        return Span(reinterpret_cast<const uint32_t *>(class_idx_data.data()), header->num_classes);
    }

    Span<const uint32_t> GetLiteralArrays() const
    {
        const Header *header = GetHeader();
        Span file(GetBase(), header->file_size);
        Span litarr_idx_data = file.SubSpan(header->literalarray_idx_off, header->num_literalarrays * sizeof(uint32_t));
        return Span(reinterpret_cast<const uint32_t *>(litarr_idx_data.data()), header->num_literalarrays);
    }

    Span<const IndexHeader> GetIndexHeaders() const
    {
        const Header *header = GetHeader();
        Span file(GetBase(), header->file_size);
        auto sp = file.SubSpan(header->index_section_off, header->num_indexes * sizeof(IndexHeader));
        return Span(reinterpret_cast<const IndexHeader *>(sp.data()), header->num_indexes);
    }

    const IndexHeader *GetIndexHeader(EntityId id) const
    {
        if (UNLIKELY(!id.IsValid() || id.GetOffset() >= GetHeader()->file_size)) {
            return nullptr;
        }
        auto headers = GetIndexHeaders();
        auto offset = id.GetOffset();
        for (const auto &header : headers) {
            if (header.start <= offset && offset < header.end) {
                return &header;
            }
        }
        return nullptr;
    }

    Span<const EntityId> GetClassIndex(const IndexHeader *index_header) const
    {
        ThrowIfWithCheck(index_header == nullptr, File::NULL_INDEX_HEADER, File::GET_CLASS_INDEX);
        auto *header = GetHeader();
        Span file(GetBase(), header->file_size);
        ASSERT(index_header != nullptr);
        auto class_idx_size = index_header->class_idx_size * EntityId::GetSize();
        ThrowIfWithCheck(index_header->class_idx_off > header->file_size || class_idx_size > header->file_size ||
                             index_header->class_idx_off > header->file_size - class_idx_size,
                         File::INVALID_INDEX_HEADER, File::GET_CLASS_INDEX);
        auto sp = file.SubSpan(index_header->class_idx_off, index_header->class_idx_size * EntityId::GetSize());
        return Span(reinterpret_cast<const EntityId *>(sp.data()), index_header->class_idx_size);
    }

    Span<const EntityId> GetClassIndex(EntityId id) const
    {
        auto *index_header = GetIndexHeader(id);
        return GetClassIndex(index_header);
    }

    Span<const EntityId> GetMethodIndex(const IndexHeader *index_header) const
    {
        ThrowIfWithCheck(index_header == nullptr, File::NULL_INDEX_HEADER, File::GET_METHOD_INDEX);
        auto *header = GetHeader();
        Span file(GetBase(), header->file_size);
        ASSERT(index_header != nullptr);
        // NOLINTNEXTLINE(clang-analyzer-core.NullDereference)
        auto method_idx_size = index_header->method_idx_size * EntityId::GetSize();
        ThrowIfWithCheck(index_header->method_idx_off > header->file_size || method_idx_size > header->file_size ||
                             index_header->method_idx_off > header->file_size - method_idx_size,
                         File::INVALID_INDEX_HEADER, File::GET_METHOD_INDEX);
        auto sp = file.SubSpan(index_header->method_idx_off, index_header->method_idx_size * EntityId::GetSize());
        return Span(reinterpret_cast<const EntityId *>(sp.data()), index_header->method_idx_size);
    }

    Span<const EntityId> GetMethodIndex(EntityId id) const
    {
        auto *index_header = GetIndexHeader(id);
        return GetMethodIndex(index_header);
    }

    Span<const EntityId> GetFieldIndex(const IndexHeader *index_header) const
    {
        ThrowIfWithCheck(index_header == nullptr, File::NULL_INDEX_HEADER, File::GET_FIELD_INDEX);
        auto *header = GetHeader();
        Span file(GetBase(), header->file_size);
        ASSERT(index_header != nullptr);
        auto field_idx_size = index_header->field_idx_size * EntityId::GetSize();
        ThrowIfWithCheck(index_header->field_idx_off > header->file_size || field_idx_size > header->file_size ||
                             index_header->field_idx_off > header->file_size - field_idx_size,
                         File::INVALID_INDEX_HEADER, File::GET_FIELD_INDEX);
        auto sp = file.SubSpan(index_header->field_idx_off, index_header->field_idx_size * EntityId::GetSize());
        return Span(reinterpret_cast<const EntityId *>(sp.data()), index_header->field_idx_size);
    }

    Span<const EntityId> GetFieldIndex(EntityId id) const
    {
        auto *index_header = GetIndexHeader(id);
        return GetFieldIndex(index_header);
    }

    Span<const EntityId> GetProtoIndex(const IndexHeader *index_header) const
    {
        ThrowIfWithCheck(index_header == nullptr, File::NULL_INDEX_HEADER, File::GET_PROTO_INDEX);
        auto *header = GetHeader();
        Span file(GetBase(), header->file_size);
        ASSERT(index_header != nullptr);
        auto proto_idx_size = index_header->proto_idx_size * EntityId::GetSize();
        ThrowIfWithCheck(index_header->proto_idx_off > header->file_size || proto_idx_size > header->file_size ||
                             index_header->proto_idx_off > header->file_size - proto_idx_size,
                         File::INVALID_INDEX_HEADER, File::GET_PROTO_INDEX);
        auto sp = file.SubSpan(index_header->proto_idx_off, index_header->proto_idx_size * EntityId::GetSize());
        return Span(reinterpret_cast<const EntityId *>(sp.data()), index_header->proto_idx_size);
    }

    Span<const EntityId> GetProtoIndex(EntityId id) const
    {
        auto *index_header = GetIndexHeader(id);
        return GetProtoIndex(index_header);
    }

    Span<const EntityId> GetLineNumberProgramIndex() const
    {
        const Header *header = GetHeader();
        Span file(GetBase(), header->file_size);
        Span lnp_idx_data = file.SubSpan(header->lnp_idx_off, header->num_lnps * EntityId::GetSize());
        return Span(reinterpret_cast<const EntityId *>(lnp_idx_data.data()), header->num_lnps);
    }

    EntityId ResolveClassIndex(EntityId id, Index idx) const
    {
        auto index = GetClassIndex(id);
        if (UNLIKELY(idx >= index.Size())) {
            return EntityId();
        }
        return index[idx];
    }

    EntityId ResolveMethodIndex(EntityId id, Index idx) const
    {
        auto index = GetMethodIndex(id);
        if (UNLIKELY(idx >= index.Size())) {
            return EntityId();
        }
        return index[idx];
    }

    EntityId ResolveOffsetByIndex(EntityId id, Index idx) const
    {
        auto index = GetMethodIndex(id);
        if (UNLIKELY(idx >= index.Size())) {
            return EntityId();
        }
        return index[idx];
    }

    EntityId ResolveFieldIndex(EntityId id, Index idx) const
    {
        auto index = GetFieldIndex(id);
        if (UNLIKELY(idx >= index.Size())) {
            return EntityId();
        }
        return index[idx];
    }

    EntityId ResolveProtoIndex(EntityId id, Index idx) const
    {
        auto index = GetProtoIndex(id);
        if (UNLIKELY(idx >= index.Size())) {
            return EntityId();
        }
        return index[idx];
    }

    EntityId ResolveLineNumberProgramIndex(Index32 idx) const
    {
        auto index = GetLineNumberProgramIndex();
        if (UNLIKELY(idx >= index.Size())) {
            return EntityId();
        }
        return index[idx];
    }

    const std::string &GetFilename() const
    {
        return FILENAME;
    }

    PandaCache *GetPandaCache() const
    {
#ifdef ENABLE_FULL_FILE_FIELDS
        return panda_cache_.get();
#else
        LOG(WARNING, PANDAFILE) << "Not Support GetPandaCache from ohos side.";
        return nullptr;
#endif
    }

    uint32_t GetFilenameHash() const
    {
        return FILENAME_HASH;
    }

    // note: intentionally returns uint64_t instead of the field type due to usage
    uint64_t GetUniqId() const
    {
        return UNIQ_ID;
    }

    const std::string &GetFullFileName() const
    {
#ifdef ENABLE_FULL_FILE_FIELDS
        return FULL_FILENAME;
#else
        LOG(FATAL, PANDAFILE) << "Not Support GetFullFileName from ohos side.";
        return FILENAME;
#endif
    }

    static constexpr uint32_t GetFileBaseOffset()
    {
        return MEMBER_OFFSET(File, base_);
    }

    Span<const panda::panda_file::EntityPairHeader> GetClassHashTable() const
    {
        return class_hash_table_;
    }

    static uint32_t CalcFilenameHash(const std::string &filename);

    PANDA_PUBLIC_API static std::unique_ptr<const File> Open(std::string_view filename, OpenMode open_mode = READ_ONLY);

    static std::unique_ptr<const File> OpenFromMemory(os::mem::ConstBytePtr &&ptr);

    static std::unique_ptr<const File> OpenFromMemory(os::mem::ConstBytePtr &&ptr, std::string_view filename);

    static std::unique_ptr<const File> OpenUncompressedArchive(int fd, const std::string_view &filename, size_t size,
                                                               uint32_t offset, OpenMode open_mode = READ_ONLY);

    void SetClassHashTable(panda::Span<const panda::panda_file::EntityPairHeader> class_hash_table) const
    {
        class_hash_table_ = class_hash_table;
    }

    bool ValidateChecksum(uint32_t *cal_checksum_out = nullptr) const;

    PANDA_PUBLIC_API void ThrowIfWithCheck(bool cond, const std::string_view &msg,
                                           const std::string_view &tag = "") const;

    static constexpr const char *INVALID_FILE_OFFSET = "Invalid file offset";
    static constexpr const char *NULL_INDEX_HEADER = "index_header is null";
    static constexpr const char *INVALID_INDEX_HEADER = "index_header is invalid";

    static constexpr const char *GET_CLASS_INDEX = "GetClassIndex";
    static constexpr const char *GET_METHOD_INDEX = "GetMethodIndex";
    static constexpr const char *GET_FIELD_INDEX = "GetFieldIndex";
    static constexpr const char *GET_PROTO_INDEX = "GetProtoIndex";

    static constexpr const char *ANNOTATION_DATA_ACCESSOR = "AnnotationDataAccessor";
    static constexpr const char *CLASS_DATA_ACCESSOR = "ClassDataAccessor";
    static constexpr const char *CODE_DATA_ACCESSOR = "CodeDataAccessor";
    static constexpr const char *FIELD_DATA_ACCESSOR = "FieldDataAccessor";
    static constexpr const char *GET_SPAN_FROM_ID = "GetSpanFromId";

    PANDA_PUBLIC_API ~File();

    NO_COPY_SEMANTIC(File);
    NO_MOVE_SEMANTIC(File);

private:
    File(std::string filename, os::mem::ConstBytePtr &&base);

    os::mem::ConstBytePtr base_;
    const std::string FILENAME;
    const uint32_t FILENAME_HASH;
#ifdef ENABLE_FULL_FILE_FIELDS
    const std::string FULL_FILENAME;
    std::unique_ptr<PandaCache> panda_cache_;
#endif
    const uint32_t UNIQ_ID;
    mutable panda::Span<const panda::panda_file::EntityPairHeader> class_hash_table_;
};

static_assert(File::GetFileBaseOffset() == 0);

inline bool operator==(const File::StringData &string_data1, const File::StringData &string_data2)
{
    if (string_data1.utf16_length != string_data2.utf16_length) {
        return false;
    }

    return utf::IsEqual(string_data1.data, string_data2.data);
}

inline bool operator!=(const File::StringData &string_data1, const File::StringData &string_data2)
{
    return !(string_data1 == string_data2);
}

inline bool operator<(const File::StringData &string_data1, const File::StringData &string_data2)
{
    if (string_data1.utf16_length == string_data2.utf16_length) {
        return utf::CompareMUtf8ToMUtf8(string_data1.data, string_data2.data) < 0;
    }

    return string_data1.utf16_length < string_data2.utf16_length;
}

bool CheckSecureMem(uintptr_t mem, size_t size);

/*
 * OpenPandaFileOrZip from location which specicify the name.
 */
std::unique_ptr<const File> OpenPandaFileOrZip(std::string_view location,
                                               panda_file::File::OpenMode open_mode = panda_file::File::READ_ONLY);

/*
 * OpenPandaFileFromMemory from file buffer.
 */
std::unique_ptr<const File> OpenPandaFileFromMemory(const void *buffer, size_t size, std::string tag = "");

/*
 * OpenPandaFileFromMemory from secure buffer.
 */
std::unique_ptr<const File> OpenPandaFileFromSecureMemory(uint8_t *buffer, size_t size);

/*
 * OpenPandaFile from location which specicify the name.
 */
std::unique_ptr<const File> OpenPandaFile(std::string_view location, std::string_view archive_filename = "",
                                          panda_file::File::OpenMode open_mode = panda_file::File::READ_ONLY);

/*
 * Check ptr point valid panda file: magic
 */
bool CheckHeader(const os::mem::ConstBytePtr &ptr, const std::string_view &filename = "");
void CheckFileVersion(const std::array<uint8_t, File::VERSION_SIZE> &file_version, const std::string_view &filename);

enum class PandaFileType : int8_t {
    FILE_FORMAT_INVALID = -1,
    FILE_DYNAMIC = 0,
    FILE_STATIC = 1,
};

/**
 * @brief Return the abc file type
 * @param[in] data : begin pos of abc file
 * @param[in] size : length of abc file
 * @return -1 indicates the abc file format invalid
 *         0 indicates the abc file is dynamic
 *         1 indicates the abc file is static
 */
PandaFileType GetFileType(const uint8_t *data, int32_t size);

// Last version which contains redundance literal array in header
constexpr std::array<uint8_t, File::VERSION_SIZE> LAST_CONTAINS_LITERAL_IN_HEADER_VERSION {12, 0, 6, 0};
bool ContainsLiteralArrayInHeader(const std::array<uint8_t, File::VERSION_SIZE> &version);

// NOLINTNEXTLINE(readability-identifier-naming)
extern const char *ARCHIVE_FILENAME;
}  // namespace panda::panda_file

namespace std {
template <>
struct hash<panda::panda_file::File::EntityId> {
    std::size_t operator()(panda::panda_file::File::EntityId id) const
    {
        return std::hash<uint32_t> {}(id.GetOffset());
    }
};

}  // namespace std

#endif  // LIBPANDAFILE_FILE_H
