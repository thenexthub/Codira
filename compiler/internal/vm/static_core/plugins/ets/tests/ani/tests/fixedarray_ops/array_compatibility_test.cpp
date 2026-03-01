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

#include "ani/scoped_objects_fix.h"
#include "ani_gtest.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
namespace ark::ets::ani::testing {

class CompatibilityTest : public AniTest {};

TEST_F(CompatibilityTest, SetShortArrayRegionErrorTests)
{
    ani_fixedarray_int array;
    ASSERT_EQ(env_->FixedArray_New_Int(5U, &array), ANI_OK);

    const auto managedArray =
        static_cast<ani_fixedarray_int>(CallEtsFunction<ani_ref>("array_compatibility_test", "getArray"));
    const auto escompatArray =
        static_cast<ani_fixedarray_int>(CallEtsFunction<ani_ref>("array_compatibility_test", "getEscompatArray"));

    // Compare that int[] and FixedArray which is created in FixedArray_New_Int function is the same type
    // It would fail when frontend and spec would begin to match each other
    ScopedManagedCodeFix s(PandaEnv::FromAniEnv(env_));
    const EtsPlatformTypes *platformTypes = PlatformTypes(PandaEtsVM::GetCurrent());
    Class *arrayClass = platformTypes->escompatArray->GetRuntimeClass();
    EtsObject *objArray = s.ToInternalType(reinterpret_cast<ani_object>(array));
    EtsObject *managedObjArray = s.ToInternalType(reinterpret_cast<ani_object>(managedArray));
    EtsObject *escompatObjArray = s.ToInternalType(reinterpret_cast<ani_object>(escompatArray));

    ASSERT_NE(managedObjArray->GetClass()->GetRuntimeClass(), objArray->GetClass()->GetRuntimeClass());
    ASSERT_EQ(managedObjArray->GetClass()->GetRuntimeClass(), arrayClass);
    ASSERT_EQ(managedObjArray->GetClass()->GetRuntimeClass(), escompatObjArray->GetClass()->GetRuntimeClass());

    // Next asserts will not fail, but Array_New_Type implementation should be adapted and assert shoul be inversed
    ASSERT_NE(arrayClass, objArray->GetClass()->GetRuntimeClass());
    ASSERT_NE(objArray->GetClass()->GetRuntimeClass(), escompatObjArray->GetClass()->GetRuntimeClass());

    // Next assert should not change
    ASSERT_EQ(arrayClass, escompatObjArray->GetClass()->GetRuntimeClass());
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
