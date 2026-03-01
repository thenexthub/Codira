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

#ifndef CPP_ABCKIT_ARKTS_MODULE_H
#define CPP_ABCKIT_ARKTS_MODULE_H

#include "../core/module.h"
#include "../base_concepts.h"
#include "import_descriptor.h"
#include "export_descriptor.h"
#include "annotation_interface.h"
#include <vector>

#include <string_view>

namespace abckit::arkts {

/**
 * @brief Module
 */
class Module final : public core::Module {
    // To access private constructor.
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    /// @brief to access private constructor
    friend class abckit::File;
    /// @brief to access private constructor
    friend class Class;
    /// @brief to access private constructor
    friend class Interface;
    /// @brief abckit::DefaultHash<Module>
    friend class abckit::DefaultHash<Module>;
    /// @brief to access private TargetCast
    friend class abckit::traits::TargetCheckCast<Module>;

public:
    /**
     * @brief Constructor Arkts API Module from the Core API with compatibility check
     * @param coreOther - Core API Module
     */
    explicit Module(const core::Module &coreOther);

    /**
     * @brief Construct a new Module object
     * @param other
     */
    Module(const Module &other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return Module&
     */
    Module &operator=(const Module &other) = default;

    /**
     * @brief Construct a new Module object
     * @param other
     */
    Module(Module &&other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return Module&
     */
    Module &operator=(Module &&other) = default;

    /**
     * @brief Destroy the Module object
     */
    ~Module() override = default;

    /**
     * @brief Sets name for module
     * @return `true` on success.
     * @param [ in ] name - Name to be set.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    bool SetName(const std::string &name) const;

    /**
     * @brief Adds import from one ArktsV1 module to another ArktsV1 module.
     * @return Pointer to the newly created import descriptor.
     * @param [ in ] imported - The module the `importing` module imports from.
     * @param [ in ] name - Import name. For namespace imports equals to "*". For default imports equals to "default".
     * For regular imports is the same as in user code.
     * @param [ in ] alias - Alias name for the import. For namespace imports is the same as in user code. For default
     * import is the same as the default import name in user code. For regular imports is the same as in user code.
     * @note Allocates
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `imported` is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    ImportDescriptor AddImportFromArktsV1ToArktsV1(Module imported, std::string_view name,
                                                   std::string_view alias) const;
    /**
     * @brief Import class from one ArktsV2 module to another ArktsV2 module.
     * @return Pointer to the newly created class.
     * @param [ in ] exported - The module the class is exported from.
     * @param [ in ] name - The name of the class to import.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `exported` is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `name` is false.
     */
    arkts::Class ImportClassFromArktsV2ToArktsV2(arkts::Module exported, std::string_view name) const;

    /**
     * @brief Import static function from one ArktsV2 module to another ArktsV2 module.
     * @return Pointer to the newly created function.
     * @param [ in ] exported - The module the function is exported from.
     * @param [ in ] functionName - The name of the function to import.
     * @param [ in ] returnType - The return type of the function.
     * @param [ in ] params - The parameter types of the function.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `exported` is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `functionName` is false.
     */
    arkts::Function ImportStaticFunctionFromArktsV2ToArktsV2(arkts::Module exported, std::string_view functionName,
                                                             std::string_view returnType,
                                                             const std::vector<const char *> &params) const;

    /**
     * @brief Import class instance method from one ArktsV2 module to another ArktsV2 module.
     * @return Pointer to the newly created function.
     * @param [ in ] exported - The module the method is exported from.
     * @param [ in ] className - The name of the class containing the method.
     * @param [ in ] methodName - The name of the method to import.
     * @param [ in ] returnType - The return type of the method.
     * @param [ in ] params - The parameter types of the method.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `exported` is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `className` or `methodName` is false.
     */
    arkts::Function ImportClassMethodFromArktsV2ToArktsV2(arkts::Module exported, std::string_view className,
                                                          std::string_view methodName, std::string_view returnType,
                                                          const std::vector<const char *> &params) const;
    /**
     * @brief Removes import `id` from Module.
     * @param [ in ] desc - Import to remove from the Module.
     * @return New state of Module.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if Module does not have the import descriptor `id`.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if Module doesn't have `ABCKIT_TARGET_ARK_TS_V1` target.
     */
    Module RemoveImport(arkts::ImportDescriptor desc) const;

    /**
     * @brief Adds export from one Arkts module to another Arkts module.
     * @return Newly created export descriptor.
     * @param [ in ] exported - The module the entity is exported from. In case of local export is the same as
     * `exporting`.
     * @param [ in ] name - Import name. For namespace imports equals to "*". For default imports equals to "default".
     * For regular imports is the same as in user code.
     * @param [ in ] alias - Alias. For namespace imports equals to "*". For default imports equals to "default".
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `exported` is false.
     */
    arkts::ExportDescriptor AddExportFromArktsV1ToArktsV1(arkts::Module exported, std::string_view name,
                                                          std::string_view alias) const;

    /**
     * @brief Removes export `ed` from Module.
     * @param [ in ] desc - Export to remove from the Module.
     * @return New state of Module.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if Module does not have the export descriptor `ed`.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if Module doesn't have `ABCKIT_TARGET_ARK_TS_V1` target.
     */
    Module RemoveExport(arkts::ExportDescriptor desc) const;

    /**
     * @brief Adds new annotation interface to the Module.
     * @return Newly constructed annotation interface.
     * @param [ in ] name - Annotation interface name.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if module Module doesn't have `ABCKIT_TARGET_ARK_TS_V1` target.
     * @note Allocates
     */
    arkts::AnnotationInterface AddAnnotationInterface(std::string_view name) const;

    /**
     * @brief Add field to the Module.
     * @return Newly Add ModuleField.
     * @param [ in ] name - Name to be set.
     * @param [ in ] type - Type to be set.
     * @param [ in ] value - Value to be set.
     * @param [ in ] fieldVisibility - fieldVisibility to be set.
     * @note Allocates
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `name` is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `type` is false.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if Module doesn't have `ABCKIT_TARGET_ARK_TS_V1` target.
     */
    arkts::ModuleField AddField(const std::string_view name, const Type &type, const Value &value,
                                AbckitArktsFieldVisibility fieldVisibility = AbckitArktsFieldVisibility::PUBLIC);

    /**
     * @brief Add field to the Module.
     * @return Newly Add ModuleField.
     * @param [ in ] name - Name to be set.
     * @param [ in ] type - Type to be set.
     * @param [ in ] fieldVisibility - fieldVisibility to be set.
     * @note Allocates
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `name` is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `type` is false.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if Module doesn't have `ABCKIT_TARGET_ARK_TS_V1` target.
     */
    arkts::ModuleField AddField(const std::string_view name, const Type &type,
                                AbckitArktsFieldVisibility fieldVisibility = AbckitArktsFieldVisibility::PUBLIC);

private:
    /**
     * @brief Converts underlying module from Core to Arkts target
     * @return AbckitArktsModule* - converted module
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `this` does not have `ABCKIT_TARGET_ARK_TS_V1` or
     * `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    AbckitArktsModule *TargetCast() const;

    ABCKIT_NO_UNIQUE_ADDRESS traits::TargetCheckCast<Module> targetChecker_;
};

}  // namespace abckit::arkts

#endif  // CPP_ABCKIT_ARKTS_MODULE_H
