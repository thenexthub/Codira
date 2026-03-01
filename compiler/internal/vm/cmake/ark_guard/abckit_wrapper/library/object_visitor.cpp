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
#include "object_visitor.h"
#include "abckit_wrapper/class.h"

bool abckit_wrapper::ObjectVisitor::operator()(Namespace *ns) const
{
    return this->Accept<Namespace, NamespaceVisitor>(ns);
}

bool abckit_wrapper::ObjectVisitor::operator()(Method *method) const
{
    return this->Accept<Method, MethodVisitor>(method);
}

bool abckit_wrapper::ObjectVisitor::operator()(Field *field) const
{
    return this->Accept<Field, FieldVisitor>(field);
}

bool abckit_wrapper::ObjectVisitor::operator()(Class *clazz) const
{
    return this->Accept<Class, ClassVisitor>(clazz);
}

bool abckit_wrapper::ObjectVisitor::operator()(AnnotationInterface *ai) const
{
    return this->Accept<AnnotationInterface, AnnotationInterfaceVisitor>(ai);
}
