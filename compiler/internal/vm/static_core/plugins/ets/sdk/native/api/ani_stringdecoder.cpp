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

#include <codecvt>
#include <locale>
#include <memory>

#include "plugins/ets/stdlib/native/core/stdlib_ani_helpers.h"
#include "tools/format_logger.h"
#include "unicode/unistr.h"
#include "Util.h"

#include "ani_stringdecoder.h"

namespace ark::ets::sdk::util {
constexpr int ERROR_CODE = 401;

UConverter *UtilHelper::CreateConverter(const std::string &encStr, UErrorCode &codeflag)
{
    UConverter *conv = ucnv_open(encStr.c_str(), &codeflag);
    if (U_FAILURE(codeflag) != 0) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        LOG_FATAL_SDK("Unable to create a UConverter object: %s\n", u_errorName(codeflag));
        return nullptr;
    }
    ucnv_setFromUCallBack(conv, UCNV_FROM_U_CALLBACK_SUBSTITUTE, nullptr, nullptr, nullptr, &codeflag);
    if (U_FAILURE(codeflag) != 0) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        LOG_FATAL_SDK("Unable to set the from Unicode callback function");
        ucnv_close(conv);
        return nullptr;
    }

    ucnv_setToUCallBack(conv, UCNV_TO_U_CALLBACK_SUBSTITUTE, nullptr, nullptr, nullptr, &codeflag);
    if (U_FAILURE(codeflag) != 0) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        LOG_FATAL_SDK("Unable to set the to Unicode callback function");
        ucnv_close(conv);
        return nullptr;
    }
    return conv;
}

StringDecoder::StringDecoder(const std::string &encoding)
{
    UErrorCode codeflag = U_ZERO_ERROR;
    conv_ = UtilHelper::CreateConverter(encoding, codeflag);
}

ani_string StringDecoder::Write(ani_env *env, const char *source, int32_t byteOffset, uint32_t length, UBool flush)
{
    size_t limit = static_cast<size_t>(ucnv_getMinCharSize(conv_)) * length;
    if (limit <= 0) {
        ThrowBusinessError(env, ERROR_CODE, "Error obtaining minimum number of input bytes");
        return nullptr;
    }
    std::vector<UChar> arr(limit + 1, 0);
    UChar *target = arr.data();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    UChar *targetEnd = target + limit;
    UErrorCode codeFlag = U_ZERO_ERROR;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    source += byteOffset;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    const char *sourceEnd = source + length;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    ucnv_toUnicode(conv_, &target, targetEnd, &source, sourceEnd, nullptr, flush, &codeFlag);
    // NOLINTNEXTLINE(readability-implicit-bool-conversion)
    if (U_FAILURE(codeFlag)) {
        std::string err = "decoder error, ";
        err += u_errorName(codeFlag);
        ThrowBusinessError(env, ERROR_CODE, err);
        return nullptr;
    }
    pendingLen_ = ucnv_toUCountPending(conv_, &codeFlag);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    pend_ = source + length - pendingLen_;
    ani_string resultStr {};
    size_t resultLen = target - arr.data();
    ANI_FATAL_IF_ERROR(env->String_NewUTF16(reinterpret_cast<uint16_t *>(arr.data()), resultLen, &resultStr));
    return resultStr;
}

ani_string StringDecoder::End(ani_env *env, const char *source, int32_t byteOffset, int32_t length)
{
    return Write(env, source, byteOffset, length, 1);
}

ani_string StringDecoder::End(ani_env *env)
{
    ani_string resultStr {};
    if (pendingLen_ == 0) {
        env->String_NewUTF8("", 0, &resultStr);
        return resultStr;
    }
    UErrorCode errorCode = U_ZERO_ERROR;
    std::vector<UChar> outputBuffer(pendingLen_);
    UChar *target = outputBuffer.data();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    UChar *const targetEnd = target + pendingLen_;
    const char *src = pend_;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    const char *const sourceEnd = src + pendingLen_;
    UBool flush = 1;
    ucnv_toUnicode(conv_, &target, targetEnd, &src, sourceEnd, nullptr, flush, &errorCode);
    // NOLINTNEXTLINE(readability-implicit-bool-conversion)
    if (U_FAILURE(errorCode)) {
        std::string err = "decoder error, ";
        err += u_errorName(errorCode);
        ThrowBusinessError(env, ERROR_CODE, err);
        return nullptr;
    }
    const size_t convertedChars = target - outputBuffer.data();
    if (convertedChars < outputBuffer.size()) {
        outputBuffer[convertedChars] = 0;
    }
    ANI_FATAL_IF_ERROR(
        env->String_NewUTF16(reinterpret_cast<uint16_t *>(outputBuffer.data()), convertedChars, &resultStr));
    return resultStr;
}
}  // namespace ark::ets::sdk::util