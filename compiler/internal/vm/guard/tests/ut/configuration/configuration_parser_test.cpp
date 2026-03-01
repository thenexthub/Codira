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

#include <gtest/gtest.h>

#include "abckit_wrapper/modifiers.h"

#include "configuration/configuration_parser.h"

using namespace testing::ext;

namespace {
const std::string CONFIG_FILE_PATH = GUARD_UNIT_TEST_DIR "ut/configuration";

void CheckKeptPath(const ark::guard::Configuration &configuration, const std::string &path, bool isKept,
                   const std::string &expectKept = "")
{
    std::string keptPath;
    ASSERT_EQ(configuration.IsKeepPath(path, keptPath), isKept)
        << "kept path check failed, path: " << path << " kept: " << keptPath;

    if (isKept && !expectKept.empty()) {
        ASSERT_EQ(keptPath, expectKept);
    }
}
}  // namespace

/**
 * @tc.name: configuration_parser_test_001
 * @tc.desc: verify whether the regular config json file without keep options is successfully parsed.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ConfigParserTest, configuration_parser_test_001, TestSize.Level0)
{
    std::string configFilePath = CONFIG_FILE_PATH + "/" + "configuration_parser_test_001.json";
    ark::guard::ConfigurationParser parser(configFilePath);
    ark::guard::Configuration configuration;
    parser.Parse(configuration);

    ASSERT_EQ(configuration.GetAbcPath(), "xxx");
    ASSERT_EQ(configuration.GetObfAbcPath(), "xxx");
    ASSERT_EQ(configuration.GetDefaultNameCachePath(), "xxx");
    ASSERT_TRUE(configuration.HasKeepConfiguration());

    ASSERT_EQ(configuration.DisableObfuscation(), true);
    ASSERT_EQ(configuration.IsRemoveLogObfEnabled(), true);
    ASSERT_EQ(configuration.GetPrintNameCache(), "xxx");
    ASSERT_EQ(configuration.GetApplyNameCache(), "xxx");

    ASSERT_EQ(configuration.GetPrintSeedsOption().enable, true);
    ASSERT_EQ(configuration.GetPrintSeedsOption().filePath, "xxx");

    ASSERT_EQ(configuration.IsFileNameObfEnabled(), true);
    ASSERT_EQ(configuration.IsKeepFileName("xxx1"), true);
    ASSERT_EQ(configuration.IsKeepFileName("xxx2"), true);
    ASSERT_EQ(configuration.IsKeepFileName("xax"), true);
    ASSERT_EQ(configuration.IsKeepFileName("xAx"), false);
    ASSERT_EQ(configuration.IsKeepFileName("1xx"), true);
    ASSERT_EQ(configuration.IsKeepFileName("zzz"), false);
}

/**
 * @tc.name: configuration_parser_test_002
 * @tc.desc: verify whether the regular config json file with keep options is successfully parsed.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ConfigParserTest, configuration_parser_test_002, TestSize.Level0)
{
    std::string configFilePath = CONFIG_FILE_PATH + "/" + "configuration_parser_test_002.json";
    ark::guard::ConfigurationParser processor(configFilePath);
    ark::guard::Configuration configuration;
    processor.Parse(configuration);

    ASSERT_EQ(configuration.GetAbcPath(), "xxx");
    ASSERT_EQ(configuration.GetObfAbcPath(), "xxx");
    ASSERT_EQ(configuration.GetDefaultNameCachePath(), "xxx");
    ASSERT_TRUE(configuration.HasKeepConfiguration());

    const auto classSpecifications = configuration.GetClassSpecifications();
    ASSERT_EQ(classSpecifications.size(), 3);
    for (const auto &specification : classSpecifications) {
        ASSERT_EQ(specification.annotationName_, "AnnotationA");
        ASSERT_EQ(specification.setAccessFlags_, 0);
        ASSERT_EQ(specification.unSetAccessFlags_, abckit_wrapper::AccessFlags::FINAL);
        ASSERT_EQ(specification.setTypeDeclarations_, abckit_wrapper::TypeDeclarations::CLASS);
        ASSERT_EQ(specification.unSetTypeDeclarations_, 0);
        ASSERT_EQ(specification.className_, "classA");
        ASSERT_EQ(specification.extensionType_, abckit_wrapper::ExtensionType::EXTENDS);
        ASSERT_EQ(specification.extensionAnnotationName_, "AnnotationB");
        ASSERT_EQ(specification.extensionClassName_, "classB");

        ASSERT_EQ(specification.fieldSpecifications_.size(), 1);
        ASSERT_EQ(specification.fieldSpecifications_[0].keepMember_, true);

        ASSERT_EQ(specification.methodSpecifications_.size(), 1);
        ASSERT_EQ(specification.methodSpecifications_[0].keepMember_, true);
    }
}

/**
 * @tc.name: configuration_parser_test_003
 * @tc.desc: verify whether the abnormal config json file is parse failed.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ConfigParserTest, configuration_parser_test_003, TestSize.Level0)
{
    std::vector<std::string> configFilePathList = {
        CONFIG_FILE_PATH + "/" + "configuration_parser_test_003_no_abcPath.json",
        CONFIG_FILE_PATH + "/" + "configuration_parser_test_003_no_obfAbcPath.json",
        CONFIG_FILE_PATH + "/" + "configuration_parser_test_003_no_defaultNameCache.json",
        CONFIG_FILE_PATH + "/" + "configuration_parser_test_003_abcPath_empty.json",
        CONFIG_FILE_PATH + "/" + "configuration_parser_test_003_obfAbcPath_empty.json",
        CONFIG_FILE_PATH + "/" + "configuration_parser_test_003_defaultNameCache_empty.json",
    };
    for (const auto &configFilePath : configFilePathList) {
        ark::guard::ConfigurationParser processor(configFilePath);
        ark::guard::Configuration configuration;
        EXPECT_DEATH(processor.Parse(configuration), "");
    }
}

/**
 * @tc.name: configuration_parser_test_004
 * @tc.desc: verify whether the config json file with required field is successfully parsed.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ConfigParserTest, configuration_parser_test_004, TestSize.Level0)
{
    std::string configFilePath = CONFIG_FILE_PATH + "/" + "configuration_parser_test_004.json";
    ark::guard::ConfigurationParser parser(configFilePath);
    ark::guard::Configuration configuration;
    parser.Parse(configuration);

    ASSERT_EQ(configuration.GetAbcPath(), "xxx");
    ASSERT_EQ(configuration.GetObfAbcPath(), "xxx");
    ASSERT_EQ(configuration.GetDefaultNameCachePath(), "xxx");
    ASSERT_FALSE(configuration.HasKeepConfiguration());

    ASSERT_EQ(configuration.DisableObfuscation(), false);
    ASSERT_EQ(configuration.IsRemoveLogObfEnabled(), false);
    ASSERT_EQ(configuration.GetPrintNameCache(), "");
    ASSERT_EQ(configuration.GetApplyNameCache(), "");

    ASSERT_EQ(configuration.GetPrintSeedsOption().enable, false);
    ASSERT_EQ(configuration.GetPrintSeedsOption().filePath, "");

    ASSERT_EQ(configuration.IsFileNameObfEnabled(), true);
    ASSERT_EQ(configuration.IsKeepFileName("xxx"), false);

    CheckKeptPath(configuration, "xxx.a", false);
}

/**
 * @tc.name: configuration_parser_test_005
 * @tc.desc: verify the config json file with keep path
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ConfigParserTest, configuration_parser_test_005, TestSize.Level0)
{
    std::string configFilePath = CONFIG_FILE_PATH + "/" + "configuration_parser_test_005.json";
    ark::guard::ConfigurationParser parser(configFilePath);
    ark::guard::Configuration configuration;
    parser.Parse(configuration);

    ASSERT_TRUE(configuration.HasKeepConfiguration());

    CheckKeptPath(configuration, "entry.src.main.Main1.a", true);
    CheckKeptPath(configuration, "entry.src.main.Main2.a", true);
    CheckKeptPath(configuration, "entry.src.main.Main3.a", true);
    CheckKeptPath(configuration, "entry.src.main.Main4.a", true);
    CheckKeptPath(configuration, "entry.src.A.B.C.main.Main4.a", true);
    CheckKeptPath(configuration, "entry.src.A.B.C.main.Main6.a", false);
}

/**
 * @tc.name: configuration_parser_test_006
 * @tc.desc: verify the config json file with keep union type field
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ConfigParserTest, configuration_parser_test_006, TestSize.Level0)
{
    std::string configFilePath = CONFIG_FILE_PATH + "/" + "configuration_parser_test_006.json";
    ark::guard::ConfigurationParser parser(configFilePath);
    ark::guard::Configuration configuration;
    parser.Parse(configuration);

    std::unordered_map<std::string, std::string> expectFieldTypeMap {
        {"v1", "{Ustd\\.core\\.Int,std\\.core\\.String}"},
        {"v2", "{Ustd\\.core\\.Byte,std\\.core\\.String}"},
        {"v3", "{Ustd\\.core\\.Short,std\\.core\\.String}"},
        {"v4", "{Ustd\\.core\\.Double,std\\.core\\.String}"},
        {"v5", "{Ustd\\.core\\.Long,std\\.core\\.String}"},
        {"v6", "{Ustd\\.core\\.Float,std\\.core\\.String}"},
        {"v7", "{Ustd\\.core\\.Double,std\\.core\\.String}"},
        {"v8", "{Ustd\\.core\\.Char,std\\.core\\.String}"},
        {"v9", "{Ustd\\.core\\.Boolean,std\\.core\\.String}"},
        {"v10", "{Ustd\\.core\\.BigInt,std\\.core\\.String}"},
        {"v11", "std\\.core\\.String"},
        {"v12", "{Ustd\\.core\\.Null,std\\.core\\.String}"},
        {"v13", "std\\.core\\.String"},
        {"v14", "{Ustd\\.core\\.Array,std\\.core\\.String}"},
        {"v15", "std\\.core\\.Object"},
        {"v16", "std\\.core\\.Int"}};

    const auto classSpecifications = configuration.GetClassSpecifications();
    ASSERT_TRUE(!classSpecifications.empty());

    for (const auto &classSpec : classSpecifications) {
        ASSERT_TRUE(!classSpec.fieldSpecifications_.empty());
        for (const auto &fieldSpec : classSpec.fieldSpecifications_) {
            auto it = expectFieldTypeMap.find(fieldSpec.memberName_);
            ASSERT_TRUE(it != expectFieldTypeMap.end()) << "not find config:" << fieldSpec.memberName_;
            ASSERT_EQ(fieldSpec.memberType_, it->second);
        }
    }
}

/**
 * @tc.name: configuration_parser_test_007
 * @tc.desc: verify the config json file with keep union type method arguments type
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ConfigParserTest, configuration_parser_test_007, TestSize.Level0)
{
    std::string configFilePath = CONFIG_FILE_PATH + "/" + "configuration_parser_test_007.json";
    ark::guard::ConfigurationParser parser(configFilePath);
    ark::guard::Configuration configuration;
    parser.Parse(configuration);

    std::unordered_map<std::string, std::string> expectMethodArgumentTypeMap {
        {"foo1", "{Ustd\\.core\\.Int,std\\.core\\.String}"},
        {"foo2", "{Ustd\\.core\\.Byte,std\\.core\\.String}"},
        {"foo3", "{Ustd\\.core\\.Short,std\\.core\\.String}"},
        {"foo4", "{Ustd\\.core\\.Double,std\\.core\\.String}"},
        {"foo5", "{Ustd\\.core\\.Long,std\\.core\\.String}"},
        {"foo6", "{Ustd\\.core\\.Float,std\\.core\\.String}"},
        {"foo7", "{Ustd\\.core\\.Double,std\\.core\\.String}"},
        {"foo8", "{Ustd\\.core\\.Char,std\\.core\\.String}"},
        {"foo9", "{Ustd\\.core\\.Boolean,std\\.core\\.String}"},
        {"foo10", "{Ustd\\.core\\.BigInt,std\\.core\\.String}"},
        {"foo11", "std\\.core\\.String"},
        {"foo12", "{Ustd\\.core\\.Null,std\\.core\\.String}"},
        {"foo13", "std\\.core\\.String"},
        {"foo14", "{Ustd\\.core\\.Array,std\\.core\\.String}"},
        {"foo15", "std\\.core\\.Object"},
        {"foo16", "std\\.core\\.Int"},
        {"foo17", "std\\.core\\.PromiseLike"},
        {"foo18", "{Ustd\\.core\\.Int,std\\.core\\.PromiseLike}"},
    };

    const auto classSpecifications = configuration.GetClassSpecifications();
    ASSERT_TRUE(!classSpecifications.empty());

    for (const auto &classSpec : classSpecifications) {
        ASSERT_TRUE(!classSpec.methodSpecifications_.empty());
        for (const auto &methodSpec : classSpec.methodSpecifications_) {
            auto it = expectMethodArgumentTypeMap.find(methodSpec.memberName_);
            ASSERT_TRUE(it != expectMethodArgumentTypeMap.end()) << "not find config:" << methodSpec.memberName_;
            ASSERT_EQ(methodSpec.memberType_, it->second);
        }
    }
}

/**
 * @tc.name: configuration_parser_test_008
 * @tc.desc: verify the config json file with keep union type method return type
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ConfigParserTest, configuration_parser_test_008, TestSize.Level0)
{
    std::string configFilePath = CONFIG_FILE_PATH + "/" + "configuration_parser_test_008.json";
    ark::guard::ConfigurationParser parser(configFilePath);
    ark::guard::Configuration configuration;
    parser.Parse(configuration);

    std::unordered_map<std::string, std::string> expectMethodReturnTypeMap {
        {"fooEx1", "{Ustd\\.core\\.Int,std\\.core\\.String}"},
        {"fooEx2", "{Ustd\\.core\\.Byte,std\\.core\\.String}"},
        {"fooEx3", "{Ustd\\.core\\.Short,std\\.core\\.String}"},
        {"fooEx4", "{Ustd\\.core\\.Double,std\\.core\\.String}"},
        {"fooEx5", "{Ustd\\.core\\.Long,std\\.core\\.String}"},
        {"fooEx6", "{Ustd\\.core\\.Float,std\\.core\\.String}"},
        {"fooEx7", "{Ustd\\.core\\.Double,std\\.core\\.String}"},
        {"fooEx8", "{Ustd\\.core\\.Char,std\\.core\\.String}"},
        {"fooEx9", "{Ustd\\.core\\.Boolean,std\\.core\\.String}"},
        {"fooEx10", "{Ustd\\.core\\.BigInt,std\\.core\\.String}"},
        {"fooEx11", "std\\.core\\.String"},
        {"fooEx12", "{Ustd\\.core\\.Null,std\\.core\\.String}"},
        {"fooEx13", "std\\.core\\.String"},
        {"fooEx14", "{Ustd\\.core\\.Array,std\\.core\\.String}"},
        {"fooEx15", "std\\.core\\.Object"},
        {"fooEx16", "std\\.core\\.Int"},
        {"fooEx17", "std\\.core\\.Promise"},
        {"fooEx18", "{Ustd\\.core\\.Promise,void}"}};

    const auto classSpecifications = configuration.GetClassSpecifications();
    ASSERT_TRUE(!classSpecifications.empty());

    for (const auto &classSpec : classSpecifications) {
        ASSERT_TRUE(!classSpec.methodSpecifications_.empty());
        for (const auto &methodSpec : classSpec.methodSpecifications_) {
            auto it = expectMethodReturnTypeMap.find(methodSpec.memberName_);
            ASSERT_TRUE(it != expectMethodReturnTypeMap.end()) << "not find config:" << methodSpec.memberName_;
            ASSERT_EQ(methodSpec.memberValue_, it->second);
        }
    }
}

/**
 * @tc.name: configuration_parser_test_009
 * @tc.desc: verify the config json file with keep union type is not sorted and will remove spaces
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ConfigParserTest, configuration_parser_test_009, TestSize.Level0)
{
    std::string configFilePath = CONFIG_FILE_PATH + "/" + "configuration_parser_test_009.json";
    ark::guard::ConfigurationParser parser(configFilePath);
    ark::guard::Configuration configuration;
    parser.Parse(configuration);

    std::unordered_map<std::string, std::string> expectMap {{"foo1", "{Ustd\\.core\\.String,std\\.core\\.Int}"},
                                                            {"foo2", "{Ustd\\.core\\.Array,std\\.core\\.String}"}};

    const auto classSpecifications = configuration.GetClassSpecifications();
    ASSERT_TRUE(!classSpecifications.empty());

    for (const auto &classSpec : classSpecifications) {
        ASSERT_TRUE(!classSpec.methodSpecifications_.empty());
        for (const auto &methodSpec : classSpec.methodSpecifications_) {
            auto it = expectMap.find(methodSpec.memberName_);
            ASSERT_TRUE(it != expectMap.end()) << "not find config:" << methodSpec.memberName_;
            ASSERT_EQ(methodSpec.memberValue_, it->second);
        }
    }
}

/**
 * @tc.name: configuration_parser_test_010
 * @tc.desc: test file name keep options
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ConfigParserTest, configuration_parser_test_010, TestSize.Level0)
{
    std::string configFilePath = CONFIG_FILE_PATH + "/" + "configuration_parser_test_010.json";
    ark::guard::ConfigurationParser parser(configFilePath);
    ark::guard::Configuration configuration;
    parser.Parse(configuration);

    ASSERT_EQ(configuration.IsKeepFileName("abc.asd.A"), true);
    ASSERT_EQ(configuration.IsKeepFileName("test.file"), true);
    ASSERT_EQ(configuration.IsKeepFileName("abc.asd.B"), false);
    ASSERT_EQ(configuration.IsKeepFileName("abc.xyz.A"), true);
    ASSERT_EQ(configuration.IsKeepFileName("abc.A..A"), false);
    ASSERT_EQ(configuration.IsKeepFileName("test.anything"), true);
    ASSERT_EQ(configuration.IsKeepFileName("test.anything.1"), false);
    ASSERT_EQ(configuration.IsKeepFileName("123.a.b.c.A"), true);
    ASSERT_EQ(configuration.IsKeepFileName("123.a.b.c.B"), false);
    ASSERT_EQ(configuration.IsKeepFileName("zxc.x.A"), true);
    ASSERT_EQ(configuration.IsKeepFileName("zxc.xy.A"), false);
    ASSERT_EQ(configuration.IsKeepFileName(""), false);
    ASSERT_EQ(configuration.IsKeepFileName("."), false);
    ASSERT_EQ(configuration.IsKeepFileName("abc"), false);
    ASSERT_EQ(configuration.IsKeepFileName("abc.asd"), false);
    ASSERT_EQ(configuration.IsKeepFileName("zxc..A"), false);
    ASSERT_EQ(configuration.IsKeepFileName("test/file"), false);
    ASSERT_EQ(configuration.IsKeepFileName("ABC.asd.A"), false);
}

/**
 * @tc.name: configuration_parser_test_011
 * @tc.desc: test file path keep options
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ConfigParserTest, configuration_parser_test_011, TestSize.Level0)
{
    std::string configFilePath = CONFIG_FILE_PATH + "/" + "configuration_parser_test_011.json";
    ark::guard::ConfigurationParser parser(configFilePath);
    ark::guard::Configuration configuration;
    parser.Parse(configuration);

    CheckKeptPath(configuration, "abc.asd.A.file", true, "abc.asd.A.");
    CheckKeptPath(configuration, "test.file.extra", true, "test.file.");
    CheckKeptPath(configuration, "abc.asd.B.file", true, "abc.asd.");
    CheckKeptPath(configuration, "abc.asd", false);
    CheckKeptPath(configuration, "abc.xyz.A.file", true, "abc.xyz.A.");
    CheckKeptPath(configuration, "test.anything.extra", true, "test.anything.");
    CheckKeptPath(configuration, "abc.def.pqr.file", true, "abc.def.pqr.");
    CheckKeptPath(configuration, "abc.a.b.c.zxc.file", true, "abc.a.b.c.zxc.");
    CheckKeptPath(configuration, "abc.x.asd.file", true, "abc.x.asd.");
    CheckKeptPath(configuration, "abc.def.pqr.zxc.file", true, "abc.def.pqr.zxc.");
    CheckKeptPath(configuration, "abc", false);
    CheckKeptPath(configuration, "abc.asd.A", true, "abc.asd.");
    CheckKeptPath(configuration, "abc.xy.asd.file", true, "abc.xy.");
    CheckKeptPath(configuration, "abc.zxc.file", true, "abc.zxc.");
    CheckKeptPath(configuration, "abc.def.pqr.zxc.asd", true, "abc.def.pqr.zxc.");
    CheckKeptPath(configuration, "abc.def.ghi.pqr", true, "abc.def.");
    CheckKeptPath(configuration, "abc.a.b.c.d.e.f.g.zxc.file", true, "abc.a.b.c.d.e.f.g.zxc.");
    CheckKeptPath(configuration, "abc.def", false);
    CheckKeptPath(configuration, "ABC.def.A.file", false);
}

/**
 * @tc.name: configuration_parser_test_012
 * @tc.desc: test file name keep options with empty config
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ConfigParserTest, configuration_parser_test_012, TestSize.Level0)
{
    std::string configFilePath = CONFIG_FILE_PATH + "/" + "configuration_parser_test_012.json";
    ark::guard::ConfigurationParser parser(configFilePath);
    ark::guard::Configuration configuration;
    parser.Parse(configuration);

    ASSERT_EQ(configuration.IsKeepFileName("test.tmp"), true);
}

/**
 * @tc.name: configuration_parser_test_013
 * @tc.desc: test universal keep options of file names and paths
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ConfigParserTest, configuration_parser_test_013, TestSize.Level0)
{
    std::string configFilePath = CONFIG_FILE_PATH + "/" + "configuration_parser_test_013.json";
    ark::guard::ConfigurationParser parser(configFilePath);
    ark::guard::Configuration configuration;
    parser.Parse(configuration);

    ASSERT_EQ(configuration.IsKeepFileName("project.x.123.tmp"), true);
    ASSERT_EQ(configuration.IsKeepFileName("build.any.path.output"), true);
    ASSERT_EQ(configuration.IsKeepFileName("file.tmp"), true);
    ASSERT_EQ(configuration.IsKeepFileName("temp.file"), true);
    ASSERT_EQ(configuration.IsKeepFileName("project.xx.y.tmp"), false);
    ASSERT_EQ(configuration.IsKeepFileName("build..output"), true);
    ASSERT_EQ(configuration.IsKeepFileName("any.thing.tmp"), false);
    ASSERT_EQ(configuration.IsKeepFileName("temp.file.extra"), false);
    ASSERT_EQ(configuration.IsKeepFileName("build.tmp"), true);
    ASSERT_EQ(configuration.IsKeepFileName("project.x.y"), false);
    ASSERT_EQ(configuration.IsKeepFileName("project.@.special.tmp"), true);
    ASSERT_EQ(configuration.IsKeepFileName("build.a.b.c.d.e.f.output"), true);
    ASSERT_EQ(configuration.IsKeepFileName("12345.tmp"), true);
    ASSERT_EQ(configuration.IsKeepFileName("unrelated.file"), false);
    ASSERT_EQ(configuration.IsKeepFileName(""), false);
    ASSERT_EQ(configuration.IsKeepFileName("project...tmp"), false);
    ASSERT_EQ(configuration.IsKeepFileName("project.a.b.c.tmp"), false);
    ASSERT_EQ(configuration.IsKeepFileName("PROJECT.X.Y.TMP"), false);
    ASSERT_EQ(configuration.IsKeepFileName("temp.f!le"), true);

    CheckKeptPath(configuration, "src.a.b.c.include.file.h", true, "src.a.b.c.include.");
    CheckKeptPath(configuration, "root.tmp.file", true, "root.tmp.");
    CheckKeptPath(configuration, "any.path.tmp.file", true, "any.path.tmp.");
    CheckKeptPath(configuration, "temp.any.path.file", true, "temp.any.path.");
    CheckKeptPath(configuration, "src.a.b.include.file", true, "src.a.b.include.");
    CheckKeptPath(configuration, "tmp.file", false);
    CheckKeptPath(configuration, "temp.file", false);
    CheckKeptPath(configuration, "src.a.b.c", false);
    CheckKeptPath(configuration, "temp.src.include.file", true, "temp.src.include.");
    CheckKeptPath(configuration, "src.lib.include.file", false);
    CheckKeptPath(configuration, "a.b.c.tmp.d.e", true, "a.b.c.tmp.");
    CheckKeptPath(configuration, "temp.a.b.c.d.e.f.g.file", true, "temp.a.b.c.d.e.f.g.");
    CheckKeptPath(configuration, "unrelated.path.file", false);
    CheckKeptPath(configuration, "", false);
    CheckKeptPath(configuration, "tmp", false);
    CheckKeptPath(configuration, "src..include.file", false);
    CheckKeptPath(configuration, "temp.src.tmp.include.file", true, "temp.src.tmp.include.");
    CheckKeptPath(configuration, "src.a-b.c_d.include.file", true, "src.a-b.c_d.include.");
    CheckKeptPath(configuration, "TEMP.path.file", false);
    CheckKeptPath(configuration, "src.tmp.file", true, "src.tmp.");
}