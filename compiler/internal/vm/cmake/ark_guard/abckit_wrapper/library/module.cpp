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
#include "abckit_wrapper/module.h"
#include "object_visitor.h"
#include "annotation_target_visitor.h"

bool abckit_wrapper::Module::SetName(const std::string &name)
{
    const auto module = this->GetArkTsImpl<abckit::core::Module, abckit::arkts::Module>();
    if (!module.has_value()) {
        return false;
    }

    return module->SetName(name);
}

void abckit_wrapper::Module::InitForObject(Object *object)
{
    object->owningModule_ = this;
    object->parent_ = this;
}

bool abckit_wrapper::Module::Accept(ModuleVisitor &visitor)
{
    return visitor.Visit(this);
}

template <typename T>
bool abckit_wrapper::Module::TypedObjectsAccept(T &visitor)
{
    for (auto &[_, object] : this->typedObjectTable_) {
        if (!std::visit(ObjectVisitor(&visitor), object)) {
            return false;
        }
    }

    return true;
}

bool abckit_wrapper::Module::NamespacesAccept(NamespaceVisitor &visitor)
{
    return this->TypedObjectsAccept(visitor);
}

bool abckit_wrapper::Module::MethodsAccept(MethodVisitor &visitor)
{
    return this->TypedObjectsAccept(visitor);
}

bool abckit_wrapper::Module::FieldsAccept(FieldVisitor &visitor)
{
    return this->TypedObjectsAccept(visitor);
}

bool abckit_wrapper::Module::ClassesAccept(ClassVisitor &visitor)
{
    return this->TypedObjectsAccept(visitor);
}

bool abckit_wrapper::Module::AnnotationInterfacesAccept(AnnotationInterfaceVisitor &visitor)
{
    return this->TypedObjectsAccept(visitor);
}

bool abckit_wrapper::Module::AnnotationsAccept(AnnotationVisitor &visitor)
{
    for (auto &[_, object] : this->typedObjectTable_) {
        if (!std::visit(AnnotationTargetVisitor(visitor), object)) {
            return false;
        }
    }

    return true;
}
