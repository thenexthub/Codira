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

#include "plugins/ets/runtime/interop_js/code_scopes.h"
#include "plugins/ets/runtime/interop_js/ets_proxy/ets_class_wrapper.h"
#include "plugins/ets/runtime/interop_js/ets_proxy/shared_reference_storage.h"
#include "plugins/ets/runtime/interop_js/js_refconvert_record.h"
#include "plugins/ets/runtime/types/ets_type.h"

namespace ark::ets::interop::js {

napi_value JSRefConvertRecord::WrapImpl(InteropCtx *ctx, EtsObject *obj)
{
    auto env = ctx->GetJSEnv();

    ets_proxy::SharedReferenceStorage *storage = ctx->GetSharedRefStorage();
    if (LIKELY(storage->HasReference(obj, env))) {
        return storage->GetJsObject(obj, env);
    }

    napi_value handlerObj = GetReferenceValue(env, handleObjCache_);

    napi_value targetObj;
    NAPI_CHECK_FATAL(napi_create_object(env, &targetObj));

    napi_value recordProto = ctx->GetCommonJSObjectCache()->GetRecordProto();
    ets_proxy::DoSetPrototype(env, targetObj, recordProto);

    std::array<napi_value, 2U> args = {targetObj, handlerObj};
    napi_value proxy = ctx->GetCommonJSObjectCache()->GetProxy();
    napi_value proxyObj;
    NAPI_CHECK_FATAL(napi_new_instance(env, proxy, args.size(), args.data(), &proxyObj));

    auto coro = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope s(coro);
    EtsHandle<EtsObject> objHandle(coro, obj);
    storage->CreateETSObjectRef(ctx, objHandle, proxyObj);

    return proxyObj;
}

napi_value JSRefConvertRecord::RecordGetHandler(napi_env env, napi_callback_info cbinfo)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_JS_TO_ETS(coro);

    size_t argc;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, cbinfo, &argc, nullptr, nullptr, nullptr));
    auto jsArgs = ctx->GetTempArgs<napi_value>(argc);
    NAPI_CHECK_FATAL(napi_get_cb_info(env, cbinfo, &argc, jsArgs->data(), nullptr, nullptr));

    std::string value = GetString(env, jsArgs[1]);
    if (value == ets_proxy::SharedReferenceStorage::PROXY_NAPI_WRAPPER) {
        napi_value result;
        napi_get_property(env, jsArgs->data()[0], jsArgs->data()[1], &result);
        return result;
    }

    ScopedManagedCodeThread managedScope(coro);

    ets_proxy::SharedReferenceStorage *storage = ctx->GetSharedRefStorage();
    ASSERT(storage != nullptr);
    ets_proxy::SharedReference *sharedRef = storage->GetReference(env, jsArgs->data()[0]);
    ASSERT(sharedRef != nullptr);

    auto *etsThis = sharedRef->GetEtsObject();
    ASSERT(etsThis != nullptr);

    EtsMethod *getMethod = etsThis->GetClass()->ResolveVirtualMethod(PlatformTypes(coro)->coreRecordGet);

    if (argc < 2U) {
        INTEROP_LOG(ERROR) << "Invalid number of arguments for $_get";
        return nullptr;
    }

    Span sp(&jsArgs[1], 1);
    return CallETSInstance(coro, ctx, getMethod->GetPandaMethod(), sp, etsThis);
}

napi_value JSRefConvertRecord::RecordSetHandler(napi_env env, napi_callback_info cbinfo)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_JS_TO_ETS(coro);
    ScopedManagedCodeThread managedScope(coro);

    size_t argc;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, cbinfo, &argc, nullptr, nullptr, nullptr));
    auto jsArgs = ctx->GetTempArgs<napi_value>(argc);
    NAPI_CHECK_FATAL(napi_get_cb_info(env, cbinfo, &argc, jsArgs->data(), nullptr, nullptr));

    ets_proxy::SharedReferenceStorage *storage = ctx->GetSharedRefStorage();
    ASSERT(storage != nullptr);
    ets_proxy::SharedReference *sharedRef = storage->GetReference(env, jsArgs->data()[0]);
    ASSERT(sharedRef != nullptr);

    auto *etsThis = sharedRef->GetEtsObject();
    ASSERT(etsThis != nullptr);

    EtsMethod *setMethod = etsThis->GetClass()->ResolveVirtualMethod(PlatformTypes(coro)->coreRecordSet);

    if (argc < 3U) {
        INTEROP_LOG(ERROR) << "Invalid number of arguments for $_set";
        return nullptr;
    }

    Span sp(&jsArgs[1], 2U);

    CallETSInstance(coro, ctx, setMethod->GetPandaMethod(), sp, etsThis);

    napi_value trueValue = GetBooleanValue(env, true);
    return trueValue;
}

EtsObject *JSRefConvertRecord::UnwrapImpl([[maybe_unused]] InteropCtx *ctx, [[maybe_unused]] napi_value jsValue)
{
    napi_env env = ctx->GetJSEnv();
    ets_proxy::SharedReference *sharedRef = ctx->GetSharedRefStorage()->GetReference(env, jsValue);

    if (LIKELY(sharedRef != nullptr)) {
        EtsObject *etsObject = sharedRef->GetEtsObject();
        if (UNLIKELY(!PlatformTypes()->coreRecord->IsAssignableFrom(etsObject->GetClass()))) {
            InteropCtx::ThrowJSTypeError(env, std::string("Object is not compatible with std.core.Record"));
            return nullptr;
        }

        return etsObject;
    }

    InteropCtx::ThrowJSTypeError(env, std::string("Object is not compatible with std.core.Record"));
    return nullptr;
}

}  // namespace ark::ets::interop::js
