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
#include "abckit_wrapper/hierarchy_dumper.h"
#include "logger.h"

namespace {
constexpr std::string_view INDENT = "   ";
}

AbckitWrapperErrorCode abckit_wrapper::HierarchyDumper::Dump()
{
    if (!this->ofstream_.is_open()) {
        LOG_E << "Failed to open dump file";
        return ERR_DUMP_ERROR;
    }

    this->fileView_.ModulesAccept(*this);

    return ERR_SUCCESS;
}

std::string abckit_wrapper::HierarchyDumper::GetIndent(Object *object)
{
    if (!object->data_.has_value()) {
        LOG_W << "object data is empty for:" << object->GetFullyQualifiedName();
        return "";
    }

    if (const auto data = std::any_cast<std::string>(&object->data_)) {
        return *data;
    }

    LOG_W << "object data is not a string for:" << object->GetFullyQualifiedName();
    return "";
}

bool abckit_wrapper::HierarchyDumper::Visit(Module *module)
{
    if (module->IsExternal()) {
        return true;
    }

    module->data_ = std::string();  // indent
    ofstream_ << "Module:" << module->GetFullyQualifiedName() << std::endl;

    module->ChildrenAccept(*this);

    return true;
}

bool abckit_wrapper::HierarchyDumper::VisitNamespace(Namespace *ns)
{
    const auto indent = GetIndent(ns->parent_.value()) + INDENT.data();
    ns->data_ = indent;
    ofstream_ << indent << "Namespace:" << ns->GetName() << std::endl;

    ns->ChildrenAccept(*this);

    return true;
}

bool abckit_wrapper::HierarchyDumper::VisitMethod(Method *method)
{
    const auto indent = GetIndent(method->parent_.value()) + INDENT.data();
    method->data_ = indent;
    ofstream_ << indent << "Function:" << method->GetDescriptor() << std::endl;

    return true;
}

bool abckit_wrapper::HierarchyDumper::VisitField(Field *field)
{
    const auto indent = GetIndent(field->parent_.value()) + INDENT.data();
    field->data_ = indent;
    ofstream_ << indent << "ModuleField:" << field->GetName() << std::endl;

    return true;
}

bool abckit_wrapper::HierarchyDumper::VisitClass(Class *clazz)
{
    const auto indent = GetIndent(clazz->parent_.value()) + INDENT.data();
    clazz->data_ = indent;
    ofstream_ << indent << "Class:" << clazz->GetName() << std::endl;

    if (clazz->superClass_.has_value()) {
        ofstream_ << indent << INDENT << "SuperClass:" << clazz->superClass_.value()->GetFullyQualifiedName()
                  << std::endl;
    }

    if (!clazz->interfaces_.empty()) {
        ofstream_ << indent << INDENT << "Interfaces:[ ";
        for (const auto &[interfaceName, _] : clazz->interfaces_) {
            ofstream_ << interfaceName << ", ";
        }
        ofstream_ << "]" << std::endl;
    }

    if (!clazz->subClasses_.empty()) {
        ofstream_ << indent << INDENT << "SubClasses:[ ";
        for (const auto &[className, _] : clazz->subClasses_) {
            ofstream_ << className << ", ";
        }
        ofstream_ << "]" << std::endl;
    }

    clazz->AnnotationsAccept(*this);

    for (const auto &[_, method] : clazz->methodTable_) {
        method->data_ = indent + INDENT.data();
        ofstream_ << indent << INDENT << "Method:" << method->GetDescriptor() << std::endl;
        method->AnnotationsAccept(*this);
    }

    for (const auto &[_, field] : clazz->fieldTable_) {
        field->data_ = indent + INDENT.data();
        ofstream_ << indent << INDENT << "Field:" << field->GetDescriptor() << std::endl;
        field->AnnotationsAccept(*this);
    }

    return true;
}

bool abckit_wrapper::HierarchyDumper::Visit(Annotation *annotation)
{
    if (annotation->IsExternal()) {
        return true;
    }

    const auto indent = GetIndent(annotation->owner_.value()) + INDENT.data();
    ofstream_ << indent << "Annotation:" << annotation->ai_.value()->GetFullyQualifiedName() << std::endl;

    return true;
}
