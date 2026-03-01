/**
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

#include "runtime/include/mem/panda_string.h"
#include "runtime/coroutines/stackful_coroutine.h"
#include "runtime/include/class_linker-inl.h"
#include "runtime/include/object_header.h"
#include "plugins/ets/runtime/ets_stubs-inl.h"
#include "plugins/ets/runtime/interop_js/call/call.h"
#include "plugins/ets/runtime/interop_js/js_convert.h"
#include "plugins/ets/runtime/interop_js/js_value.h"
#include "plugins/ets/runtime/interop_js/interop_common.h"
#include "plugins/ets/runtime/interop_js/intrinsics_api_impl.h"
#include "plugins/ets/runtime/interop_js/code_scopes.h"
#include "plugins/ets/runtime/interop_js/logger.h"
#include "plugins/ets/runtime/interop_js/napi_impl/ark_napi_helper.h"
#include "plugins/ets/runtime/interop_js/xref_object_operator.h"
#include "plugins/ets/runtime/types/ets_array.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "common_interfaces/objects/base_type.h"
#include "common_interfaces/objects/dynamic_object_accessor_util.h"

// NOLINTBEGIN(readability-identifier-naming, readability-redundant-declaration)
// CC-OFFNXT(G.FMT.10-CPP) project code style
napi_status __attribute__((weak))
napi_load_module_with_module_request(napi_env env, const char *request_name, napi_value *result);

napi_status __attribute__((weak))  // CC-OFF(G.FMT.10) project code style
napi_serialize_hybrid([[maybe_unused]] napi_env env, [[maybe_unused]] napi_value object,
                      [[maybe_unused]] napi_value transfer_list, [[maybe_unused]] napi_value clone_list,
                      [[maybe_unused]] void **result);

napi_status __attribute__((weak)) napi_deserialize_hybrid([[maybe_unused]] napi_env env, [[maybe_unused]] void *buffer,
                                                          [[maybe_unused]] napi_value *object);
// NOLINTEND(readability-identifier-naming, readability-redundant-declaration)

static bool StringStartWith(std::string_view str, std::string_view startStr)
{
    size_t startStrLen = startStr.length();
    return str.compare(0, startStrLen, startStr) == 0;
}

namespace ark::ets::interop::js {

using common::TaggedType;

[[maybe_unused]] static bool NotNativeOhmUrl(std::string_view url)
{
    constexpr std::string_view PREFIX_BUNDLE = "@bundle:";
    constexpr std::string_view PREFIX_NORMALIZED = "@normalized:";
    constexpr std::string_view PREFIX_PACKAGE = "@package:";
    return StringStartWith(url, PREFIX_BUNDLE) || StringStartWith(url, PREFIX_NORMALIZED) ||
           StringStartWith(url, PREFIX_PACKAGE) || (url[0] != '@');
}

[[maybe_unused]] static JSValue *LoadJSModule(const PandaString &moduleName)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);
    napi_value result;
    napi_status status;
    {
        ScopedNativeCodeThread etsNativeScope(coro);
        status = napi_load_module_with_module_request(env, moduleName.c_str(), &result);
        if (status != napi_ok) {
            INTEROP_LOG(ERROR) << "Unable to load module " << moduleName.c_str() << " due to Forward Exception";
            ctx->ForwardJSException(coro);
            return nullptr;
        }
    }
    if (IsUndefined<true>(env, result) || !result) {
        PandaString exp = PandaString("Unable to load module ") + moduleName.c_str() + " due to Undefined result";
        INTEROP_LOG(ERROR) << exp;
        InteropCtx::ThrowETSError(coro, exp.c_str());
        return nullptr;
    }
    return JSValue::CreateRefValue(coro, ctx, result, napi_object);
}

void JSRuntimeFinalizationRegistryCallback(EtsObject *cbarg)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return;
    }
    ASSERT(cbarg->GetClass()->GetRuntimeClass() == ctx->GetJSValueClass());
    return JSValue::FinalizeETSWeak(ctx, cbarg);
}

JSValue *JSRuntimeNewJSValueDouble(double v)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    return JSValue::CreateNumber(coro, ctx, v);
}

JSValue *JSRuntimeNewJSValueBoolean(uint8_t v)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    return JSValue::CreateBoolean(coro, ctx, static_cast<bool>(v));
}

JSValue *JSRuntimeNewJSValueString(EtsString *v)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    std::string str;
    if (v->IsUtf16()) {
        InteropCtx::Fatal("not implemented");
    } else {
        PandaVector<uint8_t> tree8Buf;
        uint8_t *data = v->IsTreeString() ? v->GetTreeStringDataMUtf8(tree8Buf) : v->GetDataMUtf8();
        str = std::string(utf::Mutf8AsCString(data), v->GetLength());
    }
    return JSValue::CreateString(coro, ctx, std::move(str));
}

JSValue *JSRuntimeNewJSValueObject(EtsObject *v)
{
    if (v == nullptr) {
        return nullptr;
    }

    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    if (v->GetClass() == PlatformTypes(coro)->interopJSValue) {
        return JSValue::FromEtsObject(v);
    }
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    auto refconv = JSRefConvertResolve(ctx, v->GetClass()->GetRuntimeClass());
    if (UNLIKELY(refconv == nullptr)) {
        // For builtin without supported converter
        ASSERT(coro->HasPendingException());
        return nullptr;
    }
    auto result = refconv->Wrap(ctx, v);

    auto res = JSConvertJSValue::UnwrapWithNullCheck(ctx, env, result);
    if (!res) {
        ctx->ForwardJSException(coro);
        return nullptr;
    }

    return res.value();
}

uint8_t JSRuntimeIsJSValue(EtsObject *v)
{
    if (v == nullptr) {
        return 0U;
    }
    auto coro = EtsCoroutine::GetCurrent();
    return static_cast<uint8_t>(v->GetClass() == PlatformTypes(coro)->interopJSValue);
}

JSValue *JSRuntimeNewJSValueBigInt(EtsBigInt *v)
{
    if (v == nullptr) {
        return nullptr;
    }

    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    auto refconv = JSRefConvertResolve(ctx, v->GetClass()->GetRuntimeClass());
    auto result = refconv->Wrap(ctx, v);

    auto res = JSConvertJSValue::UnwrapWithNullCheck(ctx, env, result);
    if (!res) {
        ctx->ForwardJSException(coro);
        return {};
    }

    return res.value();
}

double JSRuntimeGetValueDouble(JSValue *etsJsValue)
{
    return etsJsValue->GetNumber();
}

uint8_t JSRuntimeGetValueBoolean(JSValue *etsJsValue)
{
    return static_cast<uint8_t>(etsJsValue->GetBoolean());
}

EtsString *JSRuntimeGetValueString(JSValue *etsJsValue)
{
    ASSERT(etsJsValue != nullptr);

    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    auto env = ctx->GetJSEnv();

    NapiScope jsHandleScope(env);

    napi_value jsVal = JSConvertJSValue::Wrap(env, etsJsValue);
    auto res = JSConvertString::Unwrap(ctx, env, jsVal);
    if (UNLIKELY(!res)) {
        if (NapiIsExceptionPending(env)) {
            ctx->ForwardJSException(coro);
        }
        ASSERT(ctx->SanityETSExceptionPending());
        return nullptr;
    }

    return res.value();
}

EtsObject *JSRuntimeGetValueObject(EtsObject *etsValue, EtsClass *clsObj)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    auto cls = clsObj->GetRuntimeClass();

    if (etsValue == nullptr) {
        return nullptr;
    }

    // NOTE(kprokopenko): awful solution, see #14765
    if (etsValue == ctx->GetNullValue()) {
        return etsValue;
    }

    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    napi_value jsVal = JSConvertEtsObject::WrapWithNullCheck(env, etsValue);

    auto refconv = JSRefConvertResolve<true>(ctx, cls);
    if (UNLIKELY(refconv == nullptr)) {
        if (NapiIsExceptionPending(env)) {
            ctx->ForwardJSException(coro);
        }
        ASSERT(ctx->SanityETSExceptionPending());
        return nullptr;
    }

    auto res = refconv->Unwrap(ctx, jsVal);
    if (UNLIKELY(!res)) {
        if (NapiIsExceptionPending(env)) {
            ctx->ForwardJSException(coro);
        }
        res = nullptr;
    }
    return res;
}

JSValue *JSRuntimeGetUndefined()
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    return JSValue::CreateUndefined(coro, ctx);
}

JSValue *JSRuntimeGetNull()
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    return JSValue::CreateNull(coro, ctx);
}

JSValue *JSRuntimeGetGlobal()
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    napi_value global;
    {
        ScopedNativeCodeThread nativeScope(coro);
        global = GetGlobal(env);
    }
    return JSValue::CreateRefValue(coro, ctx, global, napi_object);
}

JSValue *JSRuntimeCreateObject()
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    napi_value obj;
    {
        ScopedNativeCodeThread nativeScope(coro);
        NAPI_CHECK_FATAL(napi_create_object(env, &obj));
    }
    return JSValue::CreateRefValue(coro, ctx, obj, napi_object);
}

EtsObject *JSRuntimeCreateArray()
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    napi_value array;
    {
        ScopedNativeCodeThread nativeScope(coro);
        NAPI_CHECK_FATAL(napi_create_array(env, &array));
    }
    return JSConvertEtsObject::UnwrapWithNullCheck(ctx, env, array).value();
}

uint8_t JSRuntimeInstanceOfDynamic(EtsObject *object, EtsObject *ctor)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return 0;
    }
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    auto objConv = JSRefConvertResolve(ctx, object->GetClass()->GetRuntimeClass());
    auto ctorConv = JSRefConvertResolve(ctx, ctor->GetClass()->GetRuntimeClass());
    bool res;
    auto targetObj = objConv->Wrap(ctx, object);
    auto ctorObj = ctorConv->Wrap(ctx, ctor);
    napi_status rc;
    {
        ScopedNativeCodeThread nativeScope(coro);
        rc = napi_instanceof(env, targetObj, ctorObj, &res);
    }
    if (UNLIKELY(NapiThrownGeneric(rc))) {
        ctx->ForwardJSException(coro);
        return 0;
    }
    return static_cast<uint8_t>(res);
}

uint8_t JSRuntimeInstanceOfStatic(JSValue *etsJsValue, EtsClass *etsCls)
{
    ASSERT(etsCls != nullptr);

    if (etsJsValue == nullptr) {
        return 0;
    }

    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return 0;
    }
    auto env = ctx->GetJSEnv();

    Class *cls = etsCls->GetRuntimeClass();
    if (!cls->IsInitialized() && !IsStdClass(cls)) {
        // 'js_value' haven't been created from the uninitialized class
        return 0;
    }

    // Check if object has SharedReference
    ets_proxy::SharedReference *sharedRef = [=] {
        NapiScope jsHandleScope(env);
        napi_value jsValue = JSConvertJSValue::Wrap(env, etsJsValue);
        return ctx->GetSharedRefStorage()->GetReference(env, jsValue);
    }();

    if (sharedRef != nullptr) {
        EtsObject *etsObject = sharedRef->GetEtsObject();
        return static_cast<uint8_t>(etsCls->IsAssignableFrom(etsObject->GetClass()));
    }

    if (IsStdClass(cls)) {
        // NOTE(v.cherkashin): Add support compat types, #13577
        INTEROP_LOG(FATAL) << __func__ << " doesn't support compat types";
    }

    return 0;
}

static ALWAYS_INLINE inline bool CheckEtsObjectFoundException(EtsCoroutine *coroutine, EtsObject *etsObject)
{
    if (EtsReferenceNullish(coroutine, etsObject)) {
        PandaString message = "Need object";
        ThrowEtsException(coroutine, panda_file_items::class_descriptors::TYPE_ERROR, message.c_str());
        return true;
    }
    return false;
}

std::pair<std::string_view, std::string_view> ResolveModuleName(std::string_view module)
{
    static const std::unordered_set<std::string_view> NATIVE_MODULE_LIST = {
        "@system.app",  "@ohos.app",       "@system.router", "@system.curves",
        "@ohos.curves", "@system.matrix4", "@ohos.matrix4"};

    constexpr std::string_view REQUIRE = "require";
    constexpr std::string_view REQUIRE_NAPI = "requireNapi";
    constexpr std::string_view REQUIRE_NATIVE_MODULE = "requireNativeModule";
    constexpr std::string_view SYSTEM_PLUGIN_PREFIX = "@system.";
    constexpr std::string_view SYSTEM_PLUGIN_PREFIX_TRANSFORMED = "@system:";
    constexpr size_t SYSTEM_PLUGIN_PREFIX_SIZE = SYSTEM_PLUGIN_PREFIX.size();
    constexpr std::string_view OHOS_PLUGIN_PREFIX = "@ohos.";
    constexpr std::string_view OHOS_PLUGIN_PREFIX_TRANSFORMED = "@ohos:";
    constexpr size_t OHOS_PLUGIN_PREFIX_SIZE = OHOS_PLUGIN_PREFIX.size();
    constexpr std::string_view HMS_PLUGIN_PREFIX = "@hms.";
    constexpr std::string_view HMS_PLUGIN_PREFIX_TRANSFORMED = "@hms:";
    constexpr size_t HMS_PLUGIN_PREFIX_SIZE = HMS_PLUGIN_PREFIX.size();

    if (NATIVE_MODULE_LIST.count(module) != 0) {
        return {module.substr(1), REQUIRE_NATIVE_MODULE};
    }

    if ((module.compare(0, SYSTEM_PLUGIN_PREFIX_SIZE, SYSTEM_PLUGIN_PREFIX) == 0) ||
        (module.compare(0, SYSTEM_PLUGIN_PREFIX_SIZE, SYSTEM_PLUGIN_PREFIX_TRANSFORMED) == 0)) {
        return {module.substr(SYSTEM_PLUGIN_PREFIX_SIZE), REQUIRE_NAPI};
    }

    if ((module.compare(0, OHOS_PLUGIN_PREFIX_SIZE, OHOS_PLUGIN_PREFIX) == 0) ||
        (module.compare(0, OHOS_PLUGIN_PREFIX_SIZE, OHOS_PLUGIN_PREFIX_TRANSFORMED) == 0)) {
        return {module.substr(OHOS_PLUGIN_PREFIX_SIZE), REQUIRE_NAPI};
    }

    if ((module.compare(0, HMS_PLUGIN_PREFIX_SIZE, HMS_PLUGIN_PREFIX) == 0) ||
        (module.compare(0, HMS_PLUGIN_PREFIX_SIZE, HMS_PLUGIN_PREFIX_TRANSFORMED) == 0)) {
        return {module.substr(HMS_PLUGIN_PREFIX_SIZE), REQUIRE_NAPI};
    }

    return {module, REQUIRE};
}

static bool CheckModuleLoadStatus(EtsCoroutine *coro, napi_status status, const PandaString &moduleName,
                                  napi_value loadedObj)
{
    auto ctx = InteropCtx::Current(coro);
    auto env = ctx->GetJSEnv();
    if (status != napi_ok) {
        INTEROP_LOG(ERROR) << "Unable to load module " << moduleName.c_str() << " due to Forward Exception";
        ctx->ForwardJSException(coro);
        return false;
    }
    if (IsNullOrUndefined(env, loadedObj)) {
        PandaString exp = PandaString("Unable to load module ") + moduleName.c_str() + " due to null result";
        INTEROP_LOG(ERROR) << exp;
        InteropCtx::ThrowETSError(coro, exp.c_str());
        return false;
    }
    return true;
}

static bool NeedWrapDefault(const PandaString &moduleName, const std::string_view &func)
{
    constexpr std::string_view OHOS_PLUGIN_PREFIX = "@ohos";
    constexpr size_t OHOS_PLUGIN_PREFIX_SIZE = OHOS_PLUGIN_PREFIX.size();
    return moduleName.length() >= OHOS_PLUGIN_PREFIX_SIZE &&
           moduleName.compare(0, OHOS_PLUGIN_PREFIX_SIZE, OHOS_PLUGIN_PREFIX) == 0 && func == "requireNapi";
}

JSValue *JSRuntimeLoadModule(EtsString *module)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    NapiEscapableScope jsHandleScope(env);

    PandaString moduleName = module->GetMutf8();
#if defined(PANDA_TARGET_OHOS) || defined(PANDA_JS_ETS_HYBRID_MODE)
    if (NotNativeOhmUrl(moduleName)) {
        return LoadJSModule(moduleName);
    }
#endif
    auto [mod, func] = ResolveModuleName(moduleName);

    napi_value modObj;
    {
        ScopedNativeCodeThread etsNativeScope(coro);
        napi_value requireFn;
        NAPI_CHECK_FATAL(napi_get_named_property(env, GetGlobal(env), func.data(), &requireFn));
        INTEROP_FATAL_IF(GetValueType(env, requireFn) != napi_function);
        napi_value jsName;
        NAPI_CHECK_FATAL(napi_create_string_utf8(env, mod.data(), NAPI_AUTO_LENGTH, &jsName));
        std::array<napi_value, 1> args = {jsName};
        napi_value recv;
        NAPI_CHECK_FATAL(napi_get_undefined(env, &recv));
        napi_value loadedObj;
        auto status = napi_call_function(env, recv, requireFn, args.size(), args.data(), &loadedObj);
        if (!CheckModuleLoadStatus(coro, status, moduleName, loadedObj)) {
            return nullptr;
        }
        if (NeedWrapDefault(moduleName, func) && GetValueType(env, loadedObj) != napi_function) {
            NAPI_CHECK_FATAL(napi_create_object(env, &modObj));
            NAPI_CHECK_FATAL(napi_set_named_property(env, modObj, "default", loadedObj));
        } else {
            modObj = loadedObj;
        }
        jsHandleScope.Escape(modObj);
    }

    ets_proxy::SharedReferenceStorage *storage = ctx->GetSharedRefStorage();
    ets_proxy::SharedReference *sharedRef = storage->GetReference(env, modObj);
    if (sharedRef != nullptr) {
        return JSValue::FromEtsObject(sharedRef->GetEtsObject());
    }
    return JSValue::CreateRefValue(coro, ctx, modObj, napi_object);
}

uint8_t JSRuntimeStrictEqual([[maybe_unused]] JSValue *lhs, [[maybe_unused]] JSValue *rhs)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return 0;
    }
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    bool result = false;
    NAPI_CHECK_FATAL(napi_strict_equals(env, JSConvertJSValue::WrapWithNullCheck(env, lhs),
                                        JSConvertJSValue::WrapWithNullCheck(env, rhs), &result));
    return static_cast<uint8_t>(result);
}

uint8_t JSRuntimeHasProperty(EtsObject *etsObject, EtsString *name)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    bool result = false;
    napi_status jsStatus;
    {
        ScopedNativeCodeThread nativeScope(coro);
        jsStatus = napi_has_property(env, JSConvertEtsObject::WrapWithNullCheck(env, etsObject),
                                     JSConvertString::WrapWithNullCheck(env, name), &result);
    }
    CHECK_NAPI_STATUS(jsStatus, ctx, coro, result);
    return static_cast<uint8_t>(result);
}

static napi_value JSRuntimeGetPropertyImpl(EtsCoroutine *coro, napi_env env, JSValue *object, JSValue *property)
{
    auto jsThis = JSConvertJSValue::WrapWithNullCheck(env, object);
    auto key = JSConvertJSValue::WrapWithNullCheck(env, property);

    napi_value result;
    {
        ScopedNativeCodeThread nativeScope(coro);
        NAPI_CHECK_FATAL(napi_get_property(env, jsThis, key, &result));
    }
    return result;
}

JSValue *JSRuntimeGetProperty(JSValue *object, JSValue *property)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    auto result = JSRuntimeGetPropertyImpl(coro, env, object, property);
    auto res = JSConvertJSValue::UnwrapWithNullCheck(ctx, env, result);
    if (UNLIKELY(!res)) {
        if (NapiIsExceptionPending(env)) {
            ctx->ForwardJSException(coro);
        }
        ASSERT(ctx->SanityETSExceptionPending());
        return {};
    }

    return res.value();
}

void SetPropertyWithObject(JSValue *object, JSValue *property, EtsObject *value)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    auto jsThis = JSConvertJSValue::WrapWithNullCheck(env, object);
    auto key = JSConvertJSValue::WrapWithNullCheck(env, property);
    auto jsValue = JSConvertEtsObject::WrapWithNullCheck(env, value);

    napi_status jsStatus {};
    {
        ScopedNativeCodeThread nativeScope(coro);
        jsStatus = napi_set_property(env, jsThis, key, jsValue);
    }
    if (jsStatus != napi_ok) {
        ctx->ForwardJSException(coro);
        return;
    }
}

void SetIndexedPropertyWithObject(JSValue *object, uint32_t index, EtsObject *value)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    auto jsThis = JSConvertJSValue::WrapWithNullCheck(env, object);
    auto jsValue = JSConvertEtsObject::WrapWithNullCheck(env, value);

    napi_status jsStatus {};
    {
        ScopedNativeCodeThread nativeScope(coro);
        jsStatus = napi_set_element(env, jsThis, index, jsValue);
    }
    if (jsStatus != napi_ok) {
        ctx->ForwardJSException(coro);
        return;
    }
}

void SetNamedPropertyWithObject(JSValue *object, const char *key, EtsObject *value)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    auto jsThis = JSConvertJSValue::WrapWithNullCheck(env, object);
    auto jsValue = JSConvertEtsObject::WrapWithNullCheck(env, value);

    napi_status jsStatus {};
    {
        ScopedNativeCodeThread nativeScope(coro);
        jsStatus = napi_set_named_property(env, jsThis, key, jsValue);
    }
    if (jsStatus != napi_ok) {
        ctx->ForwardJSException(coro);
        return;
    }
}

EtsObject *GetPropertyObject(JSValue *object, JSValue *property)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    auto result = JSRuntimeGetPropertyImpl(coro, env, object, property);
    return JSConvertEtsObject::UnwrapWithNullCheck(ctx, env, result).value();
}

EtsObject *GetNamedPropertyObject(JSValue *object, const char *property)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    return JSValueGetByName<JSConvertEtsObject>(ctx, object, property).value();
}

JSValue *GetNamedPropertyJSValue(JSValue *object, const char *property)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    return JSValueGetByName<JSConvertJSValue>(ctx, object, property).value();
}

EtsObject *GetPropertyObjectByString(JSValue *object, const char *property)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);
    return JSValueGetByName<JSConvertEtsObject>(ctx, object, property).value();
}

EtsObject *InvokeWithObjectReturn(EtsObject *thisObj, EtsObject *func, Span<VMHandle<ObjectHeader>> args)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    PandaVector<TaggedType> realArgs;

    for (auto &objHeader : args) {
        EtsObject *arg = EtsObject::FromCoreType(objHeader.GetPtr());
        auto realArg = JSConvertEtsObject::WrapWithNullCheck(env, arg);
        if (realArg == nullptr) {
            ctx->ForwardJSException(coro);
            return nullptr;
        }
        auto argTaggedType = ArkNapiHelper::GetTaggedType(realArg);
        realArgs.push_back(argTaggedType);
    }

    auto recvEtsObject = JSConvertEtsObject::WrapWithNullCheck(env, thisObj);
    auto thisTaggedType = ArkNapiHelper::GetTaggedType(recvEtsObject);
    auto funcEtsObject = JSConvertEtsObject::WrapWithNullCheck(env, func);
    auto funcTaggedType = ArkNapiHelper::GetTaggedType(funcEtsObject);
    TaggedType *retVal = nullptr;
    {
        ScopedNativeCodeThread nativeScope(coro);
        retVal = common::DynamicObjectAccessorUtil::CallFunction(thisTaggedType, funcTaggedType, realArgs.size(),
                                                                 realArgs.data());
    }
    if (NapiIsExceptionPending(env)) {
        ctx->ForwardJSException(coro);
        return nullptr;
    }
    return JSConvertEtsObject::UnwrapWithNullCheck(ctx, env, ArkNapiHelper::ToNapiValue(retVal)).value();
}

uint8_t JSRuntimeHasPropertyObject(EtsObject *object, EtsObject *property)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    bool result = false;
    napi_status jsStatus;
    {
        ScopedNativeCodeThread nativeScope(coro);
        jsStatus = napi_has_property(env, JSConvertEtsObject::WrapWithNullCheck(env, object),
                                     JSConvertEtsObject::WrapWithNullCheck(env, property), &result);
    }
    CHECK_NAPI_STATUS(jsStatus, ctx, coro, result);
    return static_cast<uint8_t>(result);
}

uint8_t JSRuntimeHasElement(EtsObject *object, int index)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return 0;
    }
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    bool result = false;
    napi_status jsStatus;
    {
        ScopedNativeCodeThread nativeScope(coro);
        auto objectConv = JSRefConvertResolve(ctx, object->GetClass()->GetRuntimeClass());
        jsStatus = napi_has_element(env, objectConv->Wrap(ctx, object), index, &result);
    }
    CHECK_NAPI_STATUS(jsStatus, ctx, coro, result);
    return static_cast<uint8_t>(result);
}

uint8_t JSRuntimeHasOwnProperty(EtsObject *object, EtsString *name)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return 0;
    }
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    bool result = false;
    napi_status jsStatus;
    {
        ScopedNativeCodeThread nativeScope(coro);
        jsStatus = napi_has_own_property(env, JSConvertEtsObject::WrapWithNullCheck(env, object),
                                         JSConvertString::WrapWithNullCheck(env, name), &result);
    }
    CHECK_NAPI_STATUS(jsStatus, ctx, coro, result);
    return static_cast<uint8_t>(result);
}

uint8_t JSRuntimeHasOwnPropertyObject(EtsObject *etsObject, EtsObject *etsProperty)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    bool result = false;
    napi_status jsStatus;
    {
        ScopedNativeCodeThread nativeScope(coro);
        jsStatus = napi_has_own_property(env, JSConvertEtsObject::WrapWithNullCheck(env, etsObject),
                                         JSConvertEtsObject::WrapWithNullCheck(env, etsProperty), &result);
    }
    CHECK_NAPI_STATUS(jsStatus, ctx, coro, result);
    return static_cast<uint8_t>(result);
}

EtsString *JSRuntimeTypeOf(JSValue *object)
{
    if (object == nullptr) {
        return nullptr;
    }
    switch (object->GetType()) {
        case napi_undefined:
            return EtsString::CreateFromMUtf8("undefined");
        case napi_null:
            return EtsString::CreateFromMUtf8("object");
        case napi_boolean:
            return EtsString::CreateFromMUtf8("boolean");
        case napi_number:
            return EtsString::CreateFromMUtf8("number");
        case napi_string:
            return EtsString::CreateFromMUtf8("string");
        case napi_symbol:
            return EtsString::CreateFromMUtf8("object");
        case napi_object: {
            auto coro = EtsCoroutine::GetCurrent();
            auto ctx = InteropCtx::Current(coro);
            INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
            auto env = ctx->GetJSEnv();
            NapiScope jsHandleScope(env);
            napi_value jsValue = JSConvertJSValue::Wrap(env, object);
            // (note: need to use JS_TYPE, #ICIFWJ)
            if (IsConstructor<true>(env, jsValue, "Number")) {
                return EtsString::CreateFromMUtf8("number");
            }
            if (IsConstructor<true>(env, jsValue, "Boolean")) {
                return EtsString::CreateFromMUtf8("boolean");
            }
            if (IsConstructor<true>(env, jsValue, "String")) {
                return EtsString::CreateFromMUtf8("string");
            }
            return EtsString::CreateFromMUtf8("object");
        }
        case napi_function:
            return EtsString::CreateFromMUtf8("function");
        case napi_external:
            return EtsString::CreateFromMUtf8("external");
        case napi_bigint:
            return EtsString::CreateFromMUtf8("bigint");
        default:
            UNREACHABLE();
    }
}

EtsObject *JSRuntimeInvoke(EtsObject *recv, EtsObject *func, EtsArray *args)
{
    auto coro = EtsCoroutine::GetCurrent();
    if (CheckEtsObjectFoundException(coro, func)) {
        return nullptr;
    }
    HandleScope<ObjectHeader *> scope(coro);
    size_t argc = args->GetLength();
    std::vector<VMHandle<ObjectHeader>> argsVec;
    if (argc > 0) {
        argsVec.reserve(argc);
        for (size_t i = 0; i < args->GetLength(); i++) {
            auto objHeader = args->GetCoreType()->Get<ObjectHeader *>(i);
            argsVec.emplace_back(VMHandle<ObjectHeader>(coro, objHeader));
        }
    }

    Span<VMHandle<ObjectHeader>> internalArgs = Span<VMHandle<ObjectHeader>>(argsVec.data(), argc);
    if (recv == nullptr) {
        return EtsCall(coro, func, internalArgs);
    }
    return EtsCallThis(coro, recv, func, internalArgs);
}

EtsObject *JSRuntimeInstantiate(EtsObject *callable, EtsArray *args)
{
    auto coro = EtsCoroutine::GetCurrent();

    HandleScope<ObjectHeader *> scope(coro);
    size_t argc = args->GetLength();
    std::vector<VMHandle<ObjectHeader>> argsVec;
    if (argc > 0) {
        argsVec.reserve(argc);
        for (size_t i = 0; i < args->GetLength(); i++) {
            auto objHeader = args->GetCoreType()->Get<ObjectHeader *>(i);
            argsVec.emplace_back(VMHandle<ObjectHeader>(coro, objHeader));
        }
    }
    Span<VMHandle<ObjectHeader>> internalArgs = Span<VMHandle<ObjectHeader>>(argsVec.data(), argc);
    return EtsCallNew(coro, callable, internalArgs);
}

void ESValueAnyIndexedSetter(EtsObject *etsObject, int32_t index, EtsObject *value)
{
    auto coro = EtsCoroutine::GetCurrent();
    if (CheckEtsObjectFoundException(coro, etsObject)) {
        return;
    }
    EtsStbyidx(coro, etsObject, index, value);
}

void ESValueAnyNamedSetter(EtsObject *etsObject, EtsString *etsPropName, EtsObject *value)
{
    auto coro = EtsCoroutine::GetCurrent();
    if (CheckEtsObjectFoundException(coro, etsObject)) {
        return;
    }
    PandaVector<uint8_t> tree8Buf;
    auto name = etsPropName->ConvertToStringView(&tree8Buf);
    PandaString str(name);
    panda_file::File::StringData propName = {etsPropName->GetUtf16Length(), utf::CStringAsMutf8(str.c_str())};
    EtsStbyname(coro, etsObject, propName, value);
}

EtsObject *ESValueAnyIndexedGetter(EtsObject *etsObject, int32_t index)
{
    auto coro = EtsCoroutine::GetCurrent();
    if (CheckEtsObjectFoundException(coro, etsObject)) {
        return nullptr;
    }
    return EtsLdbyidx(coro, etsObject, index);
}

EtsObject *ESValueAnyNamedGetter(EtsObject *etsObject, EtsString *etsPropName)
{
    auto coro = EtsCoroutine::GetCurrent();
    if (CheckEtsObjectFoundException(coro, etsObject)) {
        return nullptr;
    }
    PandaVector<uint8_t> tree8Buf;
    auto name = etsPropName->ConvertToStringView(&tree8Buf);
    PandaString str(name);
    panda_file::File::StringData propName = {etsPropName->GetUtf16Length(), utf::CStringAsMutf8(str.c_str())};
    return EtsLdbyname(coro, etsObject, propName);
}

void ESValueAnyStoreByValue(EtsObject *etsObject, EtsObject *key, EtsObject *value)
{
    auto coro = EtsCoroutine::GetCurrent();
    if (CheckEtsObjectFoundException(coro, etsObject)) {
        return;
    }
    EtsStbyval(coro, etsObject, key, value);
}

EtsObject *ESValueAnyLoadByValue(EtsObject *etsObject, EtsObject *value)
{
    auto coro = EtsCoroutine::GetCurrent();
    if (CheckEtsObjectFoundException(coro, etsObject)) {
        return nullptr;
    }
    return EtsLdbyval(coro, etsObject, value);
}

uint8_t ESValueAnyHasProperty(EtsObject *etsObject, EtsString *name)
{
    auto coro = EtsCoroutine::GetCurrent();
    if (CheckEtsObjectFoundException(coro, etsObject)) {
        return static_cast<uint8_t>(false);
    }
    bool result = EtsHasPropertyByName(coro, etsObject, name);
    return static_cast<uint8_t>(result);
}

uint8_t ESValueAnyHasElement(EtsObject *etsObject, int idx)
{
    auto coro = EtsCoroutine::GetCurrent();
    if (CheckEtsObjectFoundException(coro, etsObject)) {
        return static_cast<uint8_t>(false);
    }
    bool result = EtsHasPropertyByIdx(coro, etsObject, idx);
    return static_cast<uint8_t>(result);
}

uint8_t ESValueAnyHasPropertyObject(EtsObject *etsObject, EtsObject *property)
{
    auto coro = EtsCoroutine::GetCurrent();
    if (CheckEtsObjectFoundException(coro, etsObject)) {
        return static_cast<uint8_t>(false);
    }
    bool result = EtsHasPropertyByValue(coro, etsObject, property, false);
    return static_cast<uint8_t>(result);
}

uint8_t ESValueAnyInstanceOf(EtsObject *etsObject, EtsObject *ctor)
{
    auto coro = EtsCoroutine::GetCurrent();
    bool result = false;
    if (etsObject == nullptr || ctor == nullptr) {
        PandaString message = "Need object";
        ThrowEtsException(coro, panda_file_items::class_descriptors::TYPE_ERROR, message.c_str());
    } else {
        result = EtsIsinstance(coro, etsObject, ctor);
    }
    return static_cast<uint8_t>(result);
}

uint8_t ESValueAnyHasOwnProperty(EtsObject *etsObject, EtsString *name)
{
    auto coro = EtsCoroutine::GetCurrent();
    if (CheckEtsObjectFoundException(coro, etsObject)) {
        return static_cast<uint8_t>(false);
    }
    return static_cast<uint8_t>(EtsHasOwnPropertyByName(coro, etsObject, name));
}

uint8_t ESValueAnyHasOwnPropertyObject(EtsObject *etsObject, EtsObject *property)
{
    auto coro = EtsCoroutine::GetCurrent();
    if (CheckEtsObjectFoundException(coro, etsObject)) {
        return static_cast<uint8_t>(false);
    }
    return static_cast<uint8_t>(EtsHasPropertyByValue(coro, etsObject, property, true));
}

uint8_t ESValueAnyIsStaticValue(EtsObject *etsObject)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return 0;
    }
    auto env = ctx->GetJSEnv();

    ets_proxy::SharedReferenceStorage *storage = ctx->GetSharedRefStorage();
    if (storage->HasReference(etsObject, env)) {
        ets_proxy::SharedReference *sharedRef = storage->GetReference(etsObject);
        if (sharedRef->HasJSFlag()) {
            return static_cast<uint8_t>(false);
        }
    }
    return static_cast<uint8_t>(true);
}

EtsObject *CreateObject(JSValue *ctor, Span<VMHandle<ObjectHeader>> args)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    PandaVector<napi_value> realArgs;

    for (auto &objHeader : args) {
        EtsObject *arg = EtsObject::FromCoreType(objHeader.GetPtr());
        if (arg == nullptr) {
            napi_value jsUndefined;
            napi_get_undefined(env, &jsUndefined);
            realArgs.push_back(jsUndefined);
            continue;
        }
        auto refconv = JSRefConvertResolve(ctx, arg->GetClass()->GetRuntimeClass());
        auto realArg = refconv->Wrap(ctx, arg);
        realArgs.push_back(realArg);
    }

    auto initFunc = JSConvertJSValue::WrapWithNullCheck(env, ctor);
    napi_status jsStatus;
    napi_value retVal;
    {
        ScopedNativeCodeThread nativeScope(coro);
        jsStatus = napi_new_instance(env, initFunc, realArgs.size(), realArgs.data(), &retVal);
    }

    if (jsStatus != napi_ok) {
        ctx->ForwardJSException(coro);
        return nullptr;
    }
    return JSConvertEtsObject::UnwrapWithNullCheck(ctx, env, retVal).value();
}

uint8_t JSRuntimeIsPromise(JSValue *object)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    bool result = false;
    NAPI_CHECK_FATAL(napi_is_promise(env, JSConvertJSValue::WrapWithNullCheck(env, object), &result));
    return static_cast<uint8_t>(result);
}

uint8_t JSRuntimeInstanceOfStaticType(JSValue *object, EtsTypeAPIType *paramType)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);
    EtsHandleScope scope(coro);

    if (object == nullptr || paramType == nullptr) {
        return static_cast<uint8_t>(false);
    }

    EtsHandle<EtsTypeAPIType> paramTypeHandle(coro, paramType);
    EtsHandle<JSValue> objectHandle(coro, object);
    EtsField *field = paramType->GetClass()->GetFieldIDByName("cls", nullptr);
    auto clsObj = paramTypeHandle->GetFieldObject(field);
    if (clsObj == nullptr) {
        INTEROP_LOG(ERROR) << "failed to get field by name";
        return static_cast<uint8_t>(false);
    }
    auto cls = reinterpret_cast<EtsClass *>(clsObj);
    napi_value jsValue = JSConvertJSValue::Wrap(env, objectHandle.GetPtr());
    if (jsValue == nullptr) {
        return static_cast<uint8_t>(false);
    }
    auto etsObjectRes = JSConvertEtsObject::UnwrapImpl(ctx, env, jsValue);
    if (!etsObjectRes.has_value()) {
        return static_cast<uint8_t>(false);
    }
    EtsObject *etsObject = etsObjectRes.value();
    bool result = cls->IsAssignableFrom(etsObject->GetClass());
    return static_cast<uint8_t>(result);
}

EtsString *JSValueToString(JSValue *object)
{
    ASSERT(object != nullptr);

    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    napi_env env = ctx->GetJSEnv();

    NapiScope jsHandleScope(env);

    napi_value strObj;
    [[maybe_unused]] napi_status status;
    {
        auto napiValue = JSConvertJSValue::Wrap(env, object);

        ScopedNativeCodeThread nativeScope(coro);  // NOTE: Scope(Native/Managed)CodeThread should be optional here
        status = napi_coerce_to_string(env, napiValue, &strObj);
    }

#if defined(PANDA_TARGET_OHOS) || defined(PANDA_JS_ETS_HYBRID_MODE)
    // #22503 napi_coerce_to_string in arkui/napi implementation returns napi_ok in case of exception.
    if (UNLIKELY(NapiIsExceptionPending(env))) {
        ctx->ForwardJSException(coro);
        return nullptr;
    }
#else
    if (UNLIKELY(status != napi_ok)) {
        INTEROP_FATAL_IF(status != napi_string_expected && status != napi_pending_exception);
        ctx->ForwardJSException(coro);
        return nullptr;
    }
#endif

    auto res = JSConvertString::UnwrapWithNullCheck(ctx, env, strObj);
    if (UNLIKELY(!res.has_value())) {
        if (NapiIsExceptionPending(env)) {
            ctx->ForwardJSException(coro);
        }
        ASSERT(ctx->SanityETSExceptionPending());
        return nullptr;
    }
    return res.value();
}

napi_value ToLocal(void *value)
{
    return reinterpret_cast<napi_value>(value);
}

void *CompilerGetJSNamedProperty(void *val, char *propStr)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    napi_env env = ctx->GetJSEnv();
    auto jsVal = ToLocal(val);
    napi_value res;
    napi_status rc = napi_get_named_property(env, jsVal, propStr, &res);
    if (UNLIKELY(rc == napi_object_expected || NapiThrownGeneric(rc))) {
        ctx->ForwardJSException(coro);
        ASSERT(NapiIsExceptionPending(env));
        return nullptr;
    }
    return res;
}

void *CompilerGetJSProperty(void *val, void *prop)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    napi_env env = ctx->GetJSEnv();
    auto jsVal = ToLocal(val);
    napi_value res;
    napi_status rc = napi_get_property(env, jsVal, ToLocal(prop), &res);
    if (UNLIKELY(rc == napi_object_expected || NapiThrownGeneric(rc))) {
        ctx->ForwardJSException(coro);
        ASSERT(NapiIsExceptionPending(env));
        return nullptr;
    }
    return res;
}

void *CompilerGetJSElement(void *val, int32_t index)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    auto env = ctx->GetJSEnv();

    napi_value res;
    auto rc = napi_get_element(env, ToLocal(val), index, &res);
    if (UNLIKELY(NapiThrownGeneric(rc))) {
        ctx->ForwardJSException(coro);
        return nullptr;
    }
    return res;
}

void *CompilerJSCallCheck(void *fn)
{
    auto jsFn = ToLocal(fn);

    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    napi_env env = ctx->GetJSEnv();
    if (UNLIKELY(GetValueType(env, jsFn) != napi_function)) {
        ctx->ThrowJSTypeError(env, "call target is not a function");
        ctx->ForwardJSException(coro);
        return nullptr;
    }
    return fn;
}

void *CompilerJSNewInstance(void *fn, uint32_t argc, void *args)
{
    auto jsFn = ToLocal(fn);
    auto jsArgs = reinterpret_cast<napi_value *>(args);

    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    napi_env env = ctx->GetJSEnv();

    napi_value jsRet;
    napi_status jsStatus;
    {
        ScopedNativeCodeThread nativeScope(coro);
        jsStatus = napi_new_instance(env, jsFn, argc, jsArgs, &jsRet);
    }

    if (UNLIKELY(jsStatus != napi_ok)) {
        INTEROP_FATAL_IF(jsStatus != napi_pending_exception);
        ctx->ForwardJSException(coro);
        INTEROP_LOG(DEBUG) << "JSValueJSCall: exit with pending exception";
        return nullptr;
    }
    return jsRet;
}

void CreateLocalScope()
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return;
    }
    napi_env env = ctx->GetJSEnv();
    auto scopesStorage = ctx->GetLocalScopesStorage();
    ASSERT(coro->IsCurrentFrameCompiled());
    scopesStorage->CreateLocalScope(env, coro->GetCurrentFrame());
}

void CompilerDestroyLocalScope()
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return;
    }
    napi_env env = ctx->GetJSEnv();
    auto scopesStorage = ctx->GetLocalScopesStorage();
    ASSERT(coro->IsCurrentFrameCompiled());
    scopesStorage->DestroyTopLocalScope(env, coro->GetCurrentFrame());
}

void *CompilerLoadJSConstantPool()
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    return ctx->GetConstStringStorage()->GetConstantPool();
}

void CompilerInitJSCallClassForCtx(void *klassPtr)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return;
    }
    auto klass = reinterpret_cast<Class *>(klassPtr);
    ctx->GetConstStringStorage()->LoadDynamicCallClass(klass);
}

void *CompilerConvertVoidToLocal()
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    napi_env env = ctx->GetJSEnv();
    return GetUndefined(env);
}

void *CompilerConvertRefTypeToLocal(EtsObject *etsValue)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    napi_env env = ctx->GetJSEnv();

    auto ref = etsValue->GetCoreType();
    if (UNLIKELY(ref == nullptr)) {
        return GetNull(env);
    }

    auto klass = ref->template ClassAddr<Class>();
    // start fastpath
    if (klass == ctx->GetJSValueClass()) {
        auto value = JSValue::FromEtsObject(EtsObject::FromCoreType(ref));
        napi_value res = JSConvertJSValue::Wrap(env, value);
        if (UNLIKELY(res == nullptr)) {
            ctx->ForwardJSException(coro);
        }
        return res;
    }
    if (klass->IsStringClass()) {
        auto value = EtsString::FromEtsObject(EtsObject::FromCoreType(ref));
        napi_value res = JSConvertString::Wrap(env, value);
        if (UNLIKELY(res == nullptr)) {
            ctx->ForwardJSException(coro);
        }
        return res;
    }
    // start slowpath
    HandleScope<ObjectHeader *> scope(coro);
    VMHandle<ObjectHeader> handle(coro, ref);
    auto refconv = JSRefConvertResolve(ctx, klass);
    auto res = refconv->Wrap(ctx, EtsObject::FromCoreType(handle.GetPtr()));
    if (UNLIKELY(res == nullptr)) {
        ctx->ForwardJSException(coro);
    }
    return res;
}

EtsString *CompilerConvertLocalToString(void *value)
{
    auto jsVal = ToLocal(value);

    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    napi_env env = ctx->GetJSEnv();
    // NOTE(kprokopenko): can't assign undefined to EtsString *, see #14765
    if (UNLIKELY(IsNullOrUndefined(env, jsVal))) {
        return nullptr;
    }

    auto res = JSConvertString::Unwrap(ctx, env, jsVal);
    if (UNLIKELY(!res.has_value())) {
        if (NapiIsExceptionPending(env)) {
            ctx->ForwardJSException(coro);
        }
        ASSERT(ctx->SanityETSExceptionPending());
        return nullptr;
    }
    return res.value();
}

EtsObject *CompilerConvertLocalToRefType(void *klassPtr, void *value)
{
    auto jsVal = ToLocal(value);
    auto klass = reinterpret_cast<Class *>(klassPtr);

    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    napi_env env = ctx->GetJSEnv();
    if (UNLIKELY(IsNull(env, jsVal))) {
        return ctx->GetNullValue();
    }
    if (UNLIKELY(IsUndefined<true>(env, jsVal))) {
        return nullptr;
    }

    // start slowpath
    auto refconv = JSRefConvertResolve<true>(ctx, klass);
    if (UNLIKELY(refconv == nullptr)) {
        if (NapiIsExceptionPending(env)) {
            ctx->ForwardJSException(coro);
        }
        ASSERT(ctx->SanityETSExceptionPending());
        return nullptr;
    }
    ObjectHeader *res = refconv->Unwrap(ctx, jsVal)->GetCoreType();
    if (UNLIKELY(res == nullptr)) {
        if (NapiIsExceptionPending(env)) {
            ctx->ForwardJSException(coro);
        }
        ASSERT(ctx->SanityETSExceptionPending());
        return nullptr;
    }
    return EtsObject::FromCoreType(res);
}

EtsString *JSONStringify(JSValue *jsvalue)
{
    ASSERT(jsvalue != nullptr);

    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);
    auto global = GetGlobal(env);
    napi_value jsonInstance;
    napi_value stringify;
    napi_value result;
    auto object = JSConvertJSValue::Wrap(env, jsvalue);
    std::array<napi_value, 1> args = {object};
    napi_status jsStatus;

    {
        ScopedNativeCodeThread nativeScope(coro);
        NAPI_CHECK_FATAL(napi_get_named_property(env, global, "JSON", &jsonInstance));
        NAPI_CHECK_FATAL(napi_get_named_property(env, jsonInstance, "stringify", &stringify));
        jsStatus = napi_call_function(env, jsonInstance, stringify, args.size(), args.data(), &result);
    }
    if (UNLIKELY(jsStatus != napi_ok)) {
        INTEROP_FATAL_IF(jsStatus != napi_pending_exception);
        ctx->ForwardJSException(coro);
        return nullptr;
    }
    if (IsUndefined<true>(env, result)) {
        return EtsString::CreateFromMUtf8("undefined");
    }
    auto res = JSConvertString::Unwrap(ctx, env, result);
    return res.value_or(nullptr);
}

void SettleJsPromise(EtsObject *value, napi_deferred deferred, EtsInt state)
{
    auto *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro->GetWorker()->IsMainWorker());
    auto *ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return;
    }
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    napi_env env = ctx->GetJSEnv();
    napi_value completionValue;

    NapiScope napiScope(env);
    if (value == nullptr) {
        completionValue = GetUndefined(env);
    } else {
        auto refconv = JSRefConvertResolve(ctx, value->GetClass()->GetRuntimeClass());
        completionValue = refconv->Wrap(ctx, value);
    }
    ScopedNativeCodeThread nativeScope(coro);
    napi_status status = state == EtsPromise::STATE_RESOLVED ? napi_resolve_deferred(env, deferred, completionValue)
                                                             : napi_reject_deferred(env, deferred, completionValue);
    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Cannot resolve promise");
    }
}

void PromiseInteropResolve(EtsObject *value, EtsLong deferred)
{
    SettleJsPromise(value, reinterpret_cast<napi_deferred>(deferred), EtsPromise::STATE_RESOLVED);
}

void PromiseInteropReject(EtsObject *value, EtsLong deferred)
{
    SettleJsPromise(value, reinterpret_cast<napi_deferred>(deferred), EtsPromise::STATE_REJECTED);
}

JSValue *JSRuntimeGetPropertyJSValueyByKey(JSValue *objectValue, JSValue *keyValue)
{
    ASSERT(objectValue != nullptr);
    ASSERT(keyValue != nullptr);

    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    auto env = ctx->GetJSEnv();

    NapiScope jsHandleScope(env);

    napi_value objectNapiValue = JSConvertJSValue::Wrap(env, objectValue);
    napi_value keyNapiValue = JSConvertJSValue::Wrap(env, keyValue);

    napi_value result;
    auto status = napi_get_property(env, objectNapiValue, keyNapiValue, &result);
    if (UNLIKELY(status == napi_object_expected || NapiThrownGeneric(status))) {
        if (NapiIsExceptionPending(env)) {
            ctx->ForwardJSException(coro);
        }
        ASSERT(ctx->SanityETSExceptionPending());
        return nullptr;
    }
    INTEROP_FATAL_IF(status != napi_ok);

    auto res = JSConvertJSValue::UnwrapWithNullCheck(ctx, env, result);
    if (UNLIKELY(!res)) {
        if (NapiIsExceptionPending(env)) {
            ctx->ForwardJSException(coro);
        }
        ASSERT(ctx->SanityETSExceptionPending());
        return nullptr;
    }

    return res.value();
}

EtsEscompatArrayBuffer *TransferArrayBufferToStatic(ESValue *object)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    [[maybe_unused]] EtsHandleScope s(coro);
    EtsHandle<JSValue> objHandle(coro, object->GetEo());
    NapiScope jsHandleScope(env);

    napi_value dynamicArrayBuffer = JSValue::GetNapiValue(coro, ctx, objHandle);

    bool isArrayBuffer = false;
    NAPI_CHECK_FATAL(napi_is_arraybuffer(env, dynamicArrayBuffer, &isArrayBuffer));
    if (!isArrayBuffer) {
        ctx->ThrowETSError(coro, "Dynamic object is not arraybuffer");
    }

    void *data = nullptr;
    size_t byteLength = 0;
    // NOTE(dslynko, #23919): finalize semantics of resizable ArrayBuffers
    NAPI_CHECK_FATAL(napi_get_arraybuffer_info(env, dynamicArrayBuffer, &data, &byteLength));

    auto *arrayBuffer = EtsEscompatArrayBuffer::Create(coro, byteLength);
    if (UNLIKELY(arrayBuffer == nullptr)) {
        return nullptr;
    }
    auto *etsData = arrayBuffer->GetData<uint8_t *>();
    if (LIKELY(byteLength > 0 && etsData != nullptr)) {
        std::copy_n(reinterpret_cast<uint8_t *>(data), byteLength, etsData);
    }
    return arrayBuffer;
}

EtsObject *TransferArrayBufferToDynamic(EtsEscompatArrayBuffer *staticArrayBuffer)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return {};
    }
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    napi_value dynamicArrayBuffer = nullptr;
    void *data;
    // NOTE(dslynko, #23919): finalize semantics of resizable ArrayBuffers
    NAPI_CHECK_FATAL(napi_create_arraybuffer(env, staticArrayBuffer->GetByteLength(), &data, &dynamicArrayBuffer));
    std::copy_n(reinterpret_cast<const uint8_t *>(staticArrayBuffer->GetData()), staticArrayBuffer->GetByteLength(),
                reinterpret_cast<uint8_t *>(data));

    JSValue *etsJSValue = JSValue::Create(coro, ctx, dynamicArrayBuffer);
    return reinterpret_cast<EtsObject *>(etsJSValue);
}

EtsObject *CreateDynamicTypedArray(EtsEscompatArrayBuffer *staticArrayBuffer, int32_t typedArrayType, double length,
                                   double byteOffset)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return {};
    }
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    napi_value dynamicArrayBuffer = nullptr;
    void *data;
    NAPI_CHECK_FATAL(napi_create_arraybuffer(env, staticArrayBuffer->GetByteLength(), &data, &dynamicArrayBuffer));
    std::copy_n(reinterpret_cast<const uint8_t *>(staticArrayBuffer->GetData()), staticArrayBuffer->GetByteLength(),
                reinterpret_cast<uint8_t *>(data));

    napi_value dynamicTypedArray = nullptr;
    napi_create_typedarray(env, static_cast<napi_typedarray_type>(typedArrayType), length, dynamicArrayBuffer,
                           byteOffset, &dynamicTypedArray);

    JSValue *etsJSValue = JSValue::Create(coro, ctx, dynamicTypedArray);
    return reinterpret_cast<EtsObject *>(etsJSValue);
}

EtsObject *CreateDynamicDataView(EtsEscompatArrayBuffer *staticArrayBuffer, double byteLength, double byteOffset)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return {};
    }
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    napi_value dynamicArrayBuffer = nullptr;
    void *data;
    NAPI_CHECK_FATAL(napi_create_arraybuffer(env, staticArrayBuffer->GetByteLength(), &data, &dynamicArrayBuffer));
    std::copy_n(reinterpret_cast<const uint8_t *>(staticArrayBuffer->GetData()), staticArrayBuffer->GetByteLength(),
                reinterpret_cast<uint8_t *>(data));

    napi_value dynamicDataView = nullptr;
    napi_create_dataview(env, byteLength, dynamicArrayBuffer, byteOffset, &dynamicDataView);

    JSValue *etsJSValue = JSValue::Create(coro, ctx, dynamicDataView);
    return reinterpret_cast<EtsObject *>(etsJSValue);
}

void SetInteropRuntimeLinker(EtsRuntimeLinker *linker)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return;
    }
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);

    ctx->SetDefaultLinkerContext(linker);
}

EtsRuntimeLinker *GetInteropRuntimeLinker()
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);

    return ctx->GetDefaultInteropLinker();
}

EtsBoolean IsJSInteropRef(EtsObject *value)
{
    bool res = false;
    if (value->GetClass()->GetRuntimeClass()->IsXRefClass()) {
        res = true;
    }
    return ark::ets::ToEtsBoolean(res);
}

EtsLong SerializeHandle(JSValue *value)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    auto env = ctx->GetJSEnv();

    [[maybe_unused]] EtsHandleScope s(coro);
    EtsHandle<JSValue> valueHandle(coro, value);
    napi_value nv = JSValue::GetRefValue(env, valueHandle);

    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);
    void *serializedData;
    napi_status status = napi_serialize_hybrid(env, nv, undefined, undefined, &serializedData);
    INTEROP_FATAL_IF(status != napi_ok);

    return reinterpret_cast<EtsLong>(serializedData);
}

JSValue *DeserializeHandle(EtsLong value)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current();
    auto env = ctx->GetJSEnv();

    void *serializedData = reinterpret_cast<void *>(value);

    napi_value retVal;
    napi_status status = napi_deserialize_hybrid(env, serializedData, &retVal);
    INTEROP_FATAL_IF(status != napi_ok);

    return JSValue::Create(coro, ctx, retVal);
}

EtsObject *JSRuntimeInvokeDynamicFunction(EtsObject *functionObject, EtsObjectArray *args)
{
    auto coro = EtsCoroutine::GetCurrent();
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);

    HandleScope<ObjectHeader *> scope(coro);
    auto staticArgs = args;
    PandaVector<VMHandle<ObjectHeader>> argsVec;
    argsVec.reserve(staticArgs->GetLength());
    for (size_t idx = 0; idx != staticArgs->GetLength(); idx++) {
        ObjectHeader *argHeader = staticArgs->Get(idx)->GetCoreType();
        argsVec.emplace_back(VMHandle<ObjectHeader>(coro, argHeader));
    }

    EtsHandle<EtsObject> functionHandle(coro, functionObject);
    auto xRefObjectOperator = XRefObjectOperator::FromEtsObject(functionHandle);
    return xRefObjectOperator.Invoke(coro, Span<VMHandle<ObjectHeader>>(argsVec.data(), argsVec.size()));
}

}  // namespace ark::ets::interop::js
