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
#include "applicationInfo.impl.hpp"
#include <iostream>
#include "applicationInfo.proj.hpp"
#include "stdexcept"
#include "taihe/runtime.hpp"

using namespace taihe;
using namespace applicationInfo;

namespace {
// To be implemented.

class ApplicationInfoImpl {
public:
    int32_t applicationInfoImpl = 102;

    ApplicationInfoImpl() {}

    string GetName()
    {
        return "ApplicationInfo::getName";
    }

    string GetDescription()
    {
        return "ApplicationInfo::getDescription";
    }

    int32_t GetDescriptionId()
    {
        return applicationInfoImpl;
    }

    bool GetSystemApp()
    {
        return true;
    }

    bool GetEnabled()
    {
        return true;
    }

    string GetLabel()
    {
        return "ApplicationInfo::getLabel";
    }

    string GetLabelId()
    {
        return "ApplicationInfo::getLabelId";
    }

    string GetIcon()
    {
        return "ApplicationInfo::getIcon";
    }

    int32_t GetIconId()
    {
        return applicationInfoImpl;
    }

    string GetProcess()
    {
        return "ApplicationInfo::getProcess";
    }

    int32_t GetSupportedModes()
    {
        return applicationInfoImpl;
    }

    array<string> GetModuleSourceDirs()
    {
        array<string> str = {"ApplicationInfo::getProcess"};
        return str;
    }

    array<string> GetPermissions()
    {
        array<string> str = {"ApplicationInfo::getPermissions"};
        return str;
    }

    string GetEntryDir()
    {
        return "ApplicationInfo::getEntryDir";
    }

    string GetCodePath()
    {
        return "ApplicationInfo::getCodePath";
    }

    bool GetRemovable()
    {
        return true;
    }

    int32_t GetAccessTokenId()
    {
        return applicationInfoImpl;
    }

    int32_t GetUid()
    {
        return applicationInfoImpl;
    }

    string GetEntityType()
    {
        return "ApplicationInfo::getEntityType";
    }
};

ApplicationInfo GetApplicationInfo()
{
    return make_holder<ApplicationInfoImpl, ApplicationInfo>();
}
}  // namespace

TH_EXPORT_CPP_API_GetApplicationInfo(GetApplicationInfo);