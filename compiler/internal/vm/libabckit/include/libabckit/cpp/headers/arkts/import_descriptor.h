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

#ifndef CPP_ABCKIT_ARKTS_IMPORT_DESCRIPTOR_H
#define CPP_ABCKIT_ARKTS_IMPORT_DESCRIPTOR_H

#include "../core/import_descriptor.h"
#include "../base_concepts.h"

namespace abckit::arkts {

/**
 * @brief ImportDescriptor
 */
class ImportDescriptor final : public core::ImportDescriptor {
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    /// @brief to access private constructor
    friend class abckit::File;
    /// @brief to access private constructor
    friend class abckit::arkts::Module;
    /// @brief abckit::DefaultHash<ImportDescriptor>
    friend class abckit::DefaultHash<ImportDescriptor>;
    /// @brief to access private TargetCast
    friend class abckit::traits::TargetCheckCast<ImportDescriptor>;

public:
    /**
     * @brief Constructor Arkts API ImportDescriptor from the Core API with compatibility check
     * @param other - Core API ImportDescriptor
     */
    explicit ImportDescriptor(const core::ImportDescriptor &other);

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
    ImportDescriptor(ImportDescriptor &&other) = default;

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

private:
    /**
     * @brief Converts underlying import descriptor from Core to Arkts target
     * @return AbckitArktsImportDescriptor* - converted import descriptor
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `this` is does not have `ABCKIT_TARGET_ARK_TS_V1` or
     * `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    AbckitArktsImportDescriptor *TargetCast() const;

    ABCKIT_NO_UNIQUE_ADDRESS traits::TargetCheckCast<ImportDescriptor> targetChecker_;
};

}  // namespace abckit::arkts

#endif  // CPP_ABCKIT_ARKTS_IMPORT_DESCRIPTOR_H
