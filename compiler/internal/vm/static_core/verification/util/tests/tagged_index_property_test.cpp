/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "util/enum_tag.h"
#include "util/int_tag.h"
#include "util/tagged_index.h"

#include <gtest/gtest.h>
#include <rapidcheck/gtest.h>

namespace {

enum class TAGS2 { TAG0, TAG1, TAG2 };
enum class TAGS3 { TAG0, TAG1, TAG2, TAG3, TAG4, TAG5, TAG6 };

struct Int4 {
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    size_t value;
    // NOLINTNEXTLINE(google-explicit-constructor)
    operator size_t() const
    {
        return value;
    }
};

struct IntTag {
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    size_t value;
    // NOLINTNEXTLINE(google-explicit-constructor)
    operator size_t() const
    {
        return value;
    }
};

}  // namespace

namespace rc {

template <>
struct Arbitrary<TAGS2> {
    static Gen<TAGS2> arbitrary()  // NOLINT(readability-identifier-naming)
    {
        return gen::map<int>(gen::inRange(0, 0x3), [](int x) {
            switch (x) {
                case 0:
                    return TAGS2::TAG0;
                    break;

                case 1:
                    return TAGS2::TAG1;
                    break;

                case 0x2:
                    return TAGS2::TAG2;
                    break;

                default:
                    UNREACHABLE();
            }
        });
    }
};

template <>
struct Arbitrary<TAGS3> {
    static Gen<TAGS3> arbitrary()  // NOLINT(readability-identifier-naming)
    {
        return gen::map<int>(gen::inRange(0, 0x7), [](int x) {
            switch (x) {
                case 0:
                    return TAGS3::TAG0;
                    break;

                case 1:
                    return TAGS3::TAG1;
                    break;

                case 0x2:
                    return TAGS3::TAG2;
                    break;

                case 0x3:
                    return TAGS3::TAG3;
                    break;

                case 0x4:
                    return TAGS3::TAG4;
                    break;

                case 0x5:
                    return TAGS3::TAG5;
                    break;

                case 0x6:
                    return TAGS3::TAG6;
                    break;

                default:
                    UNREACHABLE();
            }
        });
    }
};

template <>
struct Arbitrary<Int4> {
    static Gen<Int4> arbitrary()  // NOLINT(readability-identifier-naming)
    {
        // NOLINTNEXTLINE(readability-magic-numbers)
        return gen::map<size_t>(gen::inRange(static_cast<size_t>(0), (std::numeric_limits<size_t>::max() >> 0x9U) + 1),
                                [](size_t val) {
                                    Int4 i {};
                                    i.value = val;
                                    return i;
                                });
    }
};

template <>
struct Arbitrary<IntTag> {
    static Gen<IntTag> arbitrary()  // NOLINT(readability-identifier-naming)
    {
        // NOLINTNEXTLINE(readability-magic-numbers)
        return gen::map<size_t>(gen::inRange(static_cast<size_t>(0x5), static_cast<size_t>(0xB)), [](size_t val) {
            IntTag i {};
            i.value = val;
            return i;
        });
    }
};

}  // namespace rc

namespace ark::verifier::test {

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace rc;

// Three tagtypes
RC_GTEST_PROP(TestCase, ThreeTagtypes, (TAGS2 tag2, TAGS3 tag3, IntTag value_tag, Int4 value))
{
    using TagType1 = TagForEnum<TAGS2, TAGS2::TAG0, TAGS2::TAG1, TAGS2::TAG2>;
    using TagType2 =
        TagForEnum<TAGS3, TAGS3::TAG0, TAGS3::TAG1, TAGS3::TAG2, TAGS3::TAG3, TAGS3::TAG4, TAGS3::TAG5, TAGS3::TAG6>;
    // NOLINTNEXTLINE(readability-magic-numbers)
    using TagType3 = TagForInt<size_t, 5, 10>;
    using TI = TaggedIndex<TagType1, TagType2, TagType3, size_t>;
    TI ind;
    RC_ASSERT_FALSE(ind.IsValid());
    ind.SetTag<0>(tag2);
    RC_ASSERT(ind.IsValid());
    ind.SetTag<1>(tag3);
    RC_ASSERT(ind.IsValid());
    ind.SetTag<2>(value_tag);
    RC_ASSERT(ind.IsValid());
    ind.SetInt(value);
    RC_ASSERT(ind.IsValid());
    RC_ASSERT(ind.GetTag<0>() == tag2);
    RC_ASSERT(ind.GetTag<1>() == tag3);
    RC_ASSERT(ind.GetTag<2>() == value_tag);
    RC_ASSERT(ind.GetInt() == value);
    RC_ASSERT(static_cast<size_t>(ind.GetIndex()) == value);
    ind.Invalidate();
    RC_ASSERT_FALSE(ind.IsValid());
    RC_ASSERT_FALSE(ind.GetIndex().IsValid());
}

}  // namespace ark::verifier::test