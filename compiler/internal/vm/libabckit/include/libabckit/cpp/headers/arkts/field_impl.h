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

#ifndef CPP_ABCKIT_ARKTS_FIELD_IMPL_H
#define CPP_ABCKIT_ARKTS_FIELD_IMPL_H

#include "annotation.h"
#include "annotation_interface.h"
#include "field.h"

namespace abckit::arkts {

inline ModuleField::ModuleField(const core::ModuleField &coreOther) : core::ModuleField(coreOther), targetChecker_(this)
{
}

inline AbckitArktsModuleField *ModuleField::TargetCast() const
{
    const auto ret = GetApiConfig()->cArktsIapi_->coreModuleFieldToArktsModuleField(GetView());
    CheckError(GetApiConfig());
    return ret;
}

inline bool ModuleField::SetName(const std::string &name) const
{
    const auto ret = GetApiConfig()->cArktsMapi_->moduleFieldSetName(TargetCast(), name.c_str());
    CheckError(GetApiConfig());
    return ret;
}

inline bool ModuleField::SetType(const Type &type) const
{
    const auto ret = GetApiConfig()->cArktsMapi_->moduleFieldSetType(TargetCast(), type.GetView());
    CheckError(GetApiConfig());
    return ret;
}

inline bool ModuleField::SetValue(const Value &value) const
{
    const auto ret = GetApiConfig()->cArktsMapi_->moduleFieldSetValue(TargetCast(), value.GetView());
    CheckError(GetApiConfig());
    return ret;
}

inline NamespaceField::NamespaceField(const core::NamespaceField &coreOther)
    : core::NamespaceField(coreOther), targetChecker_(this)
{
}

inline AbckitArktsNamespaceField *NamespaceField::TargetCast() const
{
    const auto ret = GetApiConfig()->cArktsIapi_->coreNamespaceFieldToArktsNamespaceField(GetView());
    CheckError(GetApiConfig());
    return ret;
}

inline bool NamespaceField::SetName(const std::string &name) const
{
    const auto ret = GetApiConfig()->cArktsMapi_->namespaceFieldSetName(TargetCast(), name.c_str());
    CheckError(GetApiConfig());
    return ret;
}

inline ClassField::ClassField(const core::ClassField &coreOther) : core::ClassField(coreOther), targetChecker_(this) {}

inline AbckitArktsClassField *ClassField::TargetCast() const
{
    const auto ret = GetApiConfig()->cArktsIapi_->coreClassFieldToArktsClassField(GetView());
    CheckError(GetApiConfig());
    return ret;
}

inline bool ClassField::SetName(const std::string &name) const
{
    const auto ret = GetApiConfig()->cArktsMapi_->classFieldSetName(TargetCast(), name.c_str());
    CheckError(GetApiConfig());
    return ret;
}

inline bool ClassField::SetType(const Type &type) const
{
    const auto ret = GetApiConfig()->cArktsMapi_->classFieldSetType(TargetCast(), type.GetView());
    CheckError(GetApiConfig());
    return ret;
}

inline bool ClassField::SetValue(const Value &value) const
{
    const auto ret = GetApiConfig()->cArktsMapi_->classFieldSetValue(TargetCast(), value.GetView());
    CheckError(GetApiConfig());
    return ret;
}

inline bool ClassField::AddAnnotation(const AnnotationInterface &ai) const
{
    const struct AbckitArktsAnnotationCreateParams params {
        ai.TargetCast()
    };
    auto ret = GetApiConfig()->cArktsMapi_->classFieldAddAnnotation(TargetCast(), &params);
    CheckError(GetApiConfig());
    return ret;
}

inline bool ClassField::RemoveAnnotation(const arkts::Annotation &anno) const
{
    auto ret = GetApiConfig()->cArktsMapi_->classFieldRemoveAnnotation(TargetCast(), anno.TargetCast());
    CheckError(GetApiConfig());
    return ret;
}

inline InterfaceField::InterfaceField(const core::InterfaceField &coreOther)
    : core::InterfaceField(coreOther), targetChecker_(this)
{
}

inline AbckitArktsInterfaceField *InterfaceField::TargetCast() const
{
    const auto ret = GetApiConfig()->cArktsIapi_->coreInterfaceFieldToArktsInterfaceField(GetView());
    CheckError(GetApiConfig());
    return ret;
}

inline bool InterfaceField::SetName(const std::string &name) const
{
    const auto ret = GetApiConfig()->cArktsMapi_->interfaceFieldSetName(TargetCast(), name.c_str());
    CheckError(GetApiConfig());
    return ret;
}

inline bool InterfaceField::SetType(const Type &type) const
{
    const auto ret = GetApiConfig()->cArktsMapi_->interfaceFieldSetType(TargetCast(), type.GetView());
    CheckError(GetApiConfig());
    return ret;
}

inline bool InterfaceField::AddAnnotation(const AnnotationInterface &ai) const
{
    const struct AbckitArktsAnnotationCreateParams params {
        ai.TargetCast()
    };
    auto ret = GetApiConfig()->cArktsMapi_->interfaceFieldAddAnnotation(TargetCast(), &params);
    CheckError(GetApiConfig());
    return ret;
}

inline bool InterfaceField::RemoveAnnotation(const arkts::Annotation &anno) const
{
    auto ret = GetApiConfig()->cArktsMapi_->interfaceFieldRemoveAnnotation(TargetCast(), anno.TargetCast());
    CheckError(GetApiConfig());
    return ret;
}

inline EnumField::EnumField(const core::EnumField &coreOther) : core::EnumField(coreOther), targetChecker_(this) {}

inline AbckitArktsEnumField *EnumField::TargetCast() const
{
    const auto ret = GetApiConfig()->cArktsIapi_->coreEnumFieldToArktsEnumField(GetView());
    CheckError(GetApiConfig());
    return ret;
}

inline bool EnumField::SetName(const std::string &name) const
{
    const auto ret = GetApiConfig()->cArktsMapi_->enumFieldSetName(TargetCast(), name.c_str());
    CheckError(GetApiConfig());
    return ret;
}

}  // namespace abckit::arkts

#endif  // CPP_ABCKIT_ARKTS_FIELD_IMPL_H
