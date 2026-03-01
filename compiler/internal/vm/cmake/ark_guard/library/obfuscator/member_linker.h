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

#ifndef GUARD_OBFUSCATOR_MEMBER_LINKER_H
#define GUARD_OBFUSCATOR_MEMBER_LINKER_H

#include <unordered_map>

#include "abckit_wrapper/file_view.h"

#include "error.h"

namespace ark::guard {
/**
 * @brief MemberLinker
 * link class hierarchy associated members by descriptor
 */
class MemberLinker final : public abckit_wrapper::ClassVisitor, public abckit_wrapper::MemberVisitor {
public:
    /**
     * @brief MemberLinker Constructor
     * @param fileView abckit_wrapper::FileView
     */
    explicit MemberLinker(abckit_wrapper::FileView &fileView) : fileView_(fileView) {}

    /**
     * @brief Link class hierarchy associated members by descriptor
     * @return ErrorCode::ERR_SUCCESS for success, otherwise failed
     */
    ErrorCode Link();

    /**
     * @brief Get Member Linker Last Member
     * @param member abckit_wrapper::Member *
     * @return abckit_wrapper::Member * last member
     */
    static abckit_wrapper::Member *LastMember(abckit_wrapper::Member *member);

    /**
     * @brief Get Member Linker Next Member
     * @param member abckit_wrapper::Member *
     * @return abckit_wrapper::Member * next member
     */
    static abckit_wrapper::Member *NextMember(abckit_wrapper::Member *member);

private:
    bool Visit(abckit_wrapper::Class *clazz) override;

    bool Visit(abckit_wrapper::Member *member) override;

    static void Link(abckit_wrapper::Member *member1, abckit_wrapper::Member *member2);

    abckit_wrapper::FileView &fileView_;

    /**
     * linked members value: member descriptor value: last member
     */
    std::unordered_map<std::string, abckit_wrapper::Member *> linkerMembers_;
};
}  // namespace ark::guard

#endif  // GUARD_OBFUSCATOR_MEMBER_LINKER_H
