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
#include "ohos_app_ability_want.impl.hpp"
#include "ohos_app_ability_want.proj.hpp"
#include "stdexcept"
#include "taihe/runtime.hpp"

using namespace taihe;
using namespace ohos_app_ability_want;

namespace {
// To be implemented.

class WantImpl {
public:
    optional<string> bundleName_;
    optional<string> abilityName_;
    optional<string> deviceId_;
    optional<string> uri_;
    optional<string> type_;
    optional<float> flags_;
    optional<string> action_;
    optional<array<string>> entities_;
    optional<string> moduleName_;
    optional<map<string, uintptr_t>> parameters_;

    WantImpl() {}

    void SetBundleName(optional_view<string> bundleName)
    {
        bundleName_ = bundleName;
    }

    optional<string> GetBundleName()
    {
        return bundleName_;
    }

    void SetAbilityName(optional_view<string> abilityName)
    {
        abilityName_ = abilityName;
    }

    optional<string> GetAbilityName()
    {
        return abilityName_;
    }

    void SetDeviceId(optional_view<string> deviceId)
    {
        deviceId_ = deviceId;
    }

    optional<string> GetDeviceId()
    {
        return deviceId_;
    }

    void SetUri(optional_view<string> uri)
    {
        uri_ = uri;
    }

    optional<string> GetUri()
    {
        return uri_;
    }

    void SetType(optional_view<string> type)
    {
        type_ = type;
    }

    optional<string> GetType()
    {
        return type_;
    }

    void SetFlags(optional<float> flags)
    {
        flags_ = flags;
    }

    optional<float> GetFlags()
    {
        return flags_;
    }

    void SetAction(optional_view<string> action)
    {
        action_ = action;
    }

    optional<string> GetAction()
    {
        return action_;
    }

    void SetParameters(optional_view<map<string, uintptr_t>> parameters)
    {
        this->parameters_ = parameters;
    }

    optional<map<string, uintptr_t>> GetParameters()
    {
        return parameters_;
    }

    void SetEntities(optional_view<array<string>> entities)
    {
        entities_ = entities;
    }

    optional<array<string>> GetEntities()
    {
        return entities_;
    }

    void SetModuleName(optional_view<string> moduleName)
    {
        moduleName_ = moduleName;
    }

    optional<string> GetModuleName()
    {
        return moduleName_;
    }
};

Want CreateWant()
{
    return make_holder<WantImpl, Want>();
}
}  // namespace

TH_EXPORT_CPP_API_CreateWant(CreateWant);