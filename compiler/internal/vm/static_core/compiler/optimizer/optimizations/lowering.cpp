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

#include <array>
#include "optimizer/ir/analysis.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/analysis/alias_analysis.h"
#include "optimizer/analysis/bounds_analysis.h"
#include "lowering.h"
#include "optimizer/code_generator/encode.h"

namespace ark::compiler {

void Lowering::VisitAdd([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    auto newInst = LowerBinaryOperationWithShiftedOperand<Opcode::Add>(inst);
    if (newInst == nullptr && LowerAddSub(inst) != nullptr) {
        return;
    }
    LowerMultiplyAddSub(newInst == nullptr ? inst : newInst);
}

void Lowering::VisitSub([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    auto newInst = LowerBinaryOperationWithShiftedOperand<Opcode::Sub, false>(inst);
    if (newInst == nullptr && LowerAddSub(inst) != nullptr) {
        return;
    }
    LowerMultiplyAddSub(inst);
}

void Lowering::VisitCastValueToAnyType([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    auto graph = inst->GetBasicBlock()->GetGraph();
    if (graph->IsBytecodeOptimizer() || graph->IsOsrMode()) {
        // Find way to enable it in OSR mode.
        return;
    }

    // from
    // 1.u64 Const N -> (v2)
    // 2.any CastValueToAnyType INT_TYPE v1 -> (...)
    //
    // to
    // 1.any Const Pack(N) -> (...)
    if (LowerCastValueToAnyTypeWithConst(inst)) {
        graph->GetEventWriter().EventLowering(GetOpcodeString(inst->GetOpcode()), inst->GetId(), inst->GetPc());
        COMPILER_LOG(DEBUG, LOWERING) << "Lowering is applied for " << GetOpcodeString(inst->GetOpcode());
        return;
    }
    auto anyType = inst->CastToCastValueToAnyType()->GetAnyType();
    auto baseType = AnyBaseTypeToDataType(anyType);
    // We can't propogate opject, because GC can move it
    if (baseType == DataType::REFERENCE) {
        return;
    }
    // from
    // 2.any CastValueToAnyType INT_TYPE v1 -> (v3)
    // 3     SaveState                   v2(acc)
    //
    // to
    // 3     SaveState                   v1(acc)
    auto input = inst->GetInput(0).GetInst();
    if (input->IsConst() && baseType == DataType::VOID) {
        input = graph->FindOrCreateConstant(DataType::Any(input->CastToConstant()->GetIntValue()));
    }
    for (auto it = inst->GetUsers().begin(); it != inst->GetUsers().end();) {
        auto userInst = it->GetInst();
        if (userInst->IsSaveState()) {
            userInst->SetInput(it->GetIndex(), input);
            it = inst->GetUsers().begin();
        } else {
            ++it;
        }
    }
}

void Lowering::VisitCast([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    // unsigned Load in AARCH64 zerod all high bits
    // from
    //  1.u8(u16, u32) Load ->(v2)
    //  2.u64(u32) Cast u8(u16, u32) -> (v3 ..)
    // to
    //  1.u8(u16) Load ->(v3, ..)
    auto graph = inst->GetBasicBlock()->GetGraph();
    if (graph->GetArch() != Arch::AARCH64) {
        return;
    }
    auto type = inst->GetType();
    if (DataType::IsTypeSigned(type)) {
        return;
    }
    auto inputType = inst->CastToCast()->GetOperandsType();
    if (DataType::IsTypeSigned(inputType) || DataType::Is64Bits(inputType, graph->GetArch())) {
        return;
    }
    auto inputInst = inst->GetInput(0).GetInst();
    if (!inputInst->IsLoad() || inputInst->GetType() != inputType) {
        return;
    }
    inst->ReplaceUsers(inputInst);
    inputInst->GetBasicBlock()->GetGraph()->GetEventWriter().EventLowering(GetOpcodeString(inst->GetOpcode()),
                                                                           inst->GetId(), inst->GetPc());
    COMPILER_LOG(DEBUG, LOWERING) << "Lowering is applied for " << GetOpcodeString(inst->GetOpcode());
}

template <Opcode OPC>
void Lowering::VisitBitwiseBinaryOperation([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    auto newInst = LowerBinaryOperationWithShiftedOperand<OPC>(inst);  // NOLINT(readability-magic-numbers)
    if (newInst == nullptr && LowerLogic(inst) != nullptr) {
        return;
    }
    LowerLogicWithInvertedOperand(newInst == nullptr ? inst : newInst);
}

void Lowering::VisitOr(GraphVisitor *v, Inst *inst)
{
    VisitBitwiseBinaryOperation<Opcode::Or>(v, inst);
}

void Lowering::VisitAnd(GraphVisitor *v, Inst *inst)
{
    if (static_cast<Lowering *>(v)->LowerAndAsBitfieldExtraction(inst) != nullptr) {
        return;
    }
    VisitBitwiseBinaryOperation<Opcode::And>(v, inst);
}

void Lowering::VisitXor(GraphVisitor *v, Inst *inst)
{
    VisitBitwiseBinaryOperation<Opcode::Xor>(v, inst);
}

void Lowering::VisitAndNot([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    LowerBinaryOperationWithShiftedOperand<Opcode::AndNot, false>(inst);
}

void Lowering::VisitXorNot([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    LowerBinaryOperationWithShiftedOperand<Opcode::XorNot, false>(inst);
}

void Lowering::VisitOrNot([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    LowerBinaryOperationWithShiftedOperand<Opcode::OrNot, false>(inst);
}

void Lowering::VisitSaveState([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::SaveState);
    LowerStateInst(inst->CastToSaveState());
}

void Lowering::VisitSafePoint([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::SafePoint);
    LowerStateInst(inst->CastToSafePoint());
}

void Lowering::VisitSaveStateOsr([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::SaveStateOsr);
    LowerStateInst(inst->CastToSaveStateOsr());
}

void Lowering::VisitSaveStateDeoptimize([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::SaveStateDeoptimize);
    LowerStateInst(inst->CastToSaveStateDeoptimize());
}

void Lowering::VisitBoundsCheck([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::BoundsCheck);
    LowerConstArrayIndex<BoundsCheckInstI>(inst, Opcode::BoundsCheckI);
}

void Lowering::VisitLoadArray([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::LoadArray);
    LowerConstArrayIndex<LoadInstI>(inst, Opcode::LoadArrayI);
}

void Lowering::VisitLoadCompressedStringChar([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::LoadCompressedStringChar);
    auto *enc = inst->GetBasicBlock()->GetGraph()->GetEncoder();
    if (enc->CanEncodeCompressedStringCharAtI()) {
        LowerConstArrayIndex<LoadCompressedStringCharInstI>(inst, Opcode::LoadCompressedStringCharI);
    }
}

void Lowering::VisitStoreArray([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::StoreArray);
    LowerConstArrayIndex<StoreInstI>(inst, Opcode::StoreArrayI);
}

void Lowering::VisitLoad([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::Load);
    LowerMemInstScale(inst);
}

void Lowering::VisitLoadNative([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::LoadNative);
    inst->SetOpcode(Opcode::Load);
    VisitLoad(v, inst);
}

void Lowering::VisitStore([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::Store);
    LowerMemInstScale(inst);
}

void Lowering::VisitStoreNative([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::StoreNative);
    inst->SetOpcode(Opcode::Store);
    VisitStore(v, inst);
}

void Lowering::VisitReturn([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::Return);
    LowerReturnInst(inst->CastToReturn());
}

void Lowering::VisitShr([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    LowerShift(inst);
}

void Lowering::VisitAShr([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    LowerShift(inst);
}

void Lowering::VisitShl([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    LowerShift(inst);
}

void Lowering::VisitIfImm([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::IfImm);
    static_cast<Lowering *>(v)->LowerIf(inst->CastToIfImm());
}

void Lowering::VisitMul([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    if (inst->GetInput(1).GetInst()->GetOpcode() != Opcode::Constant) {
        LowerNegateMultiply(inst);
    } else {
        LowerMulDivMod<Opcode::Mul>(inst);
    }
}

void Lowering::VisitDiv([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    if (TryReplaceDivPowerOfTwo(v, inst)) {
        return;
    }
    if (TryReplaceDivModNonPowerOfTwo(v, inst)) {
        return;
    }
    LowerMulDivMod<Opcode::Div>(inst);
}

void Lowering::ReplaceSignedDivPowerOfTwo([[maybe_unused]] GraphVisitor *v, Inst *inst, int64_t sValue)
{
    // 0.signed Parameter
    // 1.i64 Const 2^n -> {v2}
    // 2.signed DIV v0, v1 -> {v3}
    // 3.signed INST v2
    // ===>
    // 0.signed Parameter
    // 1.i64 Const n -> {v2}
    // 2.signed ASHR v0, type_size - 1 -> {v4} // 0 or -1
    // 4.signed SHR v2, type_size - n -> {v5} //  0 or 2^n - 1
    // 5.signed ADD v4, v0 -> {v6}
    // 6.signed ASHR v5, n -> {v3 or v7}
    // if n < 0 7.signed NEG v6 ->{v3}
    // 3.signed INST v6 or v7

    auto graph = inst->GetBasicBlock()->GetGraph();
    auto input0 = inst->GetInput(0).GetInst();
    int64_t n = GetPowerOfTwo(bit_cast<uint64_t>(helpers::math::AbsOrMin(sValue)));
    ASSERT(n != -1);

    auto typeSize = DataType::GetTypeSize(inst->GetType(), graph->GetArch());
    auto ashr =
        graph->CreateInstAShr(inst->GetType(), inst->GetPc(), input0, graph->FindOrCreateConstant(typeSize - 1));
    inst->InsertBefore(ashr);
    auto shr = graph->CreateInstShr(inst->GetType(), inst->GetPc(), ashr, graph->FindOrCreateConstant(typeSize - n));
    inst->InsertBefore(shr);
    auto add = graph->CreateInstAdd(inst->GetType(), inst->GetPc(), shr, input0);
    inst->InsertBefore(add);
    Inst *ashr2 = graph->CreateInstAShr(inst->GetType(), inst->GetPc(), add, graph->FindOrCreateConstant(n));

    auto result = ashr2;
    if (sValue < 0) {
        inst->InsertBefore(ashr2);
        result = graph->CreateInstNeg(inst->GetType(), inst->GetPc(), ashr2);
    }

    InsertInstruction(inst, result);

    LowerShift(ashr);
    LowerShift(shr);
    LowerShift(ashr2);
}

void Lowering::ReplaceUnsignedDivPowerOfTwo([[maybe_unused]] GraphVisitor *v, Inst *inst, uint64_t uValue)
{
    // 0.unsigned Parameter
    // 1.i64 Const 2^n -> {v2}
    // 2.un-signed DIV v0, v1 -> {v3}
    // 3.unsigned INST v2
    // ===>
    // 0.unsigned Parameter
    // 1.i64 Const n -> {v2}
    // 2.un-signed SHR v0, v1 -> {v3}
    // 3.unsigned INST v2

    auto graph = inst->GetBasicBlock()->GetGraph();
    auto input0 = inst->GetInput(0).GetInst();

    int64_t n = GetPowerOfTwo(uValue);
    ASSERT(n != -1);
    auto power = graph->FindOrCreateConstant(n);
    auto shrInst = graph->CreateInstShr(inst->GetType(), inst->GetPc(), input0, power);
    InsertInstruction(inst, shrInst);

    LowerShift(shrInst);
}

bool Lowering::SatisfyReplaceDivMovConditions(Inst *inst)
{
    if (inst->GetBasicBlock()->GetGraph()->IsBytecodeOptimizer()) {
        return false;
    }
    if (DataType::IsFloatType(inst->GetType())) {
        return false;
    }
    auto c = inst->GetInput(1).GetInst();
    return c->IsConst();
}

bool Lowering::TryReplaceDivPowerOfTwo([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    if (!SatisfyReplaceDivMovConditions(inst)) {
        return false;
    }

    uint64_t uValue = inst->GetInput(1).GetInst()->CastToConstant()->GetInt64Value();
    auto sValue = bit_cast<int64_t>(uValue);

    auto input0 = inst->GetInput(0).GetInst();
    bool isSigned = DataType::IsTypeSigned(input0->GetType());
    if ((isSigned && !helpers::math::IsPowerOfTwo(sValue)) || (!isSigned && !helpers::math::IsPowerOfTwo(uValue))) {
        return false;
    }

    if (isSigned) {
        ReplaceSignedDivPowerOfTwo(v, inst, sValue);
    } else {
        ReplaceUnsignedDivPowerOfTwo(v, inst, uValue);
    }
    return true;
}

bool Lowering::TryReplaceDivModNonPowerOfTwo([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    if (!SatisfyReplaceDivMovConditions(inst)) {
        return false;
    }

    auto *graph = inst->GetBasicBlock()->GetGraph();
    uint64_t uValue = inst->GetInput(1).GetInst()->CastToConstant()->GetInt64Value();

    auto input0 = inst->GetInput(0).GetInst();
    bool isSigned = DataType::IsTypeSigned(input0->GetType());
    auto encoder = graph->GetEncoder();
    ASSERT(encoder != nullptr);
    if (!encoder->CanOptimizeImmDivMod(uValue, isSigned)) {
        return false;
    }

    if (inst->GetOpcode() == Opcode::Div) {
        auto divImmInst = graph->CreateInstDivI(inst->GetType(), inst->GetPc(), input0, uValue);
        InsertInstruction(inst, divImmInst);
    } else {
        ASSERT(inst->GetOpcode() == Opcode::Mod);
        auto modImmInst = graph->CreateInstModI(inst->GetType(), inst->GetPc(), input0, uValue);
        InsertInstruction(inst, modImmInst);
    }
    return true;
}

bool Lowering::TryReplaceModPowerOfTwo([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    if (!SatisfyReplaceDivMovConditions(inst)) {
        return false;
    }

    uint64_t uValue = inst->GetInput(1).GetInst()->CastToConstant()->GetInt64Value();
    auto sValue = bit_cast<int64_t>(uValue);

    auto input0 = inst->GetInput(0).GetInst();
    bool isSigned = DataType::IsTypeSigned(input0->GetType());
    if ((isSigned && !helpers::math::IsPowerOfTwo(sValue)) || (!isSigned && !helpers::math::IsPowerOfTwo(uValue))) {
        return false;
    }

    if (isSigned) {
        int64_t absValue = helpers::math::AbsOrMin(sValue);
        ReplaceSignedModPowerOfTwo(v, inst, bit_cast<uint64_t>(absValue));
    } else {
        ReplaceUnsignedModPowerOfTwo(v, inst, uValue);
    }
    return true;
}

void Lowering::ReplaceSignedModPowerOfTwo([[maybe_unused]] GraphVisitor *v, Inst *inst, uint64_t absValue)
{
    // It is optimal for AARCH64, not for AMD64. But even for AMD64 significantly better than original Mod.
    // 1. ...
    // 2. Const 0x4
    // 3. Mod v1, v2
    // ====>
    // 1. ...
    // 4. Const 0x3
    // 7. Const 0xFFFFFFFFFFFFFFFC
    // 5. Add v1, v4
    // 6. SelectImm v5, v1, v1, 0, CC_LT
    // 8. And v6, v7
    // 9. Sub v1, v8
    auto graph = inst->GetBasicBlock()->GetGraph();
    auto input0 = inst->GetInput(0).GetInst();
    auto valueMinus1 = absValue - 1;
    uint32_t size = (inst->GetType() == DataType::UINT64 || inst->GetType() == DataType::INT64) ? WORD_SIZE : HALF_SIZE;
    Inst *addInst;
    if (graph->GetEncoder()->CanEncodeImmAddSubCmp(valueMinus1, size, false)) {
        addInst = graph->CreateInstAddI(inst, input0, valueMinus1);
    } else {
        auto valueMinus1Cnst = graph->FindOrCreateConstant(valueMinus1);
        addInst = graph->CreateInstAdd(inst, input0, valueMinus1Cnst);
    }
    Inst *selectInst;
    auto encoder = graph->GetEncoder();
    ASSERT(encoder != nullptr);
    if (encoder->CanEncodeImmAddSubCmp(0, size, true)) {
        selectInst = graph->CreateInstSelectImm(inst, std::array<Inst *, 3U> {addInst, input0, input0}, 0,
                                                inst->GetType(), ConditionCode::CC_LT);
    } else {
        auto zeroCnst = graph->FindOrCreateConstant(0);
        selectInst = graph->CreateInstSelect(inst, std::array<Inst *, 4U> {addInst, input0, input0, zeroCnst},
                                             inst->GetType(), ConditionCode::CC_LT);
    }
    auto maskValue = ~static_cast<uint64_t>(valueMinus1);
    Inst *andInst;
    ASSERT(encoder != nullptr);
    if (encoder->CanEncodeImmLogical(maskValue, size)) {
        andInst = graph->CreateInstAndI(inst, selectInst, maskValue);
    } else {
        auto mask = graph->FindOrCreateConstant(maskValue);
        andInst = graph->CreateInstAnd(inst, selectInst, mask);
    }
    auto subInst = graph->CreateInstSub(inst, input0, andInst);

    inst->InsertBefore(addInst);
    inst->InsertBefore(selectInst);
    inst->InsertBefore(andInst);
    InsertInstruction(inst, subInst);
}

void Lowering::ReplaceUnsignedModPowerOfTwo([[maybe_unused]] GraphVisitor *v, Inst *inst, uint64_t absValue)
{
    auto graph = inst->GetBasicBlock()->GetGraph();
    auto valueMinus1 = absValue - 1;
    uint32_t size = (inst->GetType() == DataType::UINT64 || inst->GetType() == DataType::INT64) ? WORD_SIZE : HALF_SIZE;
    Inst *andInst;
    auto encoder = graph->GetEncoder();
    ASSERT(encoder != nullptr);
    if (encoder->CanEncodeImmLogical(valueMinus1, size)) {
        andInst = graph->CreateInstAndI(inst, inst->GetInput(0).GetInst(), valueMinus1);
    } else {
        auto valueMinus1Cnst = graph->FindOrCreateConstant(valueMinus1);
        andInst = graph->CreateInstAnd(inst, inst->GetInput(0).GetInst(), valueMinus1Cnst);
    }
    InsertInstruction(inst, andInst);
}

// Base rationale: for x = a % c and y = (a + b) % c, if b <= c, we can use z = (x + b > c) ? (x + b - c) : (x + b)
// replacing y.
bool Lowering::TryReplaceModIfModInputIsAdd([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    // It is optimized for repeated mods in loop.
    // 1.i32  Phi
    // 2.i32  Mod v1, 0x5
    // 3.i32  Add v1, 0x1
    // 4.i32  Mod v3, 0x5
    // 5.i32  Add v1, 0x2
    // 6.i32  Mod v5, 0x5
    // ====>
    // 1.i32  Phi
    // 2.i32  Mod v1, 0x5
    // 3.i32  Add v2, 0x1
    // 4.i32  Sub v3, 0x5
    // 5.i32  SelectImm v4, v3, v3, 0x5, CC_GE
    // 6.i32  Add v2, 0x2
    // 7.i32  Sub v6, 0x5
    // 8.i32  SelectImm v7, v6, v6, 0x5, CC_GE

    if (!SatisfyReplaceDivMovConditions(inst)) {
        return false;
    }

    auto add = inst->GetInput(0).GetInst();
    if (add->GetOpcode() != Opcode::AddI) {
        return false;
    }
    // not support non-positive mod number
    auto modVal = static_cast<ConstantInst *>(inst->GetInput(1).GetInst())->GetIntValue();
    if (static_cast<int64_t>(modVal) < static_cast<int64_t>(1)) {
        return false;
    }

    auto addInput0 = add->GetInput(0).GetInst();
    auto baseMod = IdentifyBaseModIfModInputIsAdd(addInput0, inst, modVal);
    if (baseMod == nullptr) {
        return false;
    }

    auto addImm = add->CastToAddI()->GetImm();
    // our optimization principle is that the sum of the two operands in the addition must be less than twice the modVal
    if (addImm > modVal) {
        return false;
    }
    if (addImm == modVal) {
        ReplaceInstruction(inst, baseMod);
        return true;
    }
    auto graph = inst->GetBasicBlock()->GetGraph();
    auto newAddInst = graph->CreateInstAddI(inst, baseMod, addImm);
    auto newSubInst = graph->CreateInstSubI(inst, newAddInst, modVal);
    auto selectInst = graph->CreateInstSelectImm(inst, std::array<Inst *, 3U> {newSubInst, newAddInst, newAddInst},
                                                 modVal, inst->GetType(), ConditionCode::CC_GE);
    inst->InsertBefore(newAddInst);
    inst->InsertBefore(newSubInst);
    InsertInstruction(inst, selectInst);
    return true;
}

Inst *Lowering::IdentifyBaseModIfModInputIsAdd(Inst *inst, const Inst *modInst, uint64_t modVal)
{
    for (auto &user : inst->GetUsers()) {
        auto userInst = user.GetInst();
        auto opcode = userInst->GetOpcode();
        if (opcode == Opcode::ModI) {
            auto imm = userInst->CastToModI()->GetImm();
            if (imm == modVal && userInst->IsDominate(modInst)) {
                return userInst;
            }
        }
    }
    return nullptr;
}

void Lowering::VisitMod([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    if (TryReplaceModPowerOfTwo(v, inst)) {
        return;
    }
    if (TryReplaceModIfModInputIsAdd(v, inst)) {
        return;
    }
    if (TryReplaceDivModNonPowerOfTwo(v, inst)) {
        return;
    }
    LowerMulDivMod<Opcode::Mod>(inst);
}

void Lowering::VisitNeg([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    auto newInst = LowerNegateMultiply(inst);
    LowerUnaryOperationWithShiftedOperand<Opcode::Neg>(newInst == nullptr ? inst : newInst);
}

void Lowering::VisitDeoptimizeIf([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    LowerToDeoptimizeCompare(inst);
}

void Lowering::VisitLoadFromConstantPool([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    auto graph = inst->GetBasicBlock()->GetGraph();
    auto newInst = graph->CreateInstLoadArrayI(DataType::ANY, inst->GetPc(), inst->GetInput(0).GetInst(),
                                               inst->CastToLoadFromConstantPool()->GetTypeId());
#ifdef PANDA_COMPILER_DEBUG_INFO
    newInst->SetCurrentMethod(inst->GetCurrentMethod());
#endif
    inst->ReplaceUsers(newInst);
    inst->RemoveInputs();
    inst->GetBasicBlock()->ReplaceInst(inst, newInst);
    graph->GetEventWriter().EventLowering(GetOpcodeString(inst->GetOpcode()), inst->GetId(), inst->GetPc());
    COMPILER_LOG(DEBUG, LOWERING) << "Lowering is applied for " << GetOpcodeString(inst->GetOpcode());
}

// Replacing Compare EQ with Xor
// 1.i64 Const 0
// 2.b   ...
// 3.b   Compare EQ b v2, v1
// ===>
// 1.i64 Const 1
// 2.b   ...
// 3.i32 Xor v1, v2
void Lowering::VisitCompare(GraphVisitor *v, Inst *inst)
{
    auto input0 = inst->GetInput(0).GetInst();
    auto input1 = inst->GetInput(1).GetInst();

    if (inst->CastToCompare()->GetCc() != ConditionCode::CC_EQ) {
        return;
    }

    // Compare EQ b 0x0, v2
    if (input1->GetType() == DataType::BOOL && input0->IsConst() && input0->CastToConstant()->GetIntValue() == 0U) {
        std::swap(input0, input1);
    }

    // Compare EQ b v2, 0x0
    bool isApplicable =
        input0->GetType() == DataType::BOOL && input1->IsConst() && input1->CastToConstant()->GetIntValue() == 0U;
    if (!isApplicable) {
        return;
    }
    // Always there are more than one user of Compare, because previous pass is Cleanup
    bool onlyIfimm = true;
    for (auto &user : inst->GetUsers()) {
        if (user.GetInst()->GetOpcode() != Opcode::IfImm) {
            onlyIfimm = false;
            break;
        }
    }
    // Skip optimization, if all users is IfImm, optimization Compare+IfImm will be better
    if (onlyIfimm) {
        return;
    }
    auto graph = inst->GetBasicBlock()->GetGraph();
    auto cnst = graph->FindOrCreateConstant(1);
    auto xorInst = graph->CreateInstXor(DataType::BOOL, inst->GetPc(), input0, cnst);
#ifdef PANDA_COMPILER_DEBUG_INFO
    xorInst->SetCurrentMethod(inst->GetCurrentMethod());
#endif
    InsertInstruction(inst, xorInst);
    static_cast<Lowering *>(v)->VisitXor(v, xorInst);
}

template <size_t MAX_OPERANDS>
void Lowering::SetInputsAndInsertInstruction(OperandsCapture<MAX_OPERANDS> &operands, Inst *inst, Inst *newInst)
{
    for (size_t idx = 0; idx < MAX_OPERANDS; idx++) {
        newInst->SetInput(idx, operands.Get(idx));
    }
    InsertInstruction(inst, newInst);
}

void Lowering::LowerShift(Inst *inst)
{
    Opcode opc = inst->GetOpcode();
    ASSERT(opc == Opcode::Shr || opc == Opcode::AShr || opc == Opcode::Shl);
    auto pred = GetCheckInstAndGetConstInput(inst);
    if (pred == nullptr) {
        return;
    }
    ASSERT(pred->GetOpcode() == Opcode::Constant);
    uint64_t val = (static_cast<const ConstantInst *>(pred))->GetIntValue();
    DataType::Type type = inst->GetType();
    uint32_t size = (type == DataType::UINT64 || type == DataType::INT64) ? WORD_SIZE : HALF_SIZE;
    if (val >= size) {
        return;
    }

    auto graph = inst->GetBasicBlock()->GetGraph();
    auto encoder = graph->GetEncoder();
    ASSERT(encoder != nullptr);
    if (!encoder->CanEncodeShift(size)) {
        return;
    }

    if (TryLowerShrAsBitfieldExtraction(inst, val)) {
        return;
    }
    Inst *newInst;
    if (opc == Opcode::Shr) {
        newInst = graph->CreateInstShrI(inst, inst->GetInput(0).GetInst(), val);
    } else if (opc == Opcode::AShr) {
        newInst = graph->CreateInstAShrI(inst, inst->GetInput(0).GetInst(), val);
    } else {
        newInst = graph->CreateInstShlI(inst, inst->GetInput(0).GetInst(), val);
    }
    InsertInstruction(inst, newInst);
}

bool Lowering::TryLowerShrAsBitfieldExtraction(Inst *inst, uint64_t shrValue)
{
    Graph *graph = inst->GetBasicBlock()->GetGraph();
    if (graph->IsBytecodeOptimizer() || !graph->GetEncoder()->CanEncodeBitfieldExtractionFor(inst->GetType())) {
        return false;
    }
    if (inst->GetOpcode() != Opcode::Shr && inst->GetOpcode() != Opcode::AShr) {
        return false;
    }
    Inst *lhs = inst->GetInput(0).GetInst();
    bool isCase1 = lhs->GetOpcode() == Opcode::Shl || lhs->GetOpcode() == Opcode::ShlI;
    bool isCase2 = lhs->GetOpcode() == Opcode::And || lhs->GetOpcode() == Opcode::AndI;
    if (!isCase1 && !isCase2) {
        return false;
    }
    uint64_t val = 0;
    if (lhs->IsBinaryInst()) {  // Shl or And
        ConstantInst *lhsInput1 = GetCheckInstAndGetConstInput(lhs);
        if (lhsInput1 == nullptr) {
            return false;
        }
        val = lhsInput1->GetIntValue();
    } else if (lhs->IsBinaryImmInst()) {  // ShlI or AndI
        val = static_cast<BinaryImmOperation *>(lhs)->GetImm();
    } else {
        return false;
    }
    Inst *input = lhs->GetInput(0).GetInst();
    if (input->GetType() != inst->GetType()) {
        return false;
    }
    return (isCase1 && TryLowerShrAsBitfieldExtractionCase1(inst, input, val, shrValue)) ||
           (isCase2 && TryLowerShrAsBitfieldExtractionCase2(inst, input, val, shrValue));
}

// 0.i64 Constant (B - n1 - n2)
// 1.i64 Constant (B - n2)
// 2. ...
// 3.Type Shl v2, v0
// 4.Type Shr/AShr v3, v1
// ====>
// 0.i64 Constant (B - n1 - n2)
// 1.i64 Constant (B - n2)
// 2. ...
// 3.Type ExtractBitfield v2, n1, n2 where n1 = v1 - v0, n2 = B - v1
bool Lowering::TryLowerShrAsBitfieldExtractionCase1(Inst *inst, Inst *input, uint64_t shlValue, uint64_t shrValue)
{
    Graph *graph = inst->GetBasicBlock()->GetGraph();
    uint32_t size = DataType::GetTypeSize(inst->GetType(), graph->GetArch());

    [[maybe_unused]] Inst *lhs = inst->GetInput(0).GetInst();
    ASSERT(lhs->GetOpcode() == Opcode::Shl || lhs->GetOpcode() == Opcode::ShlI);
    ASSERT(inst->GetOpcode() == Opcode::Shr || inst->GetOpcode() == Opcode::AShr);
    ASSERT(inst->GetType() == input->GetType());
    ASSERT(shrValue < size);

    if (shlValue > shrValue) {
        return false;
    }
    uint64_t sourceBit = shrValue - shlValue;
    uint64_t width = size - shrValue;
    bool signExt = (inst->GetOpcode() == Opcode::AShr);
    InsertInstruction(inst, graph->CreateInstExtractBitfield(inst, input, sourceBit, width, signExt));
    return true;
}

// 0.i64 Constant M, where 2^n2 - 2^n1 <= M <= 2^n2 - 1
// 1.i64 Constant n1
// 2. ...
// 3.Type And v2, v0
// 4.Type Shr|AShr v3, v1
// ====>
// 0.i64 Constant M, where 2^n2 - 2^n1 <= M <= 2^n2 - 1
// 1.i64 Constant n1
// 2. ...
// 3.Type ExtractBitfield v2, n1, n2 - n1
bool Lowering::TryLowerShrAsBitfieldExtractionCase2(Inst *inst, Inst *input, uint64_t mask, uint64_t shrValue)
{
    Graph *graph = inst->GetBasicBlock()->GetGraph();
    uint32_t size = DataType::GetTypeSize(inst->GetType(), graph->GetArch());

    [[maybe_unused]] Inst *lhs = inst->GetInput(0).GetInst();
    ASSERT(lhs->GetOpcode() == Opcode::And || lhs->GetOpcode() == Opcode::AndI);
    ASSERT(inst->GetOpcode() == Opcode::Shr || inst->GetOpcode() == Opcode::AShr);
    ASSERT(inst->GetType() == input->GetType());
    ASSERT(shrValue < size);

    size_t maskSize = MinimumBitsToStore(mask);
    if (maskSize > size) {
        ASSERT(size <= HALF_SIZE);
        mask &= ((1ULL << size) - 1);
        maskSize = size;
    }
    // Checks whether 2^n2 - 2^n1 <= M <= 2^n2 - 1
    // Note: The expression (1ULL << maskSize) may be UB when maskSize == WORD_SIZE which is 64
    if (maskSize > shrValue && mask >= (maskSize < WORD_SIZE ? 1ULL << maskSize : 0) - (1ULL << shrValue)) {
        uint64_t sourceBit = shrValue;
        uint64_t width = maskSize - shrValue;
        bool signExt = (inst->GetOpcode() == Opcode::AShr) && (maskSize == size);
        InsertInstruction(inst, graph->CreateInstExtractBitfield(inst, input, sourceBit, width, signExt));
        return true;
    }
    return false;
}

constexpr Opcode Lowering::GetInstructionWithShiftedOperand(Opcode opcode)
{
    switch (opcode) {
        case Opcode::Add:
            return Opcode::AddSR;
        case Opcode::Sub:
            return Opcode::SubSR;
        case Opcode::And:
            return Opcode::AndSR;
        case Opcode::Or:
            return Opcode::OrSR;
        case Opcode::Xor:
            return Opcode::XorSR;
        case Opcode::AndNot:
            return Opcode::AndNotSR;
        case Opcode::OrNot:
            return Opcode::OrNotSR;
        case Opcode::XorNot:
            return Opcode::XorNotSR;
        case Opcode::Neg:
            return Opcode::NegSR;
        default:
            UNREACHABLE();
            return Opcode::INVALID;
    }
}

constexpr Opcode Lowering::GetInstructionWithInvertedOperand(Opcode opcode)
{
    switch (opcode) {
        case Opcode::And:
            return Opcode::AndNot;
        case Opcode::Or:
            return Opcode::OrNot;
        case Opcode::Xor:
            return Opcode::XorNot;
        default:
            return Opcode::INVALID;
    }
}

ShiftType Lowering::GetShiftTypeByOpcode(Opcode opcode)
{
    switch (opcode) {
        case Opcode::Shl:
        case Opcode::ShlI:
            return ShiftType::LSL;
        case Opcode::Shr:
        case Opcode::ShrI:
            return ShiftType::LSR;
        case Opcode::AShr:
        case Opcode::AShrI:
            return ShiftType::ASR;
        default:
            UNREACHABLE();
            return ShiftType::INVALID_SHIFT;
    }
}

ConstantInst *Lowering::GetCheckInstAndGetConstInput(Inst *inst)
{
    DataType::Type type = inst->GetType();
    if (type != DataType::INT64 && type != DataType::UINT64 && type != DataType::INT32 && type != DataType::UINT32 &&
        type != DataType::POINTER && type != DataType::BOOL) {
        return nullptr;
    }

    auto cnst = inst->GetInput(1).GetInst();
    if (!cnst->IsConst()) {
        if (!inst->IsCommutative() || !inst->GetInput(0).GetInst()->IsConst()) {
            return nullptr;
        }
        ASSERT(!DataType::IsFloatType(inst->GetType()));
        auto input = cnst;
        cnst = inst->GetInput(0).GetInst();
        inst->SetInput(0, input);
        inst->SetInput(1, cnst);
    }
    return cnst->CastToConstant();
}

ShiftOpcode Lowering::ConvertOpcode(Opcode newOpcode)
{
    switch (newOpcode) {
        case Opcode::NegSR:
            return ShiftOpcode::NEG_SR;
        case Opcode::AddSR:
            return ShiftOpcode::ADD_SR;
        case Opcode::SubSR:
            return ShiftOpcode::SUB_SR;
        case Opcode::AndSR:
            return ShiftOpcode::AND_SR;
        case Opcode::OrSR:
            return ShiftOpcode::OR_SR;
        case Opcode::XorSR:
            return ShiftOpcode::XOR_SR;
        case Opcode::AndNotSR:
            return ShiftOpcode::AND_NOT_SR;
        case Opcode::OrNotSR:
            return ShiftOpcode::OR_NOT_SR;
        case Opcode::XorNotSR:
            return ShiftOpcode::XOR_NOT_SR;
        default:
            return ShiftOpcode::INVALID_SR;
    }
}

// Ask encoder whether Constant can be an immediate for Compare
bool Lowering::ConstantFitsCompareImm(Inst *cst, uint32_t size, ConditionCode cc)
{
    ASSERT(cst->GetOpcode() == Opcode::Constant);
    if (DataType::IsFloatType(cst->GetType())) {
        return false;
    }
    auto *graph = cst->GetBasicBlock()->GetGraph();
    auto *encoder = graph->GetEncoder();
    int64_t val = cst->CastToConstant()->GetRawValue();
    if (graph->IsBytecodeOptimizer()) {
        return (size == HALF_SIZE) && (val == 0);
    }
    if (cc == ConditionCode::CC_TST_EQ || cc == ConditionCode::CC_TST_NE) {
        ASSERT(encoder != nullptr);
        return encoder->CanEncodeImmLogical(val, size);
    }
    return encoder->CanEncodeImmAddSubCmp(val, size, IsSignedConditionCode(cc));
}

Inst *Lowering::LowerAddSub(Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::Add || inst->GetOpcode() == Opcode::Sub);
    auto pred = GetCheckInstAndGetConstInput(inst);
    if (pred == nullptr) {
        return nullptr;
    }

    ASSERT(pred->GetOpcode() == Opcode::Constant);

    auto graph = pred->GetBasicBlock()->GetGraph();
    auto val = static_cast<int64_t>(pred->CastToConstant()->GetIntValue());
    DataType::Type type = inst->GetType();
    uint32_t size = (type == DataType::UINT64 || type == DataType::INT64) ? WORD_SIZE : HALF_SIZE;
    auto encoder = graph->GetEncoder();
    ASSERT(encoder != nullptr);
    if (!encoder->CanEncodeImmAddSubCmp(val, size, false)) {
        return nullptr;
    }

    bool isAdd = (inst->GetOpcode() == Opcode::Add);
    if (val < 0 && graph->GetEncoder()->CanEncodeImmAddSubCmp(-val, size, false)) {
        val = -val;
        isAdd = !isAdd;
    }

    Inst *newInst;
    if (isAdd) {
        newInst = graph->CreateInstAddI(inst, inst->GetInput(0).GetInst(), static_cast<uint64_t>(val));
    } else {
        newInst = graph->CreateInstSubI(inst, inst->GetInput(0).GetInst(), static_cast<uint64_t>(val));
    }
    InsertInstruction(inst, newInst);
    return newInst;
}

template <Opcode OPCODE>
void Lowering::LowerMulDivMod(Inst *inst)
{
    ASSERT(inst->GetOpcode() == OPCODE);
    auto graph = inst->GetBasicBlock()->GetGraph();
    if (graph->IsInstThrowable(inst)) {
        return;
    }

    auto pred = GetCheckInstAndGetConstInput(inst);
    if (pred == nullptr) {
        return;
    }

    auto val = static_cast<int64_t>(pred->CastToConstant()->GetIntValue());
    DataType::Type type = inst->GetType();
    uint32_t size = (type == DataType::UINT64 || type == DataType::INT64) ? WORD_SIZE : HALF_SIZE;
    auto encoder = graph->GetEncoder();
    ASSERT(encoder != nullptr);
    if (!encoder->CanEncodeImmMulDivMod(val, size)) {
        return;
    }

    Inst *newInst;
    // NOLINTNEXTLINE(readability-magic-numbers,readability-braces-around-statements, bugprone-branch-clone)
    if constexpr (OPCODE == Opcode::Mul) {
        newInst = graph->CreateInstMulI(inst, inst->GetInput(0).GetInst(), static_cast<uint64_t>(val));
        // NOLINTNEXTLINE(readability-misleading-indentation,readability-braces-around-statements)
    } else if constexpr (OPCODE == Opcode::Div) {
        newInst = graph->CreateInstDivI(inst, inst->GetInput(0).GetInst(), static_cast<uint64_t>(val));
        if (graph->IsBytecodeOptimizer()) {
            inst->ClearFlag(compiler::inst_flags::NO_DCE);  // In Bytecode Optimizer Div may have NO_DCE flag
            if (val == 0) {
                newInst->SetFlag(compiler::inst_flags::NO_DCE);
            }
        }
        // NOLINTNEXTLINE(readability-misleading-indentation)
    } else {
        newInst = graph->CreateInstModI(inst, inst->GetInput(0).GetInst(), static_cast<uint64_t>(val));
        if (graph->IsBytecodeOptimizer()) {
            inst->ClearFlag(compiler::inst_flags::NO_DCE);  // In Bytecode Optimizer Div may have NO_DCE flag
            if (val == 0) {
                newInst->SetFlag(compiler::inst_flags::NO_DCE);
            }
        }
    }
    InsertInstruction(inst, newInst);
}

Inst *Lowering::LowerMultiplyAddSub(Inst *inst)
{
    // Don't use MAdd/MSub for floating point inputs to avoid different results for interpreted and
    // compiled code due to better precision of target instructions implementing MAdd/MSub.
    if (DataType::GetCommonType(inst->GetType()) != DataType::INT64) {
        return nullptr;
    }

    OperandsCapture<3U> operands {};
    InstructionsCapture<2U> insts {};
    InstructionsCapture<3U> instsSub3 {};
    bool isSub = true;

    // clang-format off
    using MAddMatcher = ADD<MUL<SRC0, SRC1, Flags::S>, SRC2>;
    using MSubMatcher2Ops =
        AnyOf<SUB<SRC2, MUL<SRC0, SRC1, Flags::S>>,
              ADD<BinaryOp<Opcode::MNeg, SRC0, SRC1, Flags::S>, SRC2>>;
    using MSubMatcher3Ops =
        AnyOf<ADD<MUL<NEG<SRC0, Flags::S>, SRC1, Flags::S>, SRC2>,
              ADD<NEG<MUL<SRC0, SRC1, Flags::S>, Flags::S>, SRC2>>;
    // clang-format on

    if (MSubMatcher2Ops::Capture(inst, operands, insts)) {
        // Operands may have different types (but the same common type), but instructions
        // having different types can not be fused, because it will change semantics.
        if (!insts.HaveSameType()) {
            return nullptr;
        }
    } else if (MSubMatcher3Ops::Capture(inst, operands, instsSub3)) {
        if (!instsSub3.HaveSameType()) {
            return nullptr;
        }
    } else if (MAddMatcher::Capture(inst, operands, insts.ResetIndex())) {
        isSub = false;
        if (!insts.HaveSameType()) {
            return nullptr;
        }
    } else {
        return nullptr;
    }

    auto graph = inst->GetBasicBlock()->GetGraph();
    auto encoder = graph->GetEncoder();
    ASSERT(encoder != nullptr);
    if ((isSub && !encoder->CanEncodeMSub()) || (!isSub && !encoder->CanEncodeMAdd())) {
        return nullptr;
    }

    Inst *newInst = isSub ? graph->CreateInstMSub(inst) : graph->CreateInstMAdd(inst);
    SetInputsAndInsertInstruction(operands, inst, newInst);
    return newInst;
}

Inst *Lowering::LowerNegateMultiply(Inst *inst)
{
    OperandsCapture<2U> operands {};
    InstructionsCapture<2U> insts {};
    using MNegMatcher = AnyOf<NEG<MUL<SRC0, SRC1, Flags::S>>, MUL<NEG<SRC0, Flags::S>, SRC1>>;
    if (!MNegMatcher::Capture(inst, operands, insts) || !operands.HaveCommonType() || !insts.HaveSameType()) {
        return nullptr;
    }

    auto graph = inst->GetBasicBlock()->GetGraph();
    auto encoder = graph->GetEncoder();
    ASSERT(encoder != nullptr);
    if (!encoder->CanEncodeMNeg()) {
        return nullptr;
    }

    Inst *newInst = graph->CreateInstMNeg(inst);
    SetInputsAndInsertInstruction(operands, inst, newInst);
    return newInst;
}

bool Lowering::LowerCastValueToAnyTypeWithConst(Inst *inst)
{
    auto graph = inst->GetBasicBlock()->GetGraph();
    auto anyType = inst->CastToCastValueToAnyType()->GetAnyType();
    auto baseType = AnyBaseTypeToDataType(anyType);
    if (!IsTypeNumeric(baseType) || baseType == DataType::POINTER) {
        return false;
    }
    auto inputInst = inst->GetInput(0).GetInst();
    if (!inputInst->IsConst()) {
        return false;
    }
    auto imm = inputInst->CastToConstant()->GetRawValue();
    auto packImm = graph->GetRuntime()->GetPackConstantByPrimitiveType(anyType, imm);
    auto anyConst = inst->GetBasicBlock()->GetGraph()->FindOrCreateConstant(DataType::Any(packImm));
    inst->ReplaceUsers(anyConst);
    return true;
}

void Lowering::LowerLogicWithInvertedOperand(Inst *inst)
{
    OperandsCapture<2U> operands {};
    InstructionsCapture<2U> insts {};
    using Matcher = AnyOf<BinaryOp<Opcode::Or, SRC0, UnaryOp<Opcode::Not, SRC1, Flags::S>, Flags::C>,
                          BinaryOp<Opcode::And, SRC0, UnaryOp<Opcode::Not, SRC1, Flags::S>, Flags::C>,
                          BinaryOp<Opcode::Xor, SRC0, UnaryOp<Opcode::Not, SRC1, Flags::S>, Flags::C>>;
    if (!Matcher::Capture(inst, operands, insts) || !insts.HaveSameType()) {
        return;
    }

    auto graph = inst->GetBasicBlock()->GetGraph();
    auto encoder = graph->GetEncoder();
    auto opcode = inst->GetOpcode();
    Inst *newInst;
    if (opcode == Opcode::Or) {
        ASSERT(encoder != nullptr);
        if (!encoder->CanEncodeOrNot()) {
            return;
        }
        newInst = graph->CreateInstOrNot(inst);
    } else if (opcode == Opcode::And) {
        if (!encoder->CanEncodeAndNot()) {
            return;
        }
        newInst = graph->CreateInstAndNot(inst);
    } else {
        if (!encoder->CanEncodeXorNot()) {
            return;
        }
        newInst = graph->CreateInstXorNot(inst);
    }

    SetInputsAndInsertInstruction(operands, inst, newInst);
}

template <typename T, size_t MAX_OPERANDS>
Inst *Lowering::LowerOperationWithShiftedOperand(Inst *inst, OperandsCapture<MAX_OPERANDS> &operands, Inst *shiftInst,
                                                 Opcode newOpcode)
{
    auto graph = inst->GetBasicBlock()->GetGraph();
    auto encoder = graph->GetEncoder();

    ShiftType shiftType = GetShiftTypeByOpcode(shiftInst->GetOpcode());
    if (!encoder->CanEncodeShiftedOperand(ConvertOpcode(newOpcode), shiftType)) {
        return nullptr;
    }
    uint64_t imm = static_cast<BinaryImmOperation *>(shiftInst)->GetImm();
    auto newInst = static_cast<T *>(graph->CreateInst(newOpcode));
    newInst->SetType(inst->GetType());
    newInst->SetPc(inst->GetPc());
    newInst->SetImm(imm);
    newInst->SetShiftType(shiftType);
#ifdef PANDA_COMPILER_DEBUG_INFO
    newInst->SetCurrentMethod(inst->GetCurrentMethod());
#endif
    SetInputsAndInsertInstruction(operands, inst, newInst);
    return newInst;
}

template <Opcode OPCODE, bool IS_COMMUTATIVE>
Inst *Lowering::LowerBinaryOperationWithShiftedOperand(Inst *inst)
{
    OperandsCapture<2U> operands {};
    InstructionsCapture<2U> insts {};
    InstructionsCapture<3U> invInsts {};
    constexpr auto FLAGS = IS_COMMUTATIVE ? Flags::COMMUTATIVE : Flags::NONE;

    // We're expecting that at this point all "shift by immediate" patterns were replaced with ShlI/ShrI/AShrI
    // clang-format off
    using Matcher = AnyOf<BinaryOp<OPCODE, SRC0, SHLI<SRC1>, FLAGS>,
                          BinaryOp<OPCODE, SRC0, SHRI<SRC1>, FLAGS>,
                          BinaryOp<OPCODE, SRC0, ASHRI<SRC1>, FLAGS>>;
    // Instead of replacing instruction having inverted operand with single inverted-operand instruction
    // and then applying the rules defined above we're applying explicitly defined rules for such patterns,
    // because after inverted-operand instruction insertion there will be several users for shift operation.
    // BinaryOp won't match the IR-tree with a pattern and either more complicated checks should be introduced there
    // or DCE pass followed by additional Lowering pass should be performed.
    // To keep things simple and avoid extra Lowering passes explicit rules were added.
    using InvertedOperandMatcher = MatchIf<GetInstructionWithInvertedOperand(OPCODE) != Opcode::INVALID,
                                         AnyOf<BinaryOp<OPCODE, SRC0, NOT<SHLI<SRC1>>>,
                                         BinaryOp<OPCODE, SRC0, NOT<SHRI<SRC1>>>,
                                         BinaryOp<OPCODE, SRC0, NOT<ASHRI<SRC1>>>>>;
    // clang-format on

    if (GetCommonType(inst->GetType()) != DataType::INT64) {
        return nullptr;
    }

    Inst *shiftInst;
    Opcode newOpc;

    if (InvertedOperandMatcher::Capture(inst, operands, invInsts) && invInsts.HaveSameType()) {
        auto rightOperand =
            operands.Get(0) == inst->GetInput(0).GetInst() ? inst->GetInput(1).GetInst() : inst->GetInput(0).GetInst();
        shiftInst = rightOperand->GetInput(0).GetInst();
        newOpc = GetInstructionWithShiftedOperand(GetInstructionWithInvertedOperand(OPCODE));
    } else if (Matcher::Capture(inst, operands, insts) && insts.HaveSameType()) {
        shiftInst =
            operands.Get(0) == inst->GetInput(0).GetInst() ? inst->GetInput(1).GetInst() : inst->GetInput(0).GetInst();
        newOpc = GetInstructionWithShiftedOperand(OPCODE);
    } else {
        return nullptr;
    }

    return LowerOperationWithShiftedOperand<BinaryShiftedRegisterOperation>(inst, operands, shiftInst, newOpc);
}

template <Opcode OPCODE>
void Lowering::LowerUnaryOperationWithShiftedOperand(Inst *inst)
{
    OperandsCapture<1> operands {};
    InstructionsCapture<2U> insts {};
    // We're expecting that at this point all "shift by immediate" patterns were replaced with ShlI/ShrI/AShrI
    // clang-format off
    using Matcher = AnyOf<UnaryOp<OPCODE, SHLI<SRC0>>,
                          UnaryOp<OPCODE, SHRI<SRC0>>,
                          UnaryOp<OPCODE, ASHRI<SRC0>>>;
    // clang-format on
    if (!Matcher::Capture(inst, operands, insts) || GetCommonType(inst->GetType()) != DataType::INT64 ||
        !insts.HaveSameType()) {
        return;
    }
    LowerOperationWithShiftedOperand<UnaryShiftedRegisterOperation>(inst, operands, inst->GetInput(0).GetInst(),
                                                                    GetInstructionWithShiftedOperand(OPCODE));
}

Inst *Lowering::LowerLogic(Inst *inst)
{
    Opcode opc = inst->GetOpcode();
    ASSERT(opc == Opcode::Or || opc == Opcode::And || opc == Opcode::Xor);
    auto pred = GetCheckInstAndGetConstInput(inst);
    if (pred == nullptr) {
        return nullptr;
    }
    ASSERT(pred->GetOpcode() == Opcode::Constant);
    uint64_t val = pred->CastToConstant()->GetIntValue();
    DataType::Type type = inst->GetType();
    uint32_t size = (type == DataType::UINT64 || type == DataType::INT64) ? WORD_SIZE : HALF_SIZE;
    auto graph = inst->GetBasicBlock()->GetGraph();
    auto encoder = graph->GetEncoder();
    ASSERT(encoder != nullptr);
    if (!encoder->CanEncodeImmLogical(val, size)) {
        return nullptr;
    }
    Inst *newInst;
    if (opc == Opcode::Or) {
        newInst = graph->CreateInstOrI(inst, inst->GetInput(0).GetInst(), val);
    } else if (opc == Opcode::And) {
        newInst = graph->CreateInstAndI(inst, inst->GetInput(0).GetInst(), val);
    } else {
        newInst = graph->CreateInstXorI(inst, inst->GetInput(0).GetInst(), val);
    }
    InsertInstruction(inst, newInst);
    return newInst;
}

// 0.i64 Constant n1
// 1.i64 Constant M = 2^n2 - 1
// 2. ...
// 3.Type Shr v2, v0
// 4.Type And v3, v1
// ====>
// 0.i64 Constant n1
// 1.i64 Constant M = 2^n2 - 1
// 2. ...
// 3.Type ExtractBitfield v2, n1, n2 [ZeroExtension]
Inst *Lowering::LowerAndAsBitfieldExtraction(Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::And);
    Graph *graph = inst->GetBasicBlock()->GetGraph();
    if (graph->IsBytecodeOptimizer() || !graph->GetEncoder()->CanEncodeBitfieldExtractionFor(inst->GetType())) {
        return nullptr;
    }
    ConstantInst *pred = GetCheckInstAndGetConstInput(inst);
    if (pred == nullptr) {
        return nullptr;
    }
    uint32_t size = DataType::GetTypeSize(inst->GetType(), graph->GetArch());
    uint64_t andMask = pred->GetIntValue();
    uint64_t andMaskSize = MinimumBitsToStore(andMask);
    if (andMaskSize == 0 || andMaskSize >= size || static_cast<unsigned>(Popcount(andMask)) != andMaskSize) {
        return nullptr;
    }

    Inst *lhs = inst->GetInput(0).GetInst();
    uint64_t shrValue = std::numeric_limits<uint64_t>::max();
    if (lhs->GetOpcode() == Opcode::Shr || lhs->GetOpcode() == Opcode::AShr) {
        ConstantInst *c = GetCheckInstAndGetConstInput(lhs);
        if (c == nullptr) {
            return nullptr;
        }
        shrValue = c->GetIntValue();
    } else if (lhs->GetOpcode() == Opcode::ShrI || lhs->GetOpcode() == Opcode::AShrI) {
        shrValue = static_cast<BinaryImmOperation *>(lhs)->GetImm();
    }
    if (shrValue >= size || shrValue + andMaskSize > size) {
        return nullptr;
    }
    Inst *input = lhs->GetInput(0).GetInst();
    if (input->GetType() != inst->GetType()) {
        return nullptr;
    }
    // signExt=false, i.e. zero extension
    auto *newInst = graph->CreateInstExtractBitfield(inst, input, shrValue, andMaskSize, false);
    InsertInstruction(inst, newInst);
    return newInst;
}

// From
//  2.u64 ShlI v1, 0x3 -> (v3)
//  3.u64 Load v0, v2  -> (...)
// To
//  3.u64 Load v0, v2, Scale 0x3 -> (...)
void Lowering::LowerMemInstScale(Inst *inst)
{
    auto opcode = inst->GetOpcode();
    ASSERT(opcode == Opcode::Load || opcode == Opcode::Store);
    auto inputInst = inst->GetInput(1).GetInst();
    if (inputInst->GetOpcode() != Opcode::ShlI) {
        return;
    }
    auto graph = inst->GetBasicBlock()->GetGraph();
    auto inputType = inputInst->GetType();
    if (Is64BitsArch(graph->GetArch())) {
        if (inputType != DataType::UINT64 && inputType != DataType::INT64) {
            return;
        }
    } else {
        if (inputType != DataType::UINT32 && inputType != DataType::INT32) {
            return;
        }
    }
    auto type = inst->GetType();
    uint64_t val = inputInst->CastToShlI()->GetImm();
    uint32_t size = DataType::GetTypeSize(type, graph->GetArch());
    if (!graph->GetEncoder()->CanEncodeScale(val, size)) {
        return;
    }
    if (opcode == Opcode::Load) {
        ASSERT(inst->CastToLoad()->GetScale() == 0);
        inst->CastToLoad()->SetScale(val);
    } else {
        ASSERT(inst->CastToStore()->GetScale() == 0);
        inst->CastToStore()->SetScale(val);
    }
    inst->SetInput(1, inputInst->GetInput(0).GetInst());
    graph->GetEventWriter().EventLowering(GetOpcodeString(inst->GetOpcode()), inst->GetId(), inst->GetPc());
    COMPILER_LOG(DEBUG, LOWERING) << "Lowering is applied for " << GetOpcodeString(inst->GetOpcode());
}

template <typename LowLevelType>
void Lowering::LowerConstArrayIndex(Inst *inst, Opcode lowLevelOpcode)
{
    if (inst->GetBasicBlock()->GetGraph()->IsBytecodeOptimizer()) {
        return;
    }
    static constexpr size_t ARRAY_INDEX_INPUT = 1;
    auto inputInst = inst->GetInput(ARRAY_INDEX_INPUT).GetInst();
    ASSERT(inputInst->GetOpcode() != Opcode::BoundsCheckI);
    if (inputInst->IsConst()) {
        uint64_t value = inputInst->CastToConstant()->GetIntValue();

        auto graph = inst->GetBasicBlock()->GetGraph();
        auto newInst = graph->CreateInst(lowLevelOpcode);
        newInst->SetType(inst->GetType());
        newInst->SetPc(inst->GetPc());
#ifdef PANDA_COMPILER_DEBUG_INFO
        newInst->SetCurrentMethod(inst->GetCurrentMethod());
#endif
        static_cast<LowLevelType *>(newInst)->SetImm(value);

        // StoreInst and BoundsCheckInst have 3 inputs, LoadInst - has 2 inputs
        newInst->SetInput(0, inst->GetInput(0).GetInst());
        if (inst->GetInputsCount() == 3U) {
            newInst->SetInput(1, inst->GetInput(2U).GetInst());
        } else {
            ASSERT(inst->GetInputsCount() == 2U);
        }
        if (inst->GetOpcode() == Opcode::StoreArray) {
            newInst->CastToStoreArrayI()->SetNeedBarrier(inst->CastToStoreArray()->GetNeedBarrier());
        }

        if (inst->GetOpcode() == Opcode::LoadArray) {
            newInst->CastToLoadArrayI()->SetNeedBarrier(inst->CastToLoadArray()->GetNeedBarrier());
            newInst->CastToLoadArrayI()->SetIsArray(inst->CastToLoadArray()->IsArray());
        }
        if (inst->GetOpcode() == Opcode::BoundsCheck) {
            newInst->CastToBoundsCheckI()->SetIsArray(inst->CastToBoundsCheck()->IsArray());
            if (inst->CanDeoptimize()) {
                newInst->SetFlag(inst_flags::CAN_DEOPTIMIZE);
            }
        }

        // Replace instruction immediately because it's not removable by DCE
        if (inst->GetOpcode() != Opcode::BoundsCheck) {
            inst->ReplaceUsers(newInst);
        } else {
            auto cnst = graph->FindOrCreateConstant(value);
            inst->ReplaceUsers(cnst);
        }
        inst->RemoveInputs();
        inst->GetBasicBlock()->ReplaceInst(inst, newInst);
        graph->GetEventWriter().EventLowering(GetOpcodeString(inst->GetOpcode()), inst->GetId(), inst->GetPc());
        COMPILER_LOG(DEBUG, LOWERING) << "Lowering is applied for " << GetOpcodeString(inst->GetOpcode());
    }
}

void Lowering::LowerStateInst(SaveStateInst *saveState)
{
    size_t idx = 0;
    size_t inputsCount = saveState->GetInputsCount();
    auto graph = saveState->GetBasicBlock()->GetGraph();
    if (graph->IsBytecodeOptimizer()) {
        return;
    }
    bool skipFloats = (graph->GetArch() == Arch::AARCH32);
    while (idx < inputsCount) {
        auto inputInst = saveState->GetInput(idx).GetInst();
        // In Aarch32 floats values stores in different format then integer
        if (inputInst->GetOpcode() == Opcode::NullPtr ||
            (inputInst->IsConst() && (!skipFloats || inputInst->GetType() == DataType::INT64))) {
            uint64_t rawValue =
                inputInst->GetOpcode() == Opcode::NullPtr ? 0 : inputInst->CastToConstant()->GetRawValue();
            auto vreg = saveState->GetVirtualRegister(idx);
            auto type = inputInst->GetType();
            // There are no INT64 in dynamic
            if (type == DataType::INT64 && graph->IsDynamicMethod()) {
                type = DataType::INT32;
            }
            saveState->AppendImmediate(rawValue, vreg.Value(), type, vreg.GetVRegType());
            saveState->RemoveInput(idx);
            inputsCount--;
            graph->GetEventWriter().EventLowering(GetOpcodeString(saveState->GetOpcode()), saveState->GetId(),
                                                  saveState->GetPc());
            COMPILER_LOG(DEBUG, LOWERING) << "Lowering is applied for " << GetOpcodeString(saveState->GetOpcode());
        } else {
            idx++;
        }
    }
}

void Lowering::LowerReturnInst(FixedInputsInst1 *ret)
{
    auto graph = ret->GetBasicBlock()->GetGraph();
    if (graph->IsBytecodeOptimizer()) {
        return;
    }
    ASSERT(ret->GetOpcode() == Opcode::Return);
    auto inputInst = ret->GetInput(0).GetInst();
    if (inputInst->IsConst()) {
        uint64_t rawValue = inputInst->CastToConstant()->GetRawValue();
        auto retImm = graph->CreateInstReturnI(ret->GetType(), ret->GetPc(), rawValue);
#ifdef PANDA_COMPILER_DEBUG_INFO
        retImm->SetCurrentMethod(ret->GetCurrentMethod());
#endif

        // Replace instruction immediately because it's not removable by DCE
        ret->RemoveInputs();
        ret->GetBasicBlock()->ReplaceInst(ret, retImm);
        graph->GetEventWriter().EventLowering(GetOpcodeString(ret->GetOpcode()), ret->GetId(), ret->GetPc());
        COMPILER_LOG(DEBUG, LOWERING) << "Lowering is applied for " << GetOpcodeString(ret->GetOpcode());
    }
}

// We'd like to swap only to make second operand immediate
bool Lowering::BetterToSwapCompareInputs(Inst *cmp)
{
    ASSERT(cmp->GetOpcode() == Opcode::Compare);
    auto in0 = cmp->GetInput(0).GetInst();
    auto in1 = cmp->GetInput(1).GetInst();
    if (DataType::IsFloatType(in0->GetType())) {
        return false;
    }
    if (in0->GetOpcode() == compiler::Opcode::NullPtr) {
        return true;
    }

    if (in0->IsConst()) {
        if (in1->IsConst()) {
            DataType::Type type = cmp->CastToCompare()->GetOperandsType();
            uint32_t size = (type == DataType::UINT64 || type == DataType::INT64) ? WORD_SIZE : HALF_SIZE;
            auto cc = cmp->CastToCompare()->GetCc();
            return ConstantFitsCompareImm(in0, size, cc) && !ConstantFitsCompareImm(in1, size, cc);
        }
        return true;
    }
    return false;
}

// Optimize order of input arguments for decreasing using accumulator (Bytecodeoptimizer only).
void Lowering::OptimizeIfInput(compiler::Inst *ifInst)
{
    ASSERT(ifInst->GetOpcode() == compiler::Opcode::If);
    compiler::Inst *input0 = ifInst->GetInput(0).GetInst();
    compiler::Inst *input1 = ifInst->GetInput(1).GetInst();

    if (input0->IsDominate(input1)) {
        ifInst->SetInput(0, input1);
        ifInst->SetInput(1, input0);
        // And change CC
        auto cc = ifInst->CastToIf()->GetCc();
        cc = SwapOperandsConditionCode(cc);
        ifInst->CastToIf()->SetCc(cc);
    }
}

void Lowering::JoinFcmpInst(IfImmInst *inst, CmpInst *input)
{
    auto cc = inst->GetCc();
    ASSERT(cc == ConditionCode::CC_EQ || cc == ConditionCode::CC_NE || IsSignedConditionCode(cc));
    if (input->IsFcmpg()) {
        /* Please look at the table of vector condition flags:
         * LT => Less than, or unordered
         * LE => Less than or equal, or unordered
         * GT => Greater than
         * GE => Greater than or equal
         *
         * LO => Less than
         * LS => Less than or equal
         * HI => Greater than, or unordered
         * HS => Greater than or equal, or unordered
         *
         * So we change condition to "unsigned" for Fcmpg (which should return "greater than" for unordered
         * comparisons).
         */
        cc = InverseSignednessConditionCode(cc);
    }

    LowerIfImmToIf(inst, input, cc, input->GetOperandsType());
}

void Lowering::LowerIf(IfImmInst *inst)
{
    ASSERT(inst->GetCc() == ConditionCode::CC_NE || inst->GetCc() == ConditionCode::CC_EQ);
    ASSERT(inst->GetImm() == 0);
    if (inst->GetOperandsType() != DataType::BOOL) {
        ASSERT(GetGraph()->IsDynamicMethod());
        return;
    }
    auto input = inst->GetInput(0).GetInst();
    if (input->GetOpcode() != Opcode::Compare && input->GetOpcode() != Opcode::And) {
        return;
    }
    // Check, that inst have only IfImm user
    for (auto &user : input->GetUsers()) {
        if (user.GetInst()->GetOpcode() != Opcode::IfImm) {
            return;
        }
    }
    // Try put constant in second input
    if (input->GetOpcode() == Opcode::Compare && BetterToSwapCompareInputs(input)) {
        // Swap inputs
        auto in0 = input->GetInput(0).GetInst();
        auto in1 = input->GetInput(1).GetInst();
        input->SetInput(0, in1);
        input->SetInput(1, in0);
        // And change CC
        auto cc = input->CastToCompare()->GetCc();
        cc = SwapOperandsConditionCode(cc);
        input->CastToCompare()->SetCc(cc);
    }
    if (!GetGraph()->IsBytecodeOptimizer()) {
        for (auto &newInput : input->GetInputs()) {
            auto realNewInput = input->GetDataFlowInput(newInput.GetInst());
            if (realNewInput->IsMovableObject()) {
                ssb_.SearchAndCreateMissingObjInSaveState(GetGraph(), realNewInput, inst);
            }
        }
    }
    auto cst = input->GetInput(1).GetInst();
    DataType::Type type =
        (input->GetOpcode() == Opcode::Compare) ? input->CastToCompare()->GetOperandsType() : input->GetType();
    uint32_t size = (type == DataType::UINT64 || type == DataType::INT64) ? WORD_SIZE : HALF_SIZE;
    auto cc = input->GetOpcode() == Opcode::Compare ? input->CastToCompare()->GetCc() : ConditionCode::CC_TST_NE;
    // IfImm can be inverted
    if (inst->GetCc() == ConditionCode::CC_EQ && inst->GetImm() == 0) {
        cc = GetInverseConditionCode(cc);
    }
    if (cst->GetOpcode() == compiler::Opcode::NullPtr || (cst->IsConst() && ConstantFitsCompareImm(cst, size, cc))) {
        // In-place change for IfImm
        InPlaceLowerIfImm(inst, input, cst, cc, type);
    } else {
        LowerIfImmToIf(inst, input, cc, type);
    }
}

void Lowering::InPlaceLowerIfImm(IfImmInst *inst, Inst *input, Inst *cst, ConditionCode cc, DataType::Type inputType)
{
    auto graph = inst->GetBasicBlock()->GetGraph();
    inst->SetOperandsType(inputType);
    auto newInput = input->GetInput(0).GetInst();
    // For compare(nullptr, 0) set `nullptr` as new input
    if (cst->GetOpcode() == Opcode::NullPtr && IsZeroConstant(newInput) &&
        DataType::IsReference(inst->GetOperandsType())) {
        newInput = cst;
    }
    inst->SetInput(0, newInput);

    uint64_t val = cst->GetOpcode() == Opcode::NullPtr ? 0 : cst->CastToConstant()->GetRawValue();
    inst->SetImm(val);
    inst->SetCc(cc);
    inst->GetBasicBlock()->GetGraph()->GetEventWriter().EventLowering(GetOpcodeString(inst->GetOpcode()), inst->GetId(),
                                                                      inst->GetPc());
    COMPILER_LOG(DEBUG, LOWERING) << "Lowering is applied for " << GetOpcodeString(inst->GetOpcode());

    if (inst->GetImm() == 0 && newInput->GetOpcode() == Opcode::Cmp &&
        DataType::IsFloatType(newInput->CastToCmp()->GetOperandsType()) && !graph->IsBytecodeOptimizer()) {
        // Check inst and input are the only users of new_input
        bool join {true};
        for (auto &user : newInput->GetUsers()) {
            if (auto userInst = user.GetInst(); userInst != inst && userInst != input) {
                join = false;
                break;
            }
        }
        if (join) {
            JoinFcmpInst(inst, newInput->CastToCmp());
        }
    }
}

void Lowering::LowerIfImmToIf(IfImmInst *inst, Inst *input, ConditionCode cc, DataType::Type inputType)
{
    auto graph = inst->GetBasicBlock()->GetGraph();
    // New instruction
    auto replace = graph->CreateInstIf(DataType::NO_TYPE, inst->GetPc(), input->GetInput(0).GetInst(),
                                       input->GetInput(1).GetInst(), inputType, cc, inst->GetMethod());
#ifdef PANDA_COMPILER_DEBUG_INFO
    replace->SetCurrentMethod(inst->GetCurrentMethod());
#endif
    // Replace IfImm instruction immediately because it's not removable by DCE
    inst->RemoveInputs();
    inst->GetBasicBlock()->ReplaceInst(inst, replace);
    graph->GetEventWriter().EventLowering(GetOpcodeString(inst->GetOpcode()), inst->GetId(), inst->GetPc());
    if (graph->IsBytecodeOptimizer()) {
        OptimizeIfInput(replace);
    }
    COMPILER_LOG(DEBUG, LOWERING) << "Lowering is applied for " << GetOpcodeString(inst->GetOpcode());
}

void Lowering::LowerToDeoptimizeCompare(Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::DeoptimizeIf);
    auto graph = inst->GetBasicBlock()->GetGraph();
    ASSERT(!graph->IsBytecodeOptimizer());

    auto deoptIf = inst->CastToDeoptimizeIf();
    if (deoptIf->GetInput(0).GetInst()->GetOpcode() != Opcode::Compare) {
        return;
    }
    auto compare = deoptIf->GetInput(0).GetInst()->CastToCompare();
    if (!compare->HasSingleUser()) {
        return;
    }
    COMPILER_LOG(DEBUG, LOWERING) << __func__ << "\n" << *compare << "\n" << *deoptIf;
    auto cmpInp1 = compare->GetInput(1).GetInst();
    DataType::Type type = compare->GetOperandsType();
    uint32_t size =
        (type == DataType::UINT64 || type == DataType::INT64 || type == DataType::ANY) ? WORD_SIZE : HALF_SIZE;
    Inst *deoptCmp = nullptr;
    if ((cmpInp1->IsConst() && ConstantFitsCompareImm(cmpInp1, size, compare->GetCc())) || cmpInp1->IsNullPtr()) {
        uint64_t imm = cmpInp1->IsNullPtr() ? 0 : cmpInp1->CastToConstant()->GetRawValue();
        deoptCmp = graph->CreateInstDeoptimizeCompareImm(deoptIf, compare, imm);
    } else {
        deoptCmp = graph->CreateInstDeoptimizeCompare(deoptIf, compare);
        deoptCmp->SetInput(1, compare->GetInput(1).GetInst());
    }
    deoptCmp->SetInput(0, compare->GetInput(0).GetInst());
    deoptCmp->SetSaveState(deoptIf->GetSaveState());
#ifdef PANDA_COMPILER_DEBUG_INFO
    deoptCmp->SetCurrentMethod(inst->GetCurrentMethod());
#endif
    deoptIf->ReplaceUsers(deoptCmp);
    deoptIf->GetBasicBlock()->InsertAfter(deoptCmp, deoptIf);
    deoptIf->ClearFlag(compiler::inst_flags::NO_DCE);
    graph->GetEventWriter().EventLowering(GetOpcodeString(deoptIf->GetOpcode()), deoptIf->GetId(), deoptIf->GetPc());
    COMPILER_LOG(DEBUG, LOWERING) << "===>\n" << *deoptCmp;
}

void Lowering::InvalidateAnalyses()
{
    GetGraph()->InvalidateAnalysis<BoundsAnalysis>();
    GetGraph()->InvalidateAnalysis<AliasAnalysis>();
}

bool Lowering::RunImpl()
{
    VisitGraph();
    return true;
}
}  // namespace ark::compiler
