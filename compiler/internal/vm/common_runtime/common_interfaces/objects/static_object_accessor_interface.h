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

#ifndef COMMON_INTERFACES_OBJECTS_STATIC_OBJECT_ACCESSOR_INTERFACE_H
#define COMMON_INTERFACES_OBJECTS_STATIC_OBJECT_ACCESSOR_INTERFACE_H

#include "common_interfaces/objects/base_type.h"
#include "common_interfaces/objects/base_object.h"
#include "common_interfaces/thread/thread_holder.h"

namespace common {
// NOTE (#26214): extracted `StaticObjectAccessor` here untill `JSTaggedValue` migrated from ets_runtime
class StaticObjectAccessorInterface {
public:
    // HasProperty is used to check if the static object has a property with the given name.
    virtual bool HasProperty(ThreadHolder *thread, const BaseObject *obj, const char *name) = 0;

    // GetProperty is used to get the value of a property from a static object with the given name.
    virtual BoxedValue GetProperty(ThreadHolder *thread, const BaseObject *obj, const char *name) = 0;

    // SetProperty is used to set the value of a property in a static object with the given name.
    virtual bool SetProperty(ThreadHolder *thread, BaseObject *obj, const char *name, BoxedValue value) = 0;

    // NOTE (#26214): remove untill ets_runtime changes
    // HasElementByIdx is used to check if the static object has an element with the given index.
    virtual bool HasElementByIdx(ThreadHolder *thread, const BaseObject *obj, const uint32_t index) = 0;

    // GetElementByIdx is used to get the value of an element from a static object with the given index.
    virtual BoxedValue GetElementByIdx(ThreadHolder *thread, const BaseObject *obj, const uint32_t index) = 0;

    // SetElementByIdx is used to set the value of an element in a static object with the given index.
    // NOLINTNEXTLINE(misc-misplaced-const)
    virtual bool SetElementByIdx(ThreadHolder *thread, BaseObject *obj, uint32_t index, const BoxedValue value) = 0;
};
}  // namespace common
#endif  // COMMON_INTERFACES_OBJECTS_STATIC_OBJECT_ACCESSOR_INTERFACE_H