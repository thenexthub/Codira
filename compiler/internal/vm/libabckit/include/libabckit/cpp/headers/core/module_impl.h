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

#ifndef CPP_ABCKIT_CORE_MODULE_IMPL_H
#define CPP_ABCKIT_CORE_MODULE_IMPL_H

#include "module.h"
#include "class.h"
#include "function.h"
#include "annotation_interface.h"
#include "namespace.h"
#include "import_descriptor.h"
#include "export_descriptor.h"

namespace abckit::core {

inline const File *Module::GetFile() const
{
    return GetResource();
}

inline enum AbckitTarget Module::GetTarget() const
{
    auto tar = GetApiConfig()->cIapi_->moduleGetTarget(GetView());
    CheckError(GetApiConfig());
    return tar;
}

inline bool Module::IsExternal() const
{
    auto mod = GetApiConfig()->cIapi_->moduleIsExternal(GetView());
    CheckError(GetApiConfig());
    return mod;
}

inline std::string Module::GetName() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitString *cString = conf->cIapi_->moduleGetName(GetView());
    CheckError(conf);
    std::string str = conf->cIapi_->abckitStringToString(cString);
    CheckError(conf);
    return str;
}

inline std::vector<core::Class> Module::GetClasses() const
{
    std::vector<core::Class> classes;
    Payload<std::vector<core::Class> *> payload {&classes, GetApiConfig(), GetResource()};

    GetApiConfig()->cIapi_->moduleEnumerateClasses(GetView(), &payload, [](AbckitCoreClass *klass, void *data) {
        const auto &payload = *static_cast<Payload<std::vector<core::Class> *> *>(data);
        payload.data->push_back(core::Class(klass, payload.config, payload.resource));
        return true;
    });

    CheckError(GetApiConfig());

    return classes;
}

inline std::vector<core::Interface> Module::GetInterfaces() const
{
    std::vector<core::Interface> interfaces;
    Payload<std::vector<core::Interface> *> payload {&interfaces, GetApiConfig(), GetResource()};

    GetApiConfig()->cIapi_->moduleEnumerateInterfaces(GetView(), &payload, [](AbckitCoreInterface *iface, void *data) {
        const auto &payload = *static_cast<Payload<std::vector<core::Interface> *> *>(data);
        payload.data->push_back(core::Interface(iface, payload.config, payload.resource));
        return true;
    });

    CheckError(GetApiConfig());

    return interfaces;
}

inline std::vector<core::Enum> Module::GetEnums() const
{
    std::vector<core::Enum> enums;
    Payload<std::vector<core::Enum> *> payload {&enums, GetApiConfig(), GetResource()};

    GetApiConfig()->cIapi_->moduleEnumerateEnums(GetView(), &payload, [](AbckitCoreEnum *enm, void *data) {
        const auto &payload = *static_cast<Payload<std::vector<core::Enum> *> *>(data);
        payload.data->push_back(core::Enum(enm, payload.config, payload.resource));
        return true;
    });

    CheckError(GetApiConfig());

    return enums;
}

inline std::vector<core::Function> Module::GetTopLevelFunctions() const
{
    std::vector<core::Function> functions;
    Payload<std::vector<core::Function> *> payload {&functions, GetApiConfig(), GetResource()};

    GetApiConfig()->cIapi_->moduleEnumerateTopLevelFunctions(
        GetView(), &payload, [](AbckitCoreFunction *func, void *data) {
            const auto &payload = *static_cast<Payload<std::vector<core::Function> *> *>(data);
            payload.data->push_back(core::Function(func, payload.config, payload.resource));
            return true;
        });

    CheckError(GetApiConfig());

    return functions;
}

inline std::vector<core::ModuleField> Module::GetFields() const
{
    std::vector<core::ModuleField> fields;
    Payload<std::vector<core::ModuleField> *> payload {&fields, GetApiConfig(), GetResource()};

    GetApiConfig()->cIapi_->moduleEnumerateFields(GetView(), &payload, [](AbckitCoreModuleField *field, void *data) {
        const auto &payload = *static_cast<Payload<std::vector<core::ModuleField> *> *>(data);
        payload.data->push_back(core::ModuleField(field, payload.config, payload.resource));
        return true;
    });

    CheckError(GetApiConfig());

    return fields;
}

inline std::vector<core::AnnotationInterface> Module::GetAnnotationInterfaces() const
{
    std::vector<core::AnnotationInterface> ifaces;
    Payload<std::vector<core::AnnotationInterface> *> payload {&ifaces, GetApiConfig(), GetResource()};

    GetApiConfig()->cIapi_->moduleEnumerateAnnotationInterfaces(
        GetView(), &payload, [](AbckitCoreAnnotationInterface *func, void *data) {
            const auto &payload = *static_cast<Payload<std::vector<core::AnnotationInterface> *> *>(data);
            payload.data->push_back(core::AnnotationInterface(func, payload.config, payload.resource));
            return true;
        });

    CheckError(GetApiConfig());

    return ifaces;
}

inline std::vector<core::Namespace> Module::GetNamespaces() const
{
    std::vector<core::Namespace> namespaces;
    Payload<std::vector<core::Namespace> *> payload {&namespaces, GetApiConfig(), GetResource()};

    GetApiConfig()->cIapi_->moduleEnumerateNamespaces(GetView(), &payload, [](AbckitCoreNamespace *func, void *data) {
        const auto &payload = *static_cast<Payload<std::vector<core::Namespace> *> *>(data);
        payload.data->push_back(core::Namespace(func, payload.config, payload.resource));
        return true;
    });

    CheckError(GetApiConfig());

    return namespaces;
}

inline std::vector<core::ImportDescriptor> Module::GetImports() const
{
    std::vector<core::ImportDescriptor> imports;
    Payload<std::vector<core::ImportDescriptor> *> payload {&imports, GetApiConfig(), GetResource()};

    GetApiConfig()->cIapi_->moduleEnumerateImports(
        GetView(), &payload, [](AbckitCoreImportDescriptor *func, void *data) {
            const auto &payload = *static_cast<Payload<std::vector<core::ImportDescriptor> *> *>(data);
            payload.data->push_back(core::ImportDescriptor(func, payload.config, payload.resource));
            return true;
        });

    CheckError(GetApiConfig());

    return imports;
}

inline std::vector<core::ExportDescriptor> Module::GetExports() const
{
    std::vector<core::ExportDescriptor> exports;
    Payload<std::vector<core::ExportDescriptor> *> payload {&exports, GetApiConfig(), GetResource()};

    GetApiConfig()->cIapi_->moduleEnumerateExports(
        GetView(), &payload, [](AbckitCoreExportDescriptor *func, void *data) {
            const auto &payload = *static_cast<Payload<std::vector<core::ExportDescriptor> *> *>(data);
            payload.data->push_back(core::ExportDescriptor(func, payload.config, payload.resource));
            return true;
        });

    CheckError(GetApiConfig());

    return exports;
}

// CC-OFFNXT(G.FUD.06) perf critical
inline bool Module::EnumerateNamespaces(const std::function<bool(core::Namespace)> &cb) const
{
    Payload<const std::function<bool(core::Namespace)> &> payload {cb, GetApiConfig(), GetResource()};

    auto isNormalExit =
        GetApiConfig()->cIapi_->moduleEnumerateNamespaces(GetView(), &payload, [](AbckitCoreNamespace *ns, void *data) {
            const auto &payload = *static_cast<Payload<const std::function<bool(core::Namespace)> &> *>(data);
            return payload.data(core::Namespace(ns, payload.config, payload.resource));
        });
    CheckError(GetApiConfig());
    return isNormalExit;
}

inline bool Module::EnumerateTopLevelFunctions(const std::function<bool(core::Function)> &cb) const
{
    Payload<const std::function<bool(core::Function)> &> payload {cb, GetApiConfig(), GetResource()};

    auto isNormalExit = GetApiConfig()->cIapi_->moduleEnumerateTopLevelFunctions(
        GetView(), &payload, [](AbckitCoreFunction *func, void *data) {
            const auto &payload = *static_cast<Payload<const std::function<bool(core::Function)> &> *>(data);
            return payload.data(core::Function(func, payload.config, payload.resource));
        });
    CheckError(GetApiConfig());
    return isNormalExit;
}

inline bool Module::EnumerateClasses(const std::function<bool(core::Class)> &cb) const
{
    Payload<const std::function<bool(core::Class)> &> payload {cb, GetApiConfig(), GetResource()};

    auto isNormalExit =
        GetApiConfig()->cIapi_->moduleEnumerateClasses(GetView(), &payload, [](AbckitCoreClass *klass, void *data) {
            const auto &payload = *static_cast<Payload<const std::function<bool(core::Class)> &> *>(data);
            return payload.data(core::Class(klass, payload.config, payload.resource));
        });
    CheckError(GetApiConfig());
    return isNormalExit;
}

inline bool Module::EnumerateInterfaces(const std::function<bool(core::Interface)> &cb) const
{
    Payload<const std::function<bool(core::Interface)> &> payload {cb, GetApiConfig(), GetResource()};

    auto isNormalExit = GetApiConfig()->cIapi_->moduleEnumerateInterfaces(
        GetView(), &payload, [](AbckitCoreInterface *iface, void *data) {
            const auto &payload = *static_cast<Payload<const std::function<bool(core::Interface)> &> *>(data);
            return payload.data(core::Interface(iface, payload.config, payload.resource));
        });
    CheckError(GetApiConfig());
    return isNormalExit;
}

inline bool Module::EnumerateEnums(const std::function<bool(core::Enum)> &cb) const
{
    Payload<const std::function<bool(core::Enum)> &> payload {cb, GetApiConfig(), GetResource()};

    auto isNormalExit =
        GetApiConfig()->cIapi_->moduleEnumerateEnums(GetView(), &payload, [](AbckitCoreEnum *enm, void *data) {
            const auto &payload = *static_cast<Payload<const std::function<bool(core::Enum)> &> *>(data);
            return payload.data(core::Enum(enm, payload.config, payload.resource));
        });
    CheckError(GetApiConfig());
    return isNormalExit;
}

inline bool Module::EnumerateImports(const std::function<bool(core::ImportDescriptor)> &cb) const
{
    Payload<const std::function<bool(core::ImportDescriptor)> &> payload {cb, GetApiConfig(), GetResource()};

    auto isNormalExit = GetApiConfig()->cIapi_->moduleEnumerateImports(
        GetView(), &payload, [](AbckitCoreImportDescriptor *func, void *data) {
            const auto &payload = *static_cast<Payload<const std::function<bool(core::ImportDescriptor)> &> *>(data);
            return payload.data(core::ImportDescriptor(func, payload.config, payload.resource));
        });
    CheckError(GetApiConfig());
    return isNormalExit;
}

inline bool Module::EnumerateAnonymousFunctions(const std::function<bool(core::Function)> &cb) const
{
    Payload<const std::function<bool(core::Function)> &> payload {cb, GetApiConfig(), GetResource()};

    auto isNormalExit = GetApiConfig()->cIapi_->moduleEnumerateAnonymousFunctions(
        GetView(), &payload, [](AbckitCoreFunction *func, void *data) {
            const auto &payload = *static_cast<Payload<const std::function<bool(core::Function)> &> *>(data);
            return payload.data(core::Function(func, payload.config, payload.resource));
        });
    CheckError(GetApiConfig());
    return isNormalExit;
}

inline bool Module::EnumerateExports(const std::function<bool(core::ExportDescriptor)> &cb) const
{
    Payload<const std::function<bool(core::ExportDescriptor)> &> payload {cb, GetApiConfig(), GetResource()};

    auto isNormalExit = GetApiConfig()->cIapi_->moduleEnumerateExports(
        GetView(), &payload, [](AbckitCoreExportDescriptor *desc, void *data) {
            const auto &payload = *static_cast<Payload<const std::function<bool(core::ExportDescriptor)> &> *>(data);
            return payload.data(core::ExportDescriptor(desc, payload.config, payload.resource));
        });
    CheckError(GetApiConfig());
    return isNormalExit;
}

inline bool Module::EnumerateAnnotationInterfaces(const std::function<bool(core::AnnotationInterface)> &cb) const
{
    Payload<const std::function<bool(core::AnnotationInterface)> &> payload {cb, GetApiConfig(), GetResource()};

    auto isNormalExit = GetApiConfig()->cIapi_->moduleEnumerateAnnotationInterfaces(
        GetView(), &payload, [](AbckitCoreAnnotationInterface *desc, void *data) {
            const auto &payload = *static_cast<Payload<const std::function<bool(core::AnnotationInterface)> &> *>(data);
            return payload.data(core::AnnotationInterface(desc, payload.config, payload.resource));
        });
    CheckError(GetApiConfig());
    return isNormalExit;
}

}  // namespace abckit::core

#endif  // CPP_ABCKIT_CORE_MODULE_IMPL_H
