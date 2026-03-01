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

#ifndef PANDA_VERIFIER_UTIL_TAGGED_INDEX_HPP_
#define PANDA_VERIFIER_UTIL_TAGGED_INDEX_HPP_

#include "libarkbase/macros.h"
#include "libarkbase/utils/bit_utils.h"

#include "verification/util/index.h"
#include "verification/util/hash.h"

#include <limits>
#include <tuple>
#include <type_traits>

#include <iostream>

namespace ark::verifier {

template <typename... Tag>
class TagPack {
public:
    static constexpr size_t BITS = 0ULL;
    static constexpr size_t QUANTITY = 0ULL;
    static constexpr size_t TAG_SHIFT = 0ULL;
    static constexpr size_t TAG_BITS = 0ULL;

    static constexpr auto GetTagShift()
    {
        return std::tuple<> {};
    }
    static constexpr auto GetTagBits()
    {
        return std::tuple<> {};
    }
    template <typename, size_t>
    static constexpr auto GetTagMask()
    {
        return std::tuple<> {};
    }
    static constexpr auto GetTagHandler()
    {
        return std::tuple<> {};
    }
    template <typename Int, const size_t SHIFT>
    static constexpr void SetTags([[maybe_unused]] Int &val)
    {
    }
};

template <typename T, typename... Tag>
class TagPack<T, Tag...> : private TagPack<Tag...> {
    using Base = TagPack<Tag...>;

public:
    static constexpr size_t BITS = Base::BITS + T::BITS;
    static constexpr size_t QUANTITY = Base::QUANTITY + 1ULL;
    static constexpr size_t TAG_SHIFT = Base::BITS;
    static constexpr size_t TAG_BITS = T::BITS;

    static constexpr auto GetTagShift()
    {
        return std::tuple_cat(std::tuple<size_t> {TAG_SHIFT}, Base::GetTagShift());
    }
    static constexpr auto GetTagBits()
    {
        return std::tuple_cat(std::tuple<size_t> {TAG_BITS}, Base::GetTagBits());
    }
    template <typename Int, const size_t SHIFT>
    static constexpr auto GetMask()
    {
        using UInt = std::make_unsigned_t<Int>;
        UInt mask = ((static_cast<UInt>(1) << TAG_BITS) - static_cast<UInt>(1)) << TAG_SHIFT;
        return mask << SHIFT;
    }
    template <typename Int, const size_t SHIFT>
    static constexpr auto GetTagMask()
    {
        using UInt = std::make_unsigned_t<Int>;
        return std::tuple_cat(std::tuple<UInt> {GetMask<Int, SHIFT>()}, Base::template GetTagMask<Int, SHIFT>());
    }
    static constexpr auto GetTagHandler()
    {
        return std::tuple_cat(std::tuple<T> {}, Base::GetTagHandler());
    }

    template <typename Int, const size_t SHIFT>
    static constexpr void SetTags(const typename T::Type &tag, const typename Tag::Type &...tags, Int &val)
    {
        using UInt = std::make_unsigned_t<Int>;
        auto uintVal = static_cast<UInt>(val);
        auto mask = GetMask<Int, SHIFT>();
        uintVal &= ~mask;
        auto tagVal = static_cast<UInt>(tag);
        tagVal <<= TAG_SHIFT + SHIFT;
        tagVal &= mask;
        uintVal |= tagVal;
        val = static_cast<Int>(uintVal);
        Base::template SetTags<Int, SHIFT>(std::forward<const typename Tag::Type>(tags)..., val);
    }
};

template <typename...>
class TaggedIndexHelper0;

template <typename... Tags, typename Int>
class TaggedIndexHelper0<Int, TagPack<Tags...>> {
    using AllTags = TagPack<Tags...>;
    using UInt = std::make_unsigned_t<Int>;
    static constexpr size_t U_INT_BITS = sizeof(UInt) * 8ULL;
    static constexpr size_t ALL_TAG_BITS = AllTags::BITS;
    static constexpr size_t TAG_QUANTITY = AllTags::QUANTITY;
    static constexpr size_t INT_BITS = U_INT_BITS - ALL_TAG_BITS - 1ULL;
    static constexpr UInt ALL_TAG_MASK = ((static_cast<UInt>(1) << ALL_TAG_BITS) - static_cast<UInt>(1)) << INT_BITS;
    static constexpr UInt VALIDITY_BIT = (static_cast<UInt>(1) << (U_INT_BITS - static_cast<size_t>(1)));
    static constexpr UInt ALL_TAG_AND_VALIDITY_MASK = ALL_TAG_MASK & VALIDITY_BIT;
    static constexpr UInt VALUE_MASK = (static_cast<UInt>(1) << INT_BITS) - static_cast<UInt>(1);
    static constexpr size_t VALUE_SIGN_BIT = (static_cast<UInt>(1) << (INT_BITS - static_cast<size_t>(1)));
    static constexpr UInt MAX_VALUE = VALUE_MASK;
    static constexpr UInt INVALID = static_cast<UInt>(0);

    template <size_t TAGNUM>
    static constexpr size_t TagShift()
    {
        return INT_BITS + std::get<TAGNUM>(AllTags::GetTagShift());
    }
    template <size_t TAGNUM>
    static constexpr size_t TagBits()
    {
        return std::get<TAGNUM>(AllTags::GetTagBits());
    }
    template <size_t TAGNUM>
    static constexpr UInt TagMask()
    {
        return std::get<TAGNUM>(AllTags::template GetTagMask<UInt, INT_BITS>());
    }
    template <size_t TAGNUM>
    using TagHandler = std::tuple_element_t<TAGNUM, decltype(AllTags::GetTagHandler())>;

    void SetValid()
    {
        value_ |= VALIDITY_BIT;
    }

public:
    TaggedIndexHelper0() = default;
    explicit TaggedIndexHelper0(typename Tags::Type... tags, Int idx)
    {
        AllTags::template SetTags<UInt, INT_BITS>(std::forward<typename Tags::Type>(tags)..., value_);
        SetValid();
        SetInt(idx);
    }
    void SetInt(Int val)
    {
        ASSERT(IsValid());  // tag should be set before value
        if constexpr (std::is_signed_v<Int>) {
            if (val < 0) {
                ASSERT(static_cast<UInt>(-val) <= MAX_VALUE >> static_cast<UInt>(1));
            } else {
                ASSERT(static_cast<UInt>(val) <= MAX_VALUE >> static_cast<UInt>(1));
            }
        } else {
            ASSERT(static_cast<UInt>(val) <= MAX_VALUE);
        }
        value_ &= ~VALUE_MASK;
        value_ |= (static_cast<UInt>(val) & VALUE_MASK);
    }
    TaggedIndexHelper0 &operator=(Int val)
    {
        SetInt(val);
        return *this;
    }
    template <size_t N, typename Tag>
    void SetTag(Tag tag)
    {
        ASSERT(N < TAG_QUANTITY);
        SetValid();
        value_ &= ~TagMask<N>();
        value_ |= static_cast<UInt>(TagHandler<N>::GetIndexFor(tag)) << TagShift<N>();
    }
    TaggedIndexHelper0(const TaggedIndexHelper0 &) = default;
    TaggedIndexHelper0(TaggedIndexHelper0 &&idx) : value_ {idx.value_}
    {
        idx.Invalidate();
    }
    TaggedIndexHelper0 &operator=(const TaggedIndexHelper0 &) = default;
    TaggedIndexHelper0 &operator=(TaggedIndexHelper0 &&idx)
    {
        value_ = idx.value_;
        idx.Invalidate();
        return *this;
    }
    ~TaggedIndexHelper0() = default;
    void Invalidate()
    {
        value_ = INVALID;
    }
    bool IsValid() const
    {
        return (value_ & VALIDITY_BIT) != 0;
    }
    template <size_t N>
    auto GetTag() const
    {
        ASSERT(IsValid());
        return TagHandler<N>::GetValueFor((value_ & TagMask<N>()) >> TagShift<N>());
    }
    Int GetInt() const
    {
        ASSERT(IsValid());
        UInt val = value_ & VALUE_MASK;
        Int ival;
        if constexpr (std::is_signed_v<Int>) {
            if (val & VALUE_SIGN_BIT) {
                val |= ALL_TAG_AND_VALIDITY_MASK;  // sign-extend
                ival = static_cast<Int>(val);
            } else {
                ival = static_cast<Int>(val);
            }
        } else {
            ival = static_cast<Int>(val);
        }
        return ival;
    }
    Index<Int> GetIndex() const
    {
        if (IsValid()) {
            return GetInt();
        }
        return {};
    }
    // NOLINTNEXTLINE(google-explicit-constructor)
    operator Index<Int>() const
    {
        return GetIndex();
    }
    template <const Int INV>
    Index<Int, INV> GetIndex() const
    {
        ASSERT(static_cast<UInt>(INV) > MAX_VALUE);
        if (IsValid()) {
            return GetInt();
        }
        return {};
    }
    template <const Int INV>
    // NOLINTNEXTLINE(google-explicit-constructor)
    operator Index<Int, INV>() const
    {
        return GetIndex<INV>();
    }
    // NOLINTNEXTLINE(google-explicit-constructor)
    operator Int() const
    {
        ASSERT(IsValid());
        return GetInt();
    }
    bool operator==(const TaggedIndexHelper0 rhs) const
    {
        ASSERT(IsValid());
        ASSERT(rhs.IsValid());
        return value_ == rhs.value_;
    }
    bool operator!=(const TaggedIndexHelper0 rhs) const
    {
        ASSERT(IsValid());
        ASSERT(rhs.IsValid());
        return value_ != rhs.value_;
    }

private:
    UInt value_ {INVALID};
    template <typename T>
    friend struct std::hash;
};

struct First;
struct Second;

template <typename...>
class TaggedIndexSelectorH;

template <typename Int, typename... Tags>
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class TaggedIndexSelectorH<First, Int, std::tuple<Tags...>> : public TaggedIndexHelper0<Int, TagPack<Tags...>> {
    using Base = TaggedIndexHelper0<Int, TagPack<Tags...>>;

public:
    TaggedIndexSelectorH() = default;
    explicit TaggedIndexSelectorH(typename Tags::Type... tags, Int &val)
        : Base {std::forward<typename Tags::Type>(tags)..., val}
    {
    }

    ~TaggedIndexSelectorH() = default;
};

template <typename Int, typename... Tags>
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class TaggedIndexSelectorH<Second, Int, std::tuple<Tags...>>
    : public TaggedIndexHelper0<size_t, TagPack<Tags..., Int>> {
    using Base = TaggedIndexHelper0<Int, TagPack<Tags..., Int>>;

public:
    TaggedIndexSelectorH() = default;
    explicit TaggedIndexSelectorH(typename Tags::Type... tags, size_t &val)
        : Base {std::forward<typename Tags::Type>(tags)..., val}
    {
    }

    ~TaggedIndexSelectorH() = default;
};

template <typename Int, typename... Tags>
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class TaggedIndexSelector : public TaggedIndexSelectorH<std::conditional_t<std::is_integral_v<Int>, First, Second>, Int,
                                                        std::tuple<Tags...>> {
    using Base =
        TaggedIndexSelectorH<std::conditional_t<std::is_integral_v<Int>, First, Second>, Int, std::tuple<Tags...>>;

public:
    TaggedIndexSelector() = default;
    explicit TaggedIndexSelector(typename Tags::Type... tags, Int &val)
        : Base {std::forward<typename Tags::Type>(tags)..., val}
    {
    }

    ~TaggedIndexSelector() = default;
};

template <typename...>
class TaggedIndexHelper2;

template <typename... Tags, typename Int>
class TaggedIndexHelper2<std::tuple<Tags...>, std::tuple<Int>> {
public:
    using TagsInTuple = std::tuple<Tags...>;
    using IntType = Int;
};

template <typename... Ls, typename R, typename... Rs>
class TaggedIndexHelper2<std::tuple<Ls...>, std::tuple<R, Rs...>>
    : public TaggedIndexHelper2<std::tuple<Ls..., R>, std::tuple<Rs...>> {
};

template <typename... TagsAndInt>
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class TaggedIndex
    : public TaggedIndexSelectorH<
          std::conditional_t<
              std::is_integral_v<typename TaggedIndexHelper2<std::tuple<>, std::tuple<TagsAndInt...>>::IntType>, First,
              Second>,
          typename TaggedIndexHelper2<std::tuple<>, std::tuple<TagsAndInt...>>::IntType,
          typename TaggedIndexHelper2<std::tuple<>, std::tuple<TagsAndInt...>>::TagsInTuple> {
    using Base = TaggedIndexSelectorH<
        std::conditional_t<
            std::is_integral_v<typename TaggedIndexHelper2<std::tuple<>, std::tuple<TagsAndInt...>>::IntType>, First,
            Second>,
        typename TaggedIndexHelper2<std::tuple<>, std::tuple<TagsAndInt...>>::IntType,
        typename TaggedIndexHelper2<std::tuple<>, std::tuple<TagsAndInt...>>::TagsInTuple>;

public:
    TaggedIndex() = default;
    template <typename... Args>
    explicit TaggedIndex(Args &&...args) : Base {std::forward<Args>(args)...}
    {
    }

    ~TaggedIndex() = default;
};

}  // namespace ark::verifier

namespace std {

template <typename... TagsAndInt>
struct hash<ark::verifier::TaggedIndex<TagsAndInt...>> {
    size_t operator()(const ark::verifier::TaggedIndex<TagsAndInt...> &i) const noexcept
    {
        return ark::verifier::StdHash(i.value_);
    }
};

}  // namespace std

#endif  // !PANDA_VERIFIER_UTIL_TAGGED_INDEX_HPP_
