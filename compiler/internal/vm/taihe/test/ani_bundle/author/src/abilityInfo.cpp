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
#include <iostream>
#include "abilityInfo.impl.hpp"
#include "abilityInfo.proj.hpp"
#include "stdexcept"
#include "taihe/runtime.hpp"
using namespace taihe;
using namespace abilityInfo;

namespace {
// To be implemented.

class AbilityInfoImpl {
public:
    int32_t abilityInfoImpl = 100;

    AbilityInfoImpl() {}

    std::string GetBundleName()
    {
        return "AbilityInfo::getBundleName";
    }

    std::string GetName()
    {
        return "AbilityInfo::getName";
    }

    std::string GetLabel()
    {
        return "AbilityInfo::getLabel";
    }

    std::string GetDescription()
    {
        return "AbilityInfo::getDescription";
    }

    std::string GetIcon()
    {
        return "AbilityInfo::getIcon";
    }

    int32_t GetLabelId()
    {
        return abilityInfoImpl;
    }

    int32_t GetDescriptionId()
    {
        return abilityInfoImpl;
    }

    int32_t GetIconId()
    {
        return abilityInfoImpl;
    }

    std::string GetModuleName()
    {
        return "AbilityInfo::getModuleName";
    }

    std::string GetProcess()
    {
        return "AbilityInfo::getProcess";
    }

    std::string GetTargetAbility()
    {
        return "AbilityInfo::getTargetAbility";
    }

    int32_t GetBackgroundModes()
    {
        return abilityInfoImpl;
    }

    bool GetIsVisible()
    {
        return true;
    }

    bool GetFormEnabled()
    {
        return true;
    }

    array<string> GetPermissions()
    {
        array<string> str = {"AbilityInfo::getTargetAbility"};
        return str;
    }

    array<string> GetDeviceTypes()
    {
        array<string> str = {"AbilityInfo::getDeviceTypes"};
        return str;
    }

    array<string> GetDeviceCapabilities()
    {
        array<string> str = {"AbilityInfo::getDeviceCapabilities"};
        return str;
    }

    std::string GetReadPermission()
    {
        return "AbilityInfo::getReadPermission";
    }

    std::string GetWritePermission()
    {
        return "AbilityInfo::getWritePermission";
    }

    std::string GetUri()
    {
        return "AbilityInfo::getUri";
    }

    bool GetEnabled()
    {
        return true;
    }
};

AbilityInfo GetAbilityInfo()
{
    return make_holder<AbilityInfoImpl, AbilityInfo>();
}
}  // namespace

TH_EXPORT_CPP_API_GetAbilityInfo(GetAbilityInfo);