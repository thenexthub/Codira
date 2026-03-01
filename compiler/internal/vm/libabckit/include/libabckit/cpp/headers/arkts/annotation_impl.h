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

#ifndef CPP_ABCKIT_ARKTS_ANNOTATION_IMPL_H
#define CPP_ABCKIT_ARKTS_ANNOTATION_IMPL_H

#include "annotation.h"
#include "annotation_element.h"
#include "../value.h"
#include "../core/annotation_element.h"

// NOLINTBEGIN(performance-unnecessary-value-param)
namespace abckit::arkts {

inline AbckitArktsAnnotation *Annotation::TargetCast() const
{
    auto ret = GetApiConfig()->cArktsIapi_->coreAnnotationToArktsAnnotation(GetView());
    CheckError(GetApiConfig());
    return ret;
}

inline Annotation::Annotation(const core::Annotation &coreOther) : core::Annotation(coreOther), targetChecker_(this) {}

inline bool Annotation::SetName(const std::string &name) const
{
    const auto ret = GetApiConfig()->cArktsMapi_->annotationSetName(TargetCast(), name.c_str());
    CheckError(GetApiConfig());
    return ret;
}

inline arkts::Annotation Annotation::AddElement(const abckit::Value &val, std::string_view name) const
{
    struct AbckitArktsAnnotationElementCreateParams params {
        name.data(), val.GetView()
    };
    GetApiConfig()->cArktsMapi_->annotationAddAnnotationElement(TargetCast(), &params);
    CheckError(GetApiConfig());
    return *this;
}

inline AbckitCoreAnnotationElement *Annotation::AddAndGetElementImpl(
    AbckitArktsAnnotationElementCreateParams *params) const
{
    AbckitArktsAnnotationElement *arktsAnni =
        GetApiConfig()->cArktsMapi_->annotationAddAnnotationElement(TargetCast(), params);
    CheckError(GetApiConfig());
    AbckitCoreAnnotationElement *coreAnni =
        GetApiConfig()->cArktsIapi_->arktsAnnotationElementToCoreAnnotationElement(arktsAnni);
    CheckError(GetApiConfig());
    return coreAnni;
}

inline arkts::AnnotationElement Annotation::AddAndGetElement(const abckit::Value &val, std::string_view name) const
{
    struct AbckitArktsAnnotationElementCreateParams params {
        name.data(), val.GetView()
    };
    auto *coreAnni = AddAndGetElementImpl(&params);
    core::AnnotationElement element(coreAnni, GetApiConfig(), GetResource());
    return arkts::AnnotationElement(element);
}

inline AnnotationElement Annotation::AddAnnotationElement(std::string_view name, Value value) const
{
    struct AbckitArktsAnnotationElementCreateParams params {
        name.data(), value.GetView()
    };

    auto arktsAnnoElem = GetApiConfig()->cArktsMapi_->annotationAddAnnotationElement(TargetCast(), &params);
    CheckError(GetApiConfig());
    auto coreAnnoElenm = GetApiConfig()->cArktsIapi_->arktsAnnotationElementToCoreAnnotationElement(arktsAnnoElem);
    CheckError(GetApiConfig());
    return AnnotationElement(core::AnnotationElement(coreAnnoElenm, GetApiConfig(), GetResource()));
}

inline Annotation Annotation::RemoveAnnotationElement(AnnotationElement elem) const
{
    GetApiConfig()->cArktsMapi_->annotationRemoveAnnotationElement(TargetCast(), elem.TargetCast());
    CheckError(GetApiConfig());
    return *this;
}

}  // namespace abckit::arkts
// NOLINTEND(performance-unnecessary-value-param)

#endif  // CPP_ABCKIT_ARKTS_ANNOTATION_IMPL_H
