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

#ifndef CPP_ABCKIT_CORE_ANNOTATION_INTERFACE_IMPL_H
#define CPP_ABCKIT_CORE_ANNOTATION_INTERFACE_IMPL_H

#include "annotation_interface.h"
#include "module.h"

namespace abckit::core {

inline const File *AnnotationInterface::GetFile() const
{
    return GetResource();
}

inline bool AnnotationInterface::EnumerateFields(const std::function<bool(core::AnnotationInterfaceField)> &cb) const
{
    Payload<const std::function<bool(core::AnnotationInterfaceField)> &> payload {cb, GetApiConfig(), GetResource()};

    bool isNormalExit = GetApiConfig()->cIapi_->annotationInterfaceEnumerateFields(
        GetView(), &payload, [](AbckitCoreAnnotationInterfaceField *func, void *data) {
            const auto &payload =
                *static_cast<Payload<const std::function<bool(core::AnnotationInterfaceField)> &> *>(data);
            return payload.data(core::AnnotationInterfaceField(func, payload.config, payload.resource));
        });
    CheckError(GetApiConfig());
    return isNormalExit;
}

// CC-OFFNXT(G.FUD.06) perf critical
inline std::vector<AnnotationInterfaceField> AnnotationInterface::GetFields() const
{
    std::vector<core::AnnotationInterfaceField> namespaces;
    Payload<std::vector<core::AnnotationInterfaceField> *> payload {&namespaces, GetApiConfig(), GetResource()};

    GetApiConfig()->cIapi_->annotationInterfaceEnumerateFields(
        GetView(), &payload, [](AbckitCoreAnnotationInterfaceField *func, void *data) {
            const auto &payload = *static_cast<Payload<std::vector<core::AnnotationInterfaceField> *> *>(data);
            payload.data->push_back(core::AnnotationInterfaceField(func, payload.config, payload.resource));
            return true;
        });

    CheckError(GetApiConfig());

    return namespaces;
}

inline core::Module AnnotationInterface::GetModule() const
{
    auto mod = GetApiConfig()->cIapi_->annotationInterfaceGetModule(GetView());
    CheckError(GetApiConfig());
    return Module(mod, GetApiConfig(), GetResource());
}

inline Namespace AnnotationInterface::GetParentNamespace() const
{
    AbckitCoreNamespace *ns = GetApiConfig()->cIapi_->annotationInterfaceGetParentNamespace(GetView());
    CheckError(GetApiConfig());
    return Namespace(ns, GetApiConfig(), GetResource());
}

inline std::string AnnotationInterface::GetName() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitString *abcStr = conf->cIapi_->annotationInterfaceGetName(GetView());
    CheckError(conf);
    std::string str = conf->cIapi_->abckitStringToString(abcStr);
    CheckError(conf);
    return str;
}

}  // namespace abckit::core

#endif  // CPP_ABCKIT_CORE_ANNOTATION_INTERFACE_IMPL_H
