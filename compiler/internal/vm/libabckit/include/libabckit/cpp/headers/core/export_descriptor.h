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

#ifndef CPP_ABCKIT_CORE_EXPORT_DESCRIPTOR_H
#define CPP_ABCKIT_CORE_EXPORT_DESCRIPTOR_H

#include "../base_classes.h"

#include <string>

namespace abckit::core {

/**
 * @brief ExportDescriptor
 */
class ExportDescriptor : public ViewInResource<AbckitCoreExportDescriptor *, const File *> {
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    /// @brief to access private constructor
    friend class abckit::File;
    /// @brief to access private constructor
    friend class abckit::core::Module;
    /// @brief to access private constructor
    friend class abckit::arkts::Module;
    /// @brief to access private constructor
    friend class abckit::js::Module;
    /// @brief to access private constructor
    friend class abckit::Instruction;
    /// @brief abckit::DefaultHash<ExportDescriptor>
    friend class abckit::DefaultHash<ExportDescriptor>;
    /// @brief abckit::DynamicIsa
    friend class abckit::DynamicIsa;

protected:
    /// @brief Core API View type
    using CoreViewT = ExportDescriptor;

public:
    /**
     * @brief Construct a new Export Descriptor object
     * @param other
     */
    ExportDescriptor(const ExportDescriptor &other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return ExportDescriptor&
     */
    ExportDescriptor &operator=(const ExportDescriptor &other) = default;

    /**
     * @brief Construct a new Export Descriptor object
     * @param other
     */
    ExportDescriptor(ExportDescriptor &&other) = default;  // CC-OFF(G.CLS.07): design decision, detail: base_concepts.h

    /**
     * @brief Constructor
     * @param other
     * @return ExportDescriptor&
     */
    ExportDescriptor &operator=(ExportDescriptor &&other) = default;

    /**
     * @brief Destroy the Export Descriptor object
     */
    ~ExportDescriptor() override = default;

    /**
     * @brief Returns binary file that the export descriptor is a part of.
     * @return Pointer to the `File` that the export is a part of.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    const File *GetFile() const;

    /**
     * @brief Get the Name object
     * @return std::string
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    std::string GetName() const;

    /**
     * @brief Returns exporting module.
     * @return `core::Module` that the export `e` is a part of.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    Module GetExportingModule() const;

    /**
     * @brief Returns exported module.
     * @return `core::Module` that the export `e` exports from. For local
     * entity export equals to the exporting module. For re-exports may be different.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    Module GetExportedModule() const;

    /**
     * @brief Returns alias for export descriptor.
     * @return `std::string` - alias of the exported entity.
     * @note Allocates
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    std::string GetAlias() const;

private:
    ExportDescriptor(AbckitCoreExportDescriptor *module, const ApiConfig *conf, const File *file)
        : ViewInResource(module), conf_(conf)
    {
        SetResource(file);
    };
    const ApiConfig *conf_;

protected:
    /**
     * @brief Get the Api Config object
     * @return const ApiConfig*
     */
    const ApiConfig *GetApiConfig() const override
    {
        return conf_;
    }
};

}  // namespace abckit::core

#endif  // CPP_ABCKIT_CORE_EXPORT_DESCRIPTOR_H
