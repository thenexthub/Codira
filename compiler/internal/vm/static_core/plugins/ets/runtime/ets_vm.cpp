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

#include "plugins/ets/runtime/ets_vm.h"
#include <atomic>

#include "compiler/optimizer/ir/runtime_interface.h"
#include "include/mem/panda_smart_pointers.h"
#include "include/mem/panda_string.h"
#include "jit/profile_saver_worker.h"
#include "libarkbase/macros.h"
#include "plugins/ets/runtime/ani/ani_vm_api.h"
#include "plugins/ets/runtime/ani/verify/verify_ani_vm_api.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_handle.h"
#include "plugins/ets/runtime/ets_handle_scope.h"
#include "plugins/ets/runtime/ets_panda_file_items.h"
#include "plugins/ets/runtime/ets_runtime_interface.h"
#include "plugins/ets/runtime/ets_vm_options.h"
#include "plugins/ets/runtime/mem/ets_reference_processor.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/runtime/types/ets_promise.h"
#include "plugins/ets/runtime/types/ets_job.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "plugins/ets/runtime/types/ets_array.h"
#include "plugins/ets/runtime/types/ets_taskpool.h"
#include "runtime/compiler.h"
#include "runtime/include/runtime.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/init_icu.h"
#include "runtime/coroutines/stackful_coroutine_manager.h"
#include "runtime/coroutines/threaded_coroutine_manager.h"
#include "runtime/mem/lock_config_helper.h"
#include "runtime/class_lock.h"
#include "plugins/ets/stdlib/native/init_native_methods.h"
#include "plugins/ets/runtime/types/ets_error.h"
#include "plugins/ets/runtime/types/ets_abc_runtime_linker.h"
#include "plugins/ets/runtime/types/ets_finalizable_weak_ref_list.h"
#include "plugins/ets/runtime/types/ets_escompat_array.h"
#include "plugins/ets/runtime/intrinsics/helpers/ets_to_string_cache.h"
#include "plugins/ets/runtime/hybrid/mem/static_object_operator.h"
#include "plugins/ets/runtime/finalreg/finalization_registry_manager.h"

#include "plugins/ets/runtime/ets_object_state_table.h"
#include "libarkbase/taskmanager/task_manager.h"

namespace ark::ets {

static PandaEtsVM *g_pandaEtsVM = nullptr;

// Create MemoryManager by RuntimeOptions
static mem::MemoryManager *CreateMM(Runtime *runtime, const RuntimeOptions &options)
{
    mem::MemoryManager::HeapOptions heapOptions {
        nullptr,                                      // is_object_finalizeble_func
        nullptr,                                      // register_finalize_reference_func
        options.GetMaxGlobalRefSize(),                // max_global_ref_size
        options.IsGlobalReferenceSizeCheckEnabled(),  // is_global_reference_size_check_enabled
        MT_MODE_TASK,                                 // multithreading mode
        options.IsUseTlabForAllocations(),            // is_use_tlab_for_allocations
        options.IsStartAsZygote(),                    // is_start_as_zygote
    };

    auto ctx = runtime->GetLanguageContext(panda_file::SourceLang::ETS);
    auto allocator = runtime->GetInternalAllocator();

    mem::GCTriggerConfig gcTriggerConfig(options, panda_file::SourceLang::ETS);

    mem::GCSettings gcSettings(options, panda_file::SourceLang::ETS);

    auto gcType = Runtime::GetGCType(options, panda_file::SourceLang::ETS);

    return mem::MemoryManager::Create(ctx, allocator, gcType, gcSettings, gcTriggerConfig, heapOptions);
}

/* static */
bool PandaEtsVM::CreateTaskManagerIfNeeded(const RuntimeOptions &options)
{
    if (options.GetWorkersType() == "taskmanager" && !Runtime::IsTaskManagerUsed()) {
        taskmanager::TaskManager::Start(options.GetTaskmanagerWorkersCount(),
                                        taskmanager::StringToTaskTimeStats(options.GetTaskStatsType()));
        taskmanager::TaskManager::EnableTimerThread();
        Runtime::SetTaskManagerUsed(true);
    }
    return true;
}

/* static */
Expected<PandaEtsVM *, PandaString> PandaEtsVM::Create(Runtime *runtime, const RuntimeOptions &options)
{
    ASSERT(runtime != nullptr);

    if (!PandaEtsVM::CreateTaskManagerIfNeeded(options)) {
        return Unexpected(PandaString("Cannot create TaskManager"));
    }

    auto mm = CreateMM(runtime, options);
    if (mm == nullptr) {
        return Unexpected(PandaString("Cannot create MemoryManager"));
    }

    auto allocator = mm->GetHeapManager()->GetInternalAllocator();
    auto vm = allocator->New<PandaEtsVM>(runtime, options, mm);
    if (vm == nullptr) {
        return Unexpected(PandaString("Cannot create PandaCoreVM"));
    }

    mem::StaticObjectOperator::Initialize(vm);

    auto classLinker = EtsClassLinker::Create(runtime->GetClassLinker());
    if (!classLinker) {
        allocator->Delete(vm);
        mem::MemoryManager::Destroy(mm);
        return Unexpected(classLinker.Error());
    }
    vm->classLinker_ = std::move(classLinker.Value());

    vm->InitializeGC();
    vm->GetGC()->AddListener(vm->fullGCLongTimeListener_);

    const auto &icuPath = options.GetIcuDataPath();
    if (icuPath == "default") {
        SetIcuDirectory();
    } else {
        u_setDataDirectory(icuPath.c_str());
    }

    vm->coroutineManager_->InitializeScheduler(runtime, vm);

    g_pandaEtsVM = vm;
    return vm;
}

bool PandaEtsVM::Destroy(PandaEtsVM *vm)
{
    if (vm == nullptr) {
        return false;
    }
    g_pandaEtsVM = nullptr;

    vm->SaveProfileInfo();
    vm->UninitializeThreads();
    vm->StopGC();

    auto runtime = Runtime::GetCurrent();
    runtime->GetInternalAllocator()->Delete(vm);

    runtime->StopCoverageListener();

    return true;
}

// CC-OFFNXT(G.FUD.05) solid logic
PandaEtsVM::PandaEtsVM(Runtime *runtime, const RuntimeOptions &options, mem::MemoryManager *mm)
    : ani_vm {ani::GetVMAPI()}, runtime_(runtime), mm_(mm)
{
    ASSERT(runtime_ != nullptr);
    ASSERT(mm_ != nullptr);

    const EtsVmOptions *etsVmOptions = GetEtsVmOptions(options);
    bool isVerifyANI = etsVmOptions == nullptr ? false : etsVmOptions->IsVerifyANI();
    if (isVerifyANI) {
        aniVerifier_ = MakePandaUnique<ani::verify::ANIVerifier>();
        c_api = ani::verify::GetVerifyVMAPI();
    }

    auto heapManager = mm_->GetHeapManager();
    auto allocator = heapManager->GetInternalAllocator();

    runtimeIface_ = allocator->New<EtsRuntimeInterface>();
    if (options.IsIncrementalProfilesaverEnabled()) {
        if (!taskmanager::TaskManager::IsUsed()) {
            LOG(WARNING, RUNTIME)
                << "[profile_saver] Cannot get current taskScheduler, disable incremental profile saver.";
        } else {
            saverWorker_ = allocator->New<ProfileSaverWorker>(allocator);
            LOG(INFO, RUNTIME) << "[profile_saver] Profile saver worker created.";
        }
    } else {
        LOG(INFO, RUNTIME) << "[profile_saver] Incremental profile saver disabled.";
    }
    compiler_ = allocator->New<Compiler>(heapManager->GetCodeAllocator(), allocator, options,
                                         heapManager->GetMemStats(), runtimeIface_);
    stringTable_ = allocator->New<StringTable>();
    monitorPool_ = allocator->New<MonitorPool>(allocator);
    finalizationRegistryManager_ = allocator->New<FinalizationRegistryManager>(this);
    referenceProcessor_ = allocator->New<mem::ets::EtsReferenceProcessor>(mm_->GetGC(), this);
    unhandledObjectManager_ = allocator->New<UnhandledObjectManager>(this);
    fullGCLongTimeListener_ = allocator->New<FullGCLongTimeListener>();

    auto langStr = plugins::LangToRuntimeType(panda_file::SourceLang::ETS);
    const auto &coroType = options.GetCoroutineImpl(langStr);
    CoroutineManagerConfig cfg {
        // enable drain queue interface
        options.IsCoroutineEnableFeaturesAniDrainQueue(plugins::LangToRuntimeType(panda_file::SourceLang::ETS)),
        // enable migration
        options.IsCoroutineEnableFeaturesMigration(plugins::LangToRuntimeType(panda_file::SourceLang::ETS)),
        // enable migrate awakened coroutines
        options.IsCoroutineEnableFeaturesMigrateAwakened(plugins::LangToRuntimeType(panda_file::SourceLang::ETS)),
        // workers_count
        options.GetCoroutineWorkersCount(plugins::LangToRuntimeType(panda_file::SourceLang::ETS)),
        // exclusive workers limit
        options.GetCoroutineEWorkersLimit(plugins::LangToRuntimeType(panda_file::SourceLang::ETS)),
        // enable perf stats
        options.IsCoroutineDumpStats(plugins::LangToRuntimeType(panda_file::SourceLang::ETS)),
        // enable external timer implementation
        options.IsCoroutineEnableFeaturesEnableExternalTimer(plugins::LangToRuntimeType(panda_file::SourceLang::ETS)),
        // number of reserved workers for taskpool
        options.GetTaskpoolMode(plugins::LangToRuntimeType(panda_file::SourceLang::ETS)) ==
                ets::intrinsics::taskpool::TASKPOOL_EAWORKER_MODE
            ? ets::intrinsics::taskpool::TASKPOOL_EAWORKER_INIT_NUM
            : 0};
    if (coroType == "stackful") {
        coroutineManager_ = allocator->New<StackfulCoroutineManager>(cfg, EtsCoroutine::Create<Coroutine>);
    } else {
        coroutineManager_ = allocator->New<ThreadedCoroutineManager>(cfg, EtsCoroutine::Create<Coroutine>);
    }
    rendezvous_ = allocator->New<Rendezvous>(this);
    objStateTable_ = MakePandaUnique<EtsObjectStateTable>(allocator);
    InitializeRandomEngine();
}

PandaEtsVM::~PandaEtsVM()
{
    auto allocator = mm_->GetHeapManager()->GetInternalAllocator();
    ASSERT(allocator != nullptr);

    allocator->Delete(rendezvous_);
    allocator->Delete(runtimeIface_);
    allocator->Delete(coroutineManager_);
    allocator->Delete(referenceProcessor_);
    allocator->Delete(monitorPool_);
    allocator->Delete(finalizationRegistryManager_);
    allocator->Delete(stringTable_);
    allocator->Delete(compiler_);
    allocator->Delete(unhandledObjectManager_);
    allocator->Delete(fullGCLongTimeListener_);
    if (saverWorker_ != nullptr) {
        allocator->Delete(saverWorker_);
    }

    objStateTable_.reset();

    ASSERT(mm_ != nullptr);
    mm_->Finalize();
    mem::MemoryManager::Destroy(mm_);
}

PandaEtsVM *PandaEtsVM::GetCurrent()
{
    return g_pandaEtsVM;
}

static mem::Reference *PreallocSpecialReference(PandaEtsVM *vm, const char *desc, bool nonMovable = false)
{
    EtsClass *cls = vm->GetClassLinker()->GetClass(desc);
    if (cls == nullptr) {
        LOG(FATAL, RUNTIME) << "Cannot find a class for special object " << desc;
    }
    EtsObject *obj = nonMovable ? EtsObject::CreateNonMovable(cls) : EtsObject::Create(cls);
    if (obj == nullptr) {
        LOG(FATAL, RUNTIME) << "Cannot preallocate a special object " << desc;
    }
    return vm->GetGlobalObjectStorage()->Add(obj->GetCoreType(), ark::mem::Reference::ObjectType::GLOBAL);
}

static mem::Reference *PreallocOOMError(PandaEtsVM *vm)
{
    auto *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);

    auto *oom = EtsOutOfMemoryError::Create(coro);
    if (oom == nullptr) {
        LOG(FATAL, RUNTIME) << "Cannot preallocate OOM error";
    }

    return vm->GetGlobalObjectStorage()->Add(oom->AsObject()->GetCoreType(), ark::mem::Reference::ObjectType::GLOBAL);
}

bool PandaEtsVM::Initialize()
{
    if (!ark::intrinsics::Initialize(ark::panda_file::SourceLang::ETS)) {
        LOG(ERROR, RUNTIME) << "Failed to initialize eTS intrinsics";
        return false;
    }

    if (!classLinker_->Initialize()) {
        LOG(FATAL, ETS) << "Cannot initialize ets class linker";
    }
    classLinker_->GetEtsClassLinkerExtension()->InitializeBuiltinClasses();

    if (Runtime::GetOptions().ShouldLoadBootPandaFiles()) {
        // NOLINTNEXTLINE(google-build-using-namespace)
        using namespace panda_file_items::class_descriptors;

        ASSERT(Thread::GetCurrent() != nullptr);
        ASSERT(GetThreadManager()->GetMainThread() == Thread::GetCurrent());
        auto *coro = EtsCoroutine::GetCurrent();

        ASSERT(coro != nullptr);
        coro->GetLocalStorage().Set<EtsCoroutine::DataIdx::ETS_PLATFORM_TYPES_PTR>(
            ToUintPtr(classLinker_->GetEtsClassLinkerExtension()->GetPlatformTypes()));
        ASSERT(PlatformTypes(coro) != nullptr);

        // Should be invoked after PlatformTypes is initialized in coroutine.
        oomObjRef_ = PreallocOOMError(this);
        nullValueRef_ = PreallocSpecialReference(this, NULL_VALUE.data(), true);
        finalizableWeakRefList_ = PreallocSpecialReference(this, FINALIZABLE_WEAK_REF.data());

        coro->SetupNullValue(GetNullValue());

        if (LIKELY(Runtime::GetOptions().IsUseStringCaches())) {
            doubleToStringCache_ = DoubleToStringCache::Create(coro);
            floatToStringCache_ = FloatToStringCache::Create(coro);
            longToStringCache_ = LongToStringCache::Create(coro);
        }

        referenceProcessor_->Initialize();
        coroutineManager_->InitializeManagedStructures();
    }
    [[maybe_unused]] bool cachesCreated =
        (doubleToStringCache_ != nullptr && floatToStringCache_ != nullptr && longToStringCache_ != nullptr);
    LOG_IF(!cachesCreated, WARNING, ETS) << "Cannot initialize number-to-string caches";
    LOG_IF(cachesCreated, DEBUG, ETS) << "Initialized number-to-string caches";

    // Check if Intrinsics/native methods should be initialized, we don't want to attempt to
    // initialize  native methods in certain scenarios where we don't have ets stdlib at our disposal
    if (Runtime::GetOptions().ShouldInitializeIntrinsics()) {
        // NOTE(ksarychev, #18135): Implement napi module registration via loading a separate
        // library
        ani_env *env = EtsCoroutine::GetCurrent()->GetEtsNapiEnv();
        ark::ets::stdlib::InitNativeMethods(env);

        stdLibCache_ = CreateStdLibCache(env);
    }
    const auto lang = plugins::LangToRuntimeType(panda_file::SourceLang::ETS);
    for (const auto &path : Runtime::GetOptions().GetNativeLibraryPath(lang)) {
        nativeLibraryProvider_.AddLibraryPath(ConvertToString(path));
    }

    return true;
}

bool PandaEtsVM::InitializeFinish()
{
    if (Runtime::GetOptions().ShouldLoadBootPandaFiles()) {
        // Preinitialize StackOverflowError, so we don't need to do this when stack overflow occurred
        EtsClass *cls = classLinker_->GetClass(panda_file_items::class_descriptors::STACK_OVERFLOW_ERROR.data());
        if (cls == nullptr) {
            LOG(FATAL, ETS) << "Cannot preinitialize StackOverflowError";
            return false;
        }
    }
    // Initialize platform classes only if intrinsics were loaded, because static initializers might call them
    if (Runtime::GetOptions().ShouldInitializeIntrinsics()) {
        classLinker_->GetEtsClassLinkerExtension()->InitializeFinish();
    }
    return true;
}

void PandaEtsVM::UninitializeThreads()
{
    // Wait until all threads finish the work
    coroutineManager_->WaitForDeregistration();
    coroutineManager_->DestroyMainCoroutine();
    coroutineManager_->Finalize();
}

void PandaEtsVM::PreStartup()
{
    ASSERT(mm_ != nullptr);

    mm_->PreStartup();
}

void PandaEtsVM::PreZygoteFork()
{
    ASSERT(mm_ != nullptr);
    ASSERT(compiler_ != nullptr);
    ASSERT(coroutineManager_ != nullptr);

    mm_->PreZygoteFork();
    compiler_->PreZygoteFork();

    if (saverWorker_ != nullptr) {
        saverWorker_->PreZygoteFork();
    }
    coroutineManager_->PreZygoteFork();

    if (taskmanager::TaskManager::IsUsed()) {
        preForkWorkerCount_ = taskmanager::TaskManager::GetWorkersCount();
        taskmanager::TaskManager::DisableTimerThread();
        taskmanager::TaskManager::SetWorkersCount(0U);
    }
}

void PandaEtsVM::PostZygoteFork()
{
    ASSERT(compiler_ != nullptr);
    ASSERT(mm_ != nullptr);
    ASSERT(coroutineManager_ != nullptr);

    if (taskmanager::TaskManager::IsUsed()) {
        taskmanager::TaskManager::SetWorkersCount(preForkWorkerCount_);
        taskmanager::TaskManager::EnableTimerThread();
    }
    coroutineManager_->PostZygoteFork();
    compiler_->PostZygoteFork();
    mm_->PostZygoteFork();
    if (saverWorker_ != nullptr) {
        saverWorker_->PostZygoteFork();
    }
    // Postpone GC on application start-up
    // Postpone GCEnd method should be called on start-up ending event
    mm_->GetGC()->PostponeGCStart();

    auto runtime = Runtime::GetCurrent();
    runtime->StartCoverageListener();

    PreStartup();
}

void PandaEtsVM::InitializeGC()
{
    ASSERT(mm_ != nullptr);

    mm_->InitializeGC(this);
}

void PandaEtsVM::StartGC()
{
    ASSERT(mm_ != nullptr);

    mm_->StartGC();
}

void PandaEtsVM::StopGC()
{
    ASSERT(mm_ != nullptr);

    if (GetGC()->IsGCRunning()) {
        mm_->StopGC();
    }
}

void PandaEtsVM::HandleReferences(const GCTask &task, const mem::GC::ReferenceClearPredicateT &pred)
{
    ASSERT(mm_ != nullptr);

    auto gc = mm_->GetGC();
    ASSERT(gc != nullptr);

    LOG(DEBUG, REF_PROC) << "Start processing cleared references";
    gc->ProcessReferences(gc->GetGCPhase(), task, pred);
    LOG(DEBUG, REF_PROC) << "Finish processing cleared references";

    GetGlobalObjectStorage()->ClearUnmarkedWeakRefs(gc, pred);
}

void PandaEtsVM::HandleGCRoutineInMutator()
{
    // Handle references only in coroutine
    ASSERT(Coroutine::GetCurrent() != nullptr);
    ASSERT(GetMutatorLock()->HasLock());
    auto coroutine = EtsCoroutine::GetCurrent();
    GetFinalizationRegistryManager()->StartCleanupCoroIfNeeded(coroutine);
    coroutine->GetPandaVM()->CleanFinalizableReferenceList();
}

void PandaEtsVM::HandleGCFinished() {}

bool PandaEtsVM::CheckEntrypointSignature(Method *entrypoint)
{
    ASSERT(entrypoint != nullptr);

    if (entrypoint->GetReturnType().GetId() != panda_file::Type::TypeId::I32 &&
        entrypoint->GetReturnType().GetId() != panda_file::Type::TypeId::VOID) {
        return false;
    }

    if (entrypoint->GetNumArgs() == 0) {
        return true;
    }

    if (entrypoint->GetNumArgs() > 1) {
        return false;
    }

    auto *pf = entrypoint->GetPandaFile();
    ASSERT(pf != nullptr);
    panda_file::MethodDataAccessor mda(*pf, entrypoint->GetFileId());
    panda_file::ProtoDataAccessor pda(*pf, mda.GetProtoId());

    if (pda.GetArgType(0).GetId() != panda_file::Type::TypeId::REFERENCE) {
        return false;
    }

    auto name = pf->GetStringData(pda.GetReferenceType(0));
    std::string_view expectedName(panda_file_items::class_descriptors::STRING_ARRAY);

    return utf::IsEqual({name.data, name.utf16Length},
                        {utf::CStringAsMutf8(expectedName.data()), expectedName.length()});
}

static EtsObjectArray *CreateArgumentsArray(const std::vector<std::string> &args, PandaEtsVM *etsVm)
{
    ASSERT(etsVm != nullptr);

    const char *classDescripor = panda_file_items::class_descriptors::STRING_ARRAY.data();
    EtsClass *arrayKlass = etsVm->GetClassLinker()->GetClass(classDescripor);
    if (arrayKlass == nullptr) {
        LOG(FATAL, RUNTIME) << "Class " << classDescripor << " not found";
        return nullptr;
    }

    EtsCoroutine *coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope(coroutine);
    EtsObjectArray *etsArray = EtsObjectArray::Create(arrayKlass, args.size());
    EtsHandle<EtsObjectArray> arrayHandle(coroutine, etsArray);

    for (size_t i = 0; i < args.size(); i++) {
        EtsString *str = EtsString::CreateFromMUtf8(args[i].data(), args[i].length());
        ASSERT(arrayHandle.GetPtr() != nullptr);
        arrayHandle.GetPtr()->Set(i, str->AsObject());
    }

    return arrayHandle.GetPtr();
}

coretypes::String *PandaEtsVM::CreateString(Method *ctor, ObjectHeader *obj)
{
    EtsString *str = nullptr;
    ASSERT(ctor->GetNumArgs() > 0);  // must be at list this argument
    if (ctor->GetNumArgs() == 1) {
        str = EtsString::CreateNewEmptyString();
    } else if (ctor->GetNumArgs() == 2U) {
        ASSERT(ctor->GetArgType(1).GetId() == panda_file::Type::TypeId::REFERENCE);
        auto *strData = utf::Mutf8AsCString(ctor->GetRefArgType(1).data);
        if (std::strcmp("[C", strData) == 0) {
            auto *array = reinterpret_cast<EtsArray *>(obj);
            str = EtsString::CreateNewStringFromChars(0, array->GetLength(), array);
        } else if ((std::strcmp("Lstd/core/String;", strData) == 0)) {
            str = EtsString::CreateNewStringFromString(reinterpret_cast<EtsString *>(obj));
        } else {
            LOG(FATAL, ETS) << "Non-existent ctor";
        }
    } else {
        LOG(FATAL, ETS) << "Must be 1 or 2 ctor args";
    }
    return str == nullptr ? nullptr : str->GetCoreType();
}

Expected<int, Runtime::Error> PandaEtsVM::InvokeEntrypointImpl(Method *entrypoint, const std::vector<std::string> &args)
{
    ASSERT(Runtime::GetCurrent()->GetLanguageContext(*entrypoint).GetLanguage() == panda_file::SourceLang::ETS);

    EtsCoroutine *coroutine = EtsCoroutine::GetCurrent();
    ASSERT(coroutine != nullptr);

    ScopedManagedCodeThread sj(coroutine);
    if (!classLinker_->InitializeClass(coroutine, EtsClass::FromRuntimeClass(entrypoint->GetClass()))) {
        LOG(ERROR, RUNTIME) << "Cannot initialize class '" << entrypoint->GetClass()->GetName() << "'";
        return Unexpected(Runtime::Error::CLASS_NOT_INITIALIZED);
    }

    [[maybe_unused]] EtsHandleScope scope(coroutine);
    if (entrypoint->GetNumArgs() == 0) {
        auto v = entrypoint->Invoke(coroutine, nullptr);
        return v.GetAs<int>();
    }

    if (entrypoint->GetNumArgs() == 1) {
        EtsObjectArray *etsObjectArray = CreateArgumentsArray(args, this);
        EtsHandle<EtsObjectArray> argsHandle(coroutine, etsObjectArray);
        ASSERT(argsHandle.GetPtr() != nullptr);
        Value argVal(argsHandle.GetPtr()->AsObject()->GetCoreType());
        auto v = entrypoint->Invoke(coroutine, &argVal);

        return v.GetAs<int>();
    }

    // What if entrypoint->GetNumArgs() > 1 ?
    LOG(ERROR, RUNTIME) << "ets entrypoint has args count more than 1 : " << entrypoint->GetNumArgs();
    return Unexpected(Runtime::Error::INVALID_ENTRY_POINT);
}

ObjectHeader *PandaEtsVM::GetOOMErrorObject()
{
    auto obj = GetGlobalObjectStorage()->Get(oomObjRef_);
    ASSERT(obj != nullptr);
    return obj;
}

ObjectHeader *PandaEtsVM::GetNullValue() const
{
    auto obj = GetGlobalObjectStorage()->Get(nullValueRef_);
    ASSERT(obj != nullptr);
    return obj;
}

bool PandaEtsVM::LoadNativeLibrary(ani_env *env, const PandaString &name, bool shouldVerifyPermission,
                                   const PandaString &fileName)
{
    ASSERT_PRINT(Coroutine::GetCurrent()->IsInNativeCode(), "LoadNativeLibrary must be called at native");

    if (auto error = nativeLibraryProvider_.LoadLibrary(env, name, shouldVerifyPermission, fileName)) {
        LOG(ERROR, RUNTIME) << "Cannot load library " << name << ": " << error.value();
        return false;
    }

    return true;
}

void PandaEtsVM::HandleUncaughtException()
{
    auto coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);
    ScopedManagedCodeThread sj(coro);
    [[maybe_unused]] EtsHandleScope scope(coro);

    EtsHandle<EtsObject> exception(coro, EtsObject::FromCoreType(coro->GetException()));

    GetUnhandledObjectManager()->InvokeErrorHandler(coro, exception);
}

void HandleEmptyArguments(PandaVector<Value> &arguments, const GCRootVisitor &visitor, const EtsCoroutine *coroutine)
{
    // arguments may be empty in the following cases:
    // 1. The entrypoint is static and doesn't accept any arguments
    // 2. The coroutine is launched.
    // 3. The entrypoint is the main method
    Method *entrypoint = coroutine->GetManagedEntrypoint();
    panda_file::ShortyIterator it(entrypoint->GetShorty());
    size_t argIdx = 0;
    ++it;  // skip return type
    if (!entrypoint->IsStatic()) {
        // handle 'this' argument
        ASSERT(arguments[argIdx].IsReference());
        ASSERT(arguments[argIdx].GetAs<ObjectHeader *>() != nullptr);
        visitor(mem::GCRoot(mem::RootType::ROOT_THREAD, arguments[argIdx].GetGCRoot()));
        ++argIdx;
    }
    while (it != panda_file::ShortyIterator()) {
        if ((*it).GetId() == panda_file::Type::TypeId::REFERENCE) {
            ASSERT(arguments[argIdx].IsReference());
            ObjectHeader *arg = arguments[argIdx].GetAs<ObjectHeader *>();
            if (arg != nullptr) {
                visitor(mem::GCRoot(mem::RootType::ROOT_THREAD, arguments[argIdx].GetGCRoot()));
            }
        }
        ++it;
        ++argIdx;
    }
}

void PandaEtsVM::AddRootProvider(mem::RootProvider *provider)
{
    os::memory::LockHolder lock(rootProviderlock_);
    ASSERT(rootProviders_.find(provider) == rootProviders_.end());
    rootProviders_.insert(provider);
}

void PandaEtsVM::RemoveRootProvider(mem::RootProvider *provider)
{
    os::memory::LockHolder lock(rootProviderlock_);
    ASSERT(rootProviders_.find(provider) != rootProviders_.end());
    rootProviders_.erase(provider);
}

void PandaEtsVM::VisitVmRoots(const GCRootVisitor &visitor)
{
    PandaVM::VisitVmRoots(visitor);
    GetThreadManager()->EnumerateThreads([visitor](ManagedThread *thread) {
        const auto coroutine = EtsCoroutine::CastFromThread(thread);
        if (auto etsNapiEnv = coroutine->GetEtsNapiEnv()) {
            auto etsStorage = etsNapiEnv->GetEtsReferenceStorage();
            etsStorage->GetAsReferenceStorage()->VisitObjects(visitor, mem::RootType::ROOT_NATIVE_LOCAL);
        }
        if (!coroutine->HasManagedEntrypoint()) {
            return true;
        }
        PandaVector<Value> &arguments = coroutine->GetManagedEntrypointArguments();
        if (!arguments.empty()) {
            HandleEmptyArguments(arguments, visitor, coroutine);
        }
        return true;
    });
    if (LIKELY(Runtime::GetOptions().IsUseStringCaches())) {
        visitor(mem::GCRoot(mem::RootType::ROOT_VM, reinterpret_cast<ObjectHeader **>(&doubleToStringCache_)));
        visitor(mem::GCRoot(mem::RootType::ROOT_VM, reinterpret_cast<ObjectHeader **>(&floatToStringCache_)));
        visitor(mem::GCRoot(mem::RootType::ROOT_VM, reinterpret_cast<ObjectHeader **>(&longToStringCache_)));
        PlatformTypes(this)->VisitRoots(visitor);
    }
    {
        os::memory::LockHolder lock(rootProviderlock_);
        for (auto *rootProvider : rootProviders_) {
            rootProvider->VisitRoots(visitor);
        }
    }
    GetUnhandledObjectManager()->VisitObjects(visitor);
}

void PandaEtsVM::UpdateAndSweepVmRefs(const ReferenceUpdater &updater)
{
    PandaVM::UpdateAndSweepVmRefs(updater);
    objStateTable_->EnumerateObjectStates([&updater](EtsObjectStateInfo *info) {
        auto *obj = info->GetEtsObject()->GetCoreType();
        [[maybe_unused]] ObjectStatus status = updater(&obj);
        ASSERT(status == ObjectStatus::ALIVE_OBJECT);
        info->SetEtsObject(EtsObject::FromCoreType(obj));
    });
    finalizationRegistryManager_->UpdateAndSweep(updater);
    {
        os::memory::LockHolder lock(rootProviderlock_);
        for (auto *rootProvider : rootProviders_) {
            rootProvider->UpdateRefs([&updater](ObjectHeader **ref) {
                auto status = updater(ref);
                return status == ObjectStatus::ALIVE_OBJECT;
            });
        }
    }
}

EtsFinalizableWeakRef *PandaEtsVM::RegisterFinalizerForObject(EtsCoroutine *coro, const EtsHandle<EtsObject> &object,
                                                              void (*finalizer)(void *), void *finalizerArg)
{
    ASSERT_MANAGED_CODE();
    auto *weakRef = EtsFinalizableWeakRef::Create(coro);
    weakRef->SetFinalizer(finalizer, finalizerArg);
    weakRef->SetReferent(object.GetPtr());
    auto *coreList = GetGlobalObjectStorage()->Get(finalizableWeakRefList_);
    auto *weakRefList = EtsFinalizableWeakRefList::FromCoreType(coreList);
    os::memory::LockHolder lh(finalizableWeakRefListLock_);
    ASSERT(weakRefList != nullptr);
    weakRefList->Push(coro, weakRef);
    return weakRef;
}

bool PandaEtsVM::UnregisterFinalizerForObject(EtsCoroutine *coro, EtsFinalizableWeakRef *weakRef)
{
    auto *coreList = GetGlobalObjectStorage()->Get(finalizableWeakRefList_);
    auto *weakRefList = EtsFinalizableWeakRefList::FromCoreType(coreList);
    os::memory::LockHolder lh(finalizableWeakRefListLock_);
    ASSERT(weakRefList != nullptr);
    return weakRefList->Unlink(coro, weakRef);
}

void PandaEtsVM::CleanFinalizableReferenceList()
{
    auto *coreList = GetGlobalObjectStorage()->Get(finalizableWeakRefList_);
    auto *weakRefList = EtsFinalizableWeakRefList::FromCoreType(coreList);
    os::memory::LockHolder lh(finalizableWeakRefListLock_);
    ASSERT(weakRefList != nullptr);
    weakRefList->UnlinkClearedReferences(EtsCoroutine::GetCurrent());
}

void PandaEtsVM::BeforeShutdown()
{
    ScopedManagedCodeThread managedScope(EtsCoroutine::GetCurrent());
    auto *coreList = GetGlobalObjectStorage()->Get(finalizableWeakRefList_);
    auto *weakRefList = EtsFinalizableWeakRefList::FromCoreType(coreList);
    ASSERT(weakRefList != nullptr);
    weakRefList->TraverseAndFinalize();
}

ClassLinkerContext *PandaEtsVM::CreateApplicationRuntimeLinker(const PandaVector<PandaString> &abcFiles)
{
    auto *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);

    [[maybe_unused]] ScopedManagedCodeThread sj(coro);
    [[maybe_unused]] EtsHandleScope scope(coro);

    const auto exceptionHandler = [this, coro]() __attribute__((__noreturn__))
    // CC-OFFNXT(G.FMT.03-CPP) project code style
    {
        ASSERT(coro->HasPendingException());
        [[maybe_unused]] ScopedNativeCodeThread nj(coro);
        HandleUncaughtException();
        UNREACHABLE();
    };

    auto *klass = PlatformTypes(this)->coreAbcRuntimeLinker;
    EtsHandle<EtsAbcRuntimeLinker> linkerHandle(coro, EtsAbcRuntimeLinker::FromEtsObject(EtsObject::Create(klass)));
    ASSERT(linkerHandle.GetPtr() != nullptr);

    EtsHandle<EtsEscompatArray> pathsHandle(coro, EtsEscompatArray::Create(coro, abcFiles.size()));
    for (size_t idx = 0; idx < abcFiles.size(); ++idx) {
        auto utf8Data = reinterpret_cast<const uint8_t *>(abcFiles[idx].data());
        auto *str = EtsString::CreateFromMUtf8(abcFiles[idx].data(), utf::MUtf8ToUtf16Size(utf8Data));
        if (UNLIKELY(str == nullptr)) {
            // Handle possible OOM
            exceptionHandler();
        }
        pathsHandle->EscompatArraySetUnsafe(idx, str->AsObject());
    }
    std::array args {Value(linkerHandle->GetCoreType()), Value(nullptr), Value(pathsHandle->GetCoreType())};

    auto *ctor =
        klass->GetDirectMethod(GetLanguageContext().GetCtorName(), "Lstd/core/RuntimeLinker;Lstd/core/Array;:V");
    ASSERT(ctor != nullptr);
    ctor->GetPandaMethod()->InvokeVoid(coro, args.data());
    if (coro->HasPendingException()) {
        // Print exceptions thrown in constructor (e.g. if file not found) and exit
        exceptionHandler();
    }

    // Save global reference to created application `AbcRuntimeLinker`
    GetGlobalObjectStorage()->Add(linkerHandle->GetCoreType(), mem::Reference::ObjectType::GLOBAL);
    // Safe to return a non-managed object
    return linkerHandle->GetClassLinkerContext();
}

/* static */
void PandaEtsVM::Abort(const char *message /* = nullptr */)
{
    Runtime::Abort(message);
}
}  // namespace ark::ets
