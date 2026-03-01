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

#include <string_view>
#include "ani_gtest.h"

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
namespace ark::ets::ani::testing {

class FinalizationRegistryTest : public AniTest {
public:
    using NativeCallback = void (*)(ani_env *, ani_object);

public:
    static std::string GetModulePrefix()
    {
        return "";
    }

    void CreateFinalizationRegistry(NativeCallback cb, ani_object *finReg, ani_method *registerMethod)
    {
        ASSERT_NE(finReg, nullptr);
        ASSERT_NE(registerMethod, nullptr);

        ani_class finRegClass = nullptr;
        ASSERT_EQ(env_->FindClass("std.core.FinalizationRegistry", &finRegClass), ANI_OK);
        std::string_view registerSignature = "C{std.core.Object}C{std.core.Object}C{std.core.Object}:";
        ASSERT_EQ(env_->Class_FindMethod(finRegClass, "register", registerSignature.data(), registerMethod), ANI_OK);

        std::string_view ctorSignature = "C{std.core.Function1}:";
        ani_method ctor = nullptr;
        ASSERT_EQ(env_->Class_FindMethod(finRegClass, "<ctor>", ctorSignature.data(), &ctor), ANI_OK);

        ani_object managedCb = nullptr;
        CreateManagedCallback(cb, &managedCb);

        ASSERT_EQ(env_->Object_New(finRegClass, ctor, finReg, managedCb), ANI_OK);
    }

    void TriggerFullGC()
    {
        ASSERT_EQ(env_->Class_CallStaticMethod_Void(nativeCallbackClass_, doFullGcMethod_), ANI_OK);
    }

    ani_ref GetUndefinedRef()
    {
        return undefinedRef_;
    }

public:
    static constexpr const ani_size NUMBER_OF_REFS = 100;

protected:
    void SetUp() override
    {
        AniTest::SetUp();
        callbackClassName_ = "fin_reg_test." + GetModulePrefix() + "CallbackHolder";
        ASSERT_EQ(env_->FindClass(callbackClassName_.c_str(), &nativeCallbackClass_), ANI_OK);

        ani_native_function fn {"invoke", "C{std.core.Object}:", reinterpret_cast<void *>(NativeCallbackInvoke)};
        ASSERT_EQ(env_->Class_BindNativeMethods(nativeCallbackClass_, &fn, 1), ANI_OK);

        ASSERT_EQ(env_->Class_FindStaticMethod(nativeCallbackClass_, "doFullGC", ":", &doFullGcMethod_), ANI_OK);
        ASSERT_EQ(env_->Class_FindMethod(nativeCallbackClass_, "<ctor>", "l:", &nativeCallbackCtor_), ANI_OK);

        ASSERT_EQ(env_->GetUndefined(&undefinedRef_), ANI_OK);
    }

    std::vector<ani_option> GetExtraAniOptions() override
    {
        return {
            ani_option {"--ext:gc-type=g1-gc", nullptr},
            ani_option {"--ext:run-gc-in-place", nullptr},
            ani_option {"--ext:gc-trigger-type=debug-never", nullptr},
        };
    }

private:
    static void NativeCallbackInvoke(ani_env *env, ani_object self, ani_object cbArg)
    {
        ani_type selfType = nullptr;
        ASSERT_EQ(env->Object_GetType(self, &selfType), ANI_OK);
        ani_field cbPtrField = nullptr;
        ASSERT_EQ(env->Class_FindField(static_cast<ani_class>(selfType), "cbPtr", &cbPtrField), ANI_OK);
        ani_long cbPtr = 0;
        ASSERT_EQ(env->Object_GetField_Long(self, cbPtrField, &cbPtr), ANI_OK);
        ASSERT_NE(cbPtr, 0);
        reinterpret_cast<NativeCallback>(cbPtr)(env, cbArg);
    }

    void CreateManagedCallback(NativeCallback cb, ani_object *managedCb)
    {
        ASSERT_NE(managedCb, nullptr);

        ani_object nativeCb = nullptr;
        ASSERT_EQ(
            env_->Object_New(nativeCallbackClass_, nativeCallbackCtor_, &nativeCb, reinterpret_cast<ani_long>(cb)),
            ANI_OK);

        std::string factorySignature = "C{" + callbackClassName_ + "}:C{std.core.Function1}";
        std::string_view creatorName = "createCallbackWithArg";

        ani_ref createdManagedCb = nullptr;
        ASSERT_EQ(env_->Class_CallStaticMethodByName_Ref(nativeCallbackClass_, creatorName.data(),
                                                         factorySignature.c_str(), &createdManagedCb, nativeCb),
                  ANI_OK);
        *managedCb = static_cast<ani_object>(createdManagedCb);
    }

private:
    std::string callbackClassName_;
    ani_class nativeCallbackClass_ {nullptr};
    ani_static_method doFullGcMethod_ {nullptr};
    ani_method nativeCallbackCtor_ {nullptr};
    ani_ref undefinedRef_ {nullptr};
};

static std::atomic_size_t g_finalizationCounter = 0;

static void FinalizeWithGlobalCount([[maybe_unused]] ani_env *env, [[maybe_unused]] ani_object cbArg)
{
    // Atomic with seq_cst order reason: data race with `g_finalizationCounter` with requirement for sequentially
    // consistent order where threads observe all modifications in the same order
    g_finalizationCounter.fetch_add(1, std::memory_order_seq_cst);
}

TEST_F(FinalizationRegistryTest, test_native_finalizer)
{
    ani_object finReg = nullptr;
    ani_method registerMethod = nullptr;
    CreateFinalizationRegistry(FinalizeWithGlobalCount, &finReg, &registerMethod);

    ASSERT_EQ(env_->CreateLocalScope(NUMBER_OF_REFS), ANI_OK);

    std::string_view sampleString = "test sample string";
    for (ani_size i = 0; i < NUMBER_OF_REFS; ++i) {
        ani_string localStr = nullptr;
        ASSERT_EQ(env_->String_NewUTF8(sampleString.data(), sampleString.size(), &localStr), ANI_OK);
        // Register finalizer
        ASSERT_EQ(env_->Object_CallMethod_Void(finReg, registerMethod, localStr, GetUndefinedRef(), GetUndefinedRef()),
                  ANI_OK);
    }

    // Trigger GC and check that no finalizers were called
    TriggerFullGC();
    // Atomic with seq_cst order reason: data race with `g_finalizationCounter` with requirement for sequentially
    // consistent order where threads observe all modifications in the same order
    ASSERT_EQ(g_finalizationCounter.load(std::memory_order_seq_cst), 0);

    // Remove all created string references
    ASSERT_EQ(env_->DestroyLocalScope(), ANI_OK);

    // Trigger GC once again and check that finalizers were called
    TriggerFullGC();
    // Atomic with seq_cst order reason: data race with `g_finalizationCounter` with requirement for sequentially
    // consistent order where threads observe all modifications in the same order
    ASSERT_EQ(g_finalizationCounter.load(std::memory_order_seq_cst), NUMBER_OF_REFS);
}

static std::string GetFinalizationMarkerClassDescriptor()
{
    return "fin_reg_test." + FinalizationRegistryTest::GetModulePrefix() + "FinalizationMarker";
}

static void FinalizeWithManagedMark(ani_env *env, ani_object cbArg)
{
    auto className = GetFinalizationMarkerClassDescriptor();
    ani_class argClass = nullptr;
    ASSERT_EQ(env->FindClass(className.c_str(), &argClass), ANI_OK);
    ani_field field = nullptr;
    ASSERT_EQ(env->Class_FindField(argClass, "finalized", &field), ANI_OK);

    ani_boolean isCorrectObject = ANI_FALSE;
    ASSERT_EQ(env->Object_InstanceOf(cbArg, argClass, &isCorrectObject), ANI_OK);
    ASSERT_EQ(isCorrectObject, ANI_TRUE);

    ani_boolean value = ANI_FALSE;
    ASSERT_EQ(env->Object_GetField_Boolean(cbArg, field, &value), ANI_OK);
    // Check that finalizer is not called twice for same object
    ASSERT_EQ(value, ANI_FALSE);
    ASSERT_EQ(env->Object_SetField_Boolean(cbArg, field, ANI_TRUE), ANI_OK);
}

template <size_t N>
static void CheckMarkersArray(ani_env *env, const std::array<ani_ref, N> &finalizationMarkers, ani_class markerClass,
                              ani_boolean expected)
{
    ani_field field = nullptr;
    ASSERT_EQ(env->Class_FindField(markerClass, "finalized", &field), ANI_OK);
    for (auto ref : finalizationMarkers) {
        ani_boolean result = ANI_FALSE;
        ASSERT_EQ(env->Object_GetField_Boolean(static_cast<ani_object>(ref), field, &result), ANI_OK);
        ASSERT_EQ(result, expected);
    }
}

TEST_F(FinalizationRegistryTest, test_finalizer_with_managed_access)
{
    ani_object finReg = nullptr;
    ani_method registerMethod = nullptr;
    CreateFinalizationRegistry(FinalizeWithManagedMark, &finReg, &registerMethod);

    std::array<ani_ref, NUMBER_OF_REFS> finalizationMarkers {nullptr};
    auto markerClassDescriptor = GetFinalizationMarkerClassDescriptor();
    ani_class markerClass = nullptr;
    ASSERT_EQ(env_->FindClass(markerClassDescriptor.c_str(), &markerClass), ANI_OK);
    ani_method markerCtor = nullptr;
    ASSERT_EQ(env_->Class_FindMethod(markerClass, "<ctor>", ":", &markerCtor), ANI_OK);

    ASSERT_EQ(env_->CreateLocalScope(NUMBER_OF_REFS), ANI_OK);

    std::string_view sampleString = "test sample string";
    for (ani_size i = 0; i < NUMBER_OF_REFS; ++i) {
        ani_string localStr = nullptr;
        ASSERT_EQ(env_->String_NewUTF8(sampleString.data(), sampleString.size(), &localStr), ANI_OK);

        ani_object markerObject = nullptr;
        ASSERT_EQ(env_->Object_New(markerClass, markerCtor, &markerObject), ANI_OK);
        ani_ref globalRef = nullptr;
        ASSERT_EQ(env_->GlobalReference_Create(markerObject, &globalRef), ANI_OK);
        finalizationMarkers[i] = globalRef;
        // Register finalizer
        ASSERT_EQ(env_->Object_CallMethod_Void(finReg, registerMethod, localStr, markerObject, GetUndefinedRef()),
                  ANI_OK);
    }

    // Trigger GC and check that no finalizers were called
    TriggerFullGC();
    CheckMarkersArray(env_, finalizationMarkers, markerClass, ANI_FALSE);

    // Remove all created string references
    ASSERT_EQ(env_->DestroyLocalScope(), ANI_OK);

    // Trigger GC once again and check that finalizers were called
    TriggerFullGC();
    CheckMarkersArray(env_, finalizationMarkers, markerClass, ANI_TRUE);
}

}  // namespace ark::ets::ani::testing
// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
