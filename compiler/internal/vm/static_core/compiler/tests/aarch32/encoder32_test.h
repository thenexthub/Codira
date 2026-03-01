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

#ifndef COMPILER_TESTS_AARCH32_ENCODER32_TEST_H
#define COMPILER_TESTS_AARCH32_ENCODER32_TEST_H

#include "target/aarch32/target.h"
#include "tests/encoder_test_common.h"

namespace ark::compiler {
class Encoder32Test : public EncoderTestCommon {
public:
    Encoder32Test() : EncoderTestCommon(Arch::AARCH32) {}

    NO_COPY_SEMANTIC(Encoder32Test);
    NO_MOVE_SEMANTIC(Encoder32Test);

    ~Encoder32Test() override = default;

    // Warning! Do not use multiply times with different types!
    Reg GetParameter(TypeInfo type, int id = 0)
    {
        if (id == 0) {
            auto defaultParam = Target::Current().GetParamReg(0);
            return Reg(defaultParam.GetId(), type);
        }
        // Not supported id > 1
        ASSERT(id == 1);
        if (type == FLOAT64_TYPE) {
            auto defaultParam = Target::Current().GetParamReg(2);
            return Reg(defaultParam.GetId(), type);
        }

        if (type.GetSize() == DOUBLE_WORD_SIZE) {
            auto defaultParam = Target::Current().GetParamReg(2);
            return Reg(defaultParam.GetId(), type);
        }
        auto defaultParam = Target::Current().GetParamReg(1);
        return Reg(defaultParam.GetId(), type);
    }

    template <typename T>
    void PreWork(ArenaVector<Reg> *savedRegs = nullptr)
    {
        // Curor need to encode multiply tests due one execution
        currCursor_ = 0;
        encoder_->SetCursorOffset(0);

        ArenaVector<Reg> usedRegs(GetAllocator()->Adapter());
        if (savedRegs != nullptr) {
            usedRegs.insert(usedRegs.end(), savedRegs->begin(), savedRegs->end());
        }
        for (auto regCode : aarch32::AARCH32_TMP_REG) {
            usedRegs.emplace_back(Reg(regCode, INT32_TYPE));
        }
        GetRegfile()->SetUsedRegs(usedRegs);
        callconv_->GeneratePrologue(FrameInfo::FullPrologue());

        // Move real parameters to getter parameters
#if (PANDA_TARGET_ARM32_ABI_HARD)
        if constexpr (std::is_same<float, T>::value) {
            auto param2 = GetParameter(TypeInfo(T(0)), 1);
            auto s1Register = vixl::aarch32::s1;
            static_cast<aarch32::Aarch32Encoder *>(GetEncoder())
                ->GetMasm()
                ->Vmov(aarch32::VixlVReg(param2).S(), s1Register);
        }
#else
        if constexpr (std::is_same<float, T>::value) {
            auto param1 = GetParameter(TypeInfo(T(0)), 0);
            auto param2 = GetParameter(TypeInfo(T(0)), 1);

            auto storedValue1 = GetParameter(TypeInfo(uint32_t(0)), 0);
            auto storedValue2 = GetParameter(TypeInfo(uint32_t(0)), 1);

            GetEncoder()->EncodeMov(param1, storedValue1);
            GetEncoder()->EncodeMov(param2, storedValue2);
        }
        if constexpr (std::is_same<double, T>::value) {
            auto param1 = GetParameter(TypeInfo(T(0)), 0);
            auto param2 = GetParameter(TypeInfo(T(0)), 1);

            auto storedValue1 = GetParameter(TypeInfo(uint64_t(0)), 0);
            auto storedValue2 = GetParameter(TypeInfo(uint64_t(0)), 1);
            GetEncoder()->EncodeMov(param1, storedValue1);
            GetEncoder()->EncodeMov(param2, storedValue2);
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
            auto returnReg = GetParameter(TypeInfo(uint32_t(0)), 0);
            GetEncoder()->EncodeMov(returnReg, param);
        }
        if constexpr (std::is_same<double, T>::value) {
            auto param = GetParameter(TypeInfo(T(0)), 0);
            auto returnReg = GetParameter(TypeInfo(uint64_t(0)), 0);
            GetEncoder()->EncodeMov(returnReg, param);
        }
#endif
        callconv_->GenerateEpilogue(FrameInfo::FullPrologue(), []() {});
        encoder_->Finalize();
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

    template <size_t PARAM_IDX, bool IS_BINARY>
    void PrintParams()
    {
    }

    template <size_t PARAM_IDX, bool IS_BINARY, typename Arg, typename... Args>
    void PrintParams(Arg arg, Args... args)
    {
        std::cerr << " param" << PARAM_IDX << "=";
        if constexpr (IS_BINARY && std::is_same<double, Arg>::value) {
            std::cerr << bit_cast<uint64_t>(arg);
        } else if constexpr (IS_BINARY && std::is_same<float, Arg>::value) {
            std::cerr << bit_cast<uint32_t>(arg);
        } else {
            std::cerr << arg;
        }
        PrintParams<PARAM_IDX + 1, IS_BINARY>(args...);
    }

    template <typename U, typename... Args>
    bool CallCodeVariadicImpl(const U result, Args... args)
    {
        using FunctPtr = U (*)(Args...);
        auto size = callconv_->GetCodeSize() - currCursor_;
        void *offset = (static_cast<uint8_t *>(callconv_->GetCodeEntry()));
        void *ptr = codeAlloc_->AllocateCode(size, offset);
        auto func = reinterpret_cast<FunctPtr>(ptr);
        const U currResult = func(args...);
        ResetCodeAllocator(ptr, size);
        bool ret = false;
        if constexpr (std::is_floating_point_v<U>) {
            ret = (currResult == result) || (std::isnan(currResult) && std::isnan(result));
        } else {
            ret = (currResult - result == 0);
        }

        if (!ret) {
            std::cerr << std::hex << "Failed CallCode for";
            PrintParams<1, false>(args...);
            std::cerr << " and result=" << result << " current_result=" << currResult << "\n";
            if constexpr (std::is_floating_point_v<U> || (std::is_floating_point_v<Args> || ...)) {
                std::cerr << "In binary :";
                PrintParams<1, true>(args...);
                if constexpr (std::is_same<double, U>::value) {
                    std::cerr << " reslt=" << bit_cast<uint64_t>(result);
                    std::cerr << " current_reslt=" << bit_cast<uint64_t>(currResult);
                } else if constexpr (std::is_same<float, U>::value) {
                    std::cerr << " result=" << bit_cast<uint32_t>(result);
                    std::cerr << " current_result=" << bit_cast<uint32_t>(currResult);
                } else {
                    std::cerr << " result=" << result;
                    std::cerr << " current_result=" << currResult;
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
        using FunctPtr = T (*)(uint32_t param1, T param2);
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

#endif  // COMPILER_TESTS_AARCH32_ENCODER32_TEST_H
