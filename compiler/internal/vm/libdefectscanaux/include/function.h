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

#ifndef LIBARK_DEFECT_SCAN_AUX_INCLUDE_FUNCTION_H
#define LIBARK_DEFECT_SCAN_AUX_INCLUDE_FUNCTION_H

#include <string>
#include <vector>
#include "libpandabase/macros.h"
#include "libpandafile/file.h"
#include "graph.h"

namespace panda::defect_scan_aux {
class AbcFile;
class Class;
class Function;
class CalleeInfo;

class Function final {
public:
    Function(std::string_view record_name, std::string_view func_name, panda_file::File::EntityId m_id,
             uint32_t arg_count, const Graph &graph, const AbcFile *abc_file)
        : func_name_(func_name), record_name_(record_name), m_id_(m_id),
          arg_count_(arg_count), graph_(graph), abc_file_(abc_file)
    {
    }
    ~Function() = default;
    NO_COPY_SEMANTIC(Function);
    NO_MOVE_SEMANTIC(Function);

    const std::string &GetRecordName() const;
    const std::string &GetFunctionName() const;
    const AbcFile *GetAbcFileInstance() const;
    const Graph &GetGraph() const;
    const Class *GetClass() const;
    Function *GetParentFunction() const;
    uint32_t GetArgCount() const;
    size_t GetDefinedClassCount() const;
    size_t GetDefinedFunctionCount() const;
    size_t GetCalleeInfoCount() const;
    const Class *GetDefinedClassByIndex(size_t index) const;
    const Function *GetDefinedFunctionByIndex(size_t index) const;
    const CalleeInfo *GetCalleeInfoByIndex(size_t index) const;
    std::vector<Inst> GetReturnInstList() const;
    const CalleeInfo *GetCalleeInfoByCallInst(const Inst &call_inst) const;
    uint32_t GetAndUpdateToVisitInputForInst(const Inst &inst);

private:
    panda_file::File::EntityId GetMethodId() const;
    void SetParentFunction(Function *parent_func);
    void SetClass(const Class *clazz);
    void AddDefinedClass(const Class *def_class);
    void AddDefinedFunction(const Function *def_func);
    void AddCalleeInfo(const CalleeInfo *callee_info);

    const std::string func_name_;
    const std::string record_name_;
    panda_file::File::EntityId m_id_;
    uint32_t arg_count_;
    const Graph graph_;
    const AbcFile *abc_file_ {nullptr};
    // the class which current function belongs to
    const Class *class_ {nullptr};
    // the function where current function is defined
    Function *parent_func_ {nullptr};
    std::vector<const Class *> def_class_list_;
    std::vector<const Function *> def_func_list_;
    std::vector<const CalleeInfo *> callee_info_list_;
    std::unordered_map<uint32_t, uint32_t> to_visit_inputs_;

    friend class AbcFile;
};
}  // namespace panda::defect_scan_aux

#endif  // LIBARK_DEFECT_SCAN_AUX_INCLUDE_FUNCTION_H
