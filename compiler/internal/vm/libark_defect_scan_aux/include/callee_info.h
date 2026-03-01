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

#ifndef LIBARK_DEFECT_SCAN_AUX_INCLUDE_CALLEE_INFO_H
#define LIBARK_DEFECT_SCAN_AUX_INCLUDE_CALLEE_INFO_H

#include "graph.h"

namespace panda::defect_scan_aux {
class Class;
class Function;

class CalleeInfo final {
public:
    CalleeInfo(const Inst &call_inst, const Function *caller) : call_inst_(call_inst), caller_(caller) {}
    ~CalleeInfo() = default;
    NO_COPY_SEMANTIC(CalleeInfo);
    NO_MOVE_SEMANTIC(CalleeInfo);

    bool IsCalleeDefinite() const;
    int GetCalleeArgCount() const;
    const Inst &GetCallInst() const;
    const Function *GetCaller() const;
    const Class *GetClass() const;
    const Function *GetCallee() const;
    const std::string &GetFunctionName() const;
    const std::string &GetClassName() const;
    const std::string &GetExternalModuleName() const;
    const std::string &GetGlobalVarName() const;

private:
    void SetCalleeArgCount(int arg_count);
    void SetClass(const Class *clazz);
    void SetCallee(const Function *func);
    void SetFunctionName(std::string_view func_name);
    void SetClassName(std::string_view class_name);
    void SetExternalModuleName(std::string_view external_module_name);
    void SetGlobalVarName(std::string_view global_var_name);

    bool is_definite_ {false};
    // -1 means arg count of callee is unknown
    int arg_count_ {-1};
    const Inst call_inst_;
    const Function *caller_ {nullptr};
    const Class *class_ {nullptr};
    const Function *func_ {nullptr};
    std::string func_name_;
    std::string class_name_;
    std::string external_module_name_;
    std::string global_var_name_;

    friend class AbcFile;
};
}  // namespace panda::defect_scan_aux

#endif  // LIBARK_DEFECT_SCAN_AUX_INCLUDE_CALLEE_INFO_H