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

#ifndef LIBZIPARCHIVE_EXTRACTORTOOL_ZIP_FILE_H
#define LIBZIPARCHIVE_EXTRACTORTOOL_ZIP_FILE_H

#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "unzip.h"

#include "file_mapper.h"
#include "libarkbase/os/mutex.h"
#include "libarkbase/utils/logger.h"

namespace ark::extractor {
class ZipFileReader;
struct CentralDirEntry;
struct ZipEntry;
using ZipPos = ZPOS64_T;
using ZipEntryMap = std::unordered_map<std::string, ZipEntry>;
using BytePtr = Byte *;

// Local file header: descript in APPNOTE-6.3.4
//    local file header signature     4 bytes  (0x04034b50)
//    version needed to extract       2 bytes
//    general purpose bit flag        2 bytes
//    compression method              2 bytes  10
//    last mod file time              2 bytes
//    last mod file date              2 bytes
//    crc-32                          4 bytes
//    compressed size                 4 bytes  22
//    uncompressed size               4 bytes
//    file name length                2 bytes
//    extra field length              2 bytes  30
struct __attribute__((packed)) LocalHeader {
    uint32_t signature = 0;
    uint16_t versionNeeded = 0;
    uint16_t flags = 0;
    uint16_t compressionMethod = 0;
    uint16_t modifiedTime = 0;
    uint16_t modifiedDate = 0;
    uint32_t crc = 0;
    uint32_t compressedSize = 0;
    uint32_t uncompressedSize = 0;
    uint16_t nameSize = 0;
    uint16_t extraSize = 0;
};

// central file header
//    Central File header:
//    central file header signature   4 bytes  (0x02014b50)
//    version made by                 2 bytes
//    version needed to extract       2 bytes
//    general purpose bit flag        2 bytes  10
//    compression method              2 bytes
//    last mod file time              2 bytes
//    last mod file date              2 bytes
//    crc-32                          4 bytes  20
//    compressed size                 4 bytes
//    uncompressed size               4 bytes
//    file name length                2 bytes  30
//    extra field length              2 bytes
//    file comment length             2 bytes
//    disk number start               2 bytes
//    internal file attributes        2 bytes
//    external file attributes        4 bytes
//    relative offset of local header 4 bytes 46byte
struct __attribute__((packed)) CentralDirEntry {
    uint32_t signature = 0;
    uint16_t versionMade = 0;
    uint16_t versionNeeded = 0;
    uint16_t flags = 0;  // general purpose bit flag
    uint16_t compressionMethod = 0;
    uint16_t modifiedTime = 0;
    uint16_t modifiedDate = 0;
    uint32_t crc = 0;
    uint32_t compressedSize = 0;
    uint32_t uncompressedSize = 0;
    uint16_t nameSize = 0;
    uint16_t extraSize = 0;
    uint16_t commentSize = 0;
    uint16_t diskNumStart = 0;
    uint16_t internalAttr = 0;
    uint32_t externalAttr = 0;
    uint32_t localHeaderOffset = 0;
};

// end of central directory packed structure
//    end of central dir signature    4 bytes  (0x06054b50)
//    number of this disk             2 bytes
//    number of the disk with the
//    start of the central directory  2 bytes
//    total number of entries in the
//    central directory on this disk  2 bytes
//    total number of entries in
//    the central directory           2 bytes
//    size of the central directory   4 bytes
//    offset of start of central
//    directory with respect to
//    the starting disk number        4 bytes
//    .ZIP file comment length        2 bytes
struct __attribute__((packed)) EndDir {
    uint32_t signature = 0;
    uint16_t numDisk = 0;
    uint16_t startDiskOfCentralDir = 0;
    uint16_t totalEntriesInThisDisk = 0;
    uint16_t totalEntries = 0;
    uint32_t sizeOfCentralDir = 0;
    uint32_t offset = 0;
    uint16_t commentLen = 0;
};

// Data descriptor:
//    data descriptor signature       4 bytes  (0x06054b50)
//    crc-32                          4 bytes
//    compressed size                 4 bytes
//    uncompressed size               4 bytes
// This descriptor MUST exist if bit 3 of the general purpose bit flag is set (see below).
// It is byte aligned and immediately follows the last byte of compressed data.
struct __attribute__((packed)) DataDesc {
    uint32_t signature = 0;
    uint32_t crc = 0;
    uint32_t compressedSize = 0;
    uint32_t uncompressedSize = 0;
};

struct ZipEntry {  // NOLINT(cppcoreguidelines-special-member-functions)
    ZipEntry() = default;
    explicit ZipEntry(const CentralDirEntry &centralEntry);
    ~ZipEntry() = default;

    uint16_t compressionMethod = 0;  // NOLINT(misc-non-private-member-variables-in-classes)
    uint32_t uncompressedSize = 0;   // NOLINT(misc-non-private-member-variables-in-classes)
    uint32_t compressedSize = 0;     // NOLINT(misc-non-private-member-variables-in-classes)
    uint32_t localHeaderOffset = 0;  // NOLINT(misc-non-private-member-variables-in-classes)
    uint32_t crc = 0;                // NOLINT(misc-non-private-member-variables-in-classes)
    uint16_t flags = 0;              // NOLINT(misc-non-private-member-variables-in-classes)
    uint16_t modifiedTime = 0;       // NOLINT(misc-non-private-member-variables-in-classes)
    uint16_t modifiedDate = 0;       // NOLINT(misc-non-private-member-variables-in-classes)
    std::string fileName;            // NOLINT(misc-non-private-member-variables-in-classes)
};

struct DirTreeNode {
    bool isDir = false;
    std::unordered_map<std::string, std::shared_ptr<DirTreeNode>> children;
};

enum class CacheMode : uint32_t {
    CACHE_NONE = 0,
    CACHE_CASE,  // This mode depends on file amount in hap.
    CACHE_ALL
};

// zip file extract class for bundle format.
class ZipFile {  // NOLINT(cppcoreguidelines-special-member-functions)
public:
    explicit ZipFile(const std::string &pathName);
    ~ZipFile();
    /**
     * @brief Open zip file.
     * @return Returns true if the zip file is successfully opened; returns false otherwise.
     */
    bool Open();
    void Close();
    /**
     * @brief Get all entries in the zip file.
     * @param start Indicates the zip content location start position.
     * @param length Indicates the zip content length.
     * @return Returns the ZipEntryMap object cotain all entries.
     */
    const ZipEntryMap &GetAllEntries() const;
    /**
     * @brief Has entry by name.
     * @param entryName Indicates the entry name.
     * @return Returns true if the ZipEntry is successfully finded; returns false otherwise.
     */
    bool HasEntry(const std::string &entryName) const;

    bool IsDirExist(const std::string &dir);
    void GetAllFileList(const std::string &srcPath, std::vector<std::string> &assetList);
    void GetChildNames(const std::string &srcPath, std::set<std::string> &fileSet);

    /**
     * @brief Get entry by name.
     * @param entryName Indicates the entry name.
     * @param resultEntry Indicates the obtained ZipEntry object.
     * @return Returns true if the ZipEntry is successfully finded; returns false otherwise.
     */
    bool GetEntry(const std::string &entryName, ZipEntry &resultEntry) const;
    bool GetDataOffsetRelative(const ZipEntry &zipEntry, ZipPos &offset, uint32_t &length) const;
    bool ExtractFileFromMMap(const std::string &file, void *mmapDataPtr,
                             std::unique_ptr<uint8_t[]> &dataPtr,  // NOLINT(modernize-avoid-c-arrays)
                             size_t &len) const;

    std::unique_ptr<FileMapper> CreateFileMapper(const std::string &fileName, FileMapperType type) const;
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    bool ExtractToBufByName(const std::string &fileName, std::unique_ptr<uint8_t[]> &dataPtr, size_t &len) const;
    void SetCacheMode(CacheMode cacheMode);
    bool UseDirCache() const;

private:
    /**
     * @brief Check the EndDir object.
     * @param endDir Indicates the EndDir object to check.
     * @return Returns true if  successfully checked; returns false otherwise.
     */
    bool CheckEndDir(const EndDir &endDir) const;
    /**
     * @brief Parse the EndDir.
     * @return Returns true if  successfully Parsed; returns false otherwise.
     */
    bool ParseEndDirectory();
    /**
     * @brief Parse one entry.
     * @return Returns true if successfully parsed; returns false otherwise.
     */
    bool ParseOneEntry(uint8_t *&entryPtr);
    /**
     * @brief Parse all Entries.
     * @return Returns true if successfully parsed; returns false otherwise.
     */
    bool ParseAllEntries();
    /**
     * @brief Get LocalHeader object size.
     * @param nameSize Indicates the nameSize.
     * @param extraSize Indicates the extraSize.
     * @return Returns size of LocalHeader.
     */
    size_t GetLocalHeaderSize(const uint16_t nameSize = 0, const uint16_t extraSize = 0) const;
    /**
     * @brief Get entry data offset.
     * @param zipEntry Indicates the ZipEntry object.
     * @param extraSize Indicates the extraSize.
     * @return Returns position.
     */
    ZipPos GetEntryDataOffset(const ZipEntry &zipEntry, const uint16_t extraSize) const;
    /**
     * @brief Check data description.
     * @param zipEntry Indicates the ZipEntry object.
     * @param localHeader Indicates the localHeader object.
     * @return Returns true if successfully checked; returns false otherwise.
     */
    bool CheckDataDesc(const ZipEntry &zipEntry, const LocalHeader &localHeader) const;
    /**
     * @brief Check coherency LocalHeader object.
     * @param zipEntry Indicates the ZipEntry object.
     * @param extraSize Indicates the obtained size.
     * @return Returns true if successfully checked; returns false otherwise.
     */
    bool CheckCoherencyLocalHeader(const ZipEntry &zipEntry, uint16_t &extraSize) const;
    /**
     * @brief Get Entry start.
     * @param zipEntry Indicates the ZipEntry object.
     * @param extraSize Indicates the extra size.
     * @return Returns true if successfully Seeked; returns false otherwise.
     */
    size_t GetEntryStart(const ZipEntry &zipEntry, const uint16_t extraSize) const;
    /**
     * @brief Init zlib stream.
     * @param zstream Indicates the obtained z_stream object.
     * @return Returns true if successfully init; returns false otherwise.
     */
    bool InitZStream(z_stream &zstream) const;
    bool UnzipWithInflatedFromMMap(const ZipEntry &zipEntry, const uint16_t extraSize, void *mmapDataPtr,
                                   // NOLINTNEXTLINE(modernize-avoid-c-arrays)
                                   std::unique_ptr<uint8_t[]> &dataPtr, size_t &len) const;
    bool CopyInflateOut(z_stream &zstream, size_t inflateLen, uint8_t **dstDataPtr, BytePtr bufOut,
                        uint8_t &errorTimes) const;
    bool ReadZStreamFromMMap(const BytePtr &buffer, void *&dataPtr, z_stream &zstream,
                             uint32_t &remainCompressedSize) const;

    std::shared_ptr<DirTreeNode> GetDirRoot();
    std::shared_ptr<DirTreeNode> MakeDirTree() const;

    bool IsDirExistCache(const std::string &dir);
    void GetAllFileListCache(const std::string &srcPath, std::vector<std::string> &assetList);
    void GetChildNamesCache(const std::string &srcPath, std::set<std::string> &fileSet);

    bool IsDirExistNormal(const std::string &dir);
    void GetAllFileListNormal(const std::string &srcPath, std::vector<std::string> &assetList);
    void GetChildNamesNormal(const std::string &srcPath, std::set<std::string> &fileSet);

private:
    std::string pathName_;
    std::shared_ptr<ZipFileReader> zipFileReader_;
    EndDir endDir_;
    ZipEntryMap entriesMap_;
    os::memory::Mutex dirRootMutex_;
    std::shared_ptr<DirTreeNode> dirRoot_;
    // offset of central directory relative to zip file.
    ZipPos centralDirPos_ = 0;
    // this zip content start offset relative to zip file.
    ZipPos fileStartPos_ = 0;
    // this zip content length in the zip file.
    ZipPos fileLength_ = 0;
    bool isOpen_ = false;
    CacheMode cacheMode_ = CacheMode::CACHE_CASE;
};
}  // namespace ark::extractor
#endif
