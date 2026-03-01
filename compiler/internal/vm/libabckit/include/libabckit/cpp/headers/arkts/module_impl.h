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

#ifndef CPP_ABCKIT_ARKTS_MODULE_IMPL_H
#define CPP_ABCKIT_ARKTS_MODULE_IMPL_H

#include "module.h"
#include "import_descriptor.h"
#include "export_descriptor.h"
#include "../core/import_descriptor.h"
#include "../core/export_descriptor.h"
#include "annotation_interface.h"
#include "../core/annotation_interface.h"
#include "class.h"
#include "function.h"
#include <vector>
#include <string_view>

#include "field.h"

// NOLINTBEGIN(performance-unnecessary-value-param)
namespace abckit::arkts {

inline AbckitArktsModule *Module::TargetCast() const
{
    auto ret = GetApiConfig()->cArktsIapi_->coreModuleToArktsModule(GetView());
    CheckError(GetApiConfig());
    return ret;
}

inline Module::Module(const core::Module &coreOther) : core::Module(coreOther), targetChecker_(this) {}

inline bool Module::SetName(const std::string &name) const
{
    const auto ret = GetApiConfig()->cArktsMapi_->moduleSetName(TargetCast(), name.c_str());
    CheckError(GetApiConfig());
    return ret;
}

inline arkts::ImportDescriptor Module::AddImportFromArktsV1ToArktsV1(Module imported, std::string_view name,
                                                                     std::string_view alias) const
{
    const AbckitArktsImportFromDynamicModuleCreateParams params {name.data(), alias.data()};
    AbckitArktsImportDescriptor *arktsid =
        GetApiConfig()->cArktsMapi_->moduleAddImportFromArktsV1ToArktsV1(TargetCast(), imported.TargetCast(), &params);
    CheckError(GetApiConfig());
    AbckitCoreImportDescriptor *coreid =
        GetApiConfig()->cArktsIapi_->arktsImportDescriptorToCoreImportDescriptor(arktsid);
    CheckError(GetApiConfig());
    return arkts::ImportDescriptor(core::ImportDescriptor(coreid, GetApiConfig(), GetResource()));
}

inline arkts::Class Module::ImportClassFromArktsV2ToArktsV2(arkts::Module exported, std::string_view name) const
{
    auto arktsClass =
        GetApiConfig()->cArktsMapi_->moduleImportClassFromArktsV2toArktsV2(exported.TargetCast(), name.data());
    CheckError(GetApiConfig());
    auto coreClass = GetApiConfig()->cArktsIapi_->arktsClassToCoreClass(arktsClass);
    CheckError(GetApiConfig());
    return arkts::Class(core::Class(coreClass, GetApiConfig(), GetResource()));
}

inline arkts::Function Module::ImportStaticFunctionFromArktsV2ToArktsV2(arkts::Module exported,
                                                                        std::string_view functionName,
                                                                        std::string_view returnType,
                                                                        const std::vector<const char *> &params) const
{
    auto arktsFunction = GetApiConfig()->cArktsMapi_->moduleImportStaticFunctionFromArktsV2ToArktsV2(
        exported.TargetCast(), functionName.data(), returnType.data(), params.data(), params.size());
    CheckError(GetApiConfig());
    auto coreFunction = GetApiConfig()->cArktsIapi_->arktsFunctionToCoreFunction(arktsFunction);
    CheckError(GetApiConfig());
    return arkts::Function(core::Function(coreFunction, GetApiConfig(), GetResource()));
}

inline arkts::Function Module::ImportClassMethodFromArktsV2ToArktsV2(arkts::Module exported, std::string_view className,
                                                                     std::string_view methodName,
                                                                     std::string_view returnType,
                                                                     const std::vector<const char *> &params) const
{
    auto arktsFunction = GetApiConfig()->cArktsMapi_->moduleImportClassMethodFromArktsV2ToArktsV2(
        exported.TargetCast(), className.data(), methodName.data(), returnType.data(), params.data(), params.size());
    CheckError(GetApiConfig());
    auto coreFunction = GetApiConfig()->cArktsIapi_->arktsFunctionToCoreFunction(arktsFunction);
    CheckError(GetApiConfig());
    return arkts::Function(core::Function(coreFunction, GetApiConfig(), GetResource()));
}

inline arkts::ExportDescriptor Module::AddExportFromArktsV1ToArktsV1(arkts::Module exported, std::string_view name,
                                                                     std::string_view alias) const
{
    const AbckitArktsDynamicModuleExportCreateParams params {name.data(), alias.data()};
    auto arktsExported = exported.TargetCast();
    auto arktsEd =
        GetApiConfig()->cArktsMapi_->moduleAddExportFromArktsV1ToArktsV1(TargetCast(), arktsExported, &params);
    CheckError(GetApiConfig());
    auto coreEd = GetApiConfig()->cArktsIapi_->arktsExportDescriptorToCoreExportDescriptor(arktsEd);
    CheckError(GetApiConfig());
    return ExportDescriptor(core::ExportDescriptor(coreEd, GetApiConfig(), GetResource()));
}

inline Module Module::RemoveImport(arkts::ImportDescriptor desc) const
{
    auto arktsId = desc.TargetCast();
    auto arktsMod = TargetCast();
    GetApiConfig()->cArktsMapi_->moduleRemoveImport(arktsMod, arktsId);
    CheckError(GetApiConfig());
    return *this;
}

inline Module Module::RemoveExport(arkts::ExportDescriptor desc) const
{
    auto arktsId = desc.TargetCast();
    auto arktsMod = TargetCast();
    GetApiConfig()->cArktsMapi_->moduleRemoveExport(arktsMod, arktsId);
    CheckError(GetApiConfig());
    return *this;
}

inline arkts::AnnotationInterface Module::AddAnnotationInterface(std::string_view name) const
{
    const AbckitArktsAnnotationInterfaceCreateParams params {name.data()};
    auto arktsMod = TargetCast();
    auto arktsAi = GetApiConfig()->cArktsMapi_->moduleAddAnnotationInterface(arktsMod, &params);
    CheckError(GetApiConfig());
    auto coreAi = GetApiConfig()->cArktsIapi_->arktsAnnotationInterfaceToCoreAnnotationInterface(arktsAi);
    CheckError(GetApiConfig());
    return AnnotationInterface(core::AnnotationInterface(coreAi, GetApiConfig(), GetResource()));
}

inline arkts::ModuleField Module::AddField(const std::string_view name, const Type &type, const Value &value,
                                           AbckitArktsFieldVisibility fieldVisibility)
{
    const struct AbckitArktsFieldCreateParams params {
        name.data(), type.GetView(), value.GetView(), true, fieldVisibility
    };
    auto *arkModuleField = GetApiConfig()->cArktsMapi_->moduleAddField(TargetCast(), &params);
    CheckError(GetApiConfig());
    auto *coreModuleField = GetApiConfig()->cArktsIapi_->arktsModuleFieldToCoreModuleField(arkModuleField);
    CheckError(GetApiConfig());
    return arkts::ModuleField(core::ModuleField(coreModuleField, GetApiConfig(), GetResource()));
}

inline arkts::ModuleField Module::AddField(const std::string_view name, const Type &type,
                                           AbckitArktsFieldVisibility fieldVisibility)
{
    const struct AbckitArktsFieldCreateParams params {
        name.data(), type.GetView(), nullptr, true, fieldVisibility
    };
    auto *arkModuleField = GetApiConfig()->cArktsMapi_->moduleAddField(TargetCast(), &params);
    CheckError(GetApiConfig());
    auto *coreModuleField = GetApiConfig()->cArktsIapi_->arktsModuleFieldToCoreModuleField(arkModuleField);
    CheckError(GetApiConfig());
    return arkts::ModuleField(core::ModuleField(coreModuleField, GetApiConfig(), GetResource()));
}

}  // namespace abckit::arkts
// NOLINTEND(performance-unnecessary-value-param)

#endif  // CPP_ABCKIT_ARKTS_MODULE_IMPL_H
