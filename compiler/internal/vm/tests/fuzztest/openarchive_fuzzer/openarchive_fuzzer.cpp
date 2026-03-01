/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "openarchive_fuzzer.h"

#include "zip_archive.h"

namespace OHOS {
void OpenArchiveFuzzTest(const uint8_t *data, size_t size)
{
    // Create zip file
    const char *zip_filename = "__OpenArchiveFuzzTest.zip";
    const char *filename = "__OpenArchiveFuzzTest_tmp.data";
    int ret = panda::CreateOrAddFileIntoZip(zip_filename, filename, data, size, APPEND_STATUS_CREATE, Z_NO_COMPRESSION);
    if (ret != 0) {
        (void)remove(zip_filename);
        return;
    }

    panda::ZipArchiveHandle zipfile = nullptr;
    if (panda::OpenArchive(zipfile, zip_filename) != 0) {
        (void)remove(zip_filename);
        return;
    }
    panda::CloseArchive(zipfile);
    (void)remove(zip_filename);
}
}  // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::OpenArchiveFuzzTest(data, size);
    return 0;
}