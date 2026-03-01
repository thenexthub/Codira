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
#include "abckit_wrapper/base_module.h"
#include "logger.h"
#include "abckit_wrapper/module.h"
#include "abckit_wrapper/namespace.h"
#include "abckit_wrapper/class.h"

AbckitWrapperErrorCode abckit_wrapper::BaseModule::Init()
{
    auto rc = this->InitNamespaces();
    if (ABCKIT_WRAPPER_ERROR(rc)) {
        LOG_E << "Failed to init Namespaces:" << rc;
        return rc;
    }

    rc = this->InitFunctions();
    if (ABCKIT_WRAPPER_ERROR(rc)) {
        LOG_E << "Failed to init Functions:" << rc;
        return rc;
    }

    rc = this->InitModuleFields();
    if (ABCKIT_WRAPPER_ERROR(rc)) {
        LOG_E << "Failed to init ModuleFields:" << rc;
        return rc;
    }

    rc = this->InitClasses();
    if (ABCKIT_WRAPPER_ERROR(rc)) {
        LOG_E << "Failed to init Classes:" << rc;
        return rc;
    }

    rc = this->InitInterfaces();
    if (ABCKIT_WRAPPER_ERROR(rc)) {
        LOG_E << "Failed to init Interfaces:" << rc;
        return rc;
    }

    rc = this->InitEnums();
    if (ABCKIT_WRAPPER_ERROR(rc)) {
        LOG_E << "Failed to init Enums:" << rc;
        return rc;
    }

    rc = this->InitAnnotationInterfaces();
    if (ABCKIT_WRAPPER_ERROR(rc)) {
        LOG_E << "Failed to init AnnotationInterfaces:" << rc;
        return rc;
    }

    return ERR_SUCCESS;
}

AbckitWrapperErrorCode abckit_wrapper::BaseModule::InitNamespaces()
{
    return std::visit(
        [&](const auto &object) {
            for (auto &coreNamespace : object.GetNamespaces()) {
                auto ns = std::make_unique<Namespace>(coreNamespace);
                this->InitForObject(ns.get());
                LOG_I << "Found Namespace:" << ns->GetFullyQualifiedName();
                auto rc = ns->Init();
                if (ABCKIT_WRAPPER_ERROR(rc)) {
                    LOG_E << "Failed to init Namespace:" << coreNamespace.GetName() << " errCode:" << rc;
                    return rc;
                }
                this->children_.emplace(ns->GetFullyQualifiedName(), ns.get());
                ns->owningModule_.value()->Add(std::move(ns));
            }

            return ERR_SUCCESS;
        },
        this->impl_);
}

AbckitWrapperErrorCode abckit_wrapper::BaseModule::InitFunctions()
{
    return std::visit(
        [&](const auto &object) {
            for (auto &function : object.GetTopLevelFunctions()) {
                auto method = std::make_unique<Method>(function);
                this->InitForObject(method.get());
                LOG_I << "Found Function:" << method->GetFullyQualifiedName();
                auto rc = method->Init();
                if (ABCKIT_WRAPPER_ERROR(rc)) {
                    LOG_E << "Failed to init Function:" << function.GetName() << " errCode:" << rc;
                    return rc;
                }
                this->children_.emplace(method->GetFullyQualifiedName(), method.get());
                method->owningModule_.value()->Add(std::move(method));
            }

            return ERR_SUCCESS;
        },
        this->impl_);
}

AbckitWrapperErrorCode abckit_wrapper::BaseModule::InitModuleFields()
{
    return std::visit(
        [&](const auto &object) {
            for (auto &coreField : object.GetFields()) {
                auto field = std::make_unique<Field>(coreField);
                this->InitForObject(field.get());
                LOG_I << "Found ModuleField:" << field->GetFullyQualifiedName();
                auto rc = field->Init();
                if (ABCKIT_WRAPPER_ERROR(rc)) {
                    LOG_E << "Failed to init ModuleField:" << coreField.GetName() << " errCode:" << rc;
                    return rc;
                }
                this->children_.emplace(field->GetFullyQualifiedName(), field.get());
                field->owningModule_.value()->Add(std::move(field));
            }

            return ERR_SUCCESS;
        },
        this->impl_);
}

AbckitWrapperErrorCode abckit_wrapper::BaseModule::InitClasses()
{
    return std::visit(
        [&](const auto &object) {
            for (auto &coreClass : object.GetClasses()) {
                auto clazz = std::make_unique<Class>(coreClass);
                this->InitForObject(clazz.get());
                LOG_I << "Found Class:" << clazz->GetFullyQualifiedName();
                auto rc = clazz->Init();
                if (ABCKIT_WRAPPER_ERROR(rc)) {
                    LOG_E << "Failed to init Clazz:" << coreClass.GetName() << " errCode:" << rc;
                    return rc;
                }
                this->children_.emplace(clazz->GetFullyQualifiedName(), clazz.get());
                clazz->owningModule_.value()->Add(std::move(clazz));
            }

            return ERR_SUCCESS;
        },
        this->impl_);
}

AbckitWrapperErrorCode abckit_wrapper::BaseModule::InitInterfaces()
{
    return std::visit(
        [&](const auto &object) {
            for (auto &coreInterface : object.GetInterfaces()) {
                auto clazz = std::make_unique<Class>(coreInterface);
                this->InitForObject(clazz.get());
                LOG_I << "Found Interface:" << clazz->GetFullyQualifiedName();
                auto rc = clazz->Init();
                if (ABCKIT_WRAPPER_ERROR(rc)) {
                    LOG_E << "Failed to init Interface:" << coreInterface.GetName() << " errCode:" << rc;
                    return rc;
                }
                this->children_.emplace(clazz->GetFullyQualifiedName(), clazz.get());
                clazz->owningModule_.value()->Add(std::move(clazz));
            }

            return ERR_SUCCESS;
        },
        this->impl_);
}

AbckitWrapperErrorCode abckit_wrapper::BaseModule::InitEnums()
{
    return std::visit(
        [&](const auto &object) {
            for (auto &coreEnum : object.GetEnums()) {
                auto clazz = std::make_unique<Class>(coreEnum);
                this->InitForObject(clazz.get());
                LOG_I << "Found Enum:" << clazz->GetFullyQualifiedName();
                auto rc = clazz->Init();
                if (ABCKIT_WRAPPER_ERROR(rc)) {
                    LOG_E << "Failed to init Enum:" << coreEnum.GetName() << " errCode:" << rc;
                    return rc;
                }
                this->children_.emplace(clazz->GetFullyQualifiedName(), clazz.get());
                clazz->owningModule_.value()->Add(std::move(clazz));
            }

            return ERR_SUCCESS;
        },
        this->impl_);
}

AbckitWrapperErrorCode abckit_wrapper::BaseModule::InitAnnotationInterfaces()
{
    return std::visit(
        [&](const auto &object) {
            for (auto &annotationInterface : object.GetAnnotationInterfaces()) {
                auto ai = std::make_unique<AnnotationInterface>(annotationInterface);
                this->InitForObject(ai.get());
                LOG_I << "Found AnnotationInterface:" << ai->GetFullyQualifiedName();
                auto rc = ai->Init();
                if (ABCKIT_WRAPPER_ERROR(rc)) {
                    LOG_E << "Failed to init AnnotationInterface:" << annotationInterface.GetName()
                          << " errCode:" << rc;
                    return rc;
                }
                this->children_.emplace(ai->GetFullyQualifiedName(), ai.get());
                ai->owningModule_.value()->Add(std::move(ai));
            }

            return ERR_SUCCESS;
        },
        this->impl_);
}

bool abckit_wrapper::BaseModule::IsExternal()
{
    return std::visit([](const auto &object) { return object.IsExternal(); }, this->impl_);
}

std::string abckit_wrapper::BaseModule::GetName() const
{
    return this->GetObjectName();
}
