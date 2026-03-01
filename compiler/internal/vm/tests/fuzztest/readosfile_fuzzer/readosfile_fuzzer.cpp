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

#include "readosfile_fuzzer.h"

#include "platforms/unix/libpandabase/native_stack.h"

namespace OHOS {
void ReadOsFileFuzzTest(const uint8_t *data, size_t size)
{
#if defined(PANDA_TARGET_UNIX)
    const char *filename = "__OpenFuzzTest_data.tmp";
    FILE *fp = fopen(filename, "w");
    if (fp == nullptr) {
        return;
    }
    (void)fwrite(data, sizeof(uint8_t), size, fp);
    (void)fclose(fp);

    std::string result;
    panda::os::unix::native_stack::ReadOsFile(filename, &result);

    // Remove the temp file
    (void)remove(filename);
#endif
    // TODO(huangyu): add test for windows
}
}  // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::ReadOsFileFuzzTest(data, size);
    return 0;
}