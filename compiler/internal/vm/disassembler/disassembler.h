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

#ifndef DISASSEMBLER_DISASSEMBLER_H
#define DISASSEMBLER_DISASSEMBLER_H

#include "macros.h"
#include "utils/type_helpers.h"
#include "utils/span.h"

#include "class_data_accessor-inl.h"
#include "code_data_accessor-inl.h"
#include "debug_data_accessor-inl.h"
#include "debug_info_extractor.h"
#include "field_data_accessor-inl.h"
#include "method_data_accessor-inl.h"
#include "literal_data_accessor-inl.h"
#include "libpandafile/module_data_accessor-inl.h"
#include "param_annotations_data_accessor.h"
#include "annotation_data_accessor.h"
#include "proto_data_accessor-inl.h"
#include "file-inl.h"
#include "file.h"
#include "os/file.h"

#include "assembly-program.h"
#include "assembly-ins.h"

#include "bytecode_instruction-inl.h"
#include "libpandabase/macros.h"

#include <map>
#include <memory>
#include <string>

#include "accumulators.h"

namespace panda::disasm {
class Disassembler {
public:
    NO_COPY_SEMANTIC(Disassembler);
    DEFAULT_MOVE_SEMANTIC(Disassembler);

    Disassembler() = default;
    ~Disassembler() = default;

    void Disassemble(const std::string &filename_in, const bool quiet = false, const bool skip_strings = false);
    void CollectInfo();
    void Serialize(std::ostream &os, bool add_separators = false, bool print_information = false) const;

    static inline bool IsPandasmFriendly(const char c);
    static inline bool IsSystemType(const std::string &type_name);

    void GetRecord(pandasm::Record *record, const panda_file::File::EntityId &record_id);
    void AddMethodToTables(const panda_file::File::EntityId &method_id);
    void GetMethod(pandasm::Function *method, const panda_file::File::EntityId &method_id);
    void GetLiteralArray(pandasm::LiteralArray *lit_array, size_t index) const;
    std::optional<std::vector<std::string>> GetAnnotationByMethodName(const std::string &method_name) const;
    std::optional<std::string> GetSerializedRecord(const std::string &record_name) const;
    std::optional<std::string> GetSerializedMethodAnnotation(const std::string &method_name,
                                                                           const std::string &anno_name) const;
    std::vector<std::string> GetStrings() const;
    std::vector<std::string> GetModuleLiterals() const;
    template <typename T>
    void FillLiteralArrayData(pandasm::LiteralArray *lit_array, const panda_file::LiteralTag &tag,
                              const panda_file::LiteralDataAccessor::LiteralValue &value) const;
    pandasm::Ins *BytecodeInstructionToPandasmInstruction(BytecodeInstruction bc_ins,
                                                         panda_file::File::EntityId method_id) const;

    const ProgInfo &GetProgInfo() const
    {
        return prog_info_;
    }

    std::vector<size_t> GetColumnNumber();
    std::vector<size_t> GetLineNumber();

private:
    void GetLiteralArrays();
    void FillLiteralArrayTable(panda_file::File::EntityId &id, size_t index);
    void FillLiteralData(pandasm::LiteralArray *lit_array, const panda_file::LiteralDataAccessor::LiteralValue &value,
                         const panda_file::LiteralTag &tag) const;
    void GetRecords();
    void GetFields(pandasm::Record *record, const panda_file::File::EntityId &record_id);

    void GetMethods(const panda_file::File::EntityId &record_id);
    void GetAnnotationElements(pandasm::Function &method, const panda_file::AnnotationDataAccessor &ada,
                               const std::string &annotation_name);
    void GetMethodAnnotations(pandasm::Function &method, const panda_file::File::EntityId &method_id);
    void CreateAnnotationElement(panda_file::AnnotationDataAccessor &ada, pandasm::Function &method,
                                 const std::string &ann_name, const std::string &ann_elem_name,
                                 const std::string &ann_elem_index);
    template <typename T, typename U>
    void AddAnnotationElement(pandasm::Function &method, const std::string &annotation_name, const std::string &key,
                              const U &value);
    void GetParams(pandasm::Function *method, const panda_file::File::EntityId &code_id) const;
    IdList GetInstructions(pandasm::Function *method, panda_file::File::EntityId method_id,
                           panda_file::File::EntityId code_id) const;
    LabelTable GetExceptions(pandasm::Function *method, panda_file::File::EntityId method_id,
                             panda_file::File::EntityId code_id) const;
    bool LocateTryBlock(const BytecodeInstruction &bc_ins, const BytecodeInstruction &bc_ins_last,
                        const panda_file::CodeDataAccessor::TryBlock &try_block,
                        pandasm::Function::CatchBlock *catch_block_pa, LabelTable *label_table, size_t try_idx) const;
    bool LocateCatchBlock(const BytecodeInstruction &bc_ins, const BytecodeInstruction &bc_ins_last,
                          const panda_file::CodeDataAccessor::CatchBlock &catch_block,
                          pandasm::Function::CatchBlock *catch_block_pa, LabelTable *label_table, size_t try_idx,
                          size_t catch_idx) const;

    void GetMetaData(pandasm::Record *record, const panda_file::File::EntityId &record_id) const;
    void GetMetaData(pandasm::Function *method, const panda_file::File::EntityId &method_id) const;
    void GetMetaData(pandasm::Field *field, const panda_file::File::EntityId &field_id, bool is_scope_names_record);
    void GetMetadataFieldValue(panda_file::FieldDataAccessor &field_accessor, pandasm::Field *field,
                               bool isScopeNamesRecord);

    void GetLanguageSpecificMetadata();

    std::string AnnotationTagToString(const char tag) const;

    std::string ScalarValueToString(const panda_file::ScalarValue &value, const std::string &type);
    std::string ArrayValueToString(const panda_file::ArrayValue &value, const std::string &type, const size_t idx);

    std::string GetFullMethodName(const panda_file::File::EntityId &method_id) const;
    std::string GetMethodSignature(const panda_file::File::EntityId &method_id) const;
    std::string GetFullRecordName(const panda_file::File::EntityId &class_id) const;

    void GetRecordInfo(const panda_file::File::EntityId &record_id, RecordInfo *record_info) const;
    void GetMethodInfo(const panda_file::File::EntityId &method_id, MethodInfo *method_info) const;
    void GetInsInfo(const panda_file::File::EntityId &code_id, MethodInfo *method_info) const;

    template <typename T>
    void SerializeValues(const pandasm::LiteralArray &lit_array, T &os) const;
    std::string SerializeLiteralArray(const pandasm::LiteralArray &lit_array) const;
    void Serialize(const std::string &key, const pandasm::LiteralArray &lit_array, std::ostream &os) const;
    void Serialize(const std::string &module_offset, const std::vector<std::string> &module_array,
                   std::ostream &os) const;
    template <typename T>
    void SerializeLiterals(const pandasm::LiteralArray &lit_array, T &os) const;
    std::string LiteralTagToString(const panda_file::LiteralTag &tag) const;
    void Serialize(const pandasm::Record &record, std::ostream &os, bool print_information = false) const;
    void SerializeFields(const pandasm::Record &record, std::ostream &os, bool print_information) const;
    void Serialize(const pandasm::Function &method, std::ostream &os, bool print_information = false) const;
    void SerializeInstructions(const pandasm::Function &method, std::ostream &os,
                               const std::map<std::string, MethodInfo>::const_iterator &method_info_it,
                               bool print_method_info = false) const;
    void SerializeMethodAnnotations(const pandasm::Function &method, std::ostream &os) const;
    void SerializeMethodAnnotation(const pandasm::AnnotationData &anno, std::ostream &os) const;
    void SerializeAnnotationElement(const std::vector<pandasm::AnnotationElement> &elements, std::stringstream &ss,
                                    uint32_t idx) const;
    void SerializeStrings(const panda_file::File::EntityId &offset, const std::string &name_value,
                          std::ostream &os) const;
    void Serialize(const pandasm::Function::CatchBlock &catch_block, std::ostream &os) const;
    void Serialize(const pandasm::ItemMetadata &meta, const AnnotationList &ann_list, std::ostream &os) const;
    void SerializeLineNumberTable(const panda_file::LineNumberTable &line_number_table, std::ostream &os) const;
    void SerializeColumnNumberTable(const panda_file::ColumnNumberTable &column_number_table, std::ostream &os) const;
    void SerializeLocalVariableTable(const panda_file::LocalVariableTable &local_variable_table,
                                     const pandasm::Function &method, std::ostream &os) const;
    bool IsModuleLiteralOffset(const panda_file::File::EntityId &id) const;

    pandasm::Type PFTypeToPandasmType(const panda_file::Type &type, panda_file::ProtoDataAccessor &pda,
                                      size_t &ref_idx) const;

    pandasm::Type FieldTypeToPandasmType(const uint32_t &type) const;

    static inline std::string StringDataToString(panda_file::File::StringData sd)
    {
        return std::string(utf::Mutf8AsCString(sd.data));
    }

    pandasm::Opcode BytecodeOpcodeToPandasmOpcode(BytecodeInstruction::Opcode o) const;
    pandasm::Opcode BytecodeOpcodeToPandasmOpcode(uint8_t o) const;

    std::string IDToString(BytecodeInstruction bc_ins, panda_file::File::EntityId method_id, size_t idx) const;

    panda::panda_file::SourceLang GetRecordLanguage(panda_file::File::EntityId class_id) const;
    panda::panda_file::SourceLang GetMethodLanguage(panda_file::File::EntityId class_id) const;
    void GetLiteralArrayByOffset(pandasm::LiteralArray *lit_array, panda_file::File::EntityId offset) const;

    std::vector<std::string> GetModuleLiteralArray(panda_file::File::EntityId &module_id) const;
    std::string SerializeModuleLiteralArray(const std::vector<std::string> &module_array) const;
    std::string ModuleTagToString(panda_file::ModuleTag &tag) const;

    std::string getLiteralArrayTypeFromValue(const pandasm::LiteralArray &literal_array) const;
    void DumpLiteralArray(const pandasm::LiteralArray &literal_array, std::stringstream &ss) const;
    void SerializeFieldValue(const pandasm::Field &f, std::stringstream &ss) const;

    std::unique_ptr<const panda_file::File> file_;
    pandasm::Program prog_;

    inline std::string GetStringByOffset(uint32_t offset) const
    {
        const auto sd = file_->GetStringData(panda_file::File::EntityId(offset));
        return std::string(utf::Mutf8AsCString(sd.data));
    }

    inline bool IsValidOffset(uint32_t offset) const
    {
        return panda_file::File::EntityId(offset).IsValid() && offset < file_->GetHeader()->file_size;
    }

    inline std::string GetFileNameByPath(const std::string &file_path) const
    {
        size_t pos = file_path.find_last_of(panda::os::file::File::GetPathDelim());
        if (pos == std::string::npos) {
            return file_path;
        }

        return file_path.substr(pos + 1);
    }

    std::map<std::string, panda_file::File::EntityId> record_name_to_id_;
    std::map<std::string, panda_file::File::EntityId> method_name_to_id_;
    std::map<std::string, std::vector<std::string>> modulearray_table_;
    mutable std::map<panda_file::File::EntityId, std::string> string_offset_to_name_;

    ProgAnnotations prog_ann_;

    ProgInfo prog_info_;

    std::unique_ptr<panda_file::DebugInfoExtractor> debug_info_extractor_;

    bool quiet_;
    bool skip_strings_;
    std::unordered_set<uint32_t> module_literals_;
    std::unordered_set<uint32_t> module_request_phase_literals_;
#include "disasm_plugins.inc"
};
}  // namespace panda::disasm

#endif  // DISASSEMBLER_DISASSEMBLER_H
