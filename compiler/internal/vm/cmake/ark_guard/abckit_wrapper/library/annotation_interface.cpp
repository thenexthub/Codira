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
#include "abckit_wrapper/annotation_interface.h"

std::string abckit_wrapper::AnnotationInterface::GetName() const
{
    return this->GetObjectName();
}

bool abckit_wrapper::AnnotationInterface::SetName(const std::string &name)
{
    return this->SetObjectName<abckit::arkts::AnnotationInterface>(name);
}

bool abckit_wrapper::AnnotationInterface::Accept(AnnotationInterfaceVisitor &visitor)
{
    return visitor.Visit(this);
}

bool abckit_wrapper::AnnotationInterface::Accept(ChildVisitor &visitor)
{
    return visitor.VisitAnnotationInterface(this);
}
