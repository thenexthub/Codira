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

#include "ins_emit.h"
#include "opcode_parsing.h"
#include "operand_types_print.h"
#include "utils/number-utils.h"

namespace panda::pandasm {

bool Parser::ParseRecordFields()
{
    if (!open_ && *context_ == Token::Type::DEL_BRACE_L) {
        curr_record_->body_location.begin = GetCurrentPosition(false);
        open_ = true;

        ++context_;
    }

    curr_record_->body_presence = true;

    if (!open_) {
        context_.err = GetError("Expected keyword.", Error::ErrorType::ERR_BAD_KEYWORD);
        return false;
    }

    if (context_.Mask()) {
        return true;
    }

    if (open_ && *context_ == Token::Type::DEL_BRACE_R) {
        curr_record_->body_location.end = GetCurrentPosition(true);
        ++context_;

        open_ = false;

        return true;
    }

    curr_record_->field_list.emplace_back(program_.lang);
    curr_fld_ = &(curr_record_->field_list[curr_record_->field_list.size() - 1]);
    curr_fld_->line_of_def = line_stric_;
    context_.ins_number = curr_record_->field_list.size();

    LOG(DEBUG, ASSEMBLER) << "parse line " << line_stric_ << " as field (.field name)";
    if (!ParseRecordField()) {
        if (context_.err.err != Error::ErrorType::ERR_NONE) {
            return false;
        }

        if (open_ && *context_ == Token::Type::DEL_BRACE_R) {
            curr_record_->body_location.end = GetCurrentPosition(true);
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
        std::string field_name = std::string(context_.GiveToken().data(), context_.GiveToken().length());

        auto match_names = [&field_name](const pandasm::Field &f) { return field_name == f.name; };
        const auto iter = std::find_if(curr_record_->field_list.begin(), curr_record_->field_list.end(), match_names);
        if (iter != curr_record_->field_list.end()) {
            if (iter->is_defined) {
                context_.err =
                    GetError("Repeating field names in the same record.", Error::ErrorType::ERR_REPEATING_FIELD_NAME);

                return false;
            }

            curr_record_->field_list.erase(iter);
        }

        curr_fld_ = &(curr_record_->field_list[curr_record_->field_list.size() - 1]);

        curr_fld_->name = field_name;

        ++context_;

        return true;
    }

    context_.err = GetError("Invalid name of field.", Error::ErrorType::ERR_BAD_OPERATION_NAME);

    return false;
}

bool Parser::ParseType(Type *type)
{
    ASSERT(TypeValidName());

    std::string component_name(context_.GiveToken());
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

    *type = Type(component_name, rank);

    if (type->IsArray()) {
        program_.array_types.insert(*type);
    }

    return true;
}

bool Parser::ParseFieldType()
{
    LOG(DEBUG, ASSEMBLER) << "started searching field type value (line " << line_stric_
                          << "): " << context_.tokens[context_.number - 1].whole_line;

    if (!TypeValidName()) {
        context_.err = GetError("Not a correct type.", Error::ErrorType::ERR_BAD_FIELD_VALUE_TYPE);
        return false;
    }

    if (!ParseType(&curr_fld_->type)) {
        return false;
    }

    curr_fld_->metadata->SetFieldType(curr_fld_->type);

    LOG(DEBUG, ASSEMBLER) << "field type found (line " << line_stric_ << "): " << context_.GiveToken();

    return true;
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
        curr_record_->body_location.end = GetCurrentPosition(true);
        ++context_;
        open_ = false;
        return true;
    }

    metadata_ = curr_fld_->metadata.get();
    ParseMetaDef();

    return context_.Mask();
}

bool Parser::IsConstArray()
{
    if ((curr_array_->literals_.size() >= INTRO_CONST_ARRAY_LITERALS_NUMBER) &&
        (curr_array_->literals_[0].tag_ == panda_file::LiteralTag::TAGVALUE)) {
        return true;
    }
    return false;
}

bool Parser::ArrayElementsValidNumber()
{
    if (!IsConstArray()) {
        return true;
    }

    ASSERT(curr_array_->literals_.size() > 1);

    auto init_size = std::get<uint32_t>(curr_array_->literals_[1].value_);
    if (init_size < 1) {
        return false;
    }
    if (curr_array_->literals_.size() != init_size + INTRO_CONST_ARRAY_LITERALS_NUMBER) {
        return false;
    }

    return true;
}

void Parser::ParseAsArray(const std::vector<Token> &tokens)
{
    LOG(DEBUG, ASSEMBLER) << "started parsing of array (line " << line_stric_ << "): " << tokens[0].whole_line;
    func_def_ = false;
    record_def_ = false;
    array_def_ = true;

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

            LOG(DEBUG, ASSEMBLER) << "array body is open, line " << line_stric_ << ": " << tokens[0].whole_line;

            open_ = true;
        }

        uint32_t iter_number = 1;
        if (IsConstArray()) {
            iter_number = std::get<uint32_t>(curr_array_->literals_[1].value_);
        }

        if (iter_number < 1) {
            context_.err =
                GetError("Сonstant array must contain at least one element.", Error::ErrorType::ERR_BAD_ARRAY_SIZE);
            return;
        }

        for (uint32_t i = 0; i < iter_number; i++) {
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
            LOG(DEBUG, ASSEMBLER) << "array body is closed, line " << line_stric_ << ": " << tokens[0].whole_line;

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

    curr_array_->literals_.push_back(panda::pandasm::LiteralArray::Literal());
    curr_array_elem_ = &(curr_array_->literals_[curr_array_->literals_.size() - 1]);

    LOG(DEBUG, ASSEMBLER) << "parse line " << line_stric_ << " as array elem (.array_elem value)";
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
        curr_array_elem_->tag_ =
            static_cast<panda_file::LiteralTag>(std::get<uint8_t>(curr_array_->literals_[0].value_));
    } else {
        if (!ParseArrayElementType()) {
            return false;
        }
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
    LOG(DEBUG, ASSEMBLER) << "started searching array element type value (line " << line_stric_
                          << "): " << context_.tokens[context_.number - 1].whole_line;

    if (!TypeValidName()) {
        context_.err = GetError("Not a correct type.", Error::ErrorType::ERR_BAD_ARRAY_ELEMENT_VALUE_TYPE);
        return false;
    }

    Type type;
    if (!ParseType(&type)) {
        return false;
    }

    // workaround until #5776 is done
    auto type_name = type.GetName();
    std::replace(type_name.begin(), type_name.end(), '.', '/');
    auto type_with_slash = Type(type_name, 0);

    if (panda::pandasm::Type::IsPandaPrimitiveType(type.GetName())) {
        curr_array_elem_->tag_ = panda::pandasm::LiteralArray::GetArrayTagFromComponentType(type.GetId());
        if (program_.array_types.find(type) == program_.array_types.end()) {
            program_.array_types.emplace(type, 1);
        }
    } else if (panda::pandasm::Type::IsStringType(type_with_slash.GetName(), program_.lang)) {
        curr_array_elem_->tag_ = panda_file::LiteralTag::ARRAY_STRING;
        if (program_.array_types.find(type_with_slash) == program_.array_types.end()) {
            program_.array_types.emplace(type_with_slash, 1);
        }
    } else {
        return false;
    }

    LOG(DEBUG, ASSEMBLER) << "array element type found (line " << line_stric_ << "): " << context_.GiveToken();

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
    if (curr_array_elem_->IsBoolValue()) {
        curr_array_elem_->value_ = static_cast<bool>(n);
    }
    if (curr_array_elem_->IsByteValue()) {
        curr_array_elem_->value_ = static_cast<uint8_t>(n);
    }
    if (curr_array_elem_->IsShortValue()) {
        curr_array_elem_->value_ = static_cast<uint16_t>(n);
    }
    if (curr_array_elem_->IsIntegerValue()) {
        curr_array_elem_->value_ = static_cast<uint32_t>(n);
    }
    if (curr_array_elem_->IsLongValue()) {
        curr_array_elem_->value_ = static_cast<uint64_t>(n);
    }
    return true;
}

bool Parser::ParseArrayElementValueFloat()
{
    double n;
    if (!ParseFloat(&n, !curr_array_elem_->IsFloatValue())) {
        context_.err =
            GetError("Invalid value of array float element.", Error::ErrorType::ERR_BAD_ARRAY_ELEMENT_VALUE_FLOAT);
        return false;
    }
    if (curr_array_elem_->IsFloatValue()) {
        curr_array_elem_->value_ = static_cast<float>(n);
    } else {
        curr_array_elem_->value_ = static_cast<double>(n);
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
    curr_array_elem_->value_ = res.value();
    return true;
}

bool Parser::ParseArrayElementValue()
{
    if (curr_array_elem_->IsBoolValue() || curr_array_elem_->IsByteValue() || curr_array_elem_->IsShortValue() ||
        curr_array_elem_->IsIntegerValue() || curr_array_elem_->IsLongValue()) {
        if (!ParseArrayElementValueInteger()) {
            return false;
        }
    }
    if (curr_array_elem_->IsFloatValue() || curr_array_elem_->IsDoubleValue()) {
        if (!ParseArrayElementValueFloat()) {
            return false;
        }
    }
    if (curr_array_elem_->IsStringValue()) {
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
        curr_func_->body_location.begin = GetCurrentPosition(false);
        ++context_;
    }

    curr_func_->body_presence = true;

    if (!open_) {
        context_.err = GetError("Expected keyword.", Error::ErrorType::ERR_BAD_KEYWORD);
        return false;
    }

    if (context_.Mask()) {
        return true;
    }

    if (open_ && *context_ == Token::Type::DEL_BRACE_R) {
        curr_func_->body_location.end = GetCurrentPosition(true);
        ++context_;
        open_ = false;
        return true;
    }

    opcode_ = panda::pandasm::Opcode::INVALID;
    ids_.clear();
    imms_.clear();
    regs_.clear();
    set_label_ = false;
    label_.clear();

    LOG(DEBUG, ASSEMBLER) << "parse line " << line_stric_
                          << " as instruction ([label:] operation [operand,] [# comment])";

    ParseFunctionInstruction();

    SetError();
    if (err_.err == Error::ErrorType::ERR_NONE) {
        if (set_label_) {
            curr_func_->ins.emplace_back(new panda::pandasm::LabelIns(label_));
        } else {
            curr_func_->ins.emplace_back(panda::pandasm::Ins::CreateIns(opcode_, regs_, imms_, ids_));
        }
        curr_func_->ins.back()->ins_debug = curr_debug_;
    }

    if (open_ && *context_ == Token::Type::DEL_BRACE_R) {
        curr_func_->body_location.end = GetCurrentPosition(true);
        ++context_;
        open_ = false;
    }

    return true;
}

void Parser::ParseAsRecord(const std::vector<Token> &tokens)
{
    LOG(DEBUG, ASSEMBLER) << "started parsing of record (line " << line_stric_ << "): " << tokens[0].whole_line;
    func_def_ = false;
    record_def_ = true;
    array_def_ = false;

    if (!open_) {
        ++context_;
    } else {
        context_.err =
            GetError("No one record can be defined inside another record.", Error::ErrorType::ERR_BAD_DEFINITION);
        return;
    }

    if (ParseRecordFullSign()) {
        metadata_ = curr_record_->metadata.get();
        if (ParseMetaDef()) {
            if (!open_ && *context_ == Token::Type::DEL_BRACE_L) {
                curr_record_->body_location.begin = GetCurrentPosition(false);
                ++context_;

                LOG(DEBUG, ASSEMBLER) << "record body is open, line " << line_stric_ << ": " << tokens[0].whole_line;

                open_ = true;
            }

            if (open_ && !context_.Mask() && *context_ != Token::Type::DEL_BRACE_R) {
                ParseRecordFields();
            } else if (open_) {
                curr_record_->body_presence = true;
            }

            if (open_ && *context_ == Token::Type::DEL_BRACE_R) {
                LOG(DEBUG, ASSEMBLER) << "record body is closed, line " << line_stric_ << ": " << tokens[0].whole_line;

                curr_record_->body_location.end = GetCurrentPosition(true);
                ++context_;

                open_ = false;
            }
        }
    }
}

void Parser::ParseAsFunction(const std::vector<Token> &tokens)
{
    LOG(DEBUG, ASSEMBLER) << "started parsing of function (line " << line_stric_ << "): " << tokens[0].whole_line;
    record_def_ = false;
    func_def_ = true;
    array_def_ = false;

    if (!open_) {
        ++context_;
    } else {
        context_.err =
            GetError("No one function can be defined inside another function.", Error::ErrorType::ERR_BAD_DEFINITION);
        return;
    }

    if (ParseFunctionFullSign()) {
        metadata_ = curr_func_->metadata.get();
        if (ParseMetaDef()) {
            if (!open_ && *context_ == Token::Type::DEL_BRACE_L) {
                curr_func_->body_location.begin = GetCurrentPosition(false);
                ++context_;

                LOG(DEBUG, ASSEMBLER) << "function body is open, line " << line_stric_ << ": " << tokens[0].whole_line;

                open_ = true;
            }

            if (open_ && !context_.Mask() && *context_ != Token::Type::DEL_BRACE_R) {
                ParseFunctionCode();
            } else if (open_) {
                curr_func_->body_presence = true;
            }

            if (open_ && *context_ == Token::Type::DEL_BRACE_R) {
                LOG(DEBUG, ASSEMBLER) << "function body is closed, line " << line_stric_ << ": "
                                      << tokens[0].whole_line;

                curr_func_->body_location.end = GetCurrentPosition(true);
                ++context_;
                open_ = false;
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

    LOG(DEBUG, ASSEMBLER) << "body is closed (line " << line_stric_ << "): " << tokens[0].whole_line;

    open_ = false;
    if (func_def_) {
        curr_func_->body_location.end = GetCurrentPosition(true);
    } else if (record_def_) {
        curr_record_->body_location.end = GetCurrentPosition(true);
    } else if (array_def_) {
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

void Parser::ParseResetFunctionLabelsAndParams()
{
    if (open_ || err_.err != Error::ErrorType::ERR_NONE) {
        return;
    }

    for (const auto &f : program_.function_table) {
        for (const auto &k : f.second.label_table) {
            if (!k.second.file_location->is_defined) {
                context_.err = Error("This label does not exist.", line_stric_, Error::ErrorType::ERR_BAD_LABEL_EXT, "",
                                     k.second.file_location->bound_left, k.second.file_location->bound_right,
                                     k.second.file_location->whole_line);
                SetError();
            }
        }
    }

    for (const auto &t : context_.function_arguments_lists) {
        curr_func_ = &(program_.function_table.at(t.first));
        curr_func_->regs_num = static_cast<size_t>(curr_func_->value_of_first_param + 1);

        for (const auto &v : t.second) {
            if (!curr_func_->ins.empty() && curr_func_->ins.size() >= v.first &&
                !curr_func_->ins[v.first - 1]->Regs().empty()) {
                auto &ins =  curr_func_->ins[v.first - 1];
                auto reg = ins->Regs()[v.second];
                reg += static_cast<uint16_t>(curr_func_->value_of_first_param + 1);
                ins->SetReg(v.second, reg);
                size_t max_reg_number = (1 << ins->MaxRegEncodingWidth());
                if (reg >= max_reg_number) {
                    const auto &debug = ins->ins_debug;
                    context_.err =
                        Error("Register width mismatch.", debug.line_number, Error::ErrorType::ERR_BAD_NAME_REG, "",
                              0, 0, "");
                    SetError();
                    break;
                }
            }
        }
    }
}

void Parser::ParseResetFunctionTable()
{
    for (auto &k : program_.function_table) {
        if (!k.second.file_location->is_defined) {
            context_.err = Error("This function does not exist.", k.second.file_location->line_number,
                                 Error::ErrorType::ERR_BAD_ID_FUNCTION, "", k.second.file_location->bound_left,
                                 k.second.file_location->bound_right, k.second.file_location->whole_line);
            SetError();
        } else {
            for (auto insn_it = k.second.ins.begin(); insn_it != k.second.ins.end(); ++insn_it) {
                if (!((*insn_it)->IsCall() || (*insn_it)->IsCallRange())) {
                    continue;
                }

                size_t diff = 1;
                auto func_name = (*insn_it)->GetId(0);
                if (!IsSignatureOrMangled(func_name)) {
                    const auto it_synonym = program_.function_synonyms.find(func_name);
                    if (it_synonym == program_.function_synonyms.end()) {
                        continue;
                    } else if (it_synonym->second.size() > 1) {
                        const auto &debug = (*insn_it)->ins_debug;
                        context_.err = Error("Unable to resolve ambiguous function call", debug.line_number,
                                             Error::ErrorType::ERR_FUNCTION_MULTIPLE_ALTERNATIVES, "", 0, 0, "");
                        SetError();
                        break;
                    } else {
                        (*insn_it)->GetId(0) = program_.function_synonyms.at(func_name)[0];
                    }
                }

                if ((*insn_it)->OperandListLength() - diff >=
                    program_.function_table.at((*insn_it)->GetId(0)).GetParamsNum()) {
                    continue;
                }
                if ((*insn_it)->IsCallRange() &&
                    (static_cast<int>((*insn_it)->Regs().size()) - static_cast<int>(diff) >= 0)) {
                    continue;
                }

                const auto &debug = (*insn_it)->ins_debug;
                context_.err = Error("Function argument mismatch.", debug.line_number,
                                     Error::ErrorType::ERR_FUNCTION_ARGUMENT_MISMATCH, "", 0, 0, "");
                SetError();
            }
        }
    }
}

void Parser::ParseResetRecordTable()
{
    for (const auto &k : program_.record_table) {
        if (!k.second.file_location->is_defined) {
            context_.err = Error("This record does not exist.", k.second.file_location->line_number,
                                 Error::ErrorType::ERR_BAD_ID_RECORD, "", k.second.file_location->bound_left,
                                 k.second.file_location->bound_right, k.second.file_location->whole_line);
            SetError();
        } else if (k.second.HasImplementation() != k.second.body_presence) {
            context_.err = Error("Inconsistency of the definition of the record and its metadata.",
                                 k.second.file_location->line_number, Error::ErrorType::ERR_BAD_DEFINITION_RECORD, "",
                                 k.second.file_location->bound_left, k.second.file_location->bound_right,
                                 k.second.file_location->whole_line);
            SetError();
        } else {
            for (const auto &fld : k.second.field_list) {
                if (!fld.is_defined) {
                    context_.err =
                        Error("This field does not exist.", fld.line_of_def, Error::ErrorType::ERR_BAD_ID_FIELD, "",
                              fld.bound_left, fld.bound_right, fld.whole_line);
                    SetError();
                }
            }
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
    auto res = panda::panda_file::LanguageFromString(lang);
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

Function::CatchBlock Parser::PrepareCatchBlock(bool is_catchall, size_t size, size_t catchall_tokens_num,
                                               size_t catch_tokens_num)
{
    constexpr size_t TRY_BEGIN = 0;
    constexpr size_t TRY_END = 1;
    constexpr size_t CATCH_BEGIN = 2;
    constexpr size_t CATCH_END = 3;

    Function::CatchBlock catch_block;
    catch_block.whole_line = context_.tokens[0].whole_line;
    std::vector<std::string> label_names {"try block begin", "try block end", "catch block begin"};
    std::vector<std::string> labels;
    bool full_catch_block = (is_catchall && size == catchall_tokens_num) || (!is_catchall && size == catch_tokens_num);
    if (full_catch_block) {
        label_names.emplace_back("catch block end");
    }
    if (!is_catchall) {
        catch_block.exception_record = context_.GiveToken();
        ++context_;
    }

    bool skip_comma = is_catchall;
    for (auto label_name : label_names) {
        if (!skip_comma) {
            if (*context_ != Token::Type::DEL_COMMA) {
                context_.err = GetError("Expected comma.", Error::ErrorType::ERR_BAD_DIRECTIVE_DECLARATION);
                return catch_block;
            }
            ++context_;
        }
        skip_comma = false;
        if (!LabelValidName()) {
            context_.err =
                GetError(std::string("Invalid name of the ") + label_name + " label.", Error::ErrorType::ERR_BAD_LABEL);
            return catch_block;
        }
        labels.emplace_back(context_.GiveToken());
        AddObjectInTable(false, *label_table_);
        ++context_;
    }

    ASSERT(context_.Mask());
    catch_block.try_begin_label = labels[TRY_BEGIN];
    catch_block.try_end_label = labels[TRY_END];
    catch_block.catch_begin_label = labels[CATCH_BEGIN];
    if (full_catch_block) {
        catch_block.catch_end_label = labels[CATCH_END];
    } else {
        catch_block.catch_end_label = labels[CATCH_BEGIN];
    }
    return catch_block;
}

void Parser::ParseAsCatchDirective()
{
    ASSERT(*context_ == Token::Type::ID_CATCH || *context_ == Token::Type::ID_CATCHALL);

    constexpr size_t CATCH_DIRECTIVE_TOKENS_NUM = 8;
    constexpr size_t CATCHALL_DIRECTIVE_TOKENS_NUM = 6;
    constexpr size_t CATCH_FULL_DIRECTIVE_TOKENS_NUM = 10;
    constexpr size_t CATCHALL_FULL_DIRECTIVE_TOKENS_NUM = 8;

    bool is_catchall = *context_ == Token::Type::ID_CATCHALL;
    size_t size = context_.tokens.size();
    if (is_catchall && size != CATCHALL_DIRECTIVE_TOKENS_NUM && size != CATCHALL_FULL_DIRECTIVE_TOKENS_NUM) {
        context_.err = GetError(
            "Incorrect catch block declaration. Must be in the format: .catchall <try_begin_label>, <try_end_label>, "
            "<catch_begin_label>[, <catch_end_label>]",
            Error::ErrorType::ERR_BAD_DIRECTIVE_DECLARATION);
        return;
    }

    if (!is_catchall && size != CATCH_DIRECTIVE_TOKENS_NUM && size != CATCH_FULL_DIRECTIVE_TOKENS_NUM) {
        context_.err = GetError(
            "Incorrect catch block declaration. Must be in the format: .catch <exception_record>, <try_begin_label>, "
            "<try_end_label>, <catch_begin_label>[, <catch_end_label>]",
            Error::ErrorType::ERR_BAD_DIRECTIVE_DECLARATION);
        return;
    }

    ++context_;

    if (!is_catchall && !RecordValidName()) {
        context_.err = GetError("Invalid name of the exception record.", Error::ErrorType::ERR_BAD_RECORD_NAME);
        return;
    }

    Function::CatchBlock catch_block =
        PrepareCatchBlock(is_catchall, size, CATCHALL_FULL_DIRECTIVE_TOKENS_NUM, CATCH_FULL_DIRECTIVE_TOKENS_NUM);

    curr_func_->catch_blocks.push_back(catch_block);
}

void Parser::ParseAsCatchall(const std::vector<Token> &tokens)
{
    std::string directive_name = *context_ == Token::Type::ID_CATCH ? ".catch" : ".catchall";
    if (!func_def_) {
        context_.err = GetError(directive_name + " directive is located outside of a function body.",
                                Error::ErrorType::ERR_INCORRECT_DIRECTIVE_LOCATION);
        return;
    }

    LOG(DEBUG, ASSEMBLER) << "started parsing of " << directive_name << " directive (line " << line_stric_
                          << "): " << tokens[0].whole_line;

    ParseAsCatchDirective();
}

void Parser::ParseAsLanguage(const std::vector<Token> &tokens, bool &is_lang_parsed, bool &is_first_statement)
{
    if (is_lang_parsed) {
        context_.err = GetError("Multiple .language directives", Error::ErrorType::ERR_MULTIPLE_DIRECTIVES);
        return;
    }

    if (!is_first_statement) {
        context_.err = GetError(".language directive must be specified before any other declarations",
                                Error::ErrorType::ERR_INCORRECT_DIRECTIVE_LOCATION);
        return;
    }

    LOG(DEBUG, ASSEMBLER) << "started parsing of .language directive (line " << line_stric_
                          << "): " << tokens[0].whole_line;

    ParseAsLanguageDirective();

    is_lang_parsed = true;
}

bool Parser::ParseAfterLine(bool &is_first_statement)
{
    SetError();

    if (!context_.Mask() && err_.err == Error::ErrorType::ERR_NONE) {
        context_.err = GetError("There can be nothing after.", Error::ErrorType::ERR_BAD_END);
    }

    if (err_.err != Error::ErrorType::ERR_NONE) {
        LOG(DEBUG, ASSEMBLER) << "processing aborted (error detected)";
        return false;
    }

    LOG(DEBUG, ASSEMBLER) << "parsing of line " << line_stric_ << " is successful";

    SetError();

    is_first_statement = false;

    return true;
}

Expected<Program, Error> Parser::ParseAfterMainLoop(const std::string &file_name)
{
    ParseResetFunctionLabelsAndParams();

    if (open_ && err_.err == Error::ErrorType::ERR_NONE) {
        context_.err = Error("Code area is not closed.", curr_func_->file_location->line_number,
                             Error::ErrorType::ERR_BAD_CLOSE, "", 0, curr_func_->name.size(), curr_func_->name);
        SetError();
    }

    ParseResetTables();

    if (err_.err != Error::ErrorType::ERR_NONE) {
        return Unexpected(err_);
    }

    for (auto &func : program_.function_table) {
        if (func.second.HasImplementation()) {
            func.second.source_file = file_name;
        }
    }

    for (auto &rec : program_.record_table) {
        if (rec.second.HasImplementation()) {
            rec.second.source_file = file_name;
        }
    }

    return std::move(program_);
}

Expected<Program, Error> Parser::Parse(TokenSet &vectors_tokens, const std::string &file_name)
{
    bool is_lang_parsed = false;
    bool is_first_statement = true;

    for (const auto &tokens : vectors_tokens) {
        ++line_stric_;

        if (tokens.empty()) {
            continue;
        }

        LOG(DEBUG, ASSEMBLER) << "started parsing of line " << line_stric_ << ": " << tokens[0].whole_line;

        context_.Make(tokens);

        switch (*context_) {
            case Token::Type::ID_CATCH:
            case Token::Type::ID_CATCHALL: {
                ParseAsCatchall(tokens);
                break;
            }
            case Token::Type::ID_LANG: {
                ParseAsLanguage(tokens, is_lang_parsed, is_first_statement);
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
                if (func_def_) {
                    ParseFunctionCode();
                } else if (record_def_) {
                    ParseRecordFields();
                } else if (array_def_) {
                    ParseArrayElements();
                }
            }
        }
        if (!ParseAfterLine(is_first_statement)) {
            break;
        }
    }

    return ParseAfterMainLoop(file_name);
}

Expected<Program, Error> Parser::Parse(const std::string &source, const std::string &file_name)
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

    return Parse(v, file_name);
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

    if (curr_func_->GetParamsNum() > 0) {
        return context_.ValidateRegisterName('v') || context_.ValidateRegisterName('a', curr_func_->GetParamsNum() - 1);
    }

    return context_.ValidateRegisterName('v');
}

bool Parser::ParamValidName()
{
    return context_.ValidateParameterName(curr_func_->GetParamsNum());
}

bool IsAlphaNumeric(char c)
{
    return std::isalnum(c) != 0 || c == '_';
}

bool IsNonDigit(char c)
{
    return std::isalpha(c) != 0 || c == '_';
}

bool Parser::PrefixedValidName()
{
    auto s = context_.GiveToken();
    if (!IsNonDigit(s[0])) {
        return false;
    }

    size_t i = 1;

    while (i < s.size()) {
        if (s[i] == '.') {
            ++i;
            if (i >= s.size() || !IsNonDigit(s[i])) {
                return false;
            }
        } else if (!IsAlphaNumeric(s[i]) && s[i] != '$') {
            return false;
        }

        ++i;
    }

    return true;
}

bool Parser::RecordValidName()
{
    return PrefixedValidName();
}

bool Parser::FunctionValidName()
{
    return PrefixedValidName();
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
        if (!IsAlphaNumeric(i)) {
            return false;
        }
    }

    return true;
}

bool Parser::ParseLabel()
{
    LOG(DEBUG, ASSEMBLER) << "label search started (line " << line_stric_ << "): " << context_.tokens[0].whole_line;

    context_++;

    if (*context_ == Token::Type::DEL_COLON) {
        context_--;
        if (LabelValidName()) {
            if (AddObjectInTable(true, *label_table_)) {
                set_label_ = true;
                label_ = context_.GiveToken();

                LOG(DEBUG, ASSEMBLER) << "label detected (line " << line_stric_ << "): " << context_.GiveToken();

                context_++;
                context_++;
                return !context_.Mask();
            }

            LOG(DEBUG, ASSEMBLER) << "label is detected (line " << line_stric_ << "): " << context_.GiveToken()
                                  << ", but this label already exists";

            context_.err = GetError("This label already exists.", Error::ErrorType::ERR_BAD_LABEL_EXT);

        } else {
            LOG(DEBUG, ASSEMBLER) << "label with non-standard character is detected, attempt to create a label is "
                                     "supposed, but this cannot be any label name (line "
                                  << line_stric_ << "): " << context_.GiveToken();

            context_.err = GetError(
                "Invalid name of label. Label contains only characters: '_', '0' - '9', 'a' - 'z', 'A' - 'Z'; and "
                "starts with any letter or with '_'.",
                Error::ErrorType::ERR_BAD_LABEL);
        }

        return false;
    }

    context_--;

    LOG(DEBUG, ASSEMBLER) << "label is not detected (line " << line_stric_ << ")";

    return true;
}

static Opcode TokenToOpcode(Token::Type id)
{
    ASSERT(id > Token::Type::OPERATION && id < Token::Type::KEYWORD);
    using utype = std::underlying_type_t<Token::Type>;
    return static_cast<Opcode>(static_cast<utype>(id) - static_cast<utype>(Token::Type::OPERATION) - 1);
}

bool Parser::ParseOperation()
{
    if (context_.Mask()) {
        LOG(DEBUG, ASSEMBLER) << "no more tokens (line " << line_stric_ << "): " << context_.tokens[0].whole_line;

        return false;
    }

    if (open_ && *context_ == Token::Type::DEL_BRACE_R) {
        return false;
    }

    LOG(DEBUG, ASSEMBLER) << "operaion search started (line " << line_stric_ << "): " << context_.tokens[0].whole_line;

    if (*context_ > Token::Type::OPERATION && *context_ < Token::Type::KEYWORD) {
        SetOperationInformation();

        context_.UpSignOperation();
        opcode_ = TokenToOpcode(context_.id);

        LOG(DEBUG, ASSEMBLER) << "operatiuon is detected (line " << line_stric_ << "): " << context_.GiveToken()
                              << " (operand type: " << OperandTypePrint(opcode_) << ")";

        context_++;
        return true;
    }

    LOG(DEBUG, ASSEMBLER) << "founded " << context_.GiveToken() << ", it is not an operation (line " << line_stric_
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
        int64_t number = static_cast<int64_t>(ToNumber(p));
        if (number > *(context_.max_value_of_reg)) {
            *(context_.max_value_of_reg) = number;
        }

        regs_.push_back(static_cast<uint16_t>(number));
    } else if (p[0] == 'a') {
        p.remove_prefix(1);
        regs_.push_back(static_cast<uint16_t>(ToNumber(p)));
        context_.function_arguments_list->emplace_back(context_.ins_number, regs_.size() - 1);
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

    const auto p = std::string(context_.GiveToken().data(), context_.GiveToken().length());
    ids_.emplace_back(p);

    AddObjectInTable(false, program_.function_table);

    ++context_;

    std::string func_signature {};

    if (!ParseOperandSignature(&func_signature)) {
        return false;
    }

    if (func_signature.empty()) {
        const auto it_synonym = program_.function_synonyms.find(ids_.back());
        if (it_synonym == program_.function_synonyms.end()) {
            return true;
        }

        if (it_synonym->second.size() > 1) {
            context_.err = GetError("Unable to resolve ambiguous function call",
                                    Error::ErrorType::ERR_FUNCTION_MULTIPLE_ALTERNATIVES);
            return false;
        }

        program_.function_table.erase(p);
    } else {
        ids_.back() += func_signature;

        if (program_.function_table.find(ids_.back()) == program_.function_table.end()) {
            auto node_handle = program_.function_table.extract(p);
            node_handle.key() = ids_.back();
            program_.function_table.insert(std::move(node_handle));
        } else {
            program_.function_table.erase(p);
        }
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

        if (*context_ != Token::Type::DEL_COMMA && *context_ != Token::Type::ID) {
            break;
        }

        if (comma) {
            *sign += ",";
        }

        if (!ParseFunctionArgComma(comma)) {
            return false;
        }

        if (*context_ != Token::Type::ID) {
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

    ids_.push_back(res.value());

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

bool Parser::ParseFloat(double *value, bool is_64bit)
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

    *value = FloatNumber(p, is_64bit);
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

    imms_.push_back(n);
    ++context_;
    return true;
}

bool Parser::ParseOperandFloat(bool is_64bit)
{
    double n;
    if (!ParseFloat(&n, is_64bit)) {
        return false;
    }

    imms_.push_back(n);
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
    ids_.emplace_back(p.data(), p.length());
    AddObjectInTable(false, *label_table_);

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
    ids_.emplace_back(p.data(), p.length());
    AddObjectInTable(false, *label_table_);

    ++context_;

    return true;
}

bool Parser::ParseOperandType(Type::VerificationType ver_type)
{
    if (context_.err.err != Error::ErrorType::ERR_NONE) {
        return false;
    }

    if (*context_ != Token::Type::ID) {
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

    bool is_object = (context_.GiveToken() == "]") ? (false) : (true);

    if (is_object) {
        AddObjectInTable(false, program_.record_table);

        if (ver_type == Type::VerificationType::TYPE_ID_ARRAY) {
            GetWarning("Unexpected type_id recieved! Expected array, but object given",
                       Error::ErrorType::WAR_UNEXPECTED_TYPE_ID);
        }
    } else {
        if (!type.IsArrayContainsPrimTypes() &&
            program_.record_table.find(type.GetComponentName()) == program_.record_table.end()) {
            std::string ComponentName = type.GetComponentName();
            context_.token = ComponentName;
            AddObjectInTable(false, program_.record_table);
        }

        if (ver_type == Type::VerificationType::TYPE_ID_OBJECT) {
            GetWarning("Unexpected type_id recieved! Expected object, but array given",
                       Error::ErrorType::WAR_UNEXPECTED_TYPE_ID);
        }
    }

    ids_.push_back(type.GetName());

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
    auto array_id = std::string(p.data(), p.length());
    if (program_.literalarray_table.find(array_id) == program_.literalarray_table.end()) {
        context_.err = GetError("No array was found for this array id", Error::ErrorType::ERR_BAD_ID_ARRAY);
        return false;
    }

    ids_.emplace_back(p.data(), p.length());

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
    std::string record_full_name = std::string(p);
    // Some names of records in pandastdlib starts with 'panda.', therefore,
    // the record name is before the second dot, and the field name is after the second dot
    auto pos_point = record_full_name.find_last_of('.');
    std::string record_name = record_full_name.substr(0, pos_point);
    std::string field_name = record_full_name.substr(pos_point + 1);

    auto it_record = program_.record_table.find(record_name);
    if (it_record == program_.record_table.end()) {
        context_.token = record_name;
        AddObjectInTable(false, program_.record_table);
        it_record = program_.record_table.find(record_name);
    }

    auto it_field = std::find_if(it_record->second.field_list.begin(), it_record->second.field_list.end(),
                                 [&field_name](pandasm::Field &field) { return field_name == field.name; });

    if (!field_name.empty() && it_field == it_record->second.field_list.end()) {
        it_record->second.field_list.emplace_back(program_.lang);
        auto &field = it_record->second.field_list.back();
        field.name = field_name;
        field.line_of_def = line_stric_;
        field.whole_line = context_.tokens[context_.number - 1].whole_line;
        field.bound_left = context_.tokens[context_.number - 1].bound_left + record_name.length() + 1;
        field.bound_right = context_.tokens[context_.number - 1].bound_right;
        field.is_defined = false;
    }

    ids_.emplace_back(p.data(), p.length());

    ++context_;

    return true;
}

bool Parser::ParseOperandNone()
{
    if (context_.err.err != Error::ErrorType::ERR_NONE) {
        return false;
    }

    if (open_ && *context_ == Token::Type::DEL_BRACE_R) {
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
    auto signature = GetFunctionSignatureFromName(curr_func_->name, curr_func_->params);
    auto iter = program_.function_table.find(signature);
    if (iter == program_.function_table.end() || !iter->second.file_location->is_defined) {
        program_.function_synonyms[curr_func_->name].push_back(signature);
        program_.function_table.erase(signature);
        auto node_handle = program_.function_table.extract(curr_func_->name);
        node_handle.key() = signature;
        program_.function_table.insert(std::move(node_handle));
        curr_func_->name = signature;
        context_.max_value_of_reg = &(curr_func_->value_of_first_param);
        context_.function_arguments_list = &(context_.function_arguments_lists[curr_func_->name]);

        return true;
    }

    context_.err = GetError("This function already exists.", Error::ErrorType::ERR_BAD_ID_FUNCTION);

    return false;
}

bool Parser::ParseArrayFullSign()
{
    if (!ParseArrayName()) {
        return false;
    }

    if (*context_ == Token::Type::DEL_BRACE_L) {
        return true;
    }

    curr_array_->literals_.push_back(panda::pandasm::LiteralArray::Literal());
    curr_array_elem_ = &(curr_array_->literals_[curr_array_->literals_.size() - 1]);

    if (!ParseArrayElementType()) {
        context_.err = GetError("Invalid array type for static array.", Error::ErrorType::ERR_BAD_ARRAY_TYPE);
        return false;
    }

    curr_array_elem_->value_ = static_cast<uint8_t>(curr_array_elem_->tag_);
    curr_array_elem_->tag_ = panda_file::LiteralTag::TAGVALUE;

    if (*context_ == Token::Type::DEL_BRACE_L) {
        context_.err = GetError("No array size for static array.", Error::ErrorType::ERR_BAD_ARRAY_SIZE);
        return false;
    }

    curr_array_->literals_.push_back(panda::pandasm::LiteralArray::Literal());
    curr_array_elem_ = &(curr_array_->literals_[curr_array_->literals_.size() - 1]);
    curr_array_elem_->tag_ = panda_file::LiteralTag::INTEGER;

    if (!ParseArrayElementValueInteger()) {
        context_.err =
            GetError("Invalid value for static array size value.", Error::ErrorType::ERR_BAD_ARRAY_SIZE_VALUE);
        return false;
    }

    ++context_;

    return true;
}

bool Parser::ParseRecordName()
{
    LOG(DEBUG, ASSEMBLER) << "started searching for record name (line " << line_stric_
                          << "): " << context_.tokens[context_.number - 1].whole_line;

    if (!RecordValidName()) {
        if (*context_ == Token::Type::DEL_BRACKET_L) {
            context_.err = GetError("No record name.", Error::ErrorType::ERR_BAD_RECORD_NAME);
            return false;
        }
        context_.err = GetError("Invalid name of the record.", Error::ErrorType::ERR_BAD_RECORD_NAME);
        return false;
    }

    auto iter = program_.record_table.find(std::string(context_.GiveToken().data(), context_.GiveToken().length()));
    if (iter == program_.record_table.end() || !iter->second.file_location->is_defined) {
        SetRecordInformation();
    } else {
        context_.err = GetError("This record already exists.", Error::ErrorType::ERR_BAD_ID_RECORD);
        return false;
    }

    LOG(DEBUG, ASSEMBLER) << "record name found (line " << line_stric_ << "): " << context_.GiveToken();

    ++context_;

    return true;
}

void Parser::SetRecordInformation()
{
    AddObjectInTable(true, program_.record_table);
    curr_record_ = &(program_.record_table.at(std::string(context_.GiveToken().data(), context_.GiveToken().length())));
}

bool Parser::ParseFunctionName()
{
    LOG(DEBUG, ASSEMBLER) << "started searching for function name (line " << line_stric_
                          << "): " << context_.tokens[context_.number - 1].whole_line;

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

    LOG(DEBUG, ASSEMBLER) << "function name found (line " << line_stric_ << "): " << context_.GiveToken();

    ++context_;

    return true;
}

void Parser::SetFunctionInformation()
{
    std::string p = std::string(context_.GiveToken());
    AddObjectInTable(true, program_.function_table);
    curr_func_ = &(program_.function_table.at(p));
    label_table_ = &(curr_func_->label_table);
    curr_func_->return_type = context_.curr_func_return_type;
}

bool Parser::ParseArrayName()
{
    LOG(DEBUG, ASSEMBLER) << "started searching for array name (line " << line_stric_
                          << "): " << context_.tokens[context_.number - 1].whole_line;

    if (!ArrayValidName()) {
        if (*context_ == Token::Type::DEL_BRACKET_L) {
            context_.err = GetError("No array name.", Error::ErrorType::ERR_BAD_ARRAY_NAME);
            return false;
        }
        context_.err = GetError("Invalid name of the array.", Error::ErrorType::ERR_BAD_ARRAY_NAME);
        return false;
    }

    auto iter =
        program_.literalarray_table.find(std::string(context_.GiveToken().data(), context_.GiveToken().length()));
    if (iter == program_.literalarray_table.end()) {
        SetArrayInformation();
    } else {
        context_.err = GetError("This array already exists.", Error::ErrorType::ERR_BAD_ID_ARRAY);
        return false;
    }

    LOG(DEBUG, ASSEMBLER) << "array id found (line " << line_stric_ << "): " << context_.GiveToken();

    ++context_;

    return true;
}

void Parser::SetArrayInformation()
{
    program_.literalarray_table.try_emplace(std::string(context_.GiveToken().data(), context_.GiveToken().length()),
                                            panda::pandasm::LiteralArray());

    curr_array_ =
        &(program_.literalarray_table.at(std::string(context_.GiveToken().data(), context_.GiveToken().length())));
}

void Parser::SetOperationInformation()
{
    context_.ins_number = curr_func_->ins.size() + 1;
    curr_debug_.line_number = line_stric_;
}

bool Parser::ParseFunctionReturn()
{
    LOG(DEBUG, ASSEMBLER) << "started searching for return function value (line " << line_stric_
                          << "): " << context_.tokens[context_.number - 1].whole_line;

    if (!TypeValidName()) {
        if (*context_ == Token::Type::DEL_BRACKET_L) {
            context_.err = GetError("No return type.", Error::ErrorType::ERR_BAD_FUNCTION_RETURN_VALUE);
            return false;
        }
        context_.err = GetError("Not a return type.", Error::ErrorType::ERR_BAD_FUNCTION_RETURN_VALUE);
        return false;
    }

    if (!ParseType(&context_.curr_func_return_type)) {
        return false;
    }

    LOG(DEBUG, ASSEMBLER) << "return type found (line " << line_stric_ << "): " << context_.GiveToken();

    return true;
}

bool Parser::TypeValidName()
{
    if (Type::GetId(context_.GiveToken()) != panda_file::Type::TypeId::REFERENCE) {
        return true;
    }

    return PrefixedValidName();
}

bool Parser::ParseFunctionArg()
{
    if (*context_ != Token::Type::ID) {
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

    if (*context_ == Token::Type::DEL_LT && !ParseMetaDef()) {
        return false;
    }

    curr_func_->params.push_back(std::move(parameter));

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
    LOG(DEBUG, ASSEMBLER) << "started searching for function parameters (line " << line_stric_
                          << "): " << context_.tokens[context_.number - 1].whole_line;

    bool comma = false;

    while (true) {
        if (context_.Mask()) {
            return false;
        }

        if (context_.id != Token::Type::DEL_COMMA && context_.id != Token::Type::ID) {
            break;
        }

        if (!ParseFunctionArgComma(comma)) {
            return false;
        }

        if (!ParseFunctionArg()) {
            return false;
        }
    }

    LOG(DEBUG, ASSEMBLER) << "parameters found (line " << line_stric_ << "): ";

    return true;
}

bool Parser::ParseMetaDef()
{
    LOG(DEBUG, ASSEMBLER) << "started searching for meta information (line " << line_stric_
                          << "): " << context_.tokens[context_.number - 1].whole_line;

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

    LOG(DEBUG, ASSEMBLER) << "searching for meta information (line " << line_stric_ << ") is successful";

    if (flag && context_.err.err == Error::ErrorType::ERR_NONE) {
        ++context_;
    }

    return true;
}

void Parser::SetMetadataContextError(const Metadata::Error &err, bool has_value)
{
    constexpr int64_t NO_VALUE_OFF = -1;
    constexpr int64_t SPECIAL_OFF = -2;
    constexpr int64_t STANDARD_OFF = -3;

    switch (err.GetType()) {
        case Metadata::Error::Type::UNKNOWN_ATTRIBUTE: {
            context_.err = GetError(err.GetMessage(), Error::ErrorType::ERR_BAD_METADATA_UNKNOWN_ATTRIBUTE, 0,
                                    has_value ? STANDARD_OFF : NO_VALUE_OFF);
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
                                    has_value ? STANDARD_OFF : NO_VALUE_OFF);
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
                                    has_value ? STANDARD_OFF : NO_VALUE_OFF);
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

bool Parser::BuildMetaListAttr(bool &eq, std::string &attribute_name, std::string &attribute_value)
{
    if (eq && *context_ == Token::Type::ID_STRING) {
        auto res = ParseStringLiteral();
        if (!res) {
            return false;
        }

        attribute_value = res.value();
    } else if (eq) {
        std::string sign {};
        attribute_value = context_.GiveToken();
        ++context_;

        if (!ParseOperandSignature(&sign)) {
            return false;
        }

        --context_;
        attribute_value += sign;
    } else {
        attribute_name = context_.GiveToken();
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
        bool has_value = eq;
        if (has_value) {
            res = metadata_->SetAttributeValue(attribute_name, attribute_value);
        } else {
            res = metadata_->SetAttribute(attribute_name);
        }

        eq = false;

        if (res) {
            auto err = res.value();
            SetMetadataContextError(err, has_value);
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

    std::string attribute_name;
    std::string attribute_value;

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

        if (!BuildMetaListAttr(eq, attribute_name, attribute_value)) {
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

}  // namespace panda::pandasm
