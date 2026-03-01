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

class EnumItemGetNameTest : public AniTest {};

TEST_F(EnumItemGetNameTest, get_enum_item_name_1)
{
    std::string itemName;

    ani_enum aniEnum {};
    ASSERT_EQ(env_->FindEnum("enum_item_get_name_test.Color", &aniEnum), ANI_OK);
    ASSERT_NE(aniEnum, nullptr);
    ani_enum_item red {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnum, "RED", &red), ANI_OK);
    ani_enum_item green {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnum, "GREEN", &green), ANI_OK);
    ani_enum_item blue {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnum, "BLUE", &blue), ANI_OK);

    ani_string redName {};
    ASSERT_EQ(env_->EnumItem_GetName(red, &redName), ANI_OK);
    GetStdString(redName, itemName);
    ASSERT_STREQ(itemName.data(), "RED");
    ani_string greenName {};
    ASSERT_EQ(env_->EnumItem_GetName(green, &greenName), ANI_OK);
    GetStdString(greenName, itemName);
    ASSERT_STREQ(itemName.data(), "GREEN");
    ani_string blueName {};
    ASSERT_EQ(env_->EnumItem_GetName(blue, &blueName), ANI_OK);
    GetStdString(blueName, itemName);
    ASSERT_STREQ(itemName.data(), "BLUE");

    ani_enum aniEnumInt {};
    ASSERT_EQ(env_->FindEnum("enum_item_get_name_test.ColorInt", &aniEnumInt), ANI_OK);
    ASSERT_NE(aniEnumInt, nullptr);
    ani_enum_item redInt {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnumInt, "REDINT", &redInt), ANI_OK);
    ani_enum_item greenInt {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnumInt, "GREENINT", &greenInt), ANI_OK);
    ani_enum_item blueInt {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnumInt, "BLUEINT", &blueInt), ANI_OK);

    ani_string redIntName {};
    ASSERT_EQ(env_->EnumItem_GetName(redInt, &redIntName), ANI_OK);
    GetStdString(redIntName, itemName);
    ASSERT_STREQ(itemName.data(), "REDINT");
    ani_string greenIntName {};
    ASSERT_EQ(env_->EnumItem_GetName(greenInt, &greenIntName), ANI_OK);
    GetStdString(greenIntName, itemName);
    ASSERT_STREQ(itemName.data(), "GREENINT");
    ani_string blueIntName {};
    ASSERT_EQ(env_->EnumItem_GetName(blueInt, &blueIntName), ANI_OK);
    GetStdString(blueIntName, itemName);
    ASSERT_STREQ(itemName.data(), "BLUEINT");
}

TEST_F(EnumItemGetNameTest, get_enum_item_name_2)
{
    std::string itemName;

    ani_enum aniEnumStr {};
    ASSERT_EQ(env_->FindEnum("enum_item_get_name_test.ColorString", &aniEnumStr), ANI_OK);
    ASSERT_NE(aniEnumStr, nullptr);
    ani_enum_item redStr {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnumStr, "REDSTR", &redStr), ANI_OK);
    ani_enum_item greenStr {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnumStr, "GREENSTR", &greenStr), ANI_OK);
    ani_enum_item blueStr {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnumStr, "BLUESTR", &blueStr), ANI_OK);

    ani_string redStrName {};
    ASSERT_EQ(env_->EnumItem_GetName(redStr, &redStrName), ANI_OK);
    GetStdString(redStrName, itemName);
    ASSERT_STREQ(itemName.data(), "REDSTR");
    ani_string greenStrName {};
    ASSERT_EQ(env_->EnumItem_GetName(greenStr, &greenStrName), ANI_OK);
    GetStdString(greenStrName, itemName);
    ASSERT_STREQ(itemName.data(), "GREENSTR");
    ani_string blueStrName {};
    ASSERT_EQ(env_->EnumItem_GetName(blueStr, &blueStrName), ANI_OK);
    GetStdString(blueStrName, itemName);
    ASSERT_STREQ(itemName.data(), "BLUESTR");
}

TEST_F(EnumItemGetNameTest, invalid_arg_enum)
{
    ani_enum aniEnum {};
    ASSERT_EQ(env_->FindEnum("enum_item_get_name_test.Color", &aniEnum), ANI_OK);
    ASSERT_NE(aniEnum, nullptr);

    ani_enum_item red {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnum, "RED", &red), ANI_OK);

    ani_string res {};
    ASSERT_EQ(env_->EnumItem_GetName(nullptr, &res), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->EnumItem_GetName(red, nullptr), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->c_api->EnumItem_GetName(nullptr, red, &res), ANI_INVALID_ARGS);
}

TEST_F(EnumItemGetNameTest, get_enum_item_name_combine_scenes)
{
    ani_enum aniEnum {};
    ASSERT_EQ(env_->FindEnum("enum_item_get_name_test.Color", &aniEnum), ANI_OK);
    ASSERT_NE(aniEnum, nullptr);

    ani_enum_item red {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnum, "RED", &red), ANI_OK);

    ani_size redIndex {};
    ASSERT_EQ(env_->EnumItem_GetIndex(red, &redIndex), ANI_OK);
    ASSERT_EQ(redIndex, 0U);

    ani_enum_item red2 {};
    ASSERT_EQ(env_->Enum_GetEnumItemByIndex(aniEnum, redIndex, &red2), ANI_OK);

    ani_boolean isRedEqual = ANI_FALSE;
    ASSERT_EQ(env_->Reference_Equals(red, red2, &isRedEqual), ANI_OK);
    ASSERT_EQ(isRedEqual, ANI_TRUE);
}

TEST_F(EnumItemGetNameTest, enum_get_name_test_one_item)
{
    ani_enum aniEnum {};
    ASSERT_EQ(env_->FindEnum("enum_item_get_name_test.OneItem", &aniEnum), ANI_OK);
    ASSERT_NE(aniEnum, nullptr);

    ani_enum_item one {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnum, "ONE", &one), ANI_OK);

    ani_string fromone {};
    std::string itemName {};
    ASSERT_EQ(env_->EnumItem_GetName(one, &fromone), ANI_OK);
    GetStdString(fromone, itemName);
    ASSERT_STREQ(itemName.data(), "ONE");
}

TEST_F(EnumItemGetNameTest, check_initialization)
{
    ani_enum aniEnum {};
    ASSERT_EQ(env_->FindEnum("enum_item_get_name_test.OneItem", &aniEnum), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("enum_item_get_name_test.OneItem"));
    ani_enum_item one {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnum, "ONE", &one), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("enum_item_get_name_test.OneItem"));
}

}  // namespace ark::ets::ani::testing
