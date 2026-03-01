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

#include "function.h"
#include "libpandafile/method_data_accessor.h"
#include "abc_file.h"
#include "callee_info.h"

namespace panda::defect_scan_aux {
const std::string &Function::GetRecordName() const
{
    return record_name_;
}

const std::string &Function::GetFunctionName() const
{
    return func_name_;
}

const AbcFile *Function::GetAbcFileInstance() const
{
    return abc_file_;
}

const Graph &Function::GetGraph() const
{
    return graph_;
}

const Class *Function::GetClass() const
{
    return class_;
}

Function *Function::GetParentFunction() const
{
    return parent_func_;
}

uint32_t Function::GetArgCount() const
{
    return arg_count_;
}

size_t Function::GetDefinedClassCount() const
{
    return def_class_list_.size();
}

size_t Function::GetDefinedFunctionCount() const
{
    return def_func_list_.size();
}

size_t Function::GetCalleeInfoCount() const
{
    return callee_info_list_.size();
}

const Class *Function::GetDefinedClassByIndex(size_t index) const
{
    ASSERT(index < def_class_list_.size());
    return def_class_list_[index];
}

const Function *Function::GetDefinedFunctionByIndex(size_t index) const
{
    ASSERT(index < def_func_list_.size());
    return def_func_list_[index];
}

const CalleeInfo *Function::GetCalleeInfoByIndex(size_t index) const
{
    ASSERT(index < callee_info_list_.size());
    return callee_info_list_[index];
}

std::vector<Inst> Function::GetReturnInstList() const
{
    std::vector<Inst> ret_inst_list;
    graph_.VisitAllInstructions([&](const Inst &inst) {
        InstType type = inst.GetType();
        if (type == InstType::RETURN) {
            ret_inst_list.push_back(inst);
        }
    });
    return ret_inst_list;
}

const CalleeInfo *Function::GetCalleeInfoByCallInst(const Inst &call_inst) const
{
    for (auto callee_info : callee_info_list_) {
        auto &cur_call_inst = callee_info->GetCallInst();
        if (cur_call_inst == call_inst) {
            return callee_info;
        }
    }
    return nullptr;
}

uint32_t Function::GetAndUpdateToVisitInputForInst(const Inst &inst)
{
    auto inst_input_size = inst.GetInputInsts().size();
    ASSERT(inst_input_size != 0);
    uint32_t inst_id = inst.GetInstId();
    auto iter = to_visit_inputs_.find(inst_id);
    uint32_t to_visit_input = iter == to_visit_inputs_.end() ? 0 : iter->second;
    uint32_t next_to_visit_input = to_visit_input + 1;
    if (next_to_visit_input >= inst_input_size)
        next_to_visit_input = 0;
    to_visit_inputs_[inst_id] = next_to_visit_input;
    return to_visit_input;
}

panda_file::File::EntityId Function::GetMethodId() const
{
    return m_id_;
}

void Function::SetParentFunction(Function *parent_func)
{
    ASSERT(parent_func != nullptr);
    parent_func_ = parent_func;
}

void Function::SetClass(const Class *clazz)
{
    ASSERT(clazz != nullptr);
    class_ = clazz;
}

void Function::AddDefinedClass(const Class *def_class)
{
    ASSERT(def_class != nullptr);
    def_class_list_.push_back(def_class);
}

void Function::AddDefinedFunction(const Function *def_func)
{
    ASSERT(def_func != nullptr);
    def_func_list_.push_back(def_func);
}

void Function::AddCalleeInfo(const CalleeInfo *callee_info)
{
    ASSERT(callee_info != nullptr);
    callee_info_list_.push_back(callee_info);
}
}  // namespace panda::defect_scan_aux