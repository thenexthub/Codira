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

#ifndef PANDA_ASSEMBLER_ASSEMBLY_FILE_LOCATION_H
#define PANDA_ASSEMBLER_ASSEMBLY_FILE_LOCATION_H

#include <string>
#include <memory>
#include "libarkbase/macros.h"

namespace ark::pandasm {

inline const std::string EMPTY_LINE {};  // NOLINT(fuchsia-statically-constructed-objects)

// NOLINTBEGIN(misc-non-private-member-variables-in-classes)
class FileLocation {
    std::unique_ptr<std::string> wholeLine_; /* The line in which the field is defined */
                                             /*  Or line in which the field met, if the field is not defined */

public:
    FileLocation() = default;
    ~FileLocation() = default;

    DEFAULT_MOVE_SEMANTIC(FileLocation);
    NO_COPY_SEMANTIC(FileLocation);

    explicit FileLocation(std::string fC, std::uint16_t bL, std::uint16_t bR, std::uint32_t lN, bool d)
        : wholeLine_(std::make_unique<std::string>(std::move(fC))),
          lineNumber(lN),
          boundLeft(bL),
          boundRight(bR),
          isDefined(d)
    {
    }

    std::uint32_t lineNumber = 0U;
    std::uint16_t boundLeft = 0U;
    std::uint16_t boundRight = 0;
    bool isDefined = false;

    std::string const &WholeLine() const noexcept
    {
        return wholeLine_ != nullptr ? *wholeLine_ : EMPTY_LINE;
    }

    template <typename... Args>
    void SetLine(Args &&...args)
    {
        static_assert(std::is_constructible_v<std::string, Args...>, "Invalid string initialization value.");
        wholeLine_ = std::make_unique<std::string>(std::forward<Args>(args)...);
    }
};
// NOLINTEND(misc-non-private-member-variables-in-classes)

}  // namespace ark::pandasm

#endif  // PANDA_ASSEMBLER_ASSEMBLY_FILE_LOCATION_H
