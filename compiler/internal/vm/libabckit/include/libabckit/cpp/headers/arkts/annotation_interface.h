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

#ifndef CPP_ABCKIT_ARKTS_ANNOTATION_INTERFACE_H
#define CPP_ABCKIT_ARKTS_ANNOTATION_INTERFACE_H

#include "../core/annotation_interface.h"
#include "../base_concepts.h"

namespace abckit::arkts {

/**
 * @brief AnnotationInterface
 */
class AnnotationInterface : public core::AnnotationInterface {
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
    /// @brief abckit::DefaultHash<AnnotationInterface>
    friend class abckit::DefaultHash<AnnotationInterface>;
    /// @brief to access private TargetCast
    friend class abckit::traits::TargetCheckCast<AnnotationInterface>;

public:
    /**
     * @brief Constructor Arkts API AnnotationInterface from the Core API with compatibility check
     * @param coreOther - Core API AnnotationInterface
     */
    explicit AnnotationInterface(const core::AnnotationInterface &coreOther);

    /**
     * @brief Construct a new Annotation Interface object
     * @param other
     */
    AnnotationInterface(const AnnotationInterface &other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return AnnotationInterface&
     */
    AnnotationInterface &operator=(const AnnotationInterface &other) = default;

    /**
     * @brief Construct a new Annotation Interface object
     * @param other
     */
    AnnotationInterface(AnnotationInterface &&other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return AnnotationInterface&
     */
    AnnotationInterface &operator=(AnnotationInterface &&other) = default;

    /**
     * @brief Destroy the Annotation Interface object
     */
    ~AnnotationInterface() override = default;

    /**
     * @brief Sets name for annotation interface
     * @return `true` on success.
     * @param [ in ] name - Name to be set.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    bool SetName(const std::string &name) const;

    /**
     * @brief Add new field to the annotation interface.
     * @return Newly created annotation field.
     * @param [ in ] type - Type that is used to create the field of the Annotation Interface.
     * @param [ in ] value - Value that is used to create the field of the Annotation Interface.
     * @param [ in ] name - Name that is used to create the field of the Annotation Interface.
     * @note Allocates
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `value` is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `type` is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if annotation interface itself doesn't have `ABCKIT_TARGET_ARK_TS_V1`
     * target.
     */
    AnnotationInterfaceField AddField(std::string_view name, Type type, Value value);

    /**
     * @brief Remove field from the annotation interface.
     * @param [ in ] field - Field to remove from the Annotation Interface.
     * @return New state of AnnotationInterface.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error view itself is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `field` is false.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if annotation interface doesn't have `ABCKIT_TARGET_ARK_TS_V1`
     * target.
     * @note Set `ABCKIT_STATUS_TODO` error if annotation interface does not have the field `field`.
     */
    AnnotationInterface RemoveField(AnnotationInterfaceField field) const;

private:
    /**
     * @brief Converts annotation interface from Core to Arkts target
     * @return AbckitArktsAnnotationInterface* - converted annotation interface
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `this` is does not have `ABCKIT_TARGET_ARK_TS_V1` or
     * `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    AbckitArktsAnnotationInterface *TargetCast() const;

    ABCKIT_NO_UNIQUE_ADDRESS traits::TargetCheckCast<AnnotationInterface> targetChecker_;
};

}  // namespace abckit::arkts

#endif  // CPP_ABCKIT_ARKTS_ANNOTATION_INTERFACE_H
