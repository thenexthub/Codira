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

#ifndef LIBPANDAFILE_PROTO_DATA_ACCESSOR_H_
#define LIBPANDAFILE_PROTO_DATA_ACCESSOR_H_

#include "libarkfile/file.h"
#include "libarkfile/file_items.h"

namespace ark::panda_file {

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions, hicpp-special-member-functions)
class ProtoDataAccessor {
public:
    ProtoDataAccessor(const File &pandaFile, File::EntityId protoId) : pandaFile_(pandaFile), protoId_(protoId) {}

    ~ProtoDataAccessor() = default;

    const File &GetPandaFile() const
    {
        return pandaFile_;
    }

    File::EntityId GetProtoId() const
    {
        return protoId_;
    }

    Span<const uint8_t> GetShorty() const
    {
        return pandaFile_.GetSpanFromId(protoId_);
    }

    template <class Callback>
    void EnumerateTypes(const Callback &c);

    uint32_t GetNumArgs();

    Type GetArgType(size_t idx) const;

    Type GetReturnType() const;

    File::EntityId GetReferenceType(size_t i);

    Type GetType(size_t idx) const;

    uint32_t GetNumElements();

    size_t GetRefNum()
    {
        if (refTypesSp_.data() == nullptr) {
            SkipShorty();
        }

        return refTypesNum_;
    }

    size_t GetSize()
    {
        if (size_ == 0) {
            SkipShorty();
        }

        return size_;
    }

    bool IsEqual(ProtoDataAccessor *other);

private:
    void SkipShorty();

    const File &pandaFile_;
    File::EntityId protoId_;

    size_t elemNum_ {0};
    Span<const uint8_t> refTypesSp_ {nullptr, nullptr};
    size_t refTypesNum_ {0};
    size_t size_ {0};
};

}  // namespace ark::panda_file

#endif  // LIBPANDAFILE_PROTO_DATA_ACCESSOR_H_
