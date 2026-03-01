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

#ifndef COMPILER_TESTS_ENCODER_TEST_COMMON_H
#define COMPILER_TESTS_ENCODER_TEST_COMMON_H

#include <climits>

#include <random>
#include <gtest/gtest.h>

#include "libarkbase/utils/utils.h"
#include "libarkbase/mem/code_allocator.h"
#include "libarkbase/mem/pool_manager.h"
#include "libarkbase/mem/base_mem_stats.h"
#include "scoped_tmp_reg.h"
#include "callconv.h"

template <typename T>
const char *TypeName();
template <typename T>
const char *TypeName(T /*unused*/)
{
    return TypeName<T>();
}

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CLASS_NAME(type)                       \
    template <>                                \
    inline const char *TypeName<type>(void)    \
    {                                          \
        /* CC-OFFNXT(G.PRE.05) function gen */ \
        return #type;                          \
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

constexpr uint64_t SEED = 0x1234;
#ifndef PANDA_NIGHTLY_TEST_ON
constexpr uint64_t ITERATION = 40;
#else
constexpr uint64_t ITERATION = 4000;
#endif
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects,cert-msc51-cpp)
inline auto g_randomGenerator = std::mt19937_64(SEED);

// Max and min exponent on the basus of two float and double
inline const float MIN_EXP_BASE2_FLOAT = std::log2(FLT_MIN);
inline const float MAX_EXP_BASE2_FLOAT = std::log2(FLT_MAX) - 1.0;
inline const double MIN_EXP_BASE2_DOUBLE = std::log2(DBL_MIN);
inline const double MAX_EXP_BASE2_DOUBLE = std::log2(DBL_MAX) - 1.0;

// Masks for generete denormal float numbers
constexpr uint32_t MASK_DENORMAL_FLOAT = 0x807FFFFF;
constexpr uint64_t MASK_DENORMAL_DOUBLE = 0x800FFFFFFFFFFFFF;

// NOLINTBEGIN(readability-magic-numbers)
template <typename T>
T RandomGen()
{
    auto gen {g_randomGenerator()};

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
                    return ark::bit_cast<float, uint32_t>(gen & MASK_DENORMAL_FLOAT);
                } else {
                    return ark::bit_cast<double, uint64_t>(gen & MASK_DENORMAL_DOUBLE);
                }
            }
            default:
                break;
        }

        // Uniform distribution floating value
        std::uniform_real_distribution<T> disNum(1.0F, 2.0F);
        int8_t sign = (gen % 2U) == 0 ? 1 : -1;
        if constexpr (std::is_same_v<T, float>) {
            std::uniform_real_distribution<float> dis(MIN_EXP_BASE2_FLOAT, MAX_EXP_BASE2_FLOAT);
            return sign * disNum(g_randomGenerator) * std::pow(2.0F, dis(g_randomGenerator));
        } else if constexpr (std::is_same_v<T, double>) {
            std::uniform_real_distribution<double> dis(MIN_EXP_BASE2_DOUBLE, MAX_EXP_BASE2_DOUBLE);
            return sign * disNum(g_randomGenerator) * std::pow(2.0F, dis(g_randomGenerator));
        }

        UNREACHABLE();
    }
}

namespace ark::compiler {

class EncoderTestCommon : public ::testing::Test {
public:
    explicit EncoderTestCommon(Arch arch, bool jsNumberCast = false)
    {
        // NOLINTNEXTLINE(readability-magic-numbers)
        ark::mem::MemConfig::Initialize(64_MB, 64_MB, 64_MB, 32_MB, 0, 0);
        PoolManager::Initialize();
        allocator_ = new ArenaAllocator(SpaceType::SPACE_TYPE_COMPILER);
        encoder_ = Encoder::Create(allocator_, arch, false, jsNumberCast);
        encoder_->InitMasm();
        regfile_ = RegistersDescription::Create(allocator_, arch);
        callconv_ = CallingConvention::Create(allocator_, encoder_, regfile_, arch);
        encoder_->SetRegfile(regfile_);
        memStats_ = new BaseMemStats();
        codeAlloc_ = new (std::nothrow) CodeAllocator(memStats_);
    }
    ~EncoderTestCommon() override
    {
        Logger::Destroy();
        encoder_->~Encoder();
        delete allocator_;
        delete codeAlloc_;
        delete memStats_;
        PoolManager::Finalize();
        ark::mem::MemConfig::Finalize();
    }

    NO_COPY_SEMANTIC(EncoderTestCommon);
    NO_MOVE_SEMANTIC(EncoderTestCommon);

    CodeAllocator *GetCodeAllocator()
    {
        return codeAlloc_;
    }

    void ResetCodeAllocator(void *ptr, size_t size)
    {
        os::mem::MapRange<std::byte> memRange(static_cast<std::byte *>(ptr), size);
        memRange.MakeReadWrite();
        delete codeAlloc_;
        codeAlloc_ = new (std::nothrow) CodeAllocator(memStats_);
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
        return currCursor_;
    }

    void Dump(bool enabled)
    {
        if (enabled) {
            auto size = callconv_->GetCodeSize() - currCursor_;
            for (uint32_t i = currCursor_; i < currCursor_ + size;) {
                i = encoder_->DisasmInstr(std::cout, i, 0);
                std::cout << std::endl;
            }
        }
    }

protected:
    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    ArenaAllocator *allocator_ {nullptr};
    Encoder *encoder_ {nullptr};
    RegistersDescription *regfile_ {nullptr};
    CallingConvention *callconv_ {nullptr};
    CodeAllocator *codeAlloc_ {nullptr};
    BaseMemStats *memStats_ {nullptr};
    size_t currCursor_ {0};
    // NOLINTEND(misc-non-private-member-variables-in-classes)
};
// NOLINTEND(readability-magic-numbers)

}  // namespace ark::compiler

#endif  // COMPILER_TESTS_AARCH32_ENCODER32_TEST_H
