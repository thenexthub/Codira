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

#include "file-inl.h"
#include "file_items.h"
#include "file_item_container.h"
#include "utils/string_helpers.h"
#include "zip_archive.h"
#include "file.h"

#include "assembly-emitter.h"
#include "assembly-parser.h"

#include <cstdint>
#ifdef PANDA_TARGET_MOBILE
#include <unistd.h>
#endif

#include <vector>

#include <gtest/gtest.h>

namespace panda::panda_file::test {

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

    std::string src_filename = "src.pa";
    auto res = p.Parse(source, src_filename);
    ASSERT(p.ShowError().err == pandasm::Error::ErrorType::ERR_NONE);

    auto pf = pandasm::AsmEmitter::Emit(res.Value());
    ASSERT(pf != nullptr);

    std::vector<uint8_t> data {};
    const auto header_ptr = reinterpret_cast<const uint8_t *>(pf->GetHeader());
    data.assign(header_ptr, header_ptr + sizeof(File::Header));

    ASSERT(data.size() == sizeof(File::Header));

    return data;
}

int CreateOrAddZipPandaFile(std::vector<uint8_t> *data, const char *zip_archive_name, const char *filename, int append,
                            int level)
{
    return CreateOrAddFileIntoZip(zip_archive_name, filename, (*data).data(), (*data).size(), append, level);
}

bool CheckAnonMemoryName([[maybe_unused]] const char *zip_archive_name)
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
        if (line.find(prefix) != std::string::npos && line.find(zip_archive_name) != std::string::npos) {
            result = true;
        }
    }
    f.close();
    return result;
#else
    return true;
#endif
}

HWTEST(File, GetClassByName, testing::ext::TestSize.Level0)
{
    ItemContainer container;

    std::vector<std::string> names = {"C", "B", "A"};
    std::vector<ClassItem *> classes;

    for (auto &name : names) {
        classes.push_back(container.GetOrCreateClassItem(name));
    }

    MemoryWriter mem_writer;

    ASSERT_TRUE(container.Write(&mem_writer));

    // Read panda file from memory

    auto data = mem_writer.GetData();
    auto panda_file = GetPandaFile(&data);
    ASSERT_NE(panda_file, nullptr);

    for (size_t i = 0; i < names.size(); i++) {
        EXPECT_EQ(panda_file->GetClassId(reinterpret_cast<const uint8_t *>(names[i].c_str())).GetOffset(),
                  classes[i]->GetOffset());
    }
}

HWTEST(File, OpenPandaFile, testing::ext::TestSize.Level0)
{
    // Create ZIP
    auto data = GetEmptyPandaFileBytes();
    int ret;
    const char *zip_filename = "__OpenPandaFile__.zip";
    const char *filename1 = ARCHIVE_FILENAME;
    const char *filename2 = "classses2.abc";  // just for testing.
    ret = CreateOrAddZipPandaFile(&data, zip_filename, filename1, APPEND_STATUS_CREATE, Z_BEST_COMPRESSION);
    ASSERT_EQ(ret, 0);
    ret = CreateOrAddZipPandaFile(&data, zip_filename, filename2, APPEND_STATUS_ADDINZIP, Z_BEST_COMPRESSION);
    ASSERT_EQ(ret, 0);

    // Open from ZIP
    auto pf = OpenPandaFile(zip_filename);
    EXPECT_NE(pf, nullptr);
    EXPECT_STREQ((pf->GetFilename()).c_str(), zip_filename);

    // Open from ZIP with archive_filename
    const char *filename3 = "classses3.abc";  // just for testing.
    pf = OpenPandaFile(zip_filename, filename3);
    EXPECT_NE(pf, nullptr);

    remove(zip_filename);

    ret = CreateOrAddZipPandaFile(&data, zip_filename, filename2, APPEND_STATUS_CREATE, Z_BEST_COMPRESSION);
    ASSERT_EQ(ret, 0);

    // Open from ZIP without default archive_filename
    pf = OpenPandaFile(zip_filename);
    EXPECT_EQ(pf, nullptr);

    remove(zip_filename);
}

HWTEST(File, OpenPandaFileFromMemory, testing::ext::TestSize.Level0)
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

HWTEST(File, OpenPandaFileFromSecureMemory, testing::ext::TestSize.Level0)
{
    auto pf = OpenPandaFileFromSecureMemory(nullptr, -1);
    EXPECT_EQ(pf, nullptr);

    pf = OpenPandaFileFromSecureMemory(nullptr, 1U);
    EXPECT_EQ(pf, nullptr);
}

HWTEST(File, OpenPandaFileFromZipNameAnonMem, testing::ext::TestSize.Level0)
{
    // Create ZIP
    auto data = GetEmptyPandaFileBytes();
    int ret;
    const char *zip_filename = "__OpenPandaFileFromZipNameAnonMem__.zip";
    const char *filename1 = ARCHIVE_FILENAME;
    ret = CreateOrAddZipPandaFile(&data, zip_filename, filename1, APPEND_STATUS_CREATE, Z_BEST_COMPRESSION);
    ASSERT_EQ(ret, 0);

    // Open from ZIP
    auto pf = OpenPandaFile(zip_filename);
    EXPECT_NE(pf, nullptr);
    EXPECT_STREQ((pf->GetFilename()).c_str(), zip_filename);
    ASSERT_TRUE(CheckAnonMemoryName(zip_filename));

    remove(zip_filename);
}

HWTEST(File, OpenPandaFileOrZip, testing::ext::TestSize.Level0)
{
    // Create ZIP
    auto data = GetEmptyPandaFileBytes();
    int ret;
    const char *zip_filename = "__OpenPandaFileOrZip__.zip";
    const char *filename1 = ARCHIVE_FILENAME;
    const char *filename2 = "classes2.abc";  // just for testing.
    ret = CreateOrAddZipPandaFile(&data, zip_filename, filename1, APPEND_STATUS_CREATE, Z_BEST_COMPRESSION);
    ASSERT_EQ(ret, 0);
    ret = CreateOrAddZipPandaFile(&data, zip_filename, filename2, APPEND_STATUS_ADDINZIP, Z_BEST_COMPRESSION);
    ASSERT_EQ(ret, 0);

    // Open from ZIP
    auto pf = OpenPandaFileOrZip(zip_filename);
    EXPECT_NE(pf, nullptr);
    EXPECT_STREQ((pf->GetFilename()).c_str(), zip_filename);

    const char *zip_filename_with_entry = "__OpenPandaFileOrZip__.zip!/classes.abc";
    auto pf_new = OpenPandaFileOrZip(zip_filename_with_entry);
    EXPECT_NE(pf_new, nullptr);

    remove(zip_filename);
}

HWTEST(File, OpenPandaFileFromZipErrorHandler, testing::ext::TestSize.Level0)
{
    const char *file_name = "test_file_empty.panda";
    {
        auto writer = FileWriter(file_name);
        ASSERT_TRUE(writer);
    }
    auto pf = OpenPandaFile(file_name);
    EXPECT_EQ(pf, nullptr);

    const char *file_name_zip = "test_file_empty.zip";
    {
        auto writer = FileWriter(file_name);
        ASSERT_TRUE(writer);
        const std::vector<uint8_t> magic = {'P', 'K'};
        ASSERT_TRUE(writer.WriteBytes(magic));
    }
    pf = OpenPandaFile(file_name_zip);
    EXPECT_EQ(pf, nullptr);

    // Create ZIP
    const char *file_name_zip_with_entry = "test_file_with_entry.zip";
    std::vector<uint8_t> data = {};
    int ret = CreateOrAddZipPandaFile(&data, file_name_zip_with_entry, ARCHIVE_FILENAME,
        APPEND_STATUS_CREATE, Z_BEST_COMPRESSION);
    ASSERT_EQ(ret, 0);
    pf = OpenPandaFile(file_name_zip_with_entry);
    EXPECT_EQ(pf, nullptr);

    auto fp = fopen(file_name_zip_with_entry, "a");
    EXPECT_NE(fp, nullptr);
    const char append_str[] = "error";
    EXPECT_EQ(fwrite(append_str, sizeof(append_str), 1U, fp), 1U);
    fclose(fp);
    pf = OpenPandaFile(file_name_zip_with_entry);
    EXPECT_EQ(pf, nullptr);

    remove(file_name);
    remove(file_name_zip);
    remove(file_name_zip_with_entry);
}

HWTEST(File, HandleArchive, testing::ext::TestSize.Level0)
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
        ASSERT_TRUE(data.size() > 0U && data.size() % 4U == 0U);
    }

    // Create ZIP
    const char *zip_filename = "__HandleArchive__.zip";
    int ret = CreateOrAddZipPandaFile(&data, zip_filename, ARCHIVE_FILENAME, APPEND_STATUS_CREATE, Z_NO_COMPRESSION);
    ASSERT_EQ(ret, 0);
    auto pf = OpenPandaFile(zip_filename);
    EXPECT_NE(pf, nullptr);

    remove(ARCHIVE_FILENAME);
    remove(zip_filename);
}

HWTEST(File, CheckHeader, testing::ext::TestSize.Level0)
{
    // Write panda file to disk
    ItemContainer container;

    auto writer = FileWriter(ABC_FILE);
    ASSERT_TRUE(container.Write(&writer));

    // Read panda file from disk
    auto fp = fopen(ABC_FILE, "rb");
    EXPECT_NE(fp, nullptr);

    os::mem::ConstBytePtr ptr = os::mem::MapFile(os::file::File(fileno(fp)), os::mem::MMAP_PROT_READ,
        os::mem::MMAP_FLAG_PRIVATE, writer.GetOffset()).ToConst();
    EXPECT_NE(ptr.Get(), nullptr);
    EXPECT_TRUE(CheckHeader(ptr, ABC_FILE));
    fclose(fp);

    remove(ABC_FILE);
}

HWTEST(File, GetFileType, testing::ext::TestSize.Level0)
{
    // Write panda file to disk
    ItemContainer container;

    auto writer = FileWriter(ABC_FILE);
    ASSERT_TRUE(container.Write(&writer));

    os::file::File file = os::file::Open(ABC_FILE, os::file::Mode::READONLY);
    ASSERT_TRUE(file.IsValid());
    os::file::FileHolder fhHolder(file);
    auto res = file.GetFileSize();
    size_t size = res.Value();
    os::mem::ConstBytePtr ptr =
        os::mem::MapFile(file, os::mem::MMAP_PROT_READ, os::mem::MMAP_FLAG_PRIVATE, size).ToConst();
    const uint8_t *data = reinterpret_cast<const uint8_t *>(ptr.Get());
    panda::panda_file::PandaFileType fileType = panda::panda_file::GetFileType(data, static_cast<int32_t>(size));
    EXPECT_EQ(fileType, panda::panda_file::PandaFileType::FILE_DYNAMIC);
    remove(ABC_FILE);
}

HWTEST(File, GetMode, testing::ext::TestSize.Level0)
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

HWTEST(File, Open, testing::ext::TestSize.Level0)
{
    EXPECT_EQ(File::Open(ABC_FILE), nullptr);

    auto fp = fopen(ABC_FILE, "w");
    EXPECT_NE(fp, nullptr);
    const char write_str[] = "error";
    EXPECT_EQ(fwrite(write_str, sizeof(write_str), 1U, fp), 1U);
    fclose(fp);
    EXPECT_EQ(File::Open(ABC_FILE), nullptr);
    EXPECT_EQ(File::Open(ABC_FILE, File::OpenMode::WRITE_ONLY), nullptr);

    remove(ABC_FILE);
}

HWTEST(File, OpenUncompressedArchive, testing::ext::TestSize.Level0)
{
    // Invalid FD
    EXPECT_EQ(File::OpenUncompressedArchive(-1, ABC_FILE, 0U, 0U), nullptr);

    // Invalid Size
    EXPECT_EQ(File::OpenUncompressedArchive(1, ABC_FILE, 0U, 0U), nullptr);

    // Invalid Max Size
    EXPECT_EQ(File::OpenUncompressedArchive(1, ABC_FILE, -1, 0U), nullptr);

    // Invalid ABC File
    auto data = GetEmptyPandaFileBytes();
    auto fp = fopen(ARCHIVE_FILENAME, "w+");
    EXPECT_NE(fp, nullptr);
    data[0] = 0U;
    EXPECT_EQ(fwrite(data.data(), sizeof(uint8_t), data.size(), fp), data.size());
    (void)fseek(fp, 0, SEEK_SET);
    EXPECT_EQ(File::OpenUncompressedArchive(fileno(fp), ARCHIVE_FILENAME, sizeof(File::Header), 0U), nullptr);
    fclose(fp);

    remove(ABC_FILE);
}

HWTEST(File, CheckSecureMem, testing::ext::TestSize.Level0)
{
    uint8_t *data1 = nullptr;
    uintptr_t value1 = reinterpret_cast<uintptr_t>(data1);
    bool res1 = CheckSecureMem(value1, 0); // 0: size
    EXPECT_TRUE(res1);

    int data2 = 256;
    uintptr_t value2 = reinterpret_cast<uintptr_t>(&data2);
    bool res2 = CheckSecureMem(value2, 1000);
    EXPECT_TRUE(res2);

    int data3 = 41235235;
    uintptr_t value3 = reinterpret_cast<uintptr_t>(&data3);
    bool res3 = CheckSecureMem(value3, static_cast<size_t>(243423423523));
    EXPECT_TRUE(res3);
}

HWTEST(File, CheckLiteralArray, testing::ext::TestSize.Level0)
{
    // Write panda file to disk
    ItemContainer container;

    auto writer = FileWriter(ABC_FILE);
    ASSERT_TRUE(container.Write(&writer));

    // Read panda file from disk
    auto fp = fopen(ABC_FILE, "rb");
    EXPECT_NE(fp, nullptr);

    os::mem::ConstBytePtr ptr =
        os::mem::MapFile(os::file::File(fileno(fp)), os::mem::MMAP_PROT_READ | os::mem::MMAP_PROT_WRITE,
                         os::mem::MMAP_FLAG_PRIVATE, writer.GetOffset())
            .ToConst();
    EXPECT_NE(ptr.Get(), nullptr);
    EXPECT_TRUE(CheckHeader(ptr, ABC_FILE));

    auto header = reinterpret_cast<File::Header *>(reinterpret_cast<uintptr_t>(ptr.Get()));
    auto num_literalarrays = header->num_literalarrays;
    uint32_t i = 10;
    header->num_literalarrays = i;
    EXPECT_FALSE(CheckHeader(ptr, ABC_FILE));
    header->num_literalarrays = num_literalarrays;
    header->literalarray_idx_off = i;
    EXPECT_FALSE(CheckHeader(ptr, ABC_FILE));
    fclose(fp);

    remove(ABC_FILE);
}

}  // namespace panda::panda_file::test
