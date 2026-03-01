/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "assembler/assembly-literals.h"
#include "const_array_resolver.h"
#include "compiler/optimizer/ir/basicblock.h"
#include "compiler/optimizer/optimizations/peepholes.h"

namespace ark::bytecodeopt {

static constexpr size_t STOREARRAY_INPUTS_NUM = 3;
static constexpr size_t SINGLE_DIM_ARRAY_RANK = 1;
static constexpr size_t MIN_ARRAY_ELEMENTS_AMOUNT = 2;

bool ConstArrayResolver::RunImpl()
{
    if (irInterface_ == nullptr) {
        return false;
    }

    if (!FindConstantArrays()) {
        return false;
    }

    // delete instructions for storing array elements
    RemoveArraysFill();

    // replace old NewArray instructions with new SaveState + LoadConst instructions
    InsertLoadConstArrayInsts();

    return true;
}

static bool IsPatchAllowedOpcode(Opcode opcode)
{
    switch (opcode) {
        case Opcode::StoreArray:
        case Opcode::LoadString:
        case Opcode::Constant:
        case Opcode::Cast:
        case Opcode::SaveState:
            return true;
        default:
            return false;
    }
}

static std::optional<compiler::ConstantInst *> GetConstantIfPossible(Inst *inst)
{
    if (inst->GetOpcode() == Opcode::Cast) {
        auto input = inst->GetInput(0U).GetInst();
        if ((input->GetOpcode() == Opcode::NullPtr) || !input->IsConst()) {
            return std::nullopt;
        }
        auto constantInst = compiler::ConstFoldingCastConst(inst, input, true);
        if (constantInst != nullptr) {
            return constantInst;
        }
    }
    if (inst->IsConst()) {
        return inst->CastToConstant();
    }
    return std::nullopt;
}

std::optional<std::vector<pandasm::LiteralArray::Literal>> ConstArrayResolver::FillLiteralArray(Inst *inst, size_t size)
{
    std::vector<pandasm::LiteralArray::Literal> literals {size};
    std::vector<Inst *> storeInsts;

    auto next = inst->GetNext();
    size_t realSize = 0;

    // are looking for instructions for uninterrupted filling the array
    while (next != nullptr) {
        // check whether the instruction is allowed inside the filling patch
        if (!IsPatchAllowedOpcode(next->GetOpcode())) {
            break;
        }

        // find instructions for storing array elements
        if (next->GetOpcode() != Opcode::StoreArray) {
            next = next->GetNext();
            continue;
        }

        auto storeArrayInst = next->CastToStoreArray();
        if (storeArrayInst == nullptr || storeArrayInst->GetArray() != inst) {
            break;
        }

        // get an index of the inserted element if possible
        auto indexInst = storeArrayInst->GetIndex();
        auto indexConstInst = GetConstantIfPossible(indexInst);
        if (!indexConstInst) {
            return std::nullopt;
        }
        auto index = static_cast<size_t>((*indexConstInst)->GetIntValue());
        if (index >= size) {
            return std::nullopt;
        }

        pandasm::LiteralArray::Literal literal {};
        // create a literal from the array element, if possible
        if (!FillLiteral(storeArrayInst, &literal)) {
            // if not, then we can't create a constant literal array
            return std::nullopt;
        }

        // checks if there is free space on the [index] position in the vector
        pandasm::LiteralArray::Literal defaultLiteral {};
        if (literals[index] == defaultLiteral) {
            realSize++;
        }

        literals[index] = literal;
        storeInsts.push_back(next);
        next = next->GetNext();
    }

    // save the literal array only if it is completely filled
    // or its size exceeds the minimum number of elements to save
    if (realSize != size || storeInsts.size() < MIN_ARRAY_ELEMENTS_AMOUNT) {
        return std::nullopt;
    }

    // save the store instructions for deleting them later
    constArraysFill_.emplace(inst, std::move(storeInsts));
    return std::optional<std::vector<pandasm::LiteralArray::Literal>> {std::move(literals)};
}

void ConstArrayResolver::AddIntroLiterals(pandasm::LiteralArray *ltAr)
{
    // add an element that stores the array size (it will be stored in the first element)
    pandasm::LiteralArray::Literal lenLit;
    lenLit.tag = panda_file::LiteralTag::INTEGER;
    lenLit.value = static_cast<uint32_t>(ltAr->literals.size());
    ltAr->literals.insert(ltAr->literals.begin(), lenLit);

    // add an element that stores the array type (it will be stored in the zero element)
    pandasm::LiteralArray::Literal tagLit;
    tagLit.tag = panda_file::LiteralTag::TAGVALUE;
    tagLit.value = static_cast<uint8_t>(ltAr->literals.back().tag);
    ltAr->literals.insert(ltAr->literals.begin(), tagLit);
}

bool ConstArrayResolver::IsMultidimensionalArray(compiler::NewArrayInst *inst)
{
    auto arrayType = pandasm::Type::FromName(irInterface_->GetTypeIdByOffset(inst->GetTypeId()));
    return arrayType.GetRank() > SINGLE_DIM_ARRAY_RANK;
}

static bool IsSameBB(Inst *inst1, Inst *inst2)
{
    return inst1->GetBasicBlock() == inst2->GetBasicBlock();
}

static bool IsSameBB(Inst *inst, compiler::BasicBlock *bb)
{
    return inst->GetBasicBlock() == bb;
}

bool ConstArrayResolver::FindConstantArrays()
{
    size_t initSize = irInterface_->GetLiteralArrayTableSize();

    for (auto bb : GetGraph()->GetBlocksRPO()) {
        // go through the instructions of the basic block in reverse order
        // until we meet the instruction for storing an array element
        auto inst = bb->GetLastInst();
        while ((inst != nullptr) && IsSameBB(inst, bb)) {
            if (inst->GetOpcode() != Opcode::StoreArray) {
                inst = inst->GetPrev();
                continue;
            }

            // the patch for creating and filling an array should start with the NewArray instruction
            auto arrayInst = inst->CastToStoreArray()->GetArray();
            if (arrayInst->GetOpcode() != Opcode::NewArray) {
                inst = inst->GetPrev();
                continue;
            }
            auto newArrayInst = arrayInst->CastToNewArray();
            // the instructions included in the patch must be in one basic block
            if (!IsSameBB(inst, newArrayInst)) {
                inst = inst->GetPrev();
                continue;
            }

            // NOTE(aantipina): add the ability to save multidimensional arrays
            if (IsMultidimensionalArray(newArrayInst)) {
                inst = IsSameBB(inst, newArrayInst) ? newArrayInst->GetPrev() : inst->GetPrev();
                continue;
            }

            auto arraySizeInst =
                GetConstantIfPossible(newArrayInst->GetInput(compiler::NewArrayInst::INDEX_SIZE).GetInst());
            if (arraySizeInst == std::nullopt) {
                inst = newArrayInst->GetPrev();
                continue;
            }
            auto arraySize = (*arraySizeInst)->CastToConstant()->GetIntValue();
            if (arraySize < MIN_ARRAY_ELEMENTS_AMOUNT) {
                inst = newArrayInst->GetPrev();
                continue;
            }

            // creating a literal array, if possible
            auto rawLiteralArray = FillLiteralArray(newArrayInst, arraySize);
            if (rawLiteralArray == std::nullopt) {
                inst = newArrayInst->GetPrev();
                continue;
            }

            pandasm::LiteralArray literalArray(*rawLiteralArray);

            // save the type and length of the array in the first two elements
            AddIntroLiterals(&literalArray);
            auto id = irInterface_->GetLiteralArrayTableSize();
            irInterface_->StoreLiteralArray(std::to_string(id), std::move(literalArray));

            // save the NewArray instructions for replacing them with LoadConst instructions later
            constArraysInit_.emplace(id, newArrayInst);

            inst = newArrayInst->GetPrev();
        }
    }

    // the pass worked if the size of the literal array table increased
    return initSize < irInterface_->GetLiteralArrayTableSize();
}

void ConstArrayResolver::RemoveArraysFill()
{
    for (const auto &it : constArraysFill_) {
        for (const auto &storeInst : it.second) {
            storeInst->GetBasicBlock()->RemoveInst(storeInst);
        }
    }
}

void ConstArrayResolver::InsertLoadConstArrayInsts()
{
    for (const auto &[id, start_inst] : constArraysInit_) {
        auto method = GetGraph()->GetMethod();
        compiler::LoadConstArrayInst *newInst = GetGraph()->CreateInstLoadConstArray(REFERENCE, start_inst->GetPc());
        newInst->SetTypeId(id);
        newInst->SetMethod(method);

        start_inst->ReplaceUsers(newInst);
        start_inst->RemoveInputs();

        compiler::SaveStateInst *saveState = GetGraph()->CreateInstSaveState();
        saveState->SetPc(start_inst->GetPc());
        saveState->SetMethod(method);
        saveState->ReserveInputs(0U);

        newInst->SetInput(0U, saveState);
        start_inst->InsertBefore(saveState);
        start_inst->GetBasicBlock()->ReplaceInst(start_inst, newInst);
    }
}

static bool FillPrimitiveLiteral(pandasm::LiteralArray::Literal *literal, panda_file::Type::TypeId type,
                                 compiler::ConstantInst *valueInst)
{
    auto tag = pandasm::LiteralArray::GetArrayTagFromComponentType(type);
    literal->tag = tag;
    switch (tag) {
        case panda_file::LiteralTag::ARRAY_U1:
            literal->value = static_cast<bool>(valueInst->GetInt32Value());
            return true;
        case panda_file::LiteralTag::ARRAY_U8:
        case panda_file::LiteralTag::ARRAY_I8:
            literal->value = static_cast<uint8_t>(valueInst->GetInt32Value());
            return true;
        case panda_file::LiteralTag::ARRAY_U16:
        case panda_file::LiteralTag::ARRAY_I16:
            literal->value = static_cast<uint16_t>(valueInst->GetInt32Value());
            return true;
        case panda_file::LiteralTag::ARRAY_U32:
        case panda_file::LiteralTag::ARRAY_I32:
            literal->value = valueInst->GetInt32Value();
            return true;
        case panda_file::LiteralTag::ARRAY_U64:
        case panda_file::LiteralTag::ARRAY_I64:
            literal->value = valueInst->GetInt64Value();
            return true;
        case panda_file::LiteralTag::ARRAY_F32:
            literal->value = valueInst->GetFloatValue();
            return true;
        case panda_file::LiteralTag::ARRAY_F64:
            literal->value = valueInst->GetDoubleValue();
            return true;
        default:
            UNREACHABLE();
    }
    return false;
}

bool ConstArrayResolver::FillLiteral(compiler::StoreInst *storeArrayInst, pandasm::LiteralArray::Literal *literal)
{
    if (storeArrayInst->GetInputsCount() > STOREARRAY_INPUTS_NUM) {
        return false;
    }
    auto rawElemInst = storeArrayInst->GetStoredValue();
    auto newArrayInst = storeArrayInst->GetArray();

    auto arrayType =
        pandasm::Type::FromName(irInterface_->GetTypeIdByOffset(newArrayInst->CastToNewArray()->GetTypeId()));
    auto componentType = arrayType.GetComponentType();

    if (arrayType.IsArrayContainsPrimTypes()) {
        auto valueInst = GetConstantIfPossible(rawElemInst);
        if (valueInst == std::nullopt) {
            return false;
        }
        return FillPrimitiveLiteral(literal, componentType.GetId(), *valueInst);
    }

    auto componentTypeDescriptor = componentType.GetDescriptor();
    auto stringDescriptor = ark::panda_file::GetStringClassDescriptor(irInterface_->GetSourceLang());
    if ((rawElemInst->GetOpcode() == Opcode::LoadString) && (componentTypeDescriptor == stringDescriptor)) {
        literal->tag = panda_file::LiteralTag::ARRAY_STRING;
        std::string stringValue = irInterface_->GetStringIdByOffset(rawElemInst->CastToLoadString()->GetTypeId());
        literal->value = stringValue;
        return true;
    }

    return false;
}

}  // namespace ark::bytecodeopt
