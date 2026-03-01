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

#include "program_dump.h"
#include "abc2program_log.h"
#include "common/abc_file_utils.h"

namespace panda::abc2program {

void PandasmProgramDumper::Dump(std::ostream &os, const pandasm::Program &program)
{
    program_ = &program;
    os << std::flush;
    DumpAbcFilePath(os);
    DumpLiteralArrayTable(os);
    DumpRecordTable(os);
    DumpFunctionTable(os);
    DumpStrings(os);
}

void PandasmProgramDumper::SetAbcFilePath(const std::string &abc_file_path)
{
    abc_file_path_ = abc_file_path;
}

void PandasmProgramDumper::DumpAbcFilePath(std::ostream &os) const
{
    if (abc_file_path_.empty()) {
        return;
    }
    std::string file_abs_path = os::file::File::GetAbsolutePath(abc_file_path_).Value();
    os << DUMP_TITLE_SOURCE_BINARY << file_abs_path << DUMP_CONTENT_DOUBLE_ENDL;
}

void PandasmProgramDumper::DumpLanguage(std::ostream &os, const panda::panda_file::SourceLang lang) const
{
    os << DUMP_TITLE_LANGUAGE;
    if (lang == panda::panda_file::SourceLang::ECMASCRIPT) {
        os << DUMP_CONTENT_ECMASCRIPT;
    } else if (lang == panda::panda_file::SourceLang::JAVASCRIPT) {
        os << DUMP_CONTENT_JAVASCRIPT;
    } else if (lang == panda::panda_file::SourceLang::TYPESCRIPT) {
        os << DUMP_CONTENT_TYPESCRIPT;
    } else if (lang == panda::panda_file::SourceLang::ARKTS) {
        os << DUMP_CONTENT_ARKTS;
    } else {
        os << DUMP_CONTENT_PANDA_ASSEMBLY;
    }
    os << DUMP_CONTENT_SINGLE_ENDL;
}

void PandasmProgramDumper::DumpProgramLanguage(std::ostream &os) const
{
    DumpLanguage(os, program_->lang);
}

void PandasmProgramDumper::DumpRecordLanguage(std::ostream &os, const pandasm::Record &record) const
{
    DumpLanguage(os, record.language);
}

void PandasmProgramDumper::DumpFunctionLanguage(std::ostream &os, const pandasm::Function &function) const
{
    DumpLanguage(os, function.language);
}

void PandasmProgramDumper::DumpLiteralArrayTable(std::ostream &os) const
{
    os << DUMP_TITLE_SEPARATOR;
    os << DUMP_TITLE_LITERALS;
    os << DUMP_CONTENT_DOUBLE_ENDL;
    auto it = program_->literalarray_table.begin();
    auto end = program_->literalarray_table.end();
    // In normalized mode, sort dump result of literal arrays before output.
    if (is_normalized_) {
        std::vector<std::string> literal_arrays;
        for (; it != end; ++it) {
            auto id = PandasmDumperUtils::GetLiteralArrayIdFromName(it->first);
            auto lit_arr = SerializeLiteralArray(it->second, id);
            lit_arr += DUMP_CONTENT_DOUBLE_ENDL;
            literal_arrays.emplace_back(lit_arr);
        }
        std::sort(literal_arrays.begin(), literal_arrays.end());
        for (auto &str : literal_arrays) {
            os << str;
        }
    } else {
        for (; it != end; ++it) {
            os << it->first << DUMP_CONTENT_SPACE;
            auto id = PandasmDumperUtils::GetLiteralArrayIdFromName(it->first);
            os << SerializeLiteralArray(it->second, id);
            os << DUMP_CONTENT_DOUBLE_ENDL;
        }
    }
    os << DUMP_CONTENT_DOUBLE_ENDL;
}

void PandasmProgramDumper::DumpRecordTable(std::ostream &os) const
{
    os << DUMP_TITLE_SEPARATOR;
    os << DUMP_TITLE_RECORDS;
    os << DUMP_CONTENT_DOUBLE_ENDL;
    for (const auto &it : program_->record_table) {
        if (is_normalized_ && is_debug_ && it.first == SLOT_NUMBER_RECORD_NAME) {
            continue;
        }
        DumpRecord(os, it.second);
    }
    os << DUMP_CONTENT_SINGLE_ENDL;
}

void PandasmProgramDumper::DumpRecord(std::ostream &os, const pandasm::Record &record) const
{
    DumpRecordLanguage(os, record);
    if (is_normalized_) {
        if (AbcFileUtils::IsGlobalTypeName(record.name)) {
            return;
        }
        if (AbcFileUtils::IsESTypeAnnotationName(record.name)) {
            return;
        }
    }
    os << DUMP_TITLE_RECORD << record.name;
    if (DumpRecordMetaData(os, record)) {
        DumpFieldList(os, record);
    }
    DumpRecordSourceFile(os, record);
}

bool PandasmProgramDumper::DumpRecordMetaData(std::ostream &os, const pandasm::Record &record) const
{
    if (record.metadata->IsForeign()) {
        os << DUMP_CONTENT_SPACE << DUMP_CONTENT_ATTR_EXTERNAL;
        os << DUMP_CONTENT_SINGLE_ENDL;
        return false;
    }
    return true;
}

void PandasmProgramDumper::DumpFieldList(std::ostream &os, const pandasm::Record &record) const
{
    os << DUMP_CONTENT_SPACE << DUMP_CONTENT_LEFT_CURLY_BRACKET << DUMP_CONTENT_SINGLE_ENDL;
    for (const auto &it : record.field_list) {
        DumpField(os, it);
    }
    os << DUMP_CONTENT_RIGHT_CURLY_BRACKET << DUMP_CONTENT_SINGLE_ENDL;
}

void PandasmProgramDumper::DumpField(std::ostream &os, const pandasm::Field &field) const
{
    os << DUMP_CONTENT_TAB << field.type.GetPandasmName() << DUMP_CONTENT_SPACE << field.name;
    DumpFieldMetaData(os, field);
    os << DUMP_CONTENT_SINGLE_ENDL;
}

void PandasmProgramDumper::DumpFieldMetaData(std::ostream &os, const pandasm::Field &field) const
{
    if (field.metadata->GetValue()) {
        DumpScalarValue(os, *(field.metadata->GetValue()));
    }
}

void PandasmProgramDumper::DumpAnnotationData(std::ostream &os, const pandasm::AnnotationData &anno) const
{
    os << DUMP_CONTENT_SPACE << anno.GetName() << DUMP_CONTENT_SINGLE_ENDL;
    for (const auto &element : anno.GetElements()) {
        os << DUMP_CONTENT_SPACE << element.GetName();
        if (element.GetValue()->IsArray()) {
            DumpArrayValue(os, *(element.GetValue()->GetAsArray()));
        } else {
            DumpScalarValue(os, *(element.GetValue()->GetAsScalar()));
        }
    }
}

void PandasmProgramDumper::DumpArrayValue(std::ostream &os, const pandasm::ArrayValue &array) const
{
    for (const auto &val : array.GetValues()) {
        DumpScalarValue(os, val);
    }
}
void PandasmProgramDumper::DumpScalarValue(std::ostream &os, const pandasm::ScalarValue &scalar) const
{
    switch (scalar.GetType()) {
        case pandasm::Value::Type::U1:
        case pandasm::Value::Type::I8:
        case pandasm::Value::Type::U8:
        case pandasm::Value::Type::I16:
        case pandasm::Value::Type::U16:
        case pandasm::Value::Type::I32:
        case pandasm::Value::Type::U32:
        case pandasm::Value::Type::I64:
        case pandasm::Value::Type::U64:
        case pandasm::Value::Type::STRING_NULLPTR: {
            os << DUMP_CONTENT_SPACE << scalar.GetValue<uint64_t>();
            break;
        }
        case pandasm::Value::Type::F32:
            os << DUMP_CONTENT_SPACE << scalar.GetValue<float>();
            break;
        case pandasm::Value::Type::F64: {
            os << DUMP_CONTENT_SPACE << scalar.GetValue<double>();
            break;
        }
        case pandasm::Value::Type::STRING:
        case pandasm::Value::Type::METHOD:
        case pandasm::Value::Type::ENUM:
        case pandasm::Value::Type::LITERALARRAY: {
            if (!is_normalized_) {
                os << DUMP_CONTENT_SPACE << scalar.GetValue<std::string>();
            } else {
                auto literal_array_id_name = scalar.GetValue<std::string>();
                auto it = program_->literalarray_table.find(literal_array_id_name);
                ASSERT(it != program_->literalarray_table.end());
                auto id = PandasmDumperUtils::GetLiteralArrayIdFromName(literal_array_id_name);
                os << DUMP_CONTENT_SPACE << SerializeLiteralArray(it->second, id);
            }
            break;
        }
        case pandasm::Value::Type::RECORD: {
            pandasm::Type type = scalar.GetValue<pandasm::Type>();
            os << DUMP_CONTENT_SPACE << type.GetComponentName() << DUMP_CONTENT_SPACE << type.GetName();
            break;
        }
        case pandasm::Value::Type::ANNOTATION: {
            DumpAnnotationData(os, scalar.GetValue<pandasm::AnnotationData>());
            break;
        }
        default :
            break;
    }
}

void PandasmProgramDumper::DumpRecordSourceFile(std::ostream &os, const pandasm::Record &record) const
{
    os << DUMP_TITLE_RECORD_SOURCE_FILE << record.source_file << DUMP_CONTENT_DOUBLE_ENDL;
}

void PandasmProgramDumper::DumpFunctionTable(std::ostream &os)
{
    os << DUMP_TITLE_SEPARATOR;
    os << DUMP_TITLE_METHODS;
    os << DUMP_CONTENT_DOUBLE_ENDL;
    for (const auto &it : program_->function_table) {
        DumpFunction(os, it.second);
    }
}

void PandasmProgramDumper::DumpFunction(std::ostream &os, const pandasm::Function &function)
{
    regs_num_ = function.regs_num;
    DumpFunctionKind(os, function);
    if (!is_normalized_ || !is_debug_) {
        DumpFunctionAnnotations(os, function);
    }
    DumpFunctionLanguage(os, function);
    DumpFunctionHead(os, function);
    DumpFunctionBody(os, function);
}

void PandasmProgramDumper::DumpFunctionKind(std::ostream &os, const pandasm::Function &function) const
{
    os << DUMP_TITLE_FUNCTION_KIND << PandasmDumperUtils::GetFunctionKindString(function.function_kind);
    os << DUMP_CONTENT_SINGLE_ENDL;
}

void PandasmProgramDumper::DumpFunctionAnnotations(std::ostream &os, const pandasm::Function &function) const
{
    for (auto &annotation : function.metadata->GetAnnotations()) {
        DumpAnnotationData(os, annotation);
        os << DUMP_CONTENT_SINGLE_ENDL;
    }
}

void PandasmProgramDumper::DumpFunctionHead(std::ostream &os, const pandasm::Function &function) const
{
    os << DUMP_TITLE_FUNCTION;
    DumpFunctionReturnType(os, function);
    DumpFunctionName(os, function);
    DumpFunctionParams(os, function);
    os << DUMP_CONTENT_SPACE << DUMP_CONTENT_LEFT_CURLY_BRACKET << DUMP_CONTENT_SINGLE_ENDL;
}

void PandasmProgramDumper::DumpFunctionReturnType(std::ostream &os, const pandasm::Function &function) const
{
    os << function.return_type.GetPandasmName() << DUMP_CONTENT_SPACE;
}

void PandasmProgramDumper::DumpFunctionName(std::ostream &os, const pandasm::Function &function) const
{
    os << function.name;
}

void PandasmProgramDumper::DumpFunctionParams(std::ostream &os, const pandasm::Function &function) const
{
    os << DUMP_CONTENT_LEFT_PARENTHESIS;
    if (function.params.size() > 0) {
        DumpFunctionParamAtIndex(os, function.params[0], 0);
        for (size_t i = 1; i < function.params.size(); ++i) {
            os << DUMP_CONTENT_COMMA << DUMP_CONTENT_SPACE;
            DumpFunctionParamAtIndex(os, function.params[i], i);
        }
    }
    os << DUMP_CONTENT_RIGHT_PARENTHESIS;
}

void PandasmProgramDumper::DumpFunctionParamAtIndex(std::ostream &os,
                                                    const pandasm::Function::Parameter &param,
                                                    size_t idx) const
{
    os << param.type.GetPandasmName() << DUMP_CONTENT_SPACE << DUMP_CONTENT_FUNCTION_PARAM_NAME_PREFIX << idx;
}

void PandasmProgramDumper::DumpFunctionAttributes(std::ostream &os, const pandasm::Function &function) const
{
    log::Unimplemented(__PRETTY_FUNCTION__);
}

void PandasmProgramDumper::DumpFunctionBody(std::ostream &os, const pandasm::Function &function)
{
    DumpFunctionIns(os, function);
    DumpFunctionCatchBlocks(os, function);
    DumpFunctionDebugInfo(os, function);
    os << "}" << DUMP_CONTENT_DOUBLE_ENDL;
}

void PandasmProgramDumper::DumpFunctionIns(std::ostream &os, const pandasm::Function &function)
{
    if (is_normalized_) {
        DumpNormalizedFunctionIns(os, function);
    } else {
        DumpOriginalFunctionIns(os, function);
    }
}

void PandasmProgramDumper::DumpOriginalFunctionIns(std::ostream &os, const pandasm::Function &function)
{
    for (const pandasm::InsPtr &pa_ins : function.ins) {
        std::string insStr = pa_ins->ToString("", true, regs_num_);
        os << DUMP_CONTENT_TAB << std::setw(LINE_WIDTH)
           << std::left << insStr
           << DUMP_CONTENT_LINE_NUMBER << pa_ins->ins_debug.line_number;
        os << std::setw(COLUMN_WIDTH) << std::left << DUMP_CONTENT_SPACE
           << DUMP_CONTENT_COLUMN_NUMBER << pa_ins->ins_debug.column_number
           << DUMP_CONTENT_SINGLE_ENDL;
    }
}

void PandasmProgramDumper::DumpNormalizedFunctionIns(std::ostream &os, const pandasm::Function &function)
{
    GetFinalDumpIns(function);
    UpdateLabels4DumpIns(final_dump_ins_ptrs_, label_map_);
    DumpFinalIns(os);
}

void PandasmProgramDumper::GetFinalDumpIns(const pandasm::Function &function)
{
    label_map_.clear();
    final_dump_ins_ptrs_.clear();
    original_to_final_index_map_.clear();
    size_t valid_op_cnt = 0;
    for (size_t idx = 0; idx < function.ins.size(); idx++) {
        auto &ins = function.ins[idx];
        original_to_final_index_map_.emplace(idx, valid_op_cnt);
        if (ins->IsLabel()) {
            label_map_.emplace(ins->Label(), AbcFileUtils::GetLabelNameByInstIdx(valid_op_cnt));
        } else {
            valid_op_cnt++;
            final_dump_ins_ptrs_.emplace_back(ins->DeepCopy());
        }
    }
    original_to_final_index_map_.emplace(function.ins.size(), final_dump_ins_ptrs_.size());
}

void PandasmProgramDumper::DumpFinalIns(std::ostream &os)
{
    size_t idx = 0;
    for (pandasm::InsPtr &pa_ins : final_dump_ins_ptrs_) {
        if (PandasmDumperUtils::IsMatchLiteralId(pa_ins)) {
            ReplaceLiteralId4Ins(pa_ins);
        }

        std::string insStr = AbcFileUtils::GetLabelNameByInstIdx(idx++) + ": " + pa_ins->ToString("", true, regs_num_);
        os << DUMP_CONTENT_TAB << std::setw(LINE_WIDTH)
           << std::left << insStr
           << DUMP_CONTENT_LINE_NUMBER << pa_ins->ins_debug.line_number;
        os << std::setw(COLUMN_WIDTH) << std::left << DUMP_CONTENT_SPACE
           << DUMP_CONTENT_COLUMN_NUMBER << pa_ins->ins_debug.column_number
           << DUMP_CONTENT_SINGLE_ENDL;
    }
}

void PandasmProgramDumper::UpdateLabels4DumpIns(std::vector<pandasm::InsPtr> &dump_ins, const LabelMap &label_map) const
{
    size_t dump_ins_size = dump_ins.size();
    for (size_t i = 0; i < dump_ins_size; ++i) {
        UpdateLabels4DumpInsAtIndex(i, dump_ins, label_map);
    }
}

void PandasmProgramDumper::UpdateLabels4DumpInsAtIndex(size_t idx, std::vector<pandasm::InsPtr> &dump_ins,
                                                       const LabelMap &label_map) const
{
    pandasm::InsPtr &curr_ins = dump_ins[idx];
    if (curr_ins->IsJump()) {
        std::string mapped_label = PandasmDumperUtils::GetMappedLabel(curr_ins->GetId(0), label_map);
        if (mapped_label != "") {
            curr_ins->SetId(0, mapped_label);
        }
    }
}

void PandasmProgramDumper::DumpFunctionCatchBlocks(std::ostream &os, const pandasm::Function &function) const
{
    if (is_normalized_) {
        DumpNormalizedFunctionCatchBlocks(os, function);
    } else {
        DumpOriginalFunctionCatchBlocks(os, function);
    }
}

void PandasmProgramDumper::DumpOriginalFunctionCatchBlocks(std::ostream &os,
                                                           const pandasm::Function &function) const
{
    for (const pandasm::Function::CatchBlock &catch_block : function.catch_blocks) {
        DumpCatchBlock(os, catch_block);
    }
}

void PandasmProgramDumper::DumpNormalizedFunctionCatchBlocks(std::ostream &os,
                                                             const pandasm::Function &function) const
{
    std::vector<pandasm::Function::CatchBlock> catch_blocks;
    for (const pandasm::Function::CatchBlock &catch_block : function.catch_blocks) {
        catch_blocks.emplace_back(PandasmDumperUtils::DeepCopyCatchBlock(catch_block));
    }
    for (pandasm::Function::CatchBlock &catch_block : catch_blocks) {
        UpdateCatchBlock(catch_block);
    }
    for (const pandasm::Function::CatchBlock &catch_block : catch_blocks) {
        DumpCatchBlock(os, catch_block);
    }
}

void PandasmProgramDumper::UpdateCatchBlock(pandasm::Function::CatchBlock &catch_block) const
{
    catch_block.try_begin_label = GetUpdatedCatchBlockLabel(catch_block.try_begin_label);
    catch_block.try_end_label = GetUpdatedCatchBlockLabel(catch_block.try_end_label);
    catch_block.catch_begin_label = GetUpdatedCatchBlockLabel(catch_block.catch_begin_label);
    catch_block.catch_end_label = GetUpdatedCatchBlockLabel(catch_block.catch_end_label);
}

std::string PandasmProgramDumper::GetUpdatedCatchBlockLabel(const std::string &orignal_label) const
{
    std::string mapped_label1 = PandasmDumperUtils::GetMappedLabel(orignal_label, label_map_);
    if (mapped_label1 == "") {
        return orignal_label;
    }
    return mapped_label1;
}

void PandasmProgramDumper::DumpCatchBlock(std::ostream &os, const pandasm::Function::CatchBlock &catch_block) const
{
    if (catch_block.exception_record.empty()) {
        os << DUMP_TITLE_CATCH_ALL << DUMP_CONTENT_SINGLE_ENDL;
    } else {
        os << DUMP_TITLE_CATCH << DUMP_CONTENT_SINGLE_ENDL;
    }
    os << DUMP_CONTENT_TAB << DUMP_CONTENT_TRY_BEGIN_LABEL
       << catch_block.try_begin_label << DUMP_CONTENT_SINGLE_ENDL;
    os << DUMP_CONTENT_TAB << DUMP_CONTENT_TRY_END_LABEL
       << catch_block.try_end_label << DUMP_CONTENT_SINGLE_ENDL;
    os << DUMP_CONTENT_TAB << DUMP_CONTENT_CATCH_BEGIN_LABEL
       << catch_block.catch_begin_label << DUMP_CONTENT_SINGLE_ENDL;
    if (!is_normalized_) {
        os << DUMP_CONTENT_TAB << DUMP_CONTENT_CATCH_END_LABEL
           << catch_block.catch_end_label << DUMP_CONTENT_SINGLE_ENDL;
    }
}

void PandasmProgramDumper::DumpFunctionDebugInfo(std::ostream &os, const pandasm::Function &function)
{
    if (function.local_variable_debug.empty()) {
        return;
    }
    std::map<int32_t, panda::pandasm::debuginfo::LocalVariable> local_variable_table;
    if (is_normalized_) {
        UpdateLocalVarMap(function, local_variable_table);
    } else {
        for (const auto &variable_info : function.local_variable_debug) {
            local_variable_table[variable_info.reg] = variable_info;
        }
    }

    os << DUMP_CONTENT_SINGLE_ENDL;
    if (local_variable_table.empty()) {
        return;
    }

    os << DUMP_TITLE_LOCAL_VAR_TABLE;
    os << DUMP_CONTENT_SINGLE_ENDL;
    os << DUMP_CONTENT_LOCAL_VAR_TABLE;
    for (const auto &iter : local_variable_table) {
        const auto &variable_info = iter.second;
        os << DUMP_CONTENT_TAB
           << std::setw(START_WIDTH) << std::right << variable_info.start << DUMP_CONTENT_TRIPLE_SPACES;
        os << std::setw(END_WIDTH) << std::right << variable_info.length << DUMP_CONTENT_DOUBLE_SPACES;
        os << std::setw(REG_WIDTH) << std::right << variable_info.reg << DUMP_CONTENT_DOUBLE_SPACES;
        os << std::setw(NAME_WIDTH)
           << std::right << variable_info.name << DUMP_CONTENT_NONUPLE_SPACES << variable_info.signature;
        if (!variable_info.signature_type.empty() && variable_info.signature_type != variable_info.signature) {
            os << " (" << variable_info.signature_type << ")";
        }
        os << DUMP_CONTENT_SINGLE_ENDL;
    }
}

void PandasmProgramDumper::UpdateLocalVarMap(const pandasm::Function &function,
    std::map<int32_t, panda::pandasm::debuginfo::LocalVariable>& local_variable_table)
{
    for (const auto &variable_info : function.local_variable_debug) {
        uint32_t original_start = variable_info.start;
        uint32_t original_end = variable_info.length + variable_info.start;
        uint32_t new_start = original_to_final_index_map_[original_start];
        uint32_t new_length = original_to_final_index_map_[original_end] - new_start;
        panda::pandasm::debuginfo::LocalVariable local_var = {variable_info.name,
                                                              variable_info.signature,
                                                              variable_info.signature_type,
                                                              variable_info.reg,
                                                              new_start,
                                                              new_length};
        local_variable_table[variable_info.reg] = local_var;
    }
}

void PandasmProgramDumper::DumpStrings(std::ostream &os) const
{
    if (is_normalized_) {
        return;
    }
    os << DUMP_TITLE_SEPARATOR;
    os << DUMP_TITLE_STRING;
    for (const std::string &str : program_->strings) {
        os << str << DUMP_CONTENT_SINGLE_ENDL;
    }
}

void PandasmProgramDumper::ReplaceLiteralId4Ins(pandasm::InsPtr &pa_ins) const
{
    size_t idx = PandasmDumperUtils::GetLiteralIdIndex4Ins(pa_ins);
    std::string id_str = pa_ins->GetId(idx);
    auto it = program_->literalarray_table.find(id_str);
    ASSERT(it != program_->literalarray_table.end());
    const pandasm::LiteralArray &literal_array = it->second;
    auto id = PandasmDumperUtils::GetLiteralArrayIdFromName(it->first);
    std::string replaced_value = SerializeLiteralArray(literal_array, id);
    pa_ins->SetId(idx, replaced_value);
}

std::string PandasmProgramDumper::SerializeLiteralArray(const pandasm::LiteralArray &lit_array, uint32_t id) const
{
    if (lit_array.literals_.empty()) {
        return "";
    }
    std::stringstream ss;
    ss << DUMP_CONTENT_LEFT_CURLY_BRACKET << DUMP_CONTENT_SPACE;
    ss << lit_array.literals_.size();
    ss << DUMP_CONTENT_SPACE << DUMP_CONTENT_LEFT_SQUARE_BRACKET << DUMP_CONTENT_SPACE;
    processing_literal_array_id_set_.emplace(id);
    SerializeLiterals(lit_array, ss);
    processing_literal_array_id_set_.erase(id);
    ss << DUMP_CONTENT_RIGHT_SQUARE_BRACKET << DUMP_CONTENT_RIGHT_CURLY_BRACKET;
    return ss.str();
}

void PandasmProgramDumper::SerializeLiterals(const pandasm::LiteralArray &lit_array, std::stringstream &os) const
{
    for (size_t i = 0; i < lit_array.literals_.size(); i++) {
        SerializeLiteralsAtIndex(lit_array, os, i);
        os << DUMP_CONTENT_COMMA << DUMP_CONTENT_SPACE;
    }
}

void PandasmProgramDumper::SerializeLiteralsAtIndex(
    const pandasm::LiteralArray &lit_array, std::stringstream &os, size_t i) const
{
    const panda_file::LiteralTag &tag = lit_array.literals_[i].tag_;
    os << PandasmDumperUtils::LiteralTagToString(tag) << DUMP_CONTENT_COLON;
    const auto &val = lit_array.literals_[i].value_;
    switch (tag) {
        case panda_file::LiteralTag::BOOL:
            os << (std::get<bool>(val));
            break;
        case panda_file::LiteralTag::LITERALBUFFERINDEX:
        case panda_file::LiteralTag::INTEGER:
            os << (bit_cast<int32_t>(std::get<uint32_t>(val)));
            break;
        case panda_file::LiteralTag::DOUBLE:
            os << (std::get<double>(val));
            break;
        case panda_file::LiteralTag::STRING:
            os << "\"" << (std::get<std::string>(val)) << "\"";
            break;
        case panda_file::LiteralTag::METHOD:
        case panda_file::LiteralTag::GETTER:
        case panda_file::LiteralTag::SETTER:
        case panda_file::LiteralTag::GENERATORMETHOD:
        case panda_file::LiteralTag::ASYNCGENERATORMETHOD:
        case panda_file::LiteralTag::ETS_IMPLEMENTS:
            os << (std::get<std::string>(val));
            break;
        case panda_file::LiteralTag::NULLVALUE:
        case panda_file::LiteralTag::ACCESSOR:
            os << (static_cast<int16_t>(bit_cast<int8_t>(std::get<uint8_t>(val))));
            break;
        case panda_file::LiteralTag::METHODAFFILIATE:
            os << (std::get<uint16_t>(val));
            break;
        case panda_file::LiteralTag::LITERALARRAY:
            SerializeNestedLiteralArrayById(os, std::get<std::string>(val));
            break;
        case panda_file::LiteralTag::BUILTINTYPEINDEX:
            os << (static_cast<int16_t>(std::get<uint8_t>(val)));
            break;
        case panda_file::LiteralTag::TAGVALUE:
            os << (static_cast<int16_t>(std::get<uint8_t>(val)));
            break;
        default:
            UNREACHABLE();
    }
}

void PandasmProgramDumper::SerializeNestedLiteralArrayById(
    std::stringstream &os, const std::string &literal_array_id_name) const
{
    if (!is_normalized_) {
        os << literal_array_id_name;
        return;
    }
    auto id = PandasmDumperUtils::GetLiteralArrayIdFromName(literal_array_id_name);
    if (processing_literal_array_id_set_.find(id) == processing_literal_array_id_set_.end()) {
        auto it = program_->literalarray_table.find(literal_array_id_name);
        ASSERT(it != program_->literalarray_table.end());
        os << SerializeLiteralArray(it->second, id);
    } else {
        UNREACHABLE();
    }
}

}  // namespace panda::abc2program
