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

#include "compiler_options.h"
#include "graph_checker.h"
#include "graph_cloner.h"
#include "graph_checker_macros.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/analysis/rpo.h"
#include "optimizer/analysis/linear_order.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/ir/analysis.h"
#include "optimizer/ir/datatype.h"
#include "optimizer/ir/inst.h"
#include "optimizer/optimizations/cleanup.h"
#include "inst_checker_gen.h"
#include "intrinsic_string_flat_check.inl"

namespace ark::compiler {

GraphChecker::GraphChecker(Graph *graph)
{
    graph_ = graph;
    PreCloneChecks(graph);
    graph_ = GraphCloner(graph, GetAllocator(), GetLocalAllocator()).CloneGraph();
    GetGraph()->GetPassManager()->SetCheckMode(true);
    passName_ = std::string("");
}

GraphChecker::GraphChecker(Graph *graph, const char *passName)
{
    graph_ = graph;
    PreCloneChecks(graph);
    graph_ = GraphCloner(graph, GetAllocator(), GetLocalAllocator()).CloneGraph();
    auto graphTmp = GetGraph();
    ASSERT(graphTmp != nullptr);
    graphTmp->GetPassManager()->SetCheckMode(true);
    passName_ = std::string(passName);
    ASSERT(passName != nullptr);
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
            CHECKER_IF_NOT_PRINT(u == nullptr || u->GetPrev() == nullptr);
            while (u != nullptr) {
                CHECKER_IF_NOT_PRINT(u->GetNext() == nullptr || u->GetNext()->GetPrev() == u);
                u = u->GetNext();
            }
            for (auto &user : inst->GetUsers()) {
                [[maybe_unused]] auto userInst = user.GetInst();
                CHECKER_IF_NOT_PRINT(userInst->GetBasicBlock() != nullptr);
                CHECKER_DO_IF_NOT_AND_PRINT(CheckInstHasInput(userInst, inst),
                                            std::cerr << "Instruction is not an input to its user\n"
                                                      << "input: " << *inst << std::endl
                                                      << "user:  " << *userInst << std::endl);
            }
            for (auto &input : inst->GetInputs()) {
                [[maybe_unused]] auto inputInst = input.GetInst();
                CHECKER_DO_IF_NOT_AND_PRINT(inputInst != nullptr && inputInst->GetBasicBlock() != nullptr,
                                            std::cerr << "Instruction has invalid input:\n"
                                                      << "user: " << *inst << std::endl);
                CHECKER_DO_IF_NOT_AND_PRINT(CheckInstHasUser(inputInst, inst),
                                            std::cerr << "Instruction is not a user to its input:\n"
                                                      << "user: " << *inst << std::endl
                                                      << "input:  " << *inputInst << std::endl);
            }
            // Check `require_state` flag
            auto it = std::find_if(inst->GetInputs().begin(), inst->GetInputs().end(),
                                   [](Input input) { return input.GetInst()->IsSaveState(); });
            [[maybe_unused]] bool hasSaveState = (it != inst->GetInputs().end());
            CHECKER_DO_IF_NOT_AND_PRINT(inst->RequireState() == hasSaveState,
                                        std::cerr << "Incorrect 'require_state' flag in the inst: " << *inst);
            if (inst->RequireState()) {
                CHECKER_IF_NOT_PRINT(it->GetInst() == inst->GetSaveState());
            }
            if (inst->IsPhi()) {
                // causes failure in GraphCloner::BuildDataFlow if not checked before clone
                CHECKER_DO_IF_NOT_AND_PRINT(inst->GetInputsCount() == block->GetPredsBlocks().size(),
                                            std::cerr << "Incorrect phi's inputs count: " << inst->GetInputsCount()
                                                      << " inputs, " << block->GetPredsBlocks().size()
                                                      << " pred blocks\n"
                                                      << *inst);
            }
        }
    }
}

bool GraphChecker::Check()
{
    success_ = InstChecker::Run(GetGraph());
    CHECK_STATUS();

#ifdef COMPILER_DEBUG_CHECKS
    if (GetGraph()->IsAnalysisValid<DominatorsTree>()) {
        CheckDomTree();
    } else {
        GetGraph()->RunPass<DominatorsTree>();
    }
    CHECK_STATUS();
    if (GetGraph()->IsAnalysisValid<LoopAnalyzer>()) {
        CheckLoopAnalysis();
    } else {
        GetGraph()->RunPass<LoopAnalyzer>();
    }
    CHECK_STATUS();
    CheckStartBlock();
    CHECK_STATUS();
    CheckEndBlock();
    CHECK_STATUS();
    size_t blocksCount = 0;
    size_t blocksId = -1;
    for (auto block : GetGraph()->GetVectorBlocks()) {
        ++blocksId;
        if (block == nullptr) {
            continue;
        }
        CHECKER_MESSAGE_IF_NOT_AND_PRINT(block->GetGraph() == GetGraph(), "Block linked to incorrect graph");
        CHECKER_MESSAGE_IF_NOT_AND_PRINT(block->GetId() == blocksId,
                                         "Block ID must be equal to its ID in graph vector");
        CheckBlock(block);
        CHECK_STATUS();
        blocksCount++;
    }
    CHECKER_MESSAGE_IF_NOT_AND_PRINT(blocksCount == GetGraph()->GetBlocksRPO().size(), "There is disconnected block");
    CheckLoops();
    CHECK_STATUS();
    // Visit graph to check instructions types
    CheckGraph();
    CHECK_STATUS();
    // Check that call.Inlined and Return.Inlined in correct order
    // and check that savestate has correct link to call.inlined.
    CheckCallReturnInlined();
    CHECK_STATUS();
    if (NeedCheckSaveState()) {
        // Check that objects in stack.
        CheckSaveStateInputs();
        // Check that between savestate and it's runtime call user have not reference insts.
        CheckSaveStatesWithRuntimeCallUsers();
    }
#endif  // COMPILER_DEBUG_CHECKS
    return GetStatus();
}

#ifdef COMPILER_DEBUG_CHECKS
bool GraphChecker::NeedCheckSaveState()
{
    return !GetGraph()->IsBytecodeOptimizer() && GetGraph()->GetParentGraph() == nullptr &&
           GetGraph()->IsInliningComplete();
}
#endif  // COMPILER_DEBUG_CHECKS

void GraphChecker::CheckBlock([[maybe_unused]] BasicBlock *block)
{
#ifdef COMPILER_DEBUG_CHECKS
    CheckControlFlow(block);
    CheckDataFlow(block);
    for (auto phiInst : block->PhiInsts()) {
        CheckPhiInputs(phiInst);
    }
    if (!GetGraph()->IsLowLevelInstructionsEnabled() && !GetGraph()->IsDynamicMethod() && !GetGraph()->IsAbcKit()) {
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
#endif  // COMPILER_DEBUG_CHECKS
}

void GraphChecker::CheckControlFlow(BasicBlock *block)
{
    auto numSuccs = block->GetSuccsBlocks().size();
    CHECKER_MESSAGE_IF_NOT_AND_PRINT(block->IsEndBlock() || block->IsTryBegin() || block->IsTryEnd() ||
                                         (numSuccs > 0 && numSuccs <= MAX_SUCCS_NUM) ||
                                         block->GetLastInst()->GetOpcode() == Opcode::Throw,
                                     "Non-end block and non-try-begin block should have 1 or 2 successesors");

    for ([[maybe_unused]] auto pred : block->GetPredsBlocks()) {
        CHECKER_DO_IF_NOT_AND_PRINT(CheckBlockHasSuccessor(pred, block),
                                    std::cerr << "Block " << block->GetId() << " is not a successor to its predecessor "
                                              << pred->GetId());
    }
    for ([[maybe_unused]] auto succ : block->GetSuccsBlocks()) {
        CHECKER_MESSAGE_IF_NOT_AND_PRINT(CheckBlockHasPredecessor(succ, block),
                                         "Block is not a predecessor to its successor");
    }

    if (numSuccs == MAX_SUCCS_NUM) {
        CHECKER_MESSAGE_IF_NOT_AND_PRINT(block->GetSuccessor(0) != block->GetSuccessor(1),
                                         "Wrong CFG - block with two same successors");
    }
}

void GraphChecker::PrintFailedMethodAndPass() const
{
    auto methodName = GetGraph()->GetRuntime()->GetMethodFullName(GetGraph()->GetMethod(), false);
    std::cerr << std::endl
              << "Failed method: " << methodName << std::endl
              << (!GetPassName().empty() ? ("After pass: " + GetPassName() + "\n") : "");
}
void GraphChecker::PrintFailedMethodAndPassVisitor(GraphVisitor *v)
{
    auto graphChecker = static_cast<GraphChecker *>(v);
    auto methodName =
        graphChecker->GetGraph()->GetRuntime()->GetMethodFullName(graphChecker->GetGraph()->GetMethod(), false);
    std::cerr << std::endl
              << "Failed method: " << methodName << std::endl
              << (!graphChecker->GetPassName().empty() ? ("After pass: " + graphChecker->GetPassName() + "\n") : "");
}

void GraphChecker::CheckInputType(Inst *inst)
{
    // NOTE(mbolshov): Fix types for LiveOut in irtoc
    if (inst->GetOpcode() == Opcode::LiveOut) {
        return;
    }
    // NOTE(dkofanov): Fix get input type for call insts
    if (inst->IsCall()) {
        return;
    }
    for (size_t i = 0; i < inst->GetInputsCount(); ++i) {
        [[maybe_unused]] auto input = inst->GetInput(i).GetInst();
        // NOTE(mbolshov): Fix types for LiveIn in irtoc
        if (input->GetOpcode() == Opcode::LiveIn) {
            continue;
        }
        [[maybe_unused]] auto inputType = GetCommonType(inst->GetInputType(i));
        [[maybe_unused]] auto realInputType = GetCommonType(input->GetType());
        if (GetGraph()->IsAbcKit()) {
            if (inputType == DataType::ANY && input->IsConst() && (inst->IsIntrinsic() || inst->IsPhi())) {
                continue;
            }
        }
        CHECKER_DO_IF_NOT_AND_PRINT(
            inputType == realInputType ||
                (realInputType == DataType::ANY &&
                 (inputType == DataType::REFERENCE || inputType == DataType::POINTER)) ||
                (IsZeroConstant(input) && (inputType == DataType::REFERENCE || inputType == DataType::POINTER)) ||
                (inputType == DataType::ANY && input->IsConst() && realInputType == DataType::INT64) ||
                (GetGraph()->IsThrowApplied() &&
                 (input->GetBasicBlock()->IsEndWithThrow() ||
                  ((input->GetOpcode() == Opcode::CatchPhi || input->GetOpcode() == Opcode::Phi) &&
                   (inst->GetOpcode() == Opcode::Phi || inst->GetOpcode() == Opcode::CatchPhi)))),
            (std::cerr << "Input type don't equal to real input type\n"
                       << "inst: " << *inst << std::endl
                       << "input type: " << DataType::ToString(inputType) << std::endl
                       << "input: " << *input << std::endl,
             GetGraph()->Dump(&std::cerr)));
    }
}

uint32_t GetInliningDepth(Inst *inst)
{
    uint32_t inliningDepth = 0;
    auto ss = inst->IsSaveState() ? inst : inst->GetSaveState();
    while (ss != nullptr) {
        auto caller = static_cast<SaveStateInst *>(ss)->GetCallerInst();
        if (caller == nullptr) {
            break;
        }
        ss = caller->GetSaveState();
        inliningDepth++;
    }
    return inliningDepth;
}

void GraphChecker::CheckUserOfInt32([[maybe_unused]] BasicBlock *block, Inst *inst, User &user)
{
    ASSERT(DataType::Is32Bits(inst->GetType(), GetGraph()->GetArch()));
    auto *userInst = user.GetInst();
    if (!userInst->HasType()) {
        return;
    }
    auto arch = GetGraph()->GetArch();
    // Unsigned Load in AARCH64 zerod all high bits
#ifndef NDEBUG
    if (inst->IsLoad() && !DataType::IsTypeSigned(inst->GetType()) && arch == Arch::AARCH64 &&
        GetGraph()->IsLowLevelInstructionsEnabled()) {
#else
    if (inst->IsLoad() && !DataType::IsTypeSigned(inst->GetType()) && arch == Arch::AARCH64) {
#endif  // !NDEBUG
        return;
    }
    [[maybe_unused]] auto userInputType = userInst->GetInputType(user.GetIndex());
    [[maybe_unused]] bool refToPtr = userInputType == DataType::POINTER && inst->GetType() == DataType::REFERENCE;
    CHECKER_DO_IF_NOT_AND_PRINT(DataType::Is32Bits(userInputType, arch) || refToPtr ||
                                    (block->GetGraph()->IsDynamicMethod() && userInputType == DataType::ANY),
                                std::cerr << "Undefined high-part of input instruction for its user\n"
                                          << "input: " << *inst << std::endl
                                          << "user:  " << *userInst << std::endl);
}

void GraphChecker::CheckInstUsers(Inst *inst, [[maybe_unused]] BasicBlock *block)
{
    for ([[maybe_unused]] auto &user : inst->GetUsers()) {
        auto userInst = user.GetInst();
        CHECKER_DO_IF_NOT_AND_PRINT(CheckInstHasInput(userInst, inst),
                                    (std::cerr << "Instruction is not an input to its user\n"
                                               << "input: " << *inst << std::endl
                                               << "user:  " << *userInst << std::endl,
                                     GetGraph()->Dump(&std::cerr)));
        if (!userInst->IsPhi() && !userInst->IsCatchPhi()) {
            CHECKER_DO_IF_NOT_AND_PRINT(
                inst->IsDominate(userInst) || ((GetGraph()->IsRegAllocApplied() || GetGraph()->IsThrowApplied()) &&
                                               IsTryCatchDomination(inst->GetBasicBlock(), userInst->GetBasicBlock())),
                (std::cerr << "Instruction doesn't dominate its user\n"
                           << "input: bb " << inst->GetBasicBlock()->GetId() << *inst << std::endl
                           << "user: bb " << userInst->GetBasicBlock()->GetId() << *userInst << std::endl,
                 GetGraph()->Dump(&std::cerr)));
        }
        if (DataType::Is32Bits(inst->GetType(), GetGraph()->GetArch())) {
            CheckUserOfInt32(block, inst, user);
        }
    }
}

void GraphChecker::CheckDataFlow(BasicBlock *block)
{
    for (auto inst : block->AllInsts()) {
        CHECKER_DO_IF_NOT_AND_PRINT(inst->GetBasicBlock() == block,
                                    std::cerr << "Instruction block's pointer isn't correct" << *inst << std::endl);
        if (block != GetGraph()->GetStartBlock()) {
            CHECKER_DO_IF_NOT_AND_PRINT(inst->GetOpcode() != Opcode::Parameter,
                                        std::cerr << "Not entry block can't contain Parameter instructions" << *inst
                                                  << std::endl);
        }
        if (inst->GetPrev() == nullptr) {
            CHECKER_MESSAGE_IF_NOT_AND_PRINT(*block->AllInsts().begin() == inst,
                                             "First block instruction isn't correct");
        }
        if (inst->GetNext() == nullptr) {
            CHECKER_MESSAGE_IF_NOT_AND_PRINT(*block->AllInstsSafeReverse().begin() == inst,
                                             "Last block instruction isn't correct");
        }
        CheckInputType(inst);

#ifndef NDEBUG
        if (GetGraph()->IsInliningComplete() && GetGraph()->GetParentGraph() == nullptr) {
            // Check depth
            CHECKER_DO_IF_NOT_AND_PRINT(inst->GetInliningDepth() == GetInliningDepth(inst),
                                        std::cerr << *inst << std::endl
                                                  << "Current depth = " << inst->GetInliningDepth() << std::endl
                                                  << "Correct depth = " << GetInliningDepth(inst) << std::endl);
        }
#endif
        CheckInstUsers(inst, block);

        for ([[maybe_unused]] auto input : inst->GetInputs()) {
            CHECKER_DO_IF_NOT_AND_PRINT(CheckInstHasUser(input.GetInst(), inst),
                                        std::cerr << "Instruction is not a user to its input:\n"
                                                  << "input: " << *input.GetInst() << std::endl
                                                  << "user:  " << *inst << std::endl);
        }
    }
}

void GraphChecker::CheckCallReturnInlined()
{
    [[maybe_unused]] bool throwExit = false;
    if (GetGraph()->HasEndBlock()) {
        for (auto block : GetGraph()->GetEndBlock()->GetPredsBlocks()) {
            if (block->IsTryEnd()) {
                continue;
            }
            if (block->IsEndWithThrowOrDeoptimize()) {
                throwExit = true;
                break;
            }
        }
    }
    ArenaStack<Inst *> inlinedCalles(GetLocalAllocator()->Adapter());
    for (auto block : GetGraph()->GetBlocksRPO()) {
        for (auto inst : block->Insts()) {
            if (inst->IsCall() && static_cast<CallInst *>(inst)->IsInlined()) {
                CHECKER_MESSAGE_IF_NOT_AND_PRINT(inst->NoDest(), "Inlined call should have NO_DST flag");
                inlinedCalles.push(inst);
            } else if (inst->GetOpcode() == Opcode::ReturnInlined && block->IsEndWithThrowOrDeoptimize()) {
                // NOTE(Sergey Chernykh) fix checker
                continue;
            } else if (inst->GetOpcode() == Opcode::ReturnInlined) {
                CHECKER_IF_NOT_PRINT(!inlinedCalles.empty());
                ASSERT(inlinedCalles.top()->GetSaveState() == inst->GetSaveState() || throwExit ||
                       GetGraph()->IsRegAllocApplied());
                inlinedCalles.pop();
            }
        }
    }
    CHECKER_IF_NOT_PRINT(inlinedCalles.empty() || throwExit);
#ifndef NDEBUG
    // avoid check after ir_builder in inline pass
    if (!GetGraph()->IsInliningComplete() || GetGraph()->GetParentGraph() != nullptr) {
        return;
    }
    for (auto block : GetGraph()->GetBlocksRPO()) {
        for (auto inst : block->Insts()) {
            if (inst->IsSaveState()) {
                CheckSaveStateCaller(static_cast<SaveStateInst *>(inst));
            }
        }
    }
#endif
}

bool GraphChecker::FindCaller([[maybe_unused]] Inst *caller, BasicBlock *domBlock, ArenaStack<Inst *> *inlinedCalls)
{
    for (auto inst : domBlock->InstsSafeReverse()) {
        if (inst->GetOpcode() == Opcode::ReturnInlined) {
            inlinedCalls->push(inst);
        } else if (inst->IsCall() && static_cast<CallInst *>(inst)->IsInlined()) {
            if (!inlinedCalls->empty()) {
                inlinedCalls->pop();
            } else {
                CHECKER_IF_NOT_PRINT(caller == inst);
                return true;
            }
        }
    }
    return false;
}

void GraphChecker::CheckSaveStateCaller(SaveStateInst *savestate)
{
    CHECKER_IF_NOT_PRINT(savestate != nullptr);
    auto block = savestate->GetBasicBlock();
    ArenaStack<Inst *> inlinedCalls(GetLocalAllocator()->Adapter());
    auto caller = savestate->GetCallerInst();
    if (caller == nullptr) {
        return;
    }
    CHECKER_IF_NOT_PRINT(caller->GetBasicBlock() != nullptr);
    CHECKER_IF_NOT_PRINT(caller->GetBasicBlock()->GetGraph() == block->GetGraph());
    ASSERT(caller->IsInlined());
    auto domBlock = block;
    bool skip = true;
    for (auto inst : domBlock->InstsSafeReverse()) {
        if (inst == savestate) {
            skip = false;
        }
        if (skip) {
            continue;
        }
        if (inst->GetOpcode() == Opcode::ReturnInlined) {
            inlinedCalls.push(inst);
        } else if (inst->IsCall() && static_cast<CallInst *>(inst)->IsInlined()) {
            if (!inlinedCalls.empty()) {
                inlinedCalls.pop();
            } else {
                CHECKER_IF_NOT_PRINT(caller == inst);
                return;
            }
        }
    }
    domBlock = domBlock->GetDominator();
    while (domBlock != nullptr) {
        if (FindCaller(caller, domBlock, &inlinedCalls)) {
            return;
        }
        domBlock = domBlock->GetDominator();
    }
    UNREACHABLE();
}

void GraphChecker::CheckStartBlock()
{
    [[maybe_unused]] Inst *hasNullptr = nullptr;
    [[maybe_unused]] int32_t lastNum = -2;
    CHECKER_IF_NOT_PRINT(GetGraph()->GetStartBlock());
    CHECKER_MESSAGE_IF_NOT_AND_PRINT(GetGraph()->GetStartBlock()->GetPredsBlocks().empty(),
                                     "Start block can't have predecessors");
    CHECKER_MESSAGE_IF_NOT_AND_PRINT(GetGraph()->GetStartBlock()->GetSuccsBlocks().size() == 1,
                                     "Start block should have one successor");
    for (auto inst : GetGraph()->GetStartBlock()->AllInsts()) {
        [[maybe_unused]] Opcode opc = inst->GetOpcode();
        CHECKER_DO_IF_NOT_AND_PRINT(
            opc == Opcode::Constant || opc == Opcode::Parameter || opc == Opcode::SafePoint ||
                opc == Opcode::SpillFill || opc == Opcode::NullPtr || opc == Opcode::LoadUniqueObject ||
                opc == Opcode::NOP || opc == Opcode::LiveIn,
            std::cerr
                << "Entry block can contain Constant, Parameter, NullPtr, SafePoint, NOP or SpillFill instructions"
                << *inst << std::endl);
        if (opc == Opcode::Parameter) {
            auto argNum = inst->CastToParameter()->GetArgNumber();
            auto num = static_cast<int32_t>(argNum);
            if (argNum == ParameterInst::DYNAMIC_NUM_ARGS) {
                num = -1;
            }
            CHECKER_DO_IF_NOT_AND_PRINT(
                lastNum < num,
                std::cerr << "The argument number in the parameter must be greater than that of the previous parameter"
                          << *inst << std::endl);
            lastNum = num;
        }
        if (opc == Opcode::NullPtr) {
            CHECKER_MESSAGE_IF_NOT_AND_PRINT(hasNullptr == nullptr,
                                             "There should be not more than one NullPtr instruction");
            hasNullptr = inst;
        }
    }
}

void GraphChecker::CheckEndBlock()
{
    if (!GetGraph()->HasEndBlock()) {
        CHECKER_MESSAGE_IF_NOT_AND_PRINT(HasOuterInfiniteLoop(), "Graph without infinite loops should have end block");
        return;
    }
    CHECKER_MESSAGE_IF_NOT_AND_PRINT(GetGraph()->GetEndBlock()->GetSuccsBlocks().empty(),
                                     "End block can't have successors");
    [[maybe_unused]] auto iter = GetGraph()->GetEndBlock()->Insts();
    CHECKER_MESSAGE_IF_NOT_AND_PRINT(iter.begin() == iter.end(), "End block can't have instructions");
}

void GraphChecker::CheckGraph()
{
    size_t numInst = GetGraph()->GetCurrentInstructionId();
    ArenaVector<bool> instVec(numInst, GetLocalAllocator()->Adapter());
    for (auto &bb : GetGraph()->GetBlocksRPO()) {
        for (auto inst : bb->AllInsts()) {
            auto id = inst->GetId();
            CHECKER_DO_IF_NOT_AND_PRINT(
                id < numInst,
                (std::cerr << "Instruction ID must be less than graph instruction counter: " << numInst << "\n",
                 inst->Dump(&std::cerr)));
            CHECKER_DO_IF_NOT_AND_PRINT(
                !instVec[id], (std::cerr << "Instruction with same Id already exists:\n", inst->Dump(&std::cerr)));
            instVec[id] = true;
            CHECKER_DO_IF_NOT_AND_PRINT(
                GetGraph()->IsDynamicMethod() || inst->GetType() != DataType::ANY,
                (std::cerr << "The type ANY is supported only for dynamic languages\n", inst->Dump(&std::cerr)));
#ifndef NDEBUG
            CHECKER_DO_IF_NOT_AND_PRINT(inst->SupportsMode(GetGraph()->GetCompilerMode()),
                                        (std::cerr << "Instruction used in wrong mode\n", inst->Dump(&std::cerr)));
#endif
            VisitInstruction(inst);
        }
    }
}

void GraphChecker::CheckPhiInputs(Inst *phiInst)
{
    for (size_t index = 0; index < phiInst->GetInputsCount(); ++index) {
        [[maybe_unused]] auto pred = phiInst->CastToPhi()->GetPhiInputBb(index);
        [[maybe_unused]] auto inputBb = phiInst->CastToPhi()->GetPhiInput(pred)->GetBasicBlock();
        CHECKER_DO_IF_NOT_AND_PRINT(
            inputBb->IsDominate(pred) || ((GetGraph()->IsRegAccAllocApplied() || GetGraph()->IsThrowApplied()) &&
                                          IsTryCatchDomination(inputBb, pred)),
            (std::cerr
                 << "Block where phi-input is located should dominate predecessor block corresponding to this input\n"
                 << "Block inputBb " << inputBb->GetId() << " should dominate pred " << pred->GetId() << std::endl
                 << "phiInst " << *phiInst << " from BB " << phiInst->GetBasicBlock()->GetId() << std::endl,
             std::cerr << "inputBb ", inputBb->Dump(&std::cerr), std::cerr << "pred ", pred->Dump(&std::cerr),
             std::cerr << "phiBb ", phiInst->GetBasicBlock()->Dump(&std::cerr), GetGraph()->Dump(&std::cerr)));
    }
}

bool GraphChecker::CheckInstRegUsageSaved(const Inst *inst, Register reg) const
{
    if (reg == GetAccReg()) {
        return true;
    }
    auto graph = inst->GetBasicBlock()->GetGraph();
    // Empty vector regs mask means we are using dynamic general regs set.
    if (DataType::IsFloatType(inst->GetType()) && !graph->GetUsedRegs<DataType::FLOAT64>()->empty()) {
        return graph->GetUsedRegs<DataType::FLOAT64>()->at(reg);
    }
    return graph->GetUsedRegs<DataType::INT64>()->at(reg);
}

[[maybe_unused]] static bool CheckSpillFillMultiple(const compiler::Inst *inst)
{
    switch (inst->GetOpcode()) {
        case Opcode::Parameter:
            return false;
        case Opcode::LoadObject:
        case Opcode::NewArray:
        case Opcode::NewObject:
            // In this case for BytecodeOptimizer SpillFill will be added after instruction, not before
            // user-insturction. So this check can't find it and it is skipped.
            return !inst->GetBasicBlock()->GetGraph()->IsBytecodeOptimizer();
        default:
            return true;
    }
}

void GraphChecker::CheckNoLowLevel(BasicBlock *block)
{
    for ([[maybe_unused]] auto inst : block->Insts()) {
        CHECKER_DO_IF_NOT_AND_PRINT(!inst->IsLowLevel(), inst->Dump(&std::cerr));
    }
}

void GraphChecker::MarkBlocksInLoop(Loop *loop, Marker mrk)
{
    CHECKER_IF_NOT_PRINT(loop->IsIrreducible() || loop->IsRoot() || loop->GetHeader() != nullptr);
    CHECKER_IF_NOT_PRINT(loop->IsIrreducible() || loop->IsRoot() || loop->IsTryCatchLoop() ||
                         loop->GetPreHeader() != nullptr);
    // Mark blocks and check if marker was not set before
    for ([[maybe_unused]] auto block : loop->GetBlocks()) {
        CHECKER_IF_NOT_PRINT(!block->SetMarker(mrk));
    }

    for (auto inner : loop->GetInnerLoops()) {
        MarkBlocksInLoop(inner, mrk);
    }
}

bool GraphChecker::CheckBlockHasPredecessor(BasicBlock *block, BasicBlock *predecessor)
{
    CHECKER_IF_NOT_PRINT(block != nullptr && predecessor != nullptr);
    for (auto pred : block->GetPredsBlocks()) {
        if (pred == predecessor) {
            return true;
        }
    }
    return false;
}

bool GraphChecker::CheckBlockHasSuccessor(BasicBlock *block, BasicBlock *successor)
{
    CHECKER_IF_NOT_PRINT(block != nullptr && successor != nullptr);
    for (auto succ : block->GetSuccsBlocks()) {
        if (succ == successor) {
            return true;
        }
    }
    return false;
}

bool GraphChecker::BlockContainsInstruction(BasicBlock *block, Opcode opcode)
{
    return std::find_if(block->Insts().begin(), block->Insts().end(),
                        [opcode](Inst *inst) { return inst->GetOpcode() == opcode; }) != block->Insts().end();
}

void GraphChecker::CheckLoopHasSafePoint(Loop *loop)
{
    [[maybe_unused]] auto it =
        std::find_if(loop->GetBlocks().begin(), loop->GetBlocks().end(),
                     [this](BasicBlock *block) { return BlockContainsInstruction(block, Opcode::SafePoint); });
    // Irreducible isn't fully populated - only 'one of the headers' and back-edge,
    // SafePoint can be inserted to the another 'header' and search will be failed
    CHECKER_DO_IF_NOT_AND_PRINT(loop->IsTryCatchLoop() || loop->IsIrreducible() || it != loop->GetBlocks().end(),
                                std::cerr << "Loop " << loop->GetId() << " must have safepoint\n");
    for (auto inner : loop->GetInnerLoops()) {
        CheckLoopHasSafePoint(inner);
    }
}

void GraphChecker::CheckLoops()
{
    CHECKER_IF_NOT_PRINT(GetGraph()->GetAnalysis<LoopAnalyzer>().IsValid());
    CHECKER_IF_NOT_PRINT(GetGraph()->GetRootLoop() != nullptr);
    CHECKER_IF_NOT_PRINT(GetGraph()->GetRootLoop()->IsRoot());
    CHECKER_IF_NOT_PRINT(GetGraph()->GetRootLoop()->GetHeader() == nullptr);
    CHECKER_IF_NOT_PRINT(GetGraph()->GetRootLoop()->GetPreHeader() == nullptr);
    auto rootLoop = GetGraph()->GetRootLoop();
    auto mrk = GetGraph()->NewMarker();
    MarkBlocksInLoop(rootLoop, mrk);

    for ([[maybe_unused]] auto block : GetGraph()->GetBlocksRPO()) {
        [[maybe_unused]] auto loop = block->GetLoop();
        CHECKER_IF_NOT_PRINT(loop != nullptr);
        CHECKER_IF_NOT_PRINT(block->IsMarked(mrk));
        if (!block->IsLoopHeader()) {
            CHECKER_IF_NOT_PRINT(!block->IsOsrEntry());
            if (block->IsLoopPreHeader()) {
                CHECKER_IF_NOT_PRINT(block->GetNextLoop()->GetPreHeader() == block);
            }
            continue;
        }
        CHECKER_IF_NOT_PRINT(loop->IsTryCatchLoop() || loop->IsIrreducible() ||
                             loop->GetPreHeader()->GetNextLoop() == loop);
        if (block->IsOsrEntry()) {
            CHECKER_IF_NOT_PRINT(GetGraph()->IsOsrMode());
            auto ssOsr = block->GetFirstInst();
            while (ssOsr != nullptr && (ssOsr->IsCatchPhi() || ssOsr->GetOpcode() == Opcode::Try)) {
                ssOsr = ssOsr->GetNext();
            }
            CHECKER_IF_NOT_PRINT(ssOsr != nullptr && ssOsr->GetOpcode() == Opcode::SaveStateOsr);
        }
        [[maybe_unused]] auto preds = block->GetPredsBlocks();
        for ([[maybe_unused]] auto pred : preds) {
            CHECKER_IF_NOT_PRINT(pred->GetLoop() != loop || loop->HasBackEdge(pred));
        }

        if (!loop->IsIrreducible()) {
            for ([[maybe_unused]] auto back : loop->GetBackEdges()) {
                CHECKER_IF_NOT_PRINT(std::find(preds.begin(), preds.end(), back) != preds.end());
            }
        }
    }
    GetGraph()->EraseMarker(mrk);
    if (g_options.IsCompilerUseSafepoint() && GetGraph()->SupportManagedCode()) {
        for (auto inner : rootLoop->GetInnerLoops()) {
            CheckLoopHasSafePoint(inner);
        }
    }
}

void GraphChecker::CheckDomTree()
{
    CHECKER_IF_NOT_PRINT(GetGraph()->GetAnalysis<DominatorsTree>().IsValid());
    ArenaVector<BasicBlock *> dominators(GetGraph()->GetVectorBlocks().size(), GetLocalAllocator()->Adapter());
    for (auto block : GetGraph()->GetBlocksRPO()) {
        dominators[block->GetId()] = block->GetDominator();
    }
    // Rebuild dom-tree
    GetGraph()->InvalidateAnalysis<DominatorsTree>();
    GetGraph()->RunPass<DominatorsTree>();

    for ([[maybe_unused]] auto block : GetGraph()->GetBlocksRPO()) {
        CHECKER_DO_IF_NOT_AND_PRINT(
            dominators[block->GetId()] == block->GetDominator(),
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
    [[maybe_unused]] auto rootLoop = GetGraph()->GetRootLoop();
    for (auto block : GetGraph()->GetBlocksRPO()) {
        if (block->IsLoopHeader()) {
            loops.emplace(block, block->GetLoop());
        }
    }
    for (auto block : GetGraph()->GetBlocksRPO()) {
        if (block->GetSuccsBlocks().size() != 2U) {
            continue;
        }
        [[maybe_unused]] auto loop = block->GetLoop();
        CHECKER_DO_IF_NOT_AND_PRINT(loop->IsIrreducible() || block->GetFalseSuccessor()->GetLoop() == loop ||
                                        block->GetFalseSuccessor()->GetLoop()->GetOuterLoop() == loop ||
                                        block->GetTrueSuccessor()->GetLoop() == loop ||
                                        block->GetTrueSuccessor()->GetLoop()->GetOuterLoop() == loop,
                                    std::cerr << "One of successors of loop exit basic block id " << block->GetId()
                                              << " must be in same or inner loop" << std::endl);
    }

    // Build new loop info and compare with saved one
    GetGraph()->InvalidateAnalysis<LoopAnalyzer>();
    GetGraph()->RunPass<LoopAnalyzer>();
    CHECKER_MESSAGE_IF_NOT_AND_PRINT(*rootLoop == *GetGraph()->GetRootLoop(), "Root loop is incorrect\n");
    for (auto &[block, loop] : loops) {
        auto expectedLoop = block->GetLoop();
        // An irreducible loop can have different heads, depending on the order of traversal
        if (loop->IsIrreducible()) {
            CHECKER_IF_NOT_PRINT(expectedLoop->IsIrreducible());
            continue;
        }
        CHECKER_IF_NOT_PRINT(block->IsLoopHeader());
        if (loop == nullptr || expectedLoop == nullptr) {
            UNREACHABLE();
            return;
        }
        CHECKER_DO_IF_NOT_AND_PRINT(*loop == *expectedLoop, std::cerr << "Loop " << loop->GetId() << " is incorrect\n");
    }
}

/// Check that there is root's inner loop without exit-points
bool GraphChecker::HasOuterInfiniteLoop()
{
    const auto &loops = GetGraph()->GetRootLoop()->GetInnerLoops();
    return std::find_if(loops.begin(), loops.end(), [](const Loop *loop) { return loop->IsInfinite(); }) != loops.end();
}

bool GraphChecker::CheckInstHasInput(Inst *inst, Inst *input)
{
    CHECKER_IF_NOT_PRINT(inst != nullptr && input != nullptr);
    CHECKER_IF_NOT_PRINT(input->GetBasicBlock() != nullptr);
    CHECKER_IF_NOT_PRINT(input->GetBasicBlock()->GetGraph() != nullptr);
    for (auto node : inst->GetInputs()) {
        if (node.GetInst() == input) {
            return true;
        }
    }
    return false;
}

bool GraphChecker::CheckInstHasUser(Inst *inst, Inst *user)
{
    CHECKER_IF_NOT_PRINT(inst != nullptr && user != nullptr);
    CHECKER_IF_NOT_PRINT(user->GetBasicBlock() != nullptr);
    CHECKER_IF_NOT_PRINT(user->GetBasicBlock()->GetGraph() != nullptr);
    for (auto &node : inst->GetUsers()) {
        if (node.GetInst() == user) {
            return true;
        }
    }
    return false;
}

void GraphChecker::CheckBlockEdges(const BasicBlock &block)
{
    [[maybe_unused]] auto lastInstInBlock = block.GetLastInst();
    if (block.GetSuccsBlocks().size() > 1) {
        CHECKER_DO_IF_NOT_AND_PRINT(
            !block.IsEmpty() || block.IsTryEnd(),
            (std::cerr << "Block with 2 successors have no instructions or should be try-end" << std::endl,
             GetGraph()->Dump(&std::cerr)));
        CHECKER_DO_IF_NOT_AND_PRINT(
            block.IsTryBegin() || block.IsTryEnd() || lastInstInBlock->IsControlFlow(),
            (std::cerr << "Last instruction must be control flow in block with 2 successors" << std::endl,
             GetGraph()->Dump(&std::cerr)));
    } else if (block.GetSuccsBlocks().size() == 1) {
        if (block.GetSuccsBlocks()[0]->IsEndBlock()) {
            if (block.IsEmpty()) {
                CHECKER_IF_NOT_PRINT(block.IsTryEnd());
                return;
            }
            auto lastInst = block.GetLastInst();
            [[maybe_unused]] auto opc = lastInst->GetOpcode();
            CHECKER_MESSAGE_IF_NOT_AND_PRINT(
                lastInst->GetFlag(inst_flags::TERMINATOR),
                "Last instruction in block before exit-block must be Return or Throw instruction.");
        }
    }
}

void GraphChecker::CheckTryBeginBlock(const BasicBlock &block)
{
    CHECKER_IF_NOT_PRINT(block.IsTryBegin());
    auto tryInstIt = std::find_if(block.AllInsts().begin(), block.AllInsts().end(),
                                  [](Inst *inst) { return inst->GetOpcode() == Opcode::Try; });
    CHECKER_MESSAGE_IF_NOT_AND_PRINT(tryInstIt != block.AllInsts().end(),
                                     "Try-begin basic block should contain try-instructions");
    [[maybe_unused]] auto tryInst = (*tryInstIt)->CastToTry();
    for ([[maybe_unused]] auto succIndex : *tryInst->GetCatchEdgeIndexes()) {
        CHECKER_MESSAGE_IF_NOT_AND_PRINT(succIndex < block.GetSuccsBlocks().size(),
                                         "Try instruction holds incorrect try-begin block successor number");
    }
}

void GraphChecker::CheckJump(const BasicBlock &block)
{
    CHECKER_IF_NOT_PRINT(GetGraph()->IsRegAllocApplied());
    CHECKER_IF_NOT_PRINT(GetGraph()->IsAnalysisValid<LinearOrder>());
    if (block.IsIfBlock()) {
        const auto &blocksVector = GetGraph()->GetBlocksLinearOrder();
        auto ifBlockIt = std::find(blocksVector.begin(), blocksVector.end(), &block);
        CHECKER_IF_NOT_PRINT(ifBlockIt != blocksVector.end());
        auto blockAfterIf = std::next(ifBlockIt);
        if (blockAfterIf != blocksVector.end()) {
            CHECKER_MESSAGE_IF_NOT_AND_PRINT(*blockAfterIf != (*ifBlockIt)->GetFalseSuccessor(),
                                             "`If-block` with immediate `false`-successor shouldn't have `JumpFlag`");
            CHECKER_MESSAGE_IF_NOT_AND_PRINT(*blockAfterIf != (*ifBlockIt)->GetTrueSuccessor(),
                                             "`true`-successor should be replaced with `false`-successor");
        }
    }
    [[maybe_unused]] auto numSuccs = block.GetSuccsBlocks().size();
    CHECKER_MESSAGE_IF_NOT_AND_PRINT(numSuccs == 1 || block.IsTryBegin() || block.IsTryEnd() || block.IsIfBlock(),
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
bool GraphChecker::IsTryCatchDomination(const BasicBlock *inputBlock, const BasicBlock *userBlock) const
{
    if (!GetGraph()->IsRegAllocApplied() && !GetGraph()->IsThrowApplied()) {
        return false;
    }
    if (inputBlock->IsTry()) {
        auto blocks = GetGraph()->GetTryBeginBlocks();
        auto it =
            std::find_if(blocks.begin(), blocks.end(), [userBlock](auto &bb) { return bb->IsDominate(userBlock); });
        return it != blocks.end();
    }
    return false;
}

#ifdef COMPILER_DEBUG_CHECKS
static inline bool AllUsersDominate(Inst *inst1, Inst *inst2)
{
    for (auto &user : inst1->GetUsers()) {
        if (!user.GetInst()->IsDominate(inst2)) {
            return false;
        }
    }
    return true;
}
#endif

// It is necessary to check because if reference instruction (it is some object) will contain between SaveState and some
// RuntimeCall user of SaveState, during startup in runtime call GC can be triggered and move our object, so we
// will lost them, his new position (object is not contained in SaveState).
void GraphChecker::CheckSaveStatesWithRuntimeCallUsers()
{
#ifdef COMPILER_DEBUG_CHECKS
    for (auto &block : GetGraph()->GetBlocksRPO()) {
        for (const auto &ss : block->AllInsts()) {
            if (ss->GetOpcode() != Opcode::SaveState) {
                continue;
            }
            CheckSaveStatesWithRuntimeCallUsers(block, ss->CastToSaveState());
        }
    }
#endif
}

void GraphChecker::CheckSaveStatesWithRuntimeCallUsers(BasicBlock *block, SaveStateInst *ss)
{
    for (auto &user : ss->GetUsers()) {
        auto userInst = user.GetInst();
        if (!userInst->IsRuntimeCall() || userInst->IsCheck()) {
            continue;
        }
        ASSERT(userInst->GetBasicBlock() == ss->GetBasicBlock());
        auto it = InstSafeIterator<IterationType::ALL, IterationDirection::BACKWARD>(*block, userInst);
        for (++it; *it != ss; ++it) {
            if ((*it)->GetOpcode() == Opcode::CastAnyTypeValue) {
                continue;
            }
            // Non-reference instructions, checks, nullptr and classes cannot be moved by GC
            // The correct state when all users of Object(is "*it") between SaveState and RuntimeCall(is
            // "user_inst")
            CHECKER_DO_IF_NOT_AND_PRINT(!(*it)->IsMovableObject() || AllUsersDominate(*it, userInst),
                                        std::cerr << "We have inst v" << (*it)->GetId()
                                                  << " which is located between SaveState v" << ss->GetId()
                                                  << " and RuntimeCall v" << userInst->GetId() << ":\n"
                                                  << **it << std::endl
                                                  << *ss << std::endl
                                                  << *userInst << std::endl);
        }
    }
}

#ifdef COMPILER_DEBUG_CHECKS
// If all indirect (through other Phis) inputs of Phi cannot be moved by GC, result of Phi itself
// also cannot be moved (and does not need to be checked in CheckSaveStateInputs as input of its users)
bool GraphChecker::IsPhiSafeToSkipObjectCheck(Inst *inst, Marker visited)
{
    CHECKER_IF_NOT_PRINT(inst->IsPhi());
    CHECKER_IF_NOT_PRINT(visited != UNDEF_MARKER);
    if (inst->SetMarker(visited)) {
        return true;
    }
    for (auto &input : inst->GetInputs()) {
        auto inputInst = input.GetInst();
        if (inputInst->IsPhi()) {
            if (!IsPhiSafeToSkipObjectCheck(inputInst, visited)) {
                return false;
            }
        } else if (inputInst->IsMovableObject() || inputInst->IsCheck()) {
            // Check instructions with reference type do not need to be saved in SaveState themselves,
            // but we need to return false here for them (because their result can be moved by GC)
            return false;
        }
    }
    return true;
}

// If all indirect (through other Phis) users of Phi are SaveState instructions, result of Phi
// is not used in compiled code (and does not need to be checked in CheckSaveStateInputs as user of its inputs)
bool GraphChecker::IsPhiUserSafeToSkipObjectCheck(Inst *inst, Marker visited)
{
    CHECKER_IF_NOT_PRINT(inst->IsPhi());
    CHECKER_IF_NOT_PRINT(visited != UNDEF_MARKER);
    if (inst->SetMarker(visited)) {
        return true;
    }
    for (auto &user : inst->GetUsers()) {
        auto userInst = user.GetInst();
        if (userInst->IsSaveState() || userInst->IsCatchPhi() ||
            (userInst->IsPhi() && IsPhiUserSafeToSkipObjectCheck(userInst, visited))) {
            continue;
        }
        return false;
    }
    return true;
}

// Checks if object is correctly used in SaveStates between it and user
class ObjectSSChecker {
public:
    explicit ObjectSSChecker(Inst *object) : object_(object), visited_(object->GetBasicBlock()->GetGraph()) {}

    bool Run(const Inst *user, const BasicBlock *block, Inst *startFrom)
    {
        if (startFrom != nullptr) {
            auto it = InstSafeIterator<IterationType::ALL, IterationDirection::BACKWARD>(*block, startFrom);
            for (; it != block->AllInstsSafeReverse().end(); ++it) {
                auto inst = *it;
                if (inst == nullptr) {
                    break;
                }
                if (inst->SetMarker(visited_.GetMarker()) || inst == object_ || inst == user) {
                    return true;
                }
                if (!FindAndRemindObjectInSaveState(inst)) {
                    return false;
                }
            }
        }
        for (auto pred : block->GetPredsBlocks()) {
            // Catch-begin block has edge from try-end block, and all try-blocks should be visited from this edge.
            // `object` can be placed inside try-block - after try-begin, so that visiting try-begin is wrong
            if (block->IsCatchBegin() && pred->IsTryBegin()) {
                continue;
            }
            if (!Run(user, pred, pred->GetLastInst())) {
                return false;
            }
        }
        return true;
    }

    Inst *FailedSS()
    {
        return failedSs_;
    }

private:
    static bool FindObjectInSaveState(Inst *object, Inst *ss)
    {
        if (!object->IsMovableObject()) {
            return true;
        }
        while (ss != nullptr && object->IsDominate(ss)) {
            auto it = std::find_if(ss->GetInputs().begin(), ss->GetInputs().end(), [object, ss](Input input) {
                return ss->GetDataFlowInput(input.GetInst()) == object;
            });
            if (it != ss->GetInputs().end()) {
                return true;
            }
            auto caller = static_cast<SaveStateInst *>(ss)->GetCallerInst();
            if (caller == nullptr) {
                break;
            }
            ss = caller->GetSaveState();
        }
        return false;
    }

    bool FindAndRemindObjectInSaveState(Inst *inst)
    {
        if (IsSaveStateForGc(inst) && !FindObjectInSaveState(object_, inst)) {
            failedSs_ = inst;
            return false;
        }
        return true;
    }

    Inst *object_;
    Inst *failedSs_ {};
    MarkerHolder visited_;
};

void GraphChecker::CheckSaveStateInputs(Inst *inst, ArenaVector<User *> *users)
{
    if (!inst->IsMovableObject()) {
        return;
    }
    // true if we do not need check for this input (because it cannot be moved by GC)
    bool skipObjCheck = false;
    if (inst->GetOpcode() == Opcode::Phi) {
        MarkerHolder visited(GetGraph());
        skipObjCheck = IsPhiSafeToSkipObjectCheck(inst, visited.GetMarker());
    }

    PrepareUsers(inst, users);

    ObjectSSChecker objectSSChecker(inst);
    MarkerHolder osrVisited(GetGraph());
    for (auto &it : *users) {
        auto user = it->GetInst();
        if (user->IsCatchPhi()) {
            continue;
        }
        // true if we do not need check for this user (because it is Phi used only in SaveStates)
        bool skipObjCheckUser = false;
        BasicBlock *startBb;
        Inst *startInst;
        if (user->IsPhi()) {
            MarkerHolder visited(GetGraph());
            skipObjCheckUser = IsPhiUserSafeToSkipObjectCheck(user, visited.GetMarker());
            // check SaveStates on path between inst and the end of corresponding predecessor of Phi's block
            startBb = user->GetBasicBlock()->GetPredsBlocks()[it->GetBbNum()];
            startInst = startBb->GetLastInst();
        } else {
            // check SaveStates on path between inst and user
            startBb = user->GetBasicBlock();
            startInst = user->GetPrev();
        }
        CHECKER_DO_IF_NOT_AND_PRINT(skipObjCheck || skipObjCheckUser || objectSSChecker.Run(user, startBb, startInst),
                                    std::cerr << "Object v" << inst->GetId() << " used in v" << user->GetId()
                                              << ", but not found on the path between them in the "
                                              << objectSSChecker.FailedSS()->GetOpcodeStr() << " v"
                                              << objectSSChecker.FailedSS()->GetId() << ":\n"
                                              << *inst << std::endl
                                              << *user << std::endl
                                              << *objectSSChecker.FailedSS() << std::endl);
        CheckSaveStateOsrRec(inst, user, startBb, osrVisited.GetMarker());
    }
    users->clear();
}
#endif

void GraphChecker::CheckSaveStateInputs()
{
#ifdef COMPILER_DEBUG_CHECKS
    ArenaVector<User *> users(GetLocalAllocator()->Adapter());
    for (auto &block : GetGraph()->GetBlocksRPO()) {
        for (const auto &inst : block->AllInsts()) {
            CheckSaveStateInputs(inst, &users);
        }
    }
#endif
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
        CHECKER_IF_NOT_PRINT(GetGraph()->IsOsrMode());
        auto ss = block->GetFirstInst();
        CHECKER_IF_NOT_PRINT(ss != nullptr && ss->GetOpcode() == Opcode::SaveStateOsr);
        [[maybe_unused]] auto it =
            std::find_if(ss->GetInputs().begin(), ss->GetInputs().end(),
                         [inst, ss](Input input) { return ss->GetDataFlowInput(input.GetInst()) == inst; });
        CHECKER_DO_IF_NOT_AND_PRINT(it != ss->GetInputs().end(),
                                    std::cerr << "Inst v" << inst->GetId() << " used in v" << user->GetId()
                                              << ", but not found on the path between them in the "
                                              << ss->GetOpcodeStr() << " v" << ss->GetId() << ":\n"
                                              << *inst << std::endl
                                              << *user << std::endl
                                              << *ss << std::endl);
    }
    for (auto pred : block->GetPredsBlocks()) {
        CheckSaveStateOsrRec(inst, user, pred, visited);
    }
}

/*
 * Visitors to check instructions types
 */
void GraphChecker::VisitMov(GraphVisitor *v, Inst *inst)
{
    CheckUnaryOperationTypes(v, inst);
    UNREACHABLE();
}
void GraphChecker::VisitNeg(GraphVisitor *v, Inst *inst)
{
    CheckUnaryOperationTypes(v, inst);
}
void GraphChecker::VisitAbs(GraphVisitor *v, Inst *inst)
{
    CheckUnaryOperationTypes(v, inst);
}
void GraphChecker::VisitSqrt(GraphVisitor *v, Inst *inst)
{
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, DataType::IsFloatType(inst->GetType()),
                                        (std::cerr << "\nSqrt must have float type\n", inst->Dump(&std::cerr)));
    CheckUnaryOperationTypes(v, inst);
}

void GraphChecker::VisitAddI(GraphVisitor *v, Inst *inst)
{
    if (inst->GetType() == DataType::POINTER) {
        CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
            v, inst->GetInputType(0) == DataType::POINTER || inst->GetInputType(0) == DataType::REFERENCE,
            (std::cerr << "\nptr AddI must have ptr or ref input type\n", inst->Dump(&std::cerr)));
        return;
    }
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, DataType::GetCommonType(inst->GetType()) == DataType::INT64,
                                        (std::cerr << "\nAddI must have integer type\n", inst->Dump(&std::cerr)));
    CheckUnaryOperationTypes(v, inst);
}
void GraphChecker::VisitSubI(GraphVisitor *v, Inst *inst)
{
    if (inst->GetType() == DataType::POINTER) {
        CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
            v, inst->GetInputType(0) == DataType::POINTER || inst->GetInputType(0) == DataType::REFERENCE,
            (std::cerr << "\nptr SubI must have ptr or ref input type\n", inst->Dump(&std::cerr)));
        return;
    }
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, DataType::GetCommonType(inst->GetType()) == DataType::INT64,
                                        (std::cerr << "\nSubI must have integer type\n", inst->Dump(&std::cerr)));
    CheckUnaryOperationTypes(v, inst);
}
void GraphChecker::VisitMulI(GraphVisitor *v, Inst *inst)
{
    [[maybe_unused]] auto type = inst->GetType();
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v,
        DataType::Is32Bits(type, static_cast<GraphChecker *>(v)->GetGraph()->GetArch()) && !DataType::IsReference(type),
        (std::cerr << "\nMulI must have Int32 type\n", inst->Dump(&std::cerr)));
    CheckUnaryOperationTypes(v, inst);
}
void GraphChecker::VisitDivI(GraphVisitor *v, Inst *inst)
{
    [[maybe_unused]] auto type = inst->GetType();
    if (static_cast<GraphChecker *>(v)->GetGraph()->IsBytecodeOptimizer()) {
        CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
            v,
            DataType::Is32Bits(type, static_cast<GraphChecker *>(v)->GetGraph()->GetArch()) &&
                !DataType::IsReference(type),
            (std::cerr << "\nDivI must have Int32 type\n", inst->Dump(&std::cerr)));
    } else {
        CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
            v, !DataType::IsLessInt32(type) && !DataType::IsReference(type),
            (std::cerr << "\nDivI must have at least Int32 type\n", inst->Dump(&std::cerr)));
    }
    CheckUnaryOperationTypes(v, inst);
}
void GraphChecker::VisitModI(GraphVisitor *v, Inst *inst)
{
    [[maybe_unused]] auto type = inst->GetType();
    if (static_cast<GraphChecker *>(v)->GetGraph()->IsBytecodeOptimizer()) {
        CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
            v,
            DataType::Is32Bits(type, static_cast<GraphChecker *>(v)->GetGraph()->GetArch()) &&
                !DataType::IsReference(type),
            (std::cerr << "\nModI must have Int32 type\n", inst->Dump(&std::cerr)));
    } else {
        CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
            v, !DataType::IsLessInt32(type) && !DataType::IsReference(type),
            (std::cerr << "\nModI must have at least Int32 type\n", inst->Dump(&std::cerr)));
    }
    CheckUnaryOperationTypes(v, inst);
}
void GraphChecker::VisitAndI(GraphVisitor *v, Inst *inst)
{
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, DataType::GetCommonType(inst->GetType()) == DataType::INT64,
                                        (std::cerr << "\nAndI must have integer type\n", inst->Dump(&std::cerr)));
    CheckUnaryOperationTypes(v, inst);
}
void GraphChecker::VisitOrI(GraphVisitor *v, Inst *inst)
{
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, DataType::GetCommonType(inst->GetType()) == DataType::INT64,
                                        (std::cerr << "\nOrI must have integer type\n", inst->Dump(&std::cerr)));
    CheckUnaryOperationTypes(v, inst);
}
void GraphChecker::VisitXorI(GraphVisitor *v, Inst *inst)
{
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, DataType::GetCommonType(inst->GetType()) == DataType::INT64,
                                        (std::cerr << "\nXorI must have integer type\n", inst->Dump(&std::cerr)));
    CheckUnaryOperationTypes(v, inst);
}
void GraphChecker::VisitShlI(GraphVisitor *v, Inst *inst)
{
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, DataType::GetCommonType(inst->GetType()) == DataType::INT64,
                                        (std::cerr << "\nShlI must have integer type\n", inst->Dump(&std::cerr)));
    CheckUnaryOperationTypes(v, inst);
    [[maybe_unused]] auto imm = static_cast<BinaryImmOperation *>(inst)->GetImm();
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, imm <= DataType::GetTypeSize(inst->GetType(), static_cast<GraphChecker *>(v)->GetGraph()->GetArch()),
        (std::cerr << "\nShlI have shift more then size of type\n", inst->Dump(&std::cerr)));
}
void GraphChecker::VisitShrI(GraphVisitor *v, Inst *inst)
{
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, DataType::GetCommonType(inst->GetType()) == DataType::INT64,
                                        (std::cerr << "\nShrI must have integer type\n", inst->Dump(&std::cerr)));
    CheckUnaryOperationTypes(v, inst);
    [[maybe_unused]] auto imm = static_cast<BinaryImmOperation *>(inst)->GetImm();
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, imm <= DataType::GetTypeSize(inst->GetType(), static_cast<GraphChecker *>(v)->GetGraph()->GetArch()),
        (std::cerr << "\nShrI have shift more then size of type\n", inst->Dump(&std::cerr)));
}
void GraphChecker::VisitAShlI(GraphVisitor *v, Inst *inst)
{
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, DataType::GetCommonType(inst->GetType()) == DataType::INT64,
                                        (std::cerr << "\nAShrI must have integer type\n", inst->Dump(&std::cerr)));
    CheckUnaryOperationTypes(v, inst);
    [[maybe_unused]] auto imm = static_cast<BinaryImmOperation *>(inst)->GetImm();
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, imm <= DataType::GetTypeSize(inst->GetType(), static_cast<GraphChecker *>(v)->GetGraph()->GetArch()),
        (std::cerr << "\nAShlI have shift more then size of type\n", inst->Dump(&std::cerr)));
}
void GraphChecker::VisitNot(GraphVisitor *v, Inst *inst)
{
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, DataType::GetCommonType(inst->GetType()) == DataType::INT64,
                                        (std::cerr << "\nNot must have integer type\n", inst->Dump(&std::cerr)));
    CheckUnaryOperationTypes(v, inst);
}

// Size does't check after lowering, because for example lowering for Arm64 can remove some casts (ex. u32->u64)
bool IsIntWithPointerSize([[maybe_unused]] DataType::Type type, [[maybe_unused]] Graph *graph)
{
#ifndef NDEBUG
    auto arch = graph->GetArch();
    return DataType::GetCommonType(type) == DataType::INT64 &&
           (DataType::GetTypeSize(type, arch) == DataType::GetTypeSize(DataType::POINTER, arch) ||
            graph->IsLowLevelInstructionsEnabled());
#else
    return true;
#endif
}

void GraphChecker::VisitAdd(GraphVisitor *v, Inst *inst)
{
    auto graph = static_cast<GraphChecker *>(v)->GetGraph();
    if (!graph->SupportManagedCode() && inst->GetType() == DataType::POINTER) {
        [[maybe_unused]] auto type1 = inst->GetInput(0).GetInst()->GetType();
        [[maybe_unused]] auto type2 = inst->GetInput(1).GetInst()->GetType();
        CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
            v, type1 != type2, (std::cerr << "\nptr Add must have ptr and int input types\n", inst->Dump(&std::cerr)));
        CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
            v,
            (type1 == DataType::POINTER && IsIntWithPointerSize(type2, graph)) ||
                (type2 == DataType::POINTER && IsIntWithPointerSize(type1, graph)),
            (std::cerr << "\nptr Add must have ptr and int input types\n", inst->Dump(&std::cerr)));
        return;
    }
    CheckBinaryOperationTypes(v, inst);
}
void GraphChecker::VisitSub(GraphVisitor *v, Inst *inst)
{
    auto graph = static_cast<GraphChecker *>(v)->GetGraph();
    if (!graph->SupportManagedCode()) {
        [[maybe_unused]] auto type1 = inst->GetInput(0).GetInst()->GetType();
        [[maybe_unused]] auto type2 = inst->GetInput(1).GetInst()->GetType();
        if (inst->GetType() == DataType::POINTER) {
            CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
                v, type1 != type2,
                (std::cerr << "\nptr Sub must have ptr and int input types\n", inst->Dump(&std::cerr)));
            CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
                v, (type1 == DataType::POINTER && IsIntWithPointerSize(type2, graph)),
                (std::cerr << "\nptr Sub must have ptr and int input types\n", inst->Dump(&std::cerr)));
            return;
        }
        if (type1 == DataType::POINTER && type2 == DataType::POINTER) {
            CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
                v,
                DataType::GetCommonType(inst->GetType()) == DataType::INT64 &&
                    IsIntWithPointerSize(inst->GetType(), graph),
                (std::cerr << "\n Sub with 2 ptr inputs must have int type\n", inst->Dump(&std::cerr)));
            return;
        }
    }
    CheckBinaryOperationTypes(v, inst);
}
void GraphChecker::VisitMul(GraphVisitor *v, Inst *inst)
{
    CheckBinaryOperationTypes(v, inst);
}
void GraphChecker::VisitDiv(GraphVisitor *v, Inst *inst)
{
    CheckBinaryOperationTypes(v, inst);
}
void GraphChecker::VisitMod(GraphVisitor *v, Inst *inst)
{
    CheckBinaryOperationTypes(v, inst);
}
void GraphChecker::VisitMin(GraphVisitor *v, Inst *inst)
{
    CheckBinaryOperationTypes(v, inst);
}
void GraphChecker::VisitMax(GraphVisitor *v, Inst *inst)
{
    CheckBinaryOperationTypes(v, inst);
}
void GraphChecker::VisitShl(GraphVisitor *v, Inst *inst)
{
    CheckBinaryOperationTypes(v, inst, true);
}
void GraphChecker::VisitShr(GraphVisitor *v, Inst *inst)
{
    CheckBinaryOperationTypes(v, inst, true);
}
void GraphChecker::VisitAShr(GraphVisitor *v, Inst *inst)
{
    CheckBinaryOperationTypes(v, inst, true);
}
void GraphChecker::VisitAnd(GraphVisitor *v, Inst *inst)
{
    CheckBinaryOperationTypes(v, inst, true);
}
void GraphChecker::VisitOr(GraphVisitor *v, Inst *inst)
{
    CheckBinaryOperationTypes(v, inst, true);
}
void GraphChecker::VisitXor(GraphVisitor *v, Inst *inst)
{
    CheckBinaryOperationTypes(v, inst, true);
}

void GraphChecker::VisitAddOverflow(GraphVisitor *v, Inst *inst)
{
    CheckBinaryOverflowOperation(v, inst->CastToAddOverflow());
}
void GraphChecker::VisitSubOverflow(GraphVisitor *v, Inst *inst)
{
    CheckBinaryOverflowOperation(v, inst->CastToSubOverflow());
}
void GraphChecker::VisitLoadArray(GraphVisitor *v, Inst *inst)
{
    CheckMemoryInstruction(v, inst, inst->CastToLoadArray()->GetNeedBarrier());
}
void GraphChecker::VisitLoadArrayI(GraphVisitor *v, Inst *inst)
{
    CheckMemoryInstruction(v, inst, inst->CastToLoadArrayI()->GetNeedBarrier());
}
void GraphChecker::VisitLoadArrayPair(GraphVisitor *v, Inst *inst)
{
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, MemoryCoalescing::AcceptedType(inst->GetType()) || DataType::IsReference(inst->GetType()),
        (std::cerr << "Unallowed type of coalesced load\n", inst->Dump(&std::cerr)));
    CheckMemoryInstruction(v, inst, inst->CastToLoadArrayPair()->GetNeedBarrier());
}
void GraphChecker::VisitLoadObjectPair(GraphVisitor *v, Inst *inst)
{
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, MemoryCoalescing::AcceptedType(inst->GetType()) || DataType::IsReference(inst->GetType()),
        (std::cerr << "Unallowed type of coalesced load\n", inst->Dump(&std::cerr)));
    auto loadObj = inst->CastToLoadObjectPair();
    CheckMemoryInstruction(v, inst, loadObj->GetNeedBarrier());
    ASSERT(loadObj->GetObjectType() == MEM_OBJECT || loadObj->GetObjectType() == MEM_STATIC);
    ASSERT(loadObj->GetVolatile() == false);
    auto field0 = loadObj->GetObjField0();
    auto field1 = loadObj->GetObjField1();
    auto graph = static_cast<GraphChecker *>(v)->GetGraph();
    [[maybe_unused]] size_t offset0 = GetObjectOffset(graph, loadObj->GetObjectType(), field0, loadObj->GetTypeId0());
    [[maybe_unused]] size_t offset1 = GetObjectOffset(graph, loadObj->GetObjectType(), field1, loadObj->GetTypeId1());
    [[maybe_unused]] size_t fieldSize = GetTypeSize(inst->GetType(), graph->GetArch()) / BYTE_SIZE;
    ASSERT((offset0 + fieldSize == offset1) || (offset1 + fieldSize == offset0));
}
void GraphChecker::VisitLoadArrayPairI(GraphVisitor *v, Inst *inst)
{
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, MemoryCoalescing::AcceptedType(inst->GetType()) || DataType::IsReference(inst->GetType()),
        (std::cerr << "Unallowed type of coalesced load\n", inst->Dump(&std::cerr)));
    CheckMemoryInstruction(v, inst, inst->CastToLoadArrayPairI()->GetNeedBarrier());
}

void GraphChecker::VisitLoadPairPart(GraphVisitor *v, Inst *inst)
{
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, MemoryCoalescing::AcceptedType(inst->GetType()) || DataType::IsReference(inst->GetType()),
        (std::cerr << "Unallowed type of coalesced load\n", inst->Dump(&std::cerr)));
    CheckMemoryInstruction(v, inst);
    [[maybe_unused]] auto op1 = inst->GetInputs()[0].GetInst();
    [[maybe_unused]] auto idx = inst->CastToLoadPairPart()->GetImm();
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, op1->WithGluedInsts(),
                                        (std::cerr << "Input instruction is not a Pair\n", inst->Dump(&std::cerr)));
    if (op1->GetOpcode() == Opcode::LoadArrayPairI) {
        CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, idx < op1->CastToLoadArrayPairI()->GetDstCount(),
                                            (std::cerr << "Pair index is out of bounds\n", inst->Dump(&std::cerr)));
    } else if (op1->GetOpcode() == Opcode::LoadArrayPair) {
        CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, idx < op1->CastToLoadArrayPair()->GetDstCount(),
                                            (std::cerr << "Pair index is out of bounds\n", inst->Dump(&std::cerr)));
    } else {
        ASSERT(op1->GetOpcode() == Opcode::LoadObjectPair);
    }
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, CheckCommonTypes(inst, inst->GetInputs()[0].GetInst()),
        (std::cerr << "Types of load vector element and vector input are not compatible\n", inst->Dump(&std::cerr)));

    // Strict order here
    auto prev = inst->GetPrev();
    while (prev != nullptr && prev != op1) {
        if (prev->GetOpcode() == Opcode::LoadPairPart || prev->GetOpcode() == Opcode::SpillFill) {
            prev = prev->GetPrev();
        } else {
            break;
        }
    }
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, prev != nullptr && prev == op1,
        (std::cerr << "LoadPairPart(s) instructions must follow immediately after appropriate "
                      "LoadArrayPair(I) or LoadObjectPair\n",
         inst->Dump(&std::cerr), prev->Dump(&std::cerr), inst->GetBasicBlock()->GetGraph()->Dump(&std::cerr)));
}

void GraphChecker::VisitStoreArrayPair(GraphVisitor *v, Inst *inst)
{
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, MemoryCoalescing::AcceptedType(inst->GetType()) || DataType::IsReference(inst->GetType()),
        (std::cerr << "Unallowed type of coalesced store\n", inst->Dump(&std::cerr)));
    CheckMemoryInstruction(v, inst);
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, CheckCommonTypes(inst, inst->GetInputs()[2U].GetInst()),
        (std::cerr << "Types of store and the first stored value are not compatible\n", inst->Dump(&std::cerr)));
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, CheckCommonTypes(inst, inst->GetInputs()[3U].GetInst()),
        (std::cerr << "Types of store and the second stored value are not compatible\n", inst->Dump(&std::cerr)));
    [[maybe_unused]] bool needBarrier = inst->CastToStoreArrayPair()->GetNeedBarrier();
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, needBarrier == (inst->GetType() == DataType::REFERENCE) || inst->GetType() == DataType::ANY,
        (std::cerr << "StoreArrayPair has incorrect value NeedBarrier", inst->Dump(&std::cerr)));
}

void GraphChecker::VisitStoreObjectPair(GraphVisitor *v, Inst *inst)
{
    auto storeObj = inst->CastToStoreObjectPair();
    bool needBarrier = storeObj->GetNeedBarrier();
    CheckMemoryInstruction(v, inst, needBarrier);
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, CheckCommonTypes(inst, inst->GetInputs()[1].GetInst()),
        (std::cerr << "Types of store and the first store input are not compatible\n", inst->Dump(&std::cerr)));
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, CheckCommonTypes(inst, inst->GetInputs()[2].GetInst()),
        (std::cerr << "Types of store and the second store input are not compatible\n", inst->Dump(&std::cerr)));
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, needBarrier == (inst->GetType() == DataType::REFERENCE) || inst->GetType() == DataType::ANY,
        (std::cerr << "StoreObjectPair has incorrect value NeedBarrier", inst->Dump(&std::cerr)));

    ASSERT(storeObj->GetObjectType() == MEM_OBJECT || storeObj->GetObjectType() == MEM_STATIC);
    ASSERT(storeObj->GetVolatile() == false);
    auto field0 = storeObj->GetObjField0();
    auto field1 = storeObj->GetObjField1();
    auto graph = static_cast<GraphChecker *>(v)->GetGraph();
    [[maybe_unused]] size_t offset0 = GetObjectOffset(graph, storeObj->GetObjectType(), field0, storeObj->GetTypeId0());
    [[maybe_unused]] size_t offset1 = GetObjectOffset(graph, storeObj->GetObjectType(), field1, storeObj->GetTypeId1());
    [[maybe_unused]] size_t fieldSize = GetTypeSize(inst->GetType(), graph->GetArch()) / BYTE_SIZE;
    ASSERT((offset0 + fieldSize == offset1) || (offset1 + fieldSize == offset0));
}

void GraphChecker::VisitStoreArrayPairI(GraphVisitor *v, Inst *inst)
{
    bool needBarrier = inst->CastToStoreArrayPairI()->GetNeedBarrier();
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, MemoryCoalescing::AcceptedType(inst->GetType()) || DataType::IsReference(inst->GetType()),
        (std::cerr << "Unallowed type of coalesced store\n", inst->Dump(&std::cerr)));
    CheckMemoryInstruction(v, inst, needBarrier);
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, CheckCommonTypes(inst, inst->GetInputs()[1].GetInst()),
        (std::cerr << "Types of store and the first stored value are not compatible\n", inst->Dump(&std::cerr)));
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, CheckCommonTypes(inst, inst->GetInputs()[2U].GetInst()),
        (std::cerr << "Types of store and the second stored value are not compatible\n", inst->Dump(&std::cerr)));
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, needBarrier == (inst->GetType() == DataType::REFERENCE) || inst->GetType() == DataType::ANY,
        (std::cerr << "StoreArrayPairI has incorrect value NeedBarrier", inst->Dump(&std::cerr)));
}

void GraphChecker::VisitStore(GraphVisitor *v, Inst *inst)
{
    bool needBarrier = inst->CastToStore()->GetNeedBarrier();
    CheckMemoryInstruction(v, inst, needBarrier);
}

void GraphChecker::VisitStoreI(GraphVisitor *v, Inst *inst)
{
    bool needBarrier = inst->CastToStoreI()->GetNeedBarrier();
    CheckMemoryInstruction(v, inst, needBarrier);
}

void GraphChecker::VisitStoreArray(GraphVisitor *v, Inst *inst)
{
    bool needBarrier = inst->CastToStoreArray()->GetNeedBarrier();
    CheckMemoryInstruction(v, inst, needBarrier);
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, CheckCommonTypes(inst, inst->GetInputs()[2U].GetInst()),
        (std::cerr << "Types of store and store input are not compatible\n", inst->Dump(&std::cerr)));
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, needBarrier == (inst->GetType() == DataType::REFERENCE) || inst->GetType() == DataType::ANY,
        (std::cerr << "StoreArray has incorrect value NeedBarrier", inst->Dump(&std::cerr)));
}

void GraphChecker::VisitStoreArrayI(GraphVisitor *v, Inst *inst)
{
    bool needBarrier = inst->CastToStoreArrayI()->GetNeedBarrier();
    CheckMemoryInstruction(v, inst, needBarrier);
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, CheckCommonTypes(inst, inst->GetInputs()[1].GetInst()),
        (std::cerr << "Types of store and store input are not compatible\n", inst->Dump(&std::cerr)));
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, needBarrier == (inst->GetType() == DataType::REFERENCE) || inst->GetType() == DataType::ANY,
        (std::cerr << "StoreArrayI has incorrect value NeedBarrier", inst->Dump(&std::cerr)));
}

void GraphChecker::VisitStoreStatic(GraphVisitor *v, Inst *inst)
{
    bool needBarrier = inst->CastToStoreStatic()->GetNeedBarrier();
    CheckMemoryInstruction(v, inst, needBarrier);
    auto graph = static_cast<GraphChecker *>(v)->GetGraph();
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, CheckCommonTypes(inst, inst->GetInputs()[1].GetInst()),
        (std::cerr << "Types of store and store input are not compatible\n", inst->Dump(&std::cerr)));
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, needBarrier == (inst->GetType() == DataType::REFERENCE),
        (std::cerr << "StoreStatic has incorrect value NeedBarrier", inst->Dump(&std::cerr)));
    [[maybe_unused]] auto initInst = inst->GetInputs()[0].GetInst();
    if (initInst->IsPhi()) {
        return;
    }
    [[maybe_unused]] auto opcode = initInst->GetOpcode();
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, (opcode == Opcode::LoadAndInitClass || opcode == Opcode::LoadImmediate),
        (std::cerr << "The first input for the StoreStatic should be LoadAndInitClass or LoadImmediate",
         inst->Dump(&std::cerr), initInst->Dump(&std::cerr)));
    [[maybe_unused]] auto storeStatic = inst->CastToStoreStatic();
    [[maybe_unused]] auto classId =
        graph->GetRuntime()->GetClassIdForField(storeStatic->GetMethod(), storeStatic->GetTypeId());
    // See comment in VisitNewObject about this if statement
    if (opcode == Opcode::LoadAndInitClass && initInst->CastToLoadAndInitClass()->GetClass() == nullptr) {
        CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, initInst->CastToLoadAndInitClass()->GetTypeId() == classId,
                                            (std::cerr << "StoreStatic and LoadAndInitClass must have equal class",
                                             inst->Dump(&std::cerr), initInst->Dump(&std::cerr)));
    }
}

void GraphChecker::VisitUnresolvedStoreStatic(GraphVisitor *v, Inst *inst)
{
    bool needBarrier = inst->CastToUnresolvedStoreStatic()->GetNeedBarrier();
    CheckMemoryInstruction(v, inst, needBarrier);
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, CheckCommonTypes(inst, inst->GetInputs()[0].GetInst()),
        (std::cerr << "Types of store and store input are not compatible\n", inst->Dump(&std::cerr)));
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, needBarrier == (inst->GetType() == DataType::REFERENCE),
        (std::cerr << "UnresolvedStoreStatic has incorrect value NeedBarrier", inst->Dump(&std::cerr)));
    [[maybe_unused]] auto ss = inst->GetInputs()[1].GetInst();
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, ss->GetOpcode() == Opcode::SaveState,
        (std::cerr << "UnresolvedStoreStatic instruction second operand is not a SaveState", inst->Dump(&std::cerr),
         ss->Dump(&std::cerr)));
}

void GraphChecker::VisitStoreObject(GraphVisitor *v, Inst *inst)
{
    bool needBarrier = inst->CastToStoreObject()->GetNeedBarrier();
    CheckMemoryInstruction(v, inst, needBarrier);
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, CheckCommonTypes(inst, inst->GetInputs()[1].GetInst()),
        (std::cerr << "Types of store and store input are not compatible\n", inst->Dump(&std::cerr)));
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, needBarrier == (inst->GetType() == DataType::REFERENCE) || inst->GetType() == DataType::ANY,
        (std::cerr << "StoreObject has incorrect value NeedBarrier", inst->Dump(&std::cerr)));
    CheckObjectType(v, inst, inst->CastToStoreObject()->GetObjectType(), inst->CastToStoreObject()->GetTypeId());
}

void GraphChecker::VisitStoreObjectDynamic(GraphVisitor *v, Inst *inst)
{
    bool needBarrier = inst->CastToStoreObjectDynamic()->IsBarrier();
    CheckMemoryInstruction(v, inst, needBarrier);
}

void GraphChecker::VisitFillConstArray(GraphVisitor *v, Inst *inst)
{
    bool needBarrier = inst->CastToFillConstArray()->GetNeedBarrier();
    CheckMemoryInstruction(v, inst, needBarrier);
}

void GraphChecker::VisitStoreResolvedObjectField(GraphVisitor *v, Inst *inst)
{
    bool needBarrier = inst->CastToStoreResolvedObjectField()->GetNeedBarrier();
    CheckMemoryInstruction(v, inst, needBarrier);
}

void GraphChecker::VisitStoreResolvedObjectFieldStatic(GraphVisitor *v, Inst *inst)
{
    bool needBarrier = inst->CastToStoreResolvedObjectFieldStatic()->GetNeedBarrier();
    CheckMemoryInstruction(v, inst, needBarrier);
}

void GraphChecker::VisitLoadStatic(GraphVisitor *v, Inst *inst)
{
    CheckMemoryInstruction(v, inst, inst->CastToLoadStatic()->GetNeedBarrier());
    auto graph = static_cast<GraphChecker *>(v)->GetGraph();
    [[maybe_unused]] auto initInst = inst->GetInputs()[0].GetInst();
    if (initInst->IsPhi()) {
        return;
    }
    [[maybe_unused]] auto opcode = initInst->GetOpcode();
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, opcode == Opcode::LoadAndInitClass || opcode == Opcode::LoadImmediate,
        (std::cerr << "The first input for the LoadStatic should be LoadAndInitClass or LoadImmediate",
         inst->Dump(&std::cerr), initInst->Dump(&std::cerr)));
    [[maybe_unused]] auto loadStatic = inst->CastToLoadStatic();
    [[maybe_unused]] auto classId =
        graph->GetRuntime()->GetClassIdForField(loadStatic->GetMethod(), loadStatic->GetTypeId());
    // See comment in VisitNewObject about this if statement
    if (opcode == Opcode::LoadAndInitClass && initInst->CastToLoadAndInitClass()->GetClass() == nullptr) {
        CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, initInst->CastToLoadAndInitClass()->GetTypeId() == classId,
                                            (std::cerr << "LoadStatic and LoadAndInitClass must have equal class",
                                             inst->Dump(&std::cerr), initInst->Dump(&std::cerr)));
    }
}

void GraphChecker::VisitLoadClass([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
{
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, inst->GetType() == DataType::REFERENCE,
                                        (std::cerr << "LoadClass must have Reference type", inst->Dump(&std::cerr)));
}

void GraphChecker::VisitLoadAndInitClass([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
{
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, inst->GetType() == DataType::REFERENCE,
        (std::cerr << "LoadAndInitClass must have Reference type", inst->Dump(&std::cerr)));
}

void GraphChecker::VisitUnresolvedLoadAndInitClass([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, inst->GetType() == DataType::REFERENCE,
        (std::cerr << "UnresolvedLoadAndInitClass must have Reference type", inst->Dump(&std::cerr)));
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, inst->CastToUnresolvedLoadAndInitClass()->GetClass() == nullptr,
        (std::cerr << "UnresolvedLoadAndInitClass must have a null ClassPtr", inst->Dump(&std::cerr)));
    [[maybe_unused]] auto ss = inst->GetInputs()[0].GetInst();
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, ss->GetOpcode() == Opcode::SaveState,
        (std::cerr << "UnresolvedLoadAndInitClass instruction first operand is not a SaveState", inst->Dump(&std::cerr),
         ss->Dump(&std::cerr)));
}

void GraphChecker::VisitGetInstanceClass([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, inst->GetType() == DataType::REFERENCE,
        (std::cerr << "GetInstanceClass must have Reference type", inst->Dump(&std::cerr)));
    for (auto &user : inst->GetUsers()) {
        [[maybe_unused]] auto userInst = user.GetInst();
        CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, !userInst->IsSaveState(),
                                            (std::cerr << "Incorrect user of the GetInstanceClass",
                                             inst->Dump(&std::cerr), userInst->Dump(&std::cerr)));
    }
    ASSERT_DO(
        !inst->GetBasicBlock()->GetGraph()->IsDynamicMethod(),
        (std::cerr
         << "we can't use GetInstanceClass in dynamic, because object class can be changed. Use LoadObject MEM_CLASS"));
}

void GraphChecker::VisitNewObject([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, inst->GetType() == DataType::REFERENCE,
                                        (std::cerr << "NewObject must be have Reference type", inst->Dump(&std::cerr)));
    [[maybe_unused]] auto initInst = inst->GetInputs()[0].GetInst();
    if (initInst->IsPhi()) {
        return;
    }
    [[maybe_unused]] auto opcode = initInst->GetOpcode();
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v,
        opcode == Opcode::LoadAndInitClass || opcode == Opcode::LoadRuntimeClass ||
            opcode == Opcode::UnresolvedLoadAndInitClass || opcode == Opcode::LoadImmediate || opcode == Opcode::Phi ||
            opcode == Opcode::Select || opcode == Opcode::SelectImm,
        (std::cerr << "The first input for the NewObject should be LoadAndInitClass, "
                      "LoadRuntimeClass, UnresolvedLoadAndInitClass, LoadImmediate, Phi, Select or SelectImm",
         inst->Dump(&std::cerr), initInst->Dump(&std::cerr)));
    [[maybe_unused]] auto ssInst = inst->GetInputs()[1].GetInst();
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, ssInst->GetOpcode() == Opcode::SaveState,
                                        (std::cerr << "The second input for the NewObject should be SaveState",
                                         inst->Dump(&std::cerr), ssInst->Dump(&std::cerr)));
    // If InitClass contains an already resolved class, then IDs may be different. Because VN can remove the
    // duplicated InitClass and keep only one that is located in the inlined method and has a different id
    // accordingly.
    if (initInst->GetOpcode() == Opcode::LoadAndInitClass &&
        initInst->CastToLoadAndInitClass()->GetClass() == nullptr) {
        CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
            v, initInst->CastToLoadAndInitClass()->GetTypeId() == inst->CastToNewObject()->GetTypeId(),
            std::cerr << "NewObject and LoadAndInitClass must have equal class:\n"
                      << *inst << '\n'
                      << *initInst << std::endl);
    } else if (initInst->GetOpcode() == Opcode::UnresolvedLoadAndInitClass) {
        CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
            v, initInst->CastToUnresolvedLoadAndInitClass()->GetTypeId() == inst->CastToNewObject()->GetTypeId(),
            std::cerr << "NewObject and UnresolvedLoadAndInitClass must have equal class:\n"
                      << *inst << '\n'
                      << *initInst << std::endl);
    }
}

void GraphChecker::VisitLoadRuntimeClass([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
{
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, inst->CastToLoadRuntimeClass()->GetTypeId() == TypeIdMixin::MEM_PROMISE_CLASS_ID,
        (std::cerr << "LoadRuntimeClass should have TypeId MEM_PROMISE_CLASS_ID", inst->Dump(&std::cerr)));
}

void GraphChecker::VisitInitObject([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
{
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, g_options.IsCompilerSupportInitObjectInst(),
        (std::cerr << "Instruction InitObject isn't supported", inst->Dump(&std::cerr)));
}

void GraphChecker::VisitInitClass([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
{
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, inst->GetType() == DataType::NO_TYPE,
                                        (std::cerr << "InitClass doesn't have type", inst->Dump(&std::cerr)));
}

void GraphChecker::VisitIntrinsic([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
{
    auto *checker = static_cast<GraphChecker *>(v);
    auto stringFlatCheckArgMask = GetStringFlatCheckArgMask(inst->CastToIntrinsic()->GetIntrinsicId());
    if (checker->GetPassName() == StringFlatCheck::NAME && checker->GetGraph()->GetRuntime()->CanUseStringFlatCheck() &&
        stringFlatCheckArgMask != 0) {
        auto argsCount = inst->RequireState() ? inst->GetInputsCount() - 1 : inst->GetInputsCount();
        for (size_t i = 0; i < argsCount; i++) {
            // NOLINTNEXTLINE(hicpp-signed-bitwise)
            if ((stringFlatCheckArgMask & (1 << i)) != 0) {
                [[maybe_unused]] auto *inputInst = Inst::GetDataFlowInput(inst->GetInput(i).GetInst());
                CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
                    v, inputInst->GetFlag(inst_flags::FLAT_STRING),
                    (std::cerr << "\nInput must be a flat string\n", inst->Dump(&std::cerr)));
            }
        }
    }
    switch (inst->CastToIntrinsic()->GetIntrinsicId()) {
#include "intrinsics_graph_checker.inl"
        default: {
            return;
        }
    }
}

void GraphChecker::VisitLoadObject(GraphVisitor *v, Inst *inst)
{
    CheckMemoryInstruction(v, inst, inst->CastToLoadObject()->GetNeedBarrier());
    CheckObjectType(v, inst, inst->CastToLoadObject()->GetObjectType(), inst->CastToLoadObject()->GetTypeId());
}

void GraphChecker::VisitConstant([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
{
    [[maybe_unused]] auto type = inst->GetType();
    [[maybe_unused]] auto isDynamic = static_cast<GraphChecker *>(v)->GetGraph()->IsDynamicMethod();
    if (static_cast<GraphChecker *>(v)->GetGraph()->IsBytecodeOptimizer()) {
        CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
            v,
            type == DataType::FLOAT32 || type == DataType::FLOAT64 || type == DataType::INT64 ||
                type == DataType::INT32 || (type == DataType::ANY && isDynamic),
            (std::cerr << "Constant inst can be only FLOAT32, FLOAT64, INT32 or INT64\n", inst->Dump(&std::cerr)));

    } else {
        CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
            v,
            type == DataType::FLOAT32 || type == DataType::FLOAT64 || type == DataType::INT64 ||
                (type == DataType::ANY && isDynamic),
            (std::cerr << "Constant instruction can be only FLOAT32, FLOAT64 or INT64\n", inst->Dump(&std::cerr)));
    }
}

void GraphChecker::VisitNullPtr([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
{
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, inst->GetType() == DataType::REFERENCE,
        (std::cerr << "NullPtr instruction should have REFERENCE type only\n", inst->Dump(&std::cerr)));

    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, static_cast<GraphChecker *>(v)->IncrementNullPtrInstCounterAndGet() == 1,
        (std::cerr << "There should be not more than one NullPtr instruction in graph\n",
         inst->GetBasicBlock()->Dump(&std::cerr)));
}

void GraphChecker::VisitLoadUniqueObject([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
{
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, inst->GetType() == DataType::REFERENCE,
        (std::cerr << "LoadUniqueObject instruction should have REFERENCE type only\n", inst->Dump(&std::cerr)));

    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, static_cast<GraphChecker *>(v)->IncrementLoadUniqueObjectInstCounterAndGet() == 1,
        (std::cerr << "There should be not more than one LoadUniqueObject instruction in graph\n",
         inst->GetBasicBlock()->Dump(&std::cerr)));
}

void GraphChecker::VisitPhi([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    for ([[maybe_unused]] auto input : inst->GetInputs()) {
        CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
            v,
            CheckCommonTypes(inst, input.GetInst()) ||
                (static_cast<GraphChecker *>(v)->GetGraph()->IsThrowApplied() &&
                 (input.GetInst()->GetBasicBlock()->IsEndWithThrow() ||
                  input.GetInst()->GetOpcode() == Opcode::CatchPhi || input.GetInst()->GetOpcode() == Opcode::Phi)),
            (std::cerr << "Types of phi result and phi input are not compatible\n"
                       << *inst << std::endl
                       << *input.GetInst()));
    }
}

void GraphChecker::VisitParameter([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
{
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, inst->GetType() != DataType::NO_TYPE,
                                        (std::cerr << "The parametr doesn't have type:\n", inst->Dump(&std::cerr)));
}

void GraphChecker::VisitCompare([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    [[maybe_unused]] auto op1 = inst->GetInputs()[0].GetInst();
    [[maybe_unused]] auto op2 = inst->GetInputs()[1].GetInst();
    for (size_t i = 0; i < inst->GetInputsCount(); i++) {
        CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, inst->GetInputType(i) != DataType::NO_TYPE,
                                            std::cerr << "Source operand type is not set: " << *inst << std::endl);
    }
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, inst->GetInputType(0) == inst->GetInputType(1),
                                        std::cerr << "Conditional instruction has different inputs type: " << *inst
                                                  << std::endl);
    if (inst->GetInputType(0) == DataType::REFERENCE) {
        CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
            v,
            inst->CastToCompare()->GetCc() == ConditionCode::CC_NE ||
                inst->CastToCompare()->GetCc() == ConditionCode::CC_EQ,
            (std::cerr << "Reference compare must have CC_NE or CC_EQ: \n", inst->Dump(&std::cerr)));
        if (op1->IsConst()) {
            CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, IsZeroConstant(op1),
                                                (std::cerr << "Constant reference input must be integer 0: \n",
                                                 inst->Dump(&std::cerr), op1->Dump(&std::cerr)));
        } else {
            CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
                v, op1->GetType() == DataType::REFERENCE,
                (std::cerr << "Condition instruction 1st operand type is not a reference\n", inst->Dump(&std::cerr),
                 op1->Dump(&std::cerr)));
        }
        if (op2->IsConst()) {
            CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, IsZeroConstant(op2),
                                                (std::cerr << "Constant reference input must be integer 0: \n",
                                                 inst->Dump(&std::cerr), op2->Dump(&std::cerr)));
        } else {
            CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
                v, op2->GetType() == DataType::REFERENCE,
                (std::cerr << "Condition instruction 2nd operand type is not a reference\n", inst->Dump(&std::cerr),
                 op2->Dump(&std::cerr)));
        }
    }
    CHECKER_MESSAGE_IF_NOT_AND_PRINT_VISITOR(v, inst->GetType() == DataType::BOOL,
                                             "Condition instruction type is not a bool");
}

void GraphChecker::VisitCast([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    [[maybe_unused]] auto dstType = inst->GetType();
    [[maybe_unused]] auto srcType = inst->GetInputType(0);
    [[maybe_unused]] auto inputType = inst->GetInput(0).GetInst()->GetType();

    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, DataType::IsTypeNumeric(dstType),
        (std::cerr << "Cast instruction dst type is not a numeric type\n", inst->Dump(&std::cerr)));
    if (static_cast<GraphChecker *>(v)->GetGraph()->GetMode().SupportManagedCode()) {
        CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
            v, DataType::IsTypeNumeric(srcType),
            (std::cerr << "Cast instruction src type is not a numeric type\n", inst->Dump(&std::cerr)));
        CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
            v, DataType::IsTypeNumeric(inputType),
            (std::cerr << "Cast instruction operand type is not a numeric type\n", inst->Dump(&std::cerr)));
    }
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, DataType::GetCommonType(srcType) == DataType::GetCommonType(inputType),
                                        (std::cerr << "Incorrect src_type and input type\n", inst->Dump(&std::cerr)));
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, !(DataType::IsFloatType(srcType) && DataType::IsLessInt32(dstType)),
                                        (std::cerr << "Cast instruction from "
                                                   << DataType::internal::TYPE_NAMES.at(srcType) << " to "
                                                   << DataType::internal::TYPE_NAMES.at(dstType) << " don't support\n",
                                         inst->Dump(&std::cerr)));
}

void GraphChecker::VisitCmp([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    [[maybe_unused]] auto op1 = inst->GetInput(0).GetInst();
    [[maybe_unused]] auto op2 = inst->GetInput(1).GetInst();
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, DataType::IsTypeNumeric(op1->GetType()),
                                        (std::cerr << "Cmp instruction 1st operand type is not a numeric type\n",
                                         inst->Dump(&std::cerr), op1->Dump(&std::cerr)));
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, DataType::IsTypeNumeric(op2->GetType()),
                                        (std::cerr << "Cmp instruction 2st operand type is not a numeric type\n",
                                         inst->Dump(&std::cerr), op2->Dump(&std::cerr)));
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, DataType::GetCommonType(op1->GetType()) == DataType::GetCommonType(inst->GetInputType(0)),
        (std::cerr << "Input type and Cmp Input Type are not equal\n", inst->Dump(&std::cerr), op1->Dump(&std::cerr)));
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, DataType::GetCommonType(op2->GetType()) == DataType::GetCommonType(inst->GetInputType(1)),
        (std::cerr << "Input type and Cmp Input Type are not equal\n", inst->Dump(&std::cerr), op2->Dump(&std::cerr)));
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, inst->GetType() == DataType::INT32,
                                        (std::cerr << "Cmp instruction type is not a int32\n", inst->Dump(&std::cerr)));
    for (size_t i = 0; i < inst->GetInputsCount(); i++) {
        CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, inst->GetInputType(i) != DataType::NO_TYPE,
                                            std::cerr << "Source operand type is not set: " << *inst << std::endl);
    }
}

void GraphChecker::VisitMonitor([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    [[maybe_unused]] auto op = inst->GetInputs()[0].GetInst();
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, inst->GetType() == DataType::VOID,
                                        (std::cerr << "Monitor type is not a void", inst->Dump(&std::cerr)));
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, DataType::IsReference(op->GetType()),
                                        (std::cerr << "Monitor instruction 1st operand type is not a reference",
                                         inst->Dump(&std::cerr), op->Dump(&std::cerr)));
    [[maybe_unused]] auto op1 = inst->GetInputs()[1].GetInst();
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, op1->GetOpcode() == Opcode::SaveState,
                                        (std::cerr << "Monitor instruction second operand is not a SaveState",
                                         inst->Dump(&std::cerr), op1->Dump(&std::cerr)));
}

void GraphChecker::VisitReturn(GraphVisitor *v, Inst *inst)
{
    [[maybe_unused]] auto op = inst->GetInputs()[0].GetInst();
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, CheckCommonTypes(inst, op),
                                        (std::cerr << "Types of return and its input are not compatible\n return:\n",
                                         inst->Dump(&std::cerr), std::cerr << "\n input:\n", op->Dump(&std::cerr)));
    CheckContrlFlowInst(v, inst);
    [[maybe_unused]] auto numSuccs = inst->GetBasicBlock()->GetSuccsBlocks().size();
    CHECKER_MESSAGE_IF_NOT_AND_PRINT_VISITOR(v, numSuccs == 1, "Basic block with Return must have 1 successor");
    [[maybe_unused]] auto succ = inst->GetBasicBlock()->GetSuccsBlocks()[0];
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, succ->IsEndBlock() || succ->IsTryEnd(),
                                        std::cerr
                                            << "Basic block with Return must have end or try end block as successor:\n"
                                            << *inst << std::endl);
}

void GraphChecker::VisitReturnVoid(GraphVisitor *v, Inst *inst)
{
    CheckContrlFlowInst(v, inst);
    [[maybe_unused]] auto numSuccs = inst->GetBasicBlock()->GetSuccsBlocks().size();
    CHECKER_MESSAGE_IF_NOT_AND_PRINT_VISITOR(v, numSuccs == 1, "Basic block with ReturnVoid must have 1 successor");
    [[maybe_unused]] auto succ = inst->GetBasicBlock()->GetSuccsBlocks()[0];
    CHECKER_MESSAGE_IF_NOT_AND_PRINT_VISITOR(
        v, succ->IsEndBlock() || succ->IsTryEnd(),
        "Basic block with ReturnVoid must have end or try_end block as successor.");
}

void GraphChecker::VisitNullCheck([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    [[maybe_unused]] Inst *array = inst->GetInput(0).GetInst();
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, DataType::IsReference(array->GetType()) || array->GetType() == DataType::ANY,
                                        (std::cerr << "\n Types of input NullCheck must be REFERENCE or ANY: \n",
                                         inst->Dump(&std::cerr), array->Dump(&std::cerr)));
    [[maybe_unused]] auto ss = inst->GetInput(1).GetInst();
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, ss->GetOpcode() == Opcode::SaveState || ss->GetOpcode() == Opcode::SaveStateDeoptimize,
        (std::cerr << "\n Second input of NullCheck must be SaveState: \n", inst->Dump(&std::cerr),
         ss->Dump(&std::cerr)));
}

void GraphChecker::VisitBoundsCheck([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    for (int i = 0; i < 1; i++) {
        [[maybe_unused]] auto op = inst->GetInputs()[i].GetInst();
        [[maybe_unused]] auto opType = op->GetType();
        // NOTE(pishin): actually type should be INT32, but predecessor may be Call instruction with type u16, u8
        // e.t.c
        CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
            v,
            (op->IsConst() && opType == DataType::INT64) ||
                (DataType::GetCommonType(opType) == DataType::INT64 &&
                 Is32Bits(opType, static_cast<GraphChecker *>(v)->GetGraph()->GetArch())),
            (std::cerr << "Types of " << i << " input BoundsCheck must be INT32 or less:\n", inst->Dump(&std::cerr)));
    }
}

void GraphChecker::VisitBoundsCheckI([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
{
    CHECKER_IF_NOT_PRINT_VISITOR(v, !inst->HasUsers());
}

void GraphChecker::VisitRefTypeCheck(GraphVisitor *v, Inst *inst)
{
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, (inst->GetType() == DataType::REFERENCE),
        (std::cerr << "Types of RefTypeCheck must be REFERENCE\n", inst->Dump(&std::cerr)));
    for (unsigned i = 0; i < 2U; i++) {
        [[maybe_unused]] auto op = inst->GetInputs()[i].GetInst();
        [[maybe_unused]] auto opType = op->GetType();
        CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, (opType == DataType::REFERENCE),
                                            (std::cerr << "Types of " << i << " input RefTypeCheck must be REFERENCE\n",
                                             inst->Dump(&std::cerr), op->Dump(&std::cerr)));
    }
    CheckThrows(v, inst,
                {Opcode::StoreArray, Opcode::StoreArrayPair, Opcode::StoreArrayI, Opcode::StoreArrayPairI,
                 Opcode::SaveState, Opcode::SafePoint});
}

void GraphChecker::VisitNegativeCheck(GraphVisitor *v, Inst *inst)
{
    [[maybe_unused]] auto op = inst->GetInputs()[0].GetInst();
    [[maybe_unused]] auto opType = op->GetType();
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, DataType::GetCommonType(opType) == DataType::INT64,
        (std::cerr << "Type of NegativeCheck must be integer\n", inst->Dump(&std::cerr)));
    if (inst->GetBasicBlock()->GetGraph()->IsDynamicMethod()) {
        // In dynamic methods for negative values we creates f64 Mod, so we insert NegativeCheck before Mod
        // Lowering can change Mod to And(I)
        CheckThrows(
            v, inst,
            {Opcode::NewArray, Opcode::MultiArray, Opcode::Phi, Opcode::Mod, Opcode::ModI, Opcode::And, Opcode::AndI});
    } else {
        CheckThrows(v, inst, {Opcode::NewArray, Opcode::MultiArray, Opcode::Phi});
    }
}

void GraphChecker::VisitNotPositiveCheck(GraphVisitor *v, Inst *inst)
{
    [[maybe_unused]] auto op = inst->GetInputs()[0].GetInst();
    [[maybe_unused]] auto opType = op->GetType();
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, DataType::GetCommonType(opType) == DataType::INT64,
        (std::cerr << "Type of NotPositiveCheck must be integer\n", inst->Dump(&std::cerr)));
    ASSERT(inst->GetBasicBlock()->GetGraph()->IsDynamicMethod());
    CheckThrows(v, inst, {Opcode::Phi, Opcode::Mod, Opcode::ModI});
}

void GraphChecker::VisitZeroCheck(GraphVisitor *v, Inst *inst)
{
    [[maybe_unused]] auto op = inst->GetInputs()[0].GetInst();
    [[maybe_unused]] auto opType = op->GetType();
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, DataType::GetCommonType(opType) == DataType::INT64,
        (std::cerr << "Type of ZeroCheck input must be integer\n", inst->Dump(&std::cerr)));
    CheckThrows(v, inst, {Opcode::Div, Opcode::DivI, Opcode::Mod, Opcode::ModI, Opcode::Phi});
}

void GraphChecker::VisitDeoptimizeIf([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    [[maybe_unused]] auto op = inst->GetInput(0).GetInst();
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, op->GetType() == DataType::BOOL || op->IsBoolConst(),
        (std::cerr << "Type of first input DeoptimizeIf must be BOOL:\n", inst->Dump(&std::cerr)));
    [[maybe_unused]] auto ss = inst->GetInput(1).GetInst();
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, ss->GetOpcode() == Opcode::SaveStateDeoptimize || ss->GetOpcode() == Opcode::SaveState,
        (std::cerr << "Second input DeoptimizeIf must be SaveStateDeoptimize or SaveState:\n", inst->Dump(&std::cerr)));
}

void GraphChecker::VisitLenArray([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, inst->GetType() == DataType::INT32,
                                        (std::cerr << "Type of LenArray must be INT32:\n", inst->Dump(&std::cerr)));
    [[maybe_unused]] auto op = inst->GetInputs()[0].GetInst();
    if (op->GetOpcode() == Opcode::NullCheck) {
        op = op->GetInput(0).GetInst();
    }
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, DataType::IsReference(op->GetType()),
        (std::cerr << "Types of input LenArray must be REFERENCE:\n", inst->Dump(&std::cerr), op->Dump(&std::cerr)));
}

void GraphChecker::VisitCallVirtual([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    auto graph = inst->GetBasicBlock()->GetGraph();
    // In AbcKit mode there are no SaveStates
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, inst->GetInputs().Size() > 0 || graph->IsAbcKit(),
                                        (std::cerr << "Virtual function must have inputs:\n", inst->Dump(&std::cerr)));
    if (graph->IsAbcKit()) {
        return;
    }
    [[maybe_unused]] auto op = inst->GetInputs()[0].GetInst();
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, DataType::IsReference(op->GetType()),
                                        (std::cerr << "Types of first input CallVirtual must be REFERENCE(this):\n",
                                         inst->Dump(&std::cerr), op->Dump(&std::cerr)));
}

void GraphChecker::VisitCallNative([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    [[maybe_unused]] auto *graph = inst->GetBasicBlock()->GetGraph();
    [[maybe_unused]] auto *callNative = inst->CastToCallNative();

    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, graph->GetMode().SupportManagedCode(),
        (std::cerr << "CallNative must be only in managed code:\n", callNative->Dump(&std::cerr)));
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, callNative->IsRuntimeCall() == (callNative->GetSaveState() != nullptr),
        (std::cerr << "CallNative with runtime_call flag must have SaveState (and vice versa):\n",
         callNative->Dump(&std::cerr)));
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, callNative->GetInputsCount() > 0U && callNative->GetInput(0U).GetInst()->GetType() == DataType::POINTER,
        (std::cerr << "CallNative must have native pointer as 0 input:\n", callNative->Dump(&std::cerr)));

    if (!callNative->IsRuntimeCall()) {
        [[maybe_unused]] bool hasRefInputs = false;
        for (size_t i = 1U; i < callNative->GetInputsCount(); ++i) {
            auto input = callNative->GetInput(i);
            if (DataType::IsReference(input.GetInst()->GetType())) {
                hasRefInputs = true;
                break;
            }
        }
        CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
            v, !hasRefInputs,
            (std::cerr << "CallNative without runtime_call flag cannot have ref inputs:\n",
             callNative->Dump(&std::cerr)));
        CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
            v, !DataType::IsReference(callNative->GetType()),
            (std::cerr << "CallNative without runtime_call flag cannot have ref type:\n",
             callNative->Dump(&std::cerr)));
    }
}

void GraphChecker::VisitCallDynamic([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
{
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, static_cast<GraphChecker *>(v)->GetGraph()->IsDynamicMethod(),
        (std::cerr << "CallDynamic is supported only for dynamic languages:\n", inst->Dump(&std::cerr)));
}

void GraphChecker::VisitSaveState([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
{
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, (static_cast<SaveStateInst *>(inst))->Verify(),
                                        std::cerr << "Inconsistent SaveState instruction:\n"
                                                  << *inst << std::endl);
#ifndef NDEBUG
    auto ss = inst->CastToSaveState();
    if (ss->GetInputsWereDeleted() && !ss->GetInputsWereDeletedSafely()) {
        for (auto &user : inst->GetUsers()) {
            CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
                v, !user.GetInst()->RequireRegMap(),
                std::cerr << "Some inputs from save_state were deleted, but the user requireRegMap:\n"
                          << *inst << std::endl
                          << *(user.GetInst()) << std::endl);
        }
    }
    auto graph = inst->GetBasicBlock()->GetGraph();
    if (graph->IsDynamicMethod() && !graph->IsDynUnitTest() && !graph->IsBytecodeOptimizer()) {
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define VREG_ENV_TYPES(VREG_TYPE) VRegInfo::VRegType::VREG_TYPE,
        for (auto envType : {VREGS_ENV_TYPE_DEFS(VREG_ENV_TYPES)}) {
            bool founded = false;
            for (size_t i = 0; i < inst->GetInputsCount(); ++i) {
                auto vreg = ss->GetVirtualRegister(i);
                founded |= vreg.GetVRegType() == envType;
            }
            CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, founded,
                                                std::cerr << VRegInfo::VRegTypeToString(envType) << " not found");
        }
#undef VREG_ENV_TYPES
    }

#endif
}

void GraphChecker::VisitSafePoint([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
{
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, !inst->HasUsers(),
                                        std::cerr << "SafePoint must not have users:\n"
                                                  << *inst << std::endl);
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, (static_cast<SaveStateInst *>(inst))->Verify(),
                                        std::cerr << "Inconsistent SafePoint instruction:\n"
                                                  << *inst << std::endl);
}

void GraphChecker::VisitSaveStateOsr([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
{
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, !inst->HasUsers(),
                                        std::cerr << "SafeStateOsr must not have users:\n"
                                                  << *inst << std::endl);
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, (static_cast<SaveStateInst *>(inst))->Verify(),
                                        std::cerr << "Inconsistent SafeStateOsr instruction:\n"
                                                  << *inst << std::endl);
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, static_cast<GraphChecker *>(v)->GetGraph()->IsOsrMode(),
                                        std::cerr << "SafeStateOsr must be created in the OSR mode only\n");
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, inst->GetBasicBlock()->IsOsrEntry(),
                                        std::cerr << "SafeStateOsr's basic block must be osr-entry\n");
    auto firstInst = inst->GetBasicBlock()->GetFirstInst();
    while (firstInst != nullptr && (firstInst->IsCatchPhi() || firstInst->GetOpcode() == Opcode::Try)) {
        firstInst = firstInst->GetNext();
    }
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, firstInst == inst,
                                        std::cerr << "SafeStateOsr must be the first instruction in the basic block\n");
}

void GraphChecker::VisitThrow([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
{
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, DataType::IsReference(inst->GetInput(0).GetInst()->GetType()),
                                        std::cerr << "Throw instruction must have input with reference type: " << *inst
                                                  << std::endl);
    [[maybe_unused]] auto bb = inst->GetBasicBlock();
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, inst == bb->GetLastInst(),
        std::cerr << "Throw instruction must be last instruction in the basic block: " << *inst << std::endl);
    for ([[maybe_unused]] auto succ : bb->GetSuccsBlocks()) {
        CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
            v, succ->IsEndBlock() || succ->IsTryEnd() || succ->IsCatchBegin(),
            std::cerr << "Throw block must have end block or try-end or catch-begin block as successor\n");
    }
}

void GraphChecker::VisitCheckCast([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
{
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, DataType::IsReference(inst->GetInput(0).GetInst()->GetType()),
        std::cerr << "CheckCast instruction must have input 0 with reference type: " << *inst << std::endl);

    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, DataType::IsReference(inst->GetInput(1).GetInst()->GetType()),
        std::cerr << "CheckCast instruction must have input 1 with reference type: " << *inst << std::endl);

    [[maybe_unused]] auto saveState = inst->GetInput(2).GetInst();
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, saveState != nullptr,
                                        std::cerr << "CheckCast instruction must have SaveState as input 2: " << *inst
                                                  << std::endl);
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, !inst->HasUsers(),
                                        std::cerr << "CheckCast must not have users: " << *inst << std::endl);
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, inst->GetType() == DataType::NO_TYPE,
                                        std::cerr << "CheckCast must not have type: " << *inst << std::endl);
    if (inst->CanDeoptimize()) {
        CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
            v, (saveState->GetOpcode() == Opcode::SaveState || saveState->GetOpcode() == Opcode::SaveStateDeoptimize),
            std::cerr << "CheckCast D instruction must have SaveState or SaveStateDeoptimize as input 2: " << *inst
                      << std::endl);
    } else {
        CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
            v, saveState->GetOpcode() == Opcode::SaveState,
            std::cerr << "CheckCast instruction must have SaveState as input 2: " << *inst << std::endl);
    }
}

void GraphChecker::VisitIsInstance([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
{
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, DataType::IsReference(inst->GetInput(0).GetInst()->GetType()),
        std::cerr << "IsInstance instruction must have input 0 with reference type: " << *inst << std::endl);
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, DataType::IsReference(inst->GetInput(1).GetInst()->GetType()),
        std::cerr << "IsInstance instruction must have input 1 with reference type: " << *inst << std::endl);

    [[maybe_unused]] auto saveState = inst->GetInput(2).GetInst();
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, (saveState != nullptr && saveState->GetOpcode() == Opcode::SaveState),
                                        std::cerr << "IsInstance instruction must have SaveState as input 2: " << *inst
                                                  << std::endl);

    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, inst->GetType() == DataType::BOOL,
                                        (std::cerr << "Types of IsInstance must be BOOL:\n", inst->Dump(&std::cerr)));
}

// Shared by Select and SelectTransform
void GraphChecker::VisitSelectWithReference([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::Select || inst->GetOpcode() == Opcode::SelectTransform);
    [[maybe_unused]] auto op1 = inst->GetInput(1).GetInst();
    [[maybe_unused]] auto op2 = inst->GetInput(2U).GetInst();
    [[maybe_unused]] auto op3 = inst->GetInput(3U).GetInst();
    [[maybe_unused]] ConditionCode cc;
    if (inst->GetOpcode() == Opcode::Select) {
        cc = inst->CastToSelect()->GetCc();
    } else {
        cc = inst->CastToSelectTransform()->GetCc();
    }

    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, cc == ConditionCode::CC_NE || cc == ConditionCode::CC_EQ,
        (std::cerr << "Select reference comparison must be CC_NE or CC_EQ: \n", inst->Dump(&std::cerr)));
    if (op2->IsConst()) {
        CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, IsZeroConstant(op2),
                                            (std::cerr << "Constant reference input must be integer 0: \n",
                                             inst->Dump(&std::cerr), op1->Dump(&std::cerr)));
    } else {
        CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, op2->GetType() == DataType::REFERENCE,
                                            (std::cerr << "Select instruction 3rd operand type is not a reference\n",
                                             inst->Dump(&std::cerr), op1->Dump(&std::cerr)));
    }
    if (op3->IsConst()) {
        CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, IsZeroConstant(op3),
                                            (std::cerr << "Constant reference input must be integer 0: \n",
                                             inst->Dump(&std::cerr), op2->Dump(&std::cerr)));
    } else {
        CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, op3->GetType() == DataType::REFERENCE,
                                            (std::cerr << "Select instruction 4th operand type is not a reference\n",
                                             inst->Dump(&std::cerr), op2->Dump(&std::cerr)));
    }
}

void GraphChecker::VisitSelect(GraphVisitor *v, Inst *inst)
{
    [[maybe_unused]] auto op0 = inst->GetInput(0).GetInst();
    [[maybe_unused]] auto op1 = inst->GetInput(1).GetInst();
    [[maybe_unused]] auto op2 = inst->GetInput(2U).GetInst();
    [[maybe_unused]] auto op3 = inst->GetInput(3U).GetInst();

    for (size_t i = 0; i < inst->GetInputsCount(); i++) {
        CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, inst->GetInputType(i) != DataType::NO_TYPE,
                                            std::cerr << "Source operand type is not set: " << *inst << std::endl);
    }

    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v,
        DataType::GetCommonType(inst->GetType()) == DataType::INT64 ||
            IsFloatType(DataType::GetCommonType(inst->GetType())) || inst->GetType() == DataType::REFERENCE ||
            inst->GetType() == DataType::ANY,
        (std::cerr << "Select instruction type is not integer or reference or any", inst->Dump(&std::cerr)));
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v,
        DataType::GetCommonType(op0->GetType()) == DataType::INT64 ||
            IsFloatType(DataType::GetCommonType(op0->GetType())) || op0->GetType() == DataType::REFERENCE ||
            op0->GetType() == DataType::ANY,
        (std::cerr << "Select instruction 1st operand type is not integer or reference or any",
         inst->Dump(&std::cerr)));
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v,
        DataType::GetCommonType(op1->GetType()) == DataType::INT64 ||
            IsFloatType(DataType::GetCommonType(op1->GetType())) || op1->GetType() == DataType::REFERENCE ||
            op1->GetType() == DataType::ANY,
        (std::cerr << "Select instruction 2nd operand type is not integer or reference or any",
         inst->Dump(&std::cerr)));

    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, CheckCommonTypes(op0, op1),
        (std::cerr << "Types of two first select instruction operands are not compatible\n", op0->Dump(&std::cerr),
         op1->Dump(&std::cerr), inst->Dump(&std::cerr)));
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, CheckCommonTypes(inst, op0),
        (std::cerr << "Types of instruction result and its operands are not compatible\n", inst->Dump(&std::cerr)));

    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, inst->GetInputType(2U) == inst->GetInputType(3U),
                                        std::cerr << "Select comparison arguments has different inputs type: " << *inst
                                                  << std::endl);
    if (inst->GetInputType(2U) == DataType::REFERENCE) {
        VisitSelectWithReference(v, inst);
    }
}

void GraphChecker::VisitSelectTransform(GraphVisitor *v, Inst *inst)
{
    VisitSelect(v, inst);
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, DataType::GetCommonType(inst->GetType()) == DataType::INT64,
        (std::cerr << "SelectTransform applies to integer types only\n", inst->Dump(&std::cerr)));
}

// Shared by SelectImm and SelectImmTransform
void GraphChecker::VisitSelectImmWithReference([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::SelectImm || inst->GetOpcode() == Opcode::SelectImmTransform);
    [[maybe_unused]] auto op1 = inst->GetInput(1).GetInst();
    [[maybe_unused]] auto op2 = inst->GetInput(2U).GetInst();
    [[maybe_unused]] uint64_t op3;
    [[maybe_unused]] ConditionCode cc;
    if (inst->GetOpcode() == Opcode::SelectImm) {
        op3 = inst->CastToSelectImm()->GetImm();
        cc = inst->CastToSelectImm()->GetCc();
    } else {
        op3 = inst->CastToSelectImmTransform()->GetImm();
        cc = inst->CastToSelectImmTransform()->GetCc();
    }

    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, cc == ConditionCode::CC_NE || cc == ConditionCode::CC_EQ,
        (std::cerr << "SelectImm reference comparison must be CC_NE or CC_EQ: \n", inst->Dump(&std::cerr)));
    if (op2->IsConst()) {
        CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, IsZeroConstant(op2),
                                            (std::cerr << "Constant reference input must be integer 0: \n",
                                             inst->Dump(&std::cerr), op1->Dump(&std::cerr)));
    } else {
        CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
            v, op2->GetType() == DataType::REFERENCE,
            (std::cerr << "Condition with immediate jump 1st operand type is not a reference\n", inst->Dump(&std::cerr),
             op1->Dump(&std::cerr)));
    }
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, op3 == 0, (std::cerr << "Reference can be compared only with 0 immediate: \n", inst->Dump(&std::cerr)));
}

// Shared by SelectImm and SelectImmTransform
void GraphChecker::VisitSelectImmNotReference([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::SelectImm || inst->GetOpcode() == Opcode::SelectImmTransform);
    [[maybe_unused]] auto op2 = inst->GetInput(2U).GetInst();
    [[maybe_unused]] bool isDynamic = static_cast<GraphChecker *>(v)->GetGraph()->IsDynamicMethod();

    CHECKER_MESSAGE_IF_NOT_AND_PRINT_VISITOR(
        v,
        DataType::GetCommonType(op2->GetType()) == DataType::INT64 ||
            (isDynamic && DataType::GetCommonType(op2->GetType()) == DataType::ANY),
        "SelectImm 3rd operand type is not an integer or any");

    if (DataType::GetCommonType(op2->GetType()) == DataType::ANY) {
        [[maybe_unused]] ConditionCode cc;
        if (inst->GetOpcode() == Opcode::SelectImm) {
            cc = inst->CastToSelectImm()->GetCc();
        } else {
            cc = inst->CastToSelectImmTransform()->GetCc();
        }
        CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
            v, cc == ConditionCode::CC_NE || cc == ConditionCode::CC_EQ,
            (std::cerr << "SelectImm any comparison must be CC_NE or CC_EQ: \n", inst->Dump(&std::cerr)));
    }
}

void GraphChecker::VisitSelectImm([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
{
    [[maybe_unused]] auto op0 = inst->GetInput(0).GetInst();
    [[maybe_unused]] auto op1 = inst->GetInput(1).GetInst();
    [[maybe_unused]] bool isDynamic = static_cast<GraphChecker *>(v)->GetGraph()->IsDynamicMethod();

    for (size_t i = 0; i < inst->GetInputsCount(); i++) {
        CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, inst->GetInputType(i) != DataType::NO_TYPE,
                                            std::cerr << "Source operand type is not set: " << *inst << std::endl);
    }

    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v,
        DataType::GetCommonType(inst->GetType()) == DataType::INT64 || inst->GetType() == DataType::REFERENCE ||
            IsFloatType(DataType::GetCommonType(inst->GetType())) || (isDynamic && inst->GetType() == DataType::ANY),
        (std::cerr << "SelectImm instruction type is not integer or reference or any", inst->Dump(&std::cerr)));
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v,
        DataType::GetCommonType(op0->GetType()) == DataType::INT64 ||
            IsFloatType(DataType::GetCommonType(op0->GetType())) || op0->GetType() == DataType::REFERENCE ||
            (isDynamic && op0->GetType() == DataType::ANY),
        (std::cerr << "SelectImm instruction 1st operand type is not integer or reference or any",
         inst->Dump(&std::cerr)));
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v,
        DataType::GetCommonType(op1->GetType()) == DataType::INT64 ||
            IsFloatType(DataType::GetCommonType(op1->GetType())) || op1->GetType() == DataType::REFERENCE ||
            (isDynamic && op1->GetType() == DataType::ANY),
        (std::cerr << "SelectImm instruction 2nd operand type is not integer or reference or any",
         inst->Dump(&std::cerr)));

    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, CheckCommonTypes(op0, op1),
        (std::cerr << "Types of two first SelectImm instruction operands are not compatible\n", op0->Dump(&std::cerr),
         op1->Dump(&std::cerr), inst->Dump(&std::cerr)));
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, CheckCommonTypes(inst, op0),
        (std::cerr << "Types of instruction result and its operands are not compatible\n", inst->Dump(&std::cerr)));

    if (inst->GetInputType(2U) == DataType::REFERENCE) {
        VisitSelectImmWithReference(v, inst);
    } else {
        VisitSelectImmNotReference(v, inst);
    }
}

void GraphChecker::VisitSelectImmTransform(GraphVisitor *v, Inst *inst)
{
    VisitSelectImm(v, inst);
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, DataType::GetCommonType(inst->GetType()) == DataType::INT64,
        (std::cerr << "SelectImmTransform applies to integer types only\n", inst->Dump(&std::cerr)));
}

void GraphChecker::VisitIf(GraphVisitor *v, Inst *inst)
{
    CheckContrlFlowInst(v, inst);
    [[maybe_unused]] auto numSuccs = inst->GetBasicBlock()->GetSuccsBlocks().size();
    CHECKER_MESSAGE_IF_NOT_AND_PRINT_VISITOR(v, numSuccs == MAX_SUCCS_NUM,
                                             "Basic block with If must have 2 successesors");

    [[maybe_unused]] auto op1 = inst->GetInputs()[0].GetInst();
    [[maybe_unused]] auto op2 = inst->GetInputs()[1].GetInst();
    for (size_t i = 0; i < inst->GetInputsCount(); i++) {
        CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, inst->GetInputType(i) != DataType::NO_TYPE,
                                            std::cerr << "Source operand type is not set: " << *inst << std::endl);
    }
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, inst->GetInputType(0) == inst->GetInputType(1),
                                        std::cerr << "If has different inputs type: " << *inst << std::endl);
    if (inst->GetInputType(0) == DataType::REFERENCE) {
        [[maybe_unused]] auto cc = inst->CastToIf()->GetCc();
        CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
            v, cc == ConditionCode::CC_NE || cc == ConditionCode::CC_EQ,
            (std::cerr << "Reference comparison in If must be CC_NE or CC_EQ: \n", inst->Dump(&std::cerr)));
        if (op1->IsConst()) {
            CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, IsZeroConstant(op1),
                                                (std::cerr << "Constant reference input must be integer 0: \n",
                                                 inst->Dump(&std::cerr), op1->Dump(&std::cerr)));
        } else {
            CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, op1->GetType() == DataType::REFERENCE,
                                                (std::cerr << "If 1st operand type is not a reference\n",
                                                 inst->Dump(&std::cerr), op1->Dump(&std::cerr)));
        }
        if (op2->IsConst()) {
            CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, IsZeroConstant(op2),
                                                (std::cerr << "Constant reference input must be integer 0: \n",
                                                 inst->Dump(&std::cerr), op2->Dump(&std::cerr)));
        } else {
            CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, op2->GetType() == DataType::REFERENCE,
                                                (std::cerr << "If 2nd operand type is not a reference\n",
                                                 inst->Dump(&std::cerr), op2->Dump(&std::cerr)));
        }
    }
}

void GraphChecker::VisitIfImm(GraphVisitor *v, Inst *inst)
{
    CheckContrlFlowInst(v, inst);
    [[maybe_unused]] auto numSuccs = inst->GetBasicBlock()->GetSuccsBlocks().size();
    CHECKER_MESSAGE_IF_NOT_AND_PRINT_VISITOR(v, numSuccs == MAX_SUCCS_NUM,
                                             "Basic block with IfImm must have 2 successesors");

    [[maybe_unused]] auto op1 = inst->GetInput(0).GetInst();
    [[maybe_unused]] auto op2 = inst->CastToIfImm()->GetImm();
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, inst->GetInputType(0) != DataType::NO_TYPE,
                                        std::cerr << "Source operand type is not set: " << *inst << std::endl);
    if (inst->GetInputType(0) == DataType::REFERENCE) {
        [[maybe_unused]] auto cc = inst->CastToIfImm()->GetCc();
        CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
            v, cc == ConditionCode::CC_NE || cc == ConditionCode::CC_EQ,
            (std::cerr << "Reference comparison in IfImm must have CC_NE or CC_EQ: \n", inst->Dump(&std::cerr)));
        if (op1->IsConst()) {
            CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, IsZeroConstant(op1),
                                                (std::cerr << "Constant reference input must be integer 0: \n",
                                                 inst->Dump(&std::cerr), op1->Dump(&std::cerr)));
        } else {
            CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, op1->GetType() == DataType::REFERENCE,
                                                (std::cerr << "IfImm operand type should be here a reference: \n",
                                                 inst->Dump(&std::cerr), op1->Dump(&std::cerr)));
        }
        CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
            v, op2 == 0, (std::cerr << "Reference can be compared only with 0 immediate: \n", inst->Dump(&std::cerr)));
    } else {
        CHECKER_MESSAGE_IF_NOT_AND_PRINT_VISITOR(
            v,
            DataType::GetCommonType(op1->GetType()) == DataType::INT64 ||
                (static_cast<GraphChecker *>(v)->GetGraph()->IsDynamicMethod() && op1->GetType() == DataType::ANY),
            "IfImm operand type should be here an integer");
    }
}

void GraphChecker::VisitTry([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    [[maybe_unused]] auto bb = inst->GetBasicBlock();
    CHECKER_MESSAGE_IF_NOT_AND_PRINT_VISITOR(v, bb->IsTryBegin(),
                                             "TryInst should be placed in the try-begin basic block");
}

void GraphChecker::VisitNOP([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
{
    CHECKER_MESSAGE_IF_NOT_AND_PRINT_VISITOR(v, inst->GetUsers().Empty(), "NOP can not have users\n");
}

void GraphChecker::VisitAndNot([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    CheckBinaryOperationTypes(v, inst, true);
}
void GraphChecker::VisitOrNot([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    CheckBinaryOperationTypes(v, inst, true);
}
void GraphChecker::VisitXorNot([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    CheckBinaryOperationTypes(v, inst, true);
}
void GraphChecker::VisitMNeg([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    CheckBinaryOperationTypes(v, inst, false);
}
void GraphChecker::VisitMAdd(GraphVisitor *v, Inst *inst)
{
    CheckTernaryOperationTypes(v, inst);
}
void GraphChecker::VisitMSub(GraphVisitor *v, Inst *inst)
{
    CheckTernaryOperationTypes(v, inst);
}

void GraphChecker::VisitCompareAnyType([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
{
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, static_cast<GraphChecker *>(v)->GetGraph()->IsDynamicMethod(),
        (std::cerr << "CompareAnyType is supported only for dynamic languages:\n", inst->Dump(&std::cerr)));
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, inst->CastToCompareAnyType()->GetAllowedInputType() == profiling::AnyInputType::DEFAULT,
        (std::cerr << "CompareAnyType doesn't support special values or treating int as double\n",
         inst->Dump(&std::cerr)));
}

void GraphChecker::VisitCastAnyTypeValue([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
{
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, static_cast<GraphChecker *>(v)->GetGraph()->IsDynamicMethod(),
        (std::cerr << "CastAnyTypeValue is supported only for dynamic languages:\n", inst->Dump(&std::cerr)));
    if ((inst->CastToCastAnyTypeValue()->GetAllowedInputType() & profiling::AnyInputType::INTEGER) != 0) {
        [[maybe_unused]] auto type = AnyBaseTypeToDataType(inst->CastToCastAnyTypeValue()->GetAnyType());
        CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
            v, type == DataType::FLOAT64,
            (std::cerr << "Target type of CastAnyTypeValue with Integer allowed must be FLOAT64\n",
             inst->Dump(&std::cerr)));
    } else if (inst->CastToCastAnyTypeValue()->GetAllowedInputType() != profiling::AnyInputType::DEFAULT) {
        [[maybe_unused]] auto type = AnyBaseTypeToDataType(inst->CastToCastAnyTypeValue()->GetAnyType());
        CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
            v, type == DataType::FLOAT64 || type == DataType::INT32,
            (std::cerr << "Target type of CastAnyTypeValue with special value allowed must be FLOAT64 or INT32\n",
             inst->Dump(&std::cerr)));
    }
}

void GraphChecker::VisitCastValueToAnyType([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
{
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, static_cast<GraphChecker *>(v)->GetGraph()->IsDynamicMethod(),
        (std::cerr << "CastValueToAnyType is supported only for dynamic languages:\n", inst->Dump(&std::cerr)));

    const auto *inputInst = inst->GetInput(0).GetInst();
    auto inputType = inst->GetInputType(0);
    auto outputType = AnyBaseTypeToDataType(inst->CastToCastValueToAnyType()->GetAnyType());

    // CHECKER_DO_IF_NOT_AND_PRINT(input_type != DataType::ANY, // NOTE(vpukhov): AnyConst
    //          (std::cerr << "CastValueToAnyType cannot accept inputs of ANY type:\n", inst->Dump(&std::cerr)));

    if (inputInst->IsConst() && (inputType == DataType::Type::INT64 || inputType == DataType::Type::INT32)) {
        if (outputType == DataType::Type::BOOL) {
            CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
                v, inputInst->IsBoolConst(),
                (std::cerr << "Integral constant input not coercible to BOOL:\n", inst->Dump(&std::cerr)));
            return;
        }

        if (outputType == DataType::INT32 && inputType == DataType::INT64) {
            [[maybe_unused]] auto value = static_cast<int64_t>(inputInst->CastToConstant()->GetInt64Value());
            CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
                v, value == static_cast<int32_t>(value),
                (std::cerr << "Integral constant input not coercible to INT32:\n", inst->Dump(&std::cerr)));
            return;
        }

        if (outputType == DataType::Type::REFERENCE) {
            return;  // Always coercible
        }

        if (outputType == DataType::Type::VOID) {
            return;  // Always coercible
        }

        // Otherwise proceed with the generic check.
    }
}

void GraphChecker::VisitAnyTypeCheck([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
{
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, static_cast<GraphChecker *>(v)->GetGraph()->IsDynamicMethod(),
        (std::cerr << "AnyTypeCheck is supported only for dynamic languages:\n", inst->Dump(&std::cerr)));
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, inst->GetInput(0).GetInst()->GetType() == DataType::Type::ANY,
        (std::cerr << "First input in AnyTypeCheck must be Any type:\n", inst->Dump(&std::cerr)));
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, inst->GetInput(1).GetInst()->IsSaveState(),
        (std::cerr << "Second input in AnyTypeCheck must be SaveState:\n", inst->Dump(&std::cerr)));
    if ((inst->CastToAnyTypeCheck()->GetAllowedInputType() & profiling::AnyInputType::INTEGER) != 0) {
        [[maybe_unused]] auto type = AnyBaseTypeToDataType(inst->CastToAnyTypeCheck()->GetAnyType());
        CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
            v, type == DataType::FLOAT64,
            (std::cerr << "Target type of AnyTypeCheck with Integer allowed must be FLOAT64\n",
             inst->Dump(&std::cerr)));
    } else if (inst->CastToAnyTypeCheck()->GetAllowedInputType() != profiling::AnyInputType::DEFAULT) {
        [[maybe_unused]] auto type = AnyBaseTypeToDataType(inst->CastToAnyTypeCheck()->GetAnyType());
        CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
            v, type == DataType::FLOAT64 || type == DataType::INT32,
            (std::cerr << "Target type of AnyTypeCheck with special value allowed must be FLOAT64 or INT32\n",
             inst->Dump(&std::cerr)));
    }
}

void GraphChecker::VisitHclassCheck([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
{
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, static_cast<GraphChecker *>(v)->GetGraph()->IsDynamicMethod(),
        (std::cerr << "HclassCheck is supported only for dynamic languages:\n", inst->Dump(&std::cerr)));
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, inst->GetInput(0).GetInst()->GetType() == DataType::Type::REFERENCE,
        (std::cerr << "First input in HclassCheck must be Ref type:\n", inst->Dump(&std::cerr)));
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v,
        (inst->GetInput(0).GetInst()->GetOpcode() == Opcode::LoadObject &&
         inst->GetInput(0).GetInst()->CastToLoadObject()->GetObjectType() == ObjectType::MEM_DYN_HCLASS),
        (std::cerr << "First input in HclassCheck must be LoadObject Hclass:\n", inst->Dump(&std::cerr)));
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, inst->GetInput(1).GetInst()->IsSaveState(),
        (std::cerr << "Second input in HclassCheck must be SaveState:\n", inst->Dump(&std::cerr)));
    [[maybe_unused]] auto hclassCheck = inst->CastToHclassCheck();
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, hclassCheck->GetCheckIsFunction() || hclassCheck->GetCheckFunctionIsNotClassConstructor(),
        (std::cerr << "HclassCheck must have at least one check: IsFunction or FunctionIsNotClassConstructor\n",
         inst->Dump(&std::cerr)));
}

void GraphChecker::VisitBitcast([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
{
    [[maybe_unused]] auto arch = static_cast<GraphChecker *>(v)->GetGraph()->GetArch();
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v,
        DataType::GetTypeSize(inst->GetInput(0).GetInst()->GetType(), arch) ==
            DataType::GetTypeSize(inst->GetType(), arch),
        (std::cerr << "Size of input and output types must be equal\n", inst->Dump(&std::cerr)));
}

void GraphChecker::VisitLoadString([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
{
    CHECKER_IF_NOT_PRINT_VISITOR(v, !static_cast<GraphChecker *>(v)->GetGraph()->IsDynamicMethod() ||
                                        static_cast<GraphChecker *>(v)->GetGraph()->IsBytecodeOptimizer());
}

void GraphChecker::VisitLoadType([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
{
    CHECKER_IF_NOT_PRINT_VISITOR(v, !static_cast<GraphChecker *>(v)->GetGraph()->IsDynamicMethod());
}

void GraphChecker::VisitLoadUnresolvedType([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
{
    CHECKER_IF_NOT_PRINT_VISITOR(v, !static_cast<GraphChecker *>(v)->GetGraph()->IsDynamicMethod());
}

void GraphChecker::VisitLoadFromConstantPool([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
{
    CHECKER_IF_NOT_PRINT_VISITOR(v, static_cast<GraphChecker *>(v)->GetGraph()->IsDynamicMethod());
}

void GraphChecker::VisitLoadImmediate([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    auto loadImm = inst->CastToLoadImmediate();
    [[maybe_unused]] auto type = inst->GetType();
    [[maybe_unused]] auto objType = loadImm->GetObjectType();
    [[maybe_unused]] auto graph = static_cast<GraphChecker *>(v)->GetGraph();
    CHECKER_IF_NOT_PRINT_VISITOR(v, objType != LoadImmediateInst::ObjectType::UNKNOWN);
    if (!graph->IsJitOrOsrMode()) {
        CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
            v,
            objType == LoadImmediateInst::ObjectType::PANDA_FILE_OFFSET ||
                objType == LoadImmediateInst::ObjectType::TLS_OFFSET,
            (std::cerr << "Non-JIT/OSR LoadImmediate must be PANDA_FILE_OFFSET or TLS_OFFSET:\n",
             inst->Dump(&std::cerr)));
    }
    CHECKER_IF_NOT_PRINT_VISITOR(
        v, (type == DataType::REFERENCE &&
            (objType == LoadImmediateInst::ObjectType::CLASS || objType == LoadImmediateInst::ObjectType::OBJECT)) ||
               (type == DataType::POINTER &&
                (objType == LoadImmediateInst::ObjectType::METHOD || objType == LoadImmediateInst::ObjectType::STRING ||
                 objType == LoadImmediateInst::ObjectType::PANDA_FILE_OFFSET || loadImm->IsTlsOffset())) ||
               (type == DataType::ANY && objType == LoadImmediateInst::ObjectType::CONSTANT_POOL));
    CHECKER_IF_NOT_PRINT_VISITOR(v, objType != LoadImmediateInst::ObjectType::PANDA_FILE_OFFSET ||
                                        static_cast<GraphChecker *>(v)->GetGraph()->IsAotMode());
    CHECKER_IF_NOT_PRINT_VISITOR(v, loadImm->GetObject() != nullptr);
}

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define VISIT_BINARY_SHIFTED_REGISTER(opc)                                                      \
    void GraphChecker::Visit##opc(GraphVisitor *v, Inst *inst)                                  \
    {                                                                                           \
        CheckBinaryOperationWithShiftedOperandTypes(                                            \
            v, inst, inst->GetOpcode() != Opcode::AddSR && inst->GetOpcode() != Opcode::SubSR); \
    }

VISIT_BINARY_SHIFTED_REGISTER(AddSR)
VISIT_BINARY_SHIFTED_REGISTER(SubSR)
VISIT_BINARY_SHIFTED_REGISTER(AndSR)
VISIT_BINARY_SHIFTED_REGISTER(OrSR)
VISIT_BINARY_SHIFTED_REGISTER(XorSR)
VISIT_BINARY_SHIFTED_REGISTER(AndNotSR)
VISIT_BINARY_SHIFTED_REGISTER(OrNotSR)
VISIT_BINARY_SHIFTED_REGISTER(XorNotSR)
#undef VISIT_BINARY_SHIFTED_REGISTER

void GraphChecker::VisitExtractBitfield(GraphVisitor *v, Inst *inst)
{
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, DataType::GetCommonType(inst->GetType()) == DataType::INT64,
        (std::cerr << "ExtractBitfield must have integer type\n", inst->Dump(&std::cerr)));
    CheckUnaryOperationTypes(v, inst);

    ExtractBitfieldInst *bfx = inst->CastToExtractBitfield();
    auto arch = inst->GetBasicBlock()->GetGraph()->GetArch();
    [[maybe_unused]] auto sourceBit = bfx->GetSourceBit();
    [[maybe_unused]] auto width = bfx->GetWidth();
    [[maybe_unused]] auto sourceTypeSize = DataType::GetTypeSize(bfx->GetInput(0).GetInst()->GetType(), arch);

    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, sourceBit < sourceTypeSize,
        (std::cerr << "source bit of ExtractBitfield must be < size bits of input which is " << sourceTypeSize << "\n",
         inst->Dump(&std::cerr)));
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, width > 0, (std::cerr << "width of ExtractBitfield must be > 0\n", inst->Dump(&std::cerr)));
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, sourceBit + width <= sourceTypeSize,
        (std::cerr << "source bit + width of ExtractBitfield must be <= size bits of input which is " << sourceTypeSize
                   << "\n",
         inst->Dump(&std::cerr)));
}

void GraphChecker::VisitNegSR(GraphVisitor *v, [[maybe_unused]] Inst *inst)
{
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, DataType::GetCommonType(inst->GetType()) == DataType::INT64,
                                        (std::cerr << "NegSR must have integer type\n", inst->Dump(&std::cerr)));
    CheckUnaryOperationTypes(v, inst);
    [[maybe_unused]] auto shiftType = static_cast<UnaryShiftedRegisterOperation *>(inst)->GetShiftType();
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(v, shiftType != ShiftType::INVALID_SHIFT && shiftType != ShiftType::ROR,
                                        (std::cerr << "Operation has invalid shift type\n", inst->Dump(&std::cerr)));
}

void GraphChecker::VisitStringFlatCheck([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
{
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, inst->GetType() == DataType::REFERENCE,
        (std::cerr << "\nStringFlatCheck must have reference type\n", inst->Dump(&std::cerr)));
    CHECKER_DO_IF_NOT_AND_PRINT_VISITOR(
        v, inst->GetInput(0).GetInst()->GetType() == DataType::REFERENCE,
        (std::cerr << "\nStringFlatCheck input must have reference type\n", inst->Dump(&std::cerr)));
}
}  // namespace ark::compiler
