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

class EnumItemGetIndexTest : public AniTest {
public:
    static constexpr int32_t LOOP_COUNT = 3;
};

TEST_F(EnumItemGetIndexTest, get_enum_item_index)
{
    std::string itemName;

    ani_enum aniEnum {};
    ASSERT_EQ(env_->FindEnum("enum_item_get_index_test.Color", &aniEnum), ANI_OK);
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
}

TEST_F(EnumItemGetIndexTest, invalid_arg_enum)
{
    ani_enum aniEnum {};
    ASSERT_EQ(env_->FindEnum("enum_item_get_index_test.Color", &aniEnum), ANI_OK);
    ASSERT_NE(aniEnum, nullptr);

    ani_enum_item red {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnum, "RED", &red), ANI_OK);

    ani_size res {};
    ASSERT_EQ(env_->EnumItem_GetIndex(nullptr, &res), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->EnumItem_GetIndex(red, nullptr), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->c_api->EnumItem_GetIndex(nullptr, red, &res), ANI_INVALID_ARGS);
}

TEST_F(EnumItemGetIndexTest, enum_get_item_by_index_1)
{
    ani_enum aniEnum {};
    ASSERT_EQ(env_->FindEnum("enum_item_get_index_test.Color", &aniEnum), ANI_OK);
    ASSERT_NE(aniEnum, nullptr);

    ani_enum_item red2 {};
    std::string itemName {};
    ani_string redName {};
    ani_enum_item red {};
    ani_boolean isRedEqual = ANI_FALSE;

    for (int32_t times = 0; times < LOOP_COUNT; times++) {
        ASSERT_EQ(env_->Enum_GetEnumItemByIndex(aniEnum, 0U, &red2), ANI_OK);
        ASSERT_EQ(env_->EnumItem_GetName(red2, &redName), ANI_OK);
        GetStdString(redName, itemName);
        ASSERT_STREQ(itemName.data(), "RED");
        ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnum, itemName.c_str(), &red), ANI_OK);
        ASSERT_EQ(env_->Reference_Equals(red, red2, &isRedEqual), ANI_OK);
        ASSERT_EQ(isRedEqual, ANI_TRUE);
    }
}

TEST_F(EnumItemGetIndexTest, enum_get_item_by_index_2)
{
    ani_enum aniEnum {};
    ASSERT_EQ(env_->FindEnum("enum_item_get_index_test.Color", &aniEnum), ANI_OK);
    ASSERT_NE(aniEnum, nullptr);

    ani_enum_item red {};
    ani_enum fromRed {};
    ani_size redIndex = 5U;
    ani_enum_item red2 {};
    ani_boolean isRedEqual = ANI_FALSE;

    for (int32_t times = 0; times < LOOP_COUNT; times++) {
        ASSERT_EQ(env_->Enum_GetEnumItemByIndex(aniEnum, 0U, &red), ANI_OK);
        ASSERT_EQ(env_->EnumItem_GetEnum(red, &fromRed), ANI_OK);
        ASSERT_EQ(env_->EnumItem_GetIndex(red, &redIndex), ANI_OK);
        ASSERT_EQ(redIndex, 0U);
        ASSERT_EQ(env_->Enum_GetEnumItemByIndex(fromRed, redIndex, &red2), ANI_OK);
        ASSERT_EQ(env_->Reference_Equals(red, red2, &isRedEqual), ANI_OK);
        ASSERT_EQ(isRedEqual, ANI_TRUE);
        ASSERT_EQ(env_->Reference_Equals(aniEnum, fromRed, &isRedEqual), ANI_OK);
        ASSERT_EQ(isRedEqual, ANI_TRUE);
    }
}

TEST_F(EnumItemGetIndexTest, enum_get_item_by_index_3)
{
    ani_enum aniEnum {};
    ASSERT_EQ(env_->FindEnum("enum_item_get_index_test.Color", &aniEnum), ANI_OK);
    ASSERT_NE(aniEnum, nullptr);

    ani_enum_item red {};
    ani_enum_item green {};
    ani_enum_item blue {};
    for (int32_t times = 0; times < LOOP_COUNT; times++) {
        ASSERT_EQ(env_->Enum_GetEnumItemByIndex(aniEnum, 0U, &red), ANI_OK);
        ASSERT_EQ(env_->Enum_GetEnumItemByIndex(aniEnum, 1U, &green), ANI_OK);
        ASSERT_EQ(env_->Enum_GetEnumItemByIndex(aniEnum, 2U, &blue), ANI_OK);
    }
}

TEST_F(EnumItemGetIndexTest, enum_get_item_by_index_one_item)
{
    ani_enum aniEnum {};
    ASSERT_EQ(env_->FindEnum("enum_item_get_index_test.OneItem", &aniEnum), ANI_OK);
    ASSERT_NE(aniEnum, nullptr);

    ani_enum_item oneItem {};
    ASSERT_EQ(env_->Enum_GetEnumItemByIndex(aniEnum, 0U, &oneItem), ANI_OK);
}

TEST_F(EnumItemGetIndexTest, enum_get_index_test_one_item)
{
    ani_enum aniEnum {};
    ASSERT_EQ(env_->FindEnum("enum_item_get_index_test.OneItem", &aniEnum), ANI_OK);
    ASSERT_NE(aniEnum, nullptr);

    ani_enum_item one {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnum, "ONE", &one), ANI_OK);

    ani_size oneIndex = 5U;
    ASSERT_EQ(env_->EnumItem_GetIndex(one, &oneIndex), ANI_OK);
    ASSERT_EQ(oneIndex, 0U);
}

TEST_F(EnumItemGetIndexTest, enum_get_item_by_index_combine_scenes_001)
{
    ani_enum aniEnum {};
    ASSERT_EQ(env_->FindEnum("enum_item_get_index_test.Color", &aniEnum), ANI_OK);
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

    ani_int redValInt = 0U;
    ASSERT_EQ(env_->EnumItem_GetValue_Int(red, &redValInt), ANI_OK);
    ASSERT_EQ(redValInt, 0U);
    ani_int greenValInt = 0U;
    ASSERT_EQ(env_->EnumItem_GetValue_Int(green, &greenValInt), ANI_OK);
    ASSERT_EQ(greenValInt, 1U);
    ani_int blueValInt = 0U;
    ASSERT_EQ(env_->EnumItem_GetValue_Int(blue, &blueValInt), ANI_OK);
    ASSERT_EQ(blueValInt, 2U);

    std::string itemName {};
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

TEST_F(EnumItemGetIndexTest, check_initialization)
{
    ani_enum aniEnum {};
    ASSERT_EQ(env_->FindEnum("enum_item_get_index_test.Color", &aniEnum), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("enum_item_get_index_test.Color"));
    ani_enum_item red {};
    ASSERT_EQ(env_->Enum_GetEnumItemByIndex(aniEnum, 0U, &red), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("enum_item_get_index_test.Color"));
}

}  // namespace ark::ets::ani::testing
