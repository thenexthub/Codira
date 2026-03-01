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

#ifndef LIBABCKIT_IR_H
#define LIBABCKIT_IR_H

#ifndef __cplusplus
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#else
#include <cstddef>
#include <cstdint>
#endif

#include "declarations.h"
#include "api_version.h"

#ifdef __cplusplus
extern "C" {
#endif

enum AbckitIsaType {
    /*
     * For invalid input.
     */
    ABCKIT_ISA_TYPE_UNSUPPORTED,
    /*
     * For .abc files that use mainline ISA.
     */
    ABCKIT_ISA_TYPE_DYNAMIC,
    /*
     * Reserved for future versions.
     */
    ABCKIT_ISA_TYPE_STATIC,
};

enum { ABCKIT_TRUE_SUCC_IDX = 0, ABCKIT_FALSE_SUCC_IDX = 1 };

enum AbckitBitImmSize {
    /*
     * For invalid input.
     */
    BITSIZE_0 = 0,
    BITSIZE_4 = 4,
    BITSIZE_8 = 8,
    BITSIZE_16 = 16,
    BITSIZE_32 = 32,
    /*
     * For immediate overflow check.
     */
    BITSIZE_64 = 64
};

/**
 * @brief Struct that holds the pointers to the graph manipulation API.
 */
struct CAPI_EXPORT AbckitGraphApi {
    /**
     * @brief Returns ISA type for given graph.
     * @return ISA of the graph.
     * @param [ in ] graph - Graph to read ISA type from.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     */
    enum AbckitIsaType (*gGetIsa)(AbckitGraph *graph);

    /**
     * @brief Returns binary file from which the given `graph` was created.
     * @return Pointer to the `AbckitFile` from which given `graph` was created.
     * @param [ in ] graph - Graph to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     */
    AbckitFile *(*gGetFile)(AbckitGraph *graph);

    /* ========================================
     * Api for Graph manipulation
     * ======================================== */

    /**
     * @brief Returns start basic block of given `graph`.
     * @return Pointer to start `AbckitBasicBlock`.
     * @param [ in ] graph - Graph to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     */
    AbckitBasicBlock *(*gGetStartBasicBlock)(AbckitGraph *graph);

    /**
     * @brief Returns end basic block of given `graph`.
     * @return Pointer to end `AbckitBasicBlock`.
     * @param [ in ] graph - Graph to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     */
    AbckitBasicBlock *(*gGetEndBasicBlock)(AbckitGraph *graph);

    /**
     * @brief Returns number of basic blocks in given `graph`.
     * @return Number of basic blocks in given `graph`.
     * @param [ in ] graph - Graph to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     */
    uint32_t (*gGetNumberOfBasicBlocks)(AbckitGraph *graph);

    /**
     * @brief Enumerates basic blocks of the `graph` in reverse postorder, invoking callback `cb` for each basic block.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] graph - Graph to be inspected.
     * @param [ in, out ] data - Pointer to the user-defined data that will be passed to the callback `cb` each time
     * it is invoked.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cb` is NULL.
     */
    bool (*gVisitBlocksRpo)(AbckitGraph *graph, void *data, bool (*cb)(AbckitBasicBlock *basicBlock, void *data));

    /**
     * @brief Returns basic blocks with given `id` of the given `graph`.
     * @return Pointer to the `AbckitBasicBlock` with given `id`.
     * @param [ in ] graph - Graph to be inspected.
     * @param [ in ] id - ID of basic block.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if there is no basic block with given `id` in `graph`.
     */
    AbckitBasicBlock *(*gGetBasicBlock)(AbckitGraph *graph, uint32_t id);

    /**
     * @brief Returns parameter instruction under given `index` of the given `graph`.
     * @return Pointer to the `AbckitInst` corresponding to parameter under given `index`.
     * @param [ in ] graph - Graph to be inspected.
     * @param [ in ] index - Index of the parameter.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if there is no parameter under given `index` in `graph`.
     */
    AbckitInst *(*gGetParameter)(AbckitGraph *graph, uint32_t index);

    /**
     * @brief Returns number of instruction parameters under given `graph`.
     * @return uint32_t corresponding to number of instruction parameters.
     * @param [ in ] graph - Graph to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     */
    uint32_t (*gGetNumberOfParameters)(AbckitGraph *graph);

    /**
     * @brief Wraps basic blocks from `tryFirstBB` to `tryLastBB` into try,
     * inserts basic blocks from `catchBeginBB` to `catchEndBB` into graph.
     * Basic blocks from `catchBeginBB` to `catchEndBB` are used for exception handling.
     * @return None.
     * @param [ in ] tryFirstBB - Start basic block to wrap into try.
     * @param [ in ] tryLastBB - End basic block to wrap into try.
     * @param [ in ] catchBeginBB - Start basic block to handle exception.
     * @param [ in ] catchEndBB - End basic block to handle exception.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `tryFirstBB` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `tryLastBB` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `catchBeginBB` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `catchEndBB` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if corresponding `AbckitGraph`s owning input basic blocks differs.
     */
    void (*gInsertTryCatch)(AbckitBasicBlock *tryFirstBB, AbckitBasicBlock *tryLastBB, AbckitBasicBlock *catchBeginBB,
                            AbckitBasicBlock *catchEndBB);

    /**
     * @brief Dumps given `graph` into given file descriptor.
     * @return None.
     * @param [ in ] graph - Graph to be inspected.
     * @param [ in ] fd - File descriptor where dump is written.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Allocates
     */
    void (*gDump)(AbckitGraph *graph, int32_t fd);

    /**
     * @brief Creates I32 constant instruction and inserts it in start basic block of given `graph`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph in which instruction is inserted.
     * @param [ in ] value - value of created constant instruction.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Allocates
     */
    AbckitInst *(*gFindOrCreateConstantI32)(AbckitGraph *graph, int32_t value);

    /**
     * @brief Creates I64 constant instruction and inserts it in start basic block of given `graph`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph in which instruction is inserted.
     * @param [ in ] value - value of created constant instruction.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Allocates
     */
    AbckitInst *(*gFindOrCreateConstantI64)(AbckitGraph *graph, int64_t value);

    /**
     * @brief Creates U64 constant instruction and inserts it in start basic block of given `graph`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph in which instruction is inserted.
     * @param [ in ] value - value of created constant instruction.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Allocates
     */
    AbckitInst *(*gFindOrCreateConstantU64)(AbckitGraph *graph, uint64_t value);

    /**
     * @brief Creates F64 constant instruction and inserts it in start basic block of given `graph`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph in which instruction is inserted.
     * @param [ in ] value - value of created constant instruction.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Allocates
     */
    AbckitInst *(*gFindOrCreateConstantF64)(AbckitGraph *graph, double value);

    /**
     * @brief Removes all basic blocks unreachable from start basic block.
     * @return None.
     * @param [ in ] graph - Graph to be modified.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     */
    void (*gRunPassRemoveUnreachableBlocks)(AbckitGraph *graph);

    /* ========================================
     * Api for basic block manipulation
     * ======================================== */

    /**
     * @brief Creates empty basic block.
     * @return Pointer to the created `AbckitBasicBlock`.
     * @param [ in ] graph - Graph for which basic block is created.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Allocates
     */
    AbckitBasicBlock *(*bbCreateEmpty)(AbckitGraph *graph);

    /**
     * @brief Returns ID of given `basicBlock`.
     * @return ID of given `basicBlock`.
     * @param [ in ] basicBlock - basic block to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `basicBlock` is NULL.
     */
    uint32_t (*bbGetId)(AbckitBasicBlock *basicBlock);

    /**
     * @brief Returns graph owning given `basicBlock`.
     * @return Pointer to `AbckitGraph`.
     * @param [ in ] basicBlock - basic block to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `basicBlock` is NULL.
     */
    AbckitGraph *(*bbGetGraph)(AbckitBasicBlock *basicBlock);

    /**
     * @brief Returns the number of basic blocks preceding the given `basicBlock`.
     * @return Number of predecessor basic blocks.
     * @param [ in ] basicBlock - basic block to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `basicBlock` is NULL.
     */
    uint64_t (*bbGetPredBlockCount)(AbckitBasicBlock *basicBlock);

    /**
     * @brief Returns basic block predcessing to `basicBlock` under given `index`.
     * @return Pointer to the `AbckitBasicBlock`.
     * @param [ in ] basicBlock - basic block to be inspected.
     * @param [ in ] index - Index of predecessor basic block.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `basicBlock` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if there is no predecessor basic block under given `index`.
     */
    AbckitBasicBlock *(*bbGetPredBlock)(AbckitBasicBlock *basicBlock, uint32_t index);

    /**
     * @brief Enumerates basic blocks preceding to the given `basicBlock`, invoking callback `cb` for each basic
     * block.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] basicBlock - Basic block to be inspected.
     * @param [ in, out ] data - Pointer to the user-defined data that will be passed to the callback `cb` each time
     * it is invoked.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `basicBlock` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cb` is NULL.
     */
    bool (*bbVisitPredBlocks)(AbckitBasicBlock *basicBlock, void *data,
                              bool (*cb)(AbckitBasicBlock *predBasicBlock, void *data));

    /**
     * @brief Returns the number of basic blocks successing the given `basicBlock`.
     * @return Number of successor basic blocks.
     * @param [ in ] basicBlock - basic block to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `basicBlock` is NULL.
     */
    uint64_t (*bbGetSuccBlockCount)(AbckitBasicBlock *basicBlock);

    /**
     * @brief Returns basic block successing to `basicBlock` under given `index`.
     * @return Pointer to the `AbckitBasicBlock`.
     * @param [ in ] basicBlock - basic block to be inspected.
     * @param [ in ] index - Index of successor basic block.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `basicBlock` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if there is no successor basic block under given `index`.
     */
    AbckitBasicBlock *(*bbGetSuccBlock)(AbckitBasicBlock *basicBlock, uint32_t index);

    /**
     * @brief Inserts `succBlock` by `index` in `basicBlock` successors list
     * and shifts the rest if there were successors with a larger index.
     * @return None.
     * @param [ in ] basicBlock - Basic block for which successor will be inserted.
     * @param [ in ] succBlock - Basic block to be inserted.
     * @param [ in ] index - Index by which the `succBlock` will be inserted.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `basicBlock` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `succBlock` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `index` is larger than quantity of `basicBlock` successors.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if corresponding `AbckitGraphs`s owning `basicBlock` and `succBlock`
     * differs.
     * @note Allocates
     */
    void (*bbInsertSuccBlock)(AbckitBasicBlock *basicBlock, AbckitBasicBlock *succBlock, uint32_t index);

    /**
     * @brief Appends successor to the end of `basicBlock` successors list.
     * @return None.
     * @param [ in ] basicBlock - Basic block for which successor will be inserted.
     * @param [ in ] succBlock - Basic block to be inserted.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `basicBlock` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `succBlock` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if corresponding `AbckitGraphs`s owning `basicBlock` and `succBlock`
     * differs.
     */
    void (*bbAppendSuccBlock)(AbckitBasicBlock *basicBlock, AbckitBasicBlock *succBlock);

    /**
     * @brief Deletes the successor and shifts the rest if there were successors with a larger index.
     * @return None.
     * @param [ in ] basicBlock - Basic block for which successor will be deleted.
     * @param [ in ] index - Index of successor to be deleted.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `basicBlock` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `index` is larger than `basicBlock` successors quantity.
     */
    void (*bbDisconnectSuccBlock)(AbckitBasicBlock *basicBlock, uint32_t index);

    /**
     * @brief Enumerates basic blocks successing to the given `basicBlock`, invoking callback `cb` for each basic block.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] basicBlock - Basic block to be inspected.
     * @param [ in, out ] data - Pointer to the user-defined data that will be passed to the callback `cb` each time
     * it is invoked.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `basicBlock` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cb` is NULL.
     */
    bool (*bbVisitSuccBlocks)(AbckitBasicBlock *basicBlock, void *data,
                              bool (*cb)(AbckitBasicBlock *succBasicBlock, void *data));

    /**
     * @brief Returns successor of `basicBlock` with index 0.
     * @return Pinter to the `AbckitBasicBlock`.
     * @param [ in ] basicBlock - Basic block to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `basicBlock` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `basicBlock` has no successors.
     */
    AbckitBasicBlock *(*bbGetTrueBranch)(AbckitBasicBlock *basicBlock);

    /**
     * @brief Returns successor of `basicBlock` with index 1.
     * @return Pinter to the `AbckitBasicBlock`.
     * @param [ in ] basicBlock - Basic block to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `basicBlock` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `basicBlock` has less than one successor.
     */
    AbckitBasicBlock *(*bbGetFalseBranch)(AbckitBasicBlock *basicBlock);

    /**
     * @brief Creates new basic block and moves all instructions after `inst` into new basic block.
     * @return Pointer to newly create `AbckitBasicBlock`.
     * @param [ in ] basicBlock - Block for which instruction is splitted.
     * @param [ in ] inst - Instruction after which all instructions will be moved into new basic block.
     * @param [ in ] makeEdge - If `true` connects old and new basic blocks.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     * @note Allocates
     */
    AbckitBasicBlock *(*bbSplitBlockAfterInstruction)(AbckitBasicBlock *basicBlock, AbckitInst *inst, bool makeEdge);

    /**
     * @brief Insert `inst` at the beginning of `basicBlock`.
     * @return None.
     * @param [ in ] basicBlock - Block for which instruction is appended.
     * @param [ in ] inst - Instruction to insert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `basicBlock` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is constant.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if corresponding `AbckitGraphs`s owning `basicBlock` and `inst`
     * differs.
     * @note Allocates
     */
    void (*bbAddInstFront)(AbckitBasicBlock *basicBlock, AbckitInst *inst);

    /**
     * @brief Appends `inst` at the end of `basicBlock`.
     * @return None.
     * @param [ in ] basicBlock - Block for which instruction is appended.
     * @param [ in ] inst - Instruction to insert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `basicBlock` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is constant.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if corresponding `AbckitGraphs`s owning `basicBlock` and `inst`
     * differs.
     * @note Allocates
     */
    void (*bbAddInstBack)(AbckitBasicBlock *basicBlock, AbckitInst *inst);

    /**
     * @brief Removes all instructions from `basicBlock`.
     * @return None.
     * @param [ in ] basicBlock - basic block to clear.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `basicBlock` is NULL.
     */
    void (*bbRemoveAllInsts)(AbckitBasicBlock *basicBlock);

    /**
     * @brief Returns first instruction from `basicBlock`.
     * @return Pointer to the `AbckitInst`.
     * @param [ in ] basicBlock - Basic block to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `basicBlock` is NULL.
     */
    AbckitInst *(*bbGetFirstInst)(AbckitBasicBlock *basicBlock);

    /**
     * @brief Returns last instruction from `basicBlock`.
     * @return Pointer to the `AbckitInst`.
     * @param [ in ] basicBlock - Basic block to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `basicBlock` is NULL.
     */
    AbckitInst *(*bbGetLastInst)(AbckitBasicBlock *basicBlock);

    /**
     * @brief Returns number of instruction in `basicBlock`.
     * @return Number of instruction in `basicBlock`.
     * @param [ in ] basicBlock - Basic block to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `basicBlock` is NULL.
     */
    uint32_t (*bbGetNumberOfInstructions)(AbckitBasicBlock *basicBlock);

    /**
     * @brief Returns immediate dominator of `basicBlock`.
     * @return Pinter to the `AbckitBasicBlock`.
     * @param [ in ] basicBlock - Basic block to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `basicBlock` is NULL.
     */
    AbckitBasicBlock *(*bbGetImmediateDominator)(AbckitBasicBlock *basicBlock);

    /**
     * @brief Checks that `basicBlock` is dominated by `dominator`
     * @return True if `basicBlock` is dominated by `dominator`.
     * @param [ in ] basicBlock - Basic block to be inspected.
     * @param [ in ] dominator - Basic block to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `basicBlock` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `dominator` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if corresponding `AbckitGraphs`s owning `basicBlock` and `dominator`
     * differs.
     */
    bool (*bbCheckDominance)(AbckitBasicBlock *basicBlock, AbckitBasicBlock *dominator);

    /**
     * @brief Enumerates basic blocks dominating to the given `basicBlock`, invoking callback `cb` for each basic block.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] basicBlock - Basic block to be inspected.
     * @param [ in, out ] data - Pointer to the user-defined data that will be passed to the callback `cb` each time
     * it is invoked.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `basicBlock` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cb` is NULL.
     */
    bool (*bbVisitDominatedBlocks)(AbckitBasicBlock *basicBlock, void *data,
                                   bool (*cb)(AbckitBasicBlock *dominatedBasicBlock, void *data));

    /**
     * @brief Tells if `basicBlock` is start basic block.
     * @return `true` if `basicBlock` is start basic block, `false` otherwise.
     * @param [ in ] basicBlock - Basic block to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `basicBlock` is NULL.
     */
    bool (*bbIsStart)(AbckitBasicBlock *basicBlock);

    /**
     * @brief Tells if `basicBlock` is end basic block.
     * @return `true` if `basicBlock` is end basic block, `false` otherwise.
     * @param [ in ] basicBlock - Basic block to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `basicBlock` is NULL.
     */
    bool (*bbIsEnd)(AbckitBasicBlock *basicBlock);

    /**
     * @brief Tells if `basicBlock` is loop head basic block.
     * @return `true` if `basicBlock` is loop head basic block, `false` otherwise.
     * @param [ in ] basicBlock - Basic block to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `basicBlock` is NULL.
     */
    bool (*bbIsLoopHead)(AbckitBasicBlock *basicBlock);

    /**
     * @brief Tells if `basicBlock` is loop prehead basic block.
     * @return `true` if `basicBlock` is loop prehead basic block, `false` otherwise.
     * @param [ in ] basicBlock - Basic block to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `basicBlock` is NULL.
     */
    bool (*bbIsLoopPrehead)(AbckitBasicBlock *basicBlock);

    /**
     * @brief Tells if `basicBlock` is try begin basic block.
     * @return `true` if `basicBlock` is try begin basic block, `false` otherwise.
     * @param [ in ] basicBlock - Basic block to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `basicBlock` is NULL.
     */
    bool (*bbIsTryBegin)(AbckitBasicBlock *basicBlock);

    /**
     * @brief Tells if `basicBlock` is try basic block.
     * @return `true` if `basicBlock` is try basic block, `false` otherwise.
     * @param [ in ] basicBlock - Basic block to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `basicBlock` is NULL.
     */
    bool (*bbIsTry)(AbckitBasicBlock *basicBlock);

    /**
     * @brief Tells if `basicBlock` is try end basic block.
     * @return `true` if `basicBlock` is try end basic block, `false` otherwise.
     * @param [ in ] basicBlock - Basic block to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `basicBlock` is NULL.
     */
    bool (*bbIsTryEnd)(AbckitBasicBlock *basicBlock);

    /**
     * @brief Tells if `basicBlock` is catch begin basic block.
     * @return `true` if `basicBlock` is catch begin basic block, `false` otherwise.
     * @param [ in ] basicBlock - Basic block to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `basicBlock` is NULL.
     */
    bool (*bbIsCatchBegin)(AbckitBasicBlock *basicBlock);

    /**
     * @brief Tells if `basicBlock` is catch basic block.
     * @return `true` if `basicBlock` is catch basic block, `false` otherwise.
     * @param [ in ] basicBlock - Basic block to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `basicBlock` is NULL.
     */
    bool (*bbIsCatch)(AbckitBasicBlock *basicBlock);

    /**
     * @brief Dumps given `basicBlock` into given file descriptor.
     * @return None.
     * @param [ in ] basicBlock -  Basic block to be inspected.
     * @param [ in ] fd - File descriptor where dump is written.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `basicBlock` is NULL.
     * @note Allocates
     */
    void (*bbDump)(AbckitBasicBlock *basicBlock, int32_t fd);

    /**
     * @brief Creates phi instruction and inserts it into `basicBlock`.
     * @return Pointer to created phi instruction.
     * @param [ in ] basicBlock - Basic block where phi instruction will be inserted.
     * @param [ in ] argCount - Number of phi inputs.
     * @param [ in ] ... - Phi inputs, must be pointers to `AbckitInst`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `basicBlock` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `argCount` is zero.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if types of phi inputs are inconsistent.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if one of inputs is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if corresponding `AbckitGraph`s owning `basicBlock` and phi input
     * instruction differs.
     * @note Allocates
     */
    AbckitInst *(*bbCreatePhi)(AbckitBasicBlock *basicBlock, size_t argCount, ...);

    /**
     * @brief Creates CatchPhi instruction and sets it at the beginning of basic block `catchBegin`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] catchBegin - Basic block at the beginning of which the instruction will be inserted.
     * @param [ in ] argCount - Number of instruction's inputs
     * @param [ in ] ... - Instructions that are inputs of the new instruction.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `catchBegin` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph` is not DYNAMIC.
     */
    AbckitInst *(*bbCreateCatchPhi)(AbckitBasicBlock *catchBegin, size_t argCount, ...);

    /**
     * @brief Removes instruction from it's basic block.
     * @return None.
     * @param [ in ] inst - Instruction to be removed.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     */
    void (*iRemove)(AbckitInst *inst);

    /**
     * @brief Returns ID of instruction.
     * @return ID of instruction.
     * @param [ in ] inst - Instruction to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     */
    uint32_t (*iGetId)(AbckitInst *inst);

    /**
     * @brief Returns instruction following `inst` from `inst`'s basic block.
     * @return Pointer to next `AbckitInst`.
     * @param [ in ] inst - Instruction to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     */
    AbckitInst *(*iGetNext)(AbckitInst *inst);

    /**
     * @brief Returns instruction preceding `inst` from `inst`'s basic block.
     * @return Pointer to previous `AbckitInst`.
     * @param [ in ] inst - Instruction to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     */
    AbckitInst *(*iGetPrev)(AbckitInst *inst);

    /**
     * @brief Inserts `newInst` instruction after `ref` instruction into `ref`'s basic block.
     * @return None.
     * @param [ in ] newInst - Instruction to be inserted.
     * @param [ in ] ref - Instruction after which `newInst` will be inserted.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `newInst` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `ref` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `newInst` is constant instruction.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `ref` is constant instruction.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if corresponding `AbckitGraph`s owning `newInst` and `ref` differs.
     * @note Allocates
     */
    void (*iInsertAfter)(AbckitInst *newInst, AbckitInst *ref);

    /**
     * @brief Inserts `newInst` instruction before `ref` instruction into `ref`'s basic block.
     * @return None.
     * @param [ in ] newInst - Instruction to be inserted.
     * @param [ in ] ref - Instruction before which `newInst` will be inserted.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `newInst` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `ref` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `newInst` is constant instruction.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `ref` is constant instruction.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if corresponding `AbckitGraph`s owning `newInst` and `ref` differs.
     * @note Allocates
     */
    void (*iInsertBefore)(AbckitInst *newInst, AbckitInst *ref);

    /**
     * @brief Returns the type of the `inst` result.
     * @return Pointer to the `AbckitType`.
     * @param [ in ] inst - Instruction to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     * @note Allocates
     */
    AbckitType *(*iGetType)(AbckitInst *inst);

    /**
     * @brief Returns basic block that owns `inst`.
     * @return Pointer to the `AbckitBasicBlock`.
     * @param [ in ] inst - Instruction to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     */
    AbckitBasicBlock *(*iGetBasicBlock)(AbckitInst *inst);

    /**
     * @brief Returns graph that owns `inst`.
     * @return Pointer to the `AbckitGraph`.
     * @param [ in ] inst - Instruction to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     */
    AbckitGraph *(*iGetGraph)(AbckitInst *inst);

    /**
     * @brief Checks that `inst` is dominated by `dominator`.
     * @return `true` if `inst` is dominated by `dominator`, `false` otherwise.
     * @param [ in ] inst - Instruction to be inspected.
     * @param [ in ] dominator - Instruction to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `dominator` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if corresponding `AbckitGraph`s owning `inst` and `dominator` differs.
     */
    bool (*iCheckDominance)(AbckitInst *inst, AbckitInst *dominator);

    /**
     * @brief Checks if `inst` is "call" instruction.
     * @return `true` if `inst` is "call" instruction, `false` otherwise.
     * @param [ in ] inst - Instruction to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     */
    bool (*iCheckIsCall)(AbckitInst *inst);

    /**
     * @brief Returns number of `inst` users.
     * @return Number of `inst` users.
     * @param [ in ] inst - Instruction to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     */
    uint32_t (*iGetUserCount)(AbckitInst *inst);

    /**
     * @brief Enumerates `insts` user instructions, invoking callback `cb` for each user instruction.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] inst - Instruction to be inspected.
     * @param [ in, out ] data - Pointer to the user-defined data that will be passed to the callback `cb` each time
     * it is invoked.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cb` is NULL.
     */
    bool (*iVisitUsers)(AbckitInst *inst, void *data, bool (*cb)(AbckitInst *user, void *data));

    /**
     * @brief Returns number of `inst` inputs.
     * @return Number of `inst` inputs.
     * @param [ in ] inst - Instruction to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     */
    uint32_t (*iGetInputCount)(AbckitInst *inst);

    /**
     * @brief Returns `inst` input under given `index`.
     * @return Pointer to the input `AbckitInst`.
     * @param [ in ] inst - Instruction to be inspected.
     * @param [ in ] index - Index of input to be returned.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `index` is larger than number of `inst` inputs.
     */
    AbckitInst *(*iGetInput)(AbckitInst *inst, uint32_t index);

    /**
     * @brief Enumerates `insts` input instructions, invoking callback `cb` for each input instruction.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] inst - Instruction to be inspected.
     * @param [ in, out ] data - Pointer to the user-defined data that will be passed to the callback `cb` each time
     * it is invoked.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cb` is NULL.
     */
    bool (*iVisitInputs)(AbckitInst *inst, void *data, bool (*cb)(AbckitInst *input, size_t inputIdx, void *data));

    /**
     * @brief Sets `inst` input, overwrites existing input.
     * @return None.
     * @param [ in ] inst - Instruction to be modified.
     * @param [ in ] input - Input instruction to be set.
     * @param [ in ] index - Index of input to be set.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if corresponding `AbckitGraph`s owning `inst` and `input` differs.
     */
    void (*iSetInput)(AbckitInst *inst, AbckitInst *input, uint32_t index);

    /**
     * @brief Sets input instructions for `inst` starting from index 0, overwrites existing inputs.
     * @return None.
     * @param [ in ] inst - Instruction to be modified.
     * @param [ in ] argCount - Number of input instructions.
     * @param [ in ] ... - Instructions to be set as input for `inst`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if one of inst inputs are NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if corresponding `AbckitGraph`s owning `inst` and input inst differs.
     */
    void (*iSetInputs)(AbckitInst *inst, size_t argCount, ...);

    /**
     * @brief Appends `input` instruction to `inst` inputs.
     * @return None.
     * @param [ in ] inst - Instruction to be modified.
     * @param [ in ] input - Instruction to be appended as input.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is not applicable for input appending.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if corresponding `AbckitGraph`s owning `inst` and `input` differs.
     * @note Allocates
     */
    void (*iAppendInput)(AbckitInst *inst, AbckitInst *input);

    /**
     * @brief Dumps given `inst` into given file descriptor.
     * @return None.
     * @param [ in ] inst - Instruction to be inspected.
     * @param [ in ] fd - File descriptor where dump is written.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     * @note Allocates
     */
    void (*iDump)(AbckitInst *inst, int32_t fd);

    /**
     * @brief Returns `inst` function operand.
     * @return Pointer to the `AbckitCoreFunction`.
     * @param [ in ] inst - Instruction to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` has no function operand.
     */
    AbckitCoreFunction *(*iGetFunction)(AbckitInst *inst);

    /**
     * @brief Sets `inst` function operand.
     * @return None.
     * @param [ in ] inst - Instruction to be modified.
     * @param [ in ] function - Function to be set as `inst` operand.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `function` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` has no function operand.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if corresponding `AbckitFile`s owning `inst` and `function` differs.
     */
    void (*iSetFunction)(AbckitInst *inst, AbckitCoreFunction *function);

    /**
     * @brief Returns `inst` immediate under given `index`.
     * @return uint64_t .
     * @param [ in ] inst - Instruction to be inspected.
     * @param [ in ] index - Index of immediate.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` has no immediates.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `index` larger than `inst` immediates number.
     */
    uint64_t (*iGetImmediate)(AbckitInst *inst, size_t index);

    /**
     * @brief Sets `inst` immediate under given `index` with value `imm`.
     * @return None.
     * @param [ in ] inst - Instruction to be modified.
     * @param [ in ] index - Index of immediate to be set.
     * @param [ in ] imm - Value of immediate to be set.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` has no immediates.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `index` larger than `inst` immediates number.
     */
    void (*iSetImmediate)(AbckitInst *inst, size_t index, uint64_t imm);

    /**
     * @brief Returns size in bits of `inst` immediate under given `index`.
     * @return Size of `inst` immediate under given `index` in bits.
     * @param [ in ] inst - Instruction to be inspected.
     * @param [ in ] index - Index of immediate to get size.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` has no immediates.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `index` larger than `inst` immediates number.
     */
    enum AbckitBitImmSize (*iGetImmediateSize)(AbckitInst *inst, size_t index);

    /**
     * @brief Returns number of `inst` immediates.
     * @return Number of `inst` immediates.
     * @param [ in ] inst - Instruction to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     */
    uint64_t (*iGetImmediateCount)(AbckitInst *inst);

    /**
     * @brief Returns `inst` literal array operand.
     * @return Pointer to the `AbckitLiteralArray`.
     * @param [ in ] inst - Instruction to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` has no literal array operand.
     */
    AbckitLiteralArray *(*iGetLiteralArray)(AbckitInst *inst);

    /**
     * @brief Sets `inst` literal array operand.
     * @return None.
     * @param [ in ] inst - Instruction to be modified.
     * @param [ in ] la - Literal array to be set as operand.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `la` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` has no literal array operand.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if corresponding `AbckitFile`s owning `inst` and `la` differs.
     */
    void (*iSetLiteralArray)(AbckitInst *inst, AbckitLiteralArray *la);

    /**
     * @brief Returns `inst` string operand.
     * @return Pointer to the `AbckitString`.
     * @param [ in ] inst - Instruction to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` has no string operand.
     */
    AbckitString *(*iGetString)(AbckitInst *inst);

    /**
     * @brief Sets `inst` string operand.
     * @return None.
     * @param [ in ] inst - Instruction to be inspected.
     * @param [ in ] s - String to be set as operand.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `s` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` has no string operand.
     */
    void (*iSetString)(AbckitInst *inst, AbckitString *s);

    /**
     * @brief Returns value of I32 constant `inst`.
     * @return Value of `inst`.
     * @param [ in ] inst - Instruction to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is not a constant instruction.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is not I32 constant instruction.
     */
    int32_t (*iGetConstantValueI32)(AbckitInst *inst);

    /**
     * @brief Returns value of I64 constant `inst`.
     * @return Value of `inst`.
     * @param [ in ] inst - Instruction to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is not a constant instruction.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is not I64 constant instruction.
     */
    int64_t (*iGetConstantValueI64)(AbckitInst *inst);

    /**
     * @brief Returns value of U64 constant `inst`.
     * @return Value of `inst`.
     * @param [ in ] inst - Instruction to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is not a constant instruction.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is not U64 constant instruction.
     */
    uint64_t (*iGetConstantValueU64)(AbckitInst *inst);

    /**
     * @brief Returns value of F64 constant `inst`.
     * @return Value of `inst`.
     * @param [ in ] inst - Instruction to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is not a constant instruction.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is not F64 constant instruction.
     */
    double (*iGetConstantValueF64)(AbckitInst *inst);
};

/**
 * @brief Instantiates API for Abckit graph manipulation.
 * @return Instance of the `AbckitGraphApi` struct with valid function pointers.
 * @param [ in ] version - Version of the API to instantiate.
 * @note Set `ABCKIT_STATUS_UNKNOWN_API_VERSION` error if `version` value is not in the `AbckitApiVersion` enum.
 */
CAPI_EXPORT struct AbckitGraphApi const *AbckitGetGraphApiImpl(enum AbckitApiVersion version);

#ifdef __cplusplus
}
#endif

#endif /* LIBABCKIT_IR_H */
