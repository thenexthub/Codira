/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#ifndef PANDA_RUNTIME_ETS_ETS_STRING_BUILDER_H
#define PANDA_RUNTIME_ETS_ETS_STRING_BUILDER_H

#include "runtime/include/object_header.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "plugins/ets/runtime/types/ets_primitives.h"

namespace ark::ets {

EtsString *StringBuilderToString(ObjectHeader *sb);
ObjectHeader *StringBuilderAppendNullString(ObjectHeader *sb);
ObjectHeader *StringBuilderAppendString(ObjectHeader *sb, EtsString *str);
ObjectHeader *StringBuilderAppendStrings(ObjectHeader *sb, EtsString *str0, EtsString *str1);
ObjectHeader *StringBuilderAppendStrings(ObjectHeader *sb, EtsString *str0, EtsString *str1, EtsString *str2);
ObjectHeader *StringBuilderAppendStrings(ObjectHeader *sb, EtsString *str0, EtsString *str1, EtsString *str2,
                                         EtsString *str3);
ObjectHeader *StringBuilderAppendBool(ObjectHeader *sb, EtsBoolean v);
ObjectHeader *StringBuilderAppendChar(ObjectHeader *sb, EtsChar v);
ObjectHeader *StringBuilderAppendLong(ObjectHeader *sb, EtsLong v);
ObjectHeader *StringBuilderAppendFloat(ObjectHeader *sb, EtsFloat v);
ObjectHeader *StringBuilderAppendDouble(ObjectHeader *sb, EtsDouble v);

namespace test {
class EtsStringBuilderMembersTest;
}  // namespace test

class EtsStringBuilder : public EtsObject {
public:
    EtsStringBuilder() = delete;
    ~EtsStringBuilder() = delete;

    NO_COPY_SEMANTIC(EtsStringBuilder);
    NO_MOVE_SEMANTIC(EtsStringBuilder);

    static EtsStringBuilder *FromCoreType(ObjectHeader *obj)
    {
        return reinterpret_cast<EtsStringBuilder *>(obj);
    }

    ObjectHeader *GetCoreType()
    {
        return reinterpret_cast<ObjectHeader *>(this);
    }

    EtsObject *AsObject()
    {
        return this;
    }

    const EtsObject *AsObject() const
    {
        return this;
    }

    static EtsStringBuilder *FromEtsObject(EtsObject *etsObj)
    {
        return reinterpret_cast<EtsStringBuilder *>(etsObj);
    }

    EtsObjectArray *GetBuf() const
    {
        return EtsObjectArray::FromCoreType(ObjectAccessor::GetObject(this, MEMBER_OFFSET(EtsStringBuilder, buf_)));
    }

    void SetBuf(EtsCoroutine *coro, EtsObjectArray *buf)
    {
        ObjectAccessor::SetObject(coro, this, MEMBER_OFFSET(EtsStringBuilder, buf_), buf->GetCoreType());
    }

    EtsInt GetIndex() const
    {
        return index_;
    }

    void SetIndex(EtsInt index)
    {
        index_ = index;
    }

    EtsInt GetLength() const
    {
        return length_;
    }

    void SetLength(EtsInt length)
    {
        length_ = length;
    }

    EtsBoolean GetCompress() const
    {
        return compress_;
    }

    void SetCompress(EtsBoolean compress)
    {
        compress_ = compress;
    }

    static constexpr size_t GetBufOffset()
    {
        return MEMBER_OFFSET(EtsStringBuilder, buf_);
    }

    static constexpr size_t GetIndexOffset()
    {
        return MEMBER_OFFSET(EtsStringBuilder, index_);
    }

    static constexpr size_t GetLengthOffset()
    {
        return MEMBER_OFFSET(EtsStringBuilder, length_);
    }

    static constexpr size_t GetCompressOffset()
    {
        return MEMBER_OFFSET(EtsStringBuilder, compress_);
    }

private:
    ObjectPointer<EtsObjectArray> buf_;  // array with pointers to strings or char[]
    EtsInt index_;                       // index of the current free element in the buf
    EtsInt length_;                      // length of the resulting string
    EtsBoolean compress_;                // compress or not the resulting string

    friend class test::EtsStringBuilderMembersTest;
};

}  // namespace ark::ets

#endif  // PANDA_RUNTIME_ETS_ETS_STRING_BUILDER_H
