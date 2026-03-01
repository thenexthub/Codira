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

#ifndef CPP_ABCKIT_ARKTS_NAMESPACE_IMPL_H
#define CPP_ABCKIT_ARKTS_NAMESPACE_IMPL_H

#include "namespace.h"

namespace abckit::arkts {

inline AbckitArktsNamespace *Namespace::TargetCast() const
{
    auto ret = GetApiConfig()->cArktsIapi_->coreNamespaceToArktsNamespace(GetView());
    CheckError(GetApiConfig());
    return ret;
}

inline Namespace::Namespace(const core::Namespace &other) : core::Namespace(other), targetChecker_(this) {}

inline Function Namespace::GetConstructor() const
{
    auto arktsNamespace = GetApiConfig()->cArktsIapi_->coreNamespaceToArktsNamespace(GetView());
    CheckError(GetApiConfig());
    auto arktsCon = GetApiConfig()->cArktsIapi_->arktsV1NamespaceGetConstructor(arktsNamespace);
    CheckError(GetApiConfig());
    auto coreCon = GetApiConfig()->cArktsIapi_->arktsFunctionToCoreFunction(arktsCon);
    return Function(core::Function(coreCon, GetApiConfig(), GetResource()));
}

inline bool Namespace::SetName(const std::string &name) const
{
    const auto ret = GetApiConfig()->cArktsMapi_->namespaceSetName(TargetCast(), name.c_str());
    CheckError(GetApiConfig());
    return ret;
}

}  // namespace abckit::arkts

#endif  // CPP_ABCKIT_ARKTS_NAMESPACE_IMPL_H
