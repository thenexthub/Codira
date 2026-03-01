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

#include "os/file.h"
#include "utils/type_helpers.h"

#include <errhandlingapi.h>
#include <fcntl.h>
#include <fileapi.h>
#include <libloaderapi.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <tchar.h>
#include <share.h>

namespace panda::os::file {

static int GetFlags(Mode mode)
{
    switch (mode) {
        case Mode::READONLY:
            return _O_RDONLY | _O_BINARY;

        case Mode::READWRITE:
            return _O_RDWR | _O_BINARY;

        case Mode::WRITEONLY:
            return _O_WRONLY | _O_CREAT | _O_TRUNC | _O_BINARY;  // NOLINT(hicpp-signed-bitwise)

        case Mode::READWRITECREATE:
            return _O_RDWR | _O_CREAT | _O_BINARY;  // NOLINT(hicpp-signed-bitwise)

        default:
            break;
    }

    UNREACHABLE();
}

File Open(std::string_view filename, Mode mode)
{
    int fh;
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    const auto PERM = _S_IREAD | _S_IWRITE;
    _sopen_s(&fh, filename.data(), GetFlags(mode), _SH_DENYNO, PERM);
    return File(fh);
}

}  // namespace panda::os::file

namespace panda::os::windows::file {

Expected<std::string, Error> File::GetTmpPath()
{
    WCHAR tempPathBuffer[MAX_PATH];
    DWORD dwRetVal = GetTempPathW(MAX_PATH, tempPathBuffer);
    if (dwRetVal > MAX_PATH || (dwRetVal == 0)) {
        return Unexpected(Error(GetLastError()));
    }
    std::wstring ws(tempPathBuffer);
    return std::string(ws.begin(), ws.end());
}

Expected<std::string, Error> File::GetExecutablePath()
{
    WCHAR path[MAX_PATH];
    DWORD dwRetVal = GetModuleFileNameW(NULL, path, MAX_PATH);
    if (dwRetVal > MAX_PATH || (dwRetVal == 0)) {
        return Unexpected(Error(GetLastError()));
    }
    std::wstring ws(path);
    std::string::size_type pos = std::string(ws.begin(), ws.end()).find_last_of(File::GetPathDelim());

    return (pos != std::string::npos) ? std::string(ws.begin(), ws.end()).substr(0, pos) : std::string("");
}

}  // namespace panda::os::windows::file
