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

#ifndef CPP_ABCKIT_JS_MODULE_H
#define CPP_ABCKIT_JS_MODULE_H

#include "../core/module.h"
#include "../base_concepts.h"
#include "import_descriptor.h"
#include "export_descriptor.h"

#include <string_view>

namespace abckit::js {

/**
 * @brief Module
 */
class Module final : public core::Module {
    // To access private constructor.
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    /// @brief to access private constructor
    friend class abckit::File;
    /// @brief abckit::DefaultHash<Module>
    friend class abckit::DefaultHash<Module>;
    /// @brief to access private TargetCast
    friend class abckit::traits::TargetCheckCast<Module>;

public:
    /**
     * @brief Constructor Arkts API Module from the Core API with compatibility check
     * @param coreModule - Core API Module
     */
    explicit Module(const core::Module &coreModule);

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
     * @brief Adds import from one Js module to another Js module.
     * @return Newly created import descriptor.
     * @param [ in ] imported - The module the `importing` module imports from.
     * @param [ in ] name - Name of created import.
     * @param [ in ] alias - Alias of created import.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `imported` is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     * @note Allocates
     */
    ImportDescriptor AddImportFromJsToJs(Module imported, std::string_view name, std::string_view alias) const;

    /**
     * @brief Removes import `id` from Module.
     * @param [ in ] desc - Import to remove from the Module.
     * @return New state of Module.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `id` is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if Module does not have the import descriptor `id`.
     */
    Module RemoveImport(ImportDescriptor desc) const;

    /**
     * @brief Adds export to the Js module.
     * @return Newly created export descriptor.
     * @param [ in ] exported - The module the entity is exported from.
     * @param [ in ] name - Name of created export.
     * @param [ in ] alias - Alias of created export.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `exported` is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     * @note Allocates
     */
    ExportDescriptor AddExportFromJsToJs(Module exported, std::string_view name, std::string_view alias) const;

    /**
     * @brief Removes export `ed` from Module.
     * @param [ in ] desc - Export to remove from the Module.
     * @return New state of Module.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `ed` is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if module `m` does not have the export descriptor `ed`.
     */
    Module RemoveExport(ExportDescriptor desc) const;

private:
    /**
     * @brief Converts underlying module from Core to Js target
     * @return AbckitJsModule* - converted module
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `this` does not have `ABCKIT_TARGET_JS`
     */
    AbckitJsModule *TargetCast() const;

    ABCKIT_NO_UNIQUE_ADDRESS traits::TargetCheckCast<Module> targetChecker_;
};

}  // namespace abckit::js

#endif  // CPP_ABCKIT_JS_MODULE_H
