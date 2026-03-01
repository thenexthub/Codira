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

#include "hashbase_fuzzer.h"

#include "utils/hash_base.h"
#include "utils/murmur3_hash.h"

namespace OHOS {
    void HashBaseFuzzTest(const uint8_t* data, size_t size)
    {
        const uint32_t seed = 0xCC9E2D51U;
        std::string str(data, data + size);
        uint8_t* string_data = reinterpret_cast<uint8_t*>(const_cast<char*>(str.c_str()));
        panda::HashBase<panda::MurmurHash32<seed>>::GetHash32(data, size);
        panda::HashBase<panda::MurmurHash32<seed>>::GetHash32String(string_data);
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::HashBaseFuzzTest(data, size);
    return 0;
}

