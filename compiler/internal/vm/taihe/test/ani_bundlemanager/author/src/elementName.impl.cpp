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
#include "elementName.impl.hpp"
#include "elementName.proj.hpp"
#include "stdexcept"
#include "taihe/runtime.hpp"

using namespace taihe;
using namespace elementName;

namespace {
// To be implemented.

class ElementNameImpl {
public:
    string deviceId = "";
    string bundleName = "";
    string moduleName = "";
    string abilityName = "";
    string uri = "";
    string shortName = "";

    ElementNameImpl() {}

    void SetDevicedId(string_view deviceId)
    {
        this->deviceId = deviceId;
    }

    string GetDevicedId()
    {
        return deviceId;
    }

    void SetBundleName(string_view bundleName)
    {
        this->bundleName = bundleName;
    }

    string GetBundleName()
    {
        return bundleName;
    }

    void SetMundleName(string_view moduleName)
    {
        this->moduleName = moduleName;
    }

    string GetMundleName()
    {
        return moduleName;
    }

    void SetAbilityName(string_view abilityName)
    {
        this->abilityName = abilityName;
    }

    string GetAbilityName()
    {
        return abilityName;
    }

    void SetUri(string_view uri)
    {
        this->uri = uri;
    }

    string GetUri()
    {
        return uri;
    }

    void SetShortName(string_view shortName)
    {
        this->shortName = shortName;
    }

    string GetShortName()
    {
        return shortName;
    }
};

ElementName GetElementName()
{
    return make_holder<ElementNameImpl, ElementName>();
}
}  // namespace

TH_EXPORT_CPP_API_GetElementName(GetElementName);