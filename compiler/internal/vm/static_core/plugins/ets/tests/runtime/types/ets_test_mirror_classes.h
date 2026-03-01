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

#ifndef PANDA_RUNTIME_ETS_TEST_MIRROR_CLASSES_H
#define PANDA_RUNTIME_ETS_TEST_MIRROR_CLASSES_H

#include <cstddef>
#include <gtest/gtest.h>
#include <string_view>
#include <libarkbase/macros.h>
#include "types/ets_class.h"
#include "ets_platform_types.h"

namespace ark::ets::test {

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define MIRROR_FIELD_INFO(KLASS, FIELD, ETS_FIELD) MirrorFieldInfo(ETS_FIELD, MEMBER_OFFSET(KLASS, FIELD))

class MirrorFieldInfo {
public:
    constexpr MirrorFieldInfo(const char *name, std::size_t offset) : name_(name), offset_(offset) {}
    ~MirrorFieldInfo() = default;
    DEFAULT_COPY_SEMANTIC(MirrorFieldInfo);
    DEFAULT_MOVE_SEMANTIC(MirrorFieldInfo);

    constexpr const char *Name() const
    {
        return name_.data();
    }

    constexpr std::size_t Offset() const
    {
        return offset_;
    }

    static void CompareMemberOffsets(EtsClass *klass, const std::vector<MirrorFieldInfo> &members, bool checkAll = true)
    {
        ASSERT_NE(nullptr, klass);
        if (checkAll) {
            ASSERT_EQ(members.size(), klass->GetInstanceFieldsNumber());
        }

        for (const MirrorFieldInfo &memb : members) {
            EtsField *field = klass->GetFieldIDByName(memb.Name());
            ASSERT_NE(nullptr, field);
            ASSERT_EQ(memb.Offset(), field->GetOffset())
                << "Offsets of the field '" << memb.Name() << "' are different";
        }
    }

private:
    std::string_view name_;
    std::size_t offset_ = 0;
};

}  // namespace ark::ets::test

#endif  // PANDA_RUNTIME_ETS_TEST_MIRROR_CLASSES_H
