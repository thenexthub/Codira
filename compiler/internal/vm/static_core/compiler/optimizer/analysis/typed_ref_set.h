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

#ifndef COMPILER_OPTIMIZER_ANALYSIS_TYPED_REF_SET_H
#define COMPILER_OPTIMIZER_ANALYSIS_TYPED_REF_SET_H

#include "libarkbase/utils/bit_vector.h"
#include "optimizer/ir/inst.h"

namespace ark::compiler {

// Unique id of some object. Objects with different IDs are never equal
using Ref = uint32_t;
constexpr Ref ALL_REF = std::numeric_limits<Ref>::max();

ObjectTypeInfo Merge(ObjectTypeInfo lhs, ObjectTypeInfo rhs);

void TryImproveTypeInfo(ObjectTypeInfo &lhs, ObjectTypeInfo rhs);

/// @brief Set of unique `Ref`s (RefSet) represented as sorted vector
template <typename Allocator = ArenaAllocator>
class VectorSet {
public:
    using BitVectorT = BitVector<Allocator>;

    explicit VectorSet(Allocator *allocator) : data_(allocator->Adapter()) {}

    size_t MinBitsToStore() const
    {
        return data_.empty() ? 0 : data_.back() + 1;
    }

    size_t PopCount() const
    {
        return data_.size();
    }

    bool operator==(const VectorSet &other) const
    {
        return data_ == other.data_;
    }

    bool GetBit(size_t index) const
    {
        auto it = std::lower_bound(data_.begin(), data_.end(), index);
        return it != data_.end() && *it == index;
    }

    void SetBit(size_t index)
    {
        auto it = std::lower_bound(data_.begin(), data_.end(), index);
        if (it != data_.end() && *it == index) {
            return;
        }
        data_.insert(it, index);
    }

    BitVectorT TransitionToBitVector()
    {
        auto bitVector = BitVectorT(data_.back(), data_.get_allocator());
        for (auto ref : data_) {
            bitVector.SetBit(ref);
        }
        return bitVector;
    }

    VectorSet &operator|=(const VectorSet &other)
    {
        auto size = data_.size();
        data_.insert(data_.end(), other.data_.begin(), other.data_.end());
        std::inplace_merge(data_.begin(), data_.begin() + size, data_.end());
        data_.erase(std::unique(data_.begin(), data_.end()), data_.end());
        ASSERT(std::is_sorted(data_.begin(), data_.end()));
        return *this;
    }

    friend BitVectorT operator|=(BitVectorT &to, const VectorSet &from)
    {
        // ensure capacity in advance
        if (auto minBits = from.MinBitsToStore(); minBits > to.size()) {
            to.resize(minBits);
        }
        for (auto ref : from.data_) {
            to.SetBit(ref);
        }
        return to;
    }

    Ref GetSingle() const
    {
        ASSERT(data_.size() == 1);
        return data_[0];
    }

    const auto &GetData() const
    {
        return data_;
    }

private:
    std::vector<Ref, typename Allocator::template AdapterType<Ref>> data_;
};

template <typename Allocator, typename Callback>
void VisitBitVector(const VectorSet<Allocator> &vectorSet, Callback &&cb)
{
    for (auto ref : vectorSet.GetData()) {
        cb(ref);
    }
}

template <typename Allocator, typename Callback>
void VisitBitVector(const BitVector<Allocator> &bitVector, Callback &&cb)
{
    auto span = bitVector.GetContainerDataSpan();
    for (size_t maskIdx = 0; maskIdx < span.size(); ++maskIdx) {
        auto mask = span[maskIdx];
        size_t maskOffset = 0;
        while (mask != 0) {
            auto offset = static_cast<size_t>(Ctz(mask));
            mask = (mask >> offset) >> 1U;
            maskOffset += offset;
            size_t oneIdx = maskIdx * sizeof(mask) * BITS_PER_BYTE + maskOffset;
            ++maskOffset;  // consume current bit
            cb(oneIdx);
        }
    }
}

class UniverseSet {
public:
    size_t PopCount() const
    {
        return std::numeric_limits<size_t>::max();
    }

    bool operator==([[maybe_unused]] const UniverseSet &other) const
    {
        return true;
    }

    bool GetBit([[maybe_unused]] size_t index) const
    {
        return true;
    }

    void SetBit([[maybe_unused]] size_t index) {}
};

/**
 * @brief Set of unique `Ref`s (RefSet)
 *
 * Is represented as sorted vector while small;
 * transitions to `BitVector` when grows larger then `VECTOR_THRESHOLD`;
 * transitions to `UniverseSet` when grows larger then `BIT_VECTOR_THRESHOLD`;
 */
template <typename Allocator = StdAllocatorStub>
class SmallSet {
    using VectorSetT = VectorSet<Allocator>;
    using BitVectorT = BitVector<Allocator>;

public:
    explicit SmallSet(Allocator *allocator) : data_(VectorSetT(allocator)) {}
    // CC-OFFNXT(G.CLS.03-CPP) clang-tidy: initializer-list constructor shouldn't be explicit
    SmallSet(std::initializer_list<Ref> values, Allocator *allocator = Allocator::GetInstance()) : SmallSet(allocator)
    {
        for (auto value : values) {
            SetBit(value);
        }
    }
    ~SmallSet() = default;
    DEFAULT_COPY_SEMANTIC(SmallSet);
    DEFAULT_MOVE_SEMANTIC(SmallSet);

    bool operator==(const SmallSet &other) const
    {
        return data_ == other.data_;
    }

    size_t PopCount() const
    {
        return std::visit([](const auto &s) { return s.PopCount(); }, data_);
    }

    bool GetBit(Ref index) const
    {
        if (index == ALL_REF) {
            return std::holds_alternative<UniverseSet>(data_);
        }
        return std::visit([index](const auto &s) { return s.GetBit(index); }, data_);
    }

    void SetBit(Ref index)
    {
        if (index == ALL_REF) {
            MakeUniverse();
            return;
        }
        std::visit([index](auto &s) { s.SetBit(index); }, data_);
        CheckTransition();
    }

    bool Includes(const SmallSet &other) const
    {
        return std::visit(
            [](const auto &to, const auto &from) -> bool {
                using To = std::decay_t<decltype(to)>;
                using From = std::decay_t<decltype(from)>;
                if constexpr (std::is_same_v<To, UniverseSet>) {
                    return true;
                } else if constexpr (std::is_same_v<From, UniverseSet>) {
                    return false;
                } else {
                    return Includes(to, from);
                }
            },
            data_, other.data_);
    }

    SmallSet &operator|=(const SmallSet &other)
    {
        std::visit(
            [this](auto &to, const auto &from) {
                using To = std::decay_t<decltype(to)>;
                using From = std::decay_t<decltype(from)>;
                if constexpr (std::is_same_v<To, UniverseSet>) {
                    // do nothing
                } else if constexpr (std::is_same_v<From, UniverseSet>) {
                    MakeUniverse();
                } else if constexpr (std::is_same_v<To, VectorSetT>) {
                    if constexpr (std::is_same_v<From, VectorSetT>) {
                        to |= from;
                    } else {
                        auto newData = from;
                        newData |= to;
                        data_ = std::move(newData);
                    }
                } else {
                    static_assert(std::is_same_v<To, BitVectorT>);
                    to |= from;
                }
            },
            data_, other.data_);
        CheckTransition();
        return *this;
    }

    Ref GetSingle() const
    {
        return std::get<VectorSetT>(data_).GetSingle();
    }

    template <typename Callback>
    void Visit(Callback &&cb) const
    {
        std::visit(
            [&cb](auto &s) {
                using T = std::decay_t<decltype(s)>;
                if constexpr (std::is_same_v<T, UniverseSet>) {
                    cb(UINT_MAX);
                    UNREACHABLE();
                } else {
                    VisitBitVector(s, cb);
                }
            },
            data_);
    }

    bool IsVectorSet() const
    {
        return std::holds_alternative<VectorSetT>(data_);
    }

private:
    void CheckTransition()
    {
        if (std::holds_alternative<VectorSetT>(data_)) {
            if (PopCount() > VECTOR_THRESHOLD) {
                data_ = std::get<VectorSetT>(data_).TransitionToBitVector();
            }
        } else if (std::holds_alternative<BitVectorT>(data_)) {
            if (std::get<BitVectorT>(data_).capacity() > BIT_VECTOR_THRESHOLD) {
                MakeUniverse();
            }
        } else {
            ASSERT(std::holds_alternative<UniverseSet>(data_));
        }
    }

    void MakeUniverse()
    {
        data_ = UniverseSet {};
    }

    template <typename T>
    static bool Includes(const T &to, const VectorSetT &from)
    {
        for (auto i : from.GetData()) {
            if (!to.GetBit(i)) {
                return false;
            }
        }
        return true;
    }

    template <typename T>
    static bool Includes([[maybe_unused]] const T &to, [[maybe_unused]] const UniverseSet &from)
    {
        return std::is_same_v<T, UniverseSet>;
    }

    static bool Includes([[maybe_unused]] const VectorSetT &to, [[maybe_unused]] const BitVectorT &from)
    {
        // from is definitely larger
        return false;
    }

    static bool Includes(const BitVectorT &to, const BitVectorT &from)
    {
        auto toData = to.GetContainerDataSpan();
        auto fromData = from.GetContainerDataSpan();
        for (size_t i = 0; i < fromData.size(); i++) {
            auto toWord = i >= toData.size() ? 0 : toData[i];
            if ((toWord & fromData[i]) != fromData[i]) {
                return false;
            }
        }
        return true;
    }

private:
    std::variant<VectorSetT, BitVectorT, UniverseSet> data_;
    constexpr static size_t VECTOR_THRESHOLD = 5;
    // NOTE: temp, never trigger
    constexpr static size_t BIT_VECTOR_THRESHOLD = 200'000 * BITS_PER_UINT32;
};

template <typename Allocator = StdAllocatorStub>
class TypedRefSet : private SmallSet<Allocator> {
    using Base = SmallSet<Allocator>;

public:
    TypedRefSet(const Base &base, ObjectTypeInfo typeInfo) : Base(base), typeInfo_(typeInfo) {}
    TypedRefSet(Allocator *allocator, ObjectTypeInfo typeInfo) : Base(allocator), typeInfo_(typeInfo) {}
    ~TypedRefSet() = default;

    DEFAULT_COPY_SEMANTIC(TypedRefSet);
    DEFAULT_MOVE_SEMANTIC(TypedRefSet);

    using Base::GetBit;
    using Base::GetSingle;
    using Base::PopCount;
    using Base::Visit;

    bool operator==(const TypedRefSet &other) const
    {
        return Base::operator==(other) && typeInfo_ == other.typeInfo_;
    }

    bool operator!=(const TypedRefSet &other) const
    {
        return !(*this == other);
    }

    TypedRefSet &operator|=(const TypedRefSet &other)
    {
        Base::operator|=(other);
        typeInfo_ = Merge(typeInfo_, other.typeInfo_);
        return *this;
    }

    void SetBit(Ref index, ObjectTypeInfo typeInfo)
    {
        Base::SetBit(index);
        UpdateTypeInfoOr(typeInfo);
    }

    bool Includes(const TypedRefSet &other) const
    {
        if (typeInfo_ != Merge(typeInfo_, other.typeInfo_)) {
            return false;
        }
        return Base::Includes(other);
    }

    void TryImproveTypeInfo(ObjectTypeInfo otherKnownTypeInfo)
    {
        compiler::TryImproveTypeInfo(typeInfo_, otherKnownTypeInfo);
    }

    void UpdateTypeInfoOr(ObjectTypeInfo newPossibleTypeInfo)
    {
        typeInfo_ = Merge(typeInfo_, newPossibleTypeInfo);
    }

    ObjectTypeInfo GetTypeInfo() const
    {
        return typeInfo_;
    }

    const SmallSet<Allocator> &GetRefSet() const
    {
        return *this;
    }

private:
    ObjectTypeInfo typeInfo_ {ObjectTypeInfo::INVALID};
};

template <typename Allocator>
std::ostream &operator<<(std::ostream &os, const SmallSet<Allocator> &smallSet)
{
    os << "{";
    bool first = true;
    smallSet.Visit([&os, &first](Ref ref) {
        if (!first) {
            os << ", ";
        }
        first = false;
        os << ref;
    });
    return os << "}";
}

using ArenaSmallSet = SmallSet<ArenaAllocator>;
using ArenaTypedRefSet = TypedRefSet<ArenaAllocator>;

}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_ANALYSIS_TYPED_REF_SET_H