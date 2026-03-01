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

#include "file_items.h"
#include "file_item_container.h"
#include "file_writer.h"

#include <cstdint>

#include <utility>
#include <vector>

#include <gtest/gtest.h>

namespace panda::panda_file::test {

HWTEST(LineNumberProgramItem, EmitSpecialOpcode, testing::ext::TestSize.Level0)
{
    LineNumberProgramItem item(nullptr);

    constexpr int32_t LINE_MAX_INC = LineNumberProgramItem::LINE_RANGE + LineNumberProgramItem::LINE_BASE - 1;
    constexpr int32_t LINE_MIN_INC = LineNumberProgramItem::LINE_BASE;

    EXPECT_FALSE(item.EmitSpecialOpcode(0, LINE_MAX_INC + 1));
    EXPECT_FALSE(item.EmitSpecialOpcode(0, LINE_MIN_INC - 1));
    EXPECT_FALSE(item.EmitSpecialOpcode(100, LINE_MAX_INC));

    std::vector<std::pair<int32_t, uint32_t>> incs = {{1, LINE_MIN_INC}, {2, LINE_MAX_INC}};
    std::vector<uint8_t> data;

    for (auto [pc_inc, line_inc] : incs) {
        ASSERT_TRUE(item.EmitSpecialOpcode(pc_inc, line_inc));
        data.push_back((line_inc - LineNumberProgramItem::LINE_BASE) + (pc_inc * LineNumberProgramItem::LINE_RANGE) +
                       LineNumberProgramItem::OPCODE_BASE);
    }

    MemoryWriter writer;
    ASSERT_TRUE(item.Write(&writer));

    EXPECT_EQ(writer.GetData(), data);
}

HWTEST(LineNumberProgramItem, LanguageFromAndToString, testing::ext::TestSize.Level0)
{
    ASSERT_EQ(LanguageFromString("not ECMAScript"), static_cast<SourceLang>(1U));
    ASSERT_STREQ(LanguageToString(SourceLang::ECMASCRIPT), "ECMAScript");
    ASSERT_STREQ(LanguageToString(SourceLang::PANDA_ASSEMBLY), "PandaAssembly");
}

HWTEST(LineNumberProgramItem, GetStringClassDescriptor, testing::ext::TestSize.Level0)
{
    ASSERT_STREQ(GetStringClassDescriptor(SourceLang::ECMASCRIPT), "Lpanda/JSString;");
    ASSERT_STREQ(GetStringClassDescriptor(SourceLang::PANDA_ASSEMBLY), "Lpanda/String;");
}

HWTEST(LineNumberProgramItem, IsDynamicLanguageTest, testing::ext::TestSize.Level0)
{
    panda::panda_file::SourceLang lang = panda::panda_file::SourceLang::ECMASCRIPT;
    ASSERT_TRUE(IsDynamicLanguage(lang));
}

HWTEST(LineNumberProgramItem, ItemTypeToStringPart1, testing::ext::TestSize.Level0)
{
    for (int i = 0; i <= static_cast<int>(ItemTypes::FIELD_ITEM) - 1; ++i) {
        ItemTypes type = static_cast<ItemTypes>(i);
        std::string expected;
        switch (type) {
            case ItemTypes::ANNOTATION_ITEM:
                expected = "annotation_item";
                break;
            case ItemTypes::CATCH_BLOCK_ITEM:
                expected = "catch_block_item";
                break;
            case ItemTypes::CLASS_INDEX_ITEM:
                expected = "class_index_item";
                break;
            case ItemTypes::CLASS_ITEM:
                expected = "class_item";
                break;
            case ItemTypes::CODE_ITEM:
                expected = "code_item";
                break;
            case ItemTypes::DEBUG_INFO_ITEM:
                expected = "debug_info_item";
                break;
            case ItemTypes::END_ITEM:
                expected = "end_item";
                break;
            case ItemTypes::FIELD_INDEX_ITEM:
                expected = "field_index_item";
                break;
            case ItemTypes::FIELD_ITEM:
                expected = "field_item";
                break;
            default:
                expected = "";
                break;
        }
        EXPECT_EQ(ItemTypeToString(type), expected);
    }
}

HWTEST(LineNumberProgramItem, ItemTypeToStringPart2, testing::ext::TestSize.Level0)
{
    for (int i = static_cast<int>(ItemTypes::FOREIGN_CLASS_ITEM);
         i <= static_cast<int>(ItemTypes::LITERAL_ITEM) - 1; ++i) {
        ItemTypes type = static_cast<ItemTypes>(i);
        std::string expected;
        switch (type) {
            case ItemTypes::FOREIGN_CLASS_ITEM:
                expected = "foreign_class_item";
                break;
            case ItemTypes::FOREIGN_FIELD_ITEM:
                expected = "foreign_field_item";
                break;
            case ItemTypes::FOREIGN_METHOD_ITEM:
                expected = "foreign_method_item";
                break;
            case ItemTypes::INDEX_HEADER:
                expected = "index_header";
                break;
            case ItemTypes::INDEX_SECTION:
                expected = "index_section";
                break;
            case ItemTypes::LINE_NUMBER_PROGRAM_INDEX_ITEM:
                expected = "line_number_program_index_item";
                break;
            case ItemTypes::LINE_NUMBER_PROGRAM_ITEM:
                expected = "line_number_program_item";
                break;
            case ItemTypes::LITERAL_ARRAY_ITEM:
                expected = "literal_array_item";
                break;
            case ItemTypes::LITERAL_ITEM:
                expected = "literal_item";
                break;
            default:
                expected = "";
                break;
        }
        EXPECT_EQ(ItemTypeToString(type), expected);
    }
}

HWTEST(LineNumberProgramItem, ItemTypeToStringPart3, testing::ext::TestSize.Level0)
{
    for (int i = static_cast<int>(ItemTypes::METHOD_HANDLE_ITEM);
         i <= static_cast<int>(ItemTypes::VALUE_ITEM); ++i) {
        ItemTypes type = static_cast<ItemTypes>(i);
        std::string expected;
        switch (type) {
            case ItemTypes::METHOD_HANDLE_ITEM:
                expected = "method_handle_item";
                break;
            case ItemTypes::METHOD_INDEX_ITEM:
                expected = "method_index_item";
                break;
            case ItemTypes::METHOD_ITEM:
                expected = "method_item";
                break;
            case ItemTypes::PARAM_ANNOTATIONS_ITEM:
                expected = "param_annotations_item";
                break;
            case ItemTypes::PRIMITIVE_TYPE_ITEM:
                expected = "primitive_type_item";
                break;
            case ItemTypes::PROTO_INDEX_ITEM:
                expected = "proto_index_item";
                break;
            case ItemTypes::PROTO_ITEM:
                expected = "proto_item";
                break;
            case ItemTypes::STRING_ITEM:
                expected = "string_item";
                break;
            case ItemTypes::TRY_BLOCK_ITEM:
                expected = "try_block_item";
                break;
            case ItemTypes::VALUE_ITEM:
                expected = "value_item";
                break;
            default:
                expected = "";
                break;
        }
        EXPECT_EQ(ItemTypeToString(type), expected);
    }
}

HWTEST(LineNumberProgramItem, GetULeb128EncodedSizeTest, testing::ext::TestSize.Level0)
{
    ItemContainer container;
    const uint32_t kTestIntegerValue = 1;
    const uint64_t kTestLongValue = 1234567890;

    ScalarValueItem *int_item = container.GetOrCreateIntegerValueItem(kTestIntegerValue);
    EXPECT_EQ(int_item, container.GetOrCreateIntegerValueItem(kTestIntegerValue));
    EXPECT_EQ(int_item->GetULeb128EncodedSize(), leb128::UnsignedEncodingSize(kTestIntegerValue));

    ScalarValueItem *long_item = container.GetOrCreateLongValueItem(kTestLongValue);
    EXPECT_EQ(long_item, container.GetOrCreateLongValueItem(kTestLongValue));
    EXPECT_EQ(long_item->GetULeb128EncodedSize(), leb128::UnsignedEncodingSize(kTestLongValue));
}
}  // namespace panda::panda_file::test
