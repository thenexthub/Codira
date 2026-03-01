/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "RegExp.h"

#include "plugins/ets/stdlib/native/core/stdlib_ani_helpers.h"
#include "regexp_executor.h"

#include "runtime/coroutines/coroutine_scopes.h"
#include "plugins/ets/runtime/ani/ani_checkers.h"
#include "plugins/ets/runtime/ani/ani_interaction_api.h"
#include "plugins/ets/runtime/ani/ani_mangle.h"
#include "plugins/ets/runtime/ani/ani_type_info.h"
#include "plugins/ets/runtime/ani/scoped_objects_fix.h"
#include "plugins/ets/runtime/types/ets_box_primitive-inl.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/ets_napi_env.h"
#include "plugins/ets/runtime/ets_stubs-inl.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_array.h"
#include "plugins/ets/runtime/types/ets_escompat_array.h"
#include "plugins/ets/runtime/ets_handle_scope.h"

#include "ani.h"

#include <array>

#include <cstdint>

namespace ark::ets::stdlib {

namespace refs {
ani_class g_regexpClass;
ani_class g_regexpExecArrayClass;
ani_class g_regexpMatchArrayClass;
}  // namespace refs

namespace {

constexpr const char *LAST_INDEX_FIELD_NAME = "lastIndex";

constexpr const char *REGEXP_CLASS_NAME = "std.core.RegExp";
constexpr const char *EXEC_ARRAY_CLASS_NAME = "std.core.RegExpExecArray";
constexpr const char *MATCH_ARRAY_CLASS_NAME = "std.core.RegExpMatchArray";
constexpr const char *INDEX_FIELD_NAME = "index_";
constexpr const char *RESULT_FIELD_NAME = "resultRaw_";
constexpr const char *IS_CORRECT_FIELD_NAME = "isCorrect";
constexpr const char *GROUPS_FIELD_NAME = "groupsRaw_";

constexpr uint32_t WIDE_CHAR_SIZE = 2;
constexpr uint16_t UTF16_CONVERT_FIRST_BIT_MASK = 255U;
constexpr uint16_t UTF16_CONVERT_SECOND_BIT_SHIFT = 8U;

// NOTE(kparshukov): legacy, remove #20259
constexpr auto FLAG_GLOBAL = (1U << 0U);
constexpr auto FLAG_IGNORECASE = (1U << 1U);
constexpr auto FLAG_MULTILINE = (1U << 2U);
constexpr auto FLAG_DOTALL = (1U << 3U);
constexpr auto FLAG_UTF16 = (1U << 4U);
constexpr auto FLAG_STICKY = (1U << 5U);
constexpr auto FLAG_HASINDICES = (1U << 6U);
constexpr auto FLAG_EXT_UNICODE = (1U << 7U);

uint32_t CastToBitMask(ani_env *env, ani_string checkStr)
{
    uint32_t flagsBits = 0;
    uint32_t flagsBitsTemp = 0;
    for (const char &c : ConvertFromAniString(env, checkStr)) {
        switch (c) {
            case 'd':
                flagsBitsTemp = FLAG_HASINDICES;
                break;
            case 'g':
                flagsBitsTemp = FLAG_GLOBAL;
                break;
            case 'i':
                flagsBitsTemp = FLAG_IGNORECASE;
                break;
            case 'm':
                flagsBitsTemp = FLAG_MULTILINE;
                break;
            case 's':
                flagsBitsTemp = FLAG_DOTALL;
                break;
            case 'u':
                flagsBitsTemp = FLAG_UTF16;
                break;
            case 'v':
                flagsBitsTemp = FLAG_EXT_UNICODE;
                break;
            case 'y':
                flagsBitsTemp = FLAG_STICKY;
                break;
            default: {
                ThrowNewError(env, "std.core.IllegalArgumentError", "invalid regular expression flags");
                return 0;
            }
        }
        if ((flagsBits & flagsBitsTemp) != 0) {
            ThrowNewError(env, "std.core.IllegalArgumentError", "invalid regular expression flags");
            return 0;
        }
        flagsBits |= flagsBitsTemp;
    }
    return flagsBits;
}

bool IsUtf16(ani_env *env, ani_string str)
{
    ark::ets::ani::ScopedManagedCodeFix s(PandaEnv::FromAniEnv(env));
    auto internalString = s.ToInternalType(str);
    return internalString->IsUtf16();
}

std::vector<uint8_t> ExtractStringAsUtf16(ani_env *env, ani_string strANI, const bool isUtf16)
{
    std::vector<uint8_t> result;
    if (isUtf16) {
        ani_size bufferSize;
        ANI_FATAL_IF_ERROR(env->String_GetUTF16Size(strANI, &bufferSize));
        bufferSize++;
        if (bufferSize <= 1U) {
            return result;
        }
        auto *buffer = new uint16_t[bufferSize];
        if (buffer == nullptr) {
            ThrowNewError(env, "std.core.OutOfMemoryError", "Can't create buffer");
            return result;
        }
        ani_size strSize;
        ANI_FATAL_IF_ERROR(env->String_GetUTF16(strANI, buffer, bufferSize, &strSize));
        result.resize(bufferSize * WIDE_CHAR_SIZE + 1U);
        for (size_t i = 0U; i < bufferSize; i++) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            result[i * WIDE_CHAR_SIZE] = static_cast<uint8_t>(buffer[i] & UTF16_CONVERT_FIRST_BIT_MASK);
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            result[i * WIDE_CHAR_SIZE + 1] = static_cast<uint8_t>(buffer[i] >> UTF16_CONVERT_SECOND_BIT_SHIFT);
        }
        delete[] buffer;
    } else {
        ani_size bufferSize;
        ANI_FATAL_IF_ERROR(env->String_GetUTF8Size(strANI, &bufferSize));
        bufferSize++;
        if (bufferSize <= 1U) {
            return result;
        }
        char *buffer = new char[bufferSize];
        if (buffer == nullptr) {
            ThrowNewError(env, "std.core.OutOfMemoryError", "Can't create buffer");
            return result;
        }
        ani_size strSize;
        ANI_FATAL_IF_ERROR(env->String_GetUTF8(strANI, buffer, bufferSize, &strSize));
        result.resize(bufferSize * WIDE_CHAR_SIZE + 1U);
        for (size_t i = 0U; i < bufferSize; i++) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            result[i * WIDE_CHAR_SIZE] = static_cast<uint8_t>(buffer[i]);
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            result[i * WIDE_CHAR_SIZE + 1] = static_cast<uint8_t>(0U);
        }
        delete[] buffer;
    }
    result.emplace_back(0U);
    result.emplace_back(0U);
    return result;
}

std::vector<uint8_t> ExtractStringAsUtf8(ani_env *env, ani_string strANI)
{
    std::vector<uint8_t> result;
    ani_size bufferSize;
    ANI_FATAL_IF_ERROR(env->String_GetUTF8Size(strANI, &bufferSize));
    bufferSize++;
    if (bufferSize <= 1U) {
        return result;
    }
    char *buffer = new char[bufferSize];
    ani_size strSize;
    ANI_FATAL_IF_ERROR(env->String_GetUTF8(strANI, buffer, bufferSize, &strSize));
    const size_t vecLen = strSize + 1U;
    result.resize(vecLen);
    for (size_t i = 0U; i < vecLen; i++) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        result[i] = static_cast<uint8_t>(buffer[i]);
    }
    result.emplace_back(0U);
    delete[] buffer;
    return result;
}

std::vector<uint8_t> ExtractString(ani_env *env, ani_string strANI, const bool isUtf16, const bool asUtf16)
{
    ASSERT(!isUtf16 || asUtf16);
    if (asUtf16) {
        return ExtractStringAsUtf16(env, strANI, isUtf16);
    }
    return ExtractStringAsUtf8(env, strANI);
}

struct ExecData {
    ani_string pattern;
    ani_string input;
    ani_string flags;
    int32_t lastIndex;
    size_t patternSize;
    size_t inputSize;
    bool isUtf16Pattern;
    bool isUtf16Input;
    bool hasSlashU;
};

}  // namespace

static RegExpExecResult Execute(ani_env *env, const ExecData &data)
{
    auto re = EtsRegExp(env);
    re.SetFlags(ConvertFromAniString(env, data.flags));
    const bool isUtf16 = data.hasSlashU || data.isUtf16Input || data.isUtf16Pattern || re.IsUtf16();
    auto patternStr = ExtractString(env, data.pattern, data.isUtf16Pattern, isUtf16);
    auto inputStr = ExtractString(env, data.input, data.isUtf16Input, isUtf16);
    auto compiled = re.Compile(patternStr, isUtf16, data.patternSize);
    if (!compiled) {
        RegExpExecResult badResult;
        badResult.isSuccess = false;
        return badResult;
    }

    auto result = re.Execute(patternStr, inputStr, data.inputSize, data.lastIndex);
    re.Destroy();
    return result;
}

static void SetResultField(ani_env *env, ani_class regexpResultClass, ani_object regexpExecArray,
                           const std::vector<RegExpExecResult::IndexPair> &matches)
{
    if (matches.empty()) {
        return;
    }
    ani_field resultField;
    ANI_FATAL_IF_ERROR(env->Class_FindField(regexpResultClass, RESULT_FIELD_NAME, &resultField));

    std::string data;
    for (const auto &[index, endIndex] : matches) {
        data += std::to_string(index) + ',' + std::to_string(endIndex) + ',';
    }

    ani_string resultStr;
    ANI_FATAL_IF_ERROR(env->String_NewUTF8(data.c_str(), data.size() - 1U, &resultStr));
    ANI_FATAL_IF_ERROR(env->Object_SetField_Ref(regexpExecArray, resultField, static_cast<ani_ref>(resultStr)));
}

static void SetIsCorrectField(ani_env *env, ani_class regexpResultClass, ani_object regexpExecArrayObj, bool value)
{
    ani_field resultCorrectField;
    ANI_FATAL_IF_ERROR(env->Class_FindField(regexpResultClass, IS_CORRECT_FIELD_NAME, &resultCorrectField));
    ANI_FATAL_IF_ERROR(env->Object_SetField_Boolean(regexpExecArrayObj, resultCorrectField, value));
}

static void SetIndexField(ani_env *env, ani_class regexpResultClass, ani_object regexpExecArrayObj, uint32_t index)
{
    ani_field indexField;
    ANI_FATAL_IF_ERROR(env->Class_FindField(regexpResultClass, INDEX_FIELD_NAME, &indexField));
    ANI_FATAL_IF_ERROR(env->Object_SetField_Int(regexpExecArrayObj, indexField, static_cast<double>(index)));
}

static void SetLastIndexField(ani_env *env, ani_object regexp, ani_field lastIndexField, ani_int value)
{
    ANI_FATAL_IF_ERROR(env->Object_SetField_Int(regexp, lastIndexField, value));
}

static void SetGroupsField(ani_env *env, ani_class regexpResultClass, ani_object regexpExecArray,
                           const std::map<std::string, std::pair<int32_t, int32_t>> &groups)
{
    if (groups.empty()) {
        return;
    }
    ani_field groupsField;
    ANI_FATAL_IF_ERROR(env->Class_FindField(regexpResultClass, GROUPS_FIELD_NAME, &groupsField));
    std::string data;
    for (const auto &[key, value] : groups) {
        data += key + "," + std::to_string(value.first) + ',' + std::to_string(value.second) + ';';
    }

    ani_string groupsStr;
    ANI_FATAL_IF_ERROR(env->String_NewUTF8(data.c_str(), data.size() - 1U, &groupsStr));
    ANI_FATAL_IF_ERROR(env->Object_SetField_Ref(regexpExecArray, groupsField, static_cast<ani_ref>(groupsStr)));
}

static ani_object Compile([[maybe_unused]] ani_env **env, [[maybe_unused]] ani_object obj)
{
    // NOTE(kparshukov): remove #20259
    return obj;
}

static ani_object DoExec(ani_env *env, ani_object regexp, ani_class resultClass, ani_object resultObject,
                         ExecData execData)
{
    ani_field lastIndexField;
    ANI_FATAL_IF_ERROR(env->Class_FindField(refs::g_regexpClass, LAST_INDEX_FIELD_NAME, &lastIndexField));

    auto lastIdx = execData.lastIndex;
    const auto flagsBits = static_cast<uint8_t>(CastToBitMask(env, execData.flags));

    const bool global = (flagsBits & FLAG_GLOBAL) > 0;
    const bool sticky = (flagsBits & FLAG_STICKY) > 0;
    const bool globalOrSticky = global || sticky;
    if (!globalOrSticky || lastIdx < 0) {
        lastIdx = 0;
        execData.lastIndex = 0;
    }
    if (static_cast<size_t>(lastIdx) > execData.inputSize) {
        if (globalOrSticky) {
            SetLastIndexField(env, regexp, lastIndexField, 0);
        }
        SetIsCorrectField(env, resultClass, resultObject, false);
        return resultObject;
    }
    auto execResult = Execute(env, execData);
    if (!execResult.isSuccess) {
        if (globalOrSticky) {
            SetLastIndexField(env, regexp, lastIndexField, 0);
        }
        SetIsCorrectField(env, resultClass, resultObject, false);
        return resultObject;
    }

    if (globalOrSticky) {
        SetLastIndexField(env, regexp, lastIndexField, execResult.endIndex);
    }
    SetIsCorrectField(env, resultClass, resultObject, true);
    SetIndexField(env, resultClass, resultObject, execResult.index);
    SetResultField(env, resultClass, resultObject, execResult.indices);
    SetGroupsField(env, resultClass, resultObject, execResult.namedGroups);

    return resultObject;
}

// CC-OFFNXT(G.FUN.01, huge_method) solid logic
static ani_object Exec(ani_env *env, ani_object regexp, ani_string pattern, ani_string flags, ani_string str,
                       ani_int patternSize, ani_int strSize, ani_int lastIndex, ani_boolean hasSlashU)
{
    ani_method regexpExecArrayCtor;
    ANI_FATAL_IF_ERROR(env->Class_FindMethod(refs::g_regexpExecArrayClass, "<ctor>", ":", &regexpExecArrayCtor));
    ani_object regexpExecArrayObject;

    const bool isUtf16Pattern = IsUtf16(env, pattern);
    const bool isUtf16Input = IsUtf16(env, str);
    ExecData execData {pattern,
                       str,
                       flags,
                       static_cast<int32_t>(lastIndex),
                       static_cast<size_t>(patternSize),
                       static_cast<size_t>(strSize),
                       isUtf16Pattern,
                       isUtf16Input,
                       static_cast<bool>(hasSlashU)};

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ANI_FATAL_IF_ERROR(env->Object_New(refs::g_regexpExecArrayClass, regexpExecArrayCtor, &regexpExecArrayObject));
    return DoExec(env, regexp, refs::g_regexpExecArrayClass, regexpExecArrayObject, execData);
}

// CC-OFFNXT(G.FUN.01, huge_method) solid logic
static ani_object Match(ani_env *env, ani_object regexp, ani_string pattern, ani_string flags, ani_string str,
                        ani_int patternSize, ani_int strSize, ani_int lastIndex, ani_boolean hasSlashU)
{
    ani_method regexpMatchArrayCtor;
    ANI_FATAL_IF_ERROR(env->Class_FindMethod(refs::g_regexpMatchArrayClass, "<ctor>", ":", &regexpMatchArrayCtor));
    ani_object regexpMatchArrayObject;

    const bool isUtf16Pattern = IsUtf16(env, pattern);
    const bool isUtf16Input = IsUtf16(env, str);
    ExecData execData {pattern,
                       str,
                       flags,
                       static_cast<int32_t>(lastIndex),
                       static_cast<size_t>(patternSize),
                       static_cast<size_t>(strSize),
                       isUtf16Pattern,
                       isUtf16Input,
                       static_cast<bool>(hasSlashU)};

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ANI_FATAL_IF_ERROR(env->Object_New(refs::g_regexpMatchArrayClass, regexpMatchArrayCtor, &regexpMatchArrayObject));
    return DoExec(env, regexp, refs::g_regexpMatchArrayClass, regexpMatchArrayObject, execData);
}

void RegisterRegExpNativeMethods(ani_env *env)
{
    const auto regExpImpls = std::array {
        ani_native_function {"compile", ":C{std.core.RegExp}", reinterpret_cast<void *>(Compile)},
        ani_native_function {"execImpl",
                             "C{std.core.String}C{std.core.String}C{std.core.String}iiiz:C{std.core.RegExpExecArray}",
                             reinterpret_cast<void *>(Exec)},
        ani_native_function {"matchImpl",
                             "C{std.core.String}C{std.core.String}C{std.core.String}iiiz:C{std.core.RegExpMatchArray}",
                             reinterpret_cast<void *>(Match)}};

    ani_class regexpClassLocal;
    ANI_FATAL_IF_ERROR(env->FindClass(REGEXP_CLASS_NAME, &regexpClassLocal));
    ANI_FATAL_IF_ERROR(
        env->GlobalReference_Create(regexpClassLocal, reinterpret_cast<ani_ref *>(&refs::g_regexpClass)));
    ANI_FATAL_IF_ERROR(env->Class_BindNativeMethods(refs::g_regexpClass, regExpImpls.data(), regExpImpls.size()));

    ani_class regexpExecArrayClassLocal;
    ANI_FATAL_IF_ERROR(env->FindClass(EXEC_ARRAY_CLASS_NAME, &regexpExecArrayClassLocal));
    ANI_FATAL_IF_ERROR(env->GlobalReference_Create(regexpExecArrayClassLocal,
                                                   reinterpret_cast<ani_ref *>(&refs::g_regexpExecArrayClass)));

    ani_class regexpMatchArrayClassLocal;
    ANI_FATAL_IF_ERROR(env->FindClass(MATCH_ARRAY_CLASS_NAME, &regexpMatchArrayClassLocal));
    ANI_FATAL_IF_ERROR(env->GlobalReference_Create(regexpMatchArrayClassLocal,
                                                   reinterpret_cast<ani_ref *>(&refs::g_regexpMatchArrayClass)));
}

}  // namespace ark::ets::stdlib
