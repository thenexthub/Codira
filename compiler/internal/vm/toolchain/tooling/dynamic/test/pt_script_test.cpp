/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "tooling/dynamic/base/pt_script.h"
#include "ecmascript/tests/test_helper.h"

using namespace panda::ecmascript;
using namespace panda::ecmascript::tooling;

namespace panda::test {
class PtScriptTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        GTEST_LOG_(INFO) << "SetUpTestCase";
    }

    static void TearDownTestCase()
    {
        GTEST_LOG_(INFO) << "TearDownCase";
    }

    void SetUp() override
    {
        GTEST_LOG_(INFO) << "SetUp";
    }

    void TearDown() override
    {
        GTEST_LOG_(INFO) << "TearDown";
    }
};

HWTEST_F_L0(PtScriptTest, PtScriptCreateTest)
{
    // Test constructor and initial values
    ScriptId scriptId = 123;
    std::string fileName = "test.js";
    std::string url = "http://example.com/test.js";
    std::string source = "console.log('hello world');";

    PtScript script(scriptId, fileName, url, source);

    // Test initial values
    ASSERT_EQ(script.GetScriptId(), scriptId);
    ASSERT_EQ(script.GetFileName(), fileName);
    ASSERT_EQ(script.GetUrl(), url);
    ASSERT_EQ(script.GetScriptSource(), source);
    
    // Test default values
    ASSERT_EQ(script.GetHash(), "");
    ASSERT_EQ(script.GetSourceMapUrl(), "");
    ASSERT_EQ(script.GetEndLine(), 0);
    ASSERT_TRUE(script.GetLocations().empty());
}

HWTEST_F_L0(PtScriptTest, PtScriptScriptIdTest)
{
    // Create script with initial values
    PtScript script(0, "initial.js", "http://initial.com", "initial source");

    // Test SetScriptId and GetScriptId
    ScriptId newScriptId = 456;
    script.SetScriptId(newScriptId);
    ASSERT_EQ(script.GetScriptId(), newScriptId);
}

HWTEST_F_L0(PtScriptTest, PtScriptFileNameTest)
{
    // Create script with initial values
    PtScript script(0, "initial.js", "http://initial.com", "initial source");
    // Test SetFileName and GetFileName
    std::string newFileName = "new_file.js";
    script.SetFileName(newFileName);
    ASSERT_EQ(script.GetFileName(), newFileName);
}

HWTEST_F_L0(PtScriptTest, PtScriptUrlTest)
{
    // Create script with initial values
    PtScript script(0, "initial.js", "http://initial.com", "initial source");
    // Test SetUrl and GetUrl
    std::string newUrl = "http://newurl.com/new_file.js";
    script.SetUrl(newUrl);
    ASSERT_EQ(script.GetUrl(), newUrl);
}

HWTEST_F_L0(PtScriptTest, PtScriptHashTest)
{
    // Create script with initial values
    PtScript script(0, "initial.js", "http://initial.com", "initial source");
    // Test SetHash and GetHash
    std::string newHash = "abc123def456";
    script.SetHash(newHash);
    ASSERT_EQ(script.GetHash(), newHash);
}

HWTEST_F_L0(PtScriptTest, PtScriptScriptSourceTest)
{
    // Create script with initial values
    PtScript script(0, "initial.js", "http://initial.com", "initial source");
    // Test SetScriptSource and GetScriptSource
    std::string newSource = "console.log('new source');";
    script.SetScriptSource(newSource);
    ASSERT_EQ(script.GetScriptSource(), newSource);
}

HWTEST_F_L0(PtScriptTest, PtScriptSourceMapUrlTest)
{
    // Create script with initial values
    PtScript script(0, "initial.js", "http://initial.com", "initial source");
    // Test SetSourceMapUrl and GetSourceMapUrl
    std::string newSourceMapUrl = "http://example.com/source.map";
    script.SetSourceMapUrl(newSourceMapUrl);
    ASSERT_EQ(script.GetSourceMapUrl(), newSourceMapUrl);
}

HWTEST_F_L0(PtScriptTest, PtScriptEndLineTest)
{
    // Create script with initial values
    PtScript script(0, "initial.js", "http://initial.com", "initial source");
    // Test SetEndLine and GetEndLine
    int32_t newEndLine = 100;
    script.SetEndLine(newEndLine);
    ASSERT_EQ(script.GetEndLine(), newEndLine);
}

HWTEST_F_L0(PtScriptTest, PtScriptLocationsTest)
{
    // Create script with initial values
    PtScript script(0, "initial.js", "http://initial.com", "initial source");
    // Test SetLocations and GetLocations
    std::vector<std::shared_ptr<BreakpointReturnInfo>> locations;
    auto location1 = std::make_shared<BreakpointReturnInfo>();
    auto location2 = std::make_shared<BreakpointReturnInfo>();
    locations.push_back(location1);
    locations.push_back(location2);
    
    script.SetLocations(locations);
    auto resultLocations = script.GetLocations();
    ASSERT_EQ(resultLocations.size(), 2);
}
}  // namespace panda::test