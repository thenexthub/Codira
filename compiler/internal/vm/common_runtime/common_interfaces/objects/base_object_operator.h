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

// NOLINTBEGIN(readability-identifier-naming, cppcoreguidelines-macro-usage,
//             cppcoreguidelines-special-member-functions, modernize-deprecated-headers,
//             readability-else-after-return, readability-duplicate-include,
//             misc-non-private-member-variables-in-classes, cppcoreguidelines-pro-type-member-init,
//             google-explicit-constructor, cppcoreguidelines-pro-type-union-access,
//             modernize-use-auto, llvm-namespace-comment,
//             cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays,
//             readability-implicit-bool-conversion)

#ifndef COMMON_INTERFACES_OBJECTS_BASE_OBJECT_OPERATOR_H
#define COMMON_INTERFACES_OBJECTS_BASE_OBJECT_OPERATOR_H

#include <cstddef>
#include <cstdint>

#include "common_interfaces/heap/heap_visitor.h"
#include "common_interfaces/objects/field.h"
#include "common_interfaces/objects/ref_field.h"
#include "common_interfaces/objects/base_state_word.h"
namespace common {
class BaseObject;

class BaseObjectOperatorInterfaces {
public:
    // Get Object size.
    virtual size_t GetSize(const BaseObject *object) const = 0;
    // Check is valid object.
    virtual bool IsValidObject(const BaseObject *object) const = 0;
    // Iterate object field.
    virtual void ForEachRefField(const BaseObject *object, const RefFieldVisitor &visitor) const = 0;
    // Iterate object field And Get Object Size.
    virtual size_t ForEachRefFieldAndGetSize(const BaseObject *object, const RefFieldVisitor &visitor) const = 0;
    // Get forwarding pointer.
    virtual BaseObject *GetForwardingPointer(const BaseObject *object) const = 0;
    // Set forwarding pointer.
    virtual void SetForwardingPointerAfterExclusive(BaseObject *object, BaseObject *fwdPtr) = 0;

    virtual ~BaseObjectOperatorInterfaces() = default;
};

class BaseObjectOperator {
private:
    BaseObjectOperatorInterfaces *dynamicObjOp_;
    BaseObjectOperatorInterfaces *staticObjOp_;
    friend BaseObject;
};
}  // namespace common
#endif  // COMMON_INTERFACES_OBJECTS_BASE_OBJECT_OPERATOR_H
// NOLINTEND(readability-identifier-naming, cppcoreguidelines-macro-usage,
//           cppcoreguidelines-special-member-functions, modernize-deprecated-headers,
//           readability-else-after-return, readability-duplicate-include,
//           misc-non-private-member-variables-in-classes, cppcoreguidelines-pro-type-member-init,
//           google-explicit-constructor, cppcoreguidelines-pro-type-union-access,
//           modernize-use-auto, llvm-namespace-comment,
//           cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays,
//           readability-implicit-bool-conversion)

