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

#include "heapprofilerstopsampling_fuzzer.h"
#include "ecmascript/napi/include/jsnapi.h"
#include "agent/heapprofiler_impl.h"
#include "tooling/dynamic/dispatcher.h"

using namespace panda;
using namespace panda::ecmascript;
using namespace panda::ecmascript::tooling;

#define MAXBYTELEN sizeof(int32_t)

namespace OHOS {
    void HeapprofilerStopSamplingFuzzTest(const uint8_t* data, size_t size)
    {
        RuntimeOption option;
        option.SetLogLevel(RuntimeOption::LOG_LEVEL::ERROR);
        auto vm = JSNApi::CreateJSVM(option);
        if (size <= 0) {
            return;
        }
        int32_t input = 0;
        if (size > MAXBYTELEN) {
            size = MAXBYTELEN;
        }
        if (memcpy_s(&input, MAXBYTELEN, data, size) != 0) {
            std::cout << "memcpy_s failed!";
            UNREACHABLE();
        }
        std::string str(data, data + size);
        auto req = std::make_unique<DispatchRequest>(str);
        auto heapProfiler = std::make_unique<HeapProfilerImpl>(vm, nullptr);
        auto dispatcherImpl =
            std::make_unique<HeapProfilerImpl::DispatcherImpl>(nullptr, std::move(heapProfiler));
        dispatcherImpl->StopSampling(*req);
        JSNApi::DestroyJSVM(vm);
    }
}

// Fuzzer entry point.
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    // Run your code on data.
    OHOS::HeapprofilerStopSamplingFuzzTest(data, size);
    return 0;
}