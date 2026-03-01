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

#ifndef PANDA_RUNTIME_METHOD_INL_H_
#define PANDA_RUNTIME_METHOD_INL_H_

#include "entrypoints/entrypoints.h"
#include "libarkfile/code_data_accessor-inl.h"
#include "libarkfile/code_data_accessor.h"
#include "libarkfile/file.h"
#include "libarkfile/method_data_accessor-inl.h"
#include "libarkfile/method_data_accessor.h"
#include "libarkfile/proto_data_accessor-inl.h"
#include "libarkfile/proto_data_accessor.h"
#include "runtime/bridge/bridge.h"
#include "runtime/include/class_linker.h"
#include "runtime/include/method.h"
#include "runtime/include/runtime.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/runtime_notification.h"
#include "runtime/include/thread-inl.h"
#include "runtime/interpreter/interpreter.h"
#include "runtime/interpreter/runtime_interface.h"
#include "runtime/osr.h"

namespace ark {

inline void FrameDeleter::operator()(Frame *frame) const
{
    interpreter::RuntimeInterface::FreeFrame(thread_, frame);
}

class InvokeHelperStatic {
public:
    static constexpr bool IS_DYNAMIC = false;

    ALWAYS_INLINE inline static uint32_t GetFrameSize(uint32_t numVregs, uint32_t numDeclaredArgs,
                                                      [[maybe_unused]] uint32_t numActualArgs)
    {
        return numVregs + numDeclaredArgs;
    }

    // NOLINTNEXTLINE(misc-unused-parameters)
    ALWAYS_INLINE inline static void InitActualArgs(Frame *frame, Span<Value> argsSpan, uint32_t numVregs,
                                                    [[maybe_unused]] uint32_t numDeclaredArgs)
    {
        StaticFrameHandler staticFrameHelper(frame);
        uint32_t numActualArgs = argsSpan.Size();
        for (size_t i = 0; i < numActualArgs; ++i) {
            if (argsSpan[i].IsReference()) {
                staticFrameHelper.GetVReg(numVregs + i).SetReference(argsSpan[i].GetAs<ObjectHeader *>());
            } else {
                staticFrameHelper.GetVReg(numVregs + i).SetPrimitive(argsSpan[i].GetAs<int64_t>());
            }
        }
    }

    ALWAYS_INLINE inline static void InterpreterExecute(ManagedThread *thread, const uint8_t *pc, Frame *frame)
    {
        interpreter::Execute(thread, pc, frame);
    }

    ALWAYS_INLINE static Frame *CreateFrame([[maybe_unused]] ManagedThread *thread, uint32_t nregsSize, Method *method,
                                            Frame *prev, uint32_t nregs, uint32_t numActualArgs)
    {
        return interpreter::RuntimeInterface::CreateFrameWithActualArgsAndSize(nregsSize, nregs, numActualArgs, method,
                                                                               prev);
    }
};

class InvokeHelperDynamic {
public:
    static constexpr bool IS_DYNAMIC = true;

    ALWAYS_INLINE inline static uint32_t GetFrameSize(uint32_t numVregs, uint32_t numDeclaredArgs,
                                                      uint32_t numActualArgs)
    {
        return numVregs + std::max(numDeclaredArgs, numActualArgs);
    }

    // NOLINTNEXTLINE(misc-unused-parameters)
    ALWAYS_INLINE inline static void InitActualArgs(Frame *frame, Span<coretypes::TaggedValue> argsSpan,
                                                    [[maybe_unused]] uint32_t numVregs,
                                                    [[maybe_unused]] uint32_t numDeclaredArgs)
    {
        frame->SetDynamic();

        DynamicFrameHandler dynamicFrameHelper(frame);
        uint32_t numActualArgs = argsSpan.Size();
        for (size_t i = 0; i < numActualArgs; ++i) {
            dynamicFrameHelper.GetVReg(numVregs + i).SetValue(argsSpan[i].GetRawData());
        }

        for (size_t i = numActualArgs; i < numDeclaredArgs; i++) {
            dynamicFrameHelper.GetVReg(numVregs + i).SetValue(TaggedValue::VALUE_UNDEFINED);
        }
    }

    ALWAYS_INLINE inline static void InterpreterExecute(ManagedThread *thread, const uint8_t *pc, Frame *frame)
    {
        interpreter::Execute(thread, pc, frame);
    }

    ALWAYS_INLINE inline static coretypes::TaggedValue CompiledCodeExecute(ManagedThread *thread, Method *method,
                                                                           uint32_t numArgs,
                                                                           coretypes::TaggedValue *args)
    {
        Frame *currentFrame = thread->GetCurrentFrame();
        bool isCompiled = thread->IsCurrentFrameCompiled();

        ASSERT(numArgs >= 2U);  // NOTE(asoldatov): Adjust this check
        uint64_t ret = InvokeCompiledCodeWithArgArrayDyn(reinterpret_cast<uint64_t *>(args), numArgs, currentFrame,
                                                         method, thread);
        thread->SetCurrentFrameIsCompiled(isCompiled);
        thread->SetCurrentFrame(currentFrame);
        return coretypes::TaggedValue(ret);
    }

    ALWAYS_INLINE static Frame *CreateFrame([[maybe_unused]] ManagedThread *thread, uint32_t nregsSize, Method *method,
                                            Frame *prev, uint32_t nregs, uint32_t numActualArgs)
    {
        return interpreter::RuntimeInterface::CreateFrameWithActualArgsAndSize(nregsSize, nregs, numActualArgs, method,
                                                                               prev);
    }
};

template <class InvokeHelper, class ValueT>
ValueT Method::GetReturnValueFromException()
{
    if constexpr (InvokeHelper::IS_DYNAMIC) {  // NOLINT(readability-braces-around-statements)
        return TaggedValue::Undefined();
    } else {  // NOLINT(readability-misleading-indentation)
        if (GetReturnType().IsReference()) {
            return Value(nullptr);
        }
        return Value(static_cast<int64_t>(0));
    }
}

template <class InvokeHelper, class ValueT>
ValueT Method::GetReturnValueFromAcc(interpreter::AccVRegister &aacVreg)
{
    if constexpr (InvokeHelper::IS_DYNAMIC) {  // NOLINT(readability-braces-around-statements)
        return TaggedValue(aacVreg.GetAs<uint64_t>());
    } else {  // NOLINT(readability-misleading-indentation)
        ASSERT(GetReturnType().GetId() != panda_file::Type::TypeId::TAGGED);
        if (GetReturnType().GetId() != panda_file::Type::TypeId::VOID) {
            interpreter::StaticVRegisterRef acc = aacVreg.AsVRegRef<false>();
            if (acc.HasObject()) {
                return Value(aacVreg.GetReference());
            }
            return Value(aacVreg.GetLong());
        }
        return Value(static_cast<int64_t>(0));
    }
}

// CC-OFFNXT(G.FUD.06) perf critical
inline Value Method::InvokeCompiledCode(ManagedThread *thread, uint32_t numArgs, Value *args)
{
    Frame *currentFrame = thread->GetCurrentFrame();
    Span<Value> argsSpan(args, numArgs);
    bool isCompiled = thread->IsCurrentFrameCompiled();
    // Use frame allocator to alloc memory for parameters as thread can be terminated and
    // InvokeCompiledCodeWithArgArray will not return in this case we will get memory leak with internal
    // allocator
    mem::StackFrameAllocator *allocator = thread->GetStackFrameAllocator();
    auto valuesDeleter = [allocator](int64_t *values) {
        if (values != nullptr) {
            allocator->Free(values);
        }
    };
    auto values = PandaUniquePtr<int64_t, decltype(valuesDeleter)>(nullptr, valuesDeleter);
    if (numArgs > 0) {
        // In the worse case we are calling a dynamic method in which all arguments are pairs ot int64_t
        // That is why we allocate 2 x num_actual_args
        size_t capacity = numArgs * sizeof(int64_t);
        // All allocations though FrameAllocator must be aligned
        capacity = AlignUp(capacity, GetAlignmentInBytes(DEFAULT_FRAME_ALIGNMENT));
        values.reset(reinterpret_cast<int64_t *>(allocator->Alloc(capacity)));
        Span<int64_t> valuesSpan(values.get(), capacity);
        for (uint32_t i = 0; i < numArgs; ++i) {
            if (argsSpan[i].IsReference()) {
                valuesSpan[i] = reinterpret_cast<int64_t>(argsSpan[i].GetAs<ObjectHeader *>());
            } else {
                valuesSpan[i] = argsSpan[i].GetAs<int64_t>();
            }
        }
    }

    uint64_t retValue = InvokeCompiledCodeWithArgArray(values.get(), currentFrame, this, thread);

    thread->SetCurrentFrameIsCompiled(isCompiled);
    thread->SetCurrentFrame(currentFrame);
    if (UNLIKELY(thread->HasPendingException())) {
        retValue = 0;
    }
    return GetReturnValueFromTaggedValue(retValue);
}

template <class InvokeHelper, class ValueT>
ValueT Method::InvokeInterpretedCode(ManagedThread *thread, uint32_t numActualArgs, ValueT *args)
{
    Frame *currentFrame = thread->GetCurrentFrame();
    PandaUniquePtr<Frame, FrameDeleter> frame = InitFrame<InvokeHelper>(thread, numActualArgs, args, currentFrame);
    if (UNLIKELY(frame.get() == nullptr)) {
        ark::ThrowOutOfMemoryError("CreateFrame failed: " + GetFullName());
        return GetReturnValueFromException<InvokeHelper, ValueT>();
    }

    InvokeEntry<InvokeHelper>(thread, currentFrame, frame.get(), GetInstructions());

    ValueT res = (UNLIKELY(thread->HasPendingException()))
                     ? GetReturnValueFromException<InvokeHelper, ValueT>()
                     : GetReturnValueFromAcc<InvokeHelper, ValueT>(frame->GetAcc());
    LOG(DEBUG, INTERPRETER) << "Invoke exit: " << GetFullName();
    return res;
}

template <class InvokeHelper>
void Method::InvokeEntry(ManagedThread *thread, Frame *currentFrame, Frame *frame, const uint8_t *pc)
{
    LOG(DEBUG, INTERPRETER) << "Invoke entry: " << GetFullName();

    auto isCompiled = thread->IsCurrentFrameCompiled();
    thread->SetCurrentFrameIsCompiled(false);
    thread->SetCurrentFrame(frame);

    if (isCompiled && currentFrame != nullptr) {
        // Create C2I bridge frame in case of previous frame is a native frame or other compiler frame.
        // But create only if the previous frame is not a C2I bridge already.
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
        C2IBridge bridge;
        if (!StackWalker::IsBoundaryFrame<FrameKind::INTERPRETER>(currentFrame)) {
            bridge = {0, reinterpret_cast<uintptr_t>(currentFrame), COMPILED_CODE_TO_INTERPRETER,
                      thread->GetNativePc()};
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            frame->SetPrevFrame(reinterpret_cast<Frame *>(&bridge.v[1]));
        }
        // Workaround for
        // issues #2888 and #2925
        // We cannot make OSR on the methods called from here, because:
        // 1. If caller is native method, then C2I bridge, created above, is not complete. It can be fixed by
        //    allocating full size boundary frame.
        // 2. If caller is compiled method, then we got here from entrypoint. But currently compiler creates
        //    boundary frame with pseudo LR value, that doesn't point to the instruction after call, thereby
        //    OSR will fail. It can be fixed by addresses patching, currently codegen hasn't such machinery.
        // NOTE(msherstennikov): fix issue
        frame->DisableOsr();
        Runtime::GetCurrent()->GetNotificationManager()->MethodEntryEvent(thread, this);
        InvokeHelper::InterpreterExecute(thread, pc, frame);
        Runtime::GetCurrent()->GetNotificationManager()->MethodExitEvent(thread, this);
        thread->SetCurrentFrameIsCompiled(true);
    } else {
        Runtime::GetCurrent()->GetNotificationManager()->MethodEntryEvent(thread, this);
        InvokeHelper::InterpreterExecute(thread, pc, frame);
        Runtime::GetCurrent()->GetNotificationManager()->MethodExitEvent(thread, this);
    }
    thread->SetCurrentFrame(currentFrame);
}

inline coretypes::TaggedValue Method::InvokeDyn(ManagedThread *thread, uint32_t numArgs, coretypes::TaggedValue *args)
{
    return InvokeDyn<InvokeHelperDynamic>(thread, numArgs, args);
}

template <class InvokeHelper>
inline coretypes::TaggedValue Method::InvokeDyn(ManagedThread *thread, uint32_t numArgs, coretypes::TaggedValue *args)
{
    return InvokeImpl<InvokeHelper>(thread, numArgs, args, false);
}

inline coretypes::TaggedValue Method::InvokeContext(ManagedThread *thread, const uint8_t *pc,
                                                    coretypes::TaggedValue *acc, uint32_t nregs,
                                                    coretypes::TaggedValue *regs)
{
    return InvokeContext<InvokeHelperDynamic>(thread, pc, acc, nregs, regs);
}

template <class InvokeHelper>
// CC-OFFNXT(G.FUD.06) perf critical
inline coretypes::TaggedValue Method::InvokeContext(ManagedThread *thread, const uint8_t *pc,
                                                    coretypes::TaggedValue *acc, uint32_t nregs,
                                                    coretypes::TaggedValue *regs)
{
    static_assert(InvokeHelper::IS_DYNAMIC == true);
    ASSERT(GetReturnType().GetId() == panda_file::Type::TypeId::VOID ||
           GetReturnType().GetId() == panda_file::Type::TypeId::TAGGED);

    TaggedValue res(TaggedValue::VALUE_UNDEFINED);

    if (!Verify()) {
        auto ctx = Runtime::GetCurrent()->GetLanguageContext(*this);
        ark::ThrowVerificationException(ctx, GetFullName());
        // 'res' is not a heap object.
        // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
        return res;
    }

    Frame *currentFrame = thread->GetCurrentFrame();
    PandaUniquePtr<Frame, FrameDeleter> frame(
        // true value is is_dynamic
        interpreter::RuntimeInterface::CreateFrameWithActualArgs<true>(nregs, nregs, this, currentFrame),
        FrameDeleter(thread));
    if (UNLIKELY(frame.get() == nullptr)) {
        ark::ThrowOutOfMemoryError("CreateFrame failed: " + GetFullName());
        return res;
    }

    frame->SetDynamic();

    DynamicFrameHandler dynamicFrameHelper(frame.get());
    Span<TaggedValue> argsSpan(regs, nregs);
    for (size_t i = 0; i < nregs; ++i) {
        dynamicFrameHelper.GetVReg(i).SetValue(argsSpan[i].GetRawData());
    }

    LOG(DEBUG, INTERPRETER) << "Invoke entry: " << GetFullName();

    dynamicFrameHelper.GetAcc().SetValue(acc->GetRawData());
    InvokeEntry<InvokeHelper>(thread, currentFrame, frame.get(), pc);
    res = TaggedValue(dynamicFrameHelper.GetAcc().GetAs<uint64_t>());

    LOG(DEBUG, INTERPRETER) << "Invoke exit: " << GetFullName();
    return res;
}

template <class InvokeHelper, class ValueT>
Frame *Method::EnterNativeMethodFrame(ManagedThread *thread, uint32_t numVregs, uint32_t numArgs, ValueT *args)
{
    Frame *currentFrame = thread->GetCurrentFrame();
    PandaUniquePtr<Frame, FrameDeleter> frame =
        InitFrameWithNumVRegs<InvokeHelper, ValueT, true>(thread, numVregs, numArgs, args, currentFrame);
    if (UNLIKELY(frame.get() == nullptr)) {
        ark::ThrowOutOfMemoryError("CreateFrame failed: " + GetFullName());
        return nullptr;
    }

    LOG(DEBUG, INTERPRETER) << "Enter native frame";

    thread->SetCurrentFrame(frame.get());
    return frame.release();
}

inline void Method::ExitNativeMethodFrame(ManagedThread *thread)
{
    Frame *currentFrame = thread->GetCurrentFrame();
    ASSERT(currentFrame != nullptr);

    LOG(DEBUG, INTERPRETER) << "Exit native frame";

    thread->SetCurrentFrame(currentFrame->GetPrevFrame());
    FreeFrame(currentFrame);
}

template <class InvokeHelper, class ValueT>
PandaUniquePtr<Frame, FrameDeleter> Method::InitFrame(ManagedThread *thread, uint32_t numActualArgs, ValueT *args,
                                                      Frame *currentFrame)
{
    ASSERT(codeId_.IsValid());
    auto numVregs = GetNumVregs();
    return InitFrameWithNumVRegs<InvokeHelper, ValueT, false>(thread, numVregs, numActualArgs, args, currentFrame);
}

template <class InvokeHelper, class ValueT, bool IS_NATIVE_METHOD>
PandaUniquePtr<Frame, FrameDeleter> Method::InitFrameWithNumVRegs(ManagedThread *thread, uint32_t numVregs,
                                                                  uint32_t numActualArgs, ValueT *args,
                                                                  Frame *currentFrame)
{
    Span<ValueT> argsSpan(args, numActualArgs);

    uint32_t numDeclaredArgs = GetNumArgs();
    uint32_t frameSize = InvokeHelper::GetFrameSize(numVregs, numDeclaredArgs, numActualArgs);

    Frame *framePtr;
    // NOLINTNEXTLINE(readability-braces-around-statements)
    if constexpr (IS_NATIVE_METHOD) {
        framePtr = interpreter::RuntimeInterface::CreateNativeFrameWithActualArgs<InvokeHelper::IS_DYNAMIC>(
            frameSize, numActualArgs, this, currentFrame);
    } else {  // NOLINTNEXTLINE(readability-braces-around-statements)
        framePtr = InvokeHelper::CreateFrame(thread, Frame::GetActualSize<InvokeHelper::IS_DYNAMIC>(frameSize), this,
                                             currentFrame, frameSize, numActualArgs);
    }
    PandaUniquePtr<Frame, FrameDeleter> frame(framePtr, FrameDeleter(thread));
    if (UNLIKELY(frame.get() == nullptr)) {
        return frame;
    }

    InvokeHelper::InitActualArgs(frame.get(), argsSpan, numVregs, numDeclaredArgs);

    frame->SetInvoke();
    return frame;
}

template <class InvokeHelper, class ValueT>
ValueT Method::InvokeImpl(ManagedThread *thread, uint32_t numActualArgs, ValueT *args, bool proxyCall)
{
    DecrementHotnessCounter<true>(thread, 0, nullptr);
    if (UNLIKELY(thread->HasPendingException())) {
        return GetReturnValueFromException<InvokeHelper, ValueT>();
    }

    // Currently, proxy methods should always be invoked in the interpreter. This constraint should be relaxed once
    // we support same frame layout for interpreter and compiled methods.
    // NOTE(msherstennikov): remove `proxy_call`
    bool runInterpreter = !HasCompiledCode() || proxyCall;
    ASSERT(!(proxyCall && IsNative()));
    if (!runInterpreter) {
        if constexpr (InvokeHelper::IS_DYNAMIC) {  // NOLINT(readability-braces-around-statements)
            return InvokeHelper::CompiledCodeExecute(thread, this, numActualArgs, args);
        } else {  // NOLINT(readability-misleading-indentation)
            return InvokeCompiledCode(thread, numActualArgs, args);
        }
    }
    if (!thread->template StackOverflowCheck<true, false>()) {
        return GetReturnValueFromException<InvokeHelper, ValueT>();
    }

    return InvokeInterpretedCode<InvokeHelper>(thread, numActualArgs, args);
}

template <class AccVRegisterPtrT>
inline void Method::SetAcc([[maybe_unused]] ManagedThread *thread, [[maybe_unused]] AccVRegisterPtrT acc)
{
    if constexpr (!std::is_same_v<AccVRegisterPtrT, std::nullptr_t>) {  // NOLINT
        if (acc != nullptr) {
            thread->GetCurrentFrame()->SetAcc(*acc);
        }
    }
}

template <class AccVRegisterPtrT>
inline void Method::SetAcc(AccVRegisterPtrT acc)
{
    SetAcc<AccVRegisterPtrT>(ManagedThread::GetCurrent(), acc);
}

/**
 * Decrement method's hotness counter.
 * @param bytecode_offset Offset of the target bytecode instruction. Used only for OSR.
 * @param acc Pointer to the accumulator, it is needed because interpreter uses own Frame, not the one in the method.
 *            Used only for OSR.
 * @param osr Сompilation of the method will be started in OSR mode or not
 * @param func The object of the caller function. Used only for dynamic mode.
 * @return true if OSR has been occurred
 */
template <bool IS_CALL, class AccVRegisterPtrT>
// CC-OFFNXT(G.FUD.06) perf critical
inline bool Method::DecrementHotnessCounter(ManagedThread *thread, uintptr_t bcOffset,
                                            [[maybe_unused]] AccVRegisterPtrT acc, bool osr, TaggedValue func)
{
    ASSERT(thread != nullptr);
    // The compilation process will start performing
    // once the counter value decreases to a value that is or less than 0

    TryCreateSaverTask();

    if (GetHotnessCounter() > 0) {
        if (TryVerify<IS_CALL>()) {
            DecrementHotnessCounter();
        }
        return false;
    }

    if (!Runtime::GetCurrent()->IsProfilerEnabled()) {
        if (TryVerify<IS_CALL>()) {
            ResetHotnessCounter();
        }
        return false;
    }

    // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
    if constexpr (!ArchTraits<RUNTIME_ARCH>::SUPPORT_OSR) {
        ASSERT(!osr);
    }

    auto &options = Runtime::GetOptions();
    uint32_t threshold = options.GetCompilerHotnessThreshold();
    uint32_t profThreshold = options.GetCompilerProfilingThreshold();
    if ((profThreshold < threshold) && !IsProfiling() && !HasCompiledCode() && !IsProfiled()) {
        SetHotnessCounter(threshold - profThreshold - 1);
        if (thread->GetVM()->IsStaticProfileEnabled()) {
            StartProfiling();
        }
        if (!IsProfiling()) {
            SetProfiled();
        }
    } else if (Runtime::GetCurrent()->IsJitEnabled()) {
        CompilationStage status = GetCompilationStatus();
        if (!(status == FAILED || status == WAITING || status == COMPILATION)) {
            ASSERT((!osr) == (acc == nullptr));
            SetAcc<AccVRegisterPtrT>(thread, acc);

            if (func.IsHeapObject()) {
                return DecrementHotnessCounterForTaggedFunction<IS_CALL>(thread, bcOffset, osr, func);
            }
            if (!TryVerify<IS_CALL>()) {
                return false;
            }
            auto compiler = Runtime::GetCurrent()->GetPandaVM()->GetCompiler();
            return compiler->CompileMethod(this, bcOffset, osr, func);  // SUPPRESS_CSA(alpha.core.WasteObjHeader)
        }
        if (status == WAITING) {
            DecrementHotnessCounter();
        }
    }
    return false;
}

template <bool IS_CALL>
inline bool Method::DecrementHotnessCounterForTaggedFunction(ManagedThread *thread, uintptr_t bcOffset, bool osr,
                                                             TaggedValue func)
{
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    VMHandle<ObjectHeader> handleFunc(thread, func.GetHeapObject());
    if (TryVerify<IS_CALL>()) {
        auto compiler = Runtime::GetCurrent()->GetPandaVM()->GetCompiler();
        return compiler->CompileMethod(this, bcOffset, osr, TaggedValue(handleFunc.GetPtr()));
    }

    return false;
}

template <bool IS_CALL>
// CC-OFFNXT(G.FUD.06) perf critical
inline bool Method::TryVerify()
{
    if constexpr (!IS_CALL) {
        return true;
    }
    if (!IsIntrinsic() && (GetVerificationStage() != Method::VerificationStage::VERIFIED_OK)) {
        if (UNLIKELY(!Verify())) {
            auto ctx = Runtime::GetCurrent()->GetLanguageContext(*this);
            ark::ThrowVerificationException(ctx, GetFullName());
            return false;
        }
    }
    return true;
}

template <bool IS_CALL, class AccVRegisterPtrT>
inline bool Method::DecrementHotnessCounter(uintptr_t bytecodeOffset, AccVRegisterPtrT acc, bool osr, TaggedValue func)
{
    return DecrementHotnessCounter<IS_CALL>(ManagedThread::GetCurrent(), bytecodeOffset, acc, osr, func);
}

template <typename Callback>
void Method::EnumerateTypes(Callback handler) const
{
    panda_file::MethodDataAccessor mda(*(pandaFile_), fileId_);
    mda.EnumerateTypesInProto(handler);
}

template <typename Callback>
void Method::EnumerateTryBlocks(Callback callback) const
{
    ASSERT(!IsAbstract());

    panda_file::MethodDataAccessor mda(*(pandaFile_), fileId_);
    panda_file::CodeDataAccessor cda(*(pandaFile_), mda.GetCodeId().value());

    cda.EnumerateTryBlocks(callback);
}

template <typename Callback>
void Method::EnumerateCatchBlocks(Callback callback) const
{
    ASSERT(!IsAbstract());

    using TryBlock = panda_file::CodeDataAccessor::TryBlock;
    using CatchBlock = panda_file::CodeDataAccessor::CatchBlock;

    EnumerateTryBlocks([&callback, code = GetInstructions()](const TryBlock &tryBlock) {
        bool next = true;
        const uint8_t *tryStartPc = reinterpret_cast<uint8_t *>(reinterpret_cast<uintptr_t>(code) +
                                                                static_cast<uintptr_t>(tryBlock.GetStartPc()));
        const uint8_t *tryEndPc = reinterpret_cast<uint8_t *>(reinterpret_cast<uintptr_t>(tryStartPc) +
                                                              static_cast<uintptr_t>(tryBlock.GetLength()));
        // ugly, but API of TryBlock is bad designed: enumaration is paired with mutation & updating
        const_cast<TryBlock &>(tryBlock).EnumerateCatchBlocks(
            [&callback, &next, tryStartPc, tryEndPc](const CatchBlock &catchBlock) {
                return next = callback(tryStartPc, tryEndPc, catchBlock);
            });
        return next;
    });
}

template <typename Callback>
void Method::EnumerateExceptionHandlers(Callback callback) const
{
    ASSERT(!IsAbstract());

    using CatchBlock = panda_file::CodeDataAccessor::CatchBlock;

    EnumerateCatchBlocks([this, callback = std::move(callback)](const uint8_t *tryStartPc, const uint8_t *tryEndPc,
                                                                const CatchBlock &catchBlock) {
        auto typeIdx = catchBlock.GetTypeIdx();
        const uint8_t *pc =
            &GetInstructions()[catchBlock.GetHandlerPc()];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        size_t size = catchBlock.GetCodeSize();
        const Class *cls = nullptr;
        if (typeIdx != panda_file::INVALID_INDEX) {
            Runtime *runtime = Runtime::GetCurrent();
            auto typeId = GetClass()->ResolveClassIndex(typeIdx);
            // NOTE: remove next code, after solving #1220 '[Runtime] Proposal for class descriptors in panda files'
            //       and clean up of ClassLinker API
            // cut
            LanguageContext ctx = runtime->GetLanguageContext(*this);
            cls = runtime->GetClassLinker()->GetExtension(ctx)->GetClass(*(pandaFile_), typeId);
            // end cut
        }
        return callback(tryStartPc, tryEndPc, cls, pc, size);
    });
}

inline bool Method::InitProfilingData(ark::ProfilingData *profilingData)
{
    ProfilingData *oldValue = nullptr;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
    while (!pointer_.profilingData.compare_exchange_weak(oldValue, profilingData)) {
        if (oldValue != nullptr) {
            // We're late, some thread already started profiling.
            return false;
        }
    }
    return true;
}

}  // namespace ark

#endif  // !PANDA_RUNTIME_METHOD_INL_H_
