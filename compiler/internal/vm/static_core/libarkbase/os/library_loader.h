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

#ifndef PANDA_LIBBASE_OS_LIBRARY_LOADER_H_
#define PANDA_LIBBASE_OS_LIBRARY_LOADER_H_

#ifdef PANDA_TARGET_UNIX
#include <dlfcn.h>
#elif defined PANDA_TARGET_WINDOWS
// No platform-specific includes
#else
#error "Unsupported platform"
#endif

#include "libarkbase/os/error.h"
#include "libarkbase/utils/expected.h"

#include <string_view>

namespace ark::os::library_loader {
class LibraryHandle;

#if PANDA_TARGET_MACOS
constexpr auto DYNAMIC_LIBRARY_PREFIX = "lib";
constexpr auto DYNAMIC_LIBRARY_SUFFIX = ".dylib";
#elif PANDA_TARGET_WINDOWS
constexpr auto DYNAMIC_LIBRARY_PREFIX = "";
constexpr auto DYNAMIC_LIBRARY_SUFFIX = ".dll";
#else
constexpr auto DYNAMIC_LIBRARY_PREFIX = "lib";
constexpr auto DYNAMIC_LIBRARY_SUFFIX = ".so";
#endif

PANDA_PUBLIC_API Expected<LibraryHandle, Error> Load(std::string_view filename);

PANDA_PUBLIC_API Expected<void *, Error> ResolveSymbol(const LibraryHandle &handle, std::string_view name);

PANDA_PUBLIC_API void CloseHandle(void *handle);

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
}  // namespace ark::os::library_loader

#endif  // PANDA_LIBBASE_OS_LIBRARY_LOADER_H_
