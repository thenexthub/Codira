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

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-identifier-naming)
namespace ark::ets::ani::testing {

class WeakReferenceCreateTest : public AniTest {};

TEST_F(WeakReferenceCreateTest, from_null_ref)
{
    ani_ref nullRef;
    ASSERT_EQ(env_->GetNull(&nullRef), ANI_OK);
    ani_wref wref;
    ASSERT_EQ(env_->WeakReference_Create(nullRef, &wref), ANI_OK);
}

TEST_F(WeakReferenceCreateTest, from_undefined_ref)
{
    ani_ref undefinedRef;
    ASSERT_EQ(env_->GetUndefined(&undefinedRef), ANI_OK);
    ani_wref wref;
    ASSERT_EQ(env_->WeakReference_Create(undefinedRef, &wref), ANI_OK);
}

TEST_F(WeakReferenceCreateTest, from_object_ref)
{
    ani_ref objectRef;
    ASSERT_EQ(env_->String_NewUTF8("x", 1, reinterpret_cast<ani_string *>(&objectRef)), ANI_OK);
    ani_wref wref;
    ASSERT_EQ(env_->WeakReference_Create(objectRef, &wref), ANI_OK);
}

TEST_F(WeakReferenceCreateTest, from_object_gref)
{
    ani_ref objectRef;
    ASSERT_EQ(env_->String_NewUTF8("x", 1, reinterpret_cast<ani_string *>(&objectRef)), ANI_OK);
    ani_ref objectGRef;
    ASSERT_EQ(env_->GlobalReference_Create(objectRef, &objectGRef), ANI_OK);

    ani_wref wref;
    ASSERT_EQ(env_->WeakReference_Create(objectGRef, &wref), ANI_OK);
}

TEST_F(WeakReferenceCreateTest, invalid_result)
{
    ani_ref objectRef;
    ASSERT_EQ(env_->String_NewUTF8("x", 1, reinterpret_cast<ani_string *>(&objectRef)), ANI_OK);
    ASSERT_EQ(env_->GlobalReference_Create(objectRef, nullptr), ANI_INVALID_ARGS);
}

TEST_F(WeakReferenceCreateTest, weak_reference_case1)
{
    ani_ref undefinedRef {};
    ASSERT_EQ(env_->GetUndefined(&undefinedRef), ANI_OK);
    ani_wref wref {};
    ASSERT_EQ(env_->WeakReference_Create(undefinedRef, &wref), ANI_OK);
}

TEST_F(WeakReferenceCreateTest, weak_reference_case2)
{
    constexpr std::string_view m = "Pure P60";
    const ani_int weight = 200;

    ani_class cls {};
    ASSERT_EQ(env_->FindClass("weak_reference_create_test.MobilePhone", &cls), ANI_OK);

    ani_method ctor {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "C{std.core.String}i:", &ctor), ANI_OK);

    ani_string model {};
    ASSERT_EQ(env_->String_NewUTF8(m.data(), m.size(), &model), ANI_OK);

    ani_object phone {};
    ASSERT_EQ(env_->Object_New(cls, ctor, &phone, model, weight), ANI_OK);

    auto objectRef = reinterpret_cast<ani_ref>(phone);
    ani_wref wrefa {};
    ASSERT_EQ(env_->WeakReference_Create(objectRef, &wrefa), ANI_OK);
    ani_wref wrefb {};
    ASSERT_EQ(env_->WeakReference_Create(objectRef, &wrefb), ANI_OK);
    ani_wref wrefc {};
    ASSERT_EQ(env_->WeakReference_Create(objectRef, &wrefc), ANI_OK);
    ani_wref wrefd {};
    ASSERT_EQ(env_->WeakReference_Create(objectRef, &wrefd), ANI_OK);
    ani_wref wrefe {};
    ASSERT_EQ(env_->WeakReference_Create(objectRef, &wrefe), ANI_OK);

    ASSERT_EQ(env_->WeakReference_Delete(wrefa), ANI_OK);
    ASSERT_EQ(env_->WeakReference_Delete(wrefb), ANI_OK);
    ASSERT_EQ(env_->WeakReference_Delete(wrefc), ANI_OK);

    ani_boolean wasReleased = ANI_FALSE;
    ani_ref ref {};
    ASSERT_EQ(env_->WeakReference_GetReference(wrefd, &wasReleased, &ref), ANI_OK);
    ASSERT_FALSE(wasReleased);
    ASSERT_EQ(env_->WeakReference_GetReference(wrefe, &wasReleased, &ref), ANI_OK);
    ASSERT_FALSE(wasReleased);

    ani_wref wreff {};
    ASSERT_EQ(env_->WeakReference_Create(objectRef, &wreff), ANI_OK);
}

TEST_F(WeakReferenceCreateTest, weak_reference_case3)
{
    ani_ref objectRef {};
    ASSERT_EQ(env_->String_NewUTF8("x", 1, reinterpret_cast<ani_string *>(&objectRef)), ANI_OK);

    ani_ref gref {};
    ASSERT_EQ(env_->GlobalReference_Create(objectRef, &gref), ANI_OK);

    ani_wref wref {};
    ASSERT_EQ(env_->WeakReference_Create(objectRef, &wref), ANI_OK);

    ani_boolean wasReleased;
    ani_ref ref {};
    ASSERT_EQ(env_->WeakReference_GetReference(wref, &wasReleased, &ref), ANI_OK);

    ani_boolean isEquals = ANI_FALSE;
    ASSERT_EQ(env_->Reference_Equals(gref, ref, &isEquals), ANI_OK);
    ASSERT_EQ(isEquals, ANI_TRUE);
}

TEST_F(WeakReferenceCreateTest, weak_reference_case4)
{
    auto refObjectA = CallEtsFunction<ani_ref>("weak_reference_create_test", "getObjectA");
    auto refObjectB = CallEtsFunction<ani_ref>("weak_reference_create_test", "getObjectB");

    ani_wref wrefa {};
    ASSERT_EQ(env_->WeakReference_Create(refObjectA, &wrefa), ANI_OK);

    ani_wref wrefb {};
    ASSERT_EQ(env_->WeakReference_Create(refObjectB, &wrefb), ANI_OK);

    ani_boolean wasReleased = ANI_FALSE;
    ani_ref refa {};
    ASSERT_EQ(env_->WeakReference_GetReference(wrefa, &wasReleased, &refa), ANI_OK);

    ani_ref refb {};
    ASSERT_EQ(env_->WeakReference_GetReference(wrefb, &wasReleased, &refb), ANI_OK);

    ani_boolean isEquals = ANI_FALSE;
    ASSERT_EQ(env_->Reference_Equals(refa, refb, &isEquals), ANI_OK);
    ASSERT_EQ(isEquals, ANI_TRUE);

    ASSERT_EQ(env_->Reference_StrictEquals(refa, refb, &isEquals), ANI_OK);
    ASSERT_EQ(isEquals, ANI_TRUE);
}

TEST_F(WeakReferenceCreateTest, weak_reference_case5)
{
    auto refObjectA = CallEtsFunction<ani_ref>("weak_reference_create_test", "getObjectA");
    ani_ref refObjectB {};
    ASSERT_EQ(env_->String_NewUTF8("x", 1, reinterpret_cast<ani_string *>(&refObjectB)), ANI_OK);

    ani_wref wrefa {};
    ASSERT_EQ(env_->WeakReference_Create(refObjectA, &wrefa), ANI_OK);

    ani_wref wrefb {};
    ASSERT_EQ(env_->WeakReference_Create(refObjectB, &wrefb), ANI_OK);

    ani_boolean wasReleased = ANI_FALSE;
    ani_ref refa {};
    ASSERT_EQ(env_->WeakReference_GetReference(wrefa, &wasReleased, &refa), ANI_OK);

    ani_ref refb {};
    ASSERT_EQ(env_->WeakReference_GetReference(wrefb, &wasReleased, &refb), ANI_OK);

    ani_boolean isEquals = ANI_FALSE;
    ASSERT_EQ(env_->Reference_Equals(refa, refb, &isEquals), ANI_OK);
    ASSERT_EQ(isEquals, ANI_FALSE);

    ASSERT_EQ(env_->Reference_StrictEquals(refa, refb, &isEquals), ANI_OK);
    ASSERT_EQ(isEquals, ANI_FALSE);
}

TEST_F(WeakReferenceCreateTest, weak_reference_case6)
{
    ani_ref objectRef {};
    std::string str = "test";
    ASSERT_EQ(env_->String_NewUTF8(str.c_str(), str.length(), reinterpret_cast<ani_string *>(&objectRef)), ANI_OK);

    ani_wref wrefa {};
    ASSERT_EQ(env_->WeakReference_Create(objectRef, &wrefa), ANI_OK);

    ani_wref wrefb {};
    ASSERT_EQ(env_->WeakReference_Create(objectRef, &wrefb), ANI_OK);

    ani_boolean wasReleased = ANI_FALSE;
    ani_ref refa {};
    ASSERT_EQ(env_->WeakReference_GetReference(wrefa, &wasReleased, &refa), ANI_OK);

    ani_ref refb {};
    ASSERT_EQ(env_->WeakReference_GetReference(wrefb, &wasReleased, &refb), ANI_OK);

    ani_boolean isEquals = ANI_FALSE;
    ASSERT_EQ(env_->Reference_Equals(refa, refb, &isEquals), ANI_OK);
    ASSERT_EQ(isEquals, ANI_TRUE);

    ASSERT_EQ(env_->Reference_StrictEquals(refa, refb, &isEquals), ANI_OK);
    ASSERT_EQ(isEquals, ANI_TRUE);
}

TEST_F(WeakReferenceCreateTest, invalid_env)
{
    ani_ref undefinedRef {};
    ASSERT_EQ(env_->GetUndefined(&undefinedRef), ANI_OK);
    ani_wref wref {};
    ASSERT_EQ(env_->c_api->WeakReference_Create(nullptr, undefinedRef, &wref), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-identifier-naming)