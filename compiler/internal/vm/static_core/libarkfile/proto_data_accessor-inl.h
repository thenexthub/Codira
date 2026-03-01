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

#ifndef LIBPANDAFILE_PROTO_DATA_ACCESSOR_INL_H_
#define LIBPANDAFILE_PROTO_DATA_ACCESSOR_INL_H_

#include "libarkfile/helpers.h"

#include "libarkfile/file_items.h"
#include "libarkfile/proto_data_accessor.h"

namespace ark::panda_file {

constexpr size_t SHORTY_ELEM_SIZE = sizeof(uint16_t);
constexpr size_t SHORTY_ELEM_WIDTH = 4;
constexpr size_t SHORTY_ELEM_MASK = 0xf;
constexpr size_t SHORTY_ELEM_PER16 = std::numeric_limits<uint16_t>::digits / SHORTY_ELEM_WIDTH;

inline void ProtoDataAccessor::SkipShorty()
{
    EnumerateTypes([](Type /* unused */) {});
}

template <class Callback>
// CC-OFFNXT(G.FUD.06) solid logic
inline void ProtoDataAccessor::EnumerateTypes(const Callback &c)
{
    auto sp = pandaFile_.GetSpanFromId(protoId_);

    uint32_t v = helpers::Read<SHORTY_ELEM_SIZE>(&sp);
    uint32_t numRef = 0;
    size_ = SHORTY_ELEM_SIZE;
    while (v != 0) {
        size_t shift = (elemNum_ % SHORTY_ELEM_PER16) * SHORTY_ELEM_WIDTH;
        uint8_t elem = (v >> shift) & SHORTY_ELEM_MASK;

        if (elem == 0) {
            break;
        }

        Type t(static_cast<Type::TypeId>(elem));
        c(t);

        if (!t.IsPrimitive()) {
            ++numRef;
        }

        ++elemNum_;

        if ((elemNum_ % SHORTY_ELEM_PER16) == 0) {
            v = helpers::Read<SHORTY_ELEM_SIZE>(&sp);
            size_ += SHORTY_ELEM_SIZE;
        }
    }

    size_ += numRef * IDX_SIZE;
    refTypesNum_ = numRef;
    refTypesSp_ = sp;
}

// CC-OFFNXT(G.FUD.06) solid logic
inline bool ProtoDataAccessor::IsEqual(ProtoDataAccessor *other)
{
    size_t refNum = 0;
    size_t shortyIdx = 0;
    auto sp1 = pandaFile_.GetSpanFromId(protoId_);
    auto sp2 = other->pandaFile_.GetSpanFromId(other->protoId_);
    auto v1 = helpers::Read<SHORTY_ELEM_SIZE>(&sp1);
    auto v2 = helpers::Read<SHORTY_ELEM_SIZE>(&sp2);
    while (true) {
        size_t shift = (shortyIdx % SHORTY_ELEM_PER16) * SHORTY_ELEM_WIDTH;
        uint8_t s1 = (v1 >> shift) & SHORTY_ELEM_MASK;  // NOLINT(hicpp-signed-bitwise)
        uint8_t s2 = (v2 >> shift) & SHORTY_ELEM_MASK;  // NOLINT(hicpp-signed-bitwise)
        if (s1 != s2) {
            return false;
        }
        if (s1 == 0) {
            break;
        }
        panda_file::Type t(static_cast<panda_file::Type::TypeId>(s1));
        if (!t.IsPrimitive()) {
            ++refNum;
        }
        if ((++shortyIdx % SHORTY_ELEM_PER16) == 0) {
            v1 = helpers::Read<SHORTY_ELEM_SIZE>(&sp1);
            v2 = helpers::Read<SHORTY_ELEM_SIZE>(&sp2);
        }
    }

    // compare ref types
    for (size_t refIdx = 0; refIdx < refNum; ++refIdx) {
        auto id1 = pandaFile_.ResolveClassIndex(protoId_, helpers::Read<IDX_SIZE>(&sp1));
        auto id2 = other->pandaFile_.ResolveClassIndex(other->protoId_, helpers::Read<IDX_SIZE>(&sp2));
        if (pandaFile_.GetStringData(id1) != other->pandaFile_.GetStringData(id2)) {
            return false;
        }
    }

    return true;
}

inline uint32_t ProtoDataAccessor::GetNumElements()
{
    if (refTypesSp_.data() == nullptr) {
        SkipShorty();
    }

    return elemNum_;
}

inline uint32_t ProtoDataAccessor::GetNumArgs()
{
    return GetNumElements() - 1;
}

inline File::EntityId ProtoDataAccessor::GetReferenceType(size_t i)
{
    if (refTypesSp_.data() == nullptr) {
        SkipShorty();
    }

    auto sp = refTypesSp_.SubSpan(i * IDX_SIZE);
    auto classIdx = helpers::Read<IDX_SIZE>(&sp);
    return pandaFile_.ResolveClassIndex(protoId_, classIdx);
}

inline Type ProtoDataAccessor::GetType(size_t idx) const
{
    size_t blockIdx = idx / SHORTY_ELEM_PER16;
    size_t elemShift = (idx % SHORTY_ELEM_PER16) * SHORTY_ELEM_WIDTH;

    auto sp = pandaFile_.GetSpanFromId(protoId_);

    sp = sp.SubSpan(SHORTY_ELEM_SIZE * blockIdx);

    uint32_t v = helpers::Read<SHORTY_ELEM_SIZE>(&sp);
    return Type(static_cast<Type::TypeId>((v >> elemShift) & SHORTY_ELEM_MASK));
}

inline Type ProtoDataAccessor::GetReturnType() const
{
    return GetType(0);
}

inline Type ProtoDataAccessor::GetArgType(size_t idx) const
{
    return GetType(idx + 1);
}

}  // namespace ark::panda_file

#endif  // LIBPANDAFILE_PROTO_DATA_ACCESSOR_INL_H_
