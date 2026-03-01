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
#include "abckit_wrapper/field.h"

#include <regex>
#include "logger.h"

std::string abckit_wrapper::Field::GetName() const
{
    return this->GetObjectName();
}

std::string abckit_wrapper::Field::GetType() const
{
    return std::visit([&](const auto &fieldObj) { return fieldObj.GetType().GetName(); }, this->impl_);
}

bool abckit_wrapper::Field::IsInterfaceField() const
{
    return std::holds_alternative<abckit::core::InterfaceField>(this->impl_);
}

bool abckit_wrapper::Field::SetFieldName(const std::string &name) const
{
    const auto moduleField = this->GetArkTsImpl<abckit::core::ModuleField, abckit::arkts::ModuleField>();
    if (moduleField.has_value()) {
        return moduleField->SetName(name);
    }

    const auto nsField = this->GetArkTsImpl<abckit::core::NamespaceField, abckit::arkts::NamespaceField>();
    if (nsField.has_value()) {
        return nsField->SetName(name);
    }

    const auto classField = this->GetArkTsImpl<abckit::core::ClassField, abckit::arkts::ClassField>();
    if (classField.has_value()) {
        return classField->SetName(name);
    }

    const auto interfaceField = this->GetArkTsImpl<abckit::core::InterfaceField, abckit::arkts::InterfaceField>();
    if (interfaceField.has_value()) {
        return interfaceField->SetName(name);
    }

    const auto enumField = this->GetArkTsImpl<abckit::core::EnumField, abckit::arkts::EnumField>();
    if (enumField.has_value()) {
        return enumField->SetName(name);
    }

    return false;
}

bool abckit_wrapper::Field::SetName(const std::string &name)
{
    if (!this->SetFieldName(name)) {
        return false;
    }

    if (ABCKIT_WRAPPER_ERROR(this->InitSignature())) {
        LOG_E << "Failed to refresh signature";
        return false;
    }

    return true;
}

AbckitWrapperErrorCode abckit_wrapper::Field::Init()
{
    this->InitAccessFlags();

    auto rc = this->InitSignature();
    if (ABCKIT_WRAPPER_ERROR(rc)) {
        LOG_E << "Failed to init field descriptor for:" << this->GetFullyQualifiedName();
        return rc;
    }

    LOG_I << "signature:" << this->rawName_ + this->descriptor_;

    rc = this->InitAnnotations();
    if (ABCKIT_WRAPPER_ERROR(rc)) {
        LOG_E << "Failed to init field annotations:" << rc;
        return rc;
    }

    return ERR_SUCCESS;
}

AbckitWrapperErrorCode abckit_wrapper::Field::InitSignature()
{
    const auto cachedName = this->GetName();
    this->rawName_ = std::regex_replace(cachedName, std::regex("%%property-"), "");
    this->descriptor_ = "";
    this->cachedName_ = cachedName;
    return ERR_SUCCESS;
}

void abckit_wrapper::Field::InitAccessFlags()
{
    if (const auto classField = std::get_if<abckit::core::ClassField>(&this->impl_)) {
        this->accessFlags_ |= classField->IsPublic() ? AccessFlags::PUBLIC : AccessFlags::NONE;
        this->accessFlags_ |= classField->IsProtected() ? AccessFlags::PROTECTED : AccessFlags::NONE;
        this->accessFlags_ |= classField->IsPrivate() ? AccessFlags::PRIVATE : AccessFlags::NONE;
        this->accessFlags_ |= classField->IsInternal() ? AccessFlags::INTERNAL : AccessFlags::NONE;
        this->accessFlags_ |= classField->IsStatic() ? AccessFlags::STATIC : AccessFlags::NONE;
        this->accessFlags_ |= classField->IsFinal() ? AccessFlags::FINAL : AccessFlags::NONE;
    }
}

void abckit_wrapper::Field::InitAnnotation(Annotation *annotation)
{
    annotation->owner_ = this;
}

std::vector<abckit::core::Annotation> abckit_wrapper::Field::GetAnnotations() const
{
    if (const auto classField = std::get_if<abckit::core::ClassField>(&this->impl_)) {
        return classField->GetAnnotations();
    }

    if (const auto interfaceField = std::get_if<abckit::core::InterfaceField>(&this->impl_)) {
        return interfaceField->GetAnnotations();
    }

    return {};
}

bool abckit_wrapper::Field::Accept(FieldVisitor &visitor)
{
    return visitor.Visit(this);
}

bool abckit_wrapper::Field::Accept(ChildVisitor &visitor)
{
    return visitor.VisitField(this);
}
