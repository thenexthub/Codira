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
#include "bundleInfo.impl.hpp"
#include <iostream>
#include "bundleInfo.proj.hpp"
#include "stdexcept"
#include "taihe/runtime.hpp"

using namespace taihe;
using namespace bundleInfo;

namespace {
// To be implemented.

class UsedSceneImpl {
public:
    string when_ = "test";
    array<string> abilities_ = {"getAbilities", "not", "implemented"};

    UsedSceneImpl() {}

    array<string> GetAbilities()
    {
        return abilities_;
    }

    void SetAbilities(array_view<string> abilities)
    {
        abilities_ = abilities;
    }

    void SetWhen(string_view when)
    {
        when_ = when;
    }

    string GetWhen()
    {
        return when_;
    }
};

class ReqPermissionDetailImpl {
public:
    string name_ = "name";
    string reason_ = "reason_";
    UsedSceneImpl usedSceneImpl_;

    ReqPermissionDetailImpl() {}

    string GetName()
    {
        return name_;
    }

    void SetName(string_view name)
    {
        name_ = name;
    }

    string GetReason()
    {
        return reason_;
    }

    void SetReason(string_view reason)
    {
        reason_ = reason;
    }

    UsedSceneImpl GetUsedScene()
    {
        return GetUsedScene();
    }

    void SetUsedScene(UsedSceneImpl usedScene)
    {
        usedSceneImpl_.when_ = usedScene.when_;
        usedSceneImpl_.abilities_ = usedScene.abilities_;
    }
};

class BundleInfoImpl {
public:
    int32_t bundleInfoImpl = 127;

    BundleInfoImpl() {}

    string GetName()
    {
        return "bundleInfo::getName";
    }

    string GetType()
    {
        return "bundleInfo::getType";
    }

    string GetAppId()
    {
        return "bundleInfo::getAppId";
    }

    int32_t GetUid()
    {
        return bundleInfoImpl;
    }

    int32_t GetInstallTime()
    {
        return bundleInfoImpl;
    }

    int32_t GetUpdateTime()
    {
        return bundleInfoImpl;
    }

    array<string> GetReqPermissions()
    {
        array<string> str = {"bundleInfo::getReqPermissions"};
        return str;
    }

    string GetVendor()
    {
        return "bundleInfo::getVendor";
    }

    int32_t GetVersionCode()
    {
        return bundleInfoImpl;
    }

    string GetVersionName()
    {
        return "bundleInfo::getVersionName";
    }

    int32_t GetCompatibleVersion()
    {
        return bundleInfoImpl;
    }

    int32_t GetTargetVersion()
    {
        return bundleInfoImpl;
    }

    bool GetIsCompressNativeLibs()
    {
        return true;
    }

    string GetEntryModuleName()
    {
        return "bundleInfo::getEntryModuleName";
    }

    string GetCpuAbi()
    {
        return "bundleInfo::getCpuAbi";
    }

    string GetIsSilentInstallation()
    {
        return "bundleInfo::getIsSilentInstallation";
    }

    int32_t GetMinCompatibleVersionCode()
    {
        return bundleInfoImpl;
    }

    bool GetEntryInstallationFree()
    {
        return true;
    }

    array<int32_t> GetReqPermissionStates()
    {
        array<int32_t> arr = {bundleInfoImpl};
        return arr;
    }
};

UsedScene GetUsedScene()
{
    return make_holder<UsedSceneImpl, UsedScene>();
}

ReqPermissionDetail GetReqPermissionDetail()
{
    return make_holder<ReqPermissionDetailImpl, ReqPermissionDetail>();
}

BundleInfo GetBundleInfo()
{
    return make_holder<BundleInfoImpl, BundleInfo>();
}
}  // namespace

TH_EXPORT_CPP_API_GetUsedScene(GetUsedScene);
TH_EXPORT_CPP_API_GetReqPermissionDetail(GetReqPermissionDetail);
TH_EXPORT_CPP_API_GetBundleInfo(GetBundleInfo);