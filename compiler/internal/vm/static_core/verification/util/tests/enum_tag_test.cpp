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

#include "util/enum_tag.h"

#include "util/tests/verifier_test.h"

#include <gtest/gtest.h>

namespace {

enum class Numbers { ONE, TWO, THREE, FOUR, FIVE };

}  // namespace

namespace ark::verifier::test {

TEST_F(VerifierTest, EnumTag)
{
    using EnumTag5 = TagForEnum<Numbers, Numbers::ONE, Numbers::TWO, Numbers::THREE, Numbers::FOUR, Numbers::FIVE>;

    ASSERT_EQ(EnumTag5::SIZE, 5UL);
    EXPECT_EQ(EnumTag5::BITS, 3UL);
    EXPECT_EQ(EnumTag5::GetIndexFor(Numbers::TWO), 1UL);
    EXPECT_EQ(EnumTag5::GetValueFor(3UL), Numbers::FOUR);

    using EnumTag3 = TagForEnum<Numbers, Numbers::ONE, Numbers::TWO, Numbers::THREE>;

    ASSERT_EQ(EnumTag3::SIZE, 3UL);
    EXPECT_EQ(EnumTag3::BITS, 2UL);
    EXPECT_EQ(EnumTag3::GetIndexFor(Numbers::TWO), 1UL);
    EXPECT_EQ(EnumTag3::GetValueFor(0UL), Numbers::ONE);
}

}  // namespace ark::verifier::test