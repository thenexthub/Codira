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

#include "compiler_logger.h"
#include "optimizer/ir/inst.h"
#include "peepholes.h"
#include "optimizer/analysis/alias_analysis.h"
#include "optimizer/optimizations/const_folding.h"
#include "optimizer/optimizations/object_type_check_elimination.h"

namespace ark::compiler {

void Peepholes::InvalidateAnalyses()
{
    GetGraph()->InvalidateAnalysis<BoundsAnalysis>();
    GetGraph()->InvalidateAnalysis<AliasAnalysis>();
}

bool Peepholes::RunImpl()
{
    GetGraph()->RunPass<DominatorsTree>();
    GetGraph()->RunPass<ObjectTypePropagation>();
    VisitGraph();
    return IsApplied();
}

void Peepholes::VisitSafePoint(GraphVisitor *v, Inst *inst)
{
    if (g_options.IsCompilerSafePointsRequireRegMap()) {
        return;
    }
    if (inst->CastToSafePoint()->RemoveNumericInputs()) {
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
    }
}

void Peepholes::VisitNeg([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    if (ConstFoldingNeg(inst)) {
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
        return;
    }
    auto input = inst->GetInput(0).GetInst();
    auto inputOpc = input->GetOpcode();
    auto instType = inst->GetType();
    auto inputType = input->GetType();
    if (DataType::IsFloatType(instType)) {
        return;
    }
    // Repeated application of the Neg
    // 1. Some inst -> {v2}
    // 2.i64 neg v1 -> {v3, ...}
    // 3.i64 neg v2 -> {...}
    // ===>
    // 1. Some inst -> {vv3 users}
    // 2.i64 neg v1 -> {v3, ...}
    // 3.i64 neg v2
    if (inputOpc == Opcode::Neg && instType == inputType) {
        if (SkipThisPeepholeInOSR(inst, input->GetInput(0).GetInst())) {
            return;
        }
        inst->ReplaceUsers(input->GetInput(0).GetInst());
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
        return;
    }
    // Сhanging the parameters of subtraction with the use of Neg
    // 2.i64 sub v0, v1 -> {v3, ...}
    // 3.i64 neg v2 -> {...}
    // ===>
    // 2.i64 sub v0, v1 -> {v3, ...}
    // 3.i64 neg v2 -> {}
    // 4.i64 sub v1, v0 -> {users v3}
    if (inputOpc == Opcode::Sub && instType == inputType) {
        if (SkipThisPeepholeInOSR(inst, input->GetInput(0).GetInst()) ||
            SkipThisPeepholeInOSR(inst, input->GetInput(1).GetInst())) {
            return;
        }
        CreateAndInsertInst(Opcode::Sub, inst, input->GetInput(1).GetInst(), input->GetInput(0).GetInst());
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
        return;
    }
}

void Peepholes::VisitAbs([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    if (!DataType::IsTypeSigned(inst->GetType())) {
        if (SkipThisPeepholeInOSR(inst, inst->GetInput(0).GetInst())) {
            return;
        }
        inst->ReplaceUsers(inst->GetInput(0).GetInst());
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
        return;
    }
    if (ConstFoldingAbs(inst)) {
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
        return;
    }
}

void Peepholes::VisitNot([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    if (ConstFoldingNot(inst)) {
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
        return;
    }
}

void Peepholes::VisitAddFinalize([[maybe_unused]] GraphVisitor *v, Inst *inst, Inst *input0, Inst *input1)
{
    auto visitor = static_cast<Peepholes *>(v);
    // Some of the previous transformations might change the opcode of inst.
    if (inst->GetOpcode() == Opcode::Add) {
        if (visitor->TryReassociateShlShlAddSub(inst)) {
            PEEPHOLE_IS_APPLIED(visitor, inst);
            return;
        }

        if (input0->GetOpcode() == Opcode::Add && input1->GetOpcode() == Opcode::Sub) {
            std::swap(input0, input1);
        }

        if (input0->GetOpcode() == Opcode::Sub && input1->GetOpcode() == Opcode::Add) {
            if (visitor->TrySimplifyAddSubAdd(inst, input0, input1)) {
                return;
            }
        }

        if (input0->GetOpcode() == Opcode::Sub && input1->GetOpcode() == Opcode::Sub) {
            if (visitor->TrySimplifyAddSubSub(inst, input0, input1)) {
                return;
            }
        }
    }
}

/**
 * Case 1: Swap inputs if the first is a constant
 * 2. add const, v1 -> {...}
 * ===>
 * 2. add v1, const -> {...}
 *
 * Case 2: Addition with zero
 * 1. Some inst -> v2
 * 2. add v1, 0 -> {...}
 * ===>
 * 1. Some inst ->{...}
 * 2. add v1, 0 -> {}
 *
 * Case 3: Repeated arithmetic with constants
 * 2. add/sub v1, const1 -> {v3, ...}
 * 3. add v2, const2 -> {...}
 * ===>
 * 2. add/sub v1, const1 -> {...}
 * 3. add v1, const2 +/- const1 ->{...}
 *
 * Case 4: Addition with Neg
 * 3. neg v1 -> {5}
 * 4. neg v2 -> {5}
 * 5. add v3, v4 -> {...}
 * ===>
 * 3. neg v1 -> {}
 * 4. neg v2 -> {}
 * 5. add v1, v2 -> {4}
 * 6. neg v5 -> {users v5}
 *
 * Case 5:
 * 3. neg v2 -> {4, ...}
 * 4. add v1, v3 -> {...}
 * ===>
 * 3. neg v2 -> {...}
 * 4. sub v1, v2 -> {...}
 *
 * Case 6:
 * Adding and subtracting the same value
 * 3. sub v2, v1 -> {4, ...}
 * 4. add v3, v1 -> {...}
 * ===>
 * 3. sub v2, v1 -> {4, ...}
 * 4. add v3, v1 -> {}
 * v2 -> {users v4}
 *
 * Case 7:
 * Replacing Negation pattern (neg + add) with Compare
 * 1.i64 Constant 0x1 -> {4, ...}
 * 2.b   ... -> {3}
 * 3.i32 Neg v2 -> {4, ...}
 * 4.i32 Add v3, v1 -> {...}
 * ===>
 * 1.i64 Constant 0x1 -> {...}
 * 5.i64 Constant 0x0 -> {v6, ...}
 * 2.b   ... -> {v3, v6}
 * 3.i32 Neg v2 -> {...}
 * 4.i32 Add v3, v1
 * 6.b   Compare EQ b v2, v5 -> {USERS v4}
 *
 * Case 8:
 * Delete negation of Compare
 * 1.i64 Constant 0x1 -> {v4}
 * 2.b   Compare GT ... -> {v3}
 * 3.i32 Neg v2 -> {v4}
 * 4.i32 Add v3, v1 -> {...}
 * ===>
 * 1.i64 Constant 0x1 -> {v4}
 * 2.b   Compare LE ... -> {v3, USERS v4}
 * 3.i32 Neg v2 -> {v4}
 * 4.i32 Add v3, v1
 *
 * Case 9
 * One of the inputs of Compare is Negation pattern
 * 1.i64 Constant 0x1 -> {v4, ...}
 * 2.b   ... -> {v3}
 * 3.i32 Neg v2 -> {v4, ...}
 * 4.i32 Add v3, v1 -> {v5, ...}
 * 5.b   Compare EQ/NE b v4, ...
 * =======>
 * 1.i64 Constant 0x1 -> {v4, ...}
 * 2.b   ... -> {v3, v5}
 * 3.i32 Neg v2 -> {v4, ...}
 * 4.i32 Add v3, v1 -> {...}
 * 5.b   Compare NE/EQ b v2, ...
 */
void Peepholes::VisitAdd([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    auto visitor = static_cast<Peepholes *>(v);
    if (ConstFoldingAdd(inst)) {
        PEEPHOLE_IS_APPLIED(visitor, inst);
        return;
    }
    if (DataType::IsFloatType(inst->GetType())) {
        return;
    }
    // Case 1
    visitor->TrySwapInputs(inst);
    if (visitor->TrySimplifyAddSubWithConstInput(inst)) {
        return;
    }
    auto input0 = inst->GetInput(0).GetInst();
    auto input1 = inst->GetInput(1).GetInst();
    if (input0->GetOpcode() == Opcode::Neg && input1->GetOpcode() == Opcode::Neg) {
        // Case 4
        if (input0->HasSingleUser() && input1->HasSingleUser()) {
            if (SkipThisPeepholeInOSR(inst, input0->GetInput(0).GetInst()) ||
                SkipThisPeepholeInOSR(inst, input1->GetInput(0).GetInst())) {
                return;
            }
            inst->SetInput(0, input0->GetInput(0).GetInst());
            inst->SetInput(1, input1->GetInput(0).GetInst());
            CreateAndInsertInst(Opcode::Neg, inst, inst);
            PEEPHOLE_IS_APPLIED(visitor, inst);
            return;
        }
    } else {
        // Case 7, 8, 9
        if (visitor->TrySimplifyNegationPattern(inst)) {
            return;
        }
        // Case 5
        visitor->TrySimplifyNeg(inst);
    }
    // Case 6
    visitor->TrySimplifyAddSub<Opcode::Sub, 0>(inst, input0, input1);
    visitor->TrySimplifyAddSub<Opcode::Sub, 0>(inst, input1, input0);

    VisitAddFinalize(v, inst, input0, input1);
}

void Peepholes::VisitSubFinalize([[maybe_unused]] GraphVisitor *v, Inst *inst, Inst *input0, Inst *input1)
{
    auto visitor = static_cast<Peepholes *>(v);
    // Some of the previous transformations might change the opcode of inst.
    if (inst->GetOpcode() == Opcode::Sub) {
        if (visitor->TryReassociateShlShlAddSub(inst)) {
            PEEPHOLE_IS_APPLIED(visitor, inst);
            return;
        }

        if (input0->GetOpcode() == Opcode::Add && input1->GetOpcode() == Opcode::Add) {
            if (visitor->TrySimplifySubAddAdd(inst, input0, input1)) {
                return;
            }
        }
    }
}
/**
 * Case 1
 * Subtraction of zero
 * 1. Some inst
 * 2. sub v1, 0 -> {...}
 * ===>
 * 1. Some inst
 * 2. sub v1, 0 -> {}
 * Case 2
 * Repeated arithmetic with constants
 * 2. add/sub v1, const1 -> {v3, ...}
 * 3. sub v2, const2 -> {...}
 * ===>
 * 2. add/sub v1, const1 -> {...}
 * 3. sub v1, const2 -/+ const1 ->{...}
 * Case 3
 * Subtraction of Neg
 * 3. neg v1 -> {5, ...}
 * 4. neg v2 -> {5, ...}
 * 5. sub v3, v4 -> {...}
 * ===>
 * 3. neg v1 -> {...}
 * 4. neg v2 -> {...}
 * 5. sub v2, v1 -> {...}
 * Case 4
 * Adding and subtracting the same value
 * 3. add v1, v2 -> {4, ...}
 * 4. sub v3, v2 -> {...}
 * ===>
 * 3. add v1, v2 -> {4, ...}
 * 4. sub v3, v1 -> {}
 * v1 -> {users v4}
 * Case 5
 * 3. sub v1, v2 -> {4, ...}
 * 4. sub v1, v3 -> {...}
 * ===>
 * 3. sub v1, v2 -> {4, ...}
 * 4. sub v1, v3 -> {}
 * v2 -> {users v4}
 * Case 6
 * 1. Some inst
 * 2. sub 0, v1 -> {...}
 * ===>
 * 1. Some inst
 * 2. neg v1 -> {...}
 */
void Peepholes::VisitSub([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    auto visitor = static_cast<Peepholes *>(v);
    if (ConstFoldingSub(inst)) {
        PEEPHOLE_IS_APPLIED(visitor, inst);
        return;
    }
    if (DataType::IsFloatType(inst->GetType())) {
        return;
    }
    // Case 1, 2, 6
    if (visitor->TrySimplifyAddSubWithConstInput(inst)) {
        return;
    }
    auto input0 = inst->GetInput(0).GetInst();
    auto input1 = inst->GetInput(1).GetInst();
    // Case 3
    if (input0->GetOpcode() == Opcode::Neg && input1->GetOpcode() == Opcode::Neg) {
        auto newInput1 = input0->GetInput(0).GetInst();
        auto newInput0 = input1->GetInput(0).GetInst();
        if (SkipThisPeepholeInOSR(inst, newInput0) || SkipThisPeepholeInOSR(inst, newInput1)) {
            return;
        }
        inst->SetInput(0, newInput0);
        inst->SetInput(1, newInput1);
        PEEPHOLE_IS_APPLIED(visitor, inst);
        return;
    }
    if (input1->GetOpcode() == Opcode::Neg) {
        auto newInput = input1->GetInput(0).GetInst();
        if (SkipThisPeepholeInOSR(inst, newInput)) {
            return;
        }
        inst->SetInput(1, newInput);
        inst->SetOpcode(Opcode::Add);
        PEEPHOLE_IS_APPLIED(visitor, inst);
        return;
    }
    // Case 4
    visitor->TrySimplifyAddSub<Opcode::Add, 0>(inst, input0, input1);
    visitor->TrySimplifyAddSub<Opcode::Add, 1>(inst, input0, input1);
    // Case 5
    if (input1->GetOpcode() == Opcode::Sub && input1->GetInput(0) == input0) {
        auto prevInput = input1->GetInput(1).GetInst();
        if (inst->GetType() == prevInput->GetType()) {
            if (SkipThisPeepholeInOSR(inst, prevInput)) {
                return;
            }
            inst->ReplaceUsers(prevInput);
            PEEPHOLE_IS_APPLIED(visitor, inst);
            return;
        }
    }
    VisitSubFinalize(v, inst, input0, input1);
}

void Peepholes::VisitMulOneConst([[maybe_unused]] GraphVisitor *v, Inst *inst, Inst *input0, Inst *input1)
{
    if (!input1->IsConst()) {
        return;
    }
    auto constInst = static_cast<ConstantInst *>(input1);
    if (constInst->IsEqualConstAllTypes(1)) {
        // case 1:
        // 0. Const 1
        // 1. MUL v5, v0 -> {v2, ...}
        // 2. INS v1
        // ===>
        // 0. Const 1
        // 1. MUL v5, v0
        // 2. INS v5
        if (SkipThisPeepholeInOSR(inst, input0)) {
            return;
        }
        inst->ReplaceUsers(input0);
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
    } else if (constInst->IsEqualConstAllTypes(-1)) {
        // case 2:
        // 0. Const -1
        // 1. MUL v5, v0 -> {v2, ...}
        // 2. INS v1
        // ===>
        // 0. Const -1
        // 1. MUL v5, v0
        // 3. NEG v5 -> {v2, ...}
        // 2. INS v3

        // "inst"(mul) and **INST WITHOUT NAME**(neg) are one by one, so we don't should check SaveStateOSR between them
        CreateAndInsertInst(Opcode::Neg, inst, input0);
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
    } else if (constInst->IsEqualConstAllTypes(2U)) {
        // case 3:
        // 0. Const 2
        // 1. MUL v5, v0 -> {v2, ...}
        // 2. INS v1
        // ===>
        // 0. Const -1
        // 1. MUL v5, v0
        // 3. ADD v5 , V5 -> {v2, ...}
        // 2. INS v3

        // "inst"(mul) and **INST WITHOUT NAME**(add) are one by one, so we don't should check SaveStateOSR between them
        CreateAndInsertInst(Opcode::Add, inst, input0, input0);
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
    } else if (DataType::GetCommonType(inst->GetType()) == DataType::INT64) {
        int64_t n = GetPowerOfTwo(constInst->GetIntValue());
        if (n != -1) {
            // case 4:
            // 0. Const 2^N
            // 1. MUL v5, v0 -> {v2, ...}
            // 2. INS v1
            // ===>
            // 0. Const -1
            // 1. MUL v5, v0
            // 3. SHL v5 , N -> {v2, ...}
            // 2. INS v3

            // "inst"(mul) and **INST WITHOUT NAME**(add) are one by one, so we don't should
            // check SaveStateOSR between them
            ConstantInst *power = ConstFoldingCreateIntConst(inst, static_cast<uint64_t>(n));
            CreateAndInsertInst(Opcode::Shl, inst, input0, power);
            PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
        }
    }
}

void Peepholes::VisitMul(GraphVisitor *v, Inst *inst)
{
    auto visitor = static_cast<Peepholes *>(v);
    if (ConstFoldingMul(inst)) {
        PEEPHOLE_IS_APPLIED(visitor, inst);
        return;
    }
    if (DataType::IsFloatType(inst->GetType())) {
        return;
    }
    visitor->TrySwapInputs(inst);
    auto input0 = inst->GetInput(0).GetInst();
    auto input1 = inst->GetInput(1).GetInst();
    if (input1->IsConst()) {
        auto cnst = input1->CastToConstant();
        bool osrBlockedPeephole = false;
        if (input0->GetOpcode() == Opcode::Mul && TryCombineMulConst(inst, cnst, &osrBlockedPeephole)) {
            // 0. Const 3
            // 1. Const 7
            // 2. ...
            // 3. Mul v2, v0
            // 4. Mul v3, v1
            // ===>
            // 5. Const 21
            // 2. ...
            // 4. Mul v2, v5
            PEEPHOLE_IS_APPLIED(visitor, inst);
            return;
        }
        if (osrBlockedPeephole) {
            return;
        }
    }
    VisitMulOneConst(v, inst, input0, input1);
}

void Peepholes::VisitDiv([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    if (ConstFoldingDiv(inst)) {
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
        return;
    }
    if (DataType::IsFloatType(inst->GetType())) {
        return;
    }
    auto input0 = inst->GetInput(0).GetInst();
    auto input1 = inst->GetInput(1).GetInst();
    if (input1->IsConst()) {
        auto constInst = static_cast<ConstantInst *>(input1);
        if (constInst->IsEqualConstAllTypes(1)) {
            // case 1:
            // 0. Const 1
            // 1. DIV v5, v0 -> {v2, ...}
            // 2. INS v1
            // ===>
            // 0. Const 1
            // 1. DIV v5, v0
            // 2. INS v5
            if (SkipThisPeepholeInOSR(inst, input0)) {
                return;
            }
            inst->ReplaceUsers(input0);
            PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
        } else if (constInst->IsEqualConstAllTypes(-1)) {
            // case 2:
            // 0. Const -1
            // 1. DIV v5, v0 -> {v2, ...}
            // 2. INS v1
            // ===>
            // 0. Const -1
            // 1. DIV v5, v0
            // 3. NEG v5 -> {v2, ...}
            // 2. INS v3

            // "inst"(div) and **INST WITHOUT NAME**(neg) are one by one, we should check SaveStateOSR between
            // **INST WITHOUT NAME**(neg) and "input0", but may check only between "inst"(div) and "input0"
            if (SkipThisPeepholeInOSR(inst, input0)) {
                return;
            }
            CreateAndInsertInst(Opcode::Neg, inst, input0);
            PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
        } else if (DataType::GetCommonType(inst->GetType()) == DataType::INT64) {
            static_cast<Peepholes *>(v)->TryReplaceDivByShift(inst);
        }
    }
}

void Peepholes::VisitMin([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    if (ConstFoldingMin(inst)) {
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
        return;
    }
    if (DataType::IsFloatType(inst->GetType())) {
        return;
    }
    // Swap inputs if the first is a constant
    // 2. Min const, v1 -> {...}
    // ===>
    // 2. Min v1, const -> {...}
    static_cast<Peepholes *>(v)->TrySwapInputs(inst);
}

void Peepholes::VisitMax([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    if (ConstFoldingMax(inst)) {
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
        return;
    }
    if (DataType::IsFloatType(inst->GetType())) {
        return;
    }
    // Swap inputs if the first is a constant
    // 2. Max const, v1 -> {...}
    // ===>
    // 2. Max v1, const -> {...}
    static_cast<Peepholes *>(v)->TrySwapInputs(inst);
}

void Peepholes::VisitMod([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    if (ConstFoldingMod(inst)) {
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
        return;
    }
}

void Peepholes::VisitShl([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    if (ConstFoldingShl(inst)) {
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
        return;
    }
    static_cast<Peepholes *>(v)->TrySimplifyShifts(inst);
}

void Peepholes::VisitShr([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    if (ConstFoldingShr(inst)) {
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
        return;
    }
    static_cast<Peepholes *>(v)->TrySimplifyShifts(inst);
    // TrySimplifyShifts can replace inst by another.
    if (inst->GetUsers().Empty()) {
        return;
    }
    // case 1
    // 0.i64 Constant X
    // 1. ...
    // 2.Type Shl v1, v0
    // 3.Type Shr v2, v0
    // ====>
    // 0.i64 Constant X
    // 4.i64 Constant (1U << (Size(Type) - X)) - 1
    // 1. ...
    // 5.Type And v1, v4
    auto op1 = inst->GetInput(0).GetInst();
    auto op2 = inst->GetInput(1).GetInst();
    if (op1->GetOpcode() == Opcode::Shl && op2->IsConst() && op1->GetInput(1) == op2) {
        uint64_t val = op2->CastToConstant()->GetIntValue();
        ASSERT(inst->GetType() == op1->GetType());
        auto graph = inst->GetBasicBlock()->GetGraph();
        auto arch = graph->GetArch();
        ASSERT(DataType::GetTypeSize(inst->GetType(), arch) >= val);
        auto t = static_cast<uint8_t>(DataType::GetTypeSize(inst->GetType(), arch) - val);
        uint64_t mask = (1UL << t) - 1;
        auto newCnst = graph->FindOrCreateConstant(mask);
        if (graph->IsBytecodeOptimizer() && IsInt32Bit(inst->GetType())) {
            t = static_cast<uint8_t>(DataType::GetTypeSize(inst->GetType(), arch) - static_cast<uint32_t>(val));
            mask = static_cast<uint32_t>((1U << t) - 1);
            newCnst = graph->FindOrCreateConstant<uint32_t>(mask);
        }
        // "inst"(shr) and **INST WITHOUT NAME**(and) one by one, so we may check SaveStateOSR only between
        // "inst"(shr) and "op1->GetInput(0).GetInst()"
        if (SkipThisPeepholeInOSR(op1->GetInput(0).GetInst(), inst)) {
            return;
        }
        CreateAndInsertInst(Opcode::And, inst, op1->GetInput(0).GetInst(), newCnst);
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
    }
}

void Peepholes::VisitAShr([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    if (ConstFoldingAShr(inst)) {
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
        return;
    }
    /**
     * This VisitAShr() may new create some Cast instruction which will cause failure in the codegen stage.
     * Even though these Casts' resolving can be supported in bytecode_optimizer's CodeGen,
     * they will be translated back to 'shl && ashr', therefore, this part is skipped in bytecode_opt mode.
     */
    if (inst->GetBasicBlock()->GetGraph()->IsBytecodeOptimizer()) {
        return;
    }
    static_cast<Peepholes *>(v)->TrySimplifyShifts(inst);
    // TrySimplifyShifts can replace inst by another.
    if (inst->GetUsers().Empty()) {
        return;
    }
    // case 1
    // 0.i64 Constant
    // 1. ...
    // 2.Type Shl  v1, v0
    // 3.Type AShr v2, v0
    // ====>
    // 0.i64 Constant
    // 1. ...
    // 5. Cast v1
    auto op1 = inst->GetInput(0).GetInst();
    auto op2 = inst->GetInput(1).GetInst();
    if (op1->GetOpcode() == Opcode::Shl && op2->IsConst() && op1->GetInput(1) == op2) {
        ASSERT(inst->GetType() == op1->GetType());
        const uint64_t offsetInT8 = 24;
        const uint64_t offsetInT16 = 16;
        const uint64_t offsetInT32 = 8;
        auto offset = op2->CastToConstant()->GetIntValue();
        if (offset == offsetInT16 || offset == offsetInT8 || offset == offsetInT32) {
            // "inst"(shr) and "cast"(cast) one by one, so we may check SaveStateOSR only between "inst"(shr)
            // and "op1->GetInput(0).GetInst()"(some inst)
            if (SkipThisPeepholeInOSR(op1->GetInput(0).GetInst(), inst)) {
                return;
            }
            auto cast = CreateAndInsertInst(Opcode::Cast, inst, op1->GetInput(0).GetInst());
            cast->SetType((offset == offsetInT8)    ? DataType::INT8
                          : (offset == offsetInT16) ? DataType::INT16
                                                    : DataType::INT32);
            cast->CastToCast()->SetOperandsType(op1->GetInput(0).GetInst()->GetType());
            PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
        }
    }
}

static bool ApplyForCastU16(GraphVisitor *v, Inst *inst, Inst *input0, Inst *input1)
{
    return input1->IsConst() && input0->GetOpcode() == Opcode::Cast &&
           DataType::GetCommonType(input0->GetInput(0).GetInst()->GetType()) == DataType::INT64 &&
           IsInt32Bit(inst->GetType()) &&
           DataType::GetTypeSize(input0->GetType(), static_cast<Peepholes *>(v)->GetGraph()->GetArch()) > HALF_SIZE;
}

void Peepholes::VisitAnd([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    if (ConstFoldingAnd(inst)) {
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
        return;
    }
    // case 1:
    // 0.i64 Const ...
    // 1.i64 AND v0, v5
    // ===>
    // 0.i64 Const 25
    // 1.i64 AND v5, v0
    static_cast<Peepholes *>(v)->TrySwapInputs(inst);
    auto input0 = inst->GetInput(0).GetInst();
    auto input1 = inst->GetInput(1).GetInst();
    // NOLINTNEXTLINE(bugprone-branch-clone)
    if (input1->IsConst() && static_cast<ConstantInst *>(input1)->GetIntValue() == static_cast<uint64_t>(-1)) {
        // case 2:
        // 0.i64 Const 0xFFF..FF
        // 1.i64 AND v5, v0
        // 2.i64 INS which use v1
        // ===>
        // 0.i64 Const 0xFFF..FF
        // 1.i64 AND v5, v0
        // 2.i64 INS which use v5
        if (SkipThisPeepholeInOSR(inst, input0)) {
            return;
        }
        inst->ReplaceUsers(input0);
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
    } else if (input0 == input1 && input0->GetType() == inst->GetType()) {
        // case 3:
        // 1.i64 AND v5, v5
        // 2.i64 INS which use v1
        // ===>
        // 1.i64 AND v5, v5
        // 2.i64 INS which use v5
        if (SkipThisPeepholeInOSR(inst, input0)) {
            return;
        }
        inst->ReplaceUsers(input0);
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
    } else if (input0->GetOpcode() == Opcode::Not && input1->GetOpcode() == Opcode::Not && input0->HasSingleUser() &&
               input1->HasSingleUser()) {
        // case 4: De Morgan rules
        // 2.i64 not v0 -> {4}
        // 3.i64 not v1 -> {4}
        // 4.i64 AND v2, v3
        // ===>
        // 5.i64 OR v0, v1
        // 6.i64 not v5
        auto notInput0 = input0->GetInput(0).GetInst();
        auto notInput1 = input1->GetInput(0).GetInst();
        // "inst"(and), "or_inst"(or) and **INST WITHOUT NAME**(not) one by one, so we may check SaveStateOSR only
        // between "inst"(and) and inputs: "not_input0"(some inst), "not_input0"(some inst)
        // and "op1->GetInput(0).GetInst()"
        if (SkipThisPeepholeInOSR(inst, notInput0) || SkipThisPeepholeInOSR(inst, notInput1)) {
            return;
        }
        auto orInst = CreateAndInsertInst(Opcode::Or, inst, notInput0, notInput1);
        CreateAndInsertInst(Opcode::Not, orInst, orInst);
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
    } else if (ApplyForCastU16(v, inst, input0, input1)) {
        // case 5: IR for cast i64 to u16 is
        // 2.i32 cast i64 to i32
        // 3.i32 and v2, 0xFFFF
        // replace it with cast i64tou16
        auto val = input1->CastToConstant()->GetIntValue();
        constexpr auto MASK = (1U << HALF_SIZE) - 1;
        if (val == MASK) {
            // "inst"(and) and "cast"(cast) one by one, so we may check SaveStateOSR only between
            // "inst"(shr) and "input0->GetInput(0).GetInst()"
            if (SkipThisPeepholeInOSR(inst, input0->GetInput(0).GetInst())) {
                return;
            }
            auto cast = CreateAndInsertInst(Opcode::Cast, inst, input0->GetInput(0).GetInst());
            cast->SetType(DataType::UINT16);
            cast->CastToCast()->SetOperandsType(input0->GetInput(0).GetInst()->GetType());
            PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
        }
    }
}

void Peepholes::VisitOr([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    if (ConstFoldingOr(inst)) {
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
        return;
    }
    // case 1:
    // 0.i64 Const ...
    // 1.i64 Or v0, v5
    // ===>
    // 0.i64 Const 25
    // 1.i64 Or v5, v0
    static_cast<Peepholes *>(v)->TrySwapInputs(inst);
    auto input0 = inst->GetInput(0).GetInst();
    auto input1 = inst->GetInput(1).GetInst();
    // NOLINTNEXTLINE(bugprone-branch-clone)
    if (input0 == input1 && input0->GetType() == inst->GetType()) {
        // case 2:
        // 1.i64 OR v5, v5
        // 2.i64 INS which use v1
        // ===>
        // 1.i64 OR v5, v5
        // 2.i64 INS which use v5
        if (SkipThisPeepholeInOSR(inst, input0)) {
            return;
        }
        inst->ReplaceUsers(input0);
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
    } else if (input1->IsConst() && static_cast<ConstantInst *>(input1)->GetIntValue() == static_cast<uint64_t>(0)) {
        // case 3:
        // 0.i64 Const 0x000..00
        // 1.i64 OR v5, v0
        // 2.i64 INS which use v1
        // ===>
        // 0.i64 Const 0x000..00
        // 1.i64 OR v5, v0
        // 2.i64 INS which use v5
        if (SkipThisPeepholeInOSR(inst, input0)) {
            return;
        }
        inst->ReplaceUsers(input0);
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
    } else if (input0->GetOpcode() == Opcode::Not && input1->GetOpcode() == Opcode::Not && input0->HasSingleUser() &&
               input1->HasSingleUser()) {
        // case 4: De Morgan rules
        // 2.i64 not v0 -> {4}
        // 3.i64 not v1 -> {4}
        // 4.i64 OR v2, v3
        // ===>
        // 5.i64 AND v0, v1
        // 6.i64 not v5
        auto notInput0 = input0->GetInput(0).GetInst();
        auto notInput1 = input1->GetInput(0).GetInst();
        // "inst"(or), "and_inst"(and) and **INST WITHOUT NAME**(not) one by one, so we may check SaveStateOSR only
        // between "inst"(or) and inputs: "not_input0"(some inst), "not_input0"(some inst)
        // and "op1->GetInput(0).GetInst()"
        if (SkipThisPeepholeInOSR(inst, notInput0) || SkipThisPeepholeInOSR(inst, notInput1)) {
            return;
        }
        auto andInst = CreateAndInsertInst(Opcode::And, inst, notInput0, notInput1);
        CreateAndInsertInst(Opcode::Not, andInst, andInst);
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
    }
}

void Peepholes::VisitXor([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    if (ConstFoldingXor(inst)) {
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
        return;
    }
    auto visitor = static_cast<Peepholes *>(v);
    // Swap inputs if the first is a constant
    // 2. Xor const, v1 -> {...}
    // ===>
    // 2. Xor v1, const -> {...}
    static_cast<Peepholes *>(v)->TrySwapInputs(inst);
    auto input0 = inst->GetInput(0).GetInst();
    auto input1 = inst->GetInput(1).GetInst();

    // Remove two consecutive xor using the same constant,
    // the remaining xor without users will be removed at the next Cleanup
    // 0.i64 Const   -> (2,3)
    // 1. ...        -> (2)
    // 2. Xor v1, v0 -> (3)
    // 3. Xor v1, v0 -> (4)
    // 4. ...
    // ===>
    // 0.i64 Const   -> (2,3)
    // 1. ...        -> (2,4)
    // 2. Xor v1, v0 -> (3)
    // 3. Xor v1, v0
    // 4. ...
    //
    // Finding patterns from inputs is better, but here we start with the first Xor.
    // If we began with the second Xor, Peepholes::VisitXor would have already converted
    // the first Xor into a Compare (via CreateCompareInsteadOfXorAdd) and added it after
    // the first Xor. We must delete the Xor->Xor chain before this happens, as the
    // Compare only appears when the Xor becomes useless.
    if (TryOptimizeXorChain(inst)) {
        PEEPHOLE_IS_APPLIED(visitor, inst);
        return;
    }

    if (input1->IsConst()) {
        uint64_t val = input1->CastToConstant()->GetIntValue();
        if (static_cast<int64_t>(val) == -1) {
            // Replace xor with not:
            // 0.i64 Const -1
            // 1. ...
            // 2.i64 XOR v0, v1
            // ===>
            // 3.i64 NOT v1
            if (SkipThisPeepholeInOSR(inst, input0)) {
                return;
            }
            CreateAndInsertInst(Opcode::Not, inst, input0);
            PEEPHOLE_IS_APPLIED(visitor, inst);
        } else if (input1->CastToConstant()->GetIntValue() == 0) {
            // Result A xor 0 equal A:
            // 0.i64 Const 0
            // 1. ...
            // 2.i64 XOR v0, v1 -> v3
            // 3. INS v2
            // ===>
            // 0.i64 Const 0
            // 1. ...
            // 2.i64 XOR v0, v1
            // 3. INS v1
            if (SkipThisPeepholeInOSR(inst, input0)) {
                return;
            }
            inst->ReplaceUsers(input0);
            PEEPHOLE_IS_APPLIED(visitor, inst);
        } else if (val == 1 && input0->GetType() == DataType::BOOL) {
            // Replacing Xor with Compare for more case optimizations
            // If this compare is not optimized during pipeline, we will revert it back to xor in Lowering
            // 1.i64 Const 1
            // 2.b   ...
            // 3.i32 Xor v1, v2
            // ===>
            // 1.i64 Const 0
            // 2.b   ...
            // 3.b   Compare EQ b v2, v1
            if (SkipThisPeepholeInOSR(inst, input0)) {
                return;
            }
            CreateCompareInsteadOfXorAdd(inst);
            PEEPHOLE_IS_APPLIED(visitor, inst);
        }
    }
}

void Peepholes::VisitCmp(GraphVisitor *v, Inst *inst)
{
    auto visitor = static_cast<Peepholes *>(v);
    if (ConstFoldingCmp(inst)) {
        PEEPHOLE_IS_APPLIED(visitor, inst);
    }
}

void Peepholes::VisitCompare(GraphVisitor *v, Inst *inst)
{
    auto visitor = static_cast<Peepholes *>(v);
    // Try to replace Compare with any type to bool type
    if (visitor->TrySimplifyCompareAnyType(inst)) {
        PEEPHOLE_IS_APPLIED(visitor, inst);
    }

    /* skip preheader Compare processing until unrolling is done */
    auto graph = inst->GetBasicBlock()->GetGraph();
    if (!graph->IsBytecodeOptimizer() && g_options.IsCompilerDeferPreheaderTransform() && !graph->IsUnrollComplete()) {
        auto bb = inst->GetBasicBlock();
        if (bb->IsLoopPreHeader() && inst->HasSingleUser() && inst->GetFirstUser()->GetInst() == bb->GetLastInst() &&
            bb->GetLastInst()->GetOpcode() == Opcode::IfImm) {
            return;
        }
    }

    if (ConstFoldingCompare(inst)) {
        PEEPHOLE_IS_APPLIED(visitor, inst);
        return;
    }

    bool osrBlockedPeephole = false;
    if (visitor->TrySimplifyCompareWithBoolInput(inst, &osrBlockedPeephole)) {
        if (osrBlockedPeephole) {
            return;
        }
        PEEPHOLE_IS_APPLIED(visitor, inst);
    } else if (visitor->TrySimplifyCmpCompareWithZero(inst, &osrBlockedPeephole)) {
        if (osrBlockedPeephole) {
            return;
        }
        PEEPHOLE_IS_APPLIED(visitor, inst);
    } else if (visitor->TrySimplifyCompareAndZero(inst, &osrBlockedPeephole)) {
        if (osrBlockedPeephole) {
            return;
        }
        PEEPHOLE_IS_APPLIED(visitor, inst);
    } else if (TrySimplifyCompareLenArrayWithZero(inst)) {
        PEEPHOLE_IS_APPLIED(visitor, inst);
    } else if (visitor->TrySimplifyTestEqualInputs(inst)) {
        PEEPHOLE_IS_APPLIED(visitor, inst);
    }
}

bool Peepholes::TrySimplifyCompareAnyTypeCase1(Inst *inst, Inst *input0, Inst *input1)
{
    auto cmpInst = inst->CastToCompare();

    // 2.any  CastValueToAnyType BOOLEAN_TYPE v0 -> (v4)
    // 3.any  CastValueToAnyType BOOLEAN_TYPE v1 -> (v4)
    // 4.     Compare EQ/NE any    v2, v3
    // =======>
    // 4.     Compare EQ/NE bool   v0, v1
    if (input0->CastToCastValueToAnyType()->GetAnyType() != input1->CastToCastValueToAnyType()->GetAnyType()) {
        return false;
    }
    if (SkipThisPeepholeInOSR(cmpInst, input0->GetInput(0).GetInst()) ||
        SkipThisPeepholeInOSR(cmpInst, input1->GetInput(0).GetInst())) {
        return false;
    }
    cmpInst->SetOperandsType(DataType::BOOL);
    cmpInst->SetInput(0, input0->GetInput(0).GetInst());
    cmpInst->SetInput(1, input1->GetInput(0).GetInst());
    return true;
}

bool Peepholes::TrySimplifyCompareAnyTypeCase2(Inst *inst, Inst *input0, Inst *input1)
{
    auto graph = inst->GetBasicBlock()->GetGraph();
    auto cmpInst = inst->CastToCompare();
    auto cc = cmpInst->GetCc();
    auto ifImm = input1->CastToConstant()->GetRawValue();
    auto runtime = graph->GetRuntime();
    uint64_t newConst;

    // 3.any  CastValueToAnyType BOOLEAN_TYPE v2 -> (v4)
    // 4.     Compare EQ/NE any    v3, DYNAMIC_TRUE/FALSE
    // =======>
    // 4.     Compare EQ/NE bool   v2, 0x1/0x0
    if (SkipThisPeepholeInOSR(cmpInst, input0->GetInput(0).GetInst())) {
        return false;
    }
    if (ifImm == runtime->GetDynamicPrimitiveFalse()) {
        newConst = 0;
    } else if (ifImm == runtime->GetDynamicPrimitiveTrue()) {
        newConst = 1;
    } else {
        // In this case, we are comparing the dynamic boolean type with not boolean constant.
        // So the Compare EQ/NE alwayes false/true.
        // In this case, we can change the Compare to Constant instruction.
        // NOTE! It is assumed that there is only one Boolean type for each dynamic language.
        // Support for multiple Boolean types must be maintained separately.
        if (cc != CC_EQ && cc != CC_NE) {
            return false;
        }
        // We create constant, so we don't need to check SaveStateOSR between insts
        cmpInst->ReplaceUsers(graph->FindOrCreateConstant<uint64_t>(cc == CC_NE ? 1 : 0));
        return true;
    }
    cmpInst->SetOperandsType(DataType::BOOL);
    cmpInst->SetInput(0, input0->GetInput(0).GetInst());
    cmpInst->SetInput(1, graph->FindOrCreateConstant(newConst));
    return true;
}

bool Peepholes::TrySimplifyCompareAnyType(Inst *inst)
{
    auto graph = inst->GetBasicBlock()->GetGraph();
    if (graph->IsBytecodeOptimizer()) {
        return false;
    }
    auto cmpInst = inst->CastToCompare();
    if (cmpInst->GetOperandsType() != DataType::ANY) {
        return false;
    }

    auto input0 = cmpInst->GetInput(0).GetInst();
    auto input1 = cmpInst->GetInput(1).GetInst();
    auto cc = cmpInst->GetCc();
    if (input0 == input1 && (cc == CC_EQ || cc == CC_NE)) {
        // We create constant, so we don't need to check SaveStateOSR between insts
        cmpInst->ReplaceUsers(graph->FindOrCreateConstant<uint64_t>(cc == CC_EQ ? 1 : 0));
        return true;
    }

    if (input0->GetOpcode() != Opcode::CastValueToAnyType) {
        return false;
    }
    if (AnyBaseTypeToDataType(input0->CastToCastValueToAnyType()->GetAnyType()) != DataType::BOOL) {
        return false;
    }

    if (input1->GetOpcode() == Opcode::CastValueToAnyType) {
        return TrySimplifyCompareAnyTypeCase1(inst, input0, input1);
    }
    if (!input1->IsConst()) {
        return false;
    }

    return TrySimplifyCompareAnyTypeCase2(inst, input0, input1);
}

// This VisitIf is using only for compile IRTOC
void Peepholes::VisitIf([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    // 2.     Constant           0x0
    // 3.i64  And                v0, v1
    // 4.     If EQ/NE i64       v3, v2
    // =======>
    // 4.     If TST_EQ/TST_NE   v1, v2
    auto visitor = static_cast<Peepholes *>(v);
    if (visitor->GetGraph()->IsBytecodeOptimizer()) {
        return;
    }
    auto ifImm = inst->CastToIf();
    if (ifImm->GetCc() != CC_EQ && ifImm->GetCc() != CC_NE) {
        return;
    }

    auto lhs = ifImm->GetInput(0).GetInst();
    auto rhs = ifImm->GetInput(1).GetInst();
    Inst *andInput {nullptr};
    if (lhs->GetOpcode() == Opcode::And && IsZeroConstantOrNullPtr(rhs)) {
        andInput = lhs;
    } else if (rhs->GetOpcode() == Opcode::And && IsZeroConstantOrNullPtr(lhs)) {
        andInput = rhs;
    } else {
        return;
    }
    if (!andInput->HasSingleUser()) {
        return;
    }

    auto newInput0 = andInput->GetInput(0).GetInst();
    auto newInput1 = andInput->GetInput(1).GetInst();
    if (SkipThisPeepholeInOSR(ifImm, newInput0) || SkipThisPeepholeInOSR(ifImm, newInput1)) {
        return;
    }
    ifImm->SetCc(ifImm->GetCc() == CC_EQ ? CC_TST_EQ : CC_TST_NE);
    ifImm->SetInput(0, newInput0);
    ifImm->SetInput(1, newInput1);
    PEEPHOLE_IS_APPLIED(visitor, inst);
}

static bool TryReplaceCompareAnyType(Inst *inst, Inst *dominateInst)
{
    auto instType = inst->CastToCompareAnyType()->GetAnyType();
    auto dominateType = dominateInst->GetAnyType();
    profiling::AnyInputType dominateAllowedType {};
    if (dominateInst->GetOpcode() == Opcode::AnyTypeCheck) {
        dominateAllowedType = dominateInst->CastToAnyTypeCheck()->GetAllowedInputType();
    } else {
        ASSERT(dominateInst->GetOpcode() == Opcode::CastValueToAnyType);
    }
    ASSERT(inst->CastToCompareAnyType()->GetAllowedInputType() == profiling::AnyInputType::DEFAULT);
    auto graph = inst->GetBasicBlock()->GetGraph();
    auto language = graph->GetRuntime()->GetMethodSourceLanguage(graph->GetMethod());
    auto res = IsAnyTypeCanBeSubtypeOf(language, instType, dominateType, profiling::AnyInputType::DEFAULT,
                                       dominateAllowedType);
    if (!res) {
        // We cannot compare types in compile-time
        return false;
    }
    auto constValue = res.value();
    auto cnst = inst->GetBasicBlock()->GetGraph()->FindOrCreateConstant(constValue);
    // We replace constant, so we don't need to check SaveStateOSR between insts
    inst->ReplaceUsers(cnst);
    return true;
}

void Peepholes::VisitCompareAnyType(GraphVisitor *v, Inst *inst)
{
    auto visitor = static_cast<Peepholes *>(v);
    auto input = inst->GetInput(0).GetInst();
    // from
    //  2.any CastValueToAnyType TYPE1 -> (v3)
    //  3.b CompareAnyType TYPE2 -> (...)
    // to
    //  4. Constant (TYPE1 == TYPE2)  ->(...)
    if (input->GetOpcode() == Opcode::CastValueToAnyType || input->GetOpcode() == Opcode::AnyTypeCheck) {
        if (TryReplaceCompareAnyType(inst, input)) {
            PEEPHOLE_IS_APPLIED(visitor, inst);
            return;
        }
    }
    // from
    //  2.any AnyTypeCheck v1 TYPE1 -> (...)
    //  3.b CompareAnyType v1 TYPE2 -> (...)
    // to
    //  4. Constant (TYPE1 == TYPE2)  ->(...)
    for (auto &user : input->GetUsers()) {
        auto userInst = user.GetInst();
        if (userInst != inst && userInst->GetOpcode() == Opcode::AnyTypeCheck && userInst->IsDominate(inst) &&
            TryReplaceCompareAnyType(inst, userInst)) {
            PEEPHOLE_IS_APPLIED(visitor, inst);
            return;
        }
    }
}

void Peepholes::VisitCastCase1([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    // case 1:
    // remove redundant cast, when source type is equal with target type
    auto input = inst->GetInput(0).GetInst();
    if (SkipThisPeepholeInOSR(inst, input)) {
        return;
    }
    inst->ReplaceUsers(input);
    PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
}

void Peepholes::VisitCastCase2([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    // case 2:
    // remove redundant cast, when cast from source to target and back
    // for bytecode optimizer, this operation may cause mistake for float32-converter pass
    auto input = inst->GetInput(0).GetInst();
    auto prevType = input->GetType();
    auto currType = inst->GetType();
    auto graph = inst->GetBasicBlock()->GetGraph();
    auto arch = graph->GetArch();
    auto origInst = input->GetInput(0).GetInst();
    auto origType = origInst->GetType();
    if (currType == origType && DataType::GetTypeSize(prevType, arch) > DataType::GetTypeSize(currType, arch)) {
        if (SkipThisPeepholeInOSR(inst, origInst)) {
            return;
        }
        inst->ReplaceUsers(origInst);
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
        return;
    }
    // case 2.1:
    // join two sequent narrowing integer casts, e.g:
    // replace
    // cast i64toi32
    // cast i32toi16
    // with
    // cast i64toi16
    if (ApplyForCastJoin(inst, input, origInst, arch)) {
        auto cast = inst->CastToCast();
        if (SkipThisPeepholeInOSR(cast, origInst)) {
            return;
        }
        cast->SetOperandsType(origInst->GetType());
        cast->SetInput(0, origInst);
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
        return;
    }
}

void Peepholes::VisitCastCase3([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    auto input = inst->GetInput(0).GetInst();
    auto currType = inst->GetType();
    auto graph = inst->GetBasicBlock()->GetGraph();
    auto arch = graph->GetArch();

    // case 3:
    // i8.Cast(v1 & 0xff)        = i8.Cast(u8.Cast(v1))   = i8.Cast(v1)
    // i16.Cast(v1 & 0xffff)     = i16.Cast(u16.Cast(v1)) = i16.Cast(v1)
    // i32.Cast(v1 & 0xffffffff) = i32.Cast(u32.Cast(v1)) = i32.Cast(v1)
    auto op0 = input->GetInput(0).GetInst();
    if (graph->IsBytecodeOptimizer() && !IsCastAllowedInBytecode(op0)) {
        return;
    }
    auto op1 = input->GetInput(1).GetInst();
    auto typeSize = DataType::GetTypeSize(currType, arch);
    if (op1->IsConst() && typeSize < DOUBLE_WORD_SIZE) {
        auto val = op1->CastToConstant()->GetIntValue();
        auto mask = (1ULL << typeSize) - 1;
        if ((val & mask) == mask) {
            if (SkipThisPeepholeInOSR(inst, op0)) {
                return;
            }
            inst->SetInput(0, op0);
            inst->CastToCast()->SetOperandsType(op0->GetType());
            PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
            return;
        }
    }
}

void Peepholes::VisitCast([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    if (ConstFoldingCast(inst)) {
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
        return;
    }
    auto input = inst->GetInput(0).GetInst();
    auto prevType = input->GetType();
    auto currType = inst->GetType();
    auto graph = inst->GetBasicBlock()->GetGraph();

    if (prevType == currType) {
        VisitCastCase1(v, inst);
        return;
    }

    if (!graph->IsBytecodeOptimizer() && input->GetOpcode() == Opcode::Cast) {
        VisitCastCase2(v, inst);
        return;
    }

    if (input->GetOpcode() == Opcode::And && DataType::GetCommonType(currType) == DataType::INT64) {
        VisitCastCase3(v, inst);
        return;
    }
}

void Peepholes::VisitLenArray(GraphVisitor *v, Inst *inst)
{
    auto visitor = static_cast<Peepholes *>(v);
    auto graph = visitor->GetGraph();
    if (graph->IsBytecodeOptimizer()) {
        return;
    }
    // 1. .... ->{v2}
    // 2. NewArray v1 ->{v3,..}
    // 3. LenArray v2 ->{v4, v5...}
    // ===>
    // 1. .... ->{v2, v4, v5, ...}
    // 2. NewArray v1 ->{v3,..}
    // 3. LenArray v2
    auto input = inst->GetDataFlowInput(inst->GetInput(0).GetInst());
    if (input->GetOpcode() == Opcode::NewArray) {
        auto arraySize = input->GetDataFlowInput(input->GetInput(NewArrayInst::INDEX_SIZE).GetInst());
        // We can't restore array_size register if it was defined out of OSR loop
        if (SkipThisPeepholeInOSR(inst, arraySize)) {
            return;
        }
        inst->ReplaceUsers(arraySize);
        PEEPHOLE_IS_APPLIED(visitor, inst);
        return;
    }

    if (input->IsPhi() && visitor->TryOptimizeLenArrayForPhi(inst, input->CastToPhi())) {
        PEEPHOLE_IS_APPLIED(visitor, inst);
        return;
    }
    if (visitor->OptimizeLenArrayForMultiArray(inst, input, 0)) {
        PEEPHOLE_IS_APPLIED(visitor, inst);
    }
}

void Peepholes::VisitPhi([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    // 2.type  Phi v0, v1 -> v4, v5, ...
    // 3.type  Phi v0, v1 -> v5, v6, ...
    // ===>
    // 2.type  Phi v0, v1 -> v4, v5, v6, ...
    // 3.type  Phi v0, v1
    if (!inst->GetUsers().Empty()) {
        auto block = inst->GetBasicBlock();
        auto phi = inst->CastToPhi();
        for (auto otherPhi : block->PhiInsts()) {
            if (IsPhiUnionPossible(phi, otherPhi->CastToPhi())) {
                // In check above checked, that phi insts in same BB, so no problem with SaveStateOSR
                otherPhi->ReplaceUsers(phi);
                PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
            }
        }
    }

    if (TryEliminatePhi(inst->CastToPhi())) {
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
    }
}

void Peepholes::VisitSqrt([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    if (ConstFoldingSqrt(inst)) {
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
        return;
    }
}

void Peepholes::VisitIsInstance(GraphVisitor *v, Inst *inst)
{
    if (ObjectTypeCheckElimination::TryEliminateIsInstance(inst)) {
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
        return;
    }
}

void Peepholes::VisitCastAnyTypeValue(GraphVisitor *v, Inst *inst)
{
    if (inst->GetUsers().Empty()) {
        // Peepholes can be run before cleanup phase.
        return;
    }

    auto *cav = inst->CastToCastAnyTypeValue();
    auto *cavInput = cav->GetDataFlowInput(0);
    if (cavInput->GetOpcode() == Opcode::CastValueToAnyType) {
        auto cva = cavInput->CastToCastValueToAnyType();
        auto valueInst = cva->GetInput(0).GetInst();
        if (SkipThisPeepholeInOSR(cav, valueInst)) {
            return;
        }
        if (cva->GetAnyType() == cav->GetAnyType()) {
            // 1. .... -> v2
            // 2. CastValueToAnyType v1 -> v3
            // 3. AnyTypeCheck v2 -> v3
            // 4. CastAnyTypeValue v3 -> v5
            // 5. ...
            // ===>
            // 1. .... -> v2, v5
            // 2. CastValueToAnyType v1 -> v3
            // 3. AnyTypeCheck v2 -> v3
            // 4. CastAnyTypeValue v3
            // 5. ...
            cav->ReplaceUsers(valueInst);
            PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
            return;
        }
        auto inputType = valueInst->GetType();
        auto dstType = cav->GetType();
        auto isDoubleToInt = dstType == DataType::INT32 && inputType == DataType::FLOAT64;
        if (IsTypeNumeric(inputType) && IsTypeNumeric(dstType) && (!isDoubleToInt || cav->IsIntegerWasSeen())) {
            // 1. .... -> v2
            // 2. CastValueToAnyType v1 -> v3
            // 3. AnyTypeCheck v2 -> v3
            // 4. CastAnyTypeValue v3 -> v5
            // 5. ...
            // ===>
            // 1. .... -> v2, v6
            // 2. CastValueToAnyType v1 -> v3
            // 3. AnyTypeCheck v2 -> v3
            // 4. CastAnyTypeValue v3
            // 6. Cast v1 -> v5
            // 5. ...
            auto cast = CreateAndInsertInst(Opcode::Cast, cav, valueInst);
            cast->SetType(dstType);
            cast->CastToCast()->SetOperandsType(inputType);
            PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
        }
    }
}

void Peepholes::VisitCastValueToAnyType(GraphVisitor *v, Inst *inst)
{
    auto graph = inst->GetBasicBlock()->GetGraph();
    auto input = inst->GetInput(0).GetInst();
    auto anyType = inst->CastToCastValueToAnyType()->GetAnyType();

    if (input->GetOpcode() == Opcode::CastAnyTypeValue) {
        auto cav = input->CastToCastAnyTypeValue();
        auto anyInput = cav->GetInput(0).GetInst();
        if (SkipThisPeepholeInOSR(inst, anyInput)) {
            return;
        }
        if (anyType == cav->GetAnyType() && cav->GetAllowedInputType() == profiling::AnyInputType::DEFAULT) {
            // 1. ... -> v2
            // 2. CastAnyTypeValue v1 -> v3
            // 3. CastValueToAnyType v2 -> v4
            // 4. ...
            // ===>
            // 1. ... -> v2, v4
            // 2. CastAnyTypeValue v1 -> v3
            // 3. CastValueToAnyType v2
            // 4. ...
            inst->ReplaceUsers(anyInput);
            PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
            return;
        }
    }

    if (graph->IsBytecodeOptimizer() || graph->IsOsrMode()) {
        // Find way to enable it in OSR mode.
        return;
    }

    auto baseType = AnyBaseTypeToDataType(anyType);
    // We propogate not INT types in Lowering
    if (baseType != DataType::INT32) {
        return;
    }
    // from
    // 2.any CastValueToAnyType INT_TYPE v1 -> (v3)
    // 3     SaveState                   v2(acc)
    //
    // to
    // 3     SaveState                   v1(acc)
    bool isApplied = false;
    for (auto it = inst->GetUsers().begin(); it != inst->GetUsers().end();) {
        auto userInst = it->GetInst();
        if (userInst->IsSaveState()) {
            userInst->SetInput(it->GetIndex(), input);
            it = inst->GetUsers().begin();
            isApplied = true;
        } else {
            ++it;
        }
    }
    if (isApplied) {
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
    }
}

static bool TryPrepareEliminatePrecedingStoreOpcCast(Inst *inst, Arch arch, uint64_t &imm)
{
    auto size = DataType::GetTypeSize(inst->GetType(), arch);
    if (size >= DOUBLE_WORD_SIZE) {
        return false;
    }
    auto inputTypeMismatch = IsInputTypeMismatch(inst, 0, arch);
#ifndef NDEBUG
    ASSERT(!inputTypeMismatch);
#else
    if (inputTypeMismatch) {
        return false;
    }
#endif
    imm = (1ULL << size) - 1;
    return true;
}

template <typename T>
void Peepholes::EliminateInstPrecedingStore(GraphVisitor *v, Inst *inst)
{
    auto graph = static_cast<Peepholes *>(v)->GetGraph();
    if (graph->IsBytecodeOptimizer()) {
        return;
    }
    auto arch = graph->GetArch();
    auto typeSize = DataType::GetTypeSize(inst->GetType(), arch);
    if (DataType::GetCommonType(inst->GetType()) == DataType::INT64 && typeSize < DOUBLE_WORD_SIZE) {
        auto storeValueInst = inst->GetInput(T::STORED_INPUT_INDEX).GetInst();
        auto mask = (1ULL << typeSize) - 1;
        uint64_t imm;

        switch (storeValueInst->GetOpcode()) {
            case Opcode::And: {
                auto inputInst1 = storeValueInst->GetInput(1).GetInst();
                if (!inputInst1->IsConst()) {
                    return;
                }
                imm = inputInst1->CastToConstant()->GetIntValue();
                break;
            }
            case Opcode::Cast: {
                if (!TryPrepareEliminatePrecedingStoreOpcCast(storeValueInst, arch, imm)) {
                    return;
                }
                break;
            }
            default:
                return;
        }

        auto inputInst = storeValueInst->GetInput(0).GetInst();
        if (DataType::GetCommonType(inputInst->GetType()) != DataType::INT64) {
            return;
        }

        if ((imm & mask) == mask) {
            if (SkipThisPeepholeInOSR(inst, inputInst)) {
                return;
            }
            inst->ReplaceInput(storeValueInst, inputInst);
            PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
        }
    }
}

void Peepholes::VisitStore([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::Store);
    EliminateInstPrecedingStore<StoreMemInst>(v, inst);
}

void Peepholes::VisitStoreObject([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::StoreObject);

    auto visitor = static_cast<Peepholes *>(v);
    if (visitor->TryOptimizeBoxedLoadStoreObject(inst)) {
        PEEPHOLE_IS_APPLIED(visitor, inst);
    }

    EliminateInstPrecedingStore<StoreObjectInst>(v, inst);
}

void Peepholes::VisitStoreStatic([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::StoreStatic);
    EliminateInstPrecedingStore<StoreStaticInst>(v, inst);
}

void Peepholes::TryRemoveOverflowCheck(Inst *inst)
{
    auto block = inst->GetBasicBlock();
    auto markerHolder = MarkerHolder(block->GetGraph());
    if (CanRemoveOverflowCheck(inst, markerHolder.GetMarker())) {
        PEEPHOLE_IS_APPLIED(this, inst);
        block->RemoveOverflowCheck(inst);
        return;
    }
}

void Peepholes::VisitAddOverflowCheck([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::AddOverflowCheck);
    auto visitor = static_cast<Peepholes *>(v);
    visitor->TrySwapInputs(inst);
    if (visitor->TrySimplifyAddSubWithZeroInput(inst)) {
        inst->ClearFlag(inst_flags::NO_DCE);
    } else {
        visitor->TryRemoveOverflowCheck(inst);
    }
}

void Peepholes::VisitSubOverflowCheck([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::SubOverflowCheck);
    auto visitor = static_cast<Peepholes *>(v);
    if (visitor->TrySimplifyAddSubWithZeroInput(inst)) {
        inst->ClearFlag(inst_flags::NO_DCE);
    } else {
        visitor->TryRemoveOverflowCheck(inst);
    }
}

void Peepholes::VisitNegOverflowAndZeroCheck([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::NegOverflowAndZeroCheck);
    static_cast<Peepholes *>(v)->TryRemoveOverflowCheck(inst);
}

// This function check that we can merge two Phi instructions in one basic block.
bool Peepholes::IsPhiUnionPossible(PhiInst *phi1, PhiInst *phi2)
{
    ASSERT(phi1->GetBasicBlock() == phi2->GetBasicBlock() && phi1->GetInputsCount() == phi2->GetInputsCount());
    if (phi1 == phi2 || phi1->GetType() != phi2->GetType()) {
        return false;
    }
    for (auto predBlock : phi1->GetBasicBlock()->GetPredsBlocks()) {
        if (phi1->GetPhiDataflowInput(predBlock) != phi2->GetPhiDataflowInput(predBlock)) {
            return false;
        }
    }
    if (phi1->GetBasicBlock()->GetGraph()->IsOsrMode()) {
        // Values in vregs for phi1 and phi2 may be different in interpreter mode,
        // so we don't merge phis if they have SaveStateOsr users
        for (auto phi : {phi1, phi2}) {
            for (auto &user : phi->GetUsers()) {
                if (user.GetInst()->GetOpcode() == Opcode::SaveStateOsr) {
                    return false;
                }
            }
        }
    }
    return true;
}

// Create new instruction instead of current inst
Inst *Peepholes::CreateAndInsertInst(Opcode newOpc, Inst *currInst, Inst *input0, Inst *input1)
{
    auto bb = currInst->GetBasicBlock();
    auto graph = bb->GetGraph();
    auto newInst = graph->CreateInst(newOpc);
    newInst->SetType(currInst->GetType());
    newInst->SetPc(currInst->GetPc());
#ifdef PANDA_COMPILER_DEBUG_INFO
    newInst->SetCurrentMethod(currInst->GetCurrentMethod());
#endif
    // It is a wrapper, so we don't do logic check for SaveStateOSR
    currInst->ReplaceUsers(newInst);
    newInst->SetInput(0, input0);
    if (input1 != nullptr) {
        newInst->SetInput(1, input1);
    }
    bb->InsertAfter(newInst, currInst);
    return newInst;
}

// Try put constant in second input
void Peepholes::TrySwapInputs(Inst *inst)
{
    [[maybe_unused]] auto opc = inst->GetOpcode();
    ASSERT(opc == Opcode::Add || opc == Opcode::And || opc == Opcode::Or || opc == Opcode::Xor || opc == Opcode::Min ||
           opc == Opcode::Max || opc == Opcode::Mul || opc == Opcode::AddOverflowCheck);
    if (inst->GetInput(0).GetInst()->IsConst()) {
        inst->SwapInputs();
        PEEPHOLE_IS_APPLIED(this, inst);
    }
}

void Peepholes::TrySimplifyShifts(Inst *inst)
{
    auto opc = inst->GetOpcode();
    ASSERT(opc == Opcode::Shl || opc == Opcode::Shr || opc == Opcode::AShr);
    auto block = inst->GetBasicBlock();
    auto graph = block->GetGraph();
    auto input0 = inst->GetInput(0).GetInst();
    auto input1 = inst->GetInput(1).GetInst();
    if (input1->IsConst()) {
        auto cnst = static_cast<ConstantInst *>(input1);
        // Zero shift
        // 2. shl/shr/ashr v1, 0 -> {...}
        // 3. Some inst v2
        // ===>
        // 2. shl/shr/ashr v1, 0 -> {}
        // 3. Some inst v1
        if (cnst->IsEqualConst(0)) {
            if (SkipThisPeepholeInOSR(inst, input0)) {
                return;
            }
            inst->ReplaceUsers(input0);
            PEEPHOLE_IS_APPLIED(this, inst);
            return;
        }
        // Repeated arithmetic with constants
        // 2. shl/shr/ashr v1, const1 -> {v3, ...}
        // 3. shl/shr/ashr v2, const2 -> {...}
        // ===>
        // 2. shl/shr/ashr v1, const1 -> {...}
        // 3. shl/shr/ashr v1, const2 + const1 -> {...}
        bool osrBlockedPeephole = false;
        if (opc == input0->GetOpcode() && TryCombineShiftConst(inst, cnst, &osrBlockedPeephole)) {
            PEEPHOLE_IS_APPLIED(this, inst);
            return;
        }
        if (osrBlockedPeephole) {
            return;
        }
        uint64_t sizeMask = DataType::GetTypeSize(inst->GetType(), graph->GetArch()) - 1;
        auto cnstValue = cnst->GetIntValue();
        // Shift by a constant greater than the type size
        // 2. shl/shr/ashr v1, big_const -> {...}
        // ===>
        // 2. shl/shr/ashr v1, size_mask & big_const -> {...}
        if (graph->IsBytecodeOptimizer() && IsInt32Bit(inst->GetType())) {
            sizeMask = static_cast<uint32_t>(sizeMask);
            cnstValue = static_cast<uint32_t>(cnstValue);
        }
        if (sizeMask < cnstValue) {
            ConstantInst *shift = ConstFoldingCreateIntConst(inst, sizeMask & cnstValue);
            inst->SetInput(1, shift);
            PEEPHOLE_IS_APPLIED(this, inst);
            return;
        }
    }
}

bool Peepholes::TrySimplifyAddSubWithZeroInput(Inst *inst)
{
    auto input0 = inst->GetInput(0).GetInst();
    auto input1 = inst->GetInput(1).GetInst();
    if (input1->IsConst()) {
        auto cnst = input1->CastToConstant();
        if (cnst->IsEqualConstAllTypes(0)) {
            if (SkipThisPeepholeInOSR(inst, input0)) {
                return false;
            }
            inst->ReplaceUsers(input0);
            PEEPHOLE_IS_APPLIED(this, inst);
            return true;
        }
    }
    return false;
}

bool Peepholes::TrySimplifyAddSubWithConstInput(Inst *inst)
{
    if (TrySimplifyAddSubWithZeroInput(inst)) {
        return true;
    }
    auto input0 = inst->GetInput(0).GetInst();
    auto input1 = inst->GetInput(1).GetInst();
    if (input1->IsConst()) {
        auto cnst = input1->CastToConstant();
        if ((input0->GetOpcode() == Opcode::Add || input0->GetOpcode() == Opcode::Sub)) {
            bool osrBlockedPeephole = false;
            if (TryCombineAddSubConst(inst, cnst, &osrBlockedPeephole)) {
                PEEPHOLE_IS_APPLIED(this, inst);
                return true;
            }
            if (osrBlockedPeephole) {
                return true;
            }
        }
    } else if (input0->IsConst() && inst->GetOpcode() == Opcode::Sub && !IsFloatType(inst->GetType())) {
        // Fold `sub 0, v1` into `neg v1`.
        auto cnst = input0->CastToConstant();
        if (cnst->IsEqualConstAllTypes(0)) {
            // There can't be a SaveStateOSR between inst(sub) and new inst(neg), so we don't have a check
            CreateAndInsertInst(Opcode::Neg, inst, input1);
            PEEPHOLE_IS_APPLIED(this, inst);
            return true;
        }
    }
    return false;
}

template <Opcode OPC, int IDX>
void Peepholes::TrySimplifyAddSub(Inst *inst, Inst *input0, Inst *input1)
{
    if (input0->GetOpcode() == OPC && input0->GetInput(1 - IDX).GetInst() == input1) {
        auto prevInput = input0->GetInput(IDX).GetInst();
        if (inst->GetType() == prevInput->GetType()) {
            if (SkipThisPeepholeInOSR(inst, prevInput)) {
                return;
            }
            inst->ReplaceUsers(prevInput);
            PEEPHOLE_IS_APPLIED(this, inst);
            return;
        }
    }
}

bool Peepholes::TrySimplifyAddSubAdd(Inst *inst, Inst *input0, Inst *input1)
{
    // (a - b) + (b + c) = a + c
    if (input0->GetInput(1) == input1->GetInput(0)) {
        auto newInput0 = input0->GetInput(0).GetInst();
        auto newInput1 = input1->GetInput(1).GetInst();
        if (SkipThisPeepholeInOSR(inst, newInput0) || SkipThisPeepholeInOSR(inst, newInput1)) {
            return true;
        }
        inst->SetInput(0, newInput0);
        inst->SetInput(1, newInput1);
        PEEPHOLE_IS_APPLIED(this, inst);
        return true;
    }

    // (a - b) + (c + b) = a + c
    if (input0->GetInput(1) == input1->GetInput(1)) {
        auto newInput0 = input0->GetInput(0).GetInst();
        auto newInput1 = input1->GetInput(0).GetInst();
        if (SkipThisPeepholeInOSR(inst, newInput0) || SkipThisPeepholeInOSR(inst, newInput1)) {
            return true;
        }
        inst->SetInput(0, newInput0);
        inst->SetInput(1, newInput1);
        PEEPHOLE_IS_APPLIED(this, inst);
        return true;
    }

    return false;
}

bool Peepholes::TrySimplifyAddSubSub(Inst *inst, Inst *input0, Inst *input1)
{
    // (a - b) + (b - c) = a - c
    if (input0->GetInput(1) == input1->GetInput(0)) {
        auto newInput0 = input0->GetInput(0).GetInst();
        auto newInput1 = input1->GetInput(1).GetInst();
        if (SkipThisPeepholeInOSR(inst, newInput0) || SkipThisPeepholeInOSR(inst, newInput1)) {
            return true;
        }

        inst->SetOpcode(Opcode::Sub);
        inst->SetInput(0, newInput0);
        inst->SetInput(1, newInput1);
        PEEPHOLE_IS_APPLIED(this, inst);
        return true;
    }

    // (a - b) + (c - a) = c - b
    if (input0->GetInput(0) == input1->GetInput(1)) {
        auto newInput0 = input1->GetInput(0).GetInst();
        auto newInput1 = input0->GetInput(1).GetInst();
        if (SkipThisPeepholeInOSR(inst, newInput0) || SkipThisPeepholeInOSR(inst, newInput1)) {
            return true;
        }
        inst->SetOpcode(Opcode::Sub);
        inst->SetInput(0, newInput0);
        inst->SetInput(1, newInput1);
        PEEPHOLE_IS_APPLIED(this, inst);
        return true;
    }

    return false;
}

bool Peepholes::TrySimplifySubAddAdd(Inst *inst, Inst *input0, Inst *input1)
{
    // (a + b) - (a + c) = b - c
    if (input0->GetInput(0) == input1->GetInput(0)) {
        auto newInput0 = input0->GetInput(1).GetInst();
        auto newInput1 = input1->GetInput(1).GetInst();
        if (SkipThisPeepholeInOSR(inst, newInput0) || SkipThisPeepholeInOSR(inst, newInput1)) {
            return true;
        }
        inst->SetInput(0, newInput0);
        inst->SetInput(1, newInput1);
        PEEPHOLE_IS_APPLIED(this, inst);
        return true;
    }

    // (a + b) - (c + a) = b - c
    if (input0->GetInput(0) == input1->GetInput(1)) {
        auto newInput0 = input0->GetInput(1).GetInst();
        auto newInput1 = input1->GetInput(0).GetInst();
        if (SkipThisPeepholeInOSR(inst, newInput0) || SkipThisPeepholeInOSR(inst, newInput1)) {
            return true;
        }
        inst->SetInput(0, newInput0);
        inst->SetInput(1, newInput1);
        PEEPHOLE_IS_APPLIED(this, inst);
        return true;
    }

    // (a + b) - (c + b) = a - c
    if (input0->GetInput(1) == input1->GetInput(1)) {
        auto newInput0 = input0->GetInput(0).GetInst();
        auto newInput1 = input1->GetInput(0).GetInst();
        if (SkipThisPeepholeInOSR(inst, newInput0) || SkipThisPeepholeInOSR(inst, newInput1)) {
            return true;
        }
        inst->SetInput(0, newInput0);
        inst->SetInput(1, newInput1);
        PEEPHOLE_IS_APPLIED(this, inst);
        return true;
    }

    // (a + b) - (b + c) = a - c
    if (input0->GetInput(1) == input1->GetInput(0)) {
        auto newInput0 = input0->GetInput(0).GetInst();
        auto newInput1 = input1->GetInput(1).GetInst();
        if (SkipThisPeepholeInOSR(inst, newInput0) || SkipThisPeepholeInOSR(inst, newInput1)) {
            return true;
        }
        inst->SetInput(0, newInput0);
        inst->SetInput(1, newInput1);
        PEEPHOLE_IS_APPLIED(this, inst);
        return true;
    }

    return false;
}

static bool CanReassociateShlShlAddSub(Inst *inst)
{
    if (inst->GetOpcode() != Opcode::Sub && inst->GetOpcode() != Opcode::Add) {
        return false;
    }
    if (IsFloatType(inst->GetType())) {
        return false;
    }
    auto input1 = inst->GetInput(1).GetInst();
    if (input1->GetOpcode() != Opcode::Sub && input1->GetOpcode() != Opcode::Add) {
        return false;
    }
    // Potentially inst and input1 can have different types (e.g. UINT32 and UINT16).
    if (input1->GetType() != inst->GetType()) {
        return false;
    }
    if (!input1->HasSingleUser()) {
        return false;
    }
    auto shl0 = input1->GetInput(0).GetInst();
    if (shl0->GetOpcode() != Opcode::Shl || !shl0->HasSingleUser()) {
        return false;
    }
    auto shl1 = input1->GetInput(1).GetInst();
    if (shl1->GetOpcode() != Opcode::Shl || !shl1->HasSingleUser() || shl1->GetInput(0) != shl0->GetInput(0)) {
        return false;
    }
    if (!shl0->GetInput(1).GetInst()->IsConst() || !shl1->GetInput(1).GetInst()->IsConst()) {
        return false;
    }
    auto input0 = inst->GetInput(0).GetInst();
    bool check = !input0->IsConst() && !input0->IsParameter() && !input0->IsDominate(input1);
    return !check;
}

/**
 * The goal is to transform the following sequence:
 *
 * shl v1, v0, const1;
 * shl v2, v0, const2;
 * add/sub v3, v1, v2;
 * add/sub v5, v4, v3;
 *
 * so as to make it ready for the lowering with shifted operands.
 *
 * add v3, v1, v2;
 * add v5, v4, v3;
 * ===>
 * add v6, v4, v1;
 * add v5, v6, v2;
 *
 * add v3, v1, v2;
 * sub v5, v4, v3;
 * ===>
 * sub v6, v4, v1;
 * sub v5, v6, v2;
 *
 * sub v3, v1, v2;
 * add v5, v4, v3;
 * ===>
 * add v6, v4, v1;
 * sub v5, v6, v2;
 *
 * sub v3, v1, v2;
 * sub v5, v4, v3;
 * ===>
 * sub v6, v4, v1;
 * add v5, v6, v2;
 */
bool Peepholes::TryReassociateShlShlAddSub(Inst *inst)
{
    if (!CanReassociateShlShlAddSub(inst)) {
        return false;
    }
    auto input0 = inst->GetInput(0).GetInst();
    auto input1 = inst->GetInput(1).GetInst();
    auto shl0 = input1->GetInput(0).GetInst();
    auto shl1 = input1->GetInput(1).GetInst();
    Opcode opInput1 = inst->GetInput(1).GetInst()->GetOpcode();
    Opcode opInst = inst->GetOpcode();
    if (opInst == Opcode::Sub && opInput1 == Opcode::Add) {
        // input0 - (shl0 + shl1) -> (input0 - shl0) - shl1
        opInput1 = Opcode::Sub;
    } else if (opInst == Opcode::Add && opInput1 == Opcode::Sub) {
        // input0 + (shl0 - shl1) -> (input0 + shl0) - shl1
        opInput1 = Opcode::Add;
        opInst = Opcode::Sub;
    } else if (opInst == Opcode::Sub && opInput1 == Opcode::Sub) {
        // input0 - (shl0 - shl1) -> (input0 - shl0) + shl1
        opInst = Opcode::Add;
    } else if (opInst != Opcode::Add && opInput1 != Opcode::Add) {
        UNREACHABLE();
    }
    // "input1" and "new_input0" one by one, so we don't should to check "SaveStateOSR" between this insts,
    // same for: "inst" and **INST WITHOUT NAME**. We should check only between "inst" and "input1"
    if (SkipThisPeepholeInOSR(inst, input1)) {
        return true;
    }
    auto newInput0 = CreateAndInsertInst(opInput1, input1, input0, shl0);
    CreateAndInsertInst(opInst, inst, newInput0, shl1);
    return true;
}

void Peepholes::TrySimplifyNeg(Inst *inst)
{
    auto input0 = inst->GetInput(0).GetInst();
    auto input1 = inst->GetInput(1).GetInst();
    if ((input0->GetOpcode() == Opcode::Neg && SkipThisPeepholeInOSR(inst, input0->GetInput(0).GetInst())) ||
        (input1->GetOpcode() == Opcode::Neg && SkipThisPeepholeInOSR(inst, input1->GetInput(0).GetInst()))) {
        return;
    }

    // Case 5
    if (input0->GetOpcode() == Opcode::Neg && !input1->IsConst()) {
        inst->SetInput(0, input1);
        inst->SetInput(1, input0);
        std::swap(input0, input1);
    }
    if (input1->GetOpcode() == Opcode::Neg) {
        inst->SetOpcode(Opcode::Sub);
        inst->SetInput(1, input1->GetInput(0).GetInst());
        PEEPHOLE_IS_APPLIED(this, inst);
        return;
    }
}

bool Peepholes::TrySimplifyCompareNegation(Inst *inst)
{
    ASSERT(inst != nullptr);
    ASSERT(inst->GetOpcode() == Opcode::Add);

    // Case 9: Neg -> Add -> Compare
    bool optimized = false;
    for (auto &userAdd : inst->GetUsers()) {
        auto suspectInst = userAdd.GetInst();
        if (suspectInst->GetOpcode() != Opcode::Compare) {
            continue;
        }
        auto compareInst = suspectInst->CastToCompare();
        if (compareInst->GetOperandsType() != DataType::BOOL ||
            (compareInst->GetCc() != ConditionCode::CC_EQ && compareInst->GetCc() != ConditionCode::CC_NE)) {
            continue;
        }

        unsigned indexCast = compareInst->GetInput(0).GetInst() == inst ? 0 : 1;
        auto boolValue = inst->GetInput(0).GetInst()->GetInput(0).GetInst();
        if (SkipThisPeepholeInOSR(inst, boolValue)) {
            continue;
        }
        compareInst->SetInput(indexCast, boolValue);
        compareInst->SetCc(GetInverseConditionCode(compareInst->GetCc()));
        PEEPHOLE_IS_APPLIED(this, compareInst);
        optimized = true;
    }
    return optimized;
}

void Peepholes::TryReplaceDivByShift(Inst *inst)
{
    auto bb = inst->GetBasicBlock();
    auto graph = bb->GetGraph();
    if (graph->IsBytecodeOptimizer()) {
        return;
    }
    auto input0 = inst->GetInput(0).GetInst();
    auto input1 = inst->GetInput(1).GetInst();
    ASSERT(input1->IsConst());
    uint64_t unsignedVal = input1->CastToConstant()->GetIntValue();
    if (!DataType::IsTypeSigned(inst->GetType())) {
        // case 3:
        // 0.unsigned Parameter
        // 1.i64 Const 2^n -> {v2}
        // 2.un-signed DIV v0, v1 -> {v3}
        // 3.unsigned INST v2
        // ===>
        // 0.unsigned Parameter
        // 1.i64 Const n -> {v2}
        // 2.un-signed SHR v0, v1 -> {v3}
        // 3.unsigned INST v2
        int64_t n = GetPowerOfTwo(unsignedVal);
        if (n != -1) {
            auto power = graph->FindOrCreateConstant(n);
            CreateAndInsertInst(Opcode::Shr, inst, input0, power);
            PEEPHOLE_IS_APPLIED(this, inst);
        }
    }
}

bool Peepholes::TrySimplifyCompareCaseInputInv(Inst *inst, Inst *input)
{
    for (auto &user : inst->GetUsers()) {
        auto opcode = user.GetInst()->GetOpcode();
        if (opcode != Opcode::If && opcode != Opcode::IfImm) {
            return false;
        }
    }
    if (SkipThisPeepholeInOSR(inst, input)) {
        return true;
    }
    for (auto &user : inst->GetUsers()) {
        auto opcode = user.GetInst()->GetOpcode();
        if (opcode == Opcode::If) {
            user.GetInst()->CastToIf()->InverseConditionCode();
        } else if (opcode == Opcode::IfImm) {
            user.GetInst()->CastToIfImm()->InverseConditionCode();
        } else {
            UNREACHABLE();
        }
    }
    inst->ReplaceUsers(input);
    return true;
}

// Eliminates double comparison if possible
// 4.i64  Constant                   0x0 -> (v6)
// 5.b    ### Some abstract expression that return boolean ###
// 6.b    Compare EQ i32             v5, v4 -> (v7)
// 7.     IfImm NE b                 v6, 0x0
// =======>
// 4.i64  Constant                   0x0 -> (v6)
// 5.b    ### Some abstract expression that return boolean ###
// 6.b    Compare EQ i32             v5, v4
// 7.     IfImm EQ b                 v5, 0x0
bool Peepholes::TrySimplifyCompareWithBoolInput(Inst *inst, bool *isOsrBlocked)
{
    ASSERT(isOsrBlocked != nullptr);
    auto compare = inst->CastToCompare();
    bool swap = false;
    Inst *input = nullptr;
    ConstantInst *constInput = nullptr;
    if (!GetInputsOfCompareWithConst(compare, &input, &constInput, &swap)) {
        return false;
    }
    if (input->GetType() != DataType::BOOL) {
        return false;
    }
    ConditionCode cc = swap ? SwapOperandsConditionCode(compare->GetCc()) : compare->GetCc();
    InputCode code = GetInputCode(constInput, cc);
    if (code == INPUT_TRUE || code == INPUT_FALSE) {
        // We create constant, so we don't need to check SaveStateOSR between insts
        compare->ReplaceUsers(ConstFoldingCreateIntConst(compare, code == INPUT_TRUE ? 1 : 0));
        return true;
    }
    if (code == INPUT_ORIG) {
        if (SkipThisPeepholeInOSR(compare, input)) {
            *isOsrBlocked = true;
            return true;
        }
        compare->ReplaceUsers(input);
        return true;
    }
    if (code == INPUT_INV) {
        return TrySimplifyCompareCaseInputInv(compare, input);
    }
    UNREACHABLE();
    return false;
}

// Checks if float const is really an integer const and replace it
static bool TryReplaceFloatConstToIntConst([[maybe_unused]] Inst **castInput, Inst **constInst)
{
    ASSERT(*constInst != nullptr);
    ASSERT(*castInput != nullptr);
    auto type = (*constInst)->CastToConstant()->GetType();
    if (type == DataType::FLOAT32) {
        static constexpr auto MAX_SAFE_FLOAT32 =
            static_cast<float>(static_cast<uint32_t>(1U) << static_cast<uint32_t>(std::numeric_limits<float>::digits));
        float value = (*constInst)->CastToConstant()->GetFloatValue();
        if (value < -MAX_SAFE_FLOAT32 || value > MAX_SAFE_FLOAT32) {
            // Not applicable to numbers out of unchangeable f32->i32 range
            return false;
        }
        if (std::trunc(value) != value) {
            return false;
        }
        auto *graph = (*constInst)->GetBasicBlock()->GetGraph();
        auto *newCnst = graph->FindOrCreateConstant(static_cast<int32_t>(value));
        *constInst = newCnst;
        *castInput = (*castInput)->GetInput(0).GetInst();
        return true;
    }
    if (type == DataType::FLOAT64) {
        static constexpr auto MAX_SAFE_FLOAT64 = static_cast<double>(
            static_cast<uint64_t>(1U) << static_cast<uint64_t>(std::numeric_limits<double>::digits));
        double value = (*constInst)->CastToConstant()->GetDoubleValue();
        if (value < -MAX_SAFE_FLOAT64 || value > MAX_SAFE_FLOAT64) {
            // Not applicable to numbers out of unchangeable f64->i64 range
            return false;
        }
        if (std::trunc(value) != value) {
            return false;
        }
        auto *graph = (*constInst)->GetBasicBlock()->GetGraph();
        auto *newCnst = graph->FindOrCreateConstant(static_cast<int64_t>(value));
        *constInst = newCnst;
        auto *candidateInput = (*castInput)->GetInput(0).GetInst();
        if ((value < static_cast<double>(INT32_MIN) || value > static_cast<double>(INT32_MAX)) &&
            candidateInput->GetType() != DataType::INT64) {
            // In i32 comparison constant will be truncated to i32
            // If it is more than INT32 range, it is necessary to insert cast i32 -> i64
            auto *newCast =
                graph->CreateInstCast(DataType::INT64, INVALID_PC, candidateInput, candidateInput->GetType());
            (*castInput)->InsertAfter(newCast);
            *castInput = newCast;
        } else {
            *castInput = candidateInput;
        }
        return true;
    }
    return false;
}

// 6.i32  Cmp        v5, v1
// 7.b    Compare    v6, 0
// 9.     IfImm NE b v7, 0x0
// =======>
// 6.i32  Cmp        v5, v1
// 7.b    Compare    v5, v1
// 9.     IfImm NE b v7, 0x0
bool Peepholes::TrySimplifyCmpCompareWithZero(Inst *inst, bool *isOsrBlocked)
{
    ASSERT(isOsrBlocked != nullptr);
    auto compare = inst->CastToCompare();
    if (compare->GetBasicBlock()->GetGraph()->IsBytecodeOptimizer()) {
        return false;
    }
    bool swap = false;
    Inst *input = nullptr;
    ConstantInst *constInput = nullptr;
    if (!GetInputsOfCompareWithConst(compare, &input, &constInput, &swap)) {
        return false;
    }
    if (input->GetOpcode() != Opcode::Cmp) {
        return false;
    }
    if (!constInput->IsEqualConstAllTypes(0)) {
        return false;
    }
    auto input0 = input->GetInput(0).GetInst();
    auto input1 = input->GetInput(1).GetInst();
    if (SkipThisPeepholeInOSR(compare, input0) || SkipThisPeepholeInOSR(compare, input1)) {
        *isOsrBlocked = true;
        return true;
    }
    auto cmpOpType = input->CastToCmp()->GetOperandsType();
    if (IsFloatType(cmpOpType)) {
        ASSERT(compare->GetOperandsType() == DataType::INT32);
        if (!TrySimplifyFloatCmpCompare(&input0, &input1, &cmpOpType, input, &swap)) {
            return false;
        }
    }
    ConditionCode cc = swap ? SwapOperandsConditionCode(compare->GetCc()) : compare->GetCc();
    if (!IsTypeSigned(cmpOpType)) {
        ASSERT(cc == ConditionCode::CC_EQ || cc == ConditionCode::CC_NE || IsSignedConditionCode(cc));
        // If Cmp operands are unsigned then Compare.CC must be converted to unsigned.
        cc = InverseSignednessConditionCode(cc);
    }
    compare->SetInput(0, input0);
    compare->SetInput(1, input1);
    compare->SetOperandsType(cmpOpType);
    compare->SetCc(cc);
    return true;
}

bool Peepholes::TrySimplifyFloatCmpCompare(Inst **input0, Inst **input1, DataType::Type *cmpOpType, Inst *compareInput,
                                           bool *swap)
{
    if (CheckFcmpInputs(*input0, *input1)) {
        *input0 = (*input0)->GetInput(0).GetInst();
        *input1 = (*input1)->GetInput(0).GetInst();
        *cmpOpType = DataType::INT32;
    } else if (CheckFcmpWithConstInput(*input0, *input1)) {
        bool cmpSwap = false;
        Inst *cmpCastInput = nullptr;
        ConstantInst *cmpConstInput = nullptr;
        if (!GetInputsOfCompareWithConst(compareInput, &cmpCastInput, &cmpConstInput, swap)) {
            return false;
        }
        Inst *cmpConstInst = static_cast<Inst *>(cmpConstInput);
        if (!TryReplaceFloatConstToIntConst(&cmpCastInput, &cmpConstInst)) {
            return false;
        }
        *input0 = cmpCastInput;
        *input1 = cmpConstInst;
        *cmpOpType = (*input0)->GetType();
        *swap = (*swap) ^ cmpSwap;
    } else {
        return false;
    }
    return true;
}

bool Peepholes::TrySimplifyTestEqualInputs(Inst *inst)
{
    auto cmpInst = inst->CastToCompare();
    if (cmpInst->GetCc() != ConditionCode::CC_TST_EQ && cmpInst->GetCc() != ConditionCode::CC_TST_NE) {
        return false;
    }
    auto input0 = inst->GetInput(0).GetInst();
    auto input1 = inst->GetInput(1).GetInst();
    if (input0 != input1) {
        return false;
    }
    if (cmpInst->GetCc() == ConditionCode::CC_TST_EQ) {
        cmpInst->SetCc(ConditionCode::CC_EQ);
    } else {
        cmpInst->SetCc(ConditionCode::CC_NE);
    }
    // We create constant, so we don't need to check SaveStateOSR between insts
    cmpInst->SetInput(1, ConstFoldingCreateIntConst(input1, 0));
    return true;
}

bool Peepholes::TrySimplifyCompareAndZero(Inst *inst, bool *isOsrBlocked)
{
    ASSERT(isOsrBlocked != nullptr);
    if (inst->GetBasicBlock()->GetGraph()->IsBytecodeOptimizer()) {
        return false;
    }
    auto cmpInst = inst->CastToCompare();
    auto cc = cmpInst->GetCc();
    if (cc != CC_EQ && cc != CC_NE) {
        return false;
    }
    bool swap = false;
    Inst *input = nullptr;
    ConstantInst *constInput = nullptr;
    if (!GetInputsOfCompareWithConst(cmpInst, &input, &constInput, &swap)) {
        return false;
    }
    if (input->GetOpcode() != Opcode::And || !input->HasSingleUser() || !constInput->IsEqualConstAllTypes(0)) {
        return false;
    }
    // 2.i32 And                  v0, v1
    // 3.i32 Constant             0x0
    // 4.b   Compare CC_EQ/CC_NE  v2, v3
    // =======>
    // 4.b   Compare CC_TST_EQ/CC_TST_NE  v0, v1

    if (SkipThisPeepholeInOSR(cmpInst, input->GetInput(0).GetInst())) {
        *isOsrBlocked = true;
        return true;
    }
    cmpInst->SetCc(cc == CC_EQ ? CC_TST_EQ : CC_TST_NE);
    cmpInst->SetInput(0, input->GetInput(0).GetInst());
    cmpInst->SetInput(1, input->GetInput(1).GetInst());
    return true;
}

bool Peepholes::TrySimplifyCompareLenArrayWithZero(Inst *inst)
{
    auto compare = inst->CastToCompare();
    bool swap = false;
    Inst *input = nullptr;
    ConstantInst *constInput = nullptr;
    if (!GetInputsOfCompareWithConst(compare, &input, &constInput, &swap)) {
        return false;
    }
    if (input->GetOpcode() != Opcode::LenArray || !constInput->IsEqualConstAllTypes(0)) {
        return false;
    }

    ConditionCode cc = swap ? SwapOperandsConditionCode(compare->GetCc()) : compare->GetCc();
    if (cc == CC_GE || cc == CC_LT) {
        // We create constant, so we don't need to check SaveStateOSR between insts
        compare->ReplaceUsers(ConstFoldingCreateIntConst(compare, cc == CC_GE ? 1 : 0));
        return true;
    }
    return false;
}

// Try to combine constants when arithmetic operations with constants are repeated
template <typename T>
bool Peepholes::TryCombineConst(Inst *inst, ConstantInst *cnst1, T combine, bool *isOsrBlocked)
{
    ASSERT(isOsrBlocked != nullptr);
    auto input0 = inst->GetInput(0).GetInst();
    auto previnput1 = input0->GetInput(1).GetInst();
    if (previnput1->IsConst() && inst->GetType() == input0->GetType()) {
        if (SkipThisPeepholeInOSR(inst, input0->GetInput(0).GetInst())) {
            *isOsrBlocked = true;
            return false;
        }
        auto cnst2 = static_cast<ConstantInst *>(previnput1);
        auto graph = inst->GetBasicBlock()->GetGraph();
        ConstantInst *newCnst = nullptr;
        switch (DataType::GetCommonType(cnst1->GetType())) {
            case DataType::INT64:
                newCnst = ConstFoldingCreateIntConst(inst, combine(cnst1->GetIntValue(), cnst2->GetIntValue()));
                break;
            case DataType::FLOAT32:
                newCnst = graph->FindOrCreateConstant(combine(cnst1->GetFloatValue(), cnst2->GetFloatValue()));
                break;
            case DataType::FLOAT64:
                newCnst = graph->FindOrCreateConstant(combine(cnst1->GetDoubleValue(), cnst2->GetDoubleValue()));
                break;
            default:
                UNREACHABLE();
        }
        inst->SetInput(0, input0->GetInput(0).GetInst());
        inst->SetInput(1, newCnst);
        return true;
    }
    return false;
}

bool Peepholes::TryCombineAddSubConst(Inst *inst, ConstantInst *cnst1, bool *isOsrBlocked)
{
    ASSERT(isOsrBlocked != nullptr);
    auto opc = inst->GetOpcode();
    ASSERT(opc == Opcode::Add || opc == Opcode::Sub);
    auto input0 = inst->GetInput(0).GetInst();
    auto combine = [&opc, &input0](auto x, auto y) { return opc == input0->GetOpcode() ? x + y : x - y; };
    return TryCombineConst(inst, cnst1, combine, isOsrBlocked);
}

bool Peepholes::TryCombineShiftConst(Inst *inst, ConstantInst *cnst1, bool *isOsrBlocked)
{
    ASSERT(isOsrBlocked != nullptr);
    auto opc = inst->GetOpcode();
    ASSERT(opc == Opcode::Shl || opc == Opcode::Shr || opc == Opcode::AShr);

    auto input0 = inst->GetInput(0).GetInst();
    auto previnput1 = input0->GetInput(1).GetInst();
    if (!(previnput1->IsConst() && inst->GetType() == input0->GetType())) {
        return false;
    }
    auto graph = inst->GetBasicBlock()->GetGraph();
    uint64_t sizeMask = DataType::GetTypeSize(inst->GetType(), graph->GetArch()) - 1;
    auto cnst2 = static_cast<ConstantInst *>(previnput1);
    auto newValue = (cnst1->GetIntValue() & sizeMask) + (cnst2->GetIntValue() & sizeMask);
    // If new_value > size_mask, result is always 0 for Shr and Shl,
    // and 0 or -1 (depending on highest bit of input) for AShr
    if (newValue <= sizeMask || opc == Opcode::AShr) {
        if (SkipThisPeepholeInOSR(inst, input0->GetInput(0).GetInst())) {
            *isOsrBlocked = true;
            return false;
        }
        auto newCnst = ConstFoldingCreateIntConst(inst, std::min(newValue, sizeMask));
        inst->SetInput(0, input0->GetInput(0).GetInst());
        inst->SetInput(1, newCnst);
        return true;
    }
    auto newCnst = ConstFoldingCreateIntConst(inst, 0);
    inst->ReplaceUsers(newCnst);
    return true;
}

bool Peepholes::TryCombineMulConst(Inst *inst, ConstantInst *cnst1, bool *isOsrBlocked)
{
    ASSERT(isOsrBlocked != nullptr);
    ASSERT(inst->GetOpcode() == Opcode::Mul);
    auto combine = [](auto x, auto y) { return x * y; };
    return TryCombineConst(inst, cnst1, combine, isOsrBlocked);
}

bool Peepholes::GetInputsOfCompareWithConst(const Inst *inst, Inst **input, ConstantInst **constInput,
                                            bool *inputsSwapped)
{
    if (inst->GetOpcode() == Opcode::Compare || inst->GetOpcode() == Opcode::Cmp) {
        if (inst->GetInput(1).GetInst()->IsConst()) {
            *input = inst->GetInput(0).GetInst();
            *constInput = inst->GetInput(1).GetInst()->CastToConstant();
            *inputsSwapped = false;
            return true;
        }
        if (inst->GetInput(0).GetInst()->IsConst()) {
            *input = inst->GetInput(1).GetInst();
            *constInput = inst->GetInput(0).GetInst()->CastToConstant();
            *inputsSwapped = true;
            return true;
        }
    }
    return false;
}

Inst *GenerateXorWithOne(BasicBlock *block, Inst *ifImmInput)
{
    auto graph = block->GetGraph();
    auto xorInst = graph->CreateInstXor(DataType::BOOL, block->GetGuestPc());
    xorInst->SetInput(0, ifImmInput);
    Inst *oneConst = nullptr;
    if (graph->IsBytecodeOptimizer()) {
        oneConst = graph->FindOrCreateConstant<uint32_t>(1);
    } else {
        oneConst = graph->FindOrCreateConstant<uint64_t>(1);
    }
    xorInst->SetInput(1, oneConst);
    // We can add inst "xor" before SaveStateOSR in BasicBlock
    block->PrependInst(xorInst);
    return xorInst;
}

std::optional<bool> IsBoolPhiInverted(PhiInst *phi, IfImmInst *ifImm)
{
    auto phiInput0 = phi->GetInput(0).GetInst();
    auto phiInput1 = phi->GetInput(1).GetInst();
    if (!phiInput0->IsBoolConst() || !phiInput1->IsBoolConst()) {
        return std::nullopt;
    }
    auto constant0 = phiInput0->CastToConstant()->GetRawValue();
    auto constant1 = phiInput1->CastToConstant()->GetRawValue();
    if (constant0 == constant1) {
        return std::nullopt;
    }
    // Here constant0 and constant1 are 0 and 1 in some order

    auto invertedIf = IsIfInverted(phi->GetBasicBlock(), ifImm);
    if (invertedIf == std::nullopt) {
        return std::nullopt;
    }
    // constant0 is also index of phi input equal to 0
    if (phi->GetPhiInputBbNum(constant0) == 0) {
        return !*invertedIf;
    }
    return invertedIf;
}

bool Peepholes::TryEliminatePhi(PhiInst *phi)
{
    if (phi->GetInputsCount() != MAX_SUCCS_NUM) {
        return false;
    }

    auto bb = phi->GetBasicBlock();
    auto dom = bb->GetDominator();
    if (dom->IsEmpty()) {
        return false;
    }
    auto last = dom->GetLastInst();
    if (last->GetOpcode() != Opcode::IfImm) {
        return false;
    }

    auto graph = dom->GetGraph();
    auto ifImm = last->CastToIfImm();
    auto input = ifImm->GetInput(0).GetInst();
    // In case of the bytecode optimizer we can not generate Compare therefore we check that Peepholes has eliminated
    // Compare
    if (graph->IsBytecodeOptimizer() && input->GetOpcode() == Opcode::Compare) {
        return false;
    }
    if (input->GetType() != DataType::BOOL ||
        GetTypeSize(phi->GetType(), graph->GetArch()) != GetTypeSize(input->GetType(), graph->GetArch())) {
        return false;
    }
    auto inverted = IsBoolPhiInverted(phi, ifImm);
    if (!inverted) {
        return false;
    }
    if (*inverted) {
        // 0. Const 0
        // 1. Const 1
        // 2. v2
        // 3. IfImm NE b v2, 0x0
        // 4. Phi v0, v1 -> v5, ...
        // ===>
        // 0. Const 0
        // 1. Const 1
        // 2. v2
        // 3. IfImm NE b v2, 0x0
        // 6. Xor v2, v1 -> v5, ...
        // 4. Phi v0, v1

        // "xori"(xor) will insert as first inst in BB, so enough check between first inst and input
        if (SkipThisPeepholeInOSR(phi, input)) {
            return false;
        }
        auto xori = GenerateXorWithOne(phi->GetBasicBlock(), input);
        phi->ReplaceUsers(xori);
    } else {
        // 0. Const 0
        // 1. Const 1
        // 2. v2
        // 3. IfImm NE b v2, 0x0
        // 4. Phi v1, v0 -> v5, ...
        // ===>
        // 0. Const 0
        // 1. Const 1
        // 2. v2 -> v5, ...
        // 3. IfImm NE b v2, 0x0
        // 4. Phi v1, v0

        if (SkipThisPeepholeInOSR(phi, input)) {
            return false;
        }
        phi->ReplaceUsers(input);
    }
    return true;
}

bool Peepholes::SkipThisPeepholeInOSR(Inst *inst, Inst *newInput)
{
    auto osr = inst->GetBasicBlock()->GetGraph()->IsOsrMode();
    return osr && newInput->GetOpcode() != Opcode::Constant && IsInstInDifferentBlocks(inst, newInput);
}

void Peepholes::VisitGetInstanceClass(GraphVisitor *v, Inst *inst)
{
    auto graph = inst->GetBasicBlock()->GetGraph();
    // AOT: don't fold GetInstanceClass into LoadImmediate(class).
    // In AOT class pointer may not match runtime class pointer, causing INLINE_IC deopt.
    if (!graph->IsJitOrOsrMode()) {
        return;
    }
    auto typeInfo = inst->GetDataFlowInput(0)->GetObjectTypeInfo();
    if (typeInfo && typeInfo.IsExact()) {
        auto klass = typeInfo.GetClass();
        auto bb = inst->GetBasicBlock();
        auto classImm = graph->CreateInstLoadImmediate(DataType::REFERENCE, inst->GetPc(), klass);
        inst->ReplaceUsers(classImm);
        bb->InsertAfter(classImm, inst);
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
    }
}
void Peepholes::VisitLoadAndInitClass(GraphVisitor *v, Inst *inst)
{
    auto graph = inst->GetBasicBlock()->GetGraph();
    if (!graph->IsJitOrOsrMode()) {
        return;
    }
    auto klass = inst->CastToLoadAndInitClass()->GetClass();
    if (klass == nullptr || !graph->GetRuntime()->IsClassInitialized(reinterpret_cast<uintptr_t>(klass))) {
        return;
    }
    auto classImm = graph->CreateInstLoadImmediate(DataType::REFERENCE, inst->GetPc(), klass);
    inst->ReplaceUsers(classImm);
    inst->GetBasicBlock()->InsertAfter(classImm, inst);

    inst->ClearFlag(compiler::inst_flags::NO_DCE);
    PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
}

void Peepholes::VisitInitClass(GraphVisitor *v, Inst *inst)
{
    auto graph = inst->GetBasicBlock()->GetGraph();
    if (!graph->IsJitOrOsrMode()) {
        return;
    }
    auto klass = inst->CastToInitClass()->GetClass();
    if (klass == nullptr || !graph->GetRuntime()->IsClassInitialized(reinterpret_cast<uintptr_t>(klass))) {
        return;
    }
    inst->ClearFlag(compiler::inst_flags::NO_DCE);
    PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
}

void Peepholes::VisitIntrinsic([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    auto intrinsic = inst->CastToIntrinsic();
    // CC-OFFNXT(C_RULE_SWITCH_BRANCH_CHECKER) autogenerated code
    switch (intrinsic->GetIntrinsicId()) {
#include "intrinsics_peephole.inl"
        default: {
            return;
        }
    }
}

void Peepholes::VisitLoadClass(GraphVisitor *v, Inst *inst)
{
    auto graph = inst->GetBasicBlock()->GetGraph();
    if (!graph->IsJitOrOsrMode()) {
        return;
    }
    auto klass = inst->CastToLoadClass()->GetClass();
    if (klass == nullptr) {
        return;
    }
    auto classImm = graph->CreateInstLoadImmediate(DataType::REFERENCE, inst->GetPc(), klass);
    inst->ReplaceUsers(classImm);
    inst->GetBasicBlock()->InsertAfter(classImm, inst);

    inst->ClearFlag(compiler::inst_flags::NO_DCE);
    PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
}

void Peepholes::VisitLoadConstantPool(GraphVisitor *v, Inst *inst)
{
    auto graph = inst->GetBasicBlock()->GetGraph();
    if (!graph->IsJitOrOsrMode()) {
        return;
    }
    auto func = inst->GetInput(0).GetInst();
    void *constantPool = nullptr;
    if (func->IsParameter() && func->CastToParameter()->GetArgNumber() == 0) {
        constantPool = graph->GetRuntime()->GetConstantPool(graph->GetMethod());
    } else {
        CallInst *callerInst = nullptr;
        for (auto &user : inst->GetUsers()) {
            auto userInst = user.GetInst();
            if (userInst->GetOpcode() == Opcode::SaveState &&
                user.GetVirtualRegister().GetVRegType() == VRegType::CONST_POOL) {
                callerInst = userInst->CastToSaveState()->GetCallerInst();
                ASSERT(callerInst != nullptr);
                break;
            }
        }
        if (callerInst == nullptr) {
            return;
        }
        if (auto funcObject = callerInst->GetFunctionObject(); funcObject != 0) {
            constantPool = graph->GetRuntime()->GetConstantPool(funcObject);
        } else {
            constantPool = graph->GetRuntime()->GetConstantPool(callerInst->GetCallMethod());
        }
    }
    if (constantPool == nullptr) {
        return;
    }
    auto constantPoolImm = graph->CreateInstLoadImmediate(DataType::ANY, inst->GetPc(), constantPool,
                                                          LoadImmediateInst::ObjectType::CONSTANT_POOL);
    inst->InsertAfter(constantPoolImm);
    inst->ReplaceUsers(constantPoolImm);

    PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
}

void Peepholes::VisitLoadFromConstantPool(GraphVisitor *v, Inst *inst)
{
    auto graph = inst->GetBasicBlock()->GetGraph();
    // LoadFromConstantPool with string flag may be used for intrinsics inlining, do not optimize too early
    if (!graph->IsUnrollComplete()) {
        return;
    }
    auto constantPool = inst->GetInput(0).GetInst();
    if (constantPool->GetOpcode() != Opcode::LoadImmediate) {
        return;
    }
    auto offset = inst->CastToLoadFromConstantPool()->GetTypeId();
    auto shift = DataType::ShiftByType(DataType::ANY, graph->GetArch());
    uintptr_t mem = constantPool->CastToLoadImmediate()->GetConstantPool() +
                    graph->GetRuntime()->GetArrayDataOffset(graph->GetArch()) + (offset << shift);
    auto load = graph->CreateInstLoadObjFromConst(DataType::ANY, inst->GetPc(), mem);
    inst->InsertAfter(load);
    inst->ReplaceUsers(load);

    PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
}

// Find TypedArray constructor Call follows by NewObject instruction.
// 4.ref NewObject  ... -> (v5,v6,...)
// 5.void CallStatic escompat.TypedArray::<ctor> v4, v2, ...
// Note: Support part of constructors for now, more cases will be supported in the future.
CallInst *Peepholes::FindTypeArrayCtorCall(Inst *newObject)
{
    auto *graph = newObject->GetBasicBlock()->GetGraph();
    auto *runtime = graph->GetRuntime();
    for (auto &user : newObject->GetUsers()) {
        auto *inst = user.GetInst();
        if (inst->GetOpcode() == Opcode::NullCheck) {
            auto call = FindTypeArrayCtorCall(inst);
            if (call != nullptr) {
                return call;
            }
        }

        if (inst->GetOpcode() != Opcode::CallStatic) {
            continue;
        }
        auto call = inst->CastToCallStatic();

        auto callFirstArg = call->GetDataFlowInput(0);
        if (callFirstArg->GetOpcode() != Opcode::NewObject || newObject->GetOpcode() != Opcode::NewObject) {
            continue;
        }

        auto newObjectAsArg = callFirstArg->CastToNewObject();
        auto newObjectInput = newObject->CastToNewObject();
        if (newObjectAsArg != newObjectInput) {
            continue;
        }

        if (runtime->IsMethodTypedArrayCtor(call->GetCallMethod())) {
            return call;
        }
    }

    return nullptr;
}

Inst *Peepholes::GetTypedArraySize(Inst *newInst)
{
    Inst *arraySize = nullptr;
    switch (newInst->GetOpcode()) {
        case Opcode::Constant: {
            auto instType = newInst->GetType();
            if (instType == DataType::INT64 || instType == DataType::INT32) {
                arraySize = newInst;
            }
            break;
        }

        case Opcode::NewArray:
            arraySize = newInst->GetDataFlowInput(newInst->GetInput(NewArrayInst::INDEX_SIZE).GetInst());
            break;

        case Opcode::NewObject: {
            CallInst *callCtor = FindTypeArrayCtorCall(newInst);
            if (callCtor != nullptr) {
                auto initInst = callCtor->GetInput(1).GetInst();
                if (initInst->IsConst() &&
                    (initInst->GetType() == DataType::INT64 || initInst->GetType() == DataType::INT32)) {
                    arraySize = initInst;
                }
            }
            break;
        }

        default:
            break;
    }
    return arraySize;
}

bool Peepholes::CheckGetLengthLoad([[maybe_unused]] Inst *inst)
{
    return CheckTypedArrayLengthObject(inst);
}

// The goal is to replace TypedArray get length LoadObject with actual length
// Case 1
// 1.i64 Constant   0x5 -> (v3,...)
// 2.ref NewObject  ... -> (v3,v4,...)
// 3.void CallStatic escompat.TypedArray::<ctor> v2, v1, ...
// 4.ref NullCheck  v2p, ... -> (v5,...)
// 5.i32 LoadObject escompat.TypedArray.lengthInt v4 -> (v6)
// 6.    USE      v5
// ===>
// 1.i64 Constant   0x5 -> (v3,...)
// 2.ref NewObject  ... -> (v3,v4,...)
// 3.void CallStatic escompat.TypedArray::<ctor> v2, v1, ...
// 4.ref NullCheck  v2p, ... -> (v5,...)
// 5.i32 LoadObject escompat.TypedArray.lengthInt v4 -> (v6)
// 6.    USE      v1

// Case 2
// 1.i64 Constant   0x5 -> (v3,...)
// 2.ref NewArray (size=5) ..., v1, ... -> (v4,...)
// 3.ref NewObject  ... -> (v4,v5,...)
// 4.void CallStatic escompat.TypedArray::<ctor> v3, v2, ...
// 5.ref NullCheck  v3p, ... -> (v6,...)
// 6.i32 LoadObject escompat.TypedArray.lengthInt v5 -> (v7)
// 7.    USE      v6
// ===>
// 1.i64 Constant   0x5 -> (v3,...)
// 2.ref NewArray (size=5) ..., v1, ... -> (v4,...)
// 3.ref NewObject  ... -> (v4,v5,...)
// 4.void CallStatic escompat.TypedArray::<ctor> v3, v2, ...
// 5.ref NullCheck  v3p, ... -> (v6,...)
// 6.i32 LoadObject escompat.TypedArray.lengthInt v5 -> (v7)
// 7.    USE      v1
bool Peepholes::TryOptimizeGetLengthLoadObject(Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::LoadObject);

    if (!CheckGetLengthLoad(inst)) {
        return false;
    }

    auto newObject = inst->GetDataFlowInput(0);
    if (newObject->GetOpcode() != Opcode::NewObject) {
        return false;
    }

    CallInst *callCtor = FindTypeArrayCtorCall(newObject);
    if (callCtor == nullptr) {
        return false;
    }

    auto newArray = callCtor->GetInput(1).GetInst();
    auto arraySize = GetTypedArraySize(newArray);
    if (arraySize == nullptr || !arraySize->IsConst()) {
        return false;
    }

    // We can't restore array_size register if it was defined out of OSR loop
    if (SkipThisPeepholeInOSR(inst, arraySize)) {
        return false;
    }
    inst->ReplaceUsers(arraySize);
    return true;
}

void Peepholes::VisitLoadObject([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::LoadObject);

    auto visitor = static_cast<Peepholes *>(v);

    if (visitor->TryOptimizeGetLengthLoadObject(inst)) {
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
    }

    if (visitor->TryOptimizeBoxedLoadStoreObject(inst)) {
        PEEPHOLE_IS_APPLIED(visitor, inst);
        return;
    }
}

void Peepholes::VisitLoadStatic(GraphVisitor *v, Inst *inst)
{
    if (ConstFoldingLoadStatic(inst)) {
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
    }
}

bool Peepholes::CreateCompareInsteadOfXorAdd(Inst *oldInst)
{
    ASSERT(oldInst->GetOpcode() == Opcode::Xor || oldInst->GetOpcode() == Opcode::Add);
    auto input0 = oldInst->GetInput(0).GetInst();
    [[maybe_unused]] auto input1 = oldInst->GetInput(1).GetInst();

    if (oldInst->GetOpcode() == Opcode::Add) {
        ASSERT(input0->GetOpcode() == Opcode::Neg);
        input0 = input0->GetInput(0).GetInst();
        for (auto &userAdd : oldInst->GetUsers()) {
            if (SkipThisPeepholeInOSR(userAdd.GetInst(), input0)) {
                return false;
            }
        }
    }

    // We shouldn't check on OSR with Xor, because old_inst and cmp_inst is placed one by one
    ASSERT(input0->GetType() == DataType::BOOL && input1->IsConst() && input1->CastToConstant()->GetIntValue() == 1U);
    auto cnst = oldInst->GetBasicBlock()->GetGraph()->FindOrCreateConstant(0);
    auto cmpInst = CreateAndInsertInst(Opcode::Compare, oldInst, input0, cnst);
    cmpInst->SetType(DataType::BOOL);
    cmpInst->CastToCompare()->SetCc(ConditionCode::CC_EQ);
    cmpInst->CastToCompare()->SetOperandsType(DataType::BOOL);
    auto type = oldInst->GetType();
    if (type == DataType::UINT64 || type == DataType::INT64) {
        auto cast = cmpInst->GetBasicBlock()->GetGraph()->CreateInstCast();
        cast->SetType(type);
        cmpInst->InsertAfter(cast);
        cmpInst->ReplaceUsers(cast);
        cast->SetInput(0, cmpInst);
        cast->SetOperandsType(DataType::BOOL);
    }
    return true;
}

// Move users from LenArray to constant which used in MultiArray
// Case 1
// 1.s64 ... ->{v3}
// 2.s64 ... ->{v3}
// 3.ref MultiArray ... , v1, v2, ... ->{v4,..}
// 4.s32 LenArray v3 ->{v5, ...}
// 5.    USE      v5
// ===>
// 1.s64 ... ->{v3, v5, ...}
// 2.s64 ... ->{v3}
// 3.ref MultiArray ... , v1, v2, ... ->{v4,..}
// 4.s32 LenArray v3
// 5.    USE      v1

// Case 2
// 1.s64 ... ->{v3}
// 2.s64 ... ->{v3}
// 3.ref MultiArray ... , v1, v2, ... ->{v4,..}
// 4.ref LoadArray v3, ...
// 5.ref NullCheck v4, ...
// 6.s32 LenArray v5 ->{v7, ...}
// 7.    USE      v6
// ===>
// 1.s64 ... ->{v3}
// 2.s64 ... ->{v3, v7, ...}
// 3.ref MultiArray ... , v1, v2, ... ->{v4,..}
// 4.ref LoadArray v3, ...
// 5.ref NullCheck v4, ...
// 6.s32 LenArray v5
// 7.    USE      v2
bool Peepholes::OptimizeLenArrayForMultiArray(Inst *lenArray, Inst *inst, size_t indexSize)
{
    if (inst->GetOpcode() == Opcode::MultiArray) {
        // Arguments of MultiArray look like : class, size_0, size_1, ..., size_N, SaveState
        // When element type of multyarray is array-like object (LenArray can be applyed to it), we can get case when
        // number sequential LoadArray with LenArray more than dimension of MultiArrays. So limiting the index_size.
        // Example in unittest PeepholesTest.MultiArrayWithLenArrayOfString

        auto multiarrDimension = inst->GetInputsCount() - 2;
        if (!(indexSize < multiarrDimension)) {
            return false;
        }
        // MultiArray's sizes starts from index "1", so need add "1" to get absolute index
        auto value = inst->GetDataFlowInput(indexSize + 1);
        for (auto &it : lenArray->GetUsers()) {
            if (SkipThisPeepholeInOSR(it.GetInst(), value)) {
                return false;
            }
        }
        lenArray->ReplaceUsers(value);
        return true;
    }
    if (inst->GetOpcode() == Opcode::LoadArray) {
        auto input = inst->GetDataFlowInput(0);
        return OptimizeLenArrayForMultiArray(lenArray, input, indexSize + 1);
    }
    return false;
}

bool Peepholes::TryOptimizeLenArrayForPhi(Inst *lenArray, PhiInst *phi)
{
    ASSERT(lenArray->GetDataFlowInput(0) == phi);
    inputs_.clear();
    inputs_.reserve(phi->GetInputsCount());
    for (auto i : phi->GetInputs()) {
        auto input = i.GetInst();
        if (input->GetOpcode() != Opcode::NewArray) {
            return false;
        }
        ASSERT(input->GetDataFlowInput(1) != nullptr);
        inputs_.push_back(input->GetDataFlowInput(1));
    }
    auto newPhi = GetGraph()->CreateInstPhi(lenArray->GetType(), lenArray->GetPc());
    for (auto input : inputs_) {
        newPhi->AppendInput(input);
    }
    lenArray->ReplaceUsers(newPhi);
    phi->GetBasicBlock()->AppendPhi(newPhi);
    lenArray->GetBasicBlock()->RemoveInst(lenArray);
    return true;
}

bool Peepholes::IsNegationPattern(Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::Add);
    // Negetion pattern is:
    // 1.   Constant 0x1
    // 2.b  ...
    // 3.i32 Neg v2 -> v4
    // 4.i32 Add v3, v1
    auto input0 = inst->GetInput(0).GetInst();
    if (input0->GetOpcode() != Opcode::Neg || input0->GetInput(0).GetInst()->GetType() != DataType::BOOL ||
        !input0->HasSingleUser()) {
        return false;
    }
    // We sure, that constant may be only in input1
    auto input1 = inst->GetInput(1).GetInst();
    return input1->GetOpcode() == Opcode::Constant && input1->CastToConstant()->GetIntValue() == 1;
}

bool Peepholes::TrySimplifyNegationPattern(Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::Add);
    if (!IsNegationPattern(inst)) {
        return false;
    }
    auto suspectInst = inst->GetInput(0).GetInst()->GetInput(0).GetInst();
    // Case 8
    // We sure, that type of Neg's input is Bool. We shue, that Neg has one user
    if (suspectInst->GetOpcode() == Opcode::Compare && suspectInst->HasSingleUser()) {
        auto compareInst = suspectInst->CastToCompare();
        bool isPossible = true;
        for (auto &i : inst->GetUsers()) {
            if (SkipThisPeepholeInOSR(i.GetInst(), compareInst)) {
                isPossible = false;
                break;
            }
        }
        if (isPossible) {
            inst->ReplaceUsers(compareInst);
            compareInst->SetCc(GetInverseConditionCode(compareInst->GetCc()));
            PEEPHOLE_IS_APPLIED(this, compareInst);
            return true;
        }
    }

    // Case 9
    if (TrySimplifyCompareNegation(inst)) {
        return true;
    }

    // Case 7
    // This is used last of all if no case has worked!
    if (CreateCompareInsteadOfXorAdd(inst)) {
        PEEPHOLE_IS_APPLIED(this, inst);
        return true;
    }
    return false;
}

bool Peepholes::TryOptimizeBoxedLoadStoreObject(Inst *inst)
{
    // We can allow some optimizations over LoadObject and StoreObject instructions
    // if their types match boxed types.
    auto graph = inst->GetBasicBlock()->GetGraph();
    graph->RunPass<ObjectTypePropagation>();

    auto runtime = graph->GetRuntime();
    auto typeInfo = inst->GetDataFlowInput(0)->GetObjectTypeInfo();
    auto klass = typeInfo.GetClass();
    if (klass != nullptr && runtime->IsBoxedClass(klass)) {
        inst->ClearFlag(compiler::inst_flags::NO_CSE);
        inst->ClearFlag(compiler::inst_flags::NO_HOIST);
        return true;
    }
    return false;
}

bool Peepholes::TryOptimizeXorChain(Inst *inst)
{
    auto xorInput = inst->GetInput(0).GetInst();
    auto xorConstInput = inst->GetInput(1).GetInst();
    if (!xorConstInput->IsConst()) {
        return false;
    }
    // Output of Xor1 should be only used by Xor2
    if (!inst->HasSingleUser()) {
        return false;
    }
    auto nextXor = inst->GetFirstUser()->GetInst();
    if (nextXor->GetOpcode() != Opcode::Xor) {
        return false;
    }
    // Check that both XORs have the same type
    if (inst->GetType() != nextXor->GetType()) {
        return false;
    }
    // Both Xors should use the same constant as input
    if (nextXor->GetInput(0).GetInst() != xorConstInput && nextXor->GetInput(1).GetInst() != xorConstInput) {
        return false;
    }
    bool isPossible = true;
    for (auto &i : nextXor->GetUsers()) {
        if (SkipThisPeepholeInOSR(i.GetInst(), xorInput)) {
            isPossible = false;
            break;
        }
    }
    if (isPossible) {
        // Replace all uses of Xor2 output with the input0 of Xor1
        nextXor->ReplaceUsers(xorInput);
        return true;
    }
    return false;
}

}  // namespace ark::compiler
