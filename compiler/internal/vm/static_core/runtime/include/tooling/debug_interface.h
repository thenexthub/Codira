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
#ifndef PANDA_RUNTIME_INCLUDE_TOOLING_DEBUG_INTERFACE_H
#define PANDA_RUNTIME_INCLUDE_TOOLING_DEBUG_INTERFACE_H

#include <cstdint>
#include <list>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "libarkbase/macros.h"
#include "libarkbase/utils/expected.h"
#include "libarkfile/file.h"
#include "runtime/include/console_call_type.h"
#include "runtime/include/coretypes/tagged_value.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/thread.h"
#include "runtime/include/tooling/pt_location.h"
#include "runtime/include/tooling/pt_macros.h"
#include "runtime/include/tooling/pt_property.h"
#include "runtime/include/tooling/pt_thread.h"
#include "runtime/include/tooling/vreg_value.h"
#include "runtime/include/typed_value.h"
#include "runtime/interpreter/frame.h"

namespace ark::tooling {
class Error {
public:
    enum class Type {
        BREAKPOINT_NOT_FOUND,
        BREAKPOINT_ALREADY_EXISTS,
        ENTRY_POINT_RESOLVE_ERROR,
        FRAME_NOT_FOUND,
        NO_MORE_FRAMES,
        OPAQUE_FRAME,
        INVALID_BREAKPOINT,
        INVALID_ENTRY_POINT,
        METHOD_NOT_FOUND,
        PANDA_FILE_LOAD_ERROR,
        THREAD_NOT_FOUND,
        THREAD_NOT_SUSPENDED,
        INVALID_REGISTER,
        INVALID_VALUE,
        INVALID_EXPRESSION,
        PROPERTY_ACCESS_WATCH_NOT_FOUND,
        INVALID_PROPERTY_ACCESS_WATCH,
        PROPERTY_MODIFY_WATCH_NOT_FOUND,
        INVALID_PROPERTY_MODIFY_WATCH,
        DEPRECATED,
    };

    Error(Type type, std::string msg) : type_(type), msg_(std::move(msg)) {}

    Type GetType() const
    {
        return type_;
    }

    std::string GetMessage() const
    {
        return msg_;
    }

    ~Error() = default;

    DEFAULT_COPY_SEMANTIC(Error);
    DEFAULT_MOVE_SEMANTIC(Error);

private:
    Type type_;
    std::string msg_;
};

class PtFrame {
public:
    enum class RegisterKind { PRIMITIVE, REFERENCE, TAGGED };

    PtFrame() = default;

    virtual bool IsInterpreterFrame() const = 0;

    virtual Method *GetMethod() const = 0;

    virtual uint64_t GetVReg(size_t i) const = 0;

    virtual RegisterKind GetVRegKind(size_t i) const = 0;

    virtual size_t GetVRegNum() const = 0;

    virtual uint64_t GetArgument(size_t i) const = 0;

    virtual RegisterKind GetArgumentKind(size_t i) const = 0;

    virtual size_t GetArgumentNum() const = 0;

    virtual uint64_t GetAccumulator() const = 0;

    virtual RegisterKind GetAccumulatorKind() const = 0;

    virtual panda_file::File::EntityId GetMethodId() const = 0;

    virtual uint32_t GetBytecodeOffset() const = 0;

    virtual std::string GetPandaFile() const = 0;

    // mock API
    virtual uint32_t GetFrameId() const = 0;

    virtual ~PtFrame() = default;

    NO_COPY_SEMANTIC(PtFrame);
    NO_MOVE_SEMANTIC(PtFrame);
};

struct PtStepRange {
    uint32_t startBcOffset {0};
    uint32_t endBcOffset {0};
};

// * * * * *
// Mock API helpers
// NOTE(maksenov): cleanup
// * * * * *

using ExceptionID = panda_file::File::EntityId;
using ExecutionContextId = panda_file::File::EntityId;
using ThreadGroup = uint32_t;

using ExpressionWrapper = std::string;
using ExceptionWrapper = std::string;

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
struct ThreadInfo {
    char *name;
    size_t nameLength;
    int32_t priority;
    bool isDaemon;
    ThreadGroup threadGroup;
};

enum PauseReason {
    AMBIGUOUS,
    ASSERT,
    DEBUGCOMMAND,
    DOM,
    EVENTLISTENER,
    EXCEPTION,
    INSTRUMENTATION,
    OOM,
    OTHER,
    PROMISEREJECTION,
    XHR,
    BREAK_ON_START,
    STEP
};

struct ExecutionContextWrapper {
    ExecutionContextId id;
    std::string origin;
    std::string name;
};

enum class PtHookType {
    PT_HOOK_TYPE_BREAKPOINT,
    PT_HOOK_TYPE_LOAD_MODULE,
    PT_HOOK_TYPE_PAUSED,
    PT_HOOK_TYPE_EXCEPTION,
    PT_HOOK_TYPE_EXCEPTION_CATCH,
    PT_HOOK_TYPE_PROPERTY_ACCESS,
    PT_HOOK_TYPE_PROPERTY_MODIFICATION,
    PT_HOOK_TYPE_CONSOLE_CALL,
    PT_HOOK_TYPE_FRAME_POP,
    PT_HOOK_TYPE_GARBAGE_COLLECTION_START,
    PT_HOOK_TYPE_GARBAGE_COLLECTION_FINISH,
    PT_HOOK_TYPE_METHOD_ENTRY,
    PT_HOOK_TYPE_METHOD_EXIT,
    PT_HOOK_TYPE_SINGLE_STEP,
    PT_HOOK_TYPE_THREAD_START,
    PT_HOOK_TYPE_THREAD_END,
    PT_HOOK_TYPE_VM_DEATH,
    PT_HOOK_TYPE_VM_INITIALIZATION,
    PT_HOOK_TYPE_VM_START,
    PT_HOOK_TYPE_EXCEPTION_REVOKED,
    PT_HOOK_TYPE_EXECUTION_CONTEXT_CREATEED,
    PT_HOOK_TYPE_EXECUTION_CONTEXT_DESTROYED,
    PT_HOOK_TYPE_EXECUTION_CONTEXTS_CLEARED,
    PT_HOOK_TYPE_INSPECT_REQUESTED,
    PT_HOOK_TYPE_CLASS_LOAD,
    PT_HOOK_TYPE_CLASS_PREPARE,
    PT_HOOK_TYPE_MONITOR_WAIT,
    PT_HOOK_TYPE_MONITOR_WAITED,
    PT_HOOK_TYPE_MONITOR_CONTENDED_ENTER,
    PT_HOOK_TYPE_MONITOR_CONTENDED_ENTERED,
    PT_HOOK_TYPE_OBJECT_ALLOC,
    // The count of hooks. Don't move
    PT_HOOK_TYPE_COUNT
};

// * * * * *
// Mock API helpers ends
// * * * * *

class PtHooks {
public:
    PtHooks() = default;

    /**
     * @brief Method is called by the runtime when breakpoint hits. Thread where breakpoint hits is stopped until
     * continue or step event will be received
     * @param thread Identifier of the thread where breakpoint hits. Now the callback is called in the same
     * thread
     * @param method Method
     * @param location Breakpoint location
     */
    virtual void Breakpoint(PtThread /* thread */, Method * /* method */, const PtLocation & /* location */) {}

    /**
     * @brief Method is called by the runtime when panda file is loaded
     * @param pandaFileName Path to panda file that is loaded
     */
    virtual void LoadModule(std::string_view /* pandaFileName */) {}

    /**
     * @brief Method is called by the runtime when managed thread is attached to it
     * @param thread The attached thread
     */
    virtual void ThreadStart(PtThread /* thread */) {}

    /**
     * @brief Method is called by the runtime when managed thread is detached
     * @param thread The detached thread
     */
    virtual void ThreadEnd(PtThread /* thread */) {}

    /// @brief Method is called by the runtime when virtual machine start initialization
    virtual void VmStart() {}

    /**
     * @brief Method is called by the runtime when virtual machine finish initialization
     * @param thread The initial thread
     */
    virtual void VmInitialization(PtThread /* thread */) {}

    /// @brief Method is called by the runtime when virtual machine death
    virtual void VmDeath() {}

    /**
     * @brief Method is called by the runtime when a class is first loaded
     * @param thread Thread loading the class
     * @param klass Class being loaded
     */
    virtual void ClassLoad(PtThread /* thread */, BaseClass * /* klass */) {}

    /**
     * @brief Method is called by the runtime when class preparation is complete
     * @param thread Thread generating the class prepare
     * @param klass Class being prepared
     */
    virtual void ClassPrepare(PtThread /* thread */, BaseClass * /* klass */) {}

    /**
     * @brief Method is called by the runtime when a thread is about to wait on an object
     * @param thread The thread about to wait
     * @param object Reference to the monitor
     * @param timeout The number of milliseconds the thread will wait
     */
    virtual void MonitorWait(PtThread /* thread */, ObjectHeader * /* object */, int64_t /* timeout */) {}

    /**
     * @brief Method is called by the runtime when a thread finishes waiting on an object
     * @param thread The thread about to wait
     * @param object Reference to the monitor
     * @param timedOut True if the monitor timed out
     */
    virtual void MonitorWaited(PtThread /* thread */, ObjectHeader * /* object */, bool /* timedOut */) {}

    /**
     * @brief Method is called by the runtime when a thread is attempting to enter a monitor already acquired by another
     * thread
     * @param thread The thread about to wait
     * @param object Reference to the monitor
     */
    virtual void MonitorContendedEnter(PtThread /* thread */, ObjectHeader * /* object */) {}

    /**
     * @brief Method is called by the runtime when a thread enters a monitor after waiting for it to be released by
     * another thread
     * @param thread The thread about to wait
     * @param object Reference to the monitor
     */
    virtual void MonitorContendedEntered(PtThread /* thread */, ObjectHeader * /* object */) {}

    virtual void Exception(PtThread /* thread */, Method * /* method */, const PtLocation & /* location */,
                           ObjectHeader * /* exceptionObject */, Method * /* catchMethod */,
                           const PtLocation & /* catchLocation */)
    {
    }

    virtual void ExceptionCatch(PtThread /* thread */, Method * /* catchMethod */, const PtLocation & /* location */,
                                ObjectHeader * /* exceptionObject */)
    {
    }

    virtual void PropertyAccess(PtThread /* thread */, Method * /* catchMethod */, const PtLocation & /* location */,
                                ObjectHeader * /* object */, PtProperty /* property */)
    {
    }

    virtual void PropertyModification(PtThread /* thread */, Method * /* method */, const PtLocation & /* location */,
                                      ObjectHeader * /* object */, PtProperty /* property */, VRegValue /* newValue */)
    {
    }

    virtual void ConsoleCall(PtThread /* thread */, ConsoleCallType /* type */, uint64_t /* timestamp */,
                             const PandaVector<TypedValue> & /* arguments */)
    {
    }

    virtual void FramePop(PtThread /* thread */, Method * /* method */, bool /* wasPoppedByException */) {}

    virtual void GarbageCollectionFinish() {}

    virtual void GarbageCollectionStart() {}

    virtual void ObjectAlloc(BaseClass * /* klass */, ObjectHeader * /* object */, PtThread /* thread */,
                             size_t /* size */)
    {
    }

    virtual void MethodEntry(PtThread /* thread */, Method * /* method */) {}

    virtual void MethodExit(PtThread /* thread */, Method * /* method */, bool /* wasPoppedByException */,
                            VRegValue /* returnValue */)
    {
    }

    virtual void SingleStep(PtThread /* thread */, Method * /* method */, const PtLocation & /* location */) {}

    // * * * * *
    // Deprecated hooks
    // * * * * *

    virtual void Paused(PauseReason /* reason */) {}
    virtual void Breakpoint(PtThread /* thread */, const PtLocation & /* location */) {}
    virtual void SingleStep(PtThread /* thread */, const PtLocation & /* location */) {}

    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    virtual void ExceptionRevoked(ExceptionWrapper /* reason */, ExceptionID /* exceptionId */) {}

    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    virtual void ExecutionContextCreated(ExecutionContextWrapper /* context */) {}

    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    virtual void ExecutionContextDestroyed(ExecutionContextWrapper /* context */) {}

    virtual void ExecutionContextsCleared() {}

    // * * * * *
    // Deprecated hooks end
    // * * * * *

    virtual ~PtHooks() = default;

    NO_COPY_SEMANTIC(PtHooks);
    NO_MOVE_SEMANTIC(PtHooks);
};

class DebugInterface {
public:
    DebugInterface() = default;

    /**
     * @brief Register debug hooks in the runtime
     * @param hooks Pointer to object that implements PtHooks interface
     * @return Error if any errors occur
     */
    virtual std::optional<Error> RegisterHooks(PtHooks *hooks) = 0;

    /**
     * @brief Unregister debug hooks in the runtime
     * @return Error if any errors occur
     */
    virtual std::optional<Error> UnregisterHooks() = 0;

    /**
     * @brief Enable all debug hooks in the runtime
     * @return Error if any errors occur
     */
    virtual std::optional<Error> EnableAllGlobalHook() = 0;

    /**
     * @brief Disable all debug hooks in the runtime
     * @return Error if any errors occur
     */
    virtual std::optional<Error> DisableAllGlobalHook() = 0;

    /**
     * @brief Set notification to hook (enable/disable).
     * @param thread If thread is NONE, the notification is enabled or disabled globally
     * @param enable Enable or disable notifications (true - enable, false - disable)
     * @param hook_type Type of hook that must be enabled or disabled
     * @return Error if any errors occur
     */
    virtual std::optional<Error> SetNotification(PtThread thread, bool enable, PtHookType hookType) = 0;

    /**
     * @brief Set breakpoint to @param location
     * @param location Breakpoint location
     * @return Error if any errors occur
     */
    virtual std::optional<Error> SetBreakpoint(const PtLocation &location) = 0;

    /**
     * @brief Remove breakpoint from @param location
     * @param location Breakpoint location
     * @return Error if any errors occur
     */
    virtual std::optional<Error> RemoveBreakpoint(const PtLocation &location) = 0;

    /**
     * @brief Get Frame
     * @param thread Identifier of the thread
     * @return Frame object that implements PtFrame or Error if any errors occur
     */
    virtual Expected<std::unique_ptr<PtFrame>, Error> GetCurrentFrame(PtThread thread) const = 0;

    /**
     * @brief Enumerates managed frames in the thread @param threadId
     * @param thread Identifier of the thread
     * @param callback Callback that is called for each frame. Should return true to continue and false to stop
     * enumerating
     * @return Error if any errors occur
     */
    virtual std::optional<Error> EnumerateFrames(PtThread thread,
                                                 std::function<bool(const PtFrame &)> callback) const = 0;

    /**
     * @brief Suspend thread
     * @param thread Identifier of the thread
     * @return Error if any errors occur
     */
    virtual std::optional<Error> SuspendThread(PtThread thread) const = 0;

    /**
     * @brief Resume thread
     * @param thread Identifier of the thread
     * @return Error if any errors occur
     */
    virtual std::optional<Error> ResumeThread(PtThread thread) const = 0;

    virtual ~DebugInterface() = default;

    virtual std::optional<Error> GetThreadList(PandaVector<PtThread> *threadList) const = 0;

    virtual std::optional<Error> SetVariable(PtThread /* thread */, uint32_t /* frameDepth */, int32_t /* regNumber */,
                                             const VRegValue & /* value */) const
    {
        return {};
    }

    virtual std::optional<Error> GetVariable(PtThread /* thread */, uint32_t /* frameDepth */, int32_t /* regNumber */,
                                             VRegValue * /* value */) const
    {
        return {};
    }

    virtual std::optional<Error> EvaluateExpression(PtThread thread, uint32_t frameNumber,
                                                    const ExpressionWrapper &expr, Method **method,
                                                    VRegValue *result) const = 0;

    virtual std::optional<Error> EvaluateExpression(PtThread thread, uint32_t frameNumber, Method *method,
                                                    VRegValue *result) const = 0;

    virtual std::optional<Error> GetThreadInfo(PtThread thread, ThreadInfo *infoPtr) const = 0;

    virtual std::optional<Error> RestartFrame(PtThread thread, uint32_t frameNumber) const = 0;

    virtual std::optional<Error> SetAsyncCallStackDepth(uint32_t maxDepth) const = 0;

    virtual std::optional<Error> GetProperties(uint32_t *countPtr, char ***propertyPtr) const = 0;

    virtual std::optional<Error> NotifyFramePop(PtThread thread, uint32_t depth) const = 0;

    virtual std::optional<Error> GetThisVariableByFrame(PtThread /* thread */, uint32_t /* frameDepth */,
                                                        ObjectHeader ** /* this_ptr */)
    {
        return {};
    }

    virtual std::optional<Error> SetPropertyAccessWatch(BaseClass * /* klass */, PtProperty /* property */)
    {
        return {};
    }

    virtual std::optional<Error> ClearPropertyAccessWatch(BaseClass * /* klass */, PtProperty /* property */)
    {
        return {};
    }

    virtual std::optional<Error> SetPropertyModificationWatch(BaseClass * /* klass */, PtProperty /* property */)
    {
        return {};
    }

    virtual std::optional<Error> ClearPropertyModificationWatch(BaseClass * /* klass */, PtProperty /* property */)
    {
        return {};
    }

    NO_COPY_SEMANTIC(DebugInterface);
    NO_MOVE_SEMANTIC(DebugInterface);
};
}  // namespace ark::tooling

#endif  // PANDA_RUNTIME_INCLUDE_TOOLING_DEBUG_INTERFACE_H
