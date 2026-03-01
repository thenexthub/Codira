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

#include "abc_code_processor.h"
#include "abc_field_processor.h"
#include "abc_method_processor.h"
#include "assembly-record.h"
#include <libarkfile/include/source_lang_enum.h>

namespace ark::abc2program {

AbcCodeProcessor::AbcCodeProcessor(panda_file::File::EntityId entityId, Abc2ProgramKeyData &keyData,
                                   panda_file::File::EntityId methodId)
    : AbcFileEntityProcessor(entityId, keyData), methodId_(methodId)
{
    codeDataAccessor_ = std::make_unique<panda_file::CodeDataAccessor>(*file_, entityId_);
    codeConverter_ = std::make_unique<AbcCodeConverter>(keyData);
    FillProgramData();
}

std::vector<pandasm::Ins> &&AbcCodeProcessor::GetIns()
{
    return std::move(ins_);
}

std::vector<pandasm::Function::CatchBlock> &&AbcCodeProcessor::GetCatchBlocks()
{
    return std::move(catchBlocks_);
}

uint32_t AbcCodeProcessor::GetNumVregs() const
{
    return codeDataAccessor_->GetNumVregs();
}

static size_t GetBytecodeInstructionNumber(BytecodeInstruction bcInsFirst, BytecodeInstruction bcInsCur)
{
    size_t count = 0;

    while (bcInsFirst.GetAddress() != bcInsCur.GetAddress()) {
        count++;
        bcInsFirst = bcInsFirst.GetNext();
        if (bcInsFirst.GetAddress() > bcInsCur.GetAddress()) {
            return std::numeric_limits<size_t>::max();
        }
    }

    return count;
}

struct TranslateImmToLabelStruct {
    pandasm::Ins *paIns;
    LabelTable *labelTable;
    const uint8_t *insArr;
    BytecodeInstruction bcIns;
    BytecodeInstruction bcInsLast;
    panda_file::File::EntityId codeId;
};

static void TranslateImmToLabel(TranslateImmToLabelStruct &immToLabel)
{
    const int32_t jmpOffset = immToLabel.paIns->GetImm<int64_t>(0U);
    const auto bcInsDest = immToLabel.bcIns.JumpTo(jmpOffset);
    if (immToLabel.bcInsLast.GetAddress() <= bcInsDest.GetAddress()) {
        LOG(ERROR, ABC2PROGRAM) << "> error encountered at " << immToLabel.codeId << " (0x" << std::hex
                                << immToLabel.codeId << "). incorrect instruction at offset: 0x"
                                << (immToLabel.bcIns.GetAddress() - immToLabel.insArr) << ": invalid jump offset 0x"
                                << jmpOffset << " - jumping out of bounds!";
        return;
    }
    size_t idx = GetBytecodeInstructionNumber(BytecodeInstruction(immToLabel.insArr), bcInsDest);
    if (idx != std::numeric_limits<size_t>::max()) {
        if (immToLabel.labelTable->find(idx) == immToLabel.labelTable->end()) {
            std::stringstream ss {};
            ss << "jump_label_" << immToLabel.labelTable->size();
            (*immToLabel.labelTable)[idx] = ss.str();
        }

        immToLabel.paIns->ClearImm();
        immToLabel.paIns->EmplaceID(immToLabel.labelTable->at(idx));
    } else {
        LOG(ERROR, ABC2PROGRAM) << "> error encountered at " << immToLabel.codeId << " (0x" << std::hex
                                << immToLabel.codeId << "). incorrect instruction at offset: 0x"
                                << (immToLabel.bcIns.GetAddress() - immToLabel.insArr) << ": invalid jump offset 0x"
                                << jmpOffset << " - jumping in the middle of another instruction!";
    }
}

void AbcCodeProcessor::FillProgramData()
{
    const auto insSize = codeDataAccessor_->GetCodeSize();
    const auto insArr = codeDataAccessor_->GetInstructions();
    auto bcIns = BytecodeInstruction(insArr);
    const auto bcInsLast = bcIns.JumpTo(insSize);

    LabelTable labelTable = GetExceptions(methodId_, entityId_);

    while (bcIns.GetAddress() != bcInsLast.GetAddress()) {
        if (bcIns.HasFlag(BytecodeInstruction::Flags::FIELD_ID) ||
            bcIns.HasFlag(BytecodeInstruction::Flags::STATIC_FIELD_ID)) {
            auto idx = bcIns.GetId().AsIndex();
            auto id = file_->ResolveFieldIndex(methodId_, idx);
            CollectExternalFields(id);
        }

        pandasm::Ins paIns = codeConverter_->BytecodeInstructionToPandasmInstruction(bcIns, methodId_);
        if (paIns.IsJump()) {
            TranslateImmToLabelStruct immToLabel {&paIns, &labelTable, insArr, bcIns, bcInsLast, entityId_};
            TranslateImmToLabel(immToLabel);
        }
        // check if method id is unknown external method. if so, emplace it in table
        if (bcIns.HasFlag(BytecodeInstruction::Flags::METHOD_ID) ||
            bcIns.HasFlag(BytecodeInstruction::Flags::STATIC_METHOD_ID)) {
            const auto argMethodIdx = bcIns.GetId().AsIndex();
            const auto argMethodId = file_->ResolveMethodIndex(methodId_, argMethodIdx);

            AbcMethodProcessor methodProcessor(argMethodId, keyData_);
        }

        if (bcIns.HasFlag(BytecodeInstruction::Flags::STRING_ID)) {
            const auto offset = bcIns.GetId().AsFileId();
            stringTable_->AddStringId(offset);
        }

        ins_.emplace_back(std::move(paIns));
        bcIns = bcIns.GetNext();
    }

    for (const auto &pair : labelTable) {
        ins_[pair.first].SetLabel(pair.second);
    }
}

struct LocateCatchBlockStruct {
    const BytecodeInstruction &bcIns;
    const BytecodeInstruction &bcInsLast;
    const panda_file::CodeDataAccessor::CatchBlock &catchBlock;
    pandasm::Function::CatchBlock *catchBlockPa;
    LabelTable *labelTable;
    size_t tryIdx;
    size_t catchIdx;
};

static bool LocateCatchBlock(LocateCatchBlockStruct &locateCatchBlock)
{
    const auto handlerBeginOffset = locateCatchBlock.catchBlock.GetHandlerPc();
    const auto handlerEndOffset = handlerBeginOffset + locateCatchBlock.catchBlock.GetCodeSize();

    const auto handlerBeginBcIns = locateCatchBlock.bcIns.JumpTo(handlerBeginOffset);
    const auto handlerEndBcIns = locateCatchBlock.bcIns.JumpTo(handlerEndOffset);

    const size_t handlerBeginIdx = GetBytecodeInstructionNumber(locateCatchBlock.bcIns, handlerBeginBcIns);
    const size_t handlerEndIdx = GetBytecodeInstructionNumber(locateCatchBlock.bcIns, handlerEndBcIns);

    const bool handlerBeginOffsetInRange = locateCatchBlock.bcInsLast.GetAddress() > handlerBeginBcIns.GetAddress();
    const bool handlerEndOffsetInRange = locateCatchBlock.bcInsLast.GetAddress() > handlerEndBcIns.GetAddress();
    const bool handlerBeginOffsetValid = handlerBeginIdx != std::numeric_limits<size_t>::max();
    const bool handlerEndOffsetValid = handlerEndIdx != std::numeric_limits<size_t>::max();

    if (!handlerBeginOffsetInRange || !handlerBeginOffsetValid) {
        LOG(ERROR, ABC2PROGRAM) << "> invalid catch block begin offset! address is: 0x" << std::hex
                                << handlerBeginBcIns.GetAddress();
        return false;
    }

    auto itBegin = locateCatchBlock.labelTable->find(handlerBeginIdx);
    if (itBegin == locateCatchBlock.labelTable->end()) {
        std::stringstream ss {};
        ss << "handler_begin_label_" << locateCatchBlock.tryIdx << "_" << locateCatchBlock.catchIdx;
        locateCatchBlock.catchBlockPa->catchBeginLabel = ss.str();
        locateCatchBlock.labelTable->insert(std::pair<size_t, std::string>(handlerBeginIdx, ss.str()));
    } else {
        locateCatchBlock.catchBlockPa->catchBeginLabel = itBegin->second;
    }

    if (!handlerEndOffsetInRange || !handlerEndOffsetValid) {
        LOG(ERROR, ABC2PROGRAM) << "> invalid catch block end offset! address is: 0x" << std::hex
                                << handlerEndBcIns.GetAddress();
        return false;
    }

    auto itEnd = locateCatchBlock.labelTable->find(handlerEndIdx);
    if (itEnd == locateCatchBlock.labelTable->end()) {
        std::stringstream ss {};
        ss << "handler_end_label_" << locateCatchBlock.tryIdx << "_" << locateCatchBlock.catchIdx;
        locateCatchBlock.catchBlockPa->catchEndLabel = ss.str();
        locateCatchBlock.labelTable->insert(std::pair<size_t, std::string>(handlerEndIdx, ss.str()));
    } else {
        locateCatchBlock.catchBlockPa->catchEndLabel = itEnd->second;
    }

    return true;
}

struct LocateTryBlockStruct {
    const BytecodeInstruction &bcIns;
    const BytecodeInstruction &bcInsLast;
    const panda_file::CodeDataAccessor::TryBlock &tryBlock;
    pandasm::Function::CatchBlock *catchBlockPa;
    LabelTable *labelTable;
    size_t tryIdx;
};

static bool LocateTryBlock(LocateTryBlockStruct &locateTryBlock)
{
    const auto tryBeginBcIns = locateTryBlock.bcIns.JumpTo(locateTryBlock.tryBlock.GetStartPc());
    const auto tryEndBcIns =
        locateTryBlock.bcIns.JumpTo(locateTryBlock.tryBlock.GetStartPc() + locateTryBlock.tryBlock.GetLength());

    const size_t tryBeginIdx = GetBytecodeInstructionNumber(locateTryBlock.bcIns, tryBeginBcIns);
    const size_t tryEndIdx = GetBytecodeInstructionNumber(locateTryBlock.bcIns, tryEndBcIns);

    const bool tryBeginOffsetInRange = locateTryBlock.bcInsLast.GetAddress() > tryBeginBcIns.GetAddress();
    const bool tryEndOffsetInRange = locateTryBlock.bcInsLast.GetAddress() >= tryEndBcIns.GetAddress();
    const bool tryBeginOffsetValid = tryBeginIdx != std::numeric_limits<size_t>::max();
    const bool tryEndOffsetValid = tryEndIdx != std::numeric_limits<size_t>::max();

    if (!tryBeginOffsetInRange || !tryBeginOffsetValid) {
        LOG(ERROR, ABC2PROGRAM) << "> invalid try block begin offset! address is: 0x" << std::hex
                                << tryBeginBcIns.GetAddress();
        return false;
    }

    auto itBegin = locateTryBlock.labelTable->find(tryBeginIdx);
    if (itBegin == locateTryBlock.labelTable->end()) {
        std::stringstream ss {};
        ss << "try_begin_label_" << locateTryBlock.tryIdx;
        locateTryBlock.catchBlockPa->tryBeginLabel = ss.str();
        locateTryBlock.labelTable->insert(std::pair<size_t, std::string>(tryBeginIdx, ss.str()));
    } else {
        locateTryBlock.catchBlockPa->tryBeginLabel = itBegin->second;
    }

    if (!tryEndOffsetInRange || !tryEndOffsetValid) {
        LOG(ERROR, ABC2PROGRAM) << "> invalid try block end offset! address is: 0x" << std::hex
                                << tryEndBcIns.GetAddress();
        return false;
    }

    auto itEnd = locateTryBlock.labelTable->find(tryEndIdx);
    if (itEnd == locateTryBlock.labelTable->end()) {
        std::stringstream ss {};
        ss << "try_end_label_" << locateTryBlock.tryIdx;
        locateTryBlock.catchBlockPa->tryEndLabel = ss.str();
        locateTryBlock.labelTable->insert(std::pair<size_t, std::string>(tryEndIdx, ss.str()));
    } else {
        locateTryBlock.catchBlockPa->tryEndLabel = itEnd->second;
    }

    return true;
}

LabelTable AbcCodeProcessor::GetExceptions(panda_file::File::EntityId methodId, panda_file::File::EntityId codeId)
{
    LOG(DEBUG, ABC2PROGRAM) << "[getting exceptions]\ncode id: " << codeId << " (0x" << std::hex << codeId << ")";
    panda_file::CodeDataAccessor codeAccessor(*file_, codeId);

    const auto bcIns = BytecodeInstruction(codeAccessor.GetInstructions());
    const auto bcInsLast = bcIns.JumpTo(codeAccessor.GetCodeSize());

    size_t tryIdx = 0;
    LabelTable labelTable {};
    codeAccessor.EnumerateTryBlocks([&](panda_file::CodeDataAccessor::TryBlock &tryBlock) {
        pandasm::Function::CatchBlock catchBlockPa {};
        LocateTryBlockStruct locateTryBlock {bcIns, bcInsLast, tryBlock, &catchBlockPa, &labelTable, tryIdx};
        if (!LocateTryBlock(locateTryBlock)) {
            return false;
        }
        size_t catchIdx = 0;
        tryBlock.EnumerateCatchBlocks([&](panda_file::CodeDataAccessor::CatchBlock &catchBlock) {
            auto classIdx = catchBlock.GetTypeIdx();
            if (classIdx == panda_file::INVALID_INDEX) {
                catchBlockPa.exceptionRecord = "";
            } else {
                const auto classId = file_->ResolveClassIndex(methodId, classIdx);

                catchBlockPa.exceptionRecord = keyData_.GetFullRecordNameById(classId);
            }
            LocateCatchBlockStruct locateCatchBlock {bcIns,       bcInsLast, catchBlock, &catchBlockPa,
                                                     &labelTable, tryIdx,    catchIdx};
            if (!LocateCatchBlock(locateCatchBlock)) {
                return false;
            }

            catchBlocks_.emplace_back(catchBlockPa);
            catchBlockPa.catchBeginLabel.clear();
            catchBlockPa.catchEndLabel.clear();
            catchIdx++;

            return true;
        });
        tryIdx++;

        return true;
    });

    return labelTable;
}

void AbcCodeProcessor::CollectExternalFields(const panda_file::File::EntityId &fieldId)
{
    panda_file::FieldDataAccessor fieldAccessor(*file_, fieldId);
    if (!fieldAccessor.IsExternal()) {
        return;
    }
    auto recordName = keyData_.GetFullRecordNameById(fieldAccessor.GetClassId());
    pandasm::Record record {"", keyData_.GetFileLanguage()};

    AbcFieldProcessor fieldProcessor(fieldId, keyData_, record, true);
}

}  // namespace ark::abc2program
