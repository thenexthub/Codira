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

#ifndef CPP_ABCKIT_INSTRUCTION_H
#define CPP_ABCKIT_INSTRUCTION_H

#include "base_classes.h"
#include "core/export_descriptor.h"
#include "type.h"
#include "literal_array.h"

#include <functional>

namespace abckit {

/**
 * @brief Instruction
 */
class Instruction final : public ViewInResource<AbckitInst *, const Graph *> {
    // To access private constructor.
    // We restrict constructors in order to prevent C/C++ API mix-up by user.

    /// @brief To access private constructor
    friend class BasicBlock;
    /// @brief To access private constructor
    friend class StaticIsa;
    /// @brief To access private constructor
    friend class DynamicIsa;
    /// @brief abckit::DefaultHash<Instruction>
    friend class abckit::DefaultHash<Instruction>;
    /// @brief To access private constructor
    friend class Graph;

public:
    /**
     * @brief Construct a new empty Instruction object
     */
    Instruction() : ViewInResource(nullptr), conf_(nullptr)
    {
        SetResource(nullptr);
    };

    /**
     * @brief Construct a new Instruction object
     * @param other
     */
    Instruction(const Instruction &other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return Instruction
     */
    Instruction &operator=(const Instruction &other) = default;

    /**
     * @brief Constructor
     * @param other
     */
    Instruction(Instruction &&other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return Instruction
     */
    Instruction &operator=(Instruction &&other) = default;

    /**
     * @brief Destructor
     */
    ~Instruction() override = default;

    /**
     * @brief Inserts `newInst` instruction after `ref` instruction into `ref`'s basic block.
     * @param inst
     * @return Instruction&
     */
    Instruction InsertAfter(Instruction inst) const;

    /**
     * @brief Inserts `newInst` instruction before `ref` instruction into `ref`'s basic block.
     * @param inst
     * @return Instruction&
     */
    Instruction InsertBefore(Instruction inst) const;

    /**
     * @brief Removes instruction from it's basic block.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `bool(*this)` results in `false`
     */
    void Remove() const;

    /**
     * @brief Returns value of I64 constant `Instruction`.
     * @return Value of `Instruction`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `bool(*this)` results in `false`
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `Instruction` is not a constant instruction.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `Instruction` is not I64 constant instruction.
     */
    int64_t GetConstantValueI64() const;

    /**
     * @brief Get the String object
     * @return std::string
     */
    std::string GetString() const;

    /**
     * @brief Get the Next object
     * @return Instruction
     */
    Instruction GetNext() const;

    /**
     * @brief Get the Prev object
     * @return Instruction
     */
    Instruction GetPrev() const;

    /**
     * @brief Get the Function object
     * @return core::Function
     */
    core::Function GetFunction() const;

    /**
     * @brief Returns basic block that owns `Instruction`.
     * @return `BasicBlock` which contains this `Instruction`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `bool(*this)` results in `false`
     */
    BasicBlock GetBasicBlock() const;

    /**
     * @brief Returns number of inputs.
     * @return Number of inputs.
     */
    uint32_t GetInputCount() const;

    /**
     * @brief Returns `inst` input under given `index`.
     * @param [ in ] index - Index of input to be returned.
     * @return input `Instruction`
     */
    Instruction GetInput(uint32_t index) const;

    /**
     * @brief Sets input, overwrites existing input.
     * @return Instruction.
     * @param [ in ] index - Index of input to be set.
     * @param [ in ] input - Input instruction to be set.
     */
    Instruction SetInput(uint32_t index, Instruction input) const;

    /**
     * @brief Enumerates `insts` user instructions, invoking callback `cb` for each user instruction.
     * @param cb - Callback that will be invoked.
     * @return bool
     */
    bool VisitUsers(const std::function<bool(Instruction)> &cb) const;

    /**
     * @brief Returns number of `Instruction` users.
     * @return Number of this `Instruction` users.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `bool(*this)` results in `false`
     */
    uint32_t GetUserCount() const;

    /**
     * @brief Returns LiteralArray argument of `Instruction`.
     * @return LiteralArray
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `Instruction` is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `Instruction` has no LiteralArray argument.
     */
    LiteralArray GetLiteralArray() const;

    /**
     * @brief Returns a pointer to Graph that owns Instruction
     * @return pointer to owning Graph
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `Instruction` is false.
     */
    const Graph *GetGraph() const;

    /**
     * @brief Returns Type argument of Instruction.
     * @return `core::Export`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `Instruction` is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `Instruction` has no LiteralArray argument.
     */
    Type GetType() const;

    /**
     * @brief Returns size in bits of this `Instruction` immediate under given `index`.
     * @param [ in ] index - Index of immediate to get size.
     * @return Size of this `Instruction` immediate under given `index` in bits.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if this `Instruction` is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if this `Instruction` has no immediates.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `index` larger than this `Instruction` immediates number.
     */
    enum AbckitBitImmSize GetImmediateSize(size_t index) const;

    /**
     * @brief Returns ID of instruction.
     * @return ID of instruction.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     */
    uint32_t GetId() const;

    /**
     * @brief Checks that inst is dominated by `dominator`.
     * @return `true` if inst is dominated by `dominator`, `false` otherwise.
     * @param [ in ] dominator - Instruction to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `dominator` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if corresponding `AbckitGraph`s owning `inst` and `dominator` differs.
     */
    bool CheckDominance(Instruction dominator) const;

    /**
     * @brief Checks if `inst` is "call" instruction.
     * @return `true` if `inst` is "call" instruction, `false` otherwise.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     */
    bool CheckIsCall() const;

    /**
     * @brief Enumerates `insts` input instructions, invoking callback `cb` for each input instruction.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in, out ] data - Pointer to the user-defined data that will be passed to the callback `cb` each time
     * it is invoked.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cb` is NULL.
     */
    bool VisitInputs(void *data, bool (*cb)(AbckitInst *input, size_t inputIdx, void *data)) const;

    /**
     * @brief Sets input instructions for `inst` starting from index 0, overwrites existing inputs.
     * @return Instruction.
     * @param [ in ] instrs ... - Instructions to be set as input for `inst`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if one of inst inputs are NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if corresponding `AbckitGraph`s owning `inst` and input inst differs.
     */
    template <typename... Args>
    Instruction SetInputs(Args... instrs);

    /**
     * @brief Appends `input` instruction to `inst` inputs.
     * @return Instruction.
     * @param [ in ] input - Instruction to be appended as input.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is not applicable for input appending.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if corresponding `AbckitGraph`s owning `inst` and `input` differs.
     * @note Allocates
     */
    Instruction AppendInput(Instruction input) const;

    /**
     * @brief Dumps given `inst` into given file descriptor.
     * @return None.
     * @param [ in ] fd - File descriptor where dump is written.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     * @note Allocates
     */
    Instruction Dump(int32_t fd) const;

    /**
     * @brief Sets `inst` function operand.
     * @return None.
     * @param [ in ] function - Function to be set as `inst` operand.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `function` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` has no function operand.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if corresponding `AbckitFile`s owning `inst` and `function` differs.
     */
    Instruction SetFunction(core::Function function) const;

    /**
     * @brief Returns `inst` immediate under given `index`.
     * @return uint64_t .
     * @param [ in ] index - Index of immediate.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` has no immediates.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `index` larger than `inst` immediates number.
     */
    uint64_t GetImmediate(size_t index) const;

    /**
     * @brief Sets `inst` immediate under given `index` with value `imm`.
     * @return None.
     * @param [ in ] index - Index of immediate to be set.
     * @param [ in ] imm - Value of immediate to be set.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` has no immediates.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `index` larger than `inst` immediates number.
     */
    Instruction SetImmediate(size_t index, uint64_t imm) const;

    /**
     * @brief Returns number of `inst` immediates.
     * @return Number of `inst` immediates.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     */
    uint64_t GetImmediateCount() const;

    /**
     * @brief Sets `inst` literal array operand.
     * @return None.
     * @param [ in ] la - Literal array to be set as operand.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `la` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` has no literal array operand.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if corresponding `AbckitFile`s owning `inst` and `la` differs.
     */
    Instruction SetLiteralArray(LiteralArray la) const;

    /**
     * @brief Sets `inst` string operand.
     * @return None.
     * @param [ in ] s - String to be set as operand.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `s` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` has no string operand.
     */
    Instruction SetString(std::string_view s) const;

    /**
     * @brief Returns value of I32 constant `inst`.
     * @return Value of `inst`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is not a constant instruction.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is not I32 constant instruction.
     */
    int32_t GetConstantValueI32() const;

    /**
     * @brief Returns value of U64 constant `inst`.
     * @return Value of `inst`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is not a constant instruction.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is not U64 constant instruction.
     */
    uint64_t GetConstantValueU64() const;

    /**
     * @brief Returns value of F64 constant `inst`.
     * @return Value of `inst`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is not a constant instruction.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is not F64 constant instruction.
     */
    double GetConstantValueF64() const;

protected:
    /**
     * @brief Get the Api Config object
     * @return const ApiConfig*
     */
    const ApiConfig *GetApiConfig() const override
    {
        return conf_;
    }

private:
    /**
     * @brief Construct a new Instruction object
     * @param inst
     * @param conf
     * @param graph
     */
    Instruction(AbckitInst *inst, const ApiConfig *conf, const Graph *graph) : ViewInResource(inst), conf_(conf)
    {
        SetResource(graph);
    };
    const ApiConfig *conf_;
};

}  // namespace abckit

#endif  // CPP_ABCKIT_INSTRUCTION_H
