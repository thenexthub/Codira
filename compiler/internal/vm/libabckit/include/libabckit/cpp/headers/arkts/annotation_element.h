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

#ifndef CPP_ABCKIT_ARKTS_ANNOTATION_ELEMENT_H
#define CPP_ABCKIT_ARKTS_ANNOTATION_ELEMENT_H

#include "../core/annotation_element.h"
#include "../base_concepts.h"

namespace abckit::arkts {

/**
 * @brief AnnotationElement
 */
class AnnotationElement : public core::AnnotationElement {
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    /// @brief to access private constructor
    friend class arkts::Class;
    /// @brief to access private constructor
    friend class arkts::Function;
    /// @brief to access private constructor
    friend class arkts::Annotation;
    /// @brief abckit::DefaultHash<AnnotationElement>
    friend class abckit::DefaultHash<AnnotationElement>;
    /// @brief to access private TargetCast
    friend class abckit::traits::TargetCheckCast<AnnotationElement>;

public:
    /**
     * @brief Constructor Arkts API AnnotationElement from the Core API with compatibility check
     * @param coreOther - Core API AnnotationElement
     */
    explicit AnnotationElement(const core::AnnotationElement &coreOther);

    /**
     * @brief Construct a new Annotation Element object
     * @param other
     */
    AnnotationElement(const AnnotationElement &other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return AnnotationElement&
     */
    AnnotationElement &operator=(const AnnotationElement &other) = default;

    /**
     * @brief Construct a new Annotation Element object
     * @param other
     */
    AnnotationElement(AnnotationElement &&other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return AnnotationElement&
     */
    AnnotationElement &operator=(AnnotationElement &&other) = default;

    /**
     * @brief Destroy the Annotation Element object
     */
    ~AnnotationElement() override = default;

    /**
     * @brief Get the Name object
     * @return std::string
     */
    std::string GetName() const;

private:
    /**
     * @brief Converts annotation element from Core to Arkts target
     * @return AbckitArktsAnnotationElement* - converted annotation element
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `this` is does not have `ABCKIT_TARGET_ARK_TS_V1` or
     * `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    AbckitArktsAnnotationElement *TargetCast() const;

    ABCKIT_NO_UNIQUE_ADDRESS traits::TargetCheckCast<AnnotationElement> targetChecker_;
};

}  // namespace abckit::arkts

#endif  // CPP_ABCKIT_ARKTS_ANNOTATION_ELEMENT_H
