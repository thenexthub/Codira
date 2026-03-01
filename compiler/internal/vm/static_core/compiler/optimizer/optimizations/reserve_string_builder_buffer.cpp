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

#include "reserve_string_builder_buffer.h"
#include "compiler_logger.h"
#include "optimizer/analysis/alias_analysis.h"
#include "optimizer/analysis/bounds_analysis.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/ir/analysis.h"
#include "optimizer/ir/inst.h"
#include "optimizer/optimizations/cleanup.h"
#include "optimizer/optimizations/string_builder_utils.h"
#include "runtime/include/coretypes/string.h"

namespace ark::compiler {

constexpr size_t ARG_IDX_0 = 0;
constexpr size_t ARG_IDX_1 = 1;
constexpr size_t ARG_IDX_2 = 2;
constexpr uint64_t INVALID_COUNT = std::numeric_limits<uint64_t>::max();
constexpr size_t TLAB_MAX_ALLOC_SIZE = 4_KB;
constexpr uint64_t SB_INITIAL_BUFFER_SIZE = 16;

ReserveStringBuilderBuffer::ReserveStringBuilderBuffer(Graph *graph)
    : Optimization(graph), blockWeightsMap_ {graph->GetLocalAllocator()->Adapter()}
{
    // String Builder internal buffer size is limited by TLAB max allocation size
    // minus offset reserved for an array header (elements info, array size)
    auto arrayDataOffset = graph->GetRuntime()->GetArrayDataOffset(graph->GetArch());
    tlabArraySizeMax_ = (TLAB_MAX_ALLOC_SIZE - arrayDataOffset) / sizeof(ObjectPointerType);
}

uint64_t GetLoopIterationsCount(Loop *loop)
{
    auto loopParser = CountableLoopParser(*loop);
    auto loopInfo = loopParser.Parse();
    if (!loopInfo.has_value()) {
        return INVALID_COUNT;
    }

    auto loopCount = CountableLoopParser::GetLoopIterations(loopInfo.value());
    return loopCount.value_or(INVALID_COUNT);
}

bool HasAppendInstructions(Inst *instance, BasicBlock *block)
{
    for (auto appendInstruction : block->Insts()) {
        if (!IsStringBuilderAppend(appendInstruction)) {
            continue;
        }

        if (appendInstruction->GetDataFlowInput(0) == instance) {
            return true;
        }
    }

    return false;
}

bool HasAppendInstructions(Inst *instance, Loop *loop)
{
    for (auto innerLoop : loop->GetInnerLoops()) {
        if (HasAppendInstructions(instance, innerLoop)) {
            return true;
        }
    }

    for (auto block : loop->GetBlocks()) {
        if (HasAppendInstructions(instance, block)) {
            return true;
        }
    }

    return false;
}

Inst *GetStringBuilderAppendChainInstance(Inst *appendInstruction)
{
    // For code like this:
    //      sb.append(a).append(b).append(c) ...
    // returns 'sb' instance for any append call in a chain

    ASSERT(IsStringBuilderAppend(appendInstruction));
    ASSERT(appendInstruction->GetInputsCount() > 0);

    auto inputInst = appendInstruction->GetDataFlowInput(0);
    while (IsStringBuilderAppend(inputInst)) {
        ASSERT(inputInst->GetInputsCount() > 0);
        inputInst = inputInst->GetDataFlowInput(0);
    }

    return inputInst;
}

uint64_t CountStringBuilderAppendCalls(Inst *instance, BasicBlock *block)
{
    uint64_t count = 0;
    for (auto appendInstruction : block->Insts()) {
        if (!IsStringBuilderAppend(appendInstruction)) {
            continue;
        }

        ASSERT(appendInstruction->GetInputsCount() > 0);
        if (appendInstruction->GetDataFlowInput(0) == instance ||
            GetStringBuilderAppendChainInstance(appendInstruction) == instance) {
            count += appendInstruction->GetInputsCount() - 2U;  // -2 for sb instance and save state
        }
    }
    return count;
}

bool IsFieldStringBuilderBuffer(StoreObjectInst *storeObject)
{
    auto field = storeObject->GetObjField();
    auto runtime = storeObject->GetBasicBlock()->GetGraph()->GetRuntime();

    return runtime->IsFieldStringBuilderBuffer(field);
}

bool IsFieldStringBuilderIndex(StoreObjectInst *storeObject)
{
    auto field = storeObject->GetObjField();
    auto runtime = storeObject->GetBasicBlock()->GetGraph()->GetRuntime();

    return runtime->IsFieldStringBuilderIndex(field);
}

void ReserveStringBuilderBuffer::ReplaceInitialBufferSizeConstantInlined(Inst *instance, uint64_t appendCallsCount)
{
    for (auto &user : instance->GetUsers()) {
        auto storeObject = SkipSingleUserCheckInstruction(user.GetInst());
        if (storeObject->GetOpcode() != Opcode::StoreObject) {
            continue;
        }

        if (!IsFieldStringBuilderBuffer(storeObject->CastToStoreObject())) {
            continue;
        }

        // storeObject instruction is
        //  N.ref  StoreObject std.core.StringBuilder.buf instance, newArray
        ASSERT(storeObject->GetInputsCount() > 1);
        ASSERT(storeObject->GetDataFlowInput(0) == instance);

        // newArray instruction is
        //  M.ref  NewArray objectClass, originalSize, saveState
        auto newArray = storeObject->GetInput(1).GetInst()->CastToNewArray();
        ASSERT(newArray->GetInputsCount() > 1);
        auto originalSize = newArray->GetInput(1).GetInst();

        // Create Constant instruction for new Object[] array size
        auto newSize = GetGraph()->FindOrCreateConstant(appendCallsCount);

        // Replace originalSize itself
        newArray->SetInput(1, newSize);

        // Remove unused check instruction, because it won't be deleted automatically by Cleanup
        if (originalSize->IsCheck() && !originalSize->HasUsers()) {
            originalSize->ClearFlag(inst_flags::NO_DCE);
        }

        isApplied_ = true;
    }
}

IntrinsicInst *CreateIntrinsicStringIsCompressed(Graph *graph, Inst *arg, SaveStateInst *saveState)
{
    auto intrinsic = graph->CreateInstIntrinsic(graph->GetRuntime()->GetStringIsCompressedIntrinsicId());
    ASSERT(intrinsic->RequireState());

    intrinsic->SetType(DataType::BOOL);
    intrinsic->SetInputs(graph->GetAllocator(), {{arg, arg->GetType()}, {saveState, saveState->GetType()}});

    return intrinsic;
}

using StringBuilderConstructorSignature = StringCtorType;
StringBuilderConstructorSignature GetStringBuilderConstructorSignature(Inst *instance, Inst *&ctorCall, Inst *&ctorArg)
{
    ASSERT(IsStringBuilderInstance(instance));

    for (auto &user : instance->GetUsers()) {
        auto userInst = user.GetInst();
        if (IsMethodStringBuilderDefaultConstructor(userInst)) {
            ctorCall = userInst;
            ctorArg = nullptr;
            return StringBuilderConstructorSignature::UNKNOWN;
        }
        if (IsMethodStringBuilderConstructorWithStringArg(userInst)) {
            ctorCall = userInst;
            ASSERT(userInst->GetInputsCount() > 1);
            ctorArg = userInst->GetDataFlowInput(1);
            return StringBuilderConstructorSignature::STRING;
        }
        if (IsMethodStringBuilderConstructorWithCharArrayArg(userInst)) {
            ctorCall = userInst;
            ASSERT(userInst->GetInputsCount() > 1);
            ctorArg = userInst->GetDataFlowInput(1);
            return StringBuilderConstructorSignature::CHAR_ARRAY;
        }
    }

    UNREACHABLE();
}

Inst *CreateInstructionNewObjectsArray(Graph *graph, Inst *ctorCall, uint64_t size)
{
    auto runtime = graph->GetRuntime();
    auto method = graph->GetMethod();

    // Create LoadAndInitClass instruction for Object[] type
    auto objectsArrayClassId = runtime->GetClassOffsetObjectsArray(method);
    if (objectsArrayClassId == 0U) {
        return nullptr;
    }

    auto ctorCallMethod = ctorCall->GetSaveState()->GetMethod();
    auto objectsArrayClassIdForCall = runtime->GetClassOffsetObjectsArray(ctorCallMethod);
    auto loadClassObjectsArray = graph->CreateInstLoadAndInitClass(
        DataType::REFERENCE, ctorCall->GetPc(), CopySaveState(graph, ctorCall->GetSaveState()),
        TypeIdMixin {objectsArrayClassIdForCall, ctorCallMethod},
        runtime->ResolveType(ctorCallMethod, objectsArrayClassIdForCall));
    InsertBeforeWithSaveState(loadClassObjectsArray, ctorCall->GetSaveState());

    // Create Constant instruction for new Object[] array size
    auto sizeConstant = graph->FindOrCreateConstant(size);

    // Create NewArray instruction for Object[] type and new array size
    auto newObjectsArray = graph->CreateInstNewArray(DataType::REFERENCE, ctorCall->GetPc(), loadClassObjectsArray,
                                                     sizeConstant, CopySaveState(graph, ctorCall->GetSaveState()),
                                                     TypeIdMixin {objectsArrayClassId, method});
    InsertBeforeWithSaveState(newObjectsArray, ctorCall->GetSaveState());

    return newObjectsArray;
}

void StoreStringBuilderConstructorArgument(Graph *graph, Inst *arg, Inst *newObjectsArray, Inst *ctorCall)
{
    auto storeArray = graph->CreateInstStoreArray(DataType::REFERENCE, ctorCall->GetPc());
    storeArray->SetInput(ARG_IDX_0, newObjectsArray);
    storeArray->SetInput(ARG_IDX_1, graph->FindOrCreateConstant(0));
    storeArray->SetInput(ARG_IDX_2, arg);
    storeArray->SetNeedBarrier(true);
    InsertBeforeWithSaveState(storeArray, ctorCall->GetSaveState());
}

Inst *CreateStringBuilderConstructorArgumentLength(Graph *graph, Inst *arg, Inst *ctorCall)
{
    auto lenArray = graph->CreateInstLenArray(DataType::INT32, ctorCall->GetPc(), arg);
    InsertBeforeWithSaveState(lenArray, ctorCall->GetSaveState());

    auto argLength = graph->CreateInstShr(DataType::INT32, ctorCall->GetPc());
    argLength->SetInput(ARG_IDX_0, lenArray);
    argLength->SetInput(ARG_IDX_1, graph->FindOrCreateConstant(ark::coretypes::String::STRING_LENGTH_SHIFT));
    InsertBeforeWithSaveState(argLength, ctorCall->GetSaveState());

    return argLength;
}

Inst *CreateStringBuilderConstructorArgumentIsCompressed(Graph *graph, Inst *arg, Inst *ctorCall)
{
    auto argIsCompressed =
        CreateIntrinsicStringIsCompressed(graph, arg, CopySaveState(graph, ctorCall->GetSaveState()));
    InsertBeforeWithSaveState(argIsCompressed, ctorCall->GetSaveState());

    return argIsCompressed;
}

Inst *StoreStringBuilderBufferField(Graph *graph, Inst *buffer, Inst *instance, RuntimeInterface::ClassPtr klass,
                                    Inst *ctorCall)
{
    auto runtime = graph->GetRuntime();
    auto field = runtime->GetFieldStringBuilderBuffer(klass);
    auto storeObject = graph->CreateInstStoreObject(DataType::REFERENCE, ctorCall->GetPc(), instance, buffer,
                                                    TypeIdMixin {runtime->GetFieldId(field), graph->GetMethod()}, field,
                                                    runtime->IsFieldVolatile(field), true);
    InsertBeforeWithSaveState(storeObject, ctorCall->GetSaveState());

    return storeObject;
}

void StoreStringBuilderIndexField(Graph *graph, Inst *index, Inst *instance, RuntimeInterface::ClassPtr klass,
                                  Inst *ctorCall)
{
    auto runtime = graph->GetRuntime();
    auto field = runtime->GetFieldStringBuilderIndex(klass);
    auto storeObject = graph->CreateInstStoreObject(DataType::INT32, ctorCall->GetPc(), instance, index,
                                                    TypeIdMixin {runtime->GetFieldId(field), graph->GetMethod()}, field,
                                                    runtime->IsFieldVolatile(field), false);
    InsertBeforeWithSaveState(storeObject, ctorCall->GetSaveState());
}

void StoreStringBuilderLengthField(Graph *graph, Inst *length, Inst *instance, RuntimeInterface::ClassPtr klass,
                                   Inst *ctorCall)
{
    auto runtime = graph->GetRuntime();
    auto field = runtime->GetFieldStringBuilderLength(klass);
    auto storeObject = graph->CreateInstStoreObject(DataType::INT32, ctorCall->GetPc(), instance, length,
                                                    TypeIdMixin {runtime->GetFieldId(field), graph->GetMethod()}, field,
                                                    runtime->IsFieldVolatile(field), false);
    InsertBeforeWithSaveState(storeObject, ctorCall->GetSaveState());
}

void StoreStringBuilderIsCompressedField(Graph *graph, Inst *isCompressed, Inst *instance,
                                         RuntimeInterface::ClassPtr klass, Inst *ctorCall)
{
    auto runtime = graph->GetRuntime();
    auto field = runtime->GetFieldStringBuilderCompress(klass);
    auto storeObject = graph->CreateInstStoreObject(DataType::BOOL, ctorCall->GetPc(), instance, isCompressed,
                                                    TypeIdMixin {runtime->GetFieldId(field), graph->GetMethod()}, field,
                                                    runtime->IsFieldVolatile(field), false);
    InsertBeforeWithSaveState(storeObject, ctorCall->GetSaveState());
}

void ReserveStringBuilderBuffer::ReplaceInitialBufferSizeConstantNotInlined(Inst *instance, uint64_t appendCallsCount)
{
    Inst *ctorCall = nullptr;
    Inst *ctorArg = nullptr;
    StringBuilderConstructorSignature ctorSignature = GetStringBuilderConstructorSignature(instance, ctorCall, ctorArg);

    // Create NewArray instruction for Object[] type and new array size
    auto newObjectsArray = CreateInstructionNewObjectsArray(GetGraph(), ctorCall, appendCallsCount);
    if (newObjectsArray == nullptr) {
        return;
    }

    // Create StoreArray instruction to store constructor argument
    Inst *argLength = nullptr;
    Inst *argIsCompressed = nullptr;
    Inst *argString = ctorArg;

    switch (ctorSignature) {
        case StringBuilderConstructorSignature::UNKNOWN:
            argLength = GetGraph()->FindOrCreateConstant(0);
            argIsCompressed = GetGraph()->FindOrCreateConstant(1);
            break;

        case StringBuilderConstructorSignature::CHAR_ARRAY:
            ASSERT(ctorArg != nullptr);
            // Create string object out of char[] array
            argString = GetGraph()->CreateInstInitString(DataType::REFERENCE, ctorCall->GetPc(), ctorArg,
                                                         CopySaveState(GetGraph(), ctorCall->GetSaveState()),
                                                         StringCtorType::CHAR_ARRAY);
            InsertBeforeWithSaveState(argString, ctorCall->GetSaveState());

            // Store string object in Objects[] array
            StoreStringBuilderConstructorArgument(GetGraph(), argString, newObjectsArray, ctorCall);

            argLength = CreateStringBuilderConstructorArgumentLength(GetGraph(), argString, ctorCall);
            argIsCompressed = CreateStringBuilderConstructorArgumentIsCompressed(GetGraph(), argString, ctorCall);
            ssb_.SearchAndCreateMissingObjInSaveState(GetGraph(), argString, argIsCompressed);
            break;

        case StringBuilderConstructorSignature::STRING:
            // Store constructor argument in Objects[] array
            StoreStringBuilderConstructorArgument(GetGraph(), argString, newObjectsArray, ctorCall);

            argLength = CreateStringBuilderConstructorArgumentLength(GetGraph(), argString, ctorCall);
            argIsCompressed = CreateStringBuilderConstructorArgumentIsCompressed(GetGraph(), argString, ctorCall);
            ssb_.SearchAndCreateMissingObjInSaveState(GetGraph(), argString, argIsCompressed);
            break;

        default:
            UNREACHABLE();
    }

    auto stringBuilderClass = GetObjectClass(instance);
    ASSERT(stringBuilderClass != nullptr);

    // Create StoreObject instruction to store Object[] array
    auto storeObjectBuffer =
        StoreStringBuilderBufferField(GetGraph(), newObjectsArray, instance, stringBuilderClass, ctorCall);
    ssb_.SearchAndCreateMissingObjInSaveState(GetGraph(), newObjectsArray, storeObjectBuffer);

    // Create StoreObject instruction to store initial buffer index
    StoreStringBuilderIndexField(
        GetGraph(),
        GetGraph()->FindOrCreateConstant(ctorSignature == StringBuilderConstructorSignature::UNKNOWN ? 0 : 1), instance,
        stringBuilderClass, ctorCall);

    // Create StoreObject instruction to store initial string length
    StoreStringBuilderLengthField(GetGraph(), argLength, instance, stringBuilderClass, ctorCall);

    // Create StoreObject instruction to store initial compression flag
    StoreStringBuilderIsCompressedField(GetGraph(), argIsCompressed, instance, stringBuilderClass, ctorCall);

    ctorCall->ClearFlag(inst_flags::NO_DCE);
    isApplied_ = true;
}

uint64_t CountStringBuilderConstructorArgumentsInlined(Inst *instance)
{
    for (auto &user : instance->GetUsers()) {
        auto storeObject = SkipSingleUserCheckInstruction(user.GetInst());
        if (storeObject->GetOpcode() != Opcode::StoreObject) {
            continue;
        }

        if (!IsFieldStringBuilderIndex(storeObject->CastToStoreObject())) {
            continue;
        }

        ASSERT(storeObject->GetInputsCount() > 1);
        auto initialIndex = storeObject->GetDataFlowInput(1);
        if (!initialIndex->IsConst()) {
            continue;
        }

        return initialIndex->CastToConstant()->GetInt64Value();
    }
    return INVALID_COUNT;
}

uint64_t CountStringBuilderConstructorArgumentsNotInlined(Inst *instance)
{
    for (auto &user : instance->GetUsers()) {
        auto userInst = user.GetInst();
        if (IsMethodStringBuilderDefaultConstructor(userInst)) {
            return 0;
        }
        if (IsMethodStringBuilderConstructorWithStringArg(userInst) ||
            IsMethodStringBuilderConstructorWithCharArrayArg(userInst)) {
            return 1;
        }
    }
    return INVALID_COUNT;
}

ArenaVector<BasicBlock *> GetLoopPostExits(Loop *loop)
{
    // Find blocks immediately following after the loop

    ArenaVector<BasicBlock *> postExits {loop->GetHeader()->GetGraph()->GetLocalAllocator()->Adapter()};
    for (auto block : loop->GetBlocks()) {
        for (auto succ : block->GetSuccsBlocks()) {
            if (succ->GetLoop() == loop->GetOuterLoop()) {
                postExits.push_back(succ);
            }
        }
    }

    return postExits;
}

uint64_t ReserveStringBuilderBuffer::FindLongestPathLength(Inst *instance, Loop *loop, Marker visited)
{
    // The problem of calculating a number of append instructions calls of a given string builder instance inside a
    // given loop is equivalent to the longest path problem between loop header and exit for loop CFG weighted by
    // the number of append instructions within each block.
    // Use special 'INVALID_COUNT' value for uncountable loops

    uint64_t postExitAppendCallsCountMax = 0;
    for (auto postExit : GetLoopPostExits(loop)) {
        postExitAppendCallsCountMax =
            std::max(postExitAppendCallsCountMax, FindLongestPathLength(instance, postExit, visited));
    }
    if (postExitAppendCallsCountMax == INVALID_COUNT) {
        return INVALID_COUNT;
    }

    // Early exit for loops containing no calls to append instructions
    if (!HasAppendInstructions(instance, loop)) {
        return postExitAppendCallsCountMax;
    }

    uint64_t loopIterationsCount = GetLoopIterationsCount(loop);
    if (loopIterationsCount == INVALID_COUNT) {
        return INVALID_COUNT;
    }

    // Set stop condition
    auto stopAtOuterLoopBlock = [loop](BasicBlock *block) { return block->GetLoop() == loop->GetOuterLoop(); };
    // Find the longest path within current loop: i.e longest path from header to exit blocks
    uint64_t insideLoopAppendCallsCount =
        FindLongestPathLength(instance, loop->GetHeader(), visited, stopAtOuterLoopBlock);
    if (insideLoopAppendCallsCount == INVALID_COUNT) {
        return INVALID_COUNT;
    }

    // Weight of loop: the longest path from header to exit times the loop count value plus the longest path from post
    // exit blocks
    auto loopAppendCallsCount =
        BoundsRange::MulWithOverflowCheck(loopIterationsCount, insideLoopAppendCallsCount).value_or(INVALID_COUNT);
    return BoundsRange::AddWithOverflowCheck(loopAppendCallsCount, postExitAppendCallsCountMax).value_or(INVALID_COUNT);
}

uint64_t ReserveStringBuilderBuffer::FindLongestPathLength(Inst *instance, BasicBlock *block, Marker visited,
                                                           const BlockPredicate &stopAtBlock)
{
    // The problem of calculating a number of append instructions calls of a given string builder instance is equivalent
    // to the longest path problem between instance block and latest user block for CFG weighted by the number of append
    // instructions within each block.

    block->SetMarker(visited);
    uint64_t appendCallsCount = 0;

    // Recursively find longest path from block successors
    for (auto succ : block->GetSuccsBlocks()) {
        if (stopAtBlock(succ)) {
            continue;
        }
        if (succ->IsMarked(visited)) {
            // Found already visited successor: take its weight from map
            appendCallsCount = std::max(appendCallsCount, blockWeightsMap_[succ]);
        } else if (succ->GetLoop() == block->GetLoop()) {
            // Same loop case
            appendCallsCount = std::max(appendCallsCount, FindLongestPathLength(instance, succ, visited, stopAtBlock));
        } else if (succ->GetLoop()->GetOuterLoop() == block->GetLoop()) {
            // Edge from block to succ cr1osses loop boundary: e.g block is loop preheader, succ is loop header
            ASSERT(succ == succ->GetLoop()->GetHeader());
            appendCallsCount =
                succ->GetLoop()->IsIrreducible()
                    ? INVALID_COUNT
                    : std::max(appendCallsCount, FindLongestPathLength(instance, succ->GetLoop(), visited));
        } else if (succ->GetLoop() == block->GetLoop()->GetOuterLoop()) {
            // Edge from block to succ crosses loop boundary: e.g block is loop exit, succ is loop post exit
            // Do nothing, since we already counted loop at preheader/header boundary
        } else {
            // Invalid/unexpected/unsupported CFG
            return INVALID_COUNT;
        }
    }

    // Propagate 'INVALID_COUNT' value to the caller
    if (appendCallsCount == INVALID_COUNT) {
        return INVALID_COUNT;
    }

    // Count current block weight, add longest path from successors, and store into map
    appendCallsCount =
        BoundsRange::AddWithOverflowCheck(appendCallsCount, compiler::CountStringBuilderAppendCalls(instance, block))
            .value_or(INVALID_COUNT);
    blockWeightsMap_[block] = appendCallsCount;
    return appendCallsCount;
}

uint64_t ReserveStringBuilderBuffer::CountStringBuilderAppendCalls(Inst *instance)
{
    // The problem of calculating a number of append instruction calls of a given string builder instance is
    // equivalent to the longest path problem between instance and its latest user over a CFG weighted by the number
    // of append instructions within each block.

    blockWeightsMap_.clear();
    MarkerHolder visited {GetGraph()};

    // Traverse graph starting from instance block and find the longest path
    auto appendCallsCount = FindLongestPathLength(instance, instance->GetBasicBlock(), visited.GetMarker());

    // Check if constructor is inlined or not
    bool ctorIsInlined = !HasUser(instance, [](auto &user) {
        auto userInst = user.GetInst();
        return IsMethodStringBuilderDefaultConstructor(userInst) ||
               IsMethodStringBuilderConstructorWithStringArg(userInst) ||
               IsMethodStringBuilderConstructorWithCharArrayArg(userInst);
    });
    // Increase buffer size if constructor has args
    if (ctorIsInlined) {
        appendCallsCount =
            BoundsRange::AddWithOverflowCheck(appendCallsCount, CountStringBuilderConstructorArgumentsInlined(instance))
                .value_or(INVALID_COUNT);
    } else {
        appendCallsCount = BoundsRange::AddWithOverflowCheck(appendCallsCount,
                                                             CountStringBuilderConstructorArgumentsNotInlined(instance))
                               .value_or(INVALID_COUNT);
    }

    return appendCallsCount;
}

uint64_t ReserveStringBuilderBuffer::GetBufferSizeMin(Inst *instance) const
{
    // Returns minimal StringBuilder internal buffer size:
    //      - if instance has Return/Store* users or passed as an argument to a function, return initial buffer size
    //      - return zero (any buffer size allowed), otherwise

    for (auto &user : instance->GetUsers()) {
        auto userInst = SkipSingleUserCheckInstruction(user.GetInst());
        if (userInst->IsReturn() || userInst->GetOpcode() == Opcode::StoreStatic ||
            userInst->GetOpcode() == Opcode::StoreArray || userInst->GetOpcode() == Opcode::StoreArrayPair) {
            return SB_INITIAL_BUFFER_SIZE;
        }

        if ((userInst->GetOpcode() == Opcode::StoreObject || userInst->GetOpcode() == Opcode::StoreObjectPair) &&
            userInst->GetDataFlowInput(0) != instance) {
            return SB_INITIAL_BUFFER_SIZE;
        }

        if (userInst->GetOpcode() == Opcode::CallStatic || userInst->GetOpcode() == Opcode::CallVirtual) {
            if (!IsMethodStringBuilderDefaultConstructor(userInst) &&
                !IsMethodStringBuilderConstructorWithCharArrayArg(userInst) &&
                !IsMethodStringBuilderConstructorWithStringArg(userInst) && !IsStringBuilderAppend<true>(userInst) &&
                !IsStringBuilderToString(userInst)) {
                return SB_INITIAL_BUFFER_SIZE;
            }
        }
    }

    return 0;
}

uint64_t ReserveStringBuilderBuffer::GetBufferSizeMax() const
{
    // Returns maximal StringBuilder internal buffer size
    // NOTE(mivanov): The algorithm computes upper bound of append calls count.
    //                In case we can prove that this number is constant at runtime (e.g, does not depend on CFG path
    //                taken be a program), we can allow any buffer size by returning UINT64_MAX.

    return tlabArraySizeMax_;
}

bool ReserveStringBuilderBuffer::RunImpl()
{
    isApplied_ = false;

    if (!GetGraph()->IsAnalysisValid<BoundsAnalysis>()) {
        GetGraph()->RunPass<BoundsAnalysis>();
    }

    if (!GetGraph()->IsAnalysisValid<DominatorsTree>()) {
        GetGraph()->RunPass<DominatorsTree>();
    }

    for (auto block : GetGraph()->GetBlocksRPO()) {
        for (auto instance : block->Insts()) {
            if (!IsStringBuilderInstance(instance)) {
                continue;
            }

            // Skip instance if it used in resolved context or in phi instruction
            if (HasUser(instance, [](auto &user) {
                    auto opcode = user.GetInst()->GetOpcode();
                    return opcode == Opcode::CallResolvedStatic || opcode == Opcode::CallResolvedVirtual ||
                           opcode == Opcode::Phi || opcode == Opcode::CatchPhi;
                })) {
                continue;
            }

            auto appendCallsCount = CountStringBuilderAppendCalls(instance);
            // Check if number of calls are successfully counted and fits allowed range
            if (appendCallsCount == INVALID_COUNT || appendCallsCount <= GetBufferSizeMin(instance) ||
                appendCallsCount > GetBufferSizeMax()) {
                continue;
            }

            // Check if constructor is inlined or not
            bool ctorIsInlined = !HasUser(instance, [](auto &user) {
                auto userInst = user.GetInst();
                return IsMethodStringBuilderDefaultConstructor(userInst) ||
                       IsMethodStringBuilderConstructorWithStringArg(userInst) ||
                       IsMethodStringBuilderConstructorWithCharArrayArg(userInst);
            });
            if (ctorIsInlined) {
                // Patch inlined constructor
                ReplaceInitialBufferSizeConstantInlined(instance, appendCallsCount);
            } else {
                // Inline constructor manually and patch
                ReplaceInitialBufferSizeConstantNotInlined(instance, appendCallsCount);
            }
        }
    }

    return isApplied_;
}

}  // namespace ark::compiler
