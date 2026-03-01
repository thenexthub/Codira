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

#ifndef CPP_ABCKIT_CORE_ANNOTATION_ELEMENT_IMPL_H
#define CPP_ABCKIT_CORE_ANNOTATION_ELEMENT_IMPL_H

#include "annotation_element.h"
#include "annotation.h"

namespace abckit::core {

inline const File *AnnotationElement::GetFile() const
{
    return GetResource();
}

inline std::string AnnotationElement::GetName() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitString *cString = conf->cIapi_->annotationElementGetName(GetView());
    CheckError(conf);
    std::string str = conf->cIapi_->abckitStringToString(cString);
    CheckError(conf);
    return str;
}

inline abckit::Value AnnotationElement::GetValue() const
{
    auto value = GetApiConfig()->cIapi_->annotationElementGetValue(GetView());
    CheckError(GetApiConfig());
    return Value(value, GetApiConfig(), GetResource());
}

inline Annotation AnnotationElement::GetAnnotation() const
{
    auto anno = GetApiConfig()->cIapi_->annotationElementGetAnnotation(GetView());
    CheckError(GetApiConfig());
    return Annotation(anno, GetApiConfig(), GetResource());
}

}  // namespace abckit::core

#endif  // CPP_ABCKIT_CORE_ANNOTATION_ELEMENT_IMPL_H
