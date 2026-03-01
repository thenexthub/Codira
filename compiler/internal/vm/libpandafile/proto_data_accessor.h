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

#ifndef LIBPANDAFILE_PROTO_DATA_ACCESSOR_H
#define LIBPANDAFILE_PROTO_DATA_ACCESSOR_H

#include "file.h"
#include "file_items.h"

namespace panda::panda_file {

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions, hicpp-special-member-functions)
class ProtoDataAccessor {
public:
    ProtoDataAccessor(const File &panda_file, File::EntityId proto_id) : panda_file_(panda_file), proto_id_(proto_id) {}

    ~ProtoDataAccessor() = default;

    const File &GetPandaFile() const
    {
        return panda_file_;
    }

    File::EntityId GetProtoId() const
    {
        return proto_id_;
    }

    Span<const uint8_t> GetShorty() const
    {
        return panda_file_.GetSpanFromId(proto_id_);
    }

    template <class Callback>
    void EnumerateTypes(const Callback &c);

    uint32_t GetNumArgs();

    Type GetArgType(size_t idx) const;

    Type GetReturnType() const;

    File::EntityId GetReferenceType(size_t i);

    size_t GetRefNum()
    {
        if (ref_types_sp_.data() == nullptr) {
            SkipShorty();
        }

        return ref_types_num_;
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

    Type GetType(size_t idx) const;

    const File &panda_file_;
    File::EntityId proto_id_;

    size_t elem_num_ {0};
    Span<const uint8_t> ref_types_sp_ {nullptr, nullptr};
    size_t ref_types_num_ {0};
    size_t size_ {0};
};

}  // namespace panda::panda_file

#endif  // LIBPANDAFILE_PROTO_DATA_ACCESSOR_H
