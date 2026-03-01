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
#include "abckit_wrapper/object.h"
#include "object_visitor.h"
#include "logger.h"

namespace {
constexpr std::string_view OBJECT_FULL_NAME_SEP = ".";
}

AbckitWrapperErrorCode abckit_wrapper::Object::Init()
{
    return AbckitWrapperErrorCode::ERR_SUCCESS;
}

std::string abckit_wrapper::Object::GetFullyQualifiedName() const
{
    if (this->GetPackageName().empty()) {
        return this->GetName();
    }

    return this->GetPackageName() + OBJECT_FULL_NAME_SEP.data() + this->GetName();
}

std::string abckit_wrapper::Object::GetPackageName() const
{
    if (this->parent_.has_value()) {
        return this->parent_.value()->GetFullyQualifiedName();
    }

    return "";
}

bool abckit_wrapper::Object::ChildrenAccept(ChildVisitor &visitor)
{
    for (auto &[_, object] : this->children_) {
        if (!std::visit(ObjectVisitor(&visitor), object)) {
            return false;
        }
    }

    return true;
}

void abckit_wrapper::ObjectPool::Add(std::unique_ptr<Object> &&object)
{
    std::string objectName = object->GetFullyQualifiedName();
    if (this->objects_.find(objectName) != this->objects_.end()) {
        LOG_F << "Duplicate object:" << object->GetFullyQualifiedName();
        abort();
    }

    this->objects_.emplace(objectName, std::move(object));
}

std::optional<abckit_wrapper::Object *> abckit_wrapper::ObjectPool::Get(const std::string &objectName) const
{
    const auto it = this->objects_.find(objectName);
    if (it == this->objects_.end()) {
        return std::nullopt;
    }

    return it->second.get();
}

void abckit_wrapper::ObjectPool::ClearObjectsData()
{
    for (auto &[_, object] : this->objects_) {
        object->data_.reset();
    }
}
