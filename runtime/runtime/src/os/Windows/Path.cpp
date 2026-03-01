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


#include "os/Path.h"

namespace MapleRuntime {
namespace Os {
CString Path::GetBaseName(const char* path)
{
    char fname[_MAX_FNAME];
    char ext[_MAX_EXT];
    if (_splitpath_s(path, nullptr, 0, nullptr, 0, fname, _MAX_FNAME, ext, _MAX_EXT)) {
        return nullptr;
    }
    return CString(fname) + CString(ext);
}

bool Path::GetRealPath(const char* path, char* resolvedPath)
{
    return _fullpath(resolvedPath, path, PATH_MAX + 1) != nullptr;
}
} // namespace Os
} // namespace MapleRuntime
