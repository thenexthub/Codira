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

#include "gtest/gtest.h"
#include "libarkbase/utils/utils.h"
#include "runtime/include/coretypes/line_string.h"
#include "runtime/include/mem/panda_smart_pointers.h"
#include "runtime/include/runtime.h"

namespace ark::mem::test {

class PandaSmartPointersTest : public testing::Test {
public:
    PandaSmartPointersTest()
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);
        options.SetLimitStandardAlloc(true);
        Runtime::Create(options);
        thread_ = ark::MTManagedThread::GetCurrent();
        thread_->ManagedCodeBegin();
    }

    ~PandaSmartPointersTest() override
    {
        thread_->ManagedCodeEnd();
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(PandaSmartPointersTest);
    NO_MOVE_SEMANTIC(PandaSmartPointersTest);

private:
    ark::MTManagedThread *thread_ {};
};

int ReturnValueFromUniqPtr(PandaUniquePtr<int> ptr)
{
    return *ptr.get();
}

TEST_F(PandaSmartPointersTest, MakePandaUniqueTest)
{
    // Not array type

    static constexpr int POINTER_VALUE = 5;

    auto uniqPtr = MakePandaUnique<int>(POINTER_VALUE);
    ASSERT_NE(uniqPtr.get(), nullptr);

    int res = ReturnValueFromUniqPtr(std::move(uniqPtr));
    ASSERT_EQ(res, 5_I);
    // NOLINTNEXTLINE(clang-analyzer-cplusplus.Move)
    ASSERT_EQ(uniqPtr.get(), nullptr);

    // Unbounded array type

    static constexpr size_t SIZE = 3;

    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    auto uniqPtr2 = MakePandaUnique<int[]>(SIZE);
    ASSERT_NE(uniqPtr2.get(), nullptr);

    for (size_t i = 0; i < SIZE; ++i) {
        uniqPtr2[i] = i;
    }

    auto uniqPtr3 = std::move(uniqPtr2);
    for (size_t i = 0; i < SIZE; ++i) {
        ASSERT_EQ(uniqPtr3[i], i);
    }
    // NOLINTNEXTLINE(clang-analyzer-cplusplus.Move)
    ASSERT_EQ(uniqPtr2.get(), nullptr);
}

}  // namespace ark::mem::test
