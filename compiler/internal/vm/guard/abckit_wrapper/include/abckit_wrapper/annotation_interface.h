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

#ifndef ABCKIT_WRAPPER_ANNOTATION_INTERFACE_H
#define ABCKIT_WRAPPER_ANNOTATION_INTERFACE_H

#include "libabckit/cpp/abckit_cpp.h"
#include "object.h"
#include "visitor.h"

namespace abckit_wrapper {
/**
 * @brief AnnotationInterface
 */
class AnnotationInterface final : public Object, public SingleImplObject<abckit::core::AnnotationInterface> {
public:
    /**
     * @brief AnnotationInterface Constructor
     * @param ai abckit::core::AnnotationInterface
     */
    explicit AnnotationInterface(abckit::core::AnnotationInterface ai) : SingleImplObject(std::move(ai)) {}

    std::string GetName() const override;

    bool SetName(const std::string &name) override;

    /**
     * @brief Accept visit
     * @param visitor AnnotationInterfaceVisitor
     * @return `false` if was early exited. Otherwise - `true`.
     */
    bool Accept(AnnotationInterfaceVisitor &visitor);

    /**
     * @brief Accept ChildVisitor visit
     * @param visitor ChildVisitor
     * @return `false` if was early exited. Otherwise - `true`.
     */
    bool Accept(ChildVisitor &visitor);
};
}  // namespace abckit_wrapper

#endif  // ABCKIT_WRAPPER_ANNOTATION_INTERFACE_H
