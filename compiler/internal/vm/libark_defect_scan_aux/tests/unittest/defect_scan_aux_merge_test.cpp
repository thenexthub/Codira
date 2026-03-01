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

#include <array>
#include <iostream>

#include <gtest/gtest.h>
#include "defect_scan_aux_api.h"

namespace panda::defect_scan_aux::test {

class DefectScanAuxMergeTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};
};

HWTEST(DefectScanAuxMergeTest, AcrossAbcTest_0, testing::ext::TestSize.Level0)
{
    std::string name = DEFECT_SCAN_AUX_TEST_MERGE_ABC_DIR "across_abc_test.abc";
    auto abc_file = panda::defect_scan_aux::AbcFile::Open(name);
    ASSERT(abc_file != nullptr);

    auto file_record_size = abc_file->GetFileRecordCount();
    EXPECT_EQ(file_record_size, 3U);
    auto file_record_list = abc_file->GetFileRecordList();
    auto f1_record_name = file_record_list.begin();
    EXPECT_EQ(*f1_record_name, "Lacross_abc_test;");
    auto f2_record_name = std::next(f1_record_name, 1);
    EXPECT_EQ(*f2_record_name, "Ldatabase;");
    auto f3_record_name = std::next(f2_record_name, 1);
    EXPECT_EQ(*f3_record_name, "Luser_input;");

    auto all_class_size = abc_file->GetDefinedClassCount();
    EXPECT_EQ(all_class_size, 3U);
    auto all_func_size = abc_file->GetDefinedFunctionCount();
    EXPECT_EQ(all_func_size, 18U);

    auto class_userinput = abc_file->GetClassByName("Luser_input;#~@0=#UserInput");
    ASSERT_NE(class_userinput, nullptr);
    auto class_userinput_member_func_size = class_userinput->GetMemberFunctionCount();
    EXPECT_EQ(class_userinput_member_func_size, 3U);
    [[maybe_unused]] auto func_setText = class_userinput->GetMemberFunctionByName("Luser_input;#~@0>#setText");
    ASSERT_NE(func_setText, nullptr);
    EXPECT_EQ(func_setText->GetClass(), class_userinput);

    auto userinput_func_main_0 = abc_file->GetFunctionByName("Luser_input;func_main_0");
    auto &graph = userinput_func_main_0->GetGraph();
    auto bb_list = graph.GetBasicBlockList();
    EXPECT_EQ(bb_list.size(), 3U);

    std::string inter_name0 = abc_file->GetLocalNameByExportName("DBInterface", "Ldatabase;");
    EXPECT_EQ(inter_name0, "DatabaseInterface");
    auto ex_class = abc_file->GetExportClassByExportName("DBInterface", "Ldatabase;");
    EXPECT_EQ(ex_class->GetClassName(), "Ldatabase;#~@1=#DatabaseInterface");

    std::string inter_name1 = abc_file->GetLocalNameByExportName("getblankInstanceInterface", "Ldatabase;");
    EXPECT_EQ(inter_name1, "getblankInstance1");
    auto ex_func1 = abc_file->GetExportFunctionByExportName("getblankInstanceInterface", "Ldatabase;");
    EXPECT_EQ(ex_func1->GetFunctionName(), "Ldatabase;#*#getblankInstance1");

    auto ex_func2 = abc_file->GetExportFunctionByExportName("default", "Ldatabase;");
    EXPECT_EQ(ex_func2->GetFunctionName(), "Ldatabase;#*#getblankInstance2");
    EXPECT_NE(abc_file->GetDefinedFunctionByIndex(0), nullptr);
    EXPECT_NE(abc_file->GetDefinedClassByIndex(0), nullptr);
}

HWTEST(DefectScanAuxMergeTest, AcrossAbcTest_1, testing::ext::TestSize.Level0)
{
    std::string name = DEFECT_SCAN_AUX_TEST_MERGE_ABC_DIR "across_abc_test.abc";
    auto abc_file = panda::defect_scan_aux::AbcFile::Open(name);
    ASSERT(abc_file != nullptr);

    auto child_class = abc_file->GetClassByName("Ldatabase;#~@1=#DatabaseInterface");
    ASSERT_NE(child_class, nullptr);
    EXPECT_EQ(child_class->GetParentClassName(), "Ldatabase;#~@0=#Database");
    EXPECT_EQ(child_class->GetMemberFunctionByName("Ldatabase;#~@1>#getText"),
              child_class->GetMemberFunctionByIndex(1));
    auto database_func_main_0 = abc_file->GetFunctionByName("Ldatabase;func_main_0");
    ASSERT_NE(database_func_main_0, nullptr);
    auto callee_info = database_func_main_0->GetCalleeInfoByIndex(0);
    ASSERT_NE(callee_info, nullptr);
    EXPECT_TRUE(callee_info->IsCalleeDefinite());
    EXPECT_EQ(callee_info->GetClass(), abc_file->GetClassByName("Ldatabase;#~@0=#Database"));
    EXPECT_EQ(callee_info->GetCallee(), abc_file->GetFunctionByName("Ldatabase;#~@0>#addData"));
}

HWTEST(DefectScanAuxMergeTest, SendableClassMergeAbcTest, testing::ext::TestSize.Level0)
{
    std::string name = DEFECT_SCAN_AUX_TEST_MERGE_ABC_DIR "sendable_class_merge_abc_test.abc";
    auto abc_file = panda::defect_scan_aux::AbcFile::Open(name);
    ASSERT(abc_file != nullptr);

    // base.func_main_0
    auto f0 = abc_file->GetFunctionByName("Lbase;func_main_0");
    size_t def_class_cnt = f0->GetDefinedClassCount();
    EXPECT_EQ(def_class_cnt, 1U);
    EXPECT_EQ(f0->GetDefinedClassByIndex(0)->GetClassName(), "Lbase;#~A=#A");

    // check each defined class
    // Lbase;#~A=#A
    auto *class0 = abc_file->GetClassByName("Lbase;#~A=#A");
    ASSERT_TRUE(class0->GetParentClass() == nullptr);
    ASSERT_TRUE(class0->GetDefiningFunction() == f0);
    EXPECT_EQ(class0->GetMemberFunctionByName("Lbase;#~A=#A"),
              class0->GetMemberFunctionByIndex(0));
    EXPECT_EQ(class0->GetMemberFunctionByName("Lbase;#~A>#add"),
              class0->GetMemberFunctionByIndex(1));

    // merge_test.func_main_0
    auto f1 = abc_file->GetFunctionByName("Lmerge_test;func_main_0");
    size_t def_class_cnt1  = f1->GetDefinedClassCount();
    EXPECT_EQ(def_class_cnt1, 1U);
    EXPECT_EQ(f1->GetDefinedClassByIndex(0)->GetClassName(), "Lmerge_test;#~B=#B");

    // check each defined class
    // Lmerge_test;#~B=#B
    auto *class1 = abc_file->GetClassByName("Lmerge_test;#~B=#B");
    ASSERT_TRUE(class1->GetParentClass() == nullptr);
    ASSERT_TRUE(class1->GetDefiningFunction() == f1);
    EXPECT_EQ(class1->GetMemberFunctionByName("Lmerge_test;#~B=#B"),
              class1->GetMemberFunctionByIndex(0));
    EXPECT_EQ(class1->GetMemberFunctionByName("Lmerge_test;#~B>#get"),
              class1->GetMemberFunctionByIndex(1));
}
}  // namespace panda::defect_scan_aux::test
