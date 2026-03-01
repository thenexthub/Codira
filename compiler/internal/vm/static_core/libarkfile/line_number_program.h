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

#ifndef LIBPANDAFILE_LINE_NUMBER_PROGRAM_H
#define LIBPANDAFILE_LINE_NUMBER_PROGRAM_H

#include "libarkfile/file-inl.h"
#include "libarkfile/file_items.h"

namespace ark::panda_file {

class LineProgramState {
public:
    LineProgramState(const File &pf, File::EntityId file, size_t line, Span<const uint8_t> constantPool)
        : pf_(pf), file_(file), line_(line), constantPool_(constantPool)
    {
    }

    void AdvanceLine(int32_t v)
    {
        line_ += static_cast<size_t>(v);
    }

    void AdvancePc(uint32_t v)
    {
        address_ += v;
    }

    void SetFile(uint32_t offset)
    {
        file_ = File::EntityId(offset);
    }

    const uint8_t *GetFile() const
    {
        return pf_.GetStringData(file_).data;
    }

    bool HasFile() const
    {
        return file_.IsValid();
    }

    void SetSourceCode(uint32_t offset)
    {
        sourceCode_ = File::EntityId(offset);
    }

    const uint8_t *GetSourceCode() const
    {
        ASSERT(HasSourceCode());
        return pf_.GetStringData(sourceCode_).data;
    }

    bool HasSourceCode() const
    {
        return sourceCode_.IsValid();
    }

    size_t GetLine() const
    {
        return line_;
    }

    void SetColumn(int32_t c)
    {
        column_ = static_cast<size_t>(c);
    }

    size_t GetColumn() const
    {
        return column_;
    }

    uint32_t GetAddress() const
    {
        return address_;
    }

    uint32_t ReadULeb128()
    {
        return panda_file::helpers::ReadULeb128(&constantPool_);
    }

    int32_t ReadSLeb128()
    {
        return panda_file::helpers::ReadLeb128(&constantPool_);
    }

    const File &GetPandaFile() const
    {
        return pf_;
    }

private:
    const File &pf_;

    File::EntityId file_;
    File::EntityId sourceCode_;
    size_t line_;
    size_t column_ {0};
    Span<const uint8_t> constantPool_;

    uint32_t address_ {0};
};

template <class Handler>
class LineNumberProgramProcessor {
public:
    LineNumberProgramProcessor(const uint8_t *program, Handler *handler)
        : state_(handler->GetState()), program_(program), handler_(handler)
    {
    }

    ~LineNumberProgramProcessor() = default;

    NO_COPY_SEMANTIC(LineNumberProgramProcessor);
    NO_MOVE_SEMANTIC(LineNumberProgramProcessor);

    // CC-OFFNXT(G.FUN.01-CPP, huge_method[C]) big switch case
    void Process()
    {
        handler_->ProcessBegin();

        auto opcode = ReadOpcode();
        bool res = false;
        while (opcode != Opcode::END_SEQUENCE) {
            switch (opcode) {
                case Opcode::ADVANCE_LINE: {
                    res = HandleAdvanceLine();
                    break;
                }
                case Opcode::ADVANCE_PC: {
                    res = HandleAdvancePc();
                    break;
                }
                case Opcode::SET_FILE: {
                    res = HandleSetFile();
                    break;
                }
                case Opcode::SET_SOURCE_CODE: {
                    res = HandleSetSourceCode();
                    break;
                }
                case Opcode::SET_PROLOGUE_END: {
                    res = HandleSetPrologueEnd();
                    break;
                }
                case Opcode::SET_EPILOGUE_BEGIN: {
                    res = HandleSetEpilogueBegin();
                    break;
                }
                case Opcode::START_LOCAL: {
                    res = HandleStartLocal();
                    break;
                }
                case Opcode::START_LOCAL_EXTENDED: {
                    res = HandleStartLocalExtended();
                    break;
                }
                case Opcode::RESTART_LOCAL: {
                    res = HandleRestartLocal();
                    break;
                }
                case Opcode::END_LOCAL: {
                    res = HandleEndLocal();
                    break;
                }
                case Opcode::SET_COLUMN: {
                    res = HandleSetColumn();
                    break;
                }
                default: {
                    res = HandleSpecialOpcode(opcode);
                    break;
                }
            }

            if (!res) {
                break;
            }

            opcode = ReadOpcode();
        }

        handler_->ProcessEnd();
    }

private:
    using Opcode = LineNumberProgramItem::Opcode;

    Opcode ReadOpcode()
    {
        auto opcode = static_cast<Opcode>(*program_);
        ++program_;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return opcode;
    }

    int32_t ReadRegisterNumber()
    {
        auto [regiserNumber, n, isFull] = leb128::DecodeSigned<int32_t>(program_);
        LOG_IF(!isFull, FATAL, COMMON) << "Cannot read a register number";
        program_ += n;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return regiserNumber;
    }

    bool HandleAdvanceLine() const
    {
        auto lineDiff = state_->ReadSLeb128();
        return handler_->HandleAdvanceLine(lineDiff);
    }

    bool HandleAdvancePc() const
    {
        auto pcDiff = state_->ReadULeb128();
        return handler_->HandleAdvancePc(pcDiff);
    }

    bool HandleSetFile() const
    {
        return handler_->HandleSetFile(state_->ReadULeb128());
    }

    bool HandleSetSourceCode() const
    {
        return handler_->HandleSetSourceCode(state_->ReadULeb128());
    }

    bool HandleSetPrologueEnd() const
    {
        return handler_->HandleSetPrologueEnd();
    }

    bool HandleSetEpilogueBegin() const
    {
        return handler_->HandleSetEpilogueBegin();
    }

    bool HandleStartLocal()
    {
        auto regNumber = ReadRegisterNumber();
        auto nameIndex = state_->ReadULeb128();
        auto typeIndex = state_->ReadULeb128();
        return handler_->HandleStartLocal(regNumber, nameIndex, typeIndex);
    }

    bool HandleStartLocalExtended()
    {
        auto regNumber = ReadRegisterNumber();
        auto nameIndex = state_->ReadULeb128();
        auto typeIndex = state_->ReadULeb128();
        auto typeSignatureIndex = state_->ReadULeb128();
        return handler_->HandleStartLocalExtended(regNumber, nameIndex, typeIndex, typeSignatureIndex);
    }

    bool HandleEndLocal()
    {
        auto regNumber = ReadRegisterNumber();
        return handler_->HandleEndLocal(regNumber);
    }

    bool HandleRestartLocal()
    {
        auto regNumber = ReadRegisterNumber();
        return handler_->HandleRestartLocal(regNumber);
    }

    bool HandleSetColumn()
    {
        auto cn = state_->ReadULeb128();
        return handler_->HandleSetColumn(cn);
    }

    bool HandleSpecialOpcode(LineNumberProgramItem::Opcode opcode)
    {
        ASSERT(static_cast<uint8_t>(opcode) >= LineNumberProgramItem::OPCODE_BASE);

        auto adjustOpcode = static_cast<int32_t>(static_cast<uint8_t>(opcode) - LineNumberProgramItem::OPCODE_BASE);
        auto pcOffset = static_cast<uint32_t>(adjustOpcode / LineNumberProgramItem::LINE_RANGE);
        int32_t lineOffset = adjustOpcode % LineNumberProgramItem::LINE_RANGE + LineNumberProgramItem::LINE_BASE;
        return handler_->HandleSpecialOpcode(pcOffset, lineOffset);
    }

    LineProgramState *state_;
    const uint8_t *program_;
    Handler *handler_;
};

}  // namespace ark::panda_file

#endif  // LIBPANDAFILE_LINE_NUMBER_PROGRAM_H
