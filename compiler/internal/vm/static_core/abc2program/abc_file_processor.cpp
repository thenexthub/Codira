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

#include "abc_file_processor.h"
#include <iostream>
#include "abc_class_processor.h"
#include "abc2program_log.h"
#include "abc_file_utils.h"

#include "abc_literal_array_processor.h"
#include "abc_method_processor.h"
#include "get_language_specific_metadata.inc"
#include "mangling.h"

namespace ark::abc2program {

AbcFileProcessor::AbcFileProcessor(Abc2ProgramKeyData &keyData) : keyData_(keyData)
{
    file_ = &(keyData_.GetAbcFile());
    stringTable_ = &(keyData_.GetAbcStringTable());
    program_ = &(keyData_.GetProgram());
}

bool AbcFileProcessor::ProcessFile()
{
    ProcessClasses();
    FillLiteralArrays();
    FillProgramStrings();
    FillExternalFieldsToRecords();
    GetLanguageSpecificMetadata();
    program_->lang = keyData_.GetFileLanguage();
    return true;
}

void AbcFileProcessor::FillLiteralArrays()
{
    AbcLiteralArrayProcessor litArrayProcessor(file_->GetLiteralArraysId(), keyData_);
}

void AbcFileProcessor::ProcessClasses()
{
    const auto classes = file_->GetClasses();
    for (size_t i = 0; i < classes.size(); i++) {
        uint32_t classIdx = classes[i];
        auto classOff = file_->GetHeader()->classIdxOff + sizeof(uint32_t) * i;
        if (classIdx > file_->GetHeader()->fileSize) {
            LOG(FATAL, ABC2PROGRAM) << "> error encountered in record at " << classOff << " (0x" << std::hex << classOff
                                    << "). binary file corrupted. record offset (0x" << classIdx
                                    << ") out of bounds (0x" << file_->GetHeader()->fileSize << ")!";
            break;
        }
        panda_file::File::EntityId recordId(classIdx);
        auto language = GetRecordLanguage(recordId);
        if (language != keyData_.GetFileLanguage()) {
            if (keyData_.GetFileLanguage() == panda_file::SourceLang::PANDA_ASSEMBLY) {
                keyData_.SetFileLanguage(language);
            } else if (language != panda_file::SourceLang::PANDA_ASSEMBLY) {
                LOG(ERROR, ABC2PROGRAM) << "> possible error encountered in record at" << classOff << " (0x" << std::hex
                                        << classOff << "). record's language  ("
                                        << panda_file::LanguageToString(language) << ")  differs from file's language ("
                                        << panda_file::LanguageToString(keyData_.GetFileLanguage()) << ")!";
            }
        }
        AbcClassProcessor classProcessor(recordId, keyData_);
    }
}

void AbcFileProcessor::FillProgramStrings()
{
    program_->strings = stringTable_->GetStringSet();
}

void AbcFileProcessor::FillExternalFieldsToRecords()
{
    for (auto &[recordName, record] : program_->recordTable) {
        auto it = keyData_.GetExternalFieldTable().find(recordName);
        if (it == keyData_.GetExternalFieldTable().end()) {
            continue;
        }
        auto &[unused, fieldList] = *it;
        (void)unused;
        if (fieldList.empty()) {
            continue;
        }
        for (auto &fieldIter : fieldList) {
            if (!fieldIter.name.empty()) {
                record.fieldList.push_back(std::move(fieldIter));
            }
        }
        keyData_.GetExternalFieldTable().erase(recordName);
    }
}

std::string AbcFileProcessor::AnnotationTagToString(const char tag) const
{
    static const std::unordered_map<char, std::string> TAG_TO_STRING = {{'1', "u1"},
                                                                        {'2', "i8"},
                                                                        {'3', "u8"},
                                                                        {'4', "i16"},
                                                                        {'5', "u16"},
                                                                        {'6', "i32"},
                                                                        {'7', "u32"},
                                                                        {'8', "i64"},
                                                                        {'9', "u64"},
                                                                        {'A', "f32"},
                                                                        {'B', "f64"},
                                                                        {'C', "string"},
                                                                        {'D', "record"},
                                                                        {'E', "method"},
                                                                        {'F', "enum"},
                                                                        {'G', "annotation"},
                                                                        {'J', "method_handle"},
                                                                        {'H', "array"},
                                                                        {'K', "u1[]"},
                                                                        {'L', "i8[]"},
                                                                        {'M', "u8[]"},
                                                                        {'N', "i16[]"},
                                                                        {'O', "u16[]"},
                                                                        {'P', "i32[]"},
                                                                        {'Q', "u32[]"},
                                                                        {'R', "i64[]"},
                                                                        {'S', "u64[]"},
                                                                        {'T', "f32[]"},
                                                                        {'U', "f64[]"},
                                                                        {'V', "string[]"},
                                                                        {'W', "record[]"},
                                                                        {'X', "method[]"},
                                                                        {'Y', "enum[]"},
                                                                        {'Z', "annotation[]"},
                                                                        {'@', "method_handle[]"},
                                                                        {'*', "nullptr_string"}};

    return TAG_TO_STRING.at(tag);
}

std::string AbcFileProcessor::ScalarValueToString(const panda_file::ScalarValue &value, const std::string &type)
{
    std::stringstream ss;

    if (type == "i8") {
        auto res = value.Get<int8_t>();
        ss << static_cast<int>(res);
    } else if (type == "u1" || type == "u8") {
        auto res = value.Get<uint8_t>();
        ss << static_cast<unsigned int>(res);
    } else if (type == "i16") {
        ss << value.Get<int16_t>();
    } else if (type == "u16") {
        ss << value.Get<uint16_t>();
    } else if (type == "i32") {
        ss << value.Get<int32_t>();
    } else if (type == "u32") {
        ss << value.Get<uint32_t>();
    } else if (type == "i64") {
        ss << value.Get<int64_t>();
    } else if (type == "u64") {
        ss << value.Get<uint64_t>();
    } else if (type == "f32") {
        ss << value.Get<float>();
    } else if (type == "f64") {
        ss << value.Get<double>();
    } else if (type == "string") {
        const auto id = value.Get<panda_file::File::EntityId>();
        ss << "\"" << stringTable_->GetStringById(id) << "\"";
    } else if (type == "record") {
        const auto id = value.Get<panda_file::File::EntityId>();
        ss << keyData_.GetFullRecordNameById(id);
    } else if (type == "method") {
        const auto id = value.Get<panda_file::File::EntityId>();
        AbcMethodProcessor methodProcessor(id, keyData_);
        ss << methodProcessor.GetMethodSignature();
    } else if (type == "enum") {
        const auto id = value.Get<panda_file::File::EntityId>();
        panda_file::FieldDataAccessor fieldAccessor(*file_, id);
        ss << keyData_.GetFullRecordNameById(fieldAccessor.GetClassId()) << "."
           << stringTable_->GetStringById(fieldAccessor.GetNameId());
    } else if (type == "annotation") {
        const auto id = value.Get<panda_file::File::EntityId>();
        ss << "id_" << id;
    } else if (type == "void") {
        return std::string();
    } else if (type == "method_handle") {
    } else if (type == "nullptr_string") {
        ss << static_cast<uint32_t>(0);
    }

    return ss.str();
}

std::string AbcFileProcessor::ArrayValueToString(const panda_file::ArrayValue &value, const std::string &type,
                                                 const size_t idx)
{
    std::stringstream ss;

    if (type == "i8") {
        auto res = value.Get<int8_t>(idx);
        ss << (static_cast<int>(res));
    } else if (type == "u1" || type == "u8") {
        auto res = value.Get<uint8_t>(idx);
        ss << (static_cast<unsigned int>(res));
    } else if (type == "i16") {
        ss << (value.Get<int16_t>(idx));
    } else if (type == "u16") {
        ss << (value.Get<uint16_t>(idx));
    } else if (type == "i32") {
        ss << (value.Get<int32_t>(idx));
    } else if (type == "u32") {
        ss << (value.Get<uint32_t>(idx));
    } else if (type == "i64") {
        ss << (value.Get<int64_t>(idx));
    } else if (type == "u64") {
        ss << (value.Get<uint64_t>(idx));
    } else if (type == "f32") {
        ss << (value.Get<float>(idx));
    } else if (type == "f64") {
        ss << (value.Get<double>(idx));
    } else if (type == "string") {
        const auto id = value.Get<panda_file::File::EntityId>(idx);
        ss << ('\"') << (stringTable_->GetStringById(id)) << ('\"');
    } else if (type == "record") {
        const auto id = value.Get<panda_file::File::EntityId>(idx);
        ss << (keyData_.GetFullRecordNameById(id));
    } else if (type == "method") {
        const auto id = value.Get<panda_file::File::EntityId>(idx);
        AbcMethodProcessor methodProcessor(id, keyData_);
        ss << (methodProcessor.GetMethodSignature());
    } else if (type == "enum") {
        const auto id = value.Get<panda_file::File::EntityId>(idx);
        panda_file::FieldDataAccessor fieldAccessor(*file_, id);
        ss << keyData_.GetFullRecordNameById(fieldAccessor.GetClassId()) << "."
           << stringTable_->GetStringById(fieldAccessor.GetNameId());
    } else if (type == "annotation") {
        const auto id = value.Get<panda_file::File::EntityId>(idx);
        ss << ("id_") << (id);
    } else if (type == "method_handle") {
    } else if (type == "nullptr_string") {
    }

    return ss.str();
}

ark::panda_file::SourceLang AbcFileProcessor::GetRecordLanguage(panda_file::File::EntityId classId) const
{
    if (file_->IsExternal(classId)) {
        return ark::panda_file::SourceLang::PANDA_ASSEMBLY;
    }

    panda_file::ClassDataAccessor cda(*file_, classId);
    return cda.GetSourceLang().value_or(panda_file::SourceLang::PANDA_ASSEMBLY);
}

}  // namespace ark::abc2program
