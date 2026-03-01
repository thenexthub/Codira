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

#include "ani/ani.h"
#include "ani_gtest.h"

namespace ark::ets::ani::testing {

class FindIndexableGetterTest : public AniTest {};
class FindIndexableSetterTest : public AniTest {};
class FindIndexableSetterGetterTest : public AniTest {};

TEST_F(FindIndexableGetterTest, get_method)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("class_find_indexable_getter_setter_test.A", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method;
    ASSERT_EQ(env_->Class_FindIndexableGetter(cls, "d:C{class_find_indexable_getter_setter_test.A}", &method), ANI_OK);
    ASSERT_NE(method, nullptr);

    ASSERT_EQ(env_->Class_FindIndexableGetter(cls, "d:i", &method), ANI_NOT_FOUND);
}

TEST_F(FindIndexableSetterTest, set_method)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("class_find_indexable_getter_setter_test.A", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method stringSetter;
    ASSERT_EQ(env_->Class_FindIndexableSetter(cls, "dC{std.core.String}:", &stringSetter), ANI_OK);
    ASSERT_NE(stringSetter, nullptr);

    ani_method booleanSetter;
    ASSERT_EQ(env_->Class_FindIndexableSetter(cls, "dz:", &booleanSetter), ANI_OK);
    ASSERT_NE(booleanSetter, nullptr);
    ASSERT_NE(booleanSetter, stringSetter);

    ani_method intSetter;
    ASSERT_EQ(env_->Class_FindIndexableSetter(cls, "ii:", &intSetter), ANI_NOT_FOUND);
}

TEST_F(FindIndexableSetterGetterTest, invalid_args)
{
    ani_method method;
    ASSERT_EQ(env_->Class_FindIndexableSetter(nullptr, "ii:", &method), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Class_FindIndexableGetter(nullptr, "ii:", &method), ANI_INVALID_ARGS);
}

TEST_F(FindIndexableGetterTest, check_initialization)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_find_indexable_getter_setter_test.A", &cls), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_find_indexable_getter_setter_test.A"));
    ani_method method {};
    ASSERT_EQ(env_->Class_FindIndexableGetter(cls, "d:C{class_find_indexable_getter_setter_test.A}", &method), ANI_OK);
    ASSERT_FALSE(IsRuntimeClassInitialized("class_find_indexable_getter_setter_test.A"));
}

TEST_F(FindIndexableSetterTest, check_initialization)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_find_indexable_getter_setter_test.A", &cls), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_find_indexable_getter_setter_test.A"));
    ani_method method {};
    ASSERT_EQ(env_->Class_FindIndexableSetter(cls, "dz:", &method), ANI_OK);
    ASSERT_FALSE(IsRuntimeClassInitialized("class_find_indexable_getter_setter_test.A"));
}

}  // namespace ark::ets::ani::testing
