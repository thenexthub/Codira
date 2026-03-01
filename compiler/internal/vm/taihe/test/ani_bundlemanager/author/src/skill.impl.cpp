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
#include "skill.impl.hpp"
#include <iostream>
#include "skill.h"
#include "skill.proj.hpp"
#include "stdexcept"
#include "taihe/runtime.hpp"

using namespace taihe;
using namespace skill;

namespace {
// To be implemented.

class SkillUriImpl {
public:
    int32_t skillUriImpl = 21474;

    SkillUriImpl() {}

    string GetScheme()
    {
        return "SkillUriImpl::getScheme";
    }

    string GetHost()
    {
        return "SkillUriImpl::getHost";
    }

    int32_t GetPort()
    {
        return skillUriImpl;
    }

    string GetPath()
    {
        return "SkillUriImpl::getPath";
    }

    string GetPathStartWith()
    {
        return "SkillUriImpl::getPathStartWith";
    }

    string GetPathRegex()
    {
        return "SkillUriImpl::getPathRegex";
    }

    string GetType()
    {
        return "SkillUriImpl::getType";
    }

    string GetUtd()
    {
        return "SkillUriImpl::getUtd";
    }

    int32_t GetMaxFileSupported()
    {
        return skillUriImpl;
    }

    string GetLinkFeature()
    {
        return "SkillUriImpl::getLinkFeature";
    }
};

Skill GetSkill()
{
    return make_holder<SkillImpl, Skill>();
}

SkillUri GetSkillUri()
{
    return make_holder<SkillUriImpl, SkillUri>();
}
}  // namespace

TH_EXPORT_CPP_API_GetSkill(GetSkill);
TH_EXPORT_CPP_API_GetSkillUri(GetSkillUri);