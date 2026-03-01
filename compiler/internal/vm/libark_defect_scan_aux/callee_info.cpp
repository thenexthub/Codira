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

#include "callee_info.h"
#include "function.h"
#include "class.h"

namespace panda::defect_scan_aux {
bool CalleeInfo::IsCalleeDefinite() const
{
    return is_definite_;
}

int CalleeInfo::GetCalleeArgCount() const
{
    return arg_count_;
}

const Inst &CalleeInfo::GetCallInst() const
{
    return call_inst_;
}

const Function *CalleeInfo::GetCaller() const
{
    return caller_;
}

const Class *CalleeInfo::GetClass() const
{
    return class_;
}

const Function *CalleeInfo::GetCallee() const
{
    return func_;
}

const std::string &CalleeInfo::GetFunctionName() const
{
    return func_name_;
}

const std::string &CalleeInfo::GetClassName() const
{
    return class_name_;
}

const std::string &CalleeInfo::GetExternalModuleName() const
{
    return external_module_name_;
}

const std::string &CalleeInfo::GetGlobalVarName() const
{
    return global_var_name_;
}

void CalleeInfo::SetCalleeArgCount(int arg_count)
{
    arg_count_ = arg_count;
}

void CalleeInfo::SetClass(const Class *clazz)
{
    class_ = clazz;
}

void CalleeInfo::SetCallee(const Function *func)
{
    ASSERT(func != nullptr);
    func_ = func;
    is_definite_ = true;
    SetCalleeArgCount(static_cast<int>(func->GetArgCount()));
    SetFunctionName(func->GetFunctionName());
    SetClass(func->GetClass());
    if (class_ != nullptr) {
        SetClassName(func->GetRecordName() + class_->GetClassName());
    }
}

void CalleeInfo::SetFunctionName(std::string_view func_name)
{
    func_name_ = func_name;
}

void CalleeInfo::SetClassName(std::string_view class_name)
{
    class_name_ = class_name;
}

void CalleeInfo::SetExternalModuleName(std::string_view external_module_name)
{
    external_module_name_ = external_module_name;
}

void CalleeInfo::SetGlobalVarName(std::string_view global_var_name)
{
    global_var_name_ = global_var_name;
}
}  // namespace panda::defect_scan_aux