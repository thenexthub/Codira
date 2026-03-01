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

#include "libarkbase/utils/bit_helpers.h"

#include <cstdint>
#include <type_traits>

#include <gtest/gtest.h>

namespace ark::helpers::test {

TEST(BitHelpers, UnsignedTypeHelper)
{
    EXPECT_TRUE((std::is_same_v<UnsignedTypeHelperT<0U>, uint8_t>));
    EXPECT_TRUE((std::is_same_v<UnsignedTypeHelperT<1U>, uint8_t>));
    EXPECT_TRUE((std::is_same_v<UnsignedTypeHelperT<8U>, uint8_t>));

    EXPECT_TRUE((std::is_same_v<UnsignedTypeHelperT<9U>, uint16_t>));
    EXPECT_TRUE((std::is_same_v<UnsignedTypeHelperT<16U>, uint16_t>));

    EXPECT_TRUE((std::is_same_v<UnsignedTypeHelperT<17U>, uint32_t>));
    EXPECT_TRUE((std::is_same_v<UnsignedTypeHelperT<32U>, uint32_t>));

    EXPECT_TRUE((std::is_same_v<UnsignedTypeHelperT<33U>, uint64_t>));
    EXPECT_TRUE((std::is_same_v<UnsignedTypeHelperT<64U>, uint64_t>));

    EXPECT_TRUE((std::is_same_v<UnsignedTypeHelperT<65U>, void>));
}

TEST(BitHelpers, TypeHelper)
{
    EXPECT_TRUE((std::is_same_v<TypeHelperT<0U, false>, uint8_t>));
    EXPECT_TRUE((std::is_same_v<TypeHelperT<1U, false>, uint8_t>));
    EXPECT_TRUE((std::is_same_v<TypeHelperT<8U, false>, uint8_t>));

    EXPECT_TRUE((std::is_same_v<TypeHelperT<0U, true>, int8_t>));
    EXPECT_TRUE((std::is_same_v<TypeHelperT<1U, true>, int8_t>));
    EXPECT_TRUE((std::is_same_v<TypeHelperT<8U, true>, int8_t>));

    EXPECT_TRUE((std::is_same_v<TypeHelperT<9U, false>, uint16_t>));
    EXPECT_TRUE((std::is_same_v<TypeHelperT<16U, false>, uint16_t>));

    EXPECT_TRUE((std::is_same_v<TypeHelperT<9U, true>, int16_t>));
    EXPECT_TRUE((std::is_same_v<TypeHelperT<16U, true>, int16_t>));

    EXPECT_TRUE((std::is_same_v<TypeHelperT<17U, false>, uint32_t>));
    EXPECT_TRUE((std::is_same_v<TypeHelperT<32U, false>, uint32_t>));

    EXPECT_TRUE((std::is_same_v<TypeHelperT<17U, true>, int32_t>));
    EXPECT_TRUE((std::is_same_v<TypeHelperT<32U, true>, int32_t>));

    EXPECT_TRUE((std::is_same_v<TypeHelperT<33U, false>, uint64_t>));
    EXPECT_TRUE((std::is_same_v<TypeHelperT<64U, false>, uint64_t>));

    EXPECT_TRUE((std::is_same_v<TypeHelperT<33U, true>, int64_t>));
    EXPECT_TRUE((std::is_same_v<TypeHelperT<64U, true>, int64_t>));
}

}  // namespace ark::helpers::test
