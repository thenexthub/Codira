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

#ifndef PANDA_LIBPANDABASE_PBASE_OS_WINDOWS_FILE_H_
#define PANDA_LIBPANDABASE_PBASE_OS_WINDOWS_FILE_H_

#include "libarkbase/os/error.h"
#include "libarkbase/utils/expected.h"
#include "libarkbase/utils/logger.h"

#include <array>
#include <cerrno>
#include <cstddef>
#include <io.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>

namespace ark::os::windows::file {

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
        struct _stat64 st {};
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

    static Expected<std::string, Error> GetTmpPath();

    static Expected<std::string, Error> GetExecutablePath();

    static Expected<std::string, Error> GetAbsolutePath(std::string_view relativePath)
    {
        std::array<char, _MAX_PATH> buffer = {0};
        auto fp = _fullpath(buffer.data(), relativePath.data(), buffer.size() - 1);
        if (fp == nullptr) {
            return Unexpected(Error(errno));
        }

        return std::string(fp);
    }

    static bool IsDirectory(const std::string &path)
    {
        return HasStatMode(path, _S_IFDIR);
    }

    static bool IsRegularFile(const std::string &path)
    {
        return HasStatMode(path, _S_IFREG);
    }

    static bool IsRegularFileCaseSensitive(const std::string &path)
    {
        return HasStatModeCaseSensitive(path, _S_IFREG);
    }

    bool ClearData()
    {
        // NOTE(dkx): check we are not in RO mode

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

    static void GetEndLine(std::ostream &os, const std::size_t numberOfEndLines = 1)
    {
        for (std::size_t i = 0; i < numberOfEndLines; i++) {
            os << std::endl;
        }
    }

private:
    int fd_;

    static bool HasStatMode(const std::string &path, uint16_t mode);
    static bool HasStatModeCaseSensitive(const std::string &path, uint16_t mode);
};

}  // namespace ark::os::windows::file
#endif
