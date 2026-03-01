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

#include "types/ets_error.h"
#include "plugins/ets/tests/runtime/types/ets_test_mirror_classes.h"

namespace ark::ets::test {

class EtsErrorTest : public testing::Test {
public:
    EtsErrorTest()
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

    ~EtsErrorTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(EtsErrorTest);
    NO_MOVE_SEMANTIC(EtsErrorTest);

    static std::vector<MirrorFieldInfo> GetErrorMembers()
    {
        return std::vector<MirrorFieldInfo> {
            MIRROR_FIELD_INFO(Error, name_, "name_"),
            MIRROR_FIELD_INFO(Error, message_, "message_"),
            MIRROR_FIELD_INFO(Error, stackLines_, "stackLines"),
            MIRROR_FIELD_INFO(Error, stack_, "stack_"),
            MIRROR_FIELD_INFO(Error, cause_, "cause_"),
            MIRROR_FIELD_INFO(Error, code_, "code_"),
        };
    }

protected:
    PandaEtsVM *vm_ = nullptr;  // NOLINT(misc-non-private-member-variables-in-classes)
};

// Check both EtsErrorTest and ark::Class<escompat.Error> has the same number of fields
// and at the same offsets
TEST_F(EtsErrorTest, EscompatErrorMemoryLayout)
{
    EtsClass *errorClass = PlatformTypes(vm_)->escompatError;
    MirrorFieldInfo::CompareMemberOffsets(errorClass, GetErrorMembers());
}

}  // namespace ark::ets::test
