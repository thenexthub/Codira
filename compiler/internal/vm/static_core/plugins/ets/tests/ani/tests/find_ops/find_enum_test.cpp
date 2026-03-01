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

class EnumFindTest : public AniTest {};

void RetrieveStringFromAni(ani_env *env, ani_string string, std::string &resString)
{
    ani_size result = 0U;
    ASSERT_EQ(env->String_GetUTF8Size(string, &result), ANI_OK);
    ani_size substrOffset = 0U;
    ani_size substrSize = result;
    const ani_size bufferExtension = 10U;
    resString.resize(substrSize + bufferExtension);
    ani_size resSize = resString.size();
    result = 0U;
    auto status = env->String_GetUTF8SubString(string, substrOffset, substrSize, resString.data(), resSize, &result);
    ASSERT_EQ(status, ANI_OK);
}

TEST_F(EnumFindTest, find_enum)
{
    ani_enum aniEnum {};
    ASSERT_EQ(env_->FindEnum("find_enum_test.Color", &aniEnum), ANI_OK);
    ASSERT_NE(aniEnum, nullptr);

    ani_enum aniEnumInt {};
    ASSERT_EQ(env_->FindEnum("find_enum_test.ColorInt", &aniEnumInt), ANI_OK);
    ASSERT_NE(aniEnumInt, nullptr);

    ani_enum aniEnumString {};
    ASSERT_EQ(env_->FindEnum("find_enum_test.ColorString", &aniEnumString), ANI_OK);
    ASSERT_NE(aniEnumString, nullptr);
}

TEST_F(EnumFindTest, invalid_arg_enum)
{
    ani_enum en {};
    ASSERT_EQ(env_->FindEnum(nullptr, &en), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->FindEnum("find_enum_test.Color", nullptr), ANI_INVALID_ARGS);
}

TEST_F(EnumFindTest, invalid_arg_name)
{
    ani_enum en {};
    ASSERT_EQ(env_->FindEnum("find_enum_test.NotFound", &en), ANI_NOT_FOUND);
    ASSERT_EQ(env_->FindEnum("", &en), ANI_INVALID_DESCRIPTOR);
    ASSERT_EQ(env_->FindEnum("\t", &en), ANI_NOT_FOUND);
}

TEST_F(EnumFindTest, invalid_arg_env)
{
    ani_enum en {};
    ASSERT_EQ(env_->c_api->FindEnum(nullptr, "#Color", &en), ANI_INVALID_ARGS);
}

TEST_F(EnumFindTest, find_enum_combine_scenes_001)
{
    ani_enum aniEnum {};
    ASSERT_EQ(env_->FindEnum("find_enum_test.EnumA001", &aniEnum), ANI_OK);
    ASSERT_NE(aniEnum, nullptr);

    ani_enum_item red {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnum, "RED", &red), ANI_OK);
    ani_enum_item green {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnum, "GREEN", &green), ANI_OK);
    ani_enum_item blue {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnum, "BLUE", &blue), ANI_OK);

    ani_enum fromRed {};
    ASSERT_EQ(env_->EnumItem_GetEnum(red, &fromRed), ANI_OK);
    ani_enum fromGreen {};
    ASSERT_EQ(env_->EnumItem_GetEnum(red, &fromGreen), ANI_OK);
    ani_enum fromBlue {};
    ASSERT_EQ(env_->EnumItem_GetEnum(red, &fromBlue), ANI_OK);

    ani_int redValInt = 0;
    ASSERT_EQ(env_->EnumItem_GetValue_Int(red, &redValInt), ANI_OK);
    ASSERT_EQ(redValInt, 0U);
    ani_int greenValInt = 0;
    ASSERT_EQ(env_->EnumItem_GetValue_Int(green, &greenValInt), ANI_OK);
    ASSERT_EQ(greenValInt, 1U);
    ani_int blueValInt = 0;
    ASSERT_EQ(env_->EnumItem_GetValue_Int(blue, &blueValInt), ANI_OK);
    ASSERT_EQ(blueValInt, 2U);

    std::string itemName {};
    ani_string redName {};
    ASSERT_EQ(env_->EnumItem_GetName(red, &redName), ANI_OK);
    RetrieveStringFromAni(env_, redName, itemName);
    ASSERT_STREQ(itemName.data(), "RED");
    ani_string greenName {};
    ASSERT_EQ(env_->EnumItem_GetName(green, &greenName), ANI_OK);
    RetrieveStringFromAni(env_, greenName, itemName);
    ASSERT_STREQ(itemName.data(), "GREEN");
    ani_string blueName {};
    ASSERT_EQ(env_->EnumItem_GetName(blue, &blueName), ANI_OK);
    RetrieveStringFromAni(env_, blueName, itemName);
    ASSERT_STREQ(itemName.data(), "BLUE");

    ani_size redIndex {};
    ASSERT_EQ(env_->EnumItem_GetIndex(red, &redIndex), ANI_OK);
    ASSERT_EQ(redIndex, 0U);
    ani_size greenIndex {};
    ASSERT_EQ(env_->EnumItem_GetIndex(green, &greenIndex), ANI_OK);
    ASSERT_EQ(greenIndex, 1U);
    ani_size blueIndex {};
    ASSERT_EQ(env_->EnumItem_GetIndex(blue, &blueIndex), ANI_OK);
    ASSERT_EQ(blueIndex, 2U);
}

TEST_F(EnumFindTest, find_enum_combine_scenes_001_1)
{
    ani_enum aniEnum {};
    ASSERT_EQ(env_->FindEnum("find_enum_test.EnumA001", &aniEnum), ANI_OK);
    ASSERT_NE(aniEnum, nullptr);

    ani_enum_item red {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnum, "RED", &red), ANI_OK);
    ani_enum_item green {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnum, "GREEN", &green), ANI_OK);
    ani_enum_item blue {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnum, "BLUE", &blue), ANI_OK);

    ani_size redIndex {};
    ASSERT_EQ(env_->EnumItem_GetIndex(red, &redIndex), ANI_OK);
    ASSERT_EQ(redIndex, 0U);
    ani_size greenIndex {};
    ASSERT_EQ(env_->EnumItem_GetIndex(green, &greenIndex), ANI_OK);
    ASSERT_EQ(greenIndex, 1U);
    ani_size blueIndex {};
    ASSERT_EQ(env_->EnumItem_GetIndex(blue, &blueIndex), ANI_OK);
    ASSERT_EQ(blueIndex, 2U);

    ani_enum_item red2 {};
    ASSERT_EQ(env_->Enum_GetEnumItemByIndex(aniEnum, 0U, &red2), ANI_OK);
    ani_enum_item green2 {};
    ASSERT_EQ(env_->Enum_GetEnumItemByIndex(aniEnum, 1U, &green2), ANI_OK);
    ani_enum_item blue2 {};
    ASSERT_EQ(env_->Enum_GetEnumItemByIndex(aniEnum, 2U, &blue2), ANI_OK);

    ani_size redIndex2 {};
    ASSERT_EQ(env_->EnumItem_GetIndex(red2, &redIndex2), ANI_OK);
    ASSERT_EQ(redIndex2, 0U);
    ani_size greenIndex2 {};
    ASSERT_EQ(env_->EnumItem_GetIndex(green2, &greenIndex2), ANI_OK);
    ASSERT_EQ(greenIndex2, 1U);
    ani_size blueIndex2 {};
    ASSERT_EQ(env_->EnumItem_GetIndex(blue2, &blueIndex2), ANI_OK);
    ASSERT_EQ(blueIndex2, 2U);

    ani_int redValInt = 0;
    ASSERT_EQ(env_->EnumItem_GetValue_Int(red2, &redValInt), ANI_OK);
    ASSERT_EQ(redValInt, 0U);
    ani_int greenValInt = 0;
    ASSERT_EQ(env_->EnumItem_GetValue_Int(green2, &greenValInt), ANI_OK);
    ASSERT_EQ(greenValInt, 1U);
    ani_int blueValInt = 0;
    ASSERT_EQ(env_->EnumItem_GetValue_Int(blue2, &blueValInt), ANI_OK);
    ASSERT_EQ(blueValInt, 2U);
}

TEST_F(EnumFindTest, find_enum_combine_scenes_003)
{
    ani_enum aniEnum {};
    ASSERT_EQ(env_->FindEnum("find_enum_test.test003A.test003B.EnumA003", &aniEnum), ANI_OK);
    ASSERT_NE(aniEnum, nullptr);

    ani_enum_item red {};
    ASSERT_EQ(env_->Enum_GetEnumItemByIndex(aniEnum, 0U, &red), ANI_OK);
    ani_enum_item green {};
    ASSERT_EQ(env_->Enum_GetEnumItemByIndex(aniEnum, 1U, &green), ANI_OK);
    ani_enum_item blue {};
    ASSERT_EQ(env_->Enum_GetEnumItemByIndex(aniEnum, 2U, &blue), ANI_OK);

    ani_size redIndex {};
    ASSERT_EQ(env_->EnumItem_GetIndex(red, &redIndex), ANI_OK);
    ASSERT_EQ(redIndex, 0U);
    ani_size greenIndex {};
    ASSERT_EQ(env_->EnumItem_GetIndex(green, &greenIndex), ANI_OK);
    ASSERT_EQ(greenIndex, 1U);
    ani_size blueIndex {};
    ASSERT_EQ(env_->EnumItem_GetIndex(blue, &blueIndex), ANI_OK);
    ASSERT_EQ(blueIndex, 2U);

    ani_int redValInt = 0;
    ASSERT_EQ(env_->EnumItem_GetValue_Int(red, &redValInt), ANI_OK);
    ASSERT_EQ(redValInt, 0U);
    ani_int greenValInt = 0;
    ASSERT_EQ(env_->EnumItem_GetValue_Int(green, &greenValInt), ANI_OK);
    ASSERT_EQ(greenValInt, 1U);
    ani_int blueValInt = 0;
    ASSERT_EQ(env_->EnumItem_GetValue_Int(blue, &blueValInt), ANI_OK);
    ASSERT_EQ(blueValInt, 2U);
}

TEST_F(EnumFindTest, find_enum_combine_scenes_004)
{
    ani_enum aniEnum {};
    ASSERT_EQ(env_->FindEnum("find_enum_test.test004A.EnumA004", &aniEnum), ANI_OK);
    ASSERT_NE(aniEnum, nullptr);

    ani_enum_item red {};
    ASSERT_EQ(env_->Enum_GetEnumItemByIndex(aniEnum, 0U, &red), ANI_OK);
    ani_enum_item green {};
    ASSERT_EQ(env_->Enum_GetEnumItemByIndex(aniEnum, 1U, &green), ANI_OK);
    ani_enum_item blue {};
    ASSERT_EQ(env_->Enum_GetEnumItemByIndex(aniEnum, 2U, &blue), ANI_OK);

    std::string itemName {};
    ani_string redName {};
    ASSERT_EQ(env_->EnumItem_GetName(red, &redName), ANI_OK);
    RetrieveStringFromAni(env_, redName, itemName);
    ASSERT_STREQ(itemName.data(), "RED");
    ani_string greenName {};
    ASSERT_EQ(env_->EnumItem_GetName(green, &greenName), ANI_OK);
    RetrieveStringFromAni(env_, greenName, itemName);
    ASSERT_STREQ(itemName.data(), "GREEN");
    ani_string blueName {};
    ASSERT_EQ(env_->EnumItem_GetName(blue, &blueName), ANI_OK);
    RetrieveStringFromAni(env_, blueName, itemName);
    ASSERT_STREQ(itemName.data(), "BLUE");
}

TEST_F(EnumFindTest, check_initialization)
{
    ASSERT_FALSE(IsRuntimeClassInitialized("find_enum_test.EnumA001"));
    ani_enum aniEnum {};
    ASSERT_EQ(env_->FindEnum("find_enum_test.EnumA001", &aniEnum), ANI_OK);
    ASSERT_FALSE(IsRuntimeClassInitialized("find_enum_test.EnumA001"));
}

TEST_F(EnumFindTest, wrong_signature)
{
    ani_enum aniEnum {};
    ASSERT_EQ(env_->FindEnum("find_enum_test/EnumA001", &aniEnum), ANI_INVALID_DESCRIPTOR);
}

}  // namespace ark::ets::ani::testing
