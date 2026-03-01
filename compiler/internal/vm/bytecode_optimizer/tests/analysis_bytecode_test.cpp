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

#include <cstdio>
#include <gtest/gtest.h>

#include "abc2program/abc2program_driver.h"
#include "assembler/assembly-emitter.h"
#include "bytecode_optimizer/bytecode_analysis_results.h"
#include "bytecode_optimizer/module_constant_analyzer.h"
#include "bytecode_optimizer/optimize_bytecode.h"
#include "mem/pool_manager.h"

using namespace testing::ext;

namespace panda::bytecodeopt {
class AnalysisBytecodeTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp()
    {
        compiler::options.SetCompilerUseSafepoint(false);
        compiler::options.SetCompilerSupportInitObjectInst(true);
    }
    void TearDown() {}
};

/**
 * @tc.name: analysis_bytecode_test_001
 * @tc.desc: Verify the AnalysisBytecode function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AnalysisBytecodeTest, analysis_bytecode_test_001, TestSize.Level1)
{
    const std::string RECORD_NAME = "bytecodeAnalysis";
    const std::string TEST_FILE_NAME = std::string(GRAPH_TEST_ABC_DIR) + RECORD_NAME + ".abc";
    const std::unordered_set<uint32_t> const_slots = {0, 1};
    const std::string SOURCE_RECORD_MYAPP_TEST = "myapp/test";
    const std::string SOURCE_RECORD_NORMALIZED_MYAPP_TEST = "bundle&myapp/test&";
    const std::string ABC_FILE_NAME = "bytecodeAnalysis.unopt.abc";
    const std::string MOCK_CONST_NAME = "i";
    const std::string MOCK_CONST_VALUE = "0";
    constexpr uint32_t MOCK_SLOT = 0;

    bool exists = false;
    auto &result = BytecodeAnalysisResults::GetOrCreateBytecodeAnalysisResult(RECORD_NAME, exists);
    result.SetConstantLocalExportSlots(const_slots);
    EXPECT_FALSE(exists);

    bool ignored;
    auto &source_result =
        BytecodeAnalysisResults::GetOrCreateBytecodeAnalysisResult(SOURCE_RECORD_MYAPP_TEST, ignored);
    ModuleConstantAnalysisResult mock_result;
    ConstantValue mock_value(MOCK_CONST_VALUE);
    mock_result[0] = &mock_value;
    source_result.SetLocalExportInfo(MOCK_SLOT, MOCK_CONST_NAME);
    source_result.SetModuleConstantAnalysisResult(mock_result);

    auto &source_result_normalized =
        BytecodeAnalysisResults::GetOrCreateBytecodeAnalysisResult(SOURCE_RECORD_NORMALIZED_MYAPP_TEST, ignored);
    source_result_normalized.SetLocalExportInfo(MOCK_SLOT, MOCK_CONST_NAME);
    source_result_normalized.SetModuleConstantAnalysisResult(mock_result);

    abc2program::Abc2ProgramDriver driver;
    ASSERT_TRUE(driver.Compile(TEST_FILE_NAME));
    auto &program = driver.GetProgram();
    panda::pandasm::AsmEmitter::PandaFileToPandaAsmMaps panda_file_to_asm_maps {};
    EXPECT_TRUE(panda::pandasm::AsmEmitter::Emit(ABC_FILE_NAME, program, nullptr, &panda_file_to_asm_maps, false));
    EXPECT_TRUE(panda::bytecodeopt::AnalysisBytecode(const_cast<pandasm::Program *>(&program),
                                                     &panda_file_to_asm_maps, ABC_FILE_NAME, true, false));

    ConstantValue analysis_result_value;

    // slot 0: module variable a, Constant(INT32, 1)
    EXPECT_TRUE(BytecodeAnalysisResults::GetLocalExportConstForRecord(RECORD_NAME, 0, analysis_result_value));
    EXPECT_TRUE(analysis_result_value == ConstantValue(static_cast<int32_t>(1)));

    // slot 1: module variable b, Constant(INT32, 2)
    EXPECT_TRUE(BytecodeAnalysisResults::GetLocalExportConstForRecord(RECORD_NAME, 1, analysis_result_value));
    EXPECT_TRUE(analysis_result_value == ConstantValue(static_cast<int32_t>(2)));

    // slot 0: module variable i1, MOCK_CONST_VALUE
    EXPECT_TRUE(BytecodeAnalysisResults::GetRegularImportConstForRecord(RECORD_NAME, 0, analysis_result_value));
    EXPECT_TRUE(analysis_result_value == ConstantValue(MOCK_CONST_VALUE));

    // slot 1: module variable i2, MOCK_CONST_VALUE
    EXPECT_TRUE(BytecodeAnalysisResults::GetRegularImportConstForRecord(RECORD_NAME, 1, analysis_result_value));
    EXPECT_TRUE(analysis_result_value == ConstantValue(MOCK_CONST_VALUE));

    // slot 2: module variable i3, MOCK_CONST_VALUE
    EXPECT_TRUE(BytecodeAnalysisResults::GetRegularImportConstForRecord(RECORD_NAME, 2, analysis_result_value));
    EXPECT_TRUE(analysis_result_value == ConstantValue(MOCK_CONST_VALUE));

    // slot 3: module variable i4, MOCK_CONST_VALUE
    EXPECT_FALSE(BytecodeAnalysisResults::GetRegularImportConstForRecord(RECORD_NAME, 3, analysis_result_value));

    // namespace import m4.i
    EXPECT_TRUE(BytecodeAnalysisResults::GetModuleNamespaceConstForRecord(
        RECORD_NAME, 0, MOCK_CONST_NAME, analysis_result_value));
    EXPECT_TRUE(analysis_result_value == ConstantValue(MOCK_CONST_VALUE));
}
}  // namespace panda::bytecodeopt