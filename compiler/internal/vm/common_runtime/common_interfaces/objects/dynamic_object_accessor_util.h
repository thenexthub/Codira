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

#ifndef COMMON_INTERFACES_OBJECTS_BASE_OBJECT_ACCESSOR_UTIL_H
#define COMMON_INTERFACES_OBJECTS_BASE_OBJECT_ACCESSOR_UTIL_H

#include "common_interfaces/objects/base_object.h"
#include "common_interfaces/objects/base_type.h"

namespace common {
// The interface will be implemented in the dynamic runtime to provide the ability to access properties of 1.0 objects.
// Will switch to dynamicObjectAccessor interfaces after hybrid VM and CMC is fixed.
class PUBLIC_API DynamicObjectAccessorUtil {
public:
    // GetProperty is used to get the value of a property from a dynamic object with the given name.
    __attribute__((weak)) static TaggedType* GetProperty(const BaseObject *obj, const char *name);

    // SetProperty is used to set the value of a property in a dynamic object with the given name.
    __attribute__((weak)) static bool SetProperty(const BaseObject *obj, const char *name, TaggedType value);

    __attribute__((weak)) static TaggedType* CallFunction(TaggedType jsThis, TaggedType function, int32_t argc,
                                                          TaggedType *argv);
};
}  // namespace common
#endif  // COMMON_INTERFACES_BASE_OBJECT_ACCESSOR_UTIL_H