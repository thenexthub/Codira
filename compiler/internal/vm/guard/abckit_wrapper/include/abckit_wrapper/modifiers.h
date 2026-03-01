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

#ifndef ABCKIT_WRAPPER_MODIFIERS_H
#define ABCKIT_WRAPPER_MODIFIERS_H

#include <cstdint>

namespace abckit_wrapper {

enum AccessFlags {
    NONE = 0,
    PUBLIC = 1 << 0,
    PROTECTED = 1 << 1,
    INTERNAL = 1 << 2,
    PRIVATE = 1 << 3,
    CCTOR = 1 << 4,  // Static Constructor
    CTOR = 1 << 5,   // Constructor
    STATIC = 1 << 6,
    READONLY = 1 << 7,
    ASYNC = 1 << 8,
    ABSTRACT = 1 << 9,
    FINAL = 1 << 10,
    NATIVE = 1 << 12,
    ANNOTATION = 1 << 13,
    DECLARE = 1 << 14,
};

enum TypeDeclarations {
    INTERFACE = 1 << 0,
    CLASS = 1 << 1,
    ENUM = 1 << 2,
    TYPE = 1 << 3,
    NAMESPACE = 1 << 4,
    PACKAGE = 1 << 5,
};

enum ExtensionType {
    EXTENDS = 1 << 0,
    IMPLEMENTS = 1 << 1,
};

/**
 * @brief AccessFlagsTarget
 * object which has the access flags
 */
class AccessFlagsTarget {
public:
    /**
     * @brief whether accessFlags have targetAccessFlags
     * @param accessFlags input access flags
     * @param targetAccessFlags target access flags
     * @return `true`: (accessFlags & targetAccessFlags) != 0
     */
    static bool HasAccessFlag(uint32_t accessFlags, AccessFlags targetAccessFlags);

    /**
     * @brief AccessFlagsTarget default Destructor
     */
    virtual ~AccessFlagsTarget() = default;

    /**
     * @brief Get access flags
     * @return access flags
     */
    [[nodiscard]] uint32_t GetAccessFlags() const;

protected:
    /**
     * @brief Init access flags
     */
    virtual void InitAccessFlags() = 0;

    /**
     * access flags
     */
    uint32_t accessFlags_ = 0;
};

}  // namespace abckit_wrapper

#endif  // ABCKIT_WRAPPER_MODIFIERS_H
