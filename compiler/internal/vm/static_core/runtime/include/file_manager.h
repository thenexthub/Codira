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

#ifndef PANDA_FILE_MANAGER_H_
#define PANDA_FILE_MANAGER_H_

#include "libarkfile/file.h"
#include "runtime/compiler.h"
#include "runtime/include/mem/panda_string.h"

namespace ark {

class FileManager {
public:
    PANDA_PUBLIC_API static bool LoadAbcFile(std::string_view location, panda_file::File::OpenMode openMode);

    PANDA_PUBLIC_API static bool TryLoadAnFileForLocation(std::string_view abcPath);

    PANDA_PUBLIC_API static bool TryLoadAnFileFromLocation(std::string_view anFileLocation, PandaString &abcFilePrefix,
                                                           std::string_view pandaFileLocation);

    PANDA_PUBLIC_API static Expected<bool, std::string> LoadAnFile(std::string_view anLocation, bool force = false);
};

}  // namespace ark

#endif  // PANDA_FILE_MANAGER_H_
