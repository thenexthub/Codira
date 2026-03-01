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
#include "bundleInfo.proj.hpp"
#include "stdexcept"
#include "taihe/runtime.hpp"

using namespace taihe;
using namespace bundleInfo;

namespace {
// To be implemented.

class ReqPermissionDetailImpl {
public:
    string name_ = "";
    string moduleName_ = "";
    string reason_ = "";
    int reasonId_ = 0;

    ReqPermissionDetailImpl() {}

    void SetName(string_view name)
    {
        this->name_ = name;
    }

    string GetName()
    {
        return name_;
    }

    void SetModuleName(string_view moduleName)
    {
        this->moduleName_ = moduleName;
    }

    string GetModuleName()
    {
        return moduleName_;
    }

    void SetReason(string_view reason)
    {
        this->reason_ = reason;
    }

    string GetReason()
    {
        return reason_;
    }

    void SetReasonId(int32_t reasonId)
    {
        this->reasonId_ = reasonId;
    }

    int32_t GetReasonId()
    {
        return reasonId_;
    }
};

class UsedSceneImpl {
public:
    array<string> abilities_ = {""};
    string when_ = "";

    UsedSceneImpl() {}

    void SetAbilities(array_view<string> abilities)
    {
        abilities_ = abilities;
    }

    array<string> GetAbilities()
    {
        return abilities_;
    }

    void SetWhen(string_view when)
    {
        this->when_ = when;
    }

    string GetWhen()
    {
        return when_;
    }
};

class SignatureInfoImpl {
public:
    SignatureInfoImpl() {}

    string GetAppId()
    {
        return "SignatureInfoImpl::getAppId";
    }

    string GetFingerprint()
    {
        return "SignatureInfoImpl::getFingerprint";
    }

    string GetAppIdentifier()
    {
        return "SignatureInfoImpl::getAppIdentifier";
    }

    string GetCertificate()
    {
        return "SignatureInfoImpl::getCertificate";
    }
};

class AppCloneIdentityImpl {
public:
    int32_t appCloneIdentityImpl = 21474;

    AppCloneIdentityImpl() {}

    string GetBundleName()
    {
        return "AppCloneIdentityImpl::getBundleName";
    }

    int32_t GetAppIndex()
    {
        return appCloneIdentityImpl;
    }
};

class BundleInfoImpl {
public:
    int32_t version = 21474;

    BundleInfoImpl() {}

    string GetName()
    {
        return "BundleInfoImpl::getName";
    }

    string GetVendor()
    {
        return "BundleInfoImpl::getVendor";
    }

    int32_t GetVersionCode()
    {
        return version;
    }

    string GetVersionName()
    {
        return "BundleInfoImpl::getVersionName";
    }

    string GetMinCompatibleVersionCode()
    {
        return "BundleInfoImpl::getMinCompatibleVersionCode";
    }

    int32_t GetTargetVersion()
    {
        return version;
    }

    int32_t GetInstallTime()
    {
        return version;
    }

    int32_t GetUpdateTime()
    {
        return version;
    }

    int32_t GetAppIndex()
    {
        return version;
    }
};

BundleInfo GetBundleInfo()
{
    return make_holder<BundleInfoImpl, BundleInfo>();
}

ReqPermissionDetail GetReqPermissionDetail()
{
    return make_holder<ReqPermissionDetailImpl, ReqPermissionDetail>();
}

UsedScene GetIUsedScene()
{
    return make_holder<UsedSceneImpl, UsedScene>();
}

SignatureInfo GetISignatureInfo()
{
    return make_holder<SignatureInfoImpl, SignatureInfo>();
}

AppCloneIdentity GetAppCloneIdentity()
{
    return make_holder<AppCloneIdentityImpl, AppCloneIdentity>();
}
}  // namespace

TH_EXPORT_CPP_API_GetBundleInfo(GetBundleInfo);
TH_EXPORT_CPP_API_GetReqPermissionDetail(GetReqPermissionDetail);
TH_EXPORT_CPP_API_GetIUsedScene(GetIUsedScene);
TH_EXPORT_CPP_API_GetISignatureInfo(GetISignatureInfo);
TH_EXPORT_CPP_API_GetAppCloneIdentity(GetAppCloneIdentity);