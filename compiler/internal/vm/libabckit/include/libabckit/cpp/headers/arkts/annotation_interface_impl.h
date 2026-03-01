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

#ifndef CPP_ABCKIT_ARKTS_ANNOTATION_INTERFACE_IMPL_H
#define CPP_ABCKIT_ARKTS_ANNOTATION_INTERFACE_IMPL_H

#include "annotation_interface.h"
#include "annotation_interface_field.h"
#include "../core/annotation_interface.h"
#include "../core/annotation_interface_field.h"

// NOLINTBEGIN(performance-unnecessary-value-param)
namespace abckit::arkts {

inline AbckitArktsAnnotationInterface *AnnotationInterface::TargetCast() const
{
    auto ret = GetApiConfig()->cArktsIapi_->coreAnnotationInterfaceToArktsAnnotationInterface(GetView());
    CheckError(GetApiConfig());
    return ret;
}

inline AnnotationInterface::AnnotationInterface(const core::AnnotationInterface &coreOther)
    : core::AnnotationInterface(coreOther), targetChecker_(this)
{
}

inline bool AnnotationInterface::SetName(const std::string &name) const
{
    const auto ret = GetApiConfig()->cArktsMapi_->annotationInterfaceSetName(TargetCast(), name.c_str());
    CheckError(GetApiConfig());
    return ret;
}

inline arkts::AnnotationInterfaceField AnnotationInterface::AddField(std::string_view name, Type type, Value value)
{
    const struct AbckitArktsAnnotationInterfaceFieldCreateParams params {
        name.data(), type.GetView(), value.GetView()
    };
    auto arktsAif = GetApiConfig()->cArktsMapi_->annotationInterfaceAddField(TargetCast(), &params);
    CheckError(GetApiConfig());
    auto coreAif = GetApiConfig()->cArktsIapi_->arktsAnnotationInterfaceFieldToCoreAnnotationInterfaceField(arktsAif);
    CheckError(GetApiConfig());
    return AnnotationInterfaceField(core::AnnotationInterfaceField(coreAif, GetApiConfig(), GetResource()));
}

inline AnnotationInterface AnnotationInterface::RemoveField(AnnotationInterfaceField field) const
{
    GetApiConfig()->cArktsMapi_->annotationInterfaceRemoveField(TargetCast(), field.TargetCast());
    CheckError(GetApiConfig());
    return *this;
}

}  // namespace abckit::arkts
// NOLINTEND(performance-unnecessary-value-param)

#endif  // CPP_ABCKIT_ARKTS_ANNOTATION_INTERFACE_IMPL_H
