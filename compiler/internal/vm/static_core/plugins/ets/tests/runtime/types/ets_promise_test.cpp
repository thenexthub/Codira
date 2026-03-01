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

#include <gtest/gtest.h>

#include "ets_coroutine.h"

#include "types/ets_class.h"
#include "types/ets_promise.h"
#include "types/ets_promise_ref.h"
#include "plugins/ets/tests/runtime/types/ets_test_mirror_classes.h"

namespace ark::ets::test {

class EtsPromiseTest : public testing::Test {
public:
    EtsPromiseTest()
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(true);
        options.SetShouldInitializeIntrinsics(true);
        options.SetCompilerEnableJit(false);
        options.SetGcType("epsilon");
        options.SetLoadRuntimes({"ets"});

        auto stdlib = std::getenv("PANDA_STD_LIB");
        if (stdlib == nullptr) {
            std::cerr << "PANDA_STD_LIB env variable should be set and point to mock_stdlib.abc" << std::endl;
            std::abort();
        }
        options.SetBootPandaFiles({stdlib});

        Runtime::Create(options);
        EtsCoroutine *coroutine = EtsCoroutine::GetCurrent();
        vm_ = coroutine->GetPandaVM();
    }

    ~EtsPromiseTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(EtsPromiseTest);
    NO_MOVE_SEMANTIC(EtsPromiseTest);

    static std::vector<MirrorFieldInfo> GetPromiseMembers()
    {
        return std::vector<MirrorFieldInfo> {
            MIRROR_FIELD_INFO(EtsPromise, value_, "value"),
            MIRROR_FIELD_INFO(EtsPromise, mutex_, "mutex"),
            MIRROR_FIELD_INFO(EtsPromise, event_, "event"),
            MIRROR_FIELD_INFO(EtsPromise, callbackQueue_, "callbackQueue"),
            MIRROR_FIELD_INFO(EtsPromise, workerDomainQueue_, "workerDomainQueue"),
            MIRROR_FIELD_INFO(EtsPromise, interopObject_, "interopObject"),
            MIRROR_FIELD_INFO(EtsPromise, linkedPromise_, "linkedPromise"),
            MIRROR_FIELD_INFO(EtsPromise, queueSize_, "queueSize"),
            MIRROR_FIELD_INFO(EtsPromise, state_, "state"),
        };
    }

    static std::vector<MirrorFieldInfo> GetPromiseRefMembers()
    {
        return std::vector<MirrorFieldInfo> {MIRROR_FIELD_INFO(EtsPromiseRef, target_, "target")};
    }

protected:
    PandaEtsVM *vm_ = nullptr;  // NOLINT(misc-non-private-member-variables-in-classes)
};

// Check both EtsPromise and ark::Class<Promise> has the same number of fields
// and at the same offsets
TEST_F(EtsPromiseTest, PromiseMemoryLayout)
{
    EtsClass *promiseClass = PlatformTypes(vm_)->corePromise;
    MirrorFieldInfo::CompareMemberOffsets(promiseClass, GetPromiseMembers());
}

TEST_F(EtsPromiseTest, PromiseRefMemoryLayout)
{
    EtsClass *promiseRefClass = PlatformTypes(vm_)->corePromiseRef;
    MirrorFieldInfo::CompareMemberOffsets(promiseRefClass, GetPromiseRefMembers());
}
}  // namespace ark::ets::test
