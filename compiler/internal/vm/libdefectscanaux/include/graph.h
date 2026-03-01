/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef LIBARK_DEFECT_SCAN_AUX_INCLUDE_GRAPH_H
#define LIBARK_DEFECT_SCAN_AUX_INCLUDE_GRAPH_H

#include <string>
#include <vector>
#include "compiler/optimizer/ir/basicblock.h"
#include "compiler/optimizer/ir/graph.h"
#include "inst_type.h"

namespace panda::defect_scan_aux {
class BasicBlock;
class Graph;

// a wrapper class for compiler::Inst
class Inst {
public:
    explicit Inst(const compiler::Inst *inst) : inst_(inst)
    {
        type_ = GetInstType(inst_);
    }
    ~Inst() = default;

    bool operator==(const Inst &inst) const;
    bool operator!=(const Inst &inst) const;
    InstType GetType() const;
    bool IsInstStLexVar() const;
    bool IsInstLdLexVar() const;
    bool IsInstStGlobal() const;
    bool IsInstLdGlobal() const;
    uint16_t GetArgIndex() const;
    uint32_t GetPc() const;
    uint32_t GetInstId() const;
    BasicBlock GetBasicBlock() const;
    Graph GetGraph() const;
    std::vector<Inst> GetInputInsts() const;
    std::vector<Inst> GetUserInsts() const;
    std::vector<uint32_t> GetImms() const;

private:
    InstType GetInstType(const compiler::Inst *inst);

    const compiler::Inst *inst_ {nullptr};
    InstType type_ {InstType::INVALID_TYPE};
};

// a wrapper class for compiler::BasicBlock
class BasicBlock {
public:
    explicit BasicBlock(const compiler::BasicBlock *bb) : bb_(bb) {}
    ~BasicBlock() = default;

    bool operator==(const BasicBlock &bb) const;
    bool operator!=(const BasicBlock &bb) const;
    Graph GetGraph() const;
    std::vector<BasicBlock> GetPredBlocks() const;
    std::vector<BasicBlock> GetSuccBlocks() const;
    std::vector<Inst> GetInstList() const;

private:
    const compiler::BasicBlock *bb_ {nullptr};
};

// a wrapper class for compiler::Graph
class Graph {
public:
    using InstVisitor = std::function<void(const Inst &)>;
    explicit Graph(const compiler::Graph *graph) : graph_(graph) {}
    ~Graph() = default;

    BasicBlock GetStartBasicBlock() const;
    BasicBlock GetEndBasicBlock() const;
    std::vector<BasicBlock> GetBasicBlockList() const;
    void VisitAllInstructions(const InstVisitor visitor) const;

private:
    const compiler::Graph *graph_ {nullptr};
};
}  // namespace panda::defect_scan_aux

#endif  // LIBARK_DEFECT_SCAN_AUX_INCLUDE_GRAPH_H