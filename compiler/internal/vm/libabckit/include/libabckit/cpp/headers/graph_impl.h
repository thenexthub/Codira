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

#ifndef CPP_ABCKIT_GRAPH_IMPL_H
#define CPP_ABCKIT_GRAPH_IMPL_H

#include <cstdint>
#include "graph.h"
#include "dynamic_isa.h"
#include "basic_block.h"
#include "config.h"

// NOLINTBEGIN(performance-unnecessary-value-param)

namespace abckit {

inline DynamicIsa Graph::DynIsa() const
{
    return DynamicIsa(*this);
}

inline StaticIsa Graph::StatIsa() const
{
    return StaticIsa(*this);
}

inline AbckitIsaType Graph::GetIsa() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitIsaType type = conf->cGapi_->gGetIsa(GetResource());
    CheckError(conf);
    return type;
}

inline BasicBlock Graph::GetStartBb() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitBasicBlock *bb = conf->cGapi_->gGetStartBasicBlock(GetResource());
    CheckError(conf);
    return BasicBlock(bb, conf, this);
}

inline BasicBlock Graph::GetEndBb() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitBasicBlock *bb = conf->cGapi_->gGetEndBasicBlock(GetResource());
    CheckError(conf);
    return BasicBlock(bb, conf, this);
}

inline uint32_t Graph::GetNumBbs() const
{
    const ApiConfig *conf = GetApiConfig();
    uint32_t bbs = conf->cGapi_->gGetNumberOfBasicBlocks(GetResource());
    CheckError(conf);
    return bbs;
}

// CC-OFFNXT(G.FUD.06) perf critical
inline std::vector<BasicBlock> Graph::GetBlocksRPO() const
{
    std::vector<BasicBlock> blocks;
    Payload<std::vector<BasicBlock> *> payload {&blocks, GetApiConfig(), this};

    GetApiConfig()->cGapi_->gVisitBlocksRpo(GetResource(), &payload, [](AbckitBasicBlock *bb, void *data) {
        const auto &payload = *static_cast<Payload<std::vector<BasicBlock> *> *>(data);
        payload.data->push_back(BasicBlock(bb, payload.config, payload.resource));
        return true;
    });

    CheckError(GetApiConfig());

    return blocks;
}

// CC-OFFNXT(G.FUD.06) perf critical
inline const Graph &Graph::EnumerateBasicBlocksRpo(const std::function<bool(BasicBlock)> &cb) const
{
    Payload<const std::function<void(BasicBlock)> &> payload {cb, GetApiConfig(), this};

    GetApiConfig()->cGapi_->gVisitBlocksRpo(GetResource(), &payload, [](AbckitBasicBlock *bb, void *data) -> bool {
        const auto &payload = *static_cast<Payload<const std::function<void(BasicBlock)> &> *>(data);
        payload.data(BasicBlock(bb, payload.config, payload.resource));
        return true;
    });
    CheckError(GetApiConfig());
    return *this;
}

inline BasicBlock Graph::GetBasicBlock(uint32_t bbId) const
{
    AbckitBasicBlock *bb = GetApiConfig()->cGapi_->gGetBasicBlock(GetResource(), bbId);
    CheckError(GetApiConfig());
    return BasicBlock(bb, GetApiConfig(), this);
}

inline Instruction Graph::GetParameter(uint32_t index) const
{
    AbckitInst *inst = GetApiConfig()->cGapi_->gGetParameter(GetResource(), index);
    CheckError(GetApiConfig());
    return Instruction(inst, GetApiConfig(), this);
}

inline uint32_t Graph::GetNumberOfParameters() const
{
    uint32_t number = GetApiConfig()->cGapi_->gGetNumberOfParameters(GetResource());
    CheckError(GetApiConfig());
    return number;
}

inline const Graph &Graph::InsertTryCatch(BasicBlock tryBegin, BasicBlock tryEnd, BasicBlock catchBegin,
                                          BasicBlock catchEnd) const
{
    const ApiConfig *conf = GetApiConfig();
    conf->cGapi_->gInsertTryCatch(tryBegin.GetView(), tryEnd.GetView(), catchBegin.GetView(), catchEnd.GetView());
    CheckError(conf);
    return *this;
}

inline const Graph &Graph::Dump(int32_t fd) const
{
    const ApiConfig *conf = GetApiConfig();
    conf->cGapi_->gDump(GetResource(), fd);
    CheckError(conf);
    return *this;
}

inline Instruction Graph::FindOrCreateConstantI32(int32_t val) const
{
    AbckitInst *inst = GetApiConfig()->cGapi_->gFindOrCreateConstantI32(GetResource(), val);
    CheckError(GetApiConfig());
    return Instruction(inst, GetApiConfig(), this);
}

inline Instruction Graph::FindOrCreateConstantI64(int64_t val) const
{
    AbckitInst *inst = GetApiConfig()->cGapi_->gFindOrCreateConstantI64(GetResource(), val);
    CheckError(GetApiConfig());
    return Instruction(inst, GetApiConfig(), this);
}

inline Instruction Graph::FindOrCreateConstantU64(uint64_t val) const
{
    AbckitInst *inst = GetApiConfig()->cGapi_->gFindOrCreateConstantU64(GetResource(), val);
    CheckError(GetApiConfig());
    return Instruction(inst, GetApiConfig(), this);
}

inline Instruction Graph::FindOrCreateConstantF64(double val) const
{
    AbckitInst *inst = GetApiConfig()->cGapi_->gFindOrCreateConstantF64(GetResource(), val);
    CheckError(GetApiConfig());
    return Instruction(inst, GetApiConfig(), this);
}

inline BasicBlock Graph::CreateEmptyBb() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitBasicBlock *bb = conf->cGapi_->bbCreateEmpty(GetResource());
    CheckError(conf);
    return BasicBlock(bb, conf, this);
}

inline const Graph &Graph::RunPassRemoveUnreachableBlocks() const
{
    const ApiConfig *conf = GetApiConfig();
    conf->cGapi_->gRunPassRemoveUnreachableBlocks(GetResource());
    CheckError(conf);
    return *this;
}

}  // namespace abckit

// NOLINTEND(performance-unnecessary-value-param)

#endif  // CPP_ABCKIT_GRAPH_IMPL_H
