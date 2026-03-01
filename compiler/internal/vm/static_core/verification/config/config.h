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

#ifndef PANDA_VERIF_CONFIG_H_
#define PANDA_VERIF_CONFIG_H_

#include "libarkbase/utils/logger.h"

#include "runtime/include/mem/panda_string.h"

#include <vector>
#include <string>
#include <cstring>
#include <stdexcept>

namespace ark::verifier::config {
struct Section {
    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    PandaString name;
    std::vector<struct Section> sections;
    std::vector<PandaString> items;
    // NOLINTEND(misc-non-private-member-variables-in-classes)
    PandaString Image(size_t align = 0) const
    {
        PandaString spaces(align, ' ');
        PandaString result = spaces + name + " {\n";
        for (const auto &s : sections) {
            result += s.Image(align + 2UL);
        }
        for (const auto &i : items) {
            result += spaces;
            result += "  ";
            result += i;
            result += "\n";
        }
        result += spaces + "}\n";
        return result;
    }
    const Section &operator[](const PandaString &title) const
    {
        for (const auto &s : sections) {
            if (s.name == title) {
                return s;
            }
        }
        LOG(FATAL, VERIFIER) << "config section '" << title << "' not found";
        UNREACHABLE();
    }
};
}  // namespace ark::verifier::config

#endif  // PANDA_VERIF_CONFIG_H_
