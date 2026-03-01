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

#ifndef PANDA_ASSEMBLER_ASSEMBLY_LABEL_H
#define PANDA_ASSEMBLER_ASSEMBLY_LABEL_H

#include <string>
#include "assembly-file-location.h"

namespace ark::pandasm {

class Label {
public:
    Label() = delete;
    ~Label() = default;

    DEFAULT_MOVE_SEMANTIC(Label);
    NO_COPY_SEMANTIC(Label);

    explicit Label(std::string s) : name(std::move(s)) {}

    explicit Label(std::string s, FileLocation &&fLoc) : name(std::move(s)), fileLocation {std::move(fLoc)} {}

    std::string name;              // NOLINT(misc-non-private-member-variables-in-classes)
    FileLocation fileLocation {};  // NOLINT(misc-non-private-member-variables-in-classes)
};

}  // namespace ark::pandasm

#endif  // PANDA_ASSEMBLER_ASSEMBLY_LABEL_H
