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

#ifndef COMPILER_OPTIMIZER_IR_GRAPH_CHECKER_H
#define COMPILER_OPTIMIZER_IR_GRAPH_CHECKER_H

#include <iostream>
#include "compiler_options.h"
#include "graph.h"
#include "graph_visitor.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/analysis/rpo.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/optimizations/cleanup.h"

namespace panda::compiler {
inline std::ostream &operator<<(std::ostream &os, const std::initializer_list<Opcode> &opcs)
{
    os << "[ ";
    for (auto opc : opcs) {
        os << GetOpcodeString(opc) << " ";
    }
    os << "]";
    return os;
}

class GraphChecker : public GraphVisitor {
public:
    explicit GraphChecker(Graph *graph);
    ~GraphChecker() override
    {
        GetGraph()->GetPassManager()->SetCheckMode(false);
    }

    NO_COPY_SEMANTIC(GraphChecker);
    NO_MOVE_SEMANTIC(GraphChecker);

    void Check();

private:
    void PreCloneChecks(Graph *graph);
    void UserInputCheck(Graph *graph);
    void CheckBlock(BasicBlock *block);
    void CheckDomTree();
    void CheckLoopAnalysis();
    void CheckStartBlock();
    void CheckEndBlock();
    void CheckControlFlow(BasicBlock *block);
    void CheckDataFlow(BasicBlock *block);
    void CheckPhiInputs(Inst *phi_inst);
    void CheckInstsRegisters(BasicBlock *block);
    void CheckPhisRegisters(BasicBlock *block);
    void CheckNoLowLevel(BasicBlock *block);
    void CheckLoops();
    void CheckGraph();
    bool HasOuterInfiniteLoop();
    bool CheckInstHasInput(Inst *inst, Inst *input);
    bool CheckInstHasUser(Inst *inst, Inst *user);
    void CheckCallReturnInlined();
    void CheckSpillFillHolder(Inst *inst);
    bool CheckInstRegUsageSaved(const Inst *inst, Register reg) const;
    void MarkBlocksInLoop(Loop *loop, Marker mrk);
    bool CheckBlockHasPredecessor(BasicBlock *block, BasicBlock *predecessor);
    bool CheckBlockHasSuccessor(BasicBlock *block, BasicBlock *successor);
    void CheckLoopHasSafePoint(Loop *loop);
    void CheckBlockEdges(const BasicBlock &block);
    void CheckTryBeginBlock(const BasicBlock &block);
    void CheckJump(const BasicBlock &block);
    bool IsTryCatchDomination(const BasicBlock *input_block, const BasicBlock *user_block) const;
#ifndef NDEBUG
    bool NeedCheckSaveState();
#endif  // !NDEBUG
    void CheckSaveStateInputs();
    void CheckObjectRec(const Inst *object, const Inst *user, const BasicBlock *block, Inst *start_from,
                        Marker visited) const;
    void FindObjectInSaveState(const Inst *object, Inst *ss) const;
    void CheckSaveStatesWithRuntimeCallUsers();
    void CheckSaveStateOsrRec(const Inst *inst, const Inst *user, BasicBlock *block, Marker visited);

    Graph *GetGraph() const
    {
        return graph_;
    }

    ArenaAllocator *GetAllocator()
    {
        return &allocator_;
    }

    ArenaAllocator *GetLocalAllocator()
    {
        return &local_allocator_;
    }

    const ArenaVector<BasicBlock *> &GetBlocksToVisit() const override
    {
        return GetGraph()->GetBlocksRPO();
    }

    /*
     * Visitors to check instructions types
     */
    static void VisitReturn([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitIf([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitIfImm([[maybe_unused]] GraphVisitor *v, Inst *inst);
#include "visitor.inc"

    static bool CheckCommonTypes(Inst *inst1, Inst *inst2)
    {
        if (inst1->GetBasicBlock()->GetGraph()->IsDynamicMethod() &&
            (inst1->GetType() == DataType::ANY || inst2->GetType() == DataType::ANY)) {
            return true;
        }
        DataType::Type type1 = inst1->GetType();
        DataType::Type type2 = inst2->GetType();
        return DataType::GetCommonType(type1) == DataType::GetCommonType(type2);
    }
private:
    Graph *graph_;
    ArenaAllocator allocator_ {SpaceType::SPACE_TYPE_COMPILER, nullptr, true};
    ArenaAllocator local_allocator_ {SpaceType::SPACE_TYPE_COMPILER, nullptr, true};
    int null_ptr_inst_counter_ = 0;
};
}  // namespace panda::compiler

#endif  // COMPILER_OPTIMIZER_IR_GRAPH_CHECKER_H
