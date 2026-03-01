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

namespace ark::compiler {
// NOLINTBEGIN(readability-magic-numbers)
template <typename T>
bool TestNeg(Encoder64Test *test)
{
    // Initialize
    test->PreWork();
    // First type-dependency
    auto param = test->GetParameter(TypeInfo(T(0)));
    // Main test call
    test->GetEncoder()->EncodeNeg(param, param);
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
        T tmp = RandomGen<T>();
        // Deduced conflicting types for parameter
        // Main check - compare parameter and
        // return value
        if (!test->CallCode<T>(tmp, -tmp)) {
            return false;
        }
    }

    if constexpr (std::is_floating_point_v<T>) {
        T nan = std::numeric_limits<T>::quiet_NaN();
        if (!test->CallCode<T>(nan, nan)) {
            return false;
        }
    }

    return true;
}

TEST_F(Encoder64Test, NegTest)
{
    EXPECT_TRUE(TestNeg<int32_t>(this));
    EXPECT_TRUE(TestNeg<int64_t>(this));
    EXPECT_TRUE(TestNeg<float>(this));
    EXPECT_TRUE(TestNeg<double>(this));
}
template <typename T>
bool TestNot(Encoder64Test *test)
{
    // Initialize
    test->PreWork();
    // First type-dependency
    auto param = test->GetParameter(TypeInfo(T(0)));
    // Main test call
    test->GetEncoder()->EncodeNot(param, param);
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
        T tmp = RandomGen<T>();
        // Deduced conflicting types for parameter
        // Main check - compare parameter and
        // return value
        if (!test->CallCode<T>(tmp, ~tmp)) {  // NOLINT(hicpp-signed-bitwise)
            return false;
        }
    }
    return true;
}

TEST_F(Encoder64Test, NotTest)
{
    EXPECT_TRUE(TestNot<int32_t>(this));
    EXPECT_TRUE(TestNot<int64_t>(this));
    EXPECT_TRUE(TestNot<uint32_t>(this));
    EXPECT_TRUE(TestNot<uint64_t>(this));
}

template <typename T>
bool TestMov(Encoder64Test *test)
{
    // Initialize
    test->PreWork();
    // First type-dependency
    auto param = test->GetParameter(TypeInfo(T(0)));
    // Main test call
    test->GetEncoder()->EncodeMov(param, param);
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
        T tmp = RandomGen<T>();
        // Deduced conflicting types for parameter
        // Main check - compare parameter and
        // return value
        if (!test->CallCode<T>(tmp, tmp)) {
            return false;
        }
    }

    if constexpr (std::is_floating_point_v<T>) {
        T nan = std::numeric_limits<T>::quiet_NaN();
        if (!test->CallCode<T>(nan, nan)) {
            return false;
        }
    }

    return true;
}

template <typename Src, typename Dst>
bool TestMov2(Encoder64Test *test)
{
    static_assert(sizeof(Src) == sizeof(Dst));
    // Initialize
    test->PreWork();
    // First type-dependency
    auto input = test->GetParameter(TypeInfo(Src(0)), 0);
    auto output = test->GetParameter(TypeInfo(Dst(0)), 0);
    // Main test call
    test->GetEncoder()->EncodeMov(output, input);
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
        // Second type-dependency
        Src src = RandomGen<Src>();
        Dst dst = bit_cast<Dst>(src);
        // Deduced conflicting types for parameter
        // Main check - compare parameter and
        // return value
        if (!test->CallCode<Src, Dst>(src, dst)) {
            return false;
        }
    }

    if constexpr (std::is_floating_point_v<Src>) {
        Src nan = std::numeric_limits<Src>::quiet_NaN();
        Dst dstNan = bit_cast<Dst>(nan);
        if (!test->CallCode<Src, Dst>(nan, dstNan)) {
            return false;
        }
    }

    return true;
}

TEST_F(Encoder64Test, MovTest)
{
    EXPECT_TRUE(TestMov<int32_t>(this));
    EXPECT_TRUE(TestMov<int64_t>(this));
    EXPECT_TRUE(TestMov<float>(this));
    EXPECT_TRUE(TestMov<double>(this));

    EXPECT_TRUE((TestMov2<float, int32_t>(this)));
    EXPECT_TRUE((TestMov2<double, int64_t>(this)));
    EXPECT_TRUE((TestMov2<int32_t, float>(this)));
    EXPECT_TRUE((TestMov2<int64_t, double>(this)));

    // NOTE (igorban) : add MOVI instructions
    // & support uint64_t mov
}

// Jump w/o CC
TEST_F(Encoder64Test, JumpTest)
{
    // Test for
    // EncodeJump(LabelHolder::LabelId label)
    PreWork();

    auto param = Target::Current().GetParamReg(0);

    auto t1 = GetEncoder()->CreateLabel();
    auto t2 = GetEncoder()->CreateLabel();
    auto t3 = GetEncoder()->CreateLabel();
    auto t4 = GetEncoder()->CreateLabel();
    auto t5 = GetEncoder()->CreateLabel();

    GetEncoder()->EncodeAdd(param, param, Imm(0x1));
    GetEncoder()->EncodeJump(t1);
    GetEncoder()->EncodeMov(param, Imm(0x0));
    GetEncoder()->EncodeReturn();
    // T4
    GetEncoder()->BindLabel(t4);
    GetEncoder()->EncodeAdd(param, param, Imm(0x1));
    GetEncoder()->EncodeJump(t5);
    // Fail value
    GetEncoder()->EncodeMov(param, Imm(0x0));
    GetEncoder()->EncodeReturn();

    // T2
    GetEncoder()->BindLabel(t2);
    GetEncoder()->EncodeAdd(param, param, Imm(0x1));
    GetEncoder()->EncodeJump(t3);
    // Fail value
    GetEncoder()->EncodeMov(param, Imm(0x0));
    GetEncoder()->EncodeReturn();
    // T3
    GetEncoder()->BindLabel(t3);
    GetEncoder()->EncodeAdd(param, param, Imm(0x1));
    GetEncoder()->EncodeJump(t4);
    // Fail value
    GetEncoder()->EncodeMov(param, Imm(0x0));
    GetEncoder()->EncodeReturn();
    // T1
    GetEncoder()->BindLabel(t1);
    GetEncoder()->EncodeAdd(param, param, Imm(0x1));
    GetEncoder()->EncodeJump(t2);
    // Fail value
    GetEncoder()->EncodeMov(param, Imm(0x0));
    GetEncoder()->EncodeReturn();
    // Sucess exit
    GetEncoder()->BindLabel(t5);
    PostWork();

    if (!GetEncoder()->GetResult()) {
        std::cerr << "Unsupported \n";
        return;
    }
    Dump(false);
    for (uint64_t i = 0; i < ITERATION; ++i) {
        // Second type-dependency
        auto tmp = RandomGen<int32_t>();
        // Deduced conflicting types for parameter
        EXPECT_TRUE(CallCode<int32_t>(tmp, tmp + 5_I));
    }
}

template <typename T, bool NOT_ZERO = false>
bool TestBitTestAndBranch(Encoder64Test *test, T value, int pos, uint32_t expected)
{
    test->PreWork();
    auto param = test->GetParameter(TypeInfo(T(0)));
    auto retVal = Target::Current().GetReturnReg();
    auto label = test->GetEncoder()->CreateLabel();

    if constexpr (NOT_ZERO) {
        test->GetEncoder()->EncodeBitTestAndBranch(label, param, pos, true);
    } else {
        test->GetEncoder()->EncodeBitTestAndBranch(label, param, pos, false);
    }
    test->GetEncoder()->EncodeMov(retVal, Imm(1));
    test->GetCallconv()->GenerateEpilogue(FrameInfo::FullPrologue(), []() {});

    test->GetEncoder()->BindLabel(label);
    test->GetEncoder()->EncodeMov(retVal, Imm(0));

    test->PostWork();

    if (!test->GetEncoder()->GetResult()) {
        std::cerr << "Unsupported for " << TypeName<T>() << std::endl;
        return false;
    }
    // Change this for enable print disasm
    test->Dump(false);

    if (!test->CallCode<T>(value, expected)) {
        std::cerr << "Bit " << pos << " of 0x" << std::hex << value << " is not equal to " << std::dec << expected
                  << std::endl;
        return false;
    }

    return true;
}

template <typename T, bool NOT_ZERO = false>
bool TestBitTestAndBranch(Encoder64Test *test)
{
    size_t maxPos = std::is_same<uint64_t, T>::value ? 64U : 32U;
    for (size_t i = 0; i < maxPos; i++) {
        T value = static_cast<T>(1UL << i);
        if (!TestBitTestAndBranch<T, NOT_ZERO>(test, value, i, NOT_ZERO ? 0 : 1)) {
            return false;
        }
        if (!TestBitTestAndBranch<T, NOT_ZERO>(test, ~value, i, NOT_ZERO ? 1 : 0)) {
            return false;
        }
    }
    return true;
}

template <typename T, bool IS_EQUAL = true>
bool TestAddOverflow(Encoder64Test *test, T value1, T value2, T expected)
{
    test->PreWork();
    auto param1 = test->GetParameter(TypeInfo(T(0)), 0);
    auto param2 = test->GetParameter(TypeInfo(T(0)), 1);
    auto retVal = Target::Current().GetReturnReg(TypeInfo(T(0)));
    auto label = test->GetEncoder()->CreateLabel();
    auto end = test->GetEncoder()->CreateLabel();

    if (IS_EQUAL) {
        test->GetEncoder()->EncodeAddOverflow(label, retVal, param1, param2, Condition::VS);
        test->GetEncoder()->EncodeJump(end);
        test->GetEncoder()->BindLabel(label);
        test->GetEncoder()->EncodeMov(retVal, Imm(0));
    } else {
        test->GetEncoder()->EncodeAddOverflow(end, retVal, param1, param2, Condition::VC);
        test->GetEncoder()->EncodeMov(retVal, Imm(0));
    }

    test->GetEncoder()->BindLabel(end);

    test->PostWork();

    if (!test->GetEncoder()->GetResult()) {
        std::cerr << "Unsupported for " << TypeName<T>() << std::endl;
        return false;
    }
    // Change this for enable print disasm
    test->Dump(false);

    if (!test->CallCode<T>(value1, value2, expected)) {
        std::cerr << "AddOverflow "
                  << " of 0x" << std::hex << value1 << " and 0x" << std::hex << value2 << " is not equal to "
                  << std::dec << expected << std::endl;
        return false;
    }

    return true;
}

template <typename T>
bool TestAddOverflow(Encoder64Test *test)
{
    T min = std::numeric_limits<T>::min();
    T max = std::numeric_limits<T>::max();
    std::array<T, 7U> values = {0, 2_I, 5_I, -7_I, -10_I, max, min};
    for (uint32_t i = 0; i < values.size(); ++i) {
        for (uint32_t j = 0; j < values.size(); ++j) {
            T a0 = values[i];
            T a1 = values[j];
            T expected;
            if ((a0 > 0 && a1 > max - a0) || (a0 < 0 && a1 < min - a0)) {
                expected = 0;
            } else {
                expected = a0 + a1;
            }
            if (!TestAddOverflow<T, true>(test, a0, a1, expected) ||
                !TestAddOverflow<T, false>(test, a0, a1, expected)) {
                return false;
            }
        }
    }
    return true;
}

template <typename T, bool IS_EQUAL = true>
bool TestSubOverflow(Encoder64Test *test, T value1, T value2, T expected)
{
    test->PreWork();
    auto param1 = test->GetParameter(TypeInfo(T(0)), 0);
    auto param2 = test->GetParameter(TypeInfo(T(0)), 1);
    auto retVal = Target::Current().GetReturnReg(TypeInfo(T(0)));
    auto label = test->GetEncoder()->CreateLabel();
    auto end = test->GetEncoder()->CreateLabel();

    if (IS_EQUAL) {
        test->GetEncoder()->EncodeSubOverflow(label, retVal, param1, param2, Condition::VS);
        test->GetEncoder()->EncodeJump(end);
        test->GetEncoder()->BindLabel(label);
        test->GetEncoder()->EncodeMov(retVal, Imm(0));
    } else {
        test->GetEncoder()->EncodeSubOverflow(end, retVal, param1, param2, Condition::VC);
        test->GetEncoder()->EncodeMov(retVal, Imm(0));
    }

    test->GetEncoder()->BindLabel(end);

    test->PostWork();

    if (!test->GetEncoder()->GetResult()) {
        std::cerr << "Unsupported for " << TypeName<T>() << std::endl;
        return false;
    }
    // Change this for enable print disasm
    test->Dump(false);

    if (!test->CallCode<T>(value1, value2, expected)) {
        std::cerr << "SubOverflow "
                  << " of 0x" << std::hex << value1 << " and 0x" << std::hex << value2 << " is not equal to "
                  << std::dec << expected << std::endl;
        return false;
    }

    return true;
}

template <typename T>
bool TestSubOverflow(Encoder64Test *test)
{
    T min = std::numeric_limits<T>::min();
    T max = std::numeric_limits<T>::max();
    std::array<T, 7U> values = {0, 2_I, 5_I, -7_I, -10_I, max, min};
    for (uint32_t i = 0; i < values.size(); ++i) {
        for (uint32_t j = 0; j < values.size(); ++j) {
            T a0 = values[i];
            T a1 = values[j];
            T expected;
            if ((a1 > 0 && a0 < min + a1) || (a1 < 0 && a0 > max + a1)) {
                expected = 0;
            } else {
                expected = a0 - a1;
            }
            if (!TestSubOverflow<T, true>(test, a0, a1, expected) ||
                !TestSubOverflow<T, false>(test, a0, a1, expected)) {
                return false;
            }
        }
    }
    return true;
}

template <typename T, Condition CC>
bool TestJumpCC(Encoder64Test *test)
{
    bool isSigned = std::is_signed<T>::value;
    // Initialize
    test->PreWork();
    // First type-dependency
    auto param = test->GetParameter(TypeInfo(T(0)));
    // Main test call
    auto tsucc = test->GetEncoder()->CreateLabel();
    auto retVal = Target::Current().GetReturnReg();

    test->GetEncoder()->EncodeJump(tsucc, param, CC);
    test->GetEncoder()->EncodeMov(param, Imm(0x0));
    test->GetEncoder()->EncodeMov(retVal, Imm(0x1));
    test->GetCallconv()->GenerateEpilogue(FrameInfo::FullPrologue(), []() {});

    test->GetEncoder()->BindLabel(tsucc);
    test->GetEncoder()->EncodeMov(param, Imm(0x0));
    test->GetEncoder()->EncodeMov(retVal, Imm(0x0));
    // test->GetEncoder()->EncodeReturn(); < encoded in PostWork

    // Finalize
    test->PostWork();

    // If encode unsupported - now print error
    if (!test->GetEncoder()->GetResult()) {
        std::cerr << "Unsupported for " << TypeName<T>() << std::endl;
        return false;
    }
    // Change this for enable print disasm
    test->Dump(false);

    if constexpr (CC == Condition::EQ) {
        if (!test->CallCode<T>(0, 0)) {
            std::cerr << "zero EQ test fail \n";
            return false;
        }
    }
    if constexpr (CC == Condition::NE) {
        if (!test->CallCode<T>(0, 1)) {
            std::cerr << "zero EQ test fail \n";
            return false;
        }
    }

    // Main test loop:
    for (uint64_t i = 0; i < ITERATION; ++i) {
        // Second type-dependency
        T tmp = RandomGen<T>();
        tmp = (tmp == 0) ? 1U : tmp;  // Only non-zero values
        // Deduced conflicting types for parameter

        if constexpr (CC == Condition::EQ) {
            if (!test->CallCode<T>(tmp, 1)) {
                std::cerr << "non-zero EQ test fail " << tmp << " \n";
                return false;
            }
        }
        if constexpr (CC == Condition::NE) {
            if (!test->CallCode<T>(tmp, 0)) {
                std::cerr << "non-zero EQ test fail " << tmp << " \n";
                return false;
            }
        }
        // Main check - compare parameter and
        // return value
    }
    return true;
}

// Jump with CC
TEST_F(Encoder64Test, JumpCCTest)
{
    EXPECT_TRUE((TestJumpCC<int32_t, Condition::EQ>(this)));
    EXPECT_TRUE((TestJumpCC<int32_t, Condition::NE>(this)));
    EXPECT_TRUE((TestJumpCC<int64_t, Condition::NE>(this)));
    EXPECT_TRUE((TestJumpCC<int64_t, Condition::EQ>(this)));
    EXPECT_TRUE((TestJumpCC<uint32_t, Condition::EQ>(this)));
    EXPECT_TRUE((TestJumpCC<uint32_t, Condition::NE>(this)));
    EXPECT_TRUE((TestJumpCC<uint64_t, Condition::EQ>(this)));
    EXPECT_TRUE((TestJumpCC<uint64_t, Condition::NE>(this)));
    // TestJumpCC<float, Condition::EQ>
    // TestJumpCC<float, Condition::NE>
    // TestJumpCC<double, Condition::EQ>
    // TestJumpCC<double, Condition::NE>
}

TEST_F(Encoder64Test, BitTestAndBranchZero)
{
    EXPECT_TRUE(TestBitTestAndBranch<uint32_t>(this));
    EXPECT_TRUE(TestBitTestAndBranch<uint64_t>(this));
}

TEST_F(Encoder64Test, BitTestAndBranchNotZero)
{
    constexpr bool NOT_ZERO = true;
    EXPECT_TRUE((TestBitTestAndBranch<uint32_t, NOT_ZERO>(this)));
    EXPECT_TRUE((TestBitTestAndBranch<uint64_t, NOT_ZERO>(this)));
}

TEST_F(Encoder64Test, AddOverflow)
{
    EXPECT_TRUE((TestAddOverflow<int32_t>(this)));
    EXPECT_TRUE((TestAddOverflow<int64_t>(this)));
}
TEST_F(Encoder64Test, SubOverflow)
{
    EXPECT_TRUE((TestSubOverflow<int32_t>(this)));
    EXPECT_TRUE((TestSubOverflow<int64_t>(this)));
}

template <typename T>
bool TestLdr(Encoder64Test *test)
{
    bool isSigned = std::is_signed<T>::value;
    // Initialize
    test->PreWork();
    // First type-dependency
    // Some strange code - default parameter is addres (32 bit)

    auto param = Target::Current().GetReturnReg();
    // But return value is cutted by loaded value
    auto retVal = test->GetParameter(TypeInfo(T(0)));

    auto mem = MemRef(param);
    test->GetEncoder()->EncodeLdr(retVal, isSigned, mem);

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
        T tmp = RandomGen<T>();
        T *ptr = &tmp;
        // Test : param - Pointer to value
        // return - value (loaded by ptr)
        // Value is resulting type, but call is ptr_type
        if (!test->CallCode<int64_t, T>(reinterpret_cast<int64_t>(ptr), tmp)) {
            std::cerr << "Ldr test fail " << tmp << " \n";
            return false;
        }
    }
    return true;
}

// Load test
TEST_F(Encoder64Test, LoadTest)
{
    EXPECT_TRUE((TestLdr<int32_t>(this)));
    EXPECT_TRUE((TestLdr<int64_t>(this)));
    EXPECT_TRUE((TestLdr<uint32_t>(this)));
    EXPECT_TRUE((TestLdr<uint64_t>(this)));
    EXPECT_TRUE((TestLdr<float>(this)));
    EXPECT_TRUE((TestLdr<double>(this)));
    // NOTE(igorban) : additional test for full memory model:
    // + mem(base + index<<scale + disp)
}

template <typename T>
bool TestStr(Encoder64Test *test)
{
    // Initialize
    test->PreWork();
    // First type-dependency
    auto param = test->GetParameter(TypeInfo(int64_t(0)), 0);
    // Data to be stored:
    auto storedValue = test->GetParameter(TypeInfo(T(0)), 1);
    if constexpr ((std::is_same<float, T>::value) || std::is_same<double, T>::value) {
        storedValue = test->GetParameter(TypeInfo(T(0)), 0);
    }

    auto mem = MemRef(param);
    test->GetEncoder()->EncodeStr(storedValue, mem);
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
        T tmp = RandomGen<T>();
        T retData = 0;
        T *ptr = &retData;

        // Test : param - Pointer to value
        // return - value (loaded by ptr)
        // Value is resulting type, but call is ptr_type
        auto result = test->CallCodeStore<T>(reinterpret_cast<int64_t>(ptr), tmp);
        // Store must change retData value
        if (!(retData == tmp || (std::is_floating_point_v<T> && std::isnan(retData) && std::isnan(tmp)))) {
            std::cerr << std::hex << "Str test fail " << tmp << " retData = " << retData << "\n";
            return false;
        }
    }
    return true;
}

// Simple store test
TEST_F(Encoder64Test, StrTest)
{
    // EncodeStr(Reg src, MemRef mem)
    EXPECT_TRUE((TestStr<int32_t>(this)));
    EXPECT_TRUE((TestStr<int64_t>(this)));
    EXPECT_TRUE((TestStr<uint32_t>(this)));
    EXPECT_TRUE((TestStr<uint64_t>(this)));
    EXPECT_TRUE((TestStr<float>(this)));
    EXPECT_TRUE((TestStr<double>(this)));

    // NOTE(igorban) : additional test for full memory model:
    // + mem(base + index<<scale + disp)
}

template <typename T>
bool TestStrzMainLoop(Encoder64Test *test)
{
    // Main test loop:
    for (uint64_t i = 0; i < ITERATION; ++i) {
        // Test : param - Pointer to value
        // return - value (loaded by ptr)
        // Value is resulting type, but call is ptr_type
        if constexpr (std::is_floating_point_v<T>) {
            // Second type-dependency
            T tmp = RandomGen<T>();
            T retData[10U];  // NOLINT(modernize-avoid-c-arrays)
            T *ptr = retData;
            for (auto &el : retData) {
                el = RandomGen<T>();
            }

            auto result = test->CallCodeStore<T>(reinterpret_cast<int64_t>(ptr), tmp);
            // Store must change retData value
            bool ret = (retData[0] == tmp && std::signbit(retData[0]) == std::signbit(tmp)) ||
                       (std::isnan(retData[0]) && std::isnan(tmp)) || (std::isinf(retData[0]) && std::isinf(tmp));
            if (ret) {
                continue;
            }

            if constexpr (std::is_same<float, T>::value) {
                std::cerr << std::hex << "Strz test fail " << bit_cast<uint32_t>(tmp)
                          << " retData = " << bit_cast<uint32_t>(retData[0]) << "\n";
            }
            if constexpr (std::is_same<double, T>::value) {
                std::cerr << std::hex << "Strz test fail " << bit_cast<uint64_t>(tmp)
                          << " retData = " << bit_cast<uint64_t>(retData[0]) << "\n";
            }
            std::cerr << std::hex << "Strz test fail " << tmp << " retData = " << retData[0] << "\n";
            return false;
        } else {
            // Second type-dependency
            T tmp = RandomGen<T>();
            uint64_t retData = 0xffffffffffffffffU;
            T *ptr = reinterpret_cast<T *>(&retData);

            auto result = test->CallCodeStore<T>(reinterpret_cast<int64_t>(ptr), tmp);
            // Store must change retData value
            if (retData != tmp) {
                std::cerr << std::hex << "Strz test fail " << (uint64_t)tmp << " retData = " << (uint64_t)retData
                          << "\n";
                return false;
            }
        }
    }
    return true;
}

// Store immediate test
// TEST_F(Encoder64Test, StiTest) {
// EncodeSti(Imm src, MemRef mem)
template <typename T>
bool TestStrz(Encoder64Test *test)
{
    // Initialize
    test->PreWork();
    // First type-dependency
    auto param = test->GetParameter(TypeInfo(int64_t(0)), 0);
    // Data to be stored:
    auto storedValue = test->GetParameter(TypeInfo(T(0)), 1);
    if constexpr (std::is_floating_point_v<T>) {
        storedValue = test->GetParameter(TypeInfo(T(0)), 0);
    }

    auto mem = MemRef(param);
    test->GetEncoder()->EncodeStrz(storedValue, mem);
    // Finalize
    test->PostWork();

    // If encode unsupported - now print error
    if (!test->GetEncoder()->GetResult()) {
        std::cerr << "Unsupported for " << TypeName<T>() << std::endl;
        return false;
    }
    // Change this for enable print disasm
    test->Dump(false);
    return TestStrzMainLoop<T>(test);
}

// Store zero upper test
TEST_F(Encoder64Test, StrzTest)
{
    EXPECT_TRUE((TestStrz<uint8_t>(this)));
    EXPECT_TRUE((TestStrz<uint16_t>(this)));
    EXPECT_TRUE((TestStrz<uint32_t>(this)));
    EXPECT_TRUE((TestStrz<uint64_t>(this)));

    EXPECT_TRUE((TestStrz<float>(this)));
    EXPECT_TRUE((TestStrz<double>(this)));
}

// Return test ???? What here may be tested
// TEST_F(Encoder64Test, ReturnTest) {
// EncodeReturn()

bool Foo(uint32_t param1, uint32_t param2)
{
    // NOTE(igorban): use variables
    return (param1 == param2);
}

using FunctPtr = bool (*)(uint32_t param1, uint32_t param2);

FunctPtr g_fooPtr = &Foo;

// Call Test
TEST_F(Encoder64Test, CallTest)
{
    // Initialize
    auto linkReg = Target::Current().GetLinkReg();

    PreWork();
    static_cast<aarch64::Aarch64Encoder *>(GetEncoder())
        ->GetMasm()
        ->Push(vixl::aarch64::xzr, aarch64::VixlReg(linkReg));

    // Call Foo
    GetEncoder()->MakeCall(reinterpret_cast<void *>(g_fooPtr));

    static_cast<aarch64::Aarch64Encoder *>(GetEncoder())->GetMasm()->Pop(aarch64::VixlReg(linkReg), vixl::aarch64::xzr);
    // return value - moved to return value
    PostWork();

    // If encode unsupported - now print error
    if (!GetEncoder()->GetResult()) {
        std::cerr << "Unsupported Call-instruction \n";
        return;
    }
    // Change this for enable print disasm
    Dump(false);
    // Equality test
    auto tmp1 = RandomGen<uint32_t>();
    uint32_t tmp2 = tmp1;
    // first template arg - parameter type, second - return type
    auto result = CallCodeCall<uint32_t, bool>(tmp1, tmp2);
    // Store must change retData value
    if (!result) {
        std::cerr << std::hex << "Call test fail tmp1=" << tmp1 << " tmp2=" << tmp2 << " result =" << result << "\n";
    }
    EXPECT_TRUE(result);

    // Main test loop:
    for (uint64_t i = 0; i < ITERATION; ++i) {
        // Second type-dependency
        auto tmp1 = RandomGen<uint32_t>();
        auto tmp2 = RandomGen<uint32_t>();

        // first template arg - parameter type, second - return type
        auto result = CallCodeCall<uint32_t, bool>(tmp1, tmp2);
        auto retData = (tmp1 == tmp2);
        // Store must change retData value
        if (result != retData) {
            std::cerr << std::hex << "Call test fail tmp1=" << tmp1 << " tmp2=" << tmp2 << " retData = " << retData
                      << " result =" << result << "\n";
        }
        EXPECT_EQ(result, retData);
    }
}

template <typename T>
bool TestAbs(Encoder64Test *test)
{
    // Initialize
    test->PreWork();

    // First type-dependency
    auto param = test->GetParameter(TypeInfo(T(0)));

    // Main test call
    test->GetEncoder()->EncodeAbs(param, param);

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
        T tmp = RandomGen<T>();
        // Main check - compare parameter and
        // return value
        if (!test->CallCode(tmp, std::abs(tmp))) {
            return false;
        }
    }

    if constexpr (std::is_floating_point_v<T>) {
        T nan = std::numeric_limits<T>::quiet_NaN();
        if (!test->CallCode<T>(nan, nan)) {
            return false;
        }
    }

    return true;
}

TEST_F(Encoder64Test, AbsTest)
{
    EXPECT_TRUE(TestAbs<int32_t>(this));
    EXPECT_TRUE(TestAbs<int64_t>(this));
    EXPECT_TRUE(TestAbs<float>(this));
    EXPECT_TRUE(TestAbs<double>(this));
}

template <typename T>
bool TestSqrt(Encoder64Test *test)
{
    // Initialize
    test->PreWork();

    // First type-dependency
    auto param = test->GetParameter(TypeInfo(T(0)));

    // Main test call
    test->GetEncoder()->EncodeSqrt(param, param);

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
        T tmp = RandomGen<T>();
        // Main check - compare parameter and
        // return value
        if (!test->CallCode(tmp, std::sqrt(tmp))) {
            return false;
        }
    }

    if constexpr (std::is_floating_point_v<T>) {
        T nan = std::numeric_limits<T>::quiet_NaN();
        if (!test->CallCode<T>(nan, nan)) {
            return false;
        }
    }

    return true;
}

TEST_F(Encoder64Test, SqrtTest)
{
    EXPECT_TRUE(TestSqrt<float>(this));
    EXPECT_TRUE(TestSqrt<double>(this));
}

template <typename T>
bool TestAdd(Encoder64Test *test)
{
    // Initialize
    test->PreWork();

    // First type-dependency
    auto param1 = test->GetParameter(TypeInfo(T(0)), 0);
    auto param2 = test->GetParameter(TypeInfo(T(0)), 1);

    // Main test call
    test->GetEncoder()->EncodeAdd(param1, param1, param2);

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
        auto tmp1 = RandomGen<T>();
        auto tmp2 = RandomGen<T>();
        // Deduced conflicting types for parameter
        // Main check - compare parameter and
        // return value
        if (!test->CallCode<T>(tmp1, tmp2, tmp1 + tmp2)) {
            return false;
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
        if (!test->CallCode<T>(-std::numeric_limits<T>::infinity(), std::numeric_limits<T>::infinity(), nan)) {
            return false;
        }
        if (!test->CallCode<T>(std::numeric_limits<T>::infinity(), -std::numeric_limits<T>::infinity(), nan)) {
            return false;
        }
    }

    return true;
}

TEST_F(Encoder64Test, AddTest)
{
    EXPECT_TRUE(TestAdd<int32_t>(this));
    EXPECT_TRUE(TestAdd<int64_t>(this));
    EXPECT_TRUE(TestAdd<float>(this));
    EXPECT_TRUE(TestAdd<double>(this));
}
template <typename T>
bool TestAddImm(Encoder64Test *test)
{
    // Initialize
    test->PreWork();

    // First type-dependency
    auto param1 = test->GetParameter(TypeInfo(T(0)), 0);
    T param2 = RandomGen<T>();

    // Main test call
    test->GetEncoder()->EncodeAdd(param1, param1, Imm(param2));

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
        // Deduced conflicting types for parameter
        // Main check - compare parameter and
        // return value
        if (!test->CallCode<T>(tmp1, tmp1 + param2)) {
            return false;
        }
    }
    return true;
}

TEST_F(Encoder64Test, AddImmTest)
{
    EXPECT_TRUE(TestAddImm<int32_t>(this));
    EXPECT_TRUE(TestAddImm<int64_t>(this));
    // TestAddImm<float>
    // TestAddImm<double>
}

template <typename T>
bool TestSub(Encoder64Test *test)
{
    // Initialize
    test->PreWork();

    // First type-dependency
    auto param1 = test->GetParameter(TypeInfo(T(0)), 0);
    auto param2 = test->GetParameter(TypeInfo(T(0)), 1);

    // Main test call
    test->GetEncoder()->EncodeSub(param1, param1, param2);

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
        // Main check - compare parameter and
        // return value
        auto diff = tmp1 - tmp2;
        if (!test->CallCode<T>(tmp1, tmp2, diff)) {
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
        if (!test->CallCode<T>(std::numeric_limits<T>::infinity(), std::numeric_limits<T>::infinity(), nan)) {
            return false;
        }
        if (!test->CallCode<T>(-std::numeric_limits<T>::infinity(), -std::numeric_limits<T>::infinity(), nan)) {
            return false;
        }
    }

    return true;
}

TEST_F(Encoder64Test, SubTest)
{
    EXPECT_TRUE(TestSub<int32_t>(this));
    EXPECT_TRUE(TestSub<int64_t>(this));
    EXPECT_TRUE(TestSub<float>(this));
    EXPECT_TRUE(TestSub<double>(this));
}

template <typename T>
bool TestSubImm(Encoder64Test *test)
{
    // Initialize
    test->PreWork();

    // First type-dependency
    auto param1 = test->GetParameter(TypeInfo(T(0)), 0);
    T param2 = RandomGen<T>();

    // Main test call
    test->GetEncoder()->EncodeSub(param1, param1, Imm(param2));

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
        // Deduced conflicting types for parameter
        // Main check - compare parameter and
        // return value
        auto diff = tmp1 - param2;
        if (!test->CallCode<T>(tmp1, diff)) {
            return false;
        }
    }
    return true;
}

TEST_F(Encoder64Test, SubImmTest)
{
    EXPECT_TRUE(TestSubImm<int32_t>(this));
    EXPECT_TRUE(TestSubImm<int64_t>(this));
    // TestSubImm<float>
    // TestSubImm<double>
}

template <typename T>
bool TestMul(Encoder64Test *test)
{
    // Initialize
    test->PreWork();

    // First type-dependency
    auto param1 = test->GetParameter(TypeInfo(T(0)), 0);
    auto param2 = test->GetParameter(TypeInfo(T(0)), 1);

    // Main test call
    test->GetEncoder()->EncodeMul(param1, param1, param2);

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
        // Main check - compare parameter and
        // return value
        auto prod = tmp1 * tmp2;
        if (!test->CallCode<T>(tmp1, tmp2, prod)) {
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
        if (!test->CallCode<T>(0.0, std::numeric_limits<T>::infinity(), nan)) {
            return false;
        }
        if (!test->CallCode<T>(std::numeric_limits<T>::infinity(), 0.0, nan)) {
            return false;
        }
    }

    return true;
}

TEST_F(Encoder64Test, MulTest)
{
    EXPECT_TRUE(TestMul<int32_t>(this));
    EXPECT_TRUE(TestMul<int64_t>(this));
    EXPECT_TRUE(TestMul<float>(this));
    EXPECT_TRUE(TestMul<double>(this));
}

template <typename T>
bool TestMin(Encoder64Test *test)
{
    // Initialize
    test->PreWork();

    // First type-dependency
    auto param1 = test->GetParameter(TypeInfo(T(0)), 0);
    auto param2 = test->GetParameter(TypeInfo(T(0)), 1);

    // Main test call
    test->GetEncoder()->EncodeMin(param1, std::is_signed_v<T>, param1, param2);

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

        T result {0};
        bool areNan = (std::isnan(tmp1) || std::isnan(tmp2));
        if (std::is_floating_point_v<T> && areNan) {
            result = std::numeric_limits<T>::quiet_NaN();
        } else {
            // We do that, because auto check -0.0 and +0.0 std::max give incorrect result
            if (std::fabs(tmp1) < 1e-300 && std::fabs(tmp2) < 1e-300) {
                continue;
            }
            result = std::min(tmp1, tmp2);
        }

        // Main check - compare parameter and
        // return value
        if (!test->CallCode<T>(tmp1, tmp2, result)) {
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
        // use static_cast to make sure correct float/double type is applied
        if (!test->CallCode<T>(static_cast<T>(-0.0), static_cast<T>(+0.0), static_cast<T>(-0.0))) {
            return false;
        }
        if (!test->CallCode<T>(static_cast<T>(+0.0), static_cast<T>(-0.0), static_cast<T>(-0.0))) {
            return false;
        }
    }

    return true;
}

TEST_F(Encoder64Test, MinTest)
{
    EXPECT_TRUE(TestMin<int32_t>(this));
    EXPECT_TRUE(TestMin<int64_t>(this));
    EXPECT_TRUE(TestMin<uint32_t>(this));
    EXPECT_TRUE(TestMin<uint64_t>(this));
    EXPECT_TRUE(TestMin<float>(this));
    EXPECT_TRUE(TestMin<double>(this));
}

template <typename T>
bool TestMax(Encoder64Test *test)
{
    // Initialize
    test->PreWork();

    // First type-dependency
    auto param1 = test->GetParameter(TypeInfo(T(0)), 0);
    auto param2 = test->GetParameter(TypeInfo(T(0)), 1);

    // Main test call
    test->GetEncoder()->EncodeMax(param1, std::is_signed_v<T>, param1, param2);

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
        T result {0};
        bool areNan = (std::isnan(tmp1) || std::isnan(tmp2));
        if (std::is_floating_point_v<T> && areNan) {
            result = std::numeric_limits<T>::quiet_NaN();
        } else {
            // We do that, because auto check -0.0 and +0.0 std::max give incorrect result
            if (std::fabs(tmp1) < 1e-300 && std::fabs(tmp2) < 1e-300) {
                continue;
            }
            result = std::max(tmp1, tmp2);
        }

        // Main check - compare parameter and
        // return value
        if (!test->CallCode<T>(tmp1, tmp2, result)) {
            return false;
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
        // use static_cast to make sure correct float/double type is applied
        if (!test->CallCode<T>(static_cast<T>(-0.0), static_cast<T>(+0.0), static_cast<T>(+0.0))) {
            return false;
        }
        if (!test->CallCode<T>(static_cast<T>(+0.0), static_cast<T>(-0.0), static_cast<T>(+0.0))) {
            return false;
        }
    }

    return true;
}

TEST_F(Encoder64Test, MaxTest)
{
    EXPECT_TRUE(TestMax<int32_t>(this));
    EXPECT_TRUE(TestMax<int64_t>(this));
    EXPECT_TRUE(TestMax<uint32_t>(this));
    EXPECT_TRUE(TestMax<uint64_t>(this));
    EXPECT_TRUE(TestMax<float>(this));
    EXPECT_TRUE(TestMax<double>(this));
}

// NOLINTBEGIN(hicpp-signed-bitwise)
template <typename T>
bool TestShl(Encoder64Test *test)
{
    // Initialize
    test->PreWork();

    // First type-dependency
    auto param1 = test->GetParameter(TypeInfo(T(0)), 0);
    auto param2 = test->GetParameter(TypeInfo(T(0)), 1);

    // Main test call
    test->GetEncoder()->EncodeShl(param1, param1, param2);

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
        T tmp2 {0};
        if constexpr (std::is_same_v<T, int64_t>) {
            tmp2 = RandomGen<uint8_t>() % DOUBLE_WORD_SIZE;
        } else {
            tmp2 = RandomGen<uint8_t>() % WORD_SIZE;
        }
        // Deduced conflicting types for parameter

        // Main check - compare parameter and
        // return value
        bool result {false};

        if constexpr (std::is_same_v<T, int8_t> || std::is_same_v<T, int16_t>) {
            auto changed = tmp1 << (tmp2 & (CHAR_BIT * sizeof(T) - 1));
            result = test->CallCode<T>(tmp1, tmp2, changed);
        } else {
            auto changed = tmp1 << tmp2;
            result = test->CallCode<T>(tmp1, tmp2, changed);
        }

        if (!result) {
            return false;
        }
    }
    return true;
}

TEST_F(Encoder64Test, ShlTest)
{
    EXPECT_TRUE(TestShl<int32_t>(this));
    EXPECT_TRUE(TestShl<int64_t>(this));
}

template <typename T>
bool TestShr(Encoder64Test *test)
{
    // Initialize
    test->PreWork();

    // First type-dependency
    auto param1 = test->GetParameter(TypeInfo(T(0)), 0);
    auto param2 = test->GetParameter(TypeInfo(T(0)), 1);

    // Main test call
    test->GetEncoder()->EncodeShr(param1, param1, param2);

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
        T tmp2 {0};
        if constexpr (sizeof(T) == sizeof(int64_t)) {
            tmp2 = RandomGen<uint8_t>() % DOUBLE_WORD_SIZE;
        } else {
            tmp2 = RandomGen<uint8_t>() % WORD_SIZE;
        }
        // Deduced conflicting types for parameter

        // Main check - compare parameter and
        // return value
        bool result {false};

        if constexpr (sizeof(T) == sizeof(int64_t)) {
            result = test->CallCode<T>(tmp1, tmp2, (static_cast<uint64_t>(tmp1)) >> tmp2);
        } else {
            result =
                test->CallCode<T>(tmp1, tmp2, (static_cast<uint32_t>(tmp1)) >> (tmp2 & (CHAR_BIT * sizeof(T) - 1)));
        }

        if (!result) {
            return false;
        }
    }
    return true;
}

TEST_F(Encoder64Test, ShrTest)
{
    EXPECT_TRUE(TestShr<int32_t>(this));
    EXPECT_TRUE(TestShr<int64_t>(this));
    EXPECT_TRUE(TestShr<uint32_t>(this));
    EXPECT_TRUE(TestShr<uint64_t>(this));
}
template <typename T>
bool TestAShr(Encoder64Test *test)
{
    // Initialize
    test->PreWork();

    // First type-dependency
    auto param1 = test->GetParameter(TypeInfo(T(0)), 0);
    auto param2 = test->GetParameter(TypeInfo(T(0)), 1);

    // Main test call
    test->GetEncoder()->EncodeAShr(param1, param1, param2);

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
        T tmp2 {0};
        if constexpr (std::is_same_v<T, int64_t>) {
            tmp2 = RandomGen<uint8_t>() % DOUBLE_WORD_SIZE;
        } else {
            tmp2 = RandomGen<uint8_t>() % WORD_SIZE;
        }
        // Deduced conflicting types for parameter

        // Main check - compare parameter and
        // return value
        bool result {false};

        if constexpr (std::is_same_v<T, int8_t> || std::is_same_v<T, int16_t>) {
            result = test->CallCode<T>(tmp1, tmp2, tmp1 >> (tmp2 & (CHAR_BIT * sizeof(T) - 1)));
        } else {
            result = test->CallCode<T>(tmp1, tmp2, tmp1 >> tmp2);
        }

        if (!result) {
            return false;
        }
    }
    return true;
}

TEST_F(Encoder64Test, AShrTest)
{
    EXPECT_TRUE(TestAShr<int32_t>(this));
    EXPECT_TRUE(TestAShr<int64_t>(this));
}

template <typename T>
bool TestAnd(Encoder64Test *test)
{
    // Initialize
    test->PreWork();

    // First type-dependency
    auto param1 = test->GetParameter(TypeInfo(T(0)), 0);
    auto param2 = test->GetParameter(TypeInfo(T(0)), 1);

    // Main test call
    test->GetEncoder()->EncodeAnd(param1, param1, param2);

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
        // Main check - compare parameter and
        // return value
        if (!test->CallCode<T>(tmp1, tmp2, tmp1 & tmp2)) {
            return false;
        }
    }
    return true;
}

TEST_F(Encoder64Test, AndTest)
{
    EXPECT_TRUE(TestAnd<int32_t>(this));
    EXPECT_TRUE(TestAnd<int64_t>(this));
}

template <typename T>
bool TestOr(Encoder64Test *test)
{
    // Initialize
    test->PreWork();

    // First type-dependency
    auto param1 = test->GetParameter(TypeInfo(T(0)), 0);
    auto param2 = test->GetParameter(TypeInfo(T(0)), 1);

    // Main test call
    test->GetEncoder()->EncodeOr(param1, param1, param2);

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
        // Main check - compare parameter and
        // return value
        if (!test->CallCode<T>(tmp1, tmp2, tmp1 | tmp2)) {
            return false;
        }
    }
    return true;
}

TEST_F(Encoder64Test, OrTest)
{
    EXPECT_TRUE(TestOr<int32_t>(this));
    EXPECT_TRUE(TestOr<int64_t>(this));
}

template <typename T>
bool TestXor(Encoder64Test *test)
{
    // Initialize
    test->PreWork();

    // First type-dependency
    auto param1 = test->GetParameter(TypeInfo(T(0)), 0);
    auto param2 = test->GetParameter(TypeInfo(T(0)), 1);

    // Main test call
    test->GetEncoder()->EncodeXor(param1, param1, param2);

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
        // Main check - compare parameter and
        // return value
        if (!test->CallCode<T>(tmp1, tmp2, tmp1 ^ tmp2)) {
            return false;
        }
    }
    return true;
}

TEST_F(Encoder64Test, XorTest)
{
    EXPECT_TRUE(TestXor<int32_t>(this));
    EXPECT_TRUE(TestXor<int64_t>(this));
}

template <typename T>
bool TestShlImm(Encoder64Test *test)
{
    // Initialize
    test->PreWork();

    // First type-dependency
    auto param1 = test->GetParameter(TypeInfo(T(0)), 0);
    int64_t param2 {0};
    if constexpr (std::is_same_v<T, int64_t>) {
        param2 = RandomGen<uint8_t>() % DOUBLE_WORD_SIZE;
    } else {
        param2 = RandomGen<uint8_t>() % WORD_SIZE;
    }

    // Main test call
    test->GetEncoder()->EncodeShl(param1, param1, Imm(param2));

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

        // Deduced conflicting types for parameter

        // Main check - compare parameter and
        // return value
        bool result {false};

        if constexpr (std::is_same_v<T, int8_t> || std::is_same_v<T, int16_t>) {
            result = test->CallCode<T>(tmp1, tmp1 << (param2 & (CHAR_BIT * sizeof(T) - 1)));
        } else {
            result = test->CallCode<T>(tmp1, tmp1 << param2);
        }

        if (!result) {
            return false;
        }
    }
    return true;
}

TEST_F(Encoder64Test, ShlImmTest)
{
    EXPECT_TRUE(TestShlImm<int32_t>(this));
    EXPECT_TRUE(TestShlImm<int64_t>(this));
}

template <typename T>
bool TestShrImm(Encoder64Test *test)
{
    // Initialize
    test->PreWork();

    // First type-dependency
    auto param1 = test->GetParameter(TypeInfo(T(0)), 0);
    int64_t param2 {0};
    if constexpr (sizeof(T) == sizeof(int64_t)) {
        param2 = RandomGen<uint8_t>() % DOUBLE_WORD_SIZE;
    } else {
        param2 = RandomGen<uint8_t>() % WORD_SIZE;
    }

    // Main test call
    test->GetEncoder()->EncodeShr(param1, param1, Imm(param2));

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

        // Deduced conflicting types for parameter

        // Main check - compare parameter and
        // return value
        bool result {false};

        if constexpr (sizeof(T) == sizeof(int64_t)) {
            result = test->CallCode<T>(tmp1, (static_cast<uint64_t>(tmp1)) >> (param2 & (CHAR_BIT * sizeof(T) - 1)));
        } else {
            result = test->CallCode<T>(tmp1, (static_cast<uint32_t>(tmp1)) >> param2);
        }

        if (!result) {
            return false;
        }
    }
    return true;
}
// NOLINTEND(hicpp-signed-bitwise)

TEST_F(Encoder64Test, ShrImmTest)
{
    EXPECT_TRUE(TestShrImm<int32_t>(this));
    EXPECT_TRUE(TestShrImm<int64_t>(this));
    EXPECT_TRUE(TestShrImm<uint32_t>(this));
    EXPECT_TRUE(TestShrImm<uint64_t>(this));
}

template <typename T>
bool TestCmp(Encoder64Test *test)
{
    static_assert(std::is_integral_v<T>);
    // Initialize
    test->PreWork();

    // First type-dependency
    auto output = test->GetParameter(TypeInfo(int32_t(0)), 0);
    auto param1 = test->GetParameter(TypeInfo(T(0)), 0);
    auto param2 = test->GetParameter(TypeInfo(T(0)), 1);

    // Main test call
    test->GetEncoder()->EncodeCmp(output, param1, param2, std::is_signed_v<T> ? Condition::LT : Condition::LO);

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

        auto compare = [](T a, T b) -> int32_t { return a < b ? -1 : a > b ? 1 : 0; };
        int32_t result {compare(tmp1, tmp2)};

        // Main check - compare parameter and
        // return value
        if (!test->CallCode<T, int32_t>(tmp1, tmp2, result)) {
            return false;
        }
    }

    return true;
}

template <typename T>
bool TestFcmp(Encoder64Test *test, bool isFcmpg)
{
    static_assert(std::is_floating_point_v<T>);
    // Initialize
    test->PreWork();

    // First type-dependency
    auto output = test->GetParameter(TypeInfo(int32_t(0)), 0);
    auto param1 = test->GetParameter(TypeInfo(T(0)), 0);
    auto param2 = test->GetParameter(TypeInfo(T(0)), 1);

    // Main test call
    test->GetEncoder()->EncodeCmp(output, param1, param2, isFcmpg ? Condition::MI : Condition::LT);

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

        auto compare = [](T a, T b) -> int32_t { return a < b ? -1 : a > b ? 1 : 0; };

        int32_t result {0};
        auto isNan = (std::isnan(tmp1) || std::isnan(tmp2));
        if (isNan) {
            result = isFcmpg ? 1 : -1;
        } else {
            result = compare(tmp1, tmp2);
        }

        // Main check - compare parameter and
        // return value
        if (!test->CallCode<T, int32_t>(tmp1, tmp2, result)) {
            return false;
        }
    }

    if constexpr (std::is_floating_point_v<T>) {
        T nan = std::numeric_limits<T>::quiet_NaN();
        auto sign = isFcmpg ? 1 : -1;
        if (!test->CallCode<T, int32_t>(nan, 5.0F, sign)) {
            return false;
        }
        if (!test->CallCode<T, int32_t>(5.0F, nan, sign)) {
            return false;
        }
        if (!test->CallCode<T, int32_t>(nan, nan, sign)) {
            return false;
        }
    }

    return true;
}

TEST_F(Encoder64Test, CmpTest)
{
    EXPECT_TRUE(TestCmp<int32_t>(this));
    EXPECT_TRUE(TestCmp<int64_t>(this));
    EXPECT_TRUE(TestCmp<uint32_t>(this));
    EXPECT_TRUE(TestCmp<uint64_t>(this));
    EXPECT_TRUE(TestFcmp<float>(this, false));
    EXPECT_TRUE(TestFcmp<double>(this, false));
    EXPECT_TRUE(TestFcmp<float>(this, true));
    EXPECT_TRUE(TestFcmp<double>(this, true));
}
// NOLINTEND(readability-magic-numbers)

}  // namespace ark::compiler
