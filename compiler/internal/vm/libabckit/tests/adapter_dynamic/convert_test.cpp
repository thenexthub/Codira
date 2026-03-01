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

#include <gtest/gtest.h>
#include "adapter_dynamic/convert.h"
#include "libpandabase/macros.h"
#include "include/source_lang_enum.h"
#include "libabckit/c/metadata_core.h"

namespace libabckit::cpp_test {

class ConvertTest : public ::testing::Test {};

TEST(ConvertDynamicTest, ToStringTest)
{
    EXPECT_EQ(convert::ToString(ABCKIT_TARGET_UNKNOWN), "unknown");
    EXPECT_EQ(convert::ToString(ABCKIT_TARGET_ARK_TS_V1), "ArkTsV1");
    EXPECT_EQ(convert::ToString(ABCKIT_TARGET_ARK_TS_V2), "ArkTsV2");
    EXPECT_EQ(convert::ToString(ABCKIT_TARGET_TS), "TypeScript");
    EXPECT_EQ(convert::ToString(ABCKIT_TARGET_JS), "JavaScript");
    EXPECT_EQ(convert::ToString(ABCKIT_TARGET_NATIVE), "native");
}

TEST(ConvertDynamicTest, ToSourceLangTest)
{
    using SourceLang = panda::panda_file::SourceLang;
    EXPECT_EQ(convert::ToSourceLang(ABCKIT_TARGET_ARK_TS_V1), SourceLang::ARKTS);
    EXPECT_EQ(convert::ToSourceLang(ABCKIT_TARGET_ARK_TS_V2), SourceLang::ARKTS);
    EXPECT_EQ(convert::ToSourceLang(ABCKIT_TARGET_TS), SourceLang::TYPESCRIPT);
    EXPECT_EQ(convert::ToSourceLang(ABCKIT_TARGET_JS), SourceLang::JAVASCRIPT);
}
}  // namespace libabckit::cpp_test