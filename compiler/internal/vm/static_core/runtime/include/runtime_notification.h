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
#ifndef PANDA_RUNTIME_RUNTIME_NOTIFICATION_H_
#define PANDA_RUNTIME_RUNTIME_NOTIFICATION_H_

#include <cstdint>
#include <optional>
#include <string_view>

#include "libarkbase/os/mutex.h"
#include "runtime/include/console_call_type.h"
#include "runtime/include/coretypes/tagged_value.h"
#include "runtime/include/locks.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/mem/panda_string.h"
#include "runtime/include/runtime.h"
#include "runtime/handle_scope-inl.h"

namespace ark {

class Method;
class Class;
class Rendezvous;

class RuntimeListener {
public:
    RuntimeListener() = default;
    virtual ~RuntimeListener() = default;
    DEFAULT_COPY_SEMANTIC(RuntimeListener);
    DEFAULT_MOVE_SEMANTIC(RuntimeListener);

    virtual void LoadModule([[maybe_unused]] std::string_view name) {}

    virtual void ThreadStart([[maybe_unused]] ManagedThread *managedThread) {}
    virtual void ThreadEnd([[maybe_unused]] ManagedThread *managedThread) {}

    virtual void BytecodePcChanged([[maybe_unused]] ManagedThread *thread, [[maybe_unused]] Method *method,
                                   [[maybe_unused]] uint32_t bcOffset)
    {
    }

    virtual void GarbageCollectorStart() {}
    virtual void GarbageCollectorFinish() {}

    virtual void ExceptionThrow([[maybe_unused]] ManagedThread *thread, [[maybe_unused]] Method *method,
                                [[maybe_unused]] ObjectHeader *exceptionObject, [[maybe_unused]] uint32_t bcOffset)
    {
    }

    virtual void ExceptionCatch([[maybe_unused]] ManagedThread *thread, [[maybe_unused]] Method *method,
                                [[maybe_unused]] ObjectHeader *exceptionObject, [[maybe_unused]] uint32_t bcOffset)
    {
    }

    virtual void ConsoleCall([[maybe_unused]] ManagedThread *thread, [[maybe_unused]] ConsoleCallType type,
                             [[maybe_unused]] uint64_t timestamp,
                             [[maybe_unused]] const PandaVector<TypedValue> &arguments)
    {
    }

    virtual void VmStart() {}
    virtual void VmInitialization([[maybe_unused]] ManagedThread *managedThread) {}
    virtual void VmDeath() {}

    virtual void MethodEntry([[maybe_unused]] ManagedThread *thread, [[maybe_unused]] Method *method) {}
    virtual void MethodExit([[maybe_unused]] ManagedThread *thread, [[maybe_unused]] Method *method) {}

    virtual void ClassLoad([[maybe_unused]] Class *klass) {}
    virtual void ClassPrepare([[maybe_unused]] Class *klass) {}

    virtual void MonitorWait([[maybe_unused]] ObjectHeader *object, [[maybe_unused]] int64_t timeout) {}
    virtual void MonitorWaited([[maybe_unused]] ObjectHeader *object, [[maybe_unused]] bool timedOut) {}
    virtual void MonitorContendedEnter([[maybe_unused]] ObjectHeader *object) {}
    virtual void MonitorContendedEntered([[maybe_unused]] ObjectHeader *object) {}

    virtual void ObjectAlloc([[maybe_unused]] BaseClass *klass, [[maybe_unused]] ObjectHeader *object,
                             [[maybe_unused]] ManagedThread *thread, [[maybe_unused]] size_t size)
    {
    }

    // Deprecated events
    virtual void ThreadStart([[maybe_unused]] ManagedThread::ThreadId threadId) {}
    virtual void ThreadEnd([[maybe_unused]] ManagedThread::ThreadId threadId) {}
    virtual void VmInitialization([[maybe_unused]] ManagedThread::ThreadId threadId) {}
    virtual void ExceptionCatch([[maybe_unused]] const ManagedThread *thread, [[maybe_unused]] const Method *method,
                                [[maybe_unused]] uint32_t bcOffset)
    {
    }
};

class DebuggerListener {
public:
    DebuggerListener() = default;
    virtual ~DebuggerListener() = default;
    DEFAULT_COPY_SEMANTIC(DebuggerListener);
    DEFAULT_MOVE_SEMANTIC(DebuggerListener);

    virtual void StartDebugger() = 0;
    virtual void StopDebugger() = 0;
    virtual bool IsDebuggerConfigured() = 0;
};

class RuntimeNotificationManager {
public:
    enum Event : uint32_t {
        BYTECODE_PC_CHANGED = 0x01,
        LOAD_MODULE = 0x02,
        THREAD_EVENTS = 0x04,
        GARBAGE_COLLECTOR_EVENTS = 0x08,
        EXCEPTION_EVENTS = 0x10,
        VM_EVENTS = 0x20,
        METHOD_EVENTS = 0x40,
        CLASS_EVENTS = 0x80,
        MONITOR_EVENTS = 0x100,
        ALLOCATION_EVENTS = 0x200,
        CONSOLE_EVENTS = 0x400,
        ALL = 0xFFFFFFFF
    };

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    explicit RuntimeNotificationManager(mem::AllocatorPtr<mem::AllocatorPurpose::ALLOCATOR_PURPOSE_INTERNAL> allocator)
        : bytecodePcListeners_(allocator->Adapter()),
          loadModuleListeners_(allocator->Adapter()),
          threadEventsListeners_(allocator->Adapter()),
          garbageCollectorListeners_(allocator->Adapter()),
          exceptionListeners_(allocator->Adapter()),
          vmEventsListeners_(allocator->Adapter()),
          methodListeners_(allocator->Adapter()),
          classListeners_(allocator->Adapter()),
          monitorListeners_(allocator->Adapter())
    {
    }

    void AddListener(RuntimeListener *listener, uint32_t eventMask)
    {
        ScopedSuspendAllThreads ssat(rendezvous_);
        AddListenerIfMatches(listener, eventMask, &bytecodePcListeners_, Event::BYTECODE_PC_CHANGED,
                             &hasBytecodePcListeners_);

        AddListenerIfMatches(listener, eventMask, &loadModuleListeners_, Event::LOAD_MODULE, &hasLoadModuleListeners_);

        AddListenerIfMatches(listener, eventMask, &threadEventsListeners_, Event::THREAD_EVENTS,
                             &hasThreadEventsListeners_);

        AddListenerIfMatches(listener, eventMask, &exceptionListeners_, Event::EXCEPTION_EVENTS,
                             &hasExceptionListeners_);

        AddListenerIfMatches(listener, eventMask, &vmEventsListeners_, Event::VM_EVENTS, &hasVmEventsListeners_);

        AddListenerIfMatches(listener, eventMask, &methodListeners_, Event::METHOD_EVENTS, &hasMethodListeners_);

        AddListenerIfMatches(listener, eventMask, &classListeners_, Event::CLASS_EVENTS, &hasClassListeners_);

        AddListenerIfMatches(listener, eventMask, &monitorListeners_, Event::MONITOR_EVENTS, &hasMonitorListeners_);

        AddListenerIfMatches(listener, eventMask, &allocationListeners_, Event::ALLOCATION_EVENTS,
                             &hasAllocationListeners_);

        AddListenerIfMatches(listener, eventMask, &consoleListeners_, Event::CONSOLE_EVENTS, &hasConsoleListeners_);

        {
            // We cannot stop GC thread, so holding lock to avoid data race
            os::memory::WriteLockHolder rwlock(gcEventLock_);
            AddListenerIfMatches(listener, eventMask, &garbageCollectorListeners_, Event::GARBAGE_COLLECTOR_EVENTS,
                                 &hasGarbageCollectorListeners_);
        }
    }

    void RemoveListener(RuntimeListener *listener, uint32_t eventMask)
    {
        ScopedSuspendAllThreads ssat(rendezvous_);
        RemoveListenerIfMatches(listener, eventMask, &bytecodePcListeners_, Event::BYTECODE_PC_CHANGED,
                                &hasBytecodePcListeners_);

        RemoveListenerIfMatches(listener, eventMask, &loadModuleListeners_, Event::LOAD_MODULE,
                                &hasLoadModuleListeners_);

        RemoveListenerIfMatches(listener, eventMask, &threadEventsListeners_, Event::THREAD_EVENTS,
                                &hasThreadEventsListeners_);

        RemoveListenerIfMatches(listener, eventMask, &exceptionListeners_, Event::EXCEPTION_EVENTS,
                                &hasExceptionListeners_);

        RemoveListenerIfMatches(listener, eventMask, &vmEventsListeners_, Event::VM_EVENTS, &hasVmEventsListeners_);

        RemoveListenerIfMatches(listener, eventMask, &methodListeners_, Event::METHOD_EVENTS, &hasMethodListeners_);

        RemoveListenerIfMatches(listener, eventMask, &classListeners_, Event::CLASS_EVENTS, &hasClassListeners_);

        RemoveListenerIfMatches(listener, eventMask, &monitorListeners_, Event::MONITOR_EVENTS, &hasMonitorListeners_);

        RemoveListenerIfMatches(listener, eventMask, &allocationListeners_, Event::ALLOCATION_EVENTS,
                                &hasAllocationListeners_);

        RemoveListenerIfMatches(listener, eventMask, &consoleListeners_, Event::CONSOLE_EVENTS, &hasConsoleListeners_);

        {
            // We cannot stop GC thread, so holding lock to avoid data race
            os::memory::WriteLockHolder rwlock(gcEventLock_);
            RemoveListenerIfMatches(listener, eventMask, &garbageCollectorListeners_, Event::GARBAGE_COLLECTOR_EVENTS,
                                    &hasGarbageCollectorListeners_);
        }
    }

    void LoadModuleEvent(std::string_view name)
    {
        if (UNLIKELY(hasLoadModuleListeners_)) {
            for (auto *listener : loadModuleListeners_) {
                if (listener != nullptr) {
                    listener->LoadModule(name);
                }
            }
        }
    }

    void ThreadStartEvent(ManagedThread *managedThread)
    {
        if (UNLIKELY(hasThreadEventsListeners_)) {
            for (auto *listener : threadEventsListeners_) {
                if (listener != nullptr) {
                    listener->ThreadStart(managedThread);
                }
            }
        }
    }

    void ThreadEndEvent(ManagedThread *managedThread)
    {
        if (UNLIKELY(hasThreadEventsListeners_)) {
            for (auto *listener : threadEventsListeners_) {
                if (listener != nullptr) {
                    listener->ThreadEnd(managedThread);
                }
            }
        }
    }

    void BytecodePcChangedEvent(ManagedThread *thread, Method *method, uint32_t bcOffset)
    {
        if (UNLIKELY(hasBytecodePcListeners_)) {
            for (auto *listener : bytecodePcListeners_) {
                if (listener != nullptr) {
                    listener->BytecodePcChanged(thread, method, bcOffset);
                }
            }
        }
    }

    void GarbageCollectorStartEvent()
    {
        if (UNLIKELY(hasGarbageCollectorListeners_)) {
            os::memory::ReadLockHolder rwlock(gcEventLock_);
            for (auto *listener : garbageCollectorListeners_) {
                if (listener != nullptr) {
                    listener->GarbageCollectorStart();
                }
            }
        }
    }

    void GarbageCollectorFinishEvent()
    {
        if (UNLIKELY(hasGarbageCollectorListeners_)) {
            os::memory::ReadLockHolder rwlock(gcEventLock_);
            for (auto *listener : garbageCollectorListeners_) {
                if (listener != nullptr) {
                    listener->GarbageCollectorFinish();
                }
            }
        }
    }

    void ExceptionThrowEvent(ManagedThread *thread, Method *method, ObjectHeader *exceptionObject, uint32_t bcOffset)
    {
        if (UNLIKELY(hasExceptionListeners_)) {
            [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
            VMHandle<ObjectHeader> objHandle(thread, exceptionObject);
            for (auto *listener : exceptionListeners_) {
                if (listener != nullptr) {
                    listener->ExceptionThrow(thread, method, objHandle.GetPtr(), bcOffset);
                }
            }
        }
    }

    void ExceptionCatchEvent(ManagedThread *thread, Method *method, ObjectHeader *exceptionObject, uint32_t bcOffset)
    {
        if (UNLIKELY(hasExceptionListeners_)) {
            [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
            VMHandle<ObjectHeader> objHandle(thread, exceptionObject);
            for (auto *listener : exceptionListeners_) {
                if (listener != nullptr) {
                    listener->ExceptionCatch(thread, method, objHandle.GetPtr(), bcOffset);
                }
            }
        }
    }

    void VmStartEvent()
    {
        if (UNLIKELY(hasVmEventsListeners_)) {
            for (auto *listener : vmEventsListeners_) {
                if (listener != nullptr) {
                    listener->VmStart();
                }
            }
        }
    }

    void VmInitializationEvent(ManagedThread *managedThread)
    {
        if (UNLIKELY(hasVmEventsListeners_)) {
            for (auto *listener : vmEventsListeners_) {
                if (listener != nullptr) {
                    listener->VmInitialization(managedThread);
                }
            }
        }
    }

    // Deprecated API
    void VmInitializationEvent(ManagedThread::ThreadId threadId)
    {
        if (UNLIKELY(hasVmEventsListeners_)) {
            for (auto *listener : vmEventsListeners_) {
                if (listener != nullptr) {
                    listener->VmInitialization(threadId);
                }
            }
        }
    }

    void VmDeathEvent()
    {
        if (UNLIKELY(hasVmEventsListeners_)) {
            for (auto *listener : vmEventsListeners_) {
                if (listener != nullptr) {
                    listener->VmDeath();
                }
            }
        }
    }

    void MethodEntryEvent(ManagedThread *thread, Method *method)
    {
        if (UNLIKELY(hasMethodListeners_)) {
            for (auto *listener : methodListeners_) {
                if (listener != nullptr) {
                    listener->MethodEntry(thread, method);
                }
            }
        }
    }

    void MethodExitEvent(ManagedThread *thread, Method *method)
    {
        if (UNLIKELY(hasMethodListeners_)) {
            for (auto *listener : methodListeners_) {
                if (listener != nullptr) {
                    listener->MethodExit(thread, method);
                }
            }
        }
    }

    void ClassLoadEvent(Class *klass)
    {
        if (UNLIKELY(hasClassListeners_)) {
            for (auto *listener : classListeners_) {
                if (listener != nullptr) {
                    listener->ClassLoad(klass);
                }
            }
        }
    }

    void ClassPrepareEvent(Class *klass)
    {
        if (UNLIKELY(hasClassListeners_)) {
            for (auto *listener : classListeners_) {
                if (listener != nullptr) {
                    listener->ClassPrepare(klass);
                }
            }
        }
    }

    void MonitorWaitEvent(ObjectHeader *object, int64_t timeout)
    {
        if (UNLIKELY(hasMonitorListeners_)) {
            // If we need to support multiple monitor listeners,
            // the object must be wrapped to ObjectHandle to protect from GC move
            ASSERT(monitorListeners_.size() == 1);
            auto *listener = monitorListeners_.front();
            if (listener != nullptr) {
                listener->MonitorWait(object, timeout);
            }
        }
    }

    void MonitorWaitedEvent(ObjectHeader *object, bool timedOut)
    {
        if (UNLIKELY(hasMonitorListeners_)) {
            // If we need to support multiple monitor listeners,
            // the object must be wrapped to ObjectHandle to protect from GC move
            ASSERT(monitorListeners_.size() == 1);
            auto *listener = monitorListeners_.front();
            if (listener != nullptr) {
                monitorListeners_.front()->MonitorWaited(object, timedOut);
            }
        }
    }

    void MonitorContendedEnterEvent(ObjectHeader *object)
    {
        if (UNLIKELY(hasMonitorListeners_)) {
            // If we need to support multiple monitor listeners,
            // the object must be wrapped to ObjectHandle to protect from GC move
            ASSERT(monitorListeners_.size() == 1);
            auto *listener = monitorListeners_.front();
            if (listener != nullptr) {
                monitorListeners_.front()->MonitorContendedEnter(object);
            }
        }
    }

    void MonitorContendedEnteredEvent(ObjectHeader *object)
    {
        if (UNLIKELY(hasMonitorListeners_)) {
            // If we need to support multiple monitor listeners,
            // the object must be wrapped to ObjectHandle to protect from GC move
            ASSERT(monitorListeners_.size() == 1);
            auto *listener = monitorListeners_.front();
            if (listener != nullptr) {
                monitorListeners_.front()->MonitorContendedEntered(object);
            }
        }
    }

    bool HasAllocationListeners() const
    {
        return hasAllocationListeners_;
    }

    void ObjectAllocEvent(BaseClass *klass, ObjectHeader *object, ManagedThread *thread, size_t size) const
    {
        if (UNLIKELY(hasAllocationListeners_)) {
            // If we need to support multiple allocation listeners,
            // the object must be wrapped to ObjectHandle to protect from GC move
            ASSERT(allocationListeners_.size() == 1);
            auto *listener = allocationListeners_.front();
            if (listener != nullptr) {
                allocationListeners_.front()->ObjectAlloc(klass, object, thread, size);
            }
        }
    }

    void ConsoleCallEvent(ManagedThread *thread, ConsoleCallType type, uint64_t timestamp,
                          [[maybe_unused]] const PandaVector<TypedValue> &arguments) const
    {
        if (UNLIKELY(hasConsoleListeners_)) {
            for (auto listener : consoleListeners_) {
                if (listener != nullptr) {
                    listener->ConsoleCall(thread, type, timestamp, arguments);
                }
            }
        }
    }

    void StartDebugger()
    {
        os::memory::ReadLockHolder holder(debuggerLock_);
        for (auto *listener : debuggerListeners_) {
            listener->StartDebugger();
        }
    }

    void StopDebugger()
    {
        os::memory::ReadLockHolder holder(debuggerLock_);
        for (auto *listener : debuggerListeners_) {
            listener->StopDebugger();
        }
    }

    bool IsDebuggerConfigured()
    {
        os::memory::ReadLockHolder holder(debuggerLock_);
        for (auto *listener : debuggerListeners_) {
            if (!listener->IsDebuggerConfigured()) {
                return false;
            }
        }
        return true;
    }

    void SetRendezvous(Rendezvous *rendezvous)
    {
        rendezvous_ = rendezvous;
    }

    void AddDebuggerListener(DebuggerListener *listener)
    {
        os::memory::WriteLockHolder holder(debuggerLock_);
        debuggerListeners_.push_back(listener);
    }

    void RemoveDebuggerListener(DebuggerListener *listener)
    {
        os::memory::WriteLockHolder holder(debuggerLock_);
        RemoveListener(debuggerListeners_, listener);
    }

private:
    // NOLINTBEGIN(misc-unused-parameters)
    template <typename FlagType>
    static void AddListenerIfMatches(RuntimeListener *listener, uint32_t eventMask,
                                     PandaList<RuntimeListener *> *listenerGroup, Event eventModifier,
                                     FlagType *eventFlag)
    {
        if ((eventMask & eventModifier) != 0) {
            // If a free group item presents, use it, otherwise push back a new item
            auto it = std::find(listenerGroup->begin(), listenerGroup->end(), nullptr);
            if (it != listenerGroup->end()) {
                *it = listener;
            } else {
                listenerGroup->push_back(listener);
            }
            *eventFlag = true;
        }
    }

    template <typename Container, typename T>
    ALWAYS_INLINE void RemoveListener(Container &c, T &listener)
    {
        c.erase(std::remove_if(c.begin(), c.end(), [&listener](const T &elem) { return listener == elem; }));
    }

    template <typename FlagType>
    static void RemoveListenerIfMatches(RuntimeListener *listener, uint32_t eventMask,
                                        PandaList<RuntimeListener *> *listenerGroup, Event eventModifier,
                                        FlagType *eventFlag)
    {
        if ((eventMask & eventModifier) != 0) {
            auto it = std::find(listenerGroup->begin(), listenerGroup->end(), listener);
            if (it == listenerGroup->end()) {
                return;
            }
            // Removing a listener is not safe, because the iteration can not be completed in another thread.
            // We just null the item in the group
            *it = nullptr;

            // Check if any listener presents and update the flag if not
            if (std::find_if(listenerGroup->begin(), listenerGroup->end(),
                             [](RuntimeListener *item) { return item != nullptr; }) == listenerGroup->end()) {
                *eventFlag = false;
            }
        }
    }
    // NOLINTEND(misc-unused-parameters)

    PandaList<RuntimeListener *> bytecodePcListeners_;
    PandaList<RuntimeListener *> loadModuleListeners_;
    PandaList<RuntimeListener *> threadEventsListeners_;
    PandaList<RuntimeListener *> garbageCollectorListeners_;
    PandaList<RuntimeListener *> exceptionListeners_;
    PandaList<RuntimeListener *> vmEventsListeners_;
    PandaList<RuntimeListener *> methodListeners_;
    PandaList<RuntimeListener *> classListeners_;
    PandaList<RuntimeListener *> monitorListeners_;
    PandaList<RuntimeListener *> allocationListeners_;
    PandaList<RuntimeListener *> consoleListeners_;

    bool hasBytecodePcListeners_ = false;
    bool hasLoadModuleListeners_ = false;
    bool hasThreadEventsListeners_ = false;
    std::atomic<bool> hasGarbageCollectorListeners_ = false;
    bool hasExceptionListeners_ = false;
    bool hasVmEventsListeners_ = false;
    bool hasMethodListeners_ = false;
    bool hasClassListeners_ = false;
    bool hasMonitorListeners_ = false;
    bool hasAllocationListeners_ = false;
    bool hasConsoleListeners_ = false;
    Rendezvous *rendezvous_ {nullptr};

    os::memory::RWLock debuggerLock_;
    os::memory::RWLock gcEventLock_;
    PandaList<DebuggerListener *> debuggerListeners_;
};

}  // namespace ark

#endif  // PANDA_RUNTIME_RUNTIME_NOTIFICATION_H_
