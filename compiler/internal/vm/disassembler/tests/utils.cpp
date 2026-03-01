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

namespace panda::disasm {

void GenerateModifiedAbc(const std::vector<unsigned char> &buffer, const std::string &filename)
{
    std::ofstream abcFile(filename, std::ios::out | std::ios::binary);
    if (abcFile.fail()) {
        LOG(ERROR, DISASSEMBLER) << "Failed to open file " << filename;
        return;
    }

    abcFile.write(reinterpret_cast<const char *>(buffer.data()), buffer.size());
    abcFile.close();
}

}  // namespace panda::disasm