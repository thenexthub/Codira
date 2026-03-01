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

#ifndef CPP_ABCKIT_CORE_NAMESPACE_IMPL_H
#define CPP_ABCKIT_CORE_NAMESPACE_IMPL_H

#include "namespace.h"
#include "module.h"
#include "function.h"
#include "class.h"
#include "interface.h"
#include "enum.h"
#include "field.h"

namespace abckit::core {

inline core::Module Namespace::GetModule() const
{
    AbckitCoreModule *mod = GetApiConfig()->cIapi_->namespaceGetModule(GetView());
    CheckError(GetApiConfig());
    return Module(mod, GetApiConfig(), GetResource());
}

inline std::string Namespace::GetName() const
{
    AbckitString *abcName = GetApiConfig()->cIapi_->namespaceGetName(GetView());
    CheckError(GetApiConfig());
    std::string name = GetApiConfig()->cIapi_->abckitStringToString(abcName);
    CheckError(GetApiConfig());
    return name;
}

inline bool Namespace::IsExternal() const
{
    auto mod = GetApiConfig()->cIapi_->namespaceIsExternal(GetView());
    CheckError(GetApiConfig());
    return mod;
}

inline Namespace Namespace::GetParentNamespace() const
{
    AbckitCoreNamespace *parent = GetApiConfig()->cIapi_->namespaceGetParentNamespace(GetView());
    CheckError(GetApiConfig());
    return Namespace(parent, GetApiConfig(), GetResource());
}

inline std::vector<core::Class> Namespace::GetClasses() const
{
    std::vector<core::Class> classes;
    Payload<std::vector<core::Class> *> payload {&classes, GetApiConfig(), GetResource()};

    GetApiConfig()->cIapi_->namespaceEnumerateClasses(GetView(), &payload, [](AbckitCoreClass *klass, void *data) {
        const auto &payload = *static_cast<Payload<std::vector<core::Class> *> *>(data);
        payload.data->push_back(core::Class(klass, payload.config, payload.resource));
        return true;
    });

    CheckError(GetApiConfig());

    return classes;
}

inline std::vector<core::Interface> Namespace::GetInterfaces() const
{
    std::vector<core::Interface> interfaces;
    Payload<std::vector<core::Interface> *> payload {&interfaces, GetApiConfig(), GetResource()};

    GetApiConfig()->cIapi_->namespaceEnumerateInterfaces(
        GetView(), &payload, [](AbckitCoreInterface *iface, void *data) {
            const auto &payload = *static_cast<Payload<std::vector<core::Interface> *> *>(data);
            payload.data->push_back(core::Interface(iface, payload.config, payload.resource));
            return true;
        });

    CheckError(GetApiConfig());

    return interfaces;
}

inline std::vector<core::Enum> Namespace::GetEnums() const
{
    std::vector<core::Enum> enums;
    Payload<std::vector<core::Enum> *> payload {&enums, GetApiConfig(), GetResource()};

    GetApiConfig()->cIapi_->namespaceEnumerateEnums(GetView(), &payload, [](AbckitCoreEnum *enm, void *data) {
        const auto &payload = *static_cast<Payload<std::vector<core::Enum> *> *>(data);
        payload.data->push_back(core::Enum(enm, payload.config, payload.resource));
        return true;
    });

    CheckError(GetApiConfig());

    return enums;
}

inline std::vector<core::AnnotationInterface> Namespace::GetAnnotationInterfaces() const
{
    std::vector<core::AnnotationInterface> ifaces;
    Payload<std::vector<core::AnnotationInterface> *> payload {&ifaces, GetApiConfig(), GetResource()};

    GetApiConfig()->cIapi_->namespaceEnumerateAnnotationInterfaces(
        GetView(), &payload, [](AbckitCoreAnnotationInterface *func, void *data) {
            const auto &payload = *static_cast<Payload<std::vector<core::AnnotationInterface> *> *>(data);
            payload.data->push_back(core::AnnotationInterface(func, payload.config, payload.resource));
            return true;
        });

    CheckError(GetApiConfig());

    return ifaces;
}

inline std::vector<core::Function> Namespace::GetTopLevelFunctions() const
{
    std::vector<core::Function> functions;
    Payload<std::vector<core::Function> *> payload {&functions, GetApiConfig(), GetResource()};

    GetApiConfig()->cIapi_->namespaceEnumerateTopLevelFunctions(
        GetView(), &payload, [](AbckitCoreFunction *func, void *data) {
            const auto &payload = *static_cast<Payload<std::vector<core::Function> *> *>(data);
            payload.data->push_back(core::Function(func, payload.config, payload.resource));
            return true;
        });

    CheckError(GetApiConfig());

    return functions;
}

inline std::vector<core::NamespaceField> Namespace::GetFields() const
{
    std::vector<core::NamespaceField> fields;
    Payload<std::vector<core::NamespaceField> *> payload {&fields, GetApiConfig(), GetResource()};

    GetApiConfig()->cIapi_->namespaceEnumerateFields(
        GetView(), &payload, [](AbckitCoreNamespaceField *field, void *data) {
            const auto &payload = *static_cast<Payload<std::vector<core::NamespaceField> *> *>(data);
            payload.data->push_back(core::NamespaceField(field, payload.config, payload.resource));
            return true;
        });

    CheckError(GetApiConfig());

    return fields;
}

inline std::vector<core::Namespace> Namespace::GetNamespaces() const
{
    std::vector<core::Namespace> namespaces;
    Payload<std::vector<core::Namespace> *> payload {&namespaces, GetApiConfig(), GetResource()};

    GetApiConfig()->cIapi_->namespaceEnumerateNamespaces(
        GetView(), &payload, [](AbckitCoreNamespace *func, void *data) {
            const auto &payload = *static_cast<Payload<std::vector<core::Namespace> *> *>(data);
            payload.data->push_back(core::Namespace(func, payload.config, payload.resource));
            return true;
        });

    CheckError(GetApiConfig());

    return namespaces;
}

// CC-OFFNXT(G.FUD.06) perf critical
inline bool Namespace::EnumerateNamespaces(const std::function<bool(core::Namespace)> &cb) const
{
    Payload<const std::function<bool(core::Namespace)> &> payload {cb, GetApiConfig(), GetResource()};

    bool isNormalExit = GetApiConfig()->cIapi_->namespaceEnumerateNamespaces(
        GetView(), &payload, [](AbckitCoreNamespace *ns, void *data) {
            const auto &payload = *static_cast<Payload<const std::function<bool(core::Namespace)> &> *>(data);
            return payload.data(core::Namespace(ns, payload.config, payload.resource));
        });
    CheckError(GetApiConfig());
    return isNormalExit;
}

// CC-OFFNXT(G.FUD.06) perf critical
inline bool Namespace::EnumerateClasses(const std::function<bool(core::Class)> &cb) const
{
    Payload<const std::function<bool(core::Class)> &> payload {cb, GetApiConfig(), GetResource()};

    bool isNormalExit =
        GetApiConfig()->cIapi_->namespaceEnumerateClasses(GetView(), &payload, [](AbckitCoreClass *ns, void *data) {
            const auto &payload = *static_cast<Payload<const std::function<bool(core::Class)> &> *>(data);
            return payload.data(core::Class(ns, payload.config, payload.resource));
        });
    CheckError(GetApiConfig());
    return isNormalExit;
}

// CC-OFFNXT(G.FUD.06) perf critical
inline bool Namespace::EnumerateTopLevelFunctions(const std::function<bool(core::Function)> &cb) const
{
    Payload<const std::function<bool(core::Function)> &> payload {cb, GetApiConfig(), GetResource()};

    bool isNormalExit = GetApiConfig()->cIapi_->namespaceEnumerateTopLevelFunctions(
        GetView(), &payload, [](AbckitCoreFunction *func, void *data) {
            const auto &payload = *static_cast<Payload<const std::function<bool(core::Function)> &> *>(data);
            return payload.data(core::Function(func, payload.config, payload.resource));
        });
    CheckError(GetApiConfig());
    return isNormalExit;
}

}  // namespace abckit::core

#endif  // CPP_ABCKIT_CORE_NAMESPACE_IMPL_H
