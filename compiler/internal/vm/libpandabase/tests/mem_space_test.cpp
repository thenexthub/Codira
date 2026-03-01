/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>

#include "libpandabase/mem/space.h"

namespace panda::test {

HWTEST(MemSpaceTest, ToSpaceType, testing::ext::TestSize.Level0)
{
    const size_t index = 1U;
    ASSERT_EQ(index, static_cast<size_t>(ToSpaceType(index)));
}

HWTEST(MemSpaceTest, IsHeapSpace, testing::ext::TestSize.Level0)
{
    ASSERT_TRUE(IsHeapSpace(SpaceType::SPACE_TYPE_OBJECT));
}

HWTEST(MemSpaceTest, SpaceTypeToString, testing::ext::TestSize.Level0)
{
    EXPECT_STREQ("ark-Undefined Space", SpaceTypeToString(SpaceType::SPACE_TYPE_UNDEFINED));
    EXPECT_STREQ("ark-Object Space", SpaceTypeToString(SpaceType::SPACE_TYPE_OBJECT));
    EXPECT_STREQ("ark-Humongous Object Space", SpaceTypeToString(SpaceType::SPACE_TYPE_HUMONGOUS_OBJECT));
    EXPECT_STREQ("ark-Non Movable Space", SpaceTypeToString(SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT));
    EXPECT_STREQ("ark-Internal Space", SpaceTypeToString(SpaceType::SPACE_TYPE_INTERNAL));
    EXPECT_STREQ("ark-Code Space", SpaceTypeToString(SpaceType::SPACE_TYPE_CODE));
    EXPECT_STREQ("ark-Compiler Space", SpaceTypeToString(SpaceType::SPACE_TYPE_COMPILER));
    EXPECT_STREQ("ark-Unknown Space", SpaceTypeToString(SpaceType::SPACE_TYPE_LAST));
}

}  // namespace panda::test
