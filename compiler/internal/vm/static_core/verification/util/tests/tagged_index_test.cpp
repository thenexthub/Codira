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

#include "libarkbase/utils/utils.h"
#include "util/enum_tag.h"
#include "util/int_tag.h"
#include "util/tagged_index.h"
#include "util/tests/verifier_test.h"

#include <gtest/gtest.h>

#include <unordered_set>

namespace ark::verifier::test {

TEST_F(VerifierTest, Tagged_index)
{
    enum class Tag { TAG0, TAG1, TAG2 };
    using TagType1 = TagForEnum<Tag, Tag::TAG0, Tag::TAG1, Tag::TAG2>;
    // NOLINTNEXTLINE(readability-magic-numbers)
    using TagType2 = TagForInt<int, 5_I, 10_I>;
    TaggedIndex<TagType1, TagType2, size_t> tagind1 {};
    EXPECT_FALSE(tagind1.IsValid());
    tagind1.SetTag<0>(Tag::TAG1);
    tagind1.SetTag<1>(7_I);
    tagind1.SetInt(8_I);
    ASSERT_TRUE(tagind1.IsValid());
    EXPECT_EQ(tagind1.GetTag<0>(), Tag::TAG1);
    EXPECT_EQ(tagind1.GetTag<1>(), 7_I);
    EXPECT_EQ(tagind1, 8_I);
    tagind1.Invalidate();
    EXPECT_FALSE(tagind1.IsValid());

    TaggedIndex<TagType2, TagType1> tagind2 {};
    EXPECT_FALSE(tagind2.IsValid());
    tagind2.SetTag<1>(Tag::TAG1);
    tagind2.SetTag<0>(7_I);
    tagind2.SetInt(8_I);
    ASSERT_TRUE(tagind2.IsValid());
    EXPECT_EQ(tagind2.GetTag<1>(), Tag::TAG1);
    EXPECT_EQ(tagind2.GetTag<0>(), 7_I);
    EXPECT_EQ(tagind2, 8_I);
    tagind2.Invalidate();
    EXPECT_FALSE(tagind2.IsValid());
}

TEST_F(VerifierTest, Tagged_index_in_container)
{
    enum class Tag { TAG0, TAG1, TAG2 };
    using TagType1 = TagForEnum<Tag, Tag::TAG0, Tag::TAG1, Tag::TAG2>;
    // NOLINTNEXTLINE(readability-magic-numbers)
    using TagType2 = TagForInt<int, 5_I, 10_I>;
    using TI = TaggedIndex<TagType1, TagType2, size_t>;
    TI tagind1 {};
    std::unordered_set<TI> iSet {};
    tagind1.SetTag<0>(Tag::TAG1);
    tagind1.SetTag<1>(7_I);
    tagind1.SetInt(8_I);
    iSet.insert(tagind1);
    tagind1.SetTag<0>(Tag::TAG2);
    // NOLINTNEXTLINE(readability-magic-numbers)
    tagind1.SetTag<1>(9_I);
    tagind1.SetInt(3_I);
    iSet.insert(tagind1);
    tagind1.SetTag<0>(Tag::TAG1);
    tagind1.SetTag<1>(7_I);
    tagind1.SetInt(8_I);
    EXPECT_EQ(iSet.count(tagind1), 1);
    tagind1.SetTag<0>(Tag::TAG2);
    // NOLINTNEXTLINE(readability-magic-numbers)
    tagind1.SetTag<1>(9_I);
    tagind1.SetInt(3_I);
    EXPECT_EQ(iSet.count(tagind1), 1);
    tagind1.SetInt(4_I);
    EXPECT_EQ(iSet.count(tagind1), 0);
}

}  // namespace ark::verifier::test
