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

#include "gotonextfile_fuzzer.h"

#include <cstdio>

#include "assembler/assembly-emitter.h"
#include "assembler/assembly-parser.h"
#include "libziparchive/zip_archive.h"
#include "libpandafile/file.h"

constexpr size_t MAX_BUFFER_SIZE = 2048;
constexpr size_t MAX_DIR_SIZE = 64;

namespace OHOS {
    void GenerateZipfile(const char* data, const char* archive_name, int n, char* buf, char* archive_filename, int& i,
                         int& ret, std::vector<uint8_t>& pf_data, int level = Z_BEST_COMPRESSION)
    {
        // Delete the test archive, so it doesn't keep growing as we run this test
        (void)remove(archive_name);
        // Create and append a directory entry for testing
        ret = panda::CreateOrAddFileIntoZip(archive_name, "directory/", NULL, 0, APPEND_STATUS_CREATE, level);
        if (ret != 0) {
            return;
        }
        int err = 0;
        // Append a bunch of text files to the test archive
        for (i = (n - 1); i >= 0; --i) {
            err = sprintf_s(archive_filename, MAX_DIR_SIZE, "%d.txt", i);
            if (err != 0) {
                return;
            }
            err = sprintf_s(buf, MAX_BUFFER_SIZE, "%d %s %d", (n - 1) - i, data, i);
            if (err != 0) {
                return;
            }
            ret = panda::CreateOrAddFileIntoZip(archive_name, archive_filename, buf, strlen(buf) + 1,
                                                APPEND_STATUS_ADDINZIP, level);
            if (ret != 0) {
                return;
            }
        }
    }

    int MakePfData(std::vector<uint8_t>& pf_data)
    {
        panda::pandasm::Parser p;
        auto source = R"()";
        std::string src_filename = "src.pa";
        auto res = p.Parse(source, src_filename);
        if (p.ShowError().err != panda::pandasm::Error::ErrorType::ERR_NONE) {
            return 1;
        }
        auto pf = panda::pandasm::AsmEmitter::Emit(res.Value());
        if (pf == nullptr) {
            return 1;
        }
        const auto header_ptr = reinterpret_cast<const uint8_t *>(pf->GetHeader());
        pf_data.assign(header_ptr, header_ptr + sizeof(panda::panda_file::File::Header));
        return 0;
    }

    void GoToNextFileFuzzTest(const uint8_t* data, [[maybe_unused]] size_t size)
    {
        {
            // handle is not nullptr
            // creating an empty pandafile
            const char* s = reinterpret_cast<char*>(const_cast<uint8_t*>(data));
            std::vector<uint8_t> pf_data {};
            int err = MakePfData(pf_data);
            if (err != 0) {
                return;
            }
            static const char* archive_name = "__LIBZIPARCHIVE__ZipFile__.zip";
            const int n = 3;
            char buf[MAX_BUFFER_SIZE];
            char archive_filename[MAX_DIR_SIZE];
            int i = 0;
            int ret = 0;
            GenerateZipfile(s, archive_name, n, buf, archive_filename, i, ret, pf_data);

            // Quick Check
            panda::ZipArchiveHandle zipfile = nullptr;
            if (panda::OpenArchive(zipfile, archive_name) != 0) {
                return;
            }
            panda::GlobalStat gi = panda::GlobalStat();
            if (panda::GetGlobalFileInfo(zipfile, &gi) != 0) {
                return;
            }
            int entrynum = static_cast<int>(gi.GetNumberOfEntry());
            for (i = 0; i < entrynum; ++i) {
                panda::EntryFileStat file_stat;
                if (panda::GetCurrentFileInfo(zipfile, &file_stat) != 0) {
                    panda::CloseArchive(zipfile);
                    return;
                }
                if ((i + 1) < entrynum) {
                    if (panda::GoToNextFile(zipfile) != 0) {
                        panda::CloseArchive(zipfile);
                        return;
                    }
                }
            }
            panda::CloseArchive(zipfile);
            (void)remove(archive_name);
        }
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::GoToNextFileFuzzTest(data, size);
    return 0;
}