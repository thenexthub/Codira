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

#ifndef CPP_ABCKIT_CORE_CLASS_IMPL_H
#define CPP_ABCKIT_CORE_CLASS_IMPL_H

#include "class.h"
#include "annotation.h"
#include "function.h"
#include "module.h"

namespace abckit::core {

inline const File *Class::GetFile() const
{
    return GetResource();
}

inline std::string Class::GetName() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitString *cString = conf->cIapi_->classGetName(GetView());
    CheckError(conf);
    std::string str = conf->cIapi_->abckitStringToString(cString);
    CheckError(conf);
    return str;
}

inline bool Class::IsExternal() const
{
    const auto isExternal = GetApiConfig()->cIapi_->classIsExternal(GetView());
    CheckError(GetApiConfig());
    return isExternal;
}

inline bool Class::IsFinal() const
{
    const auto isFinal = GetApiConfig()->cIapi_->classIsFinal(GetView());
    CheckError(GetApiConfig());
    return isFinal;
}

inline bool Class::IsAbstract() const
{
    const auto isAbstract = GetApiConfig()->cIapi_->classIsAbstract(GetView());
    CheckError(GetApiConfig());
    return isAbstract;
}

// CC-OFFNXT(G.FUD.06) perf critical
inline std::vector<core::Function> Class::GetAllMethods() const
{
    std::vector<core::Function> methods;
    Payload<std::vector<core::Function> *> payload {&methods, GetApiConfig(), GetResource()};

    GetApiConfig()->cIapi_->classEnumerateMethods(GetView(), &payload, [](AbckitCoreFunction *method, void *data) {
        const auto &payload = *static_cast<Payload<std::vector<core::Function> *> *>(data);
        payload.data->push_back(core::Function(method, payload.config, payload.resource));
        return true;
    });

    CheckError(GetApiConfig());

    return methods;
}

inline std::vector<core::ClassField> Class::GetFields() const
{
    std::vector<core::ClassField> fields;
    Payload<std::vector<core::ClassField> *> payload {&fields, GetApiConfig(), GetResource()};

    GetApiConfig()->cIapi_->classEnumerateFields(GetView(), &payload, [](AbckitCoreClassField *field, void *data) {
        const auto &payload = *static_cast<Payload<std::vector<core::ClassField> *> *>(data);
        payload.data->push_back(core::ClassField(field, payload.config, payload.resource));
        return true;
    });

    CheckError(GetApiConfig());

    return fields;
}

// CC-OFFNXT(G.FUD.06) perf critical
inline std::vector<core::Annotation> Class::GetAnnotations() const
{
    std::vector<core::Annotation> anns;

    Payload<std::vector<core::Annotation> *> payload {&anns, GetApiConfig(), GetResource()};

    GetApiConfig()->cIapi_->classEnumerateAnnotations(
        GetView(), &payload, [](AbckitCoreAnnotation *method, void *data) {
            const auto &payload = *static_cast<Payload<std::vector<core::Annotation> *> *>(data);
            payload.data->push_back(core::Annotation(method, payload.config, payload.resource));
            return true;
        });

    CheckError(GetApiConfig());

    return anns;
}

inline core::Class Class::GetSuperClass() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitCoreClass *klass = conf->cIapi_->classGetSuperClass(GetView());
    CheckError(conf);
    return core::Class(klass, conf_, GetResource());
}

inline std::vector<Class> Class::GetSubClasses() const
{
    std::vector<core::Class> classes;
    Payload<std::vector<core::Class> *> payload {&classes, GetApiConfig(), GetResource()};

    GetApiConfig()->cIapi_->classEnumerateSubClasses(GetView(), &payload, [](AbckitCoreClass *klass, void *data) {
        const auto &payload = *static_cast<Payload<std::vector<core::Class> *> *>(data);
        payload.data->push_back(core::Class(klass, payload.config, payload.resource));
        return true;
    });

    CheckError(GetApiConfig());

    return classes;
}

inline std::vector<Interface> Class::GetInterfaces() const
{
    std::vector<Interface> interfaces;
    Payload<std::vector<core::Interface> *> payload {&interfaces, GetApiConfig(), GetResource()};

    GetApiConfig()->cIapi_->classEnumerateInterfaces(GetView(), &payload, [](AbckitCoreInterface *iface, void *data) {
        const auto &payload = *static_cast<Payload<std::vector<core::Interface> *> *>(data);
        payload.data->push_back(core::Interface(iface, payload.config, payload.resource));
        return true;
    });

    CheckError(GetApiConfig());

    return interfaces;
}

// CC-OFFNXT(G.FUD.06) perf critical
inline bool Class::EnumerateMethods(const std::function<bool(core::Function)> &cb) const
{
    Payload<const std::function<bool(core::Function)> &> payload {cb, GetApiConfig(), GetResource()};

    bool isNormalExit = GetApiConfig()->cIapi_->classEnumerateMethods(
        GetView(), &payload, [](AbckitCoreFunction *method, void *data) -> bool {
            const auto &payload = *static_cast<Payload<const std::function<bool(core::Function)> &> *>(data);
            return payload.data(core::Function(method, payload.config, payload.resource));
        });
    CheckError(GetApiConfig());
    return isNormalExit;
}

inline bool Class::EnumerateAnnotations(const std::function<bool(core::Annotation)> &cb) const
{
    Payload<const std::function<bool(core::Annotation)> &> payload {cb, GetApiConfig(), GetResource()};

    bool isNormalExit = GetApiConfig()->cIapi_->classEnumerateAnnotations(
        GetView(), &payload, [](AbckitCoreAnnotation *method, void *data) {
            const auto &payload = *static_cast<Payload<const std::function<bool(core::Annotation)> &> *>(data);
            return payload.data(core::Annotation(method, payload.config, payload.resource));
        });

    CheckError(GetApiConfig());
    return isNormalExit;
}

inline Module Class::GetModule() const
{
    AbckitCoreModule *mod = GetApiConfig()->cIapi_->classGetModule(GetView());
    CheckError(GetApiConfig());
    return Module(mod, GetApiConfig(), GetResource());
}

inline Function Class::GetParentFunction() const
{
    AbckitCoreFunction *func = GetApiConfig()->cIapi_->classGetParentFunction(GetView());
    CheckError(GetApiConfig());
    return Function(func, GetApiConfig(), GetResource());
}

inline Namespace Class::GetParentNamespace() const
{
    AbckitCoreNamespace *ns = GetApiConfig()->cIapi_->classGetParentNamespace(GetView());
    CheckError(GetApiConfig());
    return Namespace(ns, GetApiConfig(), GetResource());
}

}  // namespace abckit::core

#endif  // CPP_ABCKIT_CORE_CLASS_IMPL_H
