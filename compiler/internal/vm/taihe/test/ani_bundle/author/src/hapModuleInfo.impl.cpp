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
#include "hapModuleInfo.impl.hpp"
#include "hapModuleInfo.proj.hpp"
#include "stdexcept"
#include "taihe/runtime.hpp"

using namespace taihe;
using namespace hapModuleInfo;

namespace {

class HapModuleInfoImpl {
public:
    int32_t hapModuleInfoImpl = 1024;

    HapModuleInfoImpl() {}

    string GetName()
    {
        return "HapModuleInfo::getName";
    }

    string GetDescription()
    {
        return "HapModuleInfo::getDescription";
    }

    int32_t GetDescriptionId()
    {
        return hapModuleInfoImpl;
    }

    string GetIcon()
    {
        return "HapModuleInfo::getIcon";
    }

    string GetLabel()
    {
        return "HapModuleInfo::getLabel";
    }

    int32_t GetLabelId()
    {
        return hapModuleInfoImpl;
    }

    int32_t GetIconId()
    {
        return hapModuleInfoImpl;
    }

    string GetBackgroundImg()
    {
        return "HapModuleInfo::getBackgroundImg";
    }

    int32_t GetSupportedModes()
    {
        return hapModuleInfoImpl;
    }

    array<string> GetReqCapabilities()
    {
        array<string> str = {"HapModuleInfo::getReqCapabilities"};
        return str;
    }

    array<string> GetDeviceTypes()
    {
        array<string> str = {"HapModuleInfo::getDeviceTypes"};
        return str;
    }

    string GetModuleName()
    {
        return "HapModuleInfo::getModuleName";
    }

    string GetMainAbilityName()
    {
        return "HapModuleInfo::getMainAbilityName";
    }

    bool GetInstallationFree()
    {
        return true;
    }
};

HapModuleInfo GetHapModuleInfo()
{
    return make_holder<HapModuleInfoImpl, HapModuleInfo>();
}
}  // namespace

TH_EXPORT_CPP_API_GetHapModuleInfo(GetHapModuleInfo);