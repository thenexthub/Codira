/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cstdlib>
#include <ani.h>
#include <gtest/gtest.h>

#include "plugins/ets/tests/ani/ani_gtest/ani_gtest.h"

namespace ark::ets::test {

class EtsVMOOMPreallocTest : public ani::testing::AniTest {
private:
    std::vector<ani_option> GetExtraAniOptions() override
    {
        ani_option gcType = {"--ext:gc-type=epsilon-g1", nullptr};
        ani_option gcTriggerType = {"--ext:gc-trigger-type=heap-trigger", nullptr};
        ani_option jit = {"--ext:compiler-enable-jit=false", nullptr};
        return std::vector<ani_option> {gcType, gcTriggerType, jit};
    }
};

TEST_F(EtsVMOOMPreallocTest, ThrowPreallocatedOOMObjectInErrorCtorTest1)
{
    auto res = CallEtsFunction<ani_boolean>("PreallocTest", "test", 1000);
    ASSERT_EQ(res, true);
}

TEST_F(EtsVMOOMPreallocTest, ThrowPreallocatedOOMObjectInErrorCtorTest2)
{
    auto res = CallEtsFunction<ani_boolean>("PreallocTest", "test", 650);
    ASSERT_EQ(res, true);
}

class EtsVMOOMPreallocJittedTest : public ani::testing::AniTest {
private:
    std::vector<ani_option> GetExtraAniOptions() override
    {
        ani_option gcType = {"--ext:gc-type=epsilon-g1", nullptr};
        ani_option gcTriggerType = {"--ext:gc-trigger-type=heap-trigger", nullptr};
        ani_option jit = {"--ext:compiler-enable-jit=true", nullptr};
        ani_option compilerThreshold = {"--ext:compiler-hotness-threshold=0", nullptr};
        return std::vector<ani_option> {gcType, gcTriggerType, jit, compilerThreshold};
    }
};

TEST_F(EtsVMOOMPreallocJittedTest, ThrowPreallocatedOOMObjectInErrorCtorTest1)
{
    auto res = CallEtsFunction<ani_boolean>("PreallocTest", "test", 1000);
    ASSERT_EQ(res, true);
}

TEST_F(EtsVMOOMPreallocJittedTest, ThrowPreallocatedOOMObjectInErrorCtorTest2)
{
    auto res = CallEtsFunction<ani_boolean>("PreallocTest", "test", 650);
    ASSERT_EQ(res, true);
}

}  // namespace ark::ets::test
