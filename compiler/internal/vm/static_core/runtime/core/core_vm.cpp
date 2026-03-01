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

#include "libarkbase/utils/expected.h"
#include "runtime/compiler.h"
#include "runtime/core/core_vm.h"
#include "runtime/handle_scope-inl.h"
#include "runtime/include/thread.h"
#include "runtime/include/class_linker.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/mem/gc/reference-processor/empty_reference_processor.h"
#include "runtime/mem/lock_config_helper.h"
#include "runtime/mem/refstorage/global_object_storage.h"
#include "runtime/single_thread_manager.h"

namespace ark::core {

// Create MemoryManager by RuntimeOptions
static mem::MemoryManager *CreateMM(const LanguageContext &ctx, mem::InternalAllocatorPtr internalAllocator,
                                    const RuntimeOptions &options)
{
    mem::MemoryManager::HeapOptions heapOptions {
        nullptr,                                      // is_object_finalizeble_func
        nullptr,                                      // register_finalize_reference_func
        options.GetMaxGlobalRefSize(),                // max_global_ref_size
        options.IsGlobalReferenceSizeCheckEnabled(),  // is_global_reference_size_check_enabled
        MT_MODE_MULTI,                                // multithreading mode
        options.IsUseTlabForAllocations(),            // is_use_tlab_for_allocations
        options.IsStartAsZygote(),                    // is_start_as_zygote
    };

    mem::GCTriggerConfig gcTriggerConfig(options, panda_file::SourceLang::PANDA_ASSEMBLY);

    mem::GCSettings gcSettings(options, panda_file::SourceLang::PANDA_ASSEMBLY);

    mem::GCType gcType = Runtime::GetGCType(options, panda_file::SourceLang::PANDA_ASSEMBLY);

    return mem::MemoryManager::Create(ctx, internalAllocator, gcType, gcSettings, gcTriggerConfig, heapOptions);
}

/* static */
Expected<PandaCoreVM *, PandaString> PandaCoreVM::Create(Runtime *runtime, const RuntimeOptions &options)
{
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    mem::MemoryManager *mm = CreateMM(ctx, runtime->GetInternalAllocator(), options);
    if (mm == nullptr) {
        return Unexpected(PandaString("Cannot create MemoryManager"));
    }

    auto allocator = mm->GetHeapManager()->GetInternalAllocator();
    PandaCoreVM *coreVm = allocator->New<PandaCoreVM>(runtime, options, mm);
    if (coreVm == nullptr) {
        return Unexpected(PandaString("Cannot create PandaCoreVM"));
    }

    coreVm->InitializeGC();

    // Create Main Thread
    coreVm->mainThread_ = MTManagedThread::Create(runtime, coreVm);
    coreVm->mainThread_->InitBuffers();
    ASSERT(coreVm->mainThread_ == ManagedThread::GetCurrent());
    coreVm->mainThread_->InitForStackOverflowCheck(ManagedThread::STACK_OVERFLOW_RESERVED_SIZE,
                                                   ManagedThread::STACK_OVERFLOW_PROTECTED_SIZE);

    coreVm->threadManager_->SetMainThread(coreVm->mainThread_);

    return coreVm;
}

PandaCoreVM::PandaCoreVM(Runtime *runtime, const RuntimeOptions &options, mem::MemoryManager *mm)
    : runtime_(runtime), mm_(mm)
{
    mem::HeapManager *heapManager = mm_->GetHeapManager();
    mem::InternalAllocatorPtr allocator = heapManager->GetInternalAllocator();
    runtimeIface_ = allocator->New<PandaRuntimeInterface>();
    compiler_ = allocator->New<Compiler>(heapManager->GetCodeAllocator(), allocator, options,
                                         heapManager->GetMemStats(), runtimeIface_);
    stringTable_ = allocator->New<StringTable>();
    monitorPool_ = allocator->New<MonitorPool>(allocator);
    referenceProcessor_ = allocator->New<mem::EmptyReferenceProcessor>();
    threadManager_ = allocator->New<MTThreadManager>(allocator);
    rendezvous_ = allocator->New<Rendezvous>(this);
}

PandaCoreVM::~PandaCoreVM()
{
    delete mainThread_;

    mem::InternalAllocatorPtr allocator = mm_->GetHeapManager()->GetInternalAllocator();
    allocator->Delete(rendezvous_);
    allocator->Delete(runtimeIface_);
    allocator->Delete(threadManager_);
    allocator->Delete(referenceProcessor_);
    allocator->Delete(monitorPool_);
    allocator->Delete(stringTable_);
    allocator->Delete(compiler_);
    mm_->Finalize();
    mem::MemoryManager::Destroy(mm_);
}

bool PandaCoreVM::Initialize()
{
    if (!intrinsics::Initialize(ark::panda_file::SourceLang::PANDA_ASSEMBLY)) {
        LOG(ERROR, RUNTIME) << "Failed to initialize Core intrinsics";
        return false;
    }

    auto runtime = Runtime::GetCurrent();
    if (runtime->GetOptions().ShouldLoadBootPandaFiles()) {
        PreAllocOOMErrorObject();
    }

    return true;
}

void PandaCoreVM::PreAllocOOMErrorObject()
{
    auto globalObjectStorage = GetGlobalObjectStorage();
    auto runtime = Runtime::GetCurrent();
    LanguageContext ctx = runtime->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    auto *classLinker = runtime->GetClassLinker();
    auto cls = classLinker->GetExtension(ctx)->GetClass(ctx.GetOutOfMemoryErrorClassDescriptor());
    auto oomObj = ObjectHeader::Create(cls);
    if (oomObj == nullptr) {
        LOG(FATAL, RUNTIME) << "Cannot preallocate OOM Error object";
        return;
    }
    oomObjRef_ = globalObjectStorage->Add(oomObj, ark::mem::Reference::ObjectType::GLOBAL);
}

bool PandaCoreVM::InitializeFinish()
{
    // Preinitialize StackOverflowException so we don't need to do this when stack overflow occurred
    auto runtime = Runtime::GetCurrent();
    auto *classLinker = runtime->GetClassLinker();
    LanguageContext ctx = runtime->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    auto *extension = classLinker->GetExtension(ctx);
    classLinker->GetClass(ctx.GetStackOverflowErrorClassDescriptor(), true, extension->GetBootContext());

    return true;
}

void PandaCoreVM::UninitializeThreads()
{
    // Wait until all threads finish the work
    threadManager_->WaitForDeregistration();
    mainThread_->Destroy();
}

void PandaCoreVM::PreStartup()
{
    mm_->PreStartup();
}

void PandaCoreVM::PreZygoteFork()
{
    mm_->PreZygoteFork();
    compiler_->PreZygoteFork();
}

void PandaCoreVM::PostZygoteFork()
{
    compiler_->PostZygoteFork();
    mm_->PostZygoteFork();
}

void PandaCoreVM::InitializeGC()
{
    mm_->InitializeGC(this);
}

void PandaCoreVM::StartGC()
{
    mm_->StartGC();
}

void PandaCoreVM::StopGC()
{
    mm_->StopGC();
}

void PandaCoreVM::HandleReferences(const GCTask &task, const mem::GC::ReferenceClearPredicateT &pred)
{
    LOG(DEBUG, REF_PROC) << "Start processing cleared references";
    mem::GC *gc = mm_->GetGC();
    gc->ProcessReferences(gc->GetGCPhase(), task, pred);
}

// NOTE(alovkov): call ReferenceQueue.add method with cleared references
void PandaCoreVM::HandleEnqueueReferences()
{
    LOG(DEBUG, REF_PROC) << "Start HandleEnqueueReferences";
    mm_->GetGC()->EnqueueReferences();
    LOG(DEBUG, REF_PROC) << "Finish HandleEnqueueReferences";
}

void PandaCoreVM::HandleGCFinished() {}

bool PandaCoreVM::CheckEntrypointSignature(Method *entrypoint)
{
    if (entrypoint->GetNumArgs() == 0) {
        return true;
    }

    if (entrypoint->GetNumArgs() > 1) {
        return false;
    }

    auto *pf = entrypoint->GetPandaFile();
    panda_file::MethodDataAccessor mda(*pf, entrypoint->GetFileId());
    panda_file::ProtoDataAccessor pda(*pf, mda.GetProtoId());

    if (pda.GetArgType(0).GetId() != panda_file::Type::TypeId::REFERENCE) {
        return false;
    }

    auto typeId = pda.GetReferenceType(0);
    auto stringData = pf->GetStringData(typeId);
    const char className[] = "[Lpanda/String;";  // NOLINT(modernize-avoid-c-arrays)

    return utf::IsEqual({stringData.data, stringData.utf16Length},
                        {utf::CStringAsMutf8(className), sizeof(className) - 1});
}

static coretypes::Array *CreateArgumentsArray(const std::vector<std::string> &args, const LanguageContext &ctx,
                                              ClassLinker *classLinker, PandaVM *vm)
{
    const char className[] = "[Lpanda/String;";  // NOLINT(modernize-avoid-c-arrays)
    auto *arrayKlass = classLinker->GetExtension(ctx)->GetClass(utf::CStringAsMutf8(className));
    if (arrayKlass == nullptr) {
        LOG(FATAL, RUNTIME) << "Class " << className << " not found";
    }

    auto thread = MTManagedThread::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    auto *array = coretypes::Array::Create(arrayKlass, args.size());
    VMHandle<coretypes::Array> arrayHandle(thread, array);

    for (size_t i = 0; i < args.size(); i++) {
        auto *str = coretypes::String::CreateFromMUtf8(utf::CStringAsMutf8(args[i].data()), args[i].length(), ctx, vm);
        arrayHandle.GetPtr()->Set(i, str);
    }
    return arrayHandle.GetPtr();
}

Expected<int, Runtime::Error> PandaCoreVM::InvokeEntrypointImpl(Method *entrypoint,
                                                                const std::vector<std::string> &args)
{
    Runtime *runtime = Runtime::GetCurrent();
    MTManagedThread *thread = MTManagedThread::GetCurrent();
    LanguageContext ctx = runtime->GetLanguageContext(*entrypoint);
    ASSERT(ctx.GetLanguage() == panda_file::SourceLang::PANDA_ASSEMBLY);

    ScopedManagedCodeThread sj(thread);
    ClassLinker *classLinker = runtime->GetClassLinker();
    if (!classLinker->InitializeClass(thread, entrypoint->GetClass())) {
        LOG(ERROR, RUNTIME) << "Cannot initialize class '" << entrypoint->GetClass()->GetName() << "'";
        return Unexpected(Runtime::Error::CLASS_NOT_INITIALIZED);
    }

    ObjectHeader *objectHeader = nullptr;
    if (entrypoint->GetNumArgs() == 1) {
        coretypes::Array *argArray = CreateArgumentsArray(args, ctx, runtime_->GetClassLinker(), thread->GetVM());
        objectHeader = argArray;
    }

    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    VMHandle<ObjectHeader> argsHandle(thread, objectHeader);
    Value argVal(argsHandle.GetPtr());
    Value v = entrypoint->Invoke(thread, &argVal);

    return v.GetAs<int>();
}

ObjectHeader *PandaCoreVM::GetOOMErrorObject()
{
    auto globalObjectStorage = GetGlobalObjectStorage();
    auto obj = globalObjectStorage->Get(oomObjRef_);
    ASSERT(obj != nullptr);
    return obj;
}

[[noreturn]] void PandaCoreVM::HandleUncaughtException()
{
    ManagedThread *thread = ManagedThread::GetCurrent();
    ASSERT(thread != nullptr);
    LOG(ERROR, RUNTIME) << "Unhandled exception: " << thread->GetException()->ClassAddr<Class>()->GetName();
    // _exit guarantees a safe completion in case of multi-threading as static destructors aren't called
    _exit(1);
}

void PandaCoreVM::VisitVmRoots(const GCRootVisitor &visitor)
{
    PandaVM::VisitVmRoots(visitor);
    // Visit PT roots
    GetThreadManager()->EnumerateThreads([visitor](ManagedThread *thread) {
        ASSERT(MTManagedThread::ThreadIsMTManagedThread(thread));
        auto mtThread = MTManagedThread::CastFromThread(thread);
        auto ptStorage = mtThread->GetPtReferenceStorage();
        ptStorage->VisitObjects(visitor, mem::RootType::ROOT_PT_LOCAL);
        return true;
    });
}

}  // namespace ark::core
