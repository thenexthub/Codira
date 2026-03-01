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

#ifndef CPP_ABCKIT_CORE_EXPORT_DESCRIPTOR_IMPL_H
#define CPP_ABCKIT_CORE_EXPORT_DESCRIPTOR_IMPL_H

#include "export_descriptor.h"
#include "module.h"

namespace abckit::core {

inline const File *ExportDescriptor::GetFile() const
{
    return GetResource();
}

inline std::string ExportDescriptor::GetName() const
{
    AbckitString *abcName = GetApiConfig()->cIapi_->exportDescriptorGetName(GetView());
    CheckError(GetApiConfig());
    std::string name = GetApiConfig()->cIapi_->abckitStringToString(abcName);
    CheckError(GetApiConfig());
    return name;
}

inline Module ExportDescriptor::GetExportingModule() const
{
    AbckitCoreModule *mod = GetApiConfig()->cIapi_->exportDescriptorGetExportingModule(GetView());
    CheckError(GetApiConfig());
    return Module(mod, GetApiConfig(), GetResource());
}

inline Module ExportDescriptor::GetExportedModule() const
{
    AbckitCoreModule *mod = GetApiConfig()->cIapi_->exportDescriptorGetExportedModule(GetView());
    CheckError(GetApiConfig());
    return Module(mod, GetApiConfig(), GetResource());
}

inline std::string ExportDescriptor::GetAlias() const
{
    AbckitString *abcAlias = GetApiConfig()->cIapi_->exportDescriptorGetAlias(GetView());
    CheckError(GetApiConfig());
    std::string alias = GetApiConfig()->cIapi_->abckitStringToString(abcAlias);
    CheckError(GetApiConfig());
    return alias;
}

}  // namespace abckit::core

#endif  // CPP_ABCKIT_CORE_EXPORT_DESCRIPTOR_IMPL_H
