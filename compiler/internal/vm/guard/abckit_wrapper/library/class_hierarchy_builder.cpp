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
#include "class_hierarchy_builder.h"
#include "abckit_wrapper/class.h"
#include "full_name_util.h"
#include "logger.h"

AbckitWrapperErrorCode abckit_wrapper::ClassHierarchyBuilder::Build()
{
    // visit all local module classes
    this->fileView_.ModulesAccept(this->Wrap<ModuleClassVisitor>().Wrap<LocalModuleFilter>());
    this->fileView_.ClearObjectsData();

    return this->result_;
}

bool abckit_wrapper::ClassHierarchyBuilder::Visit(Class *clazz)
{
    if (clazz->IsExternal()) {
        return true;
    }

    LOG_I << "build class hierarchy for:" << clazz->GetFullyQualifiedName();
    if (!this->InitSuperClass(clazz)) {
        return false;
    }

    return this->InitInterfaces(clazz);
}

bool abckit_wrapper::ClassHierarchyBuilder::InitSuperClass(Class *clazz)
{
    if (clazz->superClass_.has_value()) {
        return true;
    }

    if (const auto klass = std::get_if<abckit::core::Class>(&clazz->impl_)) {
        const auto superClassFullName = FullNameUtil::GetFullName(klass->GetSuperClass());
        auto superClass = this->fileView_.Get<Class>(superClassFullName);
        if (!superClass.has_value()) {
            LOG_E << "Failed to get super class:" << superClassFullName;
            this->result_ = ERR_INNER_ERROR;
            return false;
        }

        LOG_I << "Found super class:" << superClassFullName;
        clazz->superClass_ = superClass.value();
        superClass.value()->subClasses_.emplace(clazz->GetFullyQualifiedName(), clazz);
    }

    return true;
}

bool abckit_wrapper::ClassHierarchyBuilder::InitInterfaces(Class *clazz)
{
    std::vector<abckit::core::Interface> interfaces;

    if (const auto klass = std::get_if<abckit::core::Class>(&clazz->impl_)) {
        interfaces = klass->GetInterfaces();
    }

    if (const auto klass = std::get_if<abckit::core::Interface>(&clazz->impl_)) {
        interfaces = klass->GetSuperInterfaces();
    }

    for (const auto &coreInterface : interfaces) {
        const auto interfaceFullName = FullNameUtil::GetFullName(coreInterface);
        auto interface = this->fileView_.Get<Class>(interfaceFullName);
        if (!interface.has_value()) {
            LOG_E << "Failed to get interface:" << interfaceFullName;
            this->result_ = ERR_INNER_ERROR;
            return false;
        }

        LOG_I << "Found implemented interface:" << interfaceFullName;
        clazz->interfaces_.emplace(interfaceFullName, interface.value());
        interface.value()->subClasses_.emplace(clazz->GetFullyQualifiedName(), clazz);
    }

    return true;
}
