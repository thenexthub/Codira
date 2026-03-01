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

#ifndef LIBARK_DEFECT_SCAN_AUX_INCLUDE_CLASS_H
#define LIBARK_DEFECT_SCAN_AUX_INCLUDE_CLASS_H

#include <string>
#include <vector>
#include "libpandabase/macros.h"

namespace panda::defect_scan_aux {
class Class;
class Function;
class AbcFile;

struct ParentClassInfo {
    const Class *class_ {nullptr};
    std::string class_name_;
    std::string external_module_name_;
    std::string global_var_name_;
};

class Class final {
public:
    Class(std::string_view class_name, std::string_view record_name,
          const AbcFile *abc_file, Function *def_func)
        : class_name_(class_name), record_name_(record_name), abc_file_(abc_file), def_func_(def_func)
    {
    }
    ~Class() = default;
    NO_COPY_SEMANTIC(Class);
    NO_MOVE_SEMANTIC(Class);

    const std::string &GetClassName() const;
    const std::string &GetRecordName() const;
    const AbcFile *GetAbcFileInstance() const;
    Function *GetDefiningFunction() const;
    size_t GetMemberFunctionCount() const;
    const Function *GetMemberFunctionByName(std::string_view func_name) const;
    const Function *GetMemberFunctionByIndex(size_t index) const;
    const std::vector<const Function *> &GetMemberFunctionList() const;
    const Class *GetParentClass() const;
    const std::string &GetParentClassName() const;
    const std::string &GetParClassExternalModuleName() const;
    const std::string &GetParClassGlobalVarName() const;

private:
    void SetParentClass(const Class *parent_class);
    void SetParentClassName(std::string_view par_class_name);
    void SetParClassExternalModuleName(std::string_view external_module_name);
    void SetParClassGlobalVarName(std::string global_var_name);
    void AddMemberFunction(const Function *func);

    std::string class_name_;
    std::string record_name_;
    const AbcFile *abc_file_ {nullptr};
    Function *def_func_ {nullptr};
    std::vector<const Function *> member_func_list_;
    ParentClassInfo par_class_info_;

    friend class AbcFile;
};
}  // namespace panda::defect_scan_aux

#endif  // LIBARK_DEFECT_SCAN_AUX_INCLUDE_CLASS_H
