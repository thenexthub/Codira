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

#include <atomic>
#include <cstdint>

#include "common_interfaces/objects/composite_base_class.h"
#include "common_interfaces/objects/base_object.h"

namespace common {
void BaseClassRoots::InitializeCompositeBaseClass(CompositeBaseClassAllocator &allocator)
{
    if (initialized_.exchange(true)) {
        return;
    }
    CreateCompositeBaseClass(ObjectType::LINE_STRING, allocator);
    CreateCompositeBaseClass(ObjectType::SLICED_STRING, allocator);
    CreateCompositeBaseClass(ObjectType::TREE_STRING, allocator);
}

void BaseClassRoots::CreateCompositeBaseClass(ObjectType type, CompositeBaseClassAllocator& allocator)
{
    CompositeBaseClass* classObject = allocator();
    classObject->class_.ClearBitField();
    classObject->class_.SetObjectType(type);
    size_t index = TypeToIndex[static_cast<size_t>(type)];
    compositeBaseClasses_[index] = classObject;
    baseClasses_[index] = &classObject->class_;
}

BaseClass* BaseClassRoots::GetBaseClass(ObjectType type) const
{
    return baseClasses_[TypeToIndex[static_cast<size_t>(type)]];
}

void BaseClassRoots::IterateCompositeBaseClass(const RefFieldVisitor& visitorFunc)
{
    if (!initialized_) {
        return;
    }
    for (auto& it : compositeBaseClasses_) {
        visitorFunc(reinterpret_cast<RefField<>&>(it));
    }
}

} // namespace panda

