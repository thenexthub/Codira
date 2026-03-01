/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include <array>
#include <iostream>

#include <gtest/gtest.h>
#include "defect_scan_aux_api.h"

namespace panda::defect_scan_aux::test {

class DefectScanAuxTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};
};

static const Function *CheckFunction(std::unique_ptr<const AbcFile> &abc_file, std::string_view func_name)
{
    if (abc_file == nullptr) {
        EXPECT_NE(abc_file, nullptr);
        return nullptr;
    }
    auto func0 = abc_file->GetFunctionByName(func_name);
    if (func0 == nullptr) {
        EXPECT_NE(func0, nullptr);
        return nullptr;
    }
    EXPECT_EQ(func0->GetFunctionName(), func_name);
    return func0;
}

static bool ContainDefinedFunction(std::unique_ptr<const AbcFile> &abc_file,
                                   const Function *par_func, std::string_view func_name)
{
    if (abc_file == nullptr) {
        EXPECT_NE(abc_file, nullptr);
        return false;
    }
    size_t df_cnt0 = par_func->GetDefinedFunctionCount();
    for (size_t i = 0; i < df_cnt0; ++i) {
        auto df = par_func->GetDefinedFunctionByIndex(i);
        if (df->GetFunctionName() == func_name) {
            EXPECT_EQ(df->GetParentFunction(), par_func);
            return true;
        }
    }
    return false;
}

static bool ContainMemberFunction(std::unique_ptr<const AbcFile> &abc_file,
                                  const Class *class0, std::string_view func_name)
{
    if (abc_file == nullptr) {
        EXPECT_NE(abc_file, nullptr);
        return false;
    }
    size_t mf_func_count = class0->GetMemberFunctionCount();
    for (size_t i = 0; i < mf_func_count; ++i) {
        auto mf = class0->GetMemberFunctionByIndex(i);
        if (mf->GetFunctionName() == func_name) {
            EXPECT_EQ(class0->GetMemberFunctionByName(func_name), mf);
            return true;
        }
    }
    auto par_class = class0->GetParentClass();
    if (par_class != nullptr) {
        return ContainMemberFunction(abc_file, par_class, func_name);
    }
    return false;
}

static const Class *CheckClass(std::unique_ptr<const AbcFile> &abc_file, std::string_view class_name)
{
    if (abc_file == nullptr) {
        EXPECT_NE(abc_file, nullptr);
        return nullptr;
    }
    auto *class0 = abc_file->GetClassByName(class_name);
    if (class0 == nullptr) {
        EXPECT_NE(class0, nullptr);
        return nullptr;
    }
    EXPECT_EQ(class0->GetClassName(), class_name);
    [[maybe_unused]] size_t mf_func_count = class0->GetMemberFunctionCount();
    ASSERT(mf_func_count >= 1);
    [[maybe_unused]] auto mf_func0 = class0->GetMemberFunctionByIndex(0);
    if (mf_func0 == nullptr) {
        EXPECT_NE(mf_func0, nullptr);
        return nullptr;
    }
    EXPECT_EQ(abc_file->GetFunctionByName(class_name), mf_func0);
    EXPECT_EQ(class0->GetMemberFunctionByName(class_name), mf_func0);
    EXPECT_EQ(mf_func0->GetClass(), class0);
    CheckFunction(abc_file, class_name);
    return class0;
}

void CheckAbcFile(std::unique_ptr<const AbcFile> &abc_file, size_t test_def_func_cnt, size_t test_def_class_cnt)
{
    ASSERT_NE(abc_file, nullptr);
    size_t def_func_cnt = abc_file->GetDefinedFunctionCount();
    EXPECT_EQ(def_func_cnt, test_def_func_cnt);
    size_t def_class_cnt = abc_file->GetDefinedClassCount();
    EXPECT_EQ(def_class_cnt, test_def_class_cnt);
}

HWTEST(DefectScanAuxTest, DefineInfoFuncTest, testing::ext::TestSize.Level0)
{
    std::string test_name = DEFECT_SCAN_AUX_TEST_ABC_DIR "define_info_test.abc";
    auto abc_file = panda::defect_scan_aux::AbcFile::Open(test_name);
    CheckAbcFile(abc_file, 30U, 10U);

    // check each defined func
    // func_main_0
    auto f0 = CheckFunction(abc_file, "func_main_0");
    ASSERT_TRUE(f0->GetClass() == nullptr);
    ASSERT_TRUE(f0->GetParentFunction() == nullptr);
    size_t dc_cnt0 = f0->GetDefinedClassCount();
    EXPECT_EQ(dc_cnt0, 2U);
    EXPECT_EQ(f0->GetDefinedClassByIndex(0)->GetClassName(), "#~@1=#Bar");
    EXPECT_EQ(f0->GetDefinedClassByIndex(1)->GetClassName(), "#~@4=#ExampleClass1");
    size_t df_cnt0 = f0->GetDefinedFunctionCount();
    EXPECT_EQ(df_cnt0, 12U);
    ASSERT_TRUE(ContainDefinedFunction(abc_file, f0, "#*#func1"));
    ASSERT_TRUE(ContainDefinedFunction(abc_file, f0, "#*#func2"));
    ASSERT_FALSE(ContainDefinedFunction(abc_file, f0, "func1"));
    ASSERT_TRUE(ContainDefinedFunction(abc_file, f0, "#~@1>#func6"));
    ASSERT_TRUE(ContainDefinedFunction(abc_file, f0, "#~@1>#getName"));
    ASSERT_TRUE(ContainDefinedFunction(abc_file, f0, "#~@1>#setName"));
    ASSERT_TRUE(ContainDefinedFunction(abc_file, f0, "#~@1>#func9"));
    ASSERT_TRUE(ContainDefinedFunction(abc_file, f0, "#*#func10"));
    ASSERT_TRUE(ContainDefinedFunction(abc_file, f0, "#~@4>#func17"));
    // func2
    auto f1 = CheckFunction(abc_file, "#*#func2");
    ASSERT_NE(f1, nullptr);
    EXPECT_EQ(f1->GetArgCount(), 5U);
    size_t df_cnt1 = f1->GetDefinedFunctionCount();
    EXPECT_EQ(df_cnt1, 2U);
    ASSERT_TRUE(ContainDefinedFunction(abc_file, f1, "#*@0*#func4"));
    // func10
    auto f2 = CheckFunction(abc_file, "#*#func10");
    EXPECT_EQ(f2->GetArgCount(), 3U);
    size_t dc_cnt2 = f2->GetDefinedClassCount();
    EXPECT_EQ(dc_cnt2, 2U);
    EXPECT_EQ(f2->GetDefinedClassByIndex(0)->GetClassName(), "#*@2~@1=#Bar");
    EXPECT_EQ(f2->GetDefinedClassByIndex(1)->GetClassName(), "#*@2~@3=#Bar2");
    size_t df_cnt2 = f2->GetDefinedFunctionCount();
    EXPECT_EQ(df_cnt2, 7U);
    EXPECT_TRUE(ContainDefinedFunction(abc_file, f2, "#*@2~@1>#baseFoo1"));
    EXPECT_TRUE(ContainDefinedFunction(abc_file, f2, "#*@2~@3>#func12"));
    EXPECT_TRUE(ContainDefinedFunction(abc_file, f2, "#*@2~@3>#a"));
    EXPECT_TRUE(ContainDefinedFunction(abc_file, f2, "#*@2~@3>#symbol"));
    EXPECT_TRUE(ContainDefinedFunction(abc_file, f2, "#*@2~@3>#func15"));
}

HWTEST(DefectScanAuxTest, DefineInfoClassTest, testing::ext::TestSize.Level0)
{
    std::string test_name = DEFECT_SCAN_AUX_TEST_ABC_DIR "define_info_test.abc";
    auto abc_file = panda::defect_scan_aux::AbcFile::Open(test_name);
    CheckAbcFile(abc_file, 30U, 10U);

    // check each defined class
    // #1#Bar
    auto f0 = CheckFunction(abc_file, "func_main_0");
    auto class0 = CheckClass(abc_file, "#~@1=#Bar");
    ASSERT_TRUE(class0->GetParentClass() == nullptr);
    ASSERT_TRUE(class0->GetDefiningFunction() == f0);
    size_t mf_count0 = class0->GetMemberFunctionCount();
    EXPECT_EQ(mf_count0, 5U);
    EXPECT_TRUE(ContainMemberFunction(abc_file, class0, "#~@1>#func6"));
    EXPECT_TRUE(ContainMemberFunction(abc_file, class0, "#~@1>#getName"));
    EXPECT_TRUE(ContainMemberFunction(abc_file, class0, "#~@1>#setName"));
    EXPECT_TRUE(ContainMemberFunction(abc_file, class0, "#~@1>#func9"));
    // #3#Bar2
    auto class1 = CheckClass(abc_file, "#*@2~@3=#Bar2");
    ASSERT_TRUE(class1->GetParentClass() != nullptr);
    ASSERT_TRUE(class1->GetDefiningFunction() == abc_file->GetFunctionByName("#*#func10"));
    size_t mf_count1 = class1->GetMemberFunctionCount();
    EXPECT_EQ(mf_count1, 5U);
    EXPECT_TRUE(ContainMemberFunction(abc_file, class1, "#*@2~@1>#baseFoo1"));
    EXPECT_TRUE(ContainMemberFunction(abc_file, class1, "#*@2~@3>#func12"));
    EXPECT_TRUE(ContainMemberFunction(abc_file, class1, "#*@2~@3>#func15"));
    // #8#ExampleClass2
    auto class2 = CheckClass(abc_file, "#*@5*@7~@6=#ExampleClass2");
    EXPECT_EQ(class2->GetParentClassName(), "#*@5~@4=#ExampleClass1");
    EXPECT_FALSE(ContainMemberFunction(abc_file, class2, "#~@4=#func17"));
    EXPECT_TRUE(ContainMemberFunction(abc_file, class2, "#*@5~@4>#func19"));
    // #9#ExtendService
    auto class3 = CheckClass(abc_file, "#*@9~@8=#ExtendService");
    ASSERT_TRUE(class3->GetParentClass() == nullptr);
    EXPECT_EQ(class3->GetParentClassName(), "BaseService");
    EXPECT_EQ(class3->GetParClassExternalModuleName(), "../base/service");
    EXPECT_TRUE(class3->GetParClassGlobalVarName().empty());
    // #10#ExtendPhoneService
    auto class4 = CheckClass(abc_file, "#*@9~@a=#ExtendPhoneService");
    ASSERT_TRUE(class4->GetParentClass() == nullptr);
    EXPECT_EQ(class4->GetParentClassName(), "PhoneService");
    EXPECT_EQ(class4->GetParClassExternalModuleName(), "../mod1");
    EXPECT_TRUE(class4->GetParClassGlobalVarName().empty());
    // #11#ExtendDataSource
    auto class5 = CheckClass(abc_file, "#*@9~@b=#ExtendDataSource");
    ASSERT_TRUE(class5->GetParentClass() == nullptr);
    EXPECT_EQ(class5->GetParentClassName(), "BasicDataSource");
    EXPECT_TRUE(class5->GetParClassExternalModuleName().empty());
    EXPECT_EQ(class5->GetParClassGlobalVarName(), "globalvar");
    // #12#ExtendDataItem
    auto class6 = CheckClass(abc_file, "#*@9~@c=#ExtendDataItem");
    ASSERT_TRUE(class6->GetParentClass() == nullptr);
    EXPECT_EQ(class6->GetParentClassName(), "DataItem");
    EXPECT_TRUE(class6->GetParClassExternalModuleName().empty());
    EXPECT_EQ(class6->GetParClassGlobalVarName(), "globalvar2.Data");
}

HWTEST(DefectScanAuxTest, DebugInfoTest, testing::ext::TestSize.Level0)
{
    std::string test_name = DEFECT_SCAN_AUX_TEST_ABC_DIR "debug_info_test.abc";
    auto abc_file = panda::defect_scan_aux::AbcFile::Open(test_name);
    ASSERT_NE(abc_file, nullptr);

    // check debug info, whether the line number obtained from call inst is correct
    auto f0 = abc_file->GetFunctionByName("#*#foo");
    ASSERT_NE(f0, nullptr);
    EXPECT_NE(abc_file->GetDefinedFunctionByIndex(0), nullptr);
    EXPECT_NE(abc_file->GetAbcFileName(), "");
    EXPECT_NE(abc_file->GetDefinedClassByIndex(0), nullptr);
    EXPECT_EQ(f0->GetCalleeInfoCount(), 3U);
    auto ci0_0 = f0->GetCalleeInfoByIndex(0);
    ASSERT_NE(ci0_0, nullptr);
    // callarg0
    EXPECT_EQ(abc_file->GetLineNumberByInst(f0, ci0_0->GetCallInst()), 305);
    auto ci0_1 = f0->GetCalleeInfoByIndex(1);
    ASSERT_NE(ci0_1, nullptr);
    // callspread
    EXPECT_EQ(abc_file->GetLineNumberByInst(f0, ci0_1->GetCallInst()), 309);
    auto ci0_2 = f0->GetCalleeInfoByIndex(2);
    ASSERT_NE(ci0_2, nullptr);
    // callirange
    EXPECT_EQ(abc_file->GetLineNumberByInst(f0, ci0_2->GetCallInst()), 311);
    // ctor of Data
    auto f1 = abc_file->GetFunctionByName("#*@2~@1=#Data");
    ASSERT_NE(f1, nullptr);
    EXPECT_EQ(f1->GetCalleeInfoCount(), 1U);
    auto ci1_0 = f1->GetCalleeInfoByIndex(0);
    // supercall
    EXPECT_EQ(abc_file->GetLineNumberByInst(f1, ci1_0->GetCallInst()), 331);
    // bar
    auto f2 = abc_file->GetFunctionByName("#*#bar");
    ASSERT_NE(f2, nullptr);
    EXPECT_EQ(f2->GetCalleeInfoCount(), 2U);
    auto ci2_0 = f2->GetCalleeInfoByIndex(0);
    ASSERT_NE(ci2_0, nullptr);
    // callithisrange
    EXPECT_EQ(abc_file->GetLineNumberByInst(f2, ci2_0->GetCallInst()), 341);
    auto ci2_1 = f2->GetCalleeInfoByIndex(1);
    ASSERT_NE(ci2_1, nullptr);
    // callithisrange
    EXPECT_EQ(abc_file->GetLineNumberByInst(f2, ci2_1->GetCallInst()), 346);
}

HWTEST(DefectScanAuxTest, CalleeInfoTest1, testing::ext::TestSize.Level0)
{
    std::string test_name = DEFECT_SCAN_AUX_TEST_ABC_DIR "callee_info_test.abc";
    auto abc_file = panda::defect_scan_aux::AbcFile::Open(test_name);
    CheckAbcFile(abc_file, 19U, 2U);

    // check callee info of each func
    // foo
    auto f0 = abc_file->GetFunctionByName("#*#foo");
    ASSERT_NE(abc_file, nullptr);
    ASSERT_EQ(abc_file->GetFunctionByName(""), nullptr);
    size_t ci_cnt0 = f0->GetCalleeInfoCount();
    EXPECT_EQ(ci_cnt0, 6U);
    auto ci0_0 = f0->GetCalleeInfoByIndex(0);
    ASSERT_NE(abc_file, nullptr);
    ASSERT_TRUE(f0->GetCalleeInfoByCallInst(ci0_0->GetCallInst()) == ci0_0);
    ASSERT_TRUE(ci0_0->IsCalleeDefinite());
    EXPECT_EQ(ci0_0->GetCalleeArgCount(), 1);
    ASSERT_TRUE(ci0_0->GetCaller() == f0);
    ASSERT_TRUE(ci0_0->GetCallee() == abc_file->GetFunctionByName("#*@0*#func2"));
    auto ci0_1 = f0->GetCalleeInfoByIndex(1);
    ASSERT_NE(ci0_1, nullptr);
    EXPECT_EQ(f0->GetCalleeInfoByCallInst(ci0_1->GetCallInst()), ci0_1);
    ASSERT_FALSE(ci0_1->IsCalleeDefinite());
    EXPECT_EQ(ci0_1->GetCalleeArgCount(), 1);
    ASSERT_TRUE(ci0_1->GetCallee() == nullptr);
    EXPECT_EQ(ci0_1->GetFunctionName(), "log");
    EXPECT_EQ(ci0_1->GetGlobalVarName(), "console");
    auto ci0_2 = f0->GetCalleeInfoByIndex(2);
    ASSERT_NE(ci0_2, nullptr);
    ASSERT_FALSE(ci0_2->IsCalleeDefinite());
    EXPECT_EQ(ci0_2->GetCalleeArgCount(), 1);
    ASSERT_TRUE(ci0_2->GetCallee() == nullptr);
    EXPECT_EQ(ci0_2->GetFunctionName(), "logd");
    EXPECT_EQ(ci0_2->GetGlobalVarName(), "globalvar.hilog");
    auto ci0_3 = f0->GetCalleeInfoByIndex(3);
    ASSERT_NE(ci0_3, nullptr);
    ASSERT_TRUE(ci0_3->IsCalleeDefinite());
    ASSERT_TRUE(ci0_3->GetCallee() == abc_file->GetFunctionByName("#*@0*#func2"));
    auto ci0_4 = f0->GetCalleeInfoByIndex(4);
    ASSERT_NE(ci0_4, nullptr);
    ASSERT_TRUE(ci0_4->IsCalleeDefinite());
    EXPECT_EQ(ci0_4->GetCalleeArgCount(), 2);
    ASSERT_TRUE(ci0_4->GetCallee() == abc_file->GetFunctionByName("#*#func1"));
    auto ci0_5 = f0->GetCalleeInfoByIndex(5);
    ASSERT_NE(ci0_5, nullptr);
    ASSERT_FALSE(ci0_5->IsCalleeDefinite());
    EXPECT_EQ(ci0_5->GetFunctionName(), "bar");
}

HWTEST(DefectScanAuxTest, CalleeInfoTest2, testing::ext::TestSize.Level0)
{
    std::string test_name = DEFECT_SCAN_AUX_TEST_ABC_DIR "callee_info_test.abc";
    auto abc_file = panda::defect_scan_aux::AbcFile::Open(test_name);
    CheckAbcFile(abc_file, 19U, 2U);

    // check callee info of each func
    // foo1
    auto f1 = abc_file->GetFunctionByName("#*#foo1");
    ASSERT_NE(f1, nullptr);
    size_t ci_cnt1 = f1->GetCalleeInfoCount();
    EXPECT_EQ(ci_cnt1, 3U);
    auto ci1_0 = f1->GetCalleeInfoByIndex(0);
    ASSERT_NE(ci1_0, nullptr);
    ASSERT_TRUE(ci1_0->IsCalleeDefinite());
    ASSERT_TRUE(ci1_0->GetCallee() == abc_file->GetFunctionByName("#*@1*#fn"));
    auto ci1_1 = f1->GetCalleeInfoByIndex(1);
    ASSERT_NE(ci1_1, nullptr);
    ASSERT_TRUE(ci1_1->IsCalleeDefinite());
    ASSERT_TRUE(ci1_1->GetCallee() == abc_file->GetFunctionByName("#*@1*#fn"));
    auto ci1_2 = f1->GetCalleeInfoByIndex(2);
    ASSERT_NE(ci1_2, nullptr);
    ASSERT_FALSE(ci1_2->IsCalleeDefinite());
    EXPECT_EQ(ci1_2->GetFunctionName(), "bind");
    // #2#ColorPoint
    auto f2 = abc_file->GetFunctionByName("#*@4~@3=#ColorPoint");
    ASSERT_NE(f2, nullptr);
    size_t ci_cnt2 = f2->GetCalleeInfoCount();
    EXPECT_EQ(ci_cnt2, 1U);
    auto ci2_0 = f2->GetCalleeInfoByIndex(0);
    ASSERT_NE(ci2_0, nullptr);
    ASSERT_TRUE(ci2_0->IsCalleeDefinite());
    ASSERT_TRUE(ci2_0->GetClass() == abc_file->GetClassByName("#~@2=#Point"));
    ASSERT_TRUE(ci2_0->GetCallee() == abc_file->GetFunctionByName("#~@2=#Point"));
    EXPECT_EQ(abc_file->GetClassByName(""), nullptr);
    // func6
    auto f3 = abc_file->GetFunctionByName("#*#func6");
    ASSERT_NE(f3, nullptr);
    size_t ci_cnt3 = f3->GetCalleeInfoCount();
    EXPECT_EQ(ci_cnt3, 1U);
    auto ci3_0 = f3->GetCalleeInfoByIndex(0);
    EXPECT_FALSE(ci3_0->IsCalleeDefinite());
    EXPECT_EQ(ci3_0->GetFunctionName(), "bar");
    EXPECT_EQ(ci3_0->GetExternalModuleName(), "./mod2");
}

HWTEST(DefectScanAuxTest, CalleeInfoTest3, testing::ext::TestSize.Level0)
{
    std::string test_name = DEFECT_SCAN_AUX_TEST_ABC_DIR "callee_info_test.abc";
    auto abc_file = panda::defect_scan_aux::AbcFile::Open(test_name);
    CheckAbcFile(abc_file, 19U, 2U);

    // check callee info of each func
    // func7
    auto f4 = abc_file->GetFunctionByName("#*@5*#func7");
    ASSERT_NE(f4, nullptr);
    size_t ci_cnt4 = f4->GetCalleeInfoCount();
    EXPECT_EQ(ci_cnt4, 2U);
    auto ci4_0 = f4->GetCalleeInfoByIndex(0);
    ASSERT_NE(ci4_0, nullptr);
    ASSERT_FALSE(ci4_0->IsCalleeDefinite());
    EXPECT_EQ(ci4_0->GetCalleeArgCount(), 2);
    EXPECT_EQ(ci4_0->GetFunctionName(), "sum");
    EXPECT_EQ(ci4_0->GetExternalModuleName(), "../../mod1");
    auto ci4_1 = f4->GetCalleeInfoByIndex(1);
    ASSERT_NE(ci4_1, nullptr);
    EXPECT_FALSE(ci4_1->IsCalleeDefinite());
    EXPECT_EQ(ci4_1->GetCalleeArgCount(), 2);
    EXPECT_EQ(ci4_1->GetFunctionName(), "sub");
    EXPECT_EQ(ci4_1->GetExternalModuleName(), "../../mod1");
    // callMemberFunc1
    auto f5 = abc_file->GetFunctionByName("#*@6*#callMemberFunc1");
    ASSERT_NE(f5, nullptr);
    size_t ci_cnt5 = f5->GetCalleeInfoCount();
    EXPECT_EQ(ci_cnt5, 2U);
    auto ci5_0 = f5->GetCalleeInfoByIndex(0);
    ASSERT_NE(ci5_0, nullptr);
    EXPECT_TRUE(ci5_0->IsCalleeDefinite());
    EXPECT_TRUE(ci5_0->GetClass() != nullptr);
    EXPECT_EQ(ci5_0->GetClass(), abc_file->GetClassByName("#~@2=#Point"));
    EXPECT_TRUE(ci5_0->GetCallee() != nullptr);
    EXPECT_EQ(ci5_0->GetCallee(), abc_file->GetFunctionByName("#~@2>#getCoordinateX"));
    auto ci5_1 = f5->GetCalleeInfoByIndex(1);
    ASSERT_NE(ci5_1, nullptr);
    EXPECT_TRUE(ci5_1->IsCalleeDefinite());
    EXPECT_TRUE(ci5_1->GetClass() != nullptr);
    EXPECT_EQ(ci5_1->GetClass(), abc_file->GetClassByName("#~@2=#Point"));
    EXPECT_TRUE(ci5_1->GetCallee() != nullptr);
    EXPECT_EQ(ci5_1->GetCallee(), abc_file->GetFunctionByName("#~@2>#setCoordinateX"));
}

HWTEST(DefectScanAuxTest, CalleeInfoTest4, testing::ext::TestSize.Level0)
{
    std::string test_name = DEFECT_SCAN_AUX_TEST_ABC_DIR "callee_info_test.abc";
    auto abc_file = panda::defect_scan_aux::AbcFile::Open(test_name);
    CheckAbcFile(abc_file, 19U, 2U);

    // check callee info of each func
    // callMemberFunc2
    auto f6 = abc_file->GetFunctionByName("#*@6*@7*#callMemberFunc2");
    ASSERT_NE(f6, nullptr);
    size_t ci_cnt6 = f6->GetCalleeInfoCount();
    EXPECT_EQ(ci_cnt6, 2U);
    auto ci6_0 = f6->GetCalleeInfoByIndex(0);
    ASSERT_NE(ci6_0, nullptr);
    EXPECT_TRUE(ci6_0->IsCalleeDefinite());
    EXPECT_TRUE(ci6_0->GetClass() != nullptr);
    EXPECT_EQ(ci6_0->GetClass(), abc_file->GetClassByName("#~@2=#Point"));
    EXPECT_TRUE(ci6_0->GetCallee() != nullptr);
    EXPECT_EQ(ci6_0->GetCallee(), abc_file->GetFunctionByName("#~@2>#plus"));
    auto ci6_1 = f6->GetCalleeInfoByIndex(1);
    ASSERT_NE(ci6_1, nullptr);
    EXPECT_FALSE(ci6_1->IsCalleeDefinite());
    EXPECT_EQ(ci6_1->GetFunctionName(), "sub");
    EXPECT_EQ(ci6_1->GetExternalModuleName(), "");
    EXPECT_EQ(ci6_1->GetGlobalVarName(), "");
    // callExClassMemberFunc1
    auto f7 = abc_file->GetFunctionByName("#*@8*#callExClassMemberFunc1");
    ASSERT_NE(f7, nullptr);
    size_t ci_cnt7 = f7->GetCalleeInfoCount();
    EXPECT_EQ(ci_cnt7, 1U);
    auto ci7_0 = f7->GetCalleeInfoByIndex(0);
    ASSERT_NE(ci7_0, nullptr);
    EXPECT_FALSE(ci7_0->IsCalleeDefinite());
    EXPECT_TRUE(ci7_0->GetClass() == nullptr);
    EXPECT_EQ(ci7_0->GetFunctionName(), "makePhoneCall");
    EXPECT_EQ(ci7_0->GetExternalModuleName(), "../../mod1");
    EXPECT_EQ(ci7_0->GetGlobalVarName(), "");
}

HWTEST(DefectScanAuxTest, GraphTest, testing::ext::TestSize.Level0)
{
    std::string test_name = DEFECT_SCAN_AUX_TEST_ABC_DIR "graph_test.abc";
    auto abc_file = panda::defect_scan_aux::AbcFile::Open(test_name);
    ASSERT_NE(abc_file, nullptr);
    auto f0 = CheckFunction(abc_file, "#*#foo");
    ASSERT_NE(f0, nullptr);
    // check api of graph
    auto &graph = f0->GetGraph();
    auto bb_list = graph.GetBasicBlockList();
    EXPECT_EQ(bb_list.size(), 8u);
    EXPECT_EQ(bb_list.front(), graph.GetStartBasicBlock());
    EXPECT_EQ(bb_list.back(), graph.GetEndBasicBlock());
    std::unordered_map<InstType, uint32_t> inst_cnt_table;
    graph.VisitAllInstructions([&inst_cnt_table](const Inst &inst) {
        auto type = inst.GetType();
        inst_cnt_table.insert_or_assign(type, inst_cnt_table[type] + 1);
    });
    uint32_t newobj_cnt = inst_cnt_table[InstType::NEWOBJRANGE_IMM8_IMM8_V8] +
                            inst_cnt_table[InstType::NEWOBJRANGE_IMM16_IMM8_V8] +
                            inst_cnt_table[InstType::WIDE_NEWOBJRANGE_PREF_IMM16_V8];
    EXPECT_EQ(newobj_cnt, 2U);
    uint32_t ldlex_cnt = inst_cnt_table[InstType::LDLEXVAR_IMM4_IMM4] +
                            inst_cnt_table[InstType::LDLEXVAR_IMM8_IMM8] +
                            inst_cnt_table[InstType::WIDE_LDLEXVAR_PREF_IMM16_IMM16];
    EXPECT_EQ(ldlex_cnt, 7U);
    EXPECT_EQ(
        inst_cnt_table[InstType::LDOBJBYNAME_IMM8_ID16] + inst_cnt_table[InstType::LDOBJBYNAME_IMM16_ID16], 4U);
    EXPECT_EQ(inst_cnt_table[InstType::CALLTHIS1_IMM8_V8_V8], 3U);
    EXPECT_EQ(inst_cnt_table[InstType::CALLTHIS2_IMM8_V8_V8_V8], 1U);
    EXPECT_EQ(inst_cnt_table[InstType::DEFINEFUNC_IMM8_ID16_IMM8] +
                                inst_cnt_table[InstType::DEFINEFUNC_IMM16_ID16_IMM8],
                            1U);
    EXPECT_EQ(inst_cnt_table[InstType::CALLARG0_IMM8], 1U);
    EXPECT_EQ(inst_cnt_table[InstType::CALLARG1_IMM8_V8], 1U);
    EXPECT_EQ(inst_cnt_table[InstType::LDEXTERNALMODULEVAR_IMM8] +
                                inst_cnt_table[InstType::WIDE_LDEXTERNALMODULEVAR_PREF_IMM16],
                            1U);
    EXPECT_EQ(inst_cnt_table[InstType::GETMODULENAMESPACE_IMM8] +
                                inst_cnt_table[InstType::WIDE_GETMODULENAMESPACE_PREF_IMM16],
                            0U);
    EXPECT_EQ(inst_cnt_table[InstType::OPCODE_PARAMETER], 5U);
}

HWTEST(DefectScanAuxTest, BasicBlockTest, testing::ext::TestSize.Level0)
{
    std::string test_name = DEFECT_SCAN_AUX_TEST_ABC_DIR "graph_test.abc";
    auto abc_file = panda::defect_scan_aux::AbcFile::Open(test_name);
    ASSERT_NE(abc_file, nullptr);
    auto f0 = CheckFunction(abc_file, "#*#foo");
    ASSERT_NE(f0, nullptr);
    // check api of basic block
    auto &graph = f0->GetGraph();
    auto bb0 = graph.GetStartBasicBlock();
    EXPECT_EQ(bb0.GetPredBlocks().size(), 0U);
    EXPECT_EQ(bb0.GetSuccBlocks().size(), 1U);
    auto bb1 = bb0.GetSuccBlocks()[0];
    EXPECT_EQ(bb1.GetPredBlocks().size(), 1U);
    EXPECT_EQ(bb1.GetPredBlocks()[0], bb0);
    auto bb1_succ_bb = bb1.GetSuccBlocks();
    EXPECT_EQ(bb1_succ_bb.size(), 2U);
    ASSERT_TRUE(
        (bb1_succ_bb[0].GetSuccBlocks().size() == 2 && bb1_succ_bb[1].GetSuccBlocks().size() == 1) ||
        (bb1_succ_bb[0].GetSuccBlocks().size() == 1 && bb1_succ_bb[1].GetSuccBlocks().size() == 2));
    auto bb2 = graph.GetEndBasicBlock();
    auto bb2_pred_bb = bb2.GetPredBlocks();
    EXPECT_EQ(bb2_pred_bb.size(), 2U);
    auto bb3 = bb2_pred_bb[0];
    auto bb4 = bb2_pred_bb[1];
    if (bb3.GetPredBlocks().size() < bb4.GetPredBlocks().size()) {
        std::swap(bb3, bb4);
    }
    EXPECT_EQ(bb3.GetPredBlocks().size(), 2U);
    EXPECT_EQ(bb4.GetPredBlocks().size(), 1U);
    EXPECT_EQ(bb4.GetPredBlocks()[0], bb1);
}

HWTEST(DefectScanAuxTest, InstTest1, testing::ext::TestSize.Level0)
{
    std::string test_name = DEFECT_SCAN_AUX_TEST_ABC_DIR "graph_test.abc";
    auto abc_file = panda::defect_scan_aux::AbcFile::Open(test_name);
    ASSERT_NE(abc_file, nullptr);
    auto f0 = CheckFunction(abc_file, "#*#foo");
    ASSERT_NE(f0, nullptr);
    // Test 1 : check api of inst
    size_t ci_cnt = f0->GetCalleeInfoCount();
    EXPECT_EQ(ci_cnt, 6U);
    auto ci0 = f0->GetCalleeInfoByIndex(ci_cnt - 1);
    auto call_inst0 = ci0->GetCallInst();
    auto call_inst0_ins = call_inst0.GetInputInsts();
    EXPECT_EQ(call_inst0_ins.size(), 4U);
    auto call_inst0_in1_type = call_inst0_ins[1].GetType();
    EXPECT_EQ(call_inst0_in1_type, InstType::CALLARG0_IMM8);
    EXPECT_EQ(call_inst0_ins[1].GetUserInsts().size(), 1U);
    EXPECT_EQ(call_inst0_ins[1].GetUserInsts()[0], call_inst0);
    auto call_inst0_in2_type = call_inst0_ins[2].GetType();
    EXPECT_EQ(call_inst0_in2_type, InstType::OPCODE_PARAMETER);
    EXPECT_EQ(call_inst0_ins[2].GetArgIndex(), 4U);
    auto param1_usrs = call_inst0_ins[2].GetUserInsts();
    ASSERT_TRUE(std::find(param1_usrs.begin(), param1_usrs.end(), call_inst0) != param1_usrs.end());
    auto call_inst0_in3_type = call_inst0_ins[3].GetType();
    ASSERT_TRUE(call_inst0_in3_type == InstType::LDOBJBYNAME_IMM8_ID16 ||
                            call_inst0_in3_type == InstType::LDOBJBYNAME_IMM16_ID16);
    EXPECT_EQ(call_inst0_ins[3].GetUserInsts().size(), 1U);
    EXPECT_EQ(call_inst0_ins[3].GetUserInsts()[0], call_inst0);
}

HWTEST(DefectScanAuxTest, InstTest2, testing::ext::TestSize.Level0)
{
    std::string test_name = DEFECT_SCAN_AUX_TEST_ABC_DIR "graph_test.abc";
    auto abc_file = panda::defect_scan_aux::AbcFile::Open(test_name);
    ASSERT_NE(abc_file, nullptr);
    auto f0 = CheckFunction(abc_file, "#*#foo");
    ASSERT_NE(f0, nullptr);
    // Test 2 : check api of inst
    size_t ci_cnt = f0->GetCalleeInfoCount();
    auto ci1 = f0->GetCalleeInfoByIndex(ci_cnt - 2);
    auto call_inst1 = ci1->GetCallInst();
    auto call_inst1_ins = call_inst1.GetInputInsts();
    EXPECT_EQ(call_inst1_ins.size(), 2U);
    auto call_inst1_in0_type = call_inst1_ins[1].GetType();
    ASSERT_TRUE(call_inst1_in0_type == InstType::LDEXTERNALMODULEVAR_IMM8 ||
                            call_inst1_in0_type == InstType::WIDE_LDEXTERNALMODULEVAR_PREF_IMM16);
    EXPECT_EQ(call_inst1_ins[1].GetUserInsts().size(), 2U);
    ASSERT_TRUE((call_inst1_ins[1].GetUserInsts()[0] == call_inst1) ||
                            (call_inst1_ins[1].GetUserInsts()[1] == call_inst1));
    auto phi_inst = call_inst1_ins[0];
    EXPECT_EQ(phi_inst.GetType(), InstType::OPCODE_PHI);
    auto phi_inst_ins = phi_inst.GetInputInsts();
    EXPECT_EQ(phi_inst_ins.size(), 2U);
    auto phi_inst_in0 = phi_inst_ins[0];
    auto phi_inst_in1 = phi_inst_ins[1];
    if (phi_inst_in0.GetType() != InstType::OPCODE_PARAMETER) {
        std::swap(phi_inst_in0, phi_inst_in1);
    }
    EXPECT_EQ(phi_inst_in0.GetType(), InstType::OPCODE_PARAMETER);
    EXPECT_EQ(phi_inst_in0.GetArgIndex(), 3U);
    auto param0_usrs = phi_inst_in0.GetUserInsts();
    ASSERT_TRUE(std::find(param0_usrs.begin(), param0_usrs.end(), phi_inst) != param0_usrs.end());
    EXPECT_EQ(phi_inst_in1.GetType(), InstType::ADD2_IMM8_V8);
    auto add2_inst_ins = phi_inst_in1.GetInputInsts();
    EXPECT_EQ(add2_inst_ins.size(), 2U);
    EXPECT_EQ(add2_inst_ins[0].GetType(), InstType::OPCODE_PARAMETER);
    EXPECT_EQ(add2_inst_ins[1].GetType(), InstType::OPCODE_PARAMETER);
}

HWTEST(DefectScanAuxTest, ModuleInfoClassTest, testing::ext::TestSize.Level0)
{
    const std::string USER_INPUT = "#~@1=#UserInput";
    const std::string UI_INPUT = "UInput";
    const std::string GET_TEXT = "#~@1>#getText";
    const std::string INNER_USER_INPUT = "#~@0=#InnerUserInput";
    const std::string GET_TEXT_BASE = "#~@0>#getTextBase";
    const std::string FUNC1 = "#*#func1";
    const std::string FUNC2 = "#*#func2";
    const std::string FUNC3 = "#*#func3";

    std::string test_name = DEFECT_SCAN_AUX_TEST_ABC_DIR "module_info_test.abc";
    auto abc_file = panda::defect_scan_aux::AbcFile::Open(test_name);
    CheckAbcFile(abc_file, 8U, 2U);

    // check exported class
    std::string inter_name0 = abc_file->GetLocalNameByExportName(UI_INPUT);
    EXPECT_EQ(inter_name0, "UserInput");
    auto ex_class = abc_file->GetExportClassByExportName(UI_INPUT);
    EXPECT_EQ(ex_class->GetClassName(), USER_INPUT);
    CheckClass(abc_file, USER_INPUT);
    size_t mf_func_count0 = ex_class->GetMemberFunctionCount();
    EXPECT_EQ(mf_func_count0, 2U);
    auto mf_func0_1 = ex_class->GetMemberFunctionByIndex(1);
    EXPECT_EQ(mf_func0_1->GetFunctionName(), GET_TEXT);
    ASSERT_TRUE(mf_func0_1->GetClass() == ex_class);
    CheckFunction(abc_file, GET_TEXT);
    auto par_class = ex_class->GetParentClass();
    ASSERT_TRUE(par_class != nullptr);
    EXPECT_EQ(par_class->GetClassName(), INNER_USER_INPUT);
    CheckClass(abc_file, INNER_USER_INPUT);
    size_t mf_func_count1 = par_class->GetMemberFunctionCount();
    EXPECT_EQ(mf_func_count1, 2U);
    auto mf_func1_1 = par_class->GetMemberFunctionByIndex(1);
    EXPECT_EQ(mf_func1_1->GetFunctionName(), GET_TEXT_BASE);
    ASSERT_TRUE(mf_func1_1->GetClass() == par_class);
    CheckFunction(abc_file, GET_TEXT_BASE);
}

HWTEST(DefectScanAuxTest, ModuleInfoTest, testing::ext::TestSize.Level0)
{
    const std::string USER_INPUT = "#~@1=#UserInput";
    const std::string UI_INPUT = "UInput";
    const std::string GET_TEXT = "#~@1>#getText";
    const std::string INNER_USER_INPUT = "#~@0=#InnerUserInput";
    const std::string GET_TEXT_BASE = "#~@0>#getTextBase";
    const std::string FUNC1 = "#*#func1";
    const std::string FUNC2 = "#*#func2";
    const std::string FUNC3 = "#*#func3";

    std::string test_name = DEFECT_SCAN_AUX_TEST_ABC_DIR "module_info_test.abc";
    auto abc_file = panda::defect_scan_aux::AbcFile::Open(test_name);
    CheckAbcFile(abc_file, 8U, 2U);

    // check exported func
    // func3
    std::string inter_name1 = abc_file->GetLocalNameByExportName("exFunc3");
    EXPECT_EQ(inter_name1, "func3");
    auto ex_func1 = abc_file->GetExportFunctionByExportName("exFunc3");
    EXPECT_EQ(ex_func1->GetFunctionName(), FUNC3);
    CheckFunction(abc_file, FUNC3);
    // func1
    std::string inter_name2 = abc_file->GetLocalNameByExportName("default");
    EXPECT_EQ(inter_name2, "func1");
    auto ex_func2 = abc_file->GetExportFunctionByExportName("default");
    EXPECT_EQ(ex_func2->GetFunctionName(), FUNC1);
    CheckFunction(abc_file, FUNC1);
    // func2
    std::string inter_name3 = abc_file->GetLocalNameByExportName("func2");
    EXPECT_EQ(inter_name3, "func2");
    auto ex_func3 = abc_file->GetExportFunctionByExportName("func2");
    EXPECT_EQ(ex_func3->GetFunctionName(), FUNC2);
    CheckFunction(abc_file, FUNC2);

    // GetModuleNameByLocalName
    std::string mod_name0 = abc_file->GetModuleNameByLocalName("var1");
    EXPECT_EQ(mod_name0, "./mod1");
    std::string mod_name1 = abc_file->GetModuleNameByLocalName("ns");
    EXPECT_EQ(mod_name1, "./mod2");
    std::string mod_name2 = abc_file->GetModuleNameByLocalName("var3");
    EXPECT_EQ(mod_name2, "../mod3");
    std::string mod_name3 = abc_file->GetModuleNameByLocalName("localVar4");
    EXPECT_EQ(mod_name3, "../../mod4");

    // GetImportNameByLocalName
    std::string im_name0 = abc_file->GetImportNameByLocalName("var1");
    EXPECT_EQ(im_name0, "default");
    std::string im_name1 = abc_file->GetImportNameByLocalName("var3");
    EXPECT_EQ(im_name1, "var3");
    std::string im_name2 = abc_file->GetImportNameByLocalName("localVar4");
    EXPECT_EQ(im_name2, "var4");
    // GetImportNameByExportName
    std::string ind_im_name0 = abc_file->GetImportNameByExportName("v");
    EXPECT_EQ(ind_im_name0, "v5");
    std::string ind_im_name1 = abc_file->GetImportNameByExportName("foo");
    EXPECT_EQ(ind_im_name1, "foo");
    // GetModuleNameByExportName
    std::string ind_mod_name0 = abc_file->GetModuleNameByExportName("v");
    EXPECT_EQ(ind_mod_name0, "./mod5");
    std::string ind_mod_name1 = abc_file->GetModuleNameByExportName("foo");
    EXPECT_EQ(ind_mod_name1, "../../mod7");
}

HWTEST(DefectScanAuxTest, AsyncFunctionTest, testing::ext::TestSize.Level0)
{
    std::string test_name = DEFECT_SCAN_AUX_TEST_ABC_DIR "async_function_test.abc";
    auto abc_file = panda::defect_scan_aux::AbcFile::Open(test_name);
    std::string test_name1 = DEFECT_SCAN_AUX_TEST_ABC_DIR;
    auto abc_file1 = panda::defect_scan_aux::AbcFile::Open(test_name1);
    EXPECT_EQ(abc_file1, nullptr);
    ASSERT(abc_file != nullptr);

    auto func_main = abc_file->GetFunctionByName("func_main_0");
    auto func_count = func_main->GetDefinedFunctionCount();

    std::unordered_map<InstType, uint32_t> inst_cnt_table;

    for (size_t i = 0; i < func_count; ++i) {
        auto func = func_main->GetDefinedFunctionByIndex(i);
        auto graph = func->GetGraph();
        graph.VisitAllInstructions([&inst_cnt_table](const Inst &inst) {
            auto type = inst.GetType();
            inst_cnt_table.insert_or_assign(type, inst_cnt_table[type] + 1);
        });
    }
    EXPECT_EQ(inst_cnt_table[InstType::CREATEASYNCGENERATOROBJ_V8], 1U);
    EXPECT_EQ(inst_cnt_table[InstType::ASYNCGENERATORRESOLVE_V8_V8_V8], 3U);
    EXPECT_EQ(inst_cnt_table[InstType::ASYNCGENERATORREJECT_V8], 1U);
    EXPECT_EQ(inst_cnt_table[InstType::GETASYNCITERATOR_IMM8], 1U);
    EXPECT_EQ(inst_cnt_table[InstType::SETGENERATORSTATE_IMM8], 4U);
    EXPECT_EQ(inst_cnt_table[InstType::CALLRUNTIME_NOTIFYCONCURRENTRESULT_PREF_NONE], 6U);
}

HWTEST(DefectScanAuxTest, SendableClassTest, testing::ext::TestSize.Level0)
{
    std::string test_name = DEFECT_SCAN_AUX_TEST_ABC_DIR "sendable_class_test.abc";
    auto abc_file = panda::defect_scan_aux::AbcFile::Open(test_name);
    ASSERT(abc_file != nullptr);

    auto f0 = CheckFunction(abc_file, "func_main_0");
    size_t def_class_cnt = f0->GetDefinedClassCount();
    EXPECT_EQ(def_class_cnt, 6U);
    EXPECT_EQ(f0->GetDefinedClassByIndex(0)->GetClassName(), "#~A=#A");
    EXPECT_EQ(f0->GetDefinedClassByIndex(1)->GetClassName(), "#~B=#B");
    EXPECT_EQ(f0->GetDefinedClassByIndex(2)->GetClassName(), "#~C=#C");
    EXPECT_EQ(f0->GetDefinedClassByIndex(3)->GetClassName(), "#~D=#D");

    // check each defined class
    // #~A=#A
    auto class0 = CheckClass(abc_file, "#~A=#A");
    ASSERT_TRUE(class0->GetParentClass() == nullptr);
    ASSERT_TRUE(class0->GetDefiningFunction() == f0);

    // check each defined class
    // #~B=#B
    auto class1 = CheckClass(abc_file, "#~B=#B");
    ASSERT_TRUE(class1->GetParentClass() == nullptr);
    ASSERT_TRUE(class1->GetDefiningFunction() == f0);

    // check each defined class
    // #~C=#C
    auto class2 = CheckClass(abc_file, "#~C=#C");
    ASSERT_TRUE(class2->GetParentClass() == nullptr);
    ASSERT_TRUE(class2->GetDefiningFunction() == f0);
    EXPECT_TRUE(ContainMemberFunction(abc_file, class2, "#~C>#func1"));
    EXPECT_TRUE(ContainMemberFunction(abc_file, class2, "#~C>#func2"));

    // check each defined class
    // #~D=#D
    auto class3 = CheckClass(abc_file, "#~D=#D");
    ASSERT_TRUE(class3->GetParentClass() != nullptr);
    ASSERT_TRUE(class3->GetDefiningFunction() == f0);
    EXPECT_EQ(class3->GetParentClassName(), "#~C=#C");
    EXPECT_TRUE(ContainMemberFunction(abc_file, class3, "#~C>#func1"));
    EXPECT_TRUE(ContainMemberFunction(abc_file, class3, "#~D>#func2"));

    // check each defined class
    // ExtendPhoneService
    auto class4 = CheckClass(abc_file, "#~@0=#ExtendPhoneService");
    ASSERT_TRUE(class4->GetParentClass() == nullptr);
    EXPECT_EQ(class4->GetParentClassName(), "PhoneService");
    EXPECT_EQ(class4->GetParClassExternalModuleName(), "../mod1");
    EXPECT_TRUE(class4->GetParClassGlobalVarName().empty());

    // check each defined class
    // ExtendDataItem
    auto class5 = CheckClass(abc_file, "#~@1=#ExtendDataItem");
    ASSERT_TRUE(class5->GetParentClass() == nullptr);
    EXPECT_EQ(class5->GetParentClassName(), "DataItem");
    EXPECT_TRUE(class5->GetParClassExternalModuleName().empty());
    EXPECT_EQ(class5->GetParClassGlobalVarName(), "globalvar2.Data");
}

HWTEST(DefectScanAuxTest, ECMA2022ClassTest, testing::ext::TestSize.Level0)
{
    std::string test_name = DEFECT_SCAN_AUX_TEST_ABC_DIR "ecma2022_class_test.abc";
    auto abc_file = panda::defect_scan_aux::AbcFile::Open(test_name);
    ASSERT(abc_file != nullptr);

    // check each defined func
    // func_main_0
    auto f0 = CheckFunction(abc_file, "func_main_0");
    ASSERT_TRUE(f0->GetClass() == nullptr);
    ASSERT_TRUE(f0->GetParentFunction() == nullptr);

    size_t def_func_cnt = abc_file->GetDefinedFunctionCount();
    EXPECT_EQ(def_func_cnt, 8U);

    size_t df_cnt0 = f0->GetDefinedFunctionCount();
    EXPECT_EQ(df_cnt0, 7U);
    ASSERT_TRUE(ContainDefinedFunction(abc_file, f0, "#~A1=#A1"));
    ASSERT_TRUE(ContainDefinedFunction(abc_file, f0, "#~A1>#"));
    ASSERT_TRUE(ContainDefinedFunction(abc_file, f0, "#~A1>#C"));
    ASSERT_TRUE(ContainDefinedFunction(abc_file, f0, "#~A1>#^1"));
    ASSERT_TRUE(ContainDefinedFunction(abc_file, f0, "#~A1>#instance_initializer"));
    ASSERT_TRUE(ContainDefinedFunction(abc_file, f0, "#~A5=#A5"));
    ASSERT_TRUE(ContainDefinedFunction(abc_file, f0, "#~A5>#a5"));

    // #~A1=#A1
    auto class1 = CheckClass(abc_file, "#~A1=#A1");
    size_t mf_count1 = class1->GetMemberFunctionCount();
    EXPECT_EQ(mf_count1, 5U);
    EXPECT_TRUE(ContainMemberFunction(abc_file, class1, "#~A1=#A1"));
    // When you use create private property to declare a class method, it will be generated as an anonymous function.
    ASSERT_TRUE(ContainMemberFunction(abc_file, class1, "#~A1>#"));
    EXPECT_TRUE(ContainMemberFunction(abc_file, class1, "#~A1>#C"));
    // When you use create private property to declare a class method, it will be generated as an anonymous function.
    ASSERT_TRUE(ContainMemberFunction(abc_file, class1, "#~A1>#^1"));
    EXPECT_TRUE(ContainMemberFunction(abc_file, class1, "#~A1>#instance_initializer"));

    // #~A5=#A5
    auto class2 = CheckClass(abc_file, "#~A5=#A5");
    size_t mf_count2 = class2->GetMemberFunctionCount();
    EXPECT_EQ(mf_count2, 2U);
    EXPECT_TRUE(ContainMemberFunction(abc_file, class2, "#~A5=#A5"));
    ASSERT_TRUE(ContainMemberFunction(abc_file, class2, "#~A5>#a5"));
}
}  // namespace panda::defect_scan_aux::test
