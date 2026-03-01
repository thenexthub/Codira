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

#include <gtest/gtest.h>
#include "ets_interop_js_gtest.h"

namespace ark::ets::interop::js::testing {

class ESCompatTest : public EtsInteropTest {};

// #22991
TEST_F(ESCompatTest, DISABLED_compat_array)
{
    ASSERT_EQ(true, RunJsTestSuite("compat_array.js"));
}

// NOTE(oignatenko) uncomment Array_TestJSLength code after interop is implemented from JS to eTS
// #22991
TEST_F(ESCompatTest, DISABLED_compat_array_length)
{
    ASSERT_EQ(true, RunJsTestSuite("array_js_suites/test_length.js"));
}

// #26303 TEST_F(ESCompatTest, compat_array_pop) array_js_suites/test_pop.js

// #22991
TEST_F(ESCompatTest, DISABLED_compat_array_fill)
{
    ASSERT_EQ(true, RunJsTestSuite("array_js_suites/test_fill.js"));
}

// #22991
TEST_F(ESCompatTest, DISABLED_compat_array_shift)
{
    ASSERT_EQ(true, RunJsTestSuite("array_js_suites/test_shift.js"));
}

// NOTE(oignatenko) uncomment Array_TestJSSlice code after interop is implemented from JS to eTS
// #22991
TEST_F(ESCompatTest, DISABLED_compat_array_slice)
{
    ASSERT_EQ(true, RunJsTestSuite("array_js_suites/test_slice.js"));
}

// NOTE(oignatenko) uncomment Array_TestJSSplice code after interop is implemented from JS to eTS
// #22991
TEST_F(ESCompatTest, DISABLED_compat_array_splice)
{
    ASSERT_EQ(true, RunJsTestSuite("array_js_suites/test_splice.js"));
}

// NOTE(oignatenko) uncomment test_to_spliced.js code after recent regression making it work in place is fixed
// #22991
TEST_F(ESCompatTest, DISABLED_compat_array_to_spliced)
{
    ASSERT_EQ(true, RunJsTestSuite("array_js_suites/test_to_spliced.js"));
}

// NOTE(oignatenko) uncomment Array_TestJSCopyWithin code after interop is implemented from JS to eTS
// #22991
TEST_F(ESCompatTest, DISABLED_compat_array_copy_within)
{
    ASSERT_EQ(true, RunJsTestSuite("array_js_suites/test_copy_within.js"));
}

// NOTE(oignatenko) uncomment Array_TestJSWith code after interop is implemented from JS to eTS
// #22991
TEST_F(ESCompatTest, DISABLED_compat_array_with)
{
    ASSERT_EQ(true, RunJsTestSuite("array_js_suites/test_with.js"));
}

// #22991
TEST_F(ESCompatTest, DISABLED_compat_array_last_index_of)
{
    ASSERT_EQ(true, RunJsTestSuite("array_js_suites/test_last_index_of.js"));
}

// NOTE(oignatenko) uncomment Array_TestJSToReversed code after interop is implemented from JS to eTS
// #22991
TEST_F(ESCompatTest, DISABLED_compat_array_to_reversed)
{
    ASSERT_EQ(true, RunJsTestSuite("array_js_suites/test_to_reversed.js"));
}

// NOTE(oignatenko) enable this after interop is implemented for this method in either or both dimensions
TEST_F(ESCompatTest, DISABLED_compat_array_sort)
{
    ASSERT_EQ(true, RunJsTestSuite("array_js_suites/test_sort.js"));
}

// NOTE(oignatenko) enable this after interop is implemented for this method in either or both dimensions
TEST_F(ESCompatTest, DISABLED_compat_array_to_sorted)
{
    ASSERT_EQ(true, RunJsTestSuite("array_js_suites/test_to_sorted.js"));
}

// NOTE(oignatenko) enable this after interop is implemented for this method in either or both dimensions
TEST_F(ESCompatTest, DISABLED_compat_array_join)
{
    ASSERT_EQ(true, RunJsTestSuite("array_js_suites/test_join.js"));
}

// NOTE(oignatenko) enable this after interop is implemented for this method in either or both dimensions
TEST_F(ESCompatTest, DISABLED_compat_array_some)
{
    ASSERT_EQ(true, RunJsTestSuite("array_js_suites/test_some.js"));
}

// NOTE(oignatenko) enable this after interop is implemented for this method in either or both dimensions
TEST_F(ESCompatTest, DISABLED_compat_array_every)
{
    ASSERT_EQ(true, RunJsTestSuite("array_js_suites/test_every.js"));
}

// NOTE(oignatenko) enable this after interop is implemented for this method in either or both dimensions
TEST_F(ESCompatTest, DISABLED_compat_array_filter)
{
    ASSERT_EQ(true, RunJsTestSuite("array_js_suites/test_filter.js"));
}

// NOTE(oignatenko) enable this after interop is implemented for this method in either or both dimensions
TEST_F(ESCompatTest, DISABLED_compat_array_filter_array)
{
    ASSERT_EQ(true, RunJsTestSuite("array_js_suites/test_filter_array.js"));
}

// NOTE(oignatenko) enable this after interop is implemented for this method in either or both dimensions
TEST_F(ESCompatTest, DISABLED_compat_array_map)
{
    ASSERT_EQ(true, RunJsTestSuite("array_js_suites/test_map.js"));
}

// NOTE(oignatenko) enable this after interop is implemented for this method in either or both dimensions
TEST_F(ESCompatTest, DISABLED_compat_array_flat_map)
{
    ASSERT_EQ(true, RunJsTestSuite("array_js_suites/test_flat_map.js"));
}

// NOTE(oignatenko) enable this after interop is implemented for this method in either or both dimensions
TEST_F(ESCompatTest, DISABLED_compat_array_reduce)
{
    ASSERT_EQ(true, RunJsTestSuite("array_js_suites/test_reduce.js"));
}

// NOTE(oignatenko) enable this after interop is implemented for this method in either or both dimensions
TEST_F(ESCompatTest, DISABLED_compat_array_reduce_right)
{
    ASSERT_EQ(true, RunJsTestSuite("array_js_suites/test_reduce_right.js"));
}

// NOTE(oignatenko) enable this after interop is implemented for this method in either or both dimensions
TEST_F(ESCompatTest, DISABLED_compat_array_find_last)
{
    ASSERT_EQ(true, RunJsTestSuite("array_js_suites/test_find_last.js"));
}

// NOTE(oignatenko) enable this after interop is implemented for this method in either or both dimensions
TEST_F(ESCompatTest, DISABLED_compat_array_find_index)
{
    ASSERT_EQ(true, RunJsTestSuite("array_js_suites/test_find_index.js"));
}

// NOTE(oignatenko) enable this after interop is implemented for this method in either or both dimensions
TEST_F(ESCompatTest, DISABLED_compat_array_find)
{
    ASSERT_EQ(true, RunJsTestSuite("array_js_suites/test_find.js"));
}

// NOTE(oignatenko) enable this after interop is implemented for this method in either or both dimensions
TEST_F(ESCompatTest, DISABLED_compat_array_is_array)
{
    ASSERT_EQ(true, RunJsTestSuite("array_js_suites/test_is_array.js"));
}

// NOTE(oignatenko) enable this after interop is implemented for this method in either or both dimensions
TEST_F(ESCompatTest, DISABLED_compat_array_from_async)
{
    ASSERT_EQ(true, RunJsTestSuite("array_js_suites/test_from_async.js"));
}

// NOTE(vpukhov): fix boxed primitives casts
TEST_F(ESCompatTest, DISABLED_compat_boolean)
{
    ASSERT_EQ(true, RunJsTestSuite("compat_boolean.js"));
}

// NOTE(vpukhov): compat accessors
TEST_F(ESCompatTest, DISABLED_compat_error)
{
    ASSERT_EQ(true, RunJsTestSuite("compat_error.js"));
}

TEST_F(ESCompatTest, json_stringify)
{
    ASSERT_EQ(true, RunJsTestSuite("compat_stringify.js"));
}

}  // namespace ark::ets::interop::js::testing
