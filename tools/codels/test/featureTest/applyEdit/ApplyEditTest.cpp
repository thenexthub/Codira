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

namespace TestLspApplyEditTest {
bool LspApplyEditTest(TestParam param)
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
    std::thread ThreadObj(StartLspServer, SingleInstance::GetInstance()->useDB);
    ThreadObj.join();

    /* Check the test case result. */
    nlohmann::json expLines = ReadExpectedResult(param.baseFile);
    ChangeApplyEditUrlForBaseFile(testFile, expLines, rootUri, isMultiModule);
    nlohmann::json result = ReadFileByMethod(p->pathOut, param.method);

    /* if case is diff show info */
    std::string info = "none";
    bool showErr = CheckApplyEditResult(expLines, result, info);
    if (!showErr) {
        std::cout << "the false reason is : " << info << std::endl;
        ShowDiff(expLines, result, param, p->messagePath);
    }
    return showErr;
}
} // namespace TestLspApplyEditTest
