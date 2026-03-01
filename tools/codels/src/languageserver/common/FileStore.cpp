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

#include "FileStore.h"

#ifndef __linux__
#include <string>
#endif

#ifdef _WIN32
#ifndef PATH_MAX
#define PATH_MAX MAX_PATH
#endif
#elif defined __linux__

#include <dirent.h>

#endif
namespace ark {
#ifdef _WIN32
void LowAbsFileName(char absPathBuff[], size_t len)
{
    if (len == 0) { return; }
    if (FileUtil::GetFileExtension(std::string(absPathBuff)) != "code") { return; }
    for (int i = static_cast<int>(len) - 1; i >= 0; i--) {
        if (absPathBuff[i] == '/' || absPathBuff[i] == '\\') { break; }
        if (isupper(absPathBuff[i])) {
            absPathBuff[i] = tolower(absPathBuff[i]);
        }
    }
}
#endif

std::string FileStore::NormalizePath(const std::string &path)
{
    if (path.length() >= PATH_MAX) {
        return path;
    }
    char absPathBuff[PATH_MAX] = {0};
#ifdef _WIN32
    _fullpath(absPathBuff, path.c_str(), PATH_MAX);
    // we use lower drive letter because cangjie generate fileHash by filePath
    absPathBuff[0] = tolower(absPathBuff[0]);
    LowAbsFileName(absPathBuff, std::string(absPathBuff).length());
#else
    if (realpath(path.c_str(), absPathBuff) == nullptr) {
        return path;
    }
#endif
    std::string str = std::string(absPathBuff);
#ifdef __linux__
    for (auto &s: str) {
        if (s == '\\') {
            s = '/';
        }
    }
#endif
    return str;
}
} // namespace ark
