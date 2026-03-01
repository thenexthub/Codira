/**
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "openuncompressedarchive_fuzzer.h"
#include "libarkfile/file.h"
#include "libziparchive/zip_archive.h"

namespace OHOS {
void CloseAndRemoveZipFile(ark::ZipArchiveHandle &handle, FILE *fp, const char *filename)
{
    ark::CloseArchiveFile(handle);
    (void)fclose(fp);
    (void)remove(filename);
}

void OpenUncompressedArchiveFuzzTest(const uint8_t *data, size_t size)
{
    // Create zip file
    const char *zip_filename = "__OpenUncompressedArchiveFuzzTest.zip";
    const char *filename = ark::panda_file::ARCHIVE_FILENAME;
    std::vector<uint8_t> buf(data, data + size);
    int ret = ark::CreateOrAddFileIntoZip(zip_filename, filename, &buf, APPEND_STATUS_CREATE, Z_NO_COMPRESSION);
    if (ret != 0) {
        (void)remove(zip_filename);
        return;
    }

    // Acquire entry
#ifdef PANDA_TARGET_WINDOWS
    constexpr char const *mode = "rb";
#else
    constexpr char const *mode = "rbe";
#endif
    FILE *fp = fopen(zip_filename, mode);
    if (fp == nullptr) {
        (void)remove(zip_filename);
        return;
    }
    ark::ZipArchiveHandle zipfile = nullptr;
    if (ark::OpenArchiveFile(zipfile, fp) != ark::ZIPARCHIVE_OK) {
        (void)fclose(fp);
        (void)remove(zip_filename);
        return;
    }
    if (ark::LocateFile(zipfile, filename) != ark::ZIPARCHIVE_OK) {
        CloseAndRemoveZipFile(zipfile, fp, zip_filename);
        return;
    }
    ark::EntryFileStat entry;
    if (ark::GetCurrentFileInfo(zipfile, &entry) != ark::ZIPARCHIVE_OK) {
        CloseAndRemoveZipFile(zipfile, fp, zip_filename);
        return;
    }
    if (ark::OpenCurrentFile(zipfile) != ark::ZIPARCHIVE_OK) {
        ark::CloseCurrentFile(zipfile);
        CloseAndRemoveZipFile(zipfile, fp, zip_filename);
        return;
    }
    ark::GetCurrentFileOffset(zipfile, &entry);
    // Call OpenUncompressedArchive
    {
        ark::panda_file::File::OpenUncompressedArchive(fileno(fp), zip_filename, entry.GetUncompressedSize(),
                                                       entry.GetOffset());
    }
    ark::CloseCurrentFile(zipfile);
    CloseAndRemoveZipFile(zipfile, fp, zip_filename);
}
}  // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::OpenUncompressedArchiveFuzzTest(data, size);
    return 0;
}
