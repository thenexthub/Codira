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

#ifndef CPP_ABCKIT_ARKTS_NAMESPACE_H
#define CPP_ABCKIT_ARKTS_NAMESPACE_H

#include "../core/namespace.h"
#include "function.h"
#include "../base_concepts.h"

namespace abckit::arkts {

/**
 * @brief Namespace
 */
class Namespace final : public core::Namespace {
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    /// @brief to access private constructor
    friend class Module;
    /// @brief abckit::DefaultHash<Namespace>
    friend class abckit::DefaultHash<Namespace>;
    /// @brief to access private TargetCast
    friend class abckit::traits::TargetCheckCast<Namespace>;

public:
    /**
     * @brief Constructor Arkts API Namespace from the Core API with compatibility check
     * @param other - Core API Namespace
     */
    explicit Namespace(const core::Namespace &other);

    /**
     * @brief Constructor
     * @param other
     */
    Namespace(const Namespace &other) = default;
    /**
     * @brief Consructor
     * @param other
     * @return Namespace&
     */
    Namespace &operator=(const Namespace &other) = default;

    /**
     * @brief Construct a new Namespace object
     * @param other
     */
    Namespace(Namespace &&other) = default;
    /**
     * @brief Constructor
     * @param other
     * @return Namespace&
     */
    Namespace &operator=(Namespace &&other) = default;

    /**
     * @brief Destroy the Namespace object
     */
    ~Namespace() override = default;

    /**
     * @brief Returns constructor function for Namespace.
     * @return Function that is invoked upon Namespace construction.
     */
    Function GetConstructor() const;

    /**
     * @brief Sets name for namespace
     * @return `true` on success.
     * @param [ in ] name - Name to be set.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    bool SetName(const std::string &name) const;

private:
    /**
     * @brief Converts underlying namespace from Core to Arkts target
     * @return AbckitArktsNamespace* - converted namespace
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `this` does not have `ABCKIT_TARGET_ARK_TS_V1` or
     * `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    AbckitArktsNamespace *TargetCast() const;

    ABCKIT_NO_UNIQUE_ADDRESS traits::TargetCheckCast<Namespace> targetChecker_;
};

}  // namespace abckit::arkts

#endif  // CPP_ABCKIT_ARKTS_NAMESPACE_H
