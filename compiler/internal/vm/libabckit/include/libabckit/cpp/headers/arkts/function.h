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

#ifndef CPP_ABCKIT_ARKTS_FUNCTION_H
#define CPP_ABCKIT_ARKTS_FUNCTION_H

#include "../core/function.h"
#include "../base_concepts.h"

namespace abckit::arkts {

/**
 * @brief Function
 */
class Function final : public core::Function {
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    /// @brief to access private constructor
    friend class Class;
    /// @brief to access private constructor
    friend class Namespace;
    /// @brief to access private constructor
    friend class Interface;
    /// @brief abckit::DefaultHash<Function>
    friend class abckit::DefaultHash<Function>;
    /// @brief to access private TargetCast
    friend class abckit::traits::TargetCheckCast<Function>;

public:
    /**
     * @brief Constructor Arkts API Function from the Core API with compatibility check
     * @param other - Core API Function
     */
    explicit Function(const core::Function &other);

    /**
     * @brief Construct a new Function object
     * @param other
     */
    Function(const Function &other) = default;

    /**
     * @brief Constructor
     * @param coreOther
     * @return Function&
     */
    Function &operator=(const Function &coreOther) = default;

    /**
     * @brief Construct a new Function object
     * @param other
     */
    Function(Function &&other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return Function&
     */
    Function &operator=(Function &&other) = default;

    /**
     * @brief Destroy the Function object
     */
    ~Function() override = default;

    /**
     * @brief Check whether the Function is native.
     * @return `true` if Function is native, `false` otherwise.
     */
    bool IsNative() const;

    /**
     * @brief Tells if method is abstract.
     * @return bool
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    bool IsAbstract() const;

    /**
     * @brief Tells if method is final.
     * @return bool
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    bool IsFinal() const;

    /**
     * @brief Tells if method is async.
     * @return bool
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    bool IsAsync() const;

    /**
     * @brief Function to add annotation to.
     * @return Newly created annotation.
     * @param [ in ] ai -  Annotation Interface that is used to create the annotation.
     * @note Allocates
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `ai` is false.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if function Function doesn't have `ABCKIT_TARGET_ARK_TS_V1` target.
     */
    Annotation AddAnnotation(AnnotationInterface ai) const;

    /**
     * @brief Remove annotation from the function.
     * @param [ in ] anno - Annotation to remove from the function `function`.
     * @return New state of Function.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `anno` is false.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if Function doesn't have `ABCKIT_TARGET_ARK_TS_V1` target.
     */
    Function RemoveAnnotation(Annotation anno) const;

    /**
     * @brief Sets name for function
     * @return `true` on success.
     * @param [ in ] name - Name to be set.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if current `ClassField` is false.
     */
    bool SetName(const std::string &name) const;

private:
    /**
     * @brief Converts underlying function from Core to Arkts target
     * @return AbckitArktsFunction* - converted function
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `this` is does not have `ABCKIT_TARGET_ARK_TS_V1` or
     * `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    AbckitArktsFunction *TargetCast() const;

    ABCKIT_NO_UNIQUE_ADDRESS traits::TargetCheckCast<Function> targetChecker_;
};

}  // namespace abckit::arkts

#endif  // CPP_ABCKIT_ARKTS_FUNCTION_H
