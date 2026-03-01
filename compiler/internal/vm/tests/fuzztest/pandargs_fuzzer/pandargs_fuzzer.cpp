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

#include "pandargs_fuzzer.h"

#include "utils/pandargs.h"

namespace OHOS {
    void PandargsFuzzTest(const uint8_t* data, size_t size)
    {
        std::string str(data, data + size);
        panda::arg_list_t arg = {str};
        panda::PandArg<panda::arg_list_t> pandarg_arg("list", arg, "Sample arg_list_t argument", "a");

        panda::PandArg<uint64_t> pandarg("uint64", static_cast<uint64_t>(size), "Sample uint64 argument");
        pandarg.GetValue();
        pandarg.SetValue(1);
        pandarg.ResetDefaultValue();

        panda::PandArg<std::string> pandarg_string("string", str, "Sample string argument");
        panda::PandArg<bool> pandarg_bool("bool", false, "Sample boolean argument");
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::PandargsFuzzTest(data, size);
    return 0;
}