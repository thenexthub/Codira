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

#ifndef BYTECODE_OPTIMIZER_CONSTANT_PROPAGATION_CONSTANT_PROPAGATION_H
#define BYTECODE_OPTIMIZER_CONSTANT_PROPAGATION_CONSTANT_PROPAGATION_H

#include "bytecode_optimizer/ir_interface.h"
#include "compiler/optimizer/ir/basicblock.h"
#include "compiler/optimizer/ir/graph.h"
#include "compiler/optimizer/ir/graph_visitor.h"
#include "compiler/optimizer/pass.h"
#include "lattice_element.h"
#include "utils/hash.h"

namespace panda::bytecodeopt {

using compiler::BasicBlock;
using compiler::Inst;
using compiler::Opcode;
using compiler::RuntimeInterface;

class ConstantPropagation : public compiler::Optimization, public compiler::GraphVisitor {
public:
    explicit ConstantPropagation(compiler::Graph *graph, const BytecodeOptIrInterface *iface);
    NO_MOVE_SEMANTIC(ConstantPropagation);
    NO_COPY_SEMANTIC(ConstantPropagation);
    ~ConstantPropagation() override = default;

    using Edge = std::pair<BasicBlock *, BasicBlock *>;
    struct EdgeHash {
        uint32_t operator()(Edge const &edge) const
        {
            auto block_hash = std::hash<BasicBlock *> {};
            uint32_t lhash = block_hash(edge.first);
            uint32_t rhash = block_hash(edge.second);
            return merge_hashes(lhash, rhash);
        }
    };

    struct EdgeEqual {
        bool operator()(Edge const &edge1, Edge const &edge2) const
        {
            return edge1.first == edge2.first && edge1.second == edge2.second;
        }
    };

    bool RunImpl() override;

    const char *GetPassName() const override
    {
        return "SCCP";
    }

    bool IsEnable() const override
    {
        return true;
    }

    const ArenaVector<BasicBlock *> &GetBlocksToVisit() const override
    {
        return GetGraph()->GetBlocksRPO();
    }

protected:
    void VisitDefault(Inst *inst) override;
    static void VisitPhi(GraphVisitor *v, Inst *inst);
    static void VisitIfImm(GraphVisitor *v, Inst *inst);
    static void VisitCompare(GraphVisitor *v, Inst *inst);
    static void VisitConstant(GraphVisitor *v, Inst *inst);
    static void VisitLoadString(GraphVisitor *v, Inst *inst);
    static void VisitIntrinsic(GraphVisitor *v, Inst *inst);
    static void VisitCastValueToAnyType(GraphVisitor *v, Inst *inst);

#include "compiler/optimizer/ir/visitor.inc"

private:
    void RunSsaEdge();
    void ReWriteInst();
    void RunFlowEdge();
    void VisitInsts(BasicBlock *bb);
    void AddUntraversedFlowEdges(BasicBlock *src, BasicBlock *dst);
    bool GetIfTargetBlock(compiler::IfImmInst *inst, uint64_t val);
    LatticeElement *GetOrCreateDefaultLattice(Inst *inst);
    LatticeElement *FoldingModuleOperation(compiler::IntrinsicInst *inst);
    LatticeElement *FoldingConstant(RuntimeInterface::IntrinsicId id);
    LatticeElement *FoldingConstant(ConstantElement *lattice, RuntimeInterface::IntrinsicId id);
    LatticeElement *FoldingConstant(ConstantElement *left, ConstantElement *right, RuntimeInterface::IntrinsicId id);
    LatticeElement *FoldingCompare(const ConstantElement *left, const ConstantElement *right,
                                   compiler::ConditionCode cc);

    LatticeElement *FoldingGreater(const ConstantElement *left, const ConstantElement *right, bool equal = false);
    LatticeElement *FoldingLess(const ConstantElement *left, const ConstantElement *right, bool equal = false);
    LatticeElement *FoldingEq(const ConstantElement *left, const ConstantElement *right);
    LatticeElement *FoldingNotEq(const ConstantElement *left, const ConstantElement *right);
    LatticeElement *FoldingLdlocalmodulevar(compiler::IntrinsicInst *inst);
    LatticeElement *FoldingLdexternalmodulevar(compiler::IntrinsicInst *inst);
    LatticeElement *FoldingLdobjbyname(compiler::IntrinsicInst *inst);

    Inst *CreateReplaceInst(Inst *base_inst, ConstantElement *lattice);
    void InsertSaveState(Inst *base_inst, Inst *save_state);
    void CheckAndAddToSsaEdge(Inst *inst, LatticeElement *current, LatticeElement *new_lattice);

private:
    ArenaUnorderedMap<Inst *, LatticeElement *> lattices_;
    ArenaQueue<Edge> flow_edges_;
    ArenaQueue<Inst *> ssa_edges_;
    ArenaUnorderedSet<Edge, EdgeHash, EdgeEqual> executable_flag_;
    ArenaUnorderedSet<uint32_t> executed_node_;
    std::string record_name_;
    const BytecodeOptIrInterface *ir_interface_;
};

}  // namespace panda::bytecodeopt

#endif  // BYTECODE_OPTIMIZER_CONSTANT_PROPAGATION_CONSTANT_PROPAGATION_H

