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

#ifndef COMMON_COMPONENTS_COMMON_TYPE_DEF_H
#define COMMON_COMPONENTS_COMMON_TYPE_DEF_H

#include <climits>
#include <cstdint>
#include <limits>

namespace common {
class BaseObject;
}

// commonly agreed type interfaces for a managed runtime:
//    they're opaque across modules, but we still want it provides a degree
//    of type safety.
namespace common {
// Those are mostly managed pointer types for GC
using HeapAddress = uint64_t; // Managed address
constexpr uintptr_t NULL_ADDRESS = 0;

// object model related types
using common::BaseObject;

// basic types for managed world: modify them together
using MIndex = uint64_t;  // index of array

// at first glance, there is no need to expose this type or at least RAW_POINTER_OBJECT.
// however in consideration that there are lots of differences for runtime apis to support different gc,
// this is acceptable.
enum class AllocType {
    MOVEABLE_OBJECT = 0,
    MOVEABLE_OLD_OBJECT,
    NONMOVABLE_OBJECT,
    RAW_POINTER_OBJECT,
    READ_ONLY_OBJECT,
};
} // namespace common

#endif // COMMON_COMPONENTS_COMMON_TYPE_DEF_H
