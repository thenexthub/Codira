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

#include <ani.h>
#include <array>
#include <string>

#include "api/ani_textdecoder.h"
#include "api/ani_textencoder.h"
#include "api/ani_stringdecoder.h"
#include "api/ani_xmlpullparser.h"
#include "api/Util.h"
#include "ohos/init_data.h"
#include "tools/format_logger.h"
#include "stdlib/native/core/stdlib_ani_helpers.h"

using TextDecoder = ark::ets::sdk::util::TextDecoder;
using StringDecoder = ark::ets::sdk::util::StringDecoder;
using XmlPullParser = ark::ets::sdk::util::XmlPullParser;

struct ArrayBufferInfo {
    void *data;
    size_t length;
    ani_status retCode;
};

static ArrayBufferInfo GetArrayInfo([[maybe_unused]] ani_env *env, ani_arraybuffer buffer)
{
    void *data;
    size_t length;
    ani_status retCode = env->ArrayBuffer_GetInfo(buffer, &data, &length);
    return {data, length, retCode};
}

static char *GetUint8ArrayInfo(ani_env *env, ani_object array, int32_t &byteLength, int32_t &byteOffset)
{
    ani_ref buffer;
    if (auto retCode = env->Object_GetFieldByName_Ref(array, "buffer", &buffer); retCode != ANI_OK) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        LOG_ERROR_SDK("TextDecoder:: env->Object_GetFieldByName_Ref() failed");
        return nullptr;
    }
    auto bufferInfo = GetArrayInfo(env, static_cast<ani_arraybuffer>(buffer));
    if (bufferInfo.retCode != ANI_OK) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        LOG_ERROR_SDK("TextDecoder:: env->ArrayBuffer_GetInfo() failed");
        return nullptr;
    }

    std::vector<int32_t> vec;
    for (const char *propName : {"byteLength", "byteOffset"}) {
        ani_int value;
        std::string fieldName = std::string {propName} + "Int";
        ani_status retCode = env->Object_GetFieldByName_Int(array, fieldName.c_str(), &value);
        if (retCode != ANI_OK) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            LOG_ERROR_SDK("TextDecoder:: env->Object_GetFieldByName_Int fieldName: %{public}s", fieldName.c_str());
            return nullptr;
        }
        vec.push_back(value);
    }
    byteLength = vec[0];
    byteOffset = vec[1];
    return reinterpret_cast<char *>(bufferInfo.data);
}

static TextDecoder *Unwrapp(ani_env *env, ani_object object)
{
    ani_long textDecoder;
    if (ANI_OK != env->Object_GetFieldByName_Long(object, "nativeDecoder_", &textDecoder)) {
        return nullptr;
    }
    return reinterpret_cast<TextDecoder *>(textDecoder);
}

static ani_string Decode(ani_env *env, ani_object object, ani_object typedArray, ani_boolean stream)
{
    int32_t byteLength;
    int32_t byteOffset;
    const char *data = GetUint8ArrayInfo(env, typedArray, byteLength, byteOffset);
    bool iflag = (stream == ANI_TRUE);
    auto textDecoder = Unwrapp(env, object);
    if (textDecoder != nullptr) {
        // NOLINTNEXTLINE(clang-analyzer-core.CallAndMessage)
        return textDecoder->DecodeToString(env, data, byteOffset, byteLength, iflag);
    }
    return nullptr;
}

static void NativeDestroy([[maybe_unused]] ani_env *env, [[maybe_unused]] ani_object object, ani_long textDecoderPtr)
{
    auto decodedPtr = reinterpret_cast<TextDecoder *>(textDecoderPtr);
    delete decodedPtr;
}

static void BindNativeDecoder(ani_env *env, ani_object object, ani_string aniEncoding, ani_int flags)
{
    std::string stringEncoding = ark::ets::stdlib::ConvertFromAniString(env, aniEncoding);
    auto nativeTextDecoder = new TextDecoder(stringEncoding, flags);
    env->Object_SetFieldByName_Long(object, "nativeDecoder_", reinterpret_cast<ani_long>(nativeTextDecoder));
}

static ani_status BindTextDecoder(ani_env *env)
{
    const char *className = "@ohos.util.util.TextDecoder";
    ani_class cls;
    if (ANI_OK != env->FindClass(className, &cls)) {
        std::cerr << "Not found '" << className << "'" << std::endl;
        return ANI_ERROR;
    }

    std::array methods = {
        ani_native_function {"bindNativeDecoder", "C{std.core.String}i:", reinterpret_cast<void *>(BindNativeDecoder)},
        ani_native_function {"decode", "C{escompat.Uint8Array}z:C{std.core.String}", reinterpret_cast<void *>(Decode)},
    };
    if (ANI_OK != env->Class_BindNativeMethods(cls, methods.data(), methods.size())) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        LOG_ERROR_SDK("TextDecoder:: Cannot bind native methods to className : %{public}s", className);
        return ANI_ERROR;
    }

    std::array staticMethods = {
        ani_native_function {"nativeDestroy", "l:", reinterpret_cast<void *>(NativeDestroy)},
    };
    if (ANI_OK != env->Class_BindStaticNativeMethods(cls, staticMethods.data(), staticMethods.size())) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        LOG_ERROR_SDK("TextDecoder:: Cannot bind static native methods to className : %{public}s", className);
        return ANI_ERROR;
    }
    return ANI_OK;
}

[[maybe_unused]] static ani_status BindTextEncoder(ani_env *env)
{
    ani_class cls;
    const char *className = "@ohos.util.util.TextEncoder";
    if (ANI_OK != env->FindClass(className, &cls)) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        LOG_ERROR_SDK("TextEncoder:: Not found %{public}s", className);
        return ANI_ERROR;
    }
    std::array barMethods = {
        ani_native_function {
            "doEncodeInto",
            "C{std.core.String}C{std.core.String}:C{std.core.ArrayBuffer}",
            reinterpret_cast<void *>(ark::ets::sdk::util::DoEncodeInto),
        },
        ani_native_function {
            "doEncodeInfoUint8Array",
            "C{std.core.String}C{std.core.String}C{escompat.Uint8Array}:C{std.core.Array}",
            reinterpret_cast<void *>(ark::ets::sdk::util::DoEncodeIntoUint8Array),
        },
    };
    if (ANI_OK != env->Class_BindStaticNativeMethods(cls, barMethods.data(), barMethods.size())) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        LOG_ERROR_SDK("TextEncoder:: Cannot static bind native methods to %{public}s", className);
        return ANI_ERROR;
    }
    SetOhosIcuDirectory();
    return ANI_OK;
}

static ani_status BindUtilHelper(ani_env *env)
{
    const char *className = "@ohos.util.util.UtilHelper";
    ani_class cls;
    if (ANI_OK != env->FindClass(className, &cls)) {
        auto msg = std::string("Cannot find \"") + className + std::string("\" class!");
        ark::ets::stdlib::ThrowNewError(env, "std.core.RuntimeError", msg.data(), "C{std.core.String}:");
        return ANI_ERROR;
    }

    const auto methods = std::array {
        ani_native_function {"generateRandomUUID", "z:C{std.core.String}",
                             reinterpret_cast<void *>(ark::ets::sdk::util::ETSApiUtilHelperGenerateRandomUUID)},
        ani_native_function {"generateRandomBinaryUUID", "z:C{escompat.Uint8Array}",
                             reinterpret_cast<void *>(ark::ets::sdk::util::ETSApiUtilHelperGenerateRandomBinaryUUID)},
        ani_native_function {"getErrorString", "i:C{std.core.String}",
                             reinterpret_cast<void *>(ark::ets::sdk::util::ETSApiUtilHelperGetErrorString)}};

    if (ANI_OK != env->Class_BindStaticNativeMethods(cls, methods.data(), methods.size())) {
        std::cerr << "Cannot bind static native methods to '" << className << "'" << std::endl;
        return ANI_ERROR;
    };
    return ANI_OK;
}

static ani_boolean AniUtilsIsUndefined(ani_env *env, ani_object aniObj)
{
    ani_boolean isUndefined = ANI_TRUE;
    env->Reference_IsUndefined(aniObj, &isUndefined);
    return isUndefined;
}

static ani_boolean AniUtilsIsTypeArray(ani_env *env, ani_object aniObj)
{
    ani_class cls {};
    env->FindClass("escompat.Uint8Array", &cls);
    ani_boolean isTypeArray = ANI_FALSE;
    env->Object_InstanceOf(aniObj, cls, &isTypeArray);
    return isTypeArray;
}

static void BindNativeStringDecoder(ani_env *env, ani_object object, ani_string aniEncoding)
{
    std::string stringEncoding = ark::ets::stdlib::ConvertFromAniString(env, aniEncoding);
    auto nativeStringDecoder = new StringDecoder(stringEncoding);
    env->Object_SetFieldByName_Long(object, "nativeDecoder_", reinterpret_cast<ani_long>(nativeStringDecoder));
}

static ani_string DoWrite(ani_env *env, ani_object object, ani_object typedArray)
{
    int32_t byteLength;
    int32_t byteOffset;
    const char *data = GetUint8ArrayInfo(env, typedArray, byteLength, byteOffset);
    auto stringDecoder = StringDecoder::Unwrapp(env, object);
    if (stringDecoder != nullptr) {
        // NOLINTNEXTLINE(clang-analyzer-core.CallAndMessage)
        return stringDecoder->Write(env, data, byteOffset, byteLength);
    }
    return nullptr;
}

static ani_string DoEnd(ani_env *env, ani_object object, ani_object typedArray)
{
    ani_boolean isUndefined = AniUtilsIsUndefined(env, typedArray);
    if (isUndefined == ANI_TRUE) {
        auto stringDecoder = StringDecoder::Unwrapp(env, object);
        if (stringDecoder != nullptr) {
            // NOLINTNEXTLINE(clang-analyzer-core.CallAndMessage)
            return stringDecoder->End(env);
        }
        return nullptr;
    }

    ani_boolean isTypeArray = AniUtilsIsTypeArray(env, typedArray);
    if (isTypeArray == ANI_FALSE) {
        return nullptr;
    }
    int32_t byteLength;
    int32_t byteOffset;
    const char *data = GetUint8ArrayInfo(env, typedArray, byteLength, byteOffset);
    auto stringDecoder = StringDecoder::Unwrapp(env, object);
    if (stringDecoder != nullptr) {
        // NOLINTNEXTLINE(clang-analyzer-core.CallAndMessage)
        return stringDecoder->End(env, data, byteOffset, byteLength);
    }
    return nullptr;
}

static ani_status BindStringDecoder(ani_env *env)
{
    ani_class cls;
    const char *className = "@ohos.util.util.StringDecoder";
    if (ANI_OK != env->FindClass(className, &cls)) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        LOG_ERROR_SDK("StringDecoder:: Not found %{public}s", className);
        return ANI_ERROR;
    }
    std::array methods = {
        ani_native_function {"bindNativeStringDecoder",
                             "C{std.core.String}:", reinterpret_cast<void *>(BindNativeStringDecoder)},
        ani_native_function {"doWrite", "C{escompat.Uint8Array}:C{std.core.String}", reinterpret_cast<void *>(DoWrite)},
        ani_native_function {"doEnd", "C{escompat.Uint8Array}:C{std.core.String}", reinterpret_cast<void *>(DoEnd)},
    };
    if (ANI_OK != env->Class_BindNativeMethods(cls, methods.data(), methods.size())) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        LOG_ERROR_SDK("StringDecoder:: Cannot bind native methods to %{public}s", className);
        return ANI_ERROR;
    }
    return ANI_OK;
}

static ani_status BindXmlPullParser(ani_env *env)
{
    ani_class cls;
    const char *className = "@ohos.xml.xml.XmlParseHelper";
    if (ANI_OK != env->FindClass(className, &cls)) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        LOG_ERROR_SDK("%{public}s:: Not found %{public}s", __FUNCTION__, className);
        return ANI_ERROR;
    }
    std::array methods = {
        ani_native_function {"bindNative", "C{std.core.String}:l", reinterpret_cast<void *>(XmlPullParser::BindNative)},
        ani_native_function {"releaseNative", "l:z", reinterpret_cast<void *>(XmlPullParser::ReleaseNative)},
        ani_native_function {"parseXmlInner", "lzzC{std.core.Function2}C{std.core.Function2}C{std.core.Function2}:z",
                             reinterpret_cast<void *>(XmlPullParser::ParseXml)},
        ani_native_function {"parserErrorInfo", "l:C{std.core.String}",
                             reinterpret_cast<void *>(XmlPullParser::GetParseError)},
    };
    if (ANI_OK != env->Class_BindStaticNativeMethods(cls, methods.data(), methods.size())) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        LOG_ERROR_SDK("%{public}s:: Cannot bind native methods to %{public}s", __FUNCTION__, className);
        return ANI_ERROR;
    }
    return ANI_OK;
}

extern "C" ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    ani_env *env;
    if (ANI_OK != vm->GetEnv(ANI_VERSION_1, &env)) {
        std::cerr << "Unsupported ANI_VERSION_1" << std::endl;
        return ANI_ERROR;
    }
    // NOLINTNEXTLINE(hicpp-signed-bitwise,-warnings-as-errors)
    auto status = static_cast<ani_status>(BindUtilHelper(env) | BindTextDecoder(env) | BindTextEncoder(env) |
                                          BindStringDecoder(env) | BindXmlPullParser(env));
    if (status != ANI_OK) {
        return ANI_ERROR;
    }
    *result = ANI_VERSION_1;
    return ANI_OK;
}
