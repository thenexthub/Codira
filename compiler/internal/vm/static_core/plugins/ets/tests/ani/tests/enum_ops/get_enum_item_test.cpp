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

class GetEnumItemTest : public AniTest {};

TEST_F(GetEnumItemTest, get_enum_item_by_name)
{
    ani_enum aniEnum {};
    ASSERT_EQ(env_->FindEnum("get_enum_item_test.ToFind", &aniEnum), ANI_OK);
    ASSERT_NE(aniEnum, nullptr);
    ani_enum_item red {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnum, "RED", &red), ANI_OK);
    ani_enum_item green {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnum, "GREEN", &green), ANI_OK);
    ani_enum_item blue {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnum, "BLUE", &blue), ANI_OK);

    ani_enum aniEnumInt {};
    ASSERT_EQ(env_->FindEnum("get_enum_item_test.ToFindInt", &aniEnumInt), ANI_OK);
    ASSERT_NE(aniEnumInt, nullptr);
    ani_enum_item redInt {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnumInt, "REDINT", &redInt), ANI_OK);
    ani_enum_item greenInt {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnumInt, "GREENINT", &greenInt), ANI_OK);
    ani_enum_item blueInt {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnumInt, "BLUEINT", &blueInt), ANI_OK);

    ani_enum aniEnumStr {};
    ASSERT_EQ(env_->FindEnum("get_enum_item_test.ToFindString", &aniEnumStr), ANI_OK);
    ASSERT_NE(aniEnumStr, nullptr);
    ani_enum_item redStr {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnumStr, "REDSTR", &redStr), ANI_OK);
    ani_enum_item greenStr {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnumStr, "GREENSTR", &greenStr), ANI_OK);
    ani_enum_item blueStr {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnumStr, "BLUESTR", &blueStr), ANI_OK);

    ani_enum aniEnumBig {};
    ASSERT_EQ(env_->FindEnum("get_enum_item_test.ToFindBig", &aniEnumBig), ANI_OK);
    ASSERT_NE(aniEnumBig, nullptr);
    ani_enum_item zero {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnumBig, "ZERO", &zero), ANI_OK);
    ani_enum_item nine {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnumBig, "NINE", &nine), ANI_OK);
}

TEST_F(GetEnumItemTest, get_wrong_enum_item_by_name)
{
    ani_enum aniEnum {};
    ASSERT_EQ(env_->FindEnum("get_enum_item_test.ToFind", &aniEnum), ANI_OK);
    ASSERT_NE(aniEnum, nullptr);

    ani_enum_item red {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnum, "red", &red), ANI_NOT_FOUND);
    ani_enum_item green {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnum, "green", &green), ANI_NOT_FOUND);
    ani_enum_item blue {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnum, "blue", &blue), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnum, "REDINT", &red), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnum, "GREENINT", &green), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnum, "BLUEINT", &blue), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnum, "REDSTR", &red), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnum, "GREENSTR", &green), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnum, "BLUESTR", &blue), ANI_NOT_FOUND);

    ani_enum aniEnumBig {};
    ASSERT_EQ(env_->FindEnum("get_enum_item_test.ToFindBig", &aniEnumBig), ANI_OK);
    ASSERT_NE(aniEnumBig, nullptr);
    ani_enum_item minus {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnumBig, "MINUSONE", &minus), ANI_NOT_FOUND);
    ani_enum_item ten {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnumBig, "TEN", &ten), ANI_NOT_FOUND);
}

TEST_F(GetEnumItemTest, get_enum_item_by_index)
{
    ani_enum aniEnum {};
    ASSERT_EQ(env_->FindEnum("get_enum_item_test.ToFind", &aniEnum), ANI_OK);
    ASSERT_NE(aniEnum, nullptr);
    ani_enum_item red {};
    ASSERT_EQ(env_->Enum_GetEnumItemByIndex(aniEnum, 0U, &red), ANI_OK);
    ani_enum_item green {};
    ASSERT_EQ(env_->Enum_GetEnumItemByIndex(aniEnum, 1U, &green), ANI_OK);
    ani_enum_item blue {};
    ASSERT_EQ(env_->Enum_GetEnumItemByIndex(aniEnum, 2U, &blue), ANI_OK);

    ani_enum aniEnumInt {};
    ASSERT_EQ(env_->FindEnum("get_enum_item_test.ToFindInt", &aniEnumInt), ANI_OK);
    ASSERT_NE(aniEnumInt, nullptr);
    ani_enum_item redInt {};
    ASSERT_EQ(env_->Enum_GetEnumItemByIndex(aniEnumInt, 0U, &redInt), ANI_OK);
    ani_enum_item greenInt {};
    ASSERT_EQ(env_->Enum_GetEnumItemByIndex(aniEnumInt, 1U, &greenInt), ANI_OK);
    ani_enum_item blueInt {};
    ASSERT_EQ(env_->Enum_GetEnumItemByIndex(aniEnumInt, 2U, &blueInt), ANI_OK);

    ani_enum aniEnumStr {};
    ASSERT_EQ(env_->FindEnum("get_enum_item_test.ToFindString", &aniEnumStr), ANI_OK);
    ASSERT_NE(aniEnumStr, nullptr);
    ani_enum_item redStr {};
    ASSERT_EQ(env_->Enum_GetEnumItemByIndex(aniEnumStr, 0U, &redStr), ANI_OK);
    ani_enum_item greenStr {};
    ASSERT_EQ(env_->Enum_GetEnumItemByIndex(aniEnumStr, 1U, &greenStr), ANI_OK);
    ani_enum_item blueStr {};
    ASSERT_EQ(env_->Enum_GetEnumItemByIndex(aniEnumStr, 2U, &blueStr), ANI_OK);

    ani_enum aniEnumBig {};
    ASSERT_EQ(env_->FindEnum("get_enum_item_test.ToFindBig", &aniEnumBig), ANI_OK);
    ASSERT_NE(aniEnumBig, nullptr);
    ani_enum_item zero {};
    ASSERT_EQ(env_->Enum_GetEnumItemByIndex(aniEnumBig, 0U, &zero), ANI_OK);
    ani_enum_item nine {};
    ASSERT_EQ(env_->Enum_GetEnumItemByIndex(aniEnumBig, 9U, &nine), ANI_OK);
}

TEST_F(GetEnumItemTest, get_wrong_enum_item_by_index)
{
    ani_enum aniEnum {};
    ASSERT_EQ(env_->FindEnum("get_enum_item_test.ToFind", &aniEnum), ANI_OK);
    ASSERT_NE(aniEnum, nullptr);

    ani_enum_item item {};
    ASSERT_EQ(env_->Enum_GetEnumItemByIndex(aniEnum, 3U, &item), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Enum_GetEnumItemByIndex(aniEnum, 4U, &item), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Enum_GetEnumItemByIndex(aniEnum, -1, &item), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Enum_GetEnumItemByIndex(aniEnum, 42U, &item), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Enum_GetEnumItemByIndex(aniEnum, -1 * 42U, &item), ANI_NOT_FOUND);

    ani_enum aniEnumBig {};
    ASSERT_EQ(env_->FindEnum("get_enum_item_test.ToFindBig", &aniEnumBig), ANI_OK);
    ASSERT_NE(aniEnumBig, nullptr);
    ani_enum_item minus {};
    ASSERT_EQ(env_->Enum_GetEnumItemByIndex(aniEnumBig, -1, &minus), ANI_NOT_FOUND);
    ani_enum_item ten {};
    ASSERT_EQ(env_->Enum_GetEnumItemByIndex(aniEnumBig, 10U, &ten), ANI_NOT_FOUND);
}

TEST_F(GetEnumItemTest, get_item_name_index_equal)
{
    ani_enum aniEnum {};
    ASSERT_EQ(env_->FindEnum("get_enum_item_test.ToFind", &aniEnum), ANI_OK);
    ASSERT_NE(aniEnum, nullptr);

    ani_enum_item redName {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnum, "RED", &redName), ANI_OK);
    ani_enum_item redIndex {};
    ASSERT_EQ(env_->Enum_GetEnumItemByIndex(aniEnum, 0U, &redIndex), ANI_OK);
    ani_boolean isRedEqual = ANI_FALSE;
    ASSERT_EQ(env_->Reference_Equals(redName, redIndex, &isRedEqual), ANI_OK);
    ASSERT_EQ(isRedEqual, ANI_TRUE);

    ani_enum_item greenName {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnum, "GREEN", &greenName), ANI_OK);
    ani_enum_item greenIndex {};
    ASSERT_EQ(env_->Enum_GetEnumItemByIndex(aniEnum, 1U, &greenIndex), ANI_OK);
    ani_boolean isGreenEqual = ANI_FALSE;
    ASSERT_EQ(env_->Reference_Equals(greenName, greenIndex, &isGreenEqual), ANI_OK);
    ASSERT_EQ(isGreenEqual, ANI_TRUE);

    ani_enum_item blueName {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnum, "BLUE", &blueName), ANI_OK);
    ani_enum_item blueIndex {};
    ASSERT_EQ(env_->Enum_GetEnumItemByIndex(aniEnum, 2U, &blueIndex), ANI_OK);
    ani_boolean isBlueEqual = ANI_FALSE;
    ASSERT_EQ(env_->Reference_Equals(blueName, blueIndex, &isBlueEqual), ANI_OK);
    ASSERT_EQ(isBlueEqual, ANI_TRUE);
}

TEST_F(GetEnumItemTest, invalid_arg_enum)
{
    ani_enum aniEnum {};
    ASSERT_EQ(env_->FindEnum("get_enum_item_test.ToFind", &aniEnum), ANI_OK);
    ASSERT_NE(aniEnum, nullptr);

    ani_enum_item red {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(nullptr, "RED", &red), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnum, nullptr, &red), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnum, "RED", nullptr), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->c_api->Enum_GetEnumItemByName(nullptr, aniEnum, "RED", &red), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnum, "", &red), ANI_NOT_FOUND);

    ASSERT_EQ(env_->Enum_GetEnumItemByIndex(nullptr, 0U, &red), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Enum_GetEnumItemByIndex(aniEnum, -1U, &red), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Enum_GetEnumItemByIndex(aniEnum, 0U, nullptr), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->c_api->Enum_GetEnumItemByIndex(nullptr, aniEnum, 0U, &red), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing
