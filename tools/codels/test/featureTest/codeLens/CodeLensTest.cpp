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

#include "gtest/gtest.h"

#include<string>
#include<thread>
#include<vector>

#include "common.h"
#include "SingleInstance.h"

using namespace test::common;

namespace TestCodeLensTest {
    bool LspCodeLensTest(TestParam param)
    {
        SingleInstance *p = SingleInstance::GetInstance();
        std::string testFile = p->messagePath + "/" + param.testFile;

        std::string rootUri;
        bool isMultiModule = false;

        if (CreateMsg(p->pathIn, testFile, rootUri, isMultiModule) != true) {
            return false;
        }
        if (IsMacroExpandTest(rootUri)) {
            return true;
        }
        if (CreateBuildScript(p->pathBuildScript, testFile)) {
            BuildDynamicBinary(p->pathBuildScript);
        }
        /* Wait until the task is complete. The join blocking mode is not used. */
        StartLspServer(SingleInstance::GetInstance()->useDB);

        /* Check the test case result. */
        std::vector<ark::CodeLens> expect = ReadExpectedCodeLensItems(param.baseFile);
        std::vector<ark::CodeLens> actual = CreateCodeLensStruct(ReadFileById(p->pathOut, param.id));

        /* if case is diff show info */
        std::string reason;
        bool showErr = CheckCodeLensResult(expect, actual, reason);
        if (!showErr) {
            nlohmann::json expLines = ReadExpectedResult(param.baseFile);
            nlohmann::json result = ReadFileById(p->pathOut, param.id);
            std::cout << "the false reason is : " << reason << std::endl;
            ShowDiff(expLines, result, param, p->messagePath);
        }
        return showErr;
    }

    class CodeLensTest : public testing::TestWithParam<struct TestParam> {
    protected:
        void SetUp()
        {
            SetUpConfig("codeLens");
        }
    };

    INSTANTIATE_TEST_SUITE_P(CodeLens, CodeLensTest, testing::ValuesIn(GetTestCaseList("codeLens")));

    TEST_P(CodeLensTest, CodeLensCase)
    {
        TestParam param = GetParam();
        ASSERT_TRUE(LspCodeLensTest(param));
    }
}
