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

#ifndef CPP_ABCKIT_CORE_IMPORT_DESCRIPTOR_IMPL_H
#define CPP_ABCKIT_CORE_IMPORT_DESCRIPTOR_IMPL_H

#include "import_descriptor.h"
#include "function.h"
#include "module.h"
namespace abckit::core {

inline const File *ImportDescriptor::GetFile() const
{
    return GetResource();
}

inline std::string ImportDescriptor::GetName() const
{
    AbckitString *abcName = GetApiConfig()->cIapi_->importDescriptorGetName(GetView());
    CheckError(GetApiConfig());
    std::string name = GetApiConfig()->cIapi_->abckitStringToString(abcName);
    CheckError(GetApiConfig());
    return name;
}

inline Module ImportDescriptor::GetImportedModule() const
{
    AbckitCoreModule *module = GetApiConfig()->cIapi_->importDescriptorGetImportedModule(GetView());
    CheckError(GetApiConfig());
    return core::Module(module, conf_, GetResource());
}

inline ImportDescriptor::ImportDescriptor(AbckitCoreImportDescriptor *module, const ApiConfig *conf, const File *file)
    : ViewInResource(module), conf_(conf)
{
    SetResource(file);
};

inline Module ImportDescriptor::GetImportingModule() const
{
    AbckitCoreModule *module = GetApiConfig()->cIapi_->importDescriptorGetImportingModule(GetView());
    CheckError(GetApiConfig());
    return core::Module(module, conf_, GetResource());
}

inline std::string ImportDescriptor::DescriptorGetAlias() const
{
    AbckitString *abcAlias = GetApiConfig()->cIapi_->importDescriptorGetAlias(GetView());
    CheckError(GetApiConfig());
    std::string alias = GetApiConfig()->cIapi_->abckitStringToString(abcAlias);
    CheckError(GetApiConfig());
    return alias;
}

}  // namespace abckit::core

#endif  // CPP_ABCKIT_CORE_IMPORT_DESCRIPTOR_IMPL_H
