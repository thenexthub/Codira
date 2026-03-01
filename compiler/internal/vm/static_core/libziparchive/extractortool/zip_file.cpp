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

#include "zip_file.h"

#include <ostream>
#include <string>

#include <securec.h>

#include "file_mapper.h"
#include "zip_file_reader.h"

namespace ark::extractor {
namespace {
constexpr uint32_t MAX_FILE_NAME = 4096;
constexpr uint32_t UNZIP_BUFFER_SIZE = 1024;
constexpr uint32_t UNZIP_BUF_IN_LEN = 160 * UNZIP_BUFFER_SIZE;   // in  buffer length: 160KB
constexpr uint32_t UNZIP_BUF_OUT_LEN = 320 * UNZIP_BUFFER_SIZE;  // out buffer length: 320KB
constexpr uint32_t LOCAL_HEADER_SIGNATURE = 0x04034b50;
constexpr uint32_t CENTRAL_SIGNATURE = 0x02014b50;
constexpr uint32_t EOCD_SIGNATURE = 0x06054b50;
constexpr uint32_t DATA_DESC_SIGNATURE = 0x08074b50;
constexpr uint32_t FLAG_DATA_DESC = 0x8;
constexpr uint8_t INFLATE_ERROR_TIMES = 5;
constexpr uint8_t MAP_FILE_SUFFIX = 4;
constexpr char FILE_SEPARATOR_CHAR = '/';
constexpr const char *WRONG_FILE_SEPARATOR = "//";
constexpr uint32_t CACHE_CASE_THRESHOLD = 10000;

// NOLINTNEXTLINE(performance-unnecessary-value-param)
void GetTreeFileList(std::shared_ptr<DirTreeNode> root, const std::string &rootPath,
                     std::vector<std::string> &assetList)
{
    if (root == nullptr) {
        return;
    }
    if (!root->isDir && !rootPath.empty()) {
        assetList.push_back(rootPath);
    } else {
        std::string prefix = rootPath;
        if (!prefix.empty()) {
            prefix.push_back(FILE_SEPARATOR_CHAR);
        }
        for (const auto &child : root->children) {
            GetTreeFileList(child.second, prefix + child.first, assetList);
        }
    }
}

// NOLINTNEXTLINE(performance-unnecessary-value-param)
void AddEntryToTree(const std::string &fileName, std::shared_ptr<DirTreeNode> root)
{
    if (root == nullptr) {
        return;
    }
    size_t cur = 0;
    auto parent = root;
    do {
        while (cur < fileName.size() && fileName[cur] == FILE_SEPARATOR_CHAR) {
            cur++;
        }
        if (cur >= fileName.size()) {
            break;
        }
        auto next = fileName.find(FILE_SEPARATOR_CHAR, cur);
        auto nodeName = fileName.substr(cur, next - cur);
        auto it = parent->children.find(nodeName);
        if (it != parent->children.end()) {
            parent = it->second;
        } else {
            auto node = std::make_shared<DirTreeNode>();
            node->isDir = next != std::string::npos;
            parent->children.emplace(nodeName, node);
            parent = node;
        }
        cur = next;
    } while (cur != std::string::npos);
}

inline bool IsRootDir(const std::string &dirName)
{
    return dirName.size() == 1 && dirName.back() == FILE_SEPARATOR_CHAR;
}
}  // namespace

ZipEntry::ZipEntry(const CentralDirEntry &centralEntry)
{
    compressionMethod = centralEntry.compressionMethod;
    uncompressedSize = centralEntry.uncompressedSize;
    compressedSize = centralEntry.compressedSize;
    localHeaderOffset = centralEntry.localHeaderOffset;
    crc = centralEntry.crc;
    flags = centralEntry.flags;
    modifiedTime = centralEntry.modifiedTime;
    modifiedDate = centralEntry.modifiedDate;
}

ZipFile::ZipFile(const std::string &pathName) : pathName_(pathName) {}  // NOLINT(modernize-pass-by-value)

ZipFile::~ZipFile()
{
    Close();
}

bool ZipFile::CheckEndDir(const EndDir &endDir) const
{
    size_t lenEndDir = sizeof(EndDir);
    if ((endDir.numDisk != 0) || (endDir.signature != EOCD_SIGNATURE) || (endDir.startDiskOfCentralDir != 0) ||
        (endDir.offset >= fileLength_) || (endDir.totalEntriesInThisDisk != endDir.totalEntries) ||
        (endDir.commentLen != 0) ||
        // central dir can't overlap end of central dir
        ((endDir.offset + endDir.sizeOfCentralDir + lenEndDir) > fileLength_)) {
        LOG(WARNING, ZIPARCHIVE) << "failed: fileLen: " << fileLength_ << ", signature: " << endDir.signature
                                 << ", numDisk: " << endDir.numDisk
                                 << ", startDiskOfCentralDir: " << endDir.startDiskOfCentralDir
                                 << ", totalEntriesInThisDisk: " << endDir.totalEntriesInThisDisk
                                 << ", totalEntries: " << endDir.totalEntries
                                 << ", sizeOfCentralDir: " << endDir.sizeOfCentralDir << ", offset: " << endDir.offset
                                 << ", commentLen: " << endDir.commentLen;
        return false;
    }
    return true;
}

bool ZipFile::ParseEndDirectory()
{
    size_t endDirLen = sizeof(EndDir);
    size_t endFilePos = fileStartPos_ + fileLength_;

    if (fileLength_ <= endDirLen) {
        LOG(ERROR, ZIPARCHIVE) << "fileStartPos_:" << fileStartPos_ << " <= fileLength_:" << fileLength_;
        return false;
    }

    size_t eocdPos = endFilePos - endDirLen;
    if (!zipFileReader_->ReadBuffer(reinterpret_cast<uint8_t *>(&endDir_), eocdPos, sizeof(EndDir))) {
        LOG(ERROR, ZIPARCHIVE) << "read EOCD failed";
        return false;
    }

    centralDirPos_ = endDir_.offset + fileStartPos_;

    return CheckEndDir(endDir_);
}

bool ZipFile::ParseOneEntry(uint8_t *&entryPtr)
{
    if (entryPtr == nullptr) {
        LOG(ERROR, ZIPARCHIVE) << "null entryPtr";
        return false;
    }

    CentralDirEntry directoryEntry;
    if (memcpy_s(&directoryEntry, sizeof(CentralDirEntry), entryPtr, sizeof(CentralDirEntry)) != EOK) {
        LOG(ERROR, ZIPARCHIVE) << "Mem copy directory entry failed";
        return false;
    }

    if (directoryEntry.signature != CENTRAL_SIGNATURE) {
        LOG(ERROR, ZIPARCHIVE) << "check signature failed";
        return false;
    }

    entryPtr += sizeof(CentralDirEntry);  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    size_t fileLength = (directoryEntry.nameSize >= MAX_FILE_NAME) ? (MAX_FILE_NAME - 1) : directoryEntry.nameSize;
    std::string fileName(fileLength, 0);
    if (memcpy_s(&(fileName[0]), fileLength, entryPtr, fileLength) != EOK) {
        LOG(ERROR, ZIPARCHIVE) << "Mem copy file name failed";
        return false;
    }

    ZipEntry currentEntry(directoryEntry);
    currentEntry.fileName = fileName;
    entriesMap_[fileName] = currentEntry;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    entryPtr += directoryEntry.nameSize + directoryEntry.extraSize + directoryEntry.commentSize;
    return true;
}

std::shared_ptr<DirTreeNode> ZipFile::MakeDirTree() const
{
    auto root = std::make_shared<DirTreeNode>();
    root->isDir = true;
    for (const auto &[fileName, entry] : entriesMap_) {
        AddEntryToTree(fileName, root);
    }
    return root;
}

std::shared_ptr<DirTreeNode> ZipFile::GetDirRoot()
{
    if (!isOpen_) {
        return nullptr;
    }
    os::memory::LockHolder lock(dirRootMutex_);
    if (dirRoot_ == nullptr) {
        dirRoot_ = MakeDirTree();
    }
    return dirRoot_;
}

bool ZipFile::ParseAllEntries()
{
    auto centralData =
        zipFileReader_->ReadBuffer(static_cast<size_t>(centralDirPos_), static_cast<size_t>(endDir_.sizeOfCentralDir));
    if (centralData.empty()) {
        LOG(ERROR, ZIPARCHIVE) << "centralData empty for " << pathName_ << " failed";
        return false;
    }

    bool ret = true;
    auto *entryPtr = reinterpret_cast<uint8_t *>(centralData.data());
    for (uint16_t i = 0; i < endDir_.totalEntries; i++) {
        if (!ParseOneEntry(entryPtr)) {
            LOG(ERROR, ZIPARCHIVE) << "Parse entry" << i << " failed";
            ret = false;
            break;
        }
    }

    return ret;
}

bool ZipFile::Open()
{
    if (isOpen_) {
        return true;
    }

    if (pathName_.length() > PATH_MAX) {
        LOG(ERROR, ZIPARCHIVE) << "pathName length > PATH_MAX";
        return false;
    }

    zipFileReader_ = ZipFileReader::CreateZipFileReader(pathName_);
    if (!zipFileReader_) {
        LOG(ERROR, ZIPARCHIVE) << "open file error: " << pathName_ << ", errno: " << errno;
        return false;
    }

    if (fileLength_ == 0) {
        auto fileLength = zipFileReader_->GetFileLen();
        fileLength_ = static_cast<ZipPos>(fileLength);
        if (fileStartPos_ >= fileLength_) {
            zipFileReader_.reset();
            return false;
        }

        fileLength_ -= fileStartPos_;
    }

    bool result = ParseEndDirectory();
    if (result) {
        result = ParseAllEntries();
    }
    // it means open file success.
    isOpen_ = true;
    return result;
}

void ZipFile::Close()
{
    if (!isOpen_ || zipFileReader_ == nullptr) {
        return;
    }

    isOpen_ = false;
    entriesMap_.clear();
    {
        os::memory::LockHolder lock(dirRootMutex_);
        dirRoot_.reset();
    }
    pathName_ = "";

    zipFileReader_.reset();
}

// Get all file zipEntry in this file
const ZipEntryMap &ZipFile::GetAllEntries() const
{
    return entriesMap_;
}

bool ZipFile::HasEntry(const std::string &entryName) const
{
    return entriesMap_.find(entryName) != entriesMap_.end();
}

void ZipFile::SetCacheMode(CacheMode cacheMode)
{
    os::memory::LockHolder lock(dirRootMutex_);
    cacheMode_ = cacheMode;
    if (!UseDirCache()) {
        dirRoot_.reset();
    }
}

bool ZipFile::UseDirCache() const
{
    auto mode = cacheMode_;
    bool useCache = mode == CacheMode::CACHE_ALL;
    if (mode == CacheMode::CACHE_CASE && entriesMap_.size() >= CACHE_CASE_THRESHOLD) {
        useCache = true;
    }
    return useCache;
}

bool ZipFile::IsDirExist(const std::string &dir)
{
    if (dir.empty()) {
        LOG(ERROR, ZIPARCHIVE) << "dir is empty";
        return false;
    }
    if (IsRootDir(dir)) {
        return true;
    }
    if (dir.find(WRONG_FILE_SEPARATOR) != std::string::npos) {
        LOG(WARNING, ZIPARCHIVE) << "Wrong format";
        return false;
    }

    auto tmpDir = dir;
    if (tmpDir.front() == FILE_SEPARATOR_CHAR) {
        tmpDir.erase(tmpDir.begin());
    }
    if (tmpDir.back() != FILE_SEPARATOR_CHAR) {
        tmpDir.push_back(FILE_SEPARATOR_CHAR);
    }
    if (entriesMap_.count(tmpDir) > 0) {
        return true;
    }
    tmpDir.pop_back();
    if (entriesMap_.count(tmpDir) > 0) {
        LOG(WARNING, ZIPARCHIVE) << "file not dir";
        return false;
    }

    if (UseDirCache()) {
        return IsDirExistCache(tmpDir);
    }
    return IsDirExistNormal(tmpDir);
}

void ZipFile::GetAllFileList(const std::string &srcPath, std::vector<std::string> &assetList)
{
    if (srcPath.empty()) {
        LOG(ERROR, ZIPARCHIVE) << "dir is empty";
        return;
    }
    if (IsRootDir(srcPath)) {
        for (const auto &[fileName, fileInfo] : entriesMap_) {
            if (!fileName.empty() && fileName.back() != FILE_SEPARATOR_CHAR) {
                assetList.push_back(fileName);
            }
        }
        return;
    }
    if (srcPath.find(WRONG_FILE_SEPARATOR) != std::string::npos) {
        LOG(WARNING, ZIPARCHIVE) << "Wrong format";
        return;
    }

    auto tmpDir = srcPath;
    if (tmpDir.front() == FILE_SEPARATOR_CHAR) {
        tmpDir.erase(tmpDir.begin());
    }
    if (tmpDir.back() != FILE_SEPARATOR_CHAR) {
        tmpDir.push_back(FILE_SEPARATOR_CHAR);
    }
    if (entriesMap_.count(tmpDir) > 0) {
        return;
    }
    tmpDir.pop_back();
    if (entriesMap_.count(tmpDir) > 0) {
        LOG(WARNING, ZIPARCHIVE) << "file not dir";
        return;
    }

    if (UseDirCache()) {
        GetAllFileListCache(tmpDir, assetList);
    } else {
        GetAllFileListNormal(tmpDir, assetList);
    }
}

void ZipFile::GetChildNames(const std::string &srcPath, std::set<std::string> &fileSet)
{
    if (srcPath.empty()) {
        LOG(ERROR, ZIPARCHIVE) << "dir is empty";
        return;
    }
    if (srcPath.find(WRONG_FILE_SEPARATOR) != std::string::npos) {
        LOG(WARNING, ZIPARCHIVE) << "Wrong format";
        return;
    }
    auto tmpDir = srcPath;
    if (!IsRootDir(tmpDir)) {
        if (tmpDir.front() == FILE_SEPARATOR_CHAR) {
            tmpDir.erase(tmpDir.begin());
        }
        if (tmpDir.back() != FILE_SEPARATOR_CHAR) {
            tmpDir.push_back(FILE_SEPARATOR_CHAR);
        }
        if (entriesMap_.count(tmpDir) > 0) {
            return;
        }
        tmpDir.pop_back();
        if (entriesMap_.count(tmpDir) > 0) {
            LOG(WARNING, ZIPARCHIVE) << "file not dir";
            return;
        }
    }

    if (UseDirCache()) {
        GetChildNamesCache(tmpDir, fileSet);
    } else {
        GetChildNamesNormal(tmpDir, fileSet);
    }
}

bool ZipFile::IsDirExistCache(const std::string &dir)
{
    auto parent = GetDirRoot();
    if (parent == nullptr) {
        LOG(ERROR, ZIPARCHIVE) << "null parent";
        return false;
    }
    size_t cur = 0;
    do {
        while (cur < dir.size() && dir[cur] == FILE_SEPARATOR_CHAR) {
            cur++;
        }
        if (cur >= dir.size()) {
            break;
        }
        auto next = dir.find(FILE_SEPARATOR_CHAR, cur);
        auto nodeName = dir.substr(cur, next - cur);
        auto it = parent->children.find(nodeName);
        if (it == parent->children.end()) {
            LOG(ERROR, ZIPARCHIVE) << "dir not found, dir : " << dir;
            return false;
        }
        parent = it->second;
        cur = next;
    } while (cur != std::string::npos);

    return true;
}

void ZipFile::GetAllFileListCache(const std::string &srcPath, std::vector<std::string> &assetList)
{
    auto parent = GetDirRoot();
    if (parent == nullptr) {
        LOG(ERROR, ZIPARCHIVE) << "null parent";
        return;
    }

    auto rootName = srcPath.back() == FILE_SEPARATOR_CHAR ? srcPath.substr(0, srcPath.length() - 1) : srcPath;

    size_t cur = 0;
    do {
        while (cur < rootName.size() && rootName[cur] == FILE_SEPARATOR_CHAR) {
            cur++;
        }
        if (cur >= rootName.size()) {
            break;
        }
        auto next = rootName.find(FILE_SEPARATOR_CHAR, cur);
        auto nodeName = rootName.substr(cur, next - cur);
        auto it = parent->children.find(nodeName);
        if (it == parent->children.end()) {
            LOG(ERROR, ZIPARCHIVE) << "srcPath not found, srcPath : " << rootName;
            return;
        }
        parent = it->second;
        cur = next;
    } while (cur != std::string::npos);

    GetTreeFileList(parent, rootName, assetList);
}

void ZipFile::GetChildNamesCache(const std::string &srcPath, std::set<std::string> &fileSet)
{
    size_t cur = 0;
    auto parent = GetDirRoot();
    if (parent == nullptr) {
        LOG(ERROR, ZIPARCHIVE) << "null parent";
        return;
    }
    do {
        while (cur < srcPath.size() && srcPath[cur] == FILE_SEPARATOR_CHAR) {
            cur++;
        }
        if (cur >= srcPath.size()) {
            break;
        }
        auto next = srcPath.find(FILE_SEPARATOR_CHAR, cur);
        auto nodeName = srcPath.substr(cur, next - cur);
        auto it = parent->children.find(nodeName);
        if (it == parent->children.end()) {
            LOG(ERROR, ZIPARCHIVE) << "srcPath not found, srcPath : " << srcPath;
            return;
        }
        parent = it->second;
        cur = next;
    } while (cur != std::string::npos);

    for (const auto &child : parent->children) {
        fileSet.insert(child.first);
    }
}

bool ZipFile::IsDirExistNormal(const std::string &dir)
{
    auto targetDir = dir;
    if (targetDir.back() != FILE_SEPARATOR_CHAR) {
        targetDir.push_back(FILE_SEPARATOR_CHAR);
    }
    for (const auto &[fileName, fileInfo] : entriesMap_) {
        if (fileName.size() > targetDir.size() && fileName.substr(0, targetDir.size()) == targetDir) {
            return true;
        }
    }
    return false;
}

void ZipFile::GetAllFileListNormal(const std::string &srcPath, std::vector<std::string> &assetList)
{
    auto targetDir = srcPath;
    if (targetDir.back() != FILE_SEPARATOR_CHAR) {
        targetDir.push_back(FILE_SEPARATOR_CHAR);
    }
    for (const auto &[fileName, fileInfo] : entriesMap_) {
        if (fileName.size() > targetDir.size() && fileName.back() != FILE_SEPARATOR_CHAR &&
            fileName.substr(0, targetDir.size()) == targetDir) {
            assetList.push_back(fileName);
        }
    }
}

void ZipFile::GetChildNamesNormal(const std::string &srcPath, std::set<std::string> &fileSet)
{
    auto targetDir = srcPath;
    if (targetDir.back() != FILE_SEPARATOR_CHAR) {
        targetDir.push_back(FILE_SEPARATOR_CHAR);
    }
    if (IsRootDir(srcPath)) {
        for (const auto &[fileName, fileInfo] : entriesMap_) {
            auto nextPos = fileName.find(FILE_SEPARATOR_CHAR);
            fileSet.insert(nextPos == std::string::npos ? fileName : fileName.substr(0, nextPos));
        }
        return;
    }
    for (const auto &[fileName, fileInfo] : entriesMap_) {
        if (fileName.size() > targetDir.size() && fileName.substr(0, targetDir.size()) == targetDir) {
            fileSet.insert(fileName.substr(targetDir.size(),
                                           fileName.find(FILE_SEPARATOR_CHAR, targetDir.size()) - targetDir.size()));
        }
    }
}

bool ZipFile::GetEntry(const std::string &entryName, ZipEntry &resultEntry) const
{
    auto iter = entriesMap_.find(entryName);
    if (iter != entriesMap_.end()) {
        resultEntry = iter->second;
        return true;
    }
    return false;
}

size_t ZipFile::GetLocalHeaderSize(const uint16_t nameSize, const uint16_t extraSize) const
{
    return sizeof(LocalHeader) + nameSize + extraSize;
}

bool ZipFile::CheckDataDesc(const ZipEntry &zipEntry, const LocalHeader &localHeader) const
{
    uint32_t crcLocal = 0;
    uint32_t compressedLocal = 0;
    uint32_t uncompressedLocal = 0;

    if ((localHeader.flags & FLAG_DATA_DESC) != 0U) {  // use data desc
        DataDesc dataDesc;
        auto descPos = zipEntry.localHeaderOffset + GetLocalHeaderSize(localHeader.nameSize, localHeader.extraSize);
        descPos += fileStartPos_ + zipEntry.compressedSize;

        if (!zipFileReader_->ReadBuffer(reinterpret_cast<uint8_t *>(&dataDesc), descPos, sizeof(DataDesc))) {
            LOG(ERROR, ZIPARCHIVE) << "ReadBuffer failed";
            return false;
        }

        if (dataDesc.signature != DATA_DESC_SIGNATURE) {
            LOG(ERROR, ZIPARCHIVE) << "check signature failed";
            return false;
        }

        crcLocal = dataDesc.crc;
        compressedLocal = dataDesc.compressedSize;
        uncompressedLocal = dataDesc.uncompressedSize;
    } else {
        crcLocal = localHeader.crc;
        compressedLocal = localHeader.compressedSize;
        uncompressedLocal = localHeader.uncompressedSize;
    }

    if ((zipEntry.crc != crcLocal) || (zipEntry.compressedSize != compressedLocal) ||
        (zipEntry.uncompressedSize != uncompressedLocal)) {
        LOG(ERROR, ZIPARCHIVE) << "size corrupted";
        return false;
    }

    return true;
}

bool ZipFile::CheckCoherencyLocalHeader(const ZipEntry &zipEntry, uint16_t &extraSize) const
{
    // current only support store and Z_DEFLATED method
    if ((zipEntry.compressionMethod != Z_DEFLATED) && (zipEntry.compressionMethod != 0)) {
        LOG(ERROR, ZIPARCHIVE) << "compressionMethod " << zipEntry.compressionMethod << " not support";
        return false;
    }

    auto nameSize = zipEntry.fileName.length();
    auto startPos = fileStartPos_ + zipEntry.localHeaderOffset;
    size_t buffSize = sizeof(LocalHeader) + nameSize;
    auto buff = zipFileReader_->ReadBuffer(startPos, buffSize);
    if (buff.size() < buffSize) {
        LOG(ERROR, ZIPARCHIVE) << "read header failed";
        return false;
    }

    LocalHeader localHeader = {0};
    if (memcpy_s(&localHeader, sizeof(LocalHeader), buff.data(), sizeof(LocalHeader)) != EOK) {
        LOG(ERROR, ZIPARCHIVE) << "memcpy localheader failed";
        return false;
    }
    if ((localHeader.signature != LOCAL_HEADER_SIGNATURE) ||
        (zipEntry.compressionMethod != localHeader.compressionMethod)) {
        LOG(ERROR, ZIPARCHIVE) << "signature or compressionMethod failed";
        return false;
    }

    if (localHeader.nameSize != nameSize && nameSize < MAX_FILE_NAME - 1) {
        LOG(ERROR, ZIPARCHIVE) << "name corrupted";
        return false;
    }
    std::string fileName = buff.substr(sizeof(LocalHeader));
    if (zipEntry.fileName != fileName) {
        LOG(ERROR, ZIPARCHIVE) << "name corrupted";
        return false;
    }

    if (!CheckDataDesc(zipEntry, localHeader)) {
        LOG(ERROR, ZIPARCHIVE) << "check data desc failed";
        return false;
    }

    extraSize = localHeader.extraSize;
    return true;
}

size_t ZipFile::GetEntryStart(const ZipEntry &zipEntry, const uint16_t extraSize) const
{
    ZipPos startOffset = zipEntry.localHeaderOffset;
    // get data offset, add signature+localheader+namesize+extrasize
    startOffset += GetLocalHeaderSize(zipEntry.fileName.length(), extraSize);
    startOffset += fileStartPos_;  // add file start relative to file stream

    return startOffset;
}

bool ZipFile::InitZStream(z_stream &zstream) const
{
    // init zlib stream
    if (memset_s(&zstream, sizeof(z_stream), 0, sizeof(z_stream)) != 0) {
        LOG(ERROR, ZIPARCHIVE) << "stream buffer init failed";
        return false;
    }
    int32_t zlibErr = inflateInit2(&zstream, -MAX_WBITS);
    if (zlibErr != Z_OK) {
        LOG(ERROR, ZIPARCHIVE) << "init failed";
        return false;
    }

    BytePtr bufOut = new (std::nothrow) Byte[UNZIP_BUF_OUT_LEN];  // NOLINT(modernize-use-auto)
    if (bufOut == nullptr) {
        LOG(ERROR, ZIPARCHIVE) << "null bufOut";
        return false;
    }

    BytePtr bufIn = new (std::nothrow) Byte[UNZIP_BUF_IN_LEN];  // NOLINT(modernize-use-auto)
    if (bufIn == nullptr) {
        LOG(ERROR, ZIPARCHIVE) << "null bufIn";
        delete[] bufOut;
        return false;
    }
    zstream.next_out = bufOut;
    zstream.next_in = bufIn;
    zstream.avail_out = UNZIP_BUF_OUT_LEN;
    return true;
}

ZipPos ZipFile::GetEntryDataOffset(const ZipEntry &zipEntry, const uint16_t extraSize) const
{
    // get entry data offset relative file
    ZipPos offset = zipEntry.localHeaderOffset;

    offset += GetLocalHeaderSize(zipEntry.fileName.length(), extraSize);
    offset += fileStartPos_;

    return offset;
}

bool ZipFile::GetDataOffsetRelative(const ZipEntry &zipEntry, ZipPos &offset, uint32_t &length) const
{
    uint16_t extraSize = 0;
    if (!CheckCoherencyLocalHeader(zipEntry, extraSize)) {
        LOG(ERROR, ZIPARCHIVE) << "check coherency local header failed";
        return false;
    }

    offset = GetEntryDataOffset(zipEntry, extraSize);
    length = zipEntry.compressedSize;
    return true;
}

// NOLINTNEXTLINE(modernize-avoid-c-arrays)
bool ZipFile::ExtractFileFromMMap(const std::string &file, void *mmapDataPtr, std::unique_ptr<uint8_t[]> &dataPtr,
                                  size_t &len) const
{
    ZipEntry zipEntry;
    if (!GetEntry(file, zipEntry)) {
        LOG(ERROR, ZIPARCHIVE) << "not find file";
        return false;
    }

    if (zipEntry.compressionMethod == 0U) {
        LOG(ERROR, ZIPARCHIVE) << "file is not extracted, file: " << file;
        return false;
    }

    uint16_t extraSize = 0;
    if (!CheckCoherencyLocalHeader(zipEntry, extraSize)) {
        LOG(ERROR, ZIPARCHIVE) << "check coherency local header failed";
        return false;
    }

    return UnzipWithInflatedFromMMap(zipEntry, extraSize, mmapDataPtr, dataPtr, len);
}

bool ZipFile::UnzipWithInflatedFromMMap(const ZipEntry &zipEntry, [[maybe_unused]] const uint16_t extraSize,
                                        // NOLINTNEXTLINE(modernize-avoid-c-arrays)
                                        void *mmapDataPtr, std::unique_ptr<uint8_t[]> &dataPtr, size_t &len) const
{
    z_stream zstream;
    if (!InitZStream(zstream)) {
        LOG(ERROR, ZIPARCHIVE) << "init zstream failed";
        return false;
    }

    BytePtr bufIn = zstream.next_in;
    BytePtr bufOut = zstream.next_out;

    bool ret = true;
    int32_t zlibErr = Z_OK;
    uint32_t remainCompressedSize = zipEntry.compressedSize;
    size_t inflateLen = 0;
    uint8_t errorTimes = 0;

    len = zipEntry.uncompressedSize;
    dataPtr = std::make_unique<uint8_t[]>(len);  // NOLINT(modernize-avoid-c-arrays)
    auto *dstDataPtr = static_cast<uint8_t *>(dataPtr.get());
    void *mmapSrcDataPtr = mmapDataPtr;

    while ((remainCompressedSize > 0) || (zstream.avail_in > 0)) {
        if (!ReadZStreamFromMMap(bufIn, mmapSrcDataPtr, zstream, remainCompressedSize)) {
            ret = false;
            break;
        }

        zlibErr = inflate(&zstream, Z_SYNC_FLUSH);
        if ((zlibErr >= Z_OK) && (zstream.msg != nullptr)) {
            LOG(ERROR, ZIPARCHIVE) << "unzip failed, zlibErr: " << zlibErr << ", msg: " << zstream.msg;
            ret = false;
            break;
        }

        inflateLen = UNZIP_BUF_OUT_LEN - zstream.avail_out;
        if (!CopyInflateOut(zstream, inflateLen, &dstDataPtr, bufOut, errorTimes)) {
            break;
        }
    }

    // free all dynamically allocated data structures except the next_in and next_out for this stream.
    zlibErr = inflateEnd(&zstream);
    if (zlibErr != Z_OK) {
        LOG(ERROR, ZIPARCHIVE) << "inflateEnd failed, zlibErr: " << zlibErr;
        ret = false;
    }

    delete[] bufOut;
    delete[] bufIn;
    return ret;
}

bool ZipFile::CopyInflateOut(z_stream &zstream, size_t inflateLen, uint8_t **dstDataPtr, BytePtr bufOut,
                             uint8_t &errorTimes) const
{
    if (inflateLen > 0) {
        if (memcpy_s(*dstDataPtr, inflateLen, bufOut, inflateLen) != EOK) {
            LOG(ERROR, ZIPARCHIVE) << "memcpy failed";
            return false;
        }

        *dstDataPtr += inflateLen;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        zstream.next_out = bufOut;
        zstream.avail_out = UNZIP_BUF_OUT_LEN;
        errorTimes = 0;
    } else {
        errorTimes++;
    }
    if (errorTimes >= INFLATE_ERROR_TIMES) {
        LOG(ERROR, ZIPARCHIVE) << "data is abnormal";
        return false;
    }

    return true;
}

bool ZipFile::ReadZStreamFromMMap(const BytePtr &buffer, void *&dataPtr, z_stream &zstream,
                                  uint32_t &remainCompressedSize) const
{
    if (dataPtr == nullptr) {
        LOG(ERROR, ZIPARCHIVE) << "dataPtr is nullptr";
        return false;
    }

    auto *srcDataPtr = static_cast<uint8_t *>(dataPtr);
    if (zstream.avail_in == 0) {
        size_t remainBytes = (remainCompressedSize > UNZIP_BUF_IN_LEN) ? UNZIP_BUF_IN_LEN : remainCompressedSize;
        size_t readBytes = sizeof(Byte) * remainBytes;
        if (memcpy_s(buffer, readBytes, srcDataPtr, readBytes) != EOK) {
            LOG(ERROR, ZIPARCHIVE) << "memcpy failed";
            return false;
        }
        srcDataPtr += readBytes;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        remainCompressedSize -= remainBytes;
        zstream.avail_in = remainBytes;
        zstream.next_in = buffer;
    }
    dataPtr = srcDataPtr;
    return true;
}

std::unique_ptr<FileMapper> ZipFile::CreateFileMapper(const std::string &fileName, FileMapperType type) const
{
    ZipEntry zipEntry;
    if (!GetEntry(fileName, zipEntry)) {
        return nullptr;
    }

    ZipPos offset = 0;
    uint32_t length = 0;
    if (!GetDataOffsetRelative(zipEntry, offset, length)) {
        LOG(ERROR, ZIPARCHIVE) << "GetDataOffsetRelative failed hapPath: " << fileName;
        return nullptr;
    }
    bool compress = zipEntry.compressionMethod > 0;
    if (type == FileMapperType::SAFE_ABC && compress) {
        LOG(WARNING, ZIPARCHIVE) << "Entry is compressed for safe: " << fileName;
    }
    std::unique_ptr<FileMapper> fileMapper = std::make_unique<FileMapper>();
    if (zipFileReader_ == nullptr) {
        LOG(ERROR, ZIPARCHIVE) << "zipFileReader_ is nullptr";
        return nullptr;
    }
    auto result = false;
    if (type == FileMapperType::NORMAL_MEM) {
        result = fileMapper->CreateFileMapper(zipFileReader_, fileName, offset, length, compress);
    } else {
        result = fileMapper->CreateFileMapper(fileName, compress, zipFileReader_->GetFd(), offset, length, type);
        if (result && type == FileMapperType::SAFE_ABC) {
            zipFileReader_->SetClosable(false);
        }
    }

    if (!result) {
        return nullptr;
    }
    return fileMapper;
}

// NOLINTNEXTLINE(modernize-avoid-c-arrays)
bool ZipFile::ExtractToBufByName(const std::string &fileName, std::unique_ptr<uint8_t[]> &dataPtr, size_t &len) const
{
    ZipEntry zipEntry;
    if (!GetEntry(fileName, zipEntry)) {
        if (fileName.length() > MAP_FILE_SUFFIX && fileName.substr(fileName.length() - MAP_FILE_SUFFIX) != ".map") {
            LOG(ERROR, ZIPARCHIVE) << "GetEntry failed hapPath: " << fileName;
        }
        return false;
    }
    uint16_t extraSize = 0;
    if (!CheckCoherencyLocalHeader(zipEntry, extraSize)) {
        LOG(ERROR, ZIPARCHIVE) << "check coherency local header failed";
        return false;
    }
    if (zipFileReader_ == nullptr) {
        LOG(ERROR, ZIPARCHIVE) << "zipFileReader_ is nullptr";
        return false;
    }
    ZipPos offset = GetEntryDataOffset(zipEntry, extraSize);
    uint32_t length = zipEntry.compressedSize;
    auto dataTmp = std::make_unique<uint8_t[]>(length);  // NOLINT(modernize-avoid-c-arrays)
    if (!zipFileReader_->ReadBuffer(dataTmp.get(), offset, length)) {
        LOG(ERROR, ZIPARCHIVE) << "read file failed, len: " << length << ", fileName: " << fileName
                               << ", offset: " << offset;
        dataTmp.reset();
        return false;
    }

    if (zipEntry.compressionMethod > 0) {
        return UnzipWithInflatedFromMMap(zipEntry, extraSize, dataTmp.get(), dataPtr, len);
    }

    len = length;
    dataPtr = std::move(dataTmp);

    return true;
}
}  // namespace ark::extractor
