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

#ifndef COMPILER_TESTS_VIXL_EXEC_MODULE_H
#define COMPILER_TESTS_VIXL_EXEC_MODULE_H

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

namespace panda::compiler {
using namespace vixl::aarch64;

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
            return static_cast<T>(static_cast<bool>(data));
        case (DataType::UINT8):
            return static_cast<T>(static_cast<uint8_t>(data));
        case (DataType::INT8):
            return static_cast<T>(static_cast<int8_t>(data));
        case (DataType::UINT16):
            return static_cast<T>(static_cast<uint16_t>(data));
        case (DataType::INT16):
            return static_cast<T>(static_cast<int16_t>(data));
        case (DataType::UINT32):
            return static_cast<T>(static_cast<uint32_t>(data));
        case (DataType::INT32):
            return static_cast<T>(static_cast<int32_t>(data));
        case (DataType::UINT64):
            return static_cast<T>(static_cast<uint64_t>(data));
        case (DataType::INT64):
            return static_cast<T>(static_cast<int64_t>(data));
        case (DataType::FLOAT32):
            return static_cast<T>(static_cast<float>(data));
        case (DataType::FLOAT64):
            return static_cast<T>(static_cast<double>(data));
    }
    return 0;
}

// Class for emulate generated arm64 code
class VixlExecModule {
public:
    VixlExecModule(panda::ArenaAllocator *allocator, RuntimeInterface *runtime_info)
        : decoder(allocator),
          simulator(allocator, &decoder, SimStack().Allocate()),
          runtime_info_(runtime_info),
          allocator_(allocator),
          params_(allocator->Adapter()) {};

    void SetInstructions(const char *start, const char *end)
    {
        start_pointer = reinterpret_cast<const Instruction *>(start);
        end_pointer = reinterpret_cast<const Instruction *>(end);
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
        constexpr DataType::Type type = GetType<T>();
        if (params_.size() < idx + 1) {
            params_.resize(idx + 1);
        }
        params_[idx] = {imm, type};
    }

    template <typename T>
    void *CreateArray(T *array, int size, ArenaAllocator *object_allocator)
    {
        void *arr_data = object_allocator->Alloc(size * sizeof(T) + runtime_info_->GetArrayDataOffset(Arch::AARCH64));
        ASSERT(IsInObjectsAddressSpace(static_cast<uintptr_t>arr_data));
        int *lenarr = reinterpret_cast<int *>(reinterpret_cast<char *>(arr_data) +
                                              runtime_info_->GetArrayLengthOffset(Arch::AARCH64));
        lenarr[0] = size;
        T *arr = reinterpret_cast<T *>(reinterpret_cast<char *>(arr_data) +
                                       runtime_info_->GetArrayDataOffset(Arch::AARCH64));

        memcpy_s(arr, size * sizeof(T), array, size * sizeof(T));
        return arr_data;
    }

    template <typename T>
    void CopyArray(void *arr_data, T *array)
    {
        int *lenarr = reinterpret_cast<int *>(reinterpret_cast<char *>(arr_data) +
                                              runtime_info_->GetArrayLengthOffset(Arch::AARCH64));
        auto size = lenarr[0];
        T *arr = reinterpret_cast<T *>(reinterpret_cast<char *>(arr_data) +
                                       runtime_info_->GetArrayDataOffset(Arch::AARCH64));
        memcpy_s(array, size * sizeof(T), arr, size * sizeof(T));
    }

    void FreeArray([[maybe_unused]] void *array)
    {
        // std::free(array)
    }

    // Return value
    int64_t GetRetValue()
    {
        return simulator.ReadXRegister(0);
    }

    template <typename T>
    T GetRetValue()
    {
        if constexpr (std::is_same_v<T, float>) {
            return simulator.ReadVRegister<T>(0);
        } else if constexpr (std::is_same_v<T, double>) {
            return simulator.ReadVRegister<T>(0);
        }
        return static_cast<T>(simulator.ReadXRegister(0));
    }

    void WriteParameters()
    {
        // we can put only 7 parameters to registers
        ASSERT(params_.size() <= 7);
        // 0 reg reserve to method
        uint32_t curr_reg_num = 1;
        uint32_t curr_vreg_num = 0;
        for (auto [imm, type] : params_) {
            if (type == DataType::BOOL) {
                simulator.WriteXRegister(curr_reg_num++, std::get<bool>(imm));
            } else if (type == DataType::INT8) {
                simulator.WriteXRegister(curr_reg_num++, std::get<int8_t>(imm));
            } else if (type == DataType::UINT8) {
                simulator.WriteXRegister(curr_reg_num++, std::get<uint8_t>(imm));
            } else if (type == DataType::INT16) {
                simulator.WriteXRegister(curr_reg_num++, std::get<int16_t>(imm));
            } else if (type == DataType::UINT16) {
                simulator.WriteXRegister(curr_reg_num++, std::get<uint16_t>(imm));
            } else if (type == DataType::INT32) {
                simulator.WriteXRegister(curr_reg_num++, std::get<int32_t>(imm));
            } else if (type == DataType::UINT32) {
                simulator.WriteXRegister(curr_reg_num++, std::get<uint32_t>(imm));
            } else if (type == DataType::INT64) {
                simulator.WriteXRegister(curr_reg_num++, std::get<int64_t>(imm));
            } else if (type == DataType::UINT64) {
                simulator.WriteXRegister(curr_reg_num++, std::get<uint64_t>(imm));
            } else if (type == DataType::FLOAT32) {
                simulator.WriteVRegister(curr_vreg_num++, std::get<float>(imm));
            } else if (type == DataType::FLOAT64) {
                simulator.WriteVRegister(curr_vreg_num++, std::get<double>(imm));
            } else {
                UNREACHABLE();
            }
        }
        ClearParameters();
    }

    // Run emulator on current code with current parameters
    void Execute()
    {
        simulator.ResetState();
        // method
        simulator.WriteXRegister(0, 0xDEADC0DE);
        // Set x28 to the dummy thread, since prolog of the compiled method writes to it.
        static std::array<uint8_t, sizeof(ManagedThread)> dummy;
        simulator.WriteCPURegister(XRegister(28), dummy.data());

        WriteParameters();
        simulator.RunFrom(reinterpret_cast<const Instruction *>(start_pointer));
    }

    // Change debbuging level
    void SetDump(bool dump)
    {
        if (dump) {
            simulator.SetTraceParameters(simulator.GetTraceParameters() | LOG_ALL);
        } else {
            simulator.SetTraceParameters(LOG_NONE);
        }
    }

    // Print encoded instructions
    void PrintInstructions()
    {
        Decoder decoder(allocator_);
        Disassembler disasm(allocator_);
        decoder.AppendVisitor(&disasm);
        for (auto instr = start_pointer; instr < end_pointer; instr += kInstructionSize) {
            decoder.Decode(instr);
            std::cout << "VIXL disasm " << reinterpret_cast<uintptr_t>(instr) << " : 0x" << std::hex;
            std::cout << std::setfill('0') << std::right << std::setw(sizeof(int64_t));
            std::cout << *(reinterpret_cast<const uint32_t *>(instr)) << ":\t" << disasm.GetOutput() << "\n";
            std::cout << std::setfill(' ');
        }
    }

    Simulator *GetSimulator()
    {
        return &simulator;
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
    Decoder decoder;
    // VIXL Simulator
    Simulator simulator;
    // Begin of executed code
    const Instruction *start_pointer {nullptr};
    // End of executed code
    const Instruction *end_pointer {nullptr};

    RuntimeInterface *runtime_info_;

    // Dummy allocated data for parameters:
    panda::ArenaAllocator *allocator_;

    Params params_;
};
}  // namespace panda::compiler

#endif  // COMPILER_TESTS_VIXL_EXEC_MODULE_H
