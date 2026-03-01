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

#include <libarkfile/include/file_format_version.h>
#include "libarkfile/file-inl.h"
#include "libarkbase/os/file.h"
#include "libarkbase/os/mem.h"
#include "libarkbase/os/filesystem.h"
#include "libarkbase/mem/mem.h"
#include "libarkfile/panda_cache.h"

#include "libarkbase/utils/hash.h"
#include "libarkbase/utils/logger.h"
#include "libarkbase/utils/utf.h"
#include "libarkbase/utils/span.h"
#include "zip_archive.h"
#include "libarkbase/trace/trace.h"
#include "securec.h"

#include <cerrno>
#include <cstring>

#include <algorithm>
#include <memory>
#include <string>
#include <variant>
#include <cstdio>
#include <map>
namespace ark::panda_file {

// NOLINTNEXTLINE(readability-identifier-naming, modernize-avoid-c-arrays)
const char *ARCHIVE_FILENAME = "classes.abc";
// NOLINTNEXTLINE(readability-identifier-naming, modernize-avoid-c-arrays)
const char *ARCHIVE_SPLIT = "!/";

const std::array<uint8_t, File::MAGIC_SIZE> File::MAGIC {'P', 'A', 'N', 'D', 'A', '\0', '\0', '\0'};

// Name anonymous maps for perfing tools finding symbol file correctly.
// NOLINTNEXTLINE(readability-identifier-naming, modernize-avoid-c-arrays)
const char *ANONMAPNAME_PERFIX = "panda-";

os::file::Mode GetMode(panda_file::File::OpenMode openMode)
{
    switch (openMode) {
        case File::READ_ONLY: {
            return os::file::Mode::READONLY;
        }
        case File::READ_WRITE: {
#ifdef PANDA_TARGET_WINDOWS
            return os::file::Mode::READWRITE;
#else
            return os::file::Mode::READONLY;
#endif
        }
        case File::WRITE_ONLY: {
            return os::file::Mode::WRITEONLY;
        }
        default: {
            break;
        }
    }

    UNREACHABLE();
}

static uint32_t GetProt(panda_file::File::OpenMode mode)
{
    uint32_t prot = os::mem::MMAP_PROT_READ;
    if (mode == File::READ_WRITE) {
        prot |= os::mem::MMAP_PROT_WRITE;
    }
    return prot;
}

class AnonMemSet {
public:
    using MemNameSet = std::map<std::string, std::string>;
    using InsertResult = std::map<std::string, std::string>::iterator;

    static AnonMemSet &GetInstance()
    {
        static AnonMemSet anonMemSet;
        return anonMemSet;
    }

    InsertResult Insert(const std::string &fileName, const std::string &anonMemName)
    {
        return memNameSet_.emplace(fileName, anonMemName).first;
    }

    void Remove(const std::string &fileName)
    {
        auto it = memNameSet_.find(fileName);
        if (it != memNameSet_.end()) {
            memNameSet_.erase(it);
        }
    }

private:
    MemNameSet memNameSet_;
};

std::unique_ptr<const File> OpenPandaFileOrZip(std::string_view location, panda_file::File::OpenMode openMode)
{
    std::string_view archiveFilename = ARCHIVE_FILENAME;
    std::size_t archiveSplitIndex = location.find(ARCHIVE_SPLIT);
    if (archiveSplitIndex != std::string::npos) {
        archiveFilename = location.substr(archiveSplitIndex + 2);  // 2 - archive split size
        location = location.substr(0, archiveSplitIndex);
    }

    return OpenPandaFile(location, archiveFilename, openMode);
}

// NOLINTNEXTLINE(google-runtime-references)
void OpenPandaFileFromZipErrorHandler(ZipArchiveHandle &handle)
{
    if (handle != nullptr) {
        if (ark::CloseArchiveFile(handle) != ZIPARCHIVE_OK) {
            LOG(ERROR, PANDAFILE) << "CloseArchiveFile failed!";
        }
    }
}

std::unique_ptr<const panda_file::File> OpenPandaFileFromZipFile(ZipArchiveHandle &handle, std::string_view location,
                                                                 EntryFileStat &entry, std::string_view archiveName)
{
    uint32_t uncompressedLength = entry.GetUncompressedSize();
    if (uncompressedLength == 0) {
        LOG(ERROR, PANDAFILE) << "Panda file has zero length!";
        return nullptr;
    }

    size_t sizeToMmap = AlignUp(uncompressedLength, ark::os::mem::GetPageSize());
    void *mem = os::mem::MapRWAnonymousRaw(sizeToMmap, false);
    if (mem == nullptr) {
        LOG(ERROR, PANDAFILE) << "Can't mmap anonymous!";
        return nullptr;
    }
    os::mem::BytePtr ptr(reinterpret_cast<std::byte *>(mem), sizeToMmap, os::mem::MmapDeleter);
    std::stringstream ss;
    ss << ANONMAPNAME_PERFIX << archiveName << " extracted in memory from " << location;
    auto it = AnonMemSet::GetInstance().Insert(std::string(location), ss.str());
    auto ret = os::mem::TagAnonymousMemory(reinterpret_cast<void *>(ptr.Get()), sizeToMmap, it->second.c_str());
    if (ret.has_value()) {
        LOG(ERROR, PANDAFILE) << "Can't tag mmap anonymous!";
        return nullptr;
    }

    auto extractError = ExtractToMemory(handle, reinterpret_cast<uint8_t *>(ptr.Get()), sizeToMmap);
    if (extractError != 0) {
        LOG(ERROR, PANDAFILE) << "Can't extract!";
        return nullptr;
    }

    os::mem::ConstBytePtr constPtr = ptr.ToConst();
    return panda_file::File::OpenFromMemory(std::move(constPtr), location);
}

// CC-OFFNXT(G.FUN.01-CPP) solid logic
// NOLINTNEXTLINE(google-runtime-references)
std::unique_ptr<const panda_file::File> HandleArchive(ZipArchiveHandle &handle, FILE *fp, std::string_view location,
                                                      EntryFileStat &entry, std::string_view archiveFilename,
                                                      panda_file::File::OpenMode openMode)
{
    std::unique_ptr<const panda_file::File> file;
    // compressed or not 4 aligned, use anonymous memory
    if (entry.IsCompressed() || (entry.GetOffset() & 0x3U) != 0) {
        file = OpenPandaFileFromZipFile(handle, location, entry, archiveFilename);
    } else {
        LOG(INFO, PANDAFILE) << "Pandafile is uncompressed and 4 bytes aligned";
        file = panda_file::File::OpenUncompressedArchive(fileno(fp), location, entry.GetUncompressedSize(),
                                                         entry.GetOffset(), openMode);
    }
    return file;
}

std::unique_ptr<const panda_file::File> OpenZipPandaFile(FILE *fp, std::string_view location,
                                                         std::string_view archiveFilename,
                                                         panda_file::File::OpenMode openMode)
{
    // Open Zipfile and do the extraction.
    ZipArchiveHandle zipfile = nullptr;
    auto openError = OpenArchiveFile(zipfile, fp);
    if (openError != ZIPARCHIVE_OK) {
        LOG(ERROR, PANDAFILE) << "Can't open archive " << location;
        return nullptr;
    }
    bool tryDefault = archiveFilename.empty();
    if (!tryDefault) {
        if (LocateFile(zipfile, archiveFilename.data()) != ZIPARCHIVE_OK) {
            LOG(INFO, PANDAFILE) << "Can't find entry with name '" << archiveFilename << "', will try "
                                 << ARCHIVE_FILENAME;
            tryDefault = true;
        }
    }
    if (tryDefault) {
        if (LocateFile(zipfile, ARCHIVE_FILENAME) != ZIPARCHIVE_OK) {
            OpenPandaFileFromZipErrorHandler(zipfile);
            LOG(ERROR, PANDAFILE) << "Can't find entry with " << ARCHIVE_FILENAME;
            fclose(fp);
            return nullptr;
        }
    }

    EntryFileStat entry = EntryFileStat();
    if (GetCurrentFileInfo(zipfile, &entry) != ZIPARCHIVE_OK) {
        OpenPandaFileFromZipErrorHandler(zipfile);
        LOG(ERROR, PANDAFILE) << "GetCurrentFileInfo error";
        return nullptr;
    }
    // check that file is not empty, otherwise crash at CloseArchiveFile
    if (entry.GetUncompressedSize() == 0) {
        OpenPandaFileFromZipErrorHandler(zipfile);
        LOG(ERROR, PANDAFILE) << "Invalid panda file '" << (tryDefault ? ARCHIVE_FILENAME : archiveFilename) << "'";
        return nullptr;
    }
    if (OpenCurrentFile(zipfile) != ZIPARCHIVE_OK) {
        CloseCurrentFile(zipfile);
        OpenPandaFileFromZipErrorHandler(zipfile);
        LOG(ERROR, PANDAFILE) << "Can't OpenCurrentFile!";
        return nullptr;
    }
    GetCurrentFileOffset(zipfile, &entry);
    auto file = HandleArchive(zipfile, fp, location, entry, archiveFilename, openMode);
    CloseCurrentFile(zipfile);
    OpenPandaFileFromZipErrorHandler(zipfile);
    return file;
}

std::unique_ptr<const panda_file::File> OpenPandaFile(std::string_view location, std::string_view archiveFilename,
                                                      panda_file::File::OpenMode openMode)
{
    trace::ScopedTrace scopedTrace("Open panda file " + std::string(location));
    uint32_t magic;

#ifdef PANDA_TARGET_WINDOWS
    constexpr char const *MODE = "rb";
#else
    constexpr char const *MODE = "rbe";
#endif

    FILE *fp = fopen(std::string(location).c_str(), MODE);
    if (fp == nullptr) {
        LOG(ERROR, PANDAFILE) << "Can't fopen location: " << location;
        return nullptr;
    }
    fseek(fp, 0, SEEK_SET);
    if (fread(&magic, sizeof(magic), 1, fp) != 1) {
        fclose(fp);
        LOG(ERROR, PANDAFILE) << "Can't read from file!(magic) " << location;
        return nullptr;
    }
    fseek(fp, 0, SEEK_SET);
    std::unique_ptr<const panda_file::File> file;
    bool isZip = IsZipMagic(magic);
    if (isZip) {
        file = OpenZipPandaFile(fp, location, archiveFilename, openMode);
    } else {
        file = panda_file::File::Open(location, openMode);
    }
    // OpenZipPandaFile will close fp directly and return nullptr if some exception happened.
    // So for zip file, only need to close fp when file is not nullptr
    if (!isZip || file != nullptr) {
        fclose(fp);
    }
    return file;
}

std::unique_ptr<const File> OpenPandaFileFromMemory(const void *buffer, size_t size, std::string tag)
{
    size_t sizeToMmap = AlignUp(size, ark::os::mem::GetPageSize());
    void *mem = os::mem::MapRWAnonymousRaw(sizeToMmap, false);
    if (mem == nullptr) {
        return nullptr;
    }

    if (memcpy_s(mem, sizeToMmap, buffer, size) != 0) {
        PLOG(ERROR, PANDAFILE) << "Failed to copy buffer into mem'";
    }

    if (!tag.empty()) {
        if (tag == "ArkTS Code") {
            std::string memAddr = std::to_string(ToUintPtr(mem));
            tag = tag + ":" + memAddr;
        }
        auto ret = os::mem::TagAnonymousMemory(mem, sizeToMmap, tag.c_str());
        if (ret.has_value()) {
            PLOG(ERROR, PANDAFILE) << "Can't tag mmap anonymous, errno: " << errno;
        }
    }

    os::mem::ConstBytePtr ptr(reinterpret_cast<std::byte *>(mem), sizeToMmap, os::mem::MmapDeleter);
    if (ptr.Get() == nullptr) {
        PLOG(ERROR, PANDAFILE) << "Failed to open panda file from memory'";
        return nullptr;
    }
    std::hash<void *> hash;
    return panda_file::File::OpenFromMemory(std::move(ptr), std::to_string(hash(mem)));
}

std::unique_ptr<const File> OpenPandaFileFromSecureMemory(uint8_t *buffer, size_t size, std::string filename)
{
    if (buffer == nullptr) {
        PLOG(ERROR, PANDAFILE) << "OpenPandaFileFromSecureMemory buffer is nullptr'";
        return nullptr;
    }

    auto *mem = reinterpret_cast<std::byte *>(buffer);
    os::mem::ConstBytePtr ptr(mem, size, [](std::byte *, size_t) noexcept {});
    if (ptr.Get() == nullptr) {
        PLOG(ERROR, PANDAFILE) << "Failed to open panda file from secure memory'";
        return nullptr;
    }

    std::hash<std::byte *> hash;
    if (filename.empty()) {  // filename is sandbox path in application
        filename = std::to_string(hash(mem));
    }
    return panda_file::File::OpenFromMemory(std::move(ptr), filename);
}

class ClassIdxIterator {
public:
    // NOLINTNEXTLINE(readability-identifier-naming)
    using value_type = const uint8_t *;
    // NOLINTNEXTLINE(readability-identifier-naming)
    using difference_type = std::ptrdiff_t;
    // NOLINTNEXTLINE(readability-identifier-naming)
    using pointer = uint32_t *;
    // NOLINTNEXTLINE(readability-identifier-naming)
    using reference = uint32_t &;
    // NOLINTNEXTLINE(readability-identifier-naming)
    using iterator_category = std::random_access_iterator_tag;

    ClassIdxIterator(const File &file, const Span<const uint32_t> &span, size_t idx)
        : file_(file), span_(span), idx_(idx)
    {
    }

    ClassIdxIterator(const ClassIdxIterator &other) = default;
    ClassIdxIterator(ClassIdxIterator &&other) = default;
    ~ClassIdxIterator() = default;

    ClassIdxIterator &operator=(const ClassIdxIterator &other)
    {
        if (&other != this) {
            idx_ = other.idx_;
        }

        return *this;
    }

    ClassIdxIterator &operator=(ClassIdxIterator &&other) noexcept
    {
        idx_ = other.idx_;
        return *this;
    }

    ClassIdxIterator &operator+=(size_t n)
    {
        idx_ += n;
        return *this;
    }

    ClassIdxIterator &operator-=(size_t n)
    {
        idx_ -= n;
        return *this;
    }

    ClassIdxIterator &operator++()
    {
        ++idx_;
        return *this;
    }

    ClassIdxIterator &operator--()
    {
        --idx_;
        return *this;
    }

    difference_type operator-(const ClassIdxIterator &other)
    {
        return static_cast<difference_type>(idx_ - other.idx_);
    }

    value_type operator*() const
    {
        uint32_t id = span_[idx_];
        return file_.GetStringData(File::EntityId(id)).data;
    }

    bool IsValid() const
    {
        return idx_ < span_.Size();
    }

    uint32_t GetId() const
    {
        return span_[idx_];
    }

    static ClassIdxIterator Begin(const File &file, const Span<const uint32_t> &span)
    {
        return ClassIdxIterator(file, span, 0);
    }

    static ClassIdxIterator End(const File &file, const Span<const uint32_t> &span)
    {
        return ClassIdxIterator(file, span, span.Size());
    }

private:
    const File &file_;
    const Span<const uint32_t> &span_;
    size_t idx_;
};

File::File(std::string filename, os::mem::ConstBytePtr &&base)
    : base_(std::forward<os::mem::ConstBytePtr>(base)),
      filename_(std::move(filename)),
      filenameHash_(CalcFilenameHash(filename_)),
      fullFilename_(os::GetAbsolutePath(filename_).empty() ? filename_ : os::GetAbsolutePath(filename_)),
      pandaCache_(std::make_unique<PandaCache>()),
      uniqId_(MergeHashes(filenameHash_, GetHash32(reinterpret_cast<const uint8_t *>(GetHeader()), sizeof(Header))))
{
}

File::~File()
{
    AnonMemSet::GetInstance().Remove(filename_);
}

inline std::string VersionToString(const std::array<uint8_t, File::VERSION_SIZE> &array)
{
    std::stringstream ss;

    for (size_t i = 0; i < File::VERSION_SIZE - 1; ++i) {
        ss << static_cast<int>(array[i]);
        ss << ".";
    }
    ss << static_cast<int>(array[File::VERSION_SIZE - 1]);

    return ss.str();
}

// We can't use default std::array's comparision operators and need to implement
// own ones due to the bug in gcc: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=95189
inline int CompareVersions(const std::array<uint8_t, File::VERSION_SIZE> &lhs,
                           const std::array<uint8_t, File::VERSION_SIZE> &rhs)
{
    for (size_t i = 0; i < File::VERSION_SIZE; i++) {
        if (lhs[i] == rhs[i]) {
            continue;
        }
        return lhs[i] - rhs[i];
    }
    return 0;
}

inline bool operator<(const std::array<uint8_t, File::VERSION_SIZE> &lhs,
                      const std::array<uint8_t, File::VERSION_SIZE> &rhs)
{
    return CompareVersions(lhs, rhs) < 0;
}

inline bool operator>(const std::array<uint8_t, File::VERSION_SIZE> &lhs,
                      const std::array<uint8_t, File::VERSION_SIZE> &rhs)
{
    return CompareVersions(lhs, rhs) > 0;
}

/* static */
std::unique_ptr<const File> File::Open(std::string_view filename, OpenMode openMode)
{
    trace::ScopedTrace scopedTrace("Open panda file " + std::string(filename));
    os::file::Mode mode = GetMode(openMode);
    os::file::File file = os::file::Open(filename, mode);
    if (!file.IsValid()) {
        PLOG(ERROR, PANDAFILE) << "Failed to open panda file '" << filename << "'";
        return nullptr;
    }

    os::file::FileHolder fhHolder(file);

    auto res = file.GetFileSize();
    if (!res) {
        PLOG(ERROR, PANDAFILE) << "Failed to get size of panda file '" << filename << "'";
        return nullptr;
    }

    size_t size = res.Value();
    if (size < sizeof(File::Header)) {
        LOG(ERROR, PANDAFILE) << "Invalid panda file '" << filename << "' - has not header";
        return nullptr;
    }

    os::mem::ConstBytePtr ptr = os::mem::MapFile(file, GetProt(openMode), os::mem::MMAP_FLAG_PRIVATE, size).ToConst();
    if (ptr.Get() == nullptr) {
        PLOG(ERROR, PANDAFILE) << "Failed to map panda file '" << filename << "'";
        return nullptr;
    }

    if (!CheckHeader(ptr, filename, size)) {
        return nullptr;
    }

    // CC-OFFNXT(G.RES.09-CPP) ctor of class is private
    return std::unique_ptr<File>(new File(filename.data(), std::move(ptr)));
}

std::unique_ptr<const File> File::OpenUncompressedArchive(int fd, const std::string_view &filename, size_t size,
                                                          uint32_t offset, OpenMode openMode)
{
    trace::ScopedTrace scopedTrace("Open panda file " + std::string(filename));
    auto file = os::file::File(fd);
    if (!file.IsValid()) {
        PLOG(ERROR, PANDAFILE) << "OpenUncompressedArchive: Failed to open panda file '" << filename << "'";
        return nullptr;
    }

    if (size < sizeof(File::Header)) {
        LOG(ERROR, PANDAFILE) << "Invalid panda file size '" << filename << "'";
        return nullptr;
    }
    LOG(DEBUG, PANDAFILE) << " size=" << size << " offset=" << offset << " " << filename;

    os::mem::ConstBytePtr ptr =
        os::mem::MapFile(file, GetProt(openMode), os::mem::MMAP_FLAG_PRIVATE, size, offset).ToConst();
    if (ptr.Get() == nullptr) {
        PLOG(ERROR, PANDAFILE) << "Failed to map panda file '" << filename << "'";
        return nullptr;
    }
    if (!CheckHeader(ptr, filename)) {
        return nullptr;
    }

    // CC-OFFNXT(G.RES.09-CPP) ctor of class is private
    return std::unique_ptr<File>(new File(filename.data(), std::move(ptr)));
}

bool ValidateChecksum(const os::mem::ConstBytePtr &ptr, const std::string_view &filename)
{
    if (ptr.Get() == nullptr || ptr.GetSize() < sizeof(File::Header)) {
        LOG(ERROR, PANDAFILE) << "Invalid panda file '" << filename << "'";
        return false;
    }
    auto header = reinterpret_cast<const File::Header *>(ptr.Get());
    auto checksumSize = sizeof(File::Header::checksum);
    auto fileContentOffset = File::MAGIC_SIZE + checksumSize;
    Span<const uint8_t> dataSpan(reinterpret_cast<const uint8_t *>(ptr.Get()), header->fileSize);
    auto pData = dataSpan.SubSpan(fileContentOffset).data();
    uint32_t calChecksum = adler32(1, pData, header->fileSize - fileContentOffset);
    if (header->checksum != calChecksum) {
        LOG(ERROR, PANDAFILE) << "Checksum mismatch. The abc file has been corrupted. Expected checksum: 0x" << std::hex
                              << header->checksum << ", Actual checksum: 0x" << std::hex << calChecksum << std::endl;
        return false;
    }
    return true;
}

bool CheckHeader(const os::mem::ConstBytePtr &ptr, const std::string_view &filename, const size_t &expectedLength)
{
    if (ptr.Get() == nullptr || ptr.GetSize() < sizeof(File::Header)) {
        LOG(ERROR, PANDAFILE) << "Invalid panda file '" << filename << "'";
        return false;
    }
    auto header = reinterpret_cast<const File::Header *>(ptr.Get());
    if (expectedLength != 0 && expectedLength != header->fileSize) {
        LOG(ERROR, PANDAFILE) << "File [" << filename << "]'s actual size [" << ptr.GetSize()
                              << "] is not equal to Header's fileSize [" << header->fileSize << "]";
        return false;
    }
    if (header->magic != File::MAGIC) {
        LOG(ERROR, PANDAFILE) << "Invalid magic number '";
        return false;
    }

    if (!ValidateChecksum(ptr)) {
        return false;
    }

    auto fileVersion = header->version;

    if (fileVersion < MIN_VERSION || fileVersion > VERSION) {
        LOG(ERROR, PANDAFILE) << "Unable to open file '" << filename << "' with bytecode version "
                              << VersionToString(fileVersion);
        if (fileVersion < MIN_VERSION) {
            LOG(ERROR, PANDAFILE) << "Minimum supported version is " << VersionToString(MIN_VERSION);
        } else {
            LOG(ERROR, PANDAFILE) << "Maximum supported version is " << VersionToString(VERSION);
        }
        return false;
    }
    return true;
}

/* static */
std::unique_ptr<const File> File::OpenFromMemory(os::mem::ConstBytePtr &&ptr)
{
    if (!CheckHeader(ptr, std::string_view())) {
        return nullptr;
    }

    // CC-OFFNXT(G.RES.09-CPP) ctor of class is private
    return std::unique_ptr<File>(new File("", std::forward<os::mem::ConstBytePtr>(ptr)));
}

/* static */
std::unique_ptr<const File> File::OpenFromMemory(os::mem::ConstBytePtr &&ptr, std::string_view filename)
{
    trace::ScopedTrace scopedTrace("Open panda file from RAM " + std::string(filename));

    if (!CheckHeader(ptr, filename)) {
        return nullptr;
    }

    // CC-OFFNXT(G.RES.09-CPP) ctor of class is private
    return std::unique_ptr<File>(new File(filename.data(), std::forward<os::mem::ConstBytePtr>(ptr)));
}

File::EntityId File::GetClassId(const uint8_t *mutf8Name) const
{
    auto classHashTable = GetClassHashTable();
    if (!classHashTable.empty()) {
        return GetClassIdFromClassHashTable(mutf8Name);
    }

    auto classIdx = GetClasses();

    auto it = std::lower_bound(ClassIdxIterator::Begin(*this, classIdx), ClassIdxIterator::End(*this, classIdx),
                               mutf8Name, utf::Mutf8Less());
    if (!it.IsValid()) {
        return EntityId();
    }

    if (utf::CompareMUtf8ToMUtf8(mutf8Name, *it) == 0) {
        return EntityId(it.GetId());
    }

    return EntityId();
}

uint32_t File::CalcFilenameHash(const std::string &filename)
{
    return GetHash32String(reinterpret_cast<const uint8_t *>(filename.c_str()));
}

File::EntityId File::GetLiteralArraysId() const
{
    const Header *header = GetHeader();
    return EntityId(header->literalarrayIdxOff);
}

File::EntityId File::GetClassIdFromClassHashTable(const uint8_t *mutf8Name) const
{
    auto classHashTable = GetClassHashTable();
    auto hash = GetHash32String(mutf8Name);
    auto pos = hash & (classHashTable.size() - 1);
    auto entityPair = &classHashTable[pos];

    if (entityPair->descriptorHash % classHashTable.size() != pos) {
        return File::EntityId();
    }

    while (true) {
        if (hash == entityPair->descriptorHash) {
            auto entityId = File::EntityId(entityPair->entityIdOffset);
            auto descriptor = GetStringData(entityId).data;
            if (entityId.IsValid() && utf::CompareMUtf8ToMUtf8(descriptor, mutf8Name) == 0) {
                return entityId;
            }
        }
        if (entityPair->nextPos == 0) {
            break;
        }
        entityPair = &classHashTable[entityPair->nextPos - 1];
    }

    return File::EntityId();
}

}  // namespace ark::panda_file
