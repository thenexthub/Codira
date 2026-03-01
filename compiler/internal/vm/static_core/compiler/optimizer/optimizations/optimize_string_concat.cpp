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

#include "optimize_string_concat.h"

#include "compiler_logger.h"

#include "optimizer/analysis/alias_analysis.h"
#include "optimizer/analysis/bounds_analysis.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/ir/analysis.h"
#include "optimizer/ir/datatype.h"
#include "optimizer/ir/graph.h"
#include "optimizer/ir/inst.h"

#include "optimizer/ir/runtime_interface.h"
#include "optimizer/optimizations/cleanup.h"
#include "optimizer/optimizations/string_builder_utils.h"

namespace ark::compiler {

OptimizeStringConcat::OptimizeStringConcat(Graph *graph)
    : Optimization(graph), arrayElements_ {graph->GetAllocator()->Adapter()}
{
}

RuntimeInterface::IdType GetStringBuilderClassId(Graph *graph)
{
    auto runtime = graph->GetRuntime();
    auto klass = runtime->GetStringBuilderClass();
    return klass == nullptr ? 0 : runtime->GetClassIdWithinFile(graph->GetMethod(), klass);
}

bool OptimizeStringConcat::RunImpl()
{
    bool isApplied = false;

    if (GetStringBuilderClassId(GetGraph()) == 0) {
        COMPILER_LOG(WARNING, OPTIMIZE_STRING_CONCAT) << "StringBuilder class not found";
        return isApplied;
    }

    if (GetGraph()->IsAotMode()) {
        // NOTE(mivanov): Creating StringBuilder.ctor calls in AOT mode not yet supported
        return isApplied;
    }

    for (auto block : GetGraph()->GetBlocksRPO()) {
        for (auto inst : block->Insts()) {
            if (!IsMethodStringConcat(inst)) {
                continue;
            }

            ReplaceStringConcatWithStringBuilderAppend(inst);
            isApplied = true;
        }
    }

    COMPILER_LOG(DEBUG, OPTIMIZE_STRING_CONCAT) << "Optimize String.concat complete";

    if (isApplied) {
        GetGraph()->RunPass<compiler::Cleanup>();
    }

    return isApplied;
}

void OptimizeStringConcat::InvalidateAnalyses()
{
    GetGraph()->InvalidateAnalysis<BoundsAnalysis>();
    GetGraph()->InvalidateAnalysis<AliasAnalysis>();
}

Inst *CreateInstructionStringBuilderInstance(Graph *graph, uint32_t pc, SaveStateInst *saveState)
{
    auto runtime = graph->GetRuntime();
    auto method = graph->GetMethod();

    auto classId = GetStringBuilderClassId(graph);
    ASSERT(classId != 0);
    auto loadClass =
        graph->CreateInstLoadAndInitClass(DataType::REFERENCE, pc, CopySaveState(graph, saveState),
                                          TypeIdMixin {classId, method}, runtime->ResolveType(method, classId));
    auto newObject = graph->CreateInstNewObject(DataType::REFERENCE, pc, loadClass, CopySaveState(graph, saveState),
                                                TypeIdMixin {classId, method});

    return newObject;
}

IntrinsicInst *CreateStringBuilderAppendStringIntrinsic(Graph *graph, Inst *instance, Inst *arg,
                                                        SaveStateInst *saveState)
{
    auto appendIntrinsic = graph->CreateInstIntrinsic(graph->GetRuntime()->GetStringBuilderAppendStringsIntrinsicId(1));
    ASSERT(appendIntrinsic->RequireState());

    appendIntrinsic->SetType(DataType::REFERENCE);
    auto saveStateClone = CopySaveState(graph, saveState);
    appendIntrinsic->SetInputs(
        graph->GetAllocator(),
        {{instance, instance->GetType()}, {arg, arg->GetType()}, {saveStateClone, saveStateClone->GetType()}});

    return appendIntrinsic;
}

IntrinsicInst *CreateStringBuilderToStringIntrinsic(Graph *graph, Inst *instance, SaveStateInst *saveState)
{
    auto toStringCall = graph->CreateInstIntrinsic(graph->GetRuntime()->GetStringBuilderToStringIntrinsicId());
    ASSERT(toStringCall->RequireState());

    toStringCall->SetType(DataType::REFERENCE);
    auto saveStateClone = CopySaveState(graph, saveState);
    toStringCall->SetInputs(graph->GetAllocator(),
                            {{instance, instance->GetType()}, {saveStateClone, saveStateClone->GetType()}});

    return toStringCall;
}

CallInst *CreateStringBuilderDefaultConstructorCall(Graph *graph, Inst *instance, SaveStateInst *saveState)
{
    auto runtime = graph->GetRuntime();
    auto method = runtime->GetStringBuilderDefaultConstructor();
    auto methodId = runtime->GetMethodId(method);

    auto ctorCall = graph->CreateInstCallStatic(DataType::VOID, instance->GetPc(), methodId, method);
    ASSERT(ctorCall->RequireState());

    auto saveStateClone = CopySaveState(graph, saveState);
    ctorCall->SetInputs(graph->GetAllocator(),
                        {{instance, instance->GetType()}, {saveStateClone, saveStateClone->GetType()}});

    return ctorCall;
}

Inst *CreateLoadArray(Graph *graph, Inst *array, Inst *index)
{
    return graph->CreateInstLoadArray(DataType::REFERENCE, array->GetPc(), array, index);
}

Inst *CreateLoadArray(Graph *graph, Inst *array, uint64_t index)
{
    return CreateLoadArray(graph, array, graph->FindOrCreateConstant(index));
}

Inst *CreateLenArray(Graph *graph, Inst *newArray)
{
    return graph->CreateInstLenArray(DataType::INT32, newArray->GetPc(), newArray);
}

void OptimizeStringConcat::FixBrokenSaveStates(Inst *source, Inst *target)
{
    if (source->IsMovableObject()) {
        ssb_.SearchAndCreateMissingObjInSaveState(GetGraph(), source, target);
    }
}

void OptimizeStringConcat::CreateAppendArgsIntrinsic(Inst *instance, Inst *arg, SaveStateInst *saveState)
{
    auto appendIntrinsic = CreateStringBuilderAppendStringIntrinsic(GetGraph(), instance, arg, saveState);
    InsertBeforeWithInputs(appendIntrinsic, saveState);

    FixBrokenSaveStates(arg, appendIntrinsic);
    FixBrokenSaveStates(instance, appendIntrinsic);

    COMPILER_LOG(DEBUG, OPTIMIZE_STRING_CONCAT)
        << "Insert StringBuilder.append intrinsic (id=" << appendIntrinsic->GetId() << ")";
}

void OptimizeStringConcat::CreateAppendArgsIntrinsics(Inst *instance, SaveStateInst *saveState)
{
    for (auto arg : arrayElements_) {
        CreateAppendArgsIntrinsic(instance, arg, saveState);
    }
}

void OptimizeStringConcat::CreateAppendArgsIntrinsics(Inst *instance, Inst *args, uint64_t arrayLengthValue,
                                                      SaveStateInst *saveState)
{
    for (uint64_t index = 0; index < arrayLengthValue; ++index) {
        auto arg = CreateLoadArray(GetGraph(), args, index);
        CreateAppendArgsIntrinsic(instance, arg, saveState);
    }
}

Inst *CreateSafePoint(Graph *graph, uint32_t pc, SaveStateInst *saveState)
{
    ASSERT(saveState != nullptr);
    auto safePoint =
        graph->CreateInstSafePoint(pc, graph->GetMethod(), saveState->GetCallerInst(), saveState->GetInliningDepth());

    for (size_t index = 0; index < saveState->GetInputsCount(); ++index) {
        safePoint->AppendInput(saveState->GetInput(index));
        safePoint->SetVirtualRegister(index, saveState->GetVirtualRegister(index));
    }

    return safePoint;
}

BasicBlock *OptimizeStringConcat::CreateAppendArgsLoop(Inst *instance, Inst *str, Inst *args,
                                                       LengthMethodInst *arrayLength, Inst *concatCall)
{
    auto preHeader = concatCall->GetBasicBlock();
    auto postExit = preHeader->SplitBlockAfterInstruction(concatCall, false);
    auto saveState = concatCall->GetSaveState();

    // Create loop CFG
    auto header = GetGraph()->CreateEmptyBlock(preHeader);
    auto backEdge = GetGraph()->CreateEmptyBlock(preHeader);
    preHeader->AddSucc(header);
    header->AddSucc(postExit);
    header->AddSucc(backEdge);
    backEdge->AddSucc(header);

    // Declare loop variables
    auto start = GetGraph()->FindOrCreateConstant(0);
    auto stop = arrayLength;
    auto step = GetGraph()->FindOrCreateConstant(1);

    auto pc = instance->GetPc();

    // Build header
    auto induction = GetGraph()->CreateInstPhi(DataType::INT32, pc);
    auto safePoint = CreateSafePoint(GetGraph(), pc, saveState);
    auto compare =
        GetGraph()->CreateInstCompare(DataType::BOOL, pc, stop, induction, DataType::INT32, ConditionCode::CC_LE);
    auto ifImm = GetGraph()->CreateInstIfImm(DataType::BOOL, pc, compare, 0, DataType::BOOL, ConditionCode::CC_NE);
    header->AppendPhi(induction);
    header->AppendInsts({
        safePoint,
        compare,
        ifImm,
    });

    // Build back edge
    auto arg = CreateLoadArray(GetGraph(), args, induction);
    auto appendIntrinsic = CreateStringBuilderAppendStringIntrinsic(GetGraph(), instance, arg, saveState);
    auto add = GetGraph()->CreateInstAdd(DataType::INT32, pc, induction, step);
    backEdge->AppendInsts({
        arg,
        appendIntrinsic->GetSaveState(),
        appendIntrinsic,
        add,
    });

    // Connect loop induction variable inputs
    induction->AppendInput(start);
    induction->AppendInput(add);

    FixBrokenSaveStates(str, appendIntrinsic);
    FixBrokenSaveStates(args, appendIntrinsic);
    FixBrokenSaveStates(arg, appendIntrinsic);
    FixBrokenSaveStates(instance, appendIntrinsic);

    COMPILER_LOG(DEBUG, OPTIMIZE_STRING_CONCAT)
        << "Insert StringBuilder.append intrinsic (id=" << appendIntrinsic->GetId() << ")";

    return postExit;
}

bool OptimizeStringConcat::HasStoreArrayUsersOnly(Inst *newArray, Inst *removable)
{
    ASSERT(newArray->GetOpcode() == Opcode::NewArray);

    MarkerHolder visited {newArray->GetBasicBlock()->GetGraph()};
    bool found = HasUserRecursively(newArray, visited.GetMarker(), [newArray, removable](auto &user) {
        auto userInst = user.GetInst();
        auto isSaveState = userInst->IsSaveState();
        auto isCheck = userInst->IsCheck();
        auto isStoreArray = userInst->GetOpcode() == Opcode::StoreArray && userInst->GetDataFlowInput(0) == newArray;
        bool isRemovable = userInst == removable;
        return !isSaveState && !isCheck && !isStoreArray && !isRemovable;
    });

    ResetUserMarkersRecursively(newArray, visited.GetMarker());
    return !found;
}

void OptimizeStringConcat::ReplaceStringConcatWithStringBuilderAppend(Inst *concatCall)
{
    // Input:
    //  let result = str.concat(...args)
    //
    // Output:
    //  let instance = new StringBuilder(str)
    //  instance.append(args[0])
    //  ...
    //  instance.append(args[args.length-1])
    //  let result = instance.toString()

    ASSERT(concatCall->GetInputsCount() > 1);

    auto str = concatCall->GetDataFlowInput(0);
    auto args = concatCall->GetDataFlowInput(1);

    auto instance = CreateInstructionStringBuilderInstance(GetGraph(), concatCall->GetPc(), concatCall->GetSaveState());
    InsertBeforeWithInputs(instance, concatCall->GetSaveState());

    auto ctorCall = CreateStringBuilderDefaultConstructorCall(GetGraph(), instance, concatCall->GetSaveState());
    InsertBeforeWithSaveState(ctorCall, concatCall->GetSaveState());
    auto appendArgIntrinsic =
        CreateStringBuilderAppendStringIntrinsic(GetGraph(), instance, str, concatCall->GetSaveState());
    InsertBeforeWithSaveState(appendArgIntrinsic, concatCall->GetSaveState());
    FixBrokenSaveStates(instance, appendArgIntrinsic);
    FixBrokenSaveStates(str, appendArgIntrinsic);

    COMPILER_LOG(DEBUG, OPTIMIZE_STRING_CONCAT)
        << "Insert StringBuilder.append intrinsic (id=" << appendArgIntrinsic->GetId() << ")";

    auto toStringCall = CreateStringBuilderToStringIntrinsic(GetGraph(), instance, concatCall->GetSaveState());

    auto arrayLength = GetArrayLengthConstant(args);
    bool collected = args->GetOpcode() == Opcode::NewArray && CollectArrayElements(args, arrayElements_);
    if (collected) {
        CreateAppendArgsIntrinsics(instance, concatCall->GetSaveState());
        InsertBeforeWithSaveState(toStringCall, concatCall->GetSaveState());
    } else if (args->GetOpcode() == Opcode::NewArray && arrayLength != nullptr) {
        CreateAppendArgsIntrinsics(instance, args, arrayLength->CastToConstant()->GetIntValue(),
                                   concatCall->GetSaveState());
        InsertBeforeWithSaveState(toStringCall, concatCall->GetSaveState());
    } else {
        arrayLength = CreateLenArray(GetGraph(), args);
        concatCall->GetSaveState()->InsertBefore(arrayLength);
        auto postExit = CreateAppendArgsLoop(instance, str, args, arrayLength->CastToLenArray(), concatCall);

        postExit->PrependInst(toStringCall);
        postExit->PrependInst(toStringCall->GetSaveState());

        InvalidateBlocksOrderAnalyzes(GetGraph());
        GetGraph()->InvalidateAnalysis<LoopAnalyzer>();
    }
    COMPILER_LOG(DEBUG, OPTIMIZE_STRING_CONCAT) << "Replace String.concat call (id=" << concatCall->GetId()
                                                << ") with StringBuilder instance (id=" << instance->GetId() << ")";

    FixBrokenSaveStates(instance, toStringCall);

    concatCall->ReplaceUsers(toStringCall);

    concatCall->ClearFlag(inst_flags::NO_DCE);
    if (concatCall->GetInput(0).GetInst()->IsCheck()) {
        concatCall->GetInput(0).GetInst()->ClearFlag(inst_flags::NO_DCE);
    }

    if (collected && HasStoreArrayUsersOnly(args, concatCall)) {
        CleanupStoreArrayInstructions(args);
        args->ClearFlag(inst_flags::NO_DCE);
    }
}

}  // namespace ark::compiler
