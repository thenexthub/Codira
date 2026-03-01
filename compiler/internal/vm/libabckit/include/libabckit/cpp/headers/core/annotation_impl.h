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

#ifndef CPP_ABCKIT_CORE_ANNOTATION_IMPL_H
#define CPP_ABCKIT_CORE_ANNOTATION_IMPL_H

#include "annotation.h"
#include "annotation_element.h"
#include "annotation_interface.h"

namespace abckit::core {

inline std::string Annotation::GetName() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitString *cString = conf->cIapi_->annotationGetName(GetView());
    CheckError(conf);
    std::string str = conf->cIapi_->abckitStringToString(cString);
    CheckError(conf);
    return str;
}

inline bool Annotation::EnumerateElements(const std::function<bool(abckit::core::AnnotationElement)> &cb) const
{
    Payload<const std::function<bool(core::AnnotationElement)> &> payload {cb, GetApiConfig(), GetResource()};

    bool isNormalExit = GetApiConfig()->cIapi_->annotationEnumerateElements(
        GetView(), &payload, [](AbckitCoreAnnotationElement *func, void *data) {
            const auto &payload = *static_cast<Payload<const std::function<bool(core::AnnotationElement)> &> *>(data);
            return payload.data(core::AnnotationElement(func, payload.config, payload.resource));
        });
    CheckError(GetApiConfig());
    return isNormalExit;
}

inline core::AnnotationInterface Annotation::GetInterface() const
{
    AnnotationInterface iface(GetApiConfig()->cIapi_->annotationGetInterface(GetView()), GetApiConfig(), GetResource());
    CheckError(GetApiConfig());
    return iface;
}

inline bool Annotation::IsExternal() const
{
    auto res = GetApiConfig()->cIapi_->annotationIsExternal(GetView());
    CheckError(GetApiConfig());
    return res;
}

}  // namespace abckit::core

#endif  // CPP_ABCKIT_CORE_ANNOTATION_IMPL_H
