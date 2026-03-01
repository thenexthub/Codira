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

#ifndef LIBPANDABASE_OS_LIBRARY_LOADER_H
#define LIBPANDABASE_OS_LIBRARY_LOADER_H

#ifdef PANDA_TARGET_UNIX
#include <dlfcn.h>
#elif defined PANDA_TARGET_WINDOWS
// No platform-specific includes
#else
#error "Unsupported platform"
#endif

#include "os/error.h"
#include "utils/expected.h"

#include <string_view>

namespace panda::os::library_loader {
class LibraryHandle;

Expected<LibraryHandle, Error> Load(std::string_view filename);

Expected<void *, Error> ResolveSymbol(const LibraryHandle &handle, std::string_view name);

void CloseHandle(void *handle);

class LibraryHandle {
public:
    explicit LibraryHandle(void *handle) : handle_(handle) {}

    LibraryHandle(LibraryHandle &&handle) noexcept
    {
        handle_ = handle.handle_;
        handle.handle_ = nullptr;
    }

    LibraryHandle &operator=(LibraryHandle &&handle) noexcept
    {
        handle_ = handle.handle_;
        handle.handle_ = nullptr;
        return *this;
    }

    bool IsValid() const
    {
        return handle_ != nullptr;
    }

    void *GetNativeHandle() const
    {
        return handle_;
    }

    ~LibraryHandle()
    {
        if (handle_ != nullptr) {
            CloseHandle(handle_);
        }
    }

private:
    void *handle_;

    NO_COPY_SEMANTIC(LibraryHandle);
};
}  // namespace panda::os::library_loader

#endif  // LIBPANDABASE_OS_LIBRARY_LOADER_H
