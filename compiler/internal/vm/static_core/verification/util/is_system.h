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

#ifndef PANDA_VERIFICATION_UTIL_IS_SYSTEM_H
#define PANDA_VERIFICATION_UTIL_IS_SYSTEM_H

#include "runtime/include/runtime.h"

namespace ark::verifier {

inline bool IsSystemFile(const panda_file::File &file)
{
    const std::string &name = file.GetFilename();
    const RuntimeOptions &options = Runtime::GetCurrent()->GetOptions();
    const auto &bootPandaFiles = options.GetBootPandaFiles();
    // IsLoadInBoot is legacy option. Update when issue #30812 is solved
    size_t filesLen = options.IsLoadInBoot() ? bootPandaFiles.size() - 1 : bootPandaFiles.size();
    for (size_t i = 0; i < filesLen; i++) {
        if (name == bootPandaFiles[i]) {
            return true;
        }
    }
    return false;
}

template <bool INCLUDE_SYNTHETIC_CLASSES = false>
bool IsSystemClass(Class const *klass)
{
    const auto *file = klass->GetPandaFile();
    if (file == nullptr) {
        return INCLUDE_SYNTHETIC_CLASSES;
    }
    return IsSystemFile(*file);
}

inline bool IsSystemOrSyntheticClass(Class const *klass)
{
    return IsSystemClass<true>(klass);
}

}  // namespace ark::verifier
#endif  // PANDA_VERIFICATION_UTIL_IS_SYSTEM_H
