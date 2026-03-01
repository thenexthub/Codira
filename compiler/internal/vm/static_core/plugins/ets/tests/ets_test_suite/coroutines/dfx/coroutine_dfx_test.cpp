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

#include <cstdlib>
#include <gtest/gtest.h>

#include "plugins/ets/tests/ani/ani_gtest/ani_gtest.h"

#include "plugins/ets/runtime/ets_coroutine.h"

#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/types/ets_error.h"

#include "runtime/include/class.h"
#include "runtime/include/object_header.h"
#include "runtime/coroutines/stackful_coroutine_manager.h"

namespace ark::ets::test {

class EtsCoroutineDFXTest : public ani::testing::AniTest {
public:
    static ani_function ResolveFunction(ani_env *env, std::string_view methodName, std::string_view signature)
    {
        ani_module md;
        [[maybe_unused]] auto status = env->FindModule("coroutine_dfx_test", &md);
        ASSERT(status == ANI_OK);
        ani_function func;
        status = env->Module_FindFunction(md, methodName.data(), signature.data(), &func);
        ASSERT(status == ANI_OK);
        return func;
    }

private:
    std::vector<ani_option> GetExtraAniOptions() override
    {
        ani_option gcType = {"--ext:gc-type=epsilon", nullptr};
        ani_option jit = {"--ext:compiler-enable-jit=false", nullptr};
        // Use coroutine-workers-count = 1 to guaranty that after schedule
        ani_option coroutineWorkersCount = {"--ext:coroutine-workers-count=1", nullptr};
        return std::vector<ani_option> {gcType, jit, coroutineWorkersCount};
    }
};

TEST_F(EtsCoroutineDFXTest, PrintStackTest)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    auto etsVm = coroutine->GetPandaVM();
    auto *coroutineManager = static_cast<StackfulCoroutineManager *>(etsVm->GetCoroutineManager());

    CallEtsFunction<ani_int>("coroutine_dfx_test", "startWaitCoro");
    coroutineManager->Schedule();
    auto coroInfo = coroutineManager->GetAllWorkerFullStatus()->OutputInfo();
    ASSERT_TRUE(coroInfo.find("Status: BLOCKED") != coroInfo.npos);
    ASSERT_TRUE(coroInfo.find("coroutine_dfx_test.ETSGLOBAL::waiter") !=
                coroInfo.npos);  // check if it contains stack with wainter()
    CallEtsFunction<ani_int>("coroutine_dfx_test", "notify");
}

}  // namespace ark::ets::test
