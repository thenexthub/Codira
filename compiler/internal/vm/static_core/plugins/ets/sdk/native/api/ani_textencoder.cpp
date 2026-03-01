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

#include <memory>
#include <securec.h>
#include <optional>
#include <vector>
#include "ani_textencoder.h"
#include "stdlib/native/core/stdlib_ani_helpers.h"

// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#define ANI_RETURN_NULLOPT_ON_FAILURE(retCode, ...) \
    if (ANI_OK != (retCode)) {                      \
        LOG_FATAL_SDK(__VA_ARGS__);                 \
        return std::nullopt;                        \
    }
#define ANI_RETURN_NULLPTR_ON_FAILURE(retCode, ...) \
    if (ANI_OK != (retCode)) {                      \
        LOG_FATAL_SDK(__VA_ARGS__);                 \
        return nullptr;                             \
    }
// NOLINTEND(cppcoreguidelines-macro-usage)

namespace ark::ets::sdk::util {
UConverterWrapper::UConverterWrapper(const char *encodingStr) : encoding(encodingStr)
{
    UErrorCode codeflag = U_ZERO_ERROR;
    converter = ucnv_open(encoding, &codeflag);
    if (U_FAILURE(codeflag) != 0) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        LOG_FATAL_SDK("ncnv_open failed with encoding '%s' and error '%s'.", encodingStr, u_errorName(codeflag));
        // converter is nullptr on failure
    }
}

std::optional<ArrayBufferInfos> GetBufferInfo(ani_env *env, ani_arraybuffer buffer)
{
    ArrayBufferInfos res {};
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ANI_RETURN_NULLOPT_ON_FAILURE(env->ArrayBuffer_GetInfo(buffer, &res.data, &res.length),
                                  "Internal failure: env->ArrayBuffer_GetInfo()");
    return res;
}

template <class... Args>
ani_object NewUint8Array(ani_env *env, const char *signature, Args... args)
{
    ani_class arrayClass;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ANI_RETURN_NULLPTR_ON_FAILURE(env->FindClass("escompat.Uint8Array", &arrayClass),
                                  "Internal failure: env->FindClass()");
    ani_method arrayCtor;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ANI_RETURN_NULLPTR_ON_FAILURE(env->Class_FindMethod(arrayClass, "<ctor>", signature, &arrayCtor),
                                  "Internal failure: env->Class_FindMethod() with signature %{public}s", signature);
    ani_object res;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ANI_RETURN_NULLPTR_ON_FAILURE(env->Object_New(arrayClass, arrayCtor, &res, args...),
                                  "Internal failure: env->Object_New()");
    return res;
}

template <class... Args>
std::optional<Uint8ArrayWithBufferInfo> NewUint8ArrayWithBufferInfo(ani_env *env, const char *signature, Args... args)
{
    ani_object res = NewUint8Array(env, signature, args...);
    if (res == nullptr) {
        return std::nullopt;
    }
    ani_ref buffer;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ANI_RETURN_NULLOPT_ON_FAILURE(env->Object_GetFieldByName_Ref(res, "buffer", &buffer),
                                  "Internal failure: env->Object_GetFieldByName_Ref() with field \"buffer\".");
    void *bufData;
    size_t bufLength;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ANI_RETURN_NULLOPT_ON_FAILURE(env->ArrayBuffer_GetInfo(static_cast<ani_arraybuffer>(buffer), &bufData, &bufLength),
                                  "Internal failure: env->ArrayBuffer_GetInfo()");
    return Uint8ArrayWithBufferInfo {res, buffer, bufData, bufLength};
}

std::optional<WriteEncodedDataResult> OtherEncode([[maybe_unused]] ani_env *env, std::u16string_view inputString,
                                                  const UConverterWrapper &cvt, char *dest, size_t destSize)
{
    // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    if (!cvt.IsValid()) {
        return std::nullopt;
    }
    char *destHead = dest;
    char *destTail = destHead + destSize;
    char *destWrittenUntil = destHead;
    size_t nInputCharsConsumed = 0;
    size_t startPos = 0;
    size_t endPos = 0;
    while (startPos < inputString.length()) {
        endPos = inputString.find('\0', startPos);
        if (endPos == std::string_view::npos) {
            endPos = inputString.length();
        }
        const UChar *inputHead = inputString.data() + startPos;
        const UChar *inputTail = inputString.data() + endPos;
        const UChar *inputReadUntil = inputHead;
        UErrorCode codeFlag = U_ZERO_ERROR;
        ucnv_fromUnicode(cvt.converter, &destWrittenUntil, destTail, &inputReadUntil, inputTail, nullptr,
                         static_cast<UBool>(true), &codeFlag);
        // Note: U_BUFFER_OVERFLOW_ERROR is expected result when the output buffer is small.
        if (codeFlag != U_ZERO_ERROR && codeFlag != U_BUFFER_OVERFLOW_ERROR) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            LOG_FATAL_SDK("TextEncoder:: Failure when converting to encoding %{public}s%{public}s", cvt.encoding,
                          u_errorName(codeFlag));
            return std::nullopt;
        }
        nInputCharsConsumed += (inputReadUntil - inputHead);
        if (endPos < inputString.length() && destWrittenUntil < destTail) {
            *destWrittenUntil++ = '\0';
        }
        if (codeFlag == U_BUFFER_OVERFLOW_ERROR || destWrittenUntil == destTail) {
            break;
        }
        startPos = endPos + 1;
    }
    return WriteEncodedDataResult {
        nInputCharsConsumed,
        static_cast<size_t>(destWrittenUntil - destHead),
    };
    // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
}

std::optional<WriteEncodedDataResult> OtherEncode(ani_env *env, ani_string inputStringObj, const UConverterWrapper &cvt,
                                                  char *dest, size_t destSize)
{
    ani_size inputSize = 0;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ANI_RETURN_NULLOPT_ON_FAILURE(env->String_GetUTF16Size(inputStringObj, &inputSize),
                                  "Internal error: env->String_GetUTF16Size() failed.");
    std::vector<char16_t> inputBuffer(inputSize + 1);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ANI_RETURN_NULLOPT_ON_FAILURE(env->String_GetUTF16(inputStringObj, reinterpret_cast<uint16_t *>(inputBuffer.data()),
                                                       inputSize + 1, &inputSize),
                                  "Internal error: env->String_GetUTF16() failed.");
    return OtherEncode(env, std::u16string_view {inputBuffer.data(), inputSize}, cvt, dest, destSize);
}

ani_arraybuffer OtherEncodeToBuffer(ani_env *env, ani_string inputStringObj, const UConverterWrapper &cvt,
                                    WriteEncodedDataResult *writeRes)
{
    ani_size inputSize = 0;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ANI_RETURN_NULLPTR_ON_FAILURE(env->String_GetUTF16Size(inputStringObj, &inputSize),
                                  "Internal error: env->String_GetUTF16Size() failed.");
    ani_arraybuffer buffer;
    void *bufferData;
    size_t bufferSize = ucnv_getMaxCharSize(cvt.converter) * inputSize;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ANI_RETURN_NULLPTR_ON_FAILURE(env->CreateArrayBuffer(bufferSize, &bufferData, &buffer),
                                  "Internal error: env->CreateArrayBuffer() failed.");
    std::vector<char16_t> inputData(inputSize + 1);

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ANI_RETURN_NULLPTR_ON_FAILURE(
        env->String_GetUTF16(inputStringObj, reinterpret_cast<uint16_t *>(inputData.data()), inputSize + 1, &inputSize),
        "Internal error: env->String_GetUTF16() failed.");
    std::optional<WriteEncodedDataResult> writeResOpt = OtherEncode(
        env, std::u16string_view {inputData.data(), inputSize}, cvt, static_cast<char *>(bufferData), bufferSize);
    if (!writeResOpt) {
        return nullptr;
    }
    if (writeRes != nullptr) {
        *writeRes = *writeResOpt;
    }
    return buffer;
}

ani_arraybuffer OtherEncodeToUint8Array(ani_env *env, ani_string inputStringObj, const UConverterWrapper &cvt)
{
    WriteEncodedDataResult writeRes {};
    ani_arraybuffer buffer = OtherEncodeToBuffer(env, inputStringObj, cvt, &writeRes);
    if (buffer == nullptr) {
        return nullptr;
    }
    return buffer;
}

std::optional<WriteEncodedDataResult> WriteEncodedData(ani_env *env, ani_string inputStringObj,
                                                       const std::string &encoding, char *dest, size_t destSizeBytes)
{
    if (encoding == "utf-8") {
        std::string utf8InputString = stdlib::ConvertFromAniString(env, inputStringObj);
        size_t nInputCharsConsumed;
        bool ok = false;
        std::string_view inputPrefix =
            ani_helper::Utf8GetPrefix(utf8InputString, destSizeBytes, &nInputCharsConsumed, &ok);
        if (!ok) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            LOG_FATAL_SDK("TextEncoder:: Failure during reading UTF-8 input.");
            return std::nullopt;
        }
        size_t resultSizeBytes = inputPrefix.length();
        if (memcpy_s(dest, destSizeBytes, inputPrefix.data(), resultSizeBytes) != EOK) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            LOG_FATAL_SDK("TextEncoder:: Failure during memcpy_s.");
            return std::nullopt;
        }
        return WriteEncodedDataResult {nInputCharsConsumed, resultSizeBytes};
    }
    if (encoding == "utf-16le" || encoding == "utf-16be") {
        std::string utf8InputString = stdlib::ConvertFromAniString(env, inputStringObj);
        size_t resultLengthLimit = destSizeBytes / 2;
        size_t nInputCharsConsumed;
        bool ok = false;
        std::u16string u16Str =
            ani_helper::Utf8ToUtf16LE(utf8InputString, resultLengthLimit, &nInputCharsConsumed, &ok);
        if (!ok) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            LOG_FATAL_SDK("TextEncoder:: Failure during conversion from UTF-8 to UTF-16.");
            return std::nullopt;
        }
        if (encoding == "utf-16be") {
            u16Str = ani_helper::Utf16LEToBE(u16Str);
        }
        size_t resultSizeBytes = u16Str.length() * 2;  // 2 : 2 bytes per u16 character
        // NOLINTNEXTLINE(bugprone-not-null-terminated-result)
        if (memcpy_s(dest, destSizeBytes, u16Str.data(), resultSizeBytes) != EOK) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            LOG_FATAL_SDK("TextEncoder:: Failure during memcpy_s");
            return std::nullopt;
        }
        return WriteEncodedDataResult {nInputCharsConsumed, resultSizeBytes};
    }
    UConverterWrapper cvt(encoding.c_str());
    if (!cvt.IsValid()) {
        return std::nullopt;
    }
    return OtherEncode(env, inputStringObj, cvt, dest, destSizeBytes);
}

ani_arraybuffer DoEncodeInto(ani_env *env, [[maybe_unused]] ani_object object, ani_string stringObj,
                             ani_string aniEncoding)
{
    ani_arraybuffer arrayBuffer = nullptr;
    void *rawData = nullptr;
    std::string encodingStr = stdlib::ConvertFromAniString(env, aniEncoding);
    if (encodingStr == "utf-8") {
        std::string inputString = stdlib::ConvertFromAniString(env, stringObj);
        ANI_FATAL_IF_ERROR(env->CreateArrayBuffer(inputString.length(), &rawData, &arrayBuffer));
        if (EOK != memcpy_s(rawData, inputString.length(), inputString.data(), inputString.length())) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            LOG_FATAL_SDK("TextEncoder:: Failure during memcpy_s.");
            return nullptr;
        }
        return arrayBuffer;
    }
    if (encodingStr == "utf-16le" || encodingStr == "utf-16be") {
        std::string inputString = stdlib::ConvertFromAniString(env, stringObj);
        bool ok = false;
        std::u16string utf16Str = ani_helper::Utf8ToUtf16LE(inputString, &ok);
        if (!ok) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            LOG_FATAL_SDK("TextEncoder:: Failure during conversion from UTF-8 to UTF-16.");
            return nullptr;
        }
        if (encodingStr == "utf-16be") {
            utf16Str = ani_helper::Utf16LEToBE(utf16Str);
        }
        size_t sizeBytes = utf16Str.length() * 2;  // 2 : 2 bytes per UTF-16 character
        ANI_FATAL_IF_ERROR(env->CreateArrayBuffer(sizeBytes, &rawData, &arrayBuffer));
        if (EOK != memcpy_s(rawData, sizeBytes, utf16Str.data(), sizeBytes)) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            LOG_FATAL_SDK("TextEncoder:: Failure during memcpy_s.");
            return nullptr;
        }
        return arrayBuffer;
    }
    UConverterWrapper cvt(encodingStr.c_str());
    if (!cvt.IsValid()) {
        return nullptr;
    }
    return OtherEncodeToUint8Array(env, stringObj, cvt);
}

ani_array DoEncodeIntoUint8Array(ani_env *env, [[maybe_unused]] ani_object object, ani_string inputStringObj,
                                 ani_string encodingObj, ani_object destObj)
{
    std::string encoding = stdlib::ConvertFromAniString(env, encodingObj);
    ani_int byteLength;
    ani_int byteOffset;
    ani_ref buffer;
    ANI_FATAL_IF_ERROR(env->Object_GetFieldByName_Int(destObj, "byteLengthInt", &byteLength));
    ANI_FATAL_IF_ERROR(env->Object_GetFieldByName_Int(destObj, "byteOffsetInt", &byteOffset));
    ANI_FATAL_IF_ERROR(env->Object_GetFieldByName_Ref(destObj, "buffer", &buffer));
    std::optional<ArrayBufferInfos> bufInfo = GetBufferInfo(env, static_cast<ani_arraybuffer>(buffer));
    if (!bufInfo) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        LOG_FATAL_SDK("TextEncoder:: Failure GetBufferInfo.");
        return nullptr;
    }
    std::optional<WriteEncodedDataResult> writeRes =
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        WriteEncodedData(env, inputStringObj, encoding, static_cast<char *>(bufInfo->data) + byteOffset, byteLength);
    if (!writeRes) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        LOG_FATAL_SDK("TextEncoder:: Failure WriteEncodedData.");
        return nullptr;
    }

    ani_ref undef {};
    ANI_FATAL_IF_ERROR(env->GetUndefined(&undef));

    ani_array pathsArray;
    // NOLINTNEXTLINE(readability-magic-numbers)
    ANI_FATAL_IF_ERROR(env->Array_New(2U, undef, &pathsArray));

    ani_class intClass {};
    ANI_FATAL_IF_ERROR(env->FindClass("std.core.Int", &intClass));

    ani_method intCtor;
    ANI_FATAL_IF_ERROR(env->Class_FindMethod(intClass, "<ctor>", "i:", &intCtor));
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    ani_int nativeParams[] = {static_cast<ani_int>(writeRes->nInputCharsConsumed),
                              static_cast<ani_int>(writeRes->resultSizeBytes)};
    for (size_t i = 0; i < 2U; ++i) {
        ani_object boxedInt {};
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        ANI_FATAL_IF_ERROR(env->Object_New(intClass, intCtor, &boxedInt, nativeParams[i]));
        ANI_FATAL_IF_ERROR(env->Array_Set(pathsArray, i, boxedInt));
    }
    return pathsArray;
}
}  // namespace ark::ets::sdk::util
