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

#include "ziparchivehandle_fuzzer.h"

#include <string>
#include "libziparchive/zip_archive.h"

namespace OHOS {
    void ZipArchiveHandleFuzzTest(const uint8_t* data, size_t size)
    {
        std::string str(data, data + size);

        {
            // ExtractToMemory test
            panda::ZipArchiveHandle handle = nullptr;
            uint8_t* buf = const_cast<uint8_t*>(data);
            panda::ExtractToMemory(handle, buf, size);
        }
        {
            // LocateFile test
            panda::ZipArchiveHandle zipfile = nullptr;
            const char* filename = str.c_str();
            panda::LocateFile(zipfile, filename);
        }
        {
            // OpenArchive test
            panda::ZipArchiveHandle handle = nullptr;
            const char* path = str.c_str();
            panda::OpenArchive(handle, path);
        }
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::ZipArchiveHandleFuzzTest(data, size);
    return 0;
}