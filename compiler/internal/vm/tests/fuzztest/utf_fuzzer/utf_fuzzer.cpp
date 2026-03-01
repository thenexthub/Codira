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

#include "utf_fuzzer.h"

#include "utils/utf.h"

namespace OHOS {
    void UtfFuzzTest(const uint8_t* data, size_t size)
    {
        using namespace panda::utf;

        std::string str(data, data + size);
        const uint8_t* string_data = CStringAsMutf8(const_cast<char*>(str.c_str()));

        Mutf8AsCString(data);
        IsAvailableNextUtf16Code(static_cast<uint16_t>(size));

        if (IsValidModifiedUTF8(string_data)) {
            Mutf8Less mutf8Less;
            mutf8Less(string_data, string_data);
        }

        SplitUtf16Pair(static_cast<uint32_t>(size));
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::UtfFuzzTest(data, size);
    return 0;
}

