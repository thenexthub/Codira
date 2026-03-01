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

#ifndef LIBPANDAFILE_LITERAL_DATA_ACCESSOR_H
#define LIBPANDAFILE_LITERAL_DATA_ACCESSOR_H

#include "file.h"
#include "field_data_accessor.h"
#include "helpers.h"

#include "utils/span.h"
#include "panda_visibility.h"

namespace panda::panda_file {
using StringData = File::StringData;

/* LiteralTag could be extended by different language
// For example, JAVA could use it to represent Array of Integer
// by adding `INTARRAY` in the future
*/
enum class LiteralTag : uint8_t {
    TAGVALUE = 0x00,
    BOOL = 0x01,
    // int_8 and tagvalue have the same range for data representation.
    INTEGER_8 = TAGVALUE,
    INTEGER = 0x02,
    FLOAT = 0x03,
    DOUBLE = 0x04,
    STRING = 0x05,
    METHOD = 0x06,
    GENERATORMETHOD = 0x07,
    ACCESSOR = 0x08,
    METHODAFFILIATE = 0x09,
    ARRAY_U1 = 0x0a,
    ARRAY_U8 = 0x0b,
    ARRAY_I8 = 0x0c,
    ARRAY_U16 = 0x0d,
    ARRAY_I16 = 0x0e,
    ARRAY_U32 = 0x0f,
    ARRAY_I32 = 0x10,
    ARRAY_U64 = 0x11,
    ARRAY_I64 = 0x12,
    ARRAY_F32 = 0x13,
    ARRAY_F64 = 0x14,
    ARRAY_STRING = 0x15,
    ASYNCGENERATORMETHOD = 0x16,
    LITERALBUFFERINDEX = 0x17,
    LITERALARRAY = 0x18,
    BUILTINTYPEINDEX = 0x19,
    GETTER = 0x1a,
    SETTER = 0x1b,
    ETS_IMPLEMENTS = 0x1c,
    NULLVALUE = 0xff
};

class LiteralDataAccessor {
public:
    PANDA_PUBLIC_API LiteralDataAccessor(const File &panda_file, File::EntityId literal_data_id);

    template <class Callback>
    void EnumerateObjectLiterals(size_t index, const Callback &cb);

    template <class Callback>
    void EnumerateLiteralVals(size_t index, const Callback &cb);

    template <class Callback>
    void EnumerateLiteralVals(File::EntityId id, const Callback &cb);

    size_t GetLiteralValsNum(File::EntityId id) const;
    size_t GetLiteralValsNum(size_t index) const;

    uint32_t GetLiteralNum() const
    {
        return literal_num_;
    }

    const File &GetPandaFile() const
    {
        return panda_file_;
    }

    File::EntityId GetLiteralDataId() const
    {
        return literal_data_id_;
    }

    File::EntityId GetLiteralArrayId(size_t index) const
    {
        if (literal_num_ == INVALID_INDEX) {
            return File::EntityId(INVALID_INDEX);
        }

        ASSERT(index < literal_num_);
        auto l_sp = literal_data_sp_.SubSpan(index * ID_SIZE);
        return File::EntityId(static_cast<uint32_t>(helpers::Read<sizeof(uint32_t)>(&l_sp)));
    }

    size_t ResolveLiteralArrayIndex(File::EntityId id) const
    {
        auto offset = id.GetOffset();

        size_t index = 0;
        while (index < literal_num_) {
            auto array_offset = helpers::Read<sizeof(uint32_t)>(literal_data_sp_.SubSpan(index * ID_SIZE));
            if (array_offset == offset) {
                break;
            }
            index++;
        }
        return index;
    }

    using LiteralValue = std::variant<bool, void *, uint8_t, uint16_t, uint32_t, uint64_t, float, double, StringData>;

private:
    static constexpr size_t LEN_SIZE = sizeof(uint32_t);

    const File &panda_file_;
    File::EntityId literal_data_id_;
    uint32_t literal_num_;
    Span<const uint8_t> literal_data_sp_ {nullptr, nullptr};
};

}  // namespace panda::panda_file

#endif  // LIBPANDAFILE_LITERAL_DATA_ACCESSOR_H
