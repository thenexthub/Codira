/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
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

#include "compiler_logger.h"
#include "select_optimization.h"

#include "optimizer/ir/basicblock.h"
#include "optimizer/analysis/alias_analysis.h"
#include "optimizer/analysis/bounds_analysis.h"
#include "optimizer/analysis/loop_analyzer.h"

namespace ark::compiler {
bool SelectOptimization::RunImpl()
{
    COMPILER_LOG(DEBUG, SELECT_OPT) << "Run " << GetPassName();
    bool result = false;

    for (BasicBlock *block : GetGraph()->GetBlocksRPO()) {
        sameOperandsMap_.clear();
        for (Inst *inst : block->Insts()) {
            if (inst->GetOpcode() != Opcode::Select && inst->GetOpcode() != Opcode::SelectImm) {
                continue;
            }
            result |= TryOptimizeSelectInst(inst);
        }
    }

    COMPILER_LOG(DEBUG, SELECT_OPT) << GetPassName() << " complete";
    return result;
}

bool SelectOptimization::TryOptimizeSelectInst(Inst *selectInst)
{
    Inst *v0 = selectInst->GetInput(0).GetInst();
    Inst *v1 = selectInst->GetInput(1).GetInst();

    bool result = false;
    if (selectInst->HasUsers()) {
        result |= TryOptimizeSelectInstWithSameOperands(selectInst, v0, v1);
    }
    if (selectInst->HasUsers()) {
        result |= TryOptimizeSelectInstWithConstantBoolOperands(selectInst, v0, v1);
    }

    return result;
}

// Check if raw `value` can be store as `type`
bool RawValueFits(Arch arch, uint64_t value, DataType::Type type)
{
    ASSERT(DataType::IsInt32Bit(type));

    uint64_t typeSize = DataType::GetTypeSize(type, arch);
    return value < (1ULL << typeSize);
}

// Check if `inst` value can be store as `type`
bool InstTypeFits(Arch arch, Inst *inst, DataType::Type type)
{
    if (inst->IsConst()) {
        return RawValueFits(arch, inst->CastToConstant()->GetRawValue(), type);
    }

    ASSERT(DataType::IsInt32Bit(type));
    ASSERT(DataType::IsInt32Bit(inst->GetType()));

    auto typeByteSize = DataType::GetTypeByteSize(type, arch);
    auto instTypeByteSize = DataType::GetTypeByteSize(inst->GetType(), arch);
    return instTypeByteSize <= typeByteSize;
}

// Case 1: Same inputs within one instruction
//   3. Select[Imm] CC v0, v0, vX, vY|Imm
//   ===>
//   Replace v3 with v0
// Case 2: Two instructions with pairwise the same inputs
//    3. Select[Imm] CC v0, v1, vX, vY|Imm
//    4. Select[Imm] CC v0, v1, vX, vY|Imm
//    ===>
//    Replace v4 with v3
bool SelectOptimization::TryOptimizeSelectInstWithSameOperands(Inst *selectInst, Inst *v0, Inst *v1)
{
    ASSERT(selectInst->HasUsers() &&
           (selectInst->GetOpcode() == Opcode::Select || selectInst->GetOpcode() == Opcode::SelectImm));

    // Case 1
    if (v0 == v1) {
        selectInst->ReplaceUsers(v0);
        return true;
    }

    // Case 2
    uint64_t const3 = 0;  // last operand is either const, or immediate
    ConditionCode cc = ConditionCode::CC_EQ;
    DataType::Type operandsType = DataType::NO_TYPE;

    if (selectInst->GetOpcode() == Opcode::Select) {
        auto v3 = selectInst->GetInput(3U).GetInst();
        if (!v3->IsConst()) {
            return false;
        }

        const3 = v3->CastToConstant()->GetRawValue();
        cc = selectInst->CastToSelect()->GetCc();
        operandsType = selectInst->CastToSelect()->GetOperandsType();
    }

    if (selectInst->GetOpcode() == Opcode::SelectImm) {
        const3 = selectInst->CastToSelectImm()->GetImm();
        cc = selectInst->CastToSelectImm()->GetCc();
        operandsType = selectInst->CastToSelectImm()->GetOperandsType();
    }

    if (!DataType::IsInt32Bit(selectInst->GetType())) {
        return false;
    }

    if (!DataType::IsInt32Bit(operandsType)) {
        return false;
    }

    return TryOptimizeSelectInstWithSameOperandsChecked(selectInst, v0, v1, const3, cc);
}

bool SelectOptimization::TryOptimizeSelectInstWithSameOperandsChecked(Inst *selectInst, Inst *v0, Inst *v1,
                                                                      uint64_t const3, ConditionCode cc)
{
    // The optimization below works on SelectImm instruction or Select instruction with constant last input
    // 3. Constant 0x0
    // 4. Select CC v0, v1, v2, v3
    // 5. SelectImm CC v0, v1, v2, 0x0
    // Instruction v4 and v5 are considered equivalent

    auto v2 = selectInst->GetInput(2U).GetInst();
    Key key = {{v0, v1, v2}, const3, cc};

    auto found = sameOperandsMap_.find(key);
    if (found != sameOperandsMap_.end()) {
        auto foundInst = found->second;
        ASSERT(foundInst->GetBasicBlock() == selectInst->GetBasicBlock());
        ASSERT(foundInst->IsDominate(selectInst));

        // Check if operands type is not wider than instruction type
        if (InstTypeFits(GetGraph()->GetArch(), v0, selectInst->GetType()) &&
            InstTypeFits(GetGraph()->GetArch(), v1, selectInst->GetType())) {
            // Move `selectInst` until it dominates all user of `foundInst`
            selectInst->GetBasicBlock()->EraseInst(selectInst);
            foundInst->InsertAfter(selectInst);

            foundInst->ReplaceUsers(selectInst);
            return true;
        }
    } else {
        sameOperandsMap_[key] = selectInst;
    }

    return false;
}

// Case: (boolean constant operands)
//   0. Constant
//   1. Constant
//   3. Constant
//   4. Select[Imm] EQ|NE v0, v1, v2, v3|Imm
//   ===>
//   Replace v4 with v2 if possible
bool SelectOptimization::TryOptimizeSelectInstWithConstantBoolOperands(Inst *selectInst, Inst *v0, Inst *v1)
{
    ASSERT(selectInst->HasUsers() &&
           (selectInst->GetOpcode() == Opcode::Select || selectInst->GetOpcode() == Opcode::SelectImm));

    if (!v0->IsConst() || !v1->IsConst()) {
        return false;
    }

    if (selectInst->GetType() != DataType::BOOL) {
        return false;
    }

    uint64_t const3 = 0;  // last operand is either const, or immediate
    ConditionCode cc = ConditionCode::CC_EQ;
    DataType::Type operandsType = DataType::NO_TYPE;

    if (selectInst->GetOpcode() == Opcode::Select) {
        auto v3 = selectInst->GetInput(3U).GetInst();
        if (!v3->IsConst()) {
            return false;
        }

        const3 = v3->CastToConstant()->GetRawValue();
        cc = selectInst->CastToSelect()->GetCc();
        operandsType = selectInst->CastToSelect()->GetOperandsType();
    }

    if (selectInst->GetOpcode() == Opcode::SelectImm) {
        const3 = selectInst->CastToSelectImm()->GetImm();
        cc = selectInst->CastToSelectImm()->GetCc();
        operandsType = selectInst->CastToSelectImm()->GetOperandsType();
    }

    if (operandsType != DataType::BOOL) {
        return false;
    }

    auto const0 = v0->CastToConstant()->GetRawValue();
    auto const1 = v1->CastToConstant()->GetRawValue();
    auto v2 = selectInst->GetInput(2U).GetInst();

    return TryOptimizeSelectInstWithConstantBoolOperandsChecked(selectInst, cc, const0, const1, v2, const3);
}

Inst *SelectOptimization::FindOrCreateXorI(Inst *selectInst, Inst *arg, uint64_t imm)
{
    // Search for `XorI arg, imm` instruction within current block
    for (auto &user : arg->GetUsers()) {
        auto userInst = user.GetInst();
        if (userInst->GetOpcode() != Opcode::XorI) {
            continue;
        }
        if (userInst->GetBasicBlock() != selectInst->GetBasicBlock()) {
            continue;
        }
        if (userInst->GetType() != selectInst->GetType()) {
            continue;
        }
        auto xori = userInst->CastToXorI();
        if (xori->GetImm() != imm) {
            continue;
        }

        return xori;
    }

    // Create new if not found
    return GetGraph()->CreateInstXorI(selectInst, arg, imm);
}

void SelectOptimization::ReplaceUsersWithInvert(Inst *selectInst, Inst *arg)
{
    auto xori = FindOrCreateXorI(selectInst, arg, 0x1);
    if (xori->GetBasicBlock() == nullptr) {
        // Found new XorI instruction
        selectInst->InsertBefore(xori);
    } else if (!xori->IsDominate(selectInst)) {
        // Found existing XorI instruction, move if it does not dominate.
        ASSERT(selectInst->GetBasicBlock() == xori->GetBasicBlock());
        xori->GetBasicBlock()->EraseInst(xori);
        selectInst->InsertBefore(xori);
    }
    selectInst->ReplaceUsers(xori);
}

// CC-OFFNXT(huge_cyclomatic_complexity[C], huge_method[C], G.FUN.01-CPP) big switch case
bool SelectOptimization::TryOptimizeSelectInstWithConstantBoolOperandsChecked(Inst *selectInst, ConditionCode cc,
                                                                              uint64_t const0, uint64_t const1,
                                                                              Inst *v2, uint64_t const3)
{
    switch (cc) {
        case ConditionCode::CC_EQ:
            // Case: (v2 == false) ? false : true
            //   0. Constant 0x0
            //   1. Constant 0x1
            //   4. SelectImm EQ v0, v1, v2, 0x0
            //  or
            //   0. Constant 0x0
            //   1. Constant 0x1
            //   3. Constant 0x0
            //   4. Select EQ v0, v1, v2, v3
            //   ===>
            //   Replace v4 with v2
            if (const3 == 0 && const0 == 0 && const1 != 0) {
                selectInst->ReplaceUsers(v2);
                return true;
            }

            // Case: (v2 == true) ? true : false
            //   0. Constant 0x1
            //   1. Constant 0x0
            //   4. SelectImm EQ v0, v1, v2, 0x1
            //  or
            //   0. Constant 0x1
            //   1. Constant 0x0
            //   3. Constant 0x1
            //   4. Select EQ v0, v1, v2, v3
            //   ===>
            //   Replace v4 with v2
            if (const3 != 0 && const0 != 0 && const1 == 0) {
                selectInst->ReplaceUsers(v2);
                return true;
            }

            // Case: (v2 == false) ? true : false
            //   0. Constant 0x1
            //   1. Constant 0x0
            //   4. SelectImm EQ v0, v1, v2, 0x0
            //  or
            //   0. Constant 0x1
            //   1. Constant 0x0
            //   3. Constant 0x0
            //   4. Select EQ v0, v1, v2, v3
            //   ===>
            //   Replace v4 with XorI v2, 0x1
            if (const3 == 0 && const0 != 0 && const1 == 0) {
                ReplaceUsersWithInvert(selectInst, v2);
                return true;
            }

            // Case: (v2 == true) ? false : true
            //   0. Constant 0x0
            //   1. Constant 0x1
            //   4. SelectImm EQ v0, v1, v2, 0x1
            //  or
            //   0. Constant 0x0
            //   1. Constant 0x1
            //   3. Constant 0x1
            //   4. Select EQ v0, v1, v2, v3
            //   ===>
            //   Replace v4 with XorI v2, 0x1
            if (const3 != 0 && const0 == 0 && const1 != 0) {
                ReplaceUsersWithInvert(selectInst, v2);
                return true;
            }

            break;
        case ConditionCode::CC_NE:
            // Case: (v2 != false) ? true : false
            //   0. Constant 0x1
            //   1. Constant 0x0
            //   4. SelectImm NE v0, v1, v2, 0x0
            //  or
            //   0. Constant 0x1
            //   1. Constant 0x0
            //   3. Constant 0x0
            //   4. Select NE v0, v1, v2, v3
            //   ===>
            //   Replace v4 with v2
            if (const3 == 0 && const0 != 0 && const1 == 0) {
                selectInst->ReplaceUsers(v2);
                return true;
            }

            // Case: (v2 != true) ? false : true
            //   0. Constant 0x0
            //   1. Constant 0x1
            //   4. SelectImm NE v0, v1, v2, 0x1
            //  or
            //   0. Constant 0x0
            //   1. Constant 0x1
            //   3. Constant 0x1
            //   4. Select NE v0, v1, v2, v3
            //   ===>
            //   Replace v4 with v2
            if (const3 != 0 && const0 == 0 && const1 != 0) {
                selectInst->ReplaceUsers(v2);
                return true;
            }

            // Case: (v2 != false) ? false : true
            //   0. Constant 0x0
            //   1. Constant 0x1
            //   4. SelectImm NE v0, v1, v2, 0x0
            //  or
            //   0. Constant 0x0
            //   1. Constant 0x1
            //   3. Constant 0x0
            //   4. Select NE v0, v1, v2, v3
            //   ===>
            //   Replace v4 with XorI v2, 0x1
            if (const3 == 0 && const0 == 0 && const1 != 0) {
                ReplaceUsersWithInvert(selectInst, v2);
                return true;
            }

            // Case: (v2 != true) ? true : false
            //   0. Constant 0x1
            //   1. Constant 0x0
            //   4. SelectImm NE v0, v1, v2, 0x1
            //  or
            //   0. Constant 0x1
            //   1. Constant 0x0
            //   3. Constant 0x1
            //   4. Select NE v0, v1, v2, v3
            //   ===>
            //   Replace v4 with XorI v2, 0x1
            if (const3 != 0 && const0 != 0 && const1 == 0) {
                ReplaceUsersWithInvert(selectInst, v2);
                return true;
            }

            break;
        default:
            break;
    }

    return false;
}

void SelectOptimization::InvalidateAnalyses()
{
    GetGraph()->InvalidateAnalysis<AliasAnalysis>();
    GetGraph()->InvalidateAnalysis<BoundsAnalysis>();
}

bool SelectOptimization::Key::operator<(const SelectOptimization::Key &other) const
{
    if (constant < other.constant) {
        return true;
    }
    if (constant > other.constant) {
        return false;
    }
    if (cc < other.cc) {
        return true;
    }
    if (cc > other.cc) {
        return false;
    }
    if (args < other.args) {
        return true;
    }
    if (args > other.args) {
        return false;
    }
    return false;
}
}  // namespace ark::compiler
