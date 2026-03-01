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

#include "graph_cloner.h"
#include "optimizer/analysis/linear_order.h"
#include "inst_checker_gen.h"
#include "graph_checker.h"

namespace panda::compiler {

GraphChecker::GraphChecker(Graph *graph)
{
    PreCloneChecks(graph);
    graph_ = GraphCloner(graph, GetAllocator(), GetLocalAllocator()).CloneGraph();
    GetGraph()->GetPassManager()->SetCheckMode(true);
}

void GraphChecker::PreCloneChecks(Graph *graph)
{
    UserInputCheck(graph);
}

void GraphChecker::UserInputCheck(Graph *graph)
{
    for (auto block : graph->GetVectorBlocks()) {
        if (block == nullptr) {
            continue;
        }
        for (auto inst : block->AllInsts()) {
            auto u = inst->GetFirstUser();
            ASSERT(u == nullptr || u->GetPrev() == nullptr);
            while (u != nullptr) {
                ASSERT(u->GetNext() == nullptr || u->GetNext()->GetPrev() == u);
                u = u->GetNext();
            }
            for (auto &user : inst->GetUsers()) {
                [[maybe_unused]] auto user_inst = user.GetInst();
                ASSERT(user_inst->GetBasicBlock() != nullptr);
                ASSERT_DO(CheckInstHasInput(user_inst, inst), std::cerr << "Instruction is not an input to its user\n"
                                                                        << "input: " << *inst << std::endl
                                                                        << "user:  " << *user_inst << std::endl);
            }
            for (auto &input : inst->GetInputs()) {
                [[maybe_unused]] auto input_inst = input.GetInst();
                ASSERT(input_inst != nullptr && input_inst->GetBasicBlock() != nullptr);
                ASSERT_DO(CheckInstHasUser(input_inst, inst), std::cerr << "Instruction is not a user to its input:\n"
                                                                        << "user: " << *inst << std::endl
                                                                        << "input:  " << *input_inst << std::endl);
            }
            // Check `require_state` flag
            auto it = std::find_if(inst->GetInputs().begin(), inst->GetInputs().end(),
                                   [](Input input) { return input.GetInst()->IsSaveState(); });
            [[maybe_unused]] bool has_save_state = (it != inst->GetInputs().end());
            ASSERT_DO(inst->RequireState() == has_save_state,
                      std::cerr << "Incorrect 'require_state' flag in the inst: " << *inst);
            if (inst->RequireState()) {
                ASSERT(it->GetInst() == inst->GetSaveState());
            }
        }
    }
}

void GraphChecker::Check()
{
    if (!GetGraph()->IsDynamicMethod()) {
        InstChecker::Run(GetGraph());
    }

#ifndef NDEBUG
    if (GetGraph()->IsAnalysisValid<DominatorsTree>()) {
        CheckDomTree();
    } else {
        GetGraph()->RunPass<DominatorsTree>();
    }
    if (GetGraph()->IsAnalysisValid<LoopAnalyzer>()) {
        CheckLoopAnalysis();
    } else {
        GetGraph()->RunPass<LoopAnalyzer>();
    }
    CheckStartBlock();
    CheckEndBlock();
    size_t blocks_count = 0;
    size_t blocks_id = -1;
    for (auto block : GetGraph()->GetVectorBlocks()) {
        ++blocks_id;
        if (block == nullptr) {
            continue;
        }
        ASSERT_PRINT(block->GetGraph() == GetGraph(), "Block linked to incorrect graph");
        ASSERT_PRINT(block->GetId() == blocks_id, "Block ID must be equal to its ID in graph vector");
        CheckBlock(block);
        blocks_count++;
    }
    ASSERT_PRINT(blocks_count == GetGraph()->GetBlocksRPO().size(), "There is disconnected block");
    CheckLoops();
    // Visit graph to check instructions types
    CheckGraph();
    // Check that call.Inlined and Return.Inlined in correct order
    // and check that savestate has correct link to call.inlined.
    CheckCallReturnInlined();
    if (NeedCheckSaveState()) {
        // Check that objects in stack.
        CheckSaveStateInputs();
        // Check that between savestate and it's runtime call user have not reference insts.
        CheckSaveStatesWithRuntimeCallUsers();
    }

#endif  // !NDEBUG
}

#ifndef NDEBUG
bool GraphChecker::NeedCheckSaveState()
{
    return !GetGraph()->IsBytecodeOptimizer() && GetGraph()->GetParentGraph() == nullptr &&
           GetGraph()->IsInliningComplete() && !GetGraph()->IsSchedulerComplete();
}
#endif  // !NDEBUG

void GraphChecker::CheckBlock([[maybe_unused]] BasicBlock *block)
{
#ifndef NDEBUG
    CheckControlFlow(block);
    CheckDataFlow(block);
    for (auto phi_inst : block->PhiInsts()) {
        CheckPhiInputs(phi_inst);
    }
    if (!GetGraph()->IsLowLevelInstructionsEnabled() && !GetGraph()->IsDynamicMethod()) {
        CheckNoLowLevel(block);
    }
    if (!block->IsEndBlock() && !block->IsStartBlock()) {
        CheckBlockEdges(*block);
    }
    if (block->IsTryBegin()) {
        CheckTryBeginBlock(*block);
    }
    if (block->NeedsJump()) {
        CheckJump(*block);
    }
#endif  // !NDEBUG
}

void GraphChecker::CheckControlFlow(BasicBlock *block)
{
    auto num_succs = block->GetSuccsBlocks().size();
    ASSERT_PRINT(block->IsEndBlock() || block->IsTryBegin() || block->IsTryEnd() ||
                     (num_succs > 0 && num_succs <= MAX_SUCCS_NUM),
                 "Non-end block and non-try-begin block should have 1 or 2 successesors");

    for ([[maybe_unused]] auto pred : block->GetPredsBlocks()) {
        ASSERT_PRINT(CheckBlockHasSuccessor(pred, block), "Block is not a successor to its predecessor");
    }
    for ([[maybe_unused]] auto succ : block->GetSuccsBlocks()) {
        ASSERT_PRINT(CheckBlockHasPredecessor(succ, block), "Block is not a predecessor to its successor");
    }

    if (num_succs == MAX_SUCCS_NUM) {
        ASSERT_PRINT(block->GetSuccessor(0) != block->GetSuccessor(1), "Wrong CFG - block with two same successors");
    }

    for ([[maybe_unused]] auto phi : block->PhiInsts()) {
        ASSERT_DO(phi->GetInputsCount() == block->GetPredsBlocks().size(),
                  std::cerr << phi->GetInputsCount() << " " << block->GetPredsBlocks().size()
                            << "Incorrect phi's inputs count" << *phi);
    }
}

void GraphChecker::CheckDataFlow(BasicBlock *block)
{
    auto graph = block->GetGraph();
    for (auto inst : block->AllInsts()) {
        ASSERT_DO(inst->GetBasicBlock() == block,
                  std::cerr << "Instruction block's pointer isn't correct" << *inst << std::endl);
        if (block != graph->GetStartBlock()) {
            ASSERT_DO(inst->GetOpcode() != Opcode::Parameter,
                      std::cerr << "Not entry block can't contain Parameter instructions" << *inst << std::endl);
        }
        if (inst->GetPrev() == nullptr) {
            ASSERT_PRINT(*block->AllInsts().begin() == inst, "First block instruction isn't correct");
        }
        if (inst->GetNext() == nullptr) {
            ASSERT_PRINT(*block->AllInstsSafeReverse().begin() == inst, "Last block instruction isn't correct");
        }
        // Inst with reference type must have no_cse and no_hoist flags.
        if (inst->GetType() == DataType::REFERENCE) {
            ASSERT(inst->IsNotCseApplicable());
            ASSERT(inst->IsNotHoistable());
        }
        for ([[maybe_unused]] auto &user : inst->GetUsers()) {
            auto user_inst = user.GetInst();
            ASSERT_DO(CheckInstHasInput(user_inst, inst), std::cerr << "Instruction is not an input to its user\n"
                                                                    << "input: " << *inst << std::endl
                                                                    << "user:  " << *user_inst << std::endl);
            if (!user_inst->IsPhi() && !user_inst->IsCatchPhi()) {
                ASSERT_DO(inst->IsDominate(user_inst) ||
                              (GetGraph()->IsRegAllocApplied() &&
                               IsTryCatchDomination(inst->GetBasicBlock(), user_inst->GetBasicBlock())),
                          std::cerr << "Instruction doesn't dominate its user\n"
                                    << "input: " << *inst << std::endl
                                    << "user:  " << *user_inst << std::endl);
            }
            auto arch = graph->GetArch();
            if (DataType::Is32Bits(inst->GetType(), arch)) {
                if (!user_inst->HasType()) {
                    continue;
                }
                [[maybe_unused]] auto user_input_type = user_inst->GetInputType(user.GetIndex());
                [[maybe_unused]] bool ref_to_ptr =
                    user_input_type == DataType::POINTER && inst->GetType() == DataType::REFERENCE;
                ASSERT_DO(DataType::Is32Bits(user_input_type, arch) || ref_to_ptr ||
                              (block->GetGraph()->IsDynamicMethod() && user_input_type == DataType::ANY),
                          std::cerr << "Undefined high-part of input instruction for its user\n"
                                    << "input: " << *inst << std::endl
                                    << "user:  " << *user_inst << std::endl);
            }
        }
        for ([[maybe_unused]] auto input : inst->GetInputs()) {
            ASSERT_DO(CheckInstHasUser(input.GetInst(), inst), std::cerr << "Instruction is not a user to its input:\n"
                                                                         << "input: " << *input.GetInst() << std::endl
                                                                         << "user:  " << *inst << std::endl);
        }
    }
}

void GraphChecker::CheckCallReturnInlined()
{
    ArenaStack<Inst *> inlined_calles(GetLocalAllocator()->Adapter());
    if (GetGraph()->HasEndBlock()) {
        for (auto block : GetGraph()->GetEndBlock()->GetPredsBlocks()) {
            if (block->IsTryEnd()) {
                continue;
            }
        }
    }
    ASSERT(inlined_calles.empty());
#ifndef NDEBUG
    // avoid check after ir_builder in inline pass
    if (!GetGraph()->IsInliningComplete() || GetGraph()->GetParentGraph() != nullptr) {
        return;
    }
#endif
}

void GraphChecker::CheckStartBlock()
{
    [[maybe_unused]] Inst *has_nullptr = nullptr;
    [[maybe_unused]] int32_t last_num = -1;
    ASSERT(GetGraph()->GetStartBlock());
    ASSERT_PRINT(GetGraph()->GetStartBlock()->GetPredsBlocks().empty(), "Start block can't have predecessors");
    ASSERT_PRINT(GetGraph()->GetStartBlock()->GetSuccsBlocks().size() == 1, "Start block should have one successor");
    for (auto inst : GetGraph()->GetStartBlock()->AllInsts()) {
        [[maybe_unused]] Opcode opc = inst->GetOpcode();
        ASSERT_DO(
            opc == Opcode::Constant || opc == Opcode::Parameter || opc == Opcode::SpillFill,
            std::cerr
                << "Entry block can contain Constant, Parameter, NullPtr, SafePoint, NOP or SpillFill instructions"
                << *inst << std::endl);
        if (opc == Opcode::Parameter) {
            auto arg_num = inst->CastToParameter()->GetArgNumber();
            ASSERT_DO(
                last_num < static_cast<int32_t>(arg_num),
                std::cerr << "The argument number in the parameter must be greater than that of the previous parameter"
                          << *inst << std::endl);
            last_num = static_cast<int32_t>(arg_num);
        }
    }
}

void GraphChecker::CheckEndBlock()
{
    if (!GetGraph()->HasEndBlock()) {
        ASSERT_PRINT(HasOuterInfiniteLoop(), "Graph without infinite loops should have end block");
        return;
    }
    ASSERT_PRINT(GetGraph()->GetEndBlock()->GetSuccsBlocks().empty(), "End block can't have successors");
    [[maybe_unused]] auto iter = GetGraph()->GetEndBlock()->Insts();
    ASSERT_PRINT(iter.begin() == iter.end(), "End block can't have instructions");
}

void GraphChecker::CheckGraph()
{
    size_t num_inst = GetGraph()->GetCurrentInstructionId();
    ArenaVector<bool> inst_vec(num_inst, GetLocalAllocator()->Adapter());
    for (auto &bb : GetGraph()->GetBlocksRPO()) {
        for (auto inst : bb->AllInsts()) {
            auto id = inst->GetId();
            ASSERT_DO(id < num_inst,
                      (std::cerr << "Instruction ID must be less than graph instruction counter: " << num_inst << "\n",
                       inst->Dump(&std::cerr)));
            ASSERT_DO(!inst_vec[id],
                      (std::cerr << "Instruction with same Id already exists:\n", inst->Dump(&std::cerr)));
            inst_vec[id] = true;
            ASSERT_DO(GetGraph()->IsDynamicMethod() || inst->GetType() != DataType::ANY,
                      (std::cerr << "The type ANY is supported only for dynamic languages\n", inst->Dump(&std::cerr)));
            ASSERT_DO(inst->SupportsMode(GetGraph()->GetCompilerMode()),
                      (std::cerr << "Instruction used in wrong mode\n", inst->Dump(&std::cerr)));
            VisitInstruction(inst);
        }
    }
}

void GraphChecker::CheckPhiInputs(Inst *phi_inst)
{
    for (size_t index = 0; index < phi_inst->GetInputsCount(); ++index) {
        [[maybe_unused]] auto pred = phi_inst->CastToPhi()->GetPhiInputBb(index);
        [[maybe_unused]] auto input_bb = phi_inst->CastToPhi()->GetPhiInput(pred)->GetBasicBlock();
        ASSERT_DO(input_bb->IsDominate(pred) || IsTryCatchDomination(input_bb, pred),
                  (std::cerr
                   << "Block where phi-input is located should dominate predecessor block corresponding to this input\n"
                   << "Block " << input_bb->GetId() << " should dominate " << pred->GetId() << std::endl
                   << *phi_inst));
    }
}

bool GraphChecker::CheckInstRegUsageSaved(const Inst *inst, Register reg) const
{
    if (reg == ACC_REG_ID) {
        return true;
    }
    auto graph = inst->GetBasicBlock()->GetGraph();
    // Empty vector regs mask means we are using dynamic general regs set.
    if (DataType::IsFloatType(inst->GetType()) && !graph->GetUsedRegs<DataType::FLOAT64>()->empty()) {
        return graph->GetUsedRegs<DataType::FLOAT64>()->at(reg);
    }
    return graph->GetUsedRegs<DataType::INT64>()->at(reg);
}

[[maybe_unused]] static bool checkSpillFillMultiple(const compiler::Inst *inst)
{
    switch (inst->GetOpcode()) {
        case Opcode::Parameter:
            return false;
        default:
            return true;
    }
}

void GraphChecker::CheckNoLowLevel(BasicBlock *block)
{
    for ([[maybe_unused]] auto inst : block->Insts()) {
        ASSERT_DO(!inst->IsLowLevel(), inst->Dump(&std::cerr));
    }
}

void GraphChecker::MarkBlocksInLoop(Loop *loop, Marker mrk)
{
    ASSERT(loop->IsIrreducible() || loop->IsRoot() || loop->GetHeader() != nullptr);
    ASSERT(loop->IsIrreducible() || loop->IsRoot() || loop->IsTryCatchLoop() || loop->GetPreHeader() != nullptr);
    // Mark blocks and check if marker was not set before
    for ([[maybe_unused]] auto block : loop->GetBlocks()) {
        ASSERT(!block->SetMarker(mrk));
    }

    for (auto inner : loop->GetInnerLoops()) {
        MarkBlocksInLoop(inner, mrk);
    }
}

bool GraphChecker::CheckBlockHasPredecessor(BasicBlock *block, BasicBlock *predecessor)
{
    ASSERT(block != nullptr && predecessor != nullptr);
    for (auto pred : block->GetPredsBlocks()) {
        if (pred == predecessor) {
            return true;
        }
    }
    return false;
}

bool GraphChecker::CheckBlockHasSuccessor(BasicBlock *block, BasicBlock *successor)
{
    ASSERT(block != nullptr && successor != nullptr);
    for (auto succ : block->GetSuccsBlocks()) {
        if (succ == successor) {
            return true;
        }
    }
    return false;
}

void GraphChecker::CheckLoopHasSafePoint(Loop *loop)
{
    ASSERT_DO(loop->IsTryCatchLoop() || loop->IsIrreducible(),
              std::cerr << "Loop " << loop->GetId() << " must have safepoint\n");
    for (auto inner : loop->GetInnerLoops()) {
        CheckLoopHasSafePoint(inner);
    }
}

void GraphChecker::CheckLoops()
{
    ASSERT(GetGraph()->GetAnalysis<LoopAnalyzer>().IsValid());
    ASSERT(GetGraph()->GetRootLoop() != nullptr);
    ASSERT(GetGraph()->GetRootLoop()->IsRoot());
    ASSERT(GetGraph()->GetRootLoop()->GetHeader() == nullptr);
    ASSERT(GetGraph()->GetRootLoop()->GetPreHeader() == nullptr);
    auto root_loop = GetGraph()->GetRootLoop();
    auto mrk = GetGraph()->NewMarker();
    MarkBlocksInLoop(root_loop, mrk);

    for ([[maybe_unused]] auto block : GetGraph()->GetBlocksRPO()) {
        [[maybe_unused]] auto loop = block->GetLoop();
        ASSERT(loop != nullptr);
        ASSERT(block->IsMarked(mrk));
        if (block->IsLoopHeader()) {
            [[maybe_unused]] auto preds = block->GetPredsBlocks();
            for ([[maybe_unused]] auto pred : preds) {
                ASSERT(pred->GetLoop() != loop || loop->HasBackEdge(pred));
            }

            if (!loop->IsIrreducible()) {
                for ([[maybe_unused]] auto back : loop->GetBackEdges()) {
                    ASSERT(std::find(preds.begin(), preds.end(), back) != preds.end());
                }
            }
        } else {
            ASSERT(!block->IsOsrEntry());
        }
    }
    GetGraph()->EraseMarker(mrk);
    if (options.IsCompilerUseSafepoint() && GetGraph()->SupportManagedCode()) {
        for (auto inner : root_loop->GetInnerLoops()) {
            CheckLoopHasSafePoint(inner);
        }
    }
}

void GraphChecker::CheckDomTree()
{
    ASSERT(GetGraph()->GetAnalysis<DominatorsTree>().IsValid());
    ArenaVector<BasicBlock *> dominators(GetGraph()->GetVectorBlocks().size(), GetLocalAllocator()->Adapter());
    for (auto block : GetGraph()->GetBlocksRPO()) {
        dominators[block->GetId()] = block->GetDominator();
    }
    // Rebuild dom-tree
    GetGraph()->InvalidateAnalysis<DominatorsTree>();
    GetGraph()->RunPass<DominatorsTree>();

    for ([[maybe_unused]] auto block : GetGraph()->GetBlocksRPO()) {
        ASSERT_DO(dominators[block->GetId()] == block->GetDominator(),
                  std::cerr << "Basic block with id " << block->GetId() << " has incorrect dominator with id "
                            << dominators[block->GetId()]->GetId() << std::endl
                            << "Correct dominator must be block with id " << block->GetDominator()->GetId() << std::endl
                            << "Note: basic blocks' ids in the original graph and in the cloned graph can be different"
                            << std::endl);
    }
}

void GraphChecker::CheckLoopAnalysis()
{
    // Save current loop info
    ArenaUnorderedMap<BasicBlock *, Loop *> loops(GetLocalAllocator()->Adapter());
    [[maybe_unused]] auto root_loop = GetGraph()->GetRootLoop();
    for (auto block : GetGraph()->GetBlocksRPO()) {
        if (block->IsLoopHeader()) {
            loops.emplace(block, block->GetLoop());
        }
    }
    // Build new loop info and compare with saved one
    GetGraph()->InvalidateAnalysis<LoopAnalyzer>();
    GetGraph()->RunPass<LoopAnalyzer>();
    ASSERT_PRINT(*root_loop == *GetGraph()->GetRootLoop(), "Root loop is incorrect\n");
    for (auto &[block, loop] : loops) {
        auto expected_loop = block->GetLoop();
        // An irreducible loop can have different heads, depending on the order of traversal
        if (loop->IsIrreducible()) {
            ASSERT(expected_loop->IsIrreducible());
            continue;
        }
        ASSERT(block->IsLoopHeader());
        if (loop == nullptr || expected_loop == nullptr) {
            UNREACHABLE();
            return;
        }
        ASSERT_DO(*loop == *expected_loop, std::cerr << "Loop " << loop->GetId() << " is incorrect\n");
    }
}

/**
 * Check that there is root's inner loop without exit-points
 */
bool GraphChecker::HasOuterInfiniteLoop()
{
    const auto &loops = GetGraph()->GetRootLoop()->GetInnerLoops();
    return std::find_if(loops.begin(), loops.end(), [](const Loop *loop) { return loop->IsInfinite(); }) != loops.end();
}

bool GraphChecker::CheckInstHasInput(Inst *inst, Inst *input)
{
    ASSERT(inst != nullptr && input != nullptr);
    ASSERT(input->GetBasicBlock() != nullptr);
    ASSERT(input->GetBasicBlock()->GetGraph() != nullptr);
    for (auto node : inst->GetInputs()) {
        if (node.GetInst() == input) {
            return true;
        }
    }
    return false;
}

bool GraphChecker::CheckInstHasUser(Inst *inst, Inst *user)
{
    ASSERT(inst != nullptr && user != nullptr);
    ASSERT(user->GetBasicBlock() != nullptr);
    ASSERT(user->GetBasicBlock()->GetGraph() != nullptr);
    for (auto &node : inst->GetUsers()) {
        if (node.GetInst() == user) {
            return true;
        }
    }
    return false;
}

void GraphChecker::CheckBlockEdges(const BasicBlock &block)
{
    [[maybe_unused]] auto last_inst_in_block = block.GetLastInst();
    if (block.GetSuccsBlocks().size() > 1) {
        ASSERT_PRINT(!block.IsEmpty() || block.IsTryEnd(),
                     "Block with 2 successors have no instructions or should be try-end");
        ASSERT_PRINT(block.IsTryBegin() || block.IsTryEnd() || last_inst_in_block->IsControlFlow(),
                     "Last instruction must be control flow in block with 2 successors");
    } else if (block.GetSuccsBlocks().size() == 1) {
        if (block.GetSuccsBlocks()[0]->IsEndBlock()) {
            if (block.IsEmpty()) {
                ASSERT(block.IsTryEnd());
                return;
            }
            auto last_inst = block.GetLastInst();
            [[maybe_unused]] auto opc = last_inst->GetOpcode();
            ASSERT_PRINT(last_inst->GetFlag(inst_flags::TERMINATOR),
                         "Last instruction in block before exit-block must be Return or Throw instruction.");
        }
    }
}

void GraphChecker::CheckTryBeginBlock(const BasicBlock &block)
{
    ASSERT(block.IsTryBegin());
    auto try_inst_it = std::find_if(block.AllInsts().begin(), block.AllInsts().end(),
                                    [](Inst *inst) { return inst->GetOpcode() == Opcode::Try; });
    ASSERT_PRINT(try_inst_it != block.AllInsts().end(), "Try-begin basic block should contain try-instructions");
    [[maybe_unused]] auto try_inst = (*try_inst_it)->CastToTry();
    for ([[maybe_unused]] auto succ_index : *try_inst->GetCatchEdgeIndexes()) {
        ASSERT_PRINT(succ_index < block.GetSuccsBlocks().size(),
                     "Try instruction holds incorrect try-begin block successor number");
    }
}

void GraphChecker::CheckJump(const BasicBlock &block)
{
    ASSERT(GetGraph()->IsRegAllocApplied());
    ASSERT(GetGraph()->IsAnalysisValid<LinearOrder>());
    if (block.IsIfBlock()) {
        const auto &blocks_vector = GetGraph()->GetBlocksLinearOrder();
        auto if_block_it = std::find(blocks_vector.begin(), blocks_vector.end(), &block);
        ASSERT(if_block_it != blocks_vector.end());
        auto block_after_if = std::next(if_block_it);
        if (block_after_if != blocks_vector.end()) {
            ASSERT_PRINT(*block_after_if != (*if_block_it)->GetFalseSuccessor(),
                         "`If-block` with immediate `false`-successor shouldn't have `JumpFlag`");
            ASSERT_PRINT(*block_after_if != (*if_block_it)->GetTrueSuccessor(),
                         "`true`-successor should be replaced with `false`-successor");
        }
    }
    [[maybe_unused]] auto num_succs = block.GetSuccsBlocks().size();
    ASSERT_PRINT(num_succs == 1 || block.IsTryBegin() || block.IsTryEnd() || block.IsIfBlock(),
                 "Basic block with Jump must have 1 successor or should be try-begin or if block");
}

/**
 * Regalloc propagates catch-phi's inputs to the users and can broke user's domination. In this case:
 * - input_block should be placed inside try block;
 * - try-begin block should dominate user_block;
 *
 * [try-begin]----------\
 *     |                |
 * [input_block]        |
 *     |                |
 * [try-end]----------->|
 *                      |
 *                [catch-begin]
 *                      |
 *                [user_block]
 */
bool GraphChecker::IsTryCatchDomination(const BasicBlock *input_block, const BasicBlock *user_block) const
{
    ASSERT(GetGraph()->IsRegAllocApplied());
    if (input_block->IsTry()) {
        auto blocks = GetGraph()->GetTryBeginBlocks();
        auto it =
            std::find_if(blocks.begin(), blocks.end(), [user_block](auto &bb) { return bb->IsDominate(user_block); });
        return it != blocks.end();
    }
    return false;
}

bool IsObjectCheckDisabledForOpcode(const Inst *inst)
{
    return inst->IsConst();
}

void GraphChecker::CheckSaveStatesWithRuntimeCallUsers()
{
#ifndef NDEBUG
    for (auto &block : GetGraph()->GetBlocksRPO()) {
        for (const auto &ss : block->AllInsts()) {
            if (ss->GetOpcode() != Opcode::SaveState) {
                continue;
            }
        }
    }
#endif
}

void PrepareUsers(Inst *inst, ArenaVector<User *> *users)
{
    for (auto &user : inst->GetUsers()) {
        users->push_back(&user);
    }
}

void GraphChecker::CheckSaveStateInputs()
{
#ifndef NDEBUG
    ArenaVector<User *> users(GetLocalAllocator()->Adapter());
    for (auto &block : GetGraph()->GetBlocksRPO()) {
        for (const auto &inst : block->AllInsts()) {
            if (IsObjectCheckDisabledForOpcode(inst)) {
                continue;
            }
            // skip phi which all inputs is disabled
            if (inst->GetOpcode() == Opcode::Phi) {
                bool skip_flag = true;
                for (const auto &input : inst->GetInputs()) {
                    skip_flag &= IsObjectCheckDisabledForOpcode(input.GetInst());
                }
                if (skip_flag) {
                    continue;
                }
            }

            PrepareUsers(inst, &users);

            auto object_visited = GetGraph()->NewMarker();
            auto osr_visited = GetGraph()->NewMarker();
            for (auto &it : users) {
                auto user = it->GetInst();
                // For Phi we need to check only pass between object and phi
                if (user->IsPhi() || user->IsCatchPhi()) {
                    continue;
                }
                // Virtual register can be overwrite
                if (user->IsSaveState()) {
                    continue;
                }
                if (inst->GetType() == DataType::REFERENCE) {
                    CheckObjectRec(inst, user, user->GetBasicBlock(), user->GetPrev(), object_visited);
                }
                CheckSaveStateOsrRec(inst, user, user->GetBasicBlock(), osr_visited);
            }
            GetGraph()->EraseMarker(object_visited);
            GetGraph()->EraseMarker(osr_visited);
            users.clear();
        }
    }
#endif
}

void GraphChecker::FindObjectInSaveState(const Inst *object, Inst *ss) const
{
    if (object != nullptr && ss != nullptr) {
        std::cerr << "Object not found in the SaveState: " << std::endl
                  << *object << std::endl
                  << " " << *ss << std::endl;
    }
    UNREACHABLE();
}

void GraphChecker::CheckObjectRec(const Inst *object, const Inst *user, const BasicBlock *block, Inst *start_from,
                                  Marker visited) const
{
    if (start_from != nullptr) {
        auto it = InstSafeIterator<IterationType::ALL, IterationDirection::BACKWARD>(*block, start_from);
        for (; it != block->AllInstsSafeReverse().end(); ++it) {
            auto inst = *it;
            if (inst == nullptr) {
                break;
            }
            if (inst->SetMarker(visited)) {
                return;
            }
            if (inst == object || inst == user) {
                return;
            }
        }
    }
    for (auto pred : block->GetPredsBlocks()) {
        // Catch-begin block has edge from try-end block, and all try-blocks should be visited from this edge.
        // `object` can be placed inside try-block - after try-begin, so that visiting try-begin is wrong
        if (block->IsCatchBegin() && pred->IsTryBegin()) {
            continue;
        }
        CheckObjectRec(object, user, pred, pred->GetLastInst(), visited);
    }
}

void GraphChecker::CheckSaveStateOsrRec(const Inst *inst, const Inst *user, BasicBlock *block, Marker visited)
{
    if (block->SetMarker(visited)) {
        return;
    }
    if (inst->GetBasicBlock() == block) {
        return;
    }
    if (block->IsOsrEntry()) {
        ASSERT(GetGraph()->IsOsrMode());
        auto ss = block->GetFirstInst();
        ASSERT(ss != nullptr);
        [[maybe_unused]] auto it =
            std::find_if(ss->GetInputs().begin(), ss->GetInputs().end(),
                         [inst, ss](Input input) { return ss->GetDataFlowInput(input.GetInst()) == inst; });
        ASSERT(it != ss->GetInputs().end());
    }
    for (auto pred : block->GetPredsBlocks()) {
        CheckSaveStateOsrRec(inst, user, pred, visited);
    }
}

/*
 * Visitors to check instructions types
 */
void GraphChecker::VisitReturn([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    [[maybe_unused]] auto op = inst->GetInputs()[0].GetInst();
    ASSERT_DO(CheckCommonTypes(inst, op), (std::cerr << "Types of return and its input are not compatible\n return:\n",
                                           inst->Dump(&std::cerr), std::cerr << "\n input:\n", op->Dump(&std::cerr)));
    [[maybe_unused]] auto num_succs = inst->GetBasicBlock()->GetSuccsBlocks().size();
    ASSERT_PRINT(num_succs == 1, "Basic block with Return must have 1 successor");
    [[maybe_unused]] auto succ = inst->GetBasicBlock()->GetSuccsBlocks()[0];
    ASSERT_DO(succ->IsEndBlock() || succ->IsTryEnd(),
              std::cerr << "Basic block with Return must have end or try end block as successor:\n"
                        << *inst << std::endl);
}

void GraphChecker::VisitIf([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    [[maybe_unused]] auto num_succs = inst->GetBasicBlock()->GetSuccsBlocks().size();
    ASSERT_PRINT(num_succs == MAX_SUCCS_NUM, "Basic block with If must have 2 successesors");

    [[maybe_unused]] auto op1 = inst->GetInputs()[0].GetInst();
    [[maybe_unused]] auto op2 = inst->GetInputs()[1].GetInst();
    for (size_t i = 0; i < inst->GetInputsCount(); i++) {
        ASSERT_DO(inst->GetInputType(i) != DataType::NO_TYPE,
                  std::cerr << "Source operand type is not set: " << *inst << std::endl);
    }
    ASSERT_DO(inst->GetInputType(0) == inst->GetInputType(1),
              std::cerr << "If has different inputs type: " << *inst << std::endl);
    if (inst->GetInputType(0) == DataType::REFERENCE) {
        [[maybe_unused]] auto cc = inst->CastToIf()->GetCc();
        ASSERT_DO(cc == ConditionCode::CC_NE || cc == ConditionCode::CC_EQ,
                  (std::cerr << "Reference comparison in If must be CC_NE or CC_EQ: \n", inst->Dump(&std::cerr)));
        if (op1->IsConst()) {
            ASSERT_DO(IsZeroConstant(op1), (std::cerr << "Constant reference input must be integer 0: \n",
                                            inst->Dump(&std::cerr), op1->Dump(&std::cerr)));
        } else {
            ASSERT_DO(op1->GetType() == DataType::REFERENCE, (std::cerr << "If 1st operand type is not a reference\n",
                                                              inst->Dump(&std::cerr), op1->Dump(&std::cerr)));
        }
        if (op2->IsConst()) {
            ASSERT_DO(IsZeroConstant(op2), (std::cerr << "Constant reference input must be integer 0: \n",
                                            inst->Dump(&std::cerr), op2->Dump(&std::cerr)));
        } else {
            ASSERT_DO(op2->GetType() == DataType::REFERENCE, (std::cerr << "If 2nd operand type is not a reference\n",
                                                              inst->Dump(&std::cerr), op2->Dump(&std::cerr)));
        }
    }
}

void GraphChecker::VisitIfImm([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    [[maybe_unused]] auto num_succs = inst->GetBasicBlock()->GetSuccsBlocks().size();
    ASSERT_PRINT(num_succs == MAX_SUCCS_NUM, "Basic block with IfImm must have 2 successesors");

    [[maybe_unused]] auto op1 = inst->GetInput(0).GetInst();
    [[maybe_unused]] auto op2 = inst->CastToIfImm()->GetImm();
    ASSERT_DO(inst->GetInputType(0) != DataType::NO_TYPE,
              std::cerr << "Source operand type is not set: " << *inst << std::endl);
    if (inst->GetInputType(0) == DataType::REFERENCE) {
        [[maybe_unused]] auto cc = inst->CastToIfImm()->GetCc();
        ASSERT_DO(cc == ConditionCode::CC_NE || cc == ConditionCode::CC_EQ,
                  (std::cerr << "Reference comparison in IfImm must have CC_NE or CC_EQ: \n", inst->Dump(&std::cerr)));
        if (op1->IsConst()) {
            ASSERT_DO(IsZeroConstant(op1), (std::cerr << "Constant reference input must be integer 0: \n",
                                            inst->Dump(&std::cerr), op1->Dump(&std::cerr)));
        } else {
            ASSERT_DO(op1->GetType() == DataType::REFERENCE,
                      (std::cerr << "IfImm operand type should be here a reference: \n", inst->Dump(&std::cerr),
                       op1->Dump(&std::cerr)));
        }
        ASSERT_DO(op2 == 0,
                  (std::cerr << "Reference can be compared only with 0 immediate: \n", inst->Dump(&std::cerr)));
    } else {
        ASSERT_PRINT(
            DataType::GetCommonType(op1->GetType()) == DataType::INT64 ||
                (static_cast<GraphChecker *>(v)->GetGraph()->IsDynamicMethod() && op1->GetType() == DataType::ANY),
            "IfImm operand type should be here an integer");
    }
}
}  // namespace panda::compiler
