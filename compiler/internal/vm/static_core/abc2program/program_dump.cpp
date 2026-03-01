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
#include "mangling.h"
#include "libarkfile/method_data_accessor-inl.h"
#include "abc2program_log.h"
#include "abc_file_utils.h"
#include "libarkbase/generated/source_language.h"

namespace ark::abc2program {

void PandasmProgramDumper::Dump(std::ostream &os, const pandasm::Program &program) const
{
    os << std::flush;
    DumpAbcFilePath(os);
    DumpProgramLanguage(os, program);
    DumpLiteralArrayTable(os, program);
    DumpRecordTable(os, program);
    DumpFunctionTable(os, program);
}

bool PandasmProgramDumper::HasNoAbcInput() const
{
    return ((file_ == nullptr) || (stringTable_ == nullptr));
}

void PandasmProgramDumper::DumpAbcFilePath(std::ostream &os) const
{
    if (HasNoAbcInput()) {
        return;
    }
    os << DUMP_TITLE_SOURCE_BINARY << file_->GetFilename() << DUMP_CONTENT_DOUBLE_ENDL;
}

void PandasmProgramDumper::DumpProgramLanguage(std::ostream &os, const pandasm::Program &program) const
{
    os << DUMP_TITLE_LANGUAGE;
    os << ark::panda_file::LanguageToString(program.lang);
    os << DUMP_CONTENT_DOUBLE_ENDL;
}

void PandasmProgramDumper::DumpLiteralArrayTable(std::ostream &os, const pandasm::Program &program) const
{
    if (HasNoAbcInput()) {
        DumpLiteralArrayTableWithoutKey(os, program);
    } else {
        DumpLiteralArrayTableWithKey(os, program);
    }
}

void PandasmProgramDumper::DumpLiteralArrayTableWithKey(std::ostream &os, const pandasm::Program &program) const
{
    os << DUMP_TITLE_SEPARATOR;
    os << DUMP_TITLE_LITERALS << DUMP_CONTENT_SINGLE_ENDL;
    for (auto &[key, litArray] : program.literalarrayTable) {
        DumpLiteralArrayWithKey(os, key, litArray, program);
    }
    os << DUMP_CONTENT_DOUBLE_ENDL;
}

void PandasmProgramDumper::DumpLiteralArrayWithKey(std::ostream &os, const std::string &key,
                                                   const pandasm::LiteralArray &litArray,
                                                   const pandasm::Program &program) const
{
    os << DUMP_TITLE_LITERAL_ARRAY;
    os << DUMP_LITERAL_ARRAY_PREFIX;
    os << key << " ";
    DumpLiteralArray(os, litArray, program);
    os << DUMP_CONTENT_SINGLE_ENDL;
}

void PandasmProgramDumper::DumpLiteralArrayTableWithoutKey(std::ostream &os, const pandasm::Program &program) const
{
    for (auto &[unused, litArray] : program.literalarrayTable) {
        (void)unused;
        DumpLiteralArray(os, litArray, program);
        os << DUMP_CONTENT_SINGLE_ENDL;
    }
    os << DUMP_CONTENT_DOUBLE_ENDL;
}

void PandasmProgramDumper::DumpRecordTable(std::ostream &os, const pandasm::Program &program) const
{
    os << DUMP_TITLE_SEPARATOR;
    os << DUMP_TITLE_RECORDS;
    os << DUMP_CONTENT_DOUBLE_ENDL;
    for (const auto &it : program.recordTable) {
        DumpRecord(os, it.second);
    }
    os << DUMP_CONTENT_SINGLE_ENDL;
}

void PandasmProgramDumper::DumpRecord(std::ostream &os, const pandasm::Record &record) const
{
    if (AbcFileUtils::IsSystemTypeName(record.name)) {
        return;
    }
    os << DUMP_TITLE_RECORD << record.name;
    if (DumpMetaData(os, *record.metadata)) {
        DumpFieldList(os, record);
    }
    DumpRecordSourceFile(os, record);
}

void PandasmProgramDumper::DumpFieldList(std::ostream &os, const pandasm::Record &record) const
{
    if (record.metadata->IsForeign() && record.fieldList.empty()) {
        os << DUMP_CONTENT_DOUBLE_ENDL;
        return;
    }
    os << " {" << DUMP_CONTENT_SINGLE_ENDL;
    for (const auto &it : record.fieldList) {
        DumpField(os, it);
        os << DUMP_CONTENT_SINGLE_ENDL;
    }
    os << DUMP_CONTENT_SINGLE_ENDL << "}" << DUMP_CONTENT_SINGLE_ENDL;
}

void PandasmProgramDumper::DumpField(std::ostream &os, const pandasm::Field &field) const
{
    os << "\t" << field.type.GetPandasmName() << " " << field.name;
    DumpMetaData(os, *field.metadata);
}

bool PandasmProgramDumper::DumpMetaData(std::ostream &os, const pandasm::ItemMetadata &meta) const
{
    auto boolAttributes = meta.GetBoolAttributes();
    auto attributes = meta.GetAttributes();
    if (boolAttributes.empty() && attributes.empty()) {
        return true;
    }

    os << " <";

    size_t size = boolAttributes.size();
    size_t idx = 0;
    for (const auto &attr : boolAttributes) {
        os << attr;
        ++idx;

        if (!attributes.empty() || idx < size) {
            os << ", ";
        }
    }

    size = attributes.size();
    idx = 0;
    for (const auto &[key, values] : attributes) {
        for (size_t i = 0; i < values.size(); i++) {
            os << key << "=" << values[i];

            if (i < values.size() - 1) {
                os << ", ";
            }
        }

        ++idx;

        if (idx < size) {
            os << ", ";
        }
    }

    os << ">";
    return true;
}

void PandasmProgramDumper::DumpRecordSourceFile(std::ostream &os, const pandasm::Record &record) const
{
    os << DUMP_TITLE_RECORD_SOURCE_FILE << record.sourceFile << DUMP_CONTENT_DOUBLE_ENDL;
}

void PandasmProgramDumper::DumpFunctionTable(std::ostream &os, const pandasm::Program &program) const
{
    os << DUMP_TITLE_SEPARATOR;
    os << DUMP_TITLE_METHODS;
    os << DUMP_CONTENT_DOUBLE_ENDL;
    for (const auto &it : program.functionStaticTable) {
        DumpFunction(os, it.second);
    }
    for (const auto &it : program.functionInstanceTable) {
        DumpFunction(os, it.second);
    }
}

void PandasmProgramDumper::DumpFunction(std::ostream &os, const pandasm::Function &function) const
{
    os << DUMP_TITLE_FUNCTION;
    os << function.returnType.GetPandasmName() << " " << function.name << "(";
    if (!function.params.empty()) {
        os << function.params[0].type.GetPandasmName() << " a0";

        for (size_t i = 1; i < function.params.size(); i++) {
            os << ", " << function.params[i].type.GetPandasmName() << " a" << (size_t)i;
        }
    }
    os << ")";

    const std::string signature = pandasm::GetFunctionSignatureFromName(function.name, function.params);

    DumpMetaData(os, *function.metadata);

    if (!function.HasImplementation()) {
        os << DUMP_CONTENT_DOUBLE_ENDL;
        return;
    }

    os << " {";

    os << DUMP_CONTENT_SINGLE_ENDL;

    DumpInstructions(os, function);

    os << "}" << DUMP_CONTENT_DOUBLE_ENDL;
}

void PandasmProgramDumper::DumpInstructions(std::ostream &os, const pandasm::Function &function) const
{
    for (const auto &instr : function.ins) {
        std::string ins = instr.ToString("", function.GetParamsNum() != 0, function.regsNum);
        if (instr.HasLabel()) {
            std::string delim = ": ";
            size_t pos = ins.find(delim);
            std::string label = ins.substr(0, pos);
            ins.erase(0, pos + delim.length());

            os << label << ":";
            os << DUMP_CONTENT_SINGLE_ENDL;
        }

        os << "\t";
        os << ins;
        os << DUMP_CONTENT_SINGLE_ENDL;
    }
}

void PandasmProgramDumper::DumpStrings(std::ostream &os, const pandasm::Program &program) const
{
    if (HasNoAbcInput()) {
        DumpStringsByProgram(os, program);
    } else {
        DumpStringsByStringTable(os, *stringTable_);
    }
}

void PandasmProgramDumper::DumpStringsByStringTable(std::ostream &os, const AbcStringTable &stringTable) const
{
    os << DUMP_TITLE_SEPARATOR;
    os << DUMP_TITLE_STRING;
    stringTable.Dump(os);
    os << DUMP_CONTENT_SINGLE_ENDL;
}

void PandasmProgramDumper::DumpStringsByProgram(std::ostream &os, const pandasm::Program &program) const
{
    os << DUMP_TITLE_SEPARATOR;
    os << DUMP_TITLE_STRING;
    os << DUMP_CONTENT_SINGLE_ENDL;
    for (const std::string &str : program.strings) {
        os << str << DUMP_CONTENT_SINGLE_ENDL;
    }
    os << DUMP_CONTENT_SINGLE_ENDL;
}

void PandasmProgramDumper::DumpLiteralArray(std::ostream &os, const pandasm::LiteralArray &litArray,
                                            const pandasm::Program &program) const
{
    if (litArray.literals.empty()) {
        return;
    }

    bool isConst = litArray.literals[0].IsArray();

    std::stringstream specifiers {};
    if (isConst) {
        specifiers << LiteralTagToString(litArray.literals[0].tag, program) << " " << litArray.literals.size() << " ";
    }
    os << specifiers.str() << "{";

    DumpValues(litArray, isConst, os, program);

    os << "}" << DUMP_CONTENT_SINGLE_ENDL;
}

template <typename T>
std::string LiteralIntegralValueToString(const pandasm::LiteralArray::Literal &lit)
{
    std::stringstream res {};
    T value = std::get<T>(lit.value);
    if (lit.IsSigned()) {
        res << +static_cast<typename std::make_signed<T>::type>(value);
    } else {
        res << +value;
    }
    return res.str();
}

std::string PandasmProgramDumper::LiteralValueToString(const pandasm::LiteralArray::Literal &lit) const
{
    if (lit.IsBoolValue()) {
        std::stringstream res {};
        res << (std::get<bool>(lit.value));
        return res.str();
    }

    if (lit.IsByteValue()) {
        return LiteralIntegralValueToString<uint8_t>(lit);
    }

    if (lit.IsShortValue()) {
        return LiteralIntegralValueToString<uint16_t>(lit);
    }

    if (lit.IsIntegerValue()) {
        return LiteralIntegralValueToString<uint32_t>(lit);
    }

    if (lit.IsLongValue()) {
        return LiteralIntegralValueToString<uint64_t>(lit);
    }

    if (lit.IsDoubleValue()) {
        std::stringstream res {};
        res << std::get<double>(lit.value);
        return res.str();
    }

    if (lit.IsFloatValue()) {
        std::stringstream res {};
        res << std::get<float>(lit.value);
        return res.str();
    }

    if (lit.IsStringValue()) {
        std::stringstream res {};
        res << "\"" << std::get<std::string>(lit.value) << "\"";
        return res.str();
    }

    UNREACHABLE();
}

void PandasmProgramDumper::DumpValues(const pandasm::LiteralArray &litArray, const bool isConst, std::ostream &os,
                                      const pandasm::Program &program) const
{
    std::string separator = (isConst) ? (" ") : ("\n");

    os << separator;

    if (isConst) {
        for (const auto &l : litArray.literals) {
            os << LiteralValueToString(l) << separator;
        }
    } else {
        bool skipTag = false;
        for (const auto &l : litArray.literals) {
            skipTag = !skipTag;
            if (skipTag) {
                continue;
            }
            os << "\t" << LiteralTagToString(l.tag, program) << " " << LiteralValueToString(l) << separator;
        }
    }
}

std::string PandasmProgramDumper::LiteralTagToString(const panda_file::LiteralTag &tag,
                                                     const pandasm::Program &program) const
{
    switch (tag) {
        case panda_file::LiteralTag::BOOL:
        case panda_file::LiteralTag::ARRAY_U1:
            return "u1";
        case panda_file::LiteralTag::ARRAY_U8:
            return "u8";
        case panda_file::LiteralTag::ARRAY_I8:
            return "i8";
        case panda_file::LiteralTag::ARRAY_U16:
            return "u16";
        case panda_file::LiteralTag::ARRAY_I16:
            return "i16";
        case panda_file::LiteralTag::ARRAY_U32:
            return "u32";
        case panda_file::LiteralTag::INTEGER:
        case panda_file::LiteralTag::ARRAY_I32:
            return "i32";
        case panda_file::LiteralTag::ARRAY_U64:
            return "u64";
        case panda_file::LiteralTag::BIGINT:
        case panda_file::LiteralTag::ARRAY_I64:
            return "i64";
        case panda_file::LiteralTag::FLOAT:
        case panda_file::LiteralTag::ARRAY_F32:
            return "f32";
        case panda_file::LiteralTag::DOUBLE:
        case panda_file::LiteralTag::ARRAY_F64:
            return "f64";
        case panda_file::LiteralTag::STRING:
        case panda_file::LiteralTag::ARRAY_STRING:
            return pandasm::Type::FromDescriptor(panda_file::GetStringClassDescriptor(program.lang)).GetPandasmName();
        case panda_file::LiteralTag::ACCESSOR:
            return "accessor";
        case panda_file::LiteralTag::NULLVALUE:
            return "nullvalue";
        case panda_file::LiteralTag::METHODAFFILIATE:
            return "method_affiliate";
        case panda_file::LiteralTag::METHOD:
            return "method";
        case panda_file::LiteralTag::GENERATORMETHOD:
            return "generator_method";
        default:
            LOG(ERROR, ABC2PROGRAM) << "Unsupported literal with tag 0x" << std::hex << static_cast<uint32_t>(tag);
            UNREACHABLE();
    }
}

}  // namespace ark::abc2program
