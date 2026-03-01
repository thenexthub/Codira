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

#include "ani_gtest.h"

namespace ark::ets::ani::testing {

class ObjectGetFieldCharTest : public AniTest {
public:
    void GetTestData(ani_object *objectResult, ani_field *fieldNameResult, ani_field *fieldIndexResult)
    {
        auto sarahRef = CallEtsFunction<ani_ref>("object_get_field_char_test", "newSarahObject");
        auto sarah = static_cast<ani_object>(sarahRef);

        ani_class cls;
        ASSERT_EQ(env_->FindClass("object_get_field_char_test.Woman", &cls), ANI_OK);

        ani_field fieldName;
        ASSERT_EQ(env_->Class_FindField(cls, "name", &fieldName), ANI_OK);

        ani_field fieldIndex;
        ASSERT_EQ(env_->Class_FindField(cls, "index", &fieldIndex), ANI_OK);

        *objectResult = sarah;
        *fieldNameResult = fieldName;
        *fieldIndexResult = fieldIndex;
    }
};

TEST_F(ObjectGetFieldCharTest, get_field_char)
{
    ani_object sarah {};
    ani_field field {};
    ani_field fieldIndex {};
    GetTestData(&sarah, &field, &fieldIndex);

    ani_char index = '\0';
    ani_char xx = 'a';
    ASSERT_EQ(env_->Object_GetField_Char(sarah, fieldIndex, &index), ANI_OK);
    ASSERT_EQ(index, xx);
}

TEST_F(ObjectGetFieldCharTest, get_field_char_invalid_env)
{
    ani_object sarah {};
    ani_field field {};
    ani_field fieldIndex {};
    GetTestData(&sarah, &field, &fieldIndex);

    ani_char index = '\0';
    ASSERT_EQ(env_->c_api->Object_GetField_Char(nullptr, sarah, fieldIndex, &index), ANI_INVALID_ARGS);
}

TEST_F(ObjectGetFieldCharTest, get_field_char_invalid_field_type)
{
    ani_object sarah {};
    ani_field field {};
    ani_field fieldIndex {};
    GetTestData(&sarah, &field, &fieldIndex);

    ani_char index = '\0';
    ASSERT_EQ(env_->Object_GetField_Char(sarah, field, &index), ANI_INVALID_TYPE);
}

TEST_F(ObjectGetFieldCharTest, invalid_argument1)
{
    ani_object sarah {};
    ani_field field {};
    ani_field fieldIndex {};
    GetTestData(&sarah, &field, &fieldIndex);

    ani_char index = '\0';
    ASSERT_EQ(env_->Object_GetField_Char(nullptr, field, &index), ANI_INVALID_ARGS);
}

TEST_F(ObjectGetFieldCharTest, invalid_argument2)
{
    ani_object sarah {};
    ani_field field {};
    ani_field fieldIndex {};
    GetTestData(&sarah, &field, &fieldIndex);

    ani_char index = '\0';
    ASSERT_EQ(env_->Object_GetField_Char(sarah, nullptr, &index), ANI_INVALID_ARGS);
}

TEST_F(ObjectGetFieldCharTest, invalid_argument3)
{
    ani_object sarah {};
    ani_field field {};
    ani_field fieldIndex {};
    GetTestData(&sarah, &field, &fieldIndex);

    ASSERT_EQ(env_->Object_GetField_Char(sarah, field, nullptr), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing
