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
#ifndef PANDA_INTERPRETER_RUNTIME_INTERFACE_H_
#define PANDA_INTERPRETER_RUNTIME_INTERFACE_H_

#include <memory>

#include "libarkbase/utils/logger.h"
#include "libarkfile/file_items.h"
#include "runtime/entrypoints/entrypoints.h"
#include "runtime/include/class_linker-inl.h"
#include "runtime/include/coretypes/array.h"
#include "runtime/include/coretypes/line_string.h"
#include "runtime/include/exceptions.h"
#include "runtime/include/field.h"
#include "runtime/include/method.h"
#include "runtime/include/runtime.h"
#include "runtime/include/managed_thread.h"
#include "runtime/include/safepoint_timer.h"
#include "runtime/mem/gc/gc.h"

namespace ark::interpreter {

class RuntimeInterface {
public:
    static constexpr bool NEED_READ_BARRIER = true;
    static constexpr bool NEED_WRITE_BARRIER = true;

    static Method *ResolveMethod(ManagedThread *thread, const Method &caller, BytecodeId id)
    {
        auto resolvedId = caller.GetClass()->ResolveMethodIndex(id.AsIndex());
        auto *classLinker = Runtime::GetCurrent()->GetClassLinker();
        auto *method = classLinker->GetMethod(caller, resolvedId);
        if (method == nullptr) {
            return nullptr;
        }

        auto *klass = method->GetClass();
        if (!klass->IsInitialized() && !classLinker->InitializeClass(thread, klass)) {
            return nullptr;
        }

        return method;
    }

    static const uint8_t *GetMethodName(const Method *caller, BytecodeId methodId)
    {
        auto resolvedId = caller->GetClass()->ResolveMethodIndex(methodId.AsIndex());
        const auto *pf = caller->GetPandaFile();
        const panda_file::MethodDataAccessor mda(*pf, resolvedId);
        return pf->GetStringData(mda.GetNameId()).data;
    }

    static Class *GetMethodClass(const Method *caller, BytecodeId methodId)
    {
        auto resolvedId = caller->GetClass()->ResolveMethodIndex(methodId.AsIndex());
        const auto *pf = caller->GetPandaFile();
        const panda_file::MethodDataAccessor mda(*pf, resolvedId);
        auto classId = mda.GetClassId();

        auto *classLinker = Runtime::GetCurrent()->GetClassLinker();
        return classLinker->GetClass(*caller, classId);
    }

    static uint32_t GetMethodArgumentsCount(Method *caller, BytecodeId methodId)
    {
        auto resolvedId = caller->GetClass()->ResolveMethodIndex(methodId.AsIndex());
        auto *pf = caller->GetPandaFile();
        panda_file::MethodDataAccessor mda(*pf, resolvedId);
        panda_file::ProtoDataAccessor pda(*pf, mda.GetProtoId());
        return pda.GetNumArgs();
    }

    static Field *ResolveField(ManagedThread *thread, const Method &caller, BytecodeId id, bool isStatic)
    {
        auto resolvedId = caller.GetClass()->ResolveFieldIndex(id.AsIndex());
        auto *classLinker = Runtime::GetCurrent()->GetClassLinker();
        auto *field = classLinker->GetField(caller, resolvedId, isStatic);
        if (field == nullptr) {
            return nullptr;
        }

        auto *klass = field->GetClass();
        if (!klass->IsInitialized() && !classLinker->InitializeClass(thread, field->GetClass())) {
            return nullptr;
        }

        return field;
    }

    template <bool NEED_INIT>
    static Class *ResolveClass(ManagedThread *thread, const Method &caller, BytecodeId id)
    {
        auto resolvedId = caller.GetClass()->ResolveClassIndex(id.AsIndex());
        ClassLinker *classLinker = Runtime::GetCurrent()->GetClassLinker();
        Class *klass = classLinker->GetClass(caller, resolvedId);

        if (klass == nullptr) {
            return nullptr;
        }

        if (NEED_INIT) {
            auto *klassLinker = Runtime::GetCurrent()->GetClassLinker();
            if (!klass->IsInitialized() && !klassLinker->InitializeClass(thread, klass)) {
                return nullptr;
            }
        }

        return klass;
    }

    static coretypes::Array *ResolveLiteralArray(PandaVM *vm, const Method &caller, BytecodeId id)
    {
        return Runtime::GetCurrent()->ResolveLiteralArray(vm, caller, id.AsIndex());
    }

    static uint32_t GetCompilerHotnessThreshold()
    {
        return Runtime::GetOptions().GetCompilerHotnessThreshold();
    }

    static bool IsCompilerEnableJit()
    {
        return Runtime::GetOptions().IsCompilerEnableJit();
    }

    static void SetCurrentFrame(ManagedThread *thread, Frame *frame)
    {
        thread->SetCurrentFrame(frame);
    }

    static RuntimeNotificationManager *GetNotificationManager()
    {
        return Runtime::GetCurrent()->GetNotificationManager();
    }

    static coretypes::Array *CreateArray(Class *klass, coretypes::ArraySizeT length)
    {
        return coretypes::Array::Create(klass, length);
    }

    static ObjectHeader *CreateObject(ManagedThread *thread, Class *klass)
    {
        ASSERT(!klass->IsArrayClass());

        if (klass->IsStringClass()) {
            LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(*klass);
            return coretypes::String::CreateEmptyString(ctx, Runtime::GetCurrent()->GetPandaVM());
        }

        if (LIKELY(klass->IsInstantiable())) {
            mem::HeapManager *heapManager = thread->GetVM()->GetHeapManager();
            auto *currentTlab = thread->GetTLAB();
            size_t size = klass->GetObjectSize();
            if (currentTlab->GetFreeSize() < size || heapManager->IsObjectFinalized(klass)) {
                // Slow path
                return ObjectHeader::Create(thread, klass);
            }
            auto *mem = currentTlab->Alloc(size);
            ASSERT(mem != nullptr);
            ObjectHeader *object = heapManager->InitObjectHeaderAtMem(klass, mem);
            object = heapManager->RegisterFinalizableIfNeeded(object, klass, size, thread);
            return object;
        }

        const auto &name = klass->GetName();
        PandaString pname(name.cbegin(), name.cend());
        ark::ThrowInstantiationError(pname);
        return nullptr;
    }

    static Value InvokeMethod(ManagedThread *thread, Method *method, Value *args)
    {
        return method->Invoke(thread, args);
    }

    static uint32_t FindCatchBlock(const Method &method, ObjectHeader *exception, uint32_t pc)
    {
        return method.FindCatchBlock(exception->ClassAddr<Class>(), pc);
    }

    static void ThrowNullPointerException()
    {
        return ark::ThrowNullPointerException();
    }

    static void ThrowArrayIndexOutOfBoundsException(coretypes::ArraySsizeT idx, coretypes::ArraySizeT length)
    {
        ark::ThrowArrayIndexOutOfBoundsException(idx, length);
    }

    static void ThrowNegativeArraySizeException(coretypes::ArraySsizeT size)
    {
        ark::ThrowNegativeArraySizeException(size);
    }

    static void ThrowArithmeticException()
    {
        ark::ThrowArithmeticException();
    }

    static void ThrowClassCastException(Class *dstType, Class *srcType)
    {
        ark::ThrowClassCastException(dstType, srcType);
    }

    static void ThrowAbstractMethodError(Method *method)
    {
        ark::ThrowAbstractMethodError(method);
    }

    static void ThrowIncompatibleClassChangeErrorForMethodConflict(Method *method)
    {
        ark::ThrowIncompatibleClassChangeErrorForMethodConflict(method);
    }

    static void ThrowOutOfMemoryError(const PandaString &msg)
    {
        ark::ThrowOutOfMemoryError(msg);
    }

    static void ThrowArrayStoreException(Class *arrayClass, Class *elemClass)
    {
        ark::ThrowArrayStoreException(arrayClass, elemClass);
    }

    static void ThrowIllegalAccessException(const PandaString &msg)
    {
        ark::ThrowIllegalAccessException(msg);
    }

    static void ThrowVerificationException(const PandaString &msg)
    {
        return ark::ThrowVerificationException(msg);
    }

    static void ThrowTypedErrorDyn(const std::string &msg)
    {
        ark::ThrowTypedErrorDyn(msg);
    }

    static void ThrowReferenceErrorDyn(const std::string &msg)
    {
        ark::ThrowReferenceErrorDyn(msg);
    }

    template <bool IS_DYNAMIC = false>
    static Frame *CreateFrame(uint32_t nregs, Method *method, Frame *prev)
    {
        return ark::CreateFrameWithSize(Frame::GetActualSize<IS_DYNAMIC>(nregs), nregs, method, prev);
    }

    template <bool IS_DYNAMIC = false>
    static Frame *CreateFrameWithActualArgs(uint32_t nregs, uint32_t numActualArgs, Method *method, Frame *prev)
    {
        return ark::CreateFrameWithActualArgsAndSize(Frame::GetActualSize<IS_DYNAMIC>(nregs), nregs, numActualArgs,
                                                     method, prev);
    }

    ALWAYS_INLINE static Frame *CreateFrameWithActualArgsAndSize(uint32_t size, uint32_t nregs, uint32_t numActualArgs,
                                                                 Method *method, Frame *prev)
    {
        return ark::CreateFrameWithActualArgsAndSize(size, nregs, numActualArgs, method, prev);
    }

    template <bool IS_DYNAMIC = false>
    static Frame *CreateNativeFrameWithActualArgs(uint32_t nregs, uint32_t numActualArgs, Method *method, Frame *prev)
    {
        return ark::CreateNativeFrameWithActualArgsAndSize(Frame::GetActualSize<IS_DYNAMIC>(nregs), nregs,
                                                           numActualArgs, method, prev);
    }

    ALWAYS_INLINE static void FreeFrame(ManagedThread *thread, Frame *frame)
    {
        thread->GetStackFrameAllocator()->Free(frame->GetExt());
    }

    static void ThreadSuspension(ManagedThread *thread)
    {
        thread->WaitSuspension();
    }

    static void ThreadRuntimeTermination(ManagedThread *thread)
    {
        thread->OnRuntimeTerminated();
    }

    static panda_file::SourceLang GetLanguageContext(Method *methodPtr)
    {
        LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(*methodPtr);
        return ctx.GetLanguage();
    }

    /**
     * @brief Executes external implementation of safepoint
     *
     * It is not-inlined version of safepoint.
     * Shouldn't be used in production in the JIT.
     */
    static void Safepoint(ManagedThread *thread)
    {
        SAFEPOINT_TIME_CHECKER(SafepointTimerTable::ResetTimers(thread->GetInternalId(), true));
#ifndef NDEBUG
        // NOTE(sarychevkonstantin, #I9624): achieve consistency between mutator lock ownership and IsManaged method
        if (Runtime::GetOptions().IsRunGcEverySafepoint() && Thread::GetCurrent()->GetMutatorLock()->HasLock()) {
            auto *vm = ManagedThread::GetCurrent()->GetVM();
            vm->GetGCTrigger()->TriggerGcIfNeeded(vm->GetGC());
        }
#endif
        ASSERT(thread != nullptr);
        ASSERT(thread->IsRuntimeCallEnabled());
        if (UNLIKELY(thread->IsRuntimeTerminated())) {
            ThreadRuntimeTermination(thread);
        }
        if (thread->IsSuspended()) {
            ThreadSuspension(thread);
        }
        SAFEPOINT_TIME_CHECKER(SafepointTimerTable::ResetTimers(thread->GetInternalId(), false));
    }

    static void Safepoint()
    {
        Safepoint(ManagedThread::GetCurrent());
    }

    static LanguageContext GetLanguageContext(const Method &method)
    {
        return Runtime::GetCurrent()->GetLanguageContext(method);
    }
};

}  // namespace ark::interpreter

#endif  // PANDA_INTERPRETER_RUNTIME_INTERFACE_H_
