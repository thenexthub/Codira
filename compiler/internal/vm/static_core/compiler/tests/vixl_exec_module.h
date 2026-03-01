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

#ifndef COMPILER_TESTS_VIXL_TEST_H
#define COMPILER_TESTS_VIXL_TEST_H

#ifndef USE_VIXL_ARM64
#error "Unsupported!"
#endif

#include "aarch64/simulator-aarch64.h"
#include "aarch64/macro-assembler-aarch64.h"
#include "optimizer/code_generator/operands.h"
#include "optimizer/ir/constants.h"
#include "optimizer/ir/runtime_interface.h"
#include "unit_test.h"

#include <cstring>
#include <cstdlib>
#include "libarkbase/utils/utils.h"

namespace ark::compiler {
using namespace vixl::aarch64;  // NOLINT(*-build-using-namespace)

template <class T>
T CutValue(uint64_t data, DataType::Type type)
{
    switch (type) {
        default:
        case (DataType::VOID):
        case (DataType::NO_TYPE):
            ASSERT(false);
            return -1;
        case (DataType::BOOL):
            return static_cast<T>((static_cast<bool>(data)));
        case (DataType::UINT8):
            return static_cast<T>((static_cast<uint8_t>(data)));
        case (DataType::INT8):
            return static_cast<T>((static_cast<int8_t>(data)));
        case (DataType::UINT16):
            return static_cast<T>((static_cast<uint16_t>(data)));
        case (DataType::INT16):
            return static_cast<T>((static_cast<int16_t>(data)));
        case (DataType::UINT32):
            return static_cast<T>((static_cast<uint32_t>(data)));
        case (DataType::INT32):
            return static_cast<T>((static_cast<int32_t>(data)));
        case (DataType::UINT64):
            return static_cast<T>((static_cast<uint64_t>(data)));
        case (DataType::INT64):
            return static_cast<T>((static_cast<int64_t>(data)));
        case (DataType::FLOAT32):
            return static_cast<T>((static_cast<float>(data)));
        case (DataType::FLOAT64):
            return static_cast<T>((static_cast<double>(data)));
    }
    return 0;
}

// Class for emulate generated arm64 code
class VixlExecModule {
public:
    VixlExecModule(ark::ArenaAllocator *allocator, RuntimeInterface *runtimeInfo)
        : decoder_(allocator),
          simulator_(allocator, &decoder_, SimStack().Allocate()),
          runtimeInfo_(runtimeInfo),
          allocator_(allocator),
          params_(allocator->Adapter()) {};

    void SetInstructions(const char *start, const char *end)
    {
        startPointer_ = reinterpret_cast<const Instruction *>(start);
        endPointer_ = reinterpret_cast<const Instruction *>(end);
    }

    // Original type
    template <typename T>
    static constexpr DataType::Type GetType()
    {
        if constexpr (std::is_same_v<T, bool>) {
            return DataType::BOOL;
        } else if constexpr (std::is_same_v<T, uint8_t>) {
            return DataType::UINT8;
        } else if constexpr (std::is_same_v<T, int8_t>) {
            return DataType::INT8;
        } else if constexpr (std::is_same_v<T, uint16_t>) {
            return DataType::UINT16;
        } else if constexpr (std::is_same_v<T, int16_t>) {
            return DataType::INT16;
        } else if constexpr (std::is_same_v<T, uint32_t>) {
            return DataType::UINT32;
        } else if constexpr (std::is_same_v<T, int32_t>) {
            return DataType::INT32;
        } else if constexpr (std::is_same_v<T, uint64_t>) {
            return DataType::UINT64;
        } else if constexpr (std::is_same_v<T, int64_t>) {
            return DataType::INT64;
        } else if constexpr (std::is_same_v<T, float>) {
            return DataType::FLOAT32;
        } else if constexpr (std::is_same_v<T, double>) {
            return DataType::FLOAT64;
        }
        return DataType::NO_TYPE;
    }

    // Set/Get one parameter - will updated during emulation
    template <class T>
    void SetParameter(uint32_t idx, T imm)
    {
        constexpr DataType::Type TYPE = GetType<T>();
        if (params_.size() < idx + 1) {
            params_.resize(idx + 1);
        }
        params_[idx] = {imm, TYPE};
    }

    template <typename... Ts>
    void SetParameters(Ts... imm)
    {
        params_.resize(sizeof...(Ts));
        uint32_t idx = 0;
        ((params_[idx++] = {imm, GetType<Ts>()}), ...);
    }

    // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    template <typename T>
    void *CreateArray(T *array, int size, ArenaAllocator *objectAllocator)
    {
        void *arrData = objectAllocator->Alloc(size * sizeof(T) + runtimeInfo_->GetArrayDataOffset(Arch::AARCH64));
        ASSERT(IsAddressInObjectsHeap(arrData));
        int *lenarr = reinterpret_cast<int *>(reinterpret_cast<char *>(arrData) +
                                              runtimeInfo_->GetArrayLengthOffset(Arch::AARCH64));
        lenarr[0] = size;
        T *arr =
            reinterpret_cast<T *>(reinterpret_cast<char *>(arrData) + runtimeInfo_->GetArrayDataOffset(Arch::AARCH64));

        MemcpyUnsafe(arr, array, size * sizeof(T));
        return arrData;
    }

    template <typename T>
    void CopyArray(void *arrData, T *array)
    {
        int *lenarr = reinterpret_cast<int *>(reinterpret_cast<char *>(arrData) +
                                              runtimeInfo_->GetArrayLengthOffset(Arch::AARCH64));
        auto size = lenarr[0];
        T *arr =
            reinterpret_cast<T *>(reinterpret_cast<char *>(arrData) + runtimeInfo_->GetArrayDataOffset(Arch::AARCH64));
        MemcpyUnsafe(array, arr, size * sizeof(T));
    }
    // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)

    void FreeArray([[maybe_unused]] void *array)
    {
        // std::free(array)
    }

    // Return value
    int64_t GetRetValue()
    {
        return simulator_.ReadXRegister(0);
    }

    template <typename T>
    T GetRetValue()
    {
        if constexpr (std::is_same_v<T, float>) {
            return simulator_.ReadVRegister<T>(0);
        } else if constexpr (std::is_same_v<T, double>) {
            return simulator_.ReadVRegister<T>(0);
        }
        return static_cast<T>(simulator_.ReadXRegister(0));
    }

    void WriteParameters()
    {
        // we can put only 7 parameters to registers
        ASSERT(params_.size() <= 7);
        // 0 reg reserve to method
        uint32_t currRegNum = 1;
        uint32_t currVregNum = 0;
        for (auto [imm, type] : params_) {
            if (type == DataType::BOOL) {
                // NOLINTNEXTLINE(readability-implicit-bool-conversion)
                simulator_.WriteXRegister(currRegNum++, std::get<bool>(imm));
            } else if (type == DataType::INT8) {
                simulator_.WriteXRegister(currRegNum++, std::get<int8_t>(imm));
            } else if (type == DataType::UINT8) {
                simulator_.WriteXRegister(currRegNum++, std::get<uint8_t>(imm));
            } else if (type == DataType::INT16) {
                simulator_.WriteXRegister(currRegNum++, std::get<int16_t>(imm));
            } else if (type == DataType::UINT16) {
                simulator_.WriteXRegister(currRegNum++, std::get<uint16_t>(imm));
            } else if (type == DataType::INT32) {
                simulator_.WriteXRegister(currRegNum++, std::get<int32_t>(imm));
            } else if (type == DataType::UINT32) {
                simulator_.WriteXRegister(currRegNum++, std::get<uint32_t>(imm));
            } else if (type == DataType::INT64) {
                simulator_.WriteXRegister(currRegNum++, std::get<int64_t>(imm));
            } else if (type == DataType::UINT64) {
                simulator_.WriteXRegister(currRegNum++, std::get<uint64_t>(imm));
            } else if (type == DataType::FLOAT32) {
                simulator_.WriteVRegister(currVregNum++, std::get<float>(imm));
            } else if (type == DataType::FLOAT64) {
                simulator_.WriteVRegister(currVregNum++, std::get<double>(imm));
            } else {
                UNREACHABLE();
            }
        }
        ClearParameters();
    }

    // Run emulator on current code with current parameters
    void Execute()
    {
        simulator_.ResetState();
        // method
        simulator_.WriteXRegister(0, 0xDEADC0DE);  // NOLINT(readability-magic-numbers)
        // Set x28 to the dummy thread, since prolog of the compiled method writes to it.
        static std::array<uint8_t, sizeof(ManagedThread)> dummy;
        simulator_.WriteCPURegister(XRegister(28U), dummy.data());  // NOLINT(readability-magic-numbers)

        WriteParameters();
        simulator_.RunFrom(reinterpret_cast<const Instruction *>(startPointer_));
    }

    // Change debbuging level
    void SetDump(bool dump)
    {
        if (dump) {
            simulator_.SetTraceParameters(simulator_.GetTraceParameters() | LOG_ALL);  // NOLINT(hicpp-signed-bitwise)
        } else {
            simulator_.SetTraceParameters(LOG_NONE);
        }
    }

    // Print encoded instructions
    void PrintInstructions()
    {
        Decoder decoder(allocator_);
        Disassembler disasm(allocator_);
        decoder.AppendVisitor(&disasm);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        for (auto instr = startPointer_; instr < endPointer_; instr += kInstructionSize) {
            decoder.Decode(instr);
            std::cout << "VIXL disasm " << reinterpret_cast<uintptr_t>(instr) << " : 0x" << std::hex;
            std::cout << std::setfill('0') << std::right << std::setw(sizeof(int64_t));
            std::cout << *(reinterpret_cast<const uint32_t *>(instr)) << ":\t" << disasm.GetOutput() << "\n";
            std::cout << std::setfill(' ');
        }
    }

    Simulator *GetSimulator()
    {
        return &simulator_;
    }

private:
    void ClearParameters()
    {
        // clear size and capacity
        params_ = Params(allocator_->Adapter());
    }

    using Params = ArenaVector<std::pair<
        std::variant<bool, int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t, float, double>,
        DataType::Type>>;

    // VIXL Instruction decoder
    Decoder decoder_;
    // VIXL Simulator
    Simulator simulator_;
    // Begin of executed code
    const Instruction *startPointer_ {nullptr};
    // End of executed code
    const Instruction *endPointer_ {nullptr};

    RuntimeInterface *runtimeInfo_;

    // Dummy allocated data for parameters:
    ark::ArenaAllocator *allocator_;

    Params params_;
};
}  // namespace ark::compiler
#endif  // COMPILER_TESTS_VIXL_TEST_H
