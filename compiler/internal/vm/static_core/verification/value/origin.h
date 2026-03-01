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

#ifndef PANDA_VERIFIER_VALUE_ORIGIN_H_
#define PANDA_VERIFIER_VALUE_ORIGIN_H_

#include "verification/util/enum_tag.h"
#include "verification/util/tagged_index.h"

#include "libarkbase/macros.h"

#include <cstdint>
#include <limits>

namespace ark::verifier {
enum class OriginType { START, INSTRUCTION };

using OriginTypeTag = TagForEnum<OriginType, OriginType::START, OriginType::INSTRUCTION>;

template <typename BytecodeInstruction>
class Origin : public TaggedIndex<OriginTypeTag> {
    using Base = TaggedIndex<OriginTypeTag>;

public:
    explicit Origin(const BytecodeInstruction &inst)
    {
        Base::SetTag<0>(OriginType::INSTRUCTION);
        Base::SetInt(inst.GetOffset());
    }

    Origin(OriginType t, size_t val)
    {
        Base::SetTag<0>(t);
        Base::SetInt(val);
    }

    bool AtStart() const
    {
        ASSERT(Base::IsValid());
        return Base::GetTag<0>() == OriginType::START;
    }

    uint32_t GetOffset() const
    {
        ASSERT(Base::IsValid());
        return static_cast<uint32_t>(Base::GetInt());
    };

    Origin() = default;
    Origin(const Origin &) = default;
    Origin(Origin &&) = default;
    ~Origin() = default;
    Origin &operator=(const Origin &) = default;
    Origin &operator=(Origin &&) = default;
};
}  // namespace ark::verifier

#endif  // PANDA_VERIFIER_VALUE_ORIGIN_H_
