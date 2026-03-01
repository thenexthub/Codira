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

#ifndef CPP_ABCKIT_ARKTS_ANNOTATION_H
#define CPP_ABCKIT_ARKTS_ANNOTATION_H

#include "../core/annotation.h"
#include "../base_concepts.h"

namespace abckit::arkts {

/**
 * @brief Annotation
 */
class Annotation : public core::Annotation {
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    /// @brief to access private constructor
    friend class arkts::Class;
    /// @brief to access private constructor
    friend class arkts::ClassField;
    /// @brief to access private constructor
    friend class arkts::Function;
    /// @brief to access private constructor
    friend class arkts::InterfaceField;
    /// @brief to access private constructor
    friend class arkts::ModuleField;
    /// @brief abckit::DefaultHash<Annotation>
    friend class abckit::DefaultHash<Annotation>;
    /// @brief to access private TargetCast
    friend class abckit::traits::TargetCheckCast<Annotation>;

public:
    /**
     * @brief Constructor Arkts API Annotation from the Core API with compatibility check
     * @param coreOther - Core API Annotation
     */
    explicit Annotation(const core::Annotation &coreOther);

    /**
     * @brief Construct a new Annotation object
     * @param other
     */
    Annotation(const Annotation &other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return Annotation&
     */
    Annotation &operator=(const Annotation &other) = default;

    /**
     * @brief Construct a new Annotation object
     * @param other
     */
    Annotation(Annotation &&other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return Annotation&
     */
    Annotation &operator=(Annotation &&other) = default;

    /**
     * @brief Destroy the Annotation object
     */
    ~Annotation() override = default;

    /**
     * @brief Sets name for annotation
     * @return `true` on success.
     * @param [ in ] name - Name to be set.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    bool SetName(const std::string &name) const;

    /**
     * @brief Add element
     * @param val
     * @param name
     * @return arkts::Annotation
     */
    arkts::Annotation AddElement(const abckit::Value &val, std::string_view name) const;

    /**
     * @brief Add and get element
     * @param val
     * @param name
     * @return arkts::AnnotationElement
     */
    arkts::AnnotationElement AddAndGetElement(const abckit::Value &val, std::string_view name) const;

    /**
     * @brief Add annotation element to the existing annotation.
     * @return Newly created annotation element.
     * @param [ in ] value - value is used to create the annotation element.
     * @param [ in ] name - name is used to create the annotation element.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error view itself is false.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if Annotation doesn't have `ABCKIT_TARGET_ARK_TS_V1` target.
     * @note Allocates
     */
    AnnotationElement AddAnnotationElement(std::string_view name, Value value) const;

    /**
     * @brief Remove annotation element `elem` from the annotation.
     * @param [ in ] elem - Annotation element to remove from the annotation.
     * @return New state of Annotation.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `elem` is false.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if Annotation  doesn't have `ABCKIT_TARGET_ARK_TS_V1` target.
     */
    Annotation RemoveAnnotationElement(AnnotationElement elem) const;

private:
    /**
     * @brief Converts annotation from Core to Arkts target
     * @return AbckitArktsAnnotation* - converted annotation
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `this` is does not have `ABCKIT_TARGET_ARK_TS_V1` or
     * `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    AbckitArktsAnnotation *TargetCast() const;

    ABCKIT_NO_UNIQUE_ADDRESS traits::TargetCheckCast<Annotation> targetChecker_;

    /**
     * @brief add and get element impl
     * @param params
     * @return AbckitCoreAnnotationElement*
     */
    AbckitCoreAnnotationElement *AddAndGetElementImpl(AbckitArktsAnnotationElementCreateParams *params) const;
};

}  // namespace abckit::arkts

#endif  // CPP_ABCKIT_ARKTS_ANNOTATION_H
