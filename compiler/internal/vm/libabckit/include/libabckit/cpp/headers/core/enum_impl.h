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

#ifndef CPP_ABCKIT_CORE_ENUM_IMPL_H
#define CPP_ABCKIT_CORE_ENUM_IMPL_H

#include "enum.h"

namespace abckit::core {

inline const File *Enum::GetFile() const
{
    return GetResource();
}

inline core::Module Enum::GetModule() const
{
    AbckitCoreModule *mod = GetApiConfig()->cIapi_->enumGetModule(GetView());
    CheckError(GetApiConfig());
    return Module(mod, GetApiConfig(), GetResource());
}

inline Namespace Enum::GetParentNamespace() const
{
    AbckitCoreNamespace *ns = GetApiConfig()->cIapi_->enumGetParentNamespace(GetView());
    CheckError(GetApiConfig());
    return Namespace(ns, GetApiConfig(), GetResource());
}

inline std::string Enum::GetName() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitString *cString = conf->cIapi_->enumGetName(GetView());
    CheckError(conf);
    std::string str = conf->cIapi_->abckitStringToString(cString);
    CheckError(conf);
    return str;
}

inline bool Enum::IsExternal() const
{
    const auto isExternal = GetApiConfig()->cIapi_->enumIsExternal(GetView());
    CheckError(GetApiConfig());
    return isExternal;
}

inline std::vector<core::Function> Enum::GetAllMethods() const
{
    std::vector<core::Function> methods;
    Payload<std::vector<core::Function> *> payload {&methods, GetApiConfig(), GetResource()};

    GetApiConfig()->cIapi_->enumEnumerateMethods(GetView(), &payload, [](AbckitCoreFunction *method, void *data) {
        const auto &payload = *static_cast<Payload<std::vector<core::Function> *> *>(data);
        payload.data->push_back(core::Function(method, payload.config, payload.resource));
        return true;
    });

    CheckError(GetApiConfig());

    return methods;
}

inline std::vector<core::EnumField> Enum::GetFields() const
{
    std::vector<core::EnumField> fields;
    Payload<std::vector<core::EnumField> *> payload {&fields, GetApiConfig(), GetResource()};

    GetApiConfig()->cIapi_->enumEnumerateFields(GetView(), &payload, [](AbckitCoreEnumField *field, void *data) {
        const auto &payload = *static_cast<Payload<std::vector<core::EnumField> *> *>(data);
        payload.data->push_back(core::EnumField(field, payload.config, payload.resource));
        return true;
    });

    CheckError(GetApiConfig());

    return fields;
}
}  // namespace abckit::core

#endif  // CPP_ABCKIT_CORE_ENUM_IMPL_H
