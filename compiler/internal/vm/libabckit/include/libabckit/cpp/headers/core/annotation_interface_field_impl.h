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

#ifndef CPP_ABCKIT_CORE_ANNOTATION_INTERFACE_FIELD_IMPL_H
#define CPP_ABCKIT_CORE_ANNOTATION_INTERFACE_FIELD_IMPL_H

#include "annotation_interface_field.h"
#include "annotation_interface.h"
#include "../value.h"

namespace abckit::core {

inline const File *AnnotationInterfaceField::GetFile() const
{
    return GetResource();
}

inline std::string AnnotationInterfaceField::GetName() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitString *abcStr = conf->cIapi_->annotationInterfaceFieldGetName(GetView());
    CheckError(conf);
    std::string str = conf->cIapi_->abckitStringToString(abcStr);
    CheckError(conf);
    return str;
}

inline core::AnnotationInterface AnnotationInterfaceField::GetInterface() const
{
    auto *ai = GetApiConfig()->cIapi_->annotationInterfaceFieldGetInterface(GetView());
    CheckError(GetApiConfig());
    return AnnotationInterface(ai, GetApiConfig(), GetResource());
}

inline Type AnnotationInterfaceField::GetType() const
{
    auto t = GetApiConfig()->cIapi_->annotationInterfaceFieldGetType(GetView());
    CheckError(GetApiConfig());
    return Type(t, GetApiConfig(), GetResource());
}

inline Value AnnotationInterfaceField::GetDefaultValue() const
{
    auto dv = GetApiConfig()->cIapi_->annotationInterfaceFieldGetDefaultValue(GetView());
    CheckError(GetApiConfig());
    return Value(dv, GetApiConfig(), GetResource());
}

}  // namespace abckit::core

#endif  // CPP_ABCKIT_CORE_ANNOTATION_INTERFACE_FIELD_IMPL_H
