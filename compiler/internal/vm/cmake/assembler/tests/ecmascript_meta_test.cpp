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

#include "extensions/ecmascript_meta.h"
#include "meta.h"

using namespace testing::ext;

namespace panda::pandasm {
class EcmascriptMetaTest : public testing::Test {
};

/**
 * @tc.name: ecmascript_meta_test_001
 * @tc.desc: Verify the RecordMetadata SetAttributeValue function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(EcmascriptMetaTest, ecmascript_meta_test_001, TestSize.Level1)
{
    pandasm::extensions::ecmascript::RecordMetadata rmd;
    pandasm::Metadata::Error err("Attribute 'ecmascript.extends' must have a value",
                                 pandasm::Metadata::Error::Type::MISSING_VALUE);

    std::optional<pandasm::Metadata::Error> result1 = rmd.SetAttribute("ecmascript.extends");
    EXPECT_TRUE(result1.has_value());
    EXPECT_EQ(err.GetMessage(), result1->GetMessage());
    EXPECT_EQ(err.GetType(), result1->GetType());

    std::optional<pandasm::Metadata::Error> result2 = rmd.SetAttribute("ecmascript.annotation");
    EXPECT_FALSE(result2.has_value());

    std::optional<pandasm::Metadata::Error> result3 = rmd.SetAttributeValue("ecmascript.extends", "value");
    EXPECT_FALSE(result3.has_value());

    std::optional<pandasm::Metadata::Error> result4 = rmd.SetAttributeValue("ecmascript.annotation", "value");
    EXPECT_TRUE(result4.has_value());
    EXPECT_EQ(result4->GetMessage(), "Attribute 'ecmascript.annotation' must not have a value");
    EXPECT_EQ(result4->GetType(), pandasm::Metadata::Error::Type::UNEXPECTED_VALUE);

    std::optional<pandasm::Metadata::Error> result5 = rmd.SetAttributeValue("attribute", "bool");
    EXPECT_TRUE(result5.has_value());
    EXPECT_EQ(result5->GetMessage(), "Unknown attribute 'attribute'");

    std::optional<pandasm::Metadata::Error> result6 = rmd.SetAttribute("ecmascript.annotation");
    EXPECT_TRUE(result6.has_value());
    EXPECT_EQ(result6->GetMessage(), "Attribute 'ecmascript.annotation' already defined");

    std::optional<pandasm::Metadata::Error> result7 = rmd.ValidateData();
    EXPECT_FALSE(result7.has_value());
}

/**
 * @tc.name: ecmascript_meta_test_002
 * @tc.desc: Verify the RecordMetadata GetAttributeValue function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(EcmascriptMetaTest, ecmascript_meta_test_002, TestSize.Level1)
{
    pandasm::extensions::ecmascript::RecordMetadata rmd;
    rmd.SetAttribute("attribute");
    rmd.SetAttribute("external");
    rmd.SetAttribute("ecmascript.annotation");
    EXPECT_FALSE(rmd.GetAttribute("attribute"));
    EXPECT_TRUE(rmd.GetAttribute("external"));
    EXPECT_TRUE(rmd.GetAttribute("ecmascript.annotation"));

    rmd.RemoveAttribute("attribute");
    EXPECT_FALSE(rmd.GetAttribute("attribute"));
    rmd.RemoveAttribute("external");
    EXPECT_FALSE(rmd.GetAttribute("external"));
    rmd.RemoveAttribute("ecmascript.annotation");
    EXPECT_FALSE(rmd.GetAttribute("ecmascript.annotation"));

    EXPECT_EQ(rmd.GetBase(), "");
    EXPECT_EQ(rmd.GetInterfaces().size(), 0);
    EXPECT_FALSE(rmd.IsAnnotation());
    EXPECT_FALSE(rmd.IsRuntimeAnnotation());
    rmd.SetAttributeValue("ecmascript.extends", "value");
    EXPECT_EQ(rmd.GetBase(), "value");

    rmd.SetAttribute("ecmascript.annotation");
    rmd.RemoveAttribute("ecmascript.annotation");
    EXPECT_FALSE(rmd.GetAttribute("ecmascript.annotation"));
    auto ret = rmd.GetAttributeValue("attribute");
    EXPECT_FALSE(ret.has_value());
}

/**
 * @tc.name: ecmascript_meta_test_003
 * @tc.desc: Verify the FieldMetadata ValidateData function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(EcmascriptMetaTest, ecmascript_meta_test_003, TestSize.Level1)
{
    pandasm::extensions::ecmascript::FieldMetadata rmd;
    pandasm::Metadata::Error err("Attribute 'ecmascript.extends' must have a value",
                                 pandasm::Metadata::Error::Type::MISSING_VALUE);

    std::optional<pandasm::Metadata::Error> result1 = rmd.SetAttribute("ecmascript.annotation");
    EXPECT_TRUE(result1.has_value());

    std::optional<pandasm::Metadata::Error> result2 = rmd.SetAttributeValue("ecmascript.extends", "value");
    EXPECT_TRUE(result2.has_value());

    std::optional<pandasm::Metadata::Error> result3 = rmd.SetAttributeValue("ecmascript.annotation", "value");
    EXPECT_TRUE(result3.has_value());
    EXPECT_EQ(result3->GetMessage(), "Unknown attribute 'ecmascript.annotation'");
    EXPECT_EQ(result3->GetType(), pandasm::Metadata::Error::Type::UNKNOWN_ATTRIBUTE);

    std::optional<pandasm::Metadata::Error> result4 = rmd.SetAttributeValue("attribute", "bool");
    EXPECT_TRUE(result4.has_value());
    EXPECT_EQ(result4->GetMessage(), "Unknown attribute 'attribute'");

    std::optional<pandasm::Metadata::Error> result5 = rmd.SetAttribute("ecmascript.annotation");
    EXPECT_TRUE(result5.has_value());
    EXPECT_EQ(result5->GetMessage(), "Unknown attribute 'ecmascript.annotation'");

    std::optional<pandasm::Metadata::Error> result6 = rmd.ValidateData();
    EXPECT_FALSE(result6.has_value());
}

/**
 * @tc.name: ecmascript_meta_test_004
 * @tc.desc: Verify the GetMessage function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(EcmascriptMetaTest, ecmascript_meta_test_004, TestSize.Level1)
{
    pandasm::Metadata::Error err("test message", pandasm::Metadata::Error::Type::MISSING_ATTRIBUTE);
    EXPECT_EQ(err.GetMessage(), "test message");
    EXPECT_EQ(err.GetType(), pandasm::Metadata::Error::Type::MISSING_ATTRIBUTE);
}
} // namespace panda::pandasm