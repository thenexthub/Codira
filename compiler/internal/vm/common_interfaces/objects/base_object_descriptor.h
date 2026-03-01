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

#ifndef COMMON_INTERFACES_OBJECTS_BASE_OBJECT_DESCRIPTOR_H
#define COMMON_INTERFACES_OBJECTS_BASE_OBJECT_DESCRIPTOR_H

#include "objects/base_object.h"
#include "objects/base_type.h"
#include "thread/thread_holder.h"

namespace panda::ecmascript {
class HandlerBase;
}
namespace common {
// for IC
using HandlerBase = panda::ecmascript::HandlerBase;

class DynamicObjectDescriptorInterface {
    // GetProperty is used to get the value of a property and its ic information
    // from a dynamic object with the given name.
    virtual std::pair<JSTaggedValue, HandlerBase> GetProperty(ThreadHolder *thread, const BaseObject *obj,
                                                              const char *name) const = 0;

    // SetProperty is used to set the value of a property and get its ic information
    // from a dynamic object with the given name.
    virtual std::pair<bool, HandlerBase> SetProperty(ThreadHolder *thread, BaseObject *obj, const char *name,
                                                     JSTaggedValue value) = 0;

    // GetElementByIdx is used to get the value of an element and its ic information
    // from a dynamic object with the give index.
    virtual std::pair<JSTaggedValue, HandlerBase> GetElementByIdx(ThreadHolder *thread, const BaseObject *obj,
                                                                  const uint32_t index) const = 0;

    // SetElementByIdx is used to set the value of an element and get its ic information
    // from a dynamic object with the given index.
    virtual std::pair<bool, HandlerBase> SetElementByIdx(ThreadHolder *thread, BaseObject *obj, const uint32_t index,
                                                         JSTaggedValue value) = 0;
};

// interfaces used to access property layout infomation for static object.
// It will be declared and implemented by static vm side.
class StaticObjectDescriptorInterface {};
}  // namespace common
#endif  // COMMON_INTERFACES_OBJECTS_BASE_OBJECT_DESCRIPTOR_H