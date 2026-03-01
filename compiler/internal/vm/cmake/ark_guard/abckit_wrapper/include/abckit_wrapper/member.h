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

#ifndef ABCKIT_WRAPPER_MEMBER_H
#define ABCKIT_WRAPPER_MEMBER_H

#include "object.h"
#include "annotation.h"
#include "modifiers.h"

namespace abckit_wrapper {

/**
 * @brief Member
 * Class method or field
 */
class Member : public Object, public AnnotationTarget, public AccessFlagsTarget {
public:
    /**
     * @brief Get member descriptor
     * 1. normal method: (param1Type, param2Type, ..., paramNType):returnType
     * 2. setter&getter method: %%set-field(fieldType) -> field; %%get-()fieldType -> field
     * 3. field: ""
     * @return member descriptor
     */
    [[nodiscard]] std::string GetDescriptor();

    /**
     * @brief Get member raw name
     * 1. normal method: methodName
     * 2. setter&getter method: %%set-field(fieldType) -> field; %%get-()fieldType -> field
     * 3. field: fieldName (ignore %%property-)
     * @return member raw name
     */
    [[nodiscard]] std::string GetRawName();

    /**
     * @brief Get member signature
     * @return member rawName + descriptor
     */
    [[nodiscard]] std::string GetSignature();

    /**
     * @brief Member accept visit
     * @param visitor MemberVisitor
     * @return `false` if was early exited. Otherwise - `true`.
     */
    bool MemberAccept(MemberVisitor &visitor);

    /**
     * @brief Tells if Member is constructor
     * @return `true` if Member is constructor
     */
    virtual bool IsConstructor() const;

protected:
    /**
     * @brief Init member's signature, signature contains rawName and descriptor
     * @return `ERR_SUCCESS` for success otherwise for false
     */
    virtual AbckitWrapperErrorCode InitSignature() = 0;

private:
    /**
     * @brief Refresh signature (if cachedName changed)
     * @return `ERR_SUCCESS` for success otherwise for false
     */
    AbckitWrapperErrorCode RefreshSignature();

public:
    /**
     * Member raw name
     */
    std::string rawName_;

    /**
     * Member descriptor
     */
    std::string descriptor_;

protected:
    /**
     * Member cached name, if member cached name changed, rawName and descriptor should be refreshed
     */
    std::string cachedName_;
};
}  // namespace abckit_wrapper

#endif  // ABCKIT_WRAPPER_MEMBER_H
