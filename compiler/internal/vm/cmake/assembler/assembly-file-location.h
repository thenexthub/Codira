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

#ifndef ASSEMBLER_ASSEMBLY_FILE_LOCATION_H
#define ASSEMBLER_ASSEMBLY_FILE_LOCATION_H

namespace panda::pandasm {

class FileLocation {
public:
    std::string whole_line = ""; /* The line in which the field is defined */
                                 /*  Or line in which the field met, if the field is not defined */
    size_t bound_left = 0;
    size_t bound_right = 0;
    size_t line_number = 0;
    bool is_defined = false;

public:
    FileLocation(std::string &f_c, size_t b_l, size_t b_r, size_t l_n, bool d)
        : whole_line(std::move(f_c)), bound_left(b_l), bound_right(b_r), line_number(l_n), is_defined(d)
    {
    }
    ~FileLocation() = default;

    DEFAULT_MOVE_SEMANTIC(FileLocation);
    DEFAULT_COPY_SEMANTIC(FileLocation);
};

}  // namespace panda::pandasm

#endif  // ASSEMBLER_ASSEMBLY_FILE_LOCATION_H
