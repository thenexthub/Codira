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
#include "target/aarch32/target.h"
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

// A32-encoding of immediates allow to encode any value in range [0, 255],
// other values should be encoded as masks.
static const uint64_t MAX_IMM_VALUE = 0x0FF;

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

namespace panda::compiler {
class Encoder32Test : public ::testing::Test {
public:
    Encoder32Test()
    {
        panda::mem::MemConfig::Initialize(64_MB, 64_MB, 64_MB, 32_MB);
        PoolManager::Initialize();
        allocator_ = new ArenaAllocator(SpaceType::SPACE_TYPE_COMPILER);
        encoder_ = Encoder::Create(allocator_, Arch::AARCH32, false);
        encoder_->InitMasm();
        regfile_ = RegistersDescription::Create(allocator_, Arch::AARCH32);
        callconv_ = CallingConvention::Create(allocator_, encoder_, regfile_, Arch::AARCH32);
        mem_stats_ = new BaseMemStats();
        code_alloc_ = new (std::nothrow) CodeAllocator(mem_stats_);
    }
    ~Encoder32Test()
    {
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
        if (id == 0) {
            auto default_param = Target::Current().GetParamReg(0);
            return Reg(default_param.GetId(), type);
        }
        // Not supported id > 1
        ASSERT(id == 1);
        if (type == FLOAT64_TYPE) {
            auto default_param = Target::Current().GetParamReg(2);
            return Reg(default_param.GetId(), type);
        }

        if (type.GetSize() == DOUBLE_WORD_SIZE) {
            auto default_param = Target::Current().GetParamReg(2);
            return Reg(default_param.GetId(), type);
        }
        auto default_param = Target::Current().GetParamReg(1);
        return Reg(default_param.GetId(), type);
    }

    template <typename T>
    void PreWork(ArenaVector<Reg> *saved_regs = nullptr)
    {
        // Curor need to encode multiply tests due one execution
        curr_cursor_ = 0;
        encoder_->SetCursorOffset(0);

        ArenaVector<Reg> used_regs(GetAllocator()->Adapter());
        if (saved_regs != nullptr) {
            used_regs.insert(used_regs.end(), saved_regs->begin(), saved_regs->end());
        }
        for (auto reg_code : aarch32::AARCH32_TMP_REG) {
            used_regs.emplace_back(Reg(reg_code, INT32_TYPE));
        }
        GetRegfile()->SetUsedRegs(used_regs);
        callconv_->GeneratePrologue(FrameInfo::FullPrologue());

        // Move real parameters to getter parameters
#if (PANDA_TARGET_ARM32_ABI_HARD)
        if constexpr (std::is_same<float, T>::value) {
            auto param2 = GetParameter(TypeInfo(T(0)), 1);
            auto s1_register = vixl::aarch32::s1;
            static_cast<aarch32::Aarch32Encoder *>(GetEncoder())
                ->GetMasm()
                ->Vmov(aarch32::VixlVReg(param2).S(), s1_register);
        }
#else
        if constexpr (std::is_same<float, T>::value) {
            auto param1 = GetParameter(TypeInfo(T(0)), 0);
            auto param2 = GetParameter(TypeInfo(T(0)), 1);

            auto stored_value_1 = GetParameter(TypeInfo(uint32_t(0)), 0);
            auto stored_value_2 = GetParameter(TypeInfo(uint32_t(0)), 1);

            GetEncoder()->EncodeMov(param1, stored_value_1);
            GetEncoder()->EncodeMov(param2, stored_value_2);
        }
        if constexpr (std::is_same<double, T>::value) {
            auto param1 = GetParameter(TypeInfo(T(0)), 0);
            auto param2 = GetParameter(TypeInfo(T(0)), 1);

            auto stored_value_1 = GetParameter(TypeInfo(uint64_t(0)), 0);
            auto stored_value_2 = GetParameter(TypeInfo(uint64_t(0)), 1);
            GetEncoder()->EncodeMov(param1, stored_value_1);
            GetEncoder()->EncodeMov(param2, stored_value_2);
        }
#endif
    }

    template <typename T>
    void PostWork()
    {
        // Move getted parameters to real return value
#if (PANDA_TARGET_ARM32_ABI_HARD)
        // Default parameter 0 == return value
#else
        if constexpr (std::is_same<float, T>::value) {
            auto param = GetParameter(TypeInfo(T(0)), 0);
            auto return_reg = GetParameter(TypeInfo(uint32_t(0)), 0);
            GetEncoder()->EncodeMov(return_reg, param);
        }
        if constexpr (std::is_same<double, T>::value) {
            auto param = GetParameter(TypeInfo(T(0)), 0);
            auto return_reg = GetParameter(TypeInfo(uint64_t(0)), 0);
            GetEncoder()->EncodeMov(return_reg, param);
        }
#endif
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
        return CallCodeVariadic<U, T>(result, param);
    }

    template <typename T, typename U>
    bool CallCode(const T &param1, const T &param2, const U &result)
    {
        return CallCodeVariadic<U, T, T>(result, param1, param2);
    }

    template <typename T>
    bool CallCode(const T &param, const T &result)
    {
        return CallCodeVariadic<T, T>(result, param);
    }

    // Using max size type: type result "T" or 32bit to check result, because in our ISA min type is 32bit.
    template <typename T, typename U = std::conditional_t<
                              (std::is_integral_v<T> && (sizeof(T) * BYTE_SIZE < WORD_SIZE)), uint32_t, T>>
    U WidenIntegralArgToUint32(T arg)
    {
        return static_cast<U>(arg);
    }

    template <typename U, typename... Args>
    bool CallCodeVariadic(const U result, Args... args)
    {
        return CallCodeVariadicImpl<>(WidenIntegralArgToUint32<>(result), WidenIntegralArgToUint32<>(args)...);
    }

    template <size_t param_idx, bool is_binary>
    void PrintParams()
    {
    }

    template <size_t param_idx, bool is_binary, typename Arg, typename... Args>
    void PrintParams(Arg arg, Args... args)
    {
        std::cerr << " param" << param_idx << "=";
        if constexpr (is_binary && std::is_same<double, Arg>::value) {
            std::cerr << bit_cast<uint64_t>(arg);
        } else if constexpr (is_binary && std::is_same<float, Arg>::value) {
            std::cerr << bit_cast<uint32_t>(arg);
        } else {
            std::cerr << arg;
        }
        PrintParams<param_idx + 1, is_binary>(args...);
    }

    template <typename U, typename... Args>
    bool CallCodeVariadicImpl(const U result, Args... args)
    {
        using funct_ptr = U (*)(Args...);
        auto size = callconv_->GetCodeSize() - curr_cursor_;
        void *offset = (static_cast<uint8_t *>(callconv_->GetCodeEntry()));
        void *ptr = code_alloc_->AllocateCode(size, offset);
        auto func = reinterpret_cast<funct_ptr>(ptr);
        const U curr_result = func(args...);
        ResetCodeAllocator(ptr, size);
        bool ret = false;
        if constexpr (std::is_floating_point_v<U>) {
            ret = (curr_result == result) || (std::isnan(curr_result) && std::isnan(result));
        } else {
            ret = (curr_result - result == 0);
        }

        if (!ret) {
            std::cerr << std::hex << "Failed CallCode for";
            PrintParams<1, false>(args...);
            std::cerr << " and result=" << result << " current_result=" << curr_result << "\n";
            if constexpr (std::is_floating_point_v<U> || (std::is_floating_point_v<Args> || ...)) {
                std::cerr << "In binary :";
                PrintParams<1, true>(args...);
                if constexpr (std::is_same<double, U>::value) {
                    std::cerr << " reslt=" << bit_cast<uint64_t>(result);
                    std::cerr << " current_reslt=" << bit_cast<uint64_t>(curr_result);
                } else if constexpr (std::is_same<float, U>::value) {
                    std::cerr << " result=" << bit_cast<uint32_t>(result);
                    std::cerr << " current_result=" << bit_cast<uint32_t>(curr_result);
                } else {
                    std::cerr << " result=" << result;
                    std::cerr << " current_result=" << curr_result;
                }
                std::cerr << "\n";
            }
#if !defined(NDEBUG)
            Dump(true);
#endif
        }
        return ret;
    }

    template <typename T>
    bool CallCode(const T &param1, const T &param2, const T &result)
    {
        return CallCodeVariadic<T, T, T>(result, param1, param2);
    }

    template <typename T>
    T CallCodeStore(uint32_t address, T param)
    {
        using funct_ptr = T (*)(uint32_t param1, T param2);
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

/*
    Tests work with one default parameter:
        r0 - for 32-bit parameter
        r0 + r1 - for 64-bit parameter

        return value - r0 for 32-bit parameter
                       r0 & r1 - for 64-bit parameter
*/

template <typename T>
bool TestNeg(Encoder32Test *test)
{
    // Initialize
    test->PreWork<T>();
    // First type-dependency
    auto param = test->GetParameter(TypeInfo(T(0)));
    // Main test call
    test->GetEncoder()->EncodeNeg(param, param);
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

TEST_F(Encoder32Test, NegTest)
{
    EXPECT_TRUE(TestNeg<int8_t>(this));
    EXPECT_TRUE(TestNeg<int16_t>(this));
    EXPECT_TRUE(TestNeg<int32_t>(this));
    EXPECT_TRUE(TestNeg<int64_t>(this));
    EXPECT_TRUE(TestNeg<float>(this));
    EXPECT_TRUE(TestNeg<double>(this));
}

template <typename T>
bool TestNot(Encoder32Test *test)
{
    // Initialize
    test->PreWork<T>();
    // First type-dependency
    auto param = test->GetParameter(TypeInfo(T(0)));
    // Main test call
    test->GetEncoder()->EncodeNot(param, param);
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

TEST_F(Encoder32Test, NotTest)
{
    EXPECT_TRUE(TestNot<int8_t>(this));
    EXPECT_TRUE(TestNot<int16_t>(this));
    EXPECT_TRUE(TestNot<int32_t>(this));
    EXPECT_TRUE(TestNot<int64_t>(this));

    EXPECT_TRUE(TestNot<uint8_t>(this));
    EXPECT_TRUE(TestNot<uint16_t>(this));
    EXPECT_TRUE(TestNot<uint32_t>(this));
    EXPECT_TRUE(TestNot<uint64_t>(this));
}

template <typename T>
bool TestMov(Encoder32Test *test)
{
    // Initialize
    test->PreWork<T>();
    // First type-dependency
    auto param = test->GetParameter(TypeInfo(T(0)));
    // Main test call
    test->GetEncoder()->EncodeMov(param, param);
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
bool TestMov2(Encoder32Test *test)
{
    static_assert(sizeof(Src) == sizeof(Dst));
    // Initialize
    test->PreWork<Src>();
    // First type-dependency
    auto input = test->GetParameter(TypeInfo(Src(0)), 0);
    auto output = test->GetParameter(TypeInfo(Dst(0)), 0);
    // Main test call
    test->GetEncoder()->EncodeMov(output, input);
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

TEST_F(Encoder32Test, MovTest)
{
    EXPECT_TRUE(TestMov<int8_t>(this));
    EXPECT_TRUE(TestMov<int16_t>(this));
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
TEST_F(Encoder32Test, JumpTest)
{
    // Test for
    // EncodeJump(LabelHolder::LabelId label)
    PreWork<uint32_t>();

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
    PostWork<uint32_t>();

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
bool TestBitTestAndBranch(Encoder32Test *test, T value, int pos, uint32_t expected)
{
    test->PreWork<T>();
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

    test->PostWork<T>();

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
bool TestBitTestAndBranch(Encoder32Test *test)
{
    size_t max_pos = std::is_same<uint64_t, T>::value ? 64 : 32;
    for (size_t i = 0; i < max_pos; i++) {
        T value = static_cast<T>(1) << i;
        if (!TestBitTestAndBranch<T, NotZero>(test, value, i, NotZero ? 0 : 1)) {
            return false;
        }
        if (!TestBitTestAndBranch<T, NotZero>(test, ~value, i, NotZero ? 1 : 0)) {
            return false;
        }
    }
    return true;
}

template <typename T, Condition cc>
bool TestJumpCC(Encoder32Test *test)
{
    bool is_signed = std::is_signed<T>::value;
    // Initialize
    test->PreWork<T>();
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
    // test->GetEncoder()->EncodeReturn(); < encoded in PostWork<T>

    // Finalize
    test->PostWork<T>();

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
TEST_F(Encoder32Test, JumpCCTest)
{
    //  EncodeJump(LabelHolder::LabelId, Reg, Condition)
    EXPECT_TRUE((TestJumpCC<int8_t, Condition::EQ>(this)));
    EXPECT_TRUE((TestJumpCC<int8_t, Condition::NE>(this)));
    EXPECT_TRUE((TestJumpCC<int16_t, Condition::EQ>(this)));
    EXPECT_TRUE((TestJumpCC<int16_t, Condition::NE>(this)));
    EXPECT_TRUE((TestJumpCC<int32_t, Condition::EQ>(this)));
    EXPECT_TRUE((TestJumpCC<int32_t, Condition::NE>(this)));
    EXPECT_TRUE((TestJumpCC<int64_t, Condition::NE>(this)));
    EXPECT_TRUE((TestJumpCC<int64_t, Condition::EQ>(this)));
    EXPECT_TRUE((TestJumpCC<uint8_t, Condition::EQ>(this)));
    EXPECT_TRUE((TestJumpCC<uint8_t, Condition::NE>(this)));
    EXPECT_TRUE((TestJumpCC<uint16_t, Condition::EQ>(this)));
    EXPECT_TRUE((TestJumpCC<uint16_t, Condition::NE>(this)));
    EXPECT_TRUE((TestJumpCC<uint32_t, Condition::EQ>(this)));
    EXPECT_TRUE((TestJumpCC<uint32_t, Condition::NE>(this)));
    EXPECT_TRUE((TestJumpCC<uint64_t, Condition::EQ>(this)));
    EXPECT_TRUE((TestJumpCC<uint64_t, Condition::NE>(this)));
}

/*  From operands.h
// Condition also used for tell comparison registers type
enum Condition {
 +  EQ,  // equal to 0
 +  NE,  // not equal to 0
    // signed
 -  LT,  // less
 -  LE,  // less than or equal
 -  GT,  // greater
 -  GE,  // greater than or equal
    // unsigned - checked from registers
 -  LO,  // less
 -  LS,  // less than or equal
 -  HI,  // greater
 -  HS,  // greater than or equal
    // Special arch-dependecy
 -  MI,  // N set            Negative
 -  PL,  // N clear          Positive or zero
 -  VS,  // V set            Overflow.
 -  VC,  // V clear          No overflow.
 -  AL,  //                  Always.
 -  NV,  // Behaves as always/al.

    INVALID_COND
}
*/

TEST_F(Encoder32Test, BitTestAndBranchZero)
{
    EXPECT_TRUE(TestBitTestAndBranch<uint32_t>(this));
    EXPECT_TRUE(TestBitTestAndBranch<uint64_t>(this));
}

TEST_F(Encoder32Test, BitTestAndBranchNotZero)
{
    constexpr bool notZero = true;
    EXPECT_TRUE((TestBitTestAndBranch<uint32_t, notZero>(this)));
    EXPECT_TRUE((TestBitTestAndBranch<uint64_t, notZero>(this)));
}

template <typename T>
bool TestLdr(Encoder32Test *test)
{
    bool is_signed = std::is_signed<T>::value;
    // Initialize
    test->PreWork<T>();
    // First type-dependency
    // Some strange code - default parameter is addres (32 bit)

    auto param = Target::Current().GetReturnReg();
    // But return value is cutted by loaded value
    auto ret_val = test->GetParameter(TypeInfo(T(0)));

    auto mem = MemRef(param);
    test->GetEncoder()->EncodeLdr(ret_val, is_signed, mem);

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
        T tmp = random_gen<T>();
        T *ptr = &tmp;
        // Test : param - Pointer to value
        //        return - value (loaded by ptr)
        // Value is resulting type, but call is ptr_type
        if (!test->CallCode<int32_t, T>(reinterpret_cast<int32_t>(ptr), tmp)) {
            std::cerr << "Ldr test fail " << tmp << " \n";
            return false;
        }
    }
    return true;
}

// Load test
TEST_F(Encoder32Test, LoadTest)
{
    //  EncodeLdr(Reg dst, bool dst_signed, MemRef mem)
    EXPECT_TRUE((TestLdr<int8_t>(this)));
    EXPECT_TRUE((TestLdr<int16_t>(this)));
    EXPECT_TRUE((TestLdr<int32_t>(this)));
    EXPECT_TRUE((TestLdr<int64_t>(this)));
    EXPECT_TRUE((TestLdr<uint8_t>(this)));
    EXPECT_TRUE((TestLdr<uint16_t>(this)));
    EXPECT_TRUE((TestLdr<uint32_t>(this)));
    EXPECT_TRUE((TestLdr<uint64_t>(this)));
    EXPECT_TRUE((TestLdr<float>(this)));
    EXPECT_TRUE((TestLdr<double>(this)));

    // TODO(igorban) : additional test for full memory model:
    //                 + mem(base + index<<scale + disp)
}

template <typename T>
bool TestStr(Encoder32Test *test)
{
    // Initialize
    test->PreWork<T>();
    // First type-dependency
    // Some strange code - default parameter is addres (32 bit)
    auto param = Target::Current().GetParamReg(0);
    // Data to be stored:
    auto stored_value = test->GetParameter(TypeInfo(T(0)), 1);

#if (PANDA_TARGET_ARM32_ABI_HARD)
    if constexpr ((std::is_same<float, T>::value) || std::is_same<double, T>::value) {
        stored_value = test->GetParameter(TypeInfo(T(0)), 0);
    }
#else
    if constexpr (std::is_same<float, T>::value) {
        stored_value = test->GetParameter(TypeInfo(uint32_t(0)), 1);
        ScopedTmpRegF32 tmp_float(test->GetEncoder());
        test->GetEncoder()->EncodeMov(tmp_float, stored_value);
        stored_value = tmp_float;
    }
    if constexpr (std::is_same<double, T>::value) {
        stored_value = test->GetParameter(TypeInfo(uint64_t(0)), 1);
        ScopedTmpRegF64 tmp_double(test->GetEncoder());
        test->GetEncoder()->EncodeMov(tmp_double, stored_value);
        stored_value = tmp_double;
    }
#endif

    auto mem = MemRef(param);
    test->GetEncoder()->EncodeStr(stored_value, mem);
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
        T tmp = random_gen<T>();
        T ret_data = 0;
        T *ptr = &ret_data;

        // Test : param - Pointer to value
        //        return - value (loaded by ptr)
        // Value is resulting type, but call is ptr_type
        auto result = test->CallCodeStore<T>(reinterpret_cast<int32_t>(ptr), tmp);
        // Store must change ret_data value
        if (!(ret_data == tmp || (std::is_floating_point_v<T> && std::isnan(ret_data) && std::isnan(tmp)))) {
            std::cerr << std::hex << "Ldr test fail " << tmp << " ret_data = " << ret_data << "\n";
            return false;
        }
    }
    return true;
}

// Simple store test
TEST_F(Encoder32Test, StrTest)
{
    //  EncodeStr(Reg src, MemRef mem)
    EXPECT_TRUE((TestStr<int8_t>(this)));
    EXPECT_TRUE((TestStr<int16_t>(this)));
    EXPECT_TRUE((TestStr<int32_t>(this)));
    EXPECT_TRUE((TestStr<int64_t>(this)));
    EXPECT_TRUE((TestStr<uint8_t>(this)));
    EXPECT_TRUE((TestStr<uint16_t>(this)));
    EXPECT_TRUE((TestStr<uint32_t>(this)));
    EXPECT_TRUE((TestStr<uint64_t>(this)));
    EXPECT_TRUE((TestStr<float>(this)));
    EXPECT_TRUE((TestStr<double>(this)));

    // TODO(igorban) : additional test for full memory model:
    //                 + mem(base + index<<scale + disp)
}

// Store immediate test
// TEST_F(Encoder32Test, StiTest)
//  EncodeSti(Imm src, MemRef mem)

// Store zero upper test
// TEST_F(Encoder32Test, StrzTest)
//  EncodeStrz(Reg src, MemRef mem)

// Return test ???? What here may be tested
// TEST_F(Encoder32Test, ReturnTest)
//  EncodeReturn()

bool foo(uint32_t param1, uint32_t param2)
{
    // TODO(igorban): use variables
    return (param1 == param2);
}

using funct_ptr = bool (*)(uint32_t param1, uint32_t param2);

funct_ptr foo_ptr = &foo;

// Call Test
TEST_F(Encoder32Test, CallTest)
{
    // Initialize
    auto link_reg = Target::Current().GetLinkReg();
    ArenaVector<Reg> used_regs(GetAllocator()->Adapter());
    used_regs.emplace_back(link_reg);

    PreWork<uint32_t>(&used_regs);

    // Call foo
    GetEncoder()->MakeCall(reinterpret_cast<void *>(foo_ptr));
    // return value - moved to return value
    PostWork<uint32_t>();

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
bool TestAbs(Encoder32Test *test)
{
    // Initialize
    test->PreWork<T>();

    // First type-dependency
    auto param = test->GetParameter(TypeInfo(T(0)));

    // Main test call
    test->GetEncoder()->EncodeAbs(param, param);

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

TEST_F(Encoder32Test, AbsTest)
{
    EXPECT_TRUE(TestAbs<int8_t>(this));
    EXPECT_TRUE(TestAbs<int16_t>(this));
    EXPECT_TRUE(TestAbs<int32_t>(this));
    // TODO (asidorov, igorban) the test failed in release mode
    // TestAbs<int64_t>
    EXPECT_TRUE(TestAbs<float>(this));
    EXPECT_TRUE(TestAbs<double>(this));
}

template <typename T>
bool TestSqrt(Encoder32Test *test)
{
    // Initialize
    test->PreWork<T>();

    // First type-dependency
    auto param = test->GetParameter(TypeInfo(T(0)));

    // Main test call
    test->GetEncoder()->EncodeSqrt(param, param);

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

TEST_F(Encoder32Test, SqrtTest)
{
    EXPECT_TRUE(TestSqrt<float>(this));
    EXPECT_TRUE(TestSqrt<double>(this));
}

template <typename T>
bool TestAdd(Encoder32Test *test)
{
    // Initialize
    test->PreWork<T>();

    // First type-dependency
    auto param1 = test->GetParameter(TypeInfo(T(0)), 0);
    auto param2 = test->GetParameter(TypeInfo(T(0)), 1);

    // Main test call
    test->GetEncoder()->EncodeAdd(param1, param1, param2);

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
        T tmp1 = random_gen<T>();
        T tmp2 = random_gen<T>();
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

TEST_F(Encoder32Test, AddTest)
{
    EXPECT_TRUE(TestAdd<int8_t>(this));
    EXPECT_TRUE(TestAdd<int16_t>(this));
    EXPECT_TRUE(TestAdd<int32_t>(this));
    EXPECT_TRUE(TestAdd<int64_t>(this));
    EXPECT_TRUE(TestAdd<float>(this));
    EXPECT_TRUE(TestAdd<double>(this));
}

template <typename T>
bool TestAddImm(Encoder32Test *test)
{
    // Initialize
    test->PreWork<T>();

    // First type-dependency
    auto param1 = test->GetParameter(TypeInfo(T(0)), 0);
    T param2 = random_gen<T>();

    // Main test call
    test->GetEncoder()->EncodeAdd(param1, param1, Imm(param2));

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

TEST_F(Encoder32Test, AddImmTest)
{
    EXPECT_TRUE(TestAddImm<int8_t>(this));
    EXPECT_TRUE(TestAddImm<int16_t>(this));
    EXPECT_TRUE(TestAddImm<int32_t>(this));
    EXPECT_TRUE(TestAddImm<int64_t>(this));
}

template <typename T>
bool TestSub(Encoder32Test *test)
{
    // Initialize
    test->PreWork<T>();

    // First type-dependency
    auto param1 = test->GetParameter(TypeInfo(T(0)), 0);
    auto param2 = test->GetParameter(TypeInfo(T(0)), 1);

    // Main test call
    test->GetEncoder()->EncodeSub(param1, param1, param2);

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

/*
        Sub 2 denormolized values - c++ return wrong result:
        // param1=1.66607e-39 param2=9.56199e-39 and result=-4.0369e-39 curr_result=-7.89592e-39
        // In binary : param1=1188945 param2=6823664 result=2150364478 curr_result=2153118367
*/

TEST_F(Encoder32Test, SubTest)
{
    EXPECT_TRUE(TestSub<int8_t>(this));
    EXPECT_TRUE(TestSub<int16_t>(this));
    EXPECT_TRUE(TestSub<int32_t>(this));
    EXPECT_TRUE(TestSub<int64_t>(this));
    EXPECT_TRUE(TestSub<float>(this));
    EXPECT_TRUE(TestSub<double>(this));
}

template <typename T>
bool TestSubImm(Encoder32Test *test)
{
    // Initialize
    test->PreWork<T>();

    // First type-dependency
    auto param1 = test->GetParameter(TypeInfo(T(0)), 0);
    T param2 = random_gen<T>();

    // Main test call
    test->GetEncoder()->EncodeSub(param1, param1, Imm(param2));

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

TEST_F(Encoder32Test, SubImmTest)
{
    EXPECT_TRUE(TestSubImm<int8_t>(this));
    EXPECT_TRUE(TestSubImm<int16_t>(this));
    // TODO (asidorov, igorban) the test failed in release mode
    // TestSubImm<int32_t>
    EXPECT_TRUE(TestSubImm<int64_t>(this));
    // TestSubImm<float>
    // TestSubImm<double>
}

template <typename T>
bool TestMul(Encoder32Test *test)
{
    // Initialize
    test->PreWork<T>();

    // First type-dependency
    auto param1 = test->GetParameter(TypeInfo(T(0)), 0);
    auto param2 = test->GetParameter(TypeInfo(T(0)), 1);

    // Main test call
    test->GetEncoder()->EncodeMul(param1, param1, param2);

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

TEST_F(Encoder32Test, MulTest)
{
    EXPECT_TRUE(TestMul<int8_t>(this));
    EXPECT_TRUE(TestMul<int16_t>(this));
    EXPECT_TRUE(TestMul<int32_t>(this));
    EXPECT_TRUE(TestMul<int64_t>(this));
    EXPECT_TRUE(TestMul<float>(this));
    EXPECT_TRUE(TestMul<double>(this));
}

template <typename T>
bool TestMin(Encoder32Test *test)
{
    // Initialize
    test->PreWork<T>();

    // First type-dependency
    auto param1 = test->GetParameter(TypeInfo(T(0)), 0);
    auto param2 = test->GetParameter(TypeInfo(T(0)), 1);

    // Main test call
    test->GetEncoder()->EncodeMin(param1, std::is_signed_v<T>, param1, param2);

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

TEST_F(Encoder32Test, MinTest)
{
    EXPECT_TRUE(TestMin<int8_t>(this));
    EXPECT_TRUE(TestMin<int16_t>(this));
    EXPECT_TRUE(TestMin<int32_t>(this));
    EXPECT_TRUE(TestMin<int64_t>(this));
    EXPECT_TRUE(TestMin<uint8_t>(this));
    EXPECT_TRUE(TestMin<uint16_t>(this));
    EXPECT_TRUE(TestMin<uint32_t>(this));
    EXPECT_TRUE(TestMin<uint64_t>(this));
    EXPECT_TRUE(TestMin<float>(this));
    EXPECT_TRUE(TestMin<double>(this));
}

template <typename T>
bool TestMax(Encoder32Test *test)
{
    // Initialize
    test->PreWork<T>();

    // First type-dependency
    auto param1 = test->GetParameter(TypeInfo(T(0)), 0);
    auto param2 = test->GetParameter(TypeInfo(T(0)), 1);

    // Main test call
    test->GetEncoder()->EncodeMax(param1, std::is_signed_v<T>, param1, param2);

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

TEST_F(Encoder32Test, MaxTest)
{
    EXPECT_TRUE(TestMax<int8_t>(this));
    EXPECT_TRUE(TestMax<int16_t>(this));
    EXPECT_TRUE(TestMax<int32_t>(this));
    EXPECT_TRUE(TestMax<int64_t>(this));
    EXPECT_TRUE(TestMax<uint8_t>(this));
    EXPECT_TRUE(TestMax<uint16_t>(this));
    EXPECT_TRUE(TestMax<uint32_t>(this));
    EXPECT_TRUE(TestMax<uint64_t>(this));
    EXPECT_TRUE(TestMax<float>(this));
    EXPECT_TRUE(TestMax<double>(this));
}

template <typename T>
bool TestShl(Encoder32Test *test)
{
    // Initialize
    test->PreWork<T>();

    // First type-dependency
    auto param1 = test->GetParameter(TypeInfo(T(0)), 0);
    auto param2 = test->GetParameter(TypeInfo(T(0)), 1);

    // Main test call
    test->GetEncoder()->EncodeShl(param1, param1, param2);

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

TEST_F(Encoder32Test, ShlTest)
{
    EXPECT_TRUE(TestShl<int8_t>(this));
    EXPECT_TRUE(TestShl<int16_t>(this));
    EXPECT_TRUE(TestShl<int32_t>(this));
    EXPECT_TRUE(TestShl<int64_t>(this));
}

template <typename T>
bool TestShr(Encoder32Test *test)
{
    // Initialize
    test->PreWork<T>();

    // First type-dependency
    auto param1 = test->GetParameter(TypeInfo(T(0)), 0);
    auto param2 = test->GetParameter(TypeInfo(T(0)), 1);

    // Main test call
    test->GetEncoder()->EncodeShr(param1, param1, param2);

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

TEST_F(Encoder32Test, ShrTest)
{
    EXPECT_TRUE(TestShr<int8_t>(this));
    EXPECT_TRUE(TestShr<int16_t>(this));
    EXPECT_TRUE(TestShr<int32_t>(this));
    EXPECT_TRUE(TestShr<int64_t>(this));
    EXPECT_TRUE(TestShr<uint8_t>(this));
    EXPECT_TRUE(TestShr<uint16_t>(this));
    EXPECT_TRUE(TestShr<uint32_t>(this));
    EXPECT_TRUE(TestShr<uint64_t>(this));
}

template <typename T>
bool TestAShr(Encoder32Test *test)
{
    // Initialize
    test->PreWork<T>();

    // First type-dependency
    auto param1 = test->GetParameter(TypeInfo(T(0)), 0);
    auto param2 = test->GetParameter(TypeInfo(T(0)), 1);

    // Main test call
    test->GetEncoder()->EncodeAShr(param1, param1, param2);

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

TEST_F(Encoder32Test, AShrTest)
{
    EXPECT_TRUE(TestAShr<int8_t>(this));
    EXPECT_TRUE(TestAShr<int16_t>(this));
    EXPECT_TRUE(TestAShr<int32_t>(this));
    EXPECT_TRUE(TestAShr<int64_t>(this));
}

template <typename T>
bool TestAnd(Encoder32Test *test)
{
    // Initialize
    test->PreWork<T>();

    // First type-dependency
    auto param1 = test->GetParameter(TypeInfo(T(0)), 0);
    auto param2 = test->GetParameter(TypeInfo(T(0)), 1);

    // Main test call
    test->GetEncoder()->EncodeAnd(param1, param1, param2);

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

TEST_F(Encoder32Test, AndTest)
{
    EXPECT_TRUE(TestAnd<int8_t>(this));
    EXPECT_TRUE(TestAnd<int16_t>(this));
    EXPECT_TRUE(TestAnd<int32_t>(this));
    EXPECT_TRUE(TestAnd<int64_t>(this));
}

template <typename T>
bool TestOr(Encoder32Test *test)
{
    // Initialize
    test->PreWork<T>();

    // First type-dependency
    auto param1 = test->GetParameter(TypeInfo(T(0)), 0);
    auto param2 = test->GetParameter(TypeInfo(T(0)), 1);

    // Main test call
    test->GetEncoder()->EncodeOr(param1, param1, param2);

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

TEST_F(Encoder32Test, OrTest)
{
    EXPECT_TRUE(TestOr<int8_t>(this));
    EXPECT_TRUE(TestOr<int16_t>(this));
    EXPECT_TRUE(TestOr<int32_t>(this));
    EXPECT_TRUE(TestOr<int64_t>(this));
}

template <typename T>
bool TestXor(Encoder32Test *test)
{
    // Initialize
    test->PreWork<T>();

    // First type-dependency
    auto param1 = test->GetParameter(TypeInfo(T(0)), 0);
    auto param2 = test->GetParameter(TypeInfo(T(0)), 1);

    // Main test call
    test->GetEncoder()->EncodeXor(param1, param1, param2);

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

TEST_F(Encoder32Test, XorTest)
{
    EXPECT_TRUE(TestXor<int8_t>(this));
    EXPECT_TRUE(TestXor<int16_t>(this));
    EXPECT_TRUE(TestXor<int32_t>(this));
    EXPECT_TRUE(TestXor<int64_t>(this));
}

template <typename T>
bool TestShlImm(Encoder32Test *test)
{
    // Initialize
    test->PreWork<T>();

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

TEST_F(Encoder32Test, ShlImmTest)
{
    EXPECT_TRUE(TestShlImm<int8_t>(this));
    EXPECT_TRUE(TestShlImm<int16_t>(this));
    EXPECT_TRUE(TestShlImm<int32_t>(this));
    EXPECT_TRUE(TestShlImm<int64_t>(this));
}

template <typename T>
bool TestShrImm(Encoder32Test *test)
{
    // Initialize
    test->PreWork<T>();

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
        T tmp1 = random_gen<T>();

        // Deduced conflicting types for parameter

        // Main check - compare parameter and
        // return value
        bool result {false};

        if constexpr (sizeof(T) == sizeof(int64_t)) {
            result = test->CallCode<T>(tmp1, (static_cast<uint64_t>(tmp1)) >> param2);
        } else {
            result = test->CallCode<T>(tmp1, (static_cast<uint32_t>(tmp1)) >> (param2 & (CHAR_BIT * sizeof(T) - 1)));
        }

        if (!result) {
            return false;
        }
    }
    return true;
}

TEST_F(Encoder32Test, ShrImmTest)
{
    EXPECT_TRUE(TestShrImm<int8_t>(this));
    EXPECT_TRUE(TestShrImm<int16_t>(this));
    EXPECT_TRUE(TestShrImm<int32_t>(this));
    EXPECT_TRUE(TestShrImm<int64_t>(this));
    EXPECT_TRUE(TestShrImm<uint8_t>(this));
    EXPECT_TRUE(TestShrImm<uint16_t>(this));
    EXPECT_TRUE(TestShrImm<uint32_t>(this));
    EXPECT_TRUE(TestShrImm<uint64_t>(this));
}

template <typename T>
bool TestCmp(Encoder32Test *test)
{
    static_assert(std::is_integral_v<T>);
    // Initialize
    test->PreWork<T>();

    // First type-dependency
    auto output = test->GetParameter(TypeInfo(int32_t(0)), 0);
    auto param1 = test->GetParameter(TypeInfo(T(0)), 0);
    auto param2 = test->GetParameter(TypeInfo(T(0)), 1);

    // Main test call
    test->GetEncoder()->EncodeCmp(output, param1, param2, std::is_signed_v<T> ? Condition::LT : Condition::LO);

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
bool TestFcmp(Encoder32Test *test, bool is_fcmpg)
{
    static_assert(std::is_floating_point_v<T>);
    // Initialize
    test->PreWork<T>();

    // First type-dependency
    auto output = test->GetParameter(TypeInfo(int32_t(0)), 0);
    auto param1 = test->GetParameter(TypeInfo(T(0)), 0);
    auto param2 = test->GetParameter(TypeInfo(T(0)), 1);
    // Main test call
    test->GetEncoder()->EncodeCmp(output, param1, param2, is_fcmpg ? Condition::MI : Condition::LT);

    // Finalize
    test->PostWork<int32_t>();

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

TEST_F(Encoder32Test, CmpTest)
{
    EXPECT_TRUE(TestCmp<int8_t>(this));
    EXPECT_TRUE(TestCmp<int16_t>(this));
    EXPECT_TRUE(TestCmp<int32_t>(this));
    EXPECT_TRUE(TestCmp<int64_t>(this));
    EXPECT_TRUE(TestCmp<uint8_t>(this));
    EXPECT_TRUE(TestCmp<uint16_t>(this));
    EXPECT_TRUE(TestCmp<uint32_t>(this));
    EXPECT_TRUE(TestCmp<uint64_t>(this));
    EXPECT_TRUE(TestFcmp<float>(this, false));
    EXPECT_TRUE(TestFcmp<double>(this, false));
    EXPECT_TRUE(TestFcmp<float>(this, true));
    EXPECT_TRUE(TestFcmp<double>(this, true));
}

template <typename T>
bool TestCmp64(Encoder32Test *test)
{
    static_assert(sizeof(T) == sizeof(int64_t));
    // Initialize
    test->PreWork<T>();

    // First type-dependency
    auto param1 = test->GetParameter(TypeInfo(T(0)), 0);
    auto param2 = test->GetParameter(TypeInfo(T(0)), 1);

    // Main test call
    test->GetEncoder()->EncodeCmp(param1, param1, param2, std::is_signed_v<T> ? Condition::LT : Condition::LO);

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

TEST_F(Encoder32Test, Cmp64Test)
{
    EXPECT_TRUE(TestCmp64<int64_t>(this));
    EXPECT_TRUE(TestCmp64<uint64_t>(this));
}

template <typename T>
bool TestCompare(Encoder32Test *test)
{
    // Initialize
    test->PreWork<T>();

    // First type-dependency
    auto param1 = test->GetParameter(TypeInfo(T(0)), 0);
    auto param2 = test->GetParameter(TypeInfo(T(0)), 1);

    // Main test call
    test->GetEncoder()->EncodeCompare(param1, param1, param2, std::is_signed_v<T> ? Condition::GE : Condition::HS);

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

TEST_F(Encoder32Test, CompareTest)
{
    EXPECT_TRUE(TestCompare<int8_t>(this));
    EXPECT_TRUE(TestCompare<int16_t>(this));
    EXPECT_TRUE(TestCompare<int32_t>(this));
    EXPECT_TRUE(TestCompare<int64_t>(this));
    EXPECT_TRUE(TestCompare<uint8_t>(this));
    EXPECT_TRUE(TestCompare<uint16_t>(this));
    EXPECT_TRUE(TestCompare<uint32_t>(this));
    EXPECT_TRUE(TestCompare<uint64_t>(this));
}

template <typename T>
bool TestCompare64(Encoder32Test *test)
{
    static_assert(sizeof(T) == sizeof(int64_t));
    // Initialize
    test->PreWork<T>();

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

TEST_F(Encoder32Test, Compare64Test)
{
    EXPECT_TRUE(TestCompare64<int64_t>(this));
    EXPECT_TRUE(TestCompare64<uint64_t>(this));
}

template <typename Src, typename Dst>
bool TestCast(Encoder32Test *test)
{
    auto link_reg = Target::Current().GetLinkReg();

    // Initialize
    test->PreWork<Src>();
    static_cast<aarch32::Aarch32Encoder *>(test->GetEncoder())->GetMasm()->Push(aarch32::VixlReg(link_reg));

    // First type-dependency
    auto input = test->GetParameter(TypeInfo(Src(0)), 0);
    auto output = test->GetParameter(TypeInfo(Dst(0)), 0);
    // Main test call
    test->GetEncoder()->EncodeCast(output, std::is_signed_v<Dst>, input, std::is_signed_v<Src>);

    // Finalize
    static_cast<aarch32::Aarch32Encoder *>(test->GetEncoder())->GetMasm()->Pop(aarch32::VixlReg(link_reg));
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

        if (!test->CallCode<Src, DstExt>(nan, Dst(0))) {
            return false;
        }
    }

    return true;
}

TEST_F(Encoder32Test, CastTest)
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

    // Test float32/64 -> float32/64
    EXPECT_TRUE((TestCast<float, float>(this)));
    EXPECT_TRUE((TestCast<double, double>(this)));
    EXPECT_TRUE((TestCast<float, double>(this)));
    EXPECT_TRUE((TestCast<double, float>(this)));

    // We DON'T support cast from float32/64 to int8/16.

    // Test float32
    EXPECT_TRUE((TestCast<float, int32_t>(this)));
    EXPECT_TRUE((TestCast<float, int64_t>(this)));
    EXPECT_TRUE((TestCast<float, uint32_t>(this)));
    EXPECT_TRUE((TestCast<float, uint64_t>(this)));

    // Test float64
    EXPECT_TRUE((TestCast<double, int32_t>(this)));
    EXPECT_TRUE((TestCast<double, int64_t>(this)));
    EXPECT_TRUE((TestCast<double, uint32_t>(this)));
    EXPECT_TRUE((TestCast<double, uint64_t>(this)));
}

template <typename T>
bool TestDiv(Encoder32Test *test)
{
    bool is_signed = std::is_signed<T>::value;

    auto link_reg = Target::Current().GetLinkReg();
    ArenaVector<Reg> used_regs(test->GetAllocator()->Adapter());
    used_regs.emplace_back(link_reg);
    test->PreWork<T>(&used_regs);

    // First type-dependency
    auto param_1 = test->GetParameter(TypeInfo(T(0)), 0);
    auto param_2 = test->GetParameter(TypeInfo(T(0)), 1);

    // Main test call
    test->GetEncoder()->EncodeDiv(param_1, is_signed, param_1, param_2);

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

TEST_F(Encoder32Test, DivTest)
{
    EXPECT_TRUE(TestDiv<int8_t>(this));
    EXPECT_TRUE(TestDiv<int16_t>(this));
    EXPECT_TRUE(TestDiv<int32_t>(this));
    EXPECT_TRUE(TestDiv<int64_t>(this));
    EXPECT_TRUE(TestDiv<uint8_t>(this));
    EXPECT_TRUE(TestDiv<uint16_t>(this));
    EXPECT_TRUE(TestDiv<uint32_t>(this));
    EXPECT_TRUE(TestDiv<uint64_t>(this));
    EXPECT_TRUE(TestDiv<float>(this));
    EXPECT_TRUE(TestDiv<double>(this));
}

template <typename T>
bool TestMod(Encoder32Test *test)
{
    bool is_signed = std::is_signed<T>::value;

    auto link_reg = Target::Current().GetLinkReg();
    ArenaVector<Reg> used_regs(test->GetAllocator()->Adapter());
    used_regs.emplace_back(link_reg);

    test->PreWork<T>(&used_regs);

    static_cast<aarch32::Aarch32Encoder *>(test->GetEncoder())->GetMasm()->PushRegister(vixl::aarch32::r10);
    static_cast<aarch32::Aarch32Encoder *>(test->GetEncoder())->GetMasm()->PushRegister(vixl::aarch32::r11);

    // First type-dependency
    auto param_1 = test->GetParameter(TypeInfo(T(0)), 0);
    auto param_2 = test->GetParameter(TypeInfo(T(0)), 1);

    // Main test call
    test->GetEncoder()->EncodeMod(param_1, is_signed, param_1, param_2);

    static_cast<aarch32::Aarch32Encoder *>(test->GetEncoder())->GetMasm()->Pop(vixl::aarch32::r11);
    static_cast<aarch32::Aarch32Encoder *>(test->GetEncoder())->GetMasm()->Pop(vixl::aarch32::r10);

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

TEST_F(Encoder32Test, ModTest)
{
    EXPECT_TRUE(TestMod<int8_t>(this));
    EXPECT_TRUE(TestMod<int16_t>(this));
    EXPECT_TRUE(TestMod<int32_t>(this));
    EXPECT_TRUE(TestMod<int64_t>(this));
    EXPECT_TRUE(TestMod<uint8_t>(this));
    EXPECT_TRUE(TestMod<uint16_t>(this));
    EXPECT_TRUE(TestMod<uint32_t>(this));
    EXPECT_TRUE(TestMod<uint64_t>(this));
    EXPECT_TRUE(TestMod<float>(this));
    EXPECT_TRUE(TestMod<double>(this));
}

// MemCopy Test
// TEST_F(Encoder32Test, MemCopyTest) {
//  EncodeMemCopy(MemRef mem_from, MemRef mem_to, size_t size)

// MemCopyz Test
// TEST_F(Encoder32Test, MemCopyzTest) {
//  EncodeMemCopyz(MemRef mem_from, MemRef mem_to, size_t size)

template <int ID, typename T>
bool TestParam(Encoder32Test *test)
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
    test->PreWork<T>(&used_regs);

    auto ret_reg = test->GetParameter(curr_param, 0);

    auto fl = test->GetEncoder()->GetFrameLayout();

    // Main test call
    if (std::holds_alternative<Reg>(par)) {
        test->GetEncoder()->EncodeMov(ret_reg, std::get<Reg>(par));
    } else {
        auto mem = MemRef(Target::Current().GetStackReg(),
                          fl.GetFrameSize<CFrameLayout::BYTES>() + std::get<uint8_t>(par) * fl.GetSlotSize());
        test->GetEncoder()->EncodeLdr(ret_reg, is_signed, mem);
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

    auto size = test->GetCallconv()->GetCodeSize();
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

TEST_F(Encoder32Test, ReadParams)
{
    EXPECT_TRUE((TestParam<0, int32_t>(this)));
    EXPECT_TRUE((TestParam<1, uint64_t>(this)));
    EXPECT_TRUE((TestParam<2, int32_t>(this)));
    EXPECT_TRUE((TestParam<3, int64_t>(this)));
    EXPECT_TRUE((TestParam<4, int32_t>(this)));
    EXPECT_TRUE((TestParam<5, int32_t>(this)));
}

template <typename T, Condition cc>
bool TestSelect(Encoder32Test *test)
{
    auto masm = static_cast<aarch32::Aarch32Encoder *>(test->GetEncoder())->GetMasm();

    // Initialize
    test->PreWork<bool>();

    // First type-dependency
    auto param0 = test->GetParameter(TypeInfo(T(0)), 0);
    auto param1 = test->GetParameter(TypeInfo(T(0)), 1);
    auto param2 = Reg(5, TypeInfo(uint32_t(0)));
    auto param3 = Reg(6, TypeInfo(uint32_t(0)));

    // Main test call
    masm->Push(aarch32::VixlReg(param2));
    masm->Push(aarch32::VixlReg(param3));
    test->GetEncoder()->EncodeMov(param2, Imm(1));
    test->GetEncoder()->EncodeMov(param3, Imm(0));
    test->GetEncoder()->EncodeSelect(Reg(param0.GetId(), TypeInfo(uint32_t(0))), param2, param3, param0, param1, cc);
    masm->Pop(aarch32::VixlReg(param3));
    masm->Pop(aarch32::VixlReg(param2));

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

TEST_F(Encoder32Test, SelectTest)
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
bool TestSelectTest(Encoder32Test *test)
{
    // Initialize
    test->PreWork<T>();

    auto param0 = test->GetParameter(TypeInfo(T(0)), 0);
    auto param1 = test->GetParameter(TypeInfo(T(0)), 1);
    auto param2 = Reg(5, TypeInfo(uint32_t(0)));
    auto param3 = Reg(6, TypeInfo(uint32_t(0)));
    [[maybe_unused]] T imm_value {};
    if (is_imm) {
        imm_value = random_gen<T>() & MAX_IMM_VALUE;
    }

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
        T tmp0 = random_gen<T>();
        T tmp1 = random_gen<T>();

        T second_operand;
        if constexpr (is_imm) {
            second_operand = imm_value;
        } else {
            second_operand = tmp1;
        }

        T and_res = tmp0 & second_operand;
        bool res = cc == Condition::TST_EQ ? and_res == 0 : and_res != 0;

        // Main check - compare parameter and return value
        if (!test->CallCode<T, bool>(tmp0, tmp1, res)) {
            return false;
        }
    }
    return true;
}

TEST_F(Encoder32Test, SelectTestTest)
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

    EXPECT_TRUE((TestSelectTest<int64_t, Condition::TST_EQ, false>(this)));
    EXPECT_TRUE((TestSelectTest<int64_t, Condition::TST_NE, false>(this)));
}

template <typename T, Condition cc>
bool TestCompareTest(Encoder32Test *test)
{
    // Initialize
    test->PreWork<T>();

    // First type-dependency
    auto param1 = test->GetParameter(TypeInfo(T(0)), 0);
    auto param2 = test->GetParameter(TypeInfo(T(0)), 1);

    // Main test call
    test->GetEncoder()->EncodeCompareTest(param1, param1, param2, cc);

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

TEST_F(Encoder32Test, CompareTestTest)
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
bool TestJumpTest(Encoder32Test *test)
{
    // Initialize
    test->PreWork<T>();

    // First type-dependency
    auto param0 = test->GetParameter(TypeInfo(T(0)), 0);
    auto param1 = test->GetParameter(TypeInfo(T(0)), 1);
    auto ret_val = Target::Current().GetParamReg(0);

    auto true_branch = test->GetEncoder()->CreateLabel();
    auto end = test->GetEncoder()->CreateLabel();
    [[maybe_unused]] T imm_value = random_gen<T>() & MAX_IMM_VALUE;

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

TEST_F(Encoder32Test, JumpTestTest)
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
    EXPECT_TRUE((TestJumpTest<uint32_t, Condition::TST_EQ, true>(this)));
    EXPECT_TRUE((TestJumpTest<uint32_t, Condition::TST_NE, true>(this)));
}

template <typename T, bool is_acquire>
bool TestLoadExclusive(Encoder32Test *test, uint64_t input_word, T expected_result)
{
    uint64_t buffer[3] = {0xFFFFFFFFFFFFFFFFULL, input_word, 0xFFFFFFFFFFFFFFFFULL};
    test->PreWork<T>();
    auto param_0 = test->GetParameter(TypeInfo(uintptr_t(0)), 0);
    auto result = test->GetParameter(TypeInfo(T(0)), 0);
    test->GetEncoder()->EncodeLdrExclusive(result, param_0, is_acquire);
    test->PostWork<T>();

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
bool TestStoreExclusiveFailed(Encoder32Test *test)
{
    uint64_t buffer = 0xFFFFFFFFFFFFFFFFULL;
    test->PreWork<T>();
    auto param_0 = test->GetParameter(TypeInfo(uintptr_t(0)), 0);
    auto tmp_reg = test->GetParameter(TypeInfo(T(0)), 1);
    auto result = test->GetParameter(TypeInfo(uint32_t(0)), 0);

    // perform store without load - should fail as there are no exlusive monitor
    test->GetEncoder()->EncodeStrExclusive(result, tmp_reg, param_0, is_release);
    test->PostWork<T>();

    if (!test->GetEncoder()->GetResult()) {
        std::cerr << "Unsupported for " << TypeName<T>() << "\n";
        return false;
    }
    // Change this for enable print disasm
    test->Dump(false);
    auto word_addr = reinterpret_cast<uintptr_t>(&buffer);
    return test->CallCodeVariadic<uint32_t, uintptr_t, T>(1, word_addr, 0);
}

template <typename T, bool is_release>
bool TestStoreExclusive(Encoder32Test *test, T value, uint64_t expected_result)
{
    // test writes value into buffer[1]
    uint64_t buffer[3] = {0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL};
    test->PreWork<T>();
    auto param_0 = test->GetParameter(TypeInfo(uintptr_t(0)), 0);
    auto param_1 = test->GetParameter(TypeInfo(T(0)), 1);
    auto result = test->GetParameter(TypeInfo(uint32_t(0)), 0);
    auto retry_label = test->GetEncoder()->CreateLabel();
    // use reg 4 instead of actual tmp reg to correctly encode double-word loads
    Reg tmp_reg(4, TypeInfo(T(0)));

    test->GetEncoder()->BindLabel(retry_label);
    test->GetEncoder()->EncodeLdrExclusive(tmp_reg, param_0, false);
    test->GetEncoder()->EncodeStrExclusive(result, param_1, param_0, is_release);
    // retry if store exclusive returned non zero value
    test->GetEncoder()->EncodeJump(retry_label, result, Condition::NE);

    test->PostWork<T>();

    if (!test->GetEncoder()->GetResult()) {
        std::cerr << "Unsupported for " << TypeName<T>() << "\n";
        return false;
    }
    // Change this for enable print disasm
    test->Dump(false);
    auto word_addr = reinterpret_cast<uintptr_t>(&buffer[1]);
    if (!test->CallCodeVariadic<uint32_t, uintptr_t, T>(0, word_addr, value)) {
        return false;
    }
    if (buffer[1] != expected_result) {
        std::cerr << "Failed: expected value " << std::hex << expected_result << " differs from actual result "
                  << buffer[1] << std::dec << std::endl;
        return false;
    }
    return true;
}

TEST_F(Encoder32Test, LoadExclusiveTest)
{
    const uint64_t SOURCE_WORD = 0x1122334455667788ULL;
    // aarch32 is little-endian, so bytes are actually stored in following order:
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

TEST_F(Encoder32Test, StoreExclusiveTest)
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

    EXPECT_TRUE((TestStoreExclusive<uint8_t, true>(this, 0x11U, 0xFFFFFFFFFFFFFF11ULL)));
    EXPECT_TRUE((TestStoreExclusive<uint16_t, true>(this, 0x1122U, 0xFFFFFFFFFFFF1122ULL)));
    EXPECT_TRUE((TestStoreExclusive<uint32_t, true>(this, 0x11223344UL, 0xFFFFFFFF11223344ULL)));
    EXPECT_TRUE((TestStoreExclusive<uint64_t, false>(this, 0xAABBCCDD11335577ULL, 0xAABBCCDD11335577ULL)));
}

TEST_F(Encoder32Test, Registers)
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
