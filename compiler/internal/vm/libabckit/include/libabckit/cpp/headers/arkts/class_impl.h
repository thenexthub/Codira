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

#ifndef CPP_ABCKIT_ARKTS_CLASS_IMPL_H
#define CPP_ABCKIT_ARKTS_CLASS_IMPL_H

#include "annotation.h"
#include "annotation_interface.h"
#include "class.h"
#include "field.h"
#include "function.h"
#include "interface.h"
#include "module.h"

// NOLINTBEGIN(performance-unnecessary-value-param)
namespace abckit::arkts {

inline AbckitArktsClass *Class::TargetCast() const
{
    const auto ret = GetApiConfig()->cArktsIapi_->coreClassToArktsClass(GetView());
    CheckError(GetApiConfig());
    return ret;
}

inline Class::Class(const core::Class &coreOther) : core::Class(coreOther), targetChecker_(this) {}

inline bool Class::SetName(const std::string &name) const
{
    const auto ret = GetApiConfig()->cArktsMapi_->classSetName(TargetCast(), name.c_str());
    CheckError(GetApiConfig());
    return ret;
}

inline bool Class::RemoveField(arkts::ClassField field) const
{
    const auto ret = GetApiConfig()->cArktsMapi_->classRemoveField(TargetCast(), field.TargetCast());
    CheckError(GetApiConfig());
    return ret;
}

inline arkts::ClassField Class::AddField(const std::string_view name, const Type &type, const Value &value,
                                         bool isStatic, AbckitArktsFieldVisibility fieldVisibility)
{
    const struct AbckitArktsFieldCreateParams params {
        name.data(), type.GetView(), value.GetView(), isStatic, fieldVisibility
    };
    auto *arkClassField = GetApiConfig()->cArktsMapi_->classAddField(TargetCast(), &params);
    CheckError(GetApiConfig());
    auto *coreClassField = GetApiConfig()->cArktsIapi_->arktsClassFieldToCoreClassField(arkClassField);
    CheckError(GetApiConfig());
    return arkts::ClassField(core::ClassField(coreClassField, GetApiConfig(), GetResource()));
}

inline arkts::ClassField Class::AddField(const std::string_view name, const Type &type, bool isStatic,
                                         AbckitArktsFieldVisibility fieldVisibility)
{
    const struct AbckitArktsFieldCreateParams params {
        name.data(), type.GetView(), nullptr, isStatic, fieldVisibility
    };
    auto *arkClassField = GetApiConfig()->cArktsMapi_->classAddField(TargetCast(), &params);
    CheckError(GetApiConfig());
    auto *coreClassField = GetApiConfig()->cArktsIapi_->arktsClassFieldToCoreClassField(arkClassField);
    CheckError(GetApiConfig());
    return arkts::ClassField(core::ClassField(coreClassField, GetApiConfig(), GetResource()));
}

inline bool Class::SetOwningModule(const Module &module) const
{
    const auto ret = GetApiConfig()->cArktsMapi_->classSetOwningModule(TargetCast(), module.TargetCast());
    CheckError(GetApiConfig());
    return ret;
}

inline Class Class::RemoveAnnotation(arkts::Annotation anno) const
{
    GetApiConfig()->cArktsMapi_->classRemoveAnnotation(TargetCast(), anno.TargetCast());
    CheckError(GetApiConfig());
    return *this;
}

inline Annotation Class::AddAnnotation(AnnotationInterface ai)
{
    auto arktsCl = TargetCast();
    auto arktsAi = ai.TargetCast();
    const struct AbckitArktsAnnotationCreateParams params {
        arktsAi
    };
    auto arktsAnno = GetApiConfig()->cArktsMapi_->classAddAnnotation(arktsCl, &params);
    CheckError(GetApiConfig());
    auto coreAnno = GetApiConfig()->cArktsIapi_->arktsAnnotationToCoreAnnotation(arktsAnno);
    CheckError(GetApiConfig());
    return Annotation(core::Annotation(coreAnno, GetApiConfig(), GetResource()));
}

inline bool Class::AddInterface(arkts::Interface iface)
{
    const auto ret = GetApiConfig()->cArktsMapi_->classAddInterface(TargetCast(), iface.TargetCast());
    CheckError(GetApiConfig());
    return ret;
}

inline bool Class::RemoveInterface(arkts::Interface iface)
{
    const auto ret = GetApiConfig()->cArktsMapi_->classRemoveInterface(TargetCast(), iface.TargetCast());
    CheckError(GetApiConfig());
    return ret;
}

inline bool Class::SetSuperClass(arkts::Class superClass)
{
    const auto ret = GetApiConfig()->cArktsMapi_->classSetSuperClass(TargetCast(), superClass.TargetCast());
    CheckError(GetApiConfig());
    return ret;
}

inline bool Class::RemoveMethod(arkts::Function method)
{
    const auto ret = GetApiConfig()->cArktsMapi_->classRemoveMethod(TargetCast(), method.TargetCast());
    CheckError(GetApiConfig());
    return ret;
}

inline Class Class::CreateClass(Module m, const std::string &name)
{
    auto klass = m.GetApiConfig()->cArktsMapi_->createClass(m.TargetCast(), name.c_str());
    CheckError(m.GetApiConfig());

    auto coreClass =
        core::Class(m.GetApiConfig()->cArktsIapi_->arktsClassToCoreClass(klass), m.GetApiConfig(), m.GetResource());
    CheckError(m.GetApiConfig());

    return Class(coreClass);
}

}  // namespace abckit::arkts
// NOLINTEND(performance-unnecessary-value-param)

#endif  // CPP_ABCKIT_ARKTS_CLASS_IMPL_H
