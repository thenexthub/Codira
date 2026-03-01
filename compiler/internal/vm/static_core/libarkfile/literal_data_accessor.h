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

#ifndef LIBPANDAFILE_LITERAL_DATA_ACCESSOR_H_
#define LIBPANDAFILE_LITERAL_DATA_ACCESSOR_H_

#include "libarkfile/file.h"
#include "libarkfile/field_data_accessor.h"
#include "libarkfile/helpers.h"

#include "libarkbase/utils/span.h"

namespace ark::panda_file {
using StringData = File::StringData;

/* LiteralTag could be extended by different language
// For example, JAVA could use it to represent Array of Integer
// by adding `INTARRAY` in the future
*/
enum class LiteralTag : uint8_t {
    TAGVALUE = 0x00,
    BOOL = 0x01,
    INTEGER = 0x02,
    FLOAT = 0x03,
    DOUBLE = 0x04,
    STRING = 0x05,
    BIGINT = 0x06,
    METHOD = 0x07,
    GENERATORMETHOD = 0x08,
    ACCESSOR = 0x09,
    METHODAFFILIATE = 0x0a,
    ARRAY_U1 = 0x0b,
    ARRAY_U8 = 0x0c,
    ARRAY_I8 = 0x0d,
    ARRAY_U16 = 0x0e,
    ARRAY_I16 = 0x0f,
    ARRAY_U32 = 0x10,
    ARRAY_I32 = 0x11,
    ARRAY_U64 = 0x12,
    ARRAY_I64 = 0x13,
    ARRAY_F32 = 0x14,
    ARRAY_F64 = 0x15,
    ARRAY_STRING = 0x16,
    ASYNCGENERATORMETHOD = 0x17,
    ASYNCMETHOD = 0x18,
    LITERALARRAY = 0x19,
    NULLVALUE = 0xff
};

class PANDA_PUBLIC_API LiteralDataAccessor {
public:
    LiteralDataAccessor(const File &pandaFile, File::EntityId literalDataId);

    template <class Callback>
    void EnumerateObjectLiterals(size_t index, const Callback &cb);

    template <class Callback>
    void EnumerateLiteralVals(size_t index, const Callback &cb);

    template <class Callback>
    void EnumerateLiteralVals(File::EntityId id, const Callback &cb);

    size_t GetLiteralValsNum(size_t index);

    uint32_t GetLiteralNum() const
    {
        return literalNum_;
    }

    const File &GetPandaFile() const
    {
        return pandaFile_;
    }

    File::EntityId GetLiteralDataId() const
    {
        return literalDataId_;
    }

    File::EntityId GetLiteralArrayId(size_t index) const
    {
        ASSERT(index < literalNum_);
        auto lSp = literalDataSp_.SubSpan(index * ID_SIZE);
        return File::EntityId(static_cast<uint32_t>(helpers::Read<sizeof(uint32_t)>(&lSp)));
    }

    size_t ResolveLiteralArrayIndex(File::EntityId id) const
    {
        auto offset = id.GetOffset();

        size_t index = 0;
        while (index < literalNum_) {
            auto arrayOffset = helpers::Read<sizeof(uint32_t)>(literalDataSp_.SubSpan(index * ID_SIZE));
            if (arrayOffset == offset) {
                break;
            }
            index++;
        }
        return index;
    }

    using LiteralValue = std::variant<bool, void *, uint8_t, uint16_t, uint32_t, uint64_t, float, double, StringData>;

private:
    static constexpr size_t LEN_SIZE = sizeof(uint32_t);

    const File &pandaFile_;
    File::EntityId literalDataId_;
    uint32_t literalNum_;
    Span<const uint8_t> literalDataSp_ {nullptr, nullptr};
};

}  // namespace ark::panda_file

#endif  // LIBPANDAFILE_Literal_DATA_ACCESSOR_H_
