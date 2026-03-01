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

class EnumItemGetValueTest : public AniTest {};

TEST_F(EnumItemGetValueTest, enum_item_get_value_int)
{
    ani_enum aniEnum {};
    ASSERT_EQ(env_->FindEnum("enum_item_get_value_test.Color", &aniEnum), ANI_OK);
    ASSERT_NE(aniEnum, nullptr);
    ani_enum_item red {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnum, "RED", &red), ANI_OK);
    ani_enum_item green {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnum, "GREEN", &green), ANI_OK);
    ani_enum_item blue {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnum, "BLUE", &blue), ANI_OK);
    ani_int redValInt {};
    ASSERT_EQ(env_->EnumItem_GetValue_Int(red, &redValInt), ANI_OK);
    ASSERT_EQ(redValInt, 0U);
    ani_int greenValInt {};
    ASSERT_EQ(env_->EnumItem_GetValue_Int(green, &greenValInt), ANI_OK);
    ASSERT_EQ(greenValInt, 1U);
    ani_int blueValInt {};
    ASSERT_EQ(env_->EnumItem_GetValue_Int(blue, &blueValInt), ANI_OK);
    ASSERT_EQ(blueValInt, 2U);

    ani_enum aniEnumInt {};
    ASSERT_EQ(env_->FindEnum("enum_item_get_value_test.ColorInt", &aniEnumInt), ANI_OK);
    ASSERT_NE(aniEnumInt, nullptr);
    ani_enum_item redInt {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnumInt, "REDINT", &redInt), ANI_OK);
    ani_enum_item greenInt {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnumInt, "GREENINT", &greenInt), ANI_OK);
    ani_enum_item blueInt {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnumInt, "BLUEINT", &blueInt), ANI_OK);
    ani_int redIntValInt {};
    ASSERT_EQ(env_->EnumItem_GetValue_Int(redInt, &redIntValInt), ANI_OK);
    ASSERT_EQ(redIntValInt, 5U);
    ani_int greenIntValInt {};
    ASSERT_EQ(env_->EnumItem_GetValue_Int(greenInt, &greenIntValInt), ANI_OK);
    ASSERT_EQ(greenIntValInt, 77U);
    ani_int blueIntValInt {};
    ASSERT_EQ(env_->EnumItem_GetValue_Int(blueInt, &blueIntValInt), ANI_OK);
    ASSERT_EQ(blueIntValInt, 9999U);
}

TEST_F(EnumItemGetValueTest, wrong_enum_item_get_value_int)
{
    ani_enum aniEnumStr {};
    ASSERT_EQ(env_->FindEnum("enum_item_get_value_test.ColorString", &aniEnumStr), ANI_OK);
    ASSERT_NE(aniEnumStr, nullptr);
    ani_enum_item redStr {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnumStr, "REDSTR", &redStr), ANI_OK);
    ani_enum_item greenStr {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnumStr, "GREENSTR", &greenStr), ANI_OK);
    ani_enum_item blueStr {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnumStr, "BLUESTR", &blueStr), ANI_OK);
    ani_int redStrValInt {};
    ASSERT_EQ(env_->EnumItem_GetValue_Int(redStr, &redStrValInt), ANI_INVALID_ARGS);
    ani_int greenStrValInt {};
    ASSERT_EQ(env_->EnumItem_GetValue_Int(greenStr, &greenStrValInt), ANI_INVALID_ARGS);
    ani_int blueStrValInt {};
    ASSERT_EQ(env_->EnumItem_GetValue_Int(blueStr, &blueStrValInt), ANI_INVALID_ARGS);
}

TEST_F(EnumItemGetValueTest, enum_item_get_value_string_1)
{
    std::string enumVal;

    ani_enum aniEnum {};
    ASSERT_EQ(env_->FindEnum("enum_item_get_value_test.Color", &aniEnum), ANI_OK);
    ASSERT_NE(aniEnum, nullptr);
    ani_enum_item red {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnum, "RED", &red), ANI_OK);
    ani_enum_item green {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnum, "GREEN", &green), ANI_OK);
    ani_enum_item blue {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnum, "BLUE", &blue), ANI_OK);
    ani_string redValStr {};
    ASSERT_EQ(env_->EnumItem_GetValue_String(red, &redValStr), ANI_OK);
    GetStdString(redValStr, enumVal);
    ASSERT_STREQ(enumVal.data(), "0");
    ani_string greenValStr {};
    ASSERT_EQ(env_->EnumItem_GetValue_String(green, &greenValStr), ANI_OK);
    GetStdString(greenValStr, enumVal);
    ASSERT_STREQ(enumVal.data(), "1");
    ani_string blueValStr {};
    ASSERT_EQ(env_->EnumItem_GetValue_String(blue, &blueValStr), ANI_OK);
    GetStdString(blueValStr, enumVal);
    ASSERT_STREQ(enumVal.data(), "2");

    ani_enum aniEnumInt {};
    ASSERT_EQ(env_->FindEnum("enum_item_get_value_test.ColorInt", &aniEnumInt), ANI_OK);
    ASSERT_NE(aniEnumInt, nullptr);
    ani_enum_item redInt {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnumInt, "REDINT", &redInt), ANI_OK);
    ani_enum_item greenInt {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnumInt, "GREENINT", &greenInt), ANI_OK);
    ani_enum_item blueInt {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnumInt, "BLUEINT", &blueInt), ANI_OK);
    ani_string redIntValStr {};
    ASSERT_EQ(env_->EnumItem_GetValue_String(redInt, &redIntValStr), ANI_OK);
    GetStdString(redIntValStr, enumVal);
    ASSERT_STREQ(enumVal.data(), "5");
    ani_string greenIntValStr {};
    ASSERT_EQ(env_->EnumItem_GetValue_String(greenInt, &greenIntValStr), ANI_OK);
    GetStdString(greenIntValStr, enumVal);
    ASSERT_STREQ(enumVal.data(), "77");
    ani_string blueIntValStr {};
    ASSERT_EQ(env_->EnumItem_GetValue_String(blueInt, &blueIntValStr), ANI_OK);
    GetStdString(blueIntValStr, enumVal);
    ASSERT_STREQ(enumVal.data(), "9999");
}

TEST_F(EnumItemGetValueTest, enum_item_get_value_string_2)
{
    std::string enumVal;

    ani_enum aniEnumStr {};
    ASSERT_EQ(env_->FindEnum("enum_item_get_value_test.ColorString", &aniEnumStr), ANI_OK);
    ASSERT_NE(aniEnumStr, nullptr);
    ani_enum_item redStr {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnumStr, "REDSTR", &redStr), ANI_OK);
    ani_enum_item greenStr {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnumStr, "GREENSTR", &greenStr), ANI_OK);
    ani_enum_item blueStr {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnumStr, "BLUESTR", &blueStr), ANI_OK);
    ani_string redStrValStr {};
    ASSERT_EQ(env_->EnumItem_GetValue_String(redStr, &redStrValStr), ANI_OK);
    GetStdString(redStrValStr, enumVal);
    ASSERT_STREQ(enumVal.data(), "str_red");
    ani_string greenStrValStr {};
    ASSERT_EQ(env_->EnumItem_GetValue_String(greenStr, &greenStrValStr), ANI_OK);
    GetStdString(greenStrValStr, enumVal);
    ASSERT_STREQ(enumVal.data(), "str_green");
    ani_string blueStrValStr {};
    ASSERT_EQ(env_->EnumItem_GetValue_String(blueStr, &blueStrValStr), ANI_OK);
    GetStdString(blueStrValStr, enumVal);
    ASSERT_STREQ(enumVal.data(), "str_blue");
}

TEST_F(EnumItemGetValueTest, invalid_arg_enum)
{
    ani_enum aniEnum {};
    ASSERT_EQ(env_->FindEnum("enum_item_get_value_test.Color", &aniEnum), ANI_OK);
    ASSERT_NE(aniEnum, nullptr);
    ani_enum_item red {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnum, "RED", &red), ANI_OK);

    ani_int redInt {};
    ASSERT_EQ(env_->EnumItem_GetValue_Int(nullptr, &redInt), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->EnumItem_GetValue_Int(red, nullptr), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->c_api->EnumItem_GetValue_Int(nullptr, red, &redInt), ANI_INVALID_ARGS);

    ani_string redString {};
    ASSERT_EQ(env_->EnumItem_GetValue_String(nullptr, &redString), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->EnumItem_GetValue_String(red, nullptr), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->c_api->EnumItem_GetValue_String(nullptr, red, &redString), ANI_INVALID_ARGS);
}

TEST_F(EnumItemGetValueTest, enum_get_value_test_one_item)
{
    ani_enum aniEnum {};
    ASSERT_EQ(env_->FindEnum("enum_item_get_value_test.OneItem", &aniEnum), ANI_OK);
    ASSERT_NE(aniEnum, nullptr);

    ani_enum_item one {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnum, "ONE", &one), ANI_OK);

    ani_int fromone = 5;
    ASSERT_EQ(env_->EnumItem_GetValue_Int(one, &fromone), ANI_OK);
}

}  // namespace ark::ets::ani::testing
