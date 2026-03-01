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

#ifndef LIBPANDAFILE_DEBUG_INFO_UPDATER_INL_H_
#define LIBPANDAFILE_DEBUG_INFO_UPDATER_INL_H_

#include <type_traits>
#include "libarkfile/debug_data_accessor.h"
#include "libarkfile/debug_data_accessor-inl.h"
#include "libarkfile/debug_helpers.h"
#include "libarkfile/line_number_program.h"

namespace ark::panda_file {

/**
 * @brief Handler implementation class for `LineNumberProgramProcessor`.
 *
 * Instances of this class traverse debug information of a single binary method
 * and create all required constant strings (names, types, etc.) in a merged file.
 */
template <typename T>
class LineNumberProgramScrapper final {
public:
    /// @param scrapper object implementing `DebugInfoUpdater`.
    explicit LineNumberProgramScrapper(T *scrapper, LineProgramState *state) : scrapper_(scrapper), state_(state)
    {
        ASSERT(scrapper_);
        ASSERT(state_);
    }

    ~LineNumberProgramScrapper() = default;

    NO_COPY_SEMANTIC(LineNumberProgramScrapper);
    NO_MOVE_SEMANTIC(LineNumberProgramScrapper);

    LineProgramState *GetState() const
    {
        return state_;
    }

    void ProcessBegin() {}

    void ProcessEnd() {}

    bool HandleAdvanceLine([[maybe_unused]] int32_t lineDiff) const
    {
        return true;
    }

    bool HandleAdvancePc([[maybe_unused]] uint32_t pcDiff) const
    {
        return true;
    }

    bool HandleSetFile(uint32_t sourceFileId) const
    {
        std::string sourceFile = debug_helpers::GetStringFromConstantPool(GetPandaFile(), sourceFileId);
        scrapper_->GetOrCreateStringItem(sourceFile);
        return true;
    }

    bool HandleSetSourceCode(uint32_t sourceCodeId) const
    {
        std::string sourceCode = debug_helpers::GetStringFromConstantPool(GetPandaFile(), sourceCodeId);
        scrapper_->GetOrCreateStringItem(sourceCode);
        return true;
    }

    bool HandleSetPrologueEnd() const
    {
        return true;
    }

    bool HandleSetEpilogueBegin() const
    {
        return true;
    }

    bool HandleStartLocal([[maybe_unused]] int32_t regNumber, uint32_t nameId, uint32_t typeId)
    {
        std::string name = debug_helpers::GetStringFromConstantPool(GetPandaFile(), nameId);
        std::string type = debug_helpers::GetStringFromConstantPool(GetPandaFile(), typeId);

        scrapper_->GetOrCreateStringItem(name);
        scrapper_->GetType(File::EntityId(typeId), type);
        return true;
    }

    bool HandleStartLocalExtended([[maybe_unused]] int32_t regNumber, uint32_t nameId, uint32_t typeId,
                                  uint32_t typeSignatureId)
    {
        std::string name = debug_helpers::GetStringFromConstantPool(GetPandaFile(), nameId);
        std::string type = debug_helpers::GetStringFromConstantPool(GetPandaFile(), typeId);
        std::string typeSign = debug_helpers::GetStringFromConstantPool(GetPandaFile(), typeSignatureId);

        scrapper_->GetOrCreateStringItem(name);
        scrapper_->GetType(File::EntityId(typeId), type);
        scrapper_->GetOrCreateStringItem(typeSign);
        return true;
    }

    bool HandleEndLocal([[maybe_unused]] int32_t regNumber)
    {
        return true;
    }

    bool HandleRestartLocal([[maybe_unused]] int32_t regNumber) const
    {
        return true;
    }

    bool HandleSetColumn([[maybe_unused]] int32_t columnNumber)
    {
        return true;
    }

    bool HandleSpecialOpcode([[maybe_unused]] uint32_t pcOffset, [[maybe_unused]] int32_t lineOffset)
    {
        return true;
    }

    const panda_file::File &GetPandaFile() const
    {
        return state_->GetPandaFile();
    }

private:
    T *scrapper_;
    LineProgramState *state_;
};

/**
 * @brief Handler implementation class for `LineNumberProgramProcessor`.
 *
 * Instances of this class traverse debug information of a single binary method
 * and emit the corresponding `LineNumberProgram` into a merged file.
 */
template <typename T>
class LineNumberProgramEmitter final {
public:
    /// @param updater object implementing `DebugInfoUpdater`.
    explicit LineNumberProgramEmitter(T *updater, LineProgramState *state, LineNumberProgramItemBase *lnpItem,
                                      std::vector<uint8_t> *constantPool)
        : updater_(updater), state_(state), lnpItem_(lnpItem), constantPool_(constantPool)
    {
        ASSERT(updater_);
        ASSERT(state_);
        ASSERT(lnpItem_);
        ASSERT(constantPool_);
    }

    ~LineNumberProgramEmitter() = default;

    NO_COPY_SEMANTIC(LineNumberProgramEmitter);
    NO_MOVE_SEMANTIC(LineNumberProgramEmitter);

    LineProgramState *GetState() const
    {
        return state_;
    }

    void ProcessBegin() {}

    void ProcessEnd()
    {
        lnpItem_->EmitEnd();
    }

    bool HandleAdvanceLine(int32_t lineDiff) const
    {
        lnpItem_->EmitAdvanceLine(constantPool_, lineDiff);
        return true;
    }

    bool HandleAdvancePc(uint32_t pcDiff) const
    {
        lnpItem_->EmitAdvancePc(constantPool_, pcDiff);
        return true;
    }

    bool HandleSetFile(uint32_t sourceFileId) const
    {
        std::string sourceFile = debug_helpers::GetStringFromConstantPool(GetPandaFile(), sourceFileId);
        auto *sourceFileItem = updater_->GetOrCreateStringItem(sourceFile);
        lnpItem_->EmitSetFile(constantPool_, sourceFileItem);
        return true;
    }

    bool HandleSetSourceCode(uint32_t sourceCodeId) const
    {
        std::string sourceCode = debug_helpers::GetStringFromConstantPool(GetPandaFile(), sourceCodeId);
        auto *sourceCodeItem = updater_->GetOrCreateStringItem(sourceCode);
        lnpItem_->EmitSetFile(constantPool_, sourceCodeItem);
        return true;
    }

    bool HandleSetPrologueEnd() const
    {
        lnpItem_->EmitPrologueEnd();
        return true;
    }

    bool HandleSetEpilogueBegin() const
    {
        lnpItem_->EmitEpilogueBegin();
        return true;
    }

    bool HandleStartLocal(int32_t regNumber, uint32_t nameId, uint32_t typeId)
    {
        std::string name = debug_helpers::GetStringFromConstantPool(GetPandaFile(), nameId);
        std::string type = debug_helpers::GetStringFromConstantPool(GetPandaFile(), typeId);

        auto *nameItem = updater_->GetOrCreateStringItem(name);
        auto *typeItem = updater_->GetType(File::EntityId(typeId), type);
        auto *typeItemName = (typeItem == nullptr) ? updater_->GetOrCreateStringItem(type) : typeItem->GetNameItem();

        lnpItem_->EmitStartLocal(constantPool_, regNumber, nameItem, typeItemName);
        return true;
    }

    bool HandleStartLocalExtended(int32_t regNumber, uint32_t nameId, uint32_t typeId, uint32_t typeSignatureId)
    {
        std::string name = debug_helpers::GetStringFromConstantPool(GetPandaFile(), nameId);
        std::string type = debug_helpers::GetStringFromConstantPool(GetPandaFile(), typeId);
        std::string typeSign = debug_helpers::GetStringFromConstantPool(GetPandaFile(), typeSignatureId);

        auto *nameItem = updater_->GetOrCreateStringItem(name);
        auto *typeItem = updater_->GetType(File::EntityId(typeId), type);
        auto *typeItemName = (typeItem == nullptr) ? updater_->GetOrCreateStringItem(type) : typeItem->GetNameItem();
        auto *typeSignatureItem = updater_->GetOrCreateStringItem(typeSign);

        lnpItem_->EmitStartLocalExtended(constantPool_, regNumber, nameItem, typeItemName, typeSignatureItem);
        return true;
    }

    bool HandleEndLocal(int32_t regNumber)
    {
        lnpItem_->EmitEndLocal(regNumber);
        return true;
    }

    bool HandleRestartLocal(int32_t regNumber) const
    {
        lnpItem_->EmitRestartLocal(regNumber);
        return true;
    }

    bool HandleSetColumn(int32_t columnNumber)
    {
        lnpItem_->EmitColumn(constantPool_, 0, columnNumber);
        return true;
    }

    bool HandleSpecialOpcode(uint32_t pcOffset, int32_t lineOffset)
    {
        lnpItem_->EmitSpecialOpcode(pcOffset, lineOffset);
        return true;
    }

    const panda_file::File &GetPandaFile() const
    {
        return state_->GetPandaFile();
    }

private:
    T *updater_;
    LineProgramState *state_;
    LineNumberProgramItemBase *lnpItem_;
    std::vector<uint8_t> *constantPool_;
};

/**
 * @brief Base class for moving debug information into merged file.
 *
 * Type `T` is a CRTP with following methods:
 * ```
 * StringItem *GetOrCreateStringItem(const std::string &s);
 * BaseClassItem *GetType(File::EntityId type_id, const std::string &type_name)
 * ```
 */
template <typename T>
class DebugInfoUpdater {
public:
    explicit DebugInfoUpdater(const File *file) : file_(file) {}

    /**
     * @brief Collects all required strings from debug information denoted by `debugInfoId`.
     *
     * This method must be called for each original method before emitting debug information in a merged file,
     * so that strings could be reused.
     */
    void Scrap(File::EntityId debugInfoId)
    {
        DebugInfoDataAccessor debugAcc(*file_, debugInfoId);
        const uint8_t *program = debugAcc.GetLineNumberProgram();

        panda_file::LineProgramState state(*file_, File::EntityId(0), debugAcc.GetLineStart(),
                                           debugAcc.GetConstantPool());

        LineNumberProgramScrapper<T> handler(This(), &state);
        LineNumberProgramProcessor programProcessor(program, &handler);
        programProcessor.Process();
    }

    /**
     * @brief Emits `LineNumberProgram` in a merged file.
     *
     * @param lnpItem target LineNumberProgram. When passing instances of `LineNumberProgramItemBase`,
     * handler will not emit any `LineNumberProgram` instructions
     * and will only write their arguments into `constantPool`.
     *
     * @param constantPool storage for `LineNumberProgram` instructions' arguments, unique for each emitted method.
     *
     * @param debugInfoId method's debug info identifier.
     */
    void Emit(LineNumberProgramItemBase *lnpItem, std::vector<uint8_t> *constantPool, File::EntityId debugInfoId)
    {
        ASSERT(lnpItem);
        ASSERT(constantPool);

        if (!lnpItem->Empty()) {
            return;
        }

        DebugInfoDataAccessor debugAcc(*file_, debugInfoId);
        const uint8_t *program = debugAcc.GetLineNumberProgram();

        panda_file::LineProgramState state(*file_, File::EntityId(0), debugAcc.GetLineStart(),
                                           debugAcc.GetConstantPool());

        LineNumberProgramEmitter<T> handler(This(), &state, lnpItem, constantPool);
        LineNumberProgramProcessor programProcessor(program, &handler);
        programProcessor.Process();
    }

protected:
    const File *GetFile() const
    {
        return file_;
    }

private:
    const File *file_;

    T *This()
    {
        static_assert(std::is_base_of_v<DebugInfoUpdater, T>);
        return static_cast<T *>(this);
    }
};
}  // namespace ark::panda_file

#endif  // LIBPANDAFILE_DEBUG_INFO_UPDATER_INL_H_
