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

#include "file_item_container.h"
#include "pgo.h"

namespace panda::panda_file::test {

static void CreateItems(ItemContainer &container)
{
    // Add classes
    ClassItem *empty_class_item = container.GetOrCreateClassItem("LTest;");
    ClassItem *class_item_a = container.GetOrCreateClassItem("LAA;");
    class_item_a->SetSuperClass(empty_class_item);

    // Add method1
    StringItem *method_name_1 = container.GetOrCreateStringItem("foo1");
    PrimitiveTypeItem *ret_type_1 = container.GetOrCreatePrimitiveTypeItem(Type::TypeId::VOID);
    std::vector<MethodParamItem> params_1;
    ProtoItem *proto_item_1 = container.GetOrCreateProtoItem(ret_type_1, params_1);
    MethodItem *method_item_1 = class_item_a->AddMethod(method_name_1, proto_item_1, ACC_PUBLIC | ACC_STATIC, params_1);

    // Set code_1
    std::vector<uint8_t> instructions_1 {1, 2, 3, 4};
    CodeItem *code_item_1 = container.CreateItem<CodeItem>(0, 2, instructions_1);
    method_item_1->SetCode(code_item_1);
    code_item_1->AddMethod(method_item_1);
}

static void CreateProfile(std::string &filePath)
{
    filePath = "TestParseProfileData_test_data.txt";
    std::ofstream test_file;
    test_file.open(filePath);
    test_file << "" << std::endl;
    test_file << "string_item:test_field" << std::endl;
    test_file << "class_item:AA" << std::endl;
    test_file << "XXXXXXX" << std::endl;
    test_file << "code_item:AA::foo1" << std::endl;
    test_file << "code_item:Test::foo4" << std::endl;
    test_file.close();
}

HWTEST(Pgo, MarkProfileItem, testing::ext::TestSize.Level0)
{
    ItemContainer container;
    CreateItems(container);
    panda::panda_file::pgo::ProfileOptimizer profile_opt;
    for (auto &item : container.GetItems()) {
        if (item->GetName() == CLASS_ITEM) {
            profile_opt.MarkProfileItem(item, true);
            EXPECT_EQ(item->GetPGORank(), panda::panda_file::PGO_CLASS_DEFAULT_COUNT + 1U);  // 1 means set pgo
            profile_opt.MarkProfileItem(item, false);
            EXPECT_EQ(item->GetPGORank(), panda::panda_file::PGO_CLASS_DEFAULT_COUNT);
        } else if (item->GetName() == STRING_ITEM) {
            profile_opt.MarkProfileItem(item, true);
            EXPECT_EQ(item->GetPGORank(), panda::panda_file::PGO_STRING_DEFAULT_COUNT + 1U);  // 1 means set pgo
            profile_opt.MarkProfileItem(item, false);
            EXPECT_EQ(item->GetPGORank(), panda::panda_file::PGO_STRING_DEFAULT_COUNT);
        } else if (item->GetName() == CODE_ITEM) {
            profile_opt.MarkProfileItem(item, true);
            EXPECT_EQ(item->GetPGORank(), panda::panda_file::PGO_CODE_DEFAULT_COUNT + 1U);  // 1 means set pgo
            profile_opt.MarkProfileItem(item, false);
            EXPECT_EQ(item->GetPGORank(), panda::panda_file::PGO_CODE_DEFAULT_COUNT);
        }
    }
}

HWTEST(Pgo, ParseProfileData0, testing::ext::TestSize.Level0)
{
    panda::panda_file::pgo::ProfileOptimizer profile_opt;
    EXPECT_FALSE(profile_opt.ParseProfileData());
}

HWTEST(Pgo, ParseProfileData1, testing::ext::TestSize.Level0)
{
    std::string profile_path;
    CreateProfile(profile_path);

    panda::panda_file::pgo::ProfileOptimizer profile_opt;
    profile_opt.SetProfilePath(profile_path);
    EXPECT_TRUE(profile_opt.ParseProfileData());
}

HWTEST(Pgo, ProfileGuidedRelayout, testing::ext::TestSize.Level0)
{
    ItemContainer container;
    CreateItems(container);

    std::string profile_path;
    CreateProfile(profile_path);

    panda::panda_file::pgo::ProfileOptimizer profile_opt;
    for (auto &item : container.GetItems()) {
        item->SetNeedsEmit(false);
    }
    profile_opt.ProfileGuidedRelayout(container.GetItems());
    for (auto &item : container.GetItems()) {
        item->SetNeedsEmit(true);
    }
    profile_opt.ProfileGuidedRelayout(container.GetItems());
    for (auto &item : container.GetItems()) {
        if (item->GetName() == CLASS_ITEM) {
            EXPECT_EQ(item->GetPGORank(), panda::panda_file::PGO_CLASS_DEFAULT_COUNT);
        }
    }
}

}  // namespace panda::panda_file::test
