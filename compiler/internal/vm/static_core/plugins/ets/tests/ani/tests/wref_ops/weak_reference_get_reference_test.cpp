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
#include "plugins/ets/runtime/ets_vm.h"

namespace ark::ets::ani::testing {

class WeakReferenceGetReferenceTest : public AniTest {};

TEST_F(WeakReferenceGetReferenceTest, from_null_ref)
{
    ani_ref nullRef;
    ASSERT_EQ(env_->GetNull(&nullRef), ANI_OK);
    ani_wref wref;
    ASSERT_EQ(env_->WeakReference_Create(nullRef, &wref), ANI_OK);

    ani_ref ref;
    ani_boolean wasReleased;
    ASSERT_EQ(env_->WeakReference_GetReference(wref, &wasReleased, &ref), ANI_OK);
    if (wasReleased == ANI_FALSE) {
        ani_boolean isStrictEquals;
        ASSERT_EQ(env_->Reference_StrictEquals(nullRef, ref, &isStrictEquals), ANI_OK);
        ASSERT_TRUE(isStrictEquals);
    }
}

TEST_F(WeakReferenceGetReferenceTest, from_undefined_ref)
{
    ani_ref undefinedRef;
    ASSERT_EQ(env_->GetUndefined(&undefinedRef), ANI_OK);
    ani_wref wref;
    ASSERT_EQ(env_->WeakReference_Create(undefinedRef, &wref), ANI_OK);

    ani_ref ref;
    ani_boolean wasReleased;
    ASSERT_EQ(env_->WeakReference_GetReference(wref, &wasReleased, &ref), ANI_OK);
    if (wasReleased == ANI_FALSE) {
        ani_boolean isStrictEquals;
        ASSERT_EQ(env_->Reference_StrictEquals(undefinedRef, ref, &isStrictEquals), ANI_OK);
        ASSERT_TRUE(isStrictEquals);
    }
}

TEST_F(WeakReferenceGetReferenceTest, from_object_ref)
{
    ani_ref objectRef;
    ASSERT_EQ(env_->String_NewUTF8("x", 1, reinterpret_cast<ani_string *>(&objectRef)), ANI_OK);
    ani_wref wref;
    ASSERT_EQ(env_->WeakReference_Create(objectRef, &wref), ANI_OK);

    ani_ref ref;
    ani_boolean wasReleased;
    ASSERT_EQ(env_->WeakReference_GetReference(wref, &wasReleased, &ref), ANI_OK);
    if (wasReleased == ANI_FALSE) {
        ani_boolean isStrictEquals;
        ASSERT_EQ(env_->Reference_StrictEquals(objectRef, ref, &isStrictEquals), ANI_OK);
        ASSERT_TRUE(isStrictEquals);
    }
}

TEST_F(WeakReferenceGetReferenceTest, freed_wref)
{
    ani_ref objectRef;
    ASSERT_EQ(env_->String_NewUTF8("x", 1, reinterpret_cast<ani_string *>(&objectRef)), ANI_OK);
    ani_wref wref;
    ASSERT_EQ(env_->WeakReference_Create(objectRef, &wref), ANI_OK);

    // 1. Delete local ref
    // 2. Collect referent (local ref) from weak reference
    ASSERT_EQ(env_->Reference_Delete(objectRef), ANI_OK);
    PandaEtsVM::GetCurrent()->GetGC()->WaitForGC(GCTask(GCTaskCause::OOM_CAUSE));

    // Check that the referent has been released
    ani_ref ref;
    ani_boolean wasReleased;
    ASSERT_EQ(env_->WeakReference_GetReference(wref, &wasReleased, &ref), ANI_OK);
    ASSERT_TRUE(wasReleased);
}

TEST_F(WeakReferenceGetReferenceTest, from_object_gref)
{
    ani_ref objectRef;
    ASSERT_EQ(env_->String_NewUTF8("x", 1, reinterpret_cast<ani_string *>(&objectRef)), ANI_OK);
    ani_ref objectGRef;
    ASSERT_EQ(env_->GlobalReference_Create(objectRef, &objectGRef), ANI_OK);
    ani_wref wref;
    ASSERT_EQ(env_->WeakReference_Create(objectGRef, &wref), ANI_OK);

    ani_ref ref;
    ani_boolean wasReleased;
    ASSERT_EQ(env_->WeakReference_GetReference(wref, &wasReleased, &ref), ANI_OK);
    if (wasReleased == ANI_FALSE) {
        ani_boolean isStrictEquals;
        ASSERT_EQ(env_->Reference_StrictEquals(objectRef, ref, &isStrictEquals), ANI_OK);
        ASSERT_TRUE(isStrictEquals);
    }
}

TEST_F(WeakReferenceGetReferenceTest, invalid_was_reslease_result)
{
    ani_ref objectRef;
    ASSERT_EQ(env_->String_NewUTF8("x", 1, reinterpret_cast<ani_string *>(&objectRef)), ANI_OK);
    ani_wref wref;
    ASSERT_EQ(env_->WeakReference_Create(objectRef, &wref), ANI_OK);

    ani_ref ref;
    ASSERT_EQ(env_->WeakReference_GetReference(wref, nullptr, &ref), ANI_INVALID_ARGS);
}

TEST_F(WeakReferenceGetReferenceTest, invalid_ref_result)
{
    ani_ref objectRef;
    ASSERT_EQ(env_->String_NewUTF8("x", 1, reinterpret_cast<ani_string *>(&objectRef)), ANI_OK);
    ani_wref wref;
    ASSERT_EQ(env_->WeakReference_Create(objectRef, &wref), ANI_OK);

    ani_boolean wasReleased;
    ASSERT_EQ(env_->WeakReference_GetReference(wref, &wasReleased, nullptr), ANI_INVALID_ARGS);
}

TEST_F(WeakReferenceGetReferenceTest, invalid_ref_env)
{
    ani_ref undefinedRef;
    ASSERT_EQ(env_->GetUndefined(&undefinedRef), ANI_OK);
    ani_wref wref {};
    ASSERT_EQ(env_->WeakReference_Create(undefinedRef, &wref), ANI_OK);

    ani_boolean wasReleased = ANI_FALSE;
    ani_ref ref {};
    ASSERT_EQ(env_->c_api->WeakReference_GetReference(nullptr, wref, &wasReleased, &ref), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing
