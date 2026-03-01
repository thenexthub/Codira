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
#ifndef PANDA_RUNTIME_TESTS_INTERPRETER_TEST_RUNTIME_INTERFACE_H_
#define PANDA_RUNTIME_TESTS_INTERPRETER_TEST_RUNTIME_INTERFACE_H_

#include <gtest/gtest.h>

#include <cstdint>

#include "libarkfile/file.h"
#include "libarkfile/file_items.h"
#include "runtime/include/coretypes/array-inl.h"
#include "runtime/include/coretypes/line_string.h"
#include "runtime/include/method.h"
#include "runtime/include/runtime.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/runtime_notification.h"
#include "runtime/interpreter/frame.h"
#include "runtime/mem/gc/gc.h"

namespace ark::interpreter::test {

class DummyGC : public ark::mem::GC {
public:
    NO_COPY_SEMANTIC(DummyGC);
    NO_MOVE_SEMANTIC(DummyGC);

    explicit DummyGC(ark::mem::ObjectAllocatorBase *objectAllocator, const ark::mem::GCSettings &settings);
    ~DummyGC() override = default;
    // NOLINTNEXTLINE(misc-unused-parameters)
    bool WaitForGC([[maybe_unused]] GCTask task) override
    {
        return false;
    }
    void InitGCBits([[maybe_unused]] ark::ObjectHeader *objHeader) override {}
    void InitGCBitsForAllocationInTLAB([[maybe_unused]] ark::ObjectHeader *objHeader) override {}
    bool Trigger([[maybe_unused]] PandaUniquePtr<GCTask> task) override
    {
        return false;
    }

    bool IsPinningSupported() const override
    {
        return true;
    }

    bool IsPostponeGCSupported() const override
    {
        return true;
    }

private:
    size_t VerifyHeap() override
    {
        return 0;
    }
    void InitializeImpl() override {}

    void PreRunPhasesImpl() override {}
    void RunPhasesImpl([[maybe_unused]] GCTask &task) override {}
    bool IsMarked([[maybe_unused]] const ObjectHeader *object) const override
    {
        return false;
    }
    void MarkObject([[maybe_unused]] ObjectHeader *object) override {}
    bool MarkObjectIfNotMarked([[maybe_unused]] ObjectHeader *object) override
    {
        return false;
    }
    void MarkReferences([[maybe_unused]] mem::GCMarkingStackType *references,
                        [[maybe_unused]] ark::mem::GCPhase gcPhase) override
    {
    }
    void VisitRoots([[maybe_unused]] const GCRootVisitor &gcRootVisitor,
                    [[maybe_unused]] mem::VisitGCRootFlags flags) override
    {
    }
    void VisitClassRoots([[maybe_unused]] const GCRootVisitor &gcRootVisitor) override {}
    void VisitCardTableRoots([[maybe_unused]] mem::CardTable *cardTable,
                             [[maybe_unused]] const GCRootVisitor &gcRootVisitor,
                             [[maybe_unused]] const MemRangeChecker &rangeChecker,
                             [[maybe_unused]] const ObjectChecker &rangeObjectChecker,
                             [[maybe_unused]] const ObjectChecker &fromObjectChecker,
                             [[maybe_unused]] const uint32_t processedFlag) override
    {
    }
    void CommonUpdateAndSweepVmRefs() override {}
    void UpdateRefsToMovedObjectsInPygoteSpace() override {}
    void ClearLocalInternalAllocatorPools() override {}
};

template <class T>
static T *ToPointer(size_t value)
{
    return reinterpret_cast<T *>(AlignUp(value, alignof(T)));
}

class RuntimeInterface {
public:
    static constexpr bool NEED_READ_BARRIER = false;
    static constexpr bool NEED_WRITE_BARRIER = false;

    using InvokeMethodHandler = std::function<Value(ManagedThread *, Method *, Value *)>;

    struct NullPointerExceptionData {
        bool expected {false};
    };

    struct ArithmeticException {
        bool expected {false};
    };

    struct ArrayIndexOutOfBoundsExceptionData {
        bool expected {false};
        coretypes::ArraySsizeT idx {};
        coretypes::ArraySizeT length {};
    };

    struct NegativeArraySizeExceptionData {
        bool expected {false};
        coretypes::ArraySsizeT size {};
    };

    struct ClassCastExceptionData {
        bool expected {false};
        Class *dstType {};
        Class *srcType {};
    };

    struct AbstractMethodError {
        bool expected {false};
        Method *method {};
    };

    struct ArrayStoreExceptionData {
        bool expected {false};
        Class *arrayClass {};
        Class *elemClass {};
    };

    static constexpr BytecodeId METHOD_ID {0xaabb};
    static constexpr BytecodeId FIELD_ID {0xeeff};
    static constexpr BytecodeId TYPE_ID {0x5566};
    static constexpr BytecodeId LITERALARRAY_ID {0x7788};

    static coretypes::Array *ResolveLiteralArray([[maybe_unused]] PandaVM *vm, [[maybe_unused]] const Method &caller,
                                                 BytecodeId id)
    {
        EXPECT_EQ(id, LITERALARRAY_ID);
        // NOLINTNEXTLINE(readability-magic-numbers)
        return ToPointer<coretypes::Array>(0x7788);
    }

    static Method *ResolveMethod([[maybe_unused]] ManagedThread *thread, [[maybe_unused]] const Method &caller,
                                 BytecodeId id)
    {
        EXPECT_EQ(id, METHOD_ID);
        return resolvedMethod_;
    }

    static Field *ResolveField([[maybe_unused]] ManagedThread *thread, [[maybe_unused]] const Method &caller,
                               BytecodeId id, [[maybe_unused]] bool isStatic)
    {
        EXPECT_EQ(id, FIELD_ID);
        return resolvedField_;
    }

    template <bool NEED_INIT>
    static Class *ResolveClass([[maybe_unused]] ManagedThread *thread, [[maybe_unused]] const Method &caller,
                               BytecodeId id)
    {
        EXPECT_EQ(id, TYPE_ID);
        return resolvedClass_;
    }

    static uint32_t FindCatchBlock([[maybe_unused]] const Method &method, [[maybe_unused]] ObjectHeader *exception,
                                   [[maybe_unused]] uint32_t pc)
    {
        return catchBlockPcOffset_;
    }

    static void SetCatchBlockPcOffset(uint32_t pcOffset)
    {
        catchBlockPcOffset_ = pcOffset;
    }

    static uint32_t GetCompilerHotnessThreshold()
    {
        return jitThreshold_;
    }

    static bool IsCompilerEnableJit()
    {
        return true;
    }

    static void SetCompilerHotnessThreshold(uint32_t threshold)
    {
        jitThreshold_ = threshold;
    }

    static void JITCompileMethod(Method *method)
    {
        method->SetCompiledEntryPoint(entryPoint_);
    }

    static void SetCurrentFrame([[maybe_unused]] ManagedThread *thread, Frame *frame)
    {
        ASSERT_NE(frame, nullptr);
    }

    static RuntimeNotificationManager *GetNotificationManager()
    {
        return nullptr;
    }

    static void SetupResolvedMethod(Method *method)
    {
        ManagedThread::GetCurrent()->GetInterpreterCache()->Clear();
        resolvedMethod_ = method;
    }

    static void SetupResolvedField(Field *field)
    {
        ManagedThread::GetCurrent()->GetInterpreterCache()->Clear();
        resolvedField_ = field;
    }

    static void SetupResolvedClass(Class *klass)
    {
        ManagedThread::GetCurrent()->GetInterpreterCache()->Clear();
        resolvedClass_ = klass;
    }

    static void SetupCatchBlockPcOffset(uint32_t pcOffset)
    {
        catchBlockPcOffset_ = pcOffset;
    }

    static void SetupNativeEntryPoint(const void *p)
    {
        entryPoint_ = p;
    }

    static coretypes::Array *CreateArray(Class *klass, coretypes::ArraySizeT length)
    {
        EXPECT_EQ(klass, arrayClass_);
        EXPECT_EQ(length, arrayLength_);
        return arrayObject_;
    }

    static void SetupArrayClass(Class *klass)
    {
        arrayClass_ = klass;
    }

    static void SetupArrayLength(coretypes::ArraySizeT length)
    {
        arrayLength_ = length;
    }

    static void SetupArrayObject(coretypes::Array *obj)
    {
        arrayObject_ = obj;
    }

    static ObjectHeader *CreateObject([[maybe_unused]] ManagedThread *thread, Class *klass)
    {
        EXPECT_EQ(klass, objectClass_);
        return object_;
    }

    static void SetupObjectClass(Class *klass)
    {
        objectClass_ = klass;
    }

    static void SetupObject(ObjectHeader *obj)
    {
        object_ = obj;
    }

    static Value InvokeMethod(ManagedThread *thread, Method *method, Value *args)
    {
        return invokeHandler_(thread, method, args);
    }

    static void SetupInvokeMethodHandler(const InvokeMethodHandler &handler)
    {
        invokeHandler_ = handler;
    }

    // Throw exceptions

    static void ThrowNullPointerException()
    {
        ASSERT_TRUE(npeData_.expected);
    }

    static void ThrowArrayIndexOutOfBoundsException(coretypes::ArraySsizeT idx, coretypes::ArraySizeT length)
    {
        ASSERT_TRUE(arrayOobExceptionData_.expected);
        ASSERT_EQ(arrayOobExceptionData_.idx, idx);
        ASSERT_EQ(arrayOobExceptionData_.length, length);
    }

    static void ThrowNegativeArraySizeException(coretypes::ArraySsizeT size)
    {
        ASSERT_TRUE(arrayNegSizeExceptionData_.expected);
        ASSERT_EQ(arrayNegSizeExceptionData_.size, size);
    }

    static void ThrowArithmeticException()
    {
        ASSERT_TRUE(arithmeticExceptionData_.expected);
    }

    static void ThrowClassCastException(Class *dstType, Class *srcType)
    {
        ASSERT_TRUE(classCastExceptionData_.expected);
        ASSERT_EQ(classCastExceptionData_.dstType, dstType);
        ASSERT_EQ(classCastExceptionData_.srcType, srcType);
    }

    static void ThrowAbstractMethodError(Method *method)
    {
        ASSERT_TRUE(abstractMethodErrorData_.expected);
        ASSERT_EQ(abstractMethodErrorData_.method, method);
    }

    static void ThrowIncompatibleClassChangeErrorForMethodConflict([[maybe_unused]] Method *method) {}

    static void ThrowOutOfMemoryError([[maybe_unused]] const PandaString &msg) {}

    static void ThrowVerificationException([[maybe_unused]] const PandaString &msg)
    {
        // ASSERT_TRUE verification_of_method_exception_data.expected
        // ASSERT_EQ verification_of_method_exception_data.msg, msg
    }

    static void ThrowArrayStoreException(Class *arrayKlass, Class *elemClass)
    {
        ASSERT_TRUE(arrayStoreExceptionData_.expected);
        ASSERT_EQ(arrayStoreExceptionData_.arrayClass, arrayKlass);
        ASSERT_EQ(arrayStoreExceptionData_.elemClass, elemClass);
    }

    static void SetArrayStoreException(ArrayStoreExceptionData data)
    {
        arrayStoreExceptionData_ = data;
    }

    static void SetNullPointerExceptionData(NullPointerExceptionData data)
    {
        npeData_ = data;
    }

    static void SetArrayIndexOutOfBoundsExceptionData(ArrayIndexOutOfBoundsExceptionData data)
    {
        arrayOobExceptionData_ = data;
    }

    static void SetNegativeArraySizeExceptionData(NegativeArraySizeExceptionData data)
    {
        arrayNegSizeExceptionData_ = data;
    }

    static void SetArithmeticExceptionData(ArithmeticException data)
    {
        arithmeticExceptionData_ = data;
    }

    static void SetClassCastExceptionData(ClassCastExceptionData data)
    {
        classCastExceptionData_ = data;
    }

    static void SetAbstractMethodErrorData(AbstractMethodError data)
    {
        abstractMethodErrorData_ = data;
    }

    template <bool IS_DYNAMIC = false>
    static Frame *CreateFrame(size_t nregs, Method *method, Frame *prev)
    {
        uint32_t extSz = EMPTY_EXT_FRAME_DATA_SIZE;
        auto allocator = Thread::GetCurrent()->GetVM()->GetHeapManager()->GetInternalAllocator();
        void *mem = allocator->Allocate(ark::Frame::GetAllocSize(ark::Frame::GetActualSize<IS_DYNAMIC>(nregs), extSz),
                                        GetLogAlignment(8), ManagedThread::GetCurrent());
        return new (Frame::FromExt(mem, extSz)) ark::Frame(mem, method, prev, nregs);
    }

    static Frame *CreateFrameWithActualArgsAndSize(uint32_t size, uint32_t nregs, uint32_t numActualArgs,
                                                   Method *method, Frame *prev)
    {
        uint32_t extSz = EMPTY_EXT_FRAME_DATA_SIZE;
        auto allocator = Thread::GetCurrent()->GetVM()->GetHeapManager()->GetInternalAllocator();
        void *mem =
            allocator->Allocate(ark::Frame::GetAllocSize(size, extSz), GetLogAlignment(8), ManagedThread::GetCurrent());
        if (UNLIKELY(mem == nullptr)) {
            return nullptr;
        }
        return new (Frame::FromExt(mem, extSz)) ark::Frame(mem, method, prev, nregs, numActualArgs);
    }

    static void FreeFrame(ManagedThread *thread, Frame *frame)
    {
        auto allocator = thread->GetVM()->GetHeapManager()->GetInternalAllocator();
        allocator->Free(frame->GetExt());
    }

    static mem::GC *GetGC()
    {
        return &ark::interpreter::test::RuntimeInterface::dummyGc_;
    }

    static const uint8_t *GetMethodName([[maybe_unused]] Method *caller, [[maybe_unused]] BytecodeId methodId)
    {
        return nullptr;
    }

    static Class *GetMethodClass([[maybe_unused]] Method *caller, [[maybe_unused]] BytecodeId methodId)
    {
        return resolvedClass_;
    }

    static uint32_t GetMethodArgumentsCount([[maybe_unused]] Method *caller, [[maybe_unused]] BytecodeId methodId)
    {
        return 0;
    }

    static void CollectRoots([[maybe_unused]] Frame *frame) {}

    static void Safepoint() {}

    static LanguageContext GetLanguageContext(const Method &method)
    {
        return Runtime::GetCurrent()->GetLanguageContext(*method.GetClass());
    }

private:
    static ArrayIndexOutOfBoundsExceptionData arrayOobExceptionData_;

    static NegativeArraySizeExceptionData arrayNegSizeExceptionData_;

    static NullPointerExceptionData npeData_;

    static ArithmeticException arithmeticExceptionData_;

    static ClassCastExceptionData classCastExceptionData_;

    static AbstractMethodError abstractMethodErrorData_;

    static ArrayStoreExceptionData arrayStoreExceptionData_;

    static coretypes::Array *arrayObject_;

    static Class *arrayClass_;

    static coretypes::ArraySizeT arrayLength_;

    static ObjectHeader *object_;

    static Class *objectClass_;

    static Class *resolvedClass_;

    static uint32_t catchBlockPcOffset_;

    static Method *resolvedMethod_;

    static Field *resolvedField_;

    static InvokeMethodHandler invokeHandler_;

    static const void *entryPoint_;

    static uint32_t jitThreshold_;

    static ark::interpreter::test::DummyGC dummyGc_;
};

}  // namespace ark::interpreter::test

#endif  // PANDA_RUNTIME_TESTS_INTERPRETER_TEST_RUNTIME_INTERFACE_H_
