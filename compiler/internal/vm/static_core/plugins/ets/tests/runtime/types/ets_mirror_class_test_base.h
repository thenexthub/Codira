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

#ifndef PANDA_PLUGINS_ETS_TESTS_RUNTIME_TYPES_ETS_MIRROR_CLASS_TEST_BASE_H
#define PANDA_PLUGINS_ETS_TESTS_RUNTIME_TYPES_ETS_MIRROR_CLASS_TEST_BASE_H

#include <gtest/gtest.h>

#include "ets_coroutine.h"
#include "ets_platform_types.h"
#include "ets_vm.h"

namespace ark::ets::test {

class EtsMirrorClassTestBase : public testing::Test {
public:
    EtsMirrorClassTestBase()
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
            UNREACHABLE();
        }
        options.SetBootPandaFiles({stdlib});

        Runtime::Create(options);
    }

    ~EtsMirrorClassTestBase() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(EtsMirrorClassTestBase);
    NO_MOVE_SEMANTIC(EtsMirrorClassTestBase);

    void SetUp() override
    {
        coroutine_ = EtsCoroutine::GetCurrent();
        vm_ = coroutine_->GetPandaVM();
        coroutine_->ManagedCodeBegin();
    }

    void TearDown() override
    {
        coroutine_->ManagedCodeEnd();
    }

    EtsClassLinker *GetClassLinker()
    {
        return vm_->GetClassLinker();
    }

    const EtsPlatformTypes *GetPlatformTypes()
    {
        return PlatformTypes(vm_);
    }

private:
    PandaEtsVM *vm_ = nullptr;  // NOLINT(misc-non-private-member-variables-in-classes)
    EtsCoroutine *coroutine_ = nullptr;
};

}  // namespace ark::ets::test

#endif  // PANDA_PLUGINS_ETS_TESTS_RUNTIME_TYPES_ETS_MIRROR_CLASS_TEST_BASE_H
