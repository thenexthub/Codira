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

#ifndef LIBABCKIT_CORE_FIELD_IMPL_H
#define LIBABCKIT_CORE_FIELD_IMPL_H

#include "field.h"

namespace abckit::core {

// ========================================
// Module Field
// ========================================

inline Module ModuleField::GetModule() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitCoreModule *module = conf->cIapi_->moduleFieldGetModule(GetView());
    CheckError(conf);
    return Module(module, conf, GetResource());
}

inline std::string ModuleField::GetName() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitString *cString = conf->cIapi_->moduleFieldGetName(GetView());
    CheckError(conf);
    std::string str = conf->cIapi_->abckitStringToString(cString);
    CheckError(conf);
    return str;
}

inline Type ModuleField::GetType() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitType *type = conf->cIapi_->moduleFieldGetType(GetView());
    CheckError(conf);
    return Type(type, conf, GetResource());
}

inline Value ModuleField::GetValue() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitValue *value = conf->cIapi_->moduleFieldGetValue(GetView());
    CheckError(conf);
    return Value(value, conf, GetResource());
}

// ========================================
// Namesapce Field
// ========================================

inline Namespace NamespaceField::GetNamespace() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitCoreNamespace *ns = conf->cIapi_->namespaceFieldGetNamespace(GetView());
    CheckError(conf);
    return Namespace(ns, conf, GetResource());
}

inline std::string NamespaceField::GetName() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitString *cString = conf->cIapi_->namespaceFieldGetName(GetView());
    CheckError(conf);
    std::string str = conf->cIapi_->abckitStringToString(cString);
    CheckError(conf);
    return str;
}

inline Type NamespaceField::GetType() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitType *type = conf->cIapi_->namespaceFieldGetType(GetView());
    CheckError(conf);
    return Type(type, conf, GetResource());
}

// ========================================
// Class Field
// ========================================

inline Class ClassField::GetClass() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitCoreClass *klass = conf->cIapi_->classFieldGetClass(GetView());
    CheckError(conf);
    return Class(klass, conf, GetResource());
}

inline std::string ClassField::GetName() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitString *cString = conf->cIapi_->classFieldGetName(GetView());
    CheckError(conf);
    std::string str = conf->cIapi_->abckitStringToString(cString);
    CheckError(conf);
    return str;
}

inline Type ClassField::GetType() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitType *type = conf->cIapi_->classFieldGetType(GetView());
    CheckError(conf);
    return Type(type, conf, GetResource());
}

inline Value ClassField::GetValue() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitValue *value = conf->cIapi_->classFieldGetValue(GetView());
    CheckError(conf);
    return Value(value, conf, GetResource());
}

inline bool ClassField::IsPublic() const
{
    const ApiConfig *conf = GetApiConfig();
    bool res = conf->cIapi_->classFieldIsPublic(GetView());
    CheckError(conf);
    return res;
}

inline bool ClassField::IsProtected() const
{
    const ApiConfig *conf = GetApiConfig();
    bool res = conf->cIapi_->classFieldIsProtected(GetView());
    CheckError(conf);
    return res;
}

inline bool ClassField::IsPrivate() const
{
    const ApiConfig *conf = GetApiConfig();
    bool res = conf->cIapi_->classFieldIsPrivate(GetView());
    CheckError(conf);
    return res;
}

inline bool ClassField::IsInternal() const
{
    const ApiConfig *conf = GetApiConfig();
    bool res = conf->cIapi_->classFieldIsInternal(GetView());
    CheckError(conf);
    return res;
}

inline bool ClassField::IsStatic() const
{
    const ApiConfig *conf = GetApiConfig();
    bool res = conf->cIapi_->classFieldIsStatic(GetView());
    CheckError(conf);
    return res;
}

inline bool ClassField::IsFinal() const
{
    const ApiConfig *conf = GetApiConfig();
    bool res = conf->cIapi_->classFieldIsFinal(GetView());
    CheckError(conf);
    return res;
}

inline std::vector<core::Annotation> ClassField::GetAnnotations() const
{
    std::vector<core::Annotation> annotations;
    Payload<std::vector<core::Annotation> *> payload {&annotations, GetApiConfig(), GetResource()};
    GetApiConfig()->cIapi_->classFieldEnumerateAnnotations(
        GetView(), &payload, [](AbckitCoreAnnotation *anno, void *data) {
            const auto &payload = *static_cast<Payload<std::vector<core::Annotation> *> *>(data);
            payload.data->push_back(core::Annotation(anno, payload.config, payload.resource));
            return true;
        });
    CheckError(GetApiConfig());
    return annotations;
}

// ========================================
// Interface Field
// ========================================

inline Interface InterfaceField::GetInterface() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitCoreInterface *iface = conf->cIapi_->interfaceFieldGetInterface(GetView());
    CheckError(conf);
    return Interface(iface, conf, GetResource());
}

inline std::string InterfaceField::GetName() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitString *cString = conf->cIapi_->interfaceFieldGetName(GetView());
    CheckError(conf);
    std::string str = conf->cIapi_->abckitStringToString(cString);
    CheckError(conf);
    return str;
}

inline Type InterfaceField::GetType() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitType *type = conf->cIapi_->interfaceFieldGetType(GetView());
    CheckError(conf);
    return Type(type, conf, GetResource());
}

inline bool InterfaceField::IsReadonly() const
{
    const ApiConfig *conf = GetApiConfig();
    bool res = conf->cIapi_->interfaceFieldIsReadonly(GetView());
    CheckError(conf);
    return res;
}

inline std::vector<core::Annotation> InterfaceField::GetAnnotations() const
{
    std::vector<core::Annotation> annotations;
    Payload<std::vector<core::Annotation> *> payload {&annotations, GetApiConfig(), GetResource()};
    GetApiConfig()->cIapi_->interfaceFieldEnumerateAnnotations(
        GetView(), &payload, [](AbckitCoreAnnotation *anno, void *data) {
            const auto &payload = *static_cast<Payload<std::vector<core::Annotation> *> *>(data);
            payload.data->push_back(core::Annotation(anno, payload.config, payload.resource));
            return true;
        });
    CheckError(GetApiConfig());
    return annotations;
}

// ========================================
// Enum Field
// ========================================

inline Enum EnumField::GetEnum() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitCoreEnum *enm = conf->cIapi_->enumFieldGetEnum(GetView());
    CheckError(conf);
    return Enum(enm, conf, GetResource());
}

inline std::string EnumField::GetName() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitString *cString = conf->cIapi_->enumFieldGetName(GetView());
    CheckError(conf);
    std::string str = conf->cIapi_->abckitStringToString(cString);
    CheckError(conf);
    return str;
}

inline Type EnumField::GetType() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitType *type = conf->cIapi_->enumFieldGetType(GetView());
    CheckError(conf);
    return Type(type, conf, GetResource());
}

inline Value EnumField::GetValue() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitValue *value = conf->cIapi_->enumFieldGetValue(GetView());
    CheckError(conf);
    return Value(value, conf, GetResource());
}

}  // namespace abckit::core

#endif  // LIBABCKIT_CORE_FIELD_IMPL_H
