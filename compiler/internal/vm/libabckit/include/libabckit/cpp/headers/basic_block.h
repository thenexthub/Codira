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

#ifndef CPP_ABCKIT_BASIC_BLOCK_H
#define CPP_ABCKIT_BASIC_BLOCK_H

#include "base_classes.h"
#include "instruction.h"

#include <cstdint>
#include <vector>

namespace abckit {

/**
 * @brief BasicBlock
 */
class BasicBlock final : public ViewInResource<AbckitBasicBlock *, const Graph *> {
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    /// @brief to access private constructor
    friend class Graph;
    /// @brief to access private constructor
    friend class Instruction;
    /// @brief abckit::DefaultHash<BasicBlock>
    friend class abckit::DefaultHash<BasicBlock>;

public:
    /**
     * @brief Constructor
     * @param other
     */
    BasicBlock(const BasicBlock &other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return BasicBlock&
     */
    BasicBlock &operator=(const BasicBlock &other) = default;

    /**
     * @brief Constructor
     * @param other
     */
    BasicBlock(BasicBlock &&other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return BasicBlock&
     */
    BasicBlock &operator=(BasicBlock &&other) = default;

    /**
     * @brief Destructor
     */
    ~BasicBlock() override = default;

    /**
     * @brief Enumerates basic blocks successing to the basicBlock, invoking callback `cb` for each basic block.
     * @param [ in ] cb - Callback that will be invoked.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `basicBlock` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cb` is NULL.
     */
    void VisitSuccBlocks(const std::function<void(BasicBlock)> &cb) const;

    /**
     * @brief Enumerates basic blocks predcessing to the basicBlock, invoking callback `cb` for each basic block.
     * @param [ in ] cb - Callback that will be invoked.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `basicBlock` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cb` is NULL.
     */
    void VisitPredBlocks(const std::function<bool(BasicBlock)> &cb) const;

    /**
     * @brief Returns the number of basic blocks successing the basicBlock.
     * @return Number of successor basic blocks.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if basicBlock is NULL.
     */
    uint64_t GetSuccCount() const;

    /**
     * @brief Get the Succ By Idx object
     * @param idx
     * @return BasicBlock
     */
    BasicBlock GetSuccByIdx(uint32_t idx) const;

    /**
     * @brief Get the Succs object
     * @return std::vector<BasicBlock>
     */
    std::vector<BasicBlock> GetSuccs() const;

    /**
     * @brief Get the Preds object
     * @return std::vector<BasicBlock>
     */
    std::vector<BasicBlock> GetPreds() const;

    /**
     * @brief Insert `inst` at the beginning of basicBlock.
     * @return None.
     * @param [ in ] inst - Instruction to insert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if basicBlock is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is constant.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if corresponding `AbckitGraphs`s owning basicBlock and `inst`
     * differs.
     * @note Allocates
     */
    BasicBlock AddInstFront(Instruction inst) const;

    /**
     * @brief Appends `inst` at the end of basicBlock.
     * @return None.
     * @param [ in ] inst - Instruction to insert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if basicBlock is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is constant.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if corresponding `AbckitGraphs`s owning basicBlock and `inst`
     * differs.
     * @note Allocates
     */
    BasicBlock AddInstBack(Instruction inst) const;

    /**
     * @brief Get the Instructions object
     * @return std::vector<Instruction>
     */
    std::vector<Instruction> GetInstructions() const;

    /**
     * @brief Returns first instruction from basicBlock.
     * @return Pointer to the `AbckitInst`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if basicBlock is NULL.
     */
    Instruction GetFirstInst() const;

    /**
     * @brief Returns last instruction.
     * @return `Instruction`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    Instruction GetLastInst() const;

    /**
     * @brief Returns ID of basicBlock.
     * @return ID of basicBlock.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if basicBlock is NULL.
     */
    uint32_t GetId() const;

    /**
     * @brief Returns graph owning basicBlock.
     * @return Pointer to `AbckitGraph`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if basicBlock is NULL.
     */
    const Graph *GetGraph() const;

    /**
     * @brief Returns the number of basic blocks preceding the basicBlock.
     * @return Number of predecessor basic blocks.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if basicBlock is NULL.
     */
    uint64_t GetPredBlockCount() const;

    /**
     * @brief Returns basic block predcessing to basicBlock under given `index`.
     * @return Pointer to the `AbckitBasicBlock`.
     * @param [ in ] index - Index of predecessor basic block.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if basicBlock is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if there is no predecessor basic block under given `index`.
     */
    BasicBlock GetPredBlock(uint32_t index) const;

    /**
     * @brief Returns the number of basic blocks successing the basicBlock.
     * @return Number of successor basic blocks.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if basicBlock is NULL.
     */
    uint64_t GetSuccBlockCount() const;

    /**
     * @brief Returns basic block successing to basicBlock under given `index`.
     * @return Pointer to the `AbckitBasicBlock`.
     * @param [ in ] index - Index of successor basic block.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if basicBlock is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if there is no successor basic block under given `index`.
     */
    BasicBlock GetSuccBlock(uint32_t index) const;

    /**
     * @brief Inserts `succBlock` by `index` in basicBlock successors list
     * and shifts the rest if there were successors with a larger index.
     * @return None.
     * @param [ in ] succBlock - Basic block to be inserted.
     * @param [ in ] index - Index by which the `succBlock` will be inserted.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if basicBlock is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `succBlock` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `index` is larger than quantity of basicBlock successors.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if corresponding `AbckitGraphs`s owning basicBlock and `succBlock`
     * differs.
     * @note Allocates
     */
    BasicBlock InsertSuccBlock(BasicBlock succBlock, uint32_t index);

    /**
     * @brief Appends successor to the end of basicBlock successors list.
     * @return None.
     * @param [ in ] succBlock - Basic block to be inserted.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if basicBlock is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `succBlock` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if corresponding `AbckitGraphs`s owning basicBlock and `succBlock`
     * differs.
     */
    BasicBlock AppendSuccBlock(BasicBlock succBlock);

    /**
     * @brief Deletes the successor and shifts the rest if there were successors with a larger index.
     * @return None.
     * @param [ in ] index - Index of successor to be deleted.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if basicBlock is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `index` is larger than basicBlock successors quantity.
     */
    BasicBlock EraseSuccBlock(uint32_t index);

    /**
     * @brief Returns successor of basicBlock with index 0.
     * @return Pinter to the `AbckitBasicBlock`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if basicBlock is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if basicBlock has no successors.
     */
    BasicBlock GetTrueBranch() const;

    /**
     * @brief Returns successor of basicBlock with index 1.
     * @return Pinter to the `AbckitBasicBlock`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if basicBlock is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if basicBlock has less than one successor.
     */
    BasicBlock GetFalseBranch() const;

    /**
     * @brief Removes all instructions from basicBlock.
     * @return None.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if basicBlock is NULL.
     */
    BasicBlock RemoveAllInsts();

    /**
     * @brief Returns number of instruction in basicBlock.
     * @return Number of instruction in basicBlock.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if basicBlock is NULL.
     */
    uint32_t GetNumberOfInstructions() const;

    /**
     * @brief Returns immediate dominator of basicBlock.
     * @return Pinter to the `AbckitBasicBlock`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if basicBlock is NULL.
     */
    BasicBlock GetImmediateDominator() const;

    /**
     * @brief Checks that basicBlock is dominated by `dominator`
     * @return True if basicBlock is dominated by `dominator`.
     * @param [ in ] dom - Basic block to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if basicBlock is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `dominator` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if corresponding `AbckitGraphs`s owning basicBlock and `dominator`
     * differs.
     */
    bool CheckDominance(BasicBlock dom) const;

    /**
     * @brief Enumerates basic blocks predcessing to the basicBlock, invoking callback `cb` for each basic block.
     * @param [ in ] cb - Callback that will be invoked.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `basicBlock` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cb` is NULL.
     */
    void VisitDominatedBlocks(const std::function<bool(BasicBlock)> &cb) const;

    /**
     * @brief Tells if basicBlock is start basic block.
     * @return `true` if basicBlock is start basic block, `false` otherwise.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if basicBlock is NULL.
     */
    bool IsStart() const;

    /**
     * @brief Tells if basicBlock is end basic block.
     * @return `true` if basicBlock is end basic block, `false` otherwise.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if basicBlock is NULL.
     */
    bool IsEnd() const;

    /**
     * @brief Tells if basicBlock is loop head basic block.
     * @return `true` if basicBlock is loop head basic block, `false` otherwise.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if basicBlock is NULL.
     */
    bool IsLoopHead() const;

    /**
     * @brief Tells if basicBlock is loop prehead basic block.
     * @return `true` if basicBlock is loop prehead basic block, `false` otherwise.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if basicBlock is NULL.
     */
    bool IsLoopPrehead() const;

    /**
     * @brief Tells if basicBlock is try begin basic block.
     * @return `true` if basicBlock is try begin basic block, `false` otherwise.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if basicBlock is NULL.
     */
    bool IsTryBegin() const;

    /**
     * @brief Tells if basicBlock is try basic block.
     * @return `true` if basicBlock is try basic block, `false` otherwise.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if basicBlock is NULL.
     */
    bool IsTry() const;

    /**
     * @brief Tells if basicBlock is try end basic block.
     * @return `true` if basicBlock is try end basic block, `false` otherwise.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if basicBlock is NULL.
     */
    bool IsTryEnd() const;

    /**
     * @brief Tells if basicBlock is catch begin basic block.
     * @return `true` if basicBlock is catch begin basic block, `false` otherwise.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if basicBlock is NULL.
     */
    bool IsCatchBegin() const;

    /**
     * @brief Tells if basicBlock is catch basic block.
     * @return `true` if basicBlock is catch basic block, `false` otherwise.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if basicBlock is NULL.
     */
    bool IsCatch() const;

    /**
     * @brief Dumps basicBlock into given file descriptor.
     * @return None.
     * @param [ in ] fd - File descriptor where dump is written.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if basicBlock is NULL.
     * @note Allocates
     */
    BasicBlock Dump(int32_t fd);

    /**
     * @brief Creates new basic block and moves all instructions after ins into new basic block.
     * @return Pointer to newly create `AbckitBasicBlock`.
     * @param [ in ] ins - Instruction after which all instructions will be moved into new basic block.
     * @param [ in ] makeEdge - If `true` connects old and new basic blocks.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     * @note Allocates
     */
    BasicBlock SplitBlockAfterInstruction(Instruction ins, bool makeEdge);

    /**
     * @brief Creates new Phi instruction
     * @return Instruction
     * @param [ in ] args ... - Phi inputs, must be pointers to `AbckitInst`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     * @note Allocates
     */
    template <typename... Args>
    Instruction CreatePhi(Args... args);

    /**
     * @brief Creates new Phi instruction
     * @return Instruction
     * @param [ in ] args ... - Phi inputs, must be pointers to `AbckitInst`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     * @note Allocates
     */
    template <typename... Args>
    Instruction CreateCatchPhi(Args... args);

protected:
    const ApiConfig *GetApiConfig() const override
    {
        return conf_;
    }

private:
    BasicBlock(AbckitBasicBlock *bb, const ApiConfig *conf, const Graph *graph) : ViewInResource(bb), conf_(conf)
    {
        SetResource(graph);
    };
    const ApiConfig *conf_;
};

}  // namespace abckit

#endif  // CPP_ABCKIT_BASIC_BLOCK_H
