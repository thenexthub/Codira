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

#include "pttypesremoteobjectgetvalue_fuzzer.h"
#include "ecmascript/napi/include/jsnapi.h"
#include "tooling/dynamic/base/pt_types.h"

using namespace panda;
using namespace panda::ecmascript;
using namespace panda::ecmascript::tooling;

namespace OHOS {
    void PtTypesRemoteObjectGetValueFuzzTest(const uint8_t* data, size_t size)
    {
        RuntimeOption option;
        option.SetLogLevel(RuntimeOption::LOG_LEVEL::ERROR);
        auto vm = JSNApi::CreateJSVM(option);
        if (size <= 0 || data == NULL) {
            return;
        }
        double input = 0;
        if (size > sizeof(double)) {
            size = sizeof(double);
        }
        if (memcpy_s(&input, sizeof(double), data, size) != 0) {
            std::cout << "memcpy_s failed";
            UNREACHABLE();
        }
        Local value(NumberRef::New(vm, input));
        RemoteObject obj;
        obj.SetValue(value);
        Local<JSValueRef> ref = obj.GetValue();
        ref.IsEmpty();
        obj.HasValue();
        JSNApi::DestroyJSVM(vm);
    }
}

// Fuzzer entry point.
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    // Run your code on data.
    OHOS::PtTypesRemoteObjectGetValueFuzzTest(data, size);
    return 0;
}
