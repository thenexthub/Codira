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

#include <vector>

#include "common_components/heap/barrier/barrier.h"
#include "common_components/heap/collector/collector.h"
#include "common_interfaces/objects/ref_field.h"
#include "common_interfaces/objects/base_object.h"

#include "common_components/tests/test_helper.h"

using namespace common;


class DummyObject : public BaseObject {
public:
    const common::TypeInfo* GetTypeInfo() const
    {
        return nullptr;
    }
    size_t GetSize() const
    {
        return sizeof(DummyObject);
    }
};


class MockCollector : public Collector {
public:
    void Init(const RuntimeParam& param) override {}
    void RunGarbageCollection(uint64_t, GCReason, GCType) override {}
    BaseObject* ForwardObject(BaseObject*) override
    {
        return nullptr;
    }
    bool ShouldIgnoreRequest(GCRequest&) override
    {
        return false;
    }
    bool IsFromObject(BaseObject*) const override
    {
        return false;
    }
    bool IsUnmovableFromObject(BaseObject*) const override
    {
        return false;
    }
    BaseObject* FindToVersion(BaseObject*) const override
    {
        return nullptr;
    }

    bool TryUpdateRefField(BaseObject* Obj, RefField<>& field, BaseObject*& toVersion) const override
    {
        toVersion = reinterpret_cast<BaseObject*>(field.GetFieldValue());
        return true;
    }

    bool TryForwardRefField(BaseObject*, RefField<>&, BaseObject*&) const override
    {
        return false;
    }
    bool TryUntagRefField(BaseObject*, RefField<>&, BaseObject*&) const override
    {
        return false;
    }
    RefField<> GetAndTryTagRefField(BaseObject*) const override
    {
        return RefField<>(nullptr);
    }
    bool IsOldPointer(RefField<>&) const override
    {
        return false;
    }
    bool IsCurrentPointer(RefField<>&) const override
    {
        return false;
    }
    void AddRawPointerObject(BaseObject*) override {}
    void RemoveRawPointerObject(BaseObject*) override {}
};


class BarrierTest : public common::test::BaseTestWithScope {
protected:
    MockCollector collector;
    Barrier barrier{collector};
    std::unique_ptr<DummyObject> dummyObj;

    void SetUp() override
    {
        dummyObj = std::make_unique<DummyObject>();
    }
};

HWTEST_F_L0(BarrierTest, ReadRefField_ReturnsExpectedValue) {
    uint64_t value = reinterpret_cast<uint64_t>(dummyObj.get());
    RefField<false> field(value);

    BaseObject* result = barrier.ReadRefField(nullptr, field);
    EXPECT_EQ(result, dummyObj.get());
}

HWTEST_F_L0(BarrierTest, WriteRefField_SetsTargetObject) {
    uint64_t initValue = 0;
    RefField<false> field(initValue);
    BaseObject* newRef = dummyObj.get();

    barrier.WriteRefField(nullptr, field, newRef);
    EXPECT_EQ(field.GetTargetObject(), newRef);
}

HWTEST_F_L0(BarrierTest, WriteStaticRef_SetsTargetObject) {
    uint64_t initValue = 0;
    RefField<false> field(initValue);
    BaseObject* newRef = dummyObj.get();

    barrier.WriteStaticRef(field, newRef);
    EXPECT_EQ(field.GetTargetObject(), newRef);
}

HWTEST_F_L0(BarrierTest, AtomicWriteRefField_UpdatesWithMemoryOrder) {
    uint64_t initValue = 0;
    RefField<true> field(initValue);
    BaseObject* newRef = dummyObj.get();

    barrier.AtomicWriteRefField(nullptr, field, newRef, std::memory_order_relaxed);
    EXPECT_EQ(field.GetTargetObject(std::memory_order_relaxed), newRef);
}

HWTEST_F_L0(BarrierTest, CompareAndSwapRefField_WorksWithSuccessAndFailure) {
    uint64_t initValue = 0;
    RefField<true> field(initValue);
    BaseObject* oldRef = nullptr;
    BaseObject* newRef = dummyObj.get();

    bool result = barrier.CompareAndSwapRefField(nullptr, field, oldRef, newRef,
                                                 std::memory_order_seq_cst, std::memory_order_relaxed);
    EXPECT_TRUE(result);
    EXPECT_EQ(field.GetTargetObject(std::memory_order_relaxed), newRef);
}

HWTEST_F_L0(BarrierTest, WriteStruct_HandlesDifferentLengths) {
    size_t srcBufferSize = 512;
    size_t dstBufferSize = 1024;
    char* srcBuffer = new char[srcBufferSize];
    char* dstBuffer = new char[dstBufferSize];

    for (size_t i = 0; i < srcBufferSize; ++i) {
        srcBuffer[i] = static_cast<char>(i % 256);
    }

    barrier.WriteStruct(nullptr, reinterpret_cast<HeapAddress>(dstBuffer), dstBufferSize,
                     reinterpret_cast<HeapAddress>(srcBuffer), srcBufferSize);

    EXPECT_EQ(memcmp(dstBuffer, srcBuffer, srcBufferSize), 0);

    for (size_t i = srcBufferSize; i < dstBufferSize; ++i) {
        EXPECT_EQ(dstBuffer[i], 0);
    }

    delete[] srcBuffer;
    delete[] dstBuffer;
}

HWTEST_F_L0(BarrierTest, ReadStaticRef_ReturnsExpectedValue) {
    uint64_t value = reinterpret_cast<uint64_t>(dummyObj.get());
    RefField<false> field(value);

    BaseObject* result = barrier.ReadStaticRef(field);
    EXPECT_EQ(result, dummyObj.get());
}

HWTEST_F_L0(BarrierTest, AtomicSwapRefField_ExchangesCorrectly) {
    uint64_t initValue = reinterpret_cast<uint64_t>(dummyObj.get());
    RefField<true> field(initValue);
    BaseObject* newRef = reinterpret_cast<BaseObject*>(0x1234567890);
    BaseObject* oldValue = barrier.AtomicSwapRefField(nullptr, field, newRef, std::memory_order_seq_cst);

    EXPECT_EQ(oldValue, dummyObj.get());
    EXPECT_EQ(field.GetTargetObject(std::memory_order_relaxed), newRef);
}

HWTEST_F_L0(BarrierTest, AtomicReadRefField_ReadsCorrectly) {
    uint64_t initValue = reinterpret_cast<uint64_t>(dummyObj.get());
    RefField<true> field(initValue);

    BaseObject* value = barrier.AtomicReadRefField(nullptr, field, std::memory_order_seq_cst);

    EXPECT_EQ(value, dummyObj.get());
}

HWTEST_F_L0(BarrierTest, CopyStructArray_CopiesDataCorrectly) {
    constexpr size_t arraySize = 100;
    char* srcBuffer = new char[arraySize];
    char* dstBuffer = new char[arraySize];

    for (size_t i = 0; i < arraySize; ++i) {
        srcBuffer[i] = static_cast<char>(i % 256);
    }

    BaseObject srcObj;
    BaseObject dstObj;

    HeapAddress srcFieldAddr = reinterpret_cast<HeapAddress>(srcBuffer);
    HeapAddress dstFieldAddr = reinterpret_cast<HeapAddress>(dstBuffer);

    barrier.CopyStructArray(&dstObj, dstFieldAddr, arraySize, &srcObj, srcFieldAddr, arraySize);

    EXPECT_EQ(memcmp(dstBuffer, srcBuffer, arraySize), 0);

    delete[] srcBuffer;
    delete[] dstBuffer;
}

HWTEST_F_L0(BarrierTest, ReadStruct_ReadsCorrectly) {
    struct TestStruct {
        int a;
        double b;
    };

    TestStruct* initValue = new TestStruct{42, 3.14};
    RefField<false> field(reinterpret_cast<uint64_t>(initValue));

    char dstBuffer[sizeof(TestStruct)];
    HeapAddress dstAddr = reinterpret_cast<HeapAddress>(dstBuffer);

    BaseObject dummyObj;
    HeapAddress srcAddr = reinterpret_cast<HeapAddress>(field.GetTargetObject());

    barrier.ReadStruct(dstAddr, &dummyObj, srcAddr, sizeof(TestStruct));

    TestStruct* result = reinterpret_cast<TestStruct*>(dstBuffer);
    EXPECT_EQ(result->a, initValue->a);
    EXPECT_EQ(result->b, initValue->b);

    delete initValue;
}

HWTEST_F_L0(BarrierTest, AtomicWriteStaticRef_NonConcurrent)
{
    DummyObject* targetObj = new DummyObject();
    DummyObject* initialObj = new DummyObject();

    RefField<true> field(reinterpret_cast<BaseObject*>(0x1));
    field.SetTargetObject(initialObj);

    MockCollector collector;
    Barrier barrier(collector);

    barrier.AtomicWriteRefField(nullptr, field, targetObj, std::memory_order_relaxed);

    EXPECT_EQ(field.GetTargetObject(std::memory_order_relaxed), targetObj);
}