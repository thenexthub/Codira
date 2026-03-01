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

#ifndef ABCKIT_WRAPPER_CHILD_VISITOR_H
#define ABCKIT_WRAPPER_CHILD_VISITOR_H

namespace abckit_wrapper {

class Namespace;
class Method;
class Field;
class Class;
class AnnotationInterface;

/**
 * @brief ChildVisitor
 */
class ChildVisitor {
public:
    /**
     * @brief ChildVisitor default Destructor
     */
    virtual ~ChildVisitor() = default;

    /**
     * @brief Visit namespace
     * @param ns visited namespace
     * @return `false` if was early exited. Otherwise - `true`.
     */
    virtual bool VisitNamespace(Namespace *ns);

    /**
     * @brief Visit method
     * @param method visited method
     * @return `false` if was early exited. Otherwise - `true`.
     */
    virtual bool VisitMethod(Method *method);

    /**
     * @brief Visit field
     * @param field visited field
     * @return `false` if was early exited. Otherwise - `true`.
     */
    virtual bool VisitField(Field *field);

    /**
     * @brief Visit class
     * @param clazz visited class
     * @return `false` if was early exited. Otherwise - `true`.
     */
    virtual bool VisitClass(Class *clazz);

    /**
     * @brief Visit annotationInterface
     * @param ai visited annotationInterface
     * @return `false` if was early exited. Otherwise - `true`.
     */
    virtual bool VisitAnnotationInterface(AnnotationInterface *ai);
};
}  // namespace abckit_wrapper

#endif  // ABCKIT_WRAPPER_CHILD_VISITOR_H
