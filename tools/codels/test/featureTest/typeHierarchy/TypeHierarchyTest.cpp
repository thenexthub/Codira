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

namespace TestLspTypeHierarchy {
    bool LspTypeHierarchyTest(TestParam param)
    {
        SingleInstance *p = SingleInstance::GetInstance();
        std::string testFile = p->messagePath + "/" + param.testFile;

        std::string rootUri;
        bool isMultiModule = false;

        if (!CreateMsg(p->pathIn, testFile, rootUri, isMultiModule)) {
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
        TypeHierarchyResult expectPrepare;
        ReadExpectedTypeHierarchyResult(param.baseFile, expectPrepare);
        TypeHierarchyResult actualPrepare;
        CreatePrepareTypeHierarchyStruct(ReadFileById(p->pathOut, param.preId),actualPrepare);

        if (!CreateMsg(p->pathIn, testFile, rootUri, isMultiModule, std::to_string(actualPrepare.symbolId))) {
            return false;
        }
        StartLspServer(SingleInstance::GetInstance()->useDB);

        TypeHierarchyResult expect;
        ReadExpectedTypeHierarchyResult(param.baseFile, expect);
        TypeHierarchyResult actual;
        CreateTypeHierarchyStruct(ReadFileById(p->pathOut, param.id), actual);

        /* if case is diff show info */
        std::string reason;
        bool showErr = CheckTypeHierarchyResult(actual, expect, reason);
        if (!showErr) {
            nlohmann::json expLines = ReadExpectedResult(param.baseFile);
            nlohmann::json result = ReadFileById(p->pathOut, param.id);
            std::cout << "the false reason is : " << reason << std::endl;
            ShowDiff(expLines, result, param, p->messagePath);
        }
        return showErr;
    }

    class TypeHierarchyTest : public testing::TestWithParam<struct TestParam> {
    protected:
        void SetUp()
        {
            SetUpConfig("typeHierarchy");
        }
    };

    INSTANTIATE_TEST_SUITE_P(TypeHierarchy, TypeHierarchyTest,
                             testing::ValuesIn(GetTestCaseList("typeHierarchy")));

    TEST_P(TypeHierarchyTest, TypeHierarchyCase)
    {
        TestParam param = GetParam();
        ASSERT_TRUE(LspTypeHierarchyTest(param));
    }
}
