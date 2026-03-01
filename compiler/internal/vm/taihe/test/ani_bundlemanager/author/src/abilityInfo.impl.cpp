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
#include "abilityInfo.impl.hpp"
#include "abilityInfo.proj.hpp"
#include "metadata.h"
#include "skill.h"
#include "stdexcept"
#include "taihe/runtime.hpp"

using namespace taihe;
using namespace abilityInfo;

namespace {
// To be implemented.

class AbilityInfoImpl {
public:
    int32_t abilityInfoImpl = 506;

    AbilityInfoImpl() {}

    string GetBundleName()
    {
        return "abilityInfoImpl::getBundleName";
    }

    string GetModuleName()
    {
        return "abilityInfoImpl::getModuleName";
    }

    string GetName()
    {
        return "abilityInfoImpl::getName";
    }

    string GetLabel()
    {
        return "abilityInfoImpl::getLabel";
    }

    int32_t GetLabelId()
    {
        return abilityInfoImpl;
    }

    string GetDescription()
    {
        return "abilityInfoImpl::getDescription";
    }

    int32_t GetDescriptionId()
    {
        return abilityInfoImpl;
    }

    string GetIcon()
    {
        return "abilityInfoImpl::getIcon";
    }

    int32_t GetIconId()
    {
        return abilityInfoImpl;
    }

    string GetProcess()
    {
        return "abilityInfoImpl::getProcess";
    }

    bool GetExported()
    {
        return true;
    }

    ::ohos::bundle::bundleManager::AbilityType GetType()
    {
        return ::ohos::bundle::bundleManager::AbilityType::key_t::DATA;
    }

    ::ohos::bundle::bundleManager::DisplayOrientation GetOrientation()
    {
        return ::ohos::bundle::bundleManager::DisplayOrientation::key_t::LANDSCAPE;
    }

    ::ohos::bundle::bundleManager::LaunchType GetLaunchType()
    {
        return ::ohos::bundle::bundleManager::LaunchType::key_t::MULTITON;
    }

    array<string> GetPermissions()
    {
        array<string> res = {"abilityInfoImpl::getPermissions"};
        return res;
    }

    string GetReadPermission()
    {
        return "abilityInfoImpl::getReadPermission";
    }

    string GetWritePermission()
    {
        string res = "abilityInfoImpl::getWritePermission";
        return res;
    }

    string GetUri()
    {
        return "abilityInfoImpl::getUri";
    }

    array<string> GetDeviceTypes()
    {
        array<string> res = {"abilityInfoImpl::getDeviceTypes"};
        return res;
    }

    array<::metadata::Metadata> GetMetadata()
    {
        metadata::Metadata data = make_holder<MetadataImpl, metadata::Metadata>();
        array<metadata::Metadata> res = {data};
        return res;
    }

    bool GetEnabled()
    {
        return true;
    }

    array<::ohos::bundle::bundleManager::SupportWindowMode> GetSupportWindowModes()
    {
        array<::ohos::bundle::bundleManager::SupportWindowMode> res = {
            ::ohos::bundle::bundleManager::SupportWindowMode::key_t::FLOATING};

        return res;
    }

    bool GetExcludeFromDock()
    {
        return true;
    }

    array<::skill::Skill> GetSkills()
    {
        ::skill::Skill data = make_holder<SkillImpl, ::skill::Skill>();
        array<::skill::Skill> res = {data};

        return res;
    }

    int32_t GetAppIndex()
    {
        return abilityInfoImpl;
    }

    int32_t GetOrientationId()
    {
        return abilityInfoImpl;
    }
};

AbilityInfo GetAbilityInfo()
{
    return make_holder<AbilityInfoImpl, AbilityInfo>();
}
}  // namespace

TH_EXPORT_CPP_API_GetAbilityInfo(GetAbilityInfo);