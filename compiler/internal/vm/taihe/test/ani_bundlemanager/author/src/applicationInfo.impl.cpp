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
#include "applicationInfo.proj.hpp"
#include "stdexcept"
#include "taihe/runtime.hpp"

using namespace taihe;
using namespace applicationInfo;

namespace {
// To be implemented.

class ApplicationInfoImpl {
public:
    int32_t applicationInfoImpl = 21474;

    ApplicationInfoImpl() {}

    string GetName()
    {
        return "ApplicationInfoImpl::getName";
    }

    string GetDescription()
    {
        return "ApplicationInfoImpl::getDescription";
    }

    int32_t GetDescriptionId()
    {
        return applicationInfoImpl;
    }

    bool GetEnabled()
    {
        return true;
    }

    string GetLabel()
    {
        return "ApplicationInfoImpl::getLabel";
    }

    int32_t GetLabelId()
    {
        return applicationInfoImpl;
    }

    string GetIcon()
    {
        return "ApplicationInfoImpl::getIcon";
    }

    int32_t GetIconId()
    {
        return applicationInfoImpl;
    }

    string GetProcess()
    {
        return "ApplicationInfoImpl::getProcess";
    }

    array<string> GetPermissions()
    {
        array<string> res = {"ApplicationInfoImpl::getPermissions"};
        return res;
    }

    string GetCodePath()
    {
        return "ApplicationInfoImpl::getCodePath";
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

    string GetAppDistributionType()
    {
        return "ApplicationInfoImpl::getAppDistributionType";
    }

    string GetAppProvisionType()
    {
        return "ApplicationInfoImpl::getAppProvisionType";
    }

    bool GetSystemApp()
    {
        return true;
    }

    bool GetDebug()
    {
        return true;
    }

    bool GetDataUnclearable()
    {
        return true;
    }

    string GetNativeLibraryPath()
    {
        return "ApplicationInfoImpl::getNativeLibraryPath";
    }

    int32_t GetAppIndex()
    {
        return applicationInfoImpl;
    }

    string GetInstallSource()
    {
        return "ApplicationInfoImpl::getInstallSource";
    }

    string GetReleaseType()
    {
        return "ApplicationInfoImpl::getReleaseType";
    }

    bool GetCloudFileSyncEnabled()
    {
        return true;
    }
};

class ModuleMetadataImpl {
public:
    ModuleMetadataImpl() {}

    string GetModuleName()
    {
        return "ModuleMetadataImpl::getModuleName";
    }
};

class MultiAppModeImpl {
public:
    int32_t multiAppModeImpl = 21474;

    MultiAppModeImpl() {}

    int32_t GetMaxCount()
    {
        return multiAppModeImpl;
    }
};

ApplicationInfo GetApplicationInfo()
{
    return make_holder<ApplicationInfoImpl, ApplicationInfo>();
}

ModuleMetadata GetModuleMetadata()
{
    return make_holder<ModuleMetadataImpl, ModuleMetadata>();
}

MultiAppMode GetMultiAppMode()
{
    return make_holder<MultiAppModeImpl, MultiAppMode>();
}
}  // namespace

TH_EXPORT_CPP_API_GetApplicationInfo(GetApplicationInfo);
TH_EXPORT_CPP_API_GetModuleMetadata(GetModuleMetadata);
TH_EXPORT_CPP_API_GetMultiAppMode(GetMultiAppMode);