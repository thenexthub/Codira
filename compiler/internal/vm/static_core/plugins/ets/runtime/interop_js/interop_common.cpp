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

#include "plugins/ets/runtime/interop_js/interop_context.h"
#include "plugins/ets/runtime/interop_js/interop_common.h"
#include "libarkbase/os/mutex.h"
#ifdef OHOS_PANDA_TRACE_ENABLE
#include "syspara/parameters.h"
#endif

namespace ark::ets::interop::js {

[[noreturn]] void InteropFatal(const char *message)
{
    InteropCtx::Fatal(message);
    UNREACHABLE();
}

[[noreturn]] void InteropFatal(const std::string &message)
{
    InteropCtx::Fatal(message.c_str());
    UNREACHABLE();
}

[[noreturn]] void InteropFatal(const char *message, napi_status status)
{
    InteropCtx::Fatal(std::string(message) + " status=" + std::to_string(status));
    UNREACHABLE();
}

#if defined(OHOS_PANDA_TRACE_ENABLE)
void InteropTrace(const char *func, const char *file, int line)
{
    static std::once_flag initFlag;
    static bool isInteropTraceEnabled = false;

    std::call_once(initFlag,
                   []() { isInteropTraceEnabled = OHOS::system::GetBoolParameter(INTEROP_TRACE_ENABLE, false); });

    if (isInteropTraceEnabled) {
#ifndef NDEBUG
        INTEROP_LOG(INFO) << "interoptrace: " << func << ":" << file << ":" << line;
#else
        INTEROP_LOG(INFO) << "interoptrace: " << func;
#endif
    }
}
#endif

std::pair<SmallVector<uint64_t, 4U>, int> GetBigInt(napi_env env, napi_value jsVal)
{
    size_t wordCount;
    NAPI_ASSERT_OK(napi_get_value_bigint_words(env, jsVal, nullptr, &wordCount, nullptr));

    int signBit;
    SmallVector<uint64_t, 4U> words;
    if (wordCount == 0) {
        bool lossless;
        signBit = 0;
        words.resize(1);
        NAPI_ASSERT_OK(napi_get_value_bigint_uint64(env, jsVal, &words[0], &lossless));
    } else {
        words.resize(wordCount);
        NAPI_ASSERT_OK(napi_get_value_bigint_words(env, jsVal, &signBit, &wordCount, words.data()));
    }

    return {words, signBit};
}

SmallVector<uint64_t, 4U> ConvertBigIntArrayFromEtsToJs(const std::vector<uint32_t> &etsArray)
{
    ASSERT(BIT_64 % BIGINT_BITS_NUM == 0);
    // BigInt in ArkTS is stored in EtsInt array. Put these bits into int64_t array
    size_t jsArraySize = etsArray.size() / 2 + etsArray.size() % 2;
    SmallVector<uint64_t, 4U> jsArray;
    jsArray.resize(jsArraySize, 0);

    size_t curJSArrayElemPos = 0;
    size_t curBitPos = 0;
    for (auto etsElem : etsArray) {
        jsArray[curJSArrayElemPos] |= static_cast<uint64_t>(etsElem) << curBitPos;
        curBitPos = curBitPos + BIGINT_BITS_NUM;
        if (curBitPos == BIT_64) {
            curBitPos = 0;
            ++curJSArrayElemPos;
        }
    }

    return jsArray;
}

static inline size_t GetBigIntEtsArraySize(size_t jsArraySize, uint64_t lastElem)
{
    if (jsArraySize == 1 && lastElem == 0) {
        return 0;
    }

    size_t etsSize = jsArraySize * 2 - 1;

    if ((lastElem >> BIGINT_BITS_NUM) > 0) {
        ++etsSize;
    }

    return etsSize;
}

std::vector<EtsInt> ConvertBigIntArrayFromJsToEts(SmallVector<uint64_t, 4U> &jsArray)
{
    size_t etsArraySize = GetBigIntEtsArraySize(jsArray.size(), jsArray.back());
    std::vector<EtsInt> etsArray(etsArraySize, 0);

    size_t curJSArrayElemPos = 0;
    size_t curBitPos = 0;
    for (size_t i = 0; i < etsArraySize; ++i) {
        etsArray[i] = static_cast<uint32_t>(jsArray[curJSArrayElemPos] >> curBitPos);

        curBitPos = curBitPos + BIGINT_BITS_NUM;
        if (curBitPos == BIT_64) {
            curBitPos = 0;
            ++curJSArrayElemPos;
        }
    }

    return etsArray;
}

void ThrowNoInteropContextException()
{
    auto *thread = ManagedThread::GetCurrent();
    ASSERT(thread != nullptr);
    auto ctx = thread->GetVM()->GetLanguageContext();
    auto descriptor = utf::CStringAsMutf8(panda_file_items::class_descriptors::NO_INTEROP_CONTEXT_ERROR.data());
    PandaString msg = "Interop call may be done only from _main_ or exclusive worker";
    ThrowException(ctx, thread, descriptor, utf::CStringAsMutf8(msg.c_str()));
}

void ThrowJSErrorNotAssignable(napi_env env, const EtsClass *fromKlass, EtsClass *toKlass)
{
    const char *from = fromKlass->GetDescriptor();
    const char *to = toKlass->GetDescriptor();
    InteropCtx::ThrowJSTypeError(env, std::string(from) + " is not assignable to " + to);
}

static bool GetPropertyStatusHandling([[maybe_unused]] napi_env env, napi_status rc)
{
#if !defined(PANDA_TARGET_OHOS) && !defined(PANDA_JS_ETS_HYBRID_MODE)
    if (UNLIKELY(rc == napi_object_expected || NapiThrownGeneric(rc))) {
        ASSERT(NapiIsExceptionPending(env));
        return false;
    }
#else
    if (UNLIKELY(rc == napi_object_expected && !NapiIsExceptionPending(env))) {
        InteropCtx::ThrowJSTypeError(env, "Cannot convert undefined or null to object");
        return false;
    }

    if (UNLIKELY(rc == napi_pending_exception || NapiThrownGeneric(rc))) {
        ASSERT(NapiIsExceptionPending(env));
        return false;
    }
#endif
    INTEROP_FATAL_IF(rc != napi_ok);
    return true;
}

bool NapiGetProperty(napi_env env, napi_value object, napi_value key, napi_value *result)
{
    napi_status rc = napi_get_property(env, object, key, result);
    return GetPropertyStatusHandling(env, rc);
}

bool NapiGetNamedProperty(napi_env env, napi_value object, const char *utf8name, napi_value *result)
{
    napi_status rc = napi_get_named_property(env, object, utf8name, result);
    return GetPropertyStatusHandling(env, rc);
}

}  // namespace ark::ets::interop::js
