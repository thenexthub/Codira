/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef LIBARK_DEFECT_SCAN_AUX_INCLUDE_ABC_FILE_H
#define LIBARK_DEFECT_SCAN_AUX_INCLUDE_ABC_FILE_H

#include <list>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>
#include "compiler/optimizer/ir/graph.h"
#include "libpandabase/mem/arena_allocator.h"
#include "libpandafile/method_data_accessor.h"
#include "libpandafile/debug_info_extractor.h"
#include "graph.h"

namespace panda::defect_scan_aux {
class Class;
class Function;
class CalleeInfo;
class ModuleRecord;

enum class ResolveType {
    CLASS_OBJECT,
    CLASS_INSTANCE,
    FUNCTION_OBJECT,
    OBJECT_VARIABLE,
    UNRESOLVED_MODULE,
    UNRESOLVED_GLOBAL_VAR,
    UNRESOLVED_OTHER,
};
using FuncInstPair = std::pair<Function *, Inst>;
using ResolveResult = std::tuple<const void *, std::string, ResolveType>;

class AbcFile final {
public:
    static std::unique_ptr<const AbcFile> Open(std::string_view abc_filename);
    ~AbcFile();

    bool IsModule(std::string_view record_name = "") const;
    bool IsMergeAbc() const;
    const std::string &GetAbcFileName() const;
    const std::vector<std::shared_ptr<Class>> &GetClassList() const;
    size_t GetDefinedFunctionCount() const;
    size_t GetDefinedClassCount() const;
    const Function *GetDefinedFunctionByIndex(size_t index) const;
    const Function *GetFunctionByName(std::string_view func_name) const;
    const Function *GetExportFunctionByExportName(std::string_view export_func_name,
                                                  std::string_view record_name = "") const;
    const Class *GetDefinedClassByIndex(size_t index) const;
    const Class *GetClassByName(std::string_view class_name) const;
    const Class *GetExportClassByExportName(std::string_view export_class_name,
                                            std::string_view record_name = "") const;
    ssize_t GetLineNumberByInst(const Function *func, const Inst &inst) const;
    const std::set<std::string> GetFileRecordList() const;
    size_t GetFileRecordCount() const;

    // used for export stat
    std::string GetLocalNameByExportName(std::string_view export_name, std::string_view record_name = "") const;
    // used exclusively for indirect export stat
    std::string GetImportNameByExportName(std::string_view export_name, std::string_view record_name = "") const;
    std::string GetModuleNameByExportName(std::string_view export_name, std::string_view record_name = "") const;
    // used for import stat
    std::string GetModuleNameByLocalName(std::string_view local_name, std::string_view record_name = "") const;
    std::string GetImportNameByLocalName(std::string_view local_name, std::string_view record_name = "") const;
    // return a string without #xx# prefix
    std::string_view GetNameWithoutHashtag(std::string_view full_name, std::string_view record_name = "") const;
    std::string GetStringByInst(const Inst &inst) const;
    std::optional<FuncInstPair> GetStLexInstByLdLexInst(FuncInstPair func_inst_pair) const;
    std::optional<FuncInstPair> GetStGlobalInstByLdGlobalInst(FuncInstPair func_inst_pair) const;

private:
    static constexpr char MODULE_CLASS[] = "L_ESModuleRecord;";
    static constexpr char MODULE_IDX_FIELD_NAME[] = "moduleRecordIdx";
    static constexpr char ENTRY_FUNCTION_NAME[] = "func_main_0";
    static constexpr char PROTOTYPE[] = "prototype";
    static constexpr char CALL[] = "call";
    static constexpr char APPLY[] = "apply";
    static constexpr char DELIM = '.';
    static constexpr char EMPTY_STR[] = "";

    AbcFile(std::string_view filename, std::unique_ptr<const panda_file::File> &&panda_file);
    NO_COPY_SEMANTIC(AbcFile);
    NO_MOVE_SEMANTIC(AbcFile);

    void ExtractDebugInfo();
    void ExtractModuleInfo();
    void ExtractMergeAbcModuleInfo();
    void ExtractModuleRecord(panda_file::File::EntityId module_id, std::unique_ptr<ModuleRecord> &module_record);
    void InitializeAllDefinedFunction();
    void ExtractDefinedClassAndFunctionInfo();
    void ExtractMergedDefinedClassAndFunctionInfo();
    void ExtractSingleDefinedClassAndFunctionInfo();
    void ExtractClassAndFunctionInfo(Function *func);
    void ExtractClassInheritInfo(Function *func) const;
    void ExtractFunctionCalleeInfo(Function *func);
    void BuildFunctionDefineChain(Function *parent_func, Function *child_func) const;
    void BuildClassAndMemberFuncRelation(Class *clazz, Function *member_func) const;
    void ExtractClassAndFunctionExportList();
    void ExtractMergedClassAndFunctionExportList();
    void ExtractSingleClassAndFunctionExportList();
    void AddExportListForMerge(const Function *func_main, const Inst &inst);
    void AddExportListForSingle(const Function *func_main, const Inst &inst);
    void ExtractMergedClassAndFunctionInfo(Function *func);
    compiler::Graph *GenerateFunctionGraph(const panda_file::MethodDataAccessor &mda, std::string_view func_name);
    ResolveResult ResolveInstCommon(Function *func, Inst inst) const;
    ResolveResult HandleLdObjByNameInstResolveResult(const Inst &ldobjbyname_inst, const ResolveResult &resolve_res,
                                                     const std::string record_name = "") const;
    ResolveResult HandleNewObjInstResolveResultCommon(const ResolveResult &resolve_res) const;
    Function *ResolveDefineFuncInstCommon(const Function *func, const Inst &def_func_inst) const;
    std::unique_ptr<Class> ResolveDefineClassWithBufferInst(Function *func, const Inst &define_class_inst) const;
    void ResolveDefineMethodWithBufferInst(Function *func, const Inst &define_class_inst) const;
    Class *GetClassFromMemberFunctionName(const std::string &member_func_name) const;
    std::unique_ptr<CalleeInfo> ResolveCallInstCommon(Function *func, const Inst &call_inst,
                                                      uint32_t func_obj_idx = 0) const;
    std::unique_ptr<CalleeInfo> ResolveSuperCallInst(Function *func, const Inst &call_inst) const;
    void ResolveDefineMethodInst(Function *member_func, const Inst &define_method_inst);
    void HandleMemberFunctionFromClassBuf(const std::string &func_name, Function *def_func, Class *def_class) const;
    void AddDefinedClass(std::shared_ptr<Class> &&def_class);
    void AddDefinedFunction(std::shared_ptr<Function> &&def_func);
    void AddMergedDefinedFunction(std::shared_ptr<Function> &&def_func);
    void AddMergedDefinedClass(std::shared_ptr<Class> &&def_class, std::string record_name);
    void AddCalleeInfo(std::unique_ptr<CalleeInfo> &&callee_info);
    void AddModuleRecord(std::string record_name, std::unique_ptr<ModuleRecord> &&module_record);
    Function *GetFunctionByNameImpl(std::string_view func_name) const;
    Class *GetClassByNameImpl(std::string_view class_name) const;
    const ModuleRecord *GetModuleRecordByName(std::string record_name) const;
    std::string GetStringByMethodId(panda_file::File::EntityId method_id) const;
    std::string GetStringByStringId(panda_file::File::EntityId string_id) const;

    std::string filename_;
    bool is_merge_abc_ {false};
    std::unique_ptr<const panda_file::File> panda_file_ {nullptr};
    std::unique_ptr<const panda_file::DebugInfoExtractor> debug_info_ {nullptr};
    std::vector<const Class *> export_class_list_;
    std::vector<const Function *> export_func_list_;
    std::unordered_map<std::string, std::vector<const Class *>> merge_export_class_map_;
    std::unordered_map<std::string, std::vector<const Function *>> merge_export_func_map_;
    std::vector<std::shared_ptr<Function>> def_func_list_;
    std::vector<std::shared_ptr<Function>> merged_def_func_list_;
    std::unordered_map<std::string, Function *> def_func_map_;
    std::unordered_map<std::string, std::vector<std::shared_ptr<Function>>> merge_def_func_map_;
    std::unordered_map<std::string, ModuleRecord *> module_record_map_;
    std::vector<std::unique_ptr<const ModuleRecord>> module_record_list_;
    std::vector<std::shared_ptr<Class>> def_class_list_;
    std::vector<std::shared_ptr<Class>> merged_def_class_list_;
    std::unordered_map<std::string, Class *> def_class_map_;
    std::unordered_map<std::string, std::vector<std::shared_ptr<Class>>> merge_def_class_map_;
    std::list<std::unique_ptr<CalleeInfo>> callee_info_list_;
    std::set<std::string> record_name_set_;
    std::unique_ptr<ArenaAllocator> allocator_ {nullptr};
    std::unique_ptr<ArenaAllocator> local_allocator_ {nullptr};
};
}  // namespace panda::defect_scan_aux

#endif  // LIBARK_DEFECT_SCAN_AUX_INCLUDE_ABC_FILE_H