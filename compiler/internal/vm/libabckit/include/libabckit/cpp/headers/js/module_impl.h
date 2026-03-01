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

#ifndef CPP_ABCKIT_JS_MODULE_IMPL_H
#define CPP_ABCKIT_JS_MODULE_IMPL_H

#include "module.h"

// NOLINTBEGIN(performance-unnecessary-value-param)
namespace abckit::js {

inline AbckitJsModule *Module::TargetCast() const
{
    auto ret = GetApiConfig()->cJsIapi_->coreModuleToJsModule(GetView());
    CheckError(GetApiConfig());
    return ret;
}

inline Module::Module(const core::Module &coreModule) : core::Module(coreModule), targetChecker_(this) {}

inline ImportDescriptor Module::AddImportFromJsToJs(Module imported, std::string_view name,
                                                    std::string_view alias) const
{
    const AbckitJsImportFromDynamicModuleCreateParams params {name.data(), alias.data()};
    auto *jsid = GetApiConfig()->cJsMapi_->moduleAddImportFromJsToJs(TargetCast(), imported.TargetCast(), &params);
    CheckError(GetApiConfig());
    auto *coreid = GetApiConfig()->cJsIapi_->jsImportDescriptorToCoreImportDescriptor(jsid);
    CheckError(GetApiConfig());
    return js::ImportDescriptor(core::ImportDescriptor(coreid, GetApiConfig(), GetResource()));
}

inline Module Module::RemoveImport(ImportDescriptor desc) const
{
    GetApiConfig()->cJsMapi_->moduleRemoveImport(TargetCast(), desc.TargetCast());
    CheckError(GetApiConfig());
    return *this;
}

inline ExportDescriptor Module::AddExportFromJsToJs(Module exported, std::string_view name,
                                                    std::string_view alias) const
{
    const AbckitJsDynamicModuleExportCreateParams params {name.data(), alias.data()};
    auto *jsed = GetApiConfig()->cJsMapi_->moduleAddExportFromJsToJs(TargetCast(), exported.TargetCast(), &params);
    CheckError(GetApiConfig());
    auto *coreed = GetApiConfig()->cJsIapi_->jsExportDescriptorToCoreExportDescriptor(jsed);
    CheckError(GetApiConfig());
    return js::ExportDescriptor(core::ExportDescriptor(coreed, GetApiConfig(), GetResource()));
}

inline Module Module::RemoveExport(ExportDescriptor desc) const
{
    GetApiConfig()->cJsMapi_->moduleRemoveExport(TargetCast(), desc.TargetCast());
    CheckError(GetApiConfig());
    return *this;
}

}  // namespace abckit::js
// NOLINTEND(performance-unnecessary-value-param)

#endif  // CPP_ABCKIT_JS_MODULE_IMPL_H