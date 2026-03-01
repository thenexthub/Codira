/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"
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

#include <limits>
#include "ani.h"
#include "array_gtest_helper.h"
#include "libarkbase/macros.h"

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
namespace ark::ets::ani::testing {

class ArrayGetLengthTest : public ArrayHelperTest {};

TEST_F(ArrayGetLengthTest, GetLengthTest)
{
    InitTest();

    ani_ref undefined {};
    ASSERT_EQ(env_->GetUndefined(&undefined), ANI_OK);
    ani_array array {};
    ASSERT_EQ(env_->Array_New(1U, undefined, &array), ANI_OK);
    ani_size sz {};
    ASSERT_EQ(env_->Array_GetLength(array, &sz), ANI_OK);
    ASSERT_EQ(sz, 1U);

    ani_object boxedBoolean {};
    ASSERT_EQ(env_->Object_New(booleanClass, booleanCtor, &boxedBoolean, ANI_TRUE), ANI_OK);
    ani_array arrayBoolean {};
    ASSERT_EQ(env_->Array_New(5U, boxedBoolean, &arrayBoolean), ANI_OK);
    ASSERT_EQ(env_->Array_GetLength(arrayBoolean, &sz), ANI_OK);
    ASSERT_EQ(sz, 5U);

    ani_object boxedInt {};
    // NOLINTNEXTLINE(readability-magic-numbers)
    ASSERT_EQ(env_->Object_New(intClass, intCtor, &boxedInt, 10U), ANI_OK);
    ani_array arrayInt {};
    ASSERT_EQ(env_->Array_New(42U, boxedInt, &arrayInt), ANI_OK);
    ASSERT_EQ(env_->Array_GetLength(arrayInt, &sz), ANI_OK);
    ASSERT_EQ(sz, 42U);
}

TEST_F(ArrayGetLengthTest, GetLengthErrorTest)
{
    InitTest();

    ani_size szOverflow = std::numeric_limits<uint32_t>::max();
    // NOLINTNEXTLINE(readability-magic-numbers)
    szOverflow += 10U;

    ani_array array {};
#ifndef PANDA_TARGET_ARM32
    ASSERT_EQ(env_->Array_New(szOverflow, nullptr, &array), ANI_INVALID_ARGS);
#endif
    ASSERT_EQ(env_->Array_New(szOverflow, nullptr, nullptr), ANI_INVALID_ARGS);

    ani_size sz {};
    ASSERT_EQ(env_->Array_GetLength(nullptr, &sz), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Array_New(5U, nullptr, &array), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Array_GetLength(array, nullptr), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
