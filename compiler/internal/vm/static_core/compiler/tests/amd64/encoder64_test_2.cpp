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

#include "libarkbase/utils/utils.h"
#include "encoder64_test.h"

namespace ark::compiler {

// NOLINTBEGIN(readability-magic-numbers)
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
    test->PostWork<T>();

    // If encode unsupported - now print error
    if (!test->GetEncoder()->GetResult()) {
        std::cerr << "Unsupported for " << TypeName<T>() << "\n";
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
    test->PostWork<Dst>();

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
        DstExt dst {};

        if (std::is_floating_point<Src>() && !std::is_floating_point<Dst>()) {
            auto minInt = std::numeric_limits<Dst>::min();
            auto maxInt = std::numeric_limits<Dst>::max();
            auto floatMinInt = static_cast<Src>(minInt);
            auto floatMaxInt = static_cast<Src>(maxInt);

            if (src > floatMinInt) {
                dst = src < floatMaxInt ? static_cast<Dst>(src) : maxInt;
            } else if (std::isnan(src)) {
                dst = 0;
            } else {
                dst = minInt;
            }
        } else {
            dst = static_cast<DstExt>(static_cast<Dst>(src));
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
        if (!test->CallCode<Src, DstExt>(nan, Dst(0))) {
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
    EXPECT_TRUE((TestCast<float, uint32_t>(this)));
    EXPECT_TRUE((TestCast<float, uint64_t>(this)));

    EXPECT_TRUE((TestCast<double, int32_t>(this)));
    EXPECT_TRUE((TestCast<double, int64_t>(this)));
    EXPECT_TRUE((TestCast<double, uint32_t>(this)));
    EXPECT_TRUE((TestCast<double, uint64_t>(this)));
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
    test->PostWork<T>();

    // If encode unsupported - now print error
    if (!test->GetEncoder()->GetResult()) {
        std::cerr << "Unsupported for " << TypeName<T>() << "\n";
        return false;
    }
    // Change this for enable print disasm
    test->Dump(false);

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
    bool isSigned = std::is_signed<T>::value;
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
        test->GetEncoder()->EncodeDiv(param1, param1, Imm(imm), isSigned);

        // Finalize
        test->PostWork<T>();

        // If encode unsupported - now print error
        if (!test->GetEncoder()->GetResult()) {
            std::cerr << "Unsupported for " << TypeName<T>() << "\n";
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
bool TestMod(Encoder64Test *test)
{
    bool isSigned = std::is_signed<T>::value;

    test->PreWork();

    // First type-dependency
    auto param1 = test->GetParameter(TypeInfo(T(0)), 0);
    auto param2 = test->GetParameter(TypeInfo(T(0)), 1);

    // Main test call
    test->GetEncoder()->EncodeMod(param1, isSigned, param1, param2);

    // Finalize
    test->PostWork<T>();

    // If encode unsupported - now print error
    if (!test->GetEncoder()->GetResult()) {
        std::cerr << "Unsupported for " << TypeName<T>() << "\n";
        return false;
    }
    // Change this for enable print disasm
    test->Dump(false);

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
            if (!test->CallCode<T>(tmp1, tmp2, fmodf(tmp1, tmp2))) {
                return false;
            }
        } else if constexpr (std::is_same<double, T>::value) {
            if (!test->CallCode<T>(tmp1, tmp2, fmod(tmp1, tmp2))) {
                return false;
            }
        } else {
            if (!test->CallCode<T>(tmp1, tmp2, static_cast<T>(tmp1 % tmp2))) {
                return false;
            }
        }
    }

    bool res = true;
    if constexpr (std::is_floating_point_v<T>) {
        T nan = std::numeric_limits<T>::quiet_NaN();
        res &= test->CallCode<T>(nan, RandomGen<T>(), nan);
        res &= test->CallCode<T>(RandomGen<T>(), nan, nan);
        res &= test->CallCode<T>(0.0, 0.0, nan);
        res &= test->CallCode<T>(std::numeric_limits<T>::infinity(), std::numeric_limits<T>::infinity(), nan);
        res &= test->CallCode<T>(-std::numeric_limits<T>::infinity(), std::numeric_limits<T>::infinity(), nan);
    }

    return res;
}

TEST_F(Encoder64Test, ModTest)
{
    EXPECT_TRUE(TestMod<int32_t>(this));
    EXPECT_TRUE(TestMod<int64_t>(this));
    EXPECT_TRUE(TestMod<uint32_t>(this));
    EXPECT_TRUE(TestMod<uint64_t>(this));
    EXPECT_TRUE(TestMod<float>(this));
    EXPECT_TRUE(TestMod<double>(this));
}

template <typename T>
bool TestModImm(Encoder64Test *test)
{
    static_assert(std::is_integral_v<T>);
    bool isSigned = std::is_signed<T>::value;
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
        test->PostWork<T>();

        // If encode unsupported - now print error
        if (!test->GetEncoder()->GetResult()) {
            std::cerr << "Unsupported for " << TypeName<T>() << "\n";
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

    constexpr std::array<TypeInfo, 6U> PARAMS {INT32_TYPE, INT64_TYPE, INT32_TYPE, INT64_TYPE, INT32_TYPE, INT32_TYPE};
    TypeInfo currParam = PARAMS[ID];

    auto paramInfo = test->GetCallconv()->GetParameterInfo(0);
    auto par = paramInfo->GetNativeParam(PARAMS[0]);
    // First ret value geted
    for (int i = 1; i <= ID; ++i) {
        par = paramInfo->GetNativeParam(PARAMS[i]);
    }

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
    test->PostWork<T>();

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
    EXPECT_TRUE((TestParam<2_I, int32_t>(this)));
    EXPECT_TRUE((TestParam<3_I, int64_t>(this)));
    EXPECT_TRUE((TestParam<4_I, int32_t>(this)));
    EXPECT_TRUE((TestParam<5_I, int32_t>(this)));
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
    test->PostWork<bool>();

    // If encode unsupported - now print error
    if (!test->GetEncoder()->GetResult()) {
        std::cerr << "Unsupported for " << TypeName<T>() << "\n";
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
    [[maybe_unused]] T immValue = RandomGen<T>();

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
    test->PostWork<bool>();

    // If encode unsupported - now print error
    if (!test->GetEncoder()->GetResult()) {
        std::cerr << "Unsupported for " << TypeName<T>() << "\n";
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

        T andRes = tmp0 & tmp1;                                          // NOLINT(hicpp-signed-bitwise)
        bool res = CC == Condition::TST_EQ ? andRes == 0 : andRes != 0;  // NOLINT(hicpp-signed-bitwise)

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
    test->PostWork<T>();

    // If encode unsupported - now print error
    if (!test->GetEncoder()->GetResult()) {
        std::cerr << "Unsupported for " << TypeName<T>() << "\n";
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
    [[maybe_unused]] T immValue = RandomGen<T>();

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
    test->PostWork<T>();

    // If encode unsupported - now print error
    if (!test->GetEncoder()->GetResult()) {
        std::cerr << "Unsupported for " << TypeName<T>() << "\n";
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
// NOLINTEND(readability-magic-numbers)

}  // namespace ark::compiler
