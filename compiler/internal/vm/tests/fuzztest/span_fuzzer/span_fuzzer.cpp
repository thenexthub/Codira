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

#include "span_fuzzer.h"

#include "utils/span.h"

#include <vector>

namespace OHOS {
    void SpanFuzzTest(const uint8_t* data, size_t size)
    {
        const int num = 1;

        std::vector<uint8_t*> dataVector(size, const_cast<uint8_t*>(data));
        panda::Span<uint8_t*> instance(dataVector.data(), dataVector.data() + dataVector.size());

        const std::vector<const uint8_t*> constDataVector(size, data);
        auto spanInstance = panda::Span(constDataVector);
        spanInstance.begin();
        spanInstance.cbegin();
        spanInstance.end();
        spanInstance.cend();
        spanInstance.crbegin();
        spanInstance.crend();
        spanInstance[num];
        spanInstance.Size();
        spanInstance.Data();
        spanInstance.First(num);
        spanInstance.Last(num);
        spanInstance.SubSpan(num, num);
        spanInstance.SubSpan(num);
        spanInstance.size();
        spanInstance.empty();
        spanInstance.data();
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::SpanFuzzTest(data, size);
    return 0;
}