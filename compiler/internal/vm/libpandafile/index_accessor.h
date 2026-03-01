/**
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef LIBPANDAFILE_INDEX_ACCESSOR_H
#define LIBPANDAFILE_INDEX_ACCESSOR_H

#include "file.h"
#include "file_items.h"

namespace panda::panda_file {

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions, hicpp-special-member-functions)
class IndexAccessor {
public:
    IndexAccessor(const File &pf, File::EntityId method_id)
    {
        constexpr size_t SKIP_NUM = 3;  // 3 : skip class_idx and proto_idx and name
        auto sp = pf.GetSpanFromId(method_id).SubSpan(IDX_SIZE * (SKIP_NUM - 1) + ID_SIZE);
        access_flags_ = helpers::ReadULeb128(&sp);
        header_index_ = access_flags_ >> (FUNTION_KIND_WIDTH + FLAG_WIDTH);
        num_headers_ = pf.GetHeader()->num_indexes;
        const auto *header = &(pf.GetIndexHeaders()[header_index_]);
        indexes_ = pf.GetMethodIndex(header);
    }

    ~IndexAccessor() = default;

    // quick way to resolve offset by 16-bit id in method's instructions
    uint32_t GetOffsetById(uint16_t idx) const
    {
        return indexes_[idx].GetOffset();
    }

    FunctionKind GetFunctionKind() const
    {
        return static_cast<FunctionKind>((access_flags_ & FUNCTION_KIND_MASK) >> FLAG_WIDTH);
    }

    uint16_t GetHeaderIndex() const
    {
        return header_index_;
    }

    uint32_t GetNumHeaders() const
    {
        return num_headers_;
    }

private:
    Span<const File::EntityId> indexes_;
    uint32_t access_flags_;
    uint16_t header_index_;
    uint32_t num_headers_;
};

}  // namespace panda::panda_file

#endif  // LIBPANDAFILE_INDEX_ACCESSOR_H
