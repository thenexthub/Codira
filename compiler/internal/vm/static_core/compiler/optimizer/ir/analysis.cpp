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

#include "analysis.h"

#include "optimizer/ir/basicblock.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "compiler_logger.h"
namespace ark::compiler {

class BasicBlock;

class IsOsrEntryBlock {
public:
    bool operator()(const BasicBlock *bb) const
    {
        return bb->IsOsrEntry();
    }
};

class IsTryBlock {
public:
    bool operator()(const BasicBlock *bb) const
    {
        return bb->IsTry();
    }
};

class IsSaveState {
public:
    bool operator()(const Inst *inst) const
    {
        return inst->IsSaveState() && inst->IsNotRemovable();
    }
};

class IsSaveStateCanTriggerGc {
public:
    bool operator()(const Inst *inst) const
    {
        return IsSaveStateForGc(inst);
    }
};

template <typename T>
bool FindBlockBetween(BasicBlock *dominateBb, BasicBlock *currentBb, Marker marker)
{
    if (dominateBb == currentBb) {
        return false;
    }
    if (currentBb->SetMarker(marker)) {
        return false;
    }
    if (T()(currentBb)) {
        return true;
    }
    for (auto pred : currentBb->GetPredsBlocks()) {
        if (FindBlockBetween<T>(dominateBb, pred, marker)) {
            return true;
        }
    }
    return false;
}

RuntimeInterface::ClassPtr GetClassPtrForObject(Inst *inst, size_t inputNum)
{
    auto objInst = inst->GetDataFlowInput(inputNum);
    if (objInst->GetOpcode() != Opcode::NewObject) {
        return nullptr;
    }
    return GetObjectClass(objInst);
}

RuntimeInterface::ClassPtr GetClass(Inst *inst)
{
    if (inst->IsClassInst()) {
        return static_cast<ClassInst *>(inst)->GetClass();
    }
    if (inst->GetOpcode() == Opcode::LoadImmediate) {
        return inst->CastToLoadImmediate()->GetClass();
    }
    if (inst->GetOpcode() == Opcode::LoadRuntimeClass) {
        return inst->CastToLoadRuntimeClass()->GetClass();
    }
    if (inst->IsPhi()) {
        auto graph = inst->GetBasicBlock()->GetGraph();
        graph->RunPass<ObjectTypePropagation>();
        auto typeInfo = inst->GetObjectTypeInfo();
        if (typeInfo.IsValid() && typeInfo.IsExact()) {
            return typeInfo.GetClass();
        }
        return nullptr;
    }
    UNREACHABLE();
    return nullptr;
}

RuntimeInterface::ClassPtr GetObjectClass(Inst *inst)
{
    ASSERT(inst->GetInputsCount() > 0);
    return GetClass(inst->GetDataFlowInput(0));
}

template bool HasOsrEntryBetween(Inst *dominate, Inst *current);
template bool HasOsrEntryBetween(BasicBlock *dominate, BasicBlock *current);

template <typename T>
bool HasOsrEntryBetween(T *dominate, T *current)
{
    ASSERT(dominate->IsDominate(current));
    BasicBlock *dominateBb = nullptr;
    BasicBlock *bb = nullptr;
    if constexpr (std::is_same_v<T, Inst>) {
        dominateBb = dominate->GetBasicBlock();
        bb = current->GetBasicBlock();
    } else if constexpr (std::is_same_v<T, BasicBlock>) {
        dominateBb = dominate;
        bb = current;
    }
    ASSERT(bb != nullptr);
    auto graph = bb->GetGraph();
    ASSERT(graph->IsOsrMode() || dominateBb != nullptr);
    if (!graph->IsOsrMode() && dominateBb->GetLoop() != bb->GetLoop()) {
        return false;
    }
    MarkerHolder marker(graph);
    return FindBlockBetween<IsOsrEntryBlock>(dominateBb, bb, marker.GetMarker());
}

bool HasTryBlockBetween(Inst *dominateInst, Inst *inst)
{
    ASSERT(dominateInst->IsDominate(inst));
    auto bb = inst->GetBasicBlock();
    MarkerHolder marker(bb->GetGraph());
    return FindBlockBetween<IsTryBlock>(dominateInst->GetBasicBlock(), bb, marker.GetMarker());
}

bool IsSbAppendStringIntrinsic(Inst *inst)
{
    if (UNLIKELY(!inst->IsIntrinsic())) {
        return false;
    }
    auto id = inst->CastToIntrinsic()->GetIntrinsicId();
    return inst->GetBasicBlock()->GetGraph()->GetRuntime()->IsIntrinsicStringBuilderAppendString(id);
}

Inst *IntrinsicStoredValue(Inst *inst)
{
    ASSERT(inst->IsIntrinsic());
    if (IsSbAppendStringIntrinsic(inst)) {
        // input 0 - StringBuilder reference
        // input 1 - String reference to be stored
        return inst->GetInput(1).GetInst();
    }
    UNREACHABLE();
    return nullptr;
}

Inst *InstStoredValue(Inst *inst, Inst **secondValue)
{
    ASSERT_PRINT(inst->IsStore() || IsSbAppendStringIntrinsic(inst),
                 "Attempt to take a stored value on non-store instruction");

    Inst *val = nullptr;
    *secondValue = nullptr;
    switch (inst->GetOpcode()) {
        case Opcode::StoreArray:
        case Opcode::StoreObject:
        case Opcode::StoreStatic:
        case Opcode::StoreArrayI:
        case Opcode::Store:
        case Opcode::StoreNative:
        case Opcode::StoreI:
        case Opcode::StoreObjectDynamic:
            // Last input is a stored value
            val = inst->GetInput(inst->GetInputsCount() - 1).GetInst();
            break;
        case Opcode::StoreResolvedObjectField:
        case Opcode::StoreResolvedObjectFieldStatic:
            val = inst->GetInput(1).GetInst();
            break;
        case Opcode::UnresolvedStoreStatic:
            val = inst->GetInput(0).GetInst();
            break;
        case Opcode::StoreArrayPair:
        case Opcode::StoreArrayPairI: {
            val = inst->GetInput(inst->GetInputsCount() - 2U).GetInst();
            auto secondInst = inst->GetInput(inst->GetInputsCount() - 1U).GetInst();
            *secondValue = inst->GetDataFlowInput(secondInst);
            break;
        }
        case Opcode::StoreObjectPair: {
            val = inst->GetInput(1).GetInst();
            auto secondInst = inst->GetInput(2).GetInst();
            *secondValue = inst->GetDataFlowInput(secondInst);
            break;
        }
        case Opcode::Intrinsic: {
            val = IntrinsicStoredValue(inst);
            break;
        }
        case Opcode::FillConstArray: {
            return nullptr;
        }
        // Unhandled store instructions has been met
        default:
            UNREACHABLE();
    }
    return inst->GetDataFlowInput(val);
}

Inst *InstStoredValue(Inst *inst)
{
    Inst *secondValue = nullptr;
    Inst *val = InstStoredValue(inst, &secondValue);
    ASSERT(secondValue == nullptr);
    return val;
}

SaveStateInst *CopySaveState(Graph *graph, SaveStateInst *inst)
{
    ASSERT(inst != nullptr);
    auto copy = static_cast<SaveStateInst *>(inst->Clone(graph));
    ASSERT(copy->GetCallerInst() == inst->GetCallerInst());
    for (size_t inputIdx = 0; inputIdx < inst->GetInputsCount(); inputIdx++) {
        copy->AppendInput(inst->GetInput(inputIdx));
        copy->SetVirtualRegister(inputIdx, inst->GetVirtualRegister(inputIdx));
    }
    copy->SetLinearNumber(inst->GetLinearNumber());
    return copy;
}

template <typename T>
bool CanArrayAccessBeImplicit(T *array, RuntimeInterface *runtime)
{
    size_t index = array->GetImm();
    auto arch = array->GetBasicBlock()->GetGraph()->GetArch();
    size_t offset = runtime->GetArrayDataOffset(arch) + (index << DataType::ShiftByType(array->GetType(), arch));
    return offset < runtime->GetProtectedMemorySize();
}

bool CanLoadArrayIBeImplicit(const LoadInstI *inst, const Graph *graph, RuntimeInterface *runtime, size_t maxoffset)
{
    ASSERT(inst->CastToLoadArrayI()->IsArray() || !runtime->IsCompressedStringsEnabled());
    auto arch = graph->GetArch();
    size_t dataoffset = inst->IsArray() ? runtime->GetArrayDataOffset(arch) : runtime->GetStringDataOffset(arch);
    size_t shift = DataType::ShiftByType(inst->GetType(), arch);
    size_t offset = dataoffset + (inst->GetImm() << shift);
    return offset < maxoffset;
}

template <typename T>
bool CanObjectAccessBeImplicit(T *object, const Graph *graph, size_t maxoffset)
{
    return GetObjectOffset(graph, object->GetObjectType(), object->GetObjField(), object->GetTypeId()) < maxoffset;
}

template <typename T>
bool CanObjectPairAccessBeImplicit(T *objpair, const Graph *graph, size_t maxoffset)
{
    return GetObjectOffset(graph, objpair->GetObjectType(), objpair->GetObjField1(), objpair->GetTypeId1()) < maxoffset;
}

bool CheckFcmpInputs(Inst *input0, Inst *input1)
{
    if (input0->GetOpcode() != Opcode::Cast || input1->GetOpcode() != Opcode::Cast) {
        return false;
    }
    if (input0->CastToCast()->GetOperandsType() != DataType::INT32 ||
        input1->CastToCast()->GetOperandsType() != DataType::INT32) {
        return false;
    }
    return true;
}

bool CheckFcmpWithConstInput(Inst *input0, Inst *input1)
{
    if (input0->IsConst() && input1->GetOpcode() == Opcode::Cast &&
        input1->CastToCast()->GetOperandsType() == DataType::INT32) {
        return true;
    }
    if (input1->IsConst() && input0->GetOpcode() == Opcode::Cast &&
        input0->CastToCast()->GetOperandsType() == DataType::INT32) {
        return true;
    }
    return false;
}

bool CheckTypedArrayLengthObject(Inst *inst)
{
    auto loadObject = inst->CastToLoadObject();
    auto runtime = inst->GetBasicBlock()->GetGraph()->GetRuntime();
    auto type = loadObject->GetObjectType();
    auto field = loadObject->GetObjField();
    if (type != ObjectType::MEM_STATIC && type != ObjectType::MEM_OBJECT) {
        return false;
    }
    return runtime->IsFieldTypedArrayLengthInt(field);
}

// Get power of 2
// if n not power of 2 return -1;
int64_t GetPowerOfTwo(uint64_t n)
{
    if (!helpers::math::IsPowerOfTwo(n)) {
        return -1;
    }
    return helpers::math::GetIntLog2(n);
}

bool IsInputTypeMismatch(Inst *inst, int32_t inputIndex, Arch arch)
{
    auto inputInst = inst->GetInput(inputIndex).GetInst();
    auto inputTypeSize = DataType::GetTypeSize(inputInst->GetType(), arch);
    auto instInputSize = DataType::GetTypeSize(inst->GetInputType(inputIndex), arch);
    return (inputTypeSize > instInputSize) ||
           (inputTypeSize == instInputSize &&
            DataType::IsTypeSigned(inputInst->GetType()) != DataType::IsTypeSigned(inst->GetInputType(inputIndex)));
}

bool ApplyForCastJoin(Inst *cast, Inst *input, Inst *origInst, Arch arch)
{
    auto inputTypeMismatch = IsInputTypeMismatch(cast, 0, arch);
#ifndef NDEBUG
    ASSERT(!inputTypeMismatch);
#else
    if (inputTypeMismatch) {
        return false;
    }
#endif
    auto inputType = input->GetType();
    auto inputTypeSize = DataType::GetTypeSize(inputType, arch);
    auto currType = cast->GetType();
    auto origType = origInst->GetType();
    return DataType::GetCommonType(inputType) == DataType::INT64 &&
           DataType::GetCommonType(currType) == DataType::INT64 &&
           DataType::GetCommonType(origType) == DataType::INT64 &&
           inputTypeSize > DataType::GetTypeSize(currType, arch) &&
           DataType::GetTypeSize(origType, arch) > inputTypeSize;
}

bool IsCastAllowedInBytecode(const Inst *inst)
{
    auto type = inst->GetType();
    switch (type) {
        case DataType::Type::INT32:
        case DataType::Type::INT64:
        case DataType::Type::FLOAT32:
        case DataType::Type::FLOAT64:
            return true;
        default:
            return false;
    }
}

// OverflowCheck can be removed if all its indirect users truncate input to int32
bool CanRemoveOverflowCheck(Inst *inst, Marker marker)
{
    if (inst->SetMarker(marker)) {
        return true;
    }
    if (inst->GetOpcode() == Opcode::AnyTypeCheck) {
        auto anyTypeCheck = inst->CastToAnyTypeCheck();
        auto graph = inst->GetBasicBlock()->GetGraph();
        auto language = graph->GetRuntime()->GetMethodSourceLanguage(graph->GetMethod());
        auto intAnyType = NumericDataTypeToAnyType(DataType::INT32, language);
        // Bail out if this AnyTypeCheck can be triggered
        if (IsAnyTypeCanBeSubtypeOf(language, anyTypeCheck->GetAnyType(), intAnyType,
                                    anyTypeCheck->GetAllowedInputType()) != true) {
            return false;
        }
    }
    switch (inst->GetOpcode()) {
        case Opcode::Shl:
        case Opcode::Shr:
        case Opcode::AShr:
        case Opcode::And:
        case Opcode::Or:
        case Opcode::Xor:
            return true;
        // Next 3 opcodes should be removed in Peepholes and ChecksElimination, but Cast(f64).i32 or Or(x, 0)
        // may be optimized out by that moment
        case Opcode::CastAnyTypeValue:
        case Opcode::CastValueToAnyType:
        case Opcode::AnyTypeCheck:

        case Opcode::AddOverflowCheck:
        case Opcode::SubOverflowCheck:
        case Opcode::NegOverflowAndZeroCheck:
        case Opcode::Phi:
        // We check SaveState because if some of its users cannot be removed, result of OverflowCheck
        // may be used in interpreter after deoptimization
        case Opcode::SaveState:
            for (auto &user : inst->GetUsers()) {
                auto userInst = user.GetInst();
                bool canRemove;
                if (DataType::IsFloatType(inst->GetType())) {
                    canRemove = userInst->GetOpcode() == Opcode::Cast && userInst->CastToCast()->IsDynamicCast();
                } else {
                    canRemove = CanRemoveOverflowCheck(userInst, marker);
                }
                if (!canRemove) {
                    return false;
                }
            }
            return true;
        default:
            return false;
    }
}

bool IsSuitableForImplicitNullCheck(const Inst *inst)
{
    auto graph = inst->GetBasicBlock()->GetGraph();
    auto runtime = graph->GetRuntime();
    size_t maxOffset = runtime->GetProtectedMemorySize();
    switch (inst->GetOpcode()) {
        case Opcode::LoadArray:
        case Opcode::StoreArray:
        case Opcode::LoadArrayPair:
        case Opcode::StoreArrayPair: {
            // we don't know array index, so offset can be more than protected memory
            return false;
        }
        case Opcode::LoadArrayI:
            return CanLoadArrayIBeImplicit(inst->CastToLoadArrayI(), graph, runtime, maxOffset);
        case Opcode::LenArray:
            return true;
        case Opcode::LoadObject:
            return CanObjectAccessBeImplicit(inst->CastToLoadObject(), graph, maxOffset);
        case Opcode::StoreObject:
            return CanObjectAccessBeImplicit(inst->CastToStoreObject(), graph, maxOffset);
        case Opcode::StoreArrayI:
            return CanArrayAccessBeImplicit(inst->CastToStoreArrayI(), runtime);
        case Opcode::LoadArrayPairI:
            return CanArrayAccessBeImplicit(inst->CastToLoadArrayPairI(), runtime);
        case Opcode::StoreArrayPairI:
            return CanArrayAccessBeImplicit(inst->CastToStoreArrayPairI(), runtime);
        case Opcode::LoadObjectPair:
            return CanObjectPairAccessBeImplicit(inst->CastToLoadObjectPair(), graph, maxOffset);
        case Opcode::StoreObjectPair:
            return CanObjectPairAccessBeImplicit(inst->CastToStoreObjectPair(), graph, maxOffset);
        default:
            return false;
    }
}

bool IsInstNotNull(const Inst *inst)
{
    ASSERT(inst != nullptr);
    // Allocations cannot return null pointer
    if (inst->IsAllocation() || inst->IsNullCheck() || inst->NoNullPtr()) {
        return true;
    }
    if (inst->IsParameter() && inst->CastToParameter()->GetArgNumber() == 0) {
        auto graph = inst->GetBasicBlock()->GetGraph();
        // The object is not null if object is first parameter and the method is virtual.
        return !graph->GetRuntime()->IsMethodStatic(graph->GetMethod());
    }
    return false;
}

// Returns true if GC can be triggered at this point
bool IsSaveStateForGc(const Inst *inst)
{
    if (inst->GetOpcode() == Opcode::SafePoint) {
        return true;
    }
    if (inst->GetOpcode() == Opcode::SaveState) {
        for (auto &user : inst->GetUsers()) {
            if (user.GetInst()->IsRuntimeCall()) {
                return true;
            }
        }
    }
    return false;
}

// Checks if input edges of phi_block come from different branches of dominating if_imm instruction
// Returns true if the first input is in true branch, false if it is in false branch, and std::nullopt
// if branches intersect
std::optional<bool> IsIfInverted(BasicBlock *phiBlock, IfImmInst *ifImm)
{
    auto ifBlock = ifImm->GetBasicBlock();
    ASSERT(ifBlock == phiBlock->GetDominator());
    ASSERT(phiBlock->GetPredsBlocks().size() == MAX_SUCCS_NUM);
    auto trueBb = ifImm->GetEdgeIfInputTrue();
    auto falseBb = ifImm->GetEdgeIfInputFalse();
    auto pred0 = phiBlock->GetPredecessor(0);
    auto pred1 = phiBlock->GetPredecessor(1);

    // Triangle case: phi block is the first in true branch
    if (trueBb == phiBlock && falseBb->GetPredsBlocks().size() == 1) {
        return pred0 != ifBlock;
    }
    // Triangle case: phi block is the first in false branch
    if (falseBb == phiBlock && trueBb->GetPredsBlocks().size() == 1) {
        return pred0 == ifBlock;
    }
    // If true_bb has more than one predecessor, there can be a path from false_bb
    // to true_bb avoiding if_imm
    if (trueBb->GetPredsBlocks().size() > 1 || falseBb->GetPredsBlocks().size() > 1) {
        return std::nullopt;
    }
    // Every path through first input edge to phi_block comes from true branch
    // Every path through second input edge to phi_block comes from false branch
    if (trueBb->IsDominate(pred0) && falseBb->IsDominate(pred1)) {
        return false;
    }
    // Every path through first input edge to phi_block comes from false branch
    // Every path through second input edge to phi_block comes from true branch
    if (falseBb->IsDominate(pred0) && trueBb->IsDominate(pred1)) {
        return true;
    }
    // True and false branches intersect
    return std::nullopt;
}

ArenaVector<Inst *> *SaveStateBridgesBuilder::SearchMissingObjInSaveStates(Graph *graph, Inst *source, Inst *target,
                                                                           Inst *stopSearch, BasicBlock *targetBlock)
{
    ASSERT(graph != nullptr);
    ASSERT(source != nullptr);
    ASSERT(targetBlock != nullptr);
    ASSERT(source->IsMovableObject());

    AllocOrClear(graph, &bridges_);

    auto visited = graph->NewMarker();
    SearchSSOnWay(targetBlock, target, source, visited, stopSearch);
    graph->EraseMarker(visited);
    return bridges_;
}

void SaveStateBridgesBuilder::SearchSSOnWay(BasicBlock *block, Inst *startFrom, Inst *sourceInst, Marker visited,
                                            Inst *stopSearch)
{
    ASSERT(block != nullptr);
    ASSERT(sourceInst != nullptr);
    ASSERT(bridges_ != nullptr);

    if (startFrom != nullptr) {
        auto it = InstSafeIterator<IterationType::ALL, IterationDirection::BACKWARD>(*block, startFrom);
        for (; it != block->AllInstsSafeReverse().end(); ++it) {
            auto inst = *it;
            if (inst == nullptr) {
                break;
            }
            COMPILER_LOG(DEBUG, BRIDGES_SS) << " See inst" << *inst;

            if (inst->SetMarker(visited)) {
                return;
            }
            if (SaveStateBridgesBuilder::IsSaveStateForGc(inst)) {
                COMPILER_LOG(DEBUG, BRIDGES_SS) << "\tSearch in SS";
                SearchInSaveStateAndFillBridgeVector(inst, sourceInst);
            }
            // When "stop_search" is nullptr second clause never causes early exit here
            if (inst == sourceInst || inst == stopSearch) {
                return;
            }
        }
    }

    auto sourceBlock = sourceInst->GetBasicBlock();
    for (auto pred : block->GetPredsBlocks()) {
        if (!sourceBlock->IsDominate(pred)) {
            continue;
        }
        SearchSSOnWay(pred, pred->GetLastInst(), sourceInst, visited, stopSearch);
    }
}

void SaveStateBridgesBuilder::SearchInSaveStateAndFillBridgeVector(Inst *inst, Inst *searchedInst)
{
    ASSERT(inst != nullptr);
    ASSERT(searchedInst != nullptr);
    ASSERT(bridges_ != nullptr);
    auto user = std::find_if(inst->GetInputs().begin(), inst->GetInputs().end(), [searchedInst, inst](Input input) {
        return inst->GetDataFlowInput(input.GetInst()) == searchedInst;
    });
    if (user == inst->GetInputs().end()) {
        COMPILER_LOG(DEBUG, BRIDGES_SS) << "\tNot found";
        bridges_->push_back(inst);
    }
}

void SaveStateBridgesBuilder::FixUsagePhiInBB(BasicBlock *block, Inst *inst)
{
    ASSERT(block != nullptr);
    ASSERT(inst != nullptr);
    if (inst->IsMovableObject()) {
        for (auto &user : inst->GetUsers()) {
            auto targetInst = user.GetInst();
            COMPILER_LOG(DEBUG, BRIDGES_SS) << " Check usage: Try to do SSB for inst: " << inst->GetId() << "\t"
                                            << " For target inst: " << targetInst->GetId() << "\n";
            // If inst usage in other BB than in all case object must exist until the end of the BB
            if (targetInst->IsPhi() || targetInst->GetBasicBlock() != block) {
                targetInst = block->GetLastInst();
            }
            SearchAndCreateMissingObjInSaveState(block->GetGraph(), inst, targetInst, block->GetFirstInst());
        }
    }
}

void SaveStateBridgesBuilder::FixUsageInstInOtherBB(BasicBlock *block, Inst *inst)
{
    ASSERT(block != nullptr);
    ASSERT(inst != nullptr);
    if (inst->IsMovableObject()) {
        for (auto &user : inst->GetUsers()) {
            auto targetInst = user.GetInst();
            // This way "in same block" checked when we saw inputs of instructions
            if (targetInst->GetBasicBlock() == block) {
                continue;
            }
            COMPILER_LOG(DEBUG, BRIDGES_SS) << " Check inputs: Try to do SSB for real source inst: " << *inst << "\n"
                                            << "  For target inst: " << *targetInst << "\n";
            // If inst usage in other BB than in all case object must must exist until the end of the BB
            targetInst = block->GetLastInst();
            SearchAndCreateMissingObjInSaveState(block->GetGraph(), inst, targetInst, block->GetFirstInst());
        }
    }
}

void SaveStateBridgesBuilder::DeleteUnrealObjInSaveState(Inst *ss)
{
    ASSERT(ss != nullptr);
    size_t indexInput = 0;
    for (auto &input : ss->GetInputs()) {
        // If the user of SS before inst
        auto inputInst = input.GetInst();
        if (ss->GetBasicBlock() == inputInst->GetBasicBlock() && ss->IsDominate(inputInst)) {
            ss->RemoveInput(indexInput);
            COMPILER_LOG(DEBUG, BRIDGES_SS) << " Fixed incorrect user in ss: " << ss->GetId() << "  "
                                            << " deleted input: " << inputInst->GetId() << "\n";
        }
        indexInput++;
    }
}

void SaveStateBridgesBuilder::FixSaveStatesInBB(BasicBlock *block)
{
    ASSERT(block != nullptr);
    bool blockInLoop = !(block->GetLoop()->IsRoot());
    // Check usage ".ref" PHI inst
    for (auto phi : block->PhiInsts()) {
        FixUsagePhiInBB(block, phi);
    }
    // Check all insts
    for (auto inst : block->Insts()) {
        if (IsSaveStateForGc(inst)) {
            DeleteUnrealObjInSaveState(inst);
        }
        // Check reference inputs of instructions
        for (auto &input : inst->GetInputs()) {
            // We record the original object in SaveState without checks
            auto realSourceInst = inst->GetDataFlowInput(input.GetInst());
            if (!realSourceInst->IsMovableObject()) {
                continue;
            }
            // In case, when usege of object in loop and definition is not in loop or usage's loop inside
            // definition's loop, we should check SaveStates till the end of BasicBlock
            if (blockInLoop && (block->GetLoop()->IsInside(realSourceInst->GetBasicBlock()->GetLoop()))) {
                COMPILER_LOG(DEBUG, BRIDGES_SS)
                    << " Check inputs: Try to do SSB for real source inst: " << *realSourceInst << "\n"
                    << "  Block in loop:  " << block->GetLoop() << " So target is end of BB:" << *(block->GetLastInst())
                    << "\n";
                SearchAndCreateMissingObjInSaveState(block->GetGraph(), realSourceInst, block->GetLastInst(),
                                                     block->GetFirstInst());
            } else {
                COMPILER_LOG(DEBUG, BRIDGES_SS)
                    << " Check inputs: Try to do SSB for real source inst: " << *realSourceInst << "\n"
                    << "  For target inst: " << *inst << "\n";
                SearchAndCreateMissingObjInSaveState(block->GetGraph(), realSourceInst, inst, block->GetFirstInst());
            }
        }
        // Check usage reference instruction
        FixUsageInstInOtherBB(block, inst);
    }
}

bool SaveStateBridgesBuilder::IsSaveStateForGc(Inst *inst)
{
    return inst->GetOpcode() == Opcode::SafePoint || inst->GetOpcode() == Opcode::SaveState ||
           inst->GetOpcode() == Opcode::SaveStateDeoptimize;
}

void SaveStateBridgesBuilder::CreateBridgeInSS(Inst *source)
{
    ASSERT(bridges_ != nullptr);
    ASSERT(source != nullptr);
    ASSERT(source->IsMovableObject());

    for (Inst *ss : *bridges_) {
        static_cast<SaveStateInst *>(ss)->AppendBridge(source);
    }
}

void SaveStateBridgesBuilder::SearchAndCreateMissingObjInSaveState(Graph *graph, Inst *source, Inst *target,
                                                                   Inst *stopSearchInst, BasicBlock *targetBlock)
{
    ASSERT(graph != nullptr);
    ASSERT(source != nullptr);
    ASSERT(source->IsMovableObject());

    if (graph->IsBytecodeOptimizer()) {
        return;  // SaveState bridges useless when bytecode optimizer enabled.
    }

    if (targetBlock == nullptr) {
        ASSERT(target != nullptr);
        targetBlock = target->GetBasicBlock();
    } else {
        // if target is nullptr, we won't traverse the target block
        ASSERT(target == nullptr || target->GetBasicBlock() == targetBlock);
    }
    SearchMissingObjInSaveStates(graph, source, target, stopSearchInst, targetBlock);
    if (!bridges_->empty()) {
        CreateBridgeInSS(source);
        COMPILER_LOG(DEBUG, BRIDGES_SS) << " Created bridge(s)";
    }
}

void SaveStateBridgesBuilder::FixInstUsageInSS(Graph *graph, Inst *inst)
{
    if (!inst->IsMovableObject()) {
        return;
    }

    AllocOrClear(graph, &users_);
    PrepareUsers(inst, users_);

    for (auto &user : *users_) {
        auto *targetInst = user->GetInst();
        COMPILER_LOG(DEBUG, BRIDGES_SS) << " Check usage: Try to do SSB for real source inst: " << *inst << "\n"
                                        << "  For target inst: " << *targetInst << "\n";
        if (targetInst->IsPhi()) {
            // check SaveStates on path between inst and the end of corresponding predecessor of Phi's block
            auto *predBlock = targetInst->GetBasicBlock()->GetPredecessor(user->GetBbNum());
            SearchAndCreateMissingObjInSaveState(graph, inst, predBlock->GetLastInst(), nullptr, predBlock);
        } else {
            SearchAndCreateMissingObjInSaveState(graph, inst, targetInst);
        }
    }
}

// Check instructions don't have their own vregs and thus are not added in SaveStates,
// but newly added Phi instructions with check inputs should be added
void SaveStateBridgesBuilder::FixPhisWithCheckInputs(BasicBlock *block)
{
    if (block == nullptr) {
        return;
    }
    auto graph = block->GetGraph();
    for (auto phi : block->PhiInsts()) {
        if (!phi->IsMovableObject()) {
            continue;
        }
        for (auto &input : phi->GetInputs()) {
            if (input.GetInst()->IsCheck()) {
                FixInstUsageInSS(graph, phi);
                break;
            }
        }
    }
}

void SaveStateBridgesBuilder::DumpBridges(std::ostream &out, Inst *source)
{
    ASSERT(source != nullptr);
    ASSERT(bridges_ != nullptr);
    out << "Inst id " << source->GetId() << " with type ";
    source->DumpOpcode(&out);
    out << "need bridge in SS id: ";
    for (auto ss : *bridges_) {
        out << ss->GetId() << " ";
    }
    out << '\n';
}

bool StoreValueCanBeObject(Inst *inst)
{
    switch (inst->GetOpcode()) {
        case Opcode::CastValueToAnyType: {
            auto type = AnyBaseTypeToDataType(inst->CastToCastValueToAnyType()->GetAnyType());
            return (type == DataType::ANY || type == DataType::REFERENCE);
        }
        case Opcode::Constant:
            return false;
        default:
            return true;
    }
}

bool IsConditionEqual(const Inst *inst0, const Inst *inst1, bool inverted)
{
    if (inst0->GetOpcode() != inst1->GetOpcode()) {
        return false;
    }
    if (inst0->GetOpcode() != Opcode::IfImm) {
        // investigate why Opcode::If cannot be lowered to Opcode::IfImm and support it if needed
        return false;
    }
    auto ifImm0 = inst0->CastToIfImm();
    auto ifImm1 = inst1->CastToIfImm();
    auto opcode = ifImm0->GetInput(0).GetInst()->GetOpcode();
    if (opcode != ifImm1->GetInput(0).GetInst()->GetOpcode()) {
        return false;
    }
    if (ifImm0->GetImm() != 0 && ifImm0->GetImm() != 1) {
        return false;
    }
    if (ifImm1->GetImm() != 0 && ifImm1->GetImm() != 1) {
        return false;
    }
    if (ifImm0->GetImm() != ifImm1->GetImm()) {
        inverted = !inverted;
    }
    if (opcode != Opcode::Compare) {
        if (ifImm0->GetInput(0).GetInst() != ifImm1->GetInput(0).GetInst()) {
            return false;
        }
        auto cc = inverted ? GetInverseConditionCode(ifImm0->GetCc()) : ifImm0->GetCc();
        return cc == ifImm1->GetCc();
    }
    auto cmp0 = ifImm0->GetInput(0).GetInst()->CastToCompare();
    auto cmp1 = ifImm1->GetInput(0).GetInst()->CastToCompare();
    if (cmp0->GetInput(0).GetInst() == cmp1->GetInput(0).GetInst() &&
        cmp0->GetInput(1).GetInst() == cmp1->GetInput(1).GetInst()) {
        if (GetInverseConditionCode(ifImm0->GetCc()) == ifImm1->GetCc()) {
            inverted = !inverted;
        } else if (ifImm0->GetCc() != ifImm1->GetCc()) {
            return false;
        }
        auto cc = inverted ? GetInverseConditionCode(cmp0->GetCc()) : cmp0->GetCc();
        return cc == cmp1->GetCc();
    }
    return false;
}

void CleanupGraphSaveStateOSR(Graph *graph)
{
    ASSERT(graph != nullptr);
    ASSERT(graph->IsOsrMode());
    graph->InvalidateAnalysis<LoopAnalyzer>();
    graph->RunPass<LoopAnalyzer>();
    for (auto block : graph->GetBlocksRPO()) {
        if (block->IsOsrEntry() && !block->IsLoopHeader()) {
            auto firstInst = block->GetFirstInst();
            if (firstInst == nullptr) {
                continue;
            }
            if (firstInst->GetOpcode() == Opcode::SaveStateOsr) {
                block->RemoveInst(firstInst);
                block->SetOsrEntry(false);
            }
        }
    }
}

template <typename T>
bool FindInThisBlock(Inst *currInst, Inst *finish)
{
    while (currInst != finish) {
        if (T()(currInst)) {
            return true;
        }
        currInst = currInst->GetPrev();
    }
    return false;
}

template <typename T>
bool FindInstBetween(Inst *domInst, BasicBlock *currentBb, Marker marker)
{
    if (currentBb->SetMarker(marker)) {
        return false;
    }
    bool isSameBlock = domInst->GetBasicBlock() == currentBb;
    auto currInst = currentBb->GetLastInst();
    Inst *finish = isSameBlock ? domInst : nullptr;
    if (FindInThisBlock<T>(currInst, finish)) {
        return true;
    }
    if (isSameBlock) {
        return false;
    }
    for (auto pred : currentBb->GetPredsBlocks()) {
        if (FindInstBetween<T>(domInst, pred, marker)) {
            return true;
        }
    }
    return false;
}

template bool HasSaveStateBetween<IsSaveState>(Inst *dom_inst, Inst *inst);
template bool HasSaveStateBetween<IsSaveStateCanTriggerGc>(Inst *dom_inst, Inst *inst);

template <typename T>
bool HasSaveStateBetween(Inst *domInst, Inst *inst)
{
    ASSERT(domInst->IsDominate(inst));
    if (domInst == inst) {
        return false;
    }
    auto bb = inst->GetBasicBlock();
    bool isSameBlock = domInst->GetBasicBlock() == bb;
    auto currInst = inst->GetPrev();
    Inst *finish = isSameBlock ? domInst : nullptr;
    if (FindInThisBlock<T>(currInst, finish)) {
        return true;
    }
    if (isSameBlock) {
        return false;
    }
    MarkerHolder marker(bb->GetGraph());
    for (auto pred : bb->GetPredsBlocks()) {
        if (FindInstBetween<T>(domInst, pred, marker.GetMarker())) {
            return true;
        }
    }
    return false;
}

void InstAppender::Append(Inst *inst)
{
    if (prev_ == nullptr) {
        block_->AppendInst(inst);
    } else {
        block_->InsertAfter(inst, prev_);
    }
    prev_ = inst;
}

void InstAppender::Append(std::initializer_list<Inst *> instructions)
{
    for (auto *inst : instructions) {
        Append(inst);
    }
}

CallInst *FindCallerInst(BasicBlock *target, Inst *start)
{
    auto block = start == nullptr ? target->GetDominator() : target;
    size_t depth = 0;
    while (block != nullptr) {
        auto iter = InstBackwardIterator<IterationType::INST>(*block, start);
        for (auto inst : iter) {
            if (inst == nullptr) {
                return nullptr;
            }
            if (inst == start) {
                continue;
            }
            if (inst->Is(Opcode::ReturnInlined)) {
                depth++;
            }
            if (!inst->IsCall()) {
                continue;
            }
            auto callInst = static_cast<CallInst *>(inst);
            auto isInlined = callInst->IsInlined();
            if (isInlined && depth == 0) {
                return callInst;
            }
            if (isInlined) {
                depth--;
            }
        }
        block = block->GetDominator();
        start = nullptr;
    }
    return nullptr;
}

void PrepareUsers(Inst *inst, ArenaVector<User *> *users)
{
    for (auto &user : inst->GetUsers()) {
        users->push_back(&user);
    }

    auto findCheckUser = [](auto begin, auto end) {
        return std::find_if(begin, end, [](User *user) { return user->GetInst()->IsCheck(); });
    };

    auto i = findCheckUser(users->begin(), users->end());
    if (i == users->end()) {
        return;
    }

    do {
        auto pos = std::distance(users->begin(), i);
        auto userInst = (*i)->GetInst();
        // skip AnyTypeChecks with primitive type
        if (userInst->GetOpcode() != Opcode::AnyTypeCheck || userInst->IsReferenceOrAny()) {
            for (auto &u : userInst->GetUsers()) {
                users->push_back(&u);
            }
        }
        i = findCheckUser(users->begin() + pos + 1, users->end());
    } while (i != users->end());

    auto newEnd = std::remove_if(users->begin(), users->end(), [](User *user) { return user->GetInst()->IsCheck(); });
    users->erase(newEnd, users->end());
}
}  // namespace ark::compiler
