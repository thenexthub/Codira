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

#include <iostream>
#include <string>

#include "gtest/gtest.h"
#include "TestCompilerInstance.h"
#include "Codira/ConditionalCompilation/ConditionalCompilation.h"

using namespace Codira;

class ConditionalCompilationTest : public ::testing::Test {
protected:
    void SetUp() override
    {
#ifdef _WIN32
        srcPath = projectPath + "\\unittests\\ConditionalCompilation\\srcFiles\\";
#else
        srcPath = projectPath + "/unittests/ConditionalCompilation/srcFiles/";
#endif
    }

#ifdef PROJECT_SOURCE_DIR
    // Gets the absolute path of the project from the compile parameter.
    std::string projectPath = PROJECT_SOURCE_DIR;
#else
    // Just in case, give it a default value.
    // Assume the initial is in the build directory.
    std::string projectPath = "..";
#endif
    std::string srcPath;
    DiagnosticEngine diag;
    CompilerInvocation invocation;
    std::unique_ptr<TestCompilerInstance> instance;
};

TEST_F(ConditionalCompilationTest, for_lsp)
{
    auto src = srcPath + "os.code";
    instance = std::make_unique<TestCompilerInstance>(invocation, diag);
    instance->compileOnePackageFromSrcFiles = true;
    invocation.globalOptions.enableMacroInLSP = true;
    instance->srcFilePaths = {src};
    instance->Compile(CompileStage::PARSE);
    instance->PerformConditionCompile();
}

TEST_F(ConditionalCompilationTest, passedCondition_for_lsp)
{
    auto src = srcPath + "os.code";
    instance = std::make_unique<TestCompilerInstance>(invocation, diag);
    instance->compileOnePackageFromSrcFiles = true;
    invocation.globalOptions.enableMacroInLSP = true;
    invocation.globalOptions.passedWhenKeyValue.insert({"test1", "abc"});
    invocation.globalOptions.passedWhenKeyValue.insert({"test2", "aaa"});
    instance->srcFilePaths = {src};
    instance->Compile(CompileStage::PARSE);
    instance->PerformConditionCompile();
}

TEST_F(ConditionalCompilationTest, passedCondition_cfgFile_for_lsp)
{
    auto src = srcPath + "os.code";
    instance = std::make_unique<TestCompilerInstance>(invocation, diag);
    instance->compileOnePackageFromSrcFiles = true;
    invocation.globalOptions.enableMacroInLSP = true;
    invocation.globalOptions.passedWhenCfgPaths.emplace_back(srcPath);
    instance->srcFilePaths = {src};
    instance->Compile(CompileStage::PARSE);
    instance->PerformConditionCompile();
}

TEST_F(ConditionalCompilationTest, packagePaths_for_lsp)
{
    auto src = srcPath + "os.code";
    instance = std::make_unique<TestCompilerInstance>(invocation, diag);
    instance->compileOnePackageFromSrcFiles = true;
    invocation.globalOptions.enableMacroInLSP = true;
    invocation.globalOptions.packagePaths.emplace_back(srcPath);

    instance->srcFilePaths = {src};
    instance->Compile(CompileStage::PARSE);
    instance->PerformConditionCompile();
}

#ifndef _WIN32
TEST_F(ConditionalCompilationTest, cfgPaths_no_file_for_lsp)
{
    auto src = srcPath + "os.code";
    instance = std::make_unique<TestCompilerInstance>(invocation, diag);
    instance->compileOnePackageFromSrcFiles = true;
    invocation.globalOptions.enableMacroInLSP = true;
    invocation.globalOptions.passedWhenCfgPaths.emplace_back("srcPath");

    instance->srcFilePaths = {src};
    instance->Compile(CompileStage::PARSE);
    instance->PerformConditionCompile();

    EXPECT_EQ(diag.GetWarningCount(), 1);
}

TEST_F(ConditionalCompilationTest, same_with_builtin_for_lsp)
{
    auto src = srcPath + "os.code";
    instance = std::make_unique<TestCompilerInstance>(invocation, diag);
    instance->compileOnePackageFromSrcFiles = true;
    invocation.globalOptions.enableMacroInLSP = true;
    invocation.globalOptions.passedWhenCfgPaths.emplace_back("srcPath");
    invocation.globalOptions.passedWhenKeyValue.insert({"os", "aaa"});
    instance->srcFilePaths = {src};
    instance->Compile(CompileStage::PARSE);
    instance->PerformConditionCompile();

    EXPECT_EQ(diag.GetErrorCount(), 1);
}

TEST_F(ConditionalCompilationTest, cfg_path_ignored_for_lsp)
{
    auto src = srcPath + "os.code";
    instance = std::make_unique<TestCompilerInstance>(invocation, diag);
    instance->compileOnePackageFromSrcFiles = true;
    invocation.globalOptions.enableMacroInLSP = true;
    invocation.globalOptions.passedWhenCfgPaths.emplace_back("srcPath");
    invocation.globalOptions.passedWhenKeyValue.insert({"test1", "abc"});
    invocation.globalOptions.passedWhenKeyValue.insert({"test2", "aaa"});
    instance->srcFilePaths = {src};
    instance->Compile(CompileStage::PARSE);
    instance->PerformConditionCompile();

    EXPECT_EQ(diag.GetWarningCount(), 1);
}
#endif
