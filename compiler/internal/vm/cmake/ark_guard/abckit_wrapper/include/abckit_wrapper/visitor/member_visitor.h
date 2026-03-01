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

#ifndef ABCKIT_WRAPPER_MEMBER_VISITOR_H
#define ABCKIT_WRAPPER_MEMBER_VISITOR_H

#include "visitor_wrapper.h"

namespace abckit_wrapper {

class Member;

/**
 * @brief MemberVisitor
 */
class MemberVisitor : public WrappedVisitor<MemberVisitor> {
public:
    /**
     * @brief Visit member
     * @param member visited member
     * @return `false` if was early exited. Otherwise - `true`.
     */
    virtual bool Visit(Member *member) = 0;
};

/**
 * @brief MemberFilter
 * filter the accepted member for visit
 */
class MemberFilter : public MemberVisitor, public BaseVisitorWrapper<MemberVisitor> {
public:
    /**
     * @brief MemberFilter Constructor
     * @param visitor MemberVisitor
     */
    explicit MemberFilter(MemberVisitor &visitor) : BaseVisitorWrapper(visitor) {}

    /**
     * @brief Given member whether accepted
     * @param member visited member
     * @return `true`: member will be visit `false`: member will be skipped
     */
    virtual bool Accepted(Member *member) = 0;

private:
    bool Visit(Member *member) override;
};

/**
 * @brief MemberAccessFlagFilter
 * filter the member by accessFlags
 * 1. (rejectFlags & member.accessFlags) != 0 skipped
 * 2. (acceptFlags & member.accessFlags) != 0 accepted
 */
class MemberAccessFlagFilter final : public MemberFilter {
public:
    /**
     * MemberAccessFlagFilter
     * @param visitor MemberVisitor
     * @param acceptFlags accept accessFlags
     * @param rejectFlags reject accessFlags
     */
    MemberAccessFlagFilter(MemberVisitor &visitor, const uint32_t acceptFlags, const uint32_t rejectFlags = 0)
        : MemberFilter(visitor), accessFlags_(acceptFlags), rejectFlags_(rejectFlags)
    {
    }

private:
    bool Accepted(Member *member) override;

    /**
     * accept accessFlags
     */
    uint32_t accessFlags_;

    /**
     * reject accessFlags
     */
    uint32_t rejectFlags_;
};
}  // namespace abckit_wrapper

#endif  // ABCKIT_WRAPPER_MEMBER_VISITOR_H