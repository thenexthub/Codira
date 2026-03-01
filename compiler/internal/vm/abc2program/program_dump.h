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

#ifndef ABC2PROGRAM_PROGRANM_DUMPER_PROGRAM_DUMP_H
#define ABC2PROGRAM_PROGRANM_DUMPER_PROGRAM_DUMP_H

#include <iostream>
#include <assembly-program.h>
#include "dump_utils.h"

namespace panda::abc2program {

class PandasmProgramDumper {
public:
    PandasmProgramDumper() {}
    PandasmProgramDumper(bool is_normalized, bool is_debug) : is_normalized_(is_normalized), is_debug_(is_debug) {}
    void Dump(std::ostream &os, const pandasm::Program &program);
    void SetAbcFilePath(const std::string &abc_file_path);

private:
    void DumpAbcFilePath(std::ostream &os) const;
    void DumpLanguage(std::ostream &os, const panda::panda_file::SourceLang lang) const;
    void DumpProgramLanguage(std::ostream &os) const;
    void DumpLiteralArrayTable(std::ostream &os) const;
    void DumpRecordTable(std::ostream &os) const;
    void DumpRecord(std::ostream &os, const pandasm::Record &record) const;
    void DumpRecordLanguage(std::ostream &os, const pandasm::Record &record) const;
    bool DumpRecordMetaData(std::ostream &os, const pandasm::Record &record) const;
    void DumpFieldList(std::ostream &os, const pandasm::Record &record) const;
    void DumpField(std::ostream &os, const pandasm::Field &field) const;
    void DumpFieldMetaData(std::ostream &os, const pandasm::Field &field) const;
    void DumpRecordSourceFile(std::ostream &os, const pandasm::Record &record) const;
    void DumpFunctionTable(std::ostream &os);
    void DumpFunction(std::ostream &os, const pandasm::Function &function);
    void DumpFunctionLanguage(std::ostream &os, const pandasm::Function &function) const;
    void DumpFunctionKind(std::ostream &os, const pandasm::Function &function) const;
    void DumpFunctionAnnotations(std::ostream &os, const pandasm::Function &function) const;
    void DumpFunctionHead(std::ostream &os, const pandasm::Function &function) const;
    void DumpFunctionReturnType(std::ostream &os, const pandasm::Function &function) const;
    void DumpFunctionName(std::ostream &os, const pandasm::Function &function) const;
    void DumpFunctionParams(std::ostream &os, const pandasm::Function &function) const;
    void DumpFunctionParamAtIndex(std::ostream &os, const pandasm::Function::Parameter &param, size_t idx) const;
    void DumpFunctionAttributes(std::ostream &os, const pandasm::Function &function) const;
    void DumpFunctionBody(std::ostream &os, const pandasm::Function &function);
    void DumpFunctionIns(std::ostream &os, const pandasm::Function &function);
    void DumpOriginalFunctionIns(std::ostream &os, const pandasm::Function &function);
    void DumpNormalizedFunctionIns(std::ostream &os, const pandasm::Function &function);
    void DumpFunctionDebugInfo(std::ostream &os, const pandasm::Function &function);
    void UpdateLocalVarMap(const pandasm::Function &function,
        std::map<int32_t, panda::pandasm::debuginfo::LocalVariable>& local_variable_table);
    void DumpAnnotationData(std::ostream &os, const pandasm::AnnotationData &anno) const;
    void DumpArrayValue(std::ostream &os, const pandasm::ArrayValue &array) const;
    void DumpScalarValue(std::ostream &os, const pandasm::ScalarValue &scalar) const;
    void GetFinalDumpIns(const pandasm::Function &function);
    void UpdateLabels4DumpIns(std::vector<pandasm::InsPtr> &dump_ins, const LabelMap &label_map) const;
    void UpdateLabels4DumpInsAtIndex(size_t idx, std::vector<pandasm::InsPtr> &dump_ins,
                                     const LabelMap &label_map) const;
    std::string GetMappedLabel(const std::string &label, const LabelMap &label_map) const;
    void DumpFinalIns(std::ostream &os);
    void DumpFunctionCatchBlocks(std::ostream &os, const pandasm::Function &function) const;
    void DumpOriginalFunctionCatchBlocks(std::ostream &os, const pandasm::Function &function) const;
    void DumpNormalizedFunctionCatchBlocks(std::ostream &os, const pandasm::Function &function) const;
    void DumpCatchBlock(std::ostream &os, const pandasm::Function::CatchBlock &catch_block) const;
    void UpdateCatchBlock(pandasm::Function::CatchBlock &catch_block) const;
    std::string GetUpdatedCatchBlockLabel(const std::string &orignal_label) const;
    void ReplaceLiteralId4Ins(pandasm::InsPtr &pa_ins) const;
    void DumpStrings(std::ostream &os) const;
    std::string SerializeLiteralArray(const pandasm::LiteralArray &lit_array, uint32_t id) const;
    void SerializeLiterals(const pandasm::LiteralArray &lit_array, std::stringstream &os) const;
    void SerializeLiteralsAtIndex(const pandasm::LiteralArray &lit_array, std::stringstream &os, size_t i) const;
    void SerializeNestedLiteralArrayById(std::stringstream &os, const std::string &literal_array_id_name) const;
    // True when the option 'dump-normalized-asm-program' is enabled. See option description for details
    bool is_normalized_ = false;
    // True when the option 'debug-info' is enabled. When both it and is_normalized_ are true, skip dump
    // of function annotation and the record with name '_ESSlotNumberAnnotation'
    bool is_debug_ = false;
    std::string abc_file_path_;
    std::vector<pandasm::InsPtr> final_dump_ins_ptrs_;
    std::unordered_map<uint32_t, uint32_t> original_to_final_index_map_;
    LabelMap label_map_;
    const pandasm::Program *program_ = nullptr;
    size_t regs_num_ = 0;
    mutable std::unordered_set<uint32_t> processing_literal_array_id_set_;
};

}  // namespace panda::abc2program

#endif  // ABC2PROGRAM_PROGRANM_DUMPER_PROGRAM_DUMP_H
