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

/**
 * @file
 *
 * Generic instantiation related unit tests.
 */

#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

#include "gtest/gtest.h"

#define private public
#include "TestCompilerInstance.h"
#include "Codira/AST/Match.h"

using namespace Codira;
using namespace AST;

class GenericTest : public testing::Test {
protected:
    void SetUp() override
    {
#ifdef _WIN32
        std::string command;
        int err = 0;
        if (FileUtil::FileExist("testTempFiles")) {
            std::string command = "rmdir /s /q testTempFiles";
            int err = system(command.c_str());
            std::cout << err;
        }
        command = "mkdir testTempFiles";
        err = system(command.c_str());
        ASSERT_EQ(0, err);

        srcPath = projectPath + "\\tests\\LLT\\Sema\\generics\\";
        packagePath = packagePath + "\\";
#else
        std::string command;
        int err = 0;
        if (FileUtil::FileExist("testTempFiles")) {
            command = "rm -rf testTempFiles";
            err = system(command.c_str());
            ASSERT_EQ(0, err);
        }
        command = "mkdir -p testTempFiles";
        err = system(command.c_str());
        ASSERT_EQ(0, err);
        srcPath = projectPath + "/tests/LLT/Sema/generics/";
        packagePath = packagePath + "/";
#endif
#ifdef __x86_64__
        invocation.globalOptions.target.arch = Codira::Triple::ArchType::X86_64;
#else
        invocation.globalOptions.target.arch = Codira::Triple::ArchType::AARCH64;
#endif
#ifdef _WIN32
        invocation.globalOptions.target.os = Codira::Triple::OSType::WINDOWS;
#elif __unix__
        invocation.globalOptions.target.os = Codira::Triple::OSType::LINUX;
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
    std::string packagePath = "testTempFiles";
    std::string srcPath;
    DiagnosticEngine diag;
    CompilerInvocation invocation;
    std::unique_ptr<TestCompilerInstance> instance;
};
