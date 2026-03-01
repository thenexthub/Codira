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

#include <filesystem>
#include <iostream>
#include <set>
#include <string>
#include <vector>
#include <array>
#include <cstdlib>
#include <bits/types/FILE.h>
#include <gtest/gtest.h>
#include "graph_test.h"

using namespace testing::ext;

namespace panda::compiler {
class DrawCfgTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp() {}
    void TearDown() {}

    void GenGraphDump(const std::string &abc_file, const std::string &dump_file,
                      const std::set<std::string> &test_method_names)
    {
        std::ofstream dump_out(dump_file);
        options.SetCompilerUseSafepoint(false);
        graph_test_.TestBuildGraphFromFile(
            abc_file,
            [&test_method_names, &dump_out](Graph *graph, std::string &method_name) {
                if (test_method_names.count(method_name) == 0) {
                    return;
                }

                graph->Dump(&dump_out);
            },
            true);
    }

    static bool DrawCfg(const std::string &dump_file, const std::vector<std::string> &args)
    {
        const char *script = DRAW_CFG_TEST_TOOLS_DIR "/draw_cfg.py";
        std::string command(script);
        for (const auto &arg : args) {
            command.push_back(' ');
            command.append(arg);
        }
        command.append(" --out " DRAW_CFG_TEST_OUT_DIR);
        command.append(" < " + dump_file);

        int ret = system(command.c_str());
        return ret == 0;
    }

    static void CleanOutputs()
    {
        std::filesystem::remove_all(DRAW_CFG_TEST_OUT_DIR);
    }

    GraphTest graph_test_;
};

/**
 * @tc.name: draw_cfg_test_001
 * @tc.desc: Verify drawing multiple functions.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DrawCfgTest, draw_cfg_test_001, TestSize.Level1)
{
    const char *abc_file = DRAW_CFG_TEST_ABC_DIR "regallocTest.abc";
    const char *dump_file = DRAW_CFG_TEST_OUT_DIR "regallocTest.dump";
    std::set<std::string> test_methods {
        "func",
        "func2",
        "func3",
        "func4",
        "func_main_0",
    };

    std::filesystem::create_directory(DRAW_CFG_TEST_OUT_DIR);
    GenGraphDump(abc_file, dump_file, test_methods);
    EXPECT_TRUE(std::filesystem::exists(dump_file));

    EXPECT_TRUE(DrawCfg(dump_file, {}));

    for (auto test_method : test_methods) {
        std::string dot_file = DRAW_CFG_TEST_OUT_DIR "cfg_L_GLOBAL;::" + test_method + ".dot";
        std::string png_file = DRAW_CFG_TEST_OUT_DIR "cfg_L_GLOBAL;::" + test_method + ".png";
        EXPECT_TRUE(std::filesystem::exists(dot_file));
        EXPECT_TRUE(std::filesystem::exists(png_file));
    }

    CleanOutputs();
}

/**
 * @tc.name: draw_cfg_test_002
 * @tc.desc: Verify drawing multiple functions without instructions.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DrawCfgTest, draw_cfg_test_002, TestSize.Level1)
{
    const char *abc_file = DRAW_CFG_TEST_ABC_DIR "regallocTest.abc";
    const char *dump_file = DRAW_CFG_TEST_OUT_DIR "regallocTest.dump";
    std::set<std::string> test_methods {
        "func",
        "func2",
        "func3",
        "func4",
        "func_main_0",
    };

    std::filesystem::create_directory(DRAW_CFG_TEST_OUT_DIR);
    GenGraphDump(abc_file, dump_file, test_methods);
    EXPECT_TRUE(std::filesystem::exists(dump_file));

    EXPECT_TRUE(DrawCfg(dump_file, {"--no-insts"}));

    for (auto test_method : test_methods) {
        std::string dot_file = DRAW_CFG_TEST_OUT_DIR "cfg_L_GLOBAL;::" + test_method + ".dot";
        std::string png_file = DRAW_CFG_TEST_OUT_DIR "cfg_L_GLOBAL;::" + test_method + ".png";
        EXPECT_TRUE(std::filesystem::exists(dot_file));
        EXPECT_TRUE(std::filesystem::exists(png_file));
    }

    CleanOutputs();
}
}  // namespace panda::compiler

