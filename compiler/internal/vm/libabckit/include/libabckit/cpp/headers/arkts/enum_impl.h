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

#ifndef CPP_ABCKIT_ARKTS_ENUM_IMPL_H
#define CPP_ABCKIT_ARKTS_ENUM_IMPL_H

#include "enum.h"

// NOLINTBEGIN(performance-unnecessary-value-param)
namespace abckit::arkts {

inline AbckitArktsEnum *Enum::TargetCast() const
{
    const auto ret = GetApiConfig()->cArktsIapi_->coreEnumToArktsEnum(GetView());
    CheckError(GetApiConfig());
    return ret;
}

inline Enum::Enum(const core::Enum &coreOther) : core::Enum(coreOther), targetChecker_(this) {}

inline bool Enum::SetName(const std::string &name) const
{
    const auto ret = GetApiConfig()->cArktsMapi_->enumSetName(TargetCast(), name.c_str());
    CheckError(GetApiConfig());
    return ret;
}

inline arkts::EnumField Enum::AddField(const std::string_view name, const Type &type, const Value &value,
                                       AbckitArktsFieldVisibility fieldVisibility)
{
    const struct AbckitArktsFieldCreateParams params {
        name.data(), type.GetView(), value.GetView(), true, fieldVisibility
    };
    auto *arkEnumField = GetApiConfig()->cArktsMapi_->enumAddField(TargetCast(), &params);
    CheckError(GetApiConfig());
    auto *coreEnumField = GetApiConfig()->cArktsIapi_->arktsEnumFieldToCoreEnumField(arkEnumField);
    CheckError(GetApiConfig());
    return arkts::EnumField(core::EnumField(coreEnumField, GetApiConfig(), GetResource()));
}

inline arkts::EnumField Enum::AddField(const std::string_view name, const Type &type,
                                       AbckitArktsFieldVisibility fieldVisibility)
{
    const struct AbckitArktsFieldCreateParams params {
        name.data(), type.GetView(), nullptr, true, fieldVisibility
    };
    auto *arkEnumField = GetApiConfig()->cArktsMapi_->enumAddField(TargetCast(), &params);
    CheckError(GetApiConfig());
    auto *coreEnumField = GetApiConfig()->cArktsIapi_->arktsEnumFieldToCoreEnumField(arkEnumField);
    CheckError(GetApiConfig());
    return arkts::EnumField(core::EnumField(coreEnumField, GetApiConfig(), GetResource()));
}

}  // namespace abckit::arkts
// NOLINTEND(performance-unnecessary-value-param)

#endif  // CPP_ABCKIT_ARKTS_ENUM_IMPL_H
