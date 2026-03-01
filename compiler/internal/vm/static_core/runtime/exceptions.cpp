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

#include "runtime/include/exceptions.h"

#include <libarkbase/utils/cframe_layout.h>

#include "runtime/bridge/bridge.h"
#include "runtime/deoptimization.h"
#include "runtime/entrypoints/entrypoints.h"
#include "runtime/include/coretypes/line_string.h"
#include "runtime/include/object_header-inl.h"
#include "runtime/include/runtime.h"
#include "runtime/include/stack_walker.h"
#include "runtime/mem/vm_handle.h"
#include "libarkbase/utils/asan_interface.h"
#include "libarkbase/utils/logger.h"
#include "libarkbase/utils/utf.h"
#include "libarkfile/class_data_accessor-inl.h"
#include "libarkfile/method_data_accessor-inl.h"
#include "libarkbase/events/events.h"
#include "runtime/handle_base-inl.h"

namespace ark {

void ThrowException(const LanguageContext &ctx, ManagedThread *thread, const uint8_t *mutf8Name,
                    const uint8_t *mutf8Msg)
{
    ctx.ThrowException(thread, mutf8Name, mutf8Msg);
}

static LanguageContext GetLanguageContext(ManagedThread *thread)
{
    ASSERT(thread != nullptr);
    return thread->GetVM()->GetLanguageContext();
}

void ThrowNullPointerException()
{
    auto *thread = ManagedThread::GetCurrent();
    auto ctx = GetLanguageContext(thread);
    ThrowNullPointerException(ctx, thread);
}

void ThrowNullPointerException(const LanguageContext &ctx, ManagedThread *thread)
{
    ThrowException(ctx, thread, ctx.GetNullPointerExceptionClassDescriptor(), nullptr);
    SetExceptionEvent(events::ExceptionType::NULL_CHECK, thread);
}

void ThrowStackOverflowException(ManagedThread *thread)
{
    auto ctx = GetLanguageContext(thread);
    ctx.ThrowStackOverflowException(thread);
}

void ThrowArrayIndexOutOfBoundsException(coretypes::ArraySsizeT idx, coretypes::ArraySizeT length)
{
    auto *thread = ManagedThread::GetCurrent();
    auto ctx = GetLanguageContext(thread);
    ThrowArrayIndexOutOfBoundsException(idx, length, ctx, thread);
}

void ThrowArrayIndexOutOfBoundsException(coretypes::ArraySsizeT idx, coretypes::ArraySizeT length,
                                         const LanguageContext &ctx, ManagedThread *thread)
{
    PandaString msg {"idx = " + ToPandaString(idx) + "; length = " + ToPandaString(length)};
    ThrowException(ctx, thread, ctx.GetArrayIndexOutOfBoundsExceptionClassDescriptor(),
                   utf::CStringAsMutf8(msg.c_str()));
    SetExceptionEvent(events::ExceptionType::BOUND_CHECK, thread);
}

void ThrowIndexOutOfBoundsException(coretypes::ArraySsizeT idx, coretypes::ArraySsizeT length)
{
    auto *thread = ManagedThread::GetCurrent();
    auto ctx = GetLanguageContext(thread);
    PandaString msg {"idx = " + ToPandaString(idx) + "; length = " + ToPandaString(length)};
    ThrowException(ctx, thread, ctx.GetIndexOutOfBoundsExceptionClassDescriptor(), utf::CStringAsMutf8(msg.c_str()));
}

void ThrowIllegalStateException(const PandaString &msg)
{
    auto *thread = ManagedThread::GetCurrent();
    auto ctx = GetLanguageContext(thread);
    ThrowException(ctx, thread, ctx.GetIllegalStateExceptionClassDescriptor(), utf::CStringAsMutf8(msg.c_str()));
}

void ThrowStringIndexOutOfBoundsException(coretypes::ArraySsizeT idx, coretypes::ArraySizeT length)
{
    auto *thread = ManagedThread::GetCurrent();
    auto ctx = GetLanguageContext(thread);
    PandaString msg {"idx = " + ToPandaString(idx) + "; length = " + ToPandaString(length)};
    ThrowException(ctx, thread, ctx.GetStringIndexOutOfBoundsExceptionClassDescriptor(),
                   utf::CStringAsMutf8(msg.c_str()));
    SetExceptionEvent(events::ExceptionType::BOUND_CHECK, thread);
}

void ThrowNegativeArraySizeException(coretypes::ArraySsizeT size)
{
    auto *thread = ManagedThread::GetCurrent();
    auto ctx = GetLanguageContext(thread);
    PandaString msg {"size = " + ToPandaString(size)};
    ThrowException(ctx, thread, ctx.GetNegativeArraySizeExceptionClassDescriptor(), utf::CStringAsMutf8(msg.c_str()));
}

void ThrowNegativeArraySizeException(const PandaString &msg)
{
    auto *thread = ManagedThread::GetCurrent();
    auto ctx = GetLanguageContext(thread);
    ThrowException(ctx, thread, ctx.GetNegativeArraySizeExceptionClassDescriptor(), utf::CStringAsMutf8(msg.c_str()));
    SetExceptionEvent(events::ExceptionType::NEGATIVE_SIZE, thread);
}

void ThrowArithmeticException()
{
    auto *thread = ManagedThread::GetCurrent();
    auto ctx = GetLanguageContext(thread);
    ThrowException(ctx, thread, ctx.GetArithmeticExceptionClassDescriptor(), utf::CStringAsMutf8("/ by zero"));
    SetExceptionEvent(events::ExceptionType::ARITHMETIC, thread);
}

void ThrowClassCastException(const Class *dstType, const Class *srcType)
{
    auto *thread = ManagedThread::GetCurrent();
    auto ctx = GetLanguageContext(thread);
    PandaString msg {srcType->GetName() + " cannot be cast to " + dstType->GetName()};
    ThrowException(ctx, thread, ctx.GetClassCastExceptionClassDescriptor(), utf::CStringAsMutf8(msg.c_str()));
    SetExceptionEvent(events::ExceptionType::CAST_CHECK, thread);
}

void ThrowAbstractMethodError(const Method *method)
{
    auto *thread = ManagedThread::GetCurrent();
    auto ctx = GetLanguageContext(thread);
    PandaString msg {"abstract method \"" + method->GetClass()->GetName() + "."};
    msg += utf::Mutf8AsCString(method->GetName().data);
    msg += "\"";
    ThrowException(ctx, thread, ctx.GetAbstractMethodErrorClassDescriptor(), utf::CStringAsMutf8(msg.c_str()));
    SetExceptionEvent(events::ExceptionType::ABSTRACT_METHOD, thread);
}

void ThrowIncompatibleClassChangeErrorForMethodConflict(const Method *method)
{
    auto *thread = ManagedThread::GetCurrent();
    auto ctx = GetLanguageContext(thread);
    PandaString msg {"Conflicting default method implementations \"" + method->GetClass()->GetName() + "."};
    msg += utf::Mutf8AsCString(method->GetName().data);
    msg += "\"";
    ThrowException(ctx, thread, ctx.GetIncompatibleClassChangeErrorDescriptor(), utf::CStringAsMutf8(msg.c_str()));
    SetExceptionEvent(events::ExceptionType::ICCE_METHOD_CONFLICT, thread);
}

void ThrowArrayStoreException(const Class *arrayClass, const Class *elementClass)
{
    PandaStringStream ss;
    ss << elementClass->GetName() << " cannot be stored in an array of type " << arrayClass->GetName();
    ThrowArrayStoreException(ss.str());
}

void ThrowArrayStoreException(const PandaString &msg)
{
    auto *thread = ManagedThread::GetCurrent();
    auto ctx = GetLanguageContext(thread);

    ThrowException(ctx, thread, ctx.GetArrayStoreExceptionClassDescriptor(), utf::CStringAsMutf8(msg.c_str()));
}

void ThrowRuntimeException(const PandaString &msg)
{
    auto *thread = ManagedThread::GetCurrent();
    auto ctx = GetLanguageContext(thread);

    ThrowException(ctx, thread, ctx.GetRuntimeExceptionClassDescriptor(), utf::CStringAsMutf8(msg.c_str()));
}

void ThrowIllegalArgumentException(const PandaString &msg)
{
    auto *thread = ManagedThread::GetCurrent();
    auto ctx = GetLanguageContext(thread);

    ThrowException(ctx, thread, ctx.GetIllegalArgumentExceptionClassDescriptor(), utf::CStringAsMutf8(msg.c_str()));
}

void ThrowClassCircularityError(const PandaString &className, const LanguageContext &ctx)
{
    auto *thread = ManagedThread::GetCurrent();
    PandaString msg = "Class or interface \"" + className + "\" is its own superclass or superinterface";
    ThrowException(ctx, thread, ctx.GetClassCircularityErrorDescriptor(), utf::CStringAsMutf8(msg.c_str()));
}

void ThrowCoroutinesLimitExceedError(const PandaString &msg)
{
    auto *thread = ManagedThread::GetCurrent();
    auto ctx = GetLanguageContext(thread);

    ThrowException(ctx, thread, ctx.GetCoroutinesLimitExceedErrorDescriptor(), utf::CStringAsMutf8(msg.c_str()));
}

NO_ADDRESS_SANITIZE void DropCFrameIfNecessary(ManagedThread *thread, StackWalker *stack, Frame *origFrame,
                                               FrameAccessor nextFrame, Method *method)
{
    if (!nextFrame.IsCFrame()) {
        if (origFrame != nullptr) {
            FreeFrame(origFrame);
        }
        thread->SetCurrentFrame(nextFrame.GetIFrame());
        LOG(DEBUG, INTEROP) << "DropCompiledFrameAndReturn. Next frame isn't CFrame";
        DropCompiledFrame(stack);
        UNREACHABLE();
    }

    if (nextFrame.GetCFrame().IsNative()) {
        if (origFrame != nullptr) {
            FreeFrame(origFrame);
        }
        LOG(DEBUG, INTEROP) << "DropCompiledFrameAndReturn. Next frame is NATIVE";
        DropCompiledFrame(stack);
        UNREACHABLE();
    }

    if (method->IsStaticConstructor()) {
        if (origFrame != nullptr) {
            FreeFrame(origFrame);
        }
        LOG(DEBUG, INTEROP) << "DropCompiledFrameAndReturn. Next frame is StaticConstructor";
        DropCompiledFrame(stack);
        UNREACHABLE();
    }
}

/**
 * The function finds the corresponding catch block for the exception in the thread.
 * The function uses thread as an exception storage because:
 *  1. thread's exception is a GC root
 *  2. we cannot use HandleScope here because the function is [[noreturn]]
 */
// NOLINTNEXTLINE(google-runtime-references)
NO_ADDRESS_SANITIZE void FindCatchBlockInCFrames(ManagedThread *thread, StackWalker *stack, Frame *origFrame)
{
    auto nextFrame = stack->GetNextFrame();
    for (; stack->HasFrame(); stack->NextFrame(), nextFrame = stack->GetNextFrame()) {
        LOG(DEBUG, INTEROP) << "Search for the catch block in " << stack->GetMethod()->GetFullName();

        auto pc = stack->GetBytecodePc();
        auto *method = stack->GetMethod();
        ASSERT(method != nullptr);
        uint32_t pcOffset = method->FindCatchBlock(thread->GetException()->ClassAddr<Class>(), pc);
        if (pcOffset != panda_file::INVALID_OFFSET) {
            if (origFrame != nullptr) {
                FreeFrame(origFrame);
            }

            LOG(DEBUG, INTEROP) << "Catch block is found in " << stack->GetMethod()->GetFullName();
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            Deoptimize(stack, method->GetInstructions() + pcOffset);
            UNREACHABLE();
        }

        thread->GetVM()->HandleReturnFrame();

        DropCFrameIfNecessary(thread, stack, origFrame, nextFrame, method);

        if (stack->IsInlined()) {
            continue;
        }

        auto prev = stack->GetCFrame().GetPrevFrame();
        if (stack->GetBoundaryFrameMethod<FrameKind::COMPILER>(prev) == FrameBridgeKind::BYPASS) {
            /**
             * There is bypass bridge and current frame is not inlined, hence we are going to exit compiled
             * function. Dynamic languages may do c2c call through runtime, so it's necessary to return to exit
             * active function properly.
             */
            if (origFrame != nullptr) {
                FreeFrame(origFrame);
            }
            LOG(DEBUG, INTEROP) << "DropCompiledFrameAndReturn. Next frame is caller's cframe";
            DropCompiledFrame(stack);
            UNREACHABLE();
        }
    }

    if (nextFrame.IsValid()) {
        LOG(DEBUG, INTEROP) << "Exception " << thread->GetException()->ClassAddr<Class>()->GetName() << " isn't found";
        EVENT_METHOD_EXIT(stack->GetMethod()->GetFullName(), events::MethodExitKind::COMPILED,
                          thread->RecordMethodExit());
        thread->GetVM()->HandleReturnFrame();
        DropCompiledFrame(stack);
    }
    UNREACHABLE();
}

/**
 * The function finds the corresponding catch block for the exception in the thread.
 * The function uses thread as an exception storage because:
 *  1. thread's exception is a GC root
 *  2. we cannot use Handl;eScope her ebecause the function is [[noreturn]]
 */
NO_ADDRESS_SANITIZE void FindCatchBlockInCallStack(ManagedThread *thread)
{
    auto stack = StackWalker::Create(thread);
    auto origFrame = stack.GetIFrame();
    ASSERT(!stack.IsCFrame());
    LOG(DEBUG, INTEROP) << "Enter in FindCatchBlockInCallStack for " << origFrame->GetMethod()->GetFullName();
    // Exception will be handled in the Method::Invoke's caller
    if (origFrame->IsInvoke()) {
        return;
    }

    stack.NextFrame();

    // NATIVE frames can handle exceptions as well
    if (!stack.HasFrame() || !stack.IsCFrame() || stack.GetCFrame().IsNative()) {
        return;
    }
    thread->GetVM()->HandleReturnFrame();
    FindCatchBlockInCFrames(thread, &stack, origFrame);
}

void ThrowFileNotFoundException(const PandaString &msg)
{
    auto *thread = ManagedThread::GetCurrent();
    auto ctx = GetLanguageContext(thread);

    ThrowException(ctx, thread, ctx.GetFileNotFoundExceptionClassDescriptor(), utf::CStringAsMutf8(msg.c_str()));
}

void ThrowIOException(const PandaString &msg)
{
    auto *thread = ManagedThread::GetCurrent();
    auto ctx = GetLanguageContext(thread);

    ThrowException(ctx, thread, ctx.GetIOExceptionClassDescriptor(), utf::CStringAsMutf8(msg.c_str()));
}

void ThrowIllegalAccessException(const PandaString &msg)
{
    auto *thread = ManagedThread::GetCurrent();
    auto ctx = GetLanguageContext(thread);

    ThrowException(ctx, thread, ctx.GetIllegalAccessExceptionClassDescriptor(), utf::CStringAsMutf8(msg.c_str()));
}

void ThrowOutOfMemoryError(ManagedThread *thread, const PandaString &msg)
{
    auto ctx = GetLanguageContext(thread);

    if (thread->IsThrowingOOM()) {
        // In case of OOM try to allocate exception object first,
        // because allocator still may have some space that will be enough for allocating OOM exception.
        // If during allocation allocator throws OOM once again, we use preallocate object without collecting stack
        // trace.
        thread->SetUsePreAllocObj(true);
    }

    thread->SetThrowingOOM(true);
    ThrowException(ctx, thread, ctx.GetOutOfMemoryErrorClassDescriptor(), utf::CStringAsMutf8(msg.c_str()));
    thread->SetThrowingOOM(false);
    SetExceptionEvent(events::ExceptionType::NATIVE, thread);
}

void ThrowOutOfMemoryError(const PandaString &msg)
{
    auto *thread = ManagedThread::GetCurrent();
    ThrowOutOfMemoryError(thread, msg);
}

void ThrowUnsupportedOperationException()
{
    auto *thread = ManagedThread::GetCurrent();
    auto ctx = GetLanguageContext(thread);
    ThrowException(ctx, thread, ctx.GetUnsupportedOperationExceptionClassDescriptor(), nullptr);
}

void ThrowVerificationException(const PandaString &msg)
{
    auto *thread = ManagedThread::GetCurrent();
    auto ctx = GetLanguageContext(thread);

    ThrowException(ctx, thread, ctx.GetVerifyErrorClassDescriptor(), utf::CStringAsMutf8(msg.c_str()));
}

void ThrowVerificationException(const LanguageContext &ctx, const PandaString &msg)
{
    auto *thread = ManagedThread::GetCurrent();

    ThrowException(ctx, thread, ctx.GetVerifyErrorClassDescriptor(), utf::CStringAsMutf8(msg.c_str()));
}

void ThrowInstantiationError(const PandaString &msg)
{
    auto *thread = ManagedThread::GetCurrent();
    auto ctx = GetLanguageContext(thread);

    ThrowException(ctx, thread, ctx.GetInstantiationErrorDescriptor(), utf::CStringAsMutf8(msg.c_str()));
    SetExceptionEvent(events::ExceptionType::INSTANTIATION_ERROR, thread);
}

void ThrowNoClassDefFoundError(const PandaString &msg)
{
    auto *thread = ManagedThread::GetCurrent();
    auto ctx = GetLanguageContext(thread);

    ThrowException(ctx, thread, ctx.GetNoClassDefFoundErrorDescriptor(), utf::CStringAsMutf8(msg.c_str()));
}

void ThrowTypedErrorDyn(const std::string &msg)
{
    auto *thread = ManagedThread::GetCurrent();
    auto ctx = GetLanguageContext(thread);
    ThrowException(ctx, thread, ctx.GetTypedErrorDescriptor(), utf::CStringAsMutf8(msg.c_str()));
}

void ThrowReferenceErrorDyn(const std::string &msg)
{
    auto *thread = ManagedThread::GetCurrent();
    auto ctx = GetLanguageContext(thread);
    ThrowException(ctx, thread, ctx.GetReferenceErrorDescriptor(), utf::CStringAsMutf8(msg.c_str()));
}

void ThrowIllegalMonitorStateException(const PandaString &msg)
{
    auto *thread = ManagedThread::GetCurrent();
    auto ctx = GetLanguageContext(thread);

    ThrowException(ctx, thread, ctx.GetIllegalMonitorStateExceptionDescriptor(), utf::CStringAsMutf8(msg.c_str()));
}

void ThrowCloneNotSupportedException()
{
    auto *thread = ManagedThread::GetCurrent();
    auto ctx = GetLanguageContext(thread);
    PandaString msg = "Class doesn't implement Cloneable";
    ThrowException(ctx, thread, ctx.GetCloneNotSupportedExceptionDescriptor(), utf::CStringAsMutf8(msg.c_str()));
}

void HandlePendingException(UnwindPolicy policy)
{
    auto *thread = ManagedThread::GetCurrent();
    ASSERT(thread != nullptr);
    ASSERT(thread->HasPendingException());
    LOG(DEBUG, INTEROP) << "HandlePendingException";

    // NOTE(konstanting): a potential candidate for moving out of the core part
    thread->GetVM()->CleanupCompiledFrameResources(thread->GetCurrentFrame());

    auto stack = StackWalker::Create(thread, policy);
    ASSERT(stack.IsCFrame());

    FindCatchBlockInCFrames(thread, &stack, nullptr);
}

}  // namespace ark
