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

#include "unit_test.h"
#include "compiler/optimizer/analysis/typed_ref_set.h"

namespace ark::compiler {
class RefSetTest : public GraphTest {};

// NOLINTBEGIN(readability-magic-numbers)
TEST_F(RefSetTest, SmallSetBasicOps)
{
    auto a = SmallSet {1, 2U, 3U};
    ASSERT_TRUE(a.IsVectorSet());
    ASSERT_EQ(3U, a.PopCount());
    a.SetBit(65U);
    ASSERT_EQ(4U, a.PopCount());
    a.SetBit(65U);
    ASSERT_EQ(4U, a.PopCount());
    a.SetBit(129U);
    ASSERT_EQ(5U, a.PopCount());
    ASSERT_TRUE(a.IsVectorSet());
    a.SetBit(65U);
    a.SetBit(2U);
    ASSERT_EQ(5U, a.PopCount());
    ASSERT_TRUE(a.IsVectorSet());
    ASSERT_TRUE(a.GetBit(65U));
    ASSERT_FALSE(a.GetBit(64U));
    ASSERT_FALSE(a.GetBit(66U));

    ASSERT_FALSE(a.GetBit(6U));
    a.SetBit(6U);
    ASSERT_TRUE(a.GetBit(6U));
    ASSERT_EQ(6U, a.PopCount());
    ASSERT_FALSE(a.IsVectorSet());
    a.SetBit(6U);
    ASSERT_EQ(6U, a.PopCount());
    ASSERT_FALSE(a.IsVectorSet());
    a.SetBit(128U);
    a.SetBit(127U);
    ASSERT_EQ(8U, a.PopCount());
}

TEST_F(RefSetTest, SmallSetIncludes1U)
{
    auto a = SmallSet {1U, 65U, 257U};
    auto b = a;
    ASSERT_TRUE(a.Includes(a));
    ASSERT_TRUE(a.Includes(b));

    b.SetBit(2U);
    ASSERT_TRUE(b.Includes(a));
    ASSERT_FALSE(a.Includes(b));

    b.SetBit(256U);
    b.SetBit(258U);
    ASSERT_TRUE(a.IsVectorSet());
    ASSERT_FALSE(b.IsVectorSet());
    ASSERT_TRUE(b.Includes(a));
    ASSERT_FALSE(a.Includes(b));
}

TEST_F(RefSetTest, SmallSetIncludes2U)
{
    auto a = SmallSet {1U};
    auto b = SmallSet {};
    ASSERT_TRUE(a.Includes(b));
    ASSERT_FALSE(b.Includes(a));
    b.SetBit(1U);
    ASSERT_TRUE(a.Includes(b));
    ASSERT_TRUE(b.Includes(a));
    a.SetBit(40U);
    a.SetBit(40U);
    b.SetBit(40U);
    ASSERT_TRUE(a.Includes(b));
    ASSERT_TRUE(b.Includes(a));

    ASSERT_TRUE(a.IsVectorSet());
    ASSERT_TRUE(b.IsVectorSet());
}

TEST_F(RefSetTest, LargeSetIncludes1U)
{
    auto a = SmallSet {1U, 2U, 3U, 4U, 5U, 6U, 100U};
    auto b = SmallSet {1U, 2U, 3U, 4U, 5U, 6U};
    ASSERT_FALSE(a.IsVectorSet());
    ASSERT_FALSE(b.IsVectorSet());

    ASSERT_TRUE(a.Includes(b));
    ASSERT_FALSE(b.Includes(a));

    b.SetBit(99U);
    ASSERT_FALSE(a.Includes(b));
    ASSERT_FALSE(b.Includes(a));

    b.SetBit(100U);
    ASSERT_FALSE(a.Includes(b));
    ASSERT_TRUE(b.Includes(a));
}

TEST_F(RefSetTest, LargeSetIncludes2U)
{
    auto a = SmallSet {1U, 2U, 3U, 6U, 9U, 10U, 101U, 102U, 103U, 107U};
    auto b = SmallSet {101U, 102U, 103U, 1U, 2U, 3U};
    ASSERT_FALSE(a.IsVectorSet());
    ASSERT_FALSE(b.IsVectorSet());

    ASSERT_TRUE(a.Includes(a));
    ASSERT_TRUE(b.Includes(b));
    ASSERT_TRUE(a.Includes(b));
    ASSERT_FALSE(b.Includes(a));

    b.SetBit(200U);
    ASSERT_FALSE(a.Includes(b));
    ASSERT_FALSE(b.Includes(a));
}

TEST_F(RefSetTest, SmallSetOr)
{
    auto a = SmallSet {1U, 3U, 9U};
    a |= SmallSet {1U, 3U, 9U};
    ASSERT_EQ(a, (SmallSet {1U, 3U, 9U}));

    a |= SmallSet {6U, 100U};
    ASSERT_EQ(a, (SmallSet {1U, 3U, 6U, 9U, 100U}));
    ASSERT_TRUE(a.IsVectorSet());

    a |= SmallSet {1U, 9U};
    ASSERT_EQ(a, (SmallSet {1U, 3U, 6U, 9U, 100U}));
    ASSERT_TRUE(a.IsVectorSet());

    a |= SmallSet {65U};
    ASSERT_EQ(a, (SmallSet {1U, 3U, 6U, 9U, 65U, 100U}));
    ASSERT_FALSE(a.IsVectorSet());
    ASSERT_TRUE(a.PopCount() == 6U);

    a = SmallSet {};
    a |= SmallSet {1U, 9U};
    ASSERT_EQ(a, (SmallSet {1U, 9U}));
    ASSERT_TRUE(a.IsVectorSet());
}

TEST_F(RefSetTest, LargeSetOr)
{
    auto a = SmallSet {1U, 3U, 9U, 65U, 67U, 68U};
    ASSERT_FALSE(a.IsVectorSet());

    a |= SmallSet {1U, 101U};
    ASSERT_EQ(a, (SmallSet {1U, 3U, 9U, 65U, 67U, 68U, 101U}));

    a |= SmallSet {1U, 9U, 10U, 65U, 99U, 100U};
    ASSERT_EQ(a, (SmallSet {1U, 3U, 9U, 10U, 65U, 67U, 68U, 99U, 100U, 101U}));
    ASSERT_FALSE(a.IsVectorSet());
    ASSERT_TRUE(a.PopCount() == 10U);
    ASSERT_TRUE(a.GetBit(10U));
}
// NOLINTEND(readability-magic-numbers)

}  //  namespace ark::compiler
