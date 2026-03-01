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

class FindSetterTest : public AniTest {
public:
    template <bool METHOD_IS_PRESENT>
    void CheckClassFindSetter(const char *clsName, const char *propertyName)
    {
        ani_class cls;
        ASSERT_EQ(env_->FindClass(clsName, &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);
        ani_method method;
        auto status = env_->Class_FindSetter(cls, propertyName, &method);
        if constexpr (METHOD_IS_PRESENT) {
            ASSERT_EQ(status, ANI_OK);
            ASSERT_NE(method, nullptr);
        } else {
            ASSERT_EQ(status, ANI_NOT_FOUND);
        }
    }
};

class FindGetterTest : public AniTest {
public:
    template <bool METHOD_IS_PRESENT>
    void CheckClassFindGetter(const char *clsName, const char *propertyName)
    {
        ani_class cls;
        ASSERT_EQ(env_->FindClass(clsName, &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);
        ani_method method;
        auto status = env_->Class_FindGetter(cls, propertyName, &method);
        if constexpr (METHOD_IS_PRESENT) {
            ASSERT_EQ(status, ANI_OK);
            ASSERT_NE(method, nullptr);
        } else {
            ASSERT_EQ(status, ANI_NOT_FOUND);
        }
    }
};

class FindGetterSetterTest : public AniTest {};

TEST_F(FindSetterTest, has_set_methods)
{
    CheckClassFindSetter<true>("class_find_setter_getter_test.ExplicitMethods", "age");
    CheckClassFindSetter<true>("class_find_setter_getter_test.StyledRectangle", "color");
    CheckClassFindSetter<true>("class_find_setter_getter_test.OnlySet", "field");
}

TEST_F(FindGetterTest, has_get_method)
{
    CheckClassFindGetter<true>("class_find_setter_getter_test.ExplicitMethods", "age");
    CheckClassFindGetter<true>("class_find_setter_getter_test.StyledRectangle", "color");
    CheckClassFindGetter<true>("class_find_setter_getter_test.OnlyGet", "field");
}

TEST_F(FindSetterTest, no_set_method)
{
    CheckClassFindSetter<false>("class_find_setter_getter_test.ImplicitMethods", "field");
    CheckClassFindSetter<false>("class_find_setter_getter_test.OnlyGet", "field");
}

TEST_F(FindGetterTest, no_get_method)
{
    CheckClassFindGetter<false>("class_find_setter_getter_test.ImplicitMethods", "field");
    CheckClassFindGetter<false>("class_find_setter_getter_test.OnlySet", "field");
}

TEST_F(FindGetterSetterTest, invalid_args)
{
    ani_method method;
    ASSERT_EQ(env_->Class_FindSetter(nullptr, "field", &method), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Class_FindGetter(nullptr, "field", &method), ANI_INVALID_ARGS);
}

TEST_F(FindSetterTest, check_initialization)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_find_setter_getter_test.ExplicitMethods", &cls), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_find_setter_getter_test.ExplicitMethods"));
    ani_method method {};
    ASSERT_EQ(env_->Class_FindSetter(cls, "age", &method), ANI_OK);
    ASSERT_FALSE(IsRuntimeClassInitialized("class_find_setter_getter_test.ExplicitMethods"));
}

TEST_F(FindGetterTest, check_initialization)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_find_setter_getter_test.ExplicitMethods", &cls), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_find_setter_getter_test.ExplicitMethods"));
    ani_method method {};
    ASSERT_EQ(env_->Class_FindGetter(cls, "age", &method), ANI_OK);
    ASSERT_FALSE(IsRuntimeClassInitialized("class_find_setter_getter_test.ExplicitMethods"));
}

}  // namespace ark::ets::ani::testing
