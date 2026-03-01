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

#include "utils.h"
#include <fstream>
#include <iomanip>
#include "utils/logger.h"

namespace panda::verifier {

void GenerateModifiedAbc(const std::vector<unsigned char> &buffer, const std::string &filename)
{
    std::ofstream abc_file(filename, std::ios::out | std::ios::binary);
    if (abc_file.fail()) {
        LOG(ERROR, VERIFIER) << "Failed to open file " << filename;
        return;
    }

    abc_file.write(reinterpret_cast<const char *>(buffer.data()), buffer.size());
    abc_file.close();
}

void ConvertToLittleEndian(std::vector<unsigned char> &inner_id, const uint32_t &id)
{
    std::vector<unsigned char> bytes;
    for (int i = 0; i < sizeof(uint32_t); ++i) {
        unsigned char byte = static_cast<unsigned char>((id >> (i * 8)) & 0xff);
        inner_id.push_back(byte);
    }
}

void ModifyBuffer(std::unordered_map<uint32_t, uint32_t> &literal_map, std::vector<unsigned char> &buffer)
{
    for (const auto &literal : literal_map) {
        size_t literal_id = literal.first;
        std::vector<unsigned char> inner_id;
        ConvertToLittleEndian(inner_id, literal.second);
        for (size_t i = literal_id; i < buffer.size(); ++i) {
            if (buffer[i] == inner_id[0] && buffer[i+1] == inner_id[1]) {
                // The purpose of this modification is to break abc
                // The abc is tampered with by setting buffer[i + 1] to buffer[i] and buffer[i + 2] to buffer[i + 1]
                buffer[i] = buffer[i + 1];
                buffer[i + 1] = buffer[i + 2];
                break;
            }
        }
    }
}

} // namespace panda::verifier