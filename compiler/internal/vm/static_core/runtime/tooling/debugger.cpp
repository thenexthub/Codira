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

#include "debugger.h"

#include "evaluation/expression_loader.h"
#include "include/mem/panda_smart_pointers.h"
#include "include/stack_walker.h"
#include "include/stack_walker-inl.h"
#include "include/thread-inl.h"
#include "include/thread_scopes.h"
#include "include/tooling/pt_location.h"
#include "include/tooling/pt_thread.h"
#include "interpreter/frame.h"
#include "libarkbase/macros.h"
#include "libarkbase/os/mem.h"
#include "libarkbase/utils/expected.h"
#include "libarkbase/utils/span.h"
#include "libarkfile/bytecode_instruction.h"
#include "pt_scoped_managed_code.h"
#include "pt_thread_info.h"
#include "runtime/handle_scope-inl.h"

namespace ark::tooling {
// NOTE(maksenov): remove PtProperty class
static PtProperty FieldToPtProperty(Field *field)
{
    return PtProperty(field);
}

static Field *PtPropertyToField(PtProperty property)
{
    return reinterpret_cast<Field *>(property.GetData());
}

std::optional<Error> Debugger::SetNotification(PtThread thread, bool enable, PtHookType hookType)
{
    if (thread == PtThread::NONE) {
        if (enable) {
            hooks_.EnableGlobalHook(hookType);
        } else {
            hooks_.DisableGlobalHook(hookType);
        }
    } else {
        ManagedThread *managedThread = thread.GetManagedThread();
        if (enable) {
            managedThread->GetPtThreadInfo()->GetHookTypeInfo().Enable(hookType);
        } else {
            managedThread->GetPtThreadInfo()->GetHookTypeInfo().Disable(hookType);
        }
    }

    return {};
}

static bool CheckLocationInClass(const panda_file::File &pf, panda_file::File::EntityId classId,
                                 const PtLocation &location, std::optional<Error> &error)
{
    panda_file::ClassDataAccessor cda(pf, classId);
    bool found = false;
    cda.EnumerateMethods([&pf, &location, &error, &found](panda_file::MethodDataAccessor mda) {
        if (mda.GetMethodId() == location.GetMethodId()) {
            found = true;
            auto codeId = mda.GetCodeId();
            uint32_t codeSize = 0;
            if (codeId.has_value()) {
                panda_file::CodeDataAccessor codeDa(pf, *codeId);
                codeSize = codeDa.GetCodeSize();
            }
            if (location.GetBytecodeOffset() >= codeSize) {
                error = Error(Error::Type::INVALID_BREAKPOINT,
                              std::string("Invalid breakpoint location: bytecode offset (") +
                                  std::to_string(location.GetBytecodeOffset()) + ") >= method code size (" +
                                  std::to_string(codeSize) + ")");
            }
            return false;
        }
        return true;
    });
    return found;
}

std::optional<Error> Debugger::CheckLocation(const PtLocation &location)
{
    std::optional<Error> res;
    runtime_->GetClassLinker()->EnumeratePandaFiles([&location, &res](const panda_file::File &pf) {
        if (pf.GetFilename() != location.GetPandaFile()) {
            return true;
        }

        auto classes = pf.GetClasses();
        bool found = false;
        for (size_t i = 0; i < classes.Size(); i++) {
            panda_file::File::EntityId id(classes[i]);
            if (pf.IsExternal(id) || id.GetOffset() > location.GetMethodId().GetOffset()) {
                continue;
            }

            found = CheckLocationInClass(pf, id, location, res);
            if (found) {
                break;
            }
        }
        if (!found) {
            res =
                Error(Error::Type::METHOD_NOT_FOUND,
                      std::string("Cannot find method with id ") + std::to_string(location.GetMethodId().GetOffset()) +
                          " in panda file '" + std::string(location.GetPandaFile()) + "'");
        }
        return false;
    });
    return res;
}

std::optional<Error> Debugger::SetBreakpoint(const PtLocation &location)
{
    auto error = CheckLocation(location);
    if (error.has_value()) {
        return error;
    }

    os::memory::WriteLockHolder wholder(rwlock_);
    if (!breakpoints_.emplace(location).second) {
        return Error(Error::Type::BREAKPOINT_ALREADY_EXISTS,
                     std::string("Breakpoint already exists: bytecode offset ") +
                         std::to_string(location.GetBytecodeOffset()));
    }

    return {};
}

std::optional<Error> Debugger::RemoveBreakpoint(const PtLocation &location)
{
    if (!EraseBreakpoint(location)) {
        return Error(Error::Type::BREAKPOINT_NOT_FOUND, "Breakpoint not found");
    }

    return {};
}

static ark::Frame *GetPandaFrame(StackWalker *pstack, uint32_t frameDepth, bool *outIsNative = nullptr)
{
    ASSERT(pstack != nullptr);
    StackWalker &stack = *pstack;

    while (stack.HasFrame() && frameDepth != 0) {
        stack.NextFrame();
        --frameDepth;
    }

    bool isNative = false;
    ark::Frame *frame = nullptr;
    if (stack.HasFrame()) {
        if (stack.IsCFrame()) {
            isNative = true;
        } else {
            frame = stack.GetIFrame();
        }
    }

    if (outIsNative != nullptr) {
        *outIsNative = isNative;
    }

    return frame;
}

static ark::Frame *GetPandaFrame(ManagedThread *thread, uint32_t frameDepth = 0, bool *outIsNative = nullptr)
{
    auto stack = StackWalker::Create(thread);
    return GetPandaFrame(&stack, frameDepth, outIsNative);
}

static interpreter::StaticVRegisterRef GetThisAddrVRegByPandaFrame(ark::Frame *frame)
{
    ASSERT(!frame->IsDynamic());
    ASSERT(frame->GetMethod()->GetNumArgs() > 0);
    uint32_t thisRegNum = frame->GetSize() - frame->GetMethod()->GetNumArgs();
    return StaticFrameHandler(frame).GetVReg(thisRegNum);
}

static interpreter::DynamicVRegisterRef GetThisAddrVRegByPandaFrameDyn(ark::Frame *frame)
{
    ASSERT(frame->IsDynamic());
    ASSERT(frame->GetMethod()->GetNumArgs() > 0);
    uint32_t thisRegNum = frame->GetSize() - frame->GetMethod()->GetNumArgs();
    return DynamicFrameHandler(frame).GetVReg(thisRegNum);
}

template <typename Callback>
Expected<ark::Frame *, Error> GetPandaFrameByPtThread(PtThread thread, uint32_t frameDepth, Callback nativeFrameHandler)
{
    ManagedThread *managedThread = thread.GetManagedThread();
    ASSERT(managedThread != nullptr);

    if (MTManagedThread::ThreadIsMTManagedThread(managedThread)) {
        // Check if thread is suspended
        MTManagedThread *mtManagedThread = MTManagedThread::CastFromThread(managedThread);
        if (MTManagedThread::GetCurrent() != mtManagedThread && !mtManagedThread->IsUserSuspended()) {
            return Unexpected(Error(Error::Type::THREAD_NOT_SUSPENDED,
                                    std::string("Thread " + std::to_string(thread.GetId()) + " is not suspended")));
        }
    }

    auto stack = StackWalker::Create(managedThread);
    ark::Frame *frame = GetPandaFrame(&stack, frameDepth, nullptr);
    if (frame == nullptr) {
        // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
        if constexpr (!std::is_same_v<decltype(nativeFrameHandler), std::nullptr_t>) {
            nativeFrameHandler(&stack);
        }
        return Unexpected(Error(Error::Type::FRAME_NOT_FOUND,
                                std::string("Frame not found or native, threadId=" + std::to_string(thread.GetId()) +
                                            " frameDepth=" + std::to_string(frameDepth))));
    }
    return frame;
}

Expected<interpreter::StaticVRegisterRef, Error> Debugger::GetVRegByPandaFrame(ark::Frame *frame,
                                                                               int32_t regNumber) const
{
    if (regNumber == -1) {
        return frame->GetAccAsVReg();
    }

    if (regNumber >= 0 && uint32_t(regNumber) < frame->GetSize()) {
        return StaticFrameHandler(frame).GetVReg(uint32_t(regNumber));
    }

    return Unexpected(
        Error(Error::Type::INVALID_REGISTER, std::string("Invalid register number: ") + std::to_string(regNumber)));
}

Expected<interpreter::DynamicVRegisterRef, Error> Debugger::GetVRegByPandaFrameDyn(ark::Frame *frame,
                                                                                   int32_t regNumber) const
{
    if (regNumber == -1) {
        return frame->template GetAccAsVReg<true>();
    }

    if (regNumber >= 0 && uint32_t(regNumber) < frame->GetSize()) {
        return DynamicFrameHandler(frame).GetVReg(uint32_t(regNumber));
    }

    return Unexpected(
        Error(Error::Type::INVALID_REGISTER, std::string("Invalid register number: ") + std::to_string(regNumber)));
}

std::optional<Error> Debugger::GetThisVariableByFrame(PtThread thread, uint32_t frameDepth, ObjectHeader **thisPtr)
{
    ASSERT_MANAGED_CODE();
    *thisPtr = nullptr;

    std::optional<Error> nativeError;

    auto nativeFrameHandler = [thread, &nativeError, thisPtr](StackWalker *stack) {
        if (!stack->GetCFrame().IsNative()) {
            return;
        }
        if (stack->GetCFrame().GetMethod()->IsStatic()) {
            nativeError =
                Error(Error::Type::INVALID_VALUE, std::string("Static native method, no this address slot, threadId=" +
                                                              std::to_string(thread.GetId())));
            return;
        }
        stack->IterateObjects([thisPtr](auto &vreg) {
            // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
            if constexpr (std::is_same_v<decltype(vreg), interpreter::StaticVRegisterRef &>) {
                ASSERT(vreg.HasObject());
                *thisPtr = vreg.GetReference();
            }
            return false;
        });
    };
    auto ret = GetPandaFrameByPtThread(thread, frameDepth, nativeFrameHandler);
    if (nativeError) {
        return nativeError;
    }
    if (*thisPtr != nullptr) {
        // The value was set by native frame handler
        return {};
    }
    if (!ret) {
        return ret.Error();
    }
    Frame *frame = ret.Value();
    if (frame->GetMethod()->IsStatic()) {
        return Error(Error::Type::INVALID_VALUE,
                     std::string("Static method, no this address slot, threadId=" + std::to_string(thread.GetId())));
    }

    if (frame->IsDynamic()) {
        auto reg = GetThisAddrVRegByPandaFrameDyn(frame);
        *thisPtr = reg.GetReference();
    } else {
        auto reg = GetThisAddrVRegByPandaFrame(ret.Value());
        *thisPtr = reg.GetReference();
    }
    return {};
}

std::optional<Error> Debugger::GetVariable(PtThread thread, uint32_t frameDepth, int32_t regNumber,
                                           VRegValue *result) const
{
    ASSERT_MANAGED_CODE();
    auto ret = GetPandaFrameByPtThread(thread, frameDepth, nullptr);
    if (!ret) {
        return ret.Error();
    }

    Frame *frame = ret.Value();
    if (frame->IsDynamic()) {
        auto reg = GetVRegByPandaFrameDyn(frame, regNumber);
        if (!reg) {
            return reg.Error();
        }

        auto vreg = reg.Value();
        *result = VRegValue(vreg.GetValue());
        return {};
    }

    auto reg = GetVRegByPandaFrame(ret.Value(), regNumber);
    if (!reg) {
        return reg.Error();
    }

    auto vreg = reg.Value();
    *result = VRegValue(vreg.GetValue());
    return {};
}

std::optional<Error> Debugger::SetVariable(PtThread thread, uint32_t frameDepth, int32_t regNumber,
                                           const VRegValue &value) const
{
    ASSERT_MANAGED_CODE();
    auto ret = GetPandaFrameByPtThread(thread, frameDepth, nullptr);
    if (!ret) {
        return ret.Error();
    }

    if (ret.Value()->IsDynamic()) {
        auto reg = GetVRegByPandaFrameDyn(ret.Value(), regNumber);
        if (!reg) {
            return reg.Error();
        }

        auto vreg = reg.Value();
        vreg.SetValue(value.GetValue());
        return {};
    }

    auto reg = GetVRegByPandaFrame(ret.Value(), regNumber);
    if (!reg) {
        return reg.Error();
    }

    auto vreg = reg.Value();
    vreg.SetValue(value.GetValue());
    return {};
}

Expected<std::unique_ptr<PtFrame>, Error> Debugger::GetCurrentFrame(PtThread thread) const
{
    ManagedThread *managedThread = thread.GetManagedThread();
    ASSERT(managedThread != nullptr);

    auto stack = StackWalker::Create(managedThread);
    Method *method = stack.GetMethod();

    Frame *interpreterFrame = nullptr;
    if (!stack.IsCFrame()) {
        interpreterFrame = stack.GetIFrame();
    }

    return {std::make_unique<PtDebugFrame>(method, interpreterFrame)};
}

std::optional<Error> Debugger::EnumerateFrames(PtThread thread, std::function<bool(const PtFrame &)> callback) const
{
    ManagedThread *managedThread = thread.GetManagedThread();
    ASSERT(managedThread != nullptr);

    auto stack = StackWalker::Create(managedThread);
    while (stack.HasFrame()) {
        Method *method = stack.GetMethod();
        Frame *frame = stack.IsCFrame() ? nullptr : stack.GetIFrame();
        PtDebugFrame debugFrame(method, frame);
        if (!callback(debugFrame)) {
            break;
        }
        stack.NextFrame();
    }

    return {};
}

std::optional<Error> Debugger::SuspendThread(PtThread thread) const
{
    ManagedThread *managedThread = thread.GetManagedThread();
    ASSERT(managedThread != nullptr);

    if (!MTManagedThread::ThreadIsMTManagedThread(managedThread)) {
        return Error(Error::Type::THREAD_NOT_FOUND,
                     std::string("Thread ") + std::to_string(thread.GetId()) + " is not MT Thread");
    }
    MTManagedThread *mtManagedThread = MTManagedThread::CastFromThread(managedThread);
    mtManagedThread->Suspend();

    return {};
}

std::optional<Error> Debugger::ResumeThread(PtThread thread) const
{
    ManagedThread *managedThread = thread.GetManagedThread();
    ASSERT(managedThread != nullptr);

    if (!MTManagedThread::ThreadIsMTManagedThread(managedThread)) {
        return Error(Error::Type::THREAD_NOT_FOUND,
                     std::string("Thread ") + std::to_string(thread.GetId()) + " is not MT Thread");
    }
    MTManagedThread *mtManagedThread = MTManagedThread::CastFromThread(managedThread);
    mtManagedThread->Resume();

    return {};
}

std::optional<Error> Debugger::EvaluateExpression(PtThread thread, uint32_t frameNumber, const ExpressionWrapper &expr,
                                                  Method **method, VRegValue *result) const
{
    ASSERT_MANAGED_CODE();
    ASSERT(method != nullptr);
    ASSERT(result != nullptr);
    ASSERT(!thread.GetManagedThread()->HasPendingException());
    // NOTE(dslynko): support custom frame depth
    ASSERT(frameNumber == 0);

    auto ret = GetPandaFrameByPtThread(thread, frameNumber, nullptr);
    if (!ret) {
        return ret.Error();
    }

    // NOTE(dslynko, #22497): must select first non-boot context as target
    auto *ctx = ret.Value()->GetMethod()->GetClass()->GetLoadContext();
    auto optMethod = LoadExpressionBytecode(ctx, expr);
    if (!optMethod) {
        return optMethod.Error();
    }

    *method = optMethod.Value();
    return EvaluateExpression(thread, frameNumber, *method, result);
}

std::optional<Error> Debugger::EvaluateExpression(PtThread thread, [[maybe_unused]] uint32_t frameNumber,
                                                  Method *method, VRegValue *result) const
{
    ASSERT_MANAGED_CODE();
    ASSERT(method != nullptr);
    ASSERT(result != nullptr);
    ASSERT(!thread.GetManagedThread()->HasPendingException());
    // NOTE(dslynko): support custom frame depth
    ASSERT(frameNumber == 0);

    auto value = method->Invoke(thread.GetManagedThread(), nullptr);
    *result = VRegValue(value.GetAsLong());
    return {};
}

std::optional<Error> Debugger::RestartFrame(PtThread thread, uint32_t frameNumber) const
{
    ManagedThread *managedThread = thread.GetManagedThread();
    ASSERT(managedThread != nullptr);

    if (!managedThread->IsUserSuspended()) {
        return Error(Error::Type::THREAD_NOT_SUSPENDED,
                     std::string("Thread ") + std::to_string(thread.GetId()) + " is not suspended");
    }

    auto stack = StackWalker::Create(managedThread);
    ark::Frame *popFrame = nullptr;
    ark::Frame *retryFrame = nullptr;
    uint32_t currentFrameNumber = 0;

    while (stack.HasFrame()) {
        if (stack.IsCFrame()) {
            return Error(Error::Type::OPAQUE_FRAME, std::string("Thread ") + std::to_string(thread.GetId()) +
                                                        ", frame at depth is executing a native method");
        }
        if (currentFrameNumber == frameNumber) {
            popFrame = stack.GetIFrame();
        } else if (currentFrameNumber == (frameNumber + 1)) {
            retryFrame = stack.GetIFrame();
            break;
        }
        ++currentFrameNumber;
        stack.NextFrame();
    }

    if (popFrame == nullptr) {
        return Error(Error::Type::FRAME_NOT_FOUND, std::string("Thread ") + std::to_string(thread.GetId()) +
                                                       " doesn't have managed frame with number " +
                                                       std::to_string(frameNumber));
    }

    if (retryFrame == nullptr) {
        return Error(Error::Type::NO_MORE_FRAMES, std::string("Thread ") + std::to_string(thread.GetId()) +
                                                      " does not have more than one frame on the call stack");
    }

    // Set force pop frames from top to target
    stack.Reset(managedThread);
    while (stack.HasFrame()) {
        ark::Frame *frame = stack.GetIFrame();
        frame->SetForcePop();
        if (frame == popFrame) {
            break;
        }
        stack.NextFrame();
    }
    retryFrame->SetRetryInstruction();

    return {};
}

std::optional<Error> Debugger::NotifyFramePop(PtThread thread, uint32_t depth) const
{
    ManagedThread *managedThread = thread.GetManagedThread();
    ASSERT(managedThread != nullptr);

    /* NOTE: (cmd) the second NotifyFramePop is error. use one debugger instance to resolve this.
    if (!mt_managed_thread->IsUserSuspended()) {
        return Error(Error::Type::THREAD_NOT_SUSPENDED,
                     std::string("Thread ") + std::to_string(thread.GetId()) + " is not suspended");
    }
    */

    bool isNative = false;
    ark::Frame *popFrame = GetPandaFrame(managedThread, depth, &isNative);
    if (popFrame == nullptr) {
        if (isNative) {
            return Error(Error::Type::OPAQUE_FRAME, std::string("Thread ") + std::to_string(thread.GetId()) +
                                                        ", frame at depth is executing a native method");
        }

        return Error(Error::Type::NO_MORE_FRAMES,
                     std::string("Thread ") + std::to_string(thread.GetId()) +
                         ", are no stack frames at the specified depth: " + std::to_string(depth));
    }

    popFrame->SetNotifyPop();
    return {};
}

void Debugger::BytecodePcChanged(ManagedThread *thread, Method *method, uint32_t bcOffset)
{
    ASSERT(bcOffset < method->GetCodeSize() && "code size of current method less then bc_offset");
    PtLocation location(method->GetPandaFile()->GetFilename().c_str(), method->GetFileId(), bcOffset);

    // Step event is reported before breakpoint, according to the spec.
    HandleStep(thread, method, location);
    HandleBreakpoint(thread, method, location);

    if (IsPropertyWatchActive()) {
        if (!HandlePropertyAccess(thread, method, location)) {
            HandlePropertyModify(thread, method, location);
        }
    }
}

void Debugger::ObjectAlloc(BaseClass *klass, ObjectHeader *object, ManagedThread *thread, size_t size)
{
    if (!vmStarted_) {
        return;
    }
    if (thread == nullptr) {
        thread = ManagedThread::GetCurrent();
        if (thread == nullptr) {
            return;
        }
    }

    hooks_.ObjectAlloc(klass, object, PtThread(thread), size);
}

void Debugger::MethodEntry(ManagedThread *managedThread, Method *method)
{
    hooks_.MethodEntry(PtThread(managedThread), method);
}

void Debugger::MethodExit(ManagedThread *managedThread, Method *method)
{
    bool isExceptionTriggered = managedThread->HasPendingException();
    VRegValue retValue(managedThread->GetCurrentFrame()->GetAcc().GetValue());
    hooks_.MethodExit(PtThread(managedThread), method, isExceptionTriggered, retValue);

    HandleNotifyFramePop(managedThread, method, isExceptionTriggered);
}

void Debugger::ClassLoad(Class *klass)
{
    auto *thread = Thread::GetCurrent();
    if (!vmStarted_ || thread->GetThreadType() == Thread::ThreadType::THREAD_TYPE_COMPILER) {
        return;
    }

    hooks_.ClassLoad(PtThread(ManagedThread::CastFromThread(thread)), klass);
}

void Debugger::ClassPrepare(Class *klass)
{
    auto *thread = Thread::GetCurrent();
    if (!vmStarted_ || thread->GetThreadType() == Thread::ThreadType::THREAD_TYPE_COMPILER) {
        return;
    }

    hooks_.ClassPrepare(PtThread(ManagedThread::CastFromThread(thread)), klass);
}

void Debugger::MonitorWait(ObjectHeader *object, int64_t timeout)
{
    hooks_.MonitorWait(PtThread(ManagedThread::GetCurrent()), object, timeout);
}

void Debugger::MonitorWaited(ObjectHeader *object, bool timedOut)
{
    hooks_.MonitorWaited(PtThread(ManagedThread::GetCurrent()), object, timedOut);
}

void Debugger::MonitorContendedEnter(ObjectHeader *object)
{
    hooks_.MonitorContendedEnter(PtThread(ManagedThread::GetCurrent()), object);
}

void Debugger::MonitorContendedEntered(ObjectHeader *object)
{
    hooks_.MonitorContendedEntered(PtThread(ManagedThread::GetCurrent()), object);
}

bool Debugger::HandleBreakpoint(ManagedThread *managedThread, Method *method, const PtLocation &location)
{
    {
        os::memory::ReadLockHolder rholder(rwlock_);
        if (!IsBreakpoint(location)) {
            return false;
        }
    }

    hooks_.Breakpoint(PtThread(managedThread), method, location);
    return true;
}

void Debugger::ExceptionThrow(ManagedThread *thread, Method *method, ObjectHeader *exceptionObject, uint32_t bcOffset)
{
    ASSERT(thread->HasPendingException());
    HandleScope<ObjectHeader *> scope(thread);
    VMHandle<ObjectHeader> handle(thread, exceptionObject);

    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(*method);
    std::pair<Method *, uint32_t> res = ctx.GetCatchMethodAndOffset(method, thread);
    auto *catchMethodFile = res.first->GetPandaFile();

    PtLocation throwLocation {method->GetPandaFile()->GetFilename().c_str(), method->GetFileId(), bcOffset};
    PtLocation catchLocation {catchMethodFile->GetFilename().c_str(), res.first->GetFileId(), res.second};

    hooks_.Exception(PtThread(thread), method, throwLocation, handle.GetPtr(), res.first, catchLocation);
}

void Debugger::ExceptionCatch(ManagedThread *thread, Method *method, ObjectHeader *exceptionObject, uint32_t bcOffset)
{
    ASSERT(!thread->HasPendingException());

    auto *pf = method->GetPandaFile();
    PtLocation catchLocation {pf->GetFilename().c_str(), method->GetFileId(), bcOffset};

    hooks_.ExceptionCatch(PtThread(thread), method, catchLocation, exceptionObject);
}

bool Debugger::HandleStep(ManagedThread *managedThread, Method *method, const PtLocation &location)
{
    hooks_.SingleStep(PtThread(managedThread), method, location);
    return true;
}

void Debugger::HandleNotifyFramePop(ManagedThread *managedThread, Method *method, bool wasPoppedByException)
{
    ark::Frame *frame = GetPandaFrame(managedThread);
    if (frame != nullptr && frame->IsNotifyPop()) {
        hooks_.FramePop(PtThread(managedThread), method, wasPoppedByException);
        frame->ClearNotifyPop();
    }
}

static Field *ResolveField(ManagedThread *thread, const Method *caller, const BytecodeInstruction &inst, bool isStatic)
{
    auto propertyIndex = inst.GetId().AsIndex();
    auto propertyId = caller->GetClass()->ResolveFieldIndex(propertyIndex);
    auto *classLinker = Runtime::GetCurrent()->GetClassLinker();
    ASSERT(classLinker);
    ASSERT(!thread->HasPendingException());
    auto *field = classLinker->GetField(*caller, propertyId, isStatic);
    if (UNLIKELY(field == nullptr)) {
        // Field might be nullptr if a class was not found
        thread->ClearException();
    }
    return field;
}

bool Debugger::HandlePropertyAccess(ManagedThread *thread, Method *method, const PtLocation &location)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    BytecodeInstruction inst(method->GetInstructions() + location.GetBytecodeOffset());
    auto opcode = inst.GetOpcode();
    bool isStatic = false;

    switch (opcode) {
        case BytecodeInstruction::Opcode::LDOBJ_V8_ID16:
        case BytecodeInstruction::Opcode::LDOBJ_64_V8_ID16:
        case BytecodeInstruction::Opcode::LDOBJ_OBJ_V8_ID16:
            break;
        case BytecodeInstruction::Opcode::LDSTATIC_ID16:
        case BytecodeInstruction::Opcode::LDSTATIC_64_ID16:
        case BytecodeInstruction::Opcode::LDSTATIC_OBJ_ID16:
            isStatic = true;
            break;
        default:
            return false;
    }

    Field *field = ResolveField(thread, method, inst, isStatic);
    if (field == nullptr) {
        return false;
    }
    Class *klass = field->GetClass();
    ASSERT(klass);

    {
        os::memory::ReadLockHolder rholder(rwlock_);
        if (FindPropertyWatch(klass->GetFileId(), field->GetFileId(), PropertyWatch::Type::ACCESS) == nullptr) {
            return false;
        }
    }

    PtProperty ptProperty = FieldToPtProperty(field);

    if (isStatic) {
        hooks_.PropertyAccess(PtThread(thread), method, location, nullptr, ptProperty);
    } else {
        interpreter::VRegister &reg = thread->GetCurrentFrame()->GetVReg(inst.GetVReg());
        hooks_.PropertyAccess(PtThread(thread), method, location, reg.GetReference(), ptProperty);
    }

    return true;
}

bool Debugger::HandlePropertyModify(ManagedThread *thread, Method *method, const PtLocation &location)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    BytecodeInstruction inst(method->GetInstructions() + location.GetBytecodeOffset());
    auto opcode = inst.GetOpcode();
    bool isStatic = false;

    switch (opcode) {
        case BytecodeInstruction::Opcode::STOBJ_V8_ID16:
        case BytecodeInstruction::Opcode::STOBJ_64_V8_ID16:
        case BytecodeInstruction::Opcode::STOBJ_OBJ_V8_ID16:
            break;
        case BytecodeInstruction::Opcode::STSTATIC_ID16:
        case BytecodeInstruction::Opcode::STSTATIC_64_ID16:
        case BytecodeInstruction::Opcode::STSTATIC_OBJ_ID16:
            isStatic = true;
            break;
        default:
            return false;
    }

    Field *field = ResolveField(thread, method, inst, isStatic);
    if (field == nullptr) {
        return false;
    }
    Class *klass = field->GetClass();
    ASSERT(klass);

    {
        os::memory::ReadLockHolder rholder(rwlock_);
        if (FindPropertyWatch(klass->GetFileId(), field->GetFileId(), PropertyWatch::Type::MODIFY) == nullptr) {
            return false;
        }
    }

    PtProperty ptProperty = FieldToPtProperty(field);

    VRegValue value(thread->GetCurrentFrame()->GetAcc().GetValue());
    if (isStatic) {
        hooks_.PropertyModification(PtThread(thread), method, location, nullptr, ptProperty, value);
    } else {
        interpreter::VRegister &reg = thread->GetCurrentFrame()->GetVReg(inst.GetVReg());
        hooks_.PropertyModification(PtThread(thread), method, location, reg.GetReference(), ptProperty, value);
    }

    return true;
}

std::optional<Error> Debugger::SetPropertyAccessWatch(BaseClass *klass, PtProperty property)
{
    os::memory::WriteLockHolder wholder(rwlock_);
    ASSERT(!klass->IsDynamicClass());
    panda_file::File::EntityId classId = static_cast<Class *>(klass)->GetFileId();
    panda_file::File::EntityId propertyId = PtPropertyToField(property)->GetFileId();
    if (FindPropertyWatch(classId, propertyId, PropertyWatch::Type::ACCESS) != nullptr) {
        return Error(Error::Type::INVALID_PROPERTY_ACCESS_WATCH,
                     std::string("Invalid property access watch, already exist, ClassID: ") +
                         std::to_string(classId.GetOffset()) +
                         ", PropertyID: " + std::to_string(propertyId.GetOffset()));
    }
    propertyWatches_.emplace_back(classId, propertyId, PropertyWatch::Type::ACCESS);
    return {};
}

std::optional<Error> Debugger::ClearPropertyAccessWatch(BaseClass *klass, PtProperty property)
{
    ASSERT(!klass->IsDynamicClass());
    panda_file::File::EntityId classId = static_cast<Class *>(klass)->GetFileId();
    panda_file::File::EntityId propertyId = PtPropertyToField(property)->GetFileId();
    if (!RemovePropertyWatch(classId, propertyId, PropertyWatch::Type::ACCESS)) {
        return Error(Error::Type::PROPERTY_ACCESS_WATCH_NOT_FOUND,
                     std::string("Property access watch not found, ClassID: ") + std::to_string(classId.GetOffset()) +
                         ", PropertyID: " + std::to_string(propertyId.GetOffset()));
    }
    return {};
}

std::optional<Error> Debugger::SetPropertyModificationWatch(BaseClass *klass, PtProperty property)
{
    os::memory::WriteLockHolder wholder(rwlock_);
    ASSERT(!klass->IsDynamicClass());
    panda_file::File::EntityId classId = static_cast<Class *>(klass)->GetFileId();
    panda_file::File::EntityId propertyId = PtPropertyToField(property)->GetFileId();
    if (FindPropertyWatch(classId, propertyId, PropertyWatch::Type::MODIFY) != nullptr) {
        return Error(Error::Type::INVALID_PROPERTY_MODIFY_WATCH,
                     std::string("Invalid property modification watch, already exist, ClassID: ") +
                         std::to_string(classId.GetOffset()) + ", PropertyID" + std::to_string(propertyId.GetOffset()));
    }
    propertyWatches_.emplace_back(classId, propertyId, PropertyWatch::Type::MODIFY);
    return {};
}

std::optional<Error> Debugger::ClearPropertyModificationWatch(BaseClass *klass, PtProperty property)
{
    ASSERT(!klass->IsDynamicClass());
    panda_file::File::EntityId classId = static_cast<Class *>(klass)->GetFileId();
    panda_file::File::EntityId propertyId = PtPropertyToField(property)->GetFileId();
    if (!RemovePropertyWatch(classId, propertyId, PropertyWatch::Type::MODIFY)) {
        return Error(Error::Type::PROPERTY_MODIFY_WATCH_NOT_FOUND,
                     std::string("Property modification watch not found, ClassID: ") +
                         std::to_string(classId.GetOffset()) + ", PropertyID" + std::to_string(propertyId.GetOffset()));
    }
    return {};
}

bool Debugger::IsBreakpoint(const PtLocation &location) const REQUIRES_SHARED(rwlock_)
{
    auto it = breakpoints_.find(location);
    return it != breakpoints_.end();
}

bool Debugger::EraseBreakpoint(const PtLocation &location)
{
    os::memory::WriteLockHolder wholder(rwlock_);
    auto it = breakpoints_.find(location);
    if (it != breakpoints_.end()) {
        breakpoints_.erase(it);
        return true;
    }
    return false;
}

const tooling::PropertyWatch *Debugger::FindPropertyWatch(panda_file::File::EntityId classId,
                                                          panda_file::File::EntityId fieldId,
                                                          tooling::PropertyWatch::Type type) const
    REQUIRES_SHARED(rwlock_)
{
    for (const auto &pw : propertyWatches_) {
        if (pw.GetClassId() == classId && pw.GetFieldId() == fieldId && pw.GetType() == type) {
            return &pw;
        }
    }

    return nullptr;
}

bool Debugger::RemovePropertyWatch(panda_file::File::EntityId classId, panda_file::File::EntityId fieldId,
                                   tooling::PropertyWatch::Type type)
{
    os::memory::WriteLockHolder wholder(rwlock_);
    auto it = propertyWatches_.begin();
    while (it != propertyWatches_.end()) {
        const auto &pw = *it;
        if (pw.GetClassId() == classId && pw.GetFieldId() == fieldId && pw.GetType() == type) {
            propertyWatches_.erase(it);
            return true;
        }

        it++;
    }

    return false;
}

template <class VRegRef>
static uint64_t GetVRegValue(VRegRef reg)
{
    return reg.HasObject() ? reinterpret_cast<uintptr_t>(reg.GetReference()) : reg.GetLong();
}

template <class VRegRef>
static PtFrame::RegisterKind GetVRegKind([[maybe_unused]] VRegRef reg)
{
    if constexpr (std::is_same<VRegRef, interpreter::DynamicVRegisterRef>::value) {
        return PtFrame::RegisterKind::TAGGED;
    } else {
        return reg.HasObject() ? PtFrame::RegisterKind::REFERENCE : PtFrame::RegisterKind::PRIMITIVE;
    }
}

template <class FrameHandler>
static void FillRegisters(Frame *interpreterFrame, PandaVector<uint64_t> &vregs,
                          PandaVector<PtFrame::RegisterKind> &vregKinds, size_t nregs, PandaVector<uint64_t> &args,
                          PandaVector<PtFrame::RegisterKind> &argKinds, size_t nargs, uint64_t &acc,
                          PtFrame::RegisterKind &accKind)
{
    FrameHandler frameHandler(interpreterFrame);

    vregs.reserve(nregs);
    vregKinds.reserve(nregs);
    for (size_t i = 0; i < nregs; i++) {
        auto vregReg = frameHandler.GetVReg(i);
        vregs.push_back(GetVRegValue(vregReg));
        vregKinds.push_back(GetVRegKind(vregReg));
    }

    args.reserve(nargs);
    argKinds.reserve(nargs);
    for (size_t i = 0; i < nargs; i++) {
        auto argReg = frameHandler.GetVReg(i + nregs);
        args.push_back(GetVRegValue(argReg));
        argKinds.push_back(GetVRegKind(argReg));
    }

    auto accReg = frameHandler.GetAccAsVReg();
    acc = GetVRegValue(accReg);
    accKind = GetVRegKind(accReg);
}

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
PtDebugFrame::PtDebugFrame(Method *method, Frame *interpreterFrame)
    : method_(method), methodId_(method->GetFileId()), pandaFile_(method->GetPandaFile()->GetFilename())
{
    isInterpreterFrame_ = interpreterFrame != nullptr;
    if (!isInterpreterFrame_) {
        return;
    }

    size_t nregs = method->GetNumVregs();
    size_t nargs = method->GetNumArgs();
    if (interpreterFrame->IsDynamic()) {
        FillRegisters<DynamicFrameHandler>(interpreterFrame, vregs_, vregKinds_, nregs, args_, argKinds_, nargs, acc_,
                                           accKind_);
    } else {
        FillRegisters<StaticFrameHandler>(interpreterFrame, vregs_, vregKinds_, nregs, args_, argKinds_, nargs, acc_,
                                          accKind_);
    }

    bcOffset_ = interpreterFrame->GetBytecodeOffset();
}

}  // namespace ark::tooling
