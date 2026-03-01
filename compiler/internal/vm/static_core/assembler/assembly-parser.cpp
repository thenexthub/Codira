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

#include "assembly-type.h"
#include "libarkfile/type_helper.h"
#include "ins_emit.h"
#include "libarkfile/modifiers.h"
#include "opcode_parsing.h"
#include "operand_types_print.h"
#include "utils/number-utils.h"

namespace ark::pandasm {

bool Parser::ParseRecordFields()
{
    if (!open_ && *context_ == Token::Type::DEL_BRACE_L) {
        currRecord_->bodyLocation.begin = GetCurrentPosition(false);
        open_ = true;

        ++context_;
    }

    currRecord_->bodyPresence = true;

    if (!open_) {
        context_.err = GetError("Expected keyword.", Error::ErrorType::ERR_BAD_KEYWORD);
        return false;
    }

    if (context_.Mask()) {
        return true;
    }

    if (open_ && *context_ == Token::Type::DEL_BRACE_R) {
        currRecord_->bodyLocation.end = GetCurrentPosition(true);
        ++context_;

        open_ = false;

        return true;
    }

    currRecord_->fieldList.emplace_back(program_.lang);
    currFld_ = &(currRecord_->fieldList[currRecord_->fieldList.size() - 1]);
    currFld_->lineOfDef = lineStric_;
    context_.insNumber = currRecord_->fieldList.size();

    LOG(DEBUG, ASSEMBLER) << "parse line " << lineStric_ << " as field (.field name)";
    if (!ParseRecordField()) {
        if (context_.err.err != Error::ErrorType::ERR_NONE) {
            return false;
        }

        if (open_ && *context_ == Token::Type::DEL_BRACE_R) {
            currRecord_->bodyLocation.end = GetCurrentPosition(true);
            ++context_;
            open_ = false;
        } else {
            context_.err = GetError("Expected a new field on the next line.", Error::ErrorType::ERR_BAD_KEYWORD);
            return false;
        }
    }

    return true;
}

bool Parser::ParseFieldName()
{
    if (PrefixedValidName()) {
        std::string fieldName = std::string(context_.GiveToken().data(), context_.GiveToken().length());

        currFld_ = &(currRecord_->fieldList[currRecord_->fieldList.size() - 1]);

        currFld_->name = fieldName;

        ++context_;

        return true;
    }

    context_.err = GetError("Invalid name of field.", Error::ErrorType::ERR_BAD_OPERATION_NAME);

    return false;
}

bool Parser::ParseType(Type *type)
{
    ASSERT(TypeValidName());

    std::string componentName;
    GetName(&componentName);
    size_t rank = 0;

    ++context_;

    while (*context_ == Token::Type::DEL_SQUARE_BRACKET_L) {
        ++context_;
        if (*context_ != Token::Type::DEL_SQUARE_BRACKET_R) {
            context_.err = GetError("Expected ']'.", Error::ErrorType::ERR_BAD_ARRAY_TYPE_BOUND);
            return false;
        }
        ++context_;
        ++rank;
    }

    *type = Type(componentName, rank);

    if (type->IsArray()) {
        program_.arrayTypes.insert(*type);
    }

    return true;
}

bool Parser::ParseFieldType()
{
    LOG(DEBUG, ASSEMBLER) << "started searching field type value (line " << lineStric_
                          << "): " << context_.tokens[context_.number - 1].wholeLine;

    if (!TypeValidName()) {
        context_.err = GetError("Not a correct type.", Error::ErrorType::ERR_BAD_FIELD_VALUE_TYPE);
        return false;
    }

    if (!ParseType(&currFld_->type)) {
        return false;
    }

    currFld_->metadata->SetFieldType(currFld_->type);

    LOG(DEBUG, ASSEMBLER) << "field type found (line " << lineStric_ << "): " << context_.GiveToken();

    return true;
}

bool IsSameField(const pandasm::Field &field, std::string_view fieldName, bool isStatic)
{
    return field.IsStatic() == isStatic && field.name == fieldName;
}

bool IsSameField(const pandasm::Field &field1, const pandasm::Field &field2)
{
    return field1.IsStatic() == field2.IsStatic() && field1.name == field2.name;
}

bool Parser::ParseRecordField()
{
    if (!ParseFieldType()) {
        return false;
    }

    if (context_.Mask()) {
        context_.err = GetError("Expected field name.", Error::ErrorType::ERR_BAD_FIELD_MISSING_NAME, +1);
        return false;
    }

    if (!ParseFieldName()) {
        return false;
    }

    if (open_ && *context_ == Token::Type::DEL_BRACE_R) {
        currRecord_->bodyLocation.end = GetCurrentPosition(true);
        ++context_;
        open_ = false;
        return true;
    }

    metadata_ = currFld_->metadata.get();
    ParseMetaDef();

    auto matchNames = [currFld = currFld_](const pandasm::Field &f) { return IsSameField(f, *currFld); };
    auto identicalField = currRecord_->fieldList.end() - 1;
    const auto iter = std::find_if(currRecord_->fieldList.begin(), identicalField, matchNames);
    if (iter != identicalField) {
        if (iter->isDefined) {
            context_.err =
                GetError("Repeating field names in the same record.", Error::ErrorType::ERR_REPEATING_FIELD_NAME);

            return false;
        }

        currRecord_->fieldList.erase(iter);
    }

    return context_.Mask();
}

bool Parser::IsConstArray()
{
    return isConstArray_;
}

bool Parser::ArrayElementsValidNumber()
{
    if (!IsConstArray()) {
        return true;
    }

    ASSERT(currArray_->literals.size() > 1);

    auto initSize = std::get<uint32_t>(currArray_->literals[1].value);
    if (initSize < 1) {
        return false;
    }
    if (currArray_->literals.size() != initSize + INTRO_CONST_ARRAY_LITERALS_NUMBER) {
        return false;
    }

    return true;
}

void Parser::ParseAsArray(const std::vector<Token> &tokens)
{
    LOG(DEBUG, ASSEMBLER) << "started parsing of array (line " << lineStric_ << "): " << tokens[0].wholeLine;
    funcDef_ = false;
    recordDef_ = false;
    arrayDef_ = true;

    if (!open_) {
        ++context_;
    } else {
        context_.err =
            GetError("No one array can be defined inside another array.", Error::ErrorType::ERR_BAD_DEFINITION);
        return;
    }

    if (ParseArrayFullSign()) {
        if (!open_ && *context_ == Token::Type::DEL_BRACE_L) {
            ++context_;

            LOG(DEBUG, ASSEMBLER) << "array body is open, line " << lineStric_ << ": " << tokens[0].wholeLine;

            open_ = true;
        }

        uint32_t iterNumber = 1;
        if (IsConstArray()) {
            iterNumber = std::get<uint32_t>(currArray_->literals[1].value);
        }

        if (iterNumber < 1) {
            context_.err =
                GetError("Constant array must contain at least one element.", Error::ErrorType::ERR_BAD_ARRAY_SIZE);
            return;
        }

        for (uint32_t i = 0; i < iterNumber; i++) {
            if (open_ && !context_.Mask() && *context_ != Token::Type::DEL_BRACE_R) {
                ParseArrayElements();
            } else {
                break;
            }
        }

        if (open_ && *context_ == Token::Type::DEL_BRACE_R) {
            if (!ArrayElementsValidNumber()) {
                context_.err =
                    GetError("Constant array must contain at least one element.", Error::ErrorType::ERR_BAD_ARRAY_SIZE);
                return;
            }
            LOG(DEBUG, ASSEMBLER) << "array body is closed, line " << lineStric_ << ": " << tokens[0].wholeLine;

            ++context_;

            open_ = false;
        }
    }
}

bool Parser::ParseArrayElements()
{
    if (!open_ && *context_ == Token::Type::DEL_BRACE_L) {
        open_ = true;

        ++context_;
    }

    if (!open_) {
        context_.err = GetError("Expected keyword.", Error::ErrorType::ERR_BAD_KEYWORD);
        return false;
    }

    if (context_.Mask()) {
        return true;
    }

    if (open_ && *context_ == Token::Type::DEL_BRACE_R) {
        if (!ArrayElementsValidNumber()) {
            context_.err =
                GetError("Constant array must contain at least one element.", Error::ErrorType::ERR_BAD_ARRAY_SIZE);
            return false;
        }
        ++context_;

        open_ = false;

        return true;
    }

    currArray_->literals.emplace_back();
    currArrayElem_ = &(currArray_->literals[currArray_->literals.size() - 1]);

    LOG(DEBUG, ASSEMBLER) << "parse line " << lineStric_ << " as array elem (.array_elem value)";
    if (!ParseArrayElement()) {
        if (context_.err.err != Error::ErrorType::ERR_NONE) {
            return false;
        }

        if (open_ && *context_ == Token::Type::DEL_BRACE_R) {
            ++context_;
            open_ = false;

            if (!ArrayElementsValidNumber()) {
                context_.err =
                    GetError("Constant array must contain at least one element.", Error::ErrorType::ERR_BAD_ARRAY_SIZE);
                return false;
            }
        } else {
            context_.err =
                GetError("Expected a new array element on the next line.", Error::ErrorType::ERR_BAD_KEYWORD);
            return false;
        }
    }

    return true;
}

bool Parser::ParseArrayElement()
{
    if (IsConstArray()) {
        currArrayElem_->tag = static_cast<panda_file::LiteralTag>(std::get<uint8_t>(currArray_->literals[0].value));
    } else {
        if (!ParseArrayElementType()) {
            return false;
        }

        auto valTag = currArrayElem_->tag;

        currArrayElem_->value = static_cast<uint8_t>(currArrayElem_->tag);
        currArrayElem_->tag = panda_file::LiteralTag::TAGVALUE;

        currArray_->literals.emplace_back();
        currArrayElem_ = &(currArray_->literals[currArray_->literals.size() - 1]);
        currArrayElem_->tag = valTag;
    }

    if (context_.Mask()) {
        context_.err =
            GetError("Expected array element value.", Error::ErrorType::ERR_BAD_ARRAY_ELEMENT_MISSING_VALUE, +1);
        return false;
    }

    if (!ParseArrayElementValue()) {
        return false;
    }

    if (open_ && *context_ == Token::Type::DEL_BRACE_R) {
        ++context_;
        open_ = false;

        if (!ArrayElementsValidNumber()) {
            context_.err =
                GetError("Constant array must contain at least one element.", Error::ErrorType::ERR_BAD_ARRAY_SIZE);
            return false;
        }
        return true;
    }

    return IsConstArray() ? true : context_.Mask();
}

bool Parser::ParseArrayElementType()
{
    LOG(DEBUG, ASSEMBLER) << "started searching array element type value (line " << lineStric_
                          << "): " << context_.tokens[context_.number - 1].wholeLine;

    if (!TypeValidName()) {
        context_.err = GetError("Not a correct type.", Error::ErrorType::ERR_BAD_ARRAY_ELEMENT_VALUE_TYPE);
        return false;
    }

    Type type;
    if (!ParseType(&type)) {
        return false;
    }

    // workaround until #5776 is done
    auto typeName = type.GetName();
    std::replace(typeName.begin(), typeName.end(), '.', '/');
    auto typeWithSlash = Type(typeName, 0);

    if (ark::pandasm::Type::IsPandaPrimitiveType(type.GetName())) {
        if (IsConstArray()) {
            currArrayElem_->tag = ark::pandasm::LiteralArray::GetArrayTagFromComponentType(type.GetId());
        } else {
            currArrayElem_->tag = ark::pandasm::LiteralArray::GetLiteralTagFromComponentType(type.GetId());
        }

        if (program_.arrayTypes.find(type) == program_.arrayTypes.end()) {
            program_.arrayTypes.emplace(type, 1);
        }
    } else if (ark::pandasm::Type::IsStringType(typeWithSlash.GetName(), program_.lang)) {
        if (IsConstArray()) {
            currArrayElem_->tag = panda_file::LiteralTag::ARRAY_STRING;
        } else {
            currArrayElem_->tag = panda_file::LiteralTag::STRING;
        }

        if (program_.arrayTypes.find(typeWithSlash) == program_.arrayTypes.end()) {
            program_.arrayTypes.emplace(typeWithSlash, 1);
        }
    } else {
        return false;
    }

    LOG(DEBUG, ASSEMBLER) << "array element type found (line " << lineStric_ << "): " << context_.GiveToken();

    return true;
}

bool Parser::ParseArrayElementValueInteger()
{
    int64_t n;
    if (!ParseInteger(&n)) {
        context_.err =
            GetError("Invalid value of array integer element.", Error::ErrorType::ERR_BAD_ARRAY_ELEMENT_VALUE_INTEGER);
        return false;
    }
    if (currArrayElem_->IsBoolValue()) {
        currArrayElem_->value = static_cast<bool>(n);
    }
    if (currArrayElem_->IsByteValue()) {
        currArrayElem_->value = static_cast<uint8_t>(n);
    }
    if (currArrayElem_->IsShortValue()) {
        currArrayElem_->value = static_cast<uint16_t>(n);
    }
    if (currArrayElem_->IsIntegerValue()) {
        currArrayElem_->value = static_cast<uint32_t>(n);
    }
    if (currArrayElem_->IsLongValue()) {
        currArrayElem_->value = static_cast<uint64_t>(n);
    }
    return true;
}

bool Parser::ParseArrayElementValueFloat()
{
    double n;
    if (!ParseFloat(&n, !currArrayElem_->IsFloatValue())) {
        context_.err =
            GetError("Invalid value of array float element.", Error::ErrorType::ERR_BAD_ARRAY_ELEMENT_VALUE_FLOAT);
        return false;
    }
    if (currArrayElem_->IsFloatValue()) {
        currArrayElem_->value = static_cast<float>(n);
    } else {
        currArrayElem_->value = static_cast<double>(n);
    }
    return true;
}

bool Parser::ParseArrayElementValueString()
{
    if (context_.err.err != Error::ErrorType::ERR_NONE) {
        return false;
    }

    auto res = ParseStringLiteral();
    if (!res) {
        context_.err =
            GetError("Invalid value of array string element.", Error::ErrorType::ERR_BAD_ARRAY_ELEMENT_VALUE_STRING);
        return false;
    }
    currArrayElem_->value = res.value();
    return true;
}

bool Parser::ParseArrayElementValue()
{
    if (currArrayElem_->IsBoolValue() || currArrayElem_->IsByteValue() || currArrayElem_->IsShortValue() ||
        currArrayElem_->IsIntegerValue() || currArrayElem_->IsLongValue()) {
        if (!ParseArrayElementValueInteger()) {
            return false;
        }
    }
    if (currArrayElem_->IsFloatValue() || currArrayElem_->IsDoubleValue()) {
        if (!ParseArrayElementValueFloat()) {
            return false;
        }
    }
    if (currArrayElem_->IsStringValue()) {
        if (!ParseArrayElementValueString()) {
            return false;
        }
    }

    ++context_;

    return true;
}

bool Parser::ParseFunctionCode()
{
    if (!open_ && *context_ == Token::Type::DEL_BRACE_L) {
        open_ = true;
        currFunc_->bodyLocation.begin = GetCurrentPosition(false);
        ++context_;
    }

    currFunc_->bodyPresence = true;

    if (!open_) {
        context_.err = GetError("Expected keyword.", Error::ErrorType::ERR_BAD_KEYWORD);
        return false;
    }

    if (context_.Mask()) {
        return true;
    }

    if (open_ && *context_ == Token::Type::DEL_BRACE_R) {
        currFunc_->bodyLocation.end = GetCurrentPosition(true);
        ++context_;
        open_ = false;
        return true;
    }

    currIns_ = &currFunc_->ins.emplace_back();

    LOG(DEBUG, ASSEMBLER) << "parse line " << lineStric_
                          << " as instruction ([label:] operation [operand,] [# comment])";

    ParseFunctionInstruction();

    if (open_ && *context_ == Token::Type::DEL_BRACE_R && !context_.Mask()) {
        currFunc_->bodyLocation.end = GetCurrentPosition(true);
        ++context_;
        open_ = false;
    }

    return true;
}

void Parser::ParseAsRecord(const std::vector<Token> &tokens)
{
    LOG(DEBUG, ASSEMBLER) << "started parsing of record (line " << lineStric_ << "): " << tokens[0].wholeLine;
    funcDef_ = false;
    recordDef_ = true;
    arrayDef_ = false;

    if (!open_) {
        ++context_;
    } else {
        context_.err =
            GetError("No one record can be defined inside another record.", Error::ErrorType::ERR_BAD_DEFINITION);
        return;
    }

    if (ParseRecordFullSign()) {
        metadata_ = currRecord_->metadata.get();
        if (ParseMetaDef()) {
            if (!open_ && *context_ == Token::Type::DEL_BRACE_L) {
                currRecord_->bodyLocation.begin = GetCurrentPosition(false);
                ++context_;

                LOG(DEBUG, ASSEMBLER) << "record body is open, line " << lineStric_ << ": " << tokens[0].wholeLine;

                open_ = true;
            }

            if (open_ && !context_.Mask() && *context_ != Token::Type::DEL_BRACE_R) {
                ParseRecordFields();
            } else if (open_) {
                currRecord_->bodyPresence = true;
            }

            if (open_ && *context_ == Token::Type::DEL_BRACE_R) {
                LOG(DEBUG, ASSEMBLER) << "record body is closed, line " << lineStric_ << ": " << tokens[0].wholeLine;

                currRecord_->bodyLocation.end = GetCurrentPosition(true);
                ++context_;

                open_ = false;
            }
        }
    }
}

void Parser::ParseAsFunction(const std::vector<Token> &tokens)
{
    LOG(DEBUG, ASSEMBLER) << "started parsing of function (line " << lineStric_ << "): " << tokens[0].wholeLine;
    recordDef_ = false;
    funcDef_ = true;
    arrayDef_ = false;

    if (!open_) {
        ++context_;
    } else {
        context_.err =
            GetError("No one function can be defined inside another function.", Error::ErrorType::ERR_BAD_DEFINITION);
        return;
    }

    if (ParseFunctionFullSign()) {
        metadata_ = currFunc_->metadata.get();
        if (ParseMetaDef()) {
            if (!open_ && *context_ == Token::Type::DEL_BRACE_L) {
                currFunc_->bodyLocation.begin = GetCurrentPosition(false);
                ++context_;

                LOG(DEBUG, ASSEMBLER) << "function body is open, line " << lineStric_ << ": " << tokens[0].wholeLine;

                open_ = true;
            }

            if (open_ && !context_.Mask() && *context_ != Token::Type::DEL_BRACE_R) {
                ParseFunctionCode();
            } else if (open_) {
                currFunc_->bodyPresence = true;
            }

            if (open_ && *context_ == Token::Type::DEL_BRACE_R) {
                LOG(DEBUG, ASSEMBLER) << "function body is closed, line " << lineStric_ << ": " << tokens[0].wholeLine;

                currFunc_->bodyLocation.end = GetCurrentPosition(true);
                ++context_;
                open_ = false;
            }
        }
        if ((currFunc_->params.empty() ||
             currFunc_->params[0].type.GetName() != pandasm::GetOwnerName(currFunc_->name) ||
             currFunc_->metadata->IsCctor())) {
            currFunc_->metadata->SetAccessFlags(currFunc_->metadata->GetAccessFlags() | ACC_STATIC);
        }
        const auto &it = ambiguousFunctionTable_.find({currFunc_->name, true});
        if (it != ambiguousFunctionTable_.end()) {
            ASSERT(ambiguousFunctionTable_.find({currFunc_->name, false}) != ambiguousFunctionTable_.end());
            const auto &homonymFunc = ambiguousFunctionTable_.at({currFunc_->name, false});
            if (homonymFunc.IsStatic() == it->second.IsStatic()) {
                context_.err = GetError("This function already exists.", Error::ErrorType::ERR_BAD_ID_FUNCTION);
            }
        }
    }
}

void Parser::ParseAsBraceRight(const std::vector<Token> &tokens)
{
    if (!open_) {
        context_.err =
            GetError("Delimiter '}' for the code area is outside a function.", Error::ErrorType::ERR_BAD_BOUND);
        return;
    }

    LOG(DEBUG, ASSEMBLER) << "body is closed (line " << lineStric_ << "): " << tokens[0].wholeLine;

    open_ = false;
    if (funcDef_) {
        currFunc_->bodyLocation.end = GetCurrentPosition(true);
    } else if (recordDef_) {
        currRecord_->bodyLocation.end = GetCurrentPosition(true);
    } else if (arrayDef_) {
        if (!ArrayElementsValidNumber()) {
            context_.err =
                GetError("Constant array must contain at least one element.", Error::ErrorType::ERR_BAD_ARRAY_SIZE);
            return;
        }
    } else {
        LOG(FATAL, ASSEMBLER) << "Internal error: either function or record must be parsed here";
    }
    ++context_;
}

void Parser::ParseResetFunctionParams(bool isHomonym)
{
    for (const auto &t : context_.functionArgumentsLists) {
        auto it = ambiguousFunctionTable_.find({t.first, isHomonym});
        ASSERT(it != ambiguousFunctionTable_.end() || isHomonym);
        if (it == ambiguousFunctionTable_.end()) {
            continue;
        }
        currFunc_ = &(it->second);
        currFunc_->regsNum = static_cast<std::uint32_t>(currFunc_->valueOfFirstParam + 1);

        for (const auto &v : t.second) {
            if (currFunc_->ins.empty() && currFunc_->ins.size() < v.first && currFunc_->ins[v.first - 1].RegEmpty()) {
                continue;
            }
            auto value = currFunc_->ins[v.first - 1].GetReg(v.second);
            value += static_cast<uint16_t>(currFunc_->valueOfFirstParam + 1);
            currFunc_->ins[v.first - 1].SetReg(v.second, value);
            // NOLINTNEXTLINE(hicpp-signed-bitwise)
            size_t maxRegNumber = (1 << currFunc_->ins[v.first - 1].MaxRegEncodingWidth());
            if (currFunc_->ins[v.first - 1].GetReg(v.second) >= maxRegNumber) {
                const auto &debug = currFunc_->ins[v.first - 1].insDebug;
                context_.err = Error("Register width mismatch.", debug.LineNumber(), Error::ErrorType::ERR_BAD_NAME_REG,
                                     "", 0, 0, "");
                SetError();
                break;
            }
        }
    }
}

void Parser::ParseResetFunctionLabelsAndParams()
{
    if (open_ || err_.err != Error::ErrorType::ERR_NONE) {
        return;
    }
    for (const auto &f : ambiguousFunctionTable_) {
        for (const auto &k : f.second.labelTable) {
            if (!k.second.fileLocation.isDefined) {
                context_.err = Error("This label does not exist.", lineStric_, Error::ErrorType::ERR_BAD_LABEL_EXT, "",
                                     k.second.fileLocation.boundLeft, k.second.fileLocation.boundRight,
                                     k.second.fileLocation.WholeLine());
                SetError();
            }
        }
    }

    ParseResetFunctionParams(false);
    ParseResetFunctionParams(true);
}

void Parser::ParseInsFromFuncTable(ark::pandasm::Function &func)
{
    for (auto &insn : func.ins) {
        if (!insn.HasFlag(InstFlags::METHOD_ID) && !insn.HasFlag(InstFlags::STATIC_METHOD_ID)) {
            // we need to correct all method_id's
            continue;
        }

        bool isInitobj = insn.opcode == Opcode::INITOBJ || insn.opcode == Opcode::INITOBJ_SHORT ||
                         insn.opcode == Opcode::INITOBJ_RANGE;
        size_t diff = isInitobj ? 0 : 1;
        auto const &funcName = insn.GetID(0U);

        if (!IsSignatureOrMangled(funcName)) {
            const auto itSynonym = program_.functionSynonyms.find(funcName);
            if (itSynonym == program_.functionSynonyms.end()) {
                continue;
            }

            if (itSynonym->second.size() > 1) {
                const auto &debug = insn.insDebug;
                context_.err = Error("Unable to resolve ambiguous function call", debug.LineNumber(),
                                     Error::ErrorType::ERR_FUNCTION_MULTIPLE_ALTERNATIVES, "", 0, 0, "");
                SetError();
                break;
            }

            insn.SetID(0U, program_.functionSynonyms.at(funcName)[0U]);
        }

        auto foundFunc = ambiguousFunctionTable_.find({insn.GetID(0U), false});
        ASSERT(foundFunc != ambiguousFunctionTable_.end());
        if (insn.OperandListLength() - diff < foundFunc->second.GetParamsNum()) {
            if (insn.IsCallRange() && (static_cast<int>(insn.RegSize()) - static_cast<int>(diff) >= 0)) {
                continue;
            }

            const auto &debug = insn.insDebug;
            context_.err = Error("Function argument mismatch.", debug.LineNumber(),
                                 Error::ErrorType::ERR_FUNCTION_ARGUMENT_MISMATCH, "", 0, 0, "");
            SetError();
        }
    }
}

bool Parser::CheckVirtualCallInsn(const ark::pandasm::Ins &insn)
{
    switch (insn.opcode) {
        case Opcode::CALL_VIRT:
        case Opcode::CALL_VIRT_SHORT:
        case Opcode::CALL_VIRT_RANGE:
        case Opcode::CALL_VIRT_ACC:
        case Opcode::CALL_VIRT_ACC_SHORT: {
            if (program_.functionStaticTable.find(insn.GetID(0U)) != program_.functionStaticTable.cend()) {
                const auto &debug = insn.insDebug;
                context_.err = Error("Virtual instruction calls static method.", debug.LineNumber(),
                                     Error::ErrorType::ERR_BAD_OPERAND, "", 0, 0, "");
                SetError();
                return false;
            }
            return true;
        }
        default:
            return true;
    }
}

void Parser::CheckVirtualCalls(const std::map<std::string, ark::pandasm::Function> &functionTable)
{
    for (const auto &func : functionTable) {
        for (const auto &insn : func.second.ins) {
            if (!CheckVirtualCallInsn(insn)) {
                return;
            }
        }
    }
}

void Parser::ParseResetFunctionTable()
{
    for (auto &[unusedKey, func] : ambiguousFunctionTable_) {
        (void)unusedKey;
        if (!func.fileLocation.isDefined) {
            context_.err = Error("This function does not exist.", func.fileLocation.lineNumber,
                                 Error::ErrorType::ERR_BAD_ID_FUNCTION, "", func.fileLocation.boundLeft,
                                 func.fileLocation.boundRight, func.fileLocation.WholeLine());
            SetError();
        } else if (func.HasImplementation() != func.bodyPresence) {
            context_.err =
                Error("Inconsistency of the definition of the function and its metadata.", func.fileLocation.lineNumber,
                      Error::ErrorType::ERR_BAD_DEFINITION_FUNCTION, "", func.fileLocation.boundLeft,
                      func.fileLocation.boundRight, func.fileLocation.WholeLine());
            SetError();
        } else {
            ParseInsFromFuncTable(func);
        }
    }
}

void Parser::ParseResetRecords(const ark::pandasm::Record &record)
{
    for (const auto &fld : record.fieldList) {
        if (!record.HasImplementation() && !fld.metadata->IsForeign()) {
            context_.err =
                Error("External record may have only external fields", fld.lineOfDef,
                      Error::ErrorType::ERR_BAD_FIELD_METADATA, "", fld.boundLeft, fld.boundRight, fld.wholeLine);
            SetError();
        }
        if (!fld.isDefined) {
            context_.err = Error("This field does not exist.", fld.lineOfDef, Error::ErrorType::ERR_BAD_ID_FIELD, "",
                                 fld.boundLeft, fld.boundRight, fld.wholeLine);
            SetError();
        }
    }
}

void Parser::ParseResetRecordTable()
{
    for (const auto &[recordName, record] : program_.recordTable) {
        if (!record.fileLocation.isDefined && !ark::pandasm::Type::IsPandaPrimitiveType(recordName) &&
            !Type::FromName(recordName).IsUnion()) {
            std::string errorStr = "Record '" + recordName + "' does not exist.";
            context_.err =
                Error(errorStr, record.fileLocation.lineNumber, Error::ErrorType::ERR_BAD_ID_RECORD, "",
                      record.fileLocation.boundLeft, record.fileLocation.boundRight, record.fileLocation.WholeLine());
            SetError();
            return;
        } else {
            ParseResetRecords(record);
        }
    }
}

void Parser::ParseResetTables()
{
    if (err_.err != Error::ErrorType::ERR_NONE) {
        return;
    }

    ParseResetFunctionTable();

    if (err_.err != Error::ErrorType::ERR_NONE) {
        return;
    }

    ParseResetRecordTable();
}

void Parser::ParseAsLanguageDirective()
{
    ++context_;

    if (context_.Mask()) {
        context_.err = GetError("Incorrect .language directive: Expected language",
                                Error::ErrorType::ERR_BAD_DIRECTIVE_DECLARATION);
        return;
    }

    auto lang = context_.GiveToken();
    auto res = ark::panda_file::LanguageFromString(lang);
    if (!res) {
        context_.err =
            GetError("Incorrect .language directive: Unknown language", Error::ErrorType::ERR_UNKNOWN_LANGUAGE);
        return;
    }

    ++context_;

    if (!context_.Mask()) {
        context_.err = GetError("Incorrect .language directive: Unexpected token",
                                Error::ErrorType::ERR_BAD_DIRECTIVE_DECLARATION);
    }

    program_.lang = res.value();
}

Function::CatchBlock Parser::PrepareCatchBlock(bool isCatchall, size_t size, size_t catchallTokensNum,
                                               size_t catchTokensNum)
{
    constexpr size_t TRY_BEGIN = 0;
    constexpr size_t TRY_END = 1;
    constexpr size_t CATCH_BEGIN = 2;
    constexpr size_t CATCH_END = 3;

    Function::CatchBlock catchBlock;
    catchBlock.wholeLine = context_.tokens[0].wholeLine;
    std::vector<std::string> labelNames {"try block begin", "try block end", "catch block begin"};
    std::vector<std::string> labels;
    bool fullCatchBlock = (isCatchall && size == catchallTokensNum) || (!isCatchall && size == catchTokensNum);
    if (fullCatchBlock) {
        labelNames.emplace_back("catch block end");
    }
    if (!isCatchall) {
        catchBlock.exceptionRecord = context_.GiveToken();
        ++context_;
    }

    bool skipComma = isCatchall;
    for (const auto &labelName : labelNames) {
        if (!skipComma) {
            if (*context_ != Token::Type::DEL_COMMA) {
                context_.err = GetError("Expected comma.", Error::ErrorType::ERR_BAD_DIRECTIVE_DECLARATION);
                return catchBlock;
            }
            ++context_;
        }
        skipComma = false;
        if (!LabelValidName()) {
            context_.err =
                GetError(std::string("Invalid name of the ") + labelName + " label.", Error::ErrorType::ERR_BAD_LABEL);
            return catchBlock;
        }
        labels.emplace_back(context_.GiveToken());
        AddObjectInTable(false, *labelTable_);
        ++context_;
    }

    ASSERT(context_.Mask());
    catchBlock.tryBeginLabel = labels[TRY_BEGIN];
    catchBlock.tryEndLabel = labels[TRY_END];
    catchBlock.catchBeginLabel = labels[CATCH_BEGIN];
    if (fullCatchBlock) {
        catchBlock.catchEndLabel = labels[CATCH_END];
    } else {
        catchBlock.catchEndLabel = labels[CATCH_BEGIN];
    }
    return catchBlock;
}

void Parser::ParseAsCatchDirective()
{
    ASSERT(*context_ == Token::Type::ID_CATCH || *context_ == Token::Type::ID_CATCHALL);

    constexpr size_t CATCH_DIRECTIVE_TOKENS_NUM = 8;
    constexpr size_t CATCHALL_DIRECTIVE_TOKENS_NUM = 6;
    constexpr size_t CATCH_FULL_DIRECTIVE_TOKENS_NUM = 10;
    constexpr size_t CATCHALL_FULL_DIRECTIVE_TOKENS_NUM = 8;

    bool isCatchall = *context_ == Token::Type::ID_CATCHALL;
    size_t size = context_.tokens.size();
    if (isCatchall && size != CATCHALL_DIRECTIVE_TOKENS_NUM && size != CATCHALL_FULL_DIRECTIVE_TOKENS_NUM) {
        context_.err = GetError(
            "Incorrect catch block declaration. Must be in the format: .catchall <try_begin_label>, <try_end_label>, "
            "<catch_begin_label>[, <catch_end_label>]",
            Error::ErrorType::ERR_BAD_DIRECTIVE_DECLARATION);
        return;
    }

    if (!isCatchall && size != CATCH_DIRECTIVE_TOKENS_NUM && size != CATCH_FULL_DIRECTIVE_TOKENS_NUM) {
        context_.err = GetError(
            "Incorrect catch block declaration. Must be in the format: .catch <exception_record>, <try_begin_label>, "
            "<try_end_label>, <catch_begin_label>[, <catch_end_label>]",
            Error::ErrorType::ERR_BAD_DIRECTIVE_DECLARATION);
        return;
    }

    ++context_;

    if (!isCatchall && !RecordValidName()) {
        context_.err = GetError("Invalid name of the exception record.", Error::ErrorType::ERR_BAD_RECORD_NAME);
        return;
    }

    Function::CatchBlock catchBlock =
        PrepareCatchBlock(isCatchall, size, CATCHALL_FULL_DIRECTIVE_TOKENS_NUM, CATCH_FULL_DIRECTIVE_TOKENS_NUM);

    currFunc_->catchBlocks.emplace_back(std::move(catchBlock));
}

void Parser::ParseAsCatchall(const std::vector<Token> &tokens)
{
    std::string directiveName = *context_ == Token::Type::ID_CATCH ? ".catch" : ".catchall";
    if (!funcDef_) {
        context_.err = GetError(directiveName + " directive is located outside of a function body.",
                                Error::ErrorType::ERR_INCORRECT_DIRECTIVE_LOCATION);
        return;
    }

    LOG(DEBUG, ASSEMBLER) << "started parsing of " << directiveName << " directive (line " << lineStric_
                          << "): " << tokens[0].wholeLine;

    ParseAsCatchDirective();
}

void Parser::ParseAsLanguage(const std::vector<Token> &tokens, bool &isLangParsed, bool &isFirstStatement)
{
    if (isLangParsed) {
        context_.err = GetError("Multiple .language directives", Error::ErrorType::ERR_MULTIPLE_DIRECTIVES);
        return;
    }

    if (!isFirstStatement) {
        context_.err = GetError(".language directive must be specified before any other declarations",
                                Error::ErrorType::ERR_INCORRECT_DIRECTIVE_LOCATION);
        return;
    }

    LOG(DEBUG, ASSEMBLER) << "started parsing of .language directive (line " << lineStric_
                          << "): " << tokens[0].wholeLine;

    ParseAsLanguageDirective();

    isLangParsed = true;
}

bool Parser::ParseAfterLine(bool &isFirstStatement)
{
    SetError();

    if (!context_.Mask() && err_.err == Error::ErrorType::ERR_NONE) {
        context_.err = GetError("There can be nothing after.", Error::ErrorType::ERR_BAD_END);
    }

    if (err_.err != Error::ErrorType::ERR_NONE) {
        LOG(DEBUG, ASSEMBLER) << "processing aborted (error detected)";
        return false;
    }

    LOG(DEBUG, ASSEMBLER) << "parsing of line " << lineStric_ << " is successful";

    SetError();

    isFirstStatement = false;

    return true;
}

Expected<Program, Error> Parser::ParseAfterMainLoop(const std::string &fileName)
{
    ParseResetFunctionLabelsAndParams();

    if (open_ && err_.err == Error::ErrorType::ERR_NONE) {
        context_.err = Error("Code area is not closed.", currFunc_->fileLocation.lineNumber,
                             Error::ErrorType::ERR_BAD_CLOSE, "", 0, currFunc_->name.size(), currFunc_->name);
        SetError();
    }

    ParseResetTables();

    if (err_.err != Error::ErrorType::ERR_NONE) {
        return Unexpected(err_);
    }

    for (auto &[key, func] : ambiguousFunctionTable_) {
        if (func.metadata->HasImplementation()) {
            func.sourceFile = fileName;
        }
        if (func.IsStatic()) {
            program_.functionStaticTable.emplace(key.first, std::move(func));
        } else {
            program_.functionInstanceTable.emplace(key.first, std::move(func));
        }
    }
    ambiguousFunctionTable_.clear();

    CheckVirtualCalls(program_.functionStaticTable);
    CheckVirtualCalls(program_.functionInstanceTable);
    if (err_.err != Error::ErrorType::ERR_NONE) {
        return Unexpected(err_);
    }

    for (auto &rec : program_.recordTable) {
        if (rec.second.HasImplementation()) {
            rec.second.sourceFile = fileName;
        }
    }

    return std::move(program_);
}

void Parser::ParseContextByType(const std::vector<Token> &tokens, bool &isLangParsed, bool &isFirstStatement)
{
    switch (*context_) {
        case Token::Type::ID_CATCH:
        case Token::Type::ID_CATCHALL: {
            ParseAsCatchall(tokens);
            break;
        }
        case Token::Type::ID_LANG: {
            ParseAsLanguage(tokens, isLangParsed, isFirstStatement);
            break;
        }
        case Token::Type::ID_REC: {
            ParseAsRecord(tokens);
            break;
        }
        case Token::Type::ID_FUN: {
            ParseAsFunction(tokens);
            break;
        }
        case Token::Type::ID_ARR: {
            ParseAsArray(tokens);
            break;
        }
        case Token::Type::DEL_BRACE_R: {
            ParseAsBraceRight(tokens);
            break;
        }
        default: {
            if (funcDef_) {
                ParseFunctionCode();
            } else if (recordDef_) {
                ParseRecordFields();
            } else if (arrayDef_) {
                ParseArrayElements();
            }
        }
    }
}

Expected<Program, Error> Parser::Parse(TokenSet &vectorsTokens, const std::string &fileName)
{
    bool isLangParsed = false;
    bool isFirstStatement = true;
    for (const auto &tokens : vectorsTokens) {
        ++lineStric_;
        if (tokens.empty()) {
            continue;
        }
        LOG(DEBUG, ASSEMBLER) << "started parsing of line " << lineStric_ << ": " << tokens[0].wholeLine;
        context_.Make(tokens);

        ParseContextByType(tokens, isLangParsed, isFirstStatement);

        if (!ParseAfterLine(isFirstStatement)) {
            break;
        }
    }

    return ParseAfterMainLoop(fileName);
}

Expected<Program, Error> Parser::Parse(const std::string &source, const std::string &fileName)
{
    auto ss = std::stringstream(source);
    std::string line;

    Lexer l;
    std::vector<std::vector<Token>> v;

    while (std::getline(ss, line)) {
        auto [tokens, error] = l.TokenizeString(line);
        if (error.err != Error::ErrorType::ERR_NONE) {
            return Unexpected(error);
        }

        v.push_back(tokens);
    }

    return Parse(v, fileName);
}

void Parser::SetError()
{
    err_ = context_.err;
}

bool Parser::RegValidName()
{
    if (context_.err.err != Error::ErrorType::ERR_NONE) {
        return false;
    }

    if (currFunc_->GetParamsNum() > 0) {
        return context_.ValidateRegisterName('v') || context_.ValidateRegisterName('a', currFunc_->GetParamsNum() - 1);
    }

    return context_.ValidateRegisterName('v');
}

bool Parser::ParamValidName()
{
    return context_.ValidateParameterName(currFunc_->GetParamsNum());
}

bool Parser::PrefixedValidNameToken(BracketOptions options, bool isUnion)
{
    auto s = context_.GiveToken();
    for (size_t i = 1; i < s.size(); ++i) {
        char const c = s[i];
        if (c == '.') {
            if (i + 1 >= s.size()) {
                return false;
            }
            if (IsNonDigit(s[i + 1])) {
                continue;
            }
            if (!IsAllowAngleBrackets(options) || s[i + 1] != '<') {
                return false;
            }
            continue;
        }
        if (IsValidNameCharacter(c)) {
            continue;
        }
        if (IsAllowBrackets(options) && (c == '[' || c == ']')) {
            continue;
        }
        if (IsAllowAngleBrackets(options) && (c == '<' || c == '>')) {
            continue;
        }
        return isUnion && (c == ',');
    }
    return true;
}

bool Parser::PrefixedValidUnionName(BracketOptions options)
{
    context_++;
    while (*context_ != Token::Type::DEL_BRACE_R) {
        if (*context_ == Token::Type::DEL_BRACE_L) {
            if (!PrefixedValidUnionName(options)) {
                return false;
            }
        } else {
            if (!PrefixedValidNameToken(options, true)) {
                return false;
            }
        }
        context_++;
    }
    return true;
}

bool Parser::PrefixedValidName(BracketOptions options)
{
    if (!IsNonDigit(context_.GiveToken()[0])) {
        return false;
    }
    if (*context_ != Token::Type::DEL_BRACE_L) {
        return PrefixedValidNameToken(options);
    }
    if (!PrefixedValidUnionName(options)) {
        return false;
    }
    context_++;
    if (!context_.Mask()) {
        if (*context_ == Token::Type::DEL_SQUARE_BRACKET_L) {
            // process '[' and ']'
            if (!PrefixedValidNameToken(options)) {
                return false;
            }
        } else {
            context_--;
        }
    }

    // move to the start of union name
    auto idx = [&]() {
        if (*context_ == Token::Type::DEL_BRACE_R) {
            context_--;
            return 1;
        }
        return 0;
    }();
    while (*context_ != Token::Type::DEL_BRACE_L || idx != 0) {
        if (*context_ == Token::Type::DEL_BRACE_R) {
            idx++;
        }
        if (*context_ == Token::Type::DEL_BRACE_L) {
            idx--;
            if (idx == 0) {
                break;
            }
        }
        context_--;
    }
    return true;
}

bool Parser::RecordValidName()
{
    return PrefixedValidName(BracketOptions::ALLOW_BRACKETS);
}

bool Parser::FunctionValidName()
{
    return PrefixedValidName(BracketOptions::ALL_BRACKETS);
}

bool Parser::ArrayValidName()
{
    return PrefixedValidName();
}

bool Parser::LabelValidName()
{
    auto token = context_.GiveToken();
    if (!IsNonDigit(token[0])) {
        return false;
    }

    token.remove_prefix(1);

    for (auto i : token) {
        if (!IsValidNameCharacter(i)) {
            return false;
        }
    }

    return true;
}

bool Parser::ParseLabel()
{
    LOG(DEBUG, ASSEMBLER) << "label search started (line " << lineStric_ << "): " << context_.tokens[0].wholeLine;

    context_++;

    if (*context_ == Token::Type::DEL_COLON) {
        context_--;
        if (LabelValidName()) {
            if (AddObjectInTable(true, *labelTable_)) {
                currIns_->SetLabel(context_.GiveToken());

                LOG(DEBUG, ASSEMBLER) << "label detected (line " << lineStric_ << "): " << context_.GiveToken();

                context_++;
                context_++;
                return !context_.Mask();
            }

            LOG(DEBUG, ASSEMBLER) << "label is detected (line " << lineStric_ << "): " << context_.GiveToken()
                                  << ", but this label already exists";

            context_.err = GetError("This label already exists.", Error::ErrorType::ERR_BAD_LABEL_EXT);
        } else {
            LOG(DEBUG, ASSEMBLER) << "label with non-standard character is detected, attempt to create a label is "
                                     "supposed, but this cannot be any label name (line "
                                  << lineStric_ << "): " << context_.GiveToken();

            context_.err = GetError(
                "Invalid name of label. Label contains only characters: '_', '0' - '9', 'a' - 'z', 'A' - 'Z'; and "
                "starts with any letter or with '_'.",
                Error::ErrorType::ERR_BAD_LABEL);
        }

        return false;
    }

    context_--;

    LOG(DEBUG, ASSEMBLER) << "label is not detected (line " << lineStric_ << ")";

    return true;
}

static Opcode TokenToOpcode(Token::Type id)
{
    ASSERT(id > Token::Type::OPERATION && id < Token::Type::KEYWORD);
    using Utype = std::underlying_type_t<Token::Type>;
    return static_cast<Opcode>(static_cast<Utype>(id) - static_cast<Utype>(Token::Type::OPERATION) - 1);
}

bool Parser::ParseOperation()
{
    if (context_.Mask()) {
        LOG(DEBUG, ASSEMBLER) << "no more tokens (line " << lineStric_ << "): " << context_.tokens[0].wholeLine;

        return false;
    }

    if (open_ && *context_ == Token::Type::DEL_BRACE_R) {
        return false;
    }

    LOG(DEBUG, ASSEMBLER) << "operaion search started (line " << lineStric_ << "): " << context_.tokens[0].wholeLine;

    if (*context_ > Token::Type::OPERATION && *context_ < Token::Type::KEYWORD) {
        SetOperationInformation();

        context_.UpSignOperation();
        currIns_->opcode = TokenToOpcode(context_.id);

        LOG(DEBUG, ASSEMBLER) << "operatiuon is detected (line " << lineStric_ << "): " << context_.GiveToken()
                              << " (operand type: " << OperandTypePrint(currIns_->opcode) << ")";

        context_++;
        return true;
    }

    LOG(DEBUG, ASSEMBLER) << "founded " << context_.GiveToken() << ", it is not an operation (line " << lineStric_
                          << ")";

    context_.err = GetError("Invalid operation.", Error::ErrorType::ERR_BAD_OPERATION_NAME);

    return false;
}

bool Parser::ParseOperandVreg()
{
    if (context_.err.err != Error::ErrorType::ERR_NONE) {
        return false;
    }

    if (*context_ != Token::Type::ID) {
        context_.err = GetError("Expected register.", Error::ErrorType::ERR_BAD_OPERAND, +1);
        return false;
    }

    std::string_view p = context_.GiveToken();
    if (p[0] == 'v') {
        p.remove_prefix(1);
        auto number = static_cast<int64_t>(ToNumber(p));
        if (number > *(context_.maxValueOfReg)) {
            *(context_.maxValueOfReg) = number;
        }

        currIns_->EmplaceReg(static_cast<uint16_t>(number));
    } else if (p[0] == 'a') {
        p.remove_prefix(1);
        currIns_->EmplaceReg(static_cast<uint16_t>(ToNumber(p)));
        context_.functionArgumentsList->emplace_back(context_.insNumber, currIns_->RegSize() - 1);
    }

    ++context_;
    return true;
}

bool Parser::ParseOperandCall()
{
    if (context_.err.err != Error::ErrorType::ERR_NONE) {
        return false;
    }

    if (!FunctionValidName()) {
        context_.err = GetError("Invalid name of function.", Error::ErrorType::ERR_BAD_NAME_REG);
        return false;
    }

    currIns_->EmplaceID(context_.GiveToken().data(), context_.GiveToken().length());

    ++context_;

    std::string funcSignature {};

    if (!ParseOperandSignature(&funcSignature)) {
        return false;
    }

    if (funcSignature.empty()) {
        const auto itSynonym = program_.functionSynonyms.find(currIns_->BackID());
        if (itSynonym == program_.functionSynonyms.end()) {
            [[maybe_unused]] auto it = ambiguousFunctionTable_.find({currIns_->BackID(), false});
            ASSERT(it == ambiguousFunctionTable_.end() || !it->second.fileLocation.isDefined);
            AddObjectInTable(false, ambiguousFunctionTable_, currIns_->BackID());
            return true;
        }

        if (itSynonym->second.size() > 1) {
            context_.err = GetError("Unable to resolve ambiguous function call",
                                    Error::ErrorType::ERR_FUNCTION_MULTIPLE_ALTERNATIVES);
            return false;
        }
    } else {
        currIns_->BackID() += funcSignature;
        AddObjectInTable(false, ambiguousFunctionTable_, currIns_->BackID());
    }

    return true;
}

bool Parser::ParseOperandSignature(std::string *sign)
{
    if (*context_ != Token::Type::DEL_COLON) {
        // no signature provided
        return true;
    }

    ++context_;

    if (*context_ != Token::Type::DEL_BRACKET_L) {
        context_.err = GetError("Expected \'(\' before signature", Error::ErrorType::ERR_BAD_SIGNATURE);
        return false;
    }

    ++context_;

    *sign += ":(";

    if (!ParseOperandSignatureTypesList(sign)) {
        return false;
    }

    if (*context_ != Token::Type::DEL_BRACKET_R) {
        context_.err = GetError("Expected \')\' at the end of the signature", Error::ErrorType::ERR_BAD_SIGNATURE);
        return false;
    }

    *sign += ")";

    ++context_;

    return true;
}

bool Parser::ParseOperandSignatureTypesList(std::string *sign)
{
    bool comma = false;

    while (true) {
        if (context_.Mask()) {
            return true;
        }

        if (*context_ != Token::Type::DEL_COMMA && *context_ != Token::Type::ID &&
            *context_ != Token::Type::DEL_BRACE_L) {
            break;
        }

        if (comma) {
            *sign += ",";
        }

        if (!ParseFunctionArgComma(comma)) {
            return false;
        }

        if (*context_ != Token::Type::ID && *context_ != Token::Type::DEL_BRACE_L) {
            context_.err = GetError("Expected signature arguments", Error::ErrorType::ERR_BAD_SIGNATURE_PARAMETERS);
            return false;
        }

        if (!TypeValidName()) {
            context_.err = GetError("Expected valid type", Error::ErrorType::ERR_BAD_TYPE);
            return false;
        }

        Type type;
        if (!ParseType(&type)) {
            return false;
        }

        *sign += type.GetName();
    }

    return true;
}

static bool IsOctal(char c)
{
    return c >= '0' && c <= '7';
}

static bool IsHex(char c)
{
    return std::isxdigit(c) != 0;
}

static uint8_t FromHex(char c)
{
    constexpr size_t DIGIT_NUM = 10;

    if (c >= '0' && c <= '9') {
        return c - '0';
    }

    if (c >= 'A' && c <= 'F') {
        return c - 'A' + DIGIT_NUM;
    }

    return c - 'a' + DIGIT_NUM;
}

static uint8_t FromOctal(char c)
{
    return c - '0';
}

Expected<char, Error> Parser::ParseOctalEscapeSequence(std::string_view s, size_t *i)
{
    constexpr size_t OCT_SHIFT = 3;

    size_t idx = *i;
    size_t n = 0;
    uint32_t r = 0;

    while (idx < s.length() && IsOctal(s[idx]) && n < OCT_SHIFT) {
        r |= FromOctal(s[idx++]);
        r <<= 3U;
        ++n;
    }

    r >>= 3U;
    *i += n;

    return r;
}

Expected<char, Error> Parser::ParseHexEscapeSequence(std::string_view s, size_t *i)
{
    constexpr size_t HEX_SHIFT = 2;

    uint32_t r = 0;
    size_t idx = *i;

    for (size_t j = 0; j < HEX_SHIFT; j++) {
        char v = s[(*i)++];

        if (!IsHex(v)) {
            return Unexpected(GetError("Invalid \\x escape sequence",
                                       Error::ErrorType::ERR_BAD_STRING_INVALID_HEX_ESCAPE_SEQUENCE, idx - HEX_SHIFT));
        }

        r |= FromHex(v);
        r <<= 4U;
    }

    r >>= 4U;

    return r;
}

Expected<char, Error> Parser::ParseEscapeSequence(std::string_view s, size_t *i)
{
    size_t idx = *i;

    char c = s[idx];

    if (IsOctal(c)) {
        return ParseOctalEscapeSequence(s, i);
    }

    ++(*i);

    switch (c) {
        case '\'':
        case '"':
        case '\\':
            return c;
        case 'a':
            return '\a';
        case 'b':
            return '\b';
        case 'f':
            return '\f';
        case 'n':
            return '\n';
        case 'r':
            return '\r';
        case 't':
            return '\t';
        case 'v':
            return '\v';
        default:
            break;
    }

    if (c == 'x') {
        return ParseHexEscapeSequence(s, i);
    }

    return Unexpected(
        GetError("Unknown escape sequence", Error::ErrorType::ERR_BAD_STRING_UNKNOWN_ESCAPE_SEQUENCE, idx - 1));
}

std::optional<std::string> Parser::ParseStringLiteral()
{
    auto token = context_.GiveToken();
    if (*context_ != Token::Type::ID_STRING) {
        context_.err = GetError("Expected string literal", Error::ErrorType::ERR_BAD_OPERAND);
        return {};
    }

    size_t i = 1; /* skip leading quote */
    size_t len = token.length();

    std::string s;

    while (i < len - 1) {
        char c = token[i++];

        if (c != '\\') {
            s.append(1, c);
            continue;
        }

        auto res = ParseEscapeSequence(token, &i);
        if (!res) {
            context_.err = res.Error();
            return {};
        }

        s.append(1, res.Value());
    }

    program_.strings.insert(s);

    return s;
}

bool Parser::ParseOperandString()
{
    if (context_.err.err != Error::ErrorType::ERR_NONE) {
        return false;
    }

    auto res = ParseStringLiteral();
    if (!res) {
        return false;
    }

    currIns_->EmplaceID(std::move(*res));

    ++context_;

    return true;
}

bool Parser::ParseOperandComma()
{
    if (context_.err.err != Error::ErrorType::ERR_NONE) {
        return false;
    }

    if (context_++ != Token::Type::DEL_COMMA) {
        if (!context_.Mask() && *context_ != Token::Type::DEL_BRACKET_R) {
            --context_;
        }

        context_.err = GetError("Expected comma.", Error::ErrorType::ERR_BAD_NUMBER_OPERANDS);
        return false;
    }

    return true;
}

bool Parser::ParseInteger(int64_t *value)
{
    if (context_.err.err != Error::ErrorType::ERR_NONE) {
        return false;
    }

    if (*context_ != Token::Type::ID) {
        if (*context_ == Token::Type::DEL_BRACE_R) {
            --context_;
        }
        context_.err = GetError("Expected immediate.", Error::ErrorType::ERR_BAD_OPERAND, +1);
        return false;
    }

    std::string_view p = context_.GiveToken();
    if (!ValidateInteger(p)) {
        context_.err = GetError("Expected integer.", Error::ErrorType::ERR_BAD_INTEGER_NAME);
        return false;
    }

    *value = IntegerNumber(p);
    if (errno == ERANGE) {
        context_.err =
            GetError("Too large immediate (length is more than 64 bit).", Error::ErrorType::ERR_BAD_INTEGER_WIDTH);
        return false;
    }

    return true;
}

bool Parser::ParseFloat(double *value, bool is64bit)
{
    if (context_.err.err != Error::ErrorType::ERR_NONE) {
        return false;
    }

    if (*context_ != Token::Type::ID) {
        if (*context_ == Token::Type::DEL_BRACE_R) {
            --context_;
        }
        context_.err = GetError("Expected immediate.", Error::ErrorType::ERR_BAD_OPERAND, +1);
        return false;
    }

    std::string_view p = context_.GiveToken();
    if (!ValidateFloat(p)) {
        context_.err = GetError("Expected float.", Error::ErrorType::ERR_BAD_FLOAT_NAME);
        return false;
    }

    *value = FloatNumber(p, is64bit);
    if (errno == ERANGE) {
        context_.err =
            GetError("Too large immediate (length is more than 64 bit).", Error::ErrorType::ERR_BAD_FLOAT_WIDTH);
        return false;
    }

    return true;
}

bool Parser::ParseOperandInteger()
{
    int64_t n;
    if (!ParseInteger(&n)) {
        return false;
    }

    currIns_->EmplaceImm(n);
    ++context_;
    return true;
}

bool Parser::ParseOperandFloat(bool is64bit)
{
    double n;
    if (!ParseFloat(&n, is64bit)) {
        return false;
    }

    currIns_->EmplaceImm(n);
    ++context_;
    return true;
}

bool Parser::ParseOperandLabel()
{
    if (context_.err.err != Error::ErrorType::ERR_NONE) {
        return false;
    }

    if (!LabelValidName()) {
        context_.err = GetError("Invalid name of Label.", Error::ErrorType::ERR_BAD_NAME_ID);
        return false;
    }

    std::string_view p = context_.GiveToken();
    currIns_->EmplaceID(p.data(), p.length());
    AddObjectInTable(false, *labelTable_);

    ++context_;

    return true;
}

bool Parser::ParseOperandId()
{
    if (context_.err.err != Error::ErrorType::ERR_NONE) {
        return false;
    }

    if (*context_ != Token::Type::ID) {
        context_.err = GetError("Expected Label.", Error::ErrorType::ERR_BAD_OPERAND);
        return false;
    }
    if (!LabelValidName()) {
        context_.err = GetError("Invalid name of Label.", Error::ErrorType::ERR_BAD_NAME_ID);
        return false;
    }

    std::string_view p = context_.GiveToken();
    currIns_->EmplaceID(p.data(), p.length());
    AddObjectInTable(false, *labelTable_);

    ++context_;

    return true;
}

void Parser::ParseComponentOperandTypeIfNeeded(Type type)
{
    if (type.IsArray()) {
        auto componentType = Type::FromName(type.GetComponentName());
        if (ark::pandasm::Type::IsPandaPrimitiveType(componentType.GetName()) ||
            program_.recordTable.find(componentType.GetName()) != program_.recordTable.end()) {
            return;
        }
        ParseComponentOperandTypeIfNeeded(componentType);
        return;
    }
    if (!type.IsUnion()) {
        AddObjectInTable(false, program_.recordTable, type.GetName());
        return;
    }

    for (const auto &constituentType : type.GetComponentNames()) {
        ParseComponentOperandTypeIfNeeded(Type::FromName(constituentType));
    }
    auto res = TryEmplaceInTable(false, program_.recordTable, type.GetName(), type.GetName());
    res.first->second.metadata->SetAttribute("external");
}

bool Parser::ParseOperandType(Type::VerificationType verType)
{
    if (context_.err.err != Error::ErrorType::ERR_NONE) {
        return false;
    }

    if (*context_ != Token::Type::ID && *context_ != Token::Type::DEL_BRACE_L) {
        context_.err = GetError("Expected type.", Error::ErrorType::ERR_BAD_OPERAND);
        return false;
    }
    if (!TypeValidName()) {
        context_.err = GetError("Invalid name of type.", Error::ErrorType::ERR_BAD_NAME_ID);
        return false;
    }

    Type type;
    if (!ParseType(&type)) {
        return false;
    }

    if (type.IsArray()) {
        if (verType == Type::VerificationType::TYPE_ID_OBJECT) {
            GetWarning("Unexpected type_id recieved! Expected object, but array given",
                       Error::ErrorType::WAR_UNEXPECTED_TYPE_ID);
        }
    } else {
        if (verType == Type::VerificationType::TYPE_ID_ARRAY) {
            GetWarning("Unexpected type_id recieved! Expected array, but object given",
                       Error::ErrorType::WAR_UNEXPECTED_TYPE_ID);
        }
    }
    ParseComponentOperandTypeIfNeeded(type);

    currIns_->EmplaceID(type.GetName());

    return true;
}

bool Parser::ParseOperandLiteralArray()
{
    if (context_.err.err != Error::ErrorType::ERR_NONE) {
        return false;
    }
    if (*context_ != Token::Type::ID) {
        context_.err = GetError("Expected array id.", Error::ErrorType::ERR_BAD_OPERAND);
        return false;
    }
    if (!ArrayValidName()) {
        context_.err = GetError("Invalid name of array.", Error::ErrorType::ERR_BAD_NAME_ID);
        return false;
    }

    std::string_view p = context_.GiveToken();
    auto arrayId = std::string(p.data(), p.length());
    if (program_.literalarrayTable.find(arrayId) == program_.literalarrayTable.end()) {
        context_.err = GetError("No array was found for this array id", Error::ErrorType::ERR_BAD_ID_ARRAY);
        return false;
    }

    currIns_->EmplaceID(p.data(), p.length());

    ++context_;

    return true;
}

bool Parser::ParseOperandField()
{
    if (context_.err.err != Error::ErrorType::ERR_NONE) {
        return false;
    }

    if (*context_ != Token::Type::ID) {
        context_.err = GetError("Expected field.", Error::ErrorType::ERR_BAD_OPERAND);
        return false;
    }
    if (!PrefixedValidName()) {
        context_.err = GetError("Invalid name of field.", Error::ErrorType::ERR_BAD_NAME_ID);
        return false;
    }

    std::string_view p = context_.GiveToken();
    std::string recordFullName = std::string(p);

    // Some names of records in pandastdlib starts with 'panda.', therefore,
    // the record name is before the second dot, and the field name is after the second dot
    auto posPoint = recordFullName.find_last_of('.');
    std::string recordName = recordFullName.substr(0, posPoint);
    std::string fieldName = recordFullName.substr(posPoint + 1);

    auto itRecord = program_.recordTable.find(recordName);
    if (itRecord == program_.recordTable.end()) {
        context_.token = recordName;
        AddObjectInTable(false, program_.recordTable);
        itRecord = program_.recordTable.find(recordName);
    }

    auto isStatic = currIns_->HasFlag(InstFlags::STATIC_FIELD_ID);
    auto itField = std::find_if(
        itRecord->second.fieldList.begin(), itRecord->second.fieldList.end(),
        [&fieldName, &isStatic](const pandasm::Field &field) { return IsSameField(field, fieldName, isStatic); });
    if (!fieldName.empty() && itField == itRecord->second.fieldList.end()) {
        itRecord->second.fieldList.emplace_back(program_.lang);
        auto &field = itRecord->second.fieldList.back();
        field.name = fieldName;
        field.lineOfDef = lineStric_;
        field.wholeLine = context_.tokens[context_.number - 1].wholeLine;
        field.boundLeft = context_.tokens[context_.number - 1].boundLeft + recordName.length() + 1;
        field.boundRight = context_.tokens[context_.number - 1].boundRight;
        field.isDefined = false;
        if (isStatic) {
            field.metadata->SetAccessFlags(ACC_STATIC);
        }
    }

    currIns_->EmplaceID(p.data(), p.length());
    ++context_;

    return true;
}

bool Parser::ParseOperandNone()
{
    if (context_.err.err != Error::ErrorType::ERR_NONE) {
        return false;
    }

    if (open_ && *context_ == Token::Type::DEL_BRACE_R) {
        if (context_.Mask()) {
            return true;
        }
        return false;
    }

    if (!context_.Mask()) {
        context_.err = GetError("Invalid number of operands.", Error::ErrorType::ERR_BAD_NUMBER_OPERANDS);
        --context_;
        return false;
    }
    return true;
}

bool Parser::ParseRecordFullSign()
{
    return ParseRecordName();
}

bool Parser::ParseFunctionFullSign()
{
    if (!ParseFunctionReturn()) {
        return false;
    }

    if (!ParseFunctionName()) {
        return false;
    }

    if (*context_ == Token::Type::DEL_BRACKET_L) {
        ++context_;

        if (ParseFunctionArgs()) {
            if (*context_ == Token::Type::DEL_BRACKET_R) {
                ++context_;
                return UpdateFunctionName();
            }
            context_.err = GetError("Expected ')'.", Error::ErrorType::ERR_BAD_ARGS_BOUND);
        }
    } else {
        context_.err = GetError("Expected '('.", Error::ErrorType::ERR_BAD_ARGS_BOUND);
    }

    return false;
}

bool Parser::UpdateFunctionName()
{
    auto signature = GetFunctionSignatureFromName(currFunc_->name, currFunc_->params);
    auto iter = ambiguousFunctionTable_.find({signature, false});
    if (iter == ambiguousFunctionTable_.end()) {
        program_.functionSynonyms[currFunc_->name].push_back(signature);

        auto nodeHandle = ambiguousFunctionTable_.extract({currFunc_->name, false});
        nodeHandle.key() = {signature, false};
        ambiguousFunctionTable_.insert(std::move(nodeHandle));
    } else if (!iter->second.fileLocation.isDefined) {
        program_.functionSynonyms[currFunc_->name].push_back(signature);

        ambiguousFunctionTable_.erase({signature, false});
        auto nodeHandle = ambiguousFunctionTable_.extract({currFunc_->name, false});
        nodeHandle.key() = {signature, false};
        ambiguousFunctionTable_.insert(std::move(nodeHandle));
    } else {
        iter = ambiguousFunctionTable_.find({signature, true});
        if (iter == ambiguousFunctionTable_.end()) {
            auto nodeHandle = ambiguousFunctionTable_.extract({currFunc_->name, false});
            nodeHandle.key() = {signature, true};
            ambiguousFunctionTable_.insert(std::move(nodeHandle));
        } else {
            ASSERT(iter->second.fileLocation.isDefined);
            context_.err = GetError("This function already exists.", Error::ErrorType::ERR_BAD_ID_FUNCTION);
            return false;
        }
    }

    currFunc_->name = signature;
    context_.maxValueOfReg = &(currFunc_->valueOfFirstParam);
    context_.functionArgumentsList = &(context_.functionArgumentsLists[currFunc_->name]);

    return true;
}

bool Parser::ParseArrayFullSign()
{
    if (!ParseArrayName()) {
        return false;
    }

    if (*context_ == Token::Type::DEL_BRACE_L) {
        isConstArray_ = false;
        return true;
    }

    isConstArray_ = true;

    currArray_->literals.emplace_back();
    currArrayElem_ = &(currArray_->literals[currArray_->literals.size() - 1]);

    if (!ParseArrayElementType()) {
        context_.err = GetError("Invalid array type for static array.", Error::ErrorType::ERR_BAD_ARRAY_TYPE);
        return false;
    }

    currArrayElem_->value = static_cast<uint8_t>(currArrayElem_->tag);
    currArrayElem_->tag = panda_file::LiteralTag::TAGVALUE;

    if (*context_ == Token::Type::DEL_BRACE_L) {
        context_.err = GetError("No array size for static array.", Error::ErrorType::ERR_BAD_ARRAY_SIZE);
        return false;
    }

    currArray_->literals.emplace_back();
    currArrayElem_ = &(currArray_->literals[currArray_->literals.size() - 1]);
    currArrayElem_->tag = panda_file::LiteralTag::INTEGER;

    if (!ParseArrayElementValueInteger()) {
        context_.err =
            GetError("Invalid value for static array size value.", Error::ErrorType::ERR_BAD_ARRAY_SIZE_VALUE);
        return false;
    }

    ++context_;

    return true;
}

void Parser::GetUnionName(std::string *name)
{
    ASSERT(*context_ == Token::Type::DEL_BRACE_L);
    context_++;
    while (*context_ != Token::Type::DEL_BRACE_R) {
        if (*context_ == Token::Type::DEL_BRACE_L) {
            (*name) += std::string(context_.GiveToken());
            GetUnionName(name);
        } else {
            (*name) += std::string(context_.GiveToken());
        }
        context_++;
    }
    (*name) += std::string(context_.GiveToken());
}

void Parser::GetName(std::string *name)
{
    auto isUnion = (*context_ == Token::Type::DEL_BRACE_L);
    *name = std::string(context_.GiveToken());
    if (isUnion) {
        GetUnionName(name);
    }
}

bool Parser::ParseRecordName()
{
    LOG(DEBUG, ASSEMBLER) << "started searching for record name (line " << lineStric_
                          << "): " << context_.tokens[context_.number - 1].wholeLine;

    if (!RecordValidName()) {
        if (*context_ == Token::Type::DEL_BRACKET_L) {
            context_.err = GetError("No record name.", Error::ErrorType::ERR_BAD_RECORD_NAME);
            return false;
        }
        context_.err = GetError("Invalid name of the record.", Error::ErrorType::ERR_BAD_RECORD_NAME);
        return false;
    }
    std::string recordName;
    GetName(&recordName);
    context_++;
    if (*context_ == Token::Type::DEL_SQUARE_BRACKET_L) {
        auto dim = ParseMultiArrayHallmark();
        if (context_.err.err != Error::ErrorType::ERR_NONE) {
            return false;
        }
        for (uint8_t i = 0; i < dim; i++) {
            recordName += "[]";
        }
    }
    if (!context_.Mask()) {
        context_--;
    }

    auto recordType = Type::FromName(recordName);
    auto iter = program_.recordTable.find(recordType.GetName());
    if (iter == program_.recordTable.end() || !iter->second.fileLocation.isDefined) {
        SetRecordInformation(recordType.GetName());
    } else {
        context_.err = GetError("This record already exists.", Error::ErrorType::ERR_BAD_ID_RECORD);
        return false;
    }

    LOG(DEBUG, ASSEMBLER) << "record name found (line " << lineStric_ << "): " << context_.GiveToken();

    ++context_;

    return true;
}

void Parser::SetRecordInformation(const std::string &recordName)
{
    AddObjectInTable(true, program_.recordTable, recordName);
    currRecord_ = &(program_.recordTable.at(recordName));
}

bool Parser::ParseFunctionName()
{
    LOG(DEBUG, ASSEMBLER) << "started searching for function name (line " << lineStric_
                          << "): " << context_.tokens[context_.number - 1].wholeLine;

    if (!FunctionValidName()) {
        if (*context_ == Token::Type::DEL_BRACKET_L) {
            context_.err = GetError("No function name.", Error::ErrorType::ERR_BAD_FUNCTION_NAME);
            return false;
        }
        context_.err = GetError("Invalid name of the function.", Error::ErrorType::ERR_BAD_FUNCTION_NAME);
        return false;
    }

    // names are mangled, so no need to check for same names here
    SetFunctionInformation();
    if (context_.err.err != Error::ErrorType::ERR_NONE) {
        return false;
    }

    LOG(DEBUG, ASSEMBLER) << "function name found (line " << lineStric_ << "): " << context_.GiveToken();

    ++context_;

    return true;
}

void Parser::SetFunctionInformation()
{
    auto funcName = std::string(context_.GiveToken());

    context_++;
    if (*context_ == Token::Type::DEL_SQUARE_BRACKET_L || context_.Mask()) {
        auto dim = ParseMultiArrayHallmark();
        for (uint8_t i = 0; i < dim; i++) {
            funcName += "[]";
        }
        if (context_.GiveToken() == ".ctor") {
            funcName += ".ctor";
            AddObjectInTable(true, ambiguousFunctionTable_, funcName);
        }
    } else {
        context_--;
        AddObjectInTable(true, ambiguousFunctionTable_);
    }
    currFunc_ = &(ambiguousFunctionTable_.at({funcName, false}));
    labelTable_ = &(currFunc_->labelTable);
    currFunc_->returnType = context_.currFuncReturnType;
}

bool Parser::ParseArrayName()
{
    LOG(DEBUG, ASSEMBLER) << "started searching for array name (line " << lineStric_
                          << "): " << context_.tokens[context_.number - 1].wholeLine;

    if (!ArrayValidName()) {
        if (*context_ == Token::Type::DEL_BRACKET_L) {
            context_.err = GetError("No array name.", Error::ErrorType::ERR_BAD_ARRAY_NAME);
            return false;
        }
        context_.err = GetError("Invalid name of the array.", Error::ErrorType::ERR_BAD_ARRAY_NAME);
        return false;
    }

    auto iter =
        program_.literalarrayTable.find(std::string(context_.GiveToken().data(), context_.GiveToken().length()));
    if (iter == program_.literalarrayTable.end()) {
        SetArrayInformation();
    } else {
        context_.err = GetError("This array already exists.", Error::ErrorType::ERR_BAD_ID_ARRAY);
        return false;
    }

    LOG(DEBUG, ASSEMBLER) << "array id found (line " << lineStric_ << "): " << context_.GiveToken();

    ++context_;

    return true;
}

void Parser::SetArrayInformation()
{
    program_.literalarrayTable.try_emplace(std::string(context_.GiveToken().data(), context_.GiveToken().length()),
                                           ark::pandasm::LiteralArray());

    currArray_ =
        &(program_.literalarrayTable.at(std::string(context_.GiveToken().data(), context_.GiveToken().length())));
}

void Parser::SetOperationInformation()
{
    context_.insNumber = currFunc_->ins.size();
    auto &currDebug = currFunc_->ins.back().insDebug;
    currDebug.SetLineNumber(lineStric_);
}

bool Parser::ParseFunctionReturn()
{
    LOG(DEBUG, ASSEMBLER) << "started searching for return function value (line " << lineStric_
                          << "): " << context_.tokens[context_.number - 1].wholeLine;

    if (!TypeValidName()) {
        if (*context_ == Token::Type::DEL_BRACKET_L) {
            context_.err = GetError("No return type.", Error::ErrorType::ERR_BAD_FUNCTION_RETURN_VALUE);
            return false;
        }
        context_.err = GetError("Not a return type.", Error::ErrorType::ERR_BAD_FUNCTION_RETURN_VALUE);
        return false;
    }

    if (!ParseType(&context_.currFuncReturnType)) {
        return false;
    }

    LOG(DEBUG, ASSEMBLER) << "return type found (line " << lineStric_ << "): " << context_.GiveToken();

    return true;
}

bool Parser::TypeValidName()
{
    if (Type::GetId(context_.GiveToken()) != panda_file::Type::TypeId::REFERENCE &&
        (*context_ != Token::Type::DEL_BRACE_L)) {
        return true;
    }

    return PrefixedValidName(BracketOptions::ALLOW_BRACKETS);
}

bool Parser::ParseFunctionArg()
{
    if (*context_ != Token::Type::ID && *context_ != Token::Type::DEL_BRACE_L) {
        context_.err = GetError("Expected identifier.", Error::ErrorType::ERR_BAD_FUNCTION_PARAMETERS);
        return false;
    }

    if (!TypeValidName()) {
        context_.err = GetError("Expected parameter type.", Error::ErrorType::ERR_BAD_TYPE);
        return false;
    }

    Type type;
    if (!ParseType(&type)) {
        return false;
    }

    if (context_.Mask()) {
        return false;
    }

    if (*context_ != Token::Type::ID) {
        context_.err = GetError("Expected identifier.", Error::ErrorType::ERR_BAD_FUNCTION_PARAMETERS);
        return false;
    }

    if (!ParamValidName()) {
        context_.err = GetError("Incorrect name of parameter.", Error::ErrorType::ERR_BAD_PARAM_NAME);
        return false;
    }

    ++context_;

    Function::Parameter parameter(type, program_.lang);
    metadata_ = parameter.GetOrCreateMetadata();

    if (*context_ == Token::Type::DEL_LT && !ParseMetaDef()) {
        return false;
    }

    currFunc_->params.push_back(std::move(parameter));

    return true;
}

bool Parser::ParseFunctionArgComma(bool &comma)
{
    if (comma && *context_ != Token::Type::DEL_COMMA) {
        context_.err = GetError("Expected comma.", Error::ErrorType::ERR_BAD_NUMBER_OPERANDS);
        return false;
    }

    if (comma) {
        ++context_;
    }

    comma = true;

    return true;
}

bool Parser::ParseFunctionArgs()
{
    LOG(DEBUG, ASSEMBLER) << "started searching for function parameters (line " << lineStric_
                          << "): " << context_.tokens[context_.number - 1].wholeLine;

    bool comma = false;

    while (true) {
        if (context_.Mask()) {
            return false;
        }

        if (context_.id != Token::Type::DEL_COMMA && context_.id != Token::Type::ID &&
            context_.id != Token::Type::DEL_BRACE_L) {
            break;
        }

        if (!ParseFunctionArgComma(comma)) {
            return false;
        }

        if (!ParseFunctionArg()) {
            return false;
        }
    }

    LOG(DEBUG, ASSEMBLER) << "parameters found (line " << lineStric_ << "): ";

    return true;
}

bool Parser::ParseMetaDef()
{
    LOG(DEBUG, ASSEMBLER) << "started searching for meta information (line " << lineStric_
                          << "): " << context_.tokens[context_.number - 1].wholeLine;

    bool flag = false;

    if (context_.Mask()) {
        return false;
    }

    if (*context_ == Token::Type::DEL_LT) {
        flag = true;
        ++context_;
    }

    if (!ParseMetaList(flag)) {
        return false;
    }

    if (!flag && *context_ == Token::Type::DEL_GT) {
        context_.err = GetError("Expected '<'.", Error::ErrorType::ERR_BAD_METADATA_BOUND);
        ++context_;
        return false;
    }

    LOG(DEBUG, ASSEMBLER) << "searching for meta information (line " << lineStric_ << ") is successful";

    if (flag && context_.err.err == Error::ErrorType::ERR_NONE) {
        ++context_;
    }

    return true;
}

void Parser::SetMetadataContextError(const Metadata::Error &err, bool hasValue)
{
    constexpr int64_t NO_VALUE_OFF = -1;
    constexpr int64_t SPECIAL_OFF = -2;
    constexpr int64_t STANDARD_OFF = -3;

    switch (err.GetType()) {
        case Metadata::Error::Type::UNKNOWN_ATTRIBUTE: {
            context_.err = GetError(err.GetMessage(), Error::ErrorType::ERR_BAD_METADATA_UNKNOWN_ATTRIBUTE, 0,
                                    hasValue ? STANDARD_OFF : NO_VALUE_OFF);
            break;
        }
        case Metadata::Error::Type::MISSING_ATTRIBUTE: {
            context_.err = GetError(err.GetMessage(), Error::ErrorType::ERR_BAD_METADATA_MISSING_ATTRIBUTE);
            break;
        }
        case Metadata::Error::Type::MISSING_VALUE: {
            context_.err = GetError(err.GetMessage(), Error::ErrorType::ERR_BAD_METADATA_MISSING_VALUE);
            break;
        }
        case Metadata::Error::Type::UNEXPECTED_ATTRIBUTE: {
            context_.err = GetError(err.GetMessage(), Error::ErrorType::ERR_BAD_METADATA_UNEXPECTED_ATTRIBUTE, 0,
                                    hasValue ? STANDARD_OFF : NO_VALUE_OFF);
            break;
        }
        case Metadata::Error::Type::UNEXPECTED_VALUE: {
            context_.err =
                GetError(err.GetMessage(), Error::ErrorType::ERR_BAD_METADATA_UNEXPECTED_VALUE, 0, SPECIAL_OFF);
            break;
        }
        case Metadata::Error::Type::INVALID_VALUE: {
            context_.err = GetError(err.GetMessage(), Error::ErrorType::ERR_BAD_METADATA_INVALID_VALUE, 0, -1);
            break;
        }
        case Metadata::Error::Type::MULTIPLE_ATTRIBUTE: {
            context_.err = GetError(err.GetMessage(), Error::ErrorType::ERR_BAD_METADATA_MULTIPLE_ATTRIBUTE, 0,
                                    hasValue ? STANDARD_OFF : NO_VALUE_OFF);
            break;
        }
        default: {
            UNREACHABLE();
        }
    }
}

bool Parser::ParseMetaListComma(bool &comma, bool eq)
{
    if (!eq && comma && *context_ != Token::Type::DEL_COMMA) {
        context_.err = GetError("Expected comma.", Error::ErrorType::ERR_BAD_NUMBER_OPERANDS);
        return false;
    }

    if (!eq && comma) {
        ++context_;
    }

    comma = true;

    return true;
}

bool Parser::MeetExpMetaList(bool eq)
{
    if (!eq && *context_ != Token::Type::ID) {
        context_.err = GetError("Expected identifier.", Error::ErrorType::ERR_BAD_DEFINITION_METADATA, +1);
        return false;
    }

    if (eq && *context_ != Token::Type::ID && *context_ != Token::Type::ID_STRING) {
        context_.err =
            GetError("Expected identifier or string literal.", Error::ErrorType::ERR_BAD_DEFINITION_METADATA, +1);
        return false;
    }

    if (!eq && !PrefixedValidName()) {
        context_.err = GetError("Invalid attribute name.", Error::ErrorType::ERR_BAD_NAME_ID);
        return false;
    }

    return true;
}

bool Parser::BuildMetaListAttr(bool &eq, std::string &attributeName, std::string &attributeValue)
{
    if (eq && *context_ == Token::Type::ID_STRING) {
        auto res = ParseStringLiteral();
        if (!res) {
            return false;
        }

        attributeValue = res.value();
    } else if (eq) {
        std::string sign {};
        attributeValue = context_.GiveToken();
        ++context_;

        if (!ParseOperandSignature(&sign)) {
            return false;
        }

        --context_;
        attributeValue += sign;
    } else {
        attributeName = context_.GiveToken();
    }

    ++context_;

    if (context_.Mask()) {
        return false;
    }

    if (*context_ == Token::Type::DEL_EQ) {
        if (eq) {
            context_.err = GetError("'=' was not expected.", Error::ErrorType::ERR_BAD_NOEXP_DELIM);
            return false;
        }

        ++context_;
        eq = true;
    } else {
        std::optional<Metadata::Error> res;
        bool hasValue = eq;
        if (hasValue) {
            res = metadata_->SetAttributeValue(attributeName, attributeValue);
        } else {
            res = metadata_->SetAttribute(attributeName);
        }

        eq = false;

        if (res) {
            auto err = res.value();
            SetMetadataContextError(err, hasValue);
            return false;
        }
    }

    return true;
}

bool Parser::ParseMetaList(bool flag)
{
    if (!flag && !context_.Mask() && *context_ != Token::Type::DEL_GT && *context_ != Token::Type::DEL_BRACE_L) {
        context_.err = GetError("No meta data expected.", Error::ErrorType::ERR_BAD_DEFINITION_METADATA);
        return false;
    }

    bool comma = false;
    bool eq = false;

    std::string attributeName;
    std::string attributeValue;

    while (true) {
        if (context_.Mask()) {
            context_.err = GetError("Expected '>'.", Error::ErrorType::ERR_BAD_METADATA_BOUND, +1);
            return false;
        }

        if (context_.id != Token::Type::DEL_COMMA && context_.id != Token::Type::ID &&
            context_.id != Token::Type::ID_STRING && context_.id != Token::Type::DEL_EQ) {
            break;
        }

        if (!ParseMetaListComma(comma, eq)) {
            return false;
        }

        if (!MeetExpMetaList(eq)) {
            return false;
        }

        if (!BuildMetaListAttr(eq, attributeName, attributeValue)) {
            return false;
        }
    }

    if (flag && *context_ != Token::Type::DEL_GT) {
        context_.err = GetError("Expected '>'.", Error::ErrorType::ERR_BAD_METADATA_BOUND);
        ++context_;

        return false;
    }

    auto res = metadata_->ValidateData();
    if (res) {
        auto err = res.value();
        SetMetadataContextError(err, false);
        return false;
    }

    return true;
}

bool Parser::ParseFunctionInstruction()
{
    if (ParseLabel()) {
        if (ParseOperation()) {
            if (ParseOperands()) {
                return true;
            }
        }
    }

    return context_.Mask();
}

static std::string GetFuncNameWithoutArgs(const std::string &fullName)
{
    auto pos = fullName.find(':');
    return fullName.substr(0, pos);
}

bool Parser::ParseOperandInitobj()
{
    if (context_.err.err != Error::ErrorType::ERR_NONE) {
        return false;
    }

    if (!FunctionValidName()) {
        context_.err = GetError("Invalid name of function.", Error::ErrorType::ERR_BAD_FUNCTION_NAME);
        return false;
    }

    auto cid = std::string(context_.GiveToken().data(), context_.GiveToken().length());

    context_++;
    if (*context_ == Token::Type::DEL_COMMA || *context_ == Token::Type::DEL_COLON || context_.Mask()) {
        if (!context_.Mask()) {
            context_--;
        }
        return ParseOperandCall();
    }
    auto dim = ParseMultiArrayHallmark();
    if (context_.err.err != Error::ErrorType::ERR_NONE) {
        return false;
    }
    if (dim < 2U) {
        context_.err = GetError("Invalid name of function.", Error::ErrorType::ERR_BAD_FUNCTION_NAME);
        return false;
    }

    for (uint8_t i = 0; i < dim; i++) {
        cid += "[]";
    }

    if (context_.GiveToken() == ".ctor") {
        cid += ".ctor";
        currIns_->EmplaceID(cid);
        bool findFuncInTable = false;
        for (const auto &[key, func] : ambiguousFunctionTable_) {
            if (GetFuncNameWithoutArgs(key.first) == cid) {
                findFuncInTable = true;
                break;
            }
        }
        if (!findFuncInTable) {
            AddObjectInTable(false, ambiguousFunctionTable_, cid);
        }
        context_++;
        return true;
    }
    return false;
}

uint8_t Parser::ParseMultiArrayHallmark()
{
    uint8_t counter = 0;
    do {
        if (*context_ == Token::Type::DEL_SQUARE_BRACKET_L) {
            context_++;
            if (*context_ == Token::Type::DEL_SQUARE_BRACKET_R) {
                context_++;
                counter++;
            } else {
                context_.err = GetError("] expected.", Error::ErrorType::ERR_BAD_FUNCTION_NAME);
                return counter;
            }
        } else {
            context_.err = GetError("[ expected.", Error::ErrorType::ERR_BAD_FUNCTION_NAME);
            return counter;
        }
    } while (!context_.Mask() &&
             (*context_ == Token::Type::DEL_SQUARE_BRACKET_L || *context_ == Token::Type::DEL_SQUARE_BRACKET_R));
    return counter;
}

}  // namespace ark::pandasm
