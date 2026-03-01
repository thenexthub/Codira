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

#include "encoder64_test.h"
#include "operands.h"
#include "runtime/entrypoints/entrypoints.h"
#include "type_info.h"

namespace ark::compiler {
// NOLINTBEGIN(readability-magic-numbers)
template <typename T>
bool TestCmp64(Encoder64Test *test)
{
    static_assert(sizeof(T) == sizeof(int64_t));
    // Initialize
    test->PreWork();

    // First type-dependency
    auto param1 = test->GetParameter(TypeInfo(T(0)), 0);
    auto param2 = test->GetParameter(TypeInfo(T(0)), 1);

    // Main test call
    test->GetEncoder()->EncodeCmp(param1, param1, param2, std::is_signed_v<T> ? Condition::LT : Condition::LO);

    // Finalize
    test->PostWork();

    // If encode unsupported - now print error
    if (!test->GetEncoder()->GetResult()) {
        std::cerr << "Unsupported for " << TypeName<T>() << std::endl;
        return false;
    }
    // Change this for enable print disasm
    test->Dump(false);

    uint64_t hi = static_cast<uint64_t>(0xABCDEFU) << (BITS_PER_BYTE * sizeof(uint32_t));

    // Main test loop:
    for (uint64_t i = 0; i < ITERATION; ++i) {
        uint32_t lo = RandomGen<T>();

        // Second type-dependency
        T tmp1 = hi | (lo + 1U);
        T tmp2 = hi | lo;
        // Deduced conflicting types for parameter

        auto compare = [](T a, T b) -> int32_t { return a < b ? -1 : a > b ? 1 : 0; };

        // Main check - compare parameter and
        // return value
        if (!test->CallCode<T, int32_t>(tmp1, tmp2, compare(tmp1, tmp2))) {
            return false;
        }
    }
    return true;
}

TEST_F(Encoder64Test, Cmp64Test)
{
    EXPECT_TRUE(TestCmp64<int64_t>(this));
    EXPECT_TRUE(TestCmp64<uint64_t>(this));
}

template <typename T>
bool TestCompare(Encoder64Test *test)
{
    // Initialize
    test->PreWork();

    // First type-dependency
    auto param1 = test->GetParameter(TypeInfo(T(0)), 0);
    auto param2 = test->GetParameter(TypeInfo(T(0)), 1);

    // Main test call
    test->GetEncoder()->EncodeCompare(param1, param1, param2, std::is_signed_v<T> ? Condition::GE : Condition::HS);

    // Finalize
    test->PostWork();

    // If encode unsupported - now print error
    if (!test->GetEncoder()->GetResult()) {
        std::cerr << "Unsupported for " << TypeName<T>() << std::endl;
        return false;
    }
    // Change this for enable print disasm
    test->Dump(false);

    // Main test loop:
    for (uint64_t i = 0; i < ITERATION; ++i) {
        // Second type-dependency
        T tmp1 = RandomGen<T>();
        T tmp2 = RandomGen<T>();
        // Deduced conflicting types for parameter

        auto compare = [](T a, T b) -> int32_t { return a >= b; };

        // Main check - compare parameter and
        // return value
        if (!test->CallCode<T, int32_t>(tmp1, tmp2, compare(tmp1, tmp2))) {
            return false;
        }
    }
    return true;
}

TEST_F(Encoder64Test, CompareTest)
{
    EXPECT_TRUE(TestCompare<int32_t>(this));
    EXPECT_TRUE(TestCompare<int64_t>(this));
    EXPECT_TRUE(TestCompare<uint32_t>(this));
    EXPECT_TRUE(TestCompare<uint64_t>(this));
}

template <typename T>
bool TestCompare64(Encoder64Test *test)
{
    static_assert(sizeof(T) == sizeof(int64_t));
    // Initialize
    test->PreWork();

    // First type-dependency
    auto param1 = test->GetParameter(TypeInfo(T(0)), 0);
    auto param2 = test->GetParameter(TypeInfo(T(0)), 1);

    // Main test call
    test->GetEncoder()->EncodeCompare(param1, param1, param2, std::is_signed_v<T> ? Condition::LT : Condition::LO);

    // Finalize
    test->PostWork();

    // If encode unsupported - now print error
    if (!test->GetEncoder()->GetResult()) {
        std::cerr << "Unsupported for " << TypeName<T>() << std::endl;
        return false;
    }
    // Change this for enable print disasm
    test->Dump(false);

    uint64_t hi = static_cast<uint64_t>(0xABCDEFU) << (BITS_PER_BYTE * sizeof(uint32_t));

    // Main test loop:
    for (uint64_t i = 0; i < ITERATION; ++i) {
        uint32_t lo = RandomGen<T>();

        // Second type-dependency
        T tmp1 = hi | (lo + 1U);
        T tmp2 = hi | lo;
        // Deduced conflicting types for parameter

        auto compare = [](T a, T b) -> int32_t { return a < b; };

        // Main check - compare parameter and
        // return value
        if (!test->CallCode<T, int32_t>(tmp1, tmp2, compare(tmp1, tmp2))) {
            return false;
        }
    }
    return true;
}

TEST_F(Encoder64Test, Compare64Test)
{
    EXPECT_TRUE(TestCompare64<int64_t>(this));
    EXPECT_TRUE(TestCompare64<uint64_t>(this));
}

template <typename Src, typename Dst>
bool TestCast(Encoder64Test *test)
{
    // Initialize
    test->PreWork();
    // First type-dependency
    auto input = test->GetParameter(TypeInfo(Src(0)), 0);
    auto output = test->GetParameter(TypeInfo(Dst(0)), 0);
    // Main test call
    test->GetEncoder()->EncodeCast(output, std::is_signed_v<Dst>, input, std::is_signed_v<Src>);

    // Finalize
    test->PostWork();

    // If encode unsupported - now print error
    if (!test->GetEncoder()->GetResult()) {
        std::cerr << "Unsupported for " << TypeName<Src>() << ", " << TypeName<Dst>() << "\n";
        return false;
    }
    // Change this for enable print disasm
    test->Dump(false);

    // Main test loop:
    for (uint64_t i = 0; i < ITERATION; ++i) {
        // Using max size type: type result "Dst" or 32bit to check result,
        // because in our ISA min type is 32bit.
        // Only integers less thаn 32bit.
        using DstExt = typename std::conditional<(sizeof(Dst) * BYTE_SIZE) >= WORD_SIZE, Dst, uint32_t>::type;

        // Second type-dependency
        auto src = RandomGen<Src>();
        auto dst = static_cast<DstExt>(static_cast<Dst>(src));

        // Other cast for float->int
        if (std::is_floating_point<Src>() && !std::is_floating_point<Dst>()) {
            auto minInt = std::numeric_limits<Dst>::min();
            auto maxInt = std::numeric_limits<Dst>::max();
            auto floatMinInt = static_cast<Src>(minInt);
            auto floatMaxInt = static_cast<Src>(maxInt);

            if (src > floatMinInt) {
                dst = src < floatMaxInt ? static_cast<Dst>(src) : maxInt;
            } else if (!std::isnan(src)) {
                dst = minInt;
            } else {
                dst = 0;
            }
        }

        // Deduced conflicting types for parameter

        // Main check - compare parameter and
        // return value
        if (!test->CallCode<Src, DstExt>(src, dst)) {
            return false;
        }
    }

    if constexpr (std::is_floating_point_v<Src> && std::is_integral_v<Dst>) {
        Src nan = std::numeric_limits<Src>::quiet_NaN();
        using DstExt = typename std::conditional<(sizeof(Dst) * BYTE_SIZE) >= WORD_SIZE, Dst, uint32_t>::type;

        if (!test->CallCode<Src, DstExt>(nan, static_cast<DstExt>((Dst(0))))) {
            return false;
        }
    }

    return true;
}

TEST_F(Encoder64Test, CastTestInt8)
{
    // Test int8
    EXPECT_TRUE((TestCast<int8_t, int8_t>(this)));
    EXPECT_TRUE((TestCast<int8_t, int16_t>(this)));
    EXPECT_TRUE((TestCast<int8_t, int32_t>(this)));
    EXPECT_TRUE((TestCast<int8_t, int64_t>(this)));

    EXPECT_TRUE((TestCast<int8_t, uint8_t>(this)));
    EXPECT_TRUE((TestCast<int8_t, uint16_t>(this)));
    EXPECT_TRUE((TestCast<int8_t, uint32_t>(this)));
    EXPECT_TRUE((TestCast<int8_t, uint64_t>(this)));
    EXPECT_TRUE((TestCast<int8_t, float>(this)));
    EXPECT_TRUE((TestCast<int8_t, double>(this)));

    EXPECT_TRUE((TestCast<uint8_t, int8_t>(this)));
    EXPECT_TRUE((TestCast<uint8_t, int16_t>(this)));
    EXPECT_TRUE((TestCast<uint8_t, int32_t>(this)));
    EXPECT_TRUE((TestCast<uint8_t, int64_t>(this)));

    EXPECT_TRUE((TestCast<uint8_t, uint8_t>(this)));
    EXPECT_TRUE((TestCast<uint8_t, uint16_t>(this)));
    EXPECT_TRUE((TestCast<uint8_t, uint32_t>(this)));
    EXPECT_TRUE((TestCast<uint8_t, uint64_t>(this)));
    EXPECT_TRUE((TestCast<uint8_t, float>(this)));
    EXPECT_TRUE((TestCast<uint8_t, double>(this)));
}

TEST_F(Encoder64Test, CastTestInt16)
{
    // Test int16
    EXPECT_TRUE((TestCast<int16_t, int8_t>(this)));
    EXPECT_TRUE((TestCast<int16_t, int16_t>(this)));
    EXPECT_TRUE((TestCast<int16_t, int32_t>(this)));
    EXPECT_TRUE((TestCast<int16_t, int64_t>(this)));

    EXPECT_TRUE((TestCast<int16_t, uint8_t>(this)));
    EXPECT_TRUE((TestCast<int16_t, uint16_t>(this)));
    EXPECT_TRUE((TestCast<int16_t, uint32_t>(this)));
    EXPECT_TRUE((TestCast<int16_t, uint64_t>(this)));
    EXPECT_TRUE((TestCast<int16_t, float>(this)));
    EXPECT_TRUE((TestCast<int16_t, double>(this)));

    EXPECT_TRUE((TestCast<uint16_t, int8_t>(this)));
    EXPECT_TRUE((TestCast<uint16_t, int16_t>(this)));
    EXPECT_TRUE((TestCast<uint16_t, int32_t>(this)));
    EXPECT_TRUE((TestCast<uint16_t, int64_t>(this)));

    EXPECT_TRUE((TestCast<uint16_t, uint8_t>(this)));
    EXPECT_TRUE((TestCast<uint16_t, uint16_t>(this)));
    EXPECT_TRUE((TestCast<uint16_t, uint32_t>(this)));
    EXPECT_TRUE((TestCast<uint16_t, uint64_t>(this)));
    EXPECT_TRUE((TestCast<uint16_t, float>(this)));
    EXPECT_TRUE((TestCast<uint16_t, double>(this)));
}

TEST_F(Encoder64Test, CastTestInt32)
{
    // Test int32
    EXPECT_TRUE((TestCast<int32_t, int8_t>(this)));
    EXPECT_TRUE((TestCast<int32_t, int16_t>(this)));
    EXPECT_TRUE((TestCast<int32_t, int32_t>(this)));
    EXPECT_TRUE((TestCast<int32_t, int64_t>(this)));

    EXPECT_TRUE((TestCast<int32_t, uint8_t>(this)));
    EXPECT_TRUE((TestCast<int32_t, uint16_t>(this)));
    EXPECT_TRUE((TestCast<int32_t, uint32_t>(this)));
    EXPECT_TRUE((TestCast<int32_t, uint64_t>(this)));
    EXPECT_TRUE((TestCast<int32_t, float>(this)));
    EXPECT_TRUE((TestCast<int32_t, double>(this)));

    EXPECT_TRUE((TestCast<uint32_t, int8_t>(this)));
    EXPECT_TRUE((TestCast<uint32_t, int16_t>(this)));
    EXPECT_TRUE((TestCast<uint32_t, int32_t>(this)));
    EXPECT_TRUE((TestCast<uint32_t, int64_t>(this)));

    EXPECT_TRUE((TestCast<uint32_t, uint8_t>(this)));
    EXPECT_TRUE((TestCast<uint32_t, uint16_t>(this)));
    EXPECT_TRUE((TestCast<uint32_t, uint32_t>(this)));
    EXPECT_TRUE((TestCast<uint32_t, uint64_t>(this)));
    EXPECT_TRUE((TestCast<uint32_t, float>(this)));
    EXPECT_TRUE((TestCast<uint32_t, double>(this)));
}

TEST_F(Encoder64Test, CastTestInt64)
{
    // Test int64
    EXPECT_TRUE((TestCast<int64_t, int8_t>(this)));
    EXPECT_TRUE((TestCast<int64_t, int16_t>(this)));
    EXPECT_TRUE((TestCast<int64_t, int32_t>(this)));
    EXPECT_TRUE((TestCast<int64_t, int64_t>(this)));

    EXPECT_TRUE((TestCast<int64_t, uint8_t>(this)));
    EXPECT_TRUE((TestCast<int64_t, uint16_t>(this)));
    EXPECT_TRUE((TestCast<int64_t, uint32_t>(this)));
    EXPECT_TRUE((TestCast<int64_t, uint64_t>(this)));
    EXPECT_TRUE((TestCast<int64_t, float>(this)));
    EXPECT_TRUE((TestCast<int64_t, double>(this)));

    EXPECT_TRUE((TestCast<uint64_t, int8_t>(this)));
    EXPECT_TRUE((TestCast<uint64_t, int16_t>(this)));
    EXPECT_TRUE((TestCast<uint64_t, int32_t>(this)));
    EXPECT_TRUE((TestCast<uint64_t, int64_t>(this)));

    EXPECT_TRUE((TestCast<uint64_t, uint8_t>(this)));
    EXPECT_TRUE((TestCast<uint64_t, uint16_t>(this)));
    EXPECT_TRUE((TestCast<uint64_t, uint32_t>(this)));
    EXPECT_TRUE((TestCast<uint64_t, uint64_t>(this)));
    EXPECT_TRUE((TestCast<uint64_t, float>(this)));
    EXPECT_TRUE((TestCast<uint64_t, double>(this)));
}

TEST_F(Encoder64Test, CastTestFloat)
{
    // Test float/double
    EXPECT_TRUE((TestCast<float, float>(this)));
    EXPECT_TRUE((TestCast<double, double>(this)));
    EXPECT_TRUE((TestCast<float, double>(this)));
    EXPECT_TRUE((TestCast<double, float>(this)));

    // We DON'T support cast from float32/64 to int8/16.

    EXPECT_TRUE((TestCast<float, int32_t>(this)));
    EXPECT_TRUE((TestCast<float, int64_t>(this)));
    EXPECT_TRUE((TestCast<double, int32_t>(this)));
    EXPECT_TRUE((TestCast<double, int64_t>(this)));

    EXPECT_TRUE((TestCast<float, uint32_t>(this)));
    EXPECT_TRUE((TestCast<float, uint64_t>(this)));
    EXPECT_TRUE((TestCast<double, uint32_t>(this)));
    EXPECT_TRUE((TestCast<double, uint64_t>(this)));
}

bool TestFcvtjzs(Encoder64Test *test)
{
    // Initialize
    test->PreWork();
    // First type-dependency
    auto input = test->GetParameter(TypeInfo(double(0)), 0);
    auto output = test->GetParameter(TypeInfo(int32_t(0)), 0);
    // Main test call
    test->GetEncoder()->EncodeFastPathDynamicCast(output, input, LabelHolder::INVALID_LABEL);

    // Finalize
    test->PostWork();

    // If encode unsupported - now print error
    if (!test->GetEncoder()->GetResult()) {
        std::cerr << "Unsupported for " << TypeName<double>() << ", " << TypeName<int32_t>() << "\n";
        return false;
    }
    // Change this for enable print disasm
    test->Dump(false);

    // Main test loop:
    for (uint64_t i = 0; i < ITERATION; ++i) {
        auto src = RandomGen<double>();
        auto dst = JsCastDoubleToInt32(src);
        if (!test->CallCode<double, int32_t>(src, dst)) {
            return false;
        }
    }

    return true;
}

TEST_F(Encoder64Test, FcvtjzsTest)
{
    if (CpuFeaturesHasJscvt()) {
        EXPECT_TRUE((TestFcvtjzs(this)));
    }
}

template <typename T>
bool TestDiv(Encoder64Test *test)
{
    bool isSigned = std::is_signed<T>::value;

    test->PreWork();

    // First type-dependency
    auto param1 = test->GetParameter(TypeInfo(T(0)), 0);
    auto param2 = test->GetParameter(TypeInfo(T(0)), 1);

    // Main test call
    test->GetEncoder()->EncodeDiv(param1, isSigned, param1, param2);

    // Finalize
    test->PostWork();

    // If encode unsupported - now print error
    if (!test->GetEncoder()->GetResult()) {
        std::cerr << "Unsupported for " << TypeName<T>() << std::endl;
        return false;
    }
    // Change this for enable print disasm
    test->Dump(false);

    // Main test loop:
    for (uint64_t i = 0; i < ITERATION; ++i) {
        // Second type-dependency
        T tmp1 = RandomGen<T>();
        T tmp2 = RandomGen<T>();
        tmp2 = (tmp2 == 0) ? 1 : tmp2;
        // Main check - compare parameter and
        // return value
        if (!test->CallCode<T>(tmp1, tmp2, (tmp1 / tmp2))) {
            return false;
        }
    }

    if constexpr (std::is_floating_point_v<T>) {
        T nan = std::numeric_limits<T>::quiet_NaN();
        if (!test->CallCode<T>(nan, RandomGen<T>(), nan)) {
            return false;
        }
        if (!test->CallCode<T>(RandomGen<T>(), nan, nan)) {
            return false;
        }
        if (!test->CallCode<T>(0.0, 0.0, nan)) {
            return false;
        }
        if (!test->CallCode<T>(std::numeric_limits<T>::infinity(), std::numeric_limits<T>::infinity(), nan)) {
            return false;
        }
        if (!test->CallCode<T>(-std::numeric_limits<T>::infinity(), std::numeric_limits<T>::infinity(), nan)) {
            return false;
        }
    }

    return true;
}

TEST_F(Encoder64Test, DivTest)
{
    EXPECT_TRUE(TestDiv<int32_t>(this));
    EXPECT_TRUE(TestDiv<int64_t>(this));
    EXPECT_TRUE(TestDiv<uint32_t>(this));
    EXPECT_TRUE(TestDiv<uint64_t>(this));
    EXPECT_TRUE(TestDiv<float>(this));
    EXPECT_TRUE(TestDiv<double>(this));
}

template <typename T>
bool TestDivImm(Encoder64Test *test)
{
    static_assert(std::is_integral_v<T>);
    bool isSigned = std::is_signed_v<T>;
    using ExtT = std::conditional_t<std::is_signed_v<T>, int64_t, uint64_t>;

    auto ceiled = std::ceil(std::sqrt(ITERATION));
    auto innerIters = static_cast<uint64_t>(ceiled);

    for (uint64_t i = 0; i < ITERATION; ++i) {
        test->PreWork();

        // First type-dependency
        auto param1 = test->GetParameter(TypeInfo(T(0)), 0);

        T imm = RandomGen<T>();
        while (!test->GetEncoder()->CanOptimizeImmDivMod(bit_cast<uint64_t>(static_cast<ExtT>(imm)), isSigned)) {
            imm = RandomGen<T>();
        }
        ASSERT(imm != T(0));

        // Main test call
        test->GetEncoder()->EncodeDiv(param1, param1, Imm(imm), isSigned);

        // Finalize
        test->PostWork();

        // If encode unsupported - now print error
        if (!test->GetEncoder()->GetResult()) {
            std::cerr << "Unsupported for " << TypeName<T>() << std::endl;
            return false;
        }
        // Change this for enable print disasm
        test->Dump(false);

        // Main test loop:
        for (uint64_t j = 0; j < innerIters; ++j) {
            // Second type-dependency
            T op1 = RandomGen<T>();
            // Main check - compare parameter and
            // return value
            if (!test->CallCode<T>(op1, (op1 / imm))) {
                return false;
            }
        }
    }

    return true;
}

TEST_F(Encoder64Test, DivImmTest)
{
    EXPECT_TRUE(TestDivImm<int32_t>(this));
    EXPECT_TRUE(TestDivImm<int64_t>(this));
    EXPECT_TRUE(TestDivImm<uint32_t>(this));
    EXPECT_TRUE(TestDivImm<uint64_t>(this));
}

template <typename T>
bool TestModImm(Encoder64Test *test)
{
    static_assert(std::is_integral_v<T>);
    bool isSigned = std::is_signed_v<T>;
    using ExtT = std::conditional_t<std::is_signed_v<T>, int64_t, uint64_t>;

    auto innerIters = static_cast<uint64_t>(std::ceil(std::sqrt(ITERATION)));

    for (uint64_t i = 0; i < ITERATION; ++i) {
        test->PreWork();

        // First type-dependency
        auto param1 = test->GetParameter(TypeInfo(T(0)), 0);

        T imm = RandomGen<T>();
        while (!test->GetEncoder()->CanOptimizeImmDivMod(bit_cast<uint64_t>(static_cast<ExtT>(imm)), isSigned)) {
            imm = RandomGen<T>();
        }
        ASSERT(imm != T(0));

        // Main test call
        test->GetEncoder()->EncodeMod(param1, param1, Imm(imm), isSigned);

        // Finalize
        test->PostWork();

        // If encode unsupported - now print error
        if (!test->GetEncoder()->GetResult()) {
            std::cerr << "Unsupported for " << TypeName<T>() << std::endl;
            return false;
        }
        // Change this for enable print disasm
        test->Dump(false);

        // Main test loop:
        for (uint64_t j = 0; j < innerIters; ++j) {
            // Second type-dependency
            T op1 = RandomGen<T>();
            // Main check - compare parameter and
            // return value
            if (!test->CallCode<T>(op1, (op1 % imm))) {
                return false;
            }
        }
    }

    return true;
}

TEST_F(Encoder64Test, ModImmTest)
{
    EXPECT_TRUE(TestModImm<int32_t>(this));
    EXPECT_TRUE(TestModImm<int64_t>(this));
    EXPECT_TRUE(TestModImm<uint32_t>(this));
    EXPECT_TRUE(TestModImm<uint64_t>(this));
}

template <typename T>
bool TestModMainLoop(Encoder64Test *test)
{
    // Main test loop:
    for (uint64_t i = 0; i < ITERATION; ++i) {
        // Second type-dependency
        T tmp1 = RandomGen<T>();
        T tmp2 = RandomGen<T>();
        if (tmp2 == 0) {
            tmp2 += 1;
        }
        // Main check - compare parameter and
        // return value
        if constexpr (std::is_same<float, T>::value) {
            auto mod = fmodf(tmp1, tmp2);
            if (!test->CallCode<T>(tmp1, tmp2, mod)) {
                return false;
            }
        } else if constexpr (std::is_same<double, T>::value) {
            auto mod = fmod(tmp1, tmp2);
            if (!test->CallCode<T>(tmp1, tmp2, mod)) {
                return false;
            }
        } else {
            auto mod = static_cast<T>(tmp1 % tmp2);
            if (!test->CallCode<T>(tmp1, tmp2, mod)) {
                return false;
            }
        }
    }

    if constexpr (std::is_floating_point_v<T>) {
        T nan = std::numeric_limits<T>::quiet_NaN();
        if (!test->CallCode<T>(RandomGen<T>(), nan, nan)) {
            return false;
        }
        if (!test->CallCode<T>(nan, RandomGen<T>(), nan)) {
            return false;
        }
        if (!test->CallCode<T>(0.0, 0.0, nan)) {
            return false;
        }
        if (!test->CallCode<T>(std::numeric_limits<T>::infinity(), std::numeric_limits<T>::infinity(), nan)) {
            return false;
        }
        if (!test->CallCode<T>(-std::numeric_limits<T>::infinity(), std::numeric_limits<T>::infinity(), nan)) {
            return false;
        }
    }
    return true;
}

template <typename T>
bool TestMod(Encoder64Test *test)
{
    bool isSigned = std::is_signed<T>::value;

    auto linkReg = Target::Current().GetLinkReg();

    test->PreWork();
    static_cast<aarch64::Aarch64Encoder *>(test->GetEncoder())
        ->GetMasm()
        ->Push(vixl::aarch64::xzr, aarch64::VixlReg(linkReg));

    // First type-dependency
    auto param1 = test->GetParameter(TypeInfo(T(0)), 0);
    auto param2 = test->GetParameter(TypeInfo(T(0)), 1);

    // Main test call
    test->GetEncoder()->EncodeMod(param1, isSigned, param1, param2);

    // Finalize
    static_cast<aarch64::Aarch64Encoder *>(test->GetEncoder())
        ->GetMasm()
        ->Pop(aarch64::VixlReg(linkReg), vixl::aarch64::xzr);
    test->PostWork();

    // If encode unsupported - now print error
    if (!test->GetEncoder()->GetResult()) {
        std::cerr << "Unsupported for " << TypeName<T>() << std::endl;
        return false;
    }
    // Change this for enable print disasm
    test->Dump(false);
    return TestModMainLoop<T>(test);
}

TEST_F(Encoder64Test, ModTest)
{
    EXPECT_TRUE(TestMod<uint32_t>(this));
    EXPECT_TRUE(TestMod<uint64_t>(this));
    EXPECT_TRUE(TestMod<int32_t>(this));
    EXPECT_TRUE(TestMod<int64_t>(this));
    EXPECT_TRUE(TestMod<float>(this));
    EXPECT_TRUE(TestMod<double>(this));
}

// MemCopy Test
// TEST_F(Encoder64Test, MemCopyTest) {
//  EncodeMemCopy(MemRef mem_from, MemRef mem_to, size_t size)

// MemCopyz Test
// TEST_F(Encoder64Test, MemCopyzTest) {
//  EncodeMemCopyz(MemRef mem_from, MemRef mem_to, size_t size)

// int32_t uint64_t int32_t  int64_t         int32_t int32_t
//   r0    r2+r3   stack0  stack2(align)   stack4
using FunctionPtr = uint64_t (*)(uint32_t, uint64_t, int32_t, int64_t, int32_t, int32_t);

template <int ID, typename T>
bool TestParamMainLoop(FunctionPtr func)
{
    // Main test loop:
    for (uint64_t i = 0; i < ITERATION; ++i) {
        // Second type-dependency
        auto param0 = RandomGen<uint32_t>();
        auto param1 = RandomGen<uint64_t>();
        auto param2 = RandomGen<int32_t>();
        auto param3 = RandomGen<int64_t>();
        auto param4 = RandomGen<int32_t>();
        auto param5 = RandomGen<int32_t>();

        // Main check - compare parameter and
        // return value
        const T currResult = func(param0, param1, param2, param3, param4, param5);
        T result;
        if constexpr (ID == 0) {
            result = param0;
        }
        if constexpr (ID == 1) {
            result = param1;
        }
        if constexpr (ID == 2U) {
            result = param2;
        }
        if constexpr (ID == 3U) {
            result = param3;
        }
        if constexpr (ID == 4U) {
            result = param4;
        }
        if constexpr (ID == 5U) {
            result = param5;
        }
        if (currResult != result) {
            return false;
        };
    }
    return true;
}

template <int ID, typename T>
bool TestParam(Encoder64Test *test)
{
    bool isSigned = std::is_signed<T>::value;

    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    constexpr TypeInfo PARAMS[6] = {INT32_TYPE, INT64_TYPE, INT32_TYPE, INT64_TYPE, INT32_TYPE, INT32_TYPE};
    TypeInfo currParam = PARAMS[ID];

    auto paramInfo = test->GetCallconv()->GetParameterInfo(0);
    auto par = paramInfo->GetNativeParam(PARAMS[0]);
    // First ret value geted
    for (int i = 1; i <= ID; ++i) {
        par = paramInfo->GetNativeParam(PARAMS[i]);
    }

    auto linkReg = Target::Current().GetLinkReg();
    ArenaVector<Reg> usedRegs(test->GetAllocator()->Adapter());
    usedRegs.emplace_back(linkReg);
    test->PreWork();

    auto retReg = test->GetParameter(currParam, 0);

    // Main test call
    if (std::holds_alternative<Reg>(par)) {
        test->GetEncoder()->EncodeMov(retReg, std::get<Reg>(par));
    } else {
        auto mem = MemRef(Target::Current().GetStackReg(), std::get<uint8_t>(par) * sizeof(uint32_t));
        test->GetEncoder()->EncodeLdr(retReg, isSigned, mem);
    }

    // If encode unsupported - now print error
    if (!test->GetEncoder()->GetResult()) {
        std::cerr << "Unsupported parameter with " << ID << "\n";
        return false;
    }

    // Finalize
    test->PostWork();

    // Change this for enable print disasm
    test->Dump(false);

    auto size = test->GetCallconv()->GetCodeSize() - test->GetCursor();
    void *offset = (static_cast<uint8_t *>(test->GetCallconv()->GetCodeEntry()));
    void *ptr = test->GetCodeAllocator()->AllocateCode(size, offset);
    auto func = reinterpret_cast<FunctionPtr>(ptr);

    return TestParamMainLoop<ID, T>(func);
}

TEST_F(Encoder64Test, ReadParams)
{
    EXPECT_TRUE((TestParam<0, int32_t>(this)));
    EXPECT_TRUE((TestParam<1, uint64_t>(this)));
    EXPECT_TRUE((TestParam<2U, int32_t>(this)));
    EXPECT_TRUE((TestParam<3U, int64_t>(this)));
    EXPECT_TRUE((TestParam<4U, int32_t>(this)));
    EXPECT_TRUE((TestParam<5U, int32_t>(this)));
}

template <typename T, Condition CC>
bool TestSelect(Encoder64Test *test)
{
    // Initialize
    test->PreWork();

    // First type-dependency
    auto param0 = test->GetParameter(TypeInfo(T(0)), 0);
    auto param1 = test->GetParameter(TypeInfo(T(0)), 1);
    auto param2 = test->GetParameter(TypeInfo(uint32_t(0)), 2U);
    auto param3 = test->GetParameter(TypeInfo(uint32_t(0)), 3U);

    // Main test call
    test->GetEncoder()->EncodeMov(param2, Imm(1));
    test->GetEncoder()->EncodeMov(param3, Imm(0));
    test->GetEncoder()->EncodeSelect({Reg(param0.GetId(), TypeInfo(uint32_t(0))), param2, param3, param0, param1, CC});

    // Finalize
    test->PostWork();

    // If encode unsupported - now print error
    if (!test->GetEncoder()->GetResult()) {
        std::cerr << "Unsupported for " << TypeName<T>() << std::endl;
        return false;
    }
    // Change this for enable print disasm
    test->Dump(false);

    // Main test loop:
    for (uint64_t i = 0; i < ITERATION; ++i) {
        // Second type-dependency
        T tmp0 = RandomGen<T>();
        T tmp1 = RandomGen<T>();

        bool res {false};
        switch (CC) {
            case Condition::LT:
            case Condition::LO:
                res = tmp0 < tmp1;
                break;
            case Condition::EQ:
                res = tmp0 == tmp1;
                break;
            case Condition::NE:
                res = tmp0 != tmp1;
                break;
            case Condition::GT:
            case Condition::HI:
                res = tmp0 > tmp1;
                break;
            default:
                UNREACHABLE();
        }

        // Main check - compare parameter and
        // return value
        if (!test->CallCode<T, bool>(tmp0, tmp1, res)) {
            return false;
        }
    }
    return true;
}

TEST_F(Encoder64Test, SelectTest)
{
    EXPECT_TRUE((TestSelect<uint32_t, Condition::LO>(this)));
    EXPECT_TRUE((TestSelect<uint32_t, Condition::EQ>(this)));
    EXPECT_TRUE((TestSelect<uint32_t, Condition::NE>(this)));
    EXPECT_TRUE((TestSelect<uint32_t, Condition::HI>(this)));

    EXPECT_TRUE((TestSelect<uint64_t, Condition::LO>(this)));
    EXPECT_TRUE((TestSelect<uint64_t, Condition::EQ>(this)));
    EXPECT_TRUE((TestSelect<uint64_t, Condition::NE>(this)));
    EXPECT_TRUE((TestSelect<uint64_t, Condition::HI>(this)));

    EXPECT_TRUE((TestSelect<int32_t, Condition::LT>(this)));
    EXPECT_TRUE((TestSelect<int32_t, Condition::EQ>(this)));
    EXPECT_TRUE((TestSelect<int32_t, Condition::NE>(this)));
    EXPECT_TRUE((TestSelect<int32_t, Condition::GT>(this)));

    EXPECT_TRUE((TestSelect<int64_t, Condition::LT>(this)));
    EXPECT_TRUE((TestSelect<int64_t, Condition::EQ>(this)));
    EXPECT_TRUE((TestSelect<int64_t, Condition::NE>(this)));
    EXPECT_TRUE((TestSelect<int64_t, Condition::GT>(this)));
}

template <typename T, Condition CC, bool IS_IMM>
bool TestSelectTest(Encoder64Test *test)
{
    // Initialize
    test->PreWork();

    auto param0 = test->GetParameter(TypeInfo(T(0)), 0);
    auto param1 = test->GetParameter(TypeInfo(T(0)), 1);
    auto param2 = test->GetParameter(TypeInfo(uint32_t(0)), 2U);
    auto param3 = test->GetParameter(TypeInfo(uint32_t(0)), 3U);
    // truncate immediate value to max imm size
    [[maybe_unused]] T immValue = RandomMaskGen<T>();

    // Main test call
    test->GetEncoder()->EncodeMov(param2, Imm(1));
    test->GetEncoder()->EncodeMov(param3, Imm(0));

    if constexpr (IS_IMM) {
        test->GetEncoder()->EncodeSelectTest(
            {Reg(param0.GetId(), TypeInfo(uint32_t(0))), param2, param3, param0, Imm(immValue), CC});
    } else {
        test->GetEncoder()->EncodeSelectTest(
            {Reg(param0.GetId(), TypeInfo(uint32_t(0))), param2, param3, param0, param1, CC});
    }

    // Finalize
    test->PostWork();

    // If encode unsupported - now print error
    if (!test->GetEncoder()->GetResult()) {
        std::cerr << "Unsupported for " << TypeName<T>() << std::endl;
        return false;
    }
    // Change this for enable print disasm
    test->Dump(false);

    // Main test loop:
    for (uint64_t i = 0; i < ITERATION; ++i) {
        T tmp0 = RandomGen<T>();
        T tmp1;

        if constexpr (IS_IMM) {
            tmp1 = immValue;
        } else {
            tmp1 = RandomGen<T>();
        }

        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        T andRes = tmp0 & tmp1;
        bool res = CC == Condition::TST_EQ ? andRes == 0 : andRes != 0;

        // Main check - compare parameter and return value
        if (!test->CallCode<T, bool>(tmp0, tmp1, res)) {
            return false;
        }
    }
    return true;
}

TEST_F(Encoder64Test, SelectTestTest)
{
    EXPECT_TRUE((TestSelectTest<uint32_t, Condition::TST_EQ, false>(this)));
    EXPECT_TRUE((TestSelectTest<uint32_t, Condition::TST_NE, false>(this)));
    EXPECT_TRUE((TestSelectTest<uint32_t, Condition::TST_EQ, true>(this)));
    EXPECT_TRUE((TestSelectTest<uint32_t, Condition::TST_NE, true>(this)));

    EXPECT_TRUE((TestSelectTest<int32_t, Condition::TST_EQ, false>(this)));
    EXPECT_TRUE((TestSelectTest<int32_t, Condition::TST_NE, false>(this)));
    EXPECT_TRUE((TestSelectTest<int32_t, Condition::TST_EQ, true>(this)));
    EXPECT_TRUE((TestSelectTest<int32_t, Condition::TST_NE, true>(this)));

    EXPECT_TRUE((TestSelectTest<uint64_t, Condition::TST_EQ, false>(this)));
    EXPECT_TRUE((TestSelectTest<uint64_t, Condition::TST_NE, false>(this)));
    EXPECT_TRUE((TestSelectTest<uint64_t, Condition::TST_EQ, true>(this)));
    EXPECT_TRUE((TestSelectTest<uint64_t, Condition::TST_NE, true>(this)));

    EXPECT_TRUE((TestSelectTest<int64_t, Condition::TST_EQ, false>(this)));
    EXPECT_TRUE((TestSelectTest<int64_t, Condition::TST_NE, false>(this)));
    EXPECT_TRUE((TestSelectTest<int64_t, Condition::TST_EQ, true>(this)));
    EXPECT_TRUE((TestSelectTest<int64_t, Condition::TST_NE, true>(this)));
}

template <typename T, Condition CC>
bool TestCompareTest(Encoder64Test *test)
{
    // Initialize
    test->PreWork();

    // First type-dependency
    auto param1 = test->GetParameter(TypeInfo(T(0)), 0);
    auto param2 = test->GetParameter(TypeInfo(T(0)), 1);

    // Main test call
    test->GetEncoder()->EncodeCompareTest(param1, param1, param2, CC);

    // Finalize
    test->PostWork();

    // If encode unsupported - now print error
    if (!test->GetEncoder()->GetResult()) {
        std::cerr << "Unsupported for " << TypeName<T>() << std::endl;
        return false;
    }
    // Change this for enable print disasm
    test->Dump(false);

    // Main test loop:
    for (uint64_t i = 0; i < ITERATION; ++i) {
        // Second type-dependency
        T tmp1 = RandomGen<T>();
        T tmp2 = RandomGen<T>();
        // Deduced conflicting types for parameter

        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        auto compare = [](T a, T b) -> int32_t { return CC == Condition::TST_EQ ? (a & b) == 0 : (a & b) != 0; };

        // Main check - compare parameter and
        // return value
        if (!test->CallCode<T, int32_t>(tmp1, tmp2, compare(tmp1, tmp2))) {
            return false;
        }
    }
    return true;
}

TEST_F(Encoder64Test, CompareTestTest)
{
    EXPECT_TRUE((TestCompareTest<int32_t, Condition::TST_EQ>(this)));
    EXPECT_TRUE((TestCompareTest<int32_t, Condition::TST_NE>(this)));
    EXPECT_TRUE((TestCompareTest<int64_t, Condition::TST_EQ>(this)));
    EXPECT_TRUE((TestCompareTest<int64_t, Condition::TST_NE>(this)));
    EXPECT_TRUE((TestCompareTest<uint32_t, Condition::TST_EQ>(this)));
    EXPECT_TRUE((TestCompareTest<uint32_t, Condition::TST_NE>(this)));
    EXPECT_TRUE((TestCompareTest<uint64_t, Condition::TST_EQ>(this)));
    EXPECT_TRUE((TestCompareTest<uint64_t, Condition::TST_NE>(this)));
}

template <typename T, Condition CC, bool IS_IMM>
bool TestJumpTest(Encoder64Test *test)
{
    // Initialize
    test->PreWork();

    // First type-dependency
    auto param0 = test->GetParameter(TypeInfo(T(0)), 0);
    auto param1 = test->GetParameter(TypeInfo(T(0)), 1);
    auto retVal = Target::Current().GetParamReg(0);

    auto trueBranch = test->GetEncoder()->CreateLabel();
    auto end = test->GetEncoder()->CreateLabel();
    [[maybe_unused]] T immValue = RandomMaskGen<T>();

    // Main test call
    if constexpr (IS_IMM) {
        test->GetEncoder()->EncodeJumpTest(trueBranch, param0, Imm(immValue), CC);
    } else {
        test->GetEncoder()->EncodeJumpTest(trueBranch, param0, param1, CC);
    }
    test->GetEncoder()->EncodeMov(retVal, Imm(0));
    test->GetEncoder()->EncodeJump(end);

    test->GetEncoder()->BindLabel(trueBranch);
    test->GetEncoder()->EncodeMov(retVal, Imm(1));

    test->GetEncoder()->BindLabel(end);
    // Finalize
    test->PostWork();

    // If encode unsupported - now print error
    if (!test->GetEncoder()->GetResult()) {
        std::cerr << "Unsupported for " << TypeName<T>() << std::endl;
        return false;
    }
    // Change this for enable print disasm
    test->Dump(false);

    // Main test loop:
    for (uint64_t i = 0; i < ITERATION; ++i) {
        // Second type-dependency
        T tmp1 = RandomGen<T>();
        T tmp2;
        if constexpr (IS_IMM) {
            tmp2 = immValue;
        } else {
            tmp2 = RandomGen<T>();
        }
        // Deduced conflicting types for parameter

        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        auto compare = [](T a, T b) -> int32_t { return CC == Condition::TST_EQ ? (a & b) == 0 : (a & b) != 0; };

        // Main check - compare parameter and
        // return value
        if (!test->CallCode<T, int32_t>(tmp1, tmp2, compare(tmp1, tmp2))) {
            return false;
        }
    }
    return true;
}

TEST_F(Encoder64Test, JumpTestTest)
{
    EXPECT_TRUE((TestJumpTest<int32_t, Condition::TST_EQ, false>(this)));
    EXPECT_TRUE((TestJumpTest<int32_t, Condition::TST_NE, false>(this)));
    EXPECT_TRUE((TestJumpTest<int64_t, Condition::TST_EQ, false>(this)));
    EXPECT_TRUE((TestJumpTest<int64_t, Condition::TST_NE, false>(this)));
    EXPECT_TRUE((TestJumpTest<uint32_t, Condition::TST_EQ, false>(this)));
    EXPECT_TRUE((TestJumpTest<uint32_t, Condition::TST_NE, false>(this)));
    EXPECT_TRUE((TestJumpTest<uint64_t, Condition::TST_EQ, false>(this)));
    EXPECT_TRUE((TestJumpTest<uint64_t, Condition::TST_NE, false>(this)));

    EXPECT_TRUE((TestJumpTest<int32_t, Condition::TST_EQ, true>(this)));
    EXPECT_TRUE((TestJumpTest<int32_t, Condition::TST_NE, true>(this)));
    EXPECT_TRUE((TestJumpTest<int64_t, Condition::TST_EQ, true>(this)));
    EXPECT_TRUE((TestJumpTest<int64_t, Condition::TST_NE, true>(this)));
    EXPECT_TRUE((TestJumpTest<uint32_t, Condition::TST_EQ, true>(this)));
    EXPECT_TRUE((TestJumpTest<uint32_t, Condition::TST_NE, true>(this)));
    EXPECT_TRUE((TestJumpTest<uint64_t, Condition::TST_EQ, true>(this)));
    EXPECT_TRUE((TestJumpTest<uint64_t, Condition::TST_NE, true>(this)));
}

template <typename T, bool IS_ACQUIRE>
bool TestLoadExclusive(Encoder64Test *test, uint64_t inputWord, T expectedResult)
{
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    uint64_t buffer[3U] = {0xFFFFFFFFFFFFFFFFULL, inputWord, 0xFFFFFFFFFFFFFFFFULL};
    test->PreWork();
    auto param0 = test->GetParameter(TypeInfo(uintptr_t(0)), 0);
    auto result = test->GetParameter(TypeInfo(T(0)), 0);
    test->GetEncoder()->EncodeLdrExclusive(result, param0, IS_ACQUIRE);
    test->PostWork();

    if (!test->GetEncoder()->GetResult()) {
        std::cerr << "Unsupported for " << TypeName<T>() << std::endl;
        return false;
    }
    // Change this for enable print disasm
    test->Dump(false);
    auto wordAddr = reinterpret_cast<uintptr_t>(&buffer[1]);
    return test->CallCode<uintptr_t, T>(wordAddr, expectedResult);
}

template <typename T, bool IS_RELEASE>
bool TestStoreExclusiveFailed(Encoder64Test *test)
{
    uint64_t buffer = 0xFFFFFFFFFFFFFFFFULL;
    test->PreWork();
    auto param0 = test->GetParameter(TypeInfo(uintptr_t(0)), 0);
    auto tmpReg = test->GetParameter(TypeInfo(T(0)), 1);
    auto result = test->GetParameter(TypeInfo(T(0)), 0);

    // perform store without load - should fail as there are no exlusive monitor
    test->GetEncoder()->EncodeStrExclusive(result, tmpReg, param0, IS_RELEASE);
    test->PostWork();

    if (!test->GetEncoder()->GetResult()) {
        std::cerr << "Unsupported for " << TypeName<T>() << std::endl;
        return false;
    }
    // Change this for enable print disasm
    test->Dump(false);
    auto wordAddr = reinterpret_cast<uintptr_t>(&buffer);
    return test->CallCode<uintptr_t, T>(wordAddr, static_cast<T>(buffer), 1);
}

template <typename T, bool IS_RELEASE>
bool TestStoreExclusive(Encoder64Test *test, T value, uint64_t expectedResult)
{
    // test writes value into buffer[1]
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    uint64_t buffer[3U] = {0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL};
    test->PreWork();
    auto param0 = test->GetParameter(TypeInfo(uintptr_t(0)), 0);
    auto param1 = test->GetParameter(TypeInfo(T(0)), 1);
    auto result = test->GetParameter(TypeInfo(T(0)), 0);
    auto retryLabel = test->GetEncoder()->CreateLabel();
    Reg tmpReg(test->GetRegfile()->GetZeroReg().GetId(), TypeInfo(T(0)));

    test->GetEncoder()->BindLabel(retryLabel);
    test->GetEncoder()->EncodeLdrExclusive(tmpReg, param0, false);
    test->GetEncoder()->EncodeStrExclusive(result, param1, param0, IS_RELEASE);
    // retry if store exclusive returned non zero value
    test->GetEncoder()->EncodeJump(retryLabel, result, Condition::NE);

    test->PostWork();

    if (!test->GetEncoder()->GetResult()) {
        std::cerr << "Unsupported for " << TypeName<T>() << std::endl;
        return false;
    }
    // Change this for enable print disasm
    test->Dump(false);
    auto wordAddr = reinterpret_cast<uintptr_t>(&buffer[1]);
    if (!test->CallCode<uintptr_t, T>(wordAddr, value, 0)) {
        return false;
    }
    if (buffer[1] != expectedResult) {
        std::cerr << "Failed: expected value " << std::hex << expectedResult << " differs from actual result "
                  << buffer[1] << std::dec << std::endl;
        return false;
    }
    return true;
}

TEST_F(Encoder64Test, LoadExclusiveTest)
{
    const uint64_t sourceWord = 0x1122334455667788ULL;
    // aarch64 is little-endian, so bytes are actually stored in following order:
    // 0x 88 77 66 55 44 33 22 11
    EXPECT_TRUE((TestLoadExclusive<uint8_t, true>(this, sourceWord, 0x88U)));
    EXPECT_TRUE((TestLoadExclusive<uint16_t, true>(this, sourceWord, 0x7788U)));
    EXPECT_TRUE((TestLoadExclusive<uint32_t, true>(this, sourceWord, 0x55667788UL)));
    EXPECT_TRUE((TestLoadExclusive<uint64_t, true>(this, sourceWord, sourceWord)));

    EXPECT_TRUE((TestLoadExclusive<uint8_t, false>(this, sourceWord, 0x88U)));
    EXPECT_TRUE((TestLoadExclusive<uint16_t, false>(this, sourceWord, 0x7788U)));
    EXPECT_TRUE((TestLoadExclusive<uint32_t, false>(this, sourceWord, 0x55667788UL)));
    EXPECT_TRUE((TestLoadExclusive<uint64_t, false>(this, sourceWord, sourceWord)));
}

TEST_F(Encoder64Test, StoreExclusiveTest)
{
    EXPECT_TRUE((TestStoreExclusiveFailed<uint8_t, true>(this)));
    EXPECT_TRUE((TestStoreExclusiveFailed<uint16_t, true>(this)));
    EXPECT_TRUE((TestStoreExclusiveFailed<uint32_t, true>(this)));
    EXPECT_TRUE((TestStoreExclusiveFailed<uint64_t, true>(this)));

    EXPECT_TRUE((TestStoreExclusiveFailed<uint8_t, false>(this)));
    EXPECT_TRUE((TestStoreExclusiveFailed<uint16_t, false>(this)));
    EXPECT_TRUE((TestStoreExclusiveFailed<uint32_t, false>(this)));
    EXPECT_TRUE((TestStoreExclusiveFailed<uint64_t, false>(this)));

    EXPECT_TRUE((TestStoreExclusive<uint8_t, false>(this, 0x11U, 0xFFFFFFFFFFFFFF11ULL)));
    EXPECT_TRUE((TestStoreExclusive<uint16_t, false>(this, 0x1122U, 0xFFFFFFFFFFFF1122ULL)));
    EXPECT_TRUE((TestStoreExclusive<uint32_t, false>(this, 0x11223344UL, 0xFFFFFFFF11223344ULL)));
    EXPECT_TRUE((TestStoreExclusive<uint64_t, false>(this, 0xAABBCCDD11335577ULL, 0xAABBCCDD11335577ULL)));
}

class Encoder64ApiTest : public Encoder64Test {
public:
    Encoder64ApiTest() : decoder_(GetAllocator()), disasm_(GetAllocator())
    {
        decoder_.visitors()->push_back(&disasm_);
    }

    std::string &GetOutput(const aarch64::Aarch64Encoder &encoder)
    {
        output_.resize(0);
        auto begin = encoder.GetMasm()->GetBuffer()->GetOffsetAddress<uint8_t *>(0);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto end = begin + encoder.GetMasm()->GetSizeOfCodeGenerated();
        for (auto instr = reinterpret_cast<const vixl::aarch64::Instruction *>(begin);
             instr < reinterpret_cast<const vixl::aarch64::Instruction *>(end); instr = instr->GetNextInstruction()) {
            decoder_.Decode(instr);
            if (!output_.empty()) {
                output_ += '\n';
            }
            output_ += disasm_.GetOutput();
        }
        return output_;
    }
    void EncoderApiTestEncodeMov(Reg dst, Reg src, const char *asmStr);
    void EncoderApiTestEncodeLdr(Reg dst, bool dstSigned, MemRef ref, const char *asmStr);
    void EncoderApiTestEncodeLdrPreIndex(Reg dst, bool dstSigned, Reg srcBase, int offset, const char *asmStr);
    void EncoderApiTestEncodeLdrPostIndex(Reg dst, bool dstSigned, Reg srcBase, int offset, const char *asmStr);
    void EncoderApiTestEncodeStr(Reg src, MemRef ref, const char *asmStr);
    void EncoderApiTestEncodeStrPreIndex(Reg src, Reg dstBase, int offset, const char *asmStr);
    void EncoderApiTestEncodeStrPostIndex(Reg src, Reg dstBase, int offset, const char *asmStr);
    void EncoderApiTestEncodeLdpPreIndex(Reg dst0, Reg dst1, bool dstSigned, Reg srcBase, int offset,
                                         const char *asmStr);
    void EncoderApiTestEncodeLdpPostIndex(Reg dst0, Reg dst1, bool dstSigned, Reg srcBase, int offset,
                                          const char *asmStr);
    void EncoderApiTestEncodeStpPreIndex(Reg src0, Reg src1, Reg dstBase, int offset, const char *asmStr);
    void EncoderApiTestEncodeStpPostIndex(Reg src0, Reg src1, Reg dstBase, int offset, const char *asmStr);
    void EncoderApiTestEncodeMemCopy(size_t size, const char *asmStr);

private:
    std::string output_;
    vixl::aarch64::Decoder decoder_;
    vixl::aarch64::Disassembler disasm_;
};

void Encoder64ApiTest::EncoderApiTestEncodeMov(Reg dst, Reg src, const char *asmStr)
{
    aarch64::Aarch64Encoder encoder(GetAllocator());
    encoder.InitMasm();
    encoder.SetRegfile(GetRegfile());
    encoder.EncodeMov(dst, src);
    encoder.Finalize();
    EXPECT_TRUE(encoder.GetResult());
    EXPECT_STREQ(asmStr, GetOutput(encoder).c_str());
}

TEST_F(Encoder64ApiTest, TestEncodeMov)
{
    EncoderApiTestEncodeMov(Reg(0, INT32_TYPE), Reg(1, INT32_TYPE), "mov w0, w1");
    EncoderApiTestEncodeMov(Reg(0, INT32_TYPE), Reg(1, INT64_TYPE), "mov w0, w1");
    EncoderApiTestEncodeMov(Reg(0, INT64_TYPE), Reg(1, INT32_TYPE), "mov x0, x1");
    EncoderApiTestEncodeMov(Reg(0, INT32_TYPE), Reg(0, INT32_TYPE), "");
    EncoderApiTestEncodeMov(Reg(0, FLOAT64_TYPE), Reg(1, FLOAT64_TYPE), "fmov d0, d1");
    EncoderApiTestEncodeMov(Reg(0, FLOAT64_TYPE), Reg(1, FLOAT32_TYPE), "fcvt d0, s1");
    EncoderApiTestEncodeMov(Reg(0, INT32_TYPE), Reg(1, FLOAT32_TYPE), "fmov w0, s1");
    EncoderApiTestEncodeMov(Reg(0, FLOAT32_TYPE), Reg(1, INT32_TYPE), "fmov s0, w1");
}

void Encoder64ApiTest::EncoderApiTestEncodeLdr(Reg dst, bool dstSigned, MemRef ref, const char *asmStr)
{
    aarch64::Aarch64Encoder encoder(GetAllocator());
    encoder.InitMasm();
    encoder.SetRegfile(GetRegfile());
    encoder.EncodeLdr(dst, dstSigned, ref);
    encoder.Finalize();
    EXPECT_TRUE(encoder.GetResult());
    EXPECT_STREQ(asmStr, GetOutput(encoder).c_str());
}

TEST_F(Encoder64ApiTest, TestEncodeLdr)
{
    EncoderApiTestEncodeLdr(Reg(0, FLOAT64_TYPE), true, MemRef(Reg(0, INT64_TYPE)), "ldr d0, [x0]");
    EncoderApiTestEncodeLdr(Reg(0, INT8_TYPE), true, MemRef(Reg(0, INT64_TYPE)), "ldrsb x0, [x0]");
    EncoderApiTestEncodeLdr(Reg(0, INT16_TYPE), true, MemRef(Reg(0, INT64_TYPE)), "ldrsh w0, [x0]");
    EncoderApiTestEncodeLdr(Reg(0, INT32_TYPE), true, MemRef(Reg(0, INT64_TYPE)), "ldr w0, [x0]");
    EncoderApiTestEncodeLdr(Reg(0, INT8_TYPE), false, MemRef(Reg(0, INT64_TYPE)), "ldrb w0, [x0]");
    EncoderApiTestEncodeLdr(Reg(0, INT16_TYPE), false, MemRef(Reg(0, INT64_TYPE)), "ldrh w0, [x0]");
    EncoderApiTestEncodeLdr(Reg(0, INT32_TYPE), false, MemRef(Reg(0, INT64_TYPE)), "ldr w0, [x0]");
    EncoderApiTestEncodeLdr(Reg(0, INT64_TYPE), false, MemRef(Reg(0, INT64_TYPE)), "ldr x0, [x0]");
}

void Encoder64ApiTest::EncoderApiTestEncodeLdrPreIndex(Reg dst, bool dstSigned, Reg srcBase, int offset,
                                                       const char *asmStr)
{
    aarch64::Aarch64Encoder encoder(GetAllocator());
    encoder.InitMasm();
    encoder.SetRegfile(GetRegfile());
    encoder.EncodeLdrPreIndex(dst, dstSigned, srcBase, Imm(offset));
    encoder.Finalize();
    EXPECT_TRUE(encoder.GetResult());
    EXPECT_STREQ(asmStr, GetOutput(encoder).c_str());
}

TEST_F(Encoder64ApiTest, TestEncodeLdrPreIndex)
{
    EncoderApiTestEncodeLdrPreIndex(Reg(0, FLOAT64_TYPE), true, Reg(0, INT64_TYPE), 16, "ldr d0, [x0, #16]!");
    EncoderApiTestEncodeLdrPreIndex(Reg(0, INT8_TYPE), true, Reg(0, INT64_TYPE), 16, "ldrsb x0, [x0, #16]!");
    EncoderApiTestEncodeLdrPreIndex(Reg(0, INT16_TYPE), true, Reg(0, INT64_TYPE), 16, "ldrsh w0, [x0, #16]!");
    EncoderApiTestEncodeLdrPreIndex(Reg(0, INT32_TYPE), true, Reg(0, INT64_TYPE), 16, "ldr w0, [x0, #16]!");
    EncoderApiTestEncodeLdrPreIndex(Reg(0, INT8_TYPE), false, Reg(0, INT64_TYPE), 16, "ldrb w0, [x0, #16]!");
    EncoderApiTestEncodeLdrPreIndex(Reg(0, INT16_TYPE), false, Reg(0, INT64_TYPE), 16, "ldrh w0, [x0, #16]!");
    EncoderApiTestEncodeLdrPreIndex(Reg(0, INT32_TYPE), false, Reg(0, INT64_TYPE), 16, "ldr w0, [x0, #16]!");
    EncoderApiTestEncodeLdrPreIndex(Reg(0, INT64_TYPE), false, Reg(0, INT64_TYPE), 16, "ldr x0, [x0, #16]!");
}

void Encoder64ApiTest::EncoderApiTestEncodeLdrPostIndex(Reg dst, bool dstSigned, Reg srcBase, int offset,
                                                        const char *asmStr)
{
    aarch64::Aarch64Encoder encoder(GetAllocator());
    encoder.InitMasm();
    encoder.SetRegfile(GetRegfile());
    encoder.EncodeLdrPostIndex(dst, dstSigned, srcBase, Imm(offset));
    encoder.Finalize();
    EXPECT_TRUE(encoder.GetResult());
    EXPECT_STREQ(asmStr, GetOutput(encoder).c_str());
}

TEST_F(Encoder64ApiTest, TestEncodeLdrPostIndex)
{
    EncoderApiTestEncodeLdrPostIndex(Reg(0, FLOAT64_TYPE), true, Reg(0, INT64_TYPE), 16, "ldr d0, [x0], #16");
    EncoderApiTestEncodeLdrPostIndex(Reg(0, INT8_TYPE), true, Reg(0, INT64_TYPE), 16, "ldrsb x0, [x0], #16");
    EncoderApiTestEncodeLdrPostIndex(Reg(0, INT16_TYPE), true, Reg(0, INT64_TYPE), 16, "ldrsh w0, [x0], #16");
    EncoderApiTestEncodeLdrPostIndex(Reg(0, INT32_TYPE), true, Reg(0, INT64_TYPE), 16, "ldr w0, [x0], #16");
    EncoderApiTestEncodeLdrPostIndex(Reg(0, INT8_TYPE), false, Reg(0, INT64_TYPE), 16, "ldrb w0, [x0], #16");
    EncoderApiTestEncodeLdrPostIndex(Reg(0, INT16_TYPE), false, Reg(0, INT64_TYPE), 16, "ldrh w0, [x0], #16");
    EncoderApiTestEncodeLdrPostIndex(Reg(0, INT32_TYPE), false, Reg(0, INT64_TYPE), 16, "ldr w0, [x0], #16");
    EncoderApiTestEncodeLdrPostIndex(Reg(0, INT64_TYPE), false, Reg(0, INT64_TYPE), 16, "ldr x0, [x0], #16");
}

void Encoder64ApiTest::EncoderApiTestEncodeStr(Reg src, MemRef ref, const char *asmStr)
{
    aarch64::Aarch64Encoder encoder(GetAllocator());
    encoder.InitMasm();
    encoder.SetRegfile(GetRegfile());
    encoder.EncodeStr(src, ref);
    encoder.Finalize();
    EXPECT_TRUE(encoder.GetResult());
    EXPECT_STREQ(asmStr, GetOutput(encoder).c_str());
}

TEST_F(Encoder64ApiTest, TestEncodeStr)
{
    EncoderApiTestEncodeStr(Reg(0, FLOAT64_TYPE), MemRef(Reg(0, INT64_TYPE)), "str d0, [x0]");
    EncoderApiTestEncodeStr(Reg(0, INT8_TYPE), MemRef(Reg(0, INT64_TYPE)), "strb w0, [x0]");
    EncoderApiTestEncodeStr(Reg(0, INT16_TYPE), MemRef(Reg(0, INT64_TYPE)), "strh w0, [x0]");
    EncoderApiTestEncodeStr(Reg(0, INT32_TYPE), MemRef(Reg(0, INT64_TYPE)), "str w0, [x0]");
    EncoderApiTestEncodeStr(Reg(0, INT64_TYPE), MemRef(Reg(0, INT64_TYPE)), "str x0, [x0]");
}

void Encoder64ApiTest::EncoderApiTestEncodeStrPreIndex(Reg src, Reg dstBase, int offset, const char *asmStr)
{
    aarch64::Aarch64Encoder encoder(GetAllocator());
    encoder.InitMasm();
    encoder.SetRegfile(GetRegfile());
    encoder.EncodeStrPreIndex(src, dstBase, Imm(offset));
    encoder.Finalize();
    EXPECT_TRUE(encoder.GetResult());
    EXPECT_STREQ(asmStr, GetOutput(encoder).c_str());
}

TEST_F(Encoder64ApiTest, TestEncodeStrPreIndex)
{
    EncoderApiTestEncodeStrPreIndex(Reg(0, FLOAT64_TYPE), Reg(0, INT64_TYPE), -16, "str d0, [x0, #-16]!");
    EncoderApiTestEncodeStrPreIndex(Reg(0, INT8_TYPE), Reg(0, INT64_TYPE), -16, "strb w0, [x0, #-16]!");
    EncoderApiTestEncodeStrPreIndex(Reg(0, INT16_TYPE), Reg(0, INT64_TYPE), -16, "strh w0, [x0, #-16]!");
    EncoderApiTestEncodeStrPreIndex(Reg(0, INT32_TYPE), Reg(0, INT64_TYPE), -16, "str w0, [x0, #-16]!");
    EncoderApiTestEncodeStrPreIndex(Reg(0, INT64_TYPE), Reg(0, INT64_TYPE), -16, "str x0, [x0, #-16]!");
}

void Encoder64ApiTest::EncoderApiTestEncodeStrPostIndex(Reg src, Reg dstBase, int offset, const char *asmStr)
{
    aarch64::Aarch64Encoder encoder(GetAllocator());
    encoder.InitMasm();
    encoder.SetRegfile(GetRegfile());
    encoder.EncodeStrPostIndex(src, dstBase, Imm(offset));
    encoder.Finalize();
    EXPECT_TRUE(encoder.GetResult());
    EXPECT_STREQ(asmStr, GetOutput(encoder).c_str());
}

TEST_F(Encoder64ApiTest, TestEncodeStrPostIndex)
{
    EncoderApiTestEncodeStrPostIndex(Reg(0, FLOAT64_TYPE), Reg(0, INT64_TYPE), -16, "str d0, [x0], #-16");
    EncoderApiTestEncodeStrPostIndex(Reg(0, INT8_TYPE), Reg(0, INT64_TYPE), -16, "strb w0, [x0], #-16");
    EncoderApiTestEncodeStrPostIndex(Reg(0, INT16_TYPE), Reg(0, INT64_TYPE), -16, "strh w0, [x0], #-16");
    EncoderApiTestEncodeStrPostIndex(Reg(0, INT32_TYPE), Reg(0, INT64_TYPE), -16, "str w0, [x0], #-16");
    EncoderApiTestEncodeStrPostIndex(Reg(0, INT64_TYPE), Reg(0, INT64_TYPE), -16, "str x0, [x0], #-16");
}

void Encoder64ApiTest::EncoderApiTestEncodeLdpPreIndex(Reg dst0, Reg dst1, bool dstSigned, Reg srcBase, int offset,
                                                       const char *asmStr)
{
    aarch64::Aarch64Encoder encoder(GetAllocator());
    encoder.InitMasm();
    encoder.SetRegfile(GetRegfile());
    encoder.EncodeLdpPreIndex(dst0, dst1, dstSigned, srcBase, Imm(offset));
    encoder.Finalize();
    EXPECT_TRUE(encoder.GetResult());
    EXPECT_STREQ(asmStr, GetOutput(encoder).c_str());
}

TEST_F(Encoder64ApiTest, TestEncodeLdpPreIndex)
{
    EncoderApiTestEncodeLdpPreIndex(Reg(1, FLOAT64_TYPE), Reg(2, FLOAT64_TYPE), true, Reg(0, INT64_TYPE), 16,
                                    "ldp d1, d2, [x0, #16]!");
    EncoderApiTestEncodeLdpPreIndex(Reg(1, FLOAT32_TYPE), Reg(2, FLOAT32_TYPE), true, Reg(0, INT64_TYPE), 16,
                                    "ldp s1, s2, [x0, #16]!");
    EncoderApiTestEncodeLdpPreIndex(Reg(1, INT32_TYPE), Reg(2, INT32_TYPE), true, Reg(0, INT64_TYPE), 16,
                                    "ldpsw x1, x2, [x0, #16]!");
    EncoderApiTestEncodeLdpPreIndex(Reg(1, INT32_TYPE), Reg(2, INT32_TYPE), false, Reg(0, INT64_TYPE), 16,
                                    "ldp w1, w2, [x0, #16]!");
    EncoderApiTestEncodeLdpPreIndex(Reg(1, INT64_TYPE), Reg(2, INT64_TYPE), false, Reg(0, INT64_TYPE), 16,
                                    "ldp x1, x2, [x0, #16]!");
}

void Encoder64ApiTest::EncoderApiTestEncodeLdpPostIndex(Reg dst0, Reg dst1, bool dstSigned, Reg srcBase, int offset,
                                                        const char *asmStr)
{
    aarch64::Aarch64Encoder encoder(GetAllocator());
    encoder.InitMasm();
    encoder.SetRegfile(GetRegfile());
    encoder.EncodeLdpPostIndex(dst0, dst1, dstSigned, srcBase, Imm(offset));
    encoder.Finalize();
    EXPECT_TRUE(encoder.GetResult());
    EXPECT_STREQ(asmStr, GetOutput(encoder).c_str());
}

TEST_F(Encoder64ApiTest, TestEncodeLdpPostIndex)
{
    EncoderApiTestEncodeLdpPostIndex(Reg(1, FLOAT64_TYPE), Reg(2, FLOAT64_TYPE), true, Reg(0, INT64_TYPE), 16,
                                     "ldp d1, d2, [x0], #16");
    EncoderApiTestEncodeLdpPostIndex(Reg(1, FLOAT32_TYPE), Reg(2, FLOAT32_TYPE), true, Reg(0, INT64_TYPE), 16,
                                     "ldp s1, s2, [x0], #16");
    EncoderApiTestEncodeLdpPostIndex(Reg(1, INT32_TYPE), Reg(2, INT32_TYPE), true, Reg(0, INT64_TYPE), 16,
                                     "ldpsw x1, x2, [x0], #16");
    EncoderApiTestEncodeLdpPostIndex(Reg(1, INT32_TYPE), Reg(2, INT32_TYPE), false, Reg(0, INT64_TYPE), 16,
                                     "ldp w1, w2, [x0], #16");
    EncoderApiTestEncodeLdpPostIndex(Reg(1, INT64_TYPE), Reg(2, INT64_TYPE), false, Reg(0, INT64_TYPE), 16,
                                     "ldp x1, x2, [x0], #16");
}

void Encoder64ApiTest::EncoderApiTestEncodeStpPreIndex(Reg src0, Reg src1, Reg dstBase, int offset, const char *asmStr)
{
    aarch64::Aarch64Encoder encoder(GetAllocator());
    encoder.InitMasm();
    encoder.SetRegfile(GetRegfile());
    encoder.EncodeStpPreIndex(src0, src1, dstBase, Imm(offset));
    encoder.Finalize();
    EXPECT_TRUE(encoder.GetResult());
    EXPECT_STREQ(asmStr, GetOutput(encoder).c_str());
}

TEST_F(Encoder64ApiTest, TestEncodeStpPreIndex)
{
    EncoderApiTestEncodeStpPreIndex(Reg(1, FLOAT64_TYPE), Reg(2, FLOAT64_TYPE), Reg(0, INT64_TYPE), 16,
                                    "stp d1, d2, [x0, #16]!");
    EncoderApiTestEncodeStpPreIndex(Reg(1, FLOAT32_TYPE), Reg(2, FLOAT32_TYPE), Reg(0, INT64_TYPE), 16,
                                    "stp s1, s2, [x0, #16]!");
    EncoderApiTestEncodeStpPreIndex(Reg(1, INT32_TYPE), Reg(2, INT32_TYPE), Reg(0, INT64_TYPE), 16,
                                    "stp w1, w2, [x0, #16]!");
    EncoderApiTestEncodeStpPreIndex(Reg(1, INT64_TYPE), Reg(2, INT64_TYPE), Reg(0, INT64_TYPE), 16,
                                    "stp x1, x2, [x0, #16]!");
}

void Encoder64ApiTest::EncoderApiTestEncodeStpPostIndex(Reg src0, Reg src1, Reg dstBase, int offset, const char *asmStr)
{
    aarch64::Aarch64Encoder encoder(GetAllocator());
    encoder.InitMasm();
    encoder.SetRegfile(GetRegfile());
    encoder.EncodeStpPostIndex(src0, src1, dstBase, Imm(offset));
    encoder.Finalize();
    EXPECT_TRUE(encoder.GetResult());
    EXPECT_STREQ(asmStr, GetOutput(encoder).c_str());
}

TEST_F(Encoder64ApiTest, TestEncodeStpPostIndex)
{
    EncoderApiTestEncodeStpPostIndex(Reg(1, FLOAT64_TYPE), Reg(2, FLOAT64_TYPE), Reg(0, INT64_TYPE), 16,
                                     "stp d1, d2, [x0], #16");
    EncoderApiTestEncodeStpPostIndex(Reg(1, FLOAT32_TYPE), Reg(2, FLOAT32_TYPE), Reg(0, INT64_TYPE), 16,
                                     "stp s1, s2, [x0], #16");
    EncoderApiTestEncodeStpPostIndex(Reg(1, INT32_TYPE), Reg(2, INT32_TYPE), Reg(0, INT64_TYPE), 16,
                                     "stp w1, w2, [x0], #16");
    EncoderApiTestEncodeStpPostIndex(Reg(1, INT64_TYPE), Reg(2, INT64_TYPE), Reg(0, INT64_TYPE), 16,
                                     "stp x1, x2, [x0], #16");
}

void Encoder64ApiTest::EncoderApiTestEncodeMemCopy(size_t size, const char *asmStr)
{
    aarch64::Aarch64Encoder encoder(GetAllocator());
    encoder.InitMasm();
    encoder.SetRegfile(GetRegfile());
    encoder.EncodeMemCopy(MemRef(Reg(0, INT64_TYPE)), MemRef(Reg(1, INT64_TYPE)), size);
    encoder.Finalize();
    EXPECT_TRUE(encoder.GetResult());
    EXPECT_STREQ(asmStr, GetOutput(encoder).c_str());
}

TEST_F(Encoder64ApiTest, TestEncodeMemCopy)
{
    EncoderApiTestEncodeMemCopy(BYTE_SIZE, "ldrb w16, [x0]\nstrb w16, [x1]");
    EncoderApiTestEncodeMemCopy(HALF_SIZE, "ldrh w16, [x0]\nstrh w16, [x1]");
    EncoderApiTestEncodeMemCopy(WORD_SIZE, "ldr w16, [x0]\nstr w16, [x1]");
    EncoderApiTestEncodeMemCopy(DOUBLE_WORD_SIZE, "ldr x16, [x0]\nstr x16, [x1]");
}

TEST_F(Encoder64Test, Registers)
{
    auto target = GetEncoder()->GetTarget();
    {
        {
            ScopedTmpReg tmp1(GetEncoder(), true);
            ScopedTmpReg tmp2(GetEncoder(), true);
            ASSERT_EQ(tmp1.GetReg(), target.GetLinkReg());
            ASSERT_NE(tmp2.GetReg(), target.GetLinkReg());
        }
        ScopedTmpReg tmp3(GetEncoder(), true);
        ASSERT_EQ(tmp3.GetReg(), target.GetLinkReg());
    }
    {
        ScopedTmpReg tmp1(GetEncoder(), false);
        ASSERT_NE(tmp1.GetReg(), target.GetLinkReg());
    }
}
// NOLINTEND(readability-magic-numbers)

}  // namespace ark::compiler
