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

#ifndef CPP_ABCKIT_ARKTS_ENUM_H
#define CPP_ABCKIT_ARKTS_ENUM_H

#include "../core/enum.h"
#include "../base_concepts.h"

namespace abckit::arkts {

/**
 * @brief enum
 */
class Enum final : public core::Enum {
    // To access private constructor.
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    /// @brief to access private constructor
    friend class Module;
    /// @brief to access private constructor
    friend class Namespace;
    /// @brief abckit::DefaultHash<Enum>
    friend class abckit::DefaultHash<Enum>;
    /// @brief to access private TargetCast
    friend class abckit::traits::TargetCheckCast<Enum>;

public:
    /**
     * @brief Constructor Arkts API Enum from the Core API with compatibility check
     * @param coreOther - Core API Interface
     */
    explicit Enum(const core::Enum &coreOther);

    /**
     * @brief Construct a new enum object
     * @param other
     */
    Enum(const Enum &other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return Enum&
     */
    Enum &operator=(const Enum &other) = default;

    /**
     * @brief Construct a new Enum object
     * @param other
     */
    Enum(Enum &&other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return Enum&
     */
    Enum &operator=(Enum &&other) = default;

    /**
     * @brief Destroy the Enum object
     */
    ~Enum() override = default;

    /**
     * @brief Sets name for Enum
     * @return `true` on success.
     * @param [ in ] name - Name to be set.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    bool SetName(const std::string &name) const;

    /**
     * @brief Add field to the enum.
     * @return Newly Add EnumField.
     * @param [ in ] name - Name to be set.
     * @param [ in ] type - Type to be set.
     * @param [ in ] value - Value to be set.
     * @param [ in ] fieldVisibility - fieldVisibility to be set.
     * @note Allocates
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `name` is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `type` is false.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if Enum doesn't have `ABCKIT_TARGET_ARK_TS_V1` target.
     */
    arkts::EnumField AddField(const std::string_view name, const Type &type, const Value &value,
                              AbckitArktsFieldVisibility fieldVisibility = AbckitArktsFieldVisibility::PUBLIC);

    /**
     * @brief Add field to the enum.
     * @return Newly Add EnumField.
     * @param [ in ] name - Name to be set.
     * @param [ in ] type - Type to be set.
     * @param [ in ] fieldVisibility - fieldVisibility to be set.
     * @note Allocates
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `name` is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `type` is false.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if Enum doesn't have `ABCKIT_TARGET_ARK_TS_V1` target.
     */
    arkts::EnumField AddField(const std::string_view name, const Type &type,
                              AbckitArktsFieldVisibility fieldVisibility = AbckitArktsFieldVisibility::PUBLIC);

private:
    /**
     * @brief Converts enum from Core to Arkts target
     * @return AbckitArktsEnum* - converted enum
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `this` is does not have `ABCKIT_TARGET_ARK_TS_V1` or
     * `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    AbckitArktsEnum *TargetCast() const;

    ABCKIT_NO_UNIQUE_ADDRESS traits::TargetCheckCast<Enum> targetChecker_;
};

}  // namespace abckit::arkts

#endif  // CPP_ABCKIT_ARKTS_ENUM_H
