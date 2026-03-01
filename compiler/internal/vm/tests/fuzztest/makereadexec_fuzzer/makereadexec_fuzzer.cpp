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

#include "makereadexec_fuzzer.h"
#include "libpandabase/mem/base_mem_stats.h"
#include "libpandabase/mem/code_allocator.h"
#include "libpandabase/mem/pool_manager.h"

namespace OHOS {
    void MakeReadExecFuzzTest(const uint8_t* data, size_t size)
    {
        {
            // T* is nullptr
            panda::os::mem::MapRange<uint8_t> tmp(nullptr, 0);
            tmp.MakeReadExec();
        }

        {
            // T* is not nullptr
            constexpr size_t MB = 1024 * 1024;
            panda::mem::MemConfig::Initialize(0, 32 * MB, 0, 32 * MB); // internal_size and code_size are set to 32MB
            panda::PoolManager::Initialize();
            {
                panda::BaseMemStats stats;
                panda::CodeAllocator ca(&stats);
                uint8_t* buff = const_cast<uint8_t*>(data);
                ca.AllocateCode(size, static_cast<void*>(buff));
                panda::os::mem::MapRange<std::byte> map_range = ca.AllocateCodeUnprotected(8U);
                ca.ProtectCode(map_range);
            }
            panda::PoolManager::Finalize();
            panda::mem::MemConfig::Finalize();
        }
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::MakeReadExecFuzzTest(data, size);
    return 0;
}