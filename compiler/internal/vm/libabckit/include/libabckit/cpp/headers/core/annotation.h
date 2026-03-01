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

#ifndef CPP_ABCKIT_CORE_ANNOTATION_H
#define CPP_ABCKIT_CORE_ANNOTATION_H

#include <functional>

#include "annotation_element.h"
#include "../base_classes.h"

namespace abckit::core {

/**
 * @brief Annotation
 */
class Annotation : public ViewInResource<AbckitCoreAnnotation *, const File *> {
    /// @brief core::Function
    friend class core::Function;
    /// @brief core::AnnotationElement
    friend class core::AnnotationElement;
    /// @brief core::Class
    friend class core::Class;
    /// @brief core::Interface
    friend class core::Interface;
    /// @brief core::ClassField
    friend class core::ClassField;
    /// @brief core::InterfaceField
    friend class core::InterfaceField;
    /// @brief arkts::Class
    friend class arkts::Class;
    /// @brief arkts::Function
    friend class arkts::Function;
    /// @brief abckit::DefaultHash<Annotation>
    friend class abckit::DefaultHash<Annotation>;

protected:
    /// @brief Core API View type
    using CoreViewT = Annotation;

public:
    /**
     * @brief Construct a new Annotation object
     * @param other
     */
    Annotation(const Annotation &other) = default;  // CC-OFF(G.CLS.07): design decision, detail: base_concepts.h

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
    Annotation(Annotation &&other) = default;  // CC-OFF(G.CLS.07): design decision, detail: base_concepts.h

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
     * @brief Get Annotation name
     * @return `std::string`
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    std::string GetName() const;

    /**
     * @brief Get the Interface of Annotation
     * @return core::AnnotationInterface
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    core::AnnotationInterface GetInterface() const;

    /**
     * @brief Tell if annotation is External
     * @return bool
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    bool IsExternal() const;

    /**
     * @brief Enumerates elements of the Annotation, invoking the callback for each element.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cb` is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    bool EnumerateElements(const std::function<bool(core::AnnotationElement)> &cb) const;

private:
    Annotation(AbckitCoreAnnotation *ann, const ApiConfig *conf, const File *file) : ViewInResource(ann), conf_(conf)
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

#endif  // CPP_ABCKIT_CORE_ANNOTATION_H
