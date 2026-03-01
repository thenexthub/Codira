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

#include "plugins/ets/runtime/interop_js/interop_context.h"
#include "plugins/ets/runtime/ets_call_stack.h"
#include "plugins/ets/runtime/interop_js/interop_context_api.h"

#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/interop_js/js_convert.h"
#include "plugins/ets/runtime/interop_js/interop_common.h"
#include "plugins/ets/runtime/interop_js/code_scopes.h"
#include "plugins/ets/runtime/interop_js/sts_vm_interface_impl.h"
#include "plugins/ets/runtime/types/ets_abc_runtime_linker.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "runtime/include/runtime.h"
#include "runtime/mem/local_object_handle.h"

#include "plugins/ets/runtime/interop_js/event_loop_module.h"
#include "plugins/ets/runtime/interop_js/timer_module.h"
#include "plugins/ets/runtime/interop_js/xgc/xgc.h"
#include "plugins/ets/runtime/interop_js/handshake.h"
#include "plugins/ets/runtime/interop_js/napi_impl/napi_impl.h"
#include "plugins/ets/runtime/interop_js/timer_helper/interop_timer_helper.h"

#if defined(PANDA_USE_ETS_UTILS)
#include "console.h"
#endif

#if defined(PANDA_TARGET_OHOS) || defined(PANDA_JS_ETS_HYBRID_MODE)
// NOLINTBEGIN(readability-identifier-naming, readability-redundant-declaration)
// CC-OFFNXT(G.FMT.10-CPP) project code style
napi_status __attribute__((weak)) napi_create_runtime(napi_env env, napi_env *resultEnv);
napi_status __attribute__((weak)) napi_destroy_runtime(napi_env env);
napi_status __attribute__((weak)) napi_throw_jsvalue(napi_env env, napi_value error);
napi_status __attribute__((weak)) napi_setup_hybrid_environment(napi_env env);
// NOLINTEND(readability-identifier-naming, readability-redundant-declaration)
#endif

// NOTE(konstanting, #23205): this function is not listed in the ENUMERATE_NAPI macro, but now runtime needs it.
// I will find a cleaner solution later, but for now let it stay like this, to make aot and verifier happy.
#if (!defined(PANDA_TARGET_OHOS) && !defined(PANDA_JS_ETS_HYBRID_MODE)) || \
    defined(PANDA_JS_ETS_HYBRID_MODE_NEED_WEAK_SYMBOLS)
extern "C" napi_status __attribute__((weak))  // CC-OFF(G.FMT.10) project code style
napi_add_env_cleanup_hook([[maybe_unused]] napi_env env, [[maybe_unused]] void (*fun)(void *arg),
                          [[maybe_unused]] void *arg)
{
    // NOTE: Empty stub. In CI currently used OHOS 4.0.8, but `napi_add_env_cleanup_hook`
    // is public since 4.1.0. Remove this method after CI upgrade.
    INTEROP_LOG(ERROR) << "napi_add_env_cleanup_hook is implemented in OHOS since 4.1.0, please update" << std::endl;
    return napi_ok;
}
#endif

namespace ark::ets::interop::js {

namespace descriptors = panda_file_items::class_descriptors;

// NOLINTNEXTLINE(modernize-avoid-c-arrays)
static constexpr std::string_view const FUNCTION_INTERFACE_DESCRIPTORS[] = {
    descriptors::FUNCTION0,   descriptors::FUNCTION1,   descriptors::FUNCTION2,   descriptors::FUNCTION3,
    descriptors::FUNCTION4,   descriptors::FUNCTION5,   descriptors::FUNCTION6,   descriptors::FUNCTION7,
    descriptors::FUNCTION8,   descriptors::FUNCTION9,   descriptors::FUNCTION10,  descriptors::FUNCTION11,
    descriptors::FUNCTION12,  descriptors::FUNCTION13,  descriptors::FUNCTION14,  descriptors::FUNCTION15,
    descriptors::FUNCTION16,  descriptors::FUNCTIONN,   descriptors::FUNCTIONR0,  descriptors::FUNCTIONR1,
    descriptors::FUNCTIONR2,  descriptors::FUNCTIONR3,  descriptors::FUNCTIONR4,  descriptors::FUNCTIONR5,
    descriptors::FUNCTIONR6,  descriptors::FUNCTIONR7,  descriptors::FUNCTIONR8,  descriptors::FUNCTIONR9,
    descriptors::FUNCTIONR10, descriptors::FUNCTIONR11, descriptors::FUNCTIONR12, descriptors::FUNCTIONR13,
    descriptors::FUNCTIONR14, descriptors::FUNCTIONR15, descriptors::FUNCTIONR16,
};

static Class *CacheClass(EtsClassLinker *etsClassLinker, std::string_view descriptor)
{
    ASSERT(etsClassLinker != nullptr);
    ASSERT(etsClassLinker->GetClass(descriptor.data()));
    auto klass = etsClassLinker->GetClass(descriptor.data())->GetRuntimeClass();
    ASSERT(klass != nullptr);
    return klass;
}

#if defined(PANDA_TARGET_OHOS)
static void AppStateCallback(int state, int64_t timeStamp)
{
    auto appState = AppState(static_cast<AppState::State>(state), timeStamp);
    auto *etsVm = static_cast<PandaEtsVM *>(Runtime::GetCurrent()->GetPandaVM());
    AppStateManager::GetCurrent()->UpdateAppState(appState);
    etsVm->GetGC()->SetFastGCFlag(appState.GetState() == AppState::State::SENSITIVE_START);
    switch (static_cast<AppState::State>(state)) {
        case AppState::State::SENSITIVE_START:
            etsVm->GetGC()->PostponeGCStart();
            break;
        case AppState::State::SENSITIVE_END:
            [[fallthrough]];
        case AppState::State::COLD_START_FINISHED:
            etsVm->GetGC()->PostponeGCEnd();
            LOG(INFO, GC) << "App cold start finished";
            break;
        default:
            break;
    }
}
#endif  // PANDA_TARGET_OHOS

static bool RegisterAppStateCallback([[maybe_unused]] napi_env env)
{
#if defined(PANDA_TARGET_OHOS)
    auto status = napi_register_appstate_callback(env, AppStateCallback);
    return status == napi_ok;
#else
    return true;
#endif
}

static bool RegisterTimerModule()
{
    ani_vm *vm = nullptr;
    ani_size count = 0;
    // NOTE(konstanting, #23205): port to ANI ASAP
    [[maybe_unused]] auto status = ANI_GetCreatedVMs(&vm, 1, &count);
    ASSERT(status == ANI_OK);

    if (count != 1) {
        INTEROP_LOG(ERROR) << "RegisterTimerModule: No VM found";
        return false;
    }

    ani_env *aniEnv = nullptr;
    status = vm->GetEnv(ANI_VERSION_1, &aniEnv);
    ASSERT(status == ANI_OK);
#ifndef PANDA_BUILD_IN_OHOS_TREE
    auto jsEnv = InteropCtx::Current()->GetJSEnv();
    napi_value global = nullptr;
    napi_get_global(jsEnv, &global);
    ark::ets::interop::js::helper::Init(jsEnv, global);
#endif
    return TimerModule::Init(aniEnv);
}

static void RegisterEventLoopModule(EtsCoroutine *coro)
{
    ASSERT(coro != nullptr);
    ASSERT(coro == coro->GetPandaVM()->GetCoroutineManager()->GetMainThread());
    coro->GetPandaVM()->CreateCallbackPosterFactory<EventLoopCallbackPosterFactoryImpl>();
    coro->GetPandaVM()->SetRunEventLoopFunction(
        [](EventLoopRunMode mode) { return EventLoop::RunEventLoop(static_cast<EventLoopRunMode>(mode)); });
    coro->GetPandaVM()->SetWalkEventLoopFunction(
        [](WalkEventLoopCallback &cb, void *args) { EventLoop::WalkEventLoop(cb, args); });
}

#if defined(PANDA_JS_ETS_HYBRID_MODE)
static PandaUniquePtr<SingleEventPoster> CreateExtSchedulingPoster()
{
    auto schedulingFunc = [] {
        auto *coro = Coroutine::GetCurrent();
        coro->GetManager()->Schedule();
#ifndef PANDA_BUILD_IN_OHOS_TREE
        auto *w = coro->GetContext<StackfulCoroutineContext>()->GetWorker();
        w->TriggerSchedulerExternally(coro);
#endif
    };
    return MakePandaUnique<SingleEventPoster>(std::move(schedulingFunc));
}
#endif  // PANDA_JS_ETS_HYBRID_MODE

std::atomic_uint32_t ConstStringStorage::qnameBufferSize_ {0U};

void ConstStringStorage::LoadDynamicCallClass(Class *klass)
{
    if (klass == lastLoadedClass_) {
        return;
    }
    lastLoadedClass_ = klass;
    auto fields = klass->GetStaticFields();
    ASSERT(fields.Size() == 1);
    auto startFrom = klass->GetFieldPrimitive<uint32_t>(fields[0]);
    napi_value jsArr;
    auto *env = Ctx()->GetJSEnv();
    if (jsStringBufferRef_ == nullptr) {
        jsArr = InitBuffer(GetStringBufferSize());
    } else {
        jsArr = GetReferenceValue(env, jsStringBufferRef_);
        if (startFrom >= stringBuffer_.size()) {
            stringBuffer_.resize(GetStringBufferSize());
        } else if (stringBuffer_[startFrom] != nullptr) {
            return;
        }
    }

    auto *pf = klass->GetPandaFile();
    panda_file::ClassDataAccessor cda(*pf, klass->GetFileId());
    [[maybe_unused]] auto annotationFound = cda.EnumerateAnnotation(
        "Lets/annotation/DynamicCall;", [this, pf, startFrom, jsArr, env](panda_file::AnnotationDataAccessor &ada) {
            for (uint32_t i = 0; i < ada.GetCount(); i++) {
                auto adae = ada.GetElement(i);
                auto *elemName = pf->GetStringData(adae.GetNameId()).data;
                if (!utf::IsEqual(utf::CStringAsMutf8("value"), elemName)) {
                    continue;
                }
                auto arr = adae.GetArrayValue();
                auto count = arr.GetCount();
                for (uint32_t j = 0, bufferIndex = startFrom; j < count; j++, bufferIndex++) {
                    panda_file::File::EntityId stringId(arr.Get<uint32_t>(j));
                    auto data = pf->GetStringData(stringId);
                    napi_value jsStr;
                    NAPI_CHECK_FATAL(
                        napi_create_string_utf8(env, utf::Mutf8AsCString(data.data), data.utf16Length, &jsStr));
                    ASSERT(stringBuffer_[bufferIndex] == nullptr);
                    napi_set_element(env, jsArr, bufferIndex, jsStr);
                    stringBuffer_[bufferIndex] = jsStr;
                }
                return true;
            }
            UNREACHABLE();
        });
    ASSERT(annotationFound.has_value());
}

napi_value ConstStringStorage::GetConstantPool()
{
    return GetReferenceValue(Ctx()->GetJSEnv(), jsStringBufferRef_);
}

napi_value ConstStringStorage::InitBuffer(size_t length)
{
    napi_value jsArr;
    NAPI_CHECK_FATAL(napi_create_array_with_length(Ctx()->GetJSEnv(), length, &jsArr));
    NAPI_CHECK_FATAL(napi_create_reference(Ctx()->GetJSEnv(), jsArr, 1, &jsStringBufferRef_));
    stringBuffer_.resize(GetStringBufferSize());
    return jsArr;
}

CommonJSObjectCache::CommonJSObjectCache(InteropCtx *ctx) : ctx_(ctx)
{
    InitializeCache();
    InitializeRecordProto();
}

CommonJSObjectCache::~CommonJSObjectCache()
{
    napi_env env = ctx_->GetJSEnv();
    if (proxyRef_ != nullptr) {
        napi_delete_reference(env, proxyRef_);
    }
    if (recordProtoRef_ != nullptr) {
        napi_delete_reference(env, recordProtoRef_);
    }
}

napi_value CommonJSObjectCache::GetProxy() const
{
    if (proxyRef_ == nullptr) {
        const_cast<CommonJSObjectCache *>(this)->InitializeCache();
    }
    return GetReferenceValue(ctx_->GetJSEnv(), proxyRef_);
}

napi_value CommonJSObjectCache::GetRecordProto() const
{
    if (recordProtoRef_ == nullptr) {
        const_cast<CommonJSObjectCache *>(this)->InitializeRecordProto();
    }
    return GetReferenceValue(ctx_->GetJSEnv(), recordProtoRef_);
}

void CommonJSObjectCache::InitializeRecordProto()
{
    napi_env env = ctx_->GetJSEnv();

    napi_value protoObj;
    NAPI_CHECK_FATAL(napi_create_object(env, &protoObj));
    napi_value trueValue = GetBooleanValue(env, true);
    NAPI_CHECK_FATAL(napi_set_named_property(env, protoObj, IS_STATIC_PROXY.data(), trueValue));
    NAPI_CHECK_FATAL(napi_create_reference(env, protoObj, 1, &recordProtoRef_));
}

void CommonJSObjectCache::InitializeCache()
{
    napi_env env = ctx_->GetJSEnv();

    napi_value global;
    NAPI_CHECK_FATAL(napi_get_global(env, &global));

    napi_value proxy;
    NAPI_CHECK_FATAL(napi_get_named_property(env, global, "Proxy", &proxy));
    NAPI_CHECK_FATAL(napi_create_reference(env, proxy, 1, &proxyRef_));
}

InteropCtx *ConstStringStorage::Ctx()
{
    ASSERT(ctx_ != nullptr);
    return ctx_;
}

std::shared_ptr<InteropCtx::SharedEtsVmState> InteropCtx::SharedEtsVmState::GetInstance(PandaEtsVM *vm)
{
    os::memory::LockHolder lock(mutex_);
    if (instance_ == nullptr) {
        instance_ = MakePandaShared<SharedEtsVmState>(vm);
    }
    return instance_;
}

// should be called when we would like to check if there are no more InteropCtx instances left
void InteropCtx::SharedEtsVmState::TryReleaseInstance()
{
    os::memory::LockHolder lock(mutex_);
    if (instance_.unique()) {
        // assuming that if the main InteropCtx is destroyed, then the VM shuts down
        instance_ = nullptr;
    }
}

js_proxy::JSProxy *InteropCtx::SharedEtsVmState::GetJsProxyInstance(EtsClass *cls) const
{
    os::memory::LockHolder lock(mutex_);
    auto item = jsProxies_.find(cls);
    if (item != jsProxies_.end()) {
        return item->second.get();
    }
    return nullptr;
}

void InteropCtx::SharedEtsVmState::SetJsProxyInstance(EtsClass *cls, js_proxy::JSProxy *proxy)
{
    os::memory::LockHolder lock(mutex_);
    jsProxies_.insert_or_assign(cls, PandaUniquePtr<js_proxy::JSProxy>(proxy));
}

js_proxy::JSProxy *InteropCtx::SharedEtsVmState::GetInterfaceProxyInstance(std::string &interfaceName) const
{
    os::memory::LockHolder lock(mutex_);
    auto item = interfaceProxies_.find(interfaceName);
    if (item != interfaceProxies_.end()) {
        return item->second.get();
    }
    return nullptr;
}

void InteropCtx::SharedEtsVmState::SetInterfaceProxyInstance(std::string &interfaceName, js_proxy::JSProxy *proxy)
{
    os::memory::LockHolder lock(mutex_);
    interfaceProxies_.insert_or_assign(interfaceName, PandaUniquePtr<js_proxy::JSProxy>(proxy));
}

InteropCtx::SharedEtsVmState::SharedEtsVmState(PandaEtsVM *vm)
{
    EtsClassLinker *etsClassLinker = vm->GetClassLinker();
    pandaEtsVm = vm;
    if (linkerCtx_ == nullptr) {
        linkerCtx_ = etsClassLinker->GetEtsClassLinkerExtension()->GetBootContext();
    }
    CacheClasses(etsClassLinker);
    etsProxyRefStorage = ets_proxy::SharedReferenceStorage::Create(pandaEtsVm);
    ASSERT(etsProxyRefStorage.get() != nullptr);

    EtsClass *promiseInteropClass =
        EtsClass::FromRuntimeClass(CacheClass(etsClassLinker, "Lstd/interop/js/PromiseInterop;"));
    ASSERT(promiseInteropClass != nullptr);
    promiseInteropConnectMethod =
        promiseInteropClass->GetStaticMethod("connectPromise", "Lstd/core/Promise;J:V")->GetPandaMethod();
    ASSERT(promiseInteropConnectMethod != nullptr);

    // xgc-related things
    stsVMInterface = MakePandaUnique<STSVMInterfaceImpl>();
    [[maybe_unused]] bool xgcCreated =
        XGC::Create(vm, etsProxyRefStorage.get(), static_cast<STSVMInterfaceImpl *>(stsVMInterface.get()));
    ASSERT(xgcCreated);

    // the event loop framework is per-EtsVM. Further on, it uses local InteropCtx instances
    // to access the JSVM-specific data
    RegisterEventLoopModule(EtsCoroutine::GetCurrent());
}

InteropCtx::SharedEtsVmState::~SharedEtsVmState()
{
    // will happen in the runtime destruction flow
    XGC::Destroy();
}

void InteropCtx::SharedEtsVmState::CacheClasses(EtsClassLinker *etsClassLinker)
{
    jsRuntimeClass = CacheClass(etsClassLinker, descriptors::JS_RUNTIME);
    jsValueClass = CacheClass(etsClassLinker, descriptors::JS_VALUE);
    esErrorClass = CacheClass(etsClassLinker, descriptors::ES_ERROR);
    objectClass = CacheClass(etsClassLinker, descriptors::OBJECT);
    stringClass = CacheClass(etsClassLinker, descriptors::STRING);
    bigintClass = CacheClass(etsClassLinker, descriptors::BIG_INT);
    nullValueClass = CacheClass(etsClassLinker, descriptors::NULL_VALUE);
    promiseClass = CacheClass(etsClassLinker, descriptors::PROMISE);
    errorClass = CacheClass(etsClassLinker, descriptors::ERROR);
    typeClass = CacheClass(etsClassLinker, descriptors::TYPE);

    boxIntClass = CacheClass(etsClassLinker, descriptors::BOX_INT);
    boxLongClass = CacheClass(etsClassLinker, descriptors::BOX_LONG);

    arrayClass = CacheClass(etsClassLinker, descriptors::ARRAY);

    arrayAsListIntClass = CacheClass(etsClassLinker, descriptors::ARRAY_AS_LIST_INT);

    for (auto descr : FUNCTION_INTERFACE_DESCRIPTORS) {
        functionalInterfaces.insert(CacheClass(etsClassLinker, descr));
    }
}

// NOLINTBEGIN(fuchsia-statically-constructed-objects)
std::shared_ptr<InteropCtx::SharedEtsVmState> InteropCtx::SharedEtsVmState::instance_ {nullptr};
os::memory::Mutex InteropCtx::SharedEtsVmState::mutex_;
ClassLinkerContext *InteropCtx::SharedEtsVmState::linkerCtx_ {nullptr};
ark::mem::Reference *InteropCtx::SharedEtsVmState::refToDefaultLinker_ {nullptr};
// NOLINTEND(fuchsia-statically-constructed-objects)

void InteropCtx::InitExternalInterfaces()
{
    interfaceTable_.SetJobQueue(MakePandaUnique<JsJobQueue>());
    // should be called in the deoptimization and exception handlers
    interfaceTable_.SetClearInteropHandleScopesFunction(
        [this](Frame *frame) { this->DestroyLocalScopeForTopFrame(frame); });
#if defined(PANDA_TARGET_OHOS) || defined(PANDA_JS_ETS_HYBRID_MODE)
    interfaceTable_.SetCreateJSRuntimeFunction([env = GetJSEnv()]() -> ExternalIfaceTable::JSEnv {
        napi_env resultJsEnv = nullptr;
        [[maybe_unused]] auto status = napi_create_runtime(env, &resultJsEnv);
        ASSERT(status == napi_ok);
        napi_value global = nullptr;
        napi_get_global(resultJsEnv, &global);
        ark::ets::interop::js::helper::Init(resultJsEnv, global);
#if defined(PANDA_USE_ETS_UTILS)
        OHOS::JsSysModule::Console::InitConsoleModule(resultJsEnv);
#endif
        return resultJsEnv;
    });

    interfaceTable_.SetGetJSEnvFunction([]() -> ExternalIfaceTable::JSEnv {
        auto *ctx = InteropCtx::Current();
        if (ctx == nullptr) {
            return nullptr;
        }
        return ctx->GetJSEnv();
    });

    interfaceTable_.SetCleanUpJSEnvFunction([](ExternalIfaceTable::JSEnv jsEnv) -> void {
        auto env = static_cast<napi_env>(jsEnv);
        [[maybe_unused]] auto status = napi_destroy_runtime(env);
        ASSERT(status == napi_ok);
    });

#endif
    interfaceTable_.SetCreateInteropCtxFunction([](Coroutine *coro, ExternalIfaceTable::JSEnv jsEnv) {
        auto env = static_cast<napi_env>(jsEnv);
        auto *etsCoro = static_cast<EtsCoroutine *>(coro);
        InteropCtx::Init(etsCoro, env);
#if defined(PANDA_TARGET_OHOS) && defined(PANDA_ETS_INTEROP_JS)
        INTEROP_CODE_SCOPE_ETS_TO_JS(etsCoro);
        // Here need to init interop in the given JSVM instance.
        TryInitInteropInJsEnv(jsEnv);
#endif
    });
}

InteropCtx::InteropCtx(EtsCoroutine *coro, napi_env env)
    : sharedEtsVmState_(SharedEtsVmState::GetInstance(coro->GetPandaVM())),
      jsEnv_(env),
      constStringStorage_(this),
      commonJSObjectCache_(this),
      stackInfoManager_(this, coro)
{
    stackInfoManager_.InitStackInfoIfNeeded();
    ecmaVMIterfaceAdaptor_ = MakePandaUnique<XGCVmAdaptor>(env, nullptr);
    // the per-EtsVM part has to be initialized first
    RegisterBuiltinJSRefConvertors(this);

    InitExternalInterfaces();
    InitJsValueFinalizationRegistry(coro);
}

InteropCtx::~InteropCtx()
{
    Refstor()->Remove(jsvalueFregistryRef_);
}

void InteropCtx::InitJsValueFinalizationRegistry(EtsCoroutine *coro)
{
    auto *method = EtsClass::FromRuntimeClass(sharedEtsVmState_->jsRuntimeClass)
                       ->GetStaticMethod("createFinalizationRegistry", ":Lstd/core/FinalizationRegistry;");
    ASSERT(method != nullptr);
    Value res;
    if (coro->IsManagedCode()) {
        res = method->GetPandaMethod()->Invoke(coro, nullptr);
    } else {
        ScopedManagedCodeThread threadScope {coro};
        res = method->GetPandaMethod()->Invoke(coro, nullptr);
    }
    ASSERT(!coro->HasPendingException());
    auto queue = EtsObject::FromCoreType(res.GetAs<ObjectHeader *>());
    ASSERT(queue != nullptr);
    jsvalueFregistryRef_ = Refstor()->Add(queue->GetCoreType(), mem::Reference::ObjectType::GLOBAL);
    ASSERT(jsvalueFregistryRef_ != nullptr);
    jsvalueFregistryRegister_ =
        queue->GetClass()
            ->GetInstanceMethod("register", "Lstd/core/Object;Lstd/core/Object;Lstd/core/Object;:V")
            ->GetPandaMethod();
    ASSERT(jsvalueFregistryRegister_ != nullptr);
}

bool InteropCtx::PushOntoFinalizationRegistry(EtsCoroutine *coro, EtsObject *obj, EtsObject *cbarg)
{
    auto queue = Refstor()->Get(jsvalueFregistryRef_);
    std::array<Value, 4U> args = {Value(queue), Value(obj->GetCoreType()), Value(cbarg->GetCoreType()),
                                  Value(static_cast<ObjectHeader *>(nullptr))};
    jsvalueFregistryRegister_->Invoke(coro, args.data());
    return !coro->HasPendingException();
}

EtsObject *InteropCtx::CreateETSCoreESError(EtsCoroutine *coro, EtsObject *etsObject)
{
    ASSERT_MANAGED_CODE();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coro);
    VMHandle<ObjectHeader> etsObjectHandle(coro, etsObject->GetCoreType());

    Method::Proto proto(Method::Proto::ShortyVector {panda_file::Type(panda_file::Type::TypeId::VOID),
                                                     panda_file::Type(panda_file::Type::TypeId::REFERENCE)},
                        Method::Proto::RefTypeVector {utf::Mutf8AsCString(GetObjectClass()->GetDescriptor())});
    auto ctorName = utf::CStringAsMutf8(panda_file_items::CTOR.data());
    auto ctor = GetESErrorClass()->GetDirectMethod(ctorName, proto);
    ASSERT(ctor != nullptr);

    auto excObj = ObjectHeader::Create(coro, GetESErrorClass());
    if (UNLIKELY(excObj == nullptr)) {
        return nullptr;
    }
    VMHandle<ObjectHeader> excHandle(coro, excObj);

    std::array<Value, 2U> args {Value(excHandle.GetPtr()), Value(etsObjectHandle.GetPtr())};
    ctor->InvokeVoid(coro, args.data());
    auto res = EtsObject::FromCoreType(excHandle.GetPtr());
    if (UNLIKELY(coro->HasPendingException())) {
        return nullptr;
    }
    return res;
}

void InteropCtx::ThrowETSError(EtsCoroutine *coro, napi_value val)
{
    auto ctx = Current(coro);

    ASSERT_MANAGED_CODE();
    if (coro->IsUsePreAllocObj()) {
        coro->SetUsePreAllocObj(false);
        coro->SetException(coro->GetVM()->GetOOMErrorObject());
        return;
    }
    ASSERT(!coro->HasPendingException());

    auto env = ctx->GetJSEnv();
    if (IsUndefined<true>(env, val)) {
        auto etsObj = JSValue::CreateUndefined(coro, ctx)->AsObject();
        EtsObject *esObj = ctx->CreateETSCoreESError(coro, etsObj);
        coro->SetException(esObj->GetCoreType());
        return;
    }

    // To catch `TypeError` or `UserError extends TypeError`
    // 1. Frontend puts catch(compat/TypeError) { <instanceof-rethrow? if UserError expected> }
    //    Where js.UserError will be wrapped into compat/TypeError
    //    NOTE(vpukhov): compat: add intrinsic to obtain JSValue from compat/ instances

    bool isInstanceof = false;
    NAPI_CHECK_FATAL(napi_is_error(env, val, &isInstanceof));
    auto objRefconv = JSRefConvertResolve(ctx, isInstanceof ? ctx->GetErrorClass() : ctx->GetESErrorClass());
    ASSERT(objRefconv != nullptr);
    LocalObjectHandle<EtsObject> etsObj(coro, objRefconv->Unwrap(ctx, val));
    if (UNLIKELY(etsObj.GetPtr() == nullptr)) {
        INTEROP_LOG(INFO) << "Something went wrong while unwrapping pending js exception";
        ASSERT(ctx->SanityETSExceptionPending());
        return;
    }

    auto klass = etsObj->GetClass()->GetRuntimeClass();
    if (LIKELY(ctx->GetErrorClass()->IsAssignableFrom(klass))) {
        coro->SetException(etsObj->GetCoreType());
        return;
    }

    // NOTE(vpukhov): should throw a special error (ESError) with cause set
    auto exc = JSConvertESError::Unwrap(ctx, ctx->GetJSEnv(), val);
    if (LIKELY(exc.has_value())) {
        ASSERT(exc != nullptr);
        coro->SetException(exc.value()->GetCoreType());
    }  // otherwise exception is already set
}

void InteropCtx::ThrowETSError(EtsCoroutine *coro, const char *msg)
{
    ASSERT_MANAGED_CODE();
    ASSERT(!coro->HasPendingException());
    ets::ThrowEtsException(coro, panda_file_items::class_descriptors::ERROR, msg);
}

void InteropCtx::ThrowJSError(napi_env env, const std::string &msg)
{
    INTEROP_LOG(INFO) << "ThrowJSError: " << msg;
    ASSERT(!NapiIsExceptionPending(env));
    NAPI_CHECK_FATAL(napi_throw_error(env, nullptr, msg.c_str()));
}

void InteropCtx::ThrowJSTypeError(napi_env env, const std::string &msg)
{
    INTEROP_LOG(INFO) << "ThrowJSTypeError: " << msg;
    ASSERT(!NapiIsExceptionPending(env));
    NAPI_CHECK_FATAL(napi_throw_type_error(env, nullptr, msg.c_str()));
}

void InteropCtx::ThrowJSValue(napi_env env, napi_value val)
{
    INTEROP_LOG(INFO) << "ThrowJSValue";
    ASSERT(!NapiIsExceptionPending(env));
#if defined(PANDA_TARGET_OHOS) || defined(PANDA_JS_ETS_HYBRID_MODE)
    NAPI_CHECK_FATAL(napi_throw_jsvalue(env, val));
#else
    // At present, UT relies on NodeJS, and the napi_throw interface is still used in UT runtime mode
    // When UT migrates to ArkJS VM, this branch needs to be removed
    NAPI_CHECK_FATAL(napi_throw(env, val));
#endif
}

napi_value InteropCtx::CreateJSTypeError(napi_env env, const std::string &code, const std::string &msg)
{
    INTEROP_LOG(INFO) << "CreateJSTypeError: code: " << code << ", msg: " << msg;
    ASSERT(!NapiIsExceptionPending(env));
    napi_value errorCode;
    NAPI_CHECK_FATAL(napi_create_string_utf8(env, code.data(), code.size(), &errorCode));
    napi_value errorMessage;
    NAPI_CHECK_FATAL(napi_create_string_utf8(env, msg.data(), msg.size(), &errorMessage));
    napi_value error;
    NAPI_CHECK_FATAL(napi_create_type_error(env, errorCode, errorMessage, &error));
    return error;
}

void InteropCtx::InitializeDefaultLinkerCtxIfNeeded(EtsRuntimeLinker *linker)
{
    os::memory::LockHolder lock(InteropCtx::SharedEtsVmState::mutex_);
    // Only cache the first application class linker context
    if (InteropCtx::SharedEtsVmState::linkerCtx_ != nullptr &&
        !InteropCtx::SharedEtsVmState::linkerCtx_->IsBootContext()) {
        return;
    }
    if (linker->GetClass() != PlatformTypes()->coreAbcRuntimeLinker) {
        return;
    }
    InteropCtx::SharedEtsVmState::linkerCtx_ = linker->GetClassLinkerContext();
    InteropCtx::SharedEtsVmState::refToDefaultLinker_ = PandaVM::GetCurrent()->GetGlobalObjectStorage()->Add(
        linker->AsObject()->GetCoreType(), mem::Reference::ObjectType::GLOBAL);
}

void InteropCtx::SetDefaultLinkerContext(EtsRuntimeLinker *linker)
{
    os::memory::LockHolder lock(SharedEtsVmState::mutex_);
    if (!linker->IsInstanceOf(PlatformTypes()->coreRuntimeLinker)) {
        return;
    }
    SharedEtsVmState::linkerCtx_ = linker->GetClassLinkerContext();
    PandaVM::GetCurrent()->GetGlobalObjectStorage()->Remove(SharedEtsVmState::refToDefaultLinker_);
    SharedEtsVmState::refToDefaultLinker_ = PandaVM::GetCurrent()->GetGlobalObjectStorage()->Add(
        linker->AsObject()->GetCoreType(), mem::Reference::ObjectType::GLOBAL);
}

EtsRuntimeLinker *InteropCtx::GetDefaultInteropLinker()
{
    auto header = PandaVM::GetCurrent()->GetGlobalObjectStorage()->Get(SharedEtsVmState::refToDefaultLinker_);
    return EtsRuntimeLinker::FromCoreType(header);
}

void InteropCtx::ForwardEtsException(EtsCoroutine *coro)
{
    auto env = GetJSEnv();
    ASSERT(coro != nullptr);
    ASSERT(coro->HasPendingException());
    ASSERT_MANAGED_CODE();
    LocalObjectHandle<ObjectHeader> exc(coro, coro->GetException());
    coro->ClearException();

    auto klass = exc->ClassAddr<Class>();
    ASSERT(GetErrorClass()->IsAssignableFrom(klass));
    JSRefConvert *refconv = JSRefConvertResolve<true>(this, klass);
    if (UNLIKELY(refconv == nullptr)) {
        INTEROP_LOG(INFO) << "Exception thrown while forwarding ets exception: " << klass->GetDescriptor();
        return;
    }
    napi_value res = refconv->Wrap(this, EtsObject::FromCoreType(exc.GetPtr()));
    if (UNLIKELY(res == nullptr)) {
        return;
    }
    ThrowJSValue(env, res);
}

void InteropCtx::ForwardJSException(EtsCoroutine *coro)
{
    auto env = GetJSEnv();
    const napi_extended_error_info *info = nullptr;
    NAPI_CHECK_FATAL(napi_get_last_error_info(env, &info));
    if (info->error_code != napi_ok && info->error_code != napi_pending_exception) {
        INTEROP_LOG(INFO) << "Napi last error: " << info->error_message;
        ThrowETSError(coro, info->error_message);
        return;
    }
    napi_value excval;
    ASSERT(NapiIsExceptionPending(env));
    NAPI_CHECK_FATAL(napi_get_and_clear_last_exception(env, &excval));
    ThrowETSError(coro, excval);
}

void JSConvertTypeCheckFailed(const char *typeName)
{
    auto ctx = InteropCtx::Current();
    auto env = ctx->GetJSEnv();
    InteropCtx::ThrowJSTypeError(env, typeName + std::string(" expected"));
}

static std::optional<std::string> GetErrorStack(napi_env env, napi_value jsErr)
{
    bool isError;
    if (napi_ok != napi_is_error(env, jsErr, &isError)) {
        return {};
    }
    if (!isError) {
        return "not an Error instance";
    }
    napi_value jsStk;
    if (napi_ok != napi_get_named_property(env, jsErr, "stack", &jsStk)) {
        return {};
    }
    size_t length;
    if (napi_ok != napi_get_value_string_utf8(env, jsStk, nullptr, 0, &length)) {
        return {};
    }
    std::string value;
    value.resize(length);
    // +1 for NULL terminated string!!!
    if (napi_ok != napi_get_value_string_utf8(env, jsStk, value.data(), value.size() + 1, &length)) {
        return {};
    }
    return value;
}

static std::optional<std::string> NapiTryDumpStack(napi_env env)
{
    bool isPending;
    if (napi_ok != napi_is_exception_pending(env, &isPending)) {
        return {};
    }

    std::string pendingErrorMsg;
    if (isPending) {
        napi_value valuePending;
        if (napi_ok != napi_get_and_clear_last_exception(env, &valuePending)) {
            return {};
        }
        auto resStk = GetErrorStack(env, valuePending);
        if (resStk.has_value()) {
            pendingErrorMsg = "\nWith pending exception:\n" + resStk.value();
        } else {
            pendingErrorMsg = "\nFailed to stringify pending exception";
        }
    }

    std::string stacktraceMsg;
    {
        napi_value jsDummyStr;
        if (napi_ok !=
            napi_create_string_utf8(env, "probe-stacktrace-not-actual-error", NAPI_AUTO_LENGTH, &jsDummyStr)) {
            return {};
        }
        napi_value jsErr;
        auto rc = napi_create_error(env, nullptr, jsDummyStr, &jsErr);
        if (napi_ok != rc) {
            return {};
        }
        auto resStk = GetErrorStack(env, jsErr);
        stacktraceMsg = resStk.has_value() ? resStk.value() : "failed to stringify probe exception";
    }

    return stacktraceMsg + pendingErrorMsg;
}

[[noreturn]] void InteropCtx::Fatal(const char *msg)
{
    INTEROP_LOG(ERROR) << "InteropCtx::Fatal: " << msg;

    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);

    INTEROP_LOG(ERROR) << "======================== ETS stack ============================";
    auto istk = ctx->GetOrCreateCallStack().GetRecords();
    auto istkIt = istk.rbegin();

    auto printIstkFrames = [&istkIt, &istk](void *fp) {
        while (istkIt != istk.rend() && fp == istkIt->etsFrame) {
            INTEROP_LOG(ERROR) << "<interop> " << (istkIt->descr != nullptr ? istkIt->descr : "unknown");
            istkIt++;
        }
    };

    for (auto stack = StackWalker::Create(coro); stack.HasFrame(); stack.NextFrame()) {
        printIstkFrames(istkIt->etsFrame);
        Method *method = stack.GetMethod();
        ASSERT(method != nullptr);
        INTEROP_LOG(ERROR) << method->GetClass()->GetName() << "." << method->GetName().data << " at "
                           << method->GetLineNumberAndSourceFile(stack.GetBytecodePc());
    }
    ASSERT(istkIt == istk.rend() || istkIt->etsFrame == nullptr);
    printIstkFrames(nullptr);

    auto env = ctx->GetJSEnv();
    INTEROP_LOG(ERROR) << (env != nullptr ? "<ets-entrypoint>" : "current js env is nullptr!");

    if (coro->HasPendingException()) {
        auto exc = EtsObject::FromCoreType(coro->GetException());
        INTEROP_LOG(ERROR) << "With pending exception: " << exc->GetClass()->GetDescriptor();
    }

    if (env != nullptr) {
        INTEROP_LOG(ERROR) << "======================== JS stack =============================";
        std::optional<std::string> jsStk = NapiTryDumpStack(env);
        if (jsStk.has_value()) {
            INTEROP_LOG(ERROR) << jsStk.value();
        } else {
            INTEROP_LOG(ERROR) << "JS stack print failed";
        }
    }

    INTEROP_LOG(ERROR) << "======================== Native stack =========================";
    PrintStack(Logger::Message(Logger::Level::ERROR, Logger::Component::ETS_INTEROP_JS, false).GetStream());
    std::abort();
}

void InteropCtx::Init(EtsCoroutine *coro, napi_env env)
{
    auto *ctx = Runtime::GetCurrent()->GetInternalAllocator()->New<InteropCtx>(coro, env);
    ASSERT(ctx != nullptr);
    auto *worker = coro->GetWorker();
    worker->GetLocalStorage().Set<CoroutineWorker::DataIdx::INTEROP_CTX_PTR>(ctx, Destroy);
    worker->GetLocalStorage().Set<CoroutineWorker::DataIdx::EXTERNAL_IFACES>(&ctx->interfaceTable_);
#ifdef PANDA_JS_ETS_HYBRID_MODE
    Handshake::VmHandshake(env, ctx);
    XGC::GetInstance()->OnAttach(ctx);
    [[maybe_unused]] auto extSchPoster = CreateExtSchedulingPoster();
    ASSERT(extSchPoster != nullptr);
#ifndef PANDA_BUILD_IN_OHOS_TREE
    worker->SetCallbackPoster(std::move(extSchPoster));
#else
    if (!worker->IsMainWorker()) {
        worker->SetCallbackPoster(std::move(extSchPoster));
    }
#endif
#endif  // PANDA_JS_ETS_HYBRID_MODE
}

void InteropCtx::Destroy(void *ptr)
{
    auto *instance = static_cast<InteropCtx *>(ptr);
#if defined(PANDA_JS_ETS_HYBRID_MODE)
    XGC::GetInstance()->OnDetach(instance);
#endif  // PANDA_JS_ETS_HYBRID_MODE
    Runtime::GetCurrent()->GetInternalAllocator()->Delete(instance);
    SharedEtsVmState::TryReleaseInstance();
}

static bool CheckRuntimeOptions([[maybe_unused]] const ark::ets::EtsCoroutine *mainCoro)
{
#if defined(PANDA_JS_ETS_HYBRID_MODE) && !defined(ARK_HYBRID)
    ASSERT(mainCoro != nullptr);
    auto gcType = mainCoro->GetVM()->GetGC()->GetType();
    if ((Runtime::GetOptions().GetXgcTriggerType() != "never") &&
        (gcType != mem::GCType::G1_GC || Runtime::GetOptions().IsNoAsyncJit())) {
        // XGC is not implemented for other GC types
        LOG(ERROR, RUNTIME) << "XGC requires GC type to be g1-gc and no-async-jit option must be false";
        return false;
    }
#endif  // PANDA_JS_ETS_HYBRID_MODE
    return true;
}

// NOTE(wupengyong, #24099): load interop module need formal plan.
bool TryInitInteropInJsEnv(void *napiEnv)
{
    auto env = static_cast<napi_env>(napiEnv);
    // NOLINTBEGIN(modernize-avoid-c-arrays,readability-identifier-naming)
    constexpr char requireNapi[] = "requireNapi";
    constexpr char interopSo[] = "ets_interop_js_napi";
    constexpr char panda[] = "Panda";
    // NOLINTEND(modernize-avoid-c-arrays,readability-identifier-naming)
    napi_value modObj;
    {
        NapiEscapableScope jsHandleScope(env);
        napi_value pandaObj;
        NAPI_CHECK_RETURN(napi_get_named_property(env, GetGlobal(env), panda, &pandaObj));
        if (!IsUndefined(env, pandaObj)) {
            return true;
        }
        napi_value requireFn;
        NAPI_CHECK_RETURN(napi_get_named_property(env, GetGlobal(env), requireNapi, &requireFn));

        INTEROP_RETURN_IF(GetValueType(env, requireFn) != napi_function);
        {
            napi_value jsName;
            NAPI_CHECK_RETURN(napi_create_string_utf8(env, interopSo, NAPI_AUTO_LENGTH, &jsName));
            std::array<napi_value, 1> args = {jsName};
            napi_value recv;
            NAPI_CHECK_RETURN(napi_get_undefined(env, &recv));
            auto status = (napi_call_function(env, recv, requireFn, args.size(), args.data(), &modObj));
            if (status == napi_pending_exception) {
                INTEROP_LOG(ERROR) << "Unable to load module due to exception";
                return false;
            }
            INTEROP_RETURN_IF(status != napi_ok);
        }
        INTEROP_RETURN_IF(IsNull(env, modObj));
        napi_value key;
        NAPI_CHECK_RETURN(napi_create_string_utf8(env, panda, NAPI_AUTO_LENGTH, &key));
        NAPI_CHECK_RETURN(napi_set_property(env, GetGlobal(env), key, modObj));
        jsHandleScope.Escape(modObj);
    }
    return true;
}

// The external interface for ANI
bool CreateMainInteropContext(ark::ets::EtsCoroutine *mainCoro, void *napiEnv)
{
    ASSERT(mainCoro->GetCoroutineManager()->GetMainThread() == mainCoro);
    if (!CheckRuntimeOptions(mainCoro)) {
        return false;
    }
#if defined(PANDA_TARGET_OHOS) || defined(PANDA_JS_ETS_HYBRID_MODE)
    auto env = static_cast<napi_env>(napiEnv);
    NAPI_CHECK_RETURN(napi_setup_hybrid_environment(env));
#endif
    AppStateManager::Create();
    {
        ScopedManagedCodeThread sm(mainCoro);
        InteropCtx::Init(mainCoro, static_cast<napi_env>(napiEnv));
    }

    // NOTE(konstanting): support instantiation in the TimerModule and move this code to the InteropCtx constructor.
    // The TimerModule should be bound to the exact JsEnv
    if (!RegisterTimerModule()) {
        // throw some errors
    }
    if (!RegisterAppStateCallback(InteropCtx::Current()->GetJSEnv())) {
        INTEROP_LOG(ERROR) << "RegisterAppStateCallback failed";
        return false;
    }

    // In the hybrid mode with JSVM=leading VM, we are binding the EtsVM lifetime to the JSVM's env lifetime
    napi_add_env_cleanup_hook(
        InteropCtx::Current()->GetJSEnv(),
        [](void *) {
            AppStateManager::Destroy();
            ark::Runtime::Destroy();
        },
        nullptr);
#if defined(PANDA_TARGET_OHOS)
    return TryInitInteropInJsEnv(napiEnv);
#else
    return true;
#endif
}

/* static */
InteropCallStack &InteropCtx::GetOrCreateCallStack()
{
    auto &coroStorage = EtsCoroutine::GetCurrent()->GetLocalStorage();
    auto *callStk = coroStorage.Get<EtsCoroutine::DataIdx::INTEROP_CALL_STACK_PTR, InteropCallStack *>();
    if (callStk == nullptr) {
        callStk = Runtime::GetCurrent()->GetInternalAllocator()->New<InteropCallStack>();
        coroStorage.Set<EtsCoroutine::DataIdx::INTEROP_CALL_STACK_PTR>(
            callStk, [](void *param) { Runtime::GetCurrent()->GetInternalAllocator()->Delete(param); });
    }
    return *callStk;
}

}  // namespace ark::ets::interop::js
