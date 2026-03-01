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

#ifndef COMMON_INTERFACES_OBJECTS_BASE_TYPE_CONVERTER_H
#define COMMON_INTERFACES_OBJECTS_BASE_TYPE_CONVERTER_H

#include "common_interfaces/objects/base_type.h"
#include "common_interfaces/thread/thread_holder.h"

namespace common {
// Type Converter implemented for dynamic vm.
class DynamicTypeConverterInterface {
public:
    // convert BaseType to JSTaggedValue
    virtual JSTaggedValue WrapTagged(ThreadHolder *thread, BaseType value) = 0;

    // convert JSTaggedValue to BaseType
    virtual BaseType UnWrapTagged(JSTaggedValue value) = 0;
};
}  // namespace common
#endif  // COMMON_INTERFACES_OBJECTS_BASE_TYPE_CONVERTER_H