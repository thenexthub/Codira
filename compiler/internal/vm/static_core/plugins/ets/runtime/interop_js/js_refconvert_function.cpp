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
#include "plugins/ets/runtime/interop_js/js_proxy/js_proxy.h"
#include "plugins/ets/runtime/interop_js/js_refconvert_function.h"
#include "plugins/ets/runtime/interop_js/code_scopes.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_type.h"

namespace ark::ets::interop::js {

JSRefConvertFunction::JSRefConvertFunction(Class *klass)
    : JSRefConvert(this), klass_ {EtsClass::FromRuntimeClass(klass)}
{
    // Mark interop function dynamic class as XRef class
    auto *classLinker = PandaEtsVM::GetCurrent()->GetClassLinker();

    // Get and store interop dynamic function class
    auto *interopDynamicFunction =
        classLinker->GetClass(panda_file_items::class_descriptors::INTEROP_DYNAMIC_FUNCTION.data());
    if (UNLIKELY(interopDynamicFunction == nullptr)) {
        // just throw exception
        auto coro = EtsCoroutine::GetCurrent();
        InteropCtx::ThrowETSError(
            coro, "Interop: Interop function dynamic class Lstd/interop/js/DynamicFunction; is not found.");
        return;
    }
    interopDynamicFunction->GetRuntimeClass()->SetXRefClass();
    this->interopDynamicFunctionClass_ = interopDynamicFunction;

    // Get and store interop create dynamic function method
    auto *createDynamicFunctionMethod = EtsClass::FromRuntimeClass(interopDynamicFunction->GetRuntimeClass())
                                            ->GetStaticMethod("CreateDynamicFunction", nullptr);
    if (UNLIKELY(createDynamicFunctionMethod == nullptr)) {
        InteropCtx::ThrowETSError(EtsCoroutine::GetCurrent(),
                                  "Interop: CreateDynamicFunction() of Lstd/interop/js/DynamicFunction; is not found.");
        return;
    }
    this->interopCreateDynamicFunctionMethod_ = createDynamicFunctionMethod;
}

napi_value EtsLambdaProxyInvoke(napi_env env, napi_callback_info cbinfo)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    INTEROP_CODE_SCOPE_JS_TO_ETS(coro);

    size_t argc;
    napi_value athis;
    void *data;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, cbinfo, &argc, nullptr, &athis, &data));
    auto jsArgs = ctx->GetTempArgs<napi_value>(argc);
    NAPI_CHECK_FATAL(napi_get_cb_info(env, cbinfo, &argc, jsArgs->data(), &athis, &data));

    // Atomic with acquire order reason: load visibility after shared reference initialization
    auto *sharedRef = AtomicLoad(static_cast<ets_proxy::SharedReference **>(data), std::memory_order_acquire);
    ASSERT(sharedRef != nullptr);

    ScopedManagedCodeThread managedScope(coro);
    auto *etsThis = sharedRef->GetEtsObject();
    ASSERT(etsThis != nullptr);
    EtsMethod *method = etsThis->GetClass()->GetInstanceMethod(INVOKE_METHOD_NAME, nullptr);
    method = method == nullptr ? etsThis->GetClass()->GetInstanceMethod(STD_CORE_FUNCTION_UNSAFECALL_METHOD, nullptr)
                               : method;
    ASSERT(method != nullptr);

    return CallETSInstance(coro, ctx, method->GetPandaMethod(), *jsArgs, etsThis);
}

napi_value JSRefConvertFunction::WrapImpl(InteropCtx *ctx, EtsObject *obj)
{
    auto coro = EtsCoroutine::GetCurrent();
    ASSERT(ctx == InteropCtx::Current(coro));
    auto env = ctx->GetJSEnv();
    [[maybe_unused]] EtsHandleScope s(coro);

    ASSERT(obj->GetClass() == klass_);

    JSValue *jsValue;
    {
        NapiScope jsHandleScope(env);

        ets_proxy::SharedReferenceStorage *storage = ctx->GetSharedRefStorage();
        if (LIKELY(storage->HasReference(obj, env))) {
            jsValue = JSValue::CreateRefValue(coro, ctx, storage->GetJsObject(obj, env), napi_function);
        } else {
            napi_value jsFn;
            auto preInitCallback = [&env, &jsFn](ets_proxy::SharedReference **uninitializedRef) {
                ASSERT(uninitializedRef != nullptr);
                NAPI_CHECK_FATAL(napi_create_function(env, ark::ets::INVOKE_METHOD_NAME, NAPI_AUTO_LENGTH,
                                                      EtsLambdaProxyInvoke, uninitializedRef, &jsFn));
                return jsFn;
            };

            EtsHandle<EtsObject> objHandle(coro, obj);
            ets_proxy::SharedReference *sharedRef = storage->CreateETSObjectRef(ctx, objHandle, jsFn, preInitCallback);
            if (UNLIKELY(sharedRef == nullptr)) {
                ASSERT(InteropCtx::SanityJSExceptionPending());
                return nullptr;
            }
            jsValue = JSValue::CreateRefValue(coro, ctx, jsFn, napi_function);
        }
    }
    if (UNLIKELY(jsValue == nullptr)) {
        return nullptr;
    }

    EtsHandle<JSValue> jsValueHandle(coro, jsValue);
    return JSValue::GetNapiValue(coro, ctx, jsValueHandle);
}

EtsObject *JSRefConvertFunction::UnwrapImpl(InteropCtx *ctx, napi_value jsFun)
{
    // Check if object has SharedReference
    // If it has, return it
    auto storage = ctx->GetSharedRefStorage();
    auto sharedRef = storage->GetReference(ctx->GetJSEnv(), jsFun);
    if (LIKELY(sharedRef != nullptr)) {
        EtsObject *functionDynamicObject = sharedRef->GetEtsObject();
        return functionDynamicObject;
    }

    auto *coro = EtsCoroutine::GetCurrent();
    HandleScope<ObjectHeader *> scope(coro);

    auto ret = interopCreateDynamicFunctionMethod_->GetPandaMethod()->Invoke(coro, {}).GetAs<ObjectHeader *>();
    // storage->CreateJSObjectRefwithWrap will trigger XGC if needed
    // after that, the ptr `ret` will be valid
    // so a handle is created to keep the object valid
    EtsHandle<EtsObject> functionDynamicObject(coro, EtsObject::FromCoreType(ret));

    // Put it into SharedReferenceStorage
    storage->CreateJSObjectRefwithWrap(ctx, functionDynamicObject, jsFun);

    // ptr `ret` is valid now,
    // so we must get the object from the handle
    return functionDynamicObject.GetPtr();
}

}  // namespace ark::ets::interop::js
