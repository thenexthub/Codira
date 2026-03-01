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

#ifndef COMPILER_TESTS_AARCH64_ENCODER64_TEST_H
#define COMPILER_TESTS_AARCH64_ENCODER64_TEST_H

#include "compiler_options.h"
#include "libarkbase/cpu_features.h"
#include "target/aarch64/target.h"
#include "tests/encoder_test_common.h"

namespace ark::compiler {

template <typename T>
T RandomMaskGen()
{
    static_assert(std::is_integral_v<T>);
    static constexpr size_t MASKS_COUNT = 4U;
    static constexpr std::array<uint64_t, MASKS_COUNT> MASKS = {0x8888888888888888ULL, 0xCCCCCCCCCCCCCCCCULL,
                                                                0xAAAAAAAAAAAAAAAAULL, 0xEEEEEEEEEEEEEEEEULL};
    auto mask = MASKS[g_randomGenerator() % MASKS_COUNT];
    auto typeSize = ark::compiler::TypeInfo(T(0)).GetSize();
    ASSERT(typeSize != 0);
    auto rot = g_randomGenerator() % typeSize;
    auto rotatedMask = (mask << rot) | (mask >> (typeSize - rot));
    return static_cast<T>(rotatedMask);
}

class Encoder64Test : public EncoderTestCommon {
public:
    Encoder64Test() : EncoderTestCommon(Arch::AARCH64, CpuFeaturesHasJscvt()) {}

    NO_COPY_SEMANTIC(Encoder64Test);
    NO_MOVE_SEMANTIC(Encoder64Test);
    ~Encoder64Test() override = default;

    // Warning! Do not use multiply times with different types!
    Reg GetParameter(TypeInfo type, int id = 0)
    {
        ASSERT(id < 4_I);
        if (type.IsFloat()) {
            return Reg(id, type);
        }

        return Target::Current().GetParamReg(id, type);
    }

    void PreWork()
    {
        // Curor need to encode multiply tests due one execution
        currCursor_ = 0;
        encoder_->SetCursorOffset(0);
        callconv_->GeneratePrologue(FrameInfo::FullPrologue());
    }

    void PostWork()
    {
        callconv_->GenerateEpilogue(FrameInfo::FullPrologue(), []() {});
        encoder_->Finalize();
    }

    template <typename T, typename U>
    bool CallCode(const T &param, const U &result)
    {
        // Using max size type: type result "U" or 32bit to check result,
        // because in our ISA min type is 32bit.
        // Only integers less thаn 32bit.
        using UExp = typename std::conditional<(sizeof(U) * BYTE_SIZE) >= WORD_SIZE, U, uint32_t>::type;
        using TExp = typename std::conditional<(sizeof(T) * BYTE_SIZE) >= WORD_SIZE, T, uint32_t>::type;
        using FunctPtr = UExp (*)(TExp data);
        auto size = callconv_->GetCodeSize() - currCursor_;
        void *offset = (static_cast<uint8_t *>(callconv_->GetCodeEntry()));
        void *ptr = codeAlloc_->AllocateCode(size, offset);
        auto func = reinterpret_cast<FunctPtr>(ptr);
        // We must make caste to transfer to function a correctly expanded number
        const UExp currResult = func(static_cast<TExp>(param));
        ResetCodeAllocator(ptr, size);
        bool ret = false;
        if constexpr (std::is_floating_point_v<T> || std::is_floating_point_v<U>) {
            ret = (currResult == result && std::signbit(currResult) == std::signbit(result)) ||
                  (std::isnan(currResult) && std::isnan(result));
        } else {
            ret = (currResult - result == 0);
        }
        if (!ret) {
            std::cerr << std::hex << "Failed CallCode for param=" << param << " and result=" << result
                      << " current_reslt=" << currResult << "\n";
            if constexpr (std::is_floating_point_v<T> || std::is_floating_point_v<U>) {
                std::cerr << "In binary :";
                if constexpr (std::is_same<double, T>::value) {
                    std::cerr << " param=" << bit_cast<uint64_t>(param);
                } else if constexpr (std::is_same<float, T>::value) {
                    std::cerr << " param=" << bit_cast<uint32_t>(param);
                }
                if constexpr (std::is_same<double, U>::value) {
                    std::cerr << " reslt=" << bit_cast<uint64_t>(result);
                    std::cerr << " current_reslt=" << bit_cast<uint64_t>(currResult);
                } else if constexpr (std::is_same<float, U>::value) {
                    std::cerr << " result=" << bit_cast<uint32_t>(result);
                    std::cerr << " current_reslt=" << bit_cast<uint32_t>(currResult);
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
        using UExp = typename std::conditional<(sizeof(U) * BYTE_SIZE) >= WORD_SIZE, U, uint32_t>::type;
        using TExp = typename std::conditional<(sizeof(T) * BYTE_SIZE) >= WORD_SIZE, T, uint32_t>::type;
        using FunctPtr = UExp (*)(TExp param1, TExp param2);
        auto size = callconv_->GetCodeSize() - currCursor_;
        void *offset = (static_cast<uint8_t *>(callconv_->GetCodeEntry()));
        void *ptr = codeAlloc_->AllocateCode(size, offset);
        auto func = reinterpret_cast<FunctPtr>(ptr);
        const UExp currResult = func(static_cast<TExp>(param1), static_cast<TExp>(param2));
        ResetCodeAllocator(ptr, size);
        bool ret = false;
        if constexpr (std::is_same<float, T>::value || std::is_same<double, T>::value) {
            ret = (currResult == result && std::signbit(currResult) == std::signbit(result)) ||
                  (std::isnan(currResult) && std::isnan(result));
        } else {
            ret = (currResult - result == 0);
        }
        if (!ret) {
            std::cerr << "Failed CallCode for param1=" << param1 << " param2=" << param2 << " and result=" << result
                      << " current_result=" << currResult << "\n";
            if constexpr (std::is_floating_point_v<T> || std::is_floating_point_v<U>) {
                std::cerr << "In binary :";
                if constexpr (std::is_same<double, T>::value) {
                    std::cerr << " param1=" << bit_cast<uint64_t>(param1) << " param2=" << bit_cast<uint64_t>(param2);
                } else if constexpr (std::is_same<float, T>::value) {
                    std::cerr << " param1=" << bit_cast<uint32_t>(param1) << " param2=" << bit_cast<uint32_t>(param2);
                }
                if constexpr (std::is_same<double, U>::value) {
                    std::cerr << " reslt=" << bit_cast<uint64_t>(result);
                    std::cerr << " current_reslt=" << bit_cast<uint64_t>(currResult);
                } else if constexpr (std::is_same<float, U>::value) {
                    std::cerr << " result=" << bit_cast<uint32_t>(result);
                    std::cerr << " current_result=" << bit_cast<uint32_t>(currResult);
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
        using TExp = typename std::conditional<(sizeof(T) * BYTE_SIZE) >= WORD_SIZE, T, uint32_t>::type;
        using FunctPtr = TExp (*)(TExp data);
        auto size = callconv_->GetCodeSize() - currCursor_;
        void *offset = (static_cast<uint8_t *>(callconv_->GetCodeEntry()));
        void *ptr = codeAlloc_->AllocateCode(size, offset);
        auto func = reinterpret_cast<FunctPtr>(ptr);
        const TExp currResult = func(static_cast<TExp>(param));
        ResetCodeAllocator(ptr, size);
        bool ret = false;
        if constexpr (std::is_same<float, T>::value || std::is_same<double, T>::value) {
            ret = (currResult == result && std::signbit(currResult) == std::signbit(result)) ||
                  (std::isnan(currResult) && std::isnan(result));
        } else {
            ret = (currResult - result == 0);
        }
        if (!ret) {
            std::cerr << std::hex << "Failed CallCode for param=" << param << " and result=" << result
                      << " current_result=" << currResult << "\n";
            if constexpr (std::is_floating_point_v<T>) {
                std::cerr << "In binary :";
                if constexpr (std::is_same<double, T>::value) {
                    std::cerr << " param=" << bit_cast<uint64_t>(param);
                    std::cerr << " reslt=" << bit_cast<uint64_t>(result);
                    std::cerr << " curr_reslt=" << bit_cast<uint64_t>(currResult);
                } else if constexpr (std::is_same<float, T>::value) {
                    std::cerr << " param=" << bit_cast<uint32_t>(param);
                    std::cerr << " currResult=" << bit_cast<uint32_t>(currResult);
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
        using TExp = typename std::conditional<(sizeof(T) * BYTE_SIZE) >= WORD_SIZE, T, uint32_t>::type;
        using FunctPtr = TExp (*)(TExp param1, TExp param2);
        auto size = callconv_->GetCodeSize() - currCursor_;
        void *offset = (static_cast<uint8_t *>(callconv_->GetCodeEntry()));
        void *ptr = codeAlloc_->AllocateCode(size, offset);
        auto func = reinterpret_cast<FunctPtr>(ptr);
        const TExp currResult = func(static_cast<TExp>(param1), static_cast<TExp>(param2));
        ResetCodeAllocator(ptr, size);
        bool ret = false;
        if constexpr (std::is_same<float, T>::value || std::is_same<double, T>::value) {
            ret = (currResult == result && std::signbit(currResult) == std::signbit(result)) ||
                  (std::isnan(currResult) && std::isnan(result));
        } else {
            ret = (currResult - result == 0);
        }
        if (!ret) {
            std::cerr << "Failed CallCode for param1=" << param1 << " param2=" << param2 << " and result=" << result
                      << " currResult=" << currResult << "\n";
            if constexpr (std::is_floating_point_v<T>) {
                std::cerr << "In binary :";
                if constexpr (std::is_same<double, T>::value) {
                    std::cerr << " param1=" << bit_cast<uint64_t>(param1) << " param2=" << bit_cast<uint64_t>(param2);
                    std::cerr << " reslt=" << bit_cast<uint64_t>(result)
                              << " currResult=" << bit_cast<uint64_t>(currResult);
                } else if constexpr (std::is_same<float, T>::value) {
                    std::cerr << " param1=" << bit_cast<uint32_t>(param1) << " param2=" << bit_cast<uint32_t>(param2);
                    std::cerr << " result=" << bit_cast<uint32_t>(result)
                              << " currResult=" << bit_cast<uint32_t>(currResult);
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
        using FunctPtr = T (*)(uint64_t param1, T param2);
        auto size = callconv_->GetCodeSize() - currCursor_;
        void *offset = (static_cast<uint8_t *>(callconv_->GetCodeEntry()));
        void *ptr = codeAlloc_->AllocateCode(size, offset);
        auto func = reinterpret_cast<FunctPtr>(ptr);
        const T currResult = func(address, param);
        ResetCodeAllocator(ptr, size);
        return currResult;
    }

    template <typename T, typename U>
    U CallCodeCall(T param1, T param2)
    {
        using FunctPtr = U (*)(T param1, T param2);
        auto size = callconv_->GetCodeSize() - currCursor_;
        void *offset = (static_cast<uint8_t *>(callconv_->GetCodeEntry()));
        void *ptr = codeAlloc_->AllocateCode(size, offset);
        auto func = reinterpret_cast<FunctPtr>(ptr);
        const U currResult = func(param1, param2);
        ResetCodeAllocator(ptr, size);
        return currResult;
    }
};

}  // namespace ark::compiler

#endif  // COMPILER_TESTS_AARCH64_ENCODER64_TEST_H
