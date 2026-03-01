/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
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

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
namespace ark::ets::ani::testing {

class IsAssignableFromUnionTest : public AniTest {
public:
    EtsClass *GetClassFromDesc(const PandaString &desc)
    {
        PandaEnv *pandaEnv = PandaEnv::FromAniEnv(env_);
        [[maybe_unused]] ScopedManagedCodeFix s(pandaEnv);
        EtsClassLinker *classLinker = pandaEnv->GetEtsVM()->GetClassLinker();
        return classLinker->GetClass(desc.c_str());
    }
};

TEST_F(IsAssignableFromUnionTest, is_assignable_class)
{
    const PandaString className_A = "Lis_assignable_from_union_test/A;";
    EtsClass *class_A = GetClassFromDesc(className_A);
    ASSERT_NE(class_A, nullptr);
    ASSERT_FALSE(class_A->IsUnionClass());

    const PandaString className_B = "Lis_assignable_from_union_test/B;";
    EtsClass *class_B = GetClassFromDesc(className_B);
    ASSERT_NE(class_B, nullptr);
    ASSERT_FALSE(class_B->IsUnionClass());

    const PandaString className_C = "Lis_assignable_from_union_test/C;";
    EtsClass *class_C = GetClassFromDesc(className_C);
    ASSERT_NE(class_C, nullptr);
    ASSERT_FALSE(class_C->IsUnionClass());

    const PandaString className_D = "Lis_assignable_from_union_test/D;";
    EtsClass *class_D = GetClassFromDesc(className_D);
    ASSERT_NE(class_D, nullptr);
    ASSERT_FALSE(class_D->IsUnionClass());

    const PandaString unionClassName_AD = "{ULis_assignable_from_union_test/A;Lis_assignable_from_union_test/D;}";
    EtsClass *class_AD = GetClassFromDesc(unionClassName_AD);
    ASSERT_NE(class_AD, nullptr);
    ASSERT_TRUE(class_AD->IsUnionClass());

    const PandaString unionClassName_ACD =
        "{ULis_assignable_from_union_test/A;Lis_assignable_from_union_test/C;Lis_assignable_from_union_test/D;}";
    EtsClass *class_ACD = GetClassFromDesc(unionClassName_ACD);
    ASSERT_NE(class_ACD, nullptr);
    ASSERT_TRUE(class_ACD->IsUnionClass());

    const PandaString unionClassName_ABC_DE =
        "{ULis_assignable_from_union_test/A;Lis_assignable_from_union_test/C;Lis_assignable_from_union_test/"
        "D;Lis_assignable_from_union_test/E;}";
    EtsClass *class_ABC_DE = GetClassFromDesc(unionClassName_ABC_DE);
    ASSERT_NE(class_ABC_DE, nullptr);
    ASSERT_TRUE(class_ABC_DE->IsUnionClass());

    ASSERT_TRUE(class_AD->IsAssignableFrom(class_A));
    ASSERT_TRUE(class_AD->IsAssignableFrom(class_D));
    ASSERT_TRUE(class_AD->IsAssignableFrom(class_B));
    ASSERT_TRUE(class_ACD->IsAssignableFrom(class_C));
    ASSERT_TRUE(class_ABC_DE->IsAssignableFrom(class_B));
    ASSERT_TRUE(class_ACD->IsAssignableFrom(class_AD));
    ASSERT_TRUE(class_ABC_DE->IsAssignableFrom(class_AD));
    ASSERT_TRUE(class_ABC_DE->IsAssignableFrom(class_ACD));
    ASSERT_TRUE(class_ABC_DE->IsAssignableFrom(class_ABC_DE));
}

TEST_F(IsAssignableFromUnionTest, is_assignable_null_class)
{
    const PandaString className_A = "Lis_assignable_from_union_test/A;";
    EtsClass *class_A = GetClassFromDesc(className_A);
    ASSERT_NE(class_A, nullptr);
    ASSERT_FALSE(class_A->IsUnionClass());

    const PandaString className_B = "Lis_assignable_from_union_test/B;";
    EtsClass *class_B = GetClassFromDesc(className_B);
    ASSERT_NE(class_B, nullptr);
    ASSERT_FALSE(class_B->IsUnionClass());

    const PandaString className_D = "Lis_assignable_from_union_test/D;";
    EtsClass *class_D = GetClassFromDesc(className_D);
    ASSERT_NE(class_D, nullptr);
    ASSERT_FALSE(class_D->IsUnionClass());

    const PandaString unionClassName_AD = "{ULis_assignable_from_union_test/A;Lis_assignable_from_union_test/D;}";
    EtsClass *class_AD = GetClassFromDesc(unionClassName_AD);
    ASSERT_NE(class_AD, nullptr);
    ASSERT_TRUE(class_AD->IsUnionClass());

    const PandaString className_Null = "Lstd/core/Null;";
    EtsClass *class_Null = GetClassFromDesc(className_Null);
    ASSERT_NE(class_Null, nullptr);
    ASSERT_FALSE(class_Null->IsUnionClass());

    const PandaString className_BNull = "{ULis_assignable_from_union_test/B;Lstd/core/Null;}";
    EtsClass *class_BNull = GetClassFromDesc(className_BNull);
    ASSERT_NE(class_BNull, nullptr);
    ASSERT_TRUE(class_BNull->IsUnionClass());

    const PandaString className_ADNull =
        "{ULis_assignable_from_union_test/A;Lis_assignable_from_union_test/D;Lstd/core/Null;}";
    EtsClass *class_ADNull = GetClassFromDesc(className_ADNull);
    ASSERT_NE(class_ADNull, nullptr);
    ASSERT_TRUE(class_ADNull->IsUnionClass());

    ASSERT_TRUE(class_ADNull->IsAssignableFrom(class_Null));
    ASSERT_TRUE(class_ADNull->IsAssignableFrom(class_A));
    ASSERT_TRUE(class_ADNull->IsAssignableFrom(class_D));
    ASSERT_TRUE(class_ADNull->IsAssignableFrom(class_B));
    ASSERT_TRUE(class_ADNull->IsAssignableFrom(class_BNull));
    ASSERT_TRUE(class_ADNull->IsAssignableFrom(class_AD));
    ASSERT_TRUE(class_ADNull->IsAssignableFrom(class_ADNull));
}

TEST_F(IsAssignableFromUnionTest, not_assignable_class)
{
    const PandaString className_A = "Lis_assignable_from_union_test/A;";
    EtsClass *class_A = GetClassFromDesc(className_A);
    ASSERT_NE(class_A, nullptr);
    ASSERT_FALSE(class_A->IsUnionClass());

    const PandaString className_C = "Lis_assignable_from_union_test/C;";
    EtsClass *class_C = GetClassFromDesc(className_C);
    ASSERT_NE(class_C, nullptr);
    ASSERT_FALSE(class_C->IsUnionClass());

    const PandaString unionClassName_AD = "{ULis_assignable_from_union_test/A;Lis_assignable_from_union_test/D;}";
    EtsClass *class_AD = GetClassFromDesc(unionClassName_AD);
    ASSERT_NE(class_AD, nullptr);
    ASSERT_TRUE(class_AD->IsUnionClass());

    const PandaString unionClassName_DE = "{ULis_assignable_from_union_test/D;Lis_assignable_from_union_test/E;}";
    EtsClass *class_DE = GetClassFromDesc(unionClassName_DE);
    ASSERT_NE(class_DE, nullptr);
    ASSERT_TRUE(class_DE->IsUnionClass());

    const PandaString unionClassName_BC = "{ULis_assignable_from_union_test/B;Lis_assignable_from_union_test/C;}";
    EtsClass *class_BC = GetClassFromDesc(unionClassName_BC);
    ASSERT_NE(class_BC, nullptr);
    ASSERT_TRUE(class_BC->IsUnionClass());

    const PandaString unionClassName_ACD =
        "{ULis_assignable_from_union_test/A;Lis_assignable_from_union_test/C;Lis_assignable_from_union_test/D;}";
    EtsClass *class_ACD = GetClassFromDesc(unionClassName_ACD);
    ASSERT_NE(class_ACD, nullptr);
    ASSERT_TRUE(class_ACD->IsUnionClass());

    const PandaString unionClassName_ABC_DE =
        "{ULis_assignable_from_union_test/A;Lis_assignable_from_union_test/C;Lis_assignable_from_union_test/"
        "D;Lis_assignable_from_union_test/E;}";
    EtsClass *class_ABC_DE = GetClassFromDesc(unionClassName_ABC_DE);
    ASSERT_NE(class_ABC_DE, nullptr);
    ASSERT_TRUE(class_ABC_DE->IsUnionClass());

    ASSERT_FALSE(class_C->IsAssignableFrom(class_ACD));
    ASSERT_FALSE(class_AD->IsAssignableFrom(class_C));
    ASSERT_FALSE(class_BC->IsAssignableFrom(class_A));
    ASSERT_FALSE(class_AD->IsAssignableFrom(class_ACD));
    ASSERT_FALSE(class_DE->IsAssignableFrom(class_ABC_DE));
}

TEST_F(IsAssignableFromUnionTest, not_assignable_null_class)
{
    const PandaString className_A = "Lis_assignable_from_union_test/A;";
    EtsClass *class_A = GetClassFromDesc(className_A);
    ASSERT_NE(class_A, nullptr);
    ASSERT_FALSE(class_A->IsUnionClass());

    const PandaString unionClassName_AD = "{ULis_assignable_from_union_test/A;Lis_assignable_from_union_test/D;}";
    EtsClass *class_AD = GetClassFromDesc(unionClassName_AD);
    ASSERT_NE(class_AD, nullptr);
    ASSERT_TRUE(class_AD->IsUnionClass());

    const PandaString unionClassName_ACD =
        "{ULis_assignable_from_union_test/A;Lis_assignable_from_union_test/C;Lis_assignable_from_union_test/D;}";
    EtsClass *class_ACD = GetClassFromDesc(unionClassName_ACD);
    ASSERT_NE(class_ACD, nullptr);
    ASSERT_TRUE(class_ACD->IsUnionClass());

    const PandaString className_Null = "Lstd/core/Null;";
    EtsClass *class_Null = GetClassFromDesc(className_Null);
    ASSERT_NE(class_Null, nullptr);
    ASSERT_FALSE(class_Null->IsUnionClass());

    const PandaString className_ANull = "{ULis_assignable_from_union_test/A;Lstd/core/Null;}";
    EtsClass *class_ANull = GetClassFromDesc(className_ANull);
    ASSERT_NE(class_ANull, nullptr);
    ASSERT_TRUE(class_ANull->IsUnionClass());

    const PandaString className_ADNull =
        "{ULis_assignable_from_union_test/A;Lis_assignable_from_union_test/D;Lstd/core/Null;}";
    EtsClass *class_ADNull = GetClassFromDesc(className_ADNull);
    ASSERT_NE(class_ADNull, nullptr);
    ASSERT_TRUE(class_ADNull->IsUnionClass());

    ASSERT_FALSE(class_Null->IsAssignableFrom(class_ANull));
    ASSERT_FALSE(class_A->IsAssignableFrom(class_ANull));
    ASSERT_FALSE(class_ACD->IsAssignableFrom(class_ADNull));
    ASSERT_FALSE(class_ADNull->IsAssignableFrom(class_ACD));
}

TEST_F(IsAssignableFromUnionTest, is_instanceof_undefined)
{
    ani_ref undefinedRef = nullptr;
    ani_boolean isUndefined = ANI_FALSE;
    ASSERT_EQ(env_->GetUndefined(&undefinedRef), ANI_OK);
    ASSERT_EQ(env_->Reference_IsUndefined(undefinedRef, &isUndefined), ANI_OK);
    ASSERT_EQ(isUndefined, ANI_TRUE);

    auto isInstanceofUndefined =
        CallEtsFunction<ani_boolean>("is_assignable_from_union_test", "isInstanceofUndefined", undefinedRef);
    ASSERT_EQ(isInstanceofUndefined, ANI_TRUE);
}

TEST_F(IsAssignableFromUnionTest, not_instanceof_undefined)
{
    ani_ref undefinedRef = nullptr;
    ani_boolean isUndefined = ANI_FALSE;
    ASSERT_EQ(env_->GetUndefined(&undefinedRef), ANI_OK);
    ASSERT_EQ(env_->Reference_IsUndefined(undefinedRef, &isUndefined), ANI_OK);
    ASSERT_EQ(isUndefined, ANI_TRUE);

    auto isInstanceofNotUndefined =
        CallEtsFunction<ani_boolean>("is_assignable_from_union_test", "isInstanceofNotUndefined", undefinedRef);
    ASSERT_NE(isInstanceofNotUndefined, ANI_TRUE);

    auto isInstanceofNull =
        CallEtsFunction<ani_boolean>("is_assignable_from_union_test", "isInstanceofNull", undefinedRef);
    ASSERT_NE(isInstanceofNull, ANI_TRUE);

    auto isInstanceofObject =
        CallEtsFunction<ani_boolean>("is_assignable_from_union_test", "isInstanceofObject", undefinedRef);
    ASSERT_NE(isInstanceofObject, ANI_TRUE);
}

TEST_F(IsAssignableFromUnionTest, is_instanceof_null)
{
    ani_ref nullRef = nullptr;
    ani_boolean isNull = ANI_FALSE;
    ASSERT_EQ(env_->GetNull(&nullRef), ANI_OK);
    ASSERT_EQ(env_->Reference_IsNull(nullRef, &isNull), ANI_OK);
    ASSERT_EQ(isNull, ANI_TRUE);

    auto isInstanceofNull = CallEtsFunction<ani_boolean>("is_assignable_from_union_test", "isInstanceofNull", nullRef);
    ASSERT_EQ(isInstanceofNull, ANI_TRUE);
}

TEST_F(IsAssignableFromUnionTest, not_instanceof_null)
{
    ani_ref nullRef = nullptr;
    ani_boolean isNull = ANI_FALSE;
    ASSERT_EQ(env_->GetNull(&nullRef), ANI_OK);
    ASSERT_EQ(env_->Reference_IsNull(nullRef, &isNull), ANI_OK);
    ASSERT_EQ(isNull, ANI_TRUE);

    auto isInstanceofNotNull =
        CallEtsFunction<ani_boolean>("is_assignable_from_union_test", "isInstanceofNotNull", nullRef);
    ASSERT_NE(isInstanceofNotNull, ANI_TRUE);

    auto isInstanceofUndefined =
        CallEtsFunction<ani_boolean>("is_assignable_from_union_test", "isInstanceofUndefined", nullRef);
    ASSERT_NE(isInstanceofUndefined, ANI_TRUE);

    auto isInstanceofObject =
        CallEtsFunction<ani_boolean>("is_assignable_from_union_test", "isInstanceofObject", nullRef);
    ASSERT_NE(isInstanceofObject, ANI_TRUE);
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
