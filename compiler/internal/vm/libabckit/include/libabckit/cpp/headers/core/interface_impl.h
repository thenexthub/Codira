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

#ifndef CPP_ABCKIT_CORE_INTERFACE_IMPL_H
#define CPP_ABCKIT_CORE_INTERFACE_IMPL_H

#include "interface.h"

namespace abckit::core {

inline const File *Interface::GetFile() const
{
    return GetResource();
}

inline std::string Interface::GetName() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitString *cString = conf->cIapi_->interfaceGetName(GetView());
    CheckError(conf);
    std::string str = conf->cIapi_->abckitStringToString(cString);
    CheckError(conf);
    return str;
}

inline bool Interface::IsExternal() const
{
    const auto isExternal = GetApiConfig()->cIapi_->interfaceIsExternal(GetView());
    CheckError(GetApiConfig());
    return isExternal;
}

inline core::Module Interface::GetModule() const
{
    AbckitCoreModule *mod = GetApiConfig()->cIapi_->interfaceGetModule(GetView());
    CheckError(GetApiConfig());
    return Module(mod, GetApiConfig(), GetResource());
}

inline Namespace Interface::GetParentNamespace() const
{
    AbckitCoreNamespace *ns = GetApiConfig()->cIapi_->interfaceGetParentNamespace(GetView());
    CheckError(GetApiConfig());
    return Namespace(ns, GetApiConfig(), GetResource());
}

inline std::vector<core::Function> Interface::GetAllMethods() const
{
    std::vector<core::Function> methods;
    Payload<std::vector<core::Function> *> payload {&methods, GetApiConfig(), GetResource()};

    GetApiConfig()->cIapi_->interfaceEnumerateMethods(GetView(), &payload, [](AbckitCoreFunction *method, void *data) {
        const auto &payload = *static_cast<Payload<std::vector<core::Function> *> *>(data);
        payload.data->push_back(core::Function(method, payload.config, payload.resource));
        return true;
    });

    CheckError(GetApiConfig());

    return methods;
}

inline std::vector<core::InterfaceField> Interface::GetFields() const
{
    std::vector<core::InterfaceField> fields;
    Payload<std::vector<core::InterfaceField> *> payload {&fields, GetApiConfig(), GetResource()};

    GetApiConfig()->cIapi_->interfaceEnumerateFields(
        GetView(), &payload, [](AbckitCoreInterfaceField *field, void *data) {
            const auto &payload = *static_cast<Payload<std::vector<core::InterfaceField> *> *>(data);
            payload.data->push_back(core::InterfaceField(field, payload.config, payload.resource));
            return true;
        });

    CheckError(GetApiConfig());

    return fields;
}

inline std::vector<core::Interface> Interface::GetSuperInterfaces() const
{
    std::vector<core::Interface> interfaces;
    Payload<std::vector<core::Interface> *> payload {&interfaces, GetApiConfig(), GetResource()};

    GetApiConfig()->cIapi_->interfaceEnumerateSuperInterfaces(
        GetView(), &payload, [](AbckitCoreInterface *iface, void *data) {
            const auto &payload = *static_cast<Payload<std::vector<core::Interface> *> *>(data);
            payload.data->push_back(core::Interface(iface, payload.config, payload.resource));
            return true;
        });

    CheckError(GetApiConfig());

    return interfaces;
}

inline std::vector<core::Interface> Interface::GetSubInterfaces() const
{
    std::vector<core::Interface> interfaces;
    Payload<std::vector<core::Interface> *> payload {&interfaces, GetApiConfig(), GetResource()};

    GetApiConfig()->cIapi_->interfaceEnumerateSubInterfaces(
        GetView(), &payload, [](AbckitCoreInterface *iface, void *data) {
            const auto &payload = *static_cast<Payload<std::vector<core::Interface> *> *>(data);
            payload.data->push_back(core::Interface(iface, payload.config, payload.resource));
            return true;
        });

    CheckError(GetApiConfig());

    return interfaces;
}

inline std::vector<core::Class> Interface::GetClasses() const
{
    std::vector<core::Class> classes;
    Payload<std::vector<core::Class> *> payload {&classes, GetApiConfig(), GetResource()};

    GetApiConfig()->cIapi_->interfaceEnumerateClasses(GetView(), &payload, [](AbckitCoreClass *klass, void *data) {
        const auto &payload = *static_cast<Payload<std::vector<core::Class> *> *>(data);
        payload.data->push_back(core::Class(klass, payload.config, payload.resource));
        return true;
    });

    CheckError(GetApiConfig());

    return classes;
}

inline std::vector<core::Annotation> Interface::GetAnnotations() const
{
    std::vector<core::Annotation> anns;
    Payload<std::vector<core::Annotation> *> payload {&anns, GetApiConfig(), GetResource()};

    GetApiConfig()->cIapi_->interfaceEnumerateAnnotations(
        GetView(), &payload, [](AbckitCoreAnnotation *method, void *data) {
            const auto &payload = *static_cast<Payload<std::vector<core::Annotation> *> *>(data);
            payload.data->push_back(core::Annotation(method, payload.config, payload.resource));
            return true;
        });

    CheckError(GetApiConfig());
    return anns;
}
}  // namespace abckit::core

#endif  // CPP_ABCKIT_CORE_INTERFACE_IMPL_H
