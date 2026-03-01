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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_JS_REFCONVERT_ARRAY_H_
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_JS_REFCONVERT_ARRAY_H_

#include "libarkbase/macros.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"
#include "plugins/ets/runtime/interop_js/interop_common.h"
#include "plugins/ets/runtime/interop_js/js_refconvert.h"
#include "plugins/ets/runtime/interop_js/js_convert.h"
#include "runtime/mem/local_object_handle.h"
#include "plugins/ets/runtime/types/ets_object.h"

namespace ark::ets::interop::js {

// JSRefConvert adapter for builtin[] types
template <typename Conv>
class JSRefConvertBuiltinArray : public JSRefConvert {
public:
    explicit JSRefConvertBuiltinArray(Class *klass) : JSRefConvert(this), klass_(klass) {}

private:
    using ElemCpptype = typename Conv::cpptype;

    static ElemCpptype GetElem(VMHandle<coretypes::Array> arr, size_t idx)
    {
        if constexpr (Conv::IS_REFTYPE) {
            auto elem = EtsObject::FromCoreType(arr->Get<ObjectHeader *>(idx));
            return FromEtsObject<std::remove_pointer_t<ElemCpptype>>(elem);
        } else {
            return arr->Get<ElemCpptype>(idx);
        }
    }

    static void SetElem(coretypes::Array *arr, size_t idx, ElemCpptype value)
    {
        if constexpr (Conv::IS_REFTYPE) {
            arr->Set(idx, AsEtsObject(value)->GetCoreType());
        } else {
            arr->Set(idx, value);
        }
    }

public:
    napi_value WrapImpl(InteropCtx *ctx, EtsObject *obj)
    {
        auto *coro = EtsCoroutine::GetCurrent();
        auto env = ctx->GetJSEnv();
        [[maybe_unused]] HandleScope<ObjectHeader *> hscope(coro);

        VMHandle<coretypes::Array> etsArr(coro, coretypes::Array::Cast(obj->GetCoreType()));
        auto len = etsArr->GetLength();

        NapiEscapableScope jsHandleScope(env);
        napi_value jsArr;
        {
            ScopedNativeCodeThread nativeScope(coro);
            NAPI_CHECK_FATAL(napi_create_array_with_length(env, len, &jsArr));
        }

        for (size_t idx = 0; idx < len; ++idx) {
            ElemCpptype etsElem = GetElem(etsArr, idx);
            auto jsElem = Conv::WrapWithNullCheck(env, etsElem);
            if (UNLIKELY(jsElem == nullptr)) {
                return nullptr;
            }
            {
                // NOTE(audovichenko): try to do not change thread state in each iteration.
                ScopedNativeCodeThread s(coro);
                napi_status rc = napi_set_element(env, jsArr, idx, jsElem);
                if (UNLIKELY(NapiThrownGeneric(rc))) {
                    return nullptr;
                }
            }
        }
        jsHandleScope.Escape(jsArr);
        return jsArr;
    }

    EtsObject *UnwrapImpl(InteropCtx *ctx, napi_value jsArr)
    {
        auto coro = EtsCoroutine::GetCurrent();
        auto env = ctx->GetJSEnv();
        {
            bool isArray;
            NAPI_CHECK_FATAL(napi_is_array(env, jsArr, &isArray));
            if (UNLIKELY(!isArray)) {
                JSConvertTypeCheckFailed("array");
                return nullptr;
            }
        }

        uint32_t len;
        napi_status rc = napi_get_array_length(env, jsArr, &len);
        if (UNLIKELY(NapiThrownGeneric(rc))) {
            return nullptr;
        }

        // NOTE(vpukhov): elide handles for primitive arrays
        LocalObjectHandle<coretypes::Array> etsArr(coro, coretypes::Array::Create(klass_, len));
        NapiScope jsHandleScope(env);

        for (size_t idx = 0; idx < len; ++idx) {
            napi_value jsElem;
            rc = napi_get_element(env, jsArr, idx, &jsElem);
            if (UNLIKELY(NapiThrownGeneric(rc))) {
                return nullptr;
            }
            auto res = Conv::UnwrapWithNullCheck(ctx, env, jsElem);
            if (UNLIKELY(!res)) {
                return nullptr;
            }
            SetElem(etsArr.GetPtr(), idx, res.value());
        }

        return EtsObject::FromCoreType(etsArr.GetPtr());
    }

private:
    Class *klass_ {};
};

template <ClassRoot CLASS_ROOT, typename Conv>
static inline void RegisterBuiltinArrayConvertor(JSRefConvertCache *cache, EtsClassLinkerExtension *ext)
{
    auto aklass = ext->GetClassRoot(CLASS_ROOT);
    cache->Insert(aklass, std::make_unique<JSRefConvertBuiltinArray<Conv>>(aklass));
}

// JSRefConvert adapter for reference[] types
class JSRefConvertReftypeArray : public JSRefConvert {
public:
    explicit JSRefConvertReftypeArray(Class *klass) : JSRefConvert(this), klass_(klass) {}

    napi_value WrapImpl(InteropCtx *ctx, EtsObject *obj)
    {
        auto coro = EtsCoroutine::GetCurrent();
        auto env = ctx->GetJSEnv();
        [[maybe_unused]] HandleScope<ObjectHeader *> hscope(coro);

        VMHandle<coretypes::Array> etsArr(coro, obj->GetCoreType());
        ASSERT(etsArr.GetPtr() != nullptr);
        auto len = etsArr->GetLength();

        NapiEscapableScope jsHandleScope(env);
        napi_value jsArr;
        {
            ScopedNativeCodeThread nativeScope(coro);
            NAPI_CHECK_FATAL(napi_create_array_with_length(env, len, &jsArr));
        }

        for (size_t idx = 0; idx < len; ++idx) {
            EtsObject *etsElem = EtsObject::FromCoreType(etsArr->Get<ObjectHeader *>(idx));
            napi_value jsElem;
            if (LIKELY(etsElem != nullptr)) {
                JSRefConvert *elemConv = GetElemConvertor(ctx, etsElem->GetClass());
                if (UNLIKELY(elemConv == nullptr)) {
                    return nullptr;
                }
                // Need to read element pointer again, since it could have been moved by GC
                etsElem = EtsObject::FromCoreType(etsArr->Get<ObjectHeader *>(idx));
                ASSERT(etsElem != nullptr);
                jsElem = elemConv->Wrap(ctx, etsElem);
                if (UNLIKELY(jsElem == nullptr)) {
                    return nullptr;
                }
            } else {
                jsElem = GetUndefined(env);
            }
            {
                ScopedNativeCodeThread s(coro);
                napi_status rc = napi_set_element(env, jsArr, idx, jsElem);
                if (UNLIKELY(NapiThrownGeneric(rc))) {
                    return nullptr;
                }
            }
        }
        jsHandleScope.Escape(jsArr);
        return jsArr;
    }

    EtsObject *UnwrapNonUndefined(InteropCtx *ctx, napi_value jsElem)
    {
        if (UNLIKELY(baseElemConv_ == nullptr)) {
            baseElemConv_ = JSRefConvertResolve(ctx, klass_->GetComponentType());
            if (UNLIKELY(baseElemConv_ == nullptr)) {
                return nullptr;
            }
        }
        EtsObject *etsElem = baseElemConv_->Unwrap(ctx, jsElem);
        if (UNLIKELY(etsElem == nullptr)) {
            return nullptr;
        }
        return etsElem;
    }

    EtsObject *UnwrapImpl(InteropCtx *ctx, napi_value jsArr)
    {
        auto coro = EtsCoroutine::GetCurrent();
        auto env = ctx->GetJSEnv();
        {
            bool isArray;
            NAPI_CHECK_FATAL(napi_is_array(env, jsArr, &isArray));
            if (UNLIKELY(!isArray)) {
                JSConvertTypeCheckFailed("array");
                return nullptr;
            }
        }

        uint32_t len;
        napi_status rc = napi_get_array_length(env, jsArr, &len);
        if (UNLIKELY(NapiThrownGeneric(rc))) {
            return nullptr;
        }

        LocalObjectHandle<coretypes::Array> etsArr(coro, coretypes::Array::Create(klass_, len));
        NapiScope jsHandleScope(env);

        for (size_t idx = 0; idx < len; ++idx) {
            napi_value jsElem;
            rc = napi_get_element(env, jsArr, idx, &jsElem);
            if (UNLIKELY(NapiThrownGeneric(rc))) {
                return nullptr;
            }
            if (LIKELY(!IsNullOrUndefined<true>(env, jsElem))) {
                auto *etsElem = UnwrapNonUndefined(ctx, jsElem);
                if (UNLIKELY(etsElem == nullptr)) {
                    return nullptr;
                }
                etsArr->Set(idx, etsElem->GetCoreType());
            }
        }

        return EtsObject::FromCoreType(etsArr.GetPtr());
    }

private:
    static constexpr auto ELEM_SIZE = ClassHelper::OBJECT_POINTER_SIZE;

    JSRefConvert *GetElemConvertor(InteropCtx *ctx, EtsClass *elemEtsKlass)
    {
        Class *elemKlass = elemEtsKlass->GetRuntimeClass();
        if (elemKlass != klass_->GetComponentType()) {
            return JSRefConvertResolve(ctx, elemKlass);
        }
        if (LIKELY(baseElemConv_ != nullptr)) {
            return baseElemConv_;
        }
        return baseElemConv_ = JSRefConvertResolve(ctx, klass_->GetComponentType());
    }

    Class *klass_ {};
    JSRefConvert *baseElemConv_ {};
};

}  // namespace ark::ets::interop::js

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_JS_REFCONVERT_ARRAY_H_
