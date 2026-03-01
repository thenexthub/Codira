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

#include <climits>

#include <random>
#include <gtest/gtest.h>

#include "mem/code_allocator.h"
#include "mem/pool_manager.h"
#include "target/aarch64/target.h"
#include "mem/base_mem_stats.h"

template <typename T>
const char *TypeName(void);
template <typename T>
const char *TypeName(T)
{
    return TypeName<T>();
}

#define CLASS_NAME(type)             \
    template <>                      \
    const char *TypeName<type>(void) \
    {                                \
        return #type;                \
    }

CLASS_NAME(int8_t)
CLASS_NAME(int16_t)
CLASS_NAME(int32_t)
CLASS_NAME(int64_t)
CLASS_NAME(uint8_t)
CLASS_NAME(uint16_t)
CLASS_NAME(uint32_t)
CLASS_NAME(uint64_t)
CLASS_NAME(float)
CLASS_NAME(double)

const uint64_t SEED = 0x1234;
const uint64_t ARRAY_SIZE = 10;
#ifndef PANDA_NIGHTLY_TEST_ON
const uint64_t ITERATION = 40;
#else
const uint64_t ITERATION = 4000;
#endif
static inline auto random_generator = std::mt19937_64(SEED);

// Max and min exponent on the basus of two float and double
static const float MIN_EXP_BASE2_FLOAT = std::log2(FLT_MIN);
static const float MAX_EXP_BASE2_FLOAT = std::log2(FLT_MAX) - 1.0;
static const double MIN_EXP_BASE2_DOUBLE = std::log2(DBL_MIN);
static const double MAX_EXP_BASE2_DOUBLE = std::log2(DBL_MAX) - 1.0;

// Masks for generete denormal float numbers
static const uint32_t MASK_DENORMAL_FLOAT = 0x807FFFFF;
static const uint64_t MASK_DENORMAL_DOUBLE = 0x800FFFFFFFFFFFFF;

template <typename T>
static T random_gen()
{
    auto gen {random_generator()};

    if constexpr (std::is_integral_v<T>) {
        return gen;
    } else {
        switch (gen % 20U) {
            case (0U):
                return std::numeric_limits<T>::quiet_NaN();

            case (1U):
                return std::numeric_limits<T>::infinity();

            case (2U):
                return -std::numeric_limits<T>::infinity();

            case (3U): {
                if constexpr (std::is_same_v<T, float>) {
                    return panda::bit_cast<float, uint32_t>(gen & MASK_DENORMAL_FLOAT);
                } else {
                    return panda::bit_cast<double, uint64_t>(gen & MASK_DENORMAL_DOUBLE);
                }
            }
            default:
                break;
        }

        // Uniform distribution floating value
        std::uniform_real_distribution<T> dis_num(1.0, 2.0);
        int8_t sign = (gen % 2) == 0 ? 1 : -1;
        if constexpr (std::is_same_v<T, float>) {
            std::uniform_real_distribution<float> dis(MIN_EXP_BASE2_FLOAT, MAX_EXP_BASE2_FLOAT);
            return sign * dis_num(random_generator) * std::pow(2.0F, dis(random_generator));
        } else if constexpr (std::is_same_v<T, double>) {
            std::uniform_real_distribution<double> dis(MIN_EXP_BASE2_DOUBLE, MAX_EXP_BASE2_DOUBLE);
            return sign * dis_num(random_generator) * std::pow(2.0, dis(random_generator));
        }

        UNREACHABLE();
    }
}

template <typename T>
T random_mask_gen()
{
    static_assert(std::is_integral_v<T>);
    static constexpr size_t MASKS_COUNT = 4;
    static constexpr std::array<uint64_t, MASKS_COUNT> MASKS = {0x8888888888888888ULL, 0xCCCCCCCCCCCCCCCCULL,
                                                                0xAAAAAAAAAAAAAAAAULL, 0xEEEEEEEEEEEEEEEEULL};
    auto mask = MASKS[random_generator() % MASKS_COUNT];
    auto type_size = panda::compiler::TypeInfo(T(0)).GetSize();
    auto rot = random_generator() % type_size;
    auto rotated_mask = (mask << rot) | (mask >> (type_size - rot));
    return static_cast<T>(rotated_mask);
}

namespace panda::compiler {
class Encoder64Test : public ::testing::Test {
public:
    Encoder64Test()
    {
        panda::mem::MemConfig::Initialize(64_MB, 64_MB, 64_MB, 32_MB);
        PoolManager::Initialize();
        allocator_ = new ArenaAllocator(SpaceType::SPACE_TYPE_COMPILER);
        encoder_ = Encoder::Create(allocator_, Arch::AARCH64, false);
        encoder_->InitMasm();
        regfile_ = RegistersDescription::Create(allocator_, Arch::AARCH64);
        callconv_ = CallingConvention::Create(allocator_, encoder_, regfile_, Arch::AARCH64);
        encoder_->SetRegfile(regfile_);
        mem_stats_ = new BaseMemStats();
        code_alloc_ = new (std::nothrow) CodeAllocator(mem_stats_);
    }

    virtual ~Encoder64Test()
    {
        Logger::Destroy();
        encoder_->~Encoder();
        delete allocator_;
        delete code_alloc_;
        delete mem_stats_;
        PoolManager::Finalize();
        panda::mem::MemConfig::Finalize();
    }

    CodeAllocator *GetCodeAllocator()
    {
        return code_alloc_;
    }

    void ResetCodeAllocator(void *ptr, size_t size)
    {
        os::mem::MapRange<std::byte> mem_range(static_cast<std::byte *>(ptr), size);
        mem_range.MakeReadWrite();
        delete code_alloc_;
        code_alloc_ = new (std::nothrow) CodeAllocator(mem_stats_);
    }

    ArenaAllocator *GetAllocator()
    {
        return allocator_;
    }

    Encoder *GetEncoder()
    {
        return encoder_;
    }

    RegistersDescription *GetRegfile()
    {
        return regfile_;
    }

    CallingConvention *GetCallconv()
    {
        return callconv_;
    }

    size_t GetCursor()
    {
        return curr_cursor_;
    }

    // Warning! Do not use multiply times with different types!
    Reg GetParameter(TypeInfo type, int id = 0)
    {
        ASSERT(id < 4);
        if (type.IsFloat()) {
            return Reg(id, type);
        }

        return Target::Current().GetParamReg(id, type);
    }

    void PreWork()
    {
        // Curor need to encode multiply tests due one execution
        curr_cursor_ = 0;
        encoder_->SetCursorOffset(0);
        callconv_->GeneratePrologue(FrameInfo::FullPrologue());
    }

    void PostWork()
    {
        callconv_->GenerateEpilogue(FrameInfo::FullPrologue(), []() {});
        encoder_->Finalize();
    }

    void Dump(bool enabled)
    {
        if (enabled) {
            auto size = callconv_->GetCodeSize() - curr_cursor_;
            for (uint32_t i = curr_cursor_; i < curr_cursor_ + size;) {
                i = encoder_->DisasmInstr(std::cout, i, 0);
                std::cout << std::endl;
            }
        }
    }

    template <typename T, typename U>
    bool CallCode(const T &param, const U &result)
    {
        // Using max size type: type result "U" or 32bit to check result,
        // because in our ISA min type is 32bit.
        // Only integers less thаn 32bit.
        typedef typename std::conditional<(sizeof(U) * BYTE_SIZE) >= WORD_SIZE, U, uint32_t>::type U_exp;
        typedef typename std::conditional<(sizeof(T) * BYTE_SIZE) >= WORD_SIZE, T, uint32_t>::type T_exp;
        using funct_ptr = U_exp (*)(T_exp data);
        auto size = callconv_->GetCodeSize() - curr_cursor_;
        void *offset = (static_cast<uint8_t *>(callconv_->GetCodeEntry()));
        void *ptr = code_alloc_->AllocateCode(size, offset);
        auto func = reinterpret_cast<funct_ptr>(ptr);
        // We must make caste to transfer to function a correctly expanded number
        const U_exp curr_result = func(static_cast<T_exp>(param));
        ResetCodeAllocator(ptr, size);
        bool ret = false;
        if constexpr (std::is_floating_point_v<T> || std::is_floating_point_v<U>) {
            ret = (curr_result == result && std::signbit(curr_result) == std::signbit(result)) ||
                  (std::isnan(curr_result) && std::isnan(result));
        } else {
            ret = (curr_result - result == 0);
        }
        if (!ret) {
            std::cerr << std::hex << "Failed CallCode for param=" << param << " and result=" << result
                      << " current_reslt=" << curr_result << "\n";
            if constexpr (std::is_floating_point_v<T> || std::is_floating_point_v<U>) {
                std::cerr << "In binary :";
                if constexpr (std::is_same<double, T>::value) {
                    std::cerr << " param=" << bit_cast<uint64_t>(param);
                } else if constexpr (std::is_same<float, T>::value) {
                    std::cerr << " param=" << bit_cast<uint32_t>(param);
                }
                if constexpr (std::is_same<double, U>::value) {
                    std::cerr << " reslt=" << bit_cast<uint64_t>(result);
                    std::cerr << " current_reslt=" << bit_cast<uint64_t>(curr_result);
                } else if constexpr (std::is_same<float, U>::value) {
                    std::cerr << " result=" << bit_cast<uint32_t>(result);
                    std::cerr << " current_reslt=" << bit_cast<uint32_t>(curr_result);
                }
                std::cerr << "\n";
            }
            Dump(true);
        }
        return ret;
    }

    template <typename T, typename U>
    bool CallCode(const T &param1, const T &param2, const U &result)
    {
        typedef typename std::conditional<(sizeof(U) * BYTE_SIZE) >= WORD_SIZE, U, uint32_t>::type U_exp;
        typedef typename std::conditional<(sizeof(T) * BYTE_SIZE) >= WORD_SIZE, T, uint32_t>::type T_exp;
        using funct_ptr = U_exp (*)(T_exp param1, T_exp param2);
        auto size = callconv_->GetCodeSize() - curr_cursor_;
        void *offset = (static_cast<uint8_t *>(callconv_->GetCodeEntry()));
        void *ptr = code_alloc_->AllocateCode(size, offset);
        auto func = reinterpret_cast<funct_ptr>(ptr);
        const U_exp curr_result = func(static_cast<T_exp>(param1), static_cast<T_exp>(param2));
        ResetCodeAllocator(ptr, size);
        bool ret = false;
        if constexpr (std::is_same<float, T>::value || std::is_same<double, T>::value) {
            ret = (curr_result == result && std::signbit(curr_result) == std::signbit(result)) ||
                  (std::isnan(curr_result) && std::isnan(result));
        } else {
            ret = (curr_result - result == 0);
        }
        if (!ret) {
            std::cerr << "Failed CallCode for param1=" << param1 << " param2=" << param2 << " and result=" << result
                      << " current_result=" << curr_result << "\n";
            if constexpr (std::is_floating_point_v<T> || std::is_floating_point_v<U>) {
                std::cerr << "In binary :";
                if constexpr (std::is_same<double, T>::value) {
                    std::cerr << " param1=" << bit_cast<uint64_t>(param1) << " param2=" << bit_cast<uint64_t>(param2);
                } else if constexpr (std::is_same<float, T>::value) {
                    std::cerr << " param1=" << bit_cast<uint32_t>(param1) << " param2=" << bit_cast<uint32_t>(param2);
                }
                if constexpr (std::is_same<double, U>::value) {
                    std::cerr << " reslt=" << bit_cast<uint64_t>(result);
                    std::cerr << " current_reslt=" << bit_cast<uint64_t>(curr_result);

                } else if constexpr (std::is_same<float, U>::value) {
                    std::cerr << " result=" << bit_cast<uint32_t>(result);
                    std::cerr << " current_result=" << bit_cast<uint32_t>(curr_result);
                }
                std::cerr << "\n";
            }
            Dump(true);
        }
        return ret;
    }

    template <typename T>
    bool CallCode(const T &param, const T &result)
    {
        typedef typename std::conditional<(sizeof(T) * BYTE_SIZE) >= WORD_SIZE, T, uint32_t>::type T_exp;
        using funct_ptr = T_exp (*)(T_exp data);
        auto size = callconv_->GetCodeSize() - curr_cursor_;
        void *offset = (static_cast<uint8_t *>(callconv_->GetCodeEntry()));
        void *ptr = code_alloc_->AllocateCode(size, offset);
        auto func = reinterpret_cast<funct_ptr>(ptr);
        const T_exp curr_result = func(static_cast<T_exp>(param));
        ResetCodeAllocator(ptr, size);
        bool ret = false;
        if constexpr (std::is_same<float, T>::value || std::is_same<double, T>::value) {
            ret = (curr_result == result && std::signbit(curr_result) == std::signbit(result)) ||
                  (std::isnan(curr_result) && std::isnan(result));
        } else {
            ret = (curr_result - result == 0);
        }
        if (!ret) {
            std::cerr << std::hex << "Failed CallCode for param=" << param << " and result=" << result
                      << " current_result=" << curr_result << "\n";
            if constexpr (std::is_floating_point_v<T>) {
                std::cerr << "In binary :";
                if constexpr (std::is_same<double, T>::value) {
                    std::cerr << " param=" << bit_cast<uint64_t>(param);
                    std::cerr << " reslt=" << bit_cast<uint64_t>(result);
                    std::cerr << " curr_reslt=" << bit_cast<uint64_t>(curr_result);
                } else if constexpr (std::is_same<float, T>::value) {
                    std::cerr << " param=" << bit_cast<uint32_t>(param);
                    std::cerr << " curr_result=" << bit_cast<uint32_t>(curr_result);
                }
                std::cerr << "\n";
            }
            Dump(true);
        }
        return ret;
    }

    template <typename T>
    bool CallCode(const T &param1, const T &param2, const T &result)
    {
        typedef typename std::conditional<(sizeof(T) * BYTE_SIZE) >= WORD_SIZE, T, uint32_t>::type T_exp;
        using funct_ptr = T_exp (*)(T_exp param1, T_exp param2);
        auto size = callconv_->GetCodeSize() - curr_cursor_;
        void *offset = (static_cast<uint8_t *>(callconv_->GetCodeEntry()));
        void *ptr = code_alloc_->AllocateCode(size, offset);
        auto func = reinterpret_cast<funct_ptr>(ptr);
        const T_exp curr_result = func(static_cast<T_exp>(param1), static_cast<T_exp>(param2));
        ResetCodeAllocator(ptr, size);
        bool ret = false;
        if constexpr (std::is_same<float, T>::value || std::is_same<double, T>::value) {
            ret = (curr_result == result && std::signbit(curr_result) == std::signbit(result)) ||
                  (std::isnan(curr_result) && std::isnan(result));
        } else {
            ret = (curr_result - result == 0);
        }
        if (!ret) {
            std::cerr << "Failed CallCode for param1=" << param1 << " param2=" << param2 << " and result=" << result
                      << " curr_result=" << curr_result << "\n";
            if constexpr (std::is_floating_point_v<T>) {
                std::cerr << "In binary :";
                if constexpr (std::is_same<double, T>::value) {
                    std::cerr << " param1=" << bit_cast<uint64_t>(param1) << " param2=" << bit_cast<uint64_t>(param2);
                    std::cerr << " reslt=" << bit_cast<uint64_t>(result)
                              << " curr_result=" << bit_cast<uint64_t>(curr_result);
                } else if constexpr (std::is_same<float, T>::value) {
                    std::cerr << " param1=" << bit_cast<uint32_t>(param1) << " param2=" << bit_cast<uint32_t>(param2);
                    std::cerr << " result=" << bit_cast<uint32_t>(result)
                              << " curr_result=" << bit_cast<uint32_t>(curr_result);
                }
                std::cerr << "\n";
            }
            Dump(true);
        }
        return ret;
    }

    template <typename T>
    T CallCodeStore(uint64_t address, T param)
    {
        using funct_ptr = T (*)(uint64_t param1, T param2);
        auto size = callconv_->GetCodeSize() - curr_cursor_;
        void *offset = (static_cast<uint8_t *>(callconv_->GetCodeEntry()));
        void *ptr = code_alloc_->AllocateCode(size, offset);
        auto func = reinterpret_cast<funct_ptr>(ptr);
        const T curr_result = func(address, param);
        ResetCodeAllocator(ptr, size);
        return curr_result;
    }

    template <typename T, typename U>
    U CallCodeCall(T param1, T param2)
    {
        using funct_ptr = U (*)(T param1, T param2);
        auto size = callconv_->GetCodeSize() - curr_cursor_;
        void *offset = (static_cast<uint8_t *>(callconv_->GetCodeEntry()));
        void *ptr = code_alloc_->AllocateCode(size, offset);
        auto func = reinterpret_cast<funct_ptr>(ptr);
        const U curr_result = func(param1, param2);
        ResetCodeAllocator(ptr, size);
        return curr_result;
    }

private:
    ArenaAllocator *allocator_ {nullptr};
    Encoder *encoder_ {nullptr};
    RegistersDescription *regfile_ {nullptr};
    CallingConvention *callconv_ {nullptr};
    CodeAllocator *code_alloc_ {nullptr};
    BaseMemStats *mem_stats_ {nullptr};
    size_t curr_cursor_ {0};
};

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
        std::cerr << "Unsupported for " << TypeName<T>() << "\n";
        return false;
    }
    // Change this for enable print disasm
    test->Dump(false);

    // Main test loop:
    for (uint64_t i = 0; i < ITERATION; ++i) {
        // Second type-dependency
        T tmp = random_gen<T>();
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
        std::cerr << "Unsupported for " << TypeName<T>() << "\n";
        return false;
    }
    // Change this for enable print disasm
    test->Dump(false);

    // Main test loop:
    for (uint64_t i = 0; i < ITERATION; ++i) {
        // Second type-dependency
        T tmp = random_gen<T>();

        // Deduced conflicting types for parameter

        // Main check - compare parameter and
        // return value
        if (!test->CallCode<T>(tmp, ~tmp)) {
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
        std::cerr << "Unsupported for " << TypeName<T>() << "\n";
        return false;
    }
    // Change this for enable print disasm
    test->Dump(false);

    // Main test loop:
    for (uint64_t i = 0; i < ITERATION; ++i) {
        // Second type-dependency
        T tmp = random_gen<T>();
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
        Src src = random_gen<Src>();
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
        Dst dst_nan = bit_cast<Dst>(nan);

        if (!test->CallCode<Src, Dst>(nan, dst_nan)) {
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

    // TODO (igorban) : add MOVI instructions
    // & support uint64_t mov
}

// Jump w/o cc
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
        auto tmp = random_gen<int32_t>();
        // Deduced conflicting types for parameter
        EXPECT_TRUE(CallCode<int32_t>(tmp, tmp + 5));
    }
}

template <typename T, bool NotZero = false>
bool TestBitTestAndBranch(Encoder64Test *test, T value, int pos, uint32_t expected)
{
    test->PreWork();
    auto param = test->GetParameter(TypeInfo(T(0)));
    auto ret_val = Target::Current().GetReturnReg();
    auto label = test->GetEncoder()->CreateLabel();

    if (NotZero) {
        test->GetEncoder()->EncodeBitTestAndBranch(label, param, pos, true);
    } else {
        test->GetEncoder()->EncodeBitTestAndBranch(label, param, pos, false);
    }
    test->GetEncoder()->EncodeMov(ret_val, Imm(1));
    test->GetCallconv()->GenerateEpilogue(FrameInfo::FullPrologue(), []() {});

    test->GetEncoder()->BindLabel(label);
    test->GetEncoder()->EncodeMov(ret_val, Imm(0));

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

template <typename T, bool NotZero = false>
bool TestBitTestAndBranch(Encoder64Test *test)
{
    size_t max_pos = std::is_same<uint64_t, T>::value ? 64 : 32;
    for (size_t i = 0; i < max_pos; i++) {
        T value = static_cast<T>(1UL << i);
        if (!TestBitTestAndBranch<T, NotZero>(test, value, i, NotZero ? 0 : 1)) {
            return false;
        }
        if (!TestBitTestAndBranch<T, NotZero>(test, ~value, i, NotZero ? 1 : 0)) {
            return false;
        }
    }
    return true;
}

template <typename T, bool is_equal = true>
bool TestAddOverflow(Encoder64Test *test, T value1, T value2, T expected)
{
    test->PreWork();
    auto param1 = test->GetParameter(TypeInfo(T(0)), 0);
    auto param2 = test->GetParameter(TypeInfo(T(0)), 1);
    auto ret_val = Target::Current().GetReturnReg(TypeInfo(T(0)));
    auto label = test->GetEncoder()->CreateLabel();
    auto end = test->GetEncoder()->CreateLabel();

    if (is_equal) {
        test->GetEncoder()->EncodeAddOverflow(label, ret_val, param1, param2, Condition::VS);
        test->GetEncoder()->EncodeJump(end);
        test->GetEncoder()->BindLabel(label);
        test->GetEncoder()->EncodeMov(ret_val, Imm(0));
    } else {
        test->GetEncoder()->EncodeAddOverflow(end, ret_val, param1, param2, Condition::VC);
        test->GetEncoder()->EncodeMov(ret_val, Imm(0));
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
    std::array<T, 7> values = {0, 2, 5, -7, -10, max, min};
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

template <typename T, bool is_equal = true>
bool TestSubOverflow(Encoder64Test *test, T value1, T value2, T expected)
{
    test->PreWork();
    auto param1 = test->GetParameter(TypeInfo(T(0)), 0);
    auto param2 = test->GetParameter(TypeInfo(T(0)), 1);
    auto ret_val = Target::Current().GetReturnReg(TypeInfo(T(0)));
    auto label = test->GetEncoder()->CreateLabel();
    auto end = test->GetEncoder()->CreateLabel();

    if (is_equal) {
        test->GetEncoder()->EncodeSubOverflow(label, ret_val, param1, param2, Condition::VS);
        test->GetEncoder()->EncodeJump(end);
        test->GetEncoder()->BindLabel(label);
        test->GetEncoder()->EncodeMov(ret_val, Imm(0));
    } else {
        test->GetEncoder()->EncodeSubOverflow(end, ret_val, param1, param2, Condition::VC);
        test->GetEncoder()->EncodeMov(ret_val, Imm(0));
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
    std::array<T, 7> values = {0, 2, 5, -7, -10, max, min};
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

template <typename T, Condition cc>
bool TestJumpCC(Encoder64Test *test)
{
    bool is_signed = std::is_signed<T>::value;
    // Initialize
    test->PreWork();
    // First type-dependency
    auto param = test->GetParameter(TypeInfo(T(0)));
    // Main test call
    auto ret_val = Target::Current().GetReturnReg();

    auto tsucc = test->GetEncoder()->CreateLabel();

    test->GetEncoder()->EncodeJump(tsucc, param, cc);
    test->GetEncoder()->EncodeMov(param, Imm(0x0));
    test->GetEncoder()->EncodeMov(ret_val, Imm(0x1));
    test->GetCallconv()->GenerateEpilogue(FrameInfo::FullPrologue(), []() {});

    test->GetEncoder()->BindLabel(tsucc);
    test->GetEncoder()->EncodeMov(param, Imm(0x0));
    test->GetEncoder()->EncodeMov(ret_val, Imm(0x0));
    // test->GetEncoder()->EncodeReturn(); < encoded in PostWork

    // Finalize
    test->PostWork();

    // If encode unsupported - now print error
    if (!test->GetEncoder()->GetResult()) {
        std::cerr << "Unsupported for " << TypeName<T>() << "\n";
        return false;
    }
    // Change this for enable print disasm
    test->Dump(false);

    if constexpr (cc == Condition::EQ) {
        if (!test->CallCode<T>(0, 0)) {
            std::cerr << "zero EQ test fail \n";
            return false;
        }
    }
    if constexpr (cc == Condition::NE) {
        if (!test->CallCode<T>(0, 1)) {
            std::cerr << "zero EQ test fail \n";
            return false;
        }
    }

    // Main test loop:
    for (uint64_t i = 0; i < ITERATION; ++i) {
        // Second type-dependency
        T tmp = random_gen<T>();
        if (tmp == 0) {  // Only non-zero values
            tmp += 1;
        }
        // Deduced conflicting types for parameter

        if constexpr (cc == Condition::EQ) {
            if (!test->CallCode<T>(tmp, 1)) {
                std::cerr << "non-zero EQ test fail " << tmp << " \n";
                return false;
            }
        }
        if constexpr (cc == Condition::NE) {
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

// Jump with cc
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
    constexpr bool notZero = true;
    EXPECT_TRUE((TestBitTestAndBranch<uint32_t, notZero>(this)));
    EXPECT_TRUE((TestBitTestAndBranch<uint64_t, notZero>(this)));
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
    bool is_signed = std::is_signed<T>::value;
    // Initialize
    test->PreWork();
    // First type-dependency
    // Some strange code - default parameter is addres (32 bit)

    auto param = Target::Current().GetReturnReg();
    // But return value is cutted by loaded value
    auto ret_val = test->GetParameter(TypeInfo(T(0)));

    auto mem = MemRef(param);
    test->GetEncoder()->EncodeLdr(ret_val, is_signed, mem);

    // Finalize
    test->PostWork();

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
        T tmp = random_gen<T>();
        T *ptr = &tmp;
        // Test : param - Pointer to value
        //        return - value (loaded by ptr)
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
    // TODO(igorban) : additional test for full memory model:
    //                 + mem(base + index<<scale + disp)
}

template <typename T>
bool TestStr(Encoder64Test *test)
{
    // Initialize
    test->PreWork();
    // First type-dependency
    auto param = test->GetParameter(TypeInfo(int64_t(0)), 0);
    // Data to be stored:
    auto stored_value = test->GetParameter(TypeInfo(T(0)), 1);
    if constexpr ((std::is_same<float, T>::value) || std::is_same<double, T>::value) {
        stored_value = test->GetParameter(TypeInfo(T(0)), 0);
    }

    auto mem = MemRef(param);
    test->GetEncoder()->EncodeStr(stored_value, mem);
    // Finalize
    test->PostWork();

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
        T tmp = random_gen<T>();
        T ret_data = 0;
        T *ptr = &ret_data;

        // Test : param - Pointer to value
        //        return - value (loaded by ptr)
        // Value is resulting type, but call is ptr_type
        auto result = test->CallCodeStore<T>(reinterpret_cast<int64_t>(ptr), tmp);
        // Store must change ret_data value
        if (!(ret_data == tmp || (std::is_floating_point_v<T> && std::isnan(ret_data) && std::isnan(tmp)))) {
            std::cerr << std::hex << "Str test fail " << tmp << " ret_data = " << ret_data << "\n";
            return false;
        }
    }
    return true;
}

// Simple store test
TEST_F(Encoder64Test, StrTest)
{
    //  EncodeStr(Reg src, MemRef mem)
    EXPECT_TRUE((TestStr<int32_t>(this)));
    EXPECT_TRUE((TestStr<int64_t>(this)));
    EXPECT_TRUE((TestStr<uint32_t>(this)));
    EXPECT_TRUE((TestStr<uint64_t>(this)));
    EXPECT_TRUE((TestStr<float>(this)));
    EXPECT_TRUE((TestStr<double>(this)));

    // TODO(igorban) : additional test for full memory model:
    //                 + mem(base + index<<scale + disp)
}

// Store immediate test
// TEST_F(Encoder64Test, StiTest) {
//  EncodeSti(Imm src, MemRef mem)

template <typename T>
bool TestStrz(Encoder64Test *test)
{
    // Initialize
    test->PreWork();
    // First type-dependency
    auto param = test->GetParameter(TypeInfo(int64_t(0)), 0);
    // Data to be stored:
    auto stored_value = test->GetParameter(TypeInfo(T(0)), 1);
    if constexpr (std::is_floating_point_v<T>) {
        stored_value = test->GetParameter(TypeInfo(T(0)), 0);
    }

    auto mem = MemRef(param);
    test->GetEncoder()->EncodeStrz(stored_value, mem);
    // Finalize
    test->PostWork();

    // If encode unsupported - now print error
    if (!test->GetEncoder()->GetResult()) {
        std::cerr << "Unsupported for " << TypeName<T>() << "\n";
        return false;
    }
    // Change this for enable print disasm
    test->Dump(false);

    // Main test loop:
    for (uint64_t i = 0; i < ITERATION; ++i) {
        // Test : param - Pointer to value
        //        return - value (loaded by ptr)
        // Value is resulting type, but call is ptr_type
        if constexpr (std::is_floating_point_v<T>) {
            // Second type-dependency
            T tmp = random_gen<T>();
            T ret_data[ARRAY_SIZE];
            T *ptr = ret_data;
            for (uint64_t j = 0; j < ARRAY_SIZE; ++j) {
                ret_data[j] = random_gen<T>();
            }

            auto result = test->CallCodeStore<T>(reinterpret_cast<int64_t>(ptr), tmp);
            // Store must change ret_data value
            bool ret = (ret_data[0] == tmp && std::signbit(ret_data[0]) == std::signbit(tmp)) ||
                       (std::isnan(ret_data[0]) && std::isnan(tmp)) || (std::isinf(ret_data[0]) && std::isinf(tmp));
            if (!ret) {
                if constexpr (std::is_same<float, T>::value) {
                    std::cerr << std::hex << "Strz test fail " << bit_cast<uint32_t>(tmp)
                              << " ret_data = " << bit_cast<uint32_t>(ret_data[0]) << "\n";
                }
                if constexpr (std::is_same<double, T>::value) {
                    std::cerr << std::hex << "Strz test fail " << bit_cast<uint64_t>(tmp)
                              << " ret_data = " << bit_cast<uint64_t>(ret_data[0]) << "\n";
                }
                std::cerr << std::hex << "Strz test fail " << tmp << " ret_data = " << ret_data[0] << "\n";
                return false;
            }
        } else {
            // Second type-dependency
            T tmp = random_gen<T>();
            uint64_t ret_data = 0xffffffffffffffff;
            T *ptr = reinterpret_cast<T *>(&ret_data);

            uint64_t result = test->CallCodeStore<T>(reinterpret_cast<int64_t>(ptr), tmp);
            // Store must change ret_data value
            if (ret_data != tmp) {
                std::cerr << std::hex << "Strz test fail " << (uint64_t)tmp << " ret_data = " << (uint64_t)ret_data
                          << "\n";
                return false;
            }
        }
    }
    return true;
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
//  EncodeReturn()

bool foo(uint32_t param1, uint32_t param2)
{
    // TODO(igorban): use variables
    return (param1 == param2);
}

using funct_ptr = bool (*)(uint32_t param1, uint32_t param2);

funct_ptr foo_ptr = &foo;

// Call Test
TEST_F(Encoder64Test, CallTest)
{
    // Initialize
    auto link_reg = Target::Current().GetLinkReg();

    PreWork();
    static_cast<aarch64::Aarch64Encoder *>(GetEncoder())
        ->GetMasm()
        ->Push(vixl::aarch64::xzr, aarch64::VixlReg(link_reg));

    // Call foo
    GetEncoder()->MakeCall(reinterpret_cast<void *>(foo_ptr));

    static_cast<aarch64::Aarch64Encoder *>(GetEncoder())
        ->GetMasm()
        ->Pop(aarch64::VixlReg(link_reg), vixl::aarch64::xzr);
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
    auto tmp1 = random_gen<uint32_t>();
    uint32_t tmp2 = tmp1;
    // first template arg - parameter type, second - return type
    auto result = CallCodeCall<uint32_t, bool>(tmp1, tmp2);
    // Store must change ret_data value
    if (!result) {
        std::cerr << std::hex << "Call test fail tmp1=" << tmp1 << " tmp2=" << tmp2 << " result =" << result << "\n";
    }
    EXPECT_TRUE(result);

    // Main test loop:
    for (uint64_t i = 0; i < ITERATION; ++i) {
        // Second type-dependency
        auto tmp1 = random_gen<uint32_t>();
        auto tmp2 = random_gen<uint32_t>();

        // first template arg - parameter type, second - return type
        auto result = CallCodeCall<uint32_t, bool>(tmp1, tmp2);
        auto ret_data = (tmp1 == tmp2);

        // Store must change ret_data value
        if (result != ret_data) {
            std::cerr << std::hex << "Call test fail tmp1=" << tmp1 << " tmp2=" << tmp2 << " ret_data = " << ret_data
                      << " result =" << result << "\n";
        }
        EXPECT_EQ(result, ret_data);
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
        std::cerr << "Unsupported for " << TypeName<T>() << "\n";
        return false;
    }
    // Change this for enable print disasm
    test->Dump(false);

    // Main test loop:
    for (uint64_t i = 0; i < ITERATION; ++i) {
        // Second type-dependency
        T tmp = random_gen<T>();
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
        std::cerr << "Unsupported for " << TypeName<T>() << "\n";
        return false;
    }
    // Change this for enable print disasm
    test->Dump(false);

    // Main test loop:
    for (uint64_t i = 0; i < ITERATION; ++i) {
        // Second type-dependency
        T tmp = random_gen<T>();
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
        std::cerr << "Unsupported for " << TypeName<T>() << "\n";
        return false;
    }
    // Change this for enable print disasm
    test->Dump(false);

    // Main test loop:
    for (uint64_t i = 0; i < ITERATION; ++i) {
        // Second type-dependency
        auto tmp1 = random_gen<T>();
        auto tmp2 = random_gen<T>();
        // Deduced conflicting types for parameter

        // Main check - compare parameter and
        // return value
        if (!test->CallCode<T>(tmp1, tmp2, tmp1 + tmp2)) {
            return false;
        }
    }

    if constexpr (std::is_floating_point_v<T>) {
        T nan = std::numeric_limits<T>::quiet_NaN();

        if (!test->CallCode<T>(nan, random_gen<T>(), nan)) {
            return false;
        }
        if (!test->CallCode<T>(random_gen<T>(), nan, nan)) {
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
    T param2 = random_gen<T>();

    // Main test call
    test->GetEncoder()->EncodeAdd(param1, param1, Imm(param2));

    // Finalize
    test->PostWork();

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
        T tmp1 = random_gen<T>();
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
        std::cerr << "Unsupported for " << TypeName<T>() << "\n";
        return false;
    }
    // Change this for enable print disasm
    test->Dump(false);

    // Main test loop:
    for (uint64_t i = 0; i < ITERATION; ++i) {
        // Second type-dependency
        T tmp1 = random_gen<T>();
        T tmp2 = random_gen<T>();
        // Deduced conflicting types for parameter

        // Main check - compare parameter and
        // return value
        if (!test->CallCode<T>(tmp1, tmp2, tmp1 - tmp2)) {
            return false;
        }
    }

    if constexpr (std::is_floating_point_v<T>) {
        T nan = std::numeric_limits<T>::quiet_NaN();

        if (!test->CallCode<T>(nan, random_gen<T>(), nan)) {
            return false;
        }
        if (!test->CallCode<T>(random_gen<T>(), nan, nan)) {
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
    T param2 = random_gen<T>();

    // Main test call
    test->GetEncoder()->EncodeSub(param1, param1, Imm(param2));

    // Finalize
    test->PostWork();

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
        T tmp1 = random_gen<T>();
        // Deduced conflicting types for parameter
        // Main check - compare parameter and
        // return value
        if (!test->CallCode<T>(tmp1, tmp1 - param2)) {
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
        std::cerr << "Unsupported for " << TypeName<T>() << "\n";
        return false;
    }
    // Change this for enable print disasm
    test->Dump(false);

    // Main test loop:
    for (uint64_t i = 0; i < ITERATION; ++i) {
        // Second type-dependency
        T tmp1 = random_gen<T>();
        T tmp2 = random_gen<T>();
        // Deduced conflicting types for parameter

        // Main check - compare parameter and
        // return value
        if (!test->CallCode<T>(tmp1, tmp2, tmp1 * tmp2)) {
            return false;
        }
    }

    if constexpr (std::is_floating_point_v<T>) {
        T nan = std::numeric_limits<T>::quiet_NaN();

        if (!test->CallCode<T>(nan, random_gen<T>(), nan)) {
            return false;
        }
        if (!test->CallCode<T>(random_gen<T>(), nan, nan)) {
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
        std::cerr << "Unsupported for " << TypeName<T>() << "\n";
        return false;
    }
    // Change this for enable print disasm
    test->Dump(false);

    // Main test loop:
    for (uint64_t i = 0; i < ITERATION; ++i) {
        // Second type-dependency
        T tmp1 = random_gen<T>();
        T tmp2 = random_gen<T>();
        // Deduced conflicting types for parameter

        T result {0};
        if (std::is_floating_point_v<T> && (std::isnan(tmp1) || std::isnan(tmp2))) {
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

        if (!test->CallCode<T>(nan, random_gen<T>(), nan)) {
            return false;
        }
        if (!test->CallCode<T>(random_gen<T>(), nan, nan)) {
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
        std::cerr << "Unsupported for " << TypeName<T>() << "\n";
        return false;
    }
    // Change this for enable print disasm
    test->Dump(false);

    // Main test loop:
    for (uint64_t i = 0; i < ITERATION; ++i) {
        // Second type-dependency
        T tmp1 = random_gen<T>();
        T tmp2 = random_gen<T>();
        // Deduced conflicting types for parameter
        T result {0};
        if (std::is_floating_point_v<T> && (std::isnan(tmp1) || std::isnan(tmp2))) {
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

        if (!test->CallCode<T>(nan, random_gen<T>(), nan)) {
            return false;
        }
        if (!test->CallCode<T>(random_gen<T>(), nan, nan)) {
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
        std::cerr << "Unsupported for " << TypeName<T>() << "\n";
        return false;
    }
    // Change this for enable print disasm
    test->Dump(false);

    // Main test loop:
    for (uint64_t i = 0; i < ITERATION; ++i) {
        // Second type-dependency
        T tmp1 = random_gen<T>();
        T tmp2 {0};
        if constexpr (std::is_same_v<T, int64_t>) {
            tmp2 = random_gen<uint8_t>() % DOUBLE_WORD_SIZE;
        } else {
            tmp2 = random_gen<uint8_t>() % WORD_SIZE;
        }
        // Deduced conflicting types for parameter

        // Main check - compare parameter and
        // return value
        bool result {false};

        if constexpr (std::is_same_v<T, int8_t> || std::is_same_v<T, int16_t>) {
            result = test->CallCode<T>(tmp1, tmp2, tmp1 << (tmp2 & (CHAR_BIT * sizeof(T) - 1)));
        } else {
            result = test->CallCode<T>(tmp1, tmp2, tmp1 << tmp2);
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
        std::cerr << "Unsupported for " << TypeName<T>() << "\n";
        return false;
    }
    // Change this for enable print disasm
    test->Dump(false);

    // Main test loop:
    for (uint64_t i = 0; i < ITERATION; ++i) {
        // Second type-dependency
        T tmp1 = random_gen<T>();
        T tmp2 {0};
        if constexpr (sizeof(T) == sizeof(int64_t)) {
            tmp2 = random_gen<uint8_t>() % DOUBLE_WORD_SIZE;
        } else {
            tmp2 = random_gen<uint8_t>() % WORD_SIZE;
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
        std::cerr << "Unsupported for " << TypeName<T>() << "\n";
        return false;
    }
    // Change this for enable print disasm
    test->Dump(false);

    // Main test loop:
    for (uint64_t i = 0; i < ITERATION; ++i) {
        // Second type-dependency
        T tmp1 = random_gen<T>();
        T tmp2 {0};
        if constexpr (std::is_same_v<T, int64_t>) {
            tmp2 = random_gen<uint8_t>() % DOUBLE_WORD_SIZE;
        } else {
            tmp2 = random_gen<uint8_t>() % WORD_SIZE;
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
        std::cerr << "Unsupported for " << TypeName<T>() << "\n";
        return false;
    }
    // Change this for enable print disasm
    test->Dump(false);

    // Main test loop:
    for (uint64_t i = 0; i < ITERATION; ++i) {
        // Second type-dependency
        T tmp1 = random_gen<T>();
        T tmp2 = random_gen<T>();
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
        std::cerr << "Unsupported for " << TypeName<T>() << "\n";
        return false;
    }
    // Change this for enable print disasm
    test->Dump(false);

    // Main test loop:
    for (uint64_t i = 0; i < ITERATION; ++i) {
        // Second type-dependency
        T tmp1 = random_gen<T>();
        T tmp2 = random_gen<T>();
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
        std::cerr << "Unsupported for " << TypeName<T>() << "\n";
        return false;
    }
    // Change this for enable print disasm
    test->Dump(false);

    // Main test loop:
    for (uint64_t i = 0; i < ITERATION; ++i) {
        // Second type-dependency
        T tmp1 = random_gen<T>();
        T tmp2 = random_gen<T>();
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
        param2 = random_gen<uint8_t>() % DOUBLE_WORD_SIZE;
    } else {
        param2 = random_gen<uint8_t>() % WORD_SIZE;
    }

    // Main test call
    test->GetEncoder()->EncodeShl(param1, param1, Imm(param2));

    // Finalize
    test->PostWork();

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
        T tmp1 = random_gen<T>();

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
        param2 = random_gen<uint8_t>() % DOUBLE_WORD_SIZE;
    } else {
        param2 = random_gen<uint8_t>() % WORD_SIZE;
    }

    // Main test call
    test->GetEncoder()->EncodeShr(param1, param1, Imm(param2));

    // Finalize
    test->PostWork();

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
        T tmp1 = random_gen<T>();

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
        std::cerr << "Unsupported for " << TypeName<T>() << "\n";
        return false;
    }
    // Change this for enable print disasm
    test->Dump(false);

    // Main test loop:
    for (uint64_t i = 0; i < ITERATION; ++i) {
        // Second type-dependency
        T tmp1 = random_gen<T>();
        T tmp2 = random_gen<T>();
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
bool TestFcmp(Encoder64Test *test, bool is_fcmpg)
{
    static_assert(std::is_floating_point_v<T>);
    // Initialize
    test->PreWork();

    // First type-dependency
    auto output = test->GetParameter(TypeInfo(int32_t(0)), 0);
    auto param1 = test->GetParameter(TypeInfo(T(0)), 0);
    auto param2 = test->GetParameter(TypeInfo(T(0)), 1);

    // Main test call
    test->GetEncoder()->EncodeCmp(output, param1, param2, is_fcmpg ? Condition::MI : Condition::LT);

    // Finalize
    test->PostWork();

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
        T tmp1 = random_gen<T>();
        T tmp2 = random_gen<T>();
        // Deduced conflicting types for parameter

        auto compare = [](T a, T b) -> int32_t { return a < b ? -1 : a > b ? 1 : 0; };

        int32_t result {0};
        if (std::isnan(tmp1) || std::isnan(tmp2)) {
            result = is_fcmpg ? 1 : -1;
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

        if (!test->CallCode<T, int32_t>(nan, 5.0, is_fcmpg ? 1 : -1)) {
            return false;
        }
        if (!test->CallCode<T, int32_t>(5.0, nan, is_fcmpg ? 1 : -1)) {
            return false;
        }
        if (!test->CallCode<T, int32_t>(nan, nan, is_fcmpg ? 1 : -1)) {
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
        std::cerr << "Unsupported for " << TypeName<T>() << "\n";
        return false;
    }
    // Change this for enable print disasm
    test->Dump(false);

    uint64_t hi = static_cast<uint64_t>(0xABCDEFU) << (BITS_PER_BYTE * sizeof(uint32_t));

    // Main test loop:
    for (uint64_t i = 0; i < ITERATION; ++i) {
        uint32_t lo = random_gen<T>();

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
        std::cerr << "Unsupported for " << TypeName<T>() << "\n";
        return false;
    }
    // Change this for enable print disasm
    test->Dump(false);

    // Main test loop:
    for (uint64_t i = 0; i < ITERATION; ++i) {
        // Second type-dependency
        T tmp1 = random_gen<T>();
        T tmp2 = random_gen<T>();
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
        std::cerr << "Unsupported for " << TypeName<T>() << "\n";
        return false;
    }
    // Change this for enable print disasm
    test->Dump(false);

    uint64_t hi = static_cast<uint64_t>(0xABCDEFU) << (BITS_PER_BYTE * sizeof(uint32_t));

    // Main test loop:
    for (uint64_t i = 0; i < ITERATION; ++i) {
        uint32_t lo = random_gen<T>();

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
        typedef typename std::conditional<(sizeof(Dst) * BYTE_SIZE) >= WORD_SIZE, Dst, uint32_t>::type DstExt;

        // Second type-dependency
        auto src = random_gen<Src>();
        DstExt dst = static_cast<DstExt>(static_cast<Dst>(src));

        // Other cast for float->int
        if (std::is_floating_point<Src>() && !std::is_floating_point<Dst>()) {
            auto MIN_INT = std::numeric_limits<Dst>::min();
            auto MAX_INT = std::numeric_limits<Dst>::max();
            auto FLOAT_MIN_INT = static_cast<Src>(MIN_INT);
            auto FLOAT_MAX_INT = static_cast<Src>(MAX_INT);

            if (src > FLOAT_MIN_INT) {
                if (src < FLOAT_MAX_INT) {
                    dst = static_cast<Dst>(src);
                } else {
                    dst = MAX_INT;
                }
            } else if (std::isnan(src)) {
                dst = 0;
            } else {
                dst = MIN_INT;
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
        typedef typename std::conditional<(sizeof(Dst) * BYTE_SIZE) >= WORD_SIZE, Dst, uint32_t>::type DstExt;

        if (!test->CallCode<Src, DstExt>(nan, static_cast<DstExt>((Dst(0))))) {
            return false;
        }
    }

    return true;
}

TEST_F(Encoder64Test, CastTest)
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

template <typename T>
bool TestDiv(Encoder64Test *test)
{
    bool is_signed = std::is_signed<T>::value;

    test->PreWork();

    // First type-dependency
    auto param_1 = test->GetParameter(TypeInfo(T(0)), 0);
    auto param_2 = test->GetParameter(TypeInfo(T(0)), 1);

    // Main test call
    test->GetEncoder()->EncodeDiv(param_1, is_signed, param_1, param_2);

    // Finalize
    test->PostWork();

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
        T tmp1 = random_gen<T>();
        T tmp2 = random_gen<T>();
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

        if (!test->CallCode<T>(nan, random_gen<T>(), nan)) {
            return false;
        }
        if (!test->CallCode<T>(random_gen<T>(), nan, nan)) {
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
bool TestMod(Encoder64Test *test)
{
    bool is_signed = std::is_signed<T>::value;

    auto link_reg = Target::Current().GetLinkReg();

    test->PreWork();
    static_cast<aarch64::Aarch64Encoder *>(test->GetEncoder())
        ->GetMasm()
        ->Push(vixl::aarch64::xzr, aarch64::VixlReg(link_reg));

    // First type-dependency
    auto param_1 = test->GetParameter(TypeInfo(T(0)), 0);
    auto param_2 = test->GetParameter(TypeInfo(T(0)), 1);

    // Main test call
    test->GetEncoder()->EncodeMod(param_1, is_signed, param_1, param_2);

    // Finalize
    static_cast<aarch64::Aarch64Encoder *>(test->GetEncoder())
        ->GetMasm()
        ->Pop(aarch64::VixlReg(link_reg), vixl::aarch64::xzr);
    test->PostWork();

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
        T tmp1 = random_gen<T>();
        T tmp2 = random_gen<T>();
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

    if constexpr (std::is_floating_point_v<T>) {
        T nan = std::numeric_limits<T>::quiet_NaN();

        if (!test->CallCode<T>(nan, random_gen<T>(), nan)) {
            return false;
        }
        if (!test->CallCode<T>(random_gen<T>(), nan, nan)) {
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

TEST_F(Encoder64Test, ModTest)
{
    EXPECT_TRUE(TestMod<int32_t>(this));
    EXPECT_TRUE(TestMod<int64_t>(this));
    EXPECT_TRUE(TestMod<uint32_t>(this));
    EXPECT_TRUE(TestMod<uint64_t>(this));
    EXPECT_TRUE(TestMod<float>(this));
    EXPECT_TRUE(TestMod<double>(this));
}

// MemCopy Test
// TEST_F(Encoder64Test, MemCopyTest) {
//  EncodeMemCopy(MemRef mem_from, MemRef mem_to, size_t size)

// MemCopyz Test
// TEST_F(Encoder64Test, MemCopyzTest) {
//  EncodeMemCopyz(MemRef mem_from, MemRef mem_to, size_t size)

template <int ID, typename T>
bool TestParam(Encoder64Test *test)
{
    // int32_t uint64_t int32_t  int64_t         int32_t int32_t
    //   r0    r2+r3   stack0  stack2(align)   stack4
    using funct_ptr = uint64_t (*)(uint32_t, uint64_t, int32_t, int64_t, int32_t, int32_t);

    bool is_signed = std::is_signed<T>::value;

    constexpr TypeInfo params[6] = {INT32_TYPE, INT64_TYPE, INT32_TYPE, INT64_TYPE, INT32_TYPE, INT32_TYPE};
    TypeInfo curr_param = params[ID];

    auto param_info = test->GetCallconv()->GetParameterInfo(0);
    auto par = param_info->GetNativeParam(params[0]);
    // First ret value geted
    for (int i = 1; i <= ID; ++i) {
        par = param_info->GetNativeParam(params[i]);
    }

    auto link_reg = Target::Current().GetLinkReg();
    ArenaVector<Reg> used_regs(test->GetAllocator()->Adapter());
    used_regs.emplace_back(link_reg);
    test->PreWork();

    auto ret_reg = test->GetParameter(curr_param, 0);

    // Main test call
    if (std::holds_alternative<Reg>(par)) {
        test->GetEncoder()->EncodeMov(ret_reg, std::get<Reg>(par));
    } else {
        auto mem = MemRef(Target::Current().GetStackReg(), std::get<uint8_t>(par) * sizeof(uint32_t));
        test->GetEncoder()->EncodeLdr(ret_reg, is_signed, mem);
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
    auto func = reinterpret_cast<funct_ptr>(ptr);

    // Main test loop:
    for (uint64_t i = 0; i < ITERATION; ++i) {
        // Second type-dependency
        int32_t param_0 = random_gen<uint32_t>();
        uint64_t param_1 = random_gen<uint64_t>();
        int32_t param_2 = random_gen<int32_t>();
        int64_t param_3 = random_gen<int64_t>();
        int32_t param_4 = random_gen<int32_t>();
        int32_t param_5 = random_gen<int32_t>();

        // Main check - compare parameter and
        // return value
        const T curr_result = func(param_0, param_1, param_2, param_3, param_4, param_5);
        T result;
        if constexpr (ID == 0) {
            result = param_0;
        }
        if constexpr (ID == 1) {
            result = param_1;
        }
        if constexpr (ID == 2) {
            result = param_2;
        }
        if constexpr (ID == 3) {
            result = param_3;
        }
        if constexpr (ID == 4) {
            result = param_4;
        }
        if constexpr (ID == 5) {
            result = param_5;
        }
        if (curr_result != result) {
            return false;
        };
    }
    return true;
}

TEST_F(Encoder64Test, ReadParams)
{
    EXPECT_TRUE((TestParam<0, int32_t>(this)));
    EXPECT_TRUE((TestParam<1, uint64_t>(this)));
    EXPECT_TRUE((TestParam<2, int32_t>(this)));
    EXPECT_TRUE((TestParam<3, int64_t>(this)));
    EXPECT_TRUE((TestParam<4, int32_t>(this)));
    EXPECT_TRUE((TestParam<5, int32_t>(this)));
}

template <typename T, Condition cc>
bool TestSelect(Encoder64Test *test)
{
    // Initialize
    test->PreWork();

    // First type-dependency
    auto param0 = test->GetParameter(TypeInfo(T(0)), 0);
    auto param1 = test->GetParameter(TypeInfo(T(0)), 1);
    auto param2 = test->GetParameter(TypeInfo(uint32_t(0)), 2);
    auto param3 = test->GetParameter(TypeInfo(uint32_t(0)), 3);

    // Main test call
    test->GetEncoder()->EncodeMov(param2, Imm(1));
    test->GetEncoder()->EncodeMov(param3, Imm(0));
    test->GetEncoder()->EncodeSelect(Reg(param0.GetId(), TypeInfo(uint32_t(0))), param2, param3, param0, param1, cc);

    // Finalize
    test->PostWork();

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
        T tmp0 = random_gen<T>();
        T tmp1 = random_gen<T>();

        bool res {false};
        switch (cc) {
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

template <typename T, Condition cc, bool is_imm>
bool TestSelectTest(Encoder64Test *test)
{
    // Initialize
    test->PreWork();

    auto param0 = test->GetParameter(TypeInfo(T(0)), 0);
    auto param1 = test->GetParameter(TypeInfo(T(0)), 1);
    auto param2 = test->GetParameter(TypeInfo(uint32_t(0)), 2);
    auto param3 = test->GetParameter(TypeInfo(uint32_t(0)), 3);
    // truncate immediate value to max imm size
    [[maybe_unused]] T imm_value = random_mask_gen<T>();

    // Main test call
    test->GetEncoder()->EncodeMov(param2, Imm(1));
    test->GetEncoder()->EncodeMov(param3, Imm(0));

    if constexpr (is_imm) {
        test->GetEncoder()->EncodeSelectTest(Reg(param0.GetId(), TypeInfo(uint32_t(0))), param2, param3, param0,
                                             Imm(imm_value), cc);
    } else {
        test->GetEncoder()->EncodeSelectTest(Reg(param0.GetId(), TypeInfo(uint32_t(0))), param2, param3, param0, param1,
                                             cc);
    }

    // Finalize
    test->PostWork();

    // If encode unsupported - now print error
    if (!test->GetEncoder()->GetResult()) {
        std::cerr << "Unsupported for " << TypeName<T>() << "\n";
        return false;
    }
    // Change this for enable print disasm
    test->Dump(false);

    // Main test loop:
    for (uint64_t i = 0; i < ITERATION; ++i) {
        T tmp0 = random_gen<T>();
        T tmp1;

        if constexpr (is_imm) {
            tmp1 = imm_value;
        } else {
            tmp1 = random_gen<T>();
        }

        T and_res = tmp0 & tmp1;
        bool res = cc == Condition::TST_EQ ? and_res == 0 : and_res != 0;

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

template <typename T, Condition cc>
bool TestCompareTest(Encoder64Test *test)
{
    // Initialize
    test->PreWork();

    // First type-dependency
    auto param1 = test->GetParameter(TypeInfo(T(0)), 0);
    auto param2 = test->GetParameter(TypeInfo(T(0)), 1);

    // Main test call
    test->GetEncoder()->EncodeCompareTest(param1, param1, param2, cc);

    // Finalize
    test->PostWork();

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
        T tmp1 = random_gen<T>();
        T tmp2 = random_gen<T>();
        // Deduced conflicting types for parameter

        auto compare = [](T a, T b) -> int32_t { return cc == Condition::TST_EQ ? (a & b) == 0 : (a & b) != 0; };

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

template <typename T, Condition cc, bool is_imm>
bool TestJumpTest(Encoder64Test *test)
{
    // Initialize
    test->PreWork();

    // First type-dependency
    auto param0 = test->GetParameter(TypeInfo(T(0)), 0);
    auto param1 = test->GetParameter(TypeInfo(T(0)), 1);
    auto ret_val = Target::Current().GetParamReg(0);

    auto true_branch = test->GetEncoder()->CreateLabel();
    auto end = test->GetEncoder()->CreateLabel();
    [[maybe_unused]] T imm_value = random_mask_gen<T>();

    // Main test call
    if constexpr (is_imm) {
        test->GetEncoder()->EncodeJumpTest(true_branch, param0, Imm(imm_value), cc);
    } else {
        test->GetEncoder()->EncodeJumpTest(true_branch, param0, param1, cc);
    }
    test->GetEncoder()->EncodeMov(ret_val, Imm(0));
    test->GetEncoder()->EncodeJump(end);

    test->GetEncoder()->BindLabel(true_branch);
    test->GetEncoder()->EncodeMov(ret_val, Imm(1));

    test->GetEncoder()->BindLabel(end);
    // Finalize
    test->PostWork();

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
        T tmp1 = random_gen<T>();
        T tmp2;
        if constexpr (is_imm) {
            tmp2 = imm_value;
        } else {
            tmp2 = random_gen<T>();
        }
        // Deduced conflicting types for parameter

        auto compare = [](T a, T b) -> int32_t { return cc == Condition::TST_EQ ? (a & b) == 0 : (a & b) != 0; };

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

template <typename T, bool is_acquire>
bool TestLoadExclusive(Encoder64Test *test, uint64_t input_word, T expected_result)
{
    uint64_t buffer[3] = {0xFFFFFFFFFFFFFFFFULL, input_word, 0xFFFFFFFFFFFFFFFFULL};
    test->PreWork();
    auto param_0 = test->GetParameter(TypeInfo(uintptr_t(0)), 0);
    auto result = test->GetParameter(TypeInfo(T(0)), 0);
    test->GetEncoder()->EncodeLdrExclusive(result, param_0, is_acquire);
    test->PostWork();

    if (!test->GetEncoder()->GetResult()) {
        std::cerr << "Unsupported for " << TypeName<T>() << "\n";
        return false;
    }
    // Change this for enable print disasm
    test->Dump(false);
    auto word_addr = reinterpret_cast<uintptr_t>(&buffer[1]);
    return test->CallCode<uintptr_t, T>(word_addr, expected_result);
}

template <typename T, bool is_release>
bool TestStoreExclusiveFailed(Encoder64Test *test)
{
    uint64_t buffer = 0xFFFFFFFFFFFFFFFFULL;
    test->PreWork();
    auto param_0 = test->GetParameter(TypeInfo(uintptr_t(0)), 0);
    auto tmp_reg = test->GetParameter(TypeInfo(T(0)), 1);
    auto result = test->GetParameter(TypeInfo(T(0)), 0);

    // perform store without load - should fail as there are no exlusive monitor
    test->GetEncoder()->EncodeStrExclusive(result, tmp_reg, param_0, is_release);
    test->PostWork();

    if (!test->GetEncoder()->GetResult()) {
        std::cerr << "Unsupported for " << TypeName<T>() << "\n";
        return false;
    }
    // Change this for enable print disasm
    test->Dump(false);
    auto word_addr = reinterpret_cast<uintptr_t>(&buffer);
    return test->CallCode<uintptr_t, T>(word_addr, static_cast<T>(buffer), 1);
}

template <typename T, bool is_release>
bool TestStoreExclusive(Encoder64Test *test, T value, uint64_t expected_result)
{
    // test writes value into buffer[1]
    uint64_t buffer[3] = {0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL};
    test->PreWork();
    auto param_0 = test->GetParameter(TypeInfo(uintptr_t(0)), 0);
    auto param_1 = test->GetParameter(TypeInfo(T(0)), 1);
    auto result = test->GetParameter(TypeInfo(T(0)), 0);
    auto retry_label = test->GetEncoder()->CreateLabel();
    Reg tmp_reg(test->GetRegfile()->GetZeroReg().GetId(), TypeInfo(T(0)));

    test->GetEncoder()->BindLabel(retry_label);
    test->GetEncoder()->EncodeLdrExclusive(tmp_reg, param_0, false);
    test->GetEncoder()->EncodeStrExclusive(result, param_1, param_0, is_release);
    // retry if store exclusive returned non zero value
    test->GetEncoder()->EncodeJump(retry_label, result, Condition::NE);

    test->PostWork();

    if (!test->GetEncoder()->GetResult()) {
        std::cerr << "Unsupported for " << TypeName<T>() << "\n";
        return false;
    }
    // Change this for enable print disasm
    test->Dump(false);
    auto word_addr = reinterpret_cast<uintptr_t>(&buffer[1]);
    if (!test->CallCode<uintptr_t, T>(word_addr, value, 0)) {
        return false;
    }
    if (buffer[1] != expected_result) {
        std::cerr << "Failed: expected value " << std::hex << expected_result << " differs from actual result "
                  << buffer[1] << std::dec << std::endl;
        return false;
    }
    return true;
}

TEST_F(Encoder64Test, LoadExclusiveTest)
{
    const uint64_t SOURCE_WORD = 0x1122334455667788ULL;
    // aarch64 is little-endian, so bytes are actually stored in following order:
    // 0x 88 77 66 55 44 33 22 11
    EXPECT_TRUE((TestLoadExclusive<uint8_t, false>(this, SOURCE_WORD, 0x88U)));
    EXPECT_TRUE((TestLoadExclusive<uint16_t, false>(this, SOURCE_WORD, 0x7788U)));
    EXPECT_TRUE((TestLoadExclusive<uint32_t, false>(this, SOURCE_WORD, 0x55667788UL)));
    EXPECT_TRUE((TestLoadExclusive<uint64_t, false>(this, SOURCE_WORD, SOURCE_WORD)));

    EXPECT_TRUE((TestLoadExclusive<uint8_t, true>(this, SOURCE_WORD, 0x88U)));
    EXPECT_TRUE((TestLoadExclusive<uint16_t, true>(this, SOURCE_WORD, 0x7788U)));
    EXPECT_TRUE((TestLoadExclusive<uint32_t, true>(this, SOURCE_WORD, 0x55667788UL)));
    EXPECT_TRUE((TestLoadExclusive<uint64_t, true>(this, SOURCE_WORD, SOURCE_WORD)));
}

TEST_F(Encoder64Test, StoreExclusiveTest)
{
    EXPECT_TRUE((TestStoreExclusiveFailed<uint8_t, false>(this)));
    EXPECT_TRUE((TestStoreExclusiveFailed<uint16_t, false>(this)));
    EXPECT_TRUE((TestStoreExclusiveFailed<uint32_t, false>(this)));
    EXPECT_TRUE((TestStoreExclusiveFailed<uint64_t, false>(this)));

    EXPECT_TRUE((TestStoreExclusiveFailed<uint8_t, true>(this)));
    EXPECT_TRUE((TestStoreExclusiveFailed<uint16_t, true>(this)));
    EXPECT_TRUE((TestStoreExclusiveFailed<uint32_t, true>(this)));
    EXPECT_TRUE((TestStoreExclusiveFailed<uint64_t, true>(this)));

    EXPECT_TRUE((TestStoreExclusive<uint8_t, false>(this, 0x11U, 0xFFFFFFFFFFFFFF11ULL)));
    EXPECT_TRUE((TestStoreExclusive<uint16_t, false>(this, 0x1122U, 0xFFFFFFFFFFFF1122ULL)));
    EXPECT_TRUE((TestStoreExclusive<uint32_t, false>(this, 0x11223344UL, 0xFFFFFFFF11223344ULL)));
    EXPECT_TRUE((TestStoreExclusive<uint64_t, false>(this, 0xAABBCCDD11335577ULL, 0xAABBCCDD11335577ULL)));
}

class Encoder64ApiTest : public Encoder64Test {
public:
    Encoder64ApiTest() : decoder_(GetAllocator()) {}

    std::string &GetOutput(const aarch64::Aarch64Encoder &encoder)
    {
        output_.resize(0);
        char buf[64];
        vixl::aarch64::Disassembler disasm(buf, sizeof(buf));
        vixl::aarch64::Decoder::ScopedVisitors sv(decoder_, {&disasm});
        auto begin = encoder.GetMasm()->GetBuffer()->GetOffsetAddress<uint8_t *>(0);
        auto end = begin + encoder.GetMasm()->GetSizeOfCodeGenerated();
        for (auto instr = reinterpret_cast<const vixl::aarch64::Instruction *>(begin);
             instr < reinterpret_cast<const vixl::aarch64::Instruction *>(end); instr = instr->GetNextInstruction()) {
            decoder_.Decode(instr);
            if (!output_.empty()) {
                output_ += '\n';
            }
            output_ += disasm.GetOutput();
        }
        return output_;
    }

protected:
    std::string output_;
    vixl::aarch64::Decoder decoder_;
};

TEST_F(Encoder64ApiTest, TestEncodeMov)
{
    {
        aarch64::Aarch64Encoder encoder(GetAllocator());
        encoder.InitMasm();
        encoder.SetRegfile(GetRegfile());
        encoder.EncodeMov(Reg(0, INT32_TYPE), Reg(1, INT32_TYPE));
        encoder.Finalize();
        EXPECT_TRUE(encoder.GetResult());
        EXPECT_STREQ("mov w0, w1", GetOutput(encoder).c_str());
    }
    {
        aarch64::Aarch64Encoder encoder(GetAllocator());
        encoder.InitMasm();
        encoder.SetRegfile(GetRegfile());
        encoder.EncodeMov(Reg(0, INT32_TYPE), Reg(1, INT64_TYPE));
        encoder.Finalize();
        EXPECT_TRUE(encoder.GetResult());
        EXPECT_STREQ("mov w0, w1", GetOutput(encoder).c_str());
    }
    {
        aarch64::Aarch64Encoder encoder(GetAllocator());
        encoder.InitMasm();
        encoder.SetRegfile(GetRegfile());
        encoder.EncodeMov(Reg(0, INT64_TYPE), Reg(1, INT32_TYPE));
        encoder.Finalize();
        EXPECT_TRUE(encoder.GetResult());
        EXPECT_STREQ("mov x0, x1", GetOutput(encoder).c_str());
    }
    {
        aarch64::Aarch64Encoder encoder(GetAllocator());
        encoder.InitMasm();
        encoder.SetRegfile(GetRegfile());
        encoder.EncodeMov(Reg(0, INT32_TYPE), Reg(0, INT32_TYPE));
        encoder.Finalize();
        EXPECT_TRUE(encoder.GetResult());
        EXPECT_STREQ("", GetOutput(encoder).c_str());
    }
    {
        aarch64::Aarch64Encoder encoder(GetAllocator());
        encoder.InitMasm();
        encoder.SetRegfile(GetRegfile());
        encoder.EncodeMov(Reg(0, FLOAT64_TYPE), Reg(1, FLOAT64_TYPE));
        encoder.Finalize();
        EXPECT_TRUE(encoder.GetResult());
        EXPECT_STREQ("fmov d0, d1", GetOutput(encoder).c_str());
    }
    {
        aarch64::Aarch64Encoder encoder(GetAllocator());
        encoder.InitMasm();
        encoder.SetRegfile(GetRegfile());
        encoder.EncodeMov(Reg(0, FLOAT64_TYPE), Reg(1, FLOAT32_TYPE));
        encoder.Finalize();
        EXPECT_TRUE(encoder.GetResult());
        EXPECT_STREQ("fcvt d0, s1", GetOutput(encoder).c_str());
    }
    {
        aarch64::Aarch64Encoder encoder(GetAllocator());
        encoder.InitMasm();
        encoder.SetRegfile(GetRegfile());
        encoder.EncodeMov(Reg(0, INT32_TYPE), Reg(1, FLOAT32_TYPE));
        encoder.Finalize();
        EXPECT_TRUE(encoder.GetResult());
        EXPECT_STREQ("fmov w0, s1", GetOutput(encoder).c_str());
    }
    {
        aarch64::Aarch64Encoder encoder(GetAllocator());
        encoder.InitMasm();
        encoder.SetRegfile(GetRegfile());
        encoder.EncodeMov(Reg(0, FLOAT32_TYPE), Reg(1, INT32_TYPE));
        encoder.Finalize();
        EXPECT_TRUE(encoder.GetResult());
        EXPECT_STREQ("fmov s0, w1", GetOutput(encoder).c_str());
    }
}

TEST_F(Encoder64ApiTest, TestEncodeLdr)
{
    {
        aarch64::Aarch64Encoder encoder(GetAllocator());
        encoder.InitMasm();
        encoder.SetRegfile(GetRegfile());
        encoder.EncodeLdr(Reg(0, FLOAT64_TYPE), true, MemRef(Reg(0, INT64_TYPE)));
        encoder.Finalize();
        EXPECT_TRUE(encoder.GetResult());
        EXPECT_STREQ("ldr d0, [x0]", GetOutput(encoder).c_str());
    }
    {
        aarch64::Aarch64Encoder encoder(GetAllocator());
        encoder.InitMasm();
        encoder.SetRegfile(GetRegfile());
        encoder.EncodeLdr(Reg(0, INT8_TYPE), true, MemRef(Reg(0, INT64_TYPE)));
        encoder.Finalize();
        EXPECT_TRUE(encoder.GetResult());
        EXPECT_STREQ("ldrsb x0, [x0]", GetOutput(encoder).c_str());
    }
    {
        aarch64::Aarch64Encoder encoder(GetAllocator());
        encoder.InitMasm();
        encoder.SetRegfile(GetRegfile());
        encoder.EncodeLdr(Reg(0, INT16_TYPE), true, MemRef(Reg(0, INT64_TYPE)));
        encoder.Finalize();
        EXPECT_TRUE(encoder.GetResult());
        EXPECT_STREQ("ldrsh w0, [x0]", GetOutput(encoder).c_str());
    }
    {
        aarch64::Aarch64Encoder encoder(GetAllocator());
        encoder.InitMasm();
        encoder.SetRegfile(GetRegfile());
        encoder.EncodeLdr(Reg(0, INT32_TYPE), true, MemRef(Reg(0, INT64_TYPE)));
        encoder.Finalize();
        EXPECT_TRUE(encoder.GetResult());
        EXPECT_STREQ("ldr w0, [x0]", GetOutput(encoder).c_str());
    }
    {
        aarch64::Aarch64Encoder encoder(GetAllocator());
        encoder.InitMasm();
        encoder.SetRegfile(GetRegfile());
        encoder.EncodeLdr(Reg(0, INT8_TYPE), false, MemRef(Reg(0, INT64_TYPE)));
        encoder.Finalize();
        EXPECT_TRUE(encoder.GetResult());
        EXPECT_STREQ("ldrb w0, [x0]", GetOutput(encoder).c_str());
    }
    {
        aarch64::Aarch64Encoder encoder(GetAllocator());
        encoder.InitMasm();
        encoder.SetRegfile(GetRegfile());
        encoder.EncodeLdr(Reg(0, INT16_TYPE), false, MemRef(Reg(0, INT64_TYPE)));
        encoder.Finalize();
        EXPECT_TRUE(encoder.GetResult());
        EXPECT_STREQ("ldrh w0, [x0]", GetOutput(encoder).c_str());
    }
    {
        aarch64::Aarch64Encoder encoder(GetAllocator());
        encoder.InitMasm();
        encoder.SetRegfile(GetRegfile());
        encoder.EncodeLdr(Reg(0, INT32_TYPE), false, MemRef(Reg(0, INT64_TYPE)));
        encoder.Finalize();
        EXPECT_TRUE(encoder.GetResult());
        EXPECT_STREQ("ldr w0, [x0]", GetOutput(encoder).c_str());
    }
    {
        aarch64::Aarch64Encoder encoder(GetAllocator());
        encoder.InitMasm();
        encoder.SetRegfile(GetRegfile());
        encoder.EncodeLdr(Reg(0, INT64_TYPE), false, MemRef(Reg(0, INT64_TYPE)));
        encoder.Finalize();
        EXPECT_TRUE(encoder.GetResult());
        EXPECT_STREQ("ldr x0, [x0]", GetOutput(encoder).c_str());
    }
}

TEST_F(Encoder64ApiTest, TestEncodeStr)
{
    {
        aarch64::Aarch64Encoder encoder(GetAllocator());
        encoder.InitMasm();
        encoder.SetRegfile(GetRegfile());
        encoder.EncodeStr(Reg(0, FLOAT64_TYPE), MemRef(Reg(0, INT64_TYPE)));
        encoder.Finalize();
        EXPECT_TRUE(encoder.GetResult());
        EXPECT_STREQ("str d0, [x0]", GetOutput(encoder).c_str());
    }
    {
        aarch64::Aarch64Encoder encoder(GetAllocator());
        encoder.InitMasm();
        encoder.SetRegfile(GetRegfile());
        encoder.EncodeStr(Reg(0, INT8_TYPE), MemRef(Reg(0, INT64_TYPE)));
        encoder.Finalize();
        EXPECT_TRUE(encoder.GetResult());
        EXPECT_STREQ("strb w0, [x0]", GetOutput(encoder).c_str());
    }
    {
        aarch64::Aarch64Encoder encoder(GetAllocator());
        encoder.InitMasm();
        encoder.SetRegfile(GetRegfile());
        encoder.EncodeStr(Reg(0, INT16_TYPE), MemRef(Reg(0, INT64_TYPE)));
        encoder.Finalize();
        EXPECT_TRUE(encoder.GetResult());
        EXPECT_STREQ("strh w0, [x0]", GetOutput(encoder).c_str());
    }
    {
        aarch64::Aarch64Encoder encoder(GetAllocator());
        encoder.InitMasm();
        encoder.SetRegfile(GetRegfile());
        encoder.EncodeStr(Reg(0, INT32_TYPE), MemRef(Reg(0, INT64_TYPE)));
        encoder.Finalize();
        EXPECT_TRUE(encoder.GetResult());
        EXPECT_STREQ("str w0, [x0]", GetOutput(encoder).c_str());
    }
    {
        aarch64::Aarch64Encoder encoder(GetAllocator());
        encoder.InitMasm();
        encoder.SetRegfile(GetRegfile());
        encoder.EncodeStr(Reg(0, INT64_TYPE), MemRef(Reg(0, INT64_TYPE)));
        encoder.Finalize();
        EXPECT_TRUE(encoder.GetResult());
        EXPECT_STREQ("str x0, [x0]", GetOutput(encoder).c_str());
    }
}

TEST_F(Encoder64ApiTest, TestEncodeMemCopy)
{
    {
        aarch64::Aarch64Encoder encoder(GetAllocator());
        encoder.InitMasm();
        encoder.SetRegfile(GetRegfile());
        encoder.EncodeMemCopy(MemRef(Reg(0, INT64_TYPE)), MemRef(Reg(1, INT64_TYPE)), BYTE_SIZE);
        encoder.Finalize();
        EXPECT_TRUE(encoder.GetResult());
        EXPECT_STREQ("ldrb w16, [x0]\nstrb w16, [x1]", GetOutput(encoder).c_str());
    }
    {
        aarch64::Aarch64Encoder encoder(GetAllocator());
        encoder.InitMasm();
        encoder.SetRegfile(GetRegfile());
        encoder.EncodeMemCopy(MemRef(Reg(0, INT64_TYPE)), MemRef(Reg(1, INT64_TYPE)), HALF_SIZE);
        encoder.Finalize();
        EXPECT_TRUE(encoder.GetResult());
        EXPECT_STREQ("ldrh w16, [x0]\nstrh w16, [x1]", GetOutput(encoder).c_str());
    }
    {
        aarch64::Aarch64Encoder encoder(GetAllocator());
        encoder.InitMasm();
        encoder.SetRegfile(GetRegfile());
        encoder.EncodeMemCopy(MemRef(Reg(0, INT64_TYPE)), MemRef(Reg(1, INT64_TYPE)), WORD_SIZE);
        encoder.Finalize();
        EXPECT_TRUE(encoder.GetResult());
        EXPECT_STREQ("ldr w16, [x0]\nstr w16, [x1]", GetOutput(encoder).c_str());
    }
    {
        aarch64::Aarch64Encoder encoder(GetAllocator());
        encoder.InitMasm();
        encoder.SetRegfile(GetRegfile());
        encoder.EncodeMemCopy(MemRef(Reg(0, INT64_TYPE)), MemRef(Reg(1, INT64_TYPE)), DOUBLE_WORD_SIZE);
        encoder.Finalize();
        EXPECT_TRUE(encoder.GetResult());
        EXPECT_STREQ("ldr x16, [x0]\nstr x16, [x1]", GetOutput(encoder).c_str());
    }
}

TEST_F(Encoder64Test, Registers)
{
    auto target = GetEncoder()->GetTarget();
    {
        {
            ScopedTmpReg tmp1(GetEncoder(), true);
            ASSERT_EQ(tmp1.GetReg(), target.GetLinkReg());
            ScopedTmpReg tmp2(GetEncoder(), true);
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
}  // namespace panda::compiler
