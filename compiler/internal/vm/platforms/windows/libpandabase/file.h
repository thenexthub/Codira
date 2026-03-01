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

#ifndef PLATFORMS_WINDOWS_LIBPANDABASE_FILE_H
#define PLATFORMS_WINDOWS_LIBPANDABASE_FILE_H

#include "os/error.h"
#include "utils/expected.h"
#include "utils/logger.h"

#include <array>
#include <cerrno>
#include <cstddef>
#include <io.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>

namespace panda::os::windows::file {

// In windows API, the length of a path has a limitation of MAX_PATH, which is defined as 260 characters,
// the "\\?\" prefix is used to specify an extended-length path for a maximum length of 32,767 characters.
static const std::string PREFIX_FOR_LONG_PATH = "\\\\?\\";

class File {
public:
    explicit File(int fd) : fd_(fd) {}
    ~File() = default;
    DEFAULT_MOVE_SEMANTIC(File);
    DEFAULT_COPY_SEMANTIC(File);

    Expected<size_t, Error> Read(void *buf, size_t n) const
    {
        auto res = _read(fd_, buf, n);
        if (res < 0) {
            return Unexpected(Error(errno));
        }
        return {static_cast<size_t>(res)};
    }

    bool ReadAll(void *buf, size_t n) const
    {
        auto res = Read(buf, n);
        if (res) {
            return res.Value() == n;
        }

        return false;
    }

    Expected<size_t, Error> Write(const void *buf, size_t n) const
    {
        auto res = _write(fd_, buf, n);
        if (res < 0) {
            return Unexpected(Error(errno));
        }
        return {static_cast<size_t>(res)};
    }

    bool WriteAll(const void *buf, size_t n) const
    {
        auto res = Write(buf, n);
        if (res) {
            return res.Value() == n;
        }

        return false;
    }

    int Close() const
    {
        return _close(fd_);
    }

    Expected<size_t, Error> GetFileSize() const
    {
        struct _stat64 st {
        };
        auto r = _fstat64(fd_, &st);
        if (r == 0) {
            return {static_cast<size_t>(st.st_size)};
        }
        return Unexpected(Error(errno));
    }

    bool IsValid() const
    {
        return fd_ != -1;
    }

    int GetFd() const
    {
        return fd_;
    }

    constexpr static std::string_view GetPathDelim()
    {
        return "\\";
    }

    static const std::string GetExtendedFilePath(const std::string &path)
    {
        if (LIKELY(path.length() < _MAX_PATH)) {
            return path;
        } else {
            return GetExtendedLengthStylePath(path);
        }
    }

    static Expected<std::string, Error> GetTmpPath();

    static Expected<std::string, Error> GetExecutablePath();

    static Expected<std::string, Error> GetAbsolutePath(std::string_view relative_path)
    {
        constexpr size_t MAX_PATH_LEN = 2048;
        std::array<char, MAX_PATH_LEN> buffer = {0};
        auto fp = _fullpath(buffer.data(), relative_path.data(), buffer.size() - 1);
        if (fp == nullptr || _access(buffer.data(), 0) == -1) {
            return Unexpected(Error(errno));
        }

        return std::string(fp);
    }

    static const std::string GetExtendedLengthStylePath(const std::string &path)
    {
        // PREFIX_FOR_LONG_PATH is added to specify it's an extended-length path,
        // and the path needs to be composed of names seperated by backslashes
        std::string extendedPath = PREFIX_FOR_LONG_PATH + path;
        std::replace(extendedPath.begin(), extendedPath.end(), '/', '\\');
        return extendedPath;
    }

    static bool IsDirectory(const std::string &path)
    {
        return HasStatMode(path, _S_IFDIR);
    }

    static bool IsRegularFile(const std::string &path)
    {
        return HasStatMode(path, _S_IFREG);
    }

    bool ClearData()
    {
        // TODO(dkx): check we are not in RO mode

        // SetLength
        {
            auto rc = _chsize(fd_, 0);
            if (rc < 0) {
                PLOG(ERROR, RUNTIME) << "Failed to reset the length";
                return false;
            }
        }

        // Move offset
        {
            auto rc = _lseek(fd_, 0, SEEK_SET);
            if (rc == -1) {
                PLOG(ERROR, RUNTIME) << "Failed to reset the offset";
                return false;
            }
            return true;
        }
    }

    bool Reset()
    {
        return _lseek(fd_, 0L, SEEK_SET) == 0;
    }

    bool SetSeek(long offset)
    {
        return _lseek(fd_, offset, SEEK_SET) >= 0;
    }

    bool SetSeekEnd()
    {
        return _lseek(fd_, 0L, SEEK_END) == 0;
    }

private:
    int fd_;

    static bool HasStatMode(const std::string &path, uint16_t mode)
    {
        struct _stat s = {};

        std::string tmp_path = path;
        if (UNLIKELY(path.length() >= _MAX_PATH)) {
            tmp_path = GetExtendedLengthStylePath(path);
        }
        if (_stat(tmp_path.c_str(), &s) != 0) {
            return false;
        }

        return static_cast<bool>(s.st_mode & mode);
    }
};

}  // namespace panda::os::windows::file

#endif  // PLATFORMS_WINDOWS_LIBPANDABASE_FILE_H
