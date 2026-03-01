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

#include "utils/utils.h"

#include <iostream>
#include <iterator>
#include <fstream>

namespace panda::utils {
// CC-OFFNXT(G.FUN.01): public API
void GetAsset(const std::string &uri, [[maybe_unused]] uint8_t **buff, [[maybe_unused]] size_t *buffSize,
              std::vector<uint8_t> &content, std::string &ami, bool &useSecureMem, [[maybe_unused]] void **mapper,
              [[maybe_unused]] bool isRestricted)
{
    size_t index = uri.find_last_of(".");
    if (index == std::string::npos) {
        std::cerr << "Invalid uri: " << uri << std::endl;
        return;
    }

    auto path = uri.substr(0, index) + ".abc";
    std::ifstream fs(path, std::ios::binary);
    if (!fs) {
        std::cerr << "Cannot read file: " << path << std::endl;
        return;
    }

    fs.unsetf(std::ios::skipws);

    std::streampos size;
    fs.seekg(0, std::ios::end);
    size = fs.tellg();
    fs.seekg(0, std::ios::beg);

    content.reserve(size);

    content.insert(content.begin(), std::istream_iterator<uint8_t>(fs), std::istream_iterator<uint8_t>());

    ami = path;
    useSecureMem = false;
}

}  // namespace panda::utils
