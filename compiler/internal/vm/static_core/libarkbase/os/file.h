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

#ifndef PANDA_LIBPANDABASE_PBASE_OS_FILE_H_
#define PANDA_LIBPANDABASE_PBASE_OS_FILE_H_

#include "libarkbase/macros.h"

#if defined(PANDA_TARGET_UNIX)
#include "platforms/unix/libarkbase/file.h"
#elif defined(PANDA_TARGET_WINDOWS)
#include "platforms/windows/libarkbase//file.h"
#else
#error "Unsupported platform"
#endif

#include <string>

namespace ark::os::file {

#if defined(PANDA_TARGET_UNIX)
using File = ark::os::unix::file::File;
#elif defined(PANDA_TARGET_WINDOWS)
using File = ark::os::windows::file::File;
#endif

class FileHolder {
public:
    explicit FileHolder(File file) : file_(file) {}

    ~FileHolder()
    {
        file_.Close();
    }

private:
    File file_;

    NO_COPY_SEMANTIC(FileHolder);
    NO_MOVE_SEMANTIC(FileHolder);
};

enum class Mode : uint32_t { READONLY, WRITEONLY, READWRITE, READWRITECREATE };

PANDA_PUBLIC_API File Open(std::string_view filename, Mode mode);

}  // namespace ark::os::file

#endif  // PANDA_LIBPANDABASE_PBASE_OS_FILE_H_
