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

#ifndef PANDA_RUNTIME_TOOLING_DEBUGGER_H
#define PANDA_RUNTIME_TOOLING_DEBUGGER_H

#include <atomic>
#include <functional>
#include <memory>
#include <string_view>

#include "include/mem/panda_containers.h"
#include "include/mem/panda_smart_pointers.h"
#include "include/method.h"
#include "include/panda_vm.h"
#include "include/runtime.h"
#include "include/runtime_notification.h"
#include "include/tooling/debug_interface.h"
#include "libarkbase/os/mutex.h"
#include "libarkbase/utils/span.h"
#include "pt_hooks_wrapper.h"
#include "runtime/thread_manager.h"

namespace ark::tooling {
// Deprecated API
// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
class Breakpoint {
public:
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    Breakpoint(Method *method, uint32_t bcOffset) : method_(method), bcOffset_(bcOffset) {}
    ~Breakpoint() = default;

    Method *GetMethod() const
    {
        return method_;
    }

    uint32_t GetBytecodeOffset() const
    {
        return bcOffset_;
    }

    bool operator==(const Breakpoint &bpoint) const
    {
        return GetMethod() == bpoint.GetMethod() && GetBytecodeOffset() == bpoint.GetBytecodeOffset();
    }

    DEFAULT_COPY_SEMANTIC(Breakpoint);
    DEFAULT_MOVE_SEMANTIC(Breakpoint);

private:
    Method *method_;
    uint32_t bcOffset_;
};

// Deprecated API
class HashBreakpoint {
public:
    size_t operator()(const Breakpoint &bpoint) const
    {
        return (std::hash<Method *>()(bpoint.GetMethod())) ^ (std::hash<uint32_t>()(bpoint.GetBytecodeOffset()));
    }
};

class HashLocation {
public:
    size_t operator()(const PtLocation &location) const
    {
        return std::hash<std::string>()(location.GetPandaFile()) ^
               std::hash<uint32_t>()(location.GetMethodId().GetOffset()) ^  // CODECHECK-NOLINT(C_RULE_ID_INDENT_CHECK)
               std::hash<uint32_t>()(location.GetBytecodeOffset());         // CODECHECK-NOLINT(C_RULE_ID_INDENT_CHECK)
    }
};

class PropertyWatch {
public:
    enum class Type { ACCESS, MODIFY };

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    PropertyWatch(panda_file::File::EntityId classId, panda_file::File::EntityId fieldId, Type type)
        : classId_(classId), fieldId_(fieldId), type_(type)
    {
    }

    ~PropertyWatch() = default;

    panda_file::File::EntityId GetClassId() const
    {
        return classId_;
    }

    panda_file::File::EntityId GetFieldId() const
    {
        return fieldId_;
    }

    Type GetType() const
    {
        return type_;
    }

private:
    NO_COPY_SEMANTIC(PropertyWatch);
    NO_MOVE_SEMANTIC(PropertyWatch);

    panda_file::File::EntityId classId_;
    panda_file::File::EntityId fieldId_;
    Type type_;
};

// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class Debugger : public DebugInterface, RuntimeListener {
public:
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    explicit Debugger(const Runtime *runtime)
        : runtime_(runtime),
          breakpoints_(GetInternalAllocatorAdapter(runtime)),
          propertyWatches_(GetInternalAllocatorAdapter(runtime)),
          vmStarted_(runtime->IsInitialized())
    {
        runtime_->GetNotificationManager()->AddListener(this, DEBUG_EVENT_MASK);
    }

    ~Debugger() override
    {
        runtime_->GetNotificationManager()->RemoveListener(this, DEBUG_EVENT_MASK);
    }

    std::optional<Error> RegisterHooks(PtHooks *hooks) override
    {
        hooks_.SetHooks(hooks);
        return {};
    }

    std::optional<Error> UnregisterHooks() override
    {
        hooks_.SetHooks(nullptr);
        return {};
    }

    std::optional<Error> EnableAllGlobalHook() override
    {
        hooks_.EnableAllGlobalHook();
        return {};
    }

    std::optional<Error> DisableAllGlobalHook() override
    {
        hooks_.DisableAllGlobalHook();
        return {};
    }

    std::optional<Error> SetNotification(PtThread thread, bool enable, PtHookType hookType) override;
    std::optional<Error> SetBreakpoint(const PtLocation &location) override;

    std::optional<Error> RemoveBreakpoint(const PtLocation &location) override;

    Expected<std::unique_ptr<PtFrame>, Error> GetCurrentFrame(PtThread thread) const override;

    std::optional<Error> EnumerateFrames(PtThread thread, std::function<bool(const PtFrame &)> callback) const override;

    // RuntimeListener methods

    void LoadModule(std::string_view filename) override
    {
        hooks_.LoadModule(filename);
    }

    void ThreadStart(ManagedThread *managedThread) override
    {
        hooks_.ThreadStart(PtThread(managedThread));
    }

    void ThreadEnd(ManagedThread *managedThread) override
    {
        hooks_.ThreadEnd(PtThread(managedThread));
    }

    void BytecodePcChanged(ManagedThread *thread, Method *method, uint32_t bcOffset) override;

    void VmStart() override
    {
        vmStarted_ = true;
        hooks_.VmStart();
    }

    void VmInitialization(ManagedThread *managedThread) override
    {
        hooks_.VmInitialization(PtThread(managedThread));
    }

    void VmDeath() override
    {
        hooks_.VmDeath();
    }

    void GarbageCollectorStart() override
    {
        hooks_.GarbageCollectionStart();
    }

    void GarbageCollectorFinish() override
    {
        ark::MTManagedThread *self = ark::MTManagedThread::GetCurrent();
        if (self == nullptr) {
            return;
        }
        hooks_.GarbageCollectionFinish();
    }

    void ObjectAlloc(BaseClass *klass, ObjectHeader *object, ManagedThread *thread, size_t size) override;

    void ExceptionThrow(ManagedThread *thread, Method *method, ObjectHeader *exceptionObject,
                        uint32_t bcOffset) override;
    void ExceptionCatch(ManagedThread *thread, Method *method, ObjectHeader *exceptionObject,
                        uint32_t bcOffset) override;

    void ConsoleCall(ManagedThread *thread, ConsoleCallType type, uint64_t timestamp,
                     const PandaVector<TypedValue> &arguments) override
    {
        hooks_.ConsoleCall(PtThread(thread), type, timestamp, arguments);
    }

    void MethodEntry(ManagedThread *managedThread, Method *method) override;
    void MethodExit(ManagedThread *managedThread, Method *method) override;

    void ClassLoad(Class *klass) override;
    void ClassPrepare(Class *klass) override;

    void MonitorWait(ObjectHeader *object, int64_t timeout) override;
    void MonitorWaited(ObjectHeader *object, bool timedOut) override;
    void MonitorContendedEnter(ObjectHeader *object) override;
    void MonitorContendedEntered(ObjectHeader *object) override;

    /*
     * Mock API for debug interphase starts:
     *
     * API's function should be revorked and input parameters should be added
     */
    std::optional<Error> GetThreadList(PandaVector<PtThread> *threadList) const override
    {
        runtime_->GetPandaVM()->GetThreadManager()->EnumerateThreads(
            [threadList](ManagedThread *managedThread) {
                ASSERT(managedThread && "thread is null");
                threadList->push_back(PtThread(managedThread));
                return true;
            },
            static_cast<unsigned int>(ark::EnumerationFlag::ALL),
            static_cast<unsigned int>(ark::EnumerationFlag::VM_THREAD));

        return {};
    }

    std::optional<Error> GetThreadInfo([[maybe_unused]] PtThread thread,
                                       [[maybe_unused]] ThreadInfo *infoPtr) const override
    {
        PT_UNIMPLEMENTED();
        return {};
    }

    std::optional<Error> SuspendThread(PtThread thread) const override;

    std::optional<Error> ResumeThread(PtThread thread) const override;

    std::optional<Error> SetVariable(PtThread thread, uint32_t frameDepth, int32_t regNumber,
                                     const VRegValue &value) const override;

    std::optional<Error> GetVariable(PtThread thread, uint32_t frameDepth, int32_t regNumber,
                                     VRegValue *result) const override;

    /**
     * @brief Loads provided bytecode and executes evaluation method.
     * File must contain public class with equally named static method taking no arguments.
     * Name of both class and method must equal source code file name, path to which must be included into binary.
     * @param thread in which to load and execute method.
     * @param frameNumber call stack depth in which context to evaluate.
     * @param expr bytecode with expression.
     * @param method pointer to save the loaded method in.
     * @param result pointer to save result in.
     */
    std::optional<Error> EvaluateExpression(PtThread thread, uint32_t frameNumber, const ExpressionWrapper &expr,
                                            Method **method, VRegValue *result) const override;

    /**
     * @brief Executes the provided evaluation method.
     * @param thread in which to execute method.
     * @param frameNumber call stack depth in which context to evaluate.
     * @param method pointer to the method.
     * @param result pointer to save result in.
     */
    std::optional<Error> EvaluateExpression(PtThread thread, uint32_t frameNumber, Method *method,
                                            VRegValue *result) const override;

    std::optional<Error> RestartFrame([[maybe_unused]] PtThread thread,
                                      [[maybe_unused]] uint32_t frameNumber) const override;

    std::optional<Error> SetAsyncCallStackDepth([[maybe_unused]] uint32_t maxDepth) const override
    {
        PT_UNIMPLEMENTED();
        return {};
    }

    std::optional<Error> GetProperties([[maybe_unused]] uint32_t *countPtr,
                                       [[maybe_unused]] char ***propertyPtr) const override
    {
        PT_UNIMPLEMENTED();
        return {};
    }

    std::optional<Error> NotifyFramePop(PtThread thread, uint32_t depth) const override;

    std::optional<Error> GetThisVariableByFrame(PtThread thread, uint32_t frameDepth, ObjectHeader **thisPtr) override;

    std::optional<Error> SetPropertyAccessWatch(BaseClass *klass, PtProperty property) override;

    std::optional<Error> ClearPropertyAccessWatch(BaseClass *klass, PtProperty property) override;

    std::optional<Error> SetPropertyModificationWatch(BaseClass *klass, PtProperty property) override;

    std::optional<Error> ClearPropertyModificationWatch(BaseClass *klass, PtProperty property) override;

private:
    Expected<interpreter::StaticVRegisterRef, Error> GetVRegByPandaFrame(ark::Frame *frame, int32_t regNumber) const;
    Expected<interpreter::DynamicVRegisterRef, Error> GetVRegByPandaFrameDyn(ark::Frame *frame,
                                                                             int32_t regNumber) const;
    std::optional<Error> CheckLocation(const PtLocation &location);
    bool IsBreakpoint(const PtLocation &location) const REQUIRES_SHARED(rwlock_);
    bool EraseBreakpoint(const PtLocation &location);

    bool IsPropertyWatchActive() const
    {
        os::memory::ReadLockHolder rholder(rwlock_);
        return !propertyWatches_.empty();
    }
    const tooling::PropertyWatch *FindPropertyWatch(panda_file::File::EntityId classId,
                                                    panda_file::File::EntityId fieldId,
                                                    tooling::PropertyWatch::Type type) const REQUIRES_SHARED(rwlock_);
    bool RemovePropertyWatch(panda_file::File::EntityId classId, panda_file::File::EntityId fieldId,
                             tooling::PropertyWatch::Type type);

    bool HandleBreakpoint(ManagedThread *managedThread, Method *method, const PtLocation &location);
    void HandleNotifyFramePop(ManagedThread *managedThread, Method *method, bool wasPoppedByException);
    bool HandleStep(ManagedThread *managedThread, Method *method, const PtLocation &location);

    bool HandlePropertyAccess(ManagedThread *thread, Method *method, const PtLocation &location);
    bool HandlePropertyModify(ManagedThread *thread, Method *method, const PtLocation &location);

    static constexpr uint32_t DEBUG_EVENT_MASK =
        RuntimeNotificationManager::Event::LOAD_MODULE | RuntimeNotificationManager::Event::THREAD_EVENTS |
        RuntimeNotificationManager::Event::BYTECODE_PC_CHANGED | RuntimeNotificationManager::Event::EXCEPTION_EVENTS |
        RuntimeNotificationManager::Event::VM_EVENTS | RuntimeNotificationManager::Event::GARBAGE_COLLECTOR_EVENTS |
        RuntimeNotificationManager::Event::METHOD_EVENTS | RuntimeNotificationManager::Event::CLASS_EVENTS |
        RuntimeNotificationManager::Event::MONITOR_EVENTS | RuntimeNotificationManager::Event::ALLOCATION_EVENTS |
        RuntimeNotificationManager::Event::CONSOLE_EVENTS;

    const Runtime *runtime_;
    PtHooksWrapper hooks_;

    mutable os::memory::RWLock rwlock_;
    PandaUnorderedSet<PtLocation, HashLocation> breakpoints_ GUARDED_BY(rwlock_);
    PandaList<PropertyWatch> propertyWatches_ GUARDED_BY(rwlock_);
    // NOTE(m.strizhak): research how to rework VM start to avoid atomic
    std::atomic_bool vmStarted_ {false};

    NO_COPY_SEMANTIC(Debugger);
    NO_MOVE_SEMANTIC(Debugger);
};

class PtDebugFrame : public PtFrame {
public:
    explicit PANDA_PUBLIC_API PtDebugFrame(Method *method, Frame *interpreterFrame);
    ~PtDebugFrame() override = default;

    bool IsInterpreterFrame() const override
    {
        return isInterpreterFrame_;
    }

    Method *GetMethod() const override
    {
        return method_;
    }

    uint64_t GetVReg(size_t i) const override
    {
        if (!isInterpreterFrame_) {
            return 0;
        }
        return vregs_[i];
    }

    RegisterKind GetVRegKind(size_t i) const override
    {
        if (!isInterpreterFrame_) {
            return PtFrame::RegisterKind::PRIMITIVE;
        }
        return vregKinds_[i];
    }

    size_t GetVRegNum() const override
    {
        return vregs_.size();
    }

    uint64_t GetArgument(size_t i) const override
    {
        if (!isInterpreterFrame_) {
            return 0;
        }
        return args_[i];
    }

    RegisterKind GetArgumentKind(size_t i) const override
    {
        if (!isInterpreterFrame_) {
            return PtFrame::RegisterKind::PRIMITIVE;
        }
        return argKinds_[i];
    }

    size_t GetArgumentNum() const override
    {
        return args_.size();
    }

    uint64_t GetAccumulator() const override
    {
        return acc_;
    }

    RegisterKind GetAccumulatorKind() const override
    {
        return accKind_;
    }

    panda_file::File::EntityId GetMethodId() const override
    {
        return methodId_;
    }

    uint32_t GetBytecodeOffset() const override
    {
        return bcOffset_;
    }

    std::string GetPandaFile() const override
    {
        return pandaFile_;
    }

    // mock API
    uint32_t GetFrameId() const override
    {
        return 0;
    }

private:
    NO_COPY_SEMANTIC(PtDebugFrame);
    NO_MOVE_SEMANTIC(PtDebugFrame);

    bool isInterpreterFrame_;
    Method *method_;
    uint64_t acc_ {0};
    RegisterKind accKind_ {PtFrame::RegisterKind::PRIMITIVE};
    PandaVector<uint64_t> vregs_;
    PandaVector<RegisterKind> vregKinds_;
    PandaVector<uint64_t> args_;
    PandaVector<RegisterKind> argKinds_;
    panda_file::File::EntityId methodId_;
    uint32_t bcOffset_ {0};
    std::string pandaFile_;
};
}  // namespace ark::tooling

#endif  // PANDA_RUNTIME_TOOLING_DEBUGGER_H
