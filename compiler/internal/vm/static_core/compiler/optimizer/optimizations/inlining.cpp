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

#include "inlining.h"
#include <cstddef>
#include "compiler_logger.h"
#include "compiler_options.h"
#include "libarkbase/generated/events_gen.h"
#include "optimizer/ir/analysis.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir_builder/ir_builder.h"
#include "optimizer/analysis/alias_analysis.h"
#include "optimizer/analysis/rpo.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/analysis/object_type_propagation.h"
#include "optimizer/optimizations/cleanup.h"
#include "optimizer/optimizations/branch_elimination.h"
#include "optimizer/optimizations/object_type_check_elimination.h"
#include "optimizer/optimizations/optimize_string_concat.h"
#include "optimizer/optimizations/peepholes.h"
#include "optimizer/optimizations/simplify_string_builder.h"
#include "runtime/include/class.h"
#include "libarkbase/events/events.h"

namespace ark::compiler {
using MethodPtr = RuntimeInterface::MethodPtr;

// Explicitly instantiate both versions of CheckMethodCanBeInlined since they can be used in subclasses
template bool Inlining::CheckMethodCanBeInlined<false, true>(const CallInst *, InlineContext *);
template bool Inlining::CheckMethodCanBeInlined<true, true>(const CallInst *, InlineContext *);
template bool Inlining::CheckMethodCanBeInlined<false, false>(const CallInst *, InlineContext *);
template bool Inlining::CheckMethodCanBeInlined<true, false>(const CallInst *, InlineContext *);

inline bool CanReplaceWithCallStatic(Opcode opcode)
{
    switch (opcode) {
        case Opcode::CallResolvedVirtual:
        case Opcode::CallVirtual:
            return true;
        default:
            return false;
    }
}

size_t Inlining::CalculateInstructionsCount(Graph *graph)
{
    size_t count = 0;
    for (auto bb : *graph) {
        if (bb == nullptr || bb->IsStartBlock() || bb->IsEndBlock()) {
            continue;
        }
        for (auto inst : bb->Insts()) {
            if (inst->IsSaveState() || inst->IsPhi()) {
                continue;
            }
            switch (inst->GetOpcode()) {
                case Opcode::Return:
                case Opcode::ReturnI:
                case Opcode::ReturnVoid:
                    break;
                default:
                    count++;
            }
        }
    }
    return count;
}

Inlining::Inlining(Graph *graph, uint32_t instructionsCount, uint32_t methodsInlined,
                   const ArenaVector<RuntimeInterface::MethodPtr> *inlinedStack)
    : Optimization(graph),
      methodsInlined_(methodsInlined),
      instructionsCount_(instructionsCount != 0 ? instructionsCount : CalculateInstructionsCount(graph)),
      instructionsLimit_(g_options.GetCompilerInliningMaxInsts()),
      returnBlocks_(graph->GetLocalAllocator()->Adapter()),
      blacklist_(graph->GetLocalAllocator()->Adapter()),
      inlinedStack_(graph->GetLocalAllocator()->Adapter()),
      vregsCount_(graph->GetVRegsCount()),
      cha_(graph->GetRuntime()->GetCha())
{
    if (inlinedStack != nullptr) {
        inlinedStack_.reserve(inlinedStack->size() + 1U);
        inlinedStack_ = *inlinedStack;
    } else {
        inlinedStack_.reserve(1U);
    }
    inlinedStack_.push_back(graph->GetMethod());
}

bool Inlining::IsInlineCachesEnabled() const
{
    return DoesArchSupportDeoptimization(GetGraph()->GetArch()) && !g_options.IsCompilerNoPicInlining();
}

#ifdef PANDA_EVENTS_ENABLED
static void EmitEvent(const Graph *graph, const CallInst *callInst, const InlineContext &ctx, events::InlineResult res)
{
    auto runtime = graph->GetRuntime();
    events::InlineKind kind;
    if (ctx.chaDevirtualize) {
        kind = events::InlineKind::VIRTUAL_CHA;
    } else if (CanReplaceWithCallStatic(callInst->GetOpcode()) || ctx.replaceToStatic) {
        kind = events::InlineKind::VIRTUAL;
    } else {
        kind = events::InlineKind::STATIC;
    }
    EVENT_INLINE(runtime->GetMethodFullName(graph->GetMethod()),
                 ctx.method != nullptr ? runtime->GetMethodFullName(ctx.method) : "null", callInst->GetId(), kind, res);
}
#else
// NOLINTNEXTLINE(readability-named-parameter)
static void EmitEvent(const Graph *, const CallInst *, const InlineContext &, events::InlineResult) {}
#endif

bool Inlining::RunImpl()
{
    GetGraph()->RunPass<LoopAnalyzer>();

    auto blacklistNames = g_options.GetCompilerInliningBlacklist();
    blacklist_.reserve(blacklistNames.size());

    for (const auto &methodName : blacklistNames) {
        blacklist_.insert(methodName);
    }
    return Do();
}

void Inlining::RunOptimizations() const
{
    if (GetGraph()->GetParentGraph() == nullptr && GetGraph()->RunPass<ObjectTypeCheckElimination>() &&
        GetGraph()->RunPass<Peepholes>()) {
        GetGraph()->RunPass<BranchElimination>();
    }
}

bool Inlining::Do()
{
    bool inlined = false;
    RunOptimizations();

    ArenaVector<BasicBlock *> hotBlocks(GetGraph()->GetLocalAllocator()->Adapter());

    hotBlocks = GetGraph()->GetVectorBlocks();
    if (GetGraph()->IsAotMode()) {
        std::stable_sort(hotBlocks.begin(), hotBlocks.end(), [](BasicBlock *a, BasicBlock *b) {
            auto ad = (a == nullptr) ? 0 : a->GetLoop()->GetDepth();
            auto bd = (b == nullptr) ? 0 : b->GetLoop()->GetDepth();
            return ad > bd;
        });
    } else {
        std::stable_sort(hotBlocks.begin(), hotBlocks.end(), [](BasicBlock *a, BasicBlock *b) {
            auto ahtn = (a == nullptr) ? 0 : a->GetHotness();
            auto bhtn = (b == nullptr) ? 0 : b->GetHotness();
            return ahtn > bhtn;
        });
    }

    LOG_INLINING(DEBUG) << "Process blocks (" << hotBlocks.size() << " blocks):";
    auto lastId = hotBlocks.size();
    for (size_t i = 0; i < lastId; i++) {
        if (SkipBlock(hotBlocks[i])) {
            LOG_INLINING(DEBUG) << "-skip (" << i << '/' << hotBlocks.size() << ")";
            continue;
        }

        [[maybe_unused]] auto hotIdx = std::find(hotBlocks.begin(), hotBlocks.end(), hotBlocks[i]) - hotBlocks.begin();
        [[maybe_unused]] auto bbId = hotBlocks[i]->GetId();
        LOG_INLINING(DEBUG) << "-process BB" << bbId << " (" << i << '/' << hotBlocks.size() << "): htn_"
                            << hotBlocks[i]->GetHotness() << " (" << hotIdx << '/' << hotBlocks.size() << ")";

        for (auto inst : hotBlocks[i]->InstsSafe()) {
            if (GetGraph()->GetVectorBlocks()[inst->GetBasicBlock()->GetId()] == nullptr) {
                break;
            }

            if (!IsInstSuitableForInline(inst)) {
                continue;
            }

            inlined |= TryInline(static_cast<CallInst *>(inst));
        }
    }

#ifndef NDEBUG
    GetGraph()->SetInliningComplete();
#endif  // NDEBUG

    return inlined;
}

bool Inlining::IsInstSuitableForInline(Inst *inst) const
{
    if (!inst->IsCall()) {
        return false;
    }
    auto callInst = static_cast<CallInst *>(inst);
    ASSERT(!callInst->IsDynamicCall());
    if (callInst->IsInlined()) {
        return false;
    }
    if (callInst->IsUnresolved() || callInst->GetCallMethod() == nullptr) {
        LOG_INLINING(DEBUG) << "Unknown method " << callInst->GetCallMethodId();
        return false;
    }
    ASSERT(callInst->GetCallMethod() != nullptr);
    return true;
}

void Inlining::InvalidateAnalyses()
{
    GetGraph()->InvalidateAnalysis<BoundsAnalysis>();
    GetGraph()->InvalidateAnalysis<AliasAnalysis>();
    GetGraph()->InvalidateAnalysis<LoopAnalyzer>();
    GetGraph()->InvalidateAnalysis<ObjectTypePropagation>();
    InvalidateBlocksOrderAnalyzes(GetGraph());
}

/**
 * Get next Parameter instruction.
 * NOTE(msherstennikov): this is temporary solution, need to find out better approach
 * @param inst current param instruction
 * @return next Parameter instruction, nullptr if there no more Parameter instructions
 */
Inst *GetNextParam(Inst *inst)
{
    for (auto nextInst = inst->GetNext(); nextInst != nullptr; nextInst = nextInst->GetNext()) {
        if (nextInst->GetOpcode() == Opcode::Parameter) {
            return nextInst;
        }
    }
    return nullptr;
}

bool Inlining::TryInline(CallInst *callInst)
{
    InlineContext ctx;
    if (!ResolveTarget(callInst, &ctx)) {
        if (IsInlineCachesEnabled()) {
            return TryInlineWithInlineCaches(callInst);
        }
        return false;
    }

    ASSERT(!callInst->IsInlined());

    LOG_INLINING(DEBUG) << "Try to inline(id=" << callInst->GetId() << (callInst->IsVirtualCall() ? ", virtual" : "")
                        << ", size=" << GetGraph()->GetRuntime()->GetMethodCodeSize(ctx.method) << ", vregs="
                        << GetGraph()->GetRuntime()->GetMethodArgumentsCount(ctx.method) +
                               GetGraph()->GetRuntime()->GetMethodRegistersCount(ctx.method) + 1
                        << (ctx.chaDevirtualize ? ", CHA" : "")
                        << "): " << GetGraph()->GetRuntime()->GetMethodFullName(ctx.method, true);

    if (DoInline(callInst, &ctx)) {
        if (IsIntrinsic(&ctx)) {
            callInst->GetBasicBlock()->RemoveInst(callInst);
        }
        EmitEvent(GetGraph(), callInst, ctx, events::InlineResult::SUCCESS);
        return true;
    }
    if (ctx.replaceToStatic && !ctx.chaDevirtualize) {
        ASSERT(ctx.method != nullptr);
        if (callInst->GetCallMethod() != ctx.method) {
            // Replace method id only if the methods are different.
            // Otherwise, leave the method id as is because:
            // 1. In aot mode the new method id can refer to a method from a different file
            // 2. In jit mode the method id does not matter
            callInst->SetCallMethodId(GetGraph()->GetRuntime()->GetMethodId(ctx.method));
        }
        callInst->SetCallMethod(ctx.method);
        if (callInst->GetOpcode() == Opcode::CallResolvedVirtual) {
            // Drop the first argument - the resolved method
            ASSERT(!callInst->GetInputs().empty());
            for (size_t i = 0; i < callInst->GetInputsCount() - 1; i++) {
                callInst->SetInput(i, callInst->GetInput(i + 1).GetInst());
            }
            callInst->RemoveInput(callInst->GetInputsCount() - 1);
            ASSERT(!callInst->GetInputTypes()->empty());
            callInst->GetInputTypes()->erase(callInst->GetInputTypes()->begin());
        }
        callInst->SetOpcode(Opcode::CallStatic);
        EmitEvent(GetGraph(), callInst, ctx, events::InlineResult::DEVIRTUALIZED);
        return true;
    }

    return false;
}

bool Inlining::TryInlineWithInlineCaches(CallInst *callInst)
{
    auto runtime = GetGraph()->GetRuntime();
    auto pic = runtime->GetInlineCaches();
    if (pic == nullptr) {
        return false;
    }

    ArenaVector<RuntimeInterface::ClassPtr> receivers(GetGraph()->GetLocalAllocator()->Adapter());
    auto callKind = pic->GetClasses(GetGraph()->GetMethod(), callInst->GetPc(), &receivers);

    if (GetGraph()->IsAotMode() && !g_options.IsCompilerInlineExternalMethodsAot()) {
        for (auto receiver : receivers) {
            if (runtime->IsClassExternal(GetGraph()->GetMethod(), receiver)) {
                LOG_INLINING(INFO) << "AOT inline caches external inlining is not supported";
                EVENT_INLINE(runtime->GetMethodFullName(GetGraph()->GetMethod()), runtime->GetClassName(receiver),
                             callInst->GetId(), events::InlineKind::VIRTUAL, events::InlineResult::SKIP_EXTERNAL);
                return false;
                // We don't support offline external inline caches yet
            }
        }
    }

    switch (callKind) {
        case InlineCachesInterface::CallKind::MEGAMORPHIC:
            EVENT_INLINE(runtime->GetMethodFullName(GetGraph()->GetMethod()), "-", callInst->GetId(),
                         events::InlineKind::VIRTUAL_POLYMORPHIC, events::InlineResult::FAIL_MEGAMORPHIC);
            return false;
        case InlineCachesInterface::CallKind::UNKNOWN:
            return false;
        case InlineCachesInterface::CallKind::MONOMORPHIC:
            return DoInlineMonomorphic(callInst, receivers[0]);
        case InlineCachesInterface::CallKind::POLYMORPHIC:
            ASSERT(callKind == InlineCachesInterface::CallKind::POLYMORPHIC);
            return DoInlinePolymorphic(callInst, &receivers);
        default:
            break;
    }
    return false;
}

bool Inlining::DoInlineMonomorphic(CallInst *callInst, RuntimeInterface::ClassPtr receiver)
{
    auto runtime = GetGraph()->GetRuntime();
    auto currMethod = GetGraph()->GetMethod();

    InlineContext ctx;
    ctx.method = runtime->ResolveVirtualMethod(receiver, callInst->GetCallMethod());

    auto callBb = callInst->GetBasicBlock();
    auto saveState = callInst->GetSaveState();
    auto objInst = callInst->GetObjectInst();

    LOG_INLINING(DEBUG) << "Try to inline monomorphic(size=" << runtime->GetMethodCodeSize(ctx.method)
                        << "): " << GetMethodFullName(GetGraph(), ctx.method);

    if (!DoInline(callInst, &ctx)) {
        return false;
    }

    // Add type guard
    auto getClsInst = GetGraph()->CreateInstGetInstanceClass(DataType::REFERENCE, callInst->GetPc(), objInst);

    Inst *loadClsInst = nullptr;
    if (GetGraph()->IsAotMode()) {
        auto classId = runtime->GetClassIdWithinFile(currMethod, receiver);
        if (classId == 0) {
            classId = runtime->GetClassIdWithinFile(currMethod, runtime->GetClass(currMethod));
        }
        ASSERT(classId != 0);
        loadClsInst = GetGraph()->CreateInstLoadClass(DataType::REFERENCE, callInst->GetPc(), saveState,
                                                      TypeIdMixin {classId, currMethod}, receiver);
    } else {
        loadClsInst = GetGraph()->CreateInstLoadImmediate(DataType::REFERENCE, callInst->GetPc(), receiver);
    }

    auto cmpInst = GetGraph()->CreateInstCompare(DataType::BOOL, callInst->GetPc(), getClsInst, loadClsInst,
                                                 DataType::REFERENCE, ConditionCode::CC_NE);
    auto deoptInst =
        GetGraph()->CreateInstDeoptimizeIf(callInst->GetPc(), cmpInst, saveState, DeoptimizeType::INLINE_IC);
    if (IsIntrinsic(&ctx)) {
        callInst->InsertBefore(loadClsInst);
        callInst->InsertBefore(getClsInst);
        callInst->InsertBefore(cmpInst);
        callInst->InsertBefore(deoptInst);
        callInst->GetBasicBlock()->RemoveInst(callInst);
    } else {
        callBb->AppendInst(loadClsInst);
        callBb->AppendInst(getClsInst);
        callBb->AppendInst(cmpInst);
        callBb->AppendInst(deoptInst);
    }

    EVENT_INLINE(runtime->GetMethodFullName(currMethod), runtime->GetMethodFullName(ctx.method), callInst->GetId(),
                 events::InlineKind::VIRTUAL_MONOMORPHIC, events::InlineResult::SUCCESS);
    return true;
}

SaveStateInst *Inlining::GetOrCloneSaveState(CallInst *callInst, BasicBlock *callBb)
{
    auto saveState = callInst->GetSaveState();
    ASSERT(saveState != nullptr);

    if (callBb == callInst->GetBasicBlock()) {
        return saveState;
    }

    auto clonedSS = CopySaveState(GetGraph(), saveState);
    callBb->AppendInst(clonedSS);

    return clonedSS;
}

void Inlining::CreateCompareClass(CallInst *callInst, Inst *getClsInst, RuntimeInterface::ClassPtr receiver,
                                  BasicBlock *callBb)
{
    auto runtime = GetGraph()->GetRuntime();
    auto currMethod = GetGraph()->GetMethod();

    Inst *loadClsInst = nullptr;
    if (GetGraph()->IsAotMode()) {
        auto saveState = GetOrCloneSaveState(callInst, callBb);
        auto classId = runtime->GetClassIdWithinFile(currMethod, receiver);
        if (classId == 0) {
            classId = runtime->GetClassIdWithinFile(currMethod, runtime->GetClass(currMethod));
        }
        ASSERT(classId != 0);
        loadClsInst = GetGraph()->CreateInstLoadClass(DataType::REFERENCE, callInst->GetPc(), saveState,
                                                      TypeIdMixin {classId, currMethod}, receiver);
    } else {
        loadClsInst = GetGraph()->CreateInstLoadImmediate(DataType::REFERENCE, callInst->GetPc(), receiver);
    }

    auto cmpInst = GetGraph()->CreateInstCompare(DataType::BOOL, callInst->GetPc(), loadClsInst, getClsInst,
                                                 DataType::REFERENCE, ConditionCode::CC_EQ);
    auto ifInst = GetGraph()->CreateInstIfImm(DataType::BOOL, callInst->GetPc(), cmpInst, 0, DataType::BOOL,
                                              ConditionCode::CC_NE);
    callBb->AppendInst(loadClsInst);
    callBb->AppendInst(cmpInst);
    callBb->AppendInst(ifInst);
}

void Inlining::InsertDeoptimizeInst(CallInst *callInst, BasicBlock *callBb, DeoptimizeType deoptType)
{
    // If last class compare returns false we need to deoptimize the method.
    // So we construct instruction DeoptimizeIf and insert instead of IfImm inst.
    auto ifInst = callBb->GetLastInst();
    ASSERT(ifInst != nullptr && ifInst->GetOpcode() == Opcode::IfImm);
    ASSERT(ifInst->CastToIfImm()->GetImm() == 0 && ifInst->CastToIfImm()->GetCc() == ConditionCode::CC_NE);

    auto compareInst = ifInst->GetInput(0).GetInst()->CastToCompare();
    ASSERT(compareInst != nullptr && compareInst->GetCc() == ConditionCode::CC_EQ);
    compareInst->SetCc(ConditionCode::CC_NE);

    SaveStateInst *saveState = nullptr;
    if (GetGraph()->IsAotMode()) {
        auto loadClsInst = compareInst->GetInput(0).GetInst();
        ASSERT(loadClsInst != nullptr && loadClsInst->GetOpcode() == Opcode::LoadClass);
        saveState = loadClsInst->GetSaveState();
    } else {
        saveState = GetOrCloneSaveState(callInst, callBb);
    }
    ASSERT(saveState != nullptr);

    auto deoptInst = GetGraph()->CreateInstDeoptimizeIf(callInst->GetPc(), compareInst, saveState, deoptType);

    callBb->RemoveInst(ifInst);
    callBb->AppendInst(deoptInst);
}

void Inlining::InsertCallInst(CallInst *callInst, BasicBlock *callBb, BasicBlock *retBb, Inst *phiInst)
{
    ASSERT(phiInst == nullptr || phiInst->GetBasicBlock() == retBb);
    // Insert new BB
    auto newCallBb = GetGraph()->CreateEmptyBlock(callBb);
    callBb->GetLoop()->AppendBlock(newCallBb);
    callBb->AddSucc(newCallBb);
    newCallBb->AddSucc(retBb);

    // Copy SaveState inst
    auto ss = callInst->GetSaveState();
    auto cloneSs = CopySaveState(GetGraph(), ss);
    newCallBb->AppendInst(cloneSs);

    // Copy Call inst
    auto cloneCall = callInst->Clone(GetGraph());
    for (auto input : callInst->GetInputs()) {
        cloneCall->AppendInput(input.GetInst());
    }
    cloneCall->SetSaveState(cloneSs);
    newCallBb->AppendInst(cloneCall);

    // Set return value in phi inst
    if (phiInst != nullptr) {
        phiInst->AppendInput(cloneCall);
    }
}

void Inlining::UpdateParameterDataflow(Graph *graphInl, Inst *callInst)
{
    // Replace inlined graph incoming dataflow edges
    auto startBb = graphInl->GetStartBlock();
    // Last input is SaveState
    if (callInst->GetInputsCount() > 1) {
        Inst *paramInst = *startBb->Insts().begin();
        if (paramInst != nullptr && paramInst->GetOpcode() != Opcode::Parameter) {
            paramInst = GetNextParam(paramInst);
        }
        while (paramInst != nullptr) {
            ASSERT(paramInst);
            auto argNum = paramInst->CastToParameter()->GetArgNumber();
            if (callInst->GetOpcode() == Opcode::CallResolvedVirtual ||
                callInst->GetOpcode() == Opcode::CallResolvedStatic) {
                ASSERT(argNum != ParameterInst::DYNAMIC_NUM_ARGS);
                argNum += 1;  // skip method_reg
            }
            Inst *input = nullptr;
            if (argNum < callInst->GetInputsCount() - 1) {
                input = callInst->GetInput(argNum).GetInst();
            } else if (argNum == ParameterInst::DYNAMIC_NUM_ARGS) {
                input = GetGraph()->FindOrCreateConstant(callInst->GetInputsCount() - 1);
            } else {
                input = GetGraph()->FindOrCreateConstant(DataType::Any(coretypes::TaggedValue::VALUE_UNDEFINED));
            }
            paramInst->ReplaceUsers(input);
            paramInst = GetNextParam(paramInst);
        }
    }
}

void UpdateExternalParameterDataflow(Graph *graphInl, Inst *callInst)
{
    // Replace inlined graph incoming dataflow edges
    auto startBb = graphInl->GetStartBlock();
    // Last input is SaveState
    if (callInst->GetInputsCount() <= 1) {
        return;
    }
    Inst *paramInst = *startBb->Insts().begin();
    if (paramInst != nullptr && paramInst->GetOpcode() != Opcode::Parameter) {
        paramInst = GetNextParam(paramInst);
    }
    ArenaVector<Inst *> worklist {graphInl->GetLocalAllocator()->Adapter()};
    while (paramInst != nullptr) {
        auto argNum = paramInst->CastToParameter()->GetArgNumber();
        ASSERT(argNum != ParameterInst::DYNAMIC_NUM_ARGS);
        if (callInst->GetOpcode() == Opcode::CallResolvedVirtual ||
            callInst->GetOpcode() == Opcode::CallResolvedStatic) {
            argNum += 1;  // skip method_reg
        }
        ASSERT(argNum < callInst->GetInputsCount() - 1);
        auto input = callInst->GetInput(argNum);
        for (auto &user : paramInst->GetUsers()) {
            if (user.GetInst()->GetOpcode() == Opcode::NullCheck) {
                user.GetInst()->ReplaceUsers(input.GetInst());
                worklist.push_back(user.GetInst());
            }
        }
        paramInst->ReplaceUsers(input.GetInst());
        paramInst = GetNextParam(paramInst);
    }
    for (auto inst : worklist) {
        auto ss = inst->GetInput(1).GetInst();
        inst->RemoveInputs();
        inst->GetBasicBlock()->EraseInst(inst);
        ss->RemoveInputs();
        ss->GetBasicBlock()->EraseInst(ss);
    }
}

static inline CallInst *CloneVirtualCallInst(CallInst *call, Graph *graph)
{
    if (call->GetOpcode() == Opcode::CallVirtual) {
        return call->Clone(graph)->CastToCallVirtual();
    }
    if (call->GetOpcode() == Opcode::CallResolvedVirtual) {
        return call->Clone(graph)->CastToCallResolvedVirtual();
    }
    UNREACHABLE();
}

bool Inlining::DoInlinePolymorphic(CallInst *callInst, ArenaVector<RuntimeInterface::ClassPtr> *receivers)
{
    LOG_INLINING(DEBUG) << "Try inline polymorphic call(" << receivers->size() << " receivers):";
    LOG_INLINING(DEBUG) << "  instruction: " << *callInst;

    bool hasUnreachableBlocks = false;
    bool hasRuntimeCalls = false;
    auto runtime = GetGraph()->GetRuntime();
    auto getClsInst = GetGraph()->CreateInstGetInstanceClass(DataType::REFERENCE, callInst->GetPc());
    PhiInst *phiInst = nullptr;
    BasicBlock *callBb = nullptr;
    BasicBlock *callContBb = nullptr;
    auto inlinedMethods = methodsInlined_;

    // For each receiver we construct BB for CallVirtual inlined, and BB for Return.Inlined
    // Inlined graph we inserts between the blocks:
    // BEFORE:
    //     call_bb:
    //         call_inst
    //         succs [call_cont_bb]
    //     call_cont_bb:
    //
    // AFTER:
    //     call_bb:
    //         compare_classes
    //     succs [new_call_bb, call_inlined_block]
    //
    //     call_inlined_block:
    //         call_inst.inlined
    //     succs [inlined_graph]
    //
    //     inlined graph:
    //     succs [return_inlined_block]
    //
    //     return_inlined_block:
    //         return.inlined
    //     succs [call_cont_bb]
    //
    //     new_call_bb:
    //     succs [call_cont_bb]
    //
    //     call_cont_bb
    //         phi(new_call_bb, return_inlined_block)
    for (auto receiver : *receivers) {
        InlineContext ctx;
        ctx.method = runtime->ResolveVirtualMethod(receiver, callInst->GetCallMethod());
        ASSERT(ctx.method != nullptr && !runtime->IsMethodAbstract(ctx.method));
        LOG_INLINING(DEBUG) << "Inline receiver " << runtime->GetMethodFullName(ctx.method);
        if (!CheckMethodCanBeInlined<true, false>(callInst, &ctx)) {
            continue;
        }
        ASSERT(ctx.intrinsicId == RuntimeInterface::IntrinsicId::INVALID);

        // Create Call.inlined
        CallInst *newCallInst = CloneVirtualCallInst(callInst, GetGraph());
        newCallInst->SetCallMethodId(runtime->GetMethodId(ctx.method));
        newCallInst->SetCallMethod(ctx.method);
        auto inlGraph = BuildGraph(&ctx, callInst, newCallInst);
        if (inlGraph.graph == nullptr) {
            continue;
        }
        vregsCount_ += inlGraph.graph->GetVRegsCount();
        auto blocks = MakeCallBbs({callInst, getClsInst}, {callBb, callContBb}, &phiInst, receivers->size());
        callBb = blocks.first;
        callContBb = blocks.second;

        CreateCompareClass(callInst, getClsInst, receiver, callBb);
        InlineReceiver({callInst, newCallInst, phiInst}, {callBb, callContBb},
                       {&hasUnreachableBlocks, &hasRuntimeCalls}, receivers->size(), inlGraph);

        GetGraph()->GetPassManager()->GetStatistics()->AddInlinedMethods(1);
        EVENT_INLINE(runtime->GetMethodFullName(GetGraph()->GetMethod()), runtime->GetMethodFullName(ctx.method),
                     callInst->GetId(), events::InlineKind::VIRTUAL_POLYMORPHIC, events::InlineResult::SUCCESS);
        LOG_INLINING(DEBUG) << "Successfully inlined: " << GetMethodFullName(GetGraph(), ctx.method);
        methodsInlined_++;
    }

    bool needToDeoptimize = (methodsInlined_ - inlinedMethods == receivers->size());
    return FinalizeInlineReceiver({callInst, getClsInst, phiInst}, {callBb, callContBb},
                                  {&hasUnreachableBlocks, &hasRuntimeCalls}, needToDeoptimize);
}

Inlining::BasicBlockPair Inlining::MakeCallBbs(InstPair insts, BasicBlockPair bbs, [[maybe_unused]] PhiInst **phiInst,
                                               [[maybe_unused]] size_t receiversSize)
{
    [[maybe_unused]] auto *callInst = static_cast<CallInst *>(insts.first);
    [[maybe_unused]] auto *getClsInst = insts.second;
    auto [callBb, callContBb] = bbs;
    if (callBb == nullptr) {
        // Split block by call instruction
        callBb = callInst->GetBasicBlock();
        callContBb = callBb->SplitBlockAfterInstruction(callInst, false);
        callBb->AppendInst(getClsInst);
        if (callInst->GetType() != DataType::VOID) {
            *phiInst = GetGraph()->CreateInstPhi(callInst->GetType(), callInst->GetPc());
            (*phiInst)->ReserveInputs(receiversSize << 1U);
            callContBb->AppendPhi(*phiInst);
        }
    } else {
        auto newCallBb = GetGraph()->CreateEmptyBlock(callBb);
        callBb->GetLoop()->AppendBlock(newCallBb);
        callBb->AddSucc(newCallBb);
        callBb = newCallBb;
    }
    return std::make_pair(callBb, callContBb);
}

void Inlining::InlineReceiver(InstTriple insts, BasicBlockPair bbs, FlagPair flags, size_t receiversSize,
                              InlinedGraph inlGraph)
{
    auto *callInst = static_cast<CallInst *>(std::get<0>(insts));
    auto *newCallInst = static_cast<CallInst *>(std::get<1>(insts));
    auto *phiInst = static_cast<PhiInst *>(std::get<2>(insts));
    auto [callBb, callContBb] = bbs;
    auto [hasUnreachableBlocks, hasRuntimeCalls] = flags;
    // Create call_inlined_block
    auto callInlinedBlock = GetGraph()->CreateEmptyBlock(callBb);
    callBb->GetLoop()->AppendBlock(callInlinedBlock);
    callBb->AddSucc(callInlinedBlock);

    // Insert Call.inlined in call_inlined_block
    newCallInst->AppendInput(callInst->GetObjectInst());
    newCallInst->AppendInput(callInst->GetSaveState());
    newCallInst->SetInlined(true);
    newCallInst->SetFlag(inst_flags::NO_DST);
    // Set NO_DCE flag, since some call instructions might not have one after inlining
    newCallInst->SetFlag(inst_flags::NO_DCE);
    callInlinedBlock->PrependInst(newCallInst);

    // Create return_inlined_block and inster PHI for non void functions
    auto returnInlinedBlock = GetGraph()->CreateEmptyBlock(callBb);
    callBb->GetLoop()->AppendBlock(returnInlinedBlock);
    PhiInst *localPhiInst = nullptr;
    if (callInst->GetType() != DataType::VOID) {
        localPhiInst = GetGraph()->CreateInstPhi(callInst->GetType(), callInst->GetPc());
        localPhiInst->ReserveInputs(receiversSize << 1U);
        returnInlinedBlock->AppendPhi(localPhiInst);
    }

    // Inlined graph between call_inlined_block and return_inlined_block
    GetGraph()->SetMaxMarkerIdx(inlGraph.graph->GetCurrentMarkerIdx());
    UpdateParameterDataflow(inlGraph.graph, callInst);
    UpdateDataflow(inlGraph.graph, callInst, localPhiInst, phiInst);
    MoveConstants(inlGraph.graph);
    UpdateControlflow(inlGraph.graph, callInlinedBlock, returnInlinedBlock);

    if (!returnInlinedBlock->GetPredsBlocks().empty()) {
        if (inlGraph.hasRuntimeCalls) {
            auto inlinedReturn =
                GetGraph()->CreateInstReturnInlined(DataType::VOID, INVALID_PC, newCallInst->GetSaveState());
            returnInlinedBlock->PrependInst(inlinedReturn);
        }
        if (callInst->GetType() != DataType::VOID) {
            ASSERT(phiInst);
            // clang-tidy think that phi_inst can be nullptr
            phiInst->AppendInput(localPhiInst);  // NOLINT
        }
        returnInlinedBlock->AddSucc(callContBb);
    } else {
        // We need remove return_inlined_block if inlined graph doesn't have Return inst(only Throw or Deoptimize)
        *hasUnreachableBlocks = true;
    }

    if (inlGraph.hasRuntimeCalls) {
        *hasRuntimeCalls = true;
    } else {
        newCallInst->GetBasicBlock()->RemoveInst(newCallInst);
    }
}

bool Inlining::FinalizeInlineReceiver(InstTriple insts, BasicBlockPair bbs, FlagPair flags, bool needToDeoptimize)
{
    auto *callInst = static_cast<CallInst *>(std::get<0>(insts));
    auto *getClsInst = std::get<1>(insts);
    auto *phiInst = static_cast<PhiInst *>(std::get<2>(insts));
    auto [callBb, callContBb] = bbs;
    auto [hasUnreachableBlocks, hasRuntimeCalls] = flags;
    if (callBb == nullptr) {
        // Nothing was inlined
        return false;
    }
    if (callContBb->GetPredsBlocks().empty() || *hasUnreachableBlocks) {
        GetGraph()->RemoveUnreachableBlocks();
    }

    getClsInst->SetInput(0, callInst->GetObjectInst());

    if (needToDeoptimize) {
        InsertDeoptimizeInst(callInst, callBb);
    } else {
        InsertCallInst(callInst, callBb, callContBb, phiInst);
    }
    if (callInst->GetType() != DataType::VOID) {
        callInst->ReplaceUsers(phiInst);
    }

    ProcessCallReturnInstructions(callInst, callContBb, *hasRuntimeCalls);

    if (*hasRuntimeCalls) {
        callInst->GetBasicBlock()->RemoveInst(callInst);
    }
    return true;
}

#ifndef NDEBUG
void CheckExternalGraph(Graph *graph)
{
    for (auto bb : graph->GetVectorBlocks()) {
        if (bb != nullptr) {
            for (auto inst : bb->AllInstsSafe()) {
                ASSERT(!inst->RequireState());
            }
        }
    }
}
#endif

Inst *Inlining::GetNewDefAndCorrectDF(Inst *callInst, Inst *oldDef)
{
    if (oldDef->IsConst()) {
        auto constant = oldDef->CastToConstant();
        auto exisingConstant = GetGraph()->FindOrAddConstant(constant);
        return exisingConstant;
    }
    if (oldDef->IsParameter()) {
        auto argNum = oldDef->CastToParameter()->GetArgNumber();
        ASSERT(argNum < callInst->GetInputsCount() - 1);
        auto input = callInst->GetInput(argNum).GetInst();
        return input;
    }
    ASSERT(oldDef->GetOpcode() == Opcode::NullPtr);
    auto exisingNullptr = GetGraph()->GetOrCreateNullPtr();
    return exisingNullptr;
}

bool Inlining::TryInlineExternal(CallInst *callInst, InlineContext *ctx)
{
    if (TryInlineExternalAot(callInst, ctx)) {
        return true;
    }
    // Skip external methods
    EmitEvent(GetGraph(), callInst, *ctx, events::InlineResult::SKIP_EXTERNAL);
    LOG_INLINING(DEBUG) << "We can't inline external method: " << GetMethodFullName(GetGraph(), ctx->method);
    return false;
}

/*
 * External methods could be inlined only if there are no instructions requiring state.
 * The only exception are NullChecks that check parameters and used by LoadObject/StoreObject.
 */
bool CheckExternalMethodInstructions(Graph *graph, CallInst *callInst)
{
    ArenaUnorderedSet<Inst *> suspiciousInstructions(graph->GetLocalAllocator()->Adapter());
    for (auto bb : graph->GetVectorBlocks()) {
        if (bb == nullptr) {
            continue;
        }
        for (auto inst : bb->InstsSafe()) {
            bool isRtCall = inst->RequireState() || inst->IsRuntimeCall();
            auto opcode = inst->GetOpcode();
            if (isRtCall && opcode == Opcode::NullCheck) {
                suspiciousInstructions.insert(inst);
            } else if (isRtCall && opcode != Opcode::NullCheck) {
                return false;
            }
            if (opcode != Opcode::LoadObject && opcode != Opcode::StoreObject) {
                continue;
            }
            auto nc = inst->GetInput(0).GetInst();
            if (nc->GetOpcode() == Opcode::NullCheck && nc->HasSingleUser()) {
                suspiciousInstructions.erase(nc);
            }
            // If LoadObject/StoreObject first input (i.e. object to load data from / store data to)
            // is a method parameter and corresponding call instruction's input is either NullCheck
            // or NewObject then the NullCheck could be removed from external method's body because
            // the parameter is known to be not null at the time load/store will be executed.
            // If we can't prove that the input is not-null then NullCheck could not be eliminated
            // and a method could not be inlined.
            auto objInput = inst->GetDataFlowInput(0);
            if (objInput->GetOpcode() != Opcode::Parameter) {
                return false;
            }
            auto paramId = objInput->CastToParameter()->GetArgNumber() + callInst->GetObjectIndex();
            if (callInst->GetInput(paramId).GetInst()->GetOpcode() != Opcode::NullCheck &&
                callInst->GetDataFlowInput(paramId)->GetOpcode() != Opcode::NewObject) {
                return false;
            }
        }
    }
    return suspiciousInstructions.empty();
}

/*
 * We can only inline external methods that don't have runtime calls.
 * The only exception from this rule are methods performing LoadObject/StoreObject
 * to/from a parameter for which we can prove that it can't be null. In that case
 * NullChecks preceding LoadObject/StoreObject are removed from inlined graph.
 */
bool Inlining::TryInlineExternalAot(CallInst *callInst, InlineContext *ctx)
{
    // We can't guarantee without cha that runtime will use this external file.
    if (!GetGraph()->GetAotData()->GetUseCha()) {
        return false;
    }
    IrBuilderExternalInliningAnalysisForLLVMAot bytecodeAnalysis(GetGraph(), ctx->method);
    if (!GetGraph()->RunPass(&bytecodeAnalysis)) {
        EmitEvent(GetGraph(), callInst, *ctx, events::InlineResult::UNSUITABLE);
        LOG_INLINING(DEBUG) << "We can't inline external method: " << GetMethodFullName(GetGraph(), ctx->method);
        return false;
    }
    auto graphInl = GetGraph()->CreateChildGraph(ctx->method);
    graphInl->SetCurrentInstructionId(GetGraph()->GetCurrentInstructionId());

    auto stats = GetGraph()->GetPassManager()->GetStatistics();
    auto savedPbcInstNum = stats->GetPbcInstNum();
    if (!TryBuildGraph(*ctx, graphInl, callInst, nullptr)) {
        stats->SetPbcInstNum(savedPbcInstNum);
        return false;
    }

    graphInl->RunPass<Cleanup>();

    // External method could be inlined only if there are no instructions requiring state
    // because compiler saves method id into stack map's inline info and there is no way
    // to distinguish id of an external method from id of some method from the same translation unit.
    // Following check ensures that there are no instructions requiring state within parsed
    // external method except NullChecks used by LoadObject/StoreObject that checks nullness
    // of parameters that known to be non-null at the call time. In that case NullChecks
    // will be eliminated and there will no instruction requiring state.
    if (!CheckExternalMethodInstructions(graphInl, callInst)) {
        stats->SetPbcInstNum(savedPbcInstNum);
        return false;
    }

    vregsCount_ += graphInl->GetVRegsCount();

    auto method = ctx->method;
    auto runtime = GetGraph()->GetRuntime();
    // Call instruction is already inlined, so change its call id to the resolved method.
    callInst->SetCallMethodId(runtime->GetMethodId(method));
    callInst->SetCallMethod(method);

    auto callBb = callInst->GetBasicBlock();
    auto callContBb = callBb->SplitBlockAfterInstruction(callInst, false);

    GetGraph()->SetMaxMarkerIdx(graphInl->GetCurrentMarkerIdx());
    // Adjust instruction id counter for parent graph, thereby avoid situation when two instructions have same id.
    GetGraph()->SetCurrentInstructionId(graphInl->GetCurrentInstructionId());

    UpdateExternalParameterDataflow(graphInl, callInst);
    UpdateDataflow(graphInl, callInst, callContBb);
    MoveConstants(graphInl);
    UpdateControlflow(graphInl, callBb, callContBb);

    if (callContBb->GetPredsBlocks().empty()) {
        GetGraph()->RemoveUnreachableBlocks();
    } else {
        returnBlocks_.push_back(callContBb);
    }

    bool needBarriers = runtime->IsMemoryBarrierRequired(method);
    ProcessCallReturnInstructions(callInst, callContBb, false, needBarriers);

#ifndef NDEBUG
    CheckExternalGraph(graphInl);
#endif

    LOG_INLINING(DEBUG) << "Successfully inlined external method: " << GetMethodFullName(GetGraph(), ctx->method);
    methodsInlined_++;
    return true;
}

bool Inlining::DoInlineIntrinsicByExpansion([[maybe_unused]] CallInst *callInst, InlineContext *ctx)
{
    // CC-OFFNXT(C_RULE_SWITCH_BRANCH_CHECKER) autogenerated code
    switch (ctx->intrinsicId) {
#include "intrinsics_inlining_expansion_switch_case.inl"
        default:
            return false;
    }
    LOG_INLINING(DEBUG) << "The method '" << GetMethodFullName(GetGraph(), ctx->method)
                        << "' is inlined by expanding intrinsic '" << GetIntrinsicName(ctx->intrinsicId) << "'";
    return true;
}

bool Inlining::DoInlineIntrinsicByEncoding(CallInst *callInst, InlineContext *ctx)
{
    auto intrinsicId = ctx->intrinsicId;
    if (!EncodesBuiltin(GetGraph()->GetRuntime(), intrinsicId, GetGraph()->GetArch())) {
        return false;
    }
    IntrinsicInst *inst = GetGraph()->CreateInstIntrinsic(callInst->GetType(), callInst->GetPc(), intrinsicId);
    bool needSaveState = inst->RequireState();

    size_t inputsCount = callInst->GetInputsCount() - (needSaveState ? 0 : 1);

    inst->ReserveInputs(inputsCount);
    inst->AllocateInputTypes(GetGraph()->GetAllocator(), inputsCount);

    auto inputs = callInst->GetInputs();
    for (size_t i = 0; i < inputsCount; ++i) {
        inst->AppendInput(inputs[i].GetInst(), callInst->GetInputType(i));
    }

    callInst->InsertAfter(inst);
    callInst->ReplaceUsers(inst);
    LOG_INLINING(DEBUG) << "The method: " << GetMethodFullName(GetGraph(), ctx->method) << "replaced to the intrinsic"
                        << GetIntrinsicName(intrinsicId);
    return true;
}

bool Inlining::DoInlineIntrinsic(CallInst *callInst, InlineContext *ctx)
{
    ASSERT(ctx->intrinsicId != RuntimeInterface::IntrinsicId::INVALID);
    ASSERT(callInst != nullptr);
    if (!DoInlineIntrinsicByExpansion(callInst, ctx) && !DoInlineIntrinsicByEncoding(callInst, ctx)) {
        return false;
    }
    if (ctx->chaDevirtualize) {
        InsertChaGuard(callInst, ctx);
    }
    return true;
}

bool Inlining::DoInlineMethod(CallInst *callInst, InlineContext *ctx)
{
    ASSERT(ctx->intrinsicId == RuntimeInterface::IntrinsicId::INVALID);
    auto method = ctx->method;

    auto runtime = GetGraph()->GetRuntime();

    if (resolveWoInline_) {
        // Return, don't inline anything
        // At this point we:
        // 1. Gave a chance to inline external method
        // 2. Set replace_to_static to true where possible
        return false;
    }

    ASSERT(!runtime->IsMethodAbstract(method));

    // Split block by call instruction
    auto callBb = callInst->GetBasicBlock();
    // NOTE (a.popov) Support inlining to the catch blocks
    if (callBb->IsCatch()) {
        return false;
    }

    auto graphInl = BuildGraph(ctx, callInst);
    if (graphInl.graph == nullptr) {
        return false;
    }

    vregsCount_ += graphInl.graph->GetVRegsCount();

    // Call instruction is already inlined, so change its call id to the resolved method.
    callInst->SetCallMethodId(runtime->GetMethodId(method));
    callInst->SetCallMethod(method);

    auto callContBb = callBb->SplitBlockAfterInstruction(callInst, false);

    GetGraph()->SetMaxMarkerIdx(graphInl.graph->GetCurrentMarkerIdx());
    UpdateParameterDataflow(graphInl.graph, callInst);
    UpdateDataflow(graphInl.graph, callInst, callContBb);

    MoveConstants(graphInl.graph);

    UpdateControlflow(graphInl.graph, callBb, callContBb);

    if (callContBb->GetPredsBlocks().empty()) {
        GetGraph()->RemoveUnreachableBlocks();
    } else {
        returnBlocks_.push_back(callContBb);
    }

    if (ctx->chaDevirtualize) {
        InsertChaGuard(callInst, ctx);
    }

    bool needBarriers = runtime->IsMemoryBarrierRequired(method);
    ProcessCallReturnInstructions(callInst, callContBb, graphInl.hasRuntimeCalls, needBarriers);

    LOG_INLINING(DEBUG) << "Successfully inlined: " << GetMethodFullName(GetGraph(), method);
    GetGraph()->GetPassManager()->GetStatistics()->AddInlinedMethods(1);
    methodsInlined_++;

    return true;
}

bool Inlining::DoInline(CallInst *callInst, InlineContext *ctx)
{
    ASSERT(!callInst->IsInlined());

    auto method = ctx->method;

    auto runtime = GetGraph()->GetRuntime();

    if (!CheckMethodCanBeInlined<false, true>(callInst, ctx)) {
        return false;
    }
    if (runtime->IsMethodExternal(GetGraph()->GetMethod(), method) && !IsIntrinsic(ctx)) {
        if (!g_options.IsCompilerInlineExternalMethods()) {
            // Skip external methods
            EmitEvent(GetGraph(), callInst, *ctx, events::InlineResult::SKIP_EXTERNAL);
            LOG_INLINING(DEBUG) << "We can't inline external method: " << GetMethodFullName(GetGraph(), ctx->method);
            return false;
        }
        if (GetGraph()->IsAotMode()) {
            if (!g_options.IsCompilerInlineExternalMethodsAot()) {
                // Skip external methods
                EmitEvent(GetGraph(), callInst, *ctx, events::InlineResult::SKIP_EXTERNAL);
                LOG_INLINING(DEBUG) << "We can't inline external method in AOT mode: "
                                    << GetMethodFullName(GetGraph(), ctx->method);
                return false;
            }
            // We can't guarantee without cha that runtime will use this external file.
            if (!GetGraph()->GetAotData()->GetUseCha()) {
                return false;
            }
            // NOTE(llvm): Temporary use old way of inlining external methods in LLVM AOT mode
            //             until default way will be fixed #27874
            if (GetGraph()->GetAotData()->IsLLVMAotMode()) {
                return TryInlineExternalAot(callInst, ctx);
            }
        }
    }

    if (IsIntrinsic(ctx)) {
        return DoInlineIntrinsic(callInst, ctx);
    }
    return DoInlineMethod(callInst, ctx);
}

void Inlining::ProcessCallReturnInstructions(CallInst *callInst, BasicBlock *callContBb, bool hasRuntimeCalls,
                                             bool needBarriers)
{
    if (hasRuntimeCalls || needBarriers) {
        // In case if inlined graph contains call to runtime we need to preserve call instruction with special `Inlined`
        // flag and create new `ReturnInlined` instruction, hereby codegen can properly handle method frames.
        callInst->SetInlined(true);
        callInst->SetFlag(inst_flags::NO_DST);
        // Set NO_DCE flag, since some call instructions might not have one after inlining
        callInst->SetFlag(inst_flags::NO_DCE);
        // Remove callInst's all inputs except SaveState and NullCheck(if exist)
        // Do not remove function (first) input for dynamic calls
        auto saveState = callInst->GetSaveState();
        ASSERT(saveState->GetOpcode() == Opcode::SaveState);
        if (callInst->GetOpcode() != Opcode::CallDynamic) {
            auto nullcheckInst = callInst->GetObjectInst();
            callInst->RemoveInputs();
            if (nullcheckInst->GetOpcode() == Opcode::NullCheck) {
                callInst->AppendInput(nullcheckInst);
            }
        } else {
            auto func = callInst->GetInput(0).GetInst();
            callInst->RemoveInputs();
            callInst->AppendInput(func);
        }
        callInst->AppendInput(saveState);
        callInst->SetType(DataType::VOID);
        for (auto bb : returnBlocks_) {
            auto inlinedReturn =
                GetGraph()->CreateInstReturnInlined(DataType::VOID, INVALID_PC, callInst->GetSaveState());
            if (bb != callContBb && (bb->IsEndWithThrowOrDeoptimize() ||
                                     (bb->IsEmpty() && bb->GetPredsBlocks()[0]->IsEndWithThrowOrDeoptimize()))) {
                auto lastInst = !bb->IsEmpty() ? bb->GetLastInst() : bb->GetPredsBlocks()[0]->GetLastInst();
                lastInst->InsertBefore(inlinedReturn);
                inlinedReturn->SetExtendedLiveness();
            } else {
                bb->PrependInst(inlinedReturn);
            }
            if (needBarriers) {
                inlinedReturn->SetFlag(inst_flags::MEM_BARRIER);
            }
        }
    } else {
        // Otherwise we remove call instruction
        auto saveState = callInst->GetSaveState();
        // Remove SaveState if it has only Call instruction in the users
        ASSERT(saveState != nullptr);
        if (saveState->GetUsers().Front().GetNext() == nullptr) {
            saveState->GetBasicBlock()->RemoveInst(saveState);
        }
        callInst->GetBasicBlock()->RemoveInst(callInst);
    }
    returnBlocks_.clear();
}

bool Inlining::CheckBytecode(CallInst *callInst, const InlineContext &ctx, bool *calleeCallRuntime)
{
    auto vregsNum = GetGraph()->GetRuntime()->GetMethodArgumentsCount(ctx.method) +
                    GetGraph()->GetRuntime()->GetMethodRegistersCount(ctx.method) + 1;
    if ((vregsCount_ + vregsNum) >= g_options.GetCompilerMaxVregsNum()) {
        EmitEvent(GetGraph(), callInst, ctx, events::InlineResult::LIMIT);
        LOG_INLINING(DEBUG) << "Reached vregs limit: current=" << vregsCount_ << ", inlined=" << vregsNum;
        return false;
    }
    if (GetGraph()->IsAotMode()) {
        // Check bytecode in external methods including nested ones
        auto *rootGraph = GetGraph();
        while (rootGraph->GetParentGraph() != nullptr) {
            rootGraph = rootGraph->GetParentGraph();
        }
        auto isExternal = GetGraph()->GetRuntime()->IsMethodExternal(rootGraph->GetMethod(), ctx.method);
        if (isExternal && !IsIntrinsic(&ctx)) {
            IrBuilderExternalInliningAnalysis bytecodeAnalysis(GetGraph(), ctx.method);
            if (!GetGraph()->RunPass(&bytecodeAnalysis)) {
                EmitEvent(GetGraph(), callInst, ctx, events::InlineResult::UNSUITABLE);
                LOG_INLINING(DEBUG) << "We can't inline external method: '" << GetMethodFullName(GetGraph(), ctx.method)
                                    << "', because it contains unsuitable bytecode";
                return false;
            }
        }
    }
    IrBuilderInliningAnalysis bytecodeAnalysis(GetGraph(), ctx.method);
    if (!GetGraph()->RunPass(&bytecodeAnalysis)) {
        EmitEvent(GetGraph(), callInst, ctx, events::InlineResult::UNSUITABLE);
        LOG_INLINING(DEBUG) << "Method contains unsuitable bytecode";
        return false;
    }

    if (bytecodeAnalysis.HasRuntimeCalls() && g_options.IsCompilerInlineSimpleOnly()) {
        EmitEvent(GetGraph(), callInst, ctx, events::InlineResult::UNSUITABLE);
        return false;
    }
    if (calleeCallRuntime != nullptr) {
        *calleeCallRuntime = bytecodeAnalysis.HasRuntimeCalls();
    }
    return true;
}

bool Inlining::TryBuildGraph(const InlineContext &ctx, Graph *graphInl, CallInst *callInst, CallInst *polyCallInst)
{
    if (!graphInl->RunPass<IrBuilder>(ctx.method, polyCallInst != nullptr ? polyCallInst : callInst,
                                      GetCurrentDepth() + 1)) {
        EmitEvent(GetGraph(), callInst, ctx, events::InlineResult::FAIL);
        LOG_INLINING(WARNING) << "Graph building failed";
        return false;
    }

    if (graphInl->HasInfiniteLoop()) {
        EmitEvent(GetGraph(), callInst, ctx, events::InlineResult::INF_LOOP);
        COMPILER_LOG(INFO, INLINING) << "Inlining of the methods with infinite loop is not supported";
        return false;
    }

    if (!g_options.IsCompilerInliningSkipAlwaysThrowMethods()) {
        return true;
    }

    bool alwaysThrow = true;
    // check that end block could be reached only through throw-blocks
    for (auto pred : graphInl->GetEndBlock()->GetPredsBlocks()) {
        auto returnInst = pred->GetLastInst();
        if (returnInst == nullptr) {
            ASSERT(pred->IsTryEnd());
            ASSERT(pred->GetPredsBlocks().size() == 1);
            pred = pred->GetPredBlockByIndex(0);
        }
        if (!pred->IsEndWithThrowOrDeoptimize()) {
            alwaysThrow = false;
            break;
        }
    }
    if (!alwaysThrow) {
        return true;
    }
    EmitEvent(GetGraph(), callInst, ctx, events::InlineResult::UNSUITABLE);
    LOG_INLINING(DEBUG) << "Method always throw an expection, skip inlining: "
                        << GetMethodFullName(GetGraph(), ctx.method);
    return false;
}

void RemoveDeadSafePoints(Graph *graphInl)
{
    for (auto bb : *graphInl) {
        if (bb == nullptr || bb->IsStartBlock() || bb->IsEndBlock()) {
            continue;
        }
        for (auto inst : bb->InstsSafe()) {
            if (!inst->IsSaveState()) {
                continue;
            }
            ASSERT(inst->GetOpcode() == Opcode::SafePoint || inst->GetOpcode() == Opcode::SaveStateDeoptimize);
            ASSERT(inst->GetUsers().Empty());
            bb->RemoveInst(inst);
        }
    }
}

bool Inlining::CheckLoops(bool *calleeCallRuntime, Graph *graphInl)
{
    // Check that inlined graph hasn't loops
    graphInl->RunPass<LoopAnalyzer>();
    if (graphInl->HasLoop()) {
        if (g_options.IsCompilerInlineSimpleOnly()) {
            LOG_INLINING(INFO) << "Inlining of the methods with loops is disabled";
            return false;
        }
        *calleeCallRuntime = true;
    } else if (!*calleeCallRuntime) {
        RemoveDeadSafePoints(graphInl);
    }
    return true;
}

/* static */
void Inlining::PropagateObjectInfo(Graph *graphInl, CallInst *callInst)
{
    // Propagate object type information to the parameters of the inlined graph
    auto index = callInst->GetObjectIndex();
    // NOLINTNEXTLINE(readability-static-accessed-through-instance)
    for (auto paramInst : graphInl->GetParameters()) {
        auto inputInst = callInst->GetDataFlowInput(index);
        paramInst->SetObjectTypeInfo(inputInst->GetObjectTypeInfo());
        index++;
    }
}

InlinedGraph Inlining::BuildGraph(InlineContext *ctx, CallInst *callInst, CallInst *polyCallInst)
{
    bool calleeCallRuntime = false;
    if (!CheckBytecode(callInst, *ctx, &calleeCallRuntime)) {
        return InlinedGraph();
    }

    auto graphInl = GetGraph()->CreateChildGraph(ctx->method);

    // Propagate instruction id counter to inlined graph, thereby avoid instructions id duplication
    ASSERT(graphInl != nullptr);
    graphInl->SetCurrentInstructionId(GetGraph()->GetCurrentInstructionId());

    auto stats = GetGraph()->GetPassManager()->GetStatistics();
    auto savedPbcInstNum = stats->GetPbcInstNum();
    if (!TryBuildGraph(*ctx, graphInl, callInst, polyCallInst)) {
        stats->SetPbcInstNum(savedPbcInstNum);
        return InlinedGraph();
    }

    PropagateObjectInfo(graphInl, callInst);

    // Run basic optimizations
    graphInl->RunPass<Cleanup>(false);
    auto peepholeApplied = graphInl->RunPass<Peepholes>();
    auto objectTypeApplied = graphInl->RunPass<ObjectTypeCheckElimination>();
    if (peepholeApplied || objectTypeApplied) {
        graphInl->RunPass<BranchElimination>();
        // after BranchElimination pass, we should do InfiniteLoop check again.
        graphInl->RunPass<LoopAnalyzer>();
        if (graphInl->HasInfiniteLoop()) {
            EmitEvent(GetGraph(), callInst, *ctx, events::InlineResult::INF_LOOP);
            stats->SetPbcInstNum(savedPbcInstNum);
            return InlinedGraph();
        }
    }
    graphInl->RunPass<Cleanup>(false);
    graphInl->RunPass<OptimizeStringConcat>();
    graphInl->RunPass<SimplifyStringBuilder>();

    auto inlinedInstsCount = CalculateInstructionsCount(graphInl);
    LOG_INLINING(DEBUG) << "Actual insts-bc ratio: (" << inlinedInstsCount << " insts) / ("
                        << GetGraph()->GetRuntime()->GetMethodCodeSize(ctx->method) << ") = "
                        << (double)inlinedInstsCount / GetGraph()->GetRuntime()->GetMethodCodeSize(ctx->method);

    graphInl->RunPass<Inlining>(instructionsCount_ + inlinedInstsCount, methodsInlined_ + 1, &inlinedStack_);

    instructionsCount_ += CalculateInstructionsCount(graphInl);

    GetGraph()->SetMaxMarkerIdx(graphInl->GetCurrentMarkerIdx());

    // Adjust instruction id counter for parent graph, thereby avoid situation when two instructions have same id.
    GetGraph()->SetCurrentInstructionId(graphInl->GetCurrentInstructionId());

    if (ctx->chaDevirtualize && !GetCha()->IsSingleImplementation(ctx->method)) {
        EmitEvent(GetGraph(), callInst, *ctx, events::InlineResult::LOST_SINGLE_IMPL);
        LOG_INLINING(WARNING) << "Method lost single implementation property while we build IR for it";
        stats->SetPbcInstNum(savedPbcInstNum);
        return InlinedGraph();
    }

    if (!CheckLoops(&calleeCallRuntime, graphInl)) {
        stats->SetPbcInstNum(savedPbcInstNum);
        return InlinedGraph();
    }
    return {graphInl, calleeCallRuntime};
}

template <bool CHECK_EXTERNAL>
bool Inlining::CheckMethodSize(const CallInst *callInst, InlineContext *ctx)
{
    size_t methodSize = GetGraph()->GetRuntime()->GetMethodCodeSize(ctx->method);
    size_t expectedInlinedInstsCount = g_options.GetCompilerInliningInstsBcRatio() * methodSize;
    bool methodIsTooBig = (expectedInlinedInstsCount + instructionsCount_) > instructionsLimit_;
    methodIsTooBig |= methodSize >= g_options.GetCompilerInliningMaxBcSize();
    if (methodIsTooBig) {
        if (methodSize <= g_options.GetCompilerInliningAlwaysInlineBcSize()) {
            methodIsTooBig = false;
            EmitEvent(GetGraph(), callInst, *ctx, events::InlineResult::IGNORE_LIMIT);
            LOG_INLINING(DEBUG) << "Ignore instructions limit: ";
        } else {
            EmitEvent(GetGraph(), callInst, *ctx, events::InlineResult::LIMIT);
            LOG_INLINING(DEBUG) << "Method is too big (d_" << inlinedStack_.size() << "):";
        }
        LOG_INLINING(DEBUG) << "instructions_count_ = " << instructionsCount_
                            << ", expected_inlined_insts_count = " << expectedInlinedInstsCount
                            << ", instructions_limit_ = " << instructionsLimit_
                            << ", (method = " << GetMethodFullName(GetGraph(), ctx->method) << ")";
    }

    if (methodIsTooBig || resolveWoInline_) {
        return CheckTooBigMethodCanBeInlined<CHECK_EXTERNAL>(callInst, ctx, methodIsTooBig);
    }
    return true;
}

template <bool CHECK_EXTERNAL>
bool Inlining::CheckTooBigMethodCanBeInlined(const CallInst *callInst, InlineContext *ctx, bool methodIsTooBig)
{
    ctx->replaceToStatic = CanReplaceWithCallStatic(callInst->GetOpcode());
    if constexpr (!CHECK_EXTERNAL) {
        if (GetGraph()->GetRuntime()->IsMethodExternal(GetGraph()->GetMethod(), ctx->method)) {
            // Do not replace to call static if --compiler-inline-external-methods=false
            ctx->replaceToStatic &= g_options.IsCompilerInlineExternalMethods();
            ctx->replaceToStatic &= !GetGraph()->IsAotMode() || g_options.IsCompilerInlineExternalMethodsAot();
            ASSERT(ctx->method != nullptr);
            // Allow to replace CallVirtual with CallStatic if the resolved method is same as the called method
            // In AOT mode the resolved method id can be different from the method id in the callInst,
            // but we'll keep the method id from the callInst because the resolved method id can be not correct
            // for aot compiled method
            ctx->replaceToStatic &= ctx->method == callInst->GetCallMethod()
                                    // Or if it's not aot mode. That is, just replace in other modes
                                    || !GetGraph()->IsAotMode();
        }
    }
    if (methodIsTooBig) {
        return false;
    }
    ASSERT(resolveWoInline_);
    // Continue and return true to give a change to TryInlineExternalAot
    return true;
}

bool Inlining::CheckDepthLimit(InlineContext *ctx)
{
    size_t recInlinedCount = std::count(inlinedStack_.begin(), inlinedStack_.end(), ctx->method);
    if ((recInlinedCount >= g_options.GetCompilerInliningRecursiveCallsLimit()) ||
        (inlinedStack_.size() >= MAX_CALL_DEPTH)) {
        LOG_INLINING(DEBUG) << "Recursive-calls-depth limit reached, method: '"
                            << GetMethodFullName(GetGraph(), ctx->method) << "', depth: " << recInlinedCount;
        return false;
    }
    bool isDepthLimitIgnored = GetCurrentDepth() >= g_options.GetCompilerInliningMaxDepth();
    bool isSmallMethod =
        GetGraph()->GetRuntime()->GetMethodCodeSize(ctx->method) <= g_options.GetCompilerInliningAlwaysInlineBcSize();
    if (isDepthLimitIgnored && !isSmallMethod) {
        LOG_INLINING(DEBUG) << "Small-method-depth limit reached, method: '"
                            << GetMethodFullName(GetGraph(), ctx->method)
                            << "', size: " << GetGraph()->GetRuntime()->GetMethodCodeSize(ctx->method);
        return false;
    }
    return true;
}

template <bool CHECK_EXTERNAL, bool CHECK_INTRINSICS>
bool Inlining::CheckMethodCanBeInlined(const CallInst *callInst, InlineContext *ctx)
{
    if (ctx->method == nullptr) {
        return false;
    }

    if (!CheckDepthLimit(ctx)) {
        return false;
    }

    if constexpr (CHECK_EXTERNAL) {
        if ((!g_options.IsCompilerInlineExternalMethods() ||
             (GetGraph()->IsAotMode() && !g_options.IsCompilerInlineExternalMethodsAot())) &&
            GetGraph()->GetRuntime()->IsMethodExternal(GetGraph()->GetMethod(), ctx->method)) {
            // Skip external methods
            EmitEvent(GetGraph(), callInst, *ctx, events::InlineResult::SKIP_EXTERNAL);
            LOG_INLINING(DEBUG) << "We can't inline external method: " << GetMethodFullName(GetGraph(), ctx->method);
            return false;
        }
    }

    if (!blacklist_.empty()) {
        std::string methodName = GetGraph()->GetRuntime()->GetMethodFullName(ctx->method);
        if (blacklist_.find(methodName) != blacklist_.end()) {
            EmitEvent(GetGraph(), callInst, *ctx, events::InlineResult::NOINLINE);
            LOG_INLINING(DEBUG) << "Method is in the blacklist: " << GetMethodFullName(GetGraph(), ctx->method);
            return false;
        }
    }

    if (!GetGraph()->GetRuntime()->IsMethodCanBeInlined(ctx->method)) {
        if constexpr (CHECK_INTRINSICS) {
            if (GetGraph()->GetRuntime()->IsMethodIntrinsic(ctx->method)) {
                ctx->intrinsicId = GetGraph()->GetRuntime()->GetIntrinsicId(ctx->method);
                return true;
            }
        }
        EmitEvent(GetGraph(), callInst, *ctx, events::InlineResult::UNSUITABLE);
        return false;
    }

    if (GetGraph()->GetRuntime()->GetMethodName(ctx->method).find("__noinline__") != std::string::npos) {
        EmitEvent(GetGraph(), callInst, *ctx, events::InlineResult::NOINLINE);
        return false;
    }
    return CheckMethodSize<CHECK_EXTERNAL>(callInst, ctx);
}

void RemoveReturnVoidInst(BasicBlock *endBlock)
{
    for (auto &pred : endBlock->GetPredsBlocks()) {
        auto returnInst = pred->GetLastInst();
        if (returnInst->GetOpcode() == Opcode::Throw || returnInst->GetOpcode() == Opcode::Deoptimize) {
            continue;
        }
        ASSERT(returnInst->GetOpcode() == Opcode::ReturnVoid);
        pred->RemoveInst(returnInst);
    }
}

/// Embed inlined dataflow graph into the caller graph. A special case where the graph is empty
void Inlining::UpdateDataflowForEmptyGraph(Inst *callInst, std::variant<BasicBlock *, PhiInst *> use,
                                           BasicBlock *endBlock)
{
    auto predBlock = endBlock->GetPredsBlocks().front();
    auto returnInst = predBlock->GetLastInst();
    ASSERT(returnInst->GetOpcode() == Opcode::Return || returnInst->GetOpcode() == Opcode::ReturnVoid ||
           predBlock->IsEndWithThrowOrDeoptimize());
    if (returnInst->GetOpcode() == Opcode::Return) {
        ASSERT(returnInst->GetInputsCount() == 1);
        auto inputInst = returnInst->GetInput(0).GetInst();
        if (std::holds_alternative<PhiInst *>(use)) {
            auto phiInst = std::get<PhiInst *>(use);
            phiInst->AppendInput(inputInst);
        } else {
            callInst->ReplaceUsers(inputInst);
        }
    }
    if (!predBlock->IsEndWithThrowOrDeoptimize()) {
        predBlock->RemoveInst(returnInst);
    }
}

/// Embed inlined dataflow graph into the caller graph.
void Inlining::UpdateDataflow(Graph *graphInl, Inst *callInst, std::variant<BasicBlock *, PhiInst *> use, Inst *newDef)
{
    // Replace inlined graph outcoming dataflow edges
    auto endBlock = graphInl->GetEndBlock();
    if (endBlock->GetPredsBlocks().size() > 1) {
        if (callInst->GetType() == DataType::VOID) {
            RemoveReturnVoidInst(endBlock);
            return;
        }
        PhiInst *phiInst = nullptr;
        if (std::holds_alternative<BasicBlock *>(use)) {
            phiInst = GetGraph()->CreateInstPhi(GetGraph()->GetRuntime()->GetMethodReturnType(graphInl->GetMethod()),
                                                INVALID_PC);
            phiInst->ReserveInputs(endBlock->GetPredsBlocks().size());
            std::get<BasicBlock *>(use)->AppendPhi(phiInst);
        } else {
            phiInst = std::get<PhiInst *>(use);
            ASSERT(phiInst != nullptr);
        }
        for (auto pred : endBlock->GetPredsBlocks()) {
            auto returnInst = pred->GetLastInst();
            if (returnInst == nullptr) {
                ASSERT(pred->IsTryEnd());
                ASSERT(pred->GetPredsBlocks().size() == 1);
                pred = pred->GetPredBlockByIndex(0);
                returnInst = pred->GetLastInst();
            }
            if (pred->IsEndWithThrowOrDeoptimize()) {
                continue;
            }
            ASSERT(returnInst->GetOpcode() == Opcode::Return);
            ASSERT(returnInst->GetInputsCount() == 1);
            phiInst->AppendInput(returnInst->GetInput(0).GetInst());
            pred->RemoveInst(returnInst);
        }
        if (newDef == nullptr) {
            newDef = phiInst;
        }
        callInst->ReplaceUsers(newDef);
    } else {
        UpdateDataflowForEmptyGraph(callInst, use, endBlock);
    }
}

/// Embed inlined controlflow graph into the caller graph.
void Inlining::UpdateControlflow(Graph *graphInl, BasicBlock *callBb, BasicBlock *callContBb)
{
    // Move all blocks from inlined graph to parent
    auto currentLoop = callBb->GetLoop();
    for (auto bb : graphInl->GetVectorBlocks()) {
        if (bb != nullptr && !bb->IsStartBlock() && !bb->IsEndBlock()) {
            bb->ClearMarkers();
            GetGraph()->AddBlock(bb);
            bb->CopyTryCatchProps(callBb);
        }
    }
    callContBb->CopyTryCatchProps(callBb);

    // Fix loop tree
    for (auto loop : graphInl->GetRootLoop()->GetInnerLoops()) {
        currentLoop->AppendInnerLoop(loop);
        loop->SetOuterLoop(currentLoop);
    }
    for (auto bb : graphInl->GetRootLoop()->GetBlocks()) {
        bb->SetLoop(currentLoop);
        currentLoop->AppendBlock(bb);
    }

    // Connect inlined graph as successor of the first part of call continuation block
    auto startBb = graphInl->GetStartBlock();
    ASSERT(startBb->GetSuccsBlocks().size() == 1);
    auto succ = startBb->GetSuccessor(0);
    succ->ReplacePred(startBb, callBb);
    startBb->GetSuccsBlocks().clear();

    ASSERT(graphInl->HasEndBlock());
    auto endBlock = graphInl->GetEndBlock();
    for (auto pred : endBlock->GetPredsBlocks()) {
        endBlock->RemovePred(pred);
        if (pred->IsEndWithThrowOrDeoptimize() ||
            (pred->IsEmpty() && pred->GetPredsBlocks()[0]->IsEndWithThrowOrDeoptimize())) {
            if (!GetGraph()->HasEndBlock()) {
                GetGraph()->CreateEndBlock();
            }
            returnBlocks_.push_back(pred);
            pred->ReplaceSucc(endBlock, GetGraph()->GetEndBlock());
        } else {
            pred->ReplaceSucc(endBlock, callContBb);
        }
    }
}

/**
 * Move constants of the inlined graph to the current one if same constant doesn't already exist.
 * If constant exists just fix callee graph's dataflow to use existing constants.
 */
void Inlining::MoveConstants(Graph *graphInl)
{
    auto startBb = graphInl->GetStartBlock();
    for (ConstantInst *constant = graphInl->GetFirstConstInst(), *nextConstant = nullptr; constant != nullptr;
         constant = nextConstant) {
        nextConstant = constant->GetNextConst();
        startBb->EraseInst(constant);
        auto exisingConstant = GetGraph()->FindOrAddConstant(constant);
        if (exisingConstant != constant) {
            constant->ReplaceUsers(exisingConstant);
        }
    }

    // Move NullPtr instruction
    if (graphInl->HasNullPtrInst()) {
        startBb->EraseInst(graphInl->GetNullPtrInst());
        auto exisingNullptr = GetGraph()->GetOrCreateNullPtr();
        graphInl->GetNullPtrInst()->ReplaceUsers(exisingNullptr);
    }
    // Move LoadUniqueObject instruction
    if (graphInl->HasUniqueObjectInst()) {
        startBb->EraseInst(graphInl->GetUniqueObjectInst());
        auto exisingUniqueObject = GetGraph()->GetOrCreateUniqueObjectInst();
        graphInl->GetUniqueObjectInst()->ReplaceUsers(exisingUniqueObject);
    }
}

bool Inlining::ResolveTarget(CallInst *callInst, InlineContext *ctx)
{
    auto runtime = GetGraph()->GetRuntime();
    auto method = callInst->GetCallMethod();
    if (callInst->GetOpcode() == Opcode::CallStatic) {
        ctx->method = method;
        return true;
    }

    if (g_options.IsCompilerNoVirtualInlining()) {
        return false;
    }

    // Resolve the method directly if: method is marked as 'final' or String or if manual devirtualization is possible.
    if (runtime->IsMethodFinal(method) || runtime->IsClassFinalOrString(runtime->GetClass(method))) {
        ctx->method = method;
        return true;
    }

    auto objectInst = callInst->GetDataFlowInput(callInst->GetObjectIndex());
    auto typeInfo = objectInst->GetObjectTypeInfo();
    if (CanUseTypeInfo(typeInfo, method)) {
        auto receiver = typeInfo.GetClass();
        MethodPtr resolvedMethod;
        if (runtime->IsInterfaceMethod(method)) {
            resolvedMethod = runtime->ResolveInterfaceMethod(receiver, method);
        } else {
            resolvedMethod = runtime->ResolveVirtualMethod(receiver, method);
        }
        if (resolvedMethod != nullptr && (typeInfo.IsExact() || runtime->IsMethodFinal(resolvedMethod))) {
            ctx->method = resolvedMethod;
            return true;
        }
        if (typeInfo.IsExact()) {
            LOG_INLINING(WARNING) << "Runtime failed to resolve method";
            return false;
        }
    }

    if (ArchTraits<RUNTIME_ARCH>::SUPPORT_DEOPTIMIZATION && !g_options.IsCompilerNoChaInlining() &&
        !GetGraph()->IsAotMode()) {
        // Try resolve via CHA
        auto cha = GetCha();
        if (cha != nullptr && cha->IsSingleImplementation(method)) {
            auto klass = runtime->GetClass(method);
            ctx->method = runtime->ResolveVirtualMethod(klass, callInst->GetCallMethod());
            if (ctx->method == nullptr) {
                return false;
            }
            ctx->chaDevirtualize = true;
            return true;
        }
    }

    return false;
}

bool Inlining::CanUseTypeInfo(ObjectTypeInfo typeInfo, RuntimeInterface::MethodPtr method)
{
    auto runtime = GetGraph()->GetRuntime();
    if (!typeInfo || runtime->IsInterface(typeInfo.GetClass())) {
        return false;
    }
    return runtime->IsAssignableFrom(runtime->GetClass(method), typeInfo.GetClass());
}

void Inlining::InsertChaGuard(CallInst *callInst, InlineContext *ctx)
{
    ASSERT(ctx->chaDevirtualize);

    auto saveState = callInst->GetSaveState();
    auto checkDeopt = GetGraph()->CreateInstIsMustDeoptimize(DataType::BOOL, callInst->GetPc());
    auto deopt =
        GetGraph()->CreateInstDeoptimizeIf(callInst->GetPc(), checkDeopt, saveState, DeoptimizeType::INLINE_CHA);
    callInst->InsertBefore(deopt);
    deopt->InsertBefore(checkDeopt);

    GetCha()->AddDependency(ctx->method, GetGraph()->GetOutermostParentGraph()->GetMethod());
    GetGraph()->GetOutermostParentGraph()->AddSingleImplementationMethod(ctx->method);
}

bool Inlining::SkipBlock(const BasicBlock *block) const
{
    if (block == nullptr || block->IsEmpty()) {
        return true;
    }
    if (!g_options.IsCompilerInliningSkipThrowBlocks() || (GetGraph()->GetThrowCounter(block) > 0)) {
        return false;
    }
    return block->IsEndWithThrowOrDeoptimize();
}
}  // namespace ark::compiler
