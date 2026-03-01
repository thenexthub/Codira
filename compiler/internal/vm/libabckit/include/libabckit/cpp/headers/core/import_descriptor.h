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

#ifndef CPP_ABCKIT_CORE_IMPORT_DESCRIPTOR_H
#define CPP_ABCKIT_CORE_IMPORT_DESCRIPTOR_H

#include "../base_classes.h"
#include "module.h"

#include <string>

namespace abckit::core {

/**
 * @brief ImportDescriptor
 */
class ImportDescriptor : public ViewInResource<AbckitCoreImportDescriptor *, const File *> {
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    /// @brief to access private constructor
    friend class abckit::File;
    /// @brief to access private constructor
    friend class core::Module;
    /// @brief to access private constructor
    friend class arkts::Module;
    /// @brief to access private constructor
    friend class js::Module;
    /// @brief to access private constructor
    friend class abckit::DynamicIsa;
    /// @brief to access private constructor
    friend class abckit::Instruction;
    /// @brief abckit::DefaultHash<ImportDescriptor>
    friend class abckit::DefaultHash<ImportDescriptor>;

protected:
    /// @brief Core API View type
    using CoreViewT = ImportDescriptor;

public:
    /**
     * @brief Construct a new empty Import Descriptor object
     */
    ImportDescriptor() : ViewInResource(nullptr), conf_(nullptr)
    {
        SetResource(nullptr);
    };

    /**
     * @brief Construct a new Import Descriptor object
     * @param other
     */
    ImportDescriptor(const ImportDescriptor &other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return ImportDescriptor&
     */
    ImportDescriptor &operator=(const ImportDescriptor &other) = default;

    /**
     * @brief Construct a new Import Descriptor object
     * @param other
     */
    ImportDescriptor(ImportDescriptor &&other) = default;  // CC-OFF(G.CLS.07-CPP) plan to break polymorphism

    /**
     * @brief Constructor
     * @param other
     * @return ImportDescriptor&
     */
    ImportDescriptor &operator=(ImportDescriptor &&other) = default;

    /**
     * @brief Destroy the Import Descriptor object
     */
    ~ImportDescriptor() override = default;

    /**
     * @brief Returns binary file that the Import Descriptor is a part of.
     * @return Pointer to the `File` that the Import is a part of.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    const File *GetFile() const;

    /**
     * @brief Returns name
     * @return std::string
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    std::string GetName() const;

    /**
     * @brief Returns imported module.
     * @return core::Module
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    Module GetImportedModule() const;

    /**
     * @brief Returns importing module.
     * @return `core::Module` that the import `i` is a part of.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    Module GetImportingModule() const;

    /**
     * @brief Returns alias for import descriptor.
     * @return `AbckitString` - the alias of the imported entity.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     * @note Allocates
     */
    std::string DescriptorGetAlias() const;

protected:
    const ApiConfig *GetApiConfig() const override
    {
        return conf_;
    }

private:
    ImportDescriptor(AbckitCoreImportDescriptor *module, const ApiConfig *conf, const File *file);

    const ApiConfig *conf_;
};

}  // namespace abckit::core

#endif  // CPP_ABCKIT_CORE_IMPORT_DESCRIPTOR_H
