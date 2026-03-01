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

#ifndef PANDA_LINKER_H
#define PANDA_LINKER_H

#include <string>
#include <vector>
#include <set>
#include "libarkfile/file_item_container.h"

namespace ark::static_linker {

struct Result {
    std::vector<std::string> errors;

    struct Stats {
        struct Times {
            uint64_t read {};
            uint64_t merge {};
            uint64_t parse {};
            uint64_t trydelete {};
            uint64_t layout {};
            uint64_t patch {};
            uint64_t write {};
            uint64_t total {};
        } elapsed;
        size_t itemsCount {};
        size_t classCount {};
        size_t codeCount {};
        size_t debugCount {};

        size_t deduplicatedForeigners {};
    } stats;
};

std::ostream &operator<<(std::ostream &o, const Result::Stats &s);

struct Config {
    bool stripDebugInfo = false;
    std::set<std::string> partial {std::string(panda_file::ItemContainer::GetGlobalClassName())};
    std::set<std::string> remainsPartial {};
    std::set<std::string> entryNames {};
    bool allFileIsEntry = false;
};

Config DefaultConfig();

Result Link(const Config &conf, const std::string &output, const std::vector<std::string> &input);
}  // namespace ark::static_linker

#endif
