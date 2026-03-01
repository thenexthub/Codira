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

#ifndef CPP_ABCKIT_INSTRUCTION_IMPL_H
#define CPP_ABCKIT_INSTRUCTION_IMPL_H

#include <cstdint>
#include "instruction.h"
#include "graph.h"
#include "file.h"
#include "core/function.h"
#include "core/import_descriptor.h"
#include "basic_block.h"
#include "core/export_descriptor.h"
#include "literal_array.h"

// NOLINTBEGIN(performance-unnecessary-value-param)
namespace abckit {

inline Instruction Instruction::InsertAfter(Instruction inst) const
{
    const ApiConfig *conf = GetApiConfig();
    conf->cGapi_->iInsertAfter(GetView(), inst.GetView());
    CheckError(conf);
    return Instruction(GetView(), GetApiConfig(), GetResource());
}

inline Instruction Instruction::InsertBefore(Instruction inst) const
{
    const ApiConfig *conf = GetApiConfig();
    conf->cGapi_->iInsertBefore(GetView(), inst.GetView());
    CheckError(conf);
    return Instruction(GetView(), GetApiConfig(), GetResource());
}

inline void Instruction::Remove() const
{
    GetApiConfig()->cGapi_->iRemove(GetView());
    CheckError(GetApiConfig());
}

inline int64_t Instruction::GetConstantValueI64() const
{
    int64_t ret = GetApiConfig()->cGapi_->iGetConstantValueI64(GetView());
    CheckError(GetApiConfig());
    return ret;
}

inline std::string Instruction::GetString() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitString *cString = conf->cGapi_->iGetString(GetView());
    CheckError(conf);
    std::string str = conf->cIapi_->abckitStringToString(cString);
    CheckError(conf);
    return str;
}

inline Instruction Instruction::GetNext() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitInst *inst = conf->cGapi_->iGetNext(GetView());
    CheckError(conf);
    return Instruction(inst, conf, GetResource());
}

inline Instruction Instruction::GetPrev() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitInst *inst = conf->cGapi_->iGetPrev(GetView());
    CheckError(conf);
    return Instruction(inst, conf, GetResource());
}

inline core::Function Instruction::GetFunction() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitCoreFunction *func = conf->cGapi_->iGetFunction(GetView());
    CheckError(conf);
    return core::Function(func, conf, GetResource()->GetFile());
}

inline BasicBlock Instruction::GetBasicBlock() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitBasicBlock *bb = conf->cGapi_->iGetBasicBlock(GetView());
    CheckError(conf);
    return BasicBlock(bb, conf, GetResource());
}

inline uint32_t Instruction::GetInputCount() const
{
    const ApiConfig *conf = GetApiConfig();
    uint32_t count = conf->cGapi_->iGetInputCount(GetView());
    CheckError(conf);
    return count;
}

inline Instruction Instruction::GetInput(uint32_t index) const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitInst *inInst = conf->cGapi_->iGetInput(GetView(), index);
    CheckError(conf);
    return Instruction(inInst, conf, GetResource());
}

inline Instruction Instruction::SetInput(uint32_t index, Instruction input) const
{
    const ApiConfig *conf = GetApiConfig();
    conf->cGapi_->iSetInput(GetView(), input.GetView(), index);
    CheckError(conf);
    return Instruction(GetView(), GetApiConfig(), GetResource());
}

template <typename... Args>
inline Instruction Instruction::SetInputs(Args... instrs)
{
    const ApiConfig *conf = GetApiConfig();
    conf->cGapi_->iSetInputs(GetView(), sizeof...(instrs), (instrs.GetView())...);
    CheckError(conf);
    return Instruction(GetView(), GetApiConfig(), GetResource());
}

inline bool Instruction::VisitUsers(const std::function<bool(Instruction)> &cb) const
{
    Payload<const std::function<bool(Instruction)> &> payload {cb, GetApiConfig(), GetResource()};

    auto isNormalExit = GetApiConfig()->cGapi_->iVisitUsers(GetView(), &payload, [](AbckitInst *user, void *data) {
        const auto &payload = *static_cast<Payload<const std::function<bool(Instruction)> &> *>(data);
        if (!payload.data(Instruction(user, payload.config, payload.resource))) {
            return false;
        };
        return true;
    });
    CheckError(GetApiConfig());
    return isNormalExit;
}

inline uint32_t Instruction::GetUserCount() const
{
    uint32_t count = GetApiConfig()->cGapi_->iGetUserCount(GetView());
    CheckError(GetApiConfig());
    return count;
}

inline LiteralArray Instruction::GetLiteralArray() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitLiteralArray *litarr = conf->cGapi_->iGetLiteralArray(GetView());
    CheckError(conf);
    return LiteralArray(litarr, conf, GetResource()->GetFile());
}

inline const Graph *Instruction::GetGraph() const
{
    return GetResource();
}

inline Type Instruction::GetType() const
{
    const ApiConfig *conf = GetApiConfig();
    auto *type = conf->cGapi_->iGetType(GetView());
    CheckError(conf);
    return Type(type, conf, GetResource()->GetFile());
}

inline enum AbckitBitImmSize Instruction::GetImmediateSize(size_t index) const
{
    const ApiConfig *conf = GetApiConfig();
    auto immBitSize = conf->cGapi_->iGetImmediateSize(GetView(), index);
    CheckError(conf);
    return immBitSize;
}

inline uint32_t Instruction::GetId() const
{
    const ApiConfig *conf = GetApiConfig();
    auto id = conf->cGapi_->iGetId(GetView());
    CheckError(conf);
    return id;
}

inline bool Instruction::CheckDominance(Instruction dominator) const
{
    const ApiConfig *conf = GetApiConfig();
    auto isDom = conf->cGapi_->iCheckDominance(GetView(), dominator.GetView());
    CheckError(conf);
    return isDom;
}

inline bool Instruction::CheckIsCall() const
{
    const ApiConfig *conf = GetApiConfig();
    auto isCall = conf->cGapi_->iCheckIsCall(GetView());
    CheckError(conf);
    return isCall;
}

inline Instruction Instruction::AppendInput(Instruction input) const
{
    const ApiConfig *conf = GetApiConfig();
    conf->cGapi_->iAppendInput(GetView(), input.GetView());
    CheckError(conf);
    return Instruction(GetView(), GetApiConfig(), GetResource());
}

inline Instruction Instruction::Dump(int32_t fd) const
{
    const ApiConfig *conf = GetApiConfig();
    conf->cGapi_->iDump(GetView(), fd);
    CheckError(conf);
    return Instruction(GetView(), GetApiConfig(), GetResource());
}

inline Instruction Instruction::SetFunction(core::Function function) const
{
    const ApiConfig *conf = GetApiConfig();
    conf->cGapi_->iSetFunction(GetView(), function.GetView());
    CheckError(conf);
    return Instruction(GetView(), GetApiConfig(), GetResource());
}

inline uint64_t Instruction::GetImmediate(size_t index) const
{
    const ApiConfig *conf = GetApiConfig();
    auto imm = conf->cGapi_->iGetImmediate(GetView(), index);
    CheckError(conf);
    return imm;
}

inline Instruction Instruction::SetImmediate(size_t index, uint64_t imm) const
{
    const ApiConfig *conf = GetApiConfig();
    conf->cGapi_->iSetImmediate(GetView(), index, imm);
    CheckError(conf);
    return Instruction(GetView(), GetApiConfig(), GetResource());
}

inline uint64_t Instruction::GetImmediateCount() const
{
    const ApiConfig *conf = GetApiConfig();
    auto count = conf->cGapi_->iGetImmediateCount(GetView());
    CheckError(conf);
    return count;
}

inline Instruction Instruction::SetLiteralArray(LiteralArray la) const
{
    const ApiConfig *conf = GetApiConfig();
    conf->cGapi_->iSetLiteralArray(GetView(), la.GetView());
    CheckError(conf);
    return Instruction(GetView(), GetApiConfig(), GetResource());
}

inline Instruction Instruction::SetString(std::string_view s) const
{
    const ApiConfig *conf = GetApiConfig();
    auto *str = conf->cMapi_->createString(GetResource()->GetFile()->GetResource(), s.data(), s.size());
    CheckError(GetApiConfig());
    conf->cGapi_->iSetString(GetView(), str);
    CheckError(conf);
    return Instruction(GetView(), GetApiConfig(), GetResource());
}

inline int32_t Instruction::GetConstantValueI32() const
{
    const ApiConfig *conf = GetApiConfig();
    auto val = conf->cGapi_->iGetConstantValueI32(GetView());
    CheckError(conf);
    return val;
}

inline uint64_t Instruction::GetConstantValueU64() const
{
    const ApiConfig *conf = GetApiConfig();
    auto val = conf->cGapi_->iGetConstantValueU64(GetView());
    CheckError(conf);
    return val;
}

inline double Instruction::GetConstantValueF64() const
{
    const ApiConfig *conf = GetApiConfig();
    auto val = conf->cGapi_->iGetConstantValueF64(GetView());
    CheckError(conf);
    return val;
}

}  // namespace abckit
// NOLINTEND(performance-unnecessary-value-param)

#endif  // CPP_ABCKIT_INSTRUCTION_IMPL_H
