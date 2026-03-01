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

#ifndef LIBPANDAFILE_METHOD_HANDLE_DATA_ACCESSOR_
#define LIBPANDAFILE_METHOD_HANDLE_DATA_ACCESSOR_

#include "libarkfile/file.h"
#include "libarkfile/file_items.h"

namespace ark::panda_file {

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions, hicpp-special-member-functions)
class MethodHandleDataAccessor {
public:
    MethodHandleDataAccessor(const File &pandaFile, File::EntityId methodHandleId);

    ~MethodHandleDataAccessor() = default;

    MethodHandleType GetType() const
    {
        return type_;
    }

    size_t GetSize() const
    {
        return size_;
    }

    const File &GetPandaFile() const
    {
        return pandaFile_;
    }

    File::EntityId GetMethodHandleId() const
    {
        return methodHandleId_;
    }

    File::EntityId GetEntityId() const
    {
        return File::EntityId(offset_);
    }

private:
    const File &pandaFile_;
    File::EntityId methodHandleId_;

    MethodHandleType type_;
    uint32_t offset_;

    size_t size_ {0};
};

}  // namespace ark::panda_file

#endif  // LIBPANDAFILE_METHOD_HANDLE_DATA_ACCESSOR_
