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

#ifndef CPP_ABCKIT_ARKTS_FUNCTION_IMPL_H
#define CPP_ABCKIT_ARKTS_FUNCTION_IMPL_H

#include "function.h"
#include "annotation.h"
#include "../core/annotation.h"
#include "annotation_interface.h"

// NOLINTBEGIN(performance-unnecessary-value-param)
namespace abckit::arkts {

inline AbckitArktsFunction *Function::TargetCast() const
{
    auto ret = GetApiConfig()->cArktsIapi_->coreFunctionToArktsFunction(GetView());
    CheckError(GetApiConfig());
    return ret;
}

inline Function::Function(const core::Function &other) : core::Function(other), targetChecker_(this) {};

inline bool Function::IsNative() const
{
    const auto ret = GetApiConfig()->cArktsIapi_->functionIsNative(TargetCast());
    CheckError(GetApiConfig());
    return ret;
}

inline bool Function::IsAbstract() const
{
    const auto ret = GetApiConfig()->cArktsIapi_->functionIsAbstract(TargetCast());
    CheckError(GetApiConfig());
    return ret;
}

inline bool Function::IsFinal() const
{
    const auto ret = GetApiConfig()->cArktsIapi_->functionIsFinal(TargetCast());
    CheckError(GetApiConfig());
    return ret;
}

inline bool Function::IsAsync() const
{
    const auto ret = GetApiConfig()->cArktsIapi_->functionIsAsync(TargetCast());
    CheckError(GetApiConfig());
    return ret;
}

inline Annotation Function::AddAnnotation(AnnotationInterface ai) const
{
    const struct AbckitArktsAnnotationCreateParams params {
        ai.TargetCast()
    };
    auto arktsAnno = GetApiConfig()->cArktsMapi_->functionAddAnnotation(TargetCast(), &params);
    CheckError(GetApiConfig());
    auto coreAnno = GetApiConfig()->cArktsIapi_->arktsAnnotationToCoreAnnotation(arktsAnno);
    CheckError(GetApiConfig());
    return Annotation(core::Annotation(coreAnno, GetApiConfig(), GetResource()));
}

inline Function Function::RemoveAnnotation(Annotation anno) const
{
    GetApiConfig()->cArktsMapi_->functionRemoveAnnotation(TargetCast(), anno.TargetCast());
    CheckError(GetApiConfig());
    return *this;
}

inline bool Function::SetName(const std::string &name) const
{
    const auto ret = GetApiConfig()->cArktsMapi_->functionSetName(TargetCast(), name.c_str());
    CheckError(GetApiConfig());
    return ret;
}

}  // namespace abckit::arkts
// NOLINTEND(performance-unnecessary-value-param)

#endif  // CPP_ABCKIT_ARKTS_FUNCTION_IMPL_H
