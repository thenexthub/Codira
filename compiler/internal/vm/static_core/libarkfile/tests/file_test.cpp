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

#include "libarkfile/file-inl.h"
#include "libarkfile/file_items.h"
#include "libarkfile/file_item_container.h"
#include "libarkfile/file_reader.h"
#include "libarkbase/utils/string_helpers.h"
#include "zip_archive.h"
#include "libarkfile/file.h"
#include "libarkfile/external/panda_file_support.cpp"

#include "assembly-emitter.h"
#include "assembly-parser.h"

#include <array>
#include <cstdint>
#ifdef PANDA_TARGET_MOBILE
#include <unistd.h>
#endif

#include <vector>

#include <gtest/gtest.h>

namespace ark::panda_file::test {

static constexpr const char *ABC_FILE = "test_file.abc";

static std::unique_ptr<const File> GetPandaFile(std::vector<uint8_t> *data)
{
    os::mem::ConstBytePtr ptr(reinterpret_cast<std::byte *>(data->data()), data->size(),
                              [](std::byte *, size_t) noexcept {});
    return File::OpenFromMemory(std::move(ptr));
}

static std::vector<uint8_t> GetEmptyPandaFileBytes()
{
    pandasm::Parser p;

    auto source = R"()";

    std::string srcFilename = "src.pa";
    auto res = p.Parse(source, srcFilename);
    ASSERT(p.ShowError().err == pandasm::Error::ErrorType::ERR_NONE);

    auto pf = pandasm::AsmEmitter::Emit(res.Value());
    ASSERT(pf != nullptr);

    std::vector<uint8_t> data {};
    const auto headerPtr = reinterpret_cast<const uint8_t *>(pf->GetHeader());
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    data.assign(headerPtr, headerPtr + sizeof(File::Header));

    ASSERT(data.size() == sizeof(File::Header));

    return data;
}

int CreateOrAddZipPandaFile(std::vector<uint8_t> *data, const char *zipArchiveName, const char *filename, int append,
                            int level)
{
    return CreateOrAddFileIntoZip(zipArchiveName, filename, data, append, level);
}

bool CheckAnonMemoryName([[maybe_unused]] const char *zipArchiveName)
{
    // check if [annon:panda-classes.abc extracted in memory from /xx/__OpenPandaFileFromZip__.zip]
#ifdef PANDA_TARGET_MOBILE
    bool result = false;
    const char *prefix = "[anon:panda-";
    int pid = getpid();
    std::stringstream ss;
    ss << "/proc/" << pid << "/maps";
    std::ifstream f;
    f.open(ss.str(), std::ios::in);
    EXPECT_TRUE(f.is_open());
    for (std::string line; std::getline(f, line);) {
        if (line.find(prefix) != std::string::npos && line.find(zipArchiveName) != std::string::npos) {
            result = true;
        }
    }
    f.close();
    return result;
#else
    return true;
#endif
}

TEST(File, OpenMemory)
{
    {
        auto data = GetEmptyPandaFileBytes();
        auto ptr = GetPandaFile(&data);
        EXPECT_NE(ptr, nullptr);
    }

    {
        auto data = GetEmptyPandaFileBytes();
        data[0] = 0x0;  // Corrupt magic

        auto ptr = GetPandaFile(&data);
        EXPECT_EQ(ptr, nullptr);
    }
}

TEST(File, GetClassByName)
{
    ItemContainer container;

    std::vector<std::string> names = {"C", "B", "A"};
    std::vector<ClassItem *> classes;
    classes.reserve(names.size());

    for (auto &name : names) {
        classes.push_back(container.GetOrCreateClassItem(name));
    }

    MemoryWriter memWriter;

    ASSERT_TRUE(container.Write(&memWriter));

    // Read panda file from memory

    auto data = memWriter.GetData();
    auto pandaFile = GetPandaFile(&data);
    ASSERT_NE(pandaFile, nullptr);

    for (size_t i = 0; i < names.size(); i++) {
        EXPECT_EQ(pandaFile->GetClassId(reinterpret_cast<const uint8_t *>(names[i].c_str())).GetOffset(),
                  classes[i]->GetOffset());
    }
}

TEST(File, OpenPandaFile)
{
    // Create ZIP
    auto data = GetEmptyPandaFileBytes();
    int ret;
    const char *zipFilename = "__OpenPandaFile__.zip";
    const char *filename1 = ARCHIVE_FILENAME;
    const char *filename2 = "classses2.abc";  // just for testing.
    ret = CreateOrAddZipPandaFile(&data, zipFilename, filename1, APPEND_STATUS_CREATE, Z_BEST_COMPRESSION);
    ASSERT_EQ(ret, 0);
    ret = CreateOrAddZipPandaFile(&data, zipFilename, filename2, APPEND_STATUS_ADDINZIP, Z_BEST_COMPRESSION);
    ASSERT_EQ(ret, 0);

    // Open from ZIP
    auto pf = OpenPandaFile(zipFilename);
    EXPECT_NE(pf, nullptr);
    EXPECT_STREQ((pf->GetFilename()).c_str(), zipFilename);
    remove(zipFilename);
}

TEST(File, OpenPandaFileFromMemory)
{
    auto pf = OpenPandaFileFromMemory(nullptr, -1);
    EXPECT_EQ(pf, nullptr);

    pf = OpenPandaFileFromMemory(nullptr, 1U);
    EXPECT_EQ(pf, nullptr);

    std::string tag = "ArkTS Code";
    pf = OpenPandaFileFromMemory(nullptr, -1, tag);
    EXPECT_EQ(pf, nullptr);

    tag = "";
    pf = OpenPandaFileFromMemory(nullptr, 1U, tag);
    EXPECT_EQ(pf, nullptr);
}

TEST(File, OpenPandaFileFromZipNameAnonMem)
{
    // Create ZIP
    auto data = GetEmptyPandaFileBytes();
    int ret;
    const char *zipFilename = "__OpenPandaFileFromZipNameAnonMem__.zip";
    const char *filename1 = ARCHIVE_FILENAME;
    ret = CreateOrAddZipPandaFile(&data, zipFilename, filename1, APPEND_STATUS_CREATE, Z_BEST_COMPRESSION);
    ASSERT_EQ(ret, 0);

    // Open from ZIP
    auto pf = OpenPandaFile(zipFilename);
    EXPECT_NE(pf, nullptr);
    EXPECT_STREQ((pf->GetFilename()).c_str(), zipFilename);
    ASSERT_TRUE(CheckAnonMemoryName(zipFilename));
    remove(zipFilename);
}

TEST(File, OpenPandaFile_Zip_Failure_FpClosed)
{
    // Create an empty zip file
    const char *zipFilename = "__TestZipFclose__.zip";

    {
        FILE *zip = fopen(zipFilename, "wbe");
        ASSERT_NE(zip, nullptr);

        // NOLINTNEXTLINE(readability-magic-numbers)
        std::array<uint8_t, 16> emptyZipData {
            // NOLINTNEXTLINE(readability-magic-numbers)
            {0x50, 0x4B, 0x05, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};

        size_t written = fwrite(emptyZipData.data(), 1, emptyZipData.size(), zip);
        ASSERT_EQ(written, sizeof(emptyZipData));

        fclose(zip);
    }

    std::unique_ptr<const panda_file::File> file = OpenPandaFile(zipFilename);
    EXPECT_EQ(file, nullptr);

    // Reopen the file to ensure it was properly closed and is accessible again.
    FILE *fp = fopen(zipFilename, "rbe");
    EXPECT_NE(fp, nullptr);
    fclose(fp);

    remove(zipFilename);
}

TEST(File, OpenPandaFile_Zip_Success_FpClosed)
{
    const char *zipFilename = "__TestNotEmptyZip__.zip";
    const char *abcFilename = ARCHIVE_FILENAME;

    auto data = GetEmptyPandaFileBytes();

    int ret = CreateOrAddZipPandaFile(&data, zipFilename, abcFilename, APPEND_STATUS_CREATE, Z_BEST_COMPRESSION);
    ASSERT_EQ(ret, 0);

    std::unique_ptr<const panda_file::File> file = OpenPandaFile(zipFilename);
    EXPECT_NE(file, nullptr);

    FILE *fp = fopen(zipFilename, "rbe");
    EXPECT_NE(fp, nullptr);
    fclose(fp);

    remove(zipFilename);
}

TEST(File, OpenPandaFile_File_Success_FpClosed)
{
    const char *filename = "__TestPandaFile__.abc";

    {
        auto data = GetEmptyPandaFileBytes();
        FILE *fp = fopen(filename, "wbe");
        ASSERT_NE(fp, nullptr);
        size_t written = fwrite(data.data(), sizeof(uint8_t), data.size(), fp);
        ASSERT_EQ(written, data.size());
        fclose(fp);
    }

    std::unique_ptr<const panda_file::File> file = OpenPandaFile(filename);
    EXPECT_NE(file, nullptr);

    FILE *check = fopen(filename, "rbe");
    EXPECT_NE(check, nullptr);
    fclose(check);

    remove(filename);
}

TEST(File, OpenPandaFile_File_Failure_FpClosed)
{
    const char *filename = "__TestBadPandaFile__.abc";

    {
        constexpr uint32_t FAKE_MAGIC = 0x12345678;
        FILE *fp = fopen(filename, "wbe");
        ASSERT_NE(fp, nullptr);
        fwrite(&FAKE_MAGIC, sizeof(FAKE_MAGIC), 1, fp);
        fclose(fp);
    }

    std::unique_ptr<const panda_file::File> file = OpenPandaFile(filename);
    EXPECT_EQ(file, nullptr);

    FILE *check = fopen(filename, "rbe");
    EXPECT_NE(check, nullptr);
    fclose(check);

    remove(filename);
}

TEST(File, OpenPandaFileOrZip)
{
    // Create ZIP
    auto data = GetEmptyPandaFileBytes();
    int ret;
    const char *zipFilename = "__OpenPandaFileOrZip__.zip";
    const char *filename1 = ARCHIVE_FILENAME;
    const char *filename2 = "classes2.abc";  // just for testing.
    ret = CreateOrAddZipPandaFile(&data, zipFilename, filename1, APPEND_STATUS_CREATE, Z_BEST_COMPRESSION);
    ASSERT_EQ(ret, 0);
    ret = CreateOrAddZipPandaFile(&data, zipFilename, filename2, APPEND_STATUS_ADDINZIP, Z_BEST_COMPRESSION);
    ASSERT_EQ(ret, 0);

    // Open from ZIP
    auto pf = OpenPandaFileOrZip(zipFilename);
    EXPECT_NE(pf, nullptr);
    EXPECT_STREQ((pf->GetFilename()).c_str(), zipFilename);
    remove(zipFilename);
}

TEST(File, OpenPandaFileFromZipErrorHandler)
{
    const char *fileName = "test_file_empty.panda";
    {
        auto writer = FileWriter(fileName);
        ASSERT_TRUE(writer);
    }
    auto pf = OpenPandaFile(fileName);
    EXPECT_EQ(pf, nullptr);

    const char *fileNameZip = "test_file_empty.zip";
    {
        auto writer = FileWriter(fileName);
        ASSERT_TRUE(writer);
        const std::vector<uint8_t> magic = {'P', 'K'};
        ASSERT_TRUE(writer.WriteBytes(magic));
    }
    pf = OpenPandaFile(fileNameZip);
    EXPECT_EQ(pf, nullptr);

    // Create ZIP
    const char *fileNameZipWithEntry = "test_file_with_entry.zip";
    std::vector<uint8_t> data = {};
    int ret = CreateOrAddZipPandaFile(&data, fileNameZipWithEntry, ARCHIVE_FILENAME, APPEND_STATUS_CREATE,
                                      Z_BEST_COMPRESSION);
    ASSERT_EQ(ret, 0);
    pf = OpenPandaFile(fileNameZipWithEntry);
    EXPECT_EQ(pf, nullptr);

    auto fp = fopen(fileNameZipWithEntry, "ae");
    EXPECT_NE(fp, nullptr);
    const char *appendStr = "error";
    EXPECT_EQ(fwrite(appendStr, 1U, 1U, fp), 1U);
    fclose(fp);
    pf = OpenPandaFile(fileNameZipWithEntry);
    EXPECT_EQ(pf, nullptr);

    remove(fileName);
    remove(fileNameZip);
    remove(fileNameZipWithEntry);
}

TEST(File, HandleArchive)
{
    {
        ItemContainer container;
        auto writer = FileWriter(ARCHIVE_FILENAME);
        ASSERT_TRUE(container.Write(&writer));
        ASSERT_TRUE(writer.Align(4U));  // to 4 bytes align
    }

    std::vector<uint8_t> data;
    {
        std::ifstream in(ARCHIVE_FILENAME, std::ios::binary);
        data.insert(data.end(), (std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        ASSERT_TRUE(!data.empty() && data.size() % 4U == 0U);
    }

    // Create ZIP
    const char *zipFilename = "__HandleArchive__.zip";
    int ret = CreateOrAddZipPandaFile(&data, zipFilename, ARCHIVE_FILENAME, APPEND_STATUS_CREATE, Z_NO_COMPRESSION);
    ASSERT_EQ(ret, 0);
    auto pf = OpenPandaFile(zipFilename);
    EXPECT_NE(pf, nullptr);

    remove(ARCHIVE_FILENAME);
    remove(zipFilename);
}

TEST(File, CheckHeader1)
{
    // Write panda file to disk
    ItemContainer container;

    auto writer = FileWriter(ABC_FILE);
    ASSERT_TRUE(container.Write(&writer));

    // Read panda file from disk
    auto fp = fopen(ABC_FILE, "rbe");
    EXPECT_NE(fp, nullptr);

    os::mem::ConstBytePtr ptr = os::mem::MapFile(os::file::File(fileno(fp)), os::mem::MMAP_PROT_READ,
                                                 os::mem::MMAP_FLAG_PRIVATE, writer.GetOffset())
                                    .ToConst();
    EXPECT_NE(ptr.Get(), nullptr);
    EXPECT_TRUE(CheckHeader(ptr, ABC_FILE));
    fclose(fp);

    remove(ABC_FILE);
}

TEST(File, GetMode)
{
    // Write panda file to disk
    ItemContainer container;

    auto writer = FileWriter(ABC_FILE);
    ASSERT_TRUE(container.Write(&writer));

    // Read panda file from disk
    EXPECT_NE(File::Open(ABC_FILE), nullptr);
    EXPECT_EQ(File::Open(ABC_FILE, File::OpenMode::WRITE_ONLY), nullptr);

    remove(ABC_FILE);
}

TEST(File, Open)
{
    EXPECT_EQ(File::Open(ABC_FILE), nullptr);

    auto fp = fopen(ABC_FILE, "we");
    EXPECT_NE(fp, nullptr);
    const char *writeStr = "error";
    EXPECT_EQ(fwrite(writeStr, 1U, 1U, fp), 1U);
    fclose(fp);
    EXPECT_EQ(File::Open(ABC_FILE), nullptr);
    EXPECT_EQ(File::Open(ABC_FILE, File::OpenMode::WRITE_ONLY), nullptr);

    remove(ABC_FILE);
}

TEST(File, TestValidChecksum)
{
    ItemContainer container;
    auto writer = FileWriter(ABC_FILE);
    ASSERT_TRUE(container.Write(&writer));

    // Read panda file from disk
    auto fp = fopen(ABC_FILE, "rbe");
    EXPECT_NE(fp, nullptr);

    os::mem::ConstBytePtr ptr = os::mem::MapFile(os::file::File(fileno(fp)), os::mem::MMAP_PROT_READ,
                                                 os::mem::MMAP_FLAG_PRIVATE, writer.GetOffset())
                                    .ToConst();
    EXPECT_TRUE(ValidateChecksum(ptr, ABC_FILE));
    fclose(fp);

    remove(ABC_FILE);
}

TEST(File, TestInvalidChecksum)
{
    ItemContainer container;
    auto writer = FileWriter(ABC_FILE);
    ASSERT_TRUE(container.Write(&writer));

    // Read panda file from disk, and make it invalid
    auto fp = fopen(ABC_FILE, "rb+e");
    EXPECT_NE(fp, nullptr);
    File::Header hdr = {};
    auto cnt = fread(&hdr, sizeof(File::Header), 1, fp);
    EXPECT_GT(cnt, 0);
    const uint32_t destructiveBytes = 100;
    hdr.numIndexes = destructiveBytes;
    fseek(fp, 0, SEEK_SET);
    cnt = fwrite(&hdr, sizeof(uint8_t), sizeof(File::Header), fp);
    EXPECT_GT(cnt, 0);
    fclose(fp);

    // Read and Check
    fp = fopen(ABC_FILE, "rbe");
    EXPECT_NE(fp, nullptr);
    cnt = fread(&hdr, sizeof(File::Header), 1, fp);
    EXPECT_GT(cnt, 0);
    os::mem::ConstBytePtr ptr = os::mem::MapFile(os::file::File(fileno(fp)), os::mem::MMAP_PROT_READ,
                                                 os::mem::MMAP_FLAG_PRIVATE, writer.GetOffset())
                                    .ToConst();
    EXPECT_FALSE(ValidateChecksum(ptr, ABC_FILE));
    fclose(fp);
    remove(ABC_FILE);
}

TEST(File, OpenUncompressedArchive)
{
    // Invalid FD
    EXPECT_EQ(File::OpenUncompressedArchive(-1, ABC_FILE, 0U, 0U), nullptr);

    // Invalid Size
    EXPECT_EQ(File::OpenUncompressedArchive(1, ABC_FILE, 0U, 0U), nullptr);

    // Invalid Max Size
    EXPECT_EQ(File::OpenUncompressedArchive(1, ABC_FILE, -1, 0U), nullptr);

    // Invalid ABC File
    auto data = GetEmptyPandaFileBytes();
    auto fp = fopen(ARCHIVE_FILENAME, "w+e");
    EXPECT_NE(fp, nullptr);
    data[0] = 0U;
    EXPECT_EQ(fwrite(data.data(), sizeof(uint8_t), data.size(), fp), data.size());
    (void)fseek(fp, 0, SEEK_SET);
    EXPECT_EQ(File::OpenUncompressedArchive(fileno(fp), ARCHIVE_FILENAME, sizeof(File::Header), 0U), nullptr);
    fclose(fp);

    remove(ABC_FILE);
}

TEST(File, CheckLiteralArray)
{
    // Write panda file to disk
    ItemContainer container;

    auto writer = FileWriter(ABC_FILE);
    ASSERT_TRUE(container.Write(&writer));

    // Read panda file from disk
    auto fp = fopen(ABC_FILE, "rbe");
    EXPECT_NE(fp, nullptr);

    os::mem::ConstBytePtr ptr =
        os::mem::MapFile(os::file::File(fileno(fp)), os::mem::MMAP_PROT_READ | os::mem::MMAP_PROT_WRITE,
                         os::mem::MMAP_FLAG_PRIVATE, writer.GetOffset())
            .ToConst();
    EXPECT_NE(ptr.Get(), nullptr);
    EXPECT_TRUE(CheckHeader(ptr, ABC_FILE));

    fclose(fp);

    remove(ABC_FILE);
}

TEST(File, OpenPandaFileUncompressed)
{
    // Create ZIP
    auto data = GetEmptyPandaFileBytes();
    std::cout << "pandafile size = " << data.size() << std::endl;
    int ret;
    const char *zipFilename = "__OpenPandaFileUncompressed__.zip";
    const char *filename1 = ARCHIVE_FILENAME;
    const char *filename2 = "class.abc";  // just for testing.
    ret = CreateOrAddZipPandaFile(&data, zipFilename, filename2, APPEND_STATUS_CREATE, Z_NO_COMPRESSION);
    ASSERT_EQ(ret, 0);
    ret = CreateOrAddZipPandaFile(&data, zipFilename, filename1, APPEND_STATUS_ADDINZIP, Z_NO_COMPRESSION);
    ASSERT_EQ(ret, 0);

    // Open from ZIP
    auto pf = OpenPandaFileOrZip(zipFilename);
    EXPECT_NE(pf, nullptr);
    EXPECT_STREQ((pf->GetFilename()).c_str(), zipFilename);
    remove(zipFilename);
}

TEST(File, LineNumberProgramDeduplication)
{
    pandasm::Parser p;

    auto source = R"delim(
        .function void foo() {
            return.void
        }

        .function void bar() {
            return.void
        }
    )delim";

    std::string srcFilename = "src.pa";
    auto res = p.Parse(source, srcFilename);
    ASSERT(p.ShowError().err == pandasm::Error::ErrorType::ERR_NONE);

    ASSERT_EQ(res.Value().functionStaticTable.size(), 2);
    for (auto &a : res.Value().functionStaticTable) {
        ASSERT_TRUE(a.second.HasDebugInfo());
    }

    auto pf = pandasm::AsmEmitter::Emit(res.Value());
    ASSERT(pf != nullptr);

    auto reader = FileReader(std::move(pf));

    ASSERT_TRUE(reader.ReadContainer());

    int lnpCnt = 0;

    ASSERT_NE(reader.GetItems()->size(), 0);

    for (const auto &a : *reader.GetItems()) {
        const auto typ = a.second->GetItemType();
        if (typ == ItemTypes::LINE_NUMBER_PROGRAM_ITEM) {
            lnpCnt++;
        }
    }

    reader.GetContainerPtr()->ComputeLayout();
    reader.GetContainerPtr()->DeduplicateCodeAndDebugInfo();

    ASSERT_EQ(lnpCnt, 1);
}

constexpr uint8_t FIZE_SIZE_ERROR_SIZE = 0x09;
TEST(File, CheckHeader)
{
    auto data = GetEmptyPandaFileBytes();
    auto f = GetPandaFile(&data);
    auto res = ark::panda_file::CheckHeader(f->GetPtr(), std::string_view(), data.size());
    EXPECT_EQ(res, true);

    const int fileSizeIndex = offsetof(ark::panda_file::File::Header, fileSize);
    data[fileSizeIndex] = FIZE_SIZE_ERROR_SIZE;  // Corrupt file size
    data[fileSizeIndex + 1] = FIZE_SIZE_ERROR_SIZE;
    data[fileSizeIndex + 2] = FIZE_SIZE_ERROR_SIZE;
    data[fileSizeIndex + 3] = FIZE_SIZE_ERROR_SIZE;

    res = ark::panda_file::CheckHeader(f->GetPtr(), std::string_view(), data.size());
    EXPECT_EQ(res, false);
}

TEST(File, OpenPandaFileFromSecureMemory)
{
    // test buffer is nullptr
    auto pf = OpenPandaFileFromSecureMemory(nullptr, -1);
    EXPECT_EQ(pf, nullptr);

    // test buff is not nullptr
    const size_t bufferSize = 1024;
    std::array<uint8_t, bufferSize> buffer = {0};
    pf = OpenPandaFileFromSecureMemory(buffer.data(), bufferSize);
    EXPECT_EQ(pf, nullptr);
}

TEST(File, LoadPandFileExt_Behavior)
{
    using Wrapper = panda_api::panda_file::PandaFileWrapper;

    // Before loading, all function pointers should be null
    EXPECT_EQ(Wrapper::pOpenPandafileFromFdExt, nullptr);
    EXPECT_EQ(Wrapper::pOpenPandafileFromMemoryExt, nullptr);
    EXPECT_EQ(Wrapper::pQueryMethodSymByOffsetExt, nullptr);
    EXPECT_EQ(Wrapper::pQueryMethodSymAndLineByOffsetExt, nullptr);
    EXPECT_EQ(Wrapper::pQueryAllMethodSymsExt, nullptr);

    // Load symbols
    panda_api::panda_file::LoadPandFileExt();

    bool allSet = Wrapper::pOpenPandafileFromFdExt != nullptr && Wrapper::pOpenPandafileFromMemoryExt != nullptr &&
                  Wrapper::pQueryMethodSymByOffsetExt != nullptr &&
                  Wrapper::pQueryMethodSymAndLineByOffsetExt != nullptr && Wrapper::pQueryAllMethodSymsExt != nullptr;

    EXPECT_TRUE(allSet) << "Function pointers are in all_set state";
}

}  // namespace ark::panda_file::test
