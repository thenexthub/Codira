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
#include "ui_user.impl.hpp"
#include <iostream>
#include "stdexcept"
#include "taihe/runtime.hpp"
#include "ui_user.proj.hpp"

class AntUserDialogActionA {
public:
    AntUserDialogActionA() {}

    ::taihe::string GetName()
    {
        return this->name;
    }

    void SetName(::taihe::string_view name)
    {
        this->name = name;
    }

    ::taihe::optional<::taihe::map<::taihe::string, ::taihe::string>> GetTraceParams()
    {
        return this->traceParams;
    }

    void SetTraceParams(::taihe::optional_view<::taihe::map<::taihe::string, ::taihe::string>> traceParams)
    {
        this->traceParams = traceParams;
    }

    void action()
    {
        std::cout << "ActionA Callback" << std::endl;
    }

private:
    ::taihe::string name = "ActionA";
    ::taihe::optional<::taihe::map<::taihe::string, ::taihe::string>> traceParams =
        ::taihe::optional<::taihe::map<::taihe::string, ::taihe::string>>(std::nullopt);
};

class AntUserDialogActionB {
public:
    AntUserDialogActionB() {}

    ::taihe::string GetName()
    {
        return this->name;
    }

    void SetName(::taihe::string_view name)
    {
        this->name = name;
    }

    ::taihe::optional<::taihe::map<::taihe::string, ::taihe::string>> GetTraceParams()
    {
        return this->traceParams;
    }

    void SetTraceParams(::taihe::optional_view<::taihe::map<::taihe::string, ::taihe::string>> traceParams)
    {
        this->traceParams = traceParams;
    }

    void action()
    {
        std::cout << "ActionB Callback" << std::endl;
    }

private:
    ::taihe::string name = "ActionB";
    ::taihe::optional<::taihe::map<::taihe::string, ::taihe::string>> traceParams =
        ::taihe::optional<::taihe::map<::taihe::string, ::taihe::string>>(std::nullopt);
};

class AntUserDialogActionC {
public:
    AntUserDialogActionC() {}

    ::taihe::string GetName()
    {
        return this->name;
    }

    void SetName(::taihe::string_view name)
    {
        this->name = name;
    }

    ::taihe::optional<::taihe::map<::taihe::string, ::taihe::string>> GetTraceParams()
    {
        return this->traceParams;
    }

    void SetTraceParams(::taihe::optional_view<::taihe::map<::taihe::string, ::taihe::string>> traceParams)
    {
        this->traceParams = traceParams;
    }

    void action()
    {
        std::cout << "ActionC Callback" << std::endl;
    }

private:
    ::taihe::string name = "ActionC";
    ::taihe::optional<::taihe::map<::taihe::string, ::taihe::string>> traceParams =
        ::taihe::optional<::taihe::map<::taihe::string, ::taihe::string>>(std::nullopt);
};

namespace {
void RunNativeBusiness(::ui::weak::AntUserUIProvider ui, ::ui::AntUserDialogBody const &body)
{
    ::ui::AntUserDialogAction actionA = ::taihe::make_holder<AntUserDialogActionA, ::ui::AntUserDialogAction>();
    ::ui::AntUserDialogAction actionB = ::taihe::make_holder<AntUserDialogActionB, ::ui::AntUserDialogAction>();
    ::ui::AntUserDialogAction actionC = ::taihe::make_holder<AntUserDialogActionC, ::ui::AntUserDialogAction>();
    ::taihe::array<::ui::AntUserDialogAction> actions = {actionA, actionB, actionC};
    ::ui::AntUserModalController controller = ui->ShowDialog(body, actions);
    controller->Dismiss();
}
}  // namespace

// Since these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_RunNativeBusiness(RunNativeBusiness);
// NOLINTEND