/**
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this graph except in compliance with the License.
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

#include <cassert>
#include "libabckit/c/abckit.h"
#include "libabckit/c/ir_core.h"

#include "libabckit/src/metadata_inspect_impl.h"
#include "libabckit/src/ir_impl.h"
#include "libabckit/src/helpers_common.h"
#include "libabckit/src/adapter_static/ir_static.h"

#include "libabckit/src/macros.h"
#include "libabckit/src/logger.h"
#include "scoped_timer.h"

#include <cstdint>
#include <iostream>

namespace libabckit {

// ========================================
// Api for Graph manipulation
// ========================================

extern "C" AbckitIsaType GgetIsa(AbckitGraph *graph)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(graph, ABCKIT_ISA_TYPE_UNSUPPORTED);
    if (IsDynamic(graph->function->owningModule->target)) {
        return AbckitIsaType::ABCKIT_ISA_TYPE_DYNAMIC;
    }
    return AbckitIsaType::ABCKIT_ISA_TYPE_STATIC;
}

extern "C" AbckitFile *GgetFile(AbckitGraph *graph)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);

    return graph->file;
}

extern "C" AbckitBasicBlock *GgetStartBasicBlock(AbckitGraph *graph)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    return GgetStartBasicBlockStatic(graph);
}

extern "C" AbckitBasicBlock *GgetEndBasicBlock([[maybe_unused]] AbckitGraph *graph)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    return GgetEndBasicBlockStatic(graph);
}

extern "C" uint32_t GgetNumberOfBasicBlocks(AbckitGraph *graph)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    return GgetNumberOfBasicBlocksStatic(graph);
}

extern "C" bool GvisitBlocksRPO([[maybe_unused]] AbckitGraph *graph, [[maybe_unused]] void *data,
                                [[maybe_unused]] bool (*cb)(AbckitBasicBlock *bb, void *data))
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    return GvisitBlocksRPOStatic(graph, data, cb);
}

extern "C" AbckitBasicBlock *GgetBasicBlock(AbckitGraph *graph, uint32_t id)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    return GgetBasicBlockStatic(graph, id);
}

extern "C" uint32_t GgetNumberOfParameters(AbckitGraph *graph)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, 0);

    return GgetNumberOfParametersStatic(graph);
}

extern "C" AbckitInst *GgetParameter(AbckitGraph *graph, uint32_t index)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    return GgetParameterStatic(graph, index);
}

extern "C" void GinsertTryCatch(AbckitBasicBlock *tryFirstBB, AbckitBasicBlock *tryLastBB,
                                AbckitBasicBlock *catchBeginBB, AbckitBasicBlock *catchEndBB)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT_VOID(tryFirstBB);
    LIBABCKIT_BAD_ARGUMENT_VOID(tryLastBB);
    LIBABCKIT_BAD_ARGUMENT_VOID(catchBeginBB);
    LIBABCKIT_BAD_ARGUMENT_VOID(catchEndBB);

    GinsertTryCatchStatic(tryFirstBB, tryLastBB, catchBeginBB, catchEndBB);
}

extern "C" void Gdump(AbckitGraph *graph, int32_t fd)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT_VOID(graph)

    GdumpStatic(graph, fd);
}

extern "C" void GrunPassRemoveUnreachableBlocks(AbckitGraph *graph)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT_VOID(graph)

    GrunPassRemoveUnreachableBlocksStatic(graph);
}

// ========================================
// Api for basic block manipulation
// ========================================

extern "C" AbckitBasicBlock *BBcreateEmpty(AbckitGraph *graph)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    return BBcreateEmptyStatic(graph);
}

extern "C" uint32_t BBgetId(AbckitBasicBlock *basicBlock)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    return BBgetIdStatic(basicBlock);
}

extern "C" AbckitGraph *BBgetGraph(AbckitBasicBlock *basicBlock)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    return BBgetGraphStatic(basicBlock);
}

extern "C" uint64_t BBgetPredBlockCount(AbckitBasicBlock *basicBlock)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    return BBgetPredBlockCountStatic(basicBlock);
}

extern "C" AbckitBasicBlock *BBgetPredBlock(AbckitBasicBlock *basicBlock, uint32_t index)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    return BBgetPredBlockStatic(basicBlock, index);
}

extern "C" bool BBvisitPredBlocks(AbckitBasicBlock *basicBlock, void *data,
                                  bool (*cb)(AbckitBasicBlock *predBasicBlock, void *data))
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    return BBvisitPredBlocksStatic(basicBlock, data, cb);
}

extern "C" uint64_t BBgetSuccBlockCount(AbckitBasicBlock *basicBlock)
{
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_TIME_EXEC;

    return BBgetSuccBlockCountStatic(basicBlock);
}

extern "C" AbckitBasicBlock *BBgetSuccBlock(AbckitBasicBlock *basicBlock, uint32_t index)
{
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_TIME_EXEC;

    return BBgetSuccBlockStatic(basicBlock, index);
}

extern "C" void BBinsertSuccBlock(AbckitBasicBlock *basicBlock, AbckitBasicBlock *succBlock, uint32_t index)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    BBinsertSuccBlockStatic(basicBlock, succBlock, index);
}

extern "C" void BBappendSuccBlock(AbckitBasicBlock *basicBlock, AbckitBasicBlock *succBlock)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    BBappendSuccBlockStatic(basicBlock, succBlock);
}

extern "C" void BBdisconnectSuccBlock(AbckitBasicBlock *bb, uint32_t index)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    BBdisconnectSuccBlockStatic(bb, index);
}

extern "C" bool BBvisitSuccBlocks(AbckitBasicBlock *basicBlock, void *data,
                                  bool (*cb)(AbckitBasicBlock *succBasicBlock, void *data))
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    return BBvisitSuccBlocksStatic(basicBlock, data, cb);
}

extern "C" AbckitBasicBlock *BBgetTrueBranch(AbckitBasicBlock *basicBlock)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    return BBgetTrueBranchStatic(basicBlock);
}

extern "C" AbckitBasicBlock *BBgetFalseBranch(AbckitBasicBlock *curBasicBlock)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    return BBgetFalseBranchStatic(curBasicBlock);
}

extern "C" AbckitBasicBlock *BBsplitBlockAfterInstruction(AbckitBasicBlock *basicBlock, AbckitInst *inst, bool makeEdge)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    return BBsplitBlockAfterInstructionStatic(basicBlock, inst, makeEdge);
}

extern "C" void BBaddInstFront(AbckitBasicBlock *basicBlock, AbckitInst *inst)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    BBaddInstFrontStatic(basicBlock, inst);
}

extern "C" void BBaddInstBack(AbckitBasicBlock *basicBlock, AbckitInst *inst)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    BBaddInstBackStatic(basicBlock, inst);
}

extern "C" void BBremoveAllInsts(AbckitBasicBlock *basicBlock)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT_VOID(basicBlock)

    return BBremoveAllInstsStatic(basicBlock);
}

extern "C" AbckitInst *BBgetFirstInst(AbckitBasicBlock *basicBlock)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    return BBgetFirstInstStatic(basicBlock);
}

extern "C" AbckitInst *BBgetLastInst(AbckitBasicBlock *basicBlock)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    return BBgetLastInstStatic(basicBlock);
}

extern "C" uint32_t BBgetNumberOfInstructions(AbckitBasicBlock *basicBlock)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    return BBgetNumberOfInstructionsStatic(basicBlock);
}

extern "C" AbckitBasicBlock *BBgetImmediateDominator(AbckitBasicBlock *basicBlock)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    return BBgetImmediateDominatorStatic(basicBlock);
}

extern "C" bool BBcheckDominance(AbckitBasicBlock *basicBlock, AbckitBasicBlock *dominator)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    return BBcheckDominanceStatic(basicBlock, dominator);
}

extern "C" bool BBvisitDominatedBlocks(AbckitBasicBlock *basicBlock, void *data,
                                       bool (*cb)(AbckitBasicBlock *dominatedBasicBlock, void *data))
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    return BBvisitDominatedBlocksStatic(basicBlock, data, cb);
}

extern "C" bool BBisStart(AbckitBasicBlock *basicBlock)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    return BBisStartStatic(basicBlock);
}

extern "C" bool BBisEnd(AbckitBasicBlock *basicBlock)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    return BBisEndStatic(basicBlock);
}

extern "C" bool BBisLoopHead(AbckitBasicBlock *basicBlock)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    return BBisLoopHeadStatic(basicBlock);
}

extern "C" bool BBisLoopPrehead(AbckitBasicBlock *basicBlock)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    return BBisLoopPreheadStatic(basicBlock);
}

extern "C" bool BBisTryBegin(AbckitBasicBlock *basicBlock)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    return BBisTryBeginStatic(basicBlock);
}

extern "C" bool BBisTry(AbckitBasicBlock *basicBlock)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    return BBisTryStatic(basicBlock);
}

extern "C" bool BBisTryEnd(AbckitBasicBlock *basicBlock)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    return BBisTryEndStatic(basicBlock);
}

extern "C" bool BBisCatchBegin(AbckitBasicBlock *basicBlock)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    return BBisCatchBeginStatic(basicBlock);
}

extern "C" bool BBisCatch(AbckitBasicBlock *basicBlock)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    return BBisCatchStatic(basicBlock);
}

extern "C" void BBdump(AbckitBasicBlock *basicBlock, int32_t fd)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    BBdumpStatic(basicBlock, fd);
}

extern "C" AbckitInst *BBcreatePhi(AbckitBasicBlock *bb, size_t argCount, ...)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(bb, nullptr);

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    std::va_list args;
    va_start(args, argCount);

    auto *inst = BBcreatePhiStatic(bb, argCount, args);
    va_end(args);
    return inst;
}

extern "C" AbckitInst *BBcreateCatchPhi(AbckitBasicBlock *catchBegin, size_t argCount, ...)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(catchBegin, nullptr);

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    std::va_list args;
    va_start(args, argCount);

    auto inst = BBcreateCatchPhiStatic(catchBegin, argCount, args);
    va_end(args);
    return inst;
}

// ========================================
// Api for instruction manipulation
// ========================================

extern "C" AbckitInst *GfindOrCreateConstantI64(AbckitGraph *graph, int64_t value)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);

    return GfindOrCreateConstantI64Static(graph, value);
}

extern "C" AbckitInst *GfindOrCreateConstantI32(AbckitGraph *graph, int32_t value)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);

    return GfindOrCreateConstantI32Static(graph, value);
}

extern "C" AbckitInst *GfindOrCreateConstantU64(AbckitGraph *graph, uint64_t value)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);

    return GfindOrCreateConstantU64Static(graph, value);
}

extern "C" AbckitInst *GfindOrCreateConstantF64(AbckitGraph *graph, double value)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);

    return GfindOrCreateConstantF64Static(graph, value);
}

extern "C" void Iremove(AbckitInst *inst)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT_VOID(inst);

    return IremoveStatic(inst);
}

extern "C" uint32_t IgetId(AbckitInst *inst)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(inst, 0);

    return IgetIdStatic(inst);
}

extern "C" AbckitInst *IgetNext(AbckitInst *inst)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(inst, nullptr);

    return IgetNextStatic(inst);
}

extern "C" AbckitInst *IgetPrev(AbckitInst *instprev)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(instprev, nullptr);

    return IgetPrevStatic(instprev);
}

extern "C" void IinsertAfter(AbckitInst *inst, AbckitInst *refInst)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    return IinsertAfterStatic(inst, refInst);
}

extern "C" void IinsertBefore(AbckitInst *inst, AbckitInst *refInst)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    return IinsertBeforeStatic(inst, refInst);
}

extern "C" AbckitType *IgetType(AbckitInst *inst)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(inst, nullptr);
    return IgetTypeStatic(inst);
}

extern "C" AbckitBasicBlock *IgetBasicBlock(AbckitInst *inst)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(inst, nullptr);
    return IgetBasicBlockStatic(inst);
}

extern "C" AbckitGraph *IgetGraph(AbckitInst *inst)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(inst, nullptr);
    return IgetGraphStatic(inst);
}

extern "C" bool IcheckIsCall(AbckitInst *inst)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(inst, false);
    return IcheckIsCallStatic(inst);
}

extern "C" bool IcheckDominance(AbckitInst *inst, AbckitInst *dominator)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(inst, false);
    LIBABCKIT_BAD_ARGUMENT(dominator, false);

    return IcheckDominanceStatic(inst, dominator);
}

extern "C" uint32_t IgetUserCount(AbckitInst *inst)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(inst, 0);

    return IgetUserCountStatic(inst);
}

extern "C" bool IvisitUsers(AbckitInst *inst, void *data, bool (*cb)(AbckitInst *user, void *data))
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(inst, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)

    return IvisitUsersStatic(inst, data, cb);
}

extern "C" uint32_t IgetInputCount(AbckitInst *inst)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(inst, 0);

    return IgetInputCountStatic(inst);
}

extern "C" AbckitInst *IgetInput(AbckitInst *inst, uint32_t index)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(inst, nullptr);

    return IgetInputStatic(inst, index);
}

extern "C" bool IvisitInputs(AbckitInst *inst, void *data, bool (*cb)(AbckitInst *input, size_t inputIdx, void *data))
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(inst, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)

    return IvisitInputsStatic(inst, data, cb);
}

extern "C" void IsetInput(AbckitInst *inst, AbckitInst *input, uint32_t index)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT_VOID(inst);
    LIBABCKIT_BAD_ARGUMENT_VOID(input);

    LIBABCKIT_WRONG_CTX_VOID(inst->graph, input->graph);
    IsetInputStatic(inst, input, index);
}

extern "C" void IsetInputs(AbckitInst *inst, size_t argCount, ...)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT_VOID(inst);

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    std::va_list args;
    va_start(args, argCount);
    IsetInputsStatic(inst, argCount, args);
    va_end(args);
}

extern "C" void IappendInput(AbckitInst *inst, AbckitInst *input)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT_VOID(inst);
    LIBABCKIT_BAD_ARGUMENT_VOID(input);

    LIBABCKIT_WRONG_CTX_VOID(inst->graph, input->graph);
    IappendInputStatic(inst, input);
}

extern "C" void Idump(AbckitInst *inst, int32_t fd)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT_VOID(inst);

    IdumpStatic(inst, fd);
}

extern "C" AbckitCoreFunction *IgetFunction(AbckitInst *inst)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(inst, nullptr);

    return IgetFunctionStatic(inst);
}

extern "C" void IsetFunction(AbckitInst *inst, AbckitCoreFunction *function)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT_VOID(inst);
    LIBABCKIT_BAD_ARGUMENT_VOID(function);

    LIBABCKIT_INTERNAL_ERROR_VOID(function->owningModule);
    LIBABCKIT_INTERNAL_ERROR_VOID(inst->graph);
    LIBABCKIT_WRONG_CTX_VOID(inst->graph->file, function->owningModule->file);
    return IsetFunctionStatic(inst, function);
}

extern "C" uint64_t IgetImmediate(AbckitInst *inst, size_t idx)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(inst, 0);

    return IgetImmediateStatic(inst, idx);
}

extern "C" void IsetImmediate(AbckitInst *inst, size_t idx, uint64_t imm)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT_VOID(inst);

    IsetImmediateStatic(inst, idx, imm);
}

extern "C" AbckitBitImmSize IgetImmediateSize(AbckitInst *inst, size_t idx)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(inst, AbckitBitImmSize::BITSIZE_0);

    return IgetImmediateSizeStatic(inst, idx);
}

extern "C" uint64_t IgetImmediateCount(AbckitInst *inst)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(inst, 0);

    return IgetImmediateCountStatic(inst);
}

extern "C" AbckitLiteralArray *IgetLiteralArray(AbckitInst *inst)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(inst, nullptr);
    return IgetLiteralArrayStatic(inst);
}

extern "C" void IsetLiteralArray(AbckitInst *inst, AbckitLiteralArray *la)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT_VOID(inst);
    LIBABCKIT_BAD_ARGUMENT_VOID(la);

    return IsetLiteralArrayStatic(inst, la);
}

extern "C" AbckitString *IgetString(AbckitInst *inst)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(inst, nullptr);

    return IgetStringStatic(inst);
}

extern "C" void IsetString(AbckitInst *inst, AbckitString *str)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT_VOID(inst);
    LIBABCKIT_BAD_ARGUMENT_VOID(str);

    return IsetStringStatic(inst, str);
}

extern "C" int32_t IgetConstantValueI32(AbckitInst *inst)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(inst, 0);

    return IgetConstantValueI32Static(inst);
}

extern "C" int64_t IgetConstantValueI64(AbckitInst *inst)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(inst, 0);

    return IgetConstantValueI64Static(inst);
}

extern "C" uint64_t IgetConstantValueU64(AbckitInst *inst)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(inst, 0);

    return IgetConstantValueU64Static(inst);
}

extern "C" double IgetConstantValueF64(AbckitInst *inst)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(inst, 0);

    return IgetConstantValueF64Static(inst);
}

AbckitGraphApi g_graphApiImpl = {
    GgetIsa,
    GgetFile,

    // ========================================
    // Api for Graph manipulation
    // ========================================

    GgetStartBasicBlock,
    GgetEndBasicBlock,
    GgetNumberOfBasicBlocks,
    GvisitBlocksRPO,
    GgetBasicBlock,
    GgetParameter,
    GgetNumberOfParameters,
    GinsertTryCatch,
    Gdump,
    GfindOrCreateConstantI32,
    GfindOrCreateConstantI64,
    GfindOrCreateConstantU64,
    GfindOrCreateConstantF64,
    GrunPassRemoveUnreachableBlocks,

    // ========================================
    // Api for basic block manipulation
    // ========================================

    BBcreateEmpty,
    BBgetId,
    BBgetGraph,
    BBgetPredBlockCount,
    BBgetPredBlock,
    BBvisitPredBlocks,
    BBgetSuccBlockCount,
    BBgetSuccBlock,
    BBinsertSuccBlock,
    BBappendSuccBlock,
    BBdisconnectSuccBlock,
    BBvisitSuccBlocks,
    BBgetTrueBranch,
    BBgetFalseBranch,
    BBsplitBlockAfterInstruction,
    BBaddInstFront,
    BBaddInstBack,
    BBremoveAllInsts,
    BBgetFirstInst,
    BBgetLastInst,
    BBgetNumberOfInstructions,
    BBgetImmediateDominator,
    BBcheckDominance,
    BBvisitDominatedBlocks,
    BBisStart,
    BBisEnd,
    BBisLoopHead,
    BBisLoopPrehead,
    BBisTryBegin,
    BBisTry,
    BBisTryEnd,
    BBisCatchBegin,
    BBisCatch,
    BBdump,
    BBcreatePhi,
    BBcreateCatchPhi,

    // ========================================
    // Api for instruction manipulation
    // ========================================

    Iremove,
    IgetId,
    IgetNext,
    IgetPrev,
    IinsertAfter,
    IinsertBefore,
    IgetType,
    IgetBasicBlock,
    IgetGraph,
    IcheckDominance,
    IcheckIsCall,
    IgetUserCount,
    IvisitUsers,
    IgetInputCount,
    IgetInput,
    IvisitInputs,
    IsetInput,
    IsetInputs,
    IappendInput,
    Idump,
    IgetFunction,
    IsetFunction,
    IgetImmediate,
    IsetImmediate,
    IgetImmediateSize,
    IgetImmediateCount,
    IgetLiteralArray,
    IsetLiteralArray,
    IgetString,
    IsetString,
    IgetConstantValueI32,
    IgetConstantValueI64,
    IgetConstantValueU64,
    IgetConstantValueF64,
};

}  // namespace libabckit

#ifdef ABCKIT_ENABLE_MOCK_IMPLEMENTATION
#include "./mock/abckit_mock.h"
#endif

extern "C" AbckitGraphApi const *AbckitGetGraphApiImpl(AbckitApiVersion version)
{
#ifdef ABCKIT_ENABLE_MOCK_IMPLEMENTATION
    return AbckitGetMockGraphApiImpl(version);
#endif
    switch (version) {
        case ABCKIT_VERSION_RELEASE_1_0_0:
            return &libabckit::g_graphApiImpl;
        default:
            libabckit::statuses::SetLastError(ABCKIT_STATUS_UNKNOWN_API_VERSION);
            return nullptr;
    }
}
