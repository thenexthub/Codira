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

#include "debuginfodataaccessor_fuzzer.h"

#include "libpandafile/class_data_accessor-inl.h"
#include "libpandafile/debug_data_accessor-inl.h"
#include "libpandafile/method_data_accessor-inl.h"

namespace OHOS {
void DebugInfoDataAccessorFuzzTest(const uint8_t* data, size_t size)
{
    try {
        auto pf = panda::panda_file::OpenPandaFileFromMemory(data, size);
        if (pf == nullptr) {
            return;
        }
        const auto &panda_file = *pf;
        for (const auto &header : panda_file.GetIndexHeaders()) {
            const auto &methods = panda_file.GetMethodIndex(&header);
            for (const auto &id : methods) {
                panda::panda_file::MethodDataAccessor mda(panda_file, id);
                auto value = mda.GetCodeId();
                if (value.has_value()) {
                    panda::panda_file::DebugInfoDataAccessor dda(panda_file, value.value());
                }
            }
        }
    } catch (panda::panda_file::helpers::FileAccessException &e) {
        // Known exception, no need exposing
    }
}
}  // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DebugInfoDataAccessorFuzzTest(data, size);
    return 0;
}
