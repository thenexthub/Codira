/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "util/int_tag.h"

#include "util/tests/verifier_test.h"

#include <gtest/gtest.h>

namespace ark::verifier::test {

TEST_F(VerifierTest, IntTag)
{
    // NOLINTNEXTLINE(readability-magic-numbers)
    using IntTag1 = TagForInt<int, 3U, 9U>;

    ASSERT_EQ(IntTag1::SIZE, 7UL);
    EXPECT_EQ(IntTag1::BITS, 3UL);
    EXPECT_EQ(IntTag1::GetIndexFor(4UL), 1UL);
    EXPECT_EQ(IntTag1::GetValueFor(3UL), 6UL);

    using IntTag2 = TagForInt<int, 5U, 5U>;

    ASSERT_EQ(IntTag2::SIZE, 1UL);
    EXPECT_EQ(IntTag2::BITS, 1UL);
    EXPECT_EQ(IntTag2::GetIndexFor(5UL), 0UL);
    EXPECT_EQ(IntTag2::GetValueFor(0UL), 5UL);
}

}  // namespace ark::verifier::test