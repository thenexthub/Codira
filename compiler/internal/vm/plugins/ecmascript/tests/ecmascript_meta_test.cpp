/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "plugins/ecmascript/assembler/extension/ecmascript_meta.h"

namespace panda::test {

HWTEST(EcmaScriptMetaTest, ValidateTest, testing::ext::TestSize.Level0)
{
    pandasm::extensions::ecmascript::RecordMetadata rmd;
    pandasm::Metadata::Error err("Attribute 'ecmascript.extends' must have a value",
                                 pandasm::Metadata::Error::Type::MISSING_VALUE);

    std::optional<pandasm::Metadata::Error> result1 = rmd.SetAttribute("ecmascript.extends");
    ASSERT_TRUE(result1.has_value());
    ASSERT_EQ(err.GetMessage(), result1->GetMessage());
    ASSERT_EQ(err.GetType(), result1->GetType());

    std::optional<pandasm::Metadata::Error> result2 = rmd.SetAttribute("ecmascript.annotation");
    ASSERT_FALSE(result2.has_value());

    std::optional<pandasm::Metadata::Error> result3 = rmd.SetAttributeValue("ecmascript.extends", "value");
    ASSERT_FALSE(result3.has_value());

    std::optional<pandasm::Metadata::Error> result4 = rmd.SetAttributeValue("ecmascript.annotation", "value");
    ASSERT_TRUE(result4.has_value());
    ASSERT_EQ(result4->GetMessage(), "Attribute 'ecmascript.annotation' must not have a value");
    ASSERT_EQ(result4->GetType(), pandasm::Metadata::Error::Type::UNEXPECTED_VALUE);

    std::optional<pandasm::Metadata::Error> result5 = rmd.SetAttributeValue("attribute", "bool");
    ASSERT_TRUE(result5.has_value());
    ASSERT_EQ(result5->GetMessage(), "Unknown attribute 'attribute'");

    std::optional<pandasm::Metadata::Error> result6 = rmd.SetAttribute("ecmascript.annotation");
    ASSERT_TRUE(result6.has_value());
    ASSERT_EQ(result6->GetMessage(), "Attribute 'ecmascript.annotation' already defined");
}

HWTEST(EcmaScriptMetaTest, SetAndRemoveFlagsTest, testing::ext::TestSize.Level0)
{
    pandasm::extensions::ecmascript::RecordMetadata rmd;
    rmd.SetAttribute("attribute");
    rmd.SetAttribute("external");
    rmd.SetAttribute("ecmascript.annotation");
    ASSERT_FALSE(rmd.GetAttribute("attribute"));
    ASSERT_TRUE(rmd.GetAttribute("external"));
    ASSERT_TRUE(rmd.GetAttribute("ecmascript.annotation"));

    rmd.RemoveAttribute("attribute");
    ASSERT_FALSE(rmd.GetAttribute("attribute"));
    rmd.RemoveAttribute("external");
    ASSERT_FALSE(rmd.GetAttribute("external"));
    rmd.RemoveAttribute("ecmascript.annotation");
    ASSERT_FALSE(rmd.GetAttribute("ecmascript.annotation"));
}

}  // namespace panda::test
