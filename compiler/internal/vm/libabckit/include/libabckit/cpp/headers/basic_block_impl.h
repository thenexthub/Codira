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

#ifndef CPP_ABCKIT_BASIC_BLOCK_IMPL_H
#define CPP_ABCKIT_BASIC_BLOCK_IMPL_H

#include <cstdint>
#include "basic_block.h"
#include "instruction.h"

// NOLINTBEGIN(performance-unnecessary-value-param)
namespace abckit {

inline uint64_t BasicBlock::GetSuccCount() const
{
    uint64_t count = GetApiConfig()->cGapi_->bbGetSuccBlockCount(GetView());
    CheckError(GetApiConfig());
    return count;
}

inline BasicBlock BasicBlock::GetSuccByIdx(uint32_t idx) const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitBasicBlock *abcBB = conf->cGapi_->bbGetSuccBlock(GetView(), idx);
    CheckError(conf);
    return BasicBlock(abcBB, conf, GetResource());
}

// CC-OFFNXT(G.FUD.06) perf critical
inline std::vector<BasicBlock> BasicBlock::GetSuccs() const
{
    std::vector<BasicBlock> bBs;
    Payload<std::vector<BasicBlock> *> payload {&bBs, GetApiConfig(), GetResource()};

    GetApiConfig()->cGapi_->bbVisitSuccBlocks(GetView(), &payload, [](AbckitBasicBlock *succ, void *data) {
        const auto &payload = *static_cast<Payload<std::vector<BasicBlock> *> *>(data);
        payload.data->push_back(BasicBlock(succ, payload.config, payload.resource));
        return true;
    });

    CheckError(GetApiConfig());

    return bBs;
}

// CC-OFFNXT(G.FUD.06) perf critical
inline std::vector<BasicBlock> BasicBlock::GetPreds() const
{
    std::vector<BasicBlock> bBs;
    Payload<std::vector<BasicBlock> *> payload {&bBs, GetApiConfig(), GetResource()};

    GetApiConfig()->cGapi_->bbVisitPredBlocks(GetView(), &payload, [](AbckitBasicBlock *succ, void *data) {
        const auto &payload = *static_cast<Payload<std::vector<BasicBlock> *> *>(data);
        payload.data->push_back(BasicBlock(succ, payload.config, payload.resource));
        return true;
    });

    CheckError(GetApiConfig());

    return bBs;
}

inline std::vector<Instruction> BasicBlock::GetInstructions() const
{
    std::vector<Instruction> insts;
    auto *conf = GetApiConfig();
    auto *inst = conf->cGapi_->bbGetFirstInst(GetView());
    CheckError(conf);
    while (inst != nullptr) {
        insts.push_back(Instruction(inst, conf, GetResource()));
        inst = conf->cGapi_->iGetNext(inst);
        CheckError(conf);
    }
    return insts;
}

inline Instruction BasicBlock::GetFirstInst() const
{
    auto *conf = GetApiConfig();
    auto *inst = conf->cGapi_->bbGetFirstInst(GetView());
    CheckError(conf);
    return Instruction(inst, conf, GetResource());
}

inline Instruction BasicBlock::GetLastInst() const
{
    auto *conf = GetApiConfig();
    auto *inst = conf->cGapi_->bbGetLastInst(GetView());
    CheckError(conf);
    return Instruction(inst, conf, GetResource());
}

inline BasicBlock BasicBlock::AddInstFront(Instruction inst) const
{
    GetApiConfig()->cGapi_->bbAddInstFront(GetView(), inst.GetView());
    CheckError(GetApiConfig());
    return BasicBlock(GetView(), GetApiConfig(), GetResource());
}

inline BasicBlock BasicBlock::AddInstBack(Instruction inst) const
{
    GetApiConfig()->cGapi_->bbAddInstBack(GetView(), inst.GetView());
    CheckError(GetApiConfig());
    return BasicBlock(GetView(), GetApiConfig(), GetResource());
}

inline void BasicBlock::VisitSuccBlocks(const std::function<void(BasicBlock)> &cb) const
{
    Payload<const std::function<void(BasicBlock)> &> payload {cb, GetApiConfig(), GetResource()};

    GetApiConfig()->cGapi_->bbVisitSuccBlocks(GetView(), &payload, [](AbckitBasicBlock *succ, void *data) {
        const auto &payload = *static_cast<Payload<const std::function<void(BasicBlock)> &> *>(data);
        payload.data(BasicBlock(succ, payload.config, payload.resource));
        return true;
    });
    CheckError(GetApiConfig());
}

inline void BasicBlock::VisitPredBlocks(const std::function<bool(BasicBlock)> &cb) const
{
    Payload<const std::function<bool(BasicBlock)> &> payload {cb, GetApiConfig(), GetResource()};

    GetApiConfig()->cGapi_->bbVisitPredBlocks(GetView(), &payload, [](AbckitBasicBlock *succ, void *data) {
        const auto &payload = *static_cast<Payload<const std::function<bool(BasicBlock)> &> *>(data);
        return payload.data(BasicBlock(succ, payload.config, payload.resource));
    });
    CheckError(GetApiConfig());
}

inline uint32_t BasicBlock::GetId() const
{
    auto id = GetApiConfig()->cGapi_->bbGetId(GetView());
    CheckError(GetApiConfig());
    return id;
}

inline const Graph *BasicBlock::GetGraph() const
{
    return GetResource();
}

inline BasicBlock BasicBlock::SplitBlockAfterInstruction(Instruction ins, bool makeEdge)
{
    auto *bb = GetApiConfig()->cGapi_->bbSplitBlockAfterInstruction(GetView(), ins.GetView(), makeEdge);
    CheckError(GetApiConfig());
    return BasicBlock(bb, GetApiConfig(), GetResource());
}

inline uint64_t BasicBlock::GetPredBlockCount() const
{
    auto count = GetApiConfig()->cGapi_->bbGetPredBlockCount(GetView());
    CheckError(GetApiConfig());
    return count;
}

inline BasicBlock BasicBlock::GetPredBlock(uint32_t index) const
{
    auto *bb = GetApiConfig()->cGapi_->bbGetPredBlock(GetView(), index);
    CheckError(GetApiConfig());
    return BasicBlock(bb, GetApiConfig(), GetResource());
}

inline uint64_t BasicBlock::GetSuccBlockCount() const
{
    auto count = GetApiConfig()->cGapi_->bbGetSuccBlockCount(GetView());
    CheckError(GetApiConfig());
    return count;
}

inline BasicBlock BasicBlock::GetSuccBlock(uint32_t index) const
{
    auto *bb = GetApiConfig()->cGapi_->bbGetSuccBlock(GetView(), index);
    CheckError(GetApiConfig());
    return BasicBlock(bb, GetApiConfig(), GetResource());
}

inline BasicBlock BasicBlock::InsertSuccBlock(BasicBlock succBlock, uint32_t index)
{
    GetApiConfig()->cGapi_->bbInsertSuccBlock(GetView(), succBlock.GetView(), index);
    CheckError(GetApiConfig());
    return BasicBlock(GetView(), GetApiConfig(), GetResource());
}

inline BasicBlock BasicBlock::AppendSuccBlock(BasicBlock succBlock)
{
    GetApiConfig()->cGapi_->bbAppendSuccBlock(GetView(), succBlock.GetView());
    CheckError(GetApiConfig());
    return BasicBlock(GetView(), GetApiConfig(), GetResource());
}

inline BasicBlock BasicBlock::EraseSuccBlock(uint32_t index)
{
    GetApiConfig()->cGapi_->bbDisconnectSuccBlock(GetView(), index);
    CheckError(GetApiConfig());
    return BasicBlock(GetView(), GetApiConfig(), GetResource());
}

inline BasicBlock BasicBlock::GetTrueBranch() const
{
    auto *bb = GetApiConfig()->cGapi_->bbGetTrueBranch(GetView());
    CheckError(GetApiConfig());
    return BasicBlock(bb, GetApiConfig(), GetResource());
}

inline BasicBlock BasicBlock::GetFalseBranch() const
{
    auto *bb = GetApiConfig()->cGapi_->bbGetFalseBranch(GetView());
    CheckError(GetApiConfig());
    return BasicBlock(bb, GetApiConfig(), GetResource());
}

inline BasicBlock BasicBlock::RemoveAllInsts()
{
    GetApiConfig()->cGapi_->bbRemoveAllInsts(GetView());
    CheckError(GetApiConfig());
    return BasicBlock(GetView(), GetApiConfig(), GetResource());
}

inline uint32_t BasicBlock::GetNumberOfInstructions() const
{
    auto num = GetApiConfig()->cGapi_->bbGetNumberOfInstructions(GetView());
    CheckError(GetApiConfig());
    return num;
}

inline BasicBlock BasicBlock::GetImmediateDominator() const
{
    auto dom = GetApiConfig()->cGapi_->bbGetImmediateDominator(GetView());
    CheckError(GetApiConfig());
    return BasicBlock(dom, GetApiConfig(), GetResource());
}

inline bool BasicBlock::CheckDominance(BasicBlock dom) const
{
    auto check = GetApiConfig()->cGapi_->bbCheckDominance(GetView(), dom.GetView());
    CheckError(GetApiConfig());
    return check;
}

inline void BasicBlock::VisitDominatedBlocks(const std::function<bool(BasicBlock)> &cb) const
{
    Payload<const std::function<bool(BasicBlock)> &> payload {cb, GetApiConfig(), GetResource()};

    GetApiConfig()->cGapi_->bbVisitDominatedBlocks(GetView(), &payload, [](AbckitBasicBlock *succ, void *data) {
        const auto &payload = *static_cast<Payload<const std::function<bool(BasicBlock)> &> *>(data);
        return payload.data(BasicBlock(succ, payload.config, payload.resource));
    });
    CheckError(GetApiConfig());
}

inline bool BasicBlock::IsStart() const
{
    auto check = GetApiConfig()->cGapi_->bbIsStart(GetView());
    CheckError(GetApiConfig());
    return check;
}

inline bool BasicBlock::IsEnd() const
{
    auto check = GetApiConfig()->cGapi_->bbIsEnd(GetView());
    CheckError(GetApiConfig());
    return check;
}

inline bool BasicBlock::IsLoopHead() const
{
    auto check = GetApiConfig()->cGapi_->bbIsLoopHead(GetView());
    CheckError(GetApiConfig());
    return check;
}

inline bool BasicBlock::IsLoopPrehead() const
{
    auto check = GetApiConfig()->cGapi_->bbIsLoopPrehead(GetView());
    CheckError(GetApiConfig());
    return check;
}

inline bool BasicBlock::IsTryBegin() const
{
    auto check = GetApiConfig()->cGapi_->bbIsTryBegin(GetView());
    CheckError(GetApiConfig());
    return check;
}

inline bool BasicBlock::IsTry() const
{
    auto check = GetApiConfig()->cGapi_->bbIsTry(GetView());
    CheckError(GetApiConfig());
    return check;
}

inline bool BasicBlock::IsTryEnd() const
{
    auto check = GetApiConfig()->cGapi_->bbIsTryEnd(GetView());
    CheckError(GetApiConfig());
    return check;
}

inline bool BasicBlock::IsCatchBegin() const
{
    auto check = GetApiConfig()->cGapi_->bbIsCatchBegin(GetView());
    CheckError(GetApiConfig());
    return check;
}

inline bool BasicBlock::IsCatch() const
{
    auto check = GetApiConfig()->cGapi_->bbIsCatch(GetView());
    CheckError(GetApiConfig());
    return check;
}

inline BasicBlock BasicBlock::Dump(int32_t fd)
{
    GetApiConfig()->cGapi_->bbDump(GetView(), fd);
    CheckError(GetApiConfig());
    return BasicBlock(GetView(), GetApiConfig(), GetResource());
}

template <typename... Args>
inline Instruction BasicBlock::CreatePhi(Args... args)
{
    auto *inst = GetApiConfig()->cGapi_->bbCreatePhi(GetView(), sizeof...(args), (args.GetView())...);
    CheckError(GetApiConfig());
    return Instruction(inst, GetApiConfig(), GetResource());
}

template <typename... Args>
inline Instruction BasicBlock::CreateCatchPhi(Args... args)
{
    auto *inst = GetApiConfig()->cGapi_->bbCreateCatchPhi(GetView(), sizeof...(args), (args.GetView())...);
    CheckError(GetApiConfig());
    return Instruction(inst, GetApiConfig(), GetResource());
}

}  // namespace abckit
// NOLINTEND(performance-unnecessary-value-param)

#endif  // CPP_ABCKIT_BASIC_BLOCK_H
