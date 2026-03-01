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

#include "baseptsbreakpointsetlocation_fuzzer.h"
#include "ecmascript/napi/include/jsnapi.h"
#include "tooling/dynamic/base/pt_events.h"
#include "tooling/dynamic/dispatcher.h"
#include "test/fuzztest/common_fuzzer/common_fuzzer.h"

using namespace panda;
using namespace panda::ecmascript;
using namespace panda::ecmascript::tooling;

#define MAXBYTELEN sizeof(int32_t)

namespace OHOS {
    void BasePtsBreakpointSetLocationFuzzTest(const uint8_t* data, size_t size)
    {
        auto cn = std::make_unique<common_fuzzer>();
        auto vm = cn->GetEcvm();
        int index = 0;
        int32_t sid = 2;
        std::string str(data, data + size);
        if (size <= 0) {
            return;
        }
        if (size > MAXBYTELEN) {
            size = MAXBYTELEN;
        }
        if (memcpy_s(&index, MAXBYTELEN, data, size) != 0) {
            std::cout << "memcpy_s failed!";
            UNREACHABLE();
        }
        BreakpointResolved breakpointResolved;
        auto location = std::make_unique<Location>();
        location->SetScriptId(sid).SetLine(index);
        breakpointResolved.SetBreakpointId("00").SetLocation(std::move(location));
        std::unique_ptr<PtJson> json = breakpointResolved.ToJson();
        breakpointResolved.SetLocation(Location::Create(*json));
        cn->DestroyEcvm(vm);
    }
}

// Fuzzer entry point.
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    // Run your code on data.
    OHOS::BasePtsBreakpointSetLocationFuzzTest(data, size);
    return 0;
}