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

#include "obfuscator/seeds_dumper.h"

#include <filesystem>
#include <gtest/gtest.h>

#include "obfuscator/obfuscator.h"
#include "obfuscator/obfuscator_data_manager.h"

#include "util/file_util.h"

using namespace testing::ext;

namespace {
constexpr std::string_view ABC_FILE_PATH = GUARD_ABC_FILE_DIR "ut/obfuscator/seeds_dumper_test.abc";
constexpr std::string_view PRINT_SEEDS_FILE_PATH = GUARD_ABC_FILE_DIR "ut/obfuscator/seeds_dumper_test.txt";

abckit_wrapper::FileView fileView;

void SetModuleKept(abckit_wrapper::FileView &fileView, const std::string &name)
{
    const auto module = fileView.GetModule(name);
    ASSERT_TRUE(module.has_value()) << "not valid object, name:" << name;

    ark::guard::ObfuscatorDataManager manager(module.value());
    auto obfuscateData = manager.GetData();
    ASSERT_TRUE(obfuscateData != nullptr);
    obfuscateData->SetKept();
}

void SetObjectKept(abckit_wrapper::FileView &fileView, const std::string &name)
{
    const auto object = fileView.GetObject(name);
    ASSERT_TRUE(object.has_value()) << "not valid object, name:" << name;

    ark::guard::ObfuscatorDataManager manager(object.value());
    auto obfuscateData = manager.GetData();
    ASSERT_TRUE(obfuscateData != nullptr);
    obfuscateData->SetKept();
}
} // namespace

class SeedsDumperTest : public testing::Test {
public:
    static void SetUpTestSuite()
    {
        ASSERT_EQ(fileView.Init(ABC_FILE_PATH), AbckitWrapperErrorCode::ERR_SUCCESS);

        SetModuleKept(fileView, "seeds_dumper_test");
        SetObjectKept(fileView, "seeds_dumper_test.field1");
        SetObjectKept(fileView, "seeds_dumper_test.foo1:void;");
        SetObjectKept(fileView, "seeds_dumper_test.ns1");
        SetObjectKept(fileView, "seeds_dumper_test.ns1.nsField1");
        SetObjectKept(fileView, "seeds_dumper_test.ns1.nsFoo1:void;");
        SetObjectKept(fileView, "seeds_dumper_test.cl1");
        SetObjectKept(fileView, "seeds_dumper_test.cl1.clField1");
        SetObjectKept(fileView, "seeds_dumper_test.cl1.cl1Foo1:seeds_dumper_test.cl1;void;");
        SetObjectKept(fileView, "seeds_dumper_test.anno1");
    }

    std::string GetExpectContent()
    {
        return
            "seeds_dumper_test\n"
            "seeds_dumper_test.cl1\n"
            "seeds_dumper_test.cl1.clField1\n"
            "seeds_dumper_test.cl1.cl1Foo1:seeds_dumper_test.cl1;void;\n"
            "seeds_dumper_test.anno1\n"
            "seeds_dumper_test.foo1:void;\n"
            "seeds_dumper_test.field1\n"
            "seeds_dumper_test.ns1\n"
            "seeds_dumper_test.ns1.nsField1\n"
            "seeds_dumper_test.ns1.nsFoo1:void;\n";
    }
};

/**
 * @tc.name: seeds_dumper_test_001
 * @tc.desc: test whether dump obfuscator keep objects to file is successful
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SeedsDumperTest, seeds_dumper_test_001, TestSize.Level1)
{
    ark::guard::PrintSeedsOption seedsOption{};
    seedsOption.enable = true;
    seedsOption.filePath = PRINT_SEEDS_FILE_PATH;
    ark::guard::SeedsDumper dumper(seedsOption);
    ASSERT_TRUE(dumper.Dump(fileView));

    ASSERT_TRUE(std::filesystem::exists(PRINT_SEEDS_FILE_PATH.data()));

    std::string output = ark::guard::FileUtil::GetFileContent(PRINT_SEEDS_FILE_PATH.data());
    const std::string expected = this->GetExpectContent();
    ASSERT_EQ(output, expected);

    ASSERT_TRUE(std::filesystem::remove(PRINT_SEEDS_FILE_PATH.data()));
}

/**
 * @tc.name: seeds_dumper_test_002
 * @tc.desc: test whether dump obfuscator keep objects to console is successful
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SeedsDumperTest, seeds_dumper_test_002, TestSize.Level1)
{
    testing::internal::CaptureStdout();

    ark::guard::PrintSeedsOption seedsOption{};
    seedsOption.enable = true;
    ark::guard::SeedsDumper dumper(seedsOption);
    ASSERT_TRUE(dumper.Dump(fileView));

    const std::string output = testing::internal::GetCapturedStdout();
    const std::string expected = this->GetExpectContent();
    ASSERT_EQ(output, expected);
}