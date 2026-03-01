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

#include "optimizer/code_generator/codegen.h"
#include "optimizer/code_generator/encode.h"
#include "optimizer/code_generator/spill_fill_encoder.h"
#include "optimizer/ir/graph.h"

namespace ark::compiler {

bool SpillFillEncoder::AreConsecutiveOps(const SpillFillData &pred, const SpillFillData &succ)
{
    bool sameSrcType = pred.SrcType() == succ.SrcType();
    bool sameDstType = pred.DstType() == succ.DstType();
    bool sameArgumentType = pred.GetCommonType() == succ.GetCommonType();
    if (!sameSrcType || !sameDstType || !sameArgumentType) {
        return false;
    }

    // Slots should be neighboring, note that offset from SP is decreasing when slot number is increasing,
    // so succ's slot number should be lower than pred's slot number.
    if (pred.SrcType() == LocationType::STACK && pred.SrcValue() != succ.SrcValue() + 1U) {
        return false;
    }
    if (pred.DstType() == LocationType::STACK && pred.DstValue() != succ.DstValue() + 1U) {
        return false;
    }
    // Parameter slots have another direction
    if (pred.SrcType() == LocationType::STACK_PARAMETER && pred.SrcValue() != succ.SrcValue() - 1U) {
        return false;
    }
    if (pred.DstType() == LocationType::STACK_PARAMETER && pred.DstValue() != succ.DstValue() - 1U) {
        return false;
    }
    return true;
}

bool SpillFillEncoder::CanCombineSpillFills(SpillFillData pred, SpillFillData succ, const Graph *graph)
{
    if (!IsCombiningEnabled(graph)) {
        return false;
    }
    // Stack slot is 64-bit wide, so we can only combine types that could be widened up to
    // 64 bit (i.e. we can' combine two floats).
    if (!DataType::Is64Bits(pred.GetCommonType(), graph->GetArch())) {
        return false;
    }

    return AreConsecutiveOps(pred, succ);
}

void SpillFillEncoder::SortSpillFillData(ArenaVector<SpillFillData> *spillFills)
{
    constexpr size_t MAX_VECTOR_LEN = MAX_NUM_REGS + MAX_NUM_VREGS;
    // Don't sort vectors that are too large in order to reduce compilation duration.
    if (spillFills->size() > MAX_VECTOR_LEN) {
        COMPILER_LOG(DEBUG, CODEGEN) << "Bypass spill fills sorting because corresponding vector is too large: "
                                     << spillFills->size();
        return;
    }
    auto it = spillFills->begin();
    while (it != spillFills->end()) {
        // Sort spill fills only within group of consecutive SpillFillData elements sharing the same spill-fill type.
        // SpillFillData elements could not be reordered within whole spill_fills array, because some of these elements
        // may be inserted by SpillFillResolver to break cyclic dependency.
        bool isFill = it->SrcType() == LocationType::STACK && it->GetDst().IsAnyRegister();
        bool isSpill = it->GetSrc().IsAnyRegister() && it->DstType() == LocationType::STACK;
        if (!isSpill && !isFill) {
            ++it;
            continue;
        }
        auto next = std::next(it);
        while (next != spillFills->end() && it->SrcType() == next->SrcType() && it->DstType() == next->DstType()) {
            ++next;
        }

        if (isSpill) {
            std::sort(it, next, [](auto sf1, auto sf2) { return sf1.DstValue() > sf2.DstValue(); });
        } else {
            ASSERT(isFill);
            std::sort(it, next, [](auto sf1, auto sf2) { return sf1.SrcValue() > sf2.SrcValue(); });
        }

        it = next;
    }
}

SpillFillEncoder::SpillFillEncoder(Codegen *codegen, Inst *inst)
    : inst_(inst->CastToSpillFill()),
      graph_(codegen->GetGraph()),
      codegen_(codegen),
      encoder_(codegen->GetEncoder()),
      fl_(codegen->GetFrameLayout())
{
    spReg_ = codegen->GetTarget().GetStackReg();
    fpReg_ = codegen->GetTarget().GetFrameReg();
}

void SpillFillEncoder::EncodeSpillFill()
{
    if (IsCombiningEnabled(graph_)) {
        SortSpillFillData(&(inst_->GetSpillFills()));
    }

    // hint on how many consecutive ops current group contain
    int consecutiveOpsHint = 0;
    for (auto it = inst_->GetSpillFills().begin(), end = inst_->GetSpillFills().end(); it != end;) {
        auto sf = *it;
        auto nextIt = std::next(it);
        SpillFillData *next = nextIt == end ? nullptr : &(*nextIt);

        // new group started
        if (consecutiveOpsHint <= 0) {
            consecutiveOpsHint = 1;
            // find how many consecutive SpillFillData have the same type, source and destination type
            // and perform read or write from consecutive stack slots.
            for (auto groupIt = it, nextGroupIt = std::next(it);
                 nextGroupIt != end && AreConsecutiveOps(*groupIt, *nextGroupIt); ++nextGroupIt) {
                consecutiveOpsHint++;
                groupIt = nextGroupIt;
            }
        }

        size_t adv = 0;
        switch (sf.SrcType()) {
            case LocationType::IMMEDIATE: {
                adv = EncodeImmToX(sf);
                break;
            }
            case LocationType::FP_REGISTER:
            case LocationType::REGISTER: {
                adv = EncodeRegisterToX(sf, next, consecutiveOpsHint);
                break;
            }
            case LocationType::STACK_PARAMETER:
            case LocationType::STACK: {
                adv = EncodeStackToX(sf, next, consecutiveOpsHint);
                break;
            }
            default:
                UNREACHABLE();
        }
        consecutiveOpsHint -= static_cast<int>(adv);
        std::advance(it, adv);
    }
}

void SpillFillEncoder::EncodeImmWithCorrectType(DataType::Type sfType, MemRef dstMem, ConstantInst *constInst)
{
    ASSERT(DataType::IsTypeNumeric(sfType));
    switch (sfType) {
        case DataType::Type::FLOAT32: {
            auto imm = constInst->GetFloatValue();
            encoder_->EncodeSti(imm, dstMem);
            break;
        }
        case DataType::Type::FLOAT64: {
            auto imm = constInst->GetDoubleValue();
            encoder_->EncodeSti(imm, dstMem);
            break;
        }
        default: {
            auto imm = constInst->GetRawValue();
            auto storeSize = Codegen::ConvertDataType(sfType, codegen_->GetArch()).GetSize() / BYTE_SIZE;
            encoder_->EncodeSti(imm, storeSize, dstMem);
            break;
        }
    }
}

size_t SpillFillEncoder::EncodeImmToX(const SpillFillData &sf)
{
    auto constInst = graph_->GetSpilledConstant(sf.SrcValue());
    ASSERT(constInst->IsConst());

    if (sf.GetDst().IsAnyRegister()) {  // imm -> register
        auto type = sf.GetType();
        if (graph_->IsDynamicMethod() && constInst->GetType() == DataType::INT64) {
            type = DataType::UINT32;
        }

        Imm imm;
#ifndef NDEBUG
        switch (type) {
            case DataType::FLOAT32:
                imm = Imm(constInst->GetFloatValue());
                break;
            case DataType::FLOAT64:
                imm = Imm(constInst->GetDoubleValue());
                break;
            default:
                ASSERT(DataType::IsTypeNumeric(type));
                imm = Imm(constInst->GetRawValue());
                break;
        }
#else
        imm = Imm(constInst->GetRawValue());
#endif
        auto dstReg = GetDstReg(sf.GetDst(), Codegen::ConvertDataType(type, codegen_->GetArch()));
        encoder_->EncodeMov(dstReg, imm);
        return 1U;
    }

    ASSERT(sf.GetDst().IsAnyStack());  // imm -> stack
    auto dstMem = codegen_->GetMemRefForSlot(codegen_->GetFrameInfo()->GetSpillsOffsetOrigin(), sf.GetDst());
    auto sfType = sf.GetCommonType();
    EncodeImmWithCorrectType(sfType, dstMem, constInst);
    return 1U;
}

size_t SpillFillEncoder::EncodeRegisterToX(const SpillFillData &sf, const SpillFillData *next, int consecutiveOpsHint)
{
    if (sf.GetDst().IsAnyRegister()) {  // register -> register
        auto srcReg = codegen_->ConvertRegister(sf.SrcValue(), sf.GetType());
        auto dstReg = GetDstReg(sf.GetDst(), srcReg.GetType());
        encoder_->EncodeMov(dstReg, srcReg);
        return 1U;
    }

    ASSERT(sf.GetDst().IsAnyStack());

    if (sf.GetDst().IsStackArgument()) {  // register -> stack_arg
        auto memRef = codegen_->GetMemRefForSlot(CFrameLayout::OffsetOrigin::SP, sf.GetDst());
        auto srcReg = codegen_->ConvertRegister(sf.SrcValue(), sf.GetType());
        // There is possible to have sequence to intrinsics with no getter/setter in interpreter:
        // compiled_code->c2i(push to frame)->interpreter(HandleCallVirtShort)->i2c(move to stack)->intrinsic
        // To do not fix it in interpreter, it is better to store 64-bits
        if (srcReg.GetSize() < DOUBLE_WORD_SIZE && !srcReg.GetType().IsFloat()) {
            srcReg = srcReg.As(Codegen::ConvertDataType(DataType::REFERENCE, codegen_->GetArch()));
        }
        encoder_->EncodeStrz(srcReg, memRef);
        return 1U;
    }

    auto memRef = codegen_->GetMemRefForSlot(codegen_->GetFrameInfo()->GetSpillsOffsetOrigin(), sf.GetDst());
    auto offset = memRef.GetDisp();
    // register -> stack
    auto srcReg = codegen_->ConvertRegister(sf.SrcValue(), sf.GetCommonType());
    // If address is no qword aligned and current group consist of even number of consecutive slots
    // then we can skip current operation.
    constexpr int COALESCE_OPS_LIMIT = 2;
    auto skipCoalescing = (consecutiveOpsHint % COALESCE_OPS_LIMIT == 1) && (offset % QUAD_WORD_SIZE_BYTES != 0);
    if (next != nullptr && CanCombineSpillFills(sf, *next, graph_) && !skipCoalescing) {
        auto nextReg = codegen_->ConvertRegister(next->SrcValue(), next->GetCommonType());
        encoder_->EncodeStp(srcReg, nextReg, memRef);
        return 2U;
    }
    encoder_->EncodeStr(srcReg, memRef);
    return 1U;
}

size_t SpillFillEncoder::EncodeStackToX(const SpillFillData &sf, const SpillFillData *next, int consecutiveOpsHint)
{
    auto offsetOrigin = codegen_->GetFrameInfo()->GetSpillsOffsetOrigin();
    auto srcMem = codegen_->GetMemRefForSlot(offsetOrigin, sf.GetSrc());
    auto offset = srcMem.GetDisp();
    auto typeInfo = Codegen::ConvertDataType(sf.GetType(), codegen_->GetArch());

    if (sf.GetDst().IsAnyRegister()) {  // stack -> register
        // If address is no qword aligned and current group consist of even number of consecutive slots
        // then we can skip current operation.
        constexpr int COALESCE_OPS_LIMIT = 2;
        auto skipCoalescing = (consecutiveOpsHint % COALESCE_OPS_LIMIT == 1) && (offset % QUAD_WORD_SIZE_BYTES != 0);
        if (next != nullptr && CanCombineSpillFills(sf, *next, graph_) && !skipCoalescing) {
            auto curReg = codegen_->ConvertRegister(sf.DstValue(), sf.GetCommonType());
            auto nextReg = codegen_->ConvertRegister(next->DstValue(), next->GetCommonType());
            encoder_->EncodeLdp(curReg, nextReg, false, srcMem);
            return 2U;
        }
        auto dstReg = GetDstReg(sf.GetDst(), typeInfo);
        encoder_->EncodeLdr(dstReg, false, srcMem);
        return 1U;
    }

    // stack -> stack
    ASSERT(sf.GetDst().IsAnyStack());
    auto dstMem = codegen_->GetMemRefForSlot(offsetOrigin, sf.GetDst());
    encoder_->EncodeMemCopy(srcMem, dstMem, DOUBLE_WORD_SIZE);  // Stack slot is 64-bit wide
    return 1U;
}
}  // namespace ark::compiler
