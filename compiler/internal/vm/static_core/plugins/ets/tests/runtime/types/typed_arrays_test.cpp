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

#include <gtest/gtest.h>

#include "ets_coroutine.h"

#include "types/ets_class.h"
#include "types/ets_arraybuffer.h"
#include "types/ets_typed_arrays.h"
#include "plugins/ets/tests/runtime/types/ets_test_mirror_classes.h"
#include "cross_values.h"
#include "intrinsics.h"

namespace ark::ets::test {
class AbstractTypedArrayTest : public testing::Test {
public:
    AbstractTypedArrayTest()
    {
        options_.SetShouldLoadBootPandaFiles(true);
        options_.SetShouldInitializeIntrinsics(true);
        options_.SetCompilerEnableJit(false);
        options_.SetGcType("epsilon");
        options_.SetLoadRuntimes({"ets"});

        auto stdlib = std::getenv("PANDA_STD_LIB");
        if (stdlib == nullptr) {
            std::cerr << "PANDA_STD_LIB env variable should be set and point to etsstdlib.abc" << std::endl;
            std::abort();
        }
        options_.SetBootPandaFiles({stdlib});

        Runtime::Create(options_);
    }

    ~AbstractTypedArrayTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(AbstractTypedArrayTest);
    NO_MOVE_SEMANTIC(AbstractTypedArrayTest);

    void SetUp() override
    {
        coroutine_ = EtsCoroutine::GetCurrent();
        vm_ = coroutine_->GetPandaVM();
    }

    void TearDown() override {}

protected:
    PandaEtsVM *vm_ = nullptr;           // NOLINT(misc-non-private-member-variables-in-classes)
    EtsCoroutine *coroutine_ = nullptr;  // NOLINT(misc-non-private-member-variables-in-classes)

    EtsClass *GetClass(const char *name) const
    {
        os::memory::WriteLockHolder lock(*vm_->GetMutatorLock());
        return GetClassInManagedCode(name);
    }

    /// To get a class from a managed code, so the whole code under the mutator lock.
    EtsClass *GetClassInManagedCode(const char *name) const
    {
        return vm_->GetClassLinker()->GetClass(name);
    }

private:
    RuntimeOptions options_;
};

/// Typed arrays intrinsics depend on memory layout
class TypedArrayRelatedMemberOffsetTest : public AbstractTypedArrayTest {
public:
    TypedArrayRelatedMemberOffsetTest() = default;
    ~TypedArrayRelatedMemberOffsetTest() override = default;

    NO_COPY_SEMANTIC(TypedArrayRelatedMemberOffsetTest);
    NO_MOVE_SEMANTIC(TypedArrayRelatedMemberOffsetTest);

    static std::vector<MirrorFieldInfo> GetTypedArrayMembers()
    {
        return std::vector<MirrorFieldInfo> {
            MirrorFieldInfo("buffer", ark::cross_values::GetTypedArrayBufferOffset(RUNTIME_ARCH)),
            MirrorFieldInfo("lengthInt", ark::cross_values::GetTypedArrayLengthOffset(RUNTIME_ARCH)),
            MirrorFieldInfo("byteOffset", ark::cross_values::GetTypedArrayByteOffsetOffset(RUNTIME_ARCH))};
    }

    static std::vector<MirrorFieldInfo> GetUArrayMembers()
    {
        return std::vector<MirrorFieldInfo> {
            MirrorFieldInfo("buffer", ark::cross_values::GetTypedUnsignedArrayBufferOffset(RUNTIME_ARCH)),
            MirrorFieldInfo("lengthInt", ark::cross_values::GetTypedUnsignedArrayLengthOffset(RUNTIME_ARCH)),
            MirrorFieldInfo("byteOffsetInt", ark::cross_values::GetTypedUnsignedArrayByteOffsetOffset(RUNTIME_ARCH))};
    }

    static std::vector<MirrorFieldInfo> GetArrayBufferMembers()
    {
        return std::vector<MirrorFieldInfo> {
            MirrorFieldInfo("data", ark::cross_values::GetArrayBufferDataOffset(RUNTIME_ARCH))};
    }
};

TEST_F(TypedArrayRelatedMemberOffsetTest, Int8ArrayLayout)
{
    auto *klass = GetClass("Lescompat/Int8Array;");
    MirrorFieldInfo::CompareMemberOffsets(klass, GetTypedArrayMembers(), false);
}

TEST_F(TypedArrayRelatedMemberOffsetTest, Int16ArrayLayout)
{
    auto *klass = GetClass("Lescompat/Int16Array;");
    MirrorFieldInfo::CompareMemberOffsets(klass, GetTypedArrayMembers(), false);
}

TEST_F(TypedArrayRelatedMemberOffsetTest, Int32ArrayLayout)
{
    auto *klass = GetClass("Lescompat/Int32Array;");
    MirrorFieldInfo::CompareMemberOffsets(klass, GetTypedArrayMembers(), false);
}

TEST_F(TypedArrayRelatedMemberOffsetTest, BigInt64ArrayLayout)
{
    auto *klass = GetClass("Lescompat/BigInt64Array;");
    MirrorFieldInfo::CompareMemberOffsets(klass, GetTypedArrayMembers(), false);
}

TEST_F(TypedArrayRelatedMemberOffsetTest, Float32ArrayLayout)
{
    auto *klass = GetClass("Lescompat/Float32Array;");
    MirrorFieldInfo::CompareMemberOffsets(klass, GetTypedArrayMembers(), false);
}

TEST_F(TypedArrayRelatedMemberOffsetTest, Float64ArrayLayout)
{
    auto *klass = GetClass("Lescompat/Float64Array;");
    MirrorFieldInfo::CompareMemberOffsets(klass, GetTypedArrayMembers(), false);
}

TEST_F(TypedArrayRelatedMemberOffsetTest, UInt8ClampedArrayLayout)
{
    auto *klass = GetClass("Lescompat/Uint8ClampedArray;");
    MirrorFieldInfo::CompareMemberOffsets(klass, GetUArrayMembers(), false);
}

TEST_F(TypedArrayRelatedMemberOffsetTest, UInt8ArrayLayout)
{
    auto *klass = GetClass("Lescompat/Uint8Array;");
    MirrorFieldInfo::CompareMemberOffsets(klass, GetUArrayMembers(), false);
}

TEST_F(TypedArrayRelatedMemberOffsetTest, UInt16ArrayLayout)
{
    auto *klass = GetClass("Lescompat/Uint16Array;");
    MirrorFieldInfo::CompareMemberOffsets(klass, GetUArrayMembers(), false);
}

TEST_F(TypedArrayRelatedMemberOffsetTest, UInt32ArrayLayout)
{
    auto *klass = GetClass("Lescompat/Uint32Array;");
    MirrorFieldInfo::CompareMemberOffsets(klass, GetUArrayMembers(), false);
}

TEST_F(TypedArrayRelatedMemberOffsetTest, UBigUInt64ArrayLayout)
{
    auto *klass = GetClass("Lescompat/BigUint64Array;");
    MirrorFieldInfo::CompareMemberOffsets(klass, GetUArrayMembers(), false);
}

TEST_F(TypedArrayRelatedMemberOffsetTest, ArrayBufferLayout)
{
    auto *klass = GetClass("Lstd/core/ArrayBuffer;");
    MirrorFieldInfo::CompareMemberOffsets(klass, GetArrayBufferMembers(), false);
}

namespace {

template <typename E>
constexpr auto GetContainsNaNFunction();

template <>
constexpr auto GetContainsNaNFunction<EtsDouble>()
{
    return [](EtsEscompatTypedArray<EtsDouble> *arr, EtsInt pos) {
        return intrinsics::EtsEscompatFloat64ArrayContainsNaN(static_cast<ark::ets::EtsEscompatFloat64Array *>(arr),
                                                              pos);
    };
}

template <>
constexpr auto GetContainsNaNFunction<EtsFloat>()
{
    return [](EtsEscompatTypedArray<EtsFloat> *arr, EtsInt pos) {
        return intrinsics::EtsEscompatFloat32ArrayContainsNaN(static_cast<ark::ets::EtsEscompatFloat32Array *>(arr),
                                                              pos);
    };
}

template <typename E>
constexpr const char *GetTypedArrayClassName();

template <>
constexpr const char *GetTypedArrayClassName<EtsDouble>()
{
    return "Lescompat/Float64Array;";
}

template <>
constexpr const char *GetTypedArrayClassName<EtsFloat>()
{
    return "Lescompat/Float32Array;";
}

template <typename E>
inline auto g_containsNaN = GetContainsNaNFunction<E>();

}  // anonymous namespace

// NOLINTBEGIN(readability-magic-numbers)

/// Test for specific operations of typed arrays of floating-point numbers: Float32Array and Float64Array.
class FloatingTypedArrayTest : public AbstractTypedArrayTest {
public:
    FloatingTypedArrayTest() = default;
    ~FloatingTypedArrayTest() override = default;

    NO_COPY_SEMANTIC(FloatingTypedArrayTest);
    NO_MOVE_SEMANTIC(FloatingTypedArrayTest);

protected:
    static constexpr EtsInt TEST_INDEX_0 = 0;
    static constexpr EtsInt TEST_INDEX_1 = 1;
    static constexpr EtsInt TEST_INDEX_2 = 7;
    static constexpr EtsInt TEST_INDEX_3 = 8;
    static constexpr EtsInt TEST_INDEX_4 = -1;
    static constexpr EtsInt TEST_INDEX_5 = -7;
    static constexpr EtsInt TEST_INDEX_6 = -8;
    static constexpr EtsInt TEST_INDEX_7 = 6;

    void SetUp() override
    {
        AbstractTypedArrayTest::SetUp();
        coroutine_->ManagedCodeBegin();
    }

    void TearDown() override
    {
        coroutine_->ManagedCodeEnd();
        AbstractTypedArrayTest::TearDown();
    }

    template <typename Elem>
    [[nodiscard]] EtsEscompatTypedArray<Elem> *CreateTypedArray(std::initializer_list<Elem> values) const
    {
        using TypedArray = EtsEscompatTypedArray<Elem>;
        auto *arrayKlass = GetClassInManagedCode(GetTypedArrayClassName<Elem>());
        ASSERT(arrayKlass != nullptr);
        auto *array = static_cast<TypedArray *>(TypedArray::Create(coroutine_, arrayKlass));
        ASSERT(array != nullptr);
        *static_cast<EtsInt *>(ToVoidPtr(ToUintPtr(array) + TypedArray::GetLengthIntOffset())) =
            static_cast<EtsInt>(values.size());

        auto byteLength = sizeof(Elem) * values.size();
        *static_cast<EtsDouble *>(ToVoidPtr(ToUintPtr(array) + TypedArray::GetByteLengthOffset())) =
            static_cast<EtsDouble>(byteLength);

        auto *arrayBuffer = EtsEscompatArrayBuffer::Create(coroutine_, byteLength);
        ASSERT(arrayBuffer != nullptr);
        auto *buffer = arrayBuffer->GetData();
        ASSERT(buffer != nullptr);

        *static_cast<ObjectPointer<EtsObject> *>(ToVoidPtr(ToUintPtr(array) + TypedArray::GetBufferOffset())) =
            arrayBuffer;
        *static_cast<EtsDouble *>(ToVoidPtr(ToUintPtr(array) + TypedArray::GetByteOffsetOffset())) = 0.0;

        std::copy(values.begin(), values.end(),
                  static_cast<Elem *>(ToVoidPtr(ToUintPtr(buffer) + static_cast<uintptr_t>(array->GetByteOffset()))));

        return array;
    }

    template <typename E>
    void TestNoNaN()
    {
        auto *arr = CreateTypedArray<E>({3.14281617, -3.4e+38, -2.02e+16, 0, 1.06e+12, 2.8e+24, 3.4e+38});
        EXPECT_EQ(arr->GetLengthInt(), 7U);
        EXPECT_FALSE(g_containsNaN<E>(arr, TEST_INDEX_0));
        EXPECT_FALSE(g_containsNaN<E>(arr, TEST_INDEX_1));
        EXPECT_FALSE(g_containsNaN<E>(arr, TEST_INDEX_2));
        EXPECT_FALSE(g_containsNaN<E>(arr, TEST_INDEX_3));
        EXPECT_FALSE(g_containsNaN<E>(arr, TEST_INDEX_4));
        EXPECT_FALSE(g_containsNaN<E>(arr, TEST_INDEX_5));
        EXPECT_FALSE(g_containsNaN<E>(arr, TEST_INDEX_6));
    }

    template <typename E>
    void TestNoNaNWithInf()
    {
        auto *arr = CreateTypedArray<E>(
            {std::numeric_limits<E>::infinity(), -3.4e+38, -2.02e+16, 0, 1.06e+12, 2.8e+24, 3.4e+38});
        EXPECT_EQ(arr->GetLengthInt(), 7U);
        EXPECT_FALSE(g_containsNaN<E>(arr, TEST_INDEX_0));
        EXPECT_FALSE(g_containsNaN<E>(arr, TEST_INDEX_1));
        EXPECT_FALSE(g_containsNaN<E>(arr, TEST_INDEX_2));
        EXPECT_FALSE(g_containsNaN<E>(arr, TEST_INDEX_3));
        EXPECT_FALSE(g_containsNaN<E>(arr, TEST_INDEX_4));
        EXPECT_FALSE(g_containsNaN<E>(arr, TEST_INDEX_5));
        EXPECT_FALSE(g_containsNaN<E>(arr, TEST_INDEX_6));

        arr =
            CreateTypedArray<E>({3.1428, std::numeric_limits<E>::infinity(), -2.02e+16, 0, 1.06e+12, 2.8e+24, 3.4e+38});
        EXPECT_EQ(arr->GetLengthInt(), 7U);
        EXPECT_FALSE(g_containsNaN<E>(arr, TEST_INDEX_0));

        arr = CreateTypedArray<E>(
            {3.1428, -3.4e+38, -2.02e+16, 0, 1.06e+12, 2.8e+24, std::numeric_limits<E>::infinity()});
        EXPECT_EQ(arr->GetLengthInt(), 7U);
        EXPECT_FALSE(g_containsNaN<E>(arr, TEST_INDEX_0));

        arr = CreateTypedArray<E>({std::numeric_limits<E>::infinity()});
        EXPECT_EQ(arr->GetLengthInt(), 1U);
        EXPECT_FALSE(g_containsNaN<E>(arr, TEST_INDEX_0));
    }

    template <typename E>
    void TestContainsNaN(E nan)
    {
        auto checkContainsNaN = [](auto *arr, int32_t nanPosition) {
            EXPECT_GT(arr->GetLengthInt(), 0U);
            EXPECT_TRUE(g_containsNaN<E>(arr, TEST_INDEX_0));
            EXPECT_TRUE(g_containsNaN<E>(arr, nanPosition));
            EXPECT_FALSE(g_containsNaN<E>(arr, nanPosition + arr->GetLengthInt()));
            EXPECT_FALSE(g_containsNaN<E>(arr, arr->GetLengthInt()));
            EXPECT_TRUE(g_containsNaN<E>(arr, -arr->GetLengthInt()));
            EXPECT_TRUE(g_containsNaN<E>(arr, nanPosition - arr->GetLengthInt()));
            if (nanPosition + 1 != arr->GetLengthInt()) {
                EXPECT_FALSE(g_containsNaN<E>(arr, nanPosition - arr->GetLengthInt() + 1));
            }
            EXPECT_TRUE(g_containsNaN<E>(arr, -arr->GetLengthInt() - 1));
        };
        const E inf = std::numeric_limits<E>::infinity();
        auto *arr = CreateTypedArray<E>({nan, 3.14281617, -3.4e+38, -2.02e+16, 0, 1.06e+12, 2.8e+24, 3.4e+38});
        EXPECT_EQ(arr->GetLengthInt(), 8U);
        checkContainsNaN(arr, TEST_INDEX_0);

        arr = CreateTypedArray<E>({nan, inf, -3.4e+38, -2.02e+16, 0, 1.06e+12, 2.8e+24, 3.4e+38});
        EXPECT_EQ(arr->GetLengthInt(), 8U);
        checkContainsNaN(arr, TEST_INDEX_0);

        arr = CreateTypedArray<E>({inf, nan, -3.4e+38, -2.02e+16, 0, 1.06e+12, 2.8e+24, 3.4e+38});
        EXPECT_EQ(arr->GetLengthInt(), 8U);
        checkContainsNaN(arr, TEST_INDEX_1);

        arr = CreateTypedArray<E>({3.4e+38, -3.4e+38, -2.02e+16, 0, 1.06e+12, 2.8e+24, nan, inf});
        EXPECT_EQ(arr->GetLengthInt(), 8U);
        checkContainsNaN(arr, TEST_INDEX_7);

        arr = CreateTypedArray<E>({3.4e+38, -3.4e+38, -2.02e+16, 0, 1.06e+12, inf, 2.8e+24, nan});
        EXPECT_EQ(arr->GetLengthInt(), 8U);
        checkContainsNaN(arr, TEST_INDEX_2);

        arr = CreateTypedArray<E>({inf, nan});
        EXPECT_EQ(arr->GetLengthInt(), 2U);
        checkContainsNaN(arr, TEST_INDEX_1);

        arr = CreateTypedArray<E>({nan, inf});
        EXPECT_EQ(arr->GetLengthInt(), 2U);
        checkContainsNaN(arr, TEST_INDEX_0);
    }

    template <typename E>
    void TestContainsNaNOnly(E nan)
    {
        auto checkContainsNaN = [](auto *arr) {
            EXPECT_GT(arr->GetLengthInt(), 0);
            EXPECT_TRUE(g_containsNaN<E>(arr, TEST_INDEX_0));
            EXPECT_TRUE(g_containsNaN<E>(arr, arr->GetLengthInt() - 1));
            EXPECT_TRUE(g_containsNaN<E>(arr, TEST_INDEX_4));
            EXPECT_FALSE(g_containsNaN<E>(arr, arr->GetLengthInt()));
            EXPECT_TRUE(g_containsNaN<E>(arr, -arr->GetLengthInt() - 1));
        };
        checkContainsNaN(CreateTypedArray({nan}));
        checkContainsNaN(CreateTypedArray({nan, nan}));
        checkContainsNaN(CreateTypedArray({nan, nan, nan}));
    }
};
// NOLINTEND(readability-magic-numbers)

TEST_F(FloatingTypedArrayTest, Float64NoNaN)
{
    TestNoNaN<EtsDouble>();
}

TEST_F(FloatingTypedArrayTest, Float64NoNaNWithInf)
{
    TestNoNaNWithInf<EtsDouble>();
}

TEST_F(FloatingTypedArrayTest, Float64ContainsQuietNaN)
{
    TestContainsNaN(std::numeric_limits<EtsDouble>::quiet_NaN());
}

TEST_F(FloatingTypedArrayTest, Float64ContainsSignalingNaN)
{
    TestContainsNaN(std::numeric_limits<EtsDouble>::signaling_NaN());
}

TEST_F(FloatingTypedArrayTest, Float64ContainsQuietNaNOnly)
{
    TestContainsNaNOnly(std::numeric_limits<EtsDouble>::quiet_NaN());
}

TEST_F(FloatingTypedArrayTest, Float64ContainsSignalingNaNOnly)
{
    TestContainsNaNOnly(std::numeric_limits<EtsDouble>::signaling_NaN());
}

TEST_F(FloatingTypedArrayTest, Float32NoNaN)
{
    TestNoNaN<EtsFloat>();
}

TEST_F(FloatingTypedArrayTest, Float32NoNaNWithInf)
{
    TestNoNaNWithInf<EtsFloat>();
}

TEST_F(FloatingTypedArrayTest, Float32ContainsQuietNaN)
{
    TestContainsNaN(std::numeric_limits<EtsFloat>::quiet_NaN());
}

TEST_F(FloatingTypedArrayTest, Float32ContainsSignalingNaN)
{
    TestContainsNaN(std::numeric_limits<EtsFloat>::signaling_NaN());
}

TEST_F(FloatingTypedArrayTest, Float32ContainsQuietNaNOnly)
{
    TestContainsNaNOnly(std::numeric_limits<EtsFloat>::quiet_NaN());
}

TEST_F(FloatingTypedArrayTest, Float32ContainsSignalingNaNOnly)
{
    TestContainsNaNOnly(std::numeric_limits<EtsFloat>::signaling_NaN());
}

}  // namespace ark::ets::test
