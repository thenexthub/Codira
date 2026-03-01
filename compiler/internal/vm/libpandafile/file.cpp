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

#include "data_protect.h"
#include "file-inl.h"
#include "file_items.h"
#include "panda_cache.h"
#include "trace/trace.h"
#include "zip_archive.h"

namespace panda::panda_file {
// NOLINTNEXTLINE(readability-identifier-naming, modernize-avoid-c-arrays)
const char *ARCHIVE_FILENAME = "classes.abc";
// NOLINTNEXTLINE(readability-identifier-naming, modernize-avoid-c-arrays)
const char *ARCHIVE_SPLIT = "!/";

const std::array<uint8_t, File::MAGIC_SIZE> File::MAGIC {'P', 'A', 'N', 'D', 'A', '\0', '\0', '\0'};

// Name anonymous maps for perfing tools finding symbol file correctly.
// NOLINTNEXTLINE(readability-identifier-naming, modernize-avoid-c-arrays)
const char *ANONMAPNAME_PERFIX = "panda-";

os::file::Mode GetMode(panda_file::File::OpenMode open_mode)
{
    switch (open_mode) {
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
        static AnonMemSet anon_mem_set;
        return anon_mem_set;
    }

    InsertResult Insert(const std::string &file_name, const std::string &anon_mem_name)
    {
        return mem_name_set_.emplace(file_name, anon_mem_name).first;
    }

    void Remove(const std::string &file_name)
    {
        auto it = mem_name_set_.find(file_name);
        if (it != mem_name_set_.end()) {
            mem_name_set_.erase(it);
        }
    }

private:
    MemNameSet mem_name_set_;
};

std::unique_ptr<const File> OpenPandaFileOrZip(std::string_view location, panda_file::File::OpenMode open_mode)
{
    std::string_view archive_filename = ARCHIVE_FILENAME;
    std::size_t archive_split_index = location.find(ARCHIVE_SPLIT);
    if (archive_split_index != std::string::npos) {
        archive_filename = location.substr(archive_split_index + 2);  // 2 - archive split size
        location = location.substr(0, archive_split_index);
    }

    return OpenPandaFile(location, archive_filename, open_mode);
}

// NOLINTNEXTLINE(google-runtime-references)
void OpenPandaFileFromZipErrorHandler(ZipArchiveHandle &handle)
{
    if (handle != nullptr) {
        if (panda::CloseArchiveFile(handle) != ZIPARCHIVE_OK) {
            LOG(ERROR, PANDAFILE) << "CloseArchiveFile failed!";
        }
    }
}

std::unique_ptr<const panda_file::File> OpenPandaFileFromZipFile(ZipArchiveHandle &handle, std::string_view location,
                                                                 EntryFileStat &entry,
                                                                 const std::string_view &archive_name)
{
    uint32_t uncompressed_length = entry.GetUncompressedSize();
    ASSERT(uncompressed_length != 0U);

    size_t size_to_mmap = AlignUp(uncompressed_length, panda::os::mem::GetPageSize());
    void *mem = os::mem::MapRWAnonymousRaw(size_to_mmap, false);
    if (mem == nullptr) {
        LOG(ERROR, PANDAFILE) << "Can't mmap anonymous!";
        return nullptr;
    }
    os::mem::BytePtr ptr(reinterpret_cast<std::byte *>(mem), size_to_mmap, os::mem::MmapDeleter);
    std::stringstream ss;
    ss << ANONMAPNAME_PERFIX << archive_name << " extracted in memory from " << location;
    auto it = AnonMemSet::GetInstance().Insert(std::string(location), ss.str());
    auto ret = os::mem::TagAnonymousMemory(reinterpret_cast<void *>(ptr.Get()), size_to_mmap, it->second.c_str());
    if (ret.has_value()) {
        LOG(ERROR, PANDAFILE) << "Can't tag mmap anonymous!";
        return nullptr;
    }

    auto extract_error = ExtractToMemory(handle, reinterpret_cast<uint8_t *>(ptr.Get()), size_to_mmap);
    if (extract_error != 0) {
        LOG(ERROR, PANDAFILE) << "Can't extract!";
        return nullptr;
    }

    os::mem::ConstBytePtr ConstPtr = ptr.ToConst();
    return panda_file::File::OpenFromMemory(std::move(ConstPtr), location);
}

// NOLINTNEXTLINE(google-runtime-references)
std::unique_ptr<const panda_file::File> HandleArchive(ZipArchiveHandle &handle, FILE *fp, std::string_view location,
                                                      EntryFileStat &entry, const std::string_view &archive_filename,
                                                      panda_file::File::OpenMode open_mode)
{
    std::unique_ptr<const panda_file::File> file;
    // compressed or not 4 aligned, use anonymous memory
    if (entry.IsCompressed() || (entry.GetOffset() & 0x3U) != 0) {
        file = OpenPandaFileFromZipFile(handle, location, entry, archive_filename);
    } else {
        LOG(INFO, PANDAFILE) << "Pandafile is uncompressed and 4 bytes aligned";
        file = panda_file::File::OpenUncompressedArchive(fileno(fp), location, entry.GetUncompressedSize(),
                                                         entry.GetOffset(), open_mode);
    }
    return file;
}

std::unique_ptr<const panda_file::File> OpenPandaFileFromZip(FILE *fp, std::string_view location,
                                                             std::string_view archive_filename,
                                                             panda_file::File::OpenMode open_mode)
{
    // Open Zipfile and do the extraction.
    ZipArchiveHandle zipfile = nullptr;
    if (OpenArchiveFile(zipfile, fp) != ZIPARCHIVE_OK) {
        LOG(ERROR, PANDAFILE) << "Can't open archive " << location;
        return nullptr;
    }
    bool try_default = archive_filename.empty();
    if (!try_default && LocateFile(zipfile, archive_filename.data()) != ZIPARCHIVE_OK) {
        LOG(INFO, PANDAFILE) << "Can't find entry with name '" <<
            archive_filename << "', will try " << ARCHIVE_FILENAME;
        try_default = true;
    }
    if (try_default && LocateFile(zipfile, ARCHIVE_FILENAME) != ZIPARCHIVE_OK) {
        OpenPandaFileFromZipErrorHandler(zipfile);
        LOG(ERROR, PANDAFILE) << "Can't find entry with " << ARCHIVE_FILENAME;
        return nullptr;
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
        LOG(ERROR, PANDAFILE) << "Invalid panda file '" << (try_default ? ARCHIVE_FILENAME : archive_filename) << "'";
        return nullptr;
    }
    if (OpenCurrentFile(zipfile) != ZIPARCHIVE_OK) {
        CloseCurrentFile(zipfile);
        OpenPandaFileFromZipErrorHandler(zipfile);
        LOG(ERROR, PANDAFILE) << "Can't OpenCurrentFile!";
        return nullptr;
    }
    GetCurrentFileOffset(zipfile, &entry);
    std::unique_ptr<const panda_file::File> file = HandleArchive(zipfile, fp, location, entry,
                                                                 archive_filename, open_mode);
    CloseCurrentFile(zipfile);
    OpenPandaFileFromZipErrorHandler(zipfile);
    return file;
}

std::unique_ptr<const panda_file::File> OpenPandaFile(std::string_view location, std::string_view archive_filename,
                                                      panda_file::File::OpenMode open_mode)
{
    trace::ScopedTrace scoped_trace("Open panda file " + std::string(location));
    uint32_t magic;

#ifdef PANDA_TARGET_WINDOWS
    constexpr char const *mode = "rb";
#else
    constexpr char const *mode = "rbe";
#endif

    FILE *fp = fopen(std::string(location).c_str(), mode);
    if (fp == nullptr) {
        LOG(ERROR, PANDAFILE) << "Can't fopen location: " << location;
        return nullptr;
    }
    (void)fseek(fp, 0, SEEK_SET);
    if (fread(&magic, sizeof(magic), 1, fp) != 1) {
        fclose(fp);
        LOG(ERROR, PANDAFILE) << "Can't read from file!(magic) " << location;
        return nullptr;
    }
    (void)fseek(fp, 0, SEEK_SET);
    std::unique_ptr<const panda_file::File> file;
    if (IsZipMagic(magic)) {
        file = OpenPandaFileFromZip(fp, location, archive_filename, open_mode);
    } else {
        file = panda_file::File::Open(location, open_mode);
    }
    fclose(fp);
    return file;
}

std::unique_ptr<const File> OpenPandaFileFromMemory(const void *buffer, size_t size, std::string tag)
{
    size_t size_to_mmap = AlignUp(size, panda::os::mem::GetPageSize());
    void *mem = os::mem::MapRWAnonymousRaw(size_to_mmap, false);
    if (mem == nullptr) {
        return nullptr;
    }

    if (memcpy_s(mem, size_to_mmap, buffer, size) != 0) {
        PLOG(ERROR, PANDAFILE) << "Failed to copy buffer into mem'";
    }

    if (!tag.empty()) {
        if (tag == "ArkTS Code") {
            std::string memAddr = std::to_string(ToUintPtr(mem));
            tag = tag + ":" + memAddr;
        }
        auto ret = os::mem::TagAnonymousMemory(mem, size_to_mmap, tag.c_str());
        if (ret.has_value()) {
            PLOG(DEBUG, PANDAFILE) << "Can't tag mmap anonymous, errno: " << errno;
        }
    }

    os::mem::ConstBytePtr ptr(reinterpret_cast<std::byte *>(mem), size_to_mmap, os::mem::MmapDeleter);
    if (ptr.Get() == nullptr) {
        PLOG(ERROR, PANDAFILE) << "Failed to open panda file from memory'";
        return nullptr;
    }
    std::hash<void *> hash;
    return panda_file::File::OpenFromMemory(std::move(ptr), std::to_string(hash(mem)));
}

std::unique_ptr<const File> OpenPandaFileFromSecureMemory(uint8_t *buffer, size_t size)
{
    if (buffer == nullptr) {
        PLOG(ERROR, PANDAFILE) << "OpenPandaFileFromSecureMemory buffer is nullptr'";
        return nullptr;
    }

    if (!CheckSecureMem(reinterpret_cast<uintptr_t>(buffer), size)) {
        PLOG(ERROR, PANDAFILE) << "Secure memory check failed, please execute in secure memory.";
        return nullptr;
    }

    std::byte *mem = reinterpret_cast<std::byte *>(buffer);
    os::mem::ConstBytePtr ptr(mem, size, nullptr);
    if (ptr.Get() == nullptr) {
        PLOG(ERROR, PANDAFILE) << "Failed to open panda file from secure memory'";
        return nullptr;
    }

    std::hash<std::byte *> hash;
    return panda_file::File::OpenFromMemory(std::move(ptr), std::to_string(hash(mem)));
}

inline bool CheckSecureMem(uintptr_t mem, size_t size)
{
    static bool has_open = false;
    static DataProtect start = DataProtect();
    static DataProtect end = DataProtect();
    uintptr_t secure_mem_start;
    uintptr_t secure_mem_end;
    if (!has_open) {
        FILE *fp = fopen(PROC_SELF_XPM_REGION_PATH, "r");
        if (fp == nullptr) {
            LOG(ERROR, PANDAFILE) << "Can not open xpm proc file, do not check secure memory anymore.";
            // No verification is performed when a file fails to be opened.
            has_open = true;
            return true;
        }
        char xpm_validate_region[XPM_PROC_LENGTH] = {0};
        size_t ret = fread(xpm_validate_region, 1, sizeof(xpm_validate_region), fp);
        if (ret <= 0) {
            LOG(ERROR, PANDAFILE) << "Read xpm proc file failed";
            fclose(fp);
            return false;
        }
        fclose(fp);
        if (sscanf_s(xpm_validate_region, "%lx-%lx", &secure_mem_start, &secure_mem_end) <= 0) {
            LOG(ERROR, PANDAFILE) << "sscanf_s xpm validate region failed";
            return false;
        }
        // The check is not performed when the file is already opened.
        has_open = true;
        LOG(DEBUG, PANDAFILE) << "Successfully open xpm region.";
        start.Update(secure_mem_start);
        end.Update(secure_mem_end);
    }
    secure_mem_start = start.GetOriginPointer();
    secure_mem_end = end.GetOriginPointer();
    // xpm proc does not exist, the read value is 0, and the check is not performed.
    if (secure_mem_start == 0 && secure_mem_end == 0) {
        LOG(ERROR, PANDAFILE) << "Secure memory check: xpm proc does not exist, do not check secure memory anymore.";
        return true;
    }
    if (mem < secure_mem_start || (size > (std::numeric_limits<uintptr_t>::max() - mem)) ||
        (mem + size) > secure_mem_end) {
        LOG(ERROR, PANDAFILE) << "Secure memory check failed, mem out of secure memory region.";
        return false;
    }
    return true;
}

class ClassIdxIterator {
public:
    using value_type = const uint8_t *;
    using difference_type = std::ptrdiff_t;
    using pointer = uint32_t *;
    using reference = uint32_t &;
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
        return idx_ - other.idx_;
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
      FILENAME(std::move(filename)),
      FILENAME_HASH(CalcFilenameHash(FILENAME)),
#ifdef ENABLE_FULL_FILE_FIELDS
      FULL_FILENAME(os::GetAbsolutePath(FILENAME)),
      panda_cache_(std::make_unique<PandaCache>()),
#endif
      UNIQ_ID(merge_hashes(FILENAME_HASH, GetHash32(reinterpret_cast<const uint8_t *>(GetHeader()), sizeof(Header))))
{
}

File::~File()
{
    AnonMemSet::GetInstance().Remove(FILENAME);
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
std::unique_ptr<const File> File::Open(std::string_view filename, OpenMode open_mode)
{
    trace::ScopedTrace scoped_trace("Open panda file " + std::string(filename));
    os::file::Mode mode = GetMode(open_mode);
    os::file::File file = os::file::Open(filename, mode);
    if (!file.IsValid()) {
        PLOG(ERROR, PANDAFILE) << "Failed to open panda file '" << filename << "'";
        return nullptr;
    }

    os::file::FileHolder fh_holder(file);

    auto res = file.GetFileSize();
    if (!res) {
        PLOG(ERROR, PANDAFILE) << "Failed to get size of panda file '" << filename << "'";
        return nullptr;
    }

    size_t size = res.Value();
    if (size < sizeof(File::Header)) {
        LOG(ERROR, PANDAFILE) << "Invalid panda file '" << filename << "' - missing or incomplete header" <<
                                 ". Abc file is corrupted";
        return nullptr;
    }

    os::mem::ConstBytePtr ptr = os::mem::MapFile(file, GetProt(open_mode), os::mem::MMAP_FLAG_PRIVATE, size).ToConst();
    if (ptr.Get() == nullptr) {
        PLOG(ERROR, PANDAFILE) << "Failed to map panda file '" << filename << "'";
        return nullptr;
    }

    if (!CheckHeader(ptr, filename)) {
        return nullptr;
    }

    return std::unique_ptr<File>(new File(filename.data(), std::move(ptr)));
}

std::unique_ptr<const File> File::OpenUncompressedArchive(int fd, const std::string_view &filename, size_t size,
                                                          uint32_t offset, OpenMode open_mode)
{
    trace::ScopedTrace scoped_trace("Open panda file " + std::string(filename));
    auto file = os::file::File(fd);
    if (!file.IsValid()) {
        PLOG(ERROR, PANDAFILE) << "OpenUncompressedArchive: Failed to open panda file '" << filename << "'";
        return nullptr;
    }

    if (size < sizeof(File::Header)) {
        LOG(ERROR, PANDAFILE) << "Invalid panda file size '" << filename << "'" << ". Abc file is corrupted";
        return nullptr;
    }
    LOG(DEBUG, PANDAFILE) << " size=" << size << " offset=" << offset << " " << filename;

    os::mem::ConstBytePtr ptr =
        os::mem::MapFile(file, GetProt(open_mode), os::mem::MMAP_FLAG_PRIVATE, size, offset).ToConst();
    if (ptr.Get() == nullptr) {
        PLOG(ERROR, PANDAFILE) << "Failed to map panda file '" << filename << "'";
        return nullptr;
    }
    if (!CheckHeader(ptr, filename)) {
        return nullptr;
    }

    return std::unique_ptr<File>(new File(filename.data(), std::move(ptr)));
}

template <typename T = uint32_t>
bool CheckHeaderElementOffset(size_t offset, size_t number, size_t file_size)
{
    auto number_size = number * sizeof(T);
    if (offset > file_size || number_size > file_size || offset > file_size - number_size) {
        return false;
    }
    return true;
}

bool CheckHeader(const os::mem::ConstBytePtr &ptr, const std::string_view &filename)
{
    if (ptr.Get() == nullptr || ptr.GetSize() < sizeof(File::Header)) {
        LOG(ERROR, PANDAFILE) << "Invalid panda file '" << filename << "'" << ". Abc file is corrupted";
        return false;
    }
    auto header = reinterpret_cast<const File::Header *>(reinterpret_cast<uintptr_t>(ptr.Get()));
    if (header->magic != File::MAGIC) {
        LOG(ERROR, PANDAFILE) << "Invalid magic number" << ". Abc file is corrupted";
        return false;
    }

    CheckFileVersion(header->version, filename);

    if (header->file_size < sizeof(File::Header) || header->file_size > ptr.GetSize()) {
        LOG(ERROR, PANDAFILE) << "Invalid panda file size " << header->file_size << ". Abc file is corrupted";
        return false;
    }

    if (!CheckHeaderElementOffset<uint8_t>(header->foreign_off, header->foreign_size, header->file_size)) {
        LOG(ERROR, PANDAFILE) << "Invalid panda file foreign_off " << header->foreign_off <<
            " or foreign_size " << header->foreign_size << ". Abc file is corrupted";
        return false;
    }

    if (!CheckHeaderElementOffset(header->class_idx_off, header->num_classes, header->file_size)) {
        LOG(ERROR, PANDAFILE) << "Invalid panda file class_idx_off " << header->class_idx_off <<
            " or num_classes " << header->num_classes << ". Abc file is corrupted";
        return false;
    }

    if (!CheckHeaderElementOffset(header->lnp_idx_off, header->num_lnps, header->file_size)) {
        LOG(ERROR, PANDAFILE) << "Invalid panda file lnp_idx_off " << header->lnp_idx_off <<
            " or num_lnps " << header->num_lnps << ". Abc file is corrupted";
        return false;
    }

    if (ContainsLiteralArrayInHeader(header->version)) {
        if (!CheckHeaderElementOffset(header->literalarray_idx_off, header->num_literalarrays, header->file_size)) {
            LOG(ERROR, PANDAFILE) << "Invalid panda file literalarray_idx_off " << header->literalarray_idx_off <<
                                     " or num_literalarrays " << header->num_literalarrays <<
                                     ". Abc file is corrupted";
            return false;
        }
    } else {
        if (header->literalarray_idx_off != INVALID_INDEX || header->num_literalarrays != INVALID_OFFSET) {
            LOG(ERROR, PANDAFILE) << "Invalid panda file literalarray_idx_off " << header->literalarray_idx_off <<
                                     " or num_literalarrays " << header->num_literalarrays <<
                                     ", The literalarray_idx_off and num_literalarrays should be reserved." <<
                                     " Abc file is corrupted";
            return false;
        }
    }

    if (!CheckHeaderElementOffset<File::IndexHeader>(header->index_section_off, header->num_indexes,
        header->file_size)) {
        LOG(ERROR, PANDAFILE) << "Invalid panda file index_section_off " << header->index_section_off <<
            " or num_indexes " << header->num_indexes << ". Abc file is corrupted";
        return false;
    }

    return true;
}

void CheckFileVersion(const std::array<uint8_t, File::VERSION_SIZE> &file_version, const std::string_view &filename)
{
#ifdef ERROR_AS_FATAL
#define LOG_LEVEL FATAL
#else
#define LOG_LEVEL ERROR
#endif
    if (file_version == version) {
        return;
    }
    if (file_version < minVersion) {
        LOG(LOG_LEVEL, PANDAFILE) << "Unable to open file '" << filename << "' with abc file version "
            << VersionToString(file_version)
            << ". Minimum supported abc file version on the current system image is " << VersionToString(minVersion)
            << ". Please upgrade the sdk tools to generate supported version of abc files "
            << "or execute the abc file on former version of system image";
    } else if (file_version > version) {
        LOG(LOG_LEVEL, PANDAFILE) << "Unable to open file '" << filename << "' with abc file version "
            << VersionToString(file_version)
            << ". Maximum supported abc file version on the current system image is " << VersionToString(version)
            << ". Please upgrade the system image or use former version of SDK tools to generate abc files";
    } else if (incompatibleVersion.count(file_version) != 0) {
        LOG(LOG_LEVEL, PANDAFILE) << "Unable to open file '" << filename << "' with  abc file version "
            << VersionToString(file_version) << ". Current system image version is "
            << VersionToString(version) << ", while abc file version is " << VersionToString(file_version)
            << ". The version "<< VersionToString(file_version)
            << " is not a compatible version, can't run on system image of version " << VersionToString(version)
            << ". Please use sdk tools and system image in pairs "
            << "and make the version of sdk tools and system image consistent";
    }
#undef LOG_LEVEL
}

PandaFileType GetFileType(const uint8_t *data, int32_t size)
{
    if (data == nullptr) {
        LOG(ERROR, PANDAFILE) << "Invalid panda file, data is null";
        return PandaFileType::FILE_FORMAT_INVALID;
    }

    uint32_t actualSize = static_cast<uint32_t>(size);
    if (actualSize < sizeof(File::Header)) {
        LOG(ERROR, PANDAFILE) << "Invalid panda file, size=" << actualSize;
        return PandaFileType::FILE_FORMAT_INVALID;
    }

    auto header = reinterpret_cast<const File::Header *>(data);
    if (actualSize != header->file_size) {
        LOG(ERROR, PANDAFILE) << "File actual size [" << actualSize << "] is not equal to Header's fileSize ["
                              << header->file_size << "]";
        return PandaFileType::FILE_FORMAT_INVALID;
    }

    if (File::MAGIC != header->magic) {
        LOG(ERROR, PANDAFILE) << "Invalid magic number. Abc file is corrupted";
        return PandaFileType::FILE_FORMAT_INVALID;
    }

    if (header->version == File::STATIC_VERSION) {
        return PandaFileType::FILE_STATIC;
    }
    return PandaFileType::FILE_DYNAMIC;
}

/* static */
std::unique_ptr<const File> File::OpenFromMemory(os::mem::ConstBytePtr &&ptr)
{
    if (!CheckHeader(ptr, std::string_view())) {
        return nullptr;
    }

    return std::unique_ptr<File>(new File("", std::forward<os::mem::ConstBytePtr>(ptr)));
}

/* static */
std::unique_ptr<const File> File::OpenFromMemory(os::mem::ConstBytePtr &&ptr, std::string_view filename)
{
    trace::ScopedTrace scoped_trace("Open panda file from RAM " + std::string(filename));

    if (!CheckHeader(ptr, filename)) {
        return nullptr;
    }

    return std::unique_ptr<File>(new File(filename.data(), std::forward<os::mem::ConstBytePtr>(ptr)));
}

File::EntityId File::GetClassId(const uint8_t *mutf8_name) const
{
    auto class_hash_table = GetClassHashTable();
    if (!class_hash_table.empty()) {
        return GetClassIdFromClassHashTable(mutf8_name);
    }

    auto class_idx = GetClasses();

    auto it = std::lower_bound(ClassIdxIterator::Begin(*this, class_idx), ClassIdxIterator::End(*this, class_idx),
                               mutf8_name, utf::Mutf8Less());
    if (!it.IsValid()) {
        return EntityId();
    }

    if (utf::CompareMUtf8ToMUtf8(mutf8_name, *it) == 0) {
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
    return EntityId(header->literalarray_idx_off);
}

File::EntityId File::GetClassIdFromClassHashTable(const uint8_t *mutf8_name) const
{
    auto class_hash_table = GetClassHashTable();
    auto hash = GetHash32String(mutf8_name);
    auto pos = hash & (class_hash_table.size() - 1);
    auto entity_pair = &class_hash_table[pos];

    if (entity_pair->descriptor_hash % class_hash_table.size() != pos) {
        return File::EntityId();
    }

    while (true) {
        if (hash == entity_pair->descriptor_hash) {
            auto entity_id = File::EntityId(entity_pair->entity_id_offset);
            auto descriptor = GetStringData(entity_id).data;
            if (entity_id.IsValid() && utf::CompareMUtf8ToMUtf8(descriptor, mutf8_name) == 0) {
                return entity_id;
            }
        }
        if (entity_pair->next_pos == 0) {
            break;
        }
        entity_pair = &class_hash_table[entity_pair->next_pos - 1];
    }

    return File::EntityId();
}


bool ContainsLiteralArrayInHeader(const std::array<uint8_t, File::VERSION_SIZE> &version)
{
    return panda::panda_file::IsVersionLessOrEqual(version, LAST_CONTAINS_LITERAL_IN_HEADER_VERSION);
}

bool File::ValidateChecksum(uint32_t *cal_checksum_out) const
{
    if (UNLIKELY(GetHeader() == nullptr)) {
        LOG(FATAL, PANDAFILE) << "Header pointer is nullptr. Abc file is corrupted";
    }
    constexpr uint32_t CHECKSUM_SIZE = 4U;
    // The checksum calculation does not include magic or checksum, so the offset needs to be added
    constexpr uint32_t FILE_CONTENT_OFFSET = File::MAGIC_SIZE + CHECKSUM_SIZE;
    uint32_t file_size = GetHeader()->file_size;
    uint32_t cal_checksum = adler32(1, GetBase() + FILE_CONTENT_OFFSET, file_size - FILE_CONTENT_OFFSET);

    if (cal_checksum_out != nullptr) {
        *cal_checksum_out = cal_checksum;
    }

    return GetHeader()->checksum == cal_checksum;
}

void File::ThrowIfWithCheck(bool cond, const std::string_view& msg, const std::string_view& tag) const
{
    if (UNLIKELY(cond)) {
#ifndef SUPPORT_KNOWN_EXCEPTION
        uint32_t cal_checksum = 0;
        bool is_checksum_match = ValidateChecksum(&cal_checksum);
        if (!is_checksum_match) {
            LOG(FATAL, PANDAFILE) << msg << ", checksum mismatch. The abc file has been corrupted. "
                                         << "Expected checksum: 0x" << std::hex << GetHeader()->checksum
                                         << ", Actual checksum: 0x" << std::hex << cal_checksum;
        }

        if (!tag.empty()) {
            LOG(FATAL, PANDAFILE) << msg << ", from method: " << tag;
        } else {
            LOG(FATAL, PANDAFILE) << msg;
        }
#else
        throw helpers::FileAccessException(msg);
#endif
    }
}
}  // namespace panda::panda_file
