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

#ifndef CPP_ABCKIT_GRAPH_H
#define CPP_ABCKIT_GRAPH_H

#include "base_classes.h"
#include "basic_block.h"
#include "dynamic_isa.h"
#include "instruction.h"
#include "static_isa.h"

#include <memory>
#include <vector>

namespace abckit {

/**
 * @brief Graph
 */
class Graph final : public Resource<AbckitGraph *> {
    // To access private constructor.
    // We restrict constructors in order to prevent C/C++ API mix-up by user.

    /// @brief To access private constructor
    friend class core::Function;
    /// @brief To access private constructor
    friend class DynamicIsa;
    /// @brief To access private constructor
    friend class StaticIsa;
    /// @brief To access private constructor
    friend class BasicBlock;

public:
    /**
     * @brief Deleted constructor
     * @param other
     */
    Graph(const Graph &other) = delete;

    /**
     * @brief Deleted constructor
     * @param other
     * @return Graph
     */
    Graph &operator=(const Graph &other) = delete;

    /**
     * @brief Constructor
     * @param other
     */
    Graph(Graph &&other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return Graph
     */
    Graph &operator=(Graph &&other) = default;

    /**
     * @brief Destructor
     */
    ~Graph() override = default;

    /**
     * @brief Returns ISA type for a graph.
     * @return ISA of the graph.
     */
    AbckitIsaType GetIsa() const;

    /**
     * @brief Returns binary file that the current `Graph` is a part of.
     * @return Pointer to the `File`
     */
    const File *GetFile() const
    {
        return file_;
    }

    /**
     * @brief Get start basic block
     * @return `BasicBlock`
     */
    BasicBlock GetStartBb() const;

    /**
     * @brief Returns end basic block of current `Graph`.
     * @return End `BasicBlock`.
     */
    BasicBlock GetEndBb() const;

    /**
     * @brief Returns number of basic blocks in a `graph`.
     * @return Number of basic blocks in a `graph`.
     */
    inline uint32_t GetNumBbs() const;

    /**
     * @brief Get blocks RPO
     * @return vector of `BasicBlock`
     */
    std::vector<BasicBlock> GetBlocksRPO() const;

    /**
     * @brief Get dynamic ISA
     * @return `DynamicIsa`
     */
    DynamicIsa DynIsa() const;

    /**
     * @brief Get static ISA
     * @return `StaticIsa`
     */
    StaticIsa StatIsa() const;

    /**
     * @brief EnumerateBasicBlocksRpo
     * @return New state of `graph`.
     * @param cb
     */
    const Graph &EnumerateBasicBlocksRpo(const std::function<bool(BasicBlock)> &cb) const;

    /**
     * @brief Returns basic blocks with given `id` of a graph.
     * @return BasicBlock with given `id`.
     * @param [ in ] bbId - ID of basic block.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if there is no basic block with given `id` in `graph`.
     */
    BasicBlock GetBasicBlock(uint32_t bbId) const;

    /**
     * @brief Returns parameter instruction under given `index` of a `graph`.
     * @return Parameter instruction under given `index` of a `graph`.
     * @param [ in ] index - Index of the parameter.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if there is no parameter under given `index` in `graph`.
     */
    Instruction GetParameter(uint32_t index) const;

    /**
     * @brief Returns number of instruction parameters under a `graph`.
     * @return uint32_t corresponding to number of instruction parameters.
     */
    uint32_t GetNumberOfParameters() const;

    /**
     * @brief Wraps basic blocks from `tryFirstBB` to `tryLastBB` into try,
     * inserts basic blocks from `catchBeginBB` to `catchEndBB` into graph.
     * Basic blocks from `catchBeginBB` to `catchEndBB` are used for exception handling.
     * @return New state of `graph`.
     * @param [ in ] tryBegin - Start basic block to wrap into try.
     * @param [ in ] tryEnd - End basic block to wrap into try.
     * @param [ in ] catchBegin - Start basic block to handle exception.
     * @param [ in ] catchEnd - End basic block to handle exception.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `tryBegin` is Empty.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `tryEnd` is Empty.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `catchBegin` is Empty.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `catchEnd` is Empty.
     */
    const Graph &InsertTryCatch(BasicBlock tryBegin, BasicBlock tryEnd, BasicBlock catchBegin,
                                BasicBlock catchEnd) const;

    /**
     * @brief Dumps a `graph` into given file descriptor.
     * @return New state of `graph`.
     * @param [ in ] fd - File descriptor where dump is written.
     */
    const Graph &Dump(int32_t fd) const;

    /**
     * @brief Creates I32 constant instruction and inserts it in start basic block of current `Graph`.
     * @return Created `Instruction`.
     * @param [ in ] val - value of created constant instruction.
     * @note Allocates
     */
    Instruction FindOrCreateConstantI32(int32_t val) const;

    /**
     * @brief Creates I64 constant instruction and inserts it in start basic block of current `Graph`.
     * @return Created `Instruction`.
     * @param [ in ] val - value of created constant instruction.
     * @note Allocates
     */
    Instruction FindOrCreateConstantI64(int64_t val) const;

    /**
     * @brief Creates U64 constant instruction and inserts it in start basic block of current `Graph`.
     * @return Created `Instruction`.
     * @param [ in ] val - value of created constant instruction.
     * @note Allocates
     */
    Instruction FindOrCreateConstantU64(uint64_t val) const;

    /**
     * @brief Creates F64 constant instruction and inserts it in start basic block of current `Graph`.
     * @return Created `Instruction`.
     * @param [ in ] val - value of created constant instruction.
     * @note Allocates
     */
    Instruction FindOrCreateConstantF64(double val) const;

    /**
     * @brief Creates empty basic block.
     * @return Created `BasicBlock`.
     * @note Allocates
     */
    BasicBlock CreateEmptyBb() const;

    /**
     * @brief Removes all basic blocks unreachable from start basic block.
     * @return New state of `graph`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `bool(*this)` results in `false`
     */
    const Graph &RunPassRemoveUnreachableBlocks() const;

protected:
    /**
     * @brief Get api config
     * @return `ApiConfig`
     */
    const ApiConfig *GetApiConfig() const override
    {
        return conf_;
    }

    /**
     * @brief Struct for using in callbacks
     */
    template <typename D>
    struct Payload {
        /**
         * @brief data
         */
        D data;
        /**
         * @brief config
         */
        const ApiConfig *config;
        /**
         * @brief resource
         */
        const Graph *resource;
    };

private:
    class GraphDeleter final : public IResourceDeleter {
    public:
        /**
         * @brief Constructor
         * @param conf
         * @param graph
         */
        GraphDeleter(const ApiConfig *conf, const Graph &graph) : conf_(conf), deleterGraph_(graph) {};

        /**
         * @brief Deleted constructor
         * @param other
         */
        GraphDeleter(const GraphDeleter &other) = delete;

        /**
         * @brief Deleted constructor
         * @param other
         */
        GraphDeleter &operator=(const GraphDeleter &other) = delete;

        /**
         * @brief Deleted constructor
         * @param other
         */
        GraphDeleter(GraphDeleter &&other) = delete;

        /**
         * @brief Deleted constructor
         * @param other
         */
        GraphDeleter &operator=(GraphDeleter &&other) = delete;

        /**
         * @brief Destructor
         */
        ~GraphDeleter() override = default;

        /**
         * @brief Delete resource
         */
        void DeleteResource() override
        {
            conf_->cApi_->destroyGraph(deleterGraph_.GetResource());
        }

    private:
        const ApiConfig *conf_;
        const Graph &deleterGraph_;
    };

    Graph(AbckitGraph *graph, const ApiConfig *conf, const File *file) : Resource(graph), conf_(conf), file_(file)
    {
        SetDeleter(std::make_unique<GraphDeleter>(conf_, *this));
    };
    // for interop with API
    const ApiConfig *conf_;
    const File *file_;
};

}  // namespace abckit

#endif  // CPP_ABCKIT_GRAPH_H
