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

#ifndef ABCKIT_WRAPPER_ANNOTATION_H
#define ABCKIT_WRAPPER_ANNOTATION_H

#include "libabckit/cpp/abckit_cpp.h"
#include "object.h"
#include "visitor.h"

namespace abckit_wrapper {

/**
 * @brief Annotation
 */
class Annotation final : public Object, public SingleImplObject<abckit::core::Annotation> {
public:
    /**
     * @brief Annotation Constructor
     * @param annotation abckit::core::Annotation
     */
    explicit Annotation(abckit::core::Annotation annotation) : SingleImplObject(std::move(annotation)) {}

    std::string GetName() const override;

    bool SetName(const std::string &name) override;

    /**
     * @brief Tells if Annotation is defined in the same binary or externally in another binary.
     * @return Returns `true` if Annotation is defined in another binary and `false` if defined locally.
     */
    bool IsExternal() const;

    /**
     * @brief Accept AnnotationVisitor visit
     * @param visitor AnnotationVisitor
     * @return `false` if was early exited. Otherwise - `true`.
     */
    bool Accept(AnnotationVisitor &visitor);

    /**
     * Annotation referenced AnnotationInterface
     */
    std::optional<AnnotationInterface *> ai_;
    /**
     * Annotation's owner object
     */
    std::optional<Object *> owner_;
};

/**
 * @brief AnnotationTarget
 * which object can be added annotation
 */
class AnnotationTarget {
public:
    /**
     * @brief AnnotationTarget default Destructor
     */
    virtual ~AnnotationTarget() = default;

    /**
     * @brief Annotations accept visit
     * @param visitor AnnotationVisitor
     * @return `false` if was early exited. Otherwise - `true`.
     */
    bool AnnotationsAccept(AnnotationVisitor &visitor);

    /**
     * @brief Get annotation name array
     * @return annotation name array
     */
    std::vector<std::string> GetAnnotationNames() const;

protected:
    /**
     * @brief Init annotations with abckit::core::Annotation vector
     * @return AbckitWrapperErrorCode
     */
    AbckitWrapperErrorCode InitAnnotations();

    /**
     * @brief Init for each annotation
     * @param annotation Annotation
     */
    virtual void InitAnnotation(Annotation *annotation) = 0;

    /**
     * @brief Get abckit::core::Annotation from annotation target impl
     * @return annotations
     */
    virtual std::vector<abckit::core::Annotation> GetAnnotations() const = 0;

    /**
     * Annotation target annotation
     */
    std::unordered_map<std::string, std::unique_ptr<Annotation>> annotations_;
};

}  // namespace abckit_wrapper

#endif  // ABCKIT_WRAPPER_ANNOTATION_H
