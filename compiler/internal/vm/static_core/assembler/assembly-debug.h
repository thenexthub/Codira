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

#ifndef PANDA_ASSEMBLER_ASSEMBLY_DEBUG_H
#define PANDA_ASSEMBLER_ASSEMBLY_DEBUG_H

#include <cstdint>
#include <limits>
#include <string>
#include <memory>
#include "libarkbase/macros.h"

namespace ark::pandasm::debuginfo {

inline const std::string EMPTY_LINE {};  // NOLINT(fuchsia-statically-constructed-objects)

// NOLINTBEGIN(misc-non-private-member-variables-in-classes)
class Ins {
    std::uint32_t lineNumber_ = 0U;
    std::uint32_t columnNumber_ = 0U;

public:
    Ins() = default;
    ~Ins() = default;
#if defined(NOT_OPTIMIZE_PERFORMANCE)
    DEFAULT_COPY_SEMANTIC(Ins);
#else
    NO_COPY_SEMANTIC(Ins);
#endif
    DEFAULT_MOVE_SEMANTIC(Ins);

    void SetLineNumber(size_t const ln)
    {
        ASSERT(ln <= std::numeric_limits<uint32_t>::max());
        lineNumber_ = ln;
    }

    void SetColumnNumber(size_t const cn)
    {
        ASSERT(cn <= std::numeric_limits<uint32_t>::max());
        columnNumber_ = cn;
    }

    std::uint32_t LineNumber() const noexcept
    {
        return lineNumber_;
    }

    std::uint32_t ColumnNumber() const noexcept
    {
        return columnNumber_;
    }

    Ins Clone() const
    {
        Ins clone {};
        clone.SetLineNumber(lineNumber_);
        clone.SetColumnNumber(columnNumber_);
        return clone;
    }
};
// NOLINTEND(misc-non-private-member-variables-in-classes)

struct LocalVariable {
    std::string name;
    std::string signature;
    std::string signatureType;
    int32_t reg = 0;
    uint32_t start = 0;
    uint32_t length = 0;
};

}  // namespace ark::pandasm::debuginfo

#endif  // PANDA_ASSEMBLER_ASSEMBLY_DEBUG_H
