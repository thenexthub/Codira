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

#ifndef ABCKIT_WRAPPER_ANNOTATION_TARGET_VISITOR_H
#define ABCKIT_WRAPPER_ANNOTATION_TARGET_VISITOR_H

#include "abckit_wrapper/annotation.h"
#include "abckit_wrapper/visitor/annotation_visitor.h"

namespace abckit_wrapper {
/**
 * @brief AnnotationTargetVisitor
 */
class AnnotationTargetVisitor {
public:
    /**
     * @brief AnnotationTargetVisitor Constructor
     * @param visitor AnnotationVisitor
     */
    explicit AnnotationTargetVisitor(AnnotationVisitor &visitor) : visitor_(visitor) {}

    /**
     * @brief Visit annotation target for Namespace (ignored)
     * @param ns visited Namespace
     * @return `false` if was early exited. Otherwise - `true`.
     */
    bool operator()(Namespace *ns) const;

    /**
     * @brief Visit annotation target for Method
     * @param method visited Method
     * @return `false` if was early exited. Otherwise - `true`.
     */
    bool operator()(Method *method) const;

    /**
     * @brief Visit annotation target for Field
     * @param field visited Field
     * @return `false` if was early exited. Otherwise - `true`.
     */
    bool operator()(Field *field) const;

    /**
     * @brief Visit annotation target for Class
     * @param clazz visited Class
     * @return `false` if was early exited. Otherwise - `true`.
     */
    bool operator()(Class *clazz) const;

    /**
     * @brief Visit annotation target for AnnotationInterface (ignored)
     * @param ai visited AnnotationInterface
     * @return `false` if was early exited. Otherwise - `true`.
     */
    bool operator()(AnnotationInterface *ai) const;

private:
    AnnotationVisitor &visitor_;
};
}  // namespace abckit_wrapper

#endif  // ABCKIT_WRAPPER_ANNOTATION_TARGET_VISITOR_H
