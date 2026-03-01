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

#ifndef CPP_ABCKIT_ARKTS_ANNOTATION_INTERFACE_FIELD_H
#define CPP_ABCKIT_ARKTS_ANNOTATION_INTERFACE_FIELD_H

#include "../core/annotation_interface_field.h"
#include "../base_concepts.h"

namespace abckit::arkts {

/**
 * @brief AnnotationInterfaceField
 */
class AnnotationInterfaceField : public core::AnnotationInterfaceField {
    // We restrict constructors in order to prevent C/C++ API mix-up by user.

    /// @brief to access private constructor
    friend class arkts::Annotation;
    /// @brief to access private constructor
    friend class arkts::AnnotationInterface;
    /// @brief abckit::DefaultHash<AnnotationInterfaceField>
    friend class abckit::DefaultHash<AnnotationInterfaceField>;
    /// @brief to access private TargetCast
    friend class abckit::traits::TargetCheckCast<AnnotationInterfaceField>;

public:
    /**
     * @brief Constructor Arkts API AnnotationInterfaceField from the Core API with compatibility check
     * @param coreOther - Core API AnnotationInterfaceField
     */
    explicit AnnotationInterfaceField(const core::AnnotationInterfaceField &coreOther);

    /**
     * @brief Construct a new Annotation Interface Field object
     *
     * @param other
     */
    AnnotationInterfaceField(const AnnotationInterfaceField &other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return AnnotationInterfaceField&
     */
    AnnotationInterfaceField &operator=(const AnnotationInterfaceField &other) = default;

    /**
     * @brief Construct a new Annotation Interface Field object
     * @param other
     */
    AnnotationInterfaceField(AnnotationInterfaceField &&other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return AnnotationInterfaceField&
     */
    AnnotationInterfaceField &operator=(AnnotationInterfaceField &&other) = default;

    /**
     * @brief Destroy the Annotation Interface Field object
     */
    ~AnnotationInterfaceField() override = default;

private:
    /**
     * @brief Converts annotation interface field from Core to Arkts target
     * @return AbckitArktsAnnotationInterfaceField* - converted annotation interface field
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `this` is does not have `ABCKIT_TARGET_ARK_TS_V1` or
     * `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    AbckitArktsAnnotationInterfaceField *TargetCast() const;

    ABCKIT_NO_UNIQUE_ADDRESS traits::TargetCheckCast<AnnotationInterfaceField> targetChecker_;
};

}  // namespace abckit::arkts

#endif  // CPP_ABCKIT_ARKTS_ANNOTATION_INTERFACE_FIELD_H
