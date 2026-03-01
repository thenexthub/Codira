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

#include "mapptrmoveconstructor_fuzzer.h"
#include "libpandabase/os/mem.h"

namespace OHOS {
    void MapPtrMoveConstructorFuzzTest(const uint8_t* data, size_t size)
    {
        uint8_t* ptr = const_cast<uint8_t*>(data);
        panda::os::mem::MapPtr<uint8_t, panda::os::mem::MapPtrType::CONST> byte_ptr1(ptr, size, nullptr);
        panda::os::mem::MapPtr<uint8_t, panda::os::mem::MapPtrType::CONST> byte_ptr2(std::move(byte_ptr1));
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::MapPtrMoveConstructorFuzzTest(data, size);
    return 0;
}