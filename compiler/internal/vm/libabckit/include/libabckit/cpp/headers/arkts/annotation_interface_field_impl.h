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

#ifndef CPP_ABCKIT_ARKTS_ANNOTATION_INTERFACE_FIELD_IMPL_H
#define CPP_ABCKIT_ARKTS_ANNOTATION_INTERFACE_FIELD_IMPL_H

#include "annotation_interface_field.h"

namespace abckit::arkts {

inline AbckitArktsAnnotationInterfaceField *AnnotationInterfaceField::TargetCast() const
{
    auto ret = GetApiConfig()->cArktsIapi_->coreAnnotationInterfaceFieldToArktsAnnotationInterfaceField(GetView());
    CheckError(GetApiConfig());
    return ret;
}

inline AnnotationInterfaceField::AnnotationInterfaceField(const core::AnnotationInterfaceField &coreOther)
    : core::AnnotationInterfaceField(coreOther), targetChecker_(this)
{
}

}  // namespace abckit::arkts

#endif  // CPP_ABCKIT_ARKTS_ANNOTATION_INTERFACE_FIELD_IMPL_H
