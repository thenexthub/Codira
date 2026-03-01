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

#include "begintracepoint_fuzzer.h"
#include <fuzzer/FuzzedDataProvider.h>

#include "trace/trace.h"

namespace OHOS {
    void BeginTracePointFuzzTest(const uint8_t* data, size_t size)
    {
        FuzzedDataProvider fdp(data, size);
        std::string traceName = fdp.ConsumeRandomLengthString();
        panda::trace::BeginTracePoint(traceName.c_str());
        panda::trace::EndTracePoint();
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::BeginTracePointFuzzTest(data, size);
    return 0;
}
