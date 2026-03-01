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

#ifndef PANDA_RUNTIME_TOOLING_PT_HOOKS_WRAPPER_H
#define PANDA_RUNTIME_TOOLING_PT_HOOKS_WRAPPER_H

#include <atomic>
#include "runtime/include/tooling/debug_interface.h"
#include "libarkbase/os/mutex.h"
#include "runtime/include/mtmanaged_thread.h"
#include "pt_thread_info.h"
#include "pt_hook_type_info.h"

namespace ark::tooling {

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
class PtHooksWrapper : public PtHooks {
public:
    void SetHooks(PtHooks *hooks)
    {
        // Atomic with release order reason: data race with hooks_
        hooks_.store(hooks, std::memory_order_release);
    }

    void EnableGlobalHook(PtHookType hookType)
    {
        globalHookTypeInfo_.Enable(hookType);
    }

    void DisableGlobalHook(PtHookType hookType)
    {
        globalHookTypeInfo_.Disable(hookType);
    }

    void EnableAllGlobalHook()
    {
        globalHookTypeInfo_.EnableAll();
    }

    void DisableAllGlobalHook()
    {
        globalHookTypeInfo_.DisableAll();
    }

    // Wrappers for hooks
    void Breakpoint(PtThread thread, Method *method, const PtLocation &location) override
    {
        // Atomic with acquire order reason: data race with hooks_
        auto *loadedHooks = hooks_.load(std::memory_order_acquire);
        if (loadedHooks == nullptr || !HookIsEnabled(PtHookType::PT_HOOK_TYPE_BREAKPOINT)) {
            return;
        }
        // Atomic with acquire order reason: data race with vmdeath_did_not_happen_
        ASSERT(vmdeathDidNotHappen_.load(std::memory_order_acquire));
        loadedHooks->Breakpoint(thread, method, location);
    }

    void LoadModule(std::string_view pandaFile) override
    {
        // Atomic with acquire order reason: data race with hooks_
        auto *loadedHooks = hooks_.load(std::memory_order_acquire);
        if (loadedHooks == nullptr || !HookIsEnabled(PtHookType::PT_HOOK_TYPE_LOAD_MODULE)) {
            return;
        }
        // Atomic with acquire order reason: data race with vmdeath_did_not_happen_
        ASSERT(vmdeathDidNotHappen_.load(std::memory_order_acquire));
        loadedHooks->LoadModule(pandaFile);
    }

    void Paused(PauseReason reason) override
    {
        // Atomic with acquire order reason: data race with hooks_
        auto *loadedHooks = hooks_.load(std::memory_order_acquire);
        if (loadedHooks == nullptr || !HookIsEnabled(PtHookType::PT_HOOK_TYPE_PAUSED)) {
            return;
        }
        // Atomic with acquire order reason: data race with vmdeath_did_not_happen_
        ASSERT(vmdeathDidNotHappen_.load(std::memory_order_acquire));
        loadedHooks->Paused(reason);
    }

    // CC-OFFNXT(G.FUN.01-CPP) Decreasing the number of arguments will decrease the clarity of the code.
    void Exception(PtThread thread, Method *method, const PtLocation &location, ObjectHeader *exceptionObject,
                   Method *catchMethod, const PtLocation &catchLocation) override
    {
        // Atomic with acquire order reason: data race with hooks_
        auto *loadedHooks = hooks_.load(std::memory_order_acquire);
        if (loadedHooks == nullptr || !HookIsEnabled(PtHookType::PT_HOOK_TYPE_EXCEPTION)) {
            return;
        }
        // Atomic with acquire order reason: data race with vmdeath_did_not_happen_
        ASSERT(vmdeathDidNotHappen_.load(std::memory_order_acquire));
        loadedHooks->Exception(thread, method, location, exceptionObject, catchMethod, catchLocation);
    }

    void ExceptionCatch(PtThread thread, Method *method, const PtLocation &location,
                        ObjectHeader *exceptionObject) override
    {
        // Atomic with acquire order reason: data race with hooks_
        auto *loadedHooks = hooks_.load(std::memory_order_acquire);
        if (loadedHooks == nullptr || !HookIsEnabled(PtHookType::PT_HOOK_TYPE_EXCEPTION_CATCH)) {
            return;
        }
        // Atomic with acquire order reason: data race with vmdeath_did_not_happen_
        ASSERT(vmdeathDidNotHappen_.load(std::memory_order_acquire));
        loadedHooks->ExceptionCatch(thread, method, location, exceptionObject);
    }

    void PropertyAccess(PtThread thread, Method *method, const PtLocation &location, ObjectHeader *object,
                        PtProperty property) override
    {
        // Atomic with acquire order reason: data race with hooks_
        auto *loadedHooks = hooks_.load(std::memory_order_acquire);
        if (loadedHooks == nullptr || !HookIsEnabled(PtHookType::PT_HOOK_TYPE_PROPERTY_ACCESS)) {
            return;
        }
        // Atomic with acquire order reason: data race with vmdeath_did_not_happen_
        ASSERT(vmdeathDidNotHappen_.load(std::memory_order_acquire));
        loadedHooks->PropertyAccess(thread, method, location, object, property);
    }

    // CC-OFFNXT(G.FUN.01-CPP) Decreasing the number of arguments will decrease the clarity of the code.
    void PropertyModification(PtThread thread, Method *method, const PtLocation &location, ObjectHeader *object,
                              PtProperty property, VRegValue newValue) override
    {
        // Atomic with acquire order reason: data race with hooks_
        auto *loadedHooks = hooks_.load(std::memory_order_acquire);
        if (loadedHooks == nullptr || !HookIsEnabled(PtHookType::PT_HOOK_TYPE_PROPERTY_MODIFICATION)) {
            return;
        }
        // Atomic with acquire order reason: data race with vmdeath_did_not_happen_
        ASSERT(vmdeathDidNotHappen_.load(std::memory_order_acquire));
        loadedHooks->PropertyModification(thread, method, location, object, property, newValue);
    }

    void ConsoleCall(PtThread thread, ConsoleCallType type, uint64_t timestamp,
                     const PandaVector<TypedValue> &arguments) override
    {
        // Atomic with acquire order reason: data race with hooks_
        auto *loadedHooks = hooks_.load(std::memory_order_acquire);
        if (loadedHooks == nullptr || !HookIsEnabled(PtHookType::PT_HOOK_TYPE_CONSOLE_CALL)) {
            return;
        }
        // Atomic with acquire order reason: data race with vmdeath_did_not_happen_
        ASSERT(vmdeathDidNotHappen_.load(std::memory_order_acquire));
        loadedHooks->ConsoleCall(thread, type, timestamp, arguments);
    }

    void FramePop(PtThread thread, Method *method, bool wasPoppedByException) override
    {
        // Atomic with acquire order reason: data race with hooks_
        auto *loadedHooks = hooks_.load(std::memory_order_acquire);
        if (loadedHooks == nullptr || !HookIsEnabled(PtHookType::PT_HOOK_TYPE_FRAME_POP)) {
            return;
        }
        // Atomic with acquire order reason: data race with vmdeath_did_not_happen_
        ASSERT(vmdeathDidNotHappen_.load(std::memory_order_acquire));
        loadedHooks->FramePop(thread, method, wasPoppedByException);
    }

    void GarbageCollectionFinish() override
    {
        // NOTE(dtrubenkov): Add an assertion when 2125 issue is resolved
        // ASSERT(ManagedThread::GetCurrent() == nullptr)
        // Atomic with acquire order reason: data race with hooks_
        auto *loadedHooks = hooks_.load(std::memory_order_acquire);
        if (loadedHooks == nullptr || !GlobalHookIsEnabled(PtHookType::PT_HOOK_TYPE_GARBAGE_COLLECTION_FINISH)) {
            return;
        }
        // Atomic with acquire order reason: data race with vmdeath_did_not_happen_
        ASSERT(vmdeathDidNotHappen_.load(std::memory_order_acquire));
        // Called in an unmanaged thread
        loadedHooks->GarbageCollectionFinish();
    }

    void GarbageCollectionStart() override
    {
        // NOTE(dtrubenkov): Add an assertion when 2125 issue is resolved
        // ASSERT(ManagedThread::GetCurrent() == nullptr)
        // Atomic with acquire order reason: data race with hooks_
        auto *loadedHooks = hooks_.load(std::memory_order_acquire);
        if (loadedHooks == nullptr || !GlobalHookIsEnabled(PtHookType::PT_HOOK_TYPE_GARBAGE_COLLECTION_START)) {
            return;
        }
        // Atomic with acquire order reason: data race with vmdeath_did_not_happen_
        ASSERT(vmdeathDidNotHappen_.load(std::memory_order_acquire));
        // Called in an unmanaged thread
        loadedHooks->GarbageCollectionStart();
    }

    void MethodEntry(PtThread thread, Method *method) override
    {
        // Atomic with acquire order reason: data race with hooks_
        auto *loadedHooks = hooks_.load(std::memory_order_acquire);
        if (loadedHooks == nullptr || !HookIsEnabled(PtHookType::PT_HOOK_TYPE_METHOD_ENTRY)) {
            return;
        }
        // Atomic with acquire order reason: data race with vmdeath_did_not_happen_
        ASSERT(vmdeathDidNotHappen_.load(std::memory_order_acquire));
        loadedHooks->MethodEntry(thread, method);
    }

    void MethodExit(PtThread thread, Method *method, bool wasPoppedByException, VRegValue returnValue) override
    {
        // Atomic with acquire order reason: data race with hooks_
        auto *loadedHooks = hooks_.load(std::memory_order_acquire);
        if (loadedHooks == nullptr || !HookIsEnabled(PtHookType::PT_HOOK_TYPE_METHOD_EXIT)) {
            return;
        }
        // Atomic with acquire order reason: data race with vmdeath_did_not_happen_
        ASSERT(vmdeathDidNotHappen_.load(std::memory_order_acquire));
        loadedHooks->MethodExit(thread, method, wasPoppedByException, returnValue);
    }

    void SingleStep(PtThread thread, Method *method, const PtLocation &location) override
    {
        // Atomic with acquire order reason: data race with hooks_
        auto *loadedHooks = hooks_.load(std::memory_order_acquire);
        if (loadedHooks == nullptr || !HookIsEnabled(PtHookType::PT_HOOK_TYPE_SINGLE_STEP)) {
            return;
        }
        // Atomic with acquire order reason: data race with vmdeath_did_not_happen_
        ASSERT(vmdeathDidNotHappen_.load(std::memory_order_acquire));
        loadedHooks->SingleStep(thread, method, location);
    }

    void ThreadStart(PtThread thread) override
    {
        // Atomic with acquire order reason: data race with hooks_
        auto *loadedHooks = hooks_.load(std::memory_order_acquire);
        if (loadedHooks == nullptr || !HookIsEnabled(PtHookType::PT_HOOK_TYPE_THREAD_START)) {
            return;
        }
        // Atomic with acquire order reason: data race with vmdeath_did_not_happen_
        ASSERT(vmdeathDidNotHappen_.load(std::memory_order_acquire));
        loadedHooks->ThreadStart(thread);
    }

    void ThreadEnd(PtThread thread) override
    {
        // Atomic with acquire order reason: data race with hooks_
        auto *loadedHooks = hooks_.load(std::memory_order_acquire);
        if (loadedHooks == nullptr || !HookIsEnabled(PtHookType::PT_HOOK_TYPE_THREAD_END)) {
            return;
        }
        // Atomic with acquire order reason: data race with vmdeath_did_not_happen_
        ASSERT(vmdeathDidNotHappen_.load(std::memory_order_acquire));
        loadedHooks->ThreadEnd(thread);
    }

    void VmStart() override
    {
        // Atomic with acquire order reason: data race with hooks_
        auto *loadedHooks = hooks_.load(std::memory_order_acquire);
        if (loadedHooks == nullptr || !HookIsEnabled(PtHookType::PT_HOOK_TYPE_VM_START)) {
            return;
        }
        // Atomic with acquire order reason: data race with vmdeath_did_not_happen_
        ASSERT(vmdeathDidNotHappen_.load(std::memory_order_acquire));
        loadedHooks->VmStart();
    }

    void VmInitialization(PtThread thread) override
    {
        // Atomic with acquire order reason: data race with hooks_
        auto *loadedHooks = hooks_.load(std::memory_order_acquire);
        if (loadedHooks == nullptr || !HookIsEnabled(PtHookType::PT_HOOK_TYPE_VM_INITIALIZATION)) {
            return;
        }
        // Atomic with acquire order reason: data race with vmdeath_did_not_happen_
        ASSERT(vmdeathDidNotHappen_.load(std::memory_order_acquire));
        loadedHooks->VmInitialization(thread);
    }

    void VmDeath() override
    {
        // Atomic with acquire order reason: data race with hooks_
        auto *loadedHooks = hooks_.load(std::memory_order_acquire);
        if (loadedHooks == nullptr || !HookIsEnabled(PtHookType::PT_HOOK_TYPE_VM_DEATH)) {
            return;
        }
        // Atomic with acquire order reason: data race with vmdeath_did_not_happen_
        ASSERT(vmdeathDidNotHappen_.load(std::memory_order_acquire));
#ifndef NDEBUG
        // Atomic with release order reason: data race with vmdeath_did_not_happen_
        vmdeathDidNotHappen_.store(false, std::memory_order_release);
#endif
        loadedHooks->VmDeath();
        SetHooks(nullptr);
    }

    void ExceptionRevoked(ExceptionWrapper reason, ExceptionID exceptionId) override
    {
        // Atomic with acquire order reason: data race with hooks_
        auto *loadedHooks = hooks_.load(std::memory_order_acquire);
        if (loadedHooks == nullptr || !HookIsEnabled(PtHookType::PT_HOOK_TYPE_EXCEPTION_REVOKED)) {
            return;
        }
        // Atomic with acquire order reason: data race with vmdeath_did_not_happen_
        ASSERT(vmdeathDidNotHappen_.load(std::memory_order_acquire));
        loadedHooks->ExceptionRevoked(reason, exceptionId);
    }

    void ExecutionContextCreated(ExecutionContextWrapper context) override
    {
        // Atomic with acquire order reason: data race with hooks_
        auto *loadedHooks = hooks_.load(std::memory_order_acquire);
        if (loadedHooks == nullptr || !HookIsEnabled(PtHookType::PT_HOOK_TYPE_EXECUTION_CONTEXT_CREATEED)) {
            return;
        }
        // Atomic with acquire order reason: data race with vmdeath_did_not_happen_
        ASSERT(vmdeathDidNotHappen_.load(std::memory_order_acquire));
        loadedHooks->ExecutionContextCreated(context);
    }

    void ExecutionContextDestroyed(ExecutionContextWrapper context) override
    {
        // Atomic with acquire order reason: data race with hooks_
        auto *loadedHooks = hooks_.load(std::memory_order_acquire);
        if (loadedHooks == nullptr || !HookIsEnabled(PtHookType::PT_HOOK_TYPE_EXECUTION_CONTEXT_DESTROYED)) {
            return;
        }
        // Atomic with acquire order reason: data race with vmdeath_did_not_happen_
        ASSERT(vmdeathDidNotHappen_.load(std::memory_order_acquire));
        loadedHooks->ExecutionContextDestroyed(context);
    }

    void ExecutionContextsCleared() override
    {
        // Atomic with acquire order reason: data race with hooks_
        auto *loadedHooks = hooks_.load(std::memory_order_acquire);
        if (loadedHooks == nullptr || !HookIsEnabled(PtHookType::PT_HOOK_TYPE_EXECUTION_CONTEXTS_CLEARED)) {
            return;
        }
        // Atomic with acquire order reason: data race with vmdeath_did_not_happen_
        ASSERT(vmdeathDidNotHappen_.load(std::memory_order_acquire));
        loadedHooks->ExecutionContextsCleared();
    }

    void ClassLoad(PtThread thread, BaseClass *klass) override
    {
        // Atomic with acquire order reason: data race with hooks_
        auto *loadedHooks = hooks_.load(std::memory_order_acquire);
        if (loadedHooks == nullptr || !HookIsEnabled(PtHookType::PT_HOOK_TYPE_CLASS_LOAD)) {
            return;
        }
        // Atomic with acquire order reason: data race with vmdeath_did_not_happen_
        ASSERT(vmdeathDidNotHappen_.load(std::memory_order_acquire));
        loadedHooks->ClassLoad(thread, klass);
    }

    void ClassPrepare(PtThread thread, BaseClass *klass) override
    {
        // Atomic with acquire order reason: data race with hooks_
        auto *loadedHooks = hooks_.load(std::memory_order_acquire);
        if (loadedHooks == nullptr || !HookIsEnabled(PtHookType::PT_HOOK_TYPE_CLASS_PREPARE)) {
            return;
        }
        // Atomic with acquire order reason: data race with vmdeath_did_not_happen_
        ASSERT(vmdeathDidNotHappen_.load(std::memory_order_acquire));
        loadedHooks->ClassPrepare(thread, klass);
    }

    void MonitorWait(PtThread thread, ObjectHeader *object, int64_t timeout) override
    {
        // Atomic with acquire order reason: data race with hooks_
        auto *loadedHooks = hooks_.load(std::memory_order_acquire);
        if (loadedHooks == nullptr || !HookIsEnabled(PtHookType::PT_HOOK_TYPE_MONITOR_WAIT)) {
            return;
        }
        // Atomic with acquire order reason: data race with vmdeath_did_not_happen_
        ASSERT(vmdeathDidNotHappen_.load(std::memory_order_acquire));
        loadedHooks->MonitorWait(thread, object, timeout);
    }

    void MonitorWaited(PtThread thread, ObjectHeader *object, bool timedOut) override
    {
        // Atomic with acquire order reason: data race with hooks_
        auto *loadedHooks = hooks_.load(std::memory_order_acquire);
        if (loadedHooks == nullptr || !HookIsEnabled(PtHookType::PT_HOOK_TYPE_MONITOR_WAITED)) {
            return;
        }
        // Atomic with acquire order reason: data race with vmdeath_did_not_happen_
        ASSERT(vmdeathDidNotHappen_.load(std::memory_order_acquire));
        loadedHooks->MonitorWaited(thread, object, timedOut);
    }

    void MonitorContendedEnter(PtThread thread, ObjectHeader *object) override
    {
        // Atomic with acquire order reason: data race with hooks_
        auto *loadedHooks = hooks_.load(std::memory_order_acquire);
        if (loadedHooks == nullptr || !HookIsEnabled(PtHookType::PT_HOOK_TYPE_MONITOR_CONTENDED_ENTER)) {
            return;
        }
        // Atomic with acquire order reason: data race with vmdeath_did_not_happen_
        ASSERT(vmdeathDidNotHappen_.load(std::memory_order_acquire));
        loadedHooks->MonitorContendedEnter(thread, object);
    }

    void MonitorContendedEntered(PtThread thread, ObjectHeader *object) override
    {
        // Atomic with acquire order reason: data race with hooks_
        auto *loadedHooks = hooks_.load(std::memory_order_acquire);
        if (loadedHooks == nullptr || !HookIsEnabled(PtHookType::PT_HOOK_TYPE_MONITOR_CONTENDED_ENTERED)) {
            return;
        }
        // Atomic with acquire order reason: data race with vmdeath_did_not_happen_
        ASSERT(vmdeathDidNotHappen_.load(std::memory_order_acquire));
        loadedHooks->MonitorContendedEntered(thread, object);
    }

    void ObjectAlloc(BaseClass *klass, ObjectHeader *object, PtThread thread, size_t size) override
    {
        // Atomic with acquire order reason: data race with hooks_
        auto *loadedHooks = hooks_.load(std::memory_order_acquire);
        if (loadedHooks == nullptr || !HookIsEnabled(PtHookType::PT_HOOK_TYPE_OBJECT_ALLOC)) {
            return;
        }
        // Atomic with acquire order reason: data race with vmdeath_did_not_happen_
        ASSERT(vmdeathDidNotHappen_.load(std::memory_order_acquire));
        loadedHooks->ObjectAlloc(klass, object, thread, size);
    }

private:
    bool GlobalHookIsEnabled(PtHookType type) const
    {
        return globalHookTypeInfo_.IsEnabled(type);
    }

    bool HookIsEnabled(PtHookType type) const
    {
        if (GlobalHookIsEnabled(type)) {
            return true;
        }

        ManagedThread *managedThread = ManagedThread::GetCurrent();
        ASSERT(managedThread != nullptr);

        // Check local value
        return managedThread->GetPtThreadInfo()->GetHookTypeInfo().IsEnabled(type);
    }

    std::atomic<PtHooks *> hooks_ {nullptr};

    PtHookTypeInfo globalHookTypeInfo_ {true};

#ifndef NDEBUG
    std::atomic_bool vmdeathDidNotHappen_ {true};
#endif
};
}  // namespace ark::tooling

#endif  // PANDA_RUNTIME_TOOLING_PT_HOOKS_WRAPPER_H
