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
#include "abckit_wrapper/class.h"
#include "abckit_wrapper/module.h"
#include "logger.h"

AbckitWrapperErrorCode abckit_wrapper::Class::Init()
{
    this->InitAccessFlags();

    auto rc = this->InitMethods();
    if (ABCKIT_WRAPPER_ERROR(rc)) {
        LOG_E << "Failed to init methods:" << rc;
        return rc;
    }

    rc = this->InitFields();
    if (ABCKIT_WRAPPER_ERROR(rc)) {
        LOG_E << "Failed to init fields:" << rc;
        return rc;
    }

    rc = this->InitAnnotations();
    if (ABCKIT_WRAPPER_ERROR(rc)) {
        LOG_E << "Failed to init annotations:" << rc;
        return rc;
    }

    return ERR_SUCCESS;
}

std::string abckit_wrapper::Class::GetName() const
{
    return this->GetObjectName();
}

bool abckit_wrapper::Class::SetName(const std::string &name)
{
    const auto clazz = this->GetArkTsImpl<abckit::core::Class, abckit::arkts::Class>();
    if (clazz.has_value()) {
        return clazz->SetName(name);
    }

    const auto interface = this->GetArkTsImpl<abckit::core::Interface, abckit::arkts::Interface>();
    if (interface.has_value()) {
        return interface->SetName(name);
    }

    const auto enm = this->GetArkTsImpl<abckit::core::Enum, abckit::arkts::Enum>();
    if (enm.has_value()) {
        return enm->SetName(name);
    }

    return false;
}

AbckitWrapperErrorCode abckit_wrapper::Class::InitMethods()
{
    return std::visit(
        [&](const auto &object) {
            for (auto &coreMethod : object.GetAllMethods()) {
                auto method = std::make_unique<Method>(coreMethod);
                method->owningModule_ = this->owningModule_;
                method->parent_ = this;
                LOG_I << "Found Method:" << method->GetFullyQualifiedName();
                auto rc = method->Init();
                if (ABCKIT_WRAPPER_ERROR(rc)) {
                    LOG_E << "Failed to init method:" << coreMethod.GetName() << " errCode:" << rc;
                    return rc;
                }
                this->methodTable_.emplace(method->GetFullyQualifiedName(), method.get());
                this->children_.emplace(method->GetFullyQualifiedName(), method.get());
                method->owningModule_.value()->Add(std::move(method));
            }

            return ERR_SUCCESS;
        },
        this->impl_);
}

AbckitWrapperErrorCode abckit_wrapper::Class::InitFields()
{
    return std::visit(
        [&](const auto &object) {
            for (auto &coreField : object.GetFields()) {
                auto field = std::make_unique<Field>(coreField);
                field->owningModule_ = this->owningModule_;
                field->parent_ = this;
                LOG_I << "Found Field:" << field->GetFullyQualifiedName();
                auto fqName = field->GetFullyQualifiedName();
                if (this->fieldTable_.find(fqName) != this->fieldTable_.end()) {
                    LOG_W << "Duplicated field:" << fqName;
                    continue;
                }
                auto rc = field->Init();
                if (ABCKIT_WRAPPER_ERROR(rc)) {
                    LOG_E << "Failed to init field:" << coreField.GetName() << " errCode:" << rc;
                    return rc;
                }
                this->fieldTable_.emplace(field->GetFullyQualifiedName(), field.get());
                this->children_.emplace(field->GetFullyQualifiedName(), field.get());
                field->owningModule_.value()->Add(std::move(field));
            }

            return ERR_SUCCESS;
        },
        this->impl_);
}

void abckit_wrapper::Class::InitAnnotation(Annotation *annotation)
{
    annotation->owner_ = this;
}

std::vector<abckit::core::Annotation> abckit_wrapper::Class::GetAnnotations() const
{
    if (const auto clazz = std::get_if<abckit::core::Class>(&this->impl_)) {
        return clazz->GetAnnotations();
    }

    if (const auto interface = std::get_if<abckit::core::Interface>(&this->impl_)) {
        return interface->GetAnnotations();
    }

    return {};
}

void abckit_wrapper::Class::InitAccessFlags()
{
    if (const auto clazz = std::get_if<abckit::core::Class>(&this->impl_)) {
        this->accessFlags_ |= clazz->IsFinal() ? AccessFlags::FINAL : AccessFlags::NONE;
        this->accessFlags_ |= clazz->IsAbstract() ? AccessFlags::ABSTRACT : AccessFlags::NONE;
    }
}

bool abckit_wrapper::Class::IsExternal() const
{
    return std::visit([](const auto &object) { return object.IsExternal(); }, this->impl_);
}

bool abckit_wrapper::Class::IsInterface() const
{
    return std::holds_alternative<abckit::core::Interface>(this->impl_);
}

bool abckit_wrapper::Class::IsClass() const
{
    return std::holds_alternative<abckit::core::Class>(this->impl_);
}

bool abckit_wrapper::Class::IsEnum() const
{
    return std::holds_alternative<abckit::core::Enum>(this->impl_);
}

bool abckit_wrapper::Class::Accept(ClassVisitor &visitor)
{
    return visitor.Visit(this);
}

bool abckit_wrapper::Class::MembersAccept(MemberVisitor &visitor)
{
    for (auto &[_, method] : this->methodTable_) {
        if (!method->MemberAccept(visitor)) {
            return false;
        }
    }

    for (auto &[_, field] : this->fieldTable_) {
        if (!field->MemberAccept(visitor)) {
            return false;
        }
    }

    return true;
}

bool abckit_wrapper::Class::Accept(ChildVisitor &visitor)
{
    return visitor.VisitClass(this);
}

bool abckit_wrapper::Class::HierarchyAccept(ClassVisitor &visitor, const bool visitThis, const bool visitSuper,
                                            const bool visitInterfaces, const bool visitSubclasses)
{
    // ignore external class hierarchy visit
    if (this->IsExternal()) {
        return true;
    }

    // 1. visit this
    if (visitThis) {
        if (!this->Accept(visitor)) {
            return false;
        }
    }

    // 2. visit super class recursively
    if (visitSuper && this->superClass_.has_value()) {
        if (!this->superClass_.value()->HierarchyAccept(visitor, true, true, visitInterfaces, false)) {
            return false;
        }
    }

    // 3. visit interfaces recursively
    if (visitInterfaces) {
        // if visit super class is false, add visit super class interfaces
        if (!visitSuper && this->superClass_.has_value()) {
            if (!this->superClass_.value()->HierarchyAccept(visitor, false, false, true, false)) {
                return false;
            }
        }

        // visit interfaces
        for (const auto &[_, interface] : this->interfaces_) {
            if (!interface->HierarchyAccept(visitor, true, false, true, false)) {
                return false;
            }
        }
    }

    // 4. visit subclasses recursively
    if (visitSubclasses) {
        for (const auto &[_, clazz] : this->subClasses_) {
            if (!clazz->HierarchyAccept(visitor, true, false, false, true)) {
                return false;
            }
        }
    }

    return true;
}

