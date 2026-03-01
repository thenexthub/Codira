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

#include "intrinsics.h"
#include <gtest/gtest.h>
#include "ets_coroutine.h"
#include "types/ets_string.h"
#include "types/ets_array.h"
#include "libarkbase/utils/utf.h"

// NOLINTBEGIN(readability-magic-numbers)
// NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)

namespace ark::ets::test {
class EtsStringBuilderTest : public testing::Test {
public:
    EtsStringBuilderTest()
    {
        options_.SetShouldLoadBootPandaFiles(true);
        options_.SetShouldInitializeIntrinsics(false);
        options_.SetCompilerEnableJit(false);
        options_.SetGcType("epsilon");
        options_.SetLoadRuntimes({"ets"});

        auto stdlib = std::getenv("PANDA_STD_LIB");
        if (stdlib == nullptr) {
            std::cerr << "PANDA_STD_LIB env variable should be set and point to mock_stdlib.abc" << std::endl;
            std::abort();
        }
        options_.SetBootPandaFiles({stdlib});

        Runtime::Create(options_);
    }

    ~EtsStringBuilderTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(EtsStringBuilderTest);
    NO_MOVE_SEMANTIC(EtsStringBuilderTest);

    void SetUp() override
    {
        coroutine_ = EtsCoroutine::GetCurrent();
        coroutine_->ManagedCodeBegin();
    }

    void TearDown() override
    {
        coroutine_->ManagedCodeEnd();
    }

private:
    RuntimeOptions options_;
    EtsCoroutine *coroutine_ = nullptr;
};

TEST_F(EtsStringBuilderTest, CharToString)
{
    EtsChar etsCh = L'٩';
    EtsString *charToString = intrinsics::StdCoreToStringChar(etsCh);

    ASSERT_TRUE(charToString->GetLength() == 1);
    ASSERT_TRUE(charToString->GetDataUtf16()[0] == etsCh);
}

}  // namespace ark::ets::test

// NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
// NOLINTEND(readability-magic-numbers)
