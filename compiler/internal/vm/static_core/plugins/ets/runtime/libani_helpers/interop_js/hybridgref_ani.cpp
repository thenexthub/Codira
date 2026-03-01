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

#include "libarkbase/os/thread.h"
#include "hybridgref_ani.h"

namespace ark::ets::hygref {

using SharedRefIndex = uint32_t;

static ani_ref g_esValueClass;
static ani_ref g_doubleClass;
static ani_ref g_storagePropertyName;
static ani_ref g_lengthPropertyName;
static ani_static_method g_methodGetGlobal;
static ani_static_method g_methodGetUndefined;
static ani_method g_methodGetPropertyByName;
static ani_method g_methodGetPropertyByIndex;
static ani_method g_methodSetPropertyByName;
static ani_method g_methodSetPropertyByIndex;
static ani_method g_methodIsUndefined;
static ani_method g_methodIsNumber;
static ani_method g_methodToNumber;
static ani_method g_doubleCtor;

static bool CacheGlobalClass(ani_env *env, const char *descriptor, ani_ref *gref, ani_class *result)
{
    ani_class cls {};
    [[maybe_unused]] auto status = env->FindClass(descriptor, &cls);
    ASSERT(status == ANI_OK);
    *result = cls;
    status = env->GlobalReference_Create(cls, gref);
    if (UNLIKELY(status != ANI_OK)) {
        return false;
    }
    ASSERT(gref != nullptr);
    return true;
}

static bool CacheGlobalString(ani_env *env, std::string_view str, ani_ref *gref)
{
    ani_string aniStr {};
    auto status = env->String_NewUTF8(str.data(), str.size(), &aniStr);
    if (UNLIKELY(status != ANI_OK)) {
        return false;
    }
    return env->GlobalReference_Create(aniStr, gref) == ANI_OK;
}

/// @brief Initializes global variables, must be called once
static bool InitializeGlobal(ani_env *env)
{
    static constexpr std::string_view HYBRIDGREF_STORAGE_NAME = "__hybridgref_internal_storage__";
    static constexpr std::string_view LENGTH_PROPERTY_NAME = "length";

    ani_class esobjectClass {};
    if (UNLIKELY(!CacheGlobalClass(env, "std.interop.ESValue", &g_esValueClass, &esobjectClass))) {
        return false;
    }
    ani_class doubleClass {};
    if (UNLIKELY(!CacheGlobalClass(env, "std.core.Double", &g_doubleClass, &doubleClass))) {
        return false;
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    [[maybe_unused]] auto status = env->Class_FindMethod(doubleClass, "<ctor>", "d:", &g_doubleCtor);
    ASSERT(status == ANI_OK);

    status = env->Class_FindStaticMethod(esobjectClass, "getGlobal", ":C{std.interop.ESValue}", &g_methodGetGlobal);
    ASSERT(status == ANI_OK);
    status = env->Class_FindMethod(esobjectClass, "getPropertySafe", "C{std.core.String}:C{std.interop.ESValue}",
                                   &g_methodGetPropertyByName);
    ASSERT(status == ANI_OK);
    status = env->Class_FindMethod(esobjectClass, "getPropertySafe", "d:C{std.interop.ESValue}",
                                   &g_methodGetPropertyByIndex);
    ASSERT(status == ANI_OK);
    status =
        env->Class_FindMethod(esobjectClass, "setProperty",
                              "C{std.core.String}X{C{std.core.Null}C{std.core.Object}}:", &g_methodSetPropertyByName);
    ASSERT(status == ANI_OK);
    status = env->Class_FindMethod(esobjectClass, "setProperty",
                                   "dX{C{std.core.Null}C{std.core.Object}}:", &g_methodSetPropertyByIndex);
    ASSERT(status == ANI_OK);
    status =
        env->Class_FindStaticMethod(esobjectClass, "%%get-Undefined", ":C{std.interop.ESValue}", &g_methodGetUndefined);
    ASSERT(status == ANI_OK);
    status = env->Class_FindMethod(esobjectClass, "isUndefined", ":z", &g_methodIsUndefined);
    ASSERT(status == ANI_OK);
    status = env->Class_FindMethod(esobjectClass, "isNumber", ":z", &g_methodIsNumber);
    ASSERT(status == ANI_OK);
    status = env->Class_FindMethod(esobjectClass, "toNumber", ":d", &g_methodToNumber);
    ASSERT(status == ANI_OK);

    return CacheGlobalString(env, HYBRIDGREF_STORAGE_NAME, &g_storagePropertyName) &&
           CacheGlobalString(env, LENGTH_PROPERTY_NAME, &g_lengthPropertyName);
}

static bool ESValueToNumber(ani_env *env, ani_object esvalue, ani_double *result)
{
    ani_boolean isNumber = ANI_FALSE;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    [[maybe_unused]] auto status = env->Object_CallMethod_Boolean(esvalue, g_methodIsNumber, &isNumber);
    if (status != ANI_OK || isNumber == ANI_FALSE) {
        return false;
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    status = env->Object_CallMethod_Double(esvalue, g_methodToNumber, result);
    ASSERT(status == ANI_OK);
    return true;
}

static bool ESValueIsUndefined(ani_env *env, ani_object ref)
{
    ani_boolean isUndef = ANI_FALSE;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    env->Object_CallMethod_Boolean(ref, g_methodIsUndefined, &isUndef);
    return (isUndef == ANI_TRUE);
}

static bool CheckCorrectThread(ani_env *env, ani_object storage)
{
    ani_ref tidHolder {};
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    auto status = env->Object_CallMethod_Ref(storage, g_methodGetPropertyByIndex, &tidHolder, 0.0);
    if (UNLIKELY(status != ANI_OK || ESValueIsUndefined(env, static_cast<ani_object>(tidHolder)))) {
        return false;
    }

    ani_double tid = 0.0;
    if (UNLIKELY(!ESValueToNumber(env, static_cast<ani_object>(tidHolder), &tid))) {
        return false;
    }
    return static_cast<uint32_t>(tid) == ark::os::thread::GetCurrentThreadId();
}

static bool InitializeGlobalOnce(ani_env *env)
{
    // Guaranteed to be called once
    static bool isInitialized = InitializeGlobal(env);
    return isInitialized;
}

/// @brief Returns ESValue which contains storage array
static bool GetStorage(ani_env *env, ani_object *result)
{
    if (UNLIKELY(!InitializeGlobalOnce(env))) {
        return false;
    }
    // Initialization sanity check
    ASSERT(g_esValueClass != nullptr);

    ani_status status;
    ani_ref jsGlobal {};
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    if (status = env->Class_CallStaticMethod_Ref(static_cast<ani_class>(g_esValueClass), g_methodGetGlobal, &jsGlobal);
        status != ANI_OK || ESValueIsUndefined(env, static_cast<ani_object>(jsGlobal))) {
        return false;
    }
    ani_ref storage {};
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    status = env->Object_CallMethod_Ref(static_cast<ani_object>(jsGlobal), g_methodGetPropertyByName, &storage,
                                        static_cast<ani_string>(g_storagePropertyName));
    if (status != ANI_OK) {
        return false;
    }

    if (ESValueIsUndefined(env, static_cast<ani_object>(storage))) {
        ani_static_method instantiateEmptyArray {};
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        status = env->Class_FindStaticMethod(static_cast<ani_class>(g_esValueClass), "instantiateEmptyArray",
                                             ":C{std.interop.ESValue}", &instantiateEmptyArray);
        if (UNLIKELY(status != ANI_OK)) {
            return false;
        }
        status =
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            env->Class_CallStaticMethod_Ref(static_cast<ani_class>(g_esValueClass), instantiateEmptyArray, &storage);
        if (UNLIKELY(status != ANI_OK)) {
            return false;
        }
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        status = env->Object_CallMethod_Void(static_cast<ani_object>(jsGlobal), g_methodSetPropertyByName,
                                             g_storagePropertyName, static_cast<ani_object>(storage));
        if (UNLIKELY(status != ANI_OK)) {
            return false;
        }
        ani_object doubleObj {};
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        status = env->Object_New(static_cast<ani_class>(g_doubleClass), g_doubleCtor, &doubleObj,
                                 static_cast<ani_double>(ark::os::thread::GetCurrentThreadId()));
        if (UNLIKELY(status != ANI_OK)) {
            return false;
        }
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        status = env->Object_CallMethod_Void(static_cast<ani_object>(storage), g_methodSetPropertyByIndex,
                                             static_cast<ani_double>(0), doubleObj);
        if (UNLIKELY(status != ANI_OK)) {
            return false;
        }
    }

    *result = static_cast<ani_object>(storage);
    return true;
}

static bool IsValidStorage(ani_env *env, ani_object storage, ani_size *outLength)
{
    if (storage == nullptr || outLength == nullptr) {
        return false;
    }

    ani_ref lenRef {};
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    auto status = env->Object_CallMethod_Ref(storage, g_methodGetPropertyByName, &lenRef,
                                             static_cast<ani_string>(g_lengthPropertyName));
    if (status != ANI_OK || ESValueIsUndefined(env, static_cast<ani_object>(lenRef))) {
        return false;
    }

    ani_double len = 0;
    if (!ESValueToNumber(env, static_cast<ani_object>(lenRef), &len)) {
        return false;
    }

    *outLength = static_cast<ani_size>(len);
    return *outLength > 0;
}

bool HybridGrefGetESValue(ani_env *env, hybridgref ref, ani_object *result)
{
    if (env == nullptr || result == nullptr) {
        return false;
    }
    ani_object storage {};
    if (!GetStorage(env, &storage)) {
        return false;
    }
    ani_size arrayLength;
    if (!IsValidStorage(env, storage, &arrayLength)) {
        return false;
    }

    if (!CheckCorrectThread(env, storage)) {
        return false;
    }

    SharedRefIndex refIndex = reinterpret_cast<uintptr_t>(ref);
    if (refIndex == 0 || refIndex >= arrayLength) {
        return false;
    }
    ani_ref refHolder {};
    auto status =
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        env->Object_CallMethod_Ref(storage, g_methodGetPropertyByIndex, &refHolder, static_cast<ani_double>(refIndex));
    if (UNLIKELY(status != ANI_OK)) {
        return false;
    }
    if (ESValueIsUndefined(env, static_cast<ani_object>(refHolder))) {
        // Index not found
        return false;
    }

    *result = static_cast<ani_object>(refHolder);
    return true;
}

bool HybridGrefCreateRefFromAni(ani_env *env, ani_ref value, hybridgref *result)
{
    if (env == nullptr || result == nullptr) {
        return false;
    }
    ani_object storage {};
    if (!GetStorage(env, &storage)) {
        return false;
    }
    ani_size arrayLength;
    if (!IsValidStorage(env, storage, &arrayLength)) {
        return false;
    }
    auto status =
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        env->Object_CallMethod_Void(storage, g_methodSetPropertyByIndex, static_cast<ani_double>(arrayLength), value);
    if (status != ANI_OK) {
        return false;
    }
    *result = reinterpret_cast<hybridgref>(arrayLength);
    return true;
}

bool HybridGrefDeleteRefFromAni(ani_env *env, hybridgref ref)
{
    if (env == nullptr) {
        return false;
    }
    ani_object storage {};
    if (!GetStorage(env, &storage)) {
        return false;
    }
    ani_size arrayLength;
    if (!IsValidStorage(env, storage, &arrayLength)) {
        return false;
    }

    if (!CheckCorrectThread(env, storage)) {
        return false;
    }

    SharedRefIndex refIndex = reinterpret_cast<uintptr_t>(ref);
    if (refIndex == 0 || refIndex >= arrayLength) {
        return false;
    }
    ani_ref undefinedHolder {};
    auto status =
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        env->Class_CallStaticMethod_Ref(static_cast<ani_class>(g_esValueClass), g_methodGetUndefined, &undefinedHolder);
    if (UNLIKELY(status != ANI_OK)) {
        return false;
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    status = env->Object_CallMethod_Void(storage, g_methodSetPropertyByIndex, static_cast<ani_double>(refIndex),
                                         undefinedHolder);
    return status == ANI_OK;
}

}  // namespace ark::ets::hygref

// NOLINTBEGIN(readability-identifier-naming)
extern "C" bool hybridgref_get_esvalue(ani_env *env, hybridgref ref, ani_object *result)
{
    return ark::ets::hygref::HybridGrefGetESValue(env, ref, result);
}

extern "C" bool hybridgref_create_from_ani(ani_env *env, ani_ref value, hybridgref *result)
{
    return ark::ets::hygref::HybridGrefCreateRefFromAni(env, value, result);
}

extern "C" bool hybridgref_delete_from_ani(ani_env *env, hybridgref ref)
{
    return ark::ets::hygref::HybridGrefDeleteRefFromAni(env, ref);
}
// NOLINTEND(readability-identifier-naming)
