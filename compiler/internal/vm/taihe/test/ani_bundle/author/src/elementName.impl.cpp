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
    optional<string> deviceId;
    string bundleName_ = "bundleName_";
    string abilityName_ = "abilityName_";
    optional<string> uri;
    optional<string> shortName;

    ElementNameImpl() {}

    void SetDevicedId(optional<string> deviceId)
    {
        this->deviceId = deviceId;
    }

    optional<string> GetDevicedId()
    {
        return deviceId;
    }

    void SetBundleName(string_view bundleName)
    {
        bundleName_ = bundleName;
    }

    string GetBundleName()
    {
        return bundleName_;
    }

    void SetAbilityName(string_view abilityName)
    {
        abilityName_ = abilityName;
    }

    string GetAbilityName()
    {
        return abilityName_;
    }

    void SetUri(optional<string> uri)
    {
        this->uri = uri;
    }

    optional<string> GetUri()
    {
        return uri;
    }

    void SetShortName(optional<string> shortName)
    {
        this->shortName = shortName;
    }

    optional<string> GetShortName()
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