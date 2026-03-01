/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_CONDITION_CHAIN_H
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_CONDITION_CHAIN_H

#include "compiler/optimizer/ir/basicblock.h"

namespace ark::compiler {
class ConditionChainManager;

/*
 * Graph:
 *          | |
 *          v v
 *          [A]------\
 *           |       |
 *           |       v
 *           |<-----[B]
 *           |       |
 *           v       v
 *       -->[S0]    [S1]<--
 *           |       |
 *           v       v
 *
 * Basic blocks A & B is a condition chain.
 * Each basic block in the chain has S0 successor which is called `multiple_predecessor_successor' (all chain basic
 * blocks are predecessors).
 * Only last basic block in chain (B) has S1 successor which is called `single_predecessor_successor`.
 * Both S0 and S1 successors can have predeccessors which are not part of the chain.
 */
class ConditionChain {
public:
    using BlockIterator = ArenaVector<BasicBlock *>::iterator;
    using BlockConstIterator = ArenaVector<BasicBlock *>::const_iterator;

    ConditionChain(BlockIterator begin, size_t size, size_t multiplePredecessorsSuccessorIndex,
                   size_t singlePredecessorSuccessorIndex)
        : begin_(begin),
          size_(size),
          multiplePredecessorsSuccessorIndex_(multiplePredecessorsSuccessorIndex),
          singlePredecessorSuccessorIndex_(singlePredecessorSuccessorIndex)
    {
    }

    BasicBlock *GetFirstBlock()
    {
        return *begin_;
    }

    const BasicBlock *GetFirstBlock() const
    {
        return *begin_;
    }

    BasicBlock *GetLastBlock()
    {
        return *(begin_ + size_ - 1);
    }

    const BasicBlock *GetLastBlock() const
    {
        return *(begin_ + size_ - 1);
    }

    BlockIterator GetBegin()
    {
        return begin_;
    }

    BlockIterator GetEnd()
    {
        return begin_ + size_;
    }

    size_t GetSize() const
    {
        return size_;
    }

    BlockConstIterator GetBegin() const
    {
        return begin_;
    }

    BlockConstIterator GetEnd() const
    {
        return begin_ + size_;
    }

    bool Contains(const BasicBlock *bb) const
    {
        auto last = begin_ + size_;
        return std::find(begin_, last, bb) != last;
    }

    void SetFirstBlock(BasicBlock *bb)
    {
        *begin_ = bb;
    }

    BasicBlock *GetMultiplePredecessorsSuccessor() const
    {
        return GetFirstBlock()->GetSuccessor(multiplePredecessorsSuccessorIndex_);
    }

    BasicBlock *GetSinglePredecessorSuccessor() const
    {
        return GetLastBlock()->GetSuccessor(singlePredecessorSuccessorIndex_);
    }

private:
    BlockIterator begin_;
    size_t size_;
    size_t multiplePredecessorsSuccessorIndex_;
    size_t singlePredecessorSuccessorIndex_;
};
}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_CONDITION_CHAIN_H
