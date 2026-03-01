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

#ifndef CPP_ABCKIT_CORE_ANNOTATION_ELEMENT_H
#define CPP_ABCKIT_CORE_ANNOTATION_ELEMENT_H

#include "../base_classes.h"
#include "../value.h"

#include <string>

namespace abckit::core {

/**
 * @brief AnnotationElement
 */
class AnnotationElement : public ViewInResource<AbckitCoreAnnotationElement *, const File *> {
    /// @brief core::Annotation
    friend class core::Annotation;
    /// @brief arkts::Annotation
    friend class arkts::Annotation;
    /// @brief core::Module
    friend class core::Module;
    /// @brief arkts::Module
    friend class arkts::Module;
    /// @brief abckit::DefaultHash<AnnotationElement>
    friend class abckit::DefaultHash<AnnotationElement>;

protected:
    /// @brief CoreViewT - Core API View type
    using CoreViewT = AnnotationElement;

public:
    /**
     * @brief Constructor
     * @param other
     */
    AnnotationElement(const AnnotationElement &other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return this
     */
    AnnotationElement &operator=(const AnnotationElement &other) = default;

    /**
     * @brief Constructor
     * @param other
     */
    AnnotationElement(AnnotationElement &&other) = default;  // CC-OFF(G.CLS.07): design decision

    /**
     * @brief Constructor
     * @param other
     * @return AnnotationElement&
     */
    AnnotationElement &operator=(AnnotationElement &&other) = default;

    /**
     * @brief Destructor
     */
    ~AnnotationElement() override = default;

    /**
     * @brief Returns binary file that the Annotation Element is a part of.
     * @return `const File *`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    const File *GetFile() const;

    /**
     * @brief Returns name for annotation element.
     * @return `std::string`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     * @note Allocates
     */
    std::string GetName() const;

    /**
     * @brief Returns value for annotation element.
     * @return `Value`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    Value GetValue() const;

    /**
     * @brief Returns annotation for Annotation Element.
     * @return core::Annotation.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    core::Annotation GetAnnotation() const;

private:
    /**
     * Constructor
     * @param conf
     * @param anne
     * @param file
     */
    AnnotationElement(AbckitCoreAnnotationElement *anne, const ApiConfig *conf, const File *file)
        : ViewInResource(anne), conf_(conf)
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

#endif  // CPP_ABCKIT_CORE_ANNOTATION_ELEMENT_H
