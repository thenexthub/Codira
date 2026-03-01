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

#ifndef ASSEMBLER_ASSEMBLY_LABEL_H
#define ASSEMBLER_ASSEMBLY_LABEL_H

#include <string>
#include "assembly-file-location.h"

namespace panda::pandasm {

struct Label {
    std::string name = "";
    std::optional<FileLocation> file_location;

    Label(std::string s, size_t b_l, size_t b_r, std::string f_c, bool d, size_t l_n)
        : name(std::move(s)), file_location({f_c, b_l, b_r, l_n, d})
    {
    }

    explicit Label(std::string s) : name(std::move(s)) {}
};

}  // namespace panda::pandasm

#endif  // ASSEMBLER_ASSEMBLY_LABEL_H
