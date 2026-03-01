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

#include "ani_gtest.h"

namespace ark::ets::ani::testing {

class GetEnumFromItemTest : public AniTest {};

TEST_F(GetEnumFromItemTest, get_enum_from_item)
{
    ani_enum aniEnum {};
    ASSERT_EQ(env_->FindEnum("enum_item_get_enum_test.Color", &aniEnum), ANI_OK);
    ASSERT_NE(aniEnum, nullptr);

    ani_enum_item red {};
    ASSERT_EQ(env_->Enum_GetEnumItemByIndex(aniEnum, 0U, &red), ANI_OK);

    ani_enum_item green {};
    ASSERT_EQ(env_->Enum_GetEnumItemByIndex(aniEnum, 1U, &green), ANI_OK);

    ani_enum_item blue {};
    ASSERT_EQ(env_->Enum_GetEnumItemByIndex(aniEnum, 2U, &blue), ANI_OK);

    ani_enum fromRed {};
    ASSERT_EQ(env_->EnumItem_GetEnum(red, &fromRed), ANI_OK);

    ani_enum fromGreen {};
    ASSERT_EQ(env_->EnumItem_GetEnum(red, &fromGreen), ANI_OK);

    ani_enum fromBlue {};
    ASSERT_EQ(env_->EnumItem_GetEnum(red, &fromBlue), ANI_OK);

    ani_boolean isRedEqual = ANI_FALSE;
    ASSERT_EQ(env_->Reference_Equals(aniEnum, fromRed, &isRedEqual), ANI_OK);
    ASSERT_EQ(isRedEqual, ANI_TRUE);

    ani_boolean isGreenEqual = ANI_FALSE;
    ASSERT_EQ(env_->Reference_Equals(aniEnum, fromGreen, &isGreenEqual), ANI_OK);
    ASSERT_EQ(isGreenEqual, ANI_TRUE);

    ani_boolean isBlueEqual = ANI_FALSE;
    ASSERT_EQ(env_->Reference_Equals(aniEnum, fromBlue, &isBlueEqual), ANI_OK);
    ASSERT_EQ(isBlueEqual, ANI_TRUE);
}

TEST_F(GetEnumFromItemTest, invalid_arg_enum)
{
    ani_enum aniEnum {};
    ASSERT_EQ(env_->FindEnum("enum_item_get_enum_test.Color", &aniEnum), ANI_OK);
    ASSERT_NE(aniEnum, nullptr);

    ani_enum_item red {};
    ASSERT_EQ(env_->Enum_GetEnumItemByIndex(aniEnum, 0U, &red), ANI_OK);

    ani_enum fromRed {};
    ASSERT_EQ(env_->EnumItem_GetEnum(nullptr, &fromRed), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->EnumItem_GetEnum(red, nullptr), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->c_api->EnumItem_GetEnum(nullptr, red, &fromRed), ANI_INVALID_ARGS);
}

TEST_F(GetEnumFromItemTest, enum_get_value_test_one_item)
{
    ani_enum aniEnum {};
    ASSERT_EQ(env_->FindEnum("enum_item_get_enum_test.OneItem", &aniEnum), ANI_OK);
    ASSERT_NE(aniEnum, nullptr);
    ani_enum_item one {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnum, "ONE", &one), ANI_OK);
    ani_enum fromone {};
    ASSERT_EQ(env_->EnumItem_GetEnum(one, &fromone), ANI_OK);
}

TEST_F(GetEnumFromItemTest, enum_item_combination_test_1)
{
    ani_enum aniEnumString {};
    ASSERT_EQ(env_->FindEnum("enum_item_get_enum_test.ColorString", &aniEnumString), ANI_OK);
    ASSERT_NE(aniEnumString, nullptr);
    ani_enum_item redString {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnumString, "REDSTR", &redString), ANI_OK);

    ani_size redStrIndex = 5U;
    ASSERT_EQ(env_->EnumItem_GetIndex(redString, &redStrIndex), ANI_OK);
    ASSERT_EQ(redStrIndex, 0U);

    ani_string redStrName {};
    std::string itemName {};
    ASSERT_EQ(env_->EnumItem_GetName(redString, &redStrName), ANI_OK);
    GetStdString(redStrName, itemName);
    ASSERT_STREQ(itemName.data(), "REDSTR");

    ani_string redStrValStr {};
    std::string enumVal {};
    ASSERT_EQ(env_->EnumItem_GetValue_String(redString, &redStrValStr), ANI_OK);
    GetStdString(redStrValStr, enumVal);
    ASSERT_STREQ(enumVal.data(), "str_red");
}

TEST_F(GetEnumFromItemTest, get_enum_from_object)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("enum_item_get_enum_test.A", &cls), ANI_OK);
    ani_method ctor {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", nullptr, &ctor), ANI_OK);
    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, ctor, &obj), ANI_OK);  // NOLINT(cppcoreguidelines-pro-type-vararg)
    ani_enum enm {};
    ASSERT_EQ(env_->EnumItem_GetEnum(static_cast<ani_enum_item>(obj), &enm), ANI_INVALID_TYPE);
}
}  // namespace ark::ets::ani::testing
