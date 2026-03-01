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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_CSE_H
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_CSE_H

#include "optimizer/ir/graph.h"
#include "optimizer/ir/analysis.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/inst.h"
#include "optimizer/pass.h"
#include "libarkbase/utils/arena_containers.h"
#include "vn.h"
#include "compiler_logger.h"
#include "libarkbase/utils/logger.h"

namespace ark::compiler {
class PANDA_PUBLIC_API Cse : public Optimization {
    using PairInsts = std::pair<Inst *, Inst *>;
    using PairVectorsInsts = std::pair<InstVector, InstVector>;

public:
    explicit Cse(Graph *graph)
        : Optimization(graph),
          replacePair_(graph->GetLocalAllocator()->Adapter()),
          minReplaceStar_(graph->GetLocalAllocator()->Adapter()),
          deletedInsts_(graph->GetLocalAllocator()->Adapter()),
          sameExpPair_(graph->GetLocalAllocator()->Adapter()),
          matchedTuple_(graph->GetLocalAllocator()->Adapter()),
          candidates_(graph->GetLocalAllocator()->Adapter())
    {
    }

    NO_MOVE_SEMANTIC(Cse);
    NO_COPY_SEMANTIC(Cse);
    ~Cse() override = default;

    bool RunImpl() override;

    bool IsEnable() const override
    {
        return g_options.IsCompilerCse();
    }

    const char *GetPassName() const override
    {
        return "Cse";
    }

private:
    void LocalCse();
    /*
       LocalCse eliminates the duplicate computations for every basic block.
       If there are two instructions whose inputs and opcodes and types are all the same,
       then we move users from second instruction to first instructions(first instruction
       is at front of the second), and then delete the second instruction.
    */

    void DomTreeCse();
    /*
       DomTreeCse eliminates the duplicate computations along dominator tree.
       If there are two instructions inst1 and inst2 with the same expression
       such that inst1's block dominates inst2's block, then we can move
       the users of inst2 into inst1, and then delete inst2. Here we make a
       convention that a block does not dominate itself. It can be viewed as
       a generation of LocalCse.
    */

    void GlobalCse();
    /*
       GlobalCse eliminates the redundant computations whose result can be obtained
       from its (two) predecessors. In this case we will use a new Phi instruction
       to take place of the redundant instruction. For example,
       __________________________________          ____________________________________
       |BB 3:         ...                |         |BB 4:           ...                |
       |      6.u32 Add  v0, v1 -> (v8)  |         |       9.u32 Add  v0, v1 -> (v15)  |
       |              ...                |         |                ...                |
       |_________________________________|         |___________________________________|
                        \                                            /
                         \                                          /
                          \                                        /
                           \                                      /
                            \____________________________________/
                            |BB 5:            ...                |
                            |       14.u32 Add  v0, v1 -> (v20)  |
                            |                 ...                |
                            |____________________________________|

        GlobalCse will add a new Phi instruction, to take place of inst 14., as follows:
        __________________________________          ____________________________________
       |BB 3:         ...                |         |BB 4:           ...                |
       | 6.u32 Add  v0, v1 -> (v8, v33p) |         | 9.u32 Add  v0, v1 -> (v15, v33p)  |
       |              ...                |         |                ...                |
       |_________________________________|         |___________________________________|
                        \                                             /
                         \                                           /
                          \                                         /
                           \                                       /
                         __ \_____________________________________/___
                        |BB 5:  33.u32 Phi  v6(bb3), v9(bb4) -> (v20) |
                        |                     ...                     |
                        |                     ...                     |
                        |_____________________________________________|
    */
    struct Exp {
        Opcode opcode;
        DataType::Type type;
        Inst *inputl;
        Inst *inputr;
    };
    static inline bool NotSameExp(Exp exp1, Exp exp2)
    {
        return exp1.opcode != exp2.opcode || exp1.type != exp2.type || exp1.inputl != exp2.inputl ||
               exp1.inputr != exp2.inputr;
    }
    /*  Exp is the key of the instruction.
     *  We will use ArenaMap<Exp, ArenaVector<Inst*>>canditates to record the insts that have been visited.
     *  The instructions that have the same Exp will be put in a ArenaVector whose key is Exp.
     *  With such map, we can search duplicate computations more efficiently.
     */

    static Exp GetExp(Inst *inst)
    {
        ASSERT(IsLegalExp(inst));
        Exp exp = {inst->GetOpcode(), inst->GetType(), inst->GetDataFlowInput(inst->GetInput(0).GetInst()),
                   inst->GetDataFlowInput(inst->GetInput(1).GetInst())};
        return exp;
    }

    static Exp GetExpCommutative(Inst *inst)
    {
        ASSERT(IsLegalExp(inst));
        Exp exp = {inst->GetOpcode(), inst->GetType(), inst->GetDataFlowInput(inst->GetInput(1).GetInst()),
                   inst->GetDataFlowInput(inst->GetInput(0).GetInst())};
        return exp;
    }

    struct Cmpexp {
        bool operator()(Exp exp1, Exp exp2) const
        {
            if (exp1.opcode != exp2.opcode) {
                return static_cast<int>(exp1.opcode) < static_cast<int>(exp2.opcode);
            }
            if (exp1.type != exp2.type) {
                return exp1.type < exp2.type;
            }
            if (exp1.inputl != exp2.inputl) {
                return exp1.inputl->GetId() < exp2.inputl->GetId();
            }
            if (exp1.inputr != exp2.inputr) {
                return exp1.inputr->GetId() < exp2.inputr->GetId();
            }
            return false;
        }
    };

    // We eliminate the duplicate expressions which contain one of the following binary operators. One can add other
    // operators if necessary.
    static bool IsLegalExp(Inst *inst)
    {
        if (inst->IsNotCseApplicable() || !inst->HasUsers()) {
            return false;
        }
        switch (inst->GetOpcode()) {
            case Opcode::Add:
            case Opcode::Sub:
            case Opcode::Mul:
            case Opcode::Div:
            case Opcode::Mod:
            case Opcode::Min:
            case Opcode::Max:
            case Opcode::Shl:
            case Opcode::Shr:
            case Opcode::AShr:
            case Opcode::And:
            case Opcode::Or:
            case Opcode::Xor:
                return true;
            default:
                return false;
        }
    }

    static bool IsCommutative(Inst *inst)
    {
        switch (inst->GetOpcode()) {
            case Opcode::Add:
            case Opcode::Mul:
            case Opcode::Min:
            case Opcode::Max:
            case Opcode::And:
            case Opcode::Or:
            case Opcode::Xor:
                return true;
            default:
                return false;
        }
    }

    class Finder {
    public:
        explicit Finder(Inst *inst) : instruction_(inst) {}
        bool operator()(Inst *instn) const
        {
            return !HasOsrEntryBetween(instn, instruction_);
        }

    private:
        Inst *instruction_;
    };

    template <typename T>
    bool NotIn(const T &candidates, Exp exp)
    {
        return candidates.find(exp) == candidates.end();
    }

    template <typename T>
    bool AllNotIn(const T &candidates, Inst *inst)
    {
        Exp exp = GetExp(inst);
        Exp expc = GetExpCommutative(inst);
        return NotIn(candidates, exp) && (!IsCommutative(inst) || NotIn(candidates, expc));
    }

    static bool InVector(const InstVector &dele, Inst *inst)
    {
        return std::find(dele.begin(), dele.end(), inst) != dele.end();
    }

    void DeleteInstLog(Inst *inst)
    {
        COMPILER_LOG(DEBUG, CSE_OPT) << " Cse deletes inst " << inst->GetId();
        GetGraph()->GetEventWriter().EventCse(inst->GetId(), inst->GetPc());
    }

    void RemoveInstsIn(InstVector *deletedInsts)
    {
        // delete redundant insts
        if (deletedInsts->empty()) {
            return;
        }
        for (auto inst : *deletedInsts) {
            DeleteInstLog(inst);
            auto bb = inst->GetBasicBlock();
            bb->RemoveInst(inst);
        }
        isApplied_ = true;
    }

    void ConvertTreeForestToStarForest();
    void EliminateAlongDomTree();
    void CollectTreeForest();
    void TryAddReplacePair(Inst *inst);
    void BuildSetOfPairs(BasicBlock *block);

private:
    bool isApplied_ = false;
    ArenaVector<PairInsts> replacePair_;
    ArenaVector<PairInsts> minReplaceStar_;
    InstVector deletedInsts_;
    ArenaMap<Exp, PairVectorsInsts, Cmpexp> sameExpPair_;
    ArenaVector<std::pair<Inst *, PairInsts>> matchedTuple_;
    ArenaMap<Exp, InstVector, Cmpexp> candidates_;
};
}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_CSE_H
