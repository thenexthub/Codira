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

#ifndef PANDA_RUNTIME_VM_CORE_CORE_VM_H_
#define PANDA_RUNTIME_VM_CORE_CORE_VM_H_

#include "libarkbase/macros.h"
#include "libarkbase/utils/expected.h"
#include "runtime/include/compiler_interface.h"
#include "runtime/include/mem/panda_smart_pointers.h"
#include "runtime/include/mem/panda_string.h"
#include "runtime/include/panda_vm.h"
#include "runtime/mem/gc/gc_phase.h"
#include "runtime/mem/refstorage/reference.h"

namespace ark {

class Method;
class Runtime;
class Compiler;

namespace core {

class PandaCoreVM final : public PandaVM {
public:
    static Expected<PandaCoreVM *, PandaString> Create(Runtime *runtime, const RuntimeOptions &options);
    ~PandaCoreVM() override;

    static PandaCoreVM *GetCurrent();

    bool Initialize() override;
    bool InitializeFinish() override;
    void UninitializeThreads() override;

    void PreStartup() override;
    void PreZygoteFork() override;
    void PostZygoteFork() override;
    void InitializeGC() override;
    void StartGC() override;
    void StopGC() override;

    void HandleReferences(const GCTask &task, const mem::GC::ReferenceClearPredicateT &pred) override;
    void HandleEnqueueReferences() override;
    void HandleGCFinished() override;

    void VisitVmRoots(const GCRootVisitor & /* visitor */) override;

    mem::HeapManager *GetHeapManager() const override
    {
        return mm_->GetHeapManager();
    }

    mem::GC *GetGC() const override
    {
        return mm_->GetGC();
    }

    mem::GCTrigger *GetGCTrigger() const override
    {
        return mm_->GetGCTrigger();
    }

    mem::GCStats *GetGCStats() const override
    {
        return mm_->GetGCStats();
    }

    ManagedThread *GetAssociatedThread() const override
    {
        return ManagedThread::GetCurrent();
    }

    StringTable *GetStringTable() const override
    {
        return stringTable_;
    }

    mem::MemStatsType *GetMemStats() const override
    {
        return mm_->GetMemStats();
    }

    const RuntimeOptions &GetOptions() const override
    {
        return Runtime::GetOptions();
    }

    MTThreadManager *GetThreadManager() const override
    {
        return threadManager_;
    }

    MonitorPool *GetMonitorPool() const override
    {
        return monitorPool_;
    }

    mem::GlobalObjectStorage *GetGlobalObjectStorage() const override
    {
        return mm_->GetGlobalObjectStorage();
    }

    ark::mem::ReferenceProcessor *GetReferenceProcessor() const override
    {
        ASSERT(referenceProcessor_ != nullptr);
        return referenceProcessor_;
    }

    LanguageContext GetLanguageContext() const override
    {
        return Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    }

    CompilerInterface *GetCompiler() const override
    {
        return compiler_;
    }

    coretypes::String *ResolveString(const panda_file::File &pf, panda_file::File::EntityId id) override
    {
        coretypes::String *str = GetStringTable()->GetInternalStringFast(pf, id);
        if (str != nullptr) {
            return str;
        }
        str = GetStringTable()->GetOrInternInternalString(pf, id, GetLanguageContext());
        return str;
    }

    coretypes::String *ResolveString(Frame *frame, panda_file::File::EntityId id) override
    {
        return PandaCoreVM::ResolveString(*frame->GetMethod()->GetPandaFile(), id);
    }

    Rendezvous *GetRendezvous() const override
    {
        return rendezvous_;
    }

    compiler::RuntimeInterface *GetCompilerRuntimeInterface() const override
    {
        return runtimeIface_;
    }

    bool IsStaticProfileEnabled() const override
    {
        return true;
    }

    ObjectHeader *GetOOMErrorObject() override;

    bool SupportGCSinglePassCompaction() const override
    {
        return true;
    }

protected:
    bool CheckEntrypointSignature(Method *entrypoint) override;
    Expected<int, Runtime::Error> InvokeEntrypointImpl(Method *entrypoint,
                                                       const std::vector<std::string> &args) override;
    [[noreturn]] void HandleUncaughtException() override;

private:
    explicit PandaCoreVM(Runtime *runtime, const RuntimeOptions &options, mem::MemoryManager *mm);

    void PreAllocOOMErrorObject();

    Runtime *runtime_ {nullptr};
    mem::MemoryManager *mm_ {nullptr};
    mem::ReferenceProcessor *referenceProcessor_ {nullptr};
    PandaVector<ObjectHeader *> gcRoots_;
    Rendezvous *rendezvous_ {nullptr};
    CompilerInterface *compiler_ {nullptr};
    MTManagedThread *mainThread_ {nullptr};
    StringTable *stringTable_ {nullptr};
    MonitorPool *monitorPool_ {nullptr};
    MTThreadManager *threadManager_ {nullptr};
    ark::mem::Reference *oomObjRef_ {nullptr};
    compiler::RuntimeInterface *runtimeIface_ {nullptr};

    NO_MOVE_SEMANTIC(PandaCoreVM);
    NO_COPY_SEMANTIC(PandaCoreVM);

    friend class mem::Allocator;
};

}  // namespace core
}  // namespace ark

#endif  // PANDA_RUNTIME_VM_CORE_CORE_VM_H_
