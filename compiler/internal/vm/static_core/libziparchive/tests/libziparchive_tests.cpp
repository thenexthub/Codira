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

#include "zip_archive.h"
#include "libarkfile/file.h"
#include "libarkbase/os/file.h"
#include "libarkbase/os/mem.h"

#include "assembly-emitter.h"
#include "assembly-parser.h"

#include <cstdio>
#include <cstdint>
#include <vector>
#include <sstream>
#include <gtest/gtest.h>
#include <cstddef>
#include <memory>
#include <securec.h>

#include <climits>
#include <cstdlib>
#include <fcntl.h>
#include <sys/stat.h>

namespace ark::test {

#define GTEST_COUT std::cerr << "[          ] [ INFO ]"

constexpr char const *FILLER_TEXT =
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Cras feugiat et odio ac sollicitudin. Maecenas "
    "lobortis ultrices eros sed pharetra. Phasellus in tortor rhoncus, aliquam augue ac, gravida elit. Sed "
    "molestie dolor a vulputate tincidunt. Proin a tellus quam. Suspendisse id feugiat elit, non ornare lacus. "
    "Mauris arcu ex, pretium quis dolor ut, porta iaculis eros. Vestibulum sagittis placerat diam, vitae efficitur "
    "turpis ultrices sit amet. Etiam elementum bibendum congue. In sit amet dolor ultricies, suscipit arcu ac, "
    "molestie urna. Mauris ultrices volutpat massa quis ultrices. Suspendisse rutrum lectus sit amet metus "
    "laoreet, non porta sapien venenatis. Fusce ut massa et purus elementum lacinia. Sed tempus bibendum pretium.";

constexpr size_t MAX_BUFFER_SIZE = 2048;
constexpr size_t MAX_DIR_SIZE = 64;

static void GenerateZipfile(const char *data, const char *archivename, int n, std::vector<uint8_t> &pfData,
                            int level = Z_BEST_COMPRESSION)
{
    // Delete the test archive, so it doesn't keep growing as we run this test
    remove(archivename);

    // Create and append a directory entry for testing
    std::vector<uint8_t> emptyVector;
    int ret = CreateOrAddFileIntoZip(archivename, "directory/", &emptyVector, APPEND_STATUS_CREATE, level);
    if (ret != 0) {
        ASSERT_EQ(1, 0) << "CreateOrAddFileIntoZip for directory failed!";
        return;
    }

    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    char archiveFilename[MAX_DIR_SIZE];
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    char buf[MAX_BUFFER_SIZE];

    // Append a bunch of text files to the test archive
    for (int i = (n - 1); i >= 0; --i) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        (void)sprintf_s(archiveFilename, sizeof(archiveFilename), "%u.txt", i);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        (void)sprintf_s(buf, sizeof(buf), "%u %s %u", (n - 1) - i, data, i);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        std::vector<uint8_t> fillerData(buf, buf + strlen(buf) + 1);
        ret = CreateOrAddFileIntoZip(archivename, archiveFilename, &fillerData, APPEND_STATUS_ADDINZIP, level);
        if (ret != 0) {
            ASSERT_EQ(1, 0) << "CreateOrAddFileIntoZip for " << i << ".txt failed!";
            return;
        }
    }

    // Append a file into directory entry for testing
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    (void)sprintf_s(buf, sizeof(buf), "%u %s %u", n, data, n);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    std::vector<uint8_t> fillerData(buf, buf + strlen(buf) + 1);
    ret = CreateOrAddFileIntoZip(archivename, "directory/indirectory.txt", &fillerData, APPEND_STATUS_ADDINZIP, level);
    if (ret != 0) {
        ASSERT_EQ(1, 0) << "CreateOrAddFileIntoZip for directory/indirectory.txt failed!";
        return;
    }

    // Add a pandafile into zip for testing
    ret = CreateOrAddFileIntoZip(archivename, "classes.abc", &pfData, APPEND_STATUS_ADDINZIP, level);
    if (ret != 0) {
        ASSERT_EQ(1, 0) << "CreateOrAddFileIntoZip for classes.abc failed!";
        return;
    }
}

static void UnzipFileCheckDirectory(const char *archivename, char *filename, int level = Z_BEST_COMPRESSION)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    (void)sprintf_s(filename, MAX_DIR_SIZE, "directory/");

    ZipArchiveHandle zipfile = nullptr;
    FILE *myfile = fopen(archivename, "rbe");

    if (OpenArchiveFile(zipfile, myfile) != 0) {
        fclose(myfile);
        ASSERT_EQ(1, 0) << "OpenArchiveFILE error.";
        return;
    }
    if (LocateFile(zipfile, filename) != 0) {
        CloseArchiveFile(zipfile);
        fclose(myfile);
        ASSERT_EQ(1, 0) << "LocateFile error.";
        return;
    }
    EntryFileStat entry = EntryFileStat();
    if (GetCurrentFileInfo(zipfile, &entry) != 0) {
        CloseArchiveFile(zipfile);
        fclose(myfile);
        ASSERT_EQ(1, 0) << "GetCurrentFileInfo test error.";
        return;
    }
    if (OpenCurrentFile(zipfile) != 0) {
        CloseCurrentFile(zipfile);
        CloseArchiveFile(zipfile);
        fclose(myfile);
        ASSERT_EQ(1, 0) << "OpenCurrentFile test error.";
        return;
    }

    GetCurrentFileOffset(zipfile, &entry);

    uint32_t uncompressedLength = entry.GetUncompressedSize();

    ASSERT_GT(entry.GetOffset(), 0);
    if (level == Z_NO_COMPRESSION) {
        ASSERT_FALSE(entry.IsCompressed());
    } else {
        ASSERT_TRUE(entry.IsCompressed());
    }

    GTEST_COUT << "Filename: " << filename << ", Uncompressed size: " << uncompressedLength
               << "Compressed size: " << entry.GetCompressedSize() << "Compressed(): " << entry.IsCompressed()
               << "entry offset: " << entry.GetOffset() << "\n";

    CloseCurrentFile(zipfile);
    CloseArchiveFile(zipfile);
    fclose(myfile);
}

static void CloseArchiveAndCurrentFile(ZipArchiveHandle zipfile, FILE *myfile)
{
    CloseCurrentFile(zipfile);
    CloseArchiveFile(zipfile);
    (void)fclose(myfile);
}

static std::string OpenArchiveAndCurrentFile(ZipArchiveHandle &zipfile, char *filename, FILE *myfile,
                                             EntryFileStat *entry)
{
    if (OpenArchiveFile(zipfile, myfile) != 0) {
        fclose(myfile);
        return "OpenArchiveFILE error.";
    }
    if (LocateFile(zipfile, filename) != 0) {
        CloseArchiveFile(zipfile);
        fclose(myfile);
        return "LocateFile error.";
    }

    if (GetCurrentFileInfo(zipfile, entry) != 0) {
        CloseArchiveFile(zipfile);
        fclose(myfile);
        return "GetCurrentFileInfo test error.";
    }
    if (OpenCurrentFile(zipfile) != 0) {
        CloseArchiveAndCurrentFile(zipfile, myfile);
        return "OpenCurrentFile test error.";
    }

    return {};
}

static std::string ExtractFile(ZipArchiveHandle &zipfile, FILE *myfile, uint32_t uncompressedLength, char *buf)
{
    // Extract to mem buffer accroding to entry info.
    uint32_t pageSize = os::mem::GetPageSize();
    ASSERT(pageSize != 0);
    uint32_t minPages = uncompressedLength / pageSize;
    uint32_t sizeToMmap = uncompressedLength % pageSize == 0 ? minPages * pageSize : (minPages + 1) * pageSize;
    // we will use mem in memcmp, so donnot poision it!
    void *mem = os::mem::MapRWAnonymousRaw(sizeToMmap, false);
    if (mem == nullptr) {
        CloseArchiveAndCurrentFile(zipfile, myfile);
        return "Can't mmap anonymous!";
    }

    if (ExtractToMemory(zipfile, reinterpret_cast<uint8_t *>(mem), sizeToMmap) != 0) {
        os::mem::UnmapRaw(mem, sizeToMmap);
        CloseArchiveAndCurrentFile(zipfile, myfile);
        return "Can't extract!";
    }

    // Make sure the extraction really succeeded.
    size_t dlen = strlen(buf);
    if (uncompressedLength != (dlen + 1)) {
        os::mem::UnmapRaw(mem, sizeToMmap);
        CloseArchiveAndCurrentFile(zipfile, myfile);
        std::ostringstream out;
        out << "ExtractToMemory() failed!, uncompressed_length is " << (uncompressedLength - 1)
            << ", original strlen is " << dlen;
        return out.str();
    }

    if (memcmp(mem, buf, dlen) != 0) {
        os::mem::UnmapRaw(mem, sizeToMmap);
        CloseArchiveAndCurrentFile(zipfile, myfile);
        return "ExtractToMemory() memcmp failed!";
    }

    os::mem::UnmapRaw(mem, sizeToMmap);

    return {};
}

static std::string ExtractPandaFile(ZipArchiveHandle &zipfile, FILE *myfile, uint32_t uncompressedLength,
                                    std::vector<uint8_t> &pfData)
{
    // Extract to mem buffer accroding to entry info.
    uint32_t pageSize = os::mem::GetPageSize();
    ASSERT(pageSize != 0);
    uint32_t minPages = uncompressedLength / pageSize;
    uint32_t sizeToMmap = uncompressedLength % pageSize == 0 ? minPages * pageSize : (minPages + 1) * pageSize;
    // we will use mem in memcmp, so donnot poision it!
    void *mem = os::mem::MapRWAnonymousRaw(sizeToMmap, false);
    if (mem == nullptr) {
        CloseArchiveAndCurrentFile(zipfile, myfile);
        return "Can't mmap anonymous!";
    }

    int ret = ExtractToMemory(zipfile, reinterpret_cast<uint8_t *>(mem), sizeToMmap);
    if (ret != 0) {
        os::mem::UnmapRaw(mem, sizeToMmap);
        CloseArchiveAndCurrentFile(zipfile, myfile);
        return "Can't extract!";
    }

    // Make sure the extraction really succeeded.
    if (uncompressedLength != pfData.size()) {
        os::mem::UnmapRaw(mem, sizeToMmap);
        CloseArchiveAndCurrentFile(zipfile, myfile);
        std::ostringstream out;
        out << "ExtractToMemory() failed!, uncompressed_length is " << uncompressedLength
            << ", original pf_data size is " << pfData.size() << "\n";
        return out.str();
    }

    if (memcmp(mem, pfData.data(), pfData.size()) != 0) {
        os::mem::UnmapRaw(mem, sizeToMmap);
        CloseArchiveAndCurrentFile(zipfile, myfile);
        return "ExtractToMemory() memcmp failed!";
    }

    os::mem::UnmapRaw(mem, sizeToMmap);

    return {};
}

static void UnzipFileCheckTxt(const char *archivename, char *filename, const char *data, int n,
                              int level = Z_BEST_COMPRESSION)
{
    for (int i = 0; i < n; i++) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        (void)sprintf_s(filename, MAX_DIR_SIZE, "%u.txt", i);

        // NOLINTNEXTLINE(modernize-avoid-c-arrays)
        char buf[MAX_BUFFER_SIZE];
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        (void)sprintf_s(buf, sizeof(buf), "%u %s %u", (n - 1) - i, data, i);

        ZipArchiveHandle zipfile = nullptr;
        FILE *myfile = fopen(archivename, "rbe");
        EntryFileStat entry = EntryFileStat();

        std::string errorMessage = OpenArchiveAndCurrentFile(zipfile, filename, myfile, &entry);
        if (!errorMessage.empty()) {
            ASSERT_EQ(1, 0) << errorMessage.c_str();
        }

        GetCurrentFileOffset(zipfile, &entry);

        uint32_t uncompressedLength = entry.GetUncompressedSize();
        if (uncompressedLength == 0) {
            CloseArchiveAndCurrentFile(zipfile, myfile);
            ASSERT_EQ(1, 0) << "Entry file has zero length! Readed bad data!";
        }
        ASSERT_GT(entry.GetOffset(), 0);
        ASSERT_EQ(uncompressedLength, strlen(buf) + 1);
        if (level == Z_NO_COMPRESSION) {
            ASSERT_EQ(uncompressedLength, entry.GetCompressedSize());
            ASSERT_FALSE(entry.IsCompressed());
        } else {
            ASSERT_GE(uncompressedLength, entry.GetCompressedSize());
            ASSERT_TRUE(entry.IsCompressed());
        }

        GTEST_COUT << "Filename: " << filename << ", Uncompressed size: " << uncompressedLength
                   << "Compressed size: " << entry.GetCompressedSize() << "Compressed(): " << entry.IsCompressed()
                   << "entry offset: " << entry.GetOffset() << "\n";

        {
            errorMessage = ExtractFile(zipfile, myfile, uncompressedLength, buf);
            if (!errorMessage.empty()) {
                ASSERT_EQ(1, 0) << errorMessage.c_str();
            }

            GTEST_COUT << "Successfully extracted file " << filename << " from " << archivename << ", size is "
                       << uncompressedLength << "\n";
        }

        CloseArchiveAndCurrentFile(zipfile, myfile);
    }
}

static void UnzipFileCheckPandaFile(const char *archivename, char *filename, std::vector<uint8_t> &pfData,
                                    int level = Z_BEST_COMPRESSION)
{
    {
        ZipArchiveHandle zipfile = nullptr;
        FILE *myfile = fopen(archivename, "rbe");
        EntryFileStat entry = EntryFileStat();

        std::string errorMessage = OpenArchiveAndCurrentFile(zipfile, filename, myfile, &entry);
        if (!errorMessage.empty()) {
            ASSERT_EQ(1, 0) << errorMessage.c_str();
        }

        GetCurrentFileOffset(zipfile, &entry);

        uint32_t uncompressedLength = entry.GetUncompressedSize();
        if (uncompressedLength == 0) {
            CloseCurrentFile(zipfile);
            CloseArchiveFile(zipfile);
            fclose(myfile);
            ASSERT_EQ(1, 0) << "Entry file has zero length! Readed bad data!";
            return;
        }
        ASSERT_GT(entry.GetOffset(), 0);
        ASSERT_EQ(uncompressedLength, pfData.size());
        if (level == Z_NO_COMPRESSION) {
            ASSERT_EQ(uncompressedLength, entry.GetCompressedSize());
            ASSERT_FALSE(entry.IsCompressed());
        } else {
            ASSERT_GE(uncompressedLength, entry.GetCompressedSize());
            ASSERT_TRUE(entry.IsCompressed());
        }

        GTEST_COUT << "Filename: " << filename << ", Uncompressed size: " << uncompressedLength
                   << "Compressed size: " << entry.GetCompressedSize() << "Compressed(): " << entry.IsCompressed()
                   << "entry offset: " << entry.GetOffset() << "\n";

        {
            errorMessage = ExtractPandaFile(zipfile, myfile, uncompressedLength, pfData);
            if (!errorMessage.empty()) {
                ASSERT_EQ(1, 0) << errorMessage.c_str();
            }
            GTEST_COUT << "Successfully extracted file " << filename << " from " << archivename << ", size is "
                       << uncompressedLength << "\n";
        }
        CloseArchiveAndCurrentFile(zipfile, myfile);
    }
}

static void UnzipFileCheckInDirectory(const char *archivename, char *filename, const char *data, int n,
                                      int level = Z_BEST_COMPRESSION)
{
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        (void)sprintf_s(filename, MAX_DIR_SIZE, "directory/indirectory.txt");

        // NOLINTNEXTLINE(modernize-avoid-c-arrays)
        char buf[MAX_BUFFER_SIZE];
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        (void)sprintf_s(buf, sizeof(buf), "%u %s %u", n, data, n);

        // Unzip Check
        ZipArchiveHandle zipfile = nullptr;
        FILE *myfile = fopen(archivename, "rbe");
        EntryFileStat entry = EntryFileStat();

        std::string errorMessage = OpenArchiveAndCurrentFile(zipfile, filename, myfile, &entry);
        if (!errorMessage.empty()) {
            ASSERT_EQ(1, 0) << errorMessage.c_str();
        }

        GetCurrentFileOffset(zipfile, &entry);

        uint32_t uncompressedLength = entry.GetUncompressedSize();
        if (uncompressedLength == 0) {
            CloseCurrentFile(zipfile);
            CloseArchiveFile(zipfile);
            fclose(myfile);
            ASSERT_EQ(1, 0) << "Entry file has zero length! Readed bad data!";
            return;
        }
        ASSERT_GT(entry.GetOffset(), 0);
        ASSERT_EQ(uncompressedLength, strlen(buf) + 1);
        if (level == Z_NO_COMPRESSION) {
            ASSERT_EQ(uncompressedLength, entry.GetCompressedSize());
            ASSERT_FALSE(entry.IsCompressed());
        } else {
            ASSERT_GE(uncompressedLength, entry.GetCompressedSize());
            ASSERT_TRUE(entry.IsCompressed());
        }
        GTEST_COUT << "Filename: " << filename << ", Uncompressed size: " << uncompressedLength
                   << "Compressed size: " << entry.GetCompressedSize() << "Compressed(): " << entry.IsCompressed()
                   << "entry offset: " << entry.GetOffset() << "\n";

        {
            errorMessage = ExtractFile(zipfile, myfile, uncompressedLength, buf);
            if (!errorMessage.empty()) {
                ASSERT_EQ(1, 0) << errorMessage.c_str();
            }

            GTEST_COUT << "Successfully extracted file " << filename << " from " << archivename << ", size is "
                       << uncompressedLength << "\n";
        }

        CloseArchiveAndCurrentFile(zipfile, myfile);
    }
}

TEST(LIBZIPARCHIVE, ZipFile)
{
    /*
     * creating an empty pandafile
     */
    std::vector<uint8_t> pfData {};
    {
        pandasm::Parser p;

        auto source = R"()";

        std::string srcFilename = "src.pa";
        auto res = p.Parse(source, srcFilename);
        ASSERT_EQ(p.ShowError().err, pandasm::Error::ErrorType::ERR_NONE);

        auto pf = pandasm::AsmEmitter::Emit(res.Value());
        ASSERT_NE(pf, nullptr);

        const auto headerPtr = reinterpret_cast<const uint8_t *>(pf->GetHeader());
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        pfData.assign(headerPtr, headerPtr + sizeof(panda_file::File::Header));
    }

    static const char *archivename = "__LIBZIPARCHIVE__ZipFile__.zip";
    const int n = 3;

    GenerateZipfile(FILLER_TEXT, archivename, n, pfData);

    // Quick Check
    ZipArchiveHandle zipfile = nullptr;
    if (OpenArchive(zipfile, archivename) != 0) {
        ASSERT_EQ(1, 0) << "OpenArchive error.";
        return;
    }

    GlobalStat gi = GlobalStat();
    if (GetGlobalFileInfo(zipfile, &gi) != 0) {
        ASSERT_EQ(1, 0) << "GetGlobalFileInfo error.";
        return;
    }
    for (int i = 0; i < (int)gi.GetNumberOfEntry(); ++i) {
        EntryFileStat fileStat {};
        if (GetCurrentFileInfo(zipfile, &fileStat) != 0) {
            CloseArchive(zipfile);
            ASSERT_EQ(1, 0) << "GetCurrentFileInfo error. Current index i = " << i;
            return;
        }
        GTEST_COUT << "Index:  " << i << ", Uncompressed size: " << fileStat.GetUncompressedSize()
                   << "Compressed size: " << fileStat.GetCompressedSize() << "Compressed(): " << fileStat.IsCompressed()
                   << "entry offset: " << fileStat.GetOffset() << "\n";
        if ((i + 1) < (int)gi.GetNumberOfEntry()) {
            if (GoToNextFile(zipfile) != 0) {
                CloseArchive(zipfile);
                ASSERT_EQ(1, 0) << "GoToNextFile error. Current index i = " << i;
                return;
            }
        }
    }

    CloseArchive(zipfile);
    remove(archivename);
    GTEST_COUT << "Success.\n";
}

TEST(LIBZIPARCHIVE, UnZipFile)
{
    /*
     * creating an empty pandafile
     */
    std::vector<uint8_t> pfData {};
    {
        pandasm::Parser p;

        auto source = R"()";

        std::string srcFilename = "src.pa";
        auto res = p.Parse(source, srcFilename);
        ASSERT_EQ(p.ShowError().err, pandasm::Error::ErrorType::ERR_NONE);

        auto pf = pandasm::AsmEmitter::Emit(res.Value());
        ASSERT_NE(pf, nullptr);

        const auto headerPtr = reinterpret_cast<const uint8_t *>(pf->GetHeader());
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        pfData.assign(headerPtr, headerPtr + sizeof(panda_file::File::Header));
    }

    // The zip filename
    static const char *archivename = "__LIBZIPARCHIVE__UnZipFile__.zip";
    const int n = 3;
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    char filename[MAX_DIR_SIZE];

    GenerateZipfile(FILLER_TEXT, archivename, n, pfData);

    UnzipFileCheckDirectory(archivename, filename);

    UnzipFileCheckTxt(archivename, filename, FILLER_TEXT, n);

    UnzipFileCheckInDirectory(archivename, filename, FILLER_TEXT, n);

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    sprintf_s(filename, MAX_DIR_SIZE, "classes.abc");
    UnzipFileCheckPandaFile(archivename, filename, pfData);

    remove(archivename);
    GTEST_COUT << "Success.\n";
}

TEST(LIBZIPARCHIVE, UnZipUncompressedFile)
{
    /*
     * creating an empty pandafile
     */
    std::vector<uint8_t> pfData {};
    {
        pandasm::Parser p;

        auto source = R"()";

        std::string srcFilename = "src.pa";
        auto res = p.Parse(source, srcFilename);
        ASSERT_EQ(p.ShowError().err, pandasm::Error::ErrorType::ERR_NONE);

        auto pf = pandasm::AsmEmitter::Emit(res.Value());
        ASSERT_NE(pf, nullptr);

        const auto headerPtr = reinterpret_cast<const uint8_t *>(pf->GetHeader());
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        pfData.assign(headerPtr, headerPtr + sizeof(panda_file::File::Header));
    }

    // The zip filename
    static const char *archivename = "__LIBZIPARCHIVE__UnZipUncompressedFile__.zip";
    const int n = 3;

    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    char filename[MAX_DIR_SIZE];

    GenerateZipfile(FILLER_TEXT, archivename, n, pfData, Z_NO_COMPRESSION);

    UnzipFileCheckDirectory(archivename, filename, Z_NO_COMPRESSION);

    UnzipFileCheckTxt(archivename, filename, FILLER_TEXT, n, Z_NO_COMPRESSION);

    UnzipFileCheckInDirectory(archivename, filename, FILLER_TEXT, n, Z_NO_COMPRESSION);

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    (void)sprintf_s(filename, MAX_DIR_SIZE, "classes.abc");
    UnzipFileCheckPandaFile(archivename, filename, pfData, Z_NO_COMPRESSION);

    remove(archivename);
    GTEST_COUT << "Success.\n";
}

TEST(LIBZIPARCHIVE, UnZipUncompressedPandaFile)
{
    /*
     * creating an empty pandafile
     */
    std::vector<uint8_t> pfData {};
    {
        pandasm::Parser p;

        auto source = R"()";

        std::string srcFilename = "src.pa";
        auto res = p.Parse(source, srcFilename);
        ASSERT_EQ(p.ShowError().err, pandasm::Error::ErrorType::ERR_NONE);

        auto pf = pandasm::AsmEmitter::Emit(res.Value());
        ASSERT_NE(pf, nullptr);

        const auto headerPtr = reinterpret_cast<const uint8_t *>(pf->GetHeader());
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        pfData.assign(headerPtr, headerPtr + sizeof(panda_file::File::Header));
    }

    // The zip filename
    static const char *archivename = "__LIBZIPARCHIVE__UnZipUncompressedPandaFile__.zip";
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    char filename[MAX_DIR_SIZE];

    // Delete the test archive, so it doesn't keep growing as we run this test
    remove(archivename);

    // Add pandafile into zip for testing
    int ret = CreateOrAddFileIntoZip(archivename, "class.abc", &pfData, APPEND_STATUS_CREATE, Z_NO_COMPRESSION);
    if (ret != 0) {
        ASSERT_EQ(1, 0) << "CreateOrAddFileIntoZip failed!";
        return;
    }
    ret = CreateOrAddFileIntoZip(archivename, "classes.abc", &pfData, APPEND_STATUS_ADDINZIP, Z_NO_COMPRESSION);
    if (ret != 0) {
        ASSERT_EQ(1, 0) << "CreateOrAddFileIntoZip failed!";
        return;
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    sprintf_s(filename, MAX_DIR_SIZE, "class.abc");
    UnzipFileCheckPandaFile(archivename, filename, pfData, Z_NO_COMPRESSION);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    sprintf_s(filename, MAX_DIR_SIZE, "classes.abc");
    UnzipFileCheckPandaFile(archivename, filename, pfData, Z_NO_COMPRESSION);

    remove(archivename);
    GTEST_COUT << "Success.\n";
}

TEST(LIBZIPARCHIVE, IllegalPathTest)
{
    static const char *archivename = "__LIBZIPARCHIVE__ILLEGALPATHTEST__.zip";
    int ret =
        CreateOrAddFileIntoZip(archivename, "illegal_path.abc", nullptr, APPEND_STATUS_ADDINZIP, Z_NO_COMPRESSION);
    ASSERT_EQ(ret, ZIPARCHIVE_ERR);
    ZipArchiveHandle zipfile = nullptr;
    ASSERT_EQ(OpenArchive(zipfile, archivename), ZIPARCHIVE_ERR);
    GlobalStat gi = GlobalStat();
    ASSERT_EQ(GetGlobalFileInfo(zipfile, &gi), ZIPARCHIVE_ERR);
    ASSERT_EQ(GoToNextFile(zipfile), ZIPARCHIVE_ERR);
    ASSERT_EQ(LocateFile(zipfile, "illegal_path.abc"), ZIPARCHIVE_ERR);
    EntryFileStat entry = EntryFileStat();
    ASSERT_EQ(GetCurrentFileInfo(zipfile, &entry), ZIPARCHIVE_ERR);
    ASSERT_EQ(CloseArchive(zipfile), ZIPARCHIVE_ERR);

    ASSERT_EQ(OpenArchiveFile(zipfile, nullptr), ZIPARCHIVE_ERR);
    ASSERT_EQ(OpenCurrentFile(zipfile), ZIPARCHIVE_ERR);
    ASSERT_EQ(CloseCurrentFile(zipfile), ZIPARCHIVE_ERR);
    ASSERT_EQ(CloseArchiveFile(zipfile), ZIPARCHIVE_ERR);
}
}  // namespace ark::test
