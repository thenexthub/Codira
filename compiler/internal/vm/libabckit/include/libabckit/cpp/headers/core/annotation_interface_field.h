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

#ifndef CPP_ABCKIT_CORE_ANNOTATION_INTERFACE_FIELD_H
#define CPP_ABCKIT_CORE_ANNOTATION_INTERFACE_FIELD_H

#include "../base_classes.h"
#include "../type.h"

#include <string>

namespace abckit::core {

/**
 * @brief AnnotationInterfaceField
 */
class AnnotationInterfaceField : public ViewInResource<AbckitCoreAnnotationInterfaceField *, const File *> {
    /// @brief core::Annotation
    friend class core::Annotation;
    /// @brief arkts::Annotation
    friend class arkts::Annotation;
    /// @brief core::AnnotationInterface
    friend class core::AnnotationInterface;
    /// @brief arkts::AnnotationInterface
    friend class arkts::AnnotationInterface;
    /// @brief abckit::DefaultHash<AnnotationInterfaceField>
    friend class abckit::DefaultHash<AnnotationInterfaceField>;

protected:
    /// @brief Core API View type
    using CoreViewT = AnnotationInterfaceField;

public:
    /**
     * @brief Construct a new Annotation Interface Field object
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
    AnnotationInterfaceField(AnnotationInterfaceField &&other) = default;  // CC-OFF(G.CLS.07): design decision

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

    /**
     * @brief Returns binary file that the given interface field is a part of.
     * @return `const File *`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    const File *GetFile() const;

    /**
     * @brief Returns name for interface field.
     * @return `std::string`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     * @note Allocates
     */
    std::string GetName() const;

    /**
     * @brief Returns interface for interface field.
     * @return `core::AnnotationInterface`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    core::AnnotationInterface GetInterface() const;

    /**
     * @brief Returns type for interface field.
     * @return `Type`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    Type GetType() const;

    /**
     * @brief Returns default value for interface field.
     * @return `Value`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    Value GetDefaultValue() const;

private:
    AnnotationInterfaceField(AbckitCoreAnnotationInterfaceField *anni, const ApiConfig *conf, const File *file)
        : ViewInResource(anni), conf_(conf)
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

#endif  // CPP_ABCKIT_CORE_ANNOTATION_INTERFACE_FIELD_H
