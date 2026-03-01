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

namespace ark::ets::test {

class EtsVMInitPreallocTest : public ani::testing::AniTest {
private:
    std::vector<ani_option> GetExtraAniOptions() override
    {
        // Can't use epsilon GC, because epsilon gc use slots for allocate.
        // It is hard to fill all possible slots, more easier is to fill regions in g1-gc allocator.
        ani_option gcType = {"--ext:gc-type=g1-gc", nullptr};
        ani_option gcTriggerType = {"--ext:gc-trigger-type=debug-never", nullptr};
        ani_option jit = {"--ext:compiler-enable-jit=false", nullptr};
        return std::vector<ani_option> {ani_option {heapSz_.c_str(), nullptr}, gcType, gcTriggerType, jit};
    }
    // NOLINTNEXTLINE(readability-magic-numbers)
    std::string heapSz_ = std::string("--ext:heap-size-limit=") + std::to_string(32_MB);
};

TEST_F(EtsVMInitPreallocTest, PreallocatedOOMObjectTest)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    auto etsVm = coroutine->GetPandaVM();
    auto oomObj = etsVm->GetOOMErrorObject();

    ASSERT_NE(oomObj, nullptr);

    auto oomClass = oomObj->ClassAddr<ark::Class>();
    ASSERT_NE(oomClass->GetDescriptor(), nullptr);

    auto ctx = coroutine->GetLanguageContext();
    auto descriptorGot = utf::Mutf8AsCString(oomClass->GetDescriptor());
    auto descriptorExp = utf::Mutf8AsCString(ctx.GetOutOfMemoryErrorClassDescriptor());
    ASSERT_STREQ(descriptorGot, descriptorExp);
}

TEST_F(EtsVMInitPreallocTest, ThrowPreallocatedOOMObjectTest)
{
    ani_string oomName;
    ani_string oomMsg;
    ani_string oomStack;

    ASSERT_EQ(env_->String_NewUTF8(EtsOutOfMemoryError::OOM_ERROR_NAME.data(),
                                   EtsOutOfMemoryError::OOM_ERROR_NAME.size(), &oomName),
              ANI_OK);
    ASSERT_EQ(env_->String_NewUTF8(EtsOutOfMemoryError::DEFAULT_OOM_MSG.data(),
                                   EtsOutOfMemoryError::DEFAULT_OOM_MSG.size(), &oomMsg),
              ANI_OK);
    ASSERT_EQ(env_->String_NewUTF8(EtsOutOfMemoryError::DEFAULT_OOM_STACK.data(),
                                   EtsOutOfMemoryError::DEFAULT_OOM_STACK.size(), &oomStack),
              ANI_OK);

    auto res = CallEtsFunction<ani_boolean>("PreallocTest", "testThrowingPreallocOOMObject", oomName, oomMsg, oomStack);
    ASSERT_EQ(res, true);
}

// 26982 64-bit pointer support
#if !defined(PANDA_TARGET_ARM64) || defined(PANDA_32_BIT_MANAGED_POINTER)
TEST_F(EtsVMInitPreallocTest, ThrowPreallocatedOOMObjectTwiceTest)
{
    ani_string oomName;
    ani_string oomMsg;
    ani_string oomStack;

    ASSERT_EQ(env_->String_NewUTF8(EtsOutOfMemoryError::OOM_ERROR_NAME.data(),
                                   EtsOutOfMemoryError::OOM_ERROR_NAME.size(), &oomName),
              ANI_OK);
    ASSERT_EQ(env_->String_NewUTF8(EtsOutOfMemoryError::DEFAULT_OOM_MSG.data(),
                                   EtsOutOfMemoryError::DEFAULT_OOM_MSG.size(), &oomMsg),
              ANI_OK);
    ASSERT_EQ(env_->String_NewUTF8(EtsOutOfMemoryError::DEFAULT_OOM_STACK.data(),
                                   EtsOutOfMemoryError::DEFAULT_OOM_STACK.size(), &oomStack),
              ANI_OK);

    auto res =
        CallEtsFunction<ani_boolean>("PreallocTest", "testThrowingPreallocOOMObjectTwice", oomName, oomMsg, oomStack);
    ASSERT_EQ(res, true);
}
#endif

}  // namespace ark::ets::test
