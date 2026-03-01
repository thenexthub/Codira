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

#ifndef BYTECODE_OPTIMIZER_BYTECODE_ANALYSIS_RESULT_H
#define BYTECODE_OPTIMIZER_BYTECODE_ANALYSIS_RESULT_H

#include <mutex>
#include "assembler/assembly-emitter.h"
#include "bytecode_optimizer/module_constant_analyzer.h"
#include "bytecode_optimizer/constant_propagation/constant_value.h"

namespace panda::bytecodeopt {

class BytecodeAnalysisResult {
public:
    explicit BytecodeAnalysisResult() = default;

    const std::unordered_set<uint32_t> &GetConstantLocalExportSlots()
    {
        return constant_local_export_slots_;
    }

    void SetModuleConstantAnalysisResult(const ModuleConstantAnalysisResult &result);
    void SetConstantLocalExportSlots(const std::unordered_set<uint32_t> &slots);
    void SetLocalExportInfo(uint32_t slot, const std::string &external_name);
    void SetRegularImportInfo(uint32_t slot, const std::string &import_name, const std::string &source_record);
    void SetNamespaceImportInfo(uint32_t slot, const std::string &source_record);
    void Dump(std::ostream &os);

private:
    using LocalExportInfo = std::vector<std::string>;
    struct RegularImportInfo {
        std::string import_name;
        std::string source_record_name;
    };

    bool GetExportedConstantValue(const std::string &name, ConstantValue &value) const;
    bool GetLocalExportInfo(uint32_t slot, uint32_t name_idx, std::string &external_name) const;
    bool GetRegularImportInfo(uint32_t slot, std::string &import_name, std::string &source_record) const;
    bool GetNamespaceImportInfo(uint32_t slot, std::string &source_record) const;

    std::unordered_map<std::string, ConstantValue> constant_local_export_values_;
    std::unordered_set<uint32_t> constant_local_export_slots_;
    std::vector<LocalExportInfo> local_export_slot_external_names_;
    std::unordered_map<uint32_t, RegularImportInfo> regular_import_slot_infos_;
    std::unordered_map<uint32_t, std::string> namespace_import_slot_source_record_names_;

    friend class BytecodeAnalysisResults;
};

using BytecodeAnalysisResultMap = std::unordered_map<std::string, std::unique_ptr<BytecodeAnalysisResult>>;
using BytecodeMapsMap =
        std::unordered_map<std::string, std::unique_ptr<pandasm::AsmEmitter::PandaFileToPandaAsmMaps>>;

class BytecodeAnalysisResults {
public:
    static pandasm::AsmEmitter::PandaFileToPandaAsmMaps &GetOrCreateBytecodeMaps(const std::string &filename,
                                                                                 bool &exists);
    static void DeleteBytecodeMaps(const std::string &filename);

    static BytecodeAnalysisResult &GetOrCreateBytecodeAnalysisResult(const std::string &recordname, bool &exists);

    static void Clear();

    // Following methods should only be called when no writer modifying analysis results exists
    static bool GetLocalExportConstForRecord(const std::string &recordname, uint32_t local_export_slot,
                                             ConstantValue &value);
    static bool GetRegularImportConstForRecord(const std::string &recordname, uint32_t regular_import_slot,
                                               ConstantValue &value);
    static bool GetModuleNamespaceConstForRecord(const std::string &recordname, uint32_t module_namespace_slot,
                                                 const std::string &property_name, ConstantValue &value);
private:
    static BytecodeAnalysisResultMap analysis_results_;
    static BytecodeMapsMap bytecode_maps_;
    static std::mutex mutex_;

    template<typename T>
    static T& GetOrCreateElementInMap(std::unordered_map<std::string, std::unique_ptr<T>> &map,
                                      const std::string &name, bool &exists)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        auto iter = map.find(name);
        if (iter == map.end()) {
            exists = false;
            auto new_element = std::make_unique<T>();
            auto ret = map.emplace(name, std::move(new_element));
            ASSERT(ret.second == true);
            return *ret.first->second;
        }
        return *(iter->second);
    }
};

}  // namespace panda::bytecodeopt

#endif  // BYTECODE_OPTIMIZER_BYTECODE_ANALYSIS_RESULT_H
