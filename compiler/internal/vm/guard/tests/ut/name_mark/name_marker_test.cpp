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

#include <filesystem>

#include "configuration/configuration_parser.h"
#include "obfuscator/name_marker.h"
#include "obfuscator/obfuscator_data_manager.h"

using namespace testing::ext;

namespace {
void CheckModuleKept(abckit_wrapper::FileView &fileView, const std::string &name, bool expected)
{
    const auto module = fileView.GetModule(name);
    ASSERT_TRUE(module.has_value()) << "not valid module, name:" << name;

    ark::guard::ObfuscatorDataManager manager(module.value());
    auto obfuscateData = manager.GetData();
    ASSERT_TRUE(obfuscateData != nullptr);
    const auto isKept = obfuscateData->IsKept();
    ASSERT_EQ(isKept, expected) << name << " isKept not match, expect:" << expected << ", but actual is:" << isKept;
}

void CheckObjectKept(abckit_wrapper::FileView &fileView, const std::string &name, bool expected)
{
    const auto object = fileView.GetObject(name);
    ASSERT_TRUE(object.has_value()) << "not valid object, name:" << name;

    ark::guard::ObfuscatorDataManager manager(object.value());
    auto obfuscateData = manager.GetData();
    ASSERT_TRUE(obfuscateData != nullptr);
    const auto isKept = obfuscateData->IsKept();
    ASSERT_EQ(isKept, expected) << name << " isKept not match, expect:" << expected << ", but actual is:" << isKept;
}

void AssertKept(abckit_wrapper::FileView &fileView, const std::string &name)
{
    CheckObjectKept(fileView, name, true);
}

void AssertNotKept(abckit_wrapper::FileView &fileView, const std::string &name)
{
    CheckObjectKept(fileView, name, false);
}

void AssertModuleKept(abckit_wrapper::FileView &fileView, const std::string &name)
{
    CheckModuleKept(fileView, name, true);
}

void AssertModuleNotKept(abckit_wrapper::FileView &fileView, const std::string &name)
{
    CheckModuleKept(fileView, name, false);
}

void NameMarkerTestExecute(const std::string &abcFilePath, const std::string &configFilePath,
                           abckit_wrapper::FileView &fileView)
{
    ark::guard::Configuration configuration;

    ark::guard::ConfigurationParser configParser(configFilePath);
    configParser.Parse(configuration);

    ASSERT_EQ(fileView.Init(abcFilePath), AbckitWrapperErrorCode::ERR_SUCCESS);

    ark::guard::NameMarker nameMarker(configuration);
    nameMarker.Execute(fileView);
}
}  // namespace

/**
 * @tc.name: name_marker_class_test_001
 * @tc.desc: test name marker module class
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(NameMarkerTest, name_marker_class_test_001, TestSize.Level1)
{
    const std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/name_mark/code_sample/class_test_001.abc";
    const std::string configFilePath = GUARD_UNIT_TEST_DIR "ut/name_mark/code_sample/class_test_001_config.json";
    abckit_wrapper::FileView fileView;
    NameMarkerTestExecute(abcFilePath, configFilePath, fileView);

    AssertModuleNotKept(fileView, "class_test_001");

    AssertKept(fileView, "class_test_001.ClassA");
    AssertKept(fileView, "class_test_001.ClassC");
    AssertKept(fileView, "class_test_001.ClassE");

    AssertNotKept(fileView, "class_test_001.ClassB");
    AssertNotKept(fileView, "class_test_001.ClassD");
    AssertNotKept(fileView, "class_test_001.ClassF");
    AssertNotKept(fileView, "class_test_001.ClassG");
}

/**
 * @tc.name: name_marker_class_test_002
 * @tc.desc: test name marker module class method and with annotation
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(NameMarkerTest, name_marker_class_test_002, TestSize.Level1)
{
    const std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/name_mark/code_sample/class_test_002.abc";
    const std::string configFilePath = GUARD_UNIT_TEST_DIR "ut/name_mark/code_sample/class_test_002_config.json";
    abckit_wrapper::FileView fileView;
    NameMarkerTestExecute(abcFilePath, configFilePath, fileView);

    AssertModuleNotKept(fileView, "class_test_002");

    AssertKept(fileView, "class_test_002.ClassA");
    AssertKept(fileView, "class_test_002.ClassA.method0:class_test_002.ClassA;void;");
    AssertKept(fileView, "class_test_002.ClassA.method1:class_test_002.ClassA;void;");

    AssertKept(fileView, "class_test_002.ClassB");
    AssertKept(fileView, "class_test_002.ClassB.method0:class_test_002.ClassB;void;");
    AssertNotKept(fileView, "class_test_002.ClassB.method1:class_test_002.ClassB;void;");

    AssertKept(fileView, "class_test_002.ClassC");
    AssertKept(fileView, "class_test_002.ClassC.method0:class_test_002.ClassC;void;");
    AssertNotKept(fileView, "class_test_002.ClassC.method1:class_test_002.ClassC;void;");

    AssertKept(fileView, "class_test_002.ClassD");
    AssertKept(fileView, "class_test_002.ClassD.method0:class_test_002.ClassD;void;");
    AssertNotKept(fileView, "class_test_002.ClassD.method1:class_test_002.ClassD;void;");

    AssertKept(fileView, "class_test_002.ClassE");
    AssertNotKept(fileView, "class_test_002.ClassE.method0:class_test_002.ClassE;void;");
    AssertNotKept(fileView, "class_test_002.ClassE.method1:class_test_002.ClassE;void;");
    AssertKept(fileView, "class_test_002.ClassE.method2:class_test_002.ClassE;void;");

    AssertKept(fileView, "class_test_002.ClassF");
    AssertNotKept(fileView, "class_test_002.ClassF.method0:class_test_002.ClassF;void;");
    AssertKept(fileView, "class_test_002.ClassF.method1:class_test_002.ClassF;void;");
    AssertNotKept(fileView, "class_test_002.ClassF.method2:class_test_002.ClassF;void;");
}

/**
 * @tc.name: name_marker_class_test_003
 * @tc.desc: test name marker module class field with annotation
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(NameMarkerTest, name_marker_class_test_003, TestSize.Level1)
{
    const std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/name_mark/code_sample/class_test_003.abc";
    const std::string configFilePath = GUARD_UNIT_TEST_DIR "ut/name_mark/code_sample/class_test_003_config.json";
    abckit_wrapper::FileView fileView;
    NameMarkerTestExecute(abcFilePath, configFilePath, fileView);

    AssertModuleNotKept(fileView, "class_test_003");

    AssertKept(fileView, "class_test_003.ClassA");
    AssertKept(fileView, "class_test_003.ClassA.field1");

    AssertKept(fileView, "class_test_003.ClassB");
    AssertKept(fileView, "class_test_003.ClassB.field1");
    AssertNotKept(fileView, "class_test_003.ClassB.field2");

    AssertKept(fileView, "class_test_003.ClassC");
    AssertKept(fileView, "class_test_003.ClassC.field1");
    AssertNotKept(fileView, "class_test_003.ClassC.field2");

    AssertKept(fileView, "class_test_003.ClassD");
    AssertKept(fileView, "class_test_003.ClassD.field1");

    AssertKept(fileView, "class_test_003.ClassE");
    AssertKept(fileView, "class_test_003.ClassE.field1");
    AssertNotKept(fileView, "class_test_003.ClassE.field2");

    AssertKept(fileView, "class_test_003.ClassF");
    AssertKept(fileView, "class_test_003.ClassF.field1");
    AssertNotKept(fileView, "class_test_003.ClassF.field2");

    AssertKept(fileView, "class_test_003.ClassF");
    AssertKept(fileView, "class_test_003.ClassF.field1");
    AssertNotKept(fileView, "class_test_003.ClassF.field2");

    AssertKept(fileView, "class_test_003.ClassG");
    AssertNotKept(fileView, "class_test_003.ClassG.field1");
    AssertNotKept(fileView, "class_test_003.ClassG.field2");
    AssertKept(fileView, "class_test_003.ClassG.field3");
    AssertNotKept(fileView, "class_test_003.ClassG.field4");

    AssertKept(fileView, "class_test_003.ClassH");
    AssertNotKept(fileView, "class_test_003.ClassH.field1");
    AssertNotKept(fileView, "class_test_003.ClassH.field2");
    AssertKept(fileView, "class_test_003.ClassH.field3");
    AssertNotKept(fileView, "class_test_003.ClassH.field4");

    AssertKept(fileView, "class_test_003.ClassI");
    AssertNotKept(fileView, "class_test_003.ClassI.field1");
    AssertNotKept(fileView, "class_test_003.ClassI.field2");
    AssertNotKept(fileView, "class_test_003.ClassI.field3");
    AssertKept(fileView, "class_test_003.ClassI.field4");

    AssertKept(fileView, "class_test_003.ClassJ");
    AssertNotKept(fileView, "class_test_003.ClassJ.field1");
    AssertNotKept(fileView, "class_test_003.ClassJ.field2");
    AssertNotKept(fileView, "class_test_003.ClassJ.field3");
    AssertKept(fileView, "class_test_003.ClassJ.field4");
}

/**
 * @tc.name: name_marker_class_test_004
 * @tc.desc: test name marker namespace class
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(NameMarkerTest, name_marker_class_test_004, TestSize.Level1)
{
    const std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/name_mark/code_sample/class_test_004.abc";
    const std::string configFilePath = GUARD_UNIT_TEST_DIR "ut/name_mark/code_sample/class_test_004_config.json";
    abckit_wrapper::FileView fileView;
    NameMarkerTestExecute(abcFilePath, configFilePath, fileView);

    AssertModuleNotKept(fileView, "class_test_004");

    AssertKept(fileView, "class_test_004.Ns1.ClassA");
    AssertKept(fileView, "class_test_004.Ns1.ClassC");
    AssertKept(fileView, "class_test_004.Ns1.ClassE");

    AssertNotKept(fileView, "class_test_004.Ns1.ClassB");
    AssertNotKept(fileView, "class_test_004.Ns1.ClassD");
    AssertNotKept(fileView, "class_test_004.Ns1.ClassF");
    AssertNotKept(fileView, "class_test_004.Ns1.ClassG");
}

/**
 * @tc.name: name_marker_class_test_005
 * @tc.desc: test name marker namespace class method and with annotation
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(NameMarkerTest, name_marker_class_test_005, TestSize.Level1)
{
    const std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/name_mark/code_sample/class_test_005.abc";
    const std::string configFilePath = GUARD_UNIT_TEST_DIR "ut/name_mark/code_sample/class_test_005_config.json";
    abckit_wrapper::FileView fileView;
    NameMarkerTestExecute(abcFilePath, configFilePath, fileView);

    AssertModuleNotKept(fileView, "class_test_005");

    AssertKept(fileView, "class_test_005.Ns1.ClassA");
    AssertKept(fileView, "class_test_005.Ns1.ClassA.method0:class_test_005.Ns1.ClassA;void;");
    AssertKept(fileView, "class_test_005.Ns1.ClassA.method1:class_test_005.Ns1.ClassA;void;");

    AssertKept(fileView, "class_test_005.Ns1.ClassB");
    AssertKept(fileView, "class_test_005.Ns1.ClassB.method0:class_test_005.Ns1.ClassB;void;");
    AssertNotKept(fileView, "class_test_005.Ns1.ClassB.method1:class_test_005.Ns1.ClassB;void;");

    AssertKept(fileView, "class_test_005.Ns1.ClassC");
    AssertKept(fileView, "class_test_005.Ns1.ClassC.method0:class_test_005.Ns1.ClassC;void;");
    AssertNotKept(fileView, "class_test_005.Ns1.ClassC.method1:class_test_005.Ns1.ClassC;void;");
}

/**
 * @tc.name: name_marker_class_test_006
 * @tc.desc: test name marker namespace class field with annotation
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(NameMarkerTest, name_marker_class_test_006, TestSize.Level1)
{
    const std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/name_mark/code_sample/class_test_006.abc";
    const std::string configFilePath = GUARD_UNIT_TEST_DIR "ut/name_mark/code_sample/class_test_006_config.json";
    abckit_wrapper::FileView fileView;
    NameMarkerTestExecute(abcFilePath, configFilePath, fileView);

    AssertModuleNotKept(fileView, "class_test_006");

    AssertKept(fileView, "class_test_006.Ns1.ClassA");
    AssertKept(fileView, "class_test_006.Ns1.ClassA.field1");
    AssertKept(fileView, "class_test_006.Ns1.ClassA.field1");

    AssertKept(fileView, "class_test_006.Ns1.ClassB");
    AssertKept(fileView, "class_test_006.Ns1.ClassB.field1");
    AssertNotKept(fileView, "class_test_006.Ns1.ClassB.field2");

    AssertKept(fileView, "class_test_006.Ns1.ClassC");
    AssertKept(fileView, "class_test_006.Ns1.ClassC.field1");
    AssertNotKept(fileView, "class_test_006.Ns1.ClassC.field2");
}

/**
 * @tc.name: name_marker_class_test_007
 * @tc.desc: test name marker keep declare type is class and not class
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(NameMarkerTest, name_marker_class_test_007, TestSize.Level1)
{
    const std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/name_mark/code_sample/class_test_007.abc";
    const std::string configFilePath = GUARD_UNIT_TEST_DIR "ut/name_mark/code_sample/class_test_007_config.json";
    abckit_wrapper::FileView fileView;
    NameMarkerTestExecute(abcFilePath, configFilePath, fileView);

    AssertModuleNotKept(fileView, "class_test_007");

    AssertKept(fileView, "class_test_007.ClassA");
    AssertKept(fileView, "class_test_007.ClassA.field1");
    AssertKept(fileView, "class_test_007.ClassA.method1:class_test_007.ClassA;void;");

    AssertKept(fileView, "class_test_007.Ns1.ClassB");
    AssertKept(fileView, "class_test_007.Ns1.ClassB.field2");
    AssertKept(fileView, "class_test_007.Ns1.ClassB.method2:class_test_007.Ns1.ClassB;void;");
}

/**
 * @tc.name: name_marker_class_test_008
 * @tc.desc: test name marker class field with specified type, type may also be a union type
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(NameMarkerTest, name_marker_class_test_008, TestSize.Level1)
{
    const std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/name_mark/code_sample/class_test_008.abc";
    const std::string configFilePath = GUARD_UNIT_TEST_DIR "ut/name_mark/code_sample/class_test_008_config.json";
    abckit_wrapper::FileView fileView;
    NameMarkerTestExecute(abcFilePath, configFilePath, fileView);

    AssertModuleNotKept(fileView, "class_test_008");

    AssertKept(fileView, "class_test_008.ClassA");
    AssertKept(fileView, "class_test_008.ClassA.field1");
    AssertKept(fileView, "class_test_008.ClassA.field2");

    AssertKept(fileView, "class_test_008.ClassB");
    AssertNotKept(fileView, "class_test_008.ClassB.field1");
    AssertKept(fileView, "class_test_008.ClassB.field2");
}

/**
 * @tc.name: name_marker_class_test_009
 * @tc.desc: test name marker class method with specified type, type may also be a union type
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(NameMarkerTest, name_marker_class_test_009, TestSize.Level1)
{
    const std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/name_mark/code_sample/class_test_009.abc";
    const std::string configFilePath = GUARD_UNIT_TEST_DIR "ut/name_mark/code_sample/class_test_009_config.json";
    abckit_wrapper::FileView fileView;
    NameMarkerTestExecute(abcFilePath, configFilePath, fileView);

    AssertModuleNotKept(fileView, "class_test_009");
    AssertNotKept(fileView, "class_test_009.ClassA");

    AssertKept(fileView,
               "class_test_009.ClassC.method1:class_test_009.ClassC;i32;class_test_009.ClassA;class_test_009.ClassB;");
    AssertKept(fileView,
               "class_test_009.ClassC.method2:class_test_009.ClassC;i32;{Uclass_test_009.ClassA,std.core.String};{"
               "Uclass_test_009.ClassB,std.core.String};");
    AssertKept(fileView,
               "class_test_009.ClassC.method3:class_test_009.ClassC;i32;std.core.Array;class_test_009.ClassB;");
}

/**
 * @tc.name: name_marker_class_test_010
 * @tc.desc: test name marker class, method and field name with wildcard
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(NameMarkerTest, name_marker_class_test_010, TestSize.Level1)
{
    const std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/name_mark/code_sample/class_test_010.abc";
    const std::string configFilePath = GUARD_UNIT_TEST_DIR "ut/name_mark/code_sample/class_test_010_config.json";
    abckit_wrapper::FileView fileView;
    NameMarkerTestExecute(abcFilePath, configFilePath, fileView);

    AssertModuleNotKept(fileView, "class_test_010");

    AssertKept(fileView, "class_test_010.ClassA");
    AssertKept(fileView, "class_test_010.TestB");
    AssertKept(fileView, "class_test_010.HelloC");
    AssertKept(fileView, "class_test_010.BarFooBar");

    AssertKept(fileView, "class_test_010.Dog1");
    AssertKept(fileView, "class_test_010.Dog1.method:class_test_010.Dog1;void;");

    AssertKept(fileView, "class_test_010.Dog2");
    AssertKept(fileView, "class_test_010.Dog2.helloWorld:class_test_010.Dog2;void;");

    AssertKept(fileView, "class_test_010.Dog3");
    AssertKept(fileView, "class_test_010.Dog3.test:class_test_010.Dog3;void;");

    AssertKept(fileView, "class_test_010.Dog4");
    AssertKept(fileView, "class_test_010.Dog4.timeFooTime:class_test_010.Dog4;void;");

    AssertKept(fileView, "class_test_010.Cat1");
    AssertKept(fileView, "class_test_010.Cat1.method");

    AssertKept(fileView, "class_test_010.Cat2");
    AssertKept(fileView, "class_test_010.Cat2.helloWorld");

    AssertKept(fileView, "class_test_010.Cat3");
    AssertKept(fileView, "class_test_010.Cat3.test");

    AssertKept(fileView, "class_test_010.Cat4");
    AssertKept(fileView, "class_test_010.Cat4.timeFooTime");
}

/**
 * @tc.name: name_marker_class_test_011
 * @tc.desc: test name marker method arguments type, method return type and field type with wildcard
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(NameMarkerTest, name_marker_class_test_011, TestSize.Level1)
{
    const std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/name_mark/code_sample/class_test_011.abc";
    const std::string configFilePath = GUARD_UNIT_TEST_DIR "ut/name_mark/code_sample/class_test_011_config.json";
    abckit_wrapper::FileView fileView;
    NameMarkerTestExecute(abcFilePath, configFilePath, fileView);

    AssertModuleNotKept(fileView, "class_test_011");

    AssertKept(fileView, "class_test_011.A1");
    AssertKept(fileView, "class_test_011.A1.foo:class_test_011.A1;i32;void;");

    AssertKept(fileView, "class_test_011.A2");
    AssertKept(fileView, "class_test_011.A2.foo:class_test_011.A2;i32;std.core.String;void;");

    AssertKept(fileView, "class_test_011.A3");
    AssertKept(fileView, "class_test_011.A3.foo:class_test_011.A3;i64;void;");

    AssertKept(fileView, "class_test_011.A4");
    AssertKept(fileView, "class_test_011.A4.foo:class_test_011.A4;i64;i32;void;");

    AssertKept(fileView, "class_test_011.B1");
    AssertKept(fileView, "class_test_011.B1.foo:class_test_011.B1;i32;");

    AssertKept(fileView, "class_test_011.B2");
    AssertKept(fileView, "class_test_011.B2.foo:class_test_011.B2;i32;{Ustd.core.Int,std.core.String};");

    AssertKept(fileView, "class_test_011.B3");
    AssertKept(fileView, "class_test_011.B3.foo:class_test_011.B3;i64;");

    AssertKept(fileView, "class_test_011.C1");
    AssertKept(fileView, "class_test_011.C1.field");
    AssertKept(fileView, "class_test_011.C1.field2");

    AssertKept(fileView, "class_test_011.C2");
    AssertKept(fileView, "class_test_011.C2.field");

    AssertKept(fileView, "class_test_011.C3");
    AssertKept(fileView, "class_test_011.C3.field");

    AssertKept(fileView, "class_test_011.C4");
    AssertKept(fileView, "class_test_011.C4.field");
}

/**
 * @tc.name: name_marker_class_test_012
 * @tc.desc: test whether % wildcard matcher string and bigint and union type
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(NameMarkerTest, name_marker_class_test_012, TestSize.Level1)
{
    const std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/name_mark/code_sample/class_test_012.abc";
    const std::string configFilePath = GUARD_UNIT_TEST_DIR "ut/name_mark/code_sample/class_test_012_config.json";
    abckit_wrapper::FileView fileView;
    NameMarkerTestExecute(abcFilePath, configFilePath, fileView);

    AssertModuleNotKept(fileView, "class_test_012");

    AssertKept(fileView, "class_test_012.ClassB");
    AssertKept(fileView, "class_test_012.ClassB.f1");
    AssertKept(fileView, "class_test_012.ClassB.f2");
    AssertKept(fileView, "class_test_012.ClassC");
    AssertKept(fileView, "class_test_012.ClassC.foo1:class_test_012.ClassC;{Ustd.core.Int,std.core.String};");
    AssertNotKept(fileView, "class_test_012.ClassB.f3");
    AssertNotKept(fileView, "class_test_012.ClassB.f4");
}

/**
 * @tc.name: name_marker_class_test_013
 * @tc.desc: test the keep effect in the scenario where the types do not match
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(NameMarkerTest, name_marker_class_test_013, TestSize.Level1)
{
    const std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/name_mark/code_sample/class_test_013.abc";
    const std::string configFilePath = GUARD_UNIT_TEST_DIR "ut/name_mark/code_sample/class_test_013_config.json";
    abckit_wrapper::FileView fileView;
    NameMarkerTestExecute(abcFilePath, configFilePath, fileView);

    AssertModuleNotKept(fileView, "class_test_013");

    AssertKept(fileView, "class_test_013.ClassA");
    AssertKept(fileView, "class_test_013.ClassA.f1");
    AssertKept(fileView, "class_test_013.ClassB");
    AssertKept(fileView, "class_test_013.ClassB.f1");
    AssertNotKept(fileView, "class_test_013.ClassC");
    AssertKept(fileView, "class_test_013.ClassC.f1");

    AssertNotKept(fileView, "class_test_013.ClassD");
    AssertNotKept(fileView, "class_test_013.ClassD.f1");
    AssertNotKept(fileView, "class_test_013.ClassE");
    AssertNotKept(fileView, "class_test_013.ClassE.f1");
    AssertNotKept(fileView, "class_test_013.ClassF");
    AssertNotKept(fileView, "class_test_013.ClassF.f1");
}

/**
 * @tc.name: name_marker_interface_test_001
 * @tc.desc: test name marker module interface
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(NameMarkerTest, name_marker_interface_test_001, TestSize.Level1)
{
    const std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/name_mark/code_sample/interface_test_001.abc";
    const std::string configFilePath =
        GUARD_UNIT_TEST_DIR "ut/name_mark/code_sample/interface_test_001_config.json";
    abckit_wrapper::FileView fileView;
    NameMarkerTestExecute(abcFilePath, configFilePath, fileView);

    AssertModuleNotKept(fileView, "interface_test_001");

    AssertKept(fileView, "interface_test_001.Interface1");
    AssertKept(fileView, "interface_test_001.Interface2");
}

/**
 * @tc.name: name_marker_interface_test_002
 * @tc.desc: test name marker module interface method and with annotation
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(NameMarkerTest, name_marker_interface_test_002, TestSize.Level1)
{
    const std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/name_mark/code_sample/interface_test_002.abc";
    const std::string configFilePath =
        GUARD_UNIT_TEST_DIR "ut/name_mark/code_sample/interface_test_002_config.json";
    abckit_wrapper::FileView fileView;
    NameMarkerTestExecute(abcFilePath, configFilePath, fileView);

    AssertModuleNotKept(fileView, "interface_test_002");

    AssertKept(fileView, "interface_test_002.Interface1");
    AssertKept(fileView, "interface_test_002.Interface1.iMethod1:interface_test_002.Interface1;void;");
    AssertKept(fileView, "interface_test_002.Interface1.iMethod2:interface_test_002.Interface1;void;");

    AssertKept(fileView, "interface_test_002.Interface2");
    AssertKept(fileView, "interface_test_002.Interface2.iMethod3:interface_test_002.Interface2;void;");
    AssertNotKept(fileView, "interface_test_002.Interface2.iMethod4:interface_test_002.Interface2;void;");
}

/**
 * @tc.name: name_marker_interface_test_003
 * @tc.desc: test name marker module interface field with annotation
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(NameMarkerTest, name_marker_interface_test_003, TestSize.Level1)
{
    const std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/name_mark/code_sample/interface_test_003.abc";
    const std::string configFilePath =
        GUARD_UNIT_TEST_DIR "ut/name_mark/code_sample/interface_test_003_config.json";
    abckit_wrapper::FileView fileView;
    NameMarkerTestExecute(abcFilePath, configFilePath, fileView);

    AssertModuleNotKept(fileView, "interface_test_003");

    AssertKept(fileView, "interface_test_003.Interface1");
    AssertKept(fileView, "interface_test_003.Interface1.%%get-iField1:interface_test_003.Interface1;i32;");
    AssertKept(fileView, "interface_test_003.Interface1.%%set-iField1:interface_test_003.Interface1;i32;void;");
    AssertKept(fileView, "interface_test_003.Interface1.%%get-iField2:interface_test_003.Interface1;i32;");
    AssertKept(fileView, "interface_test_003.Interface1.%%set-iField2:interface_test_003.Interface1;i32;void;");

    AssertKept(fileView, "interface_test_003.Interface2");
    AssertKept(fileView, "interface_test_003.Interface2.%%get-iField3:interface_test_003.Interface2;i32;");
    AssertKept(fileView, "interface_test_003.Interface2.%%set-iField3:interface_test_003.Interface2;i32;void;");
    AssertNotKept(fileView, "interface_test_003.Interface2.%%get-iField4:interface_test_003.Interface2;i32;");
    AssertNotKept(fileView, "interface_test_003.Interface2.%%set-iField4:interface_test_003.Interface2;i32;void;");
}

/**
 * @tc.name: name_marker_interface_test_004
 * @tc.desc: test name marker namespace interface
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(NameMarkerTest, name_marker_interface_test_004, TestSize.Level1)
{
    const std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/name_mark/code_sample/interface_test_004.abc";
    const std::string configFilePath =
        GUARD_UNIT_TEST_DIR "ut/name_mark/code_sample/interface_test_004_config.json";
    abckit_wrapper::FileView fileView;
    NameMarkerTestExecute(abcFilePath, configFilePath, fileView);

    AssertModuleNotKept(fileView, "interface_test_004");

    AssertKept(fileView, "interface_test_004.Ns1.Interface1");
    AssertKept(fileView, "interface_test_004.Ns1.Interface2");
}

/**
 * @tc.name: name_marker_interface_test_005
 * @tc.desc: test name marker namespace interface method and with annotation
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(NameMarkerTest, name_marker_interface_test_005, TestSize.Level1)
{
    const std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/name_mark/code_sample/interface_test_005.abc";
    const std::string configFilePath =
        GUARD_UNIT_TEST_DIR "ut/name_mark/code_sample/interface_test_005_config.json";
    abckit_wrapper::FileView fileView;
    NameMarkerTestExecute(abcFilePath, configFilePath, fileView);

    AssertModuleNotKept(fileView, "interface_test_005");

    AssertKept(fileView, "interface_test_005.Ns1.Interface1");
    AssertKept(fileView, "interface_test_005.Ns1.Interface1.iMethod1:interface_test_005.Ns1.Interface1;void;");
    AssertKept(fileView, "interface_test_005.Ns1.Interface1.iMethod2:interface_test_005.Ns1.Interface1;void;");

    AssertKept(fileView, "interface_test_005.Ns1.Interface2");
    AssertKept(fileView, "interface_test_005.Ns1.Interface2.iMethod3:interface_test_005.Ns1.Interface2;void;");
    AssertNotKept(fileView, "interface_test_005.Ns1.Interface2.iMethod4:interface_test_005.Ns1.Interface2;void;");
}

/**
 * @tc.name: name_marker_interface_test_006
 * @tc.desc: test name marker namespace interface field with annotation
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(NameMarkerTest, name_marker_interface_test_006, TestSize.Level1)
{
    const std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/name_mark/code_sample/interface_test_006.abc";
    const std::string configFilePath =
        GUARD_UNIT_TEST_DIR "ut/name_mark/code_sample/interface_test_006_config.json";
    abckit_wrapper::FileView fileView;
    NameMarkerTestExecute(abcFilePath, configFilePath, fileView);

    AssertModuleNotKept(fileView, "interface_test_006");

    AssertKept(fileView, "interface_test_006.Ns1.Interface1");
    AssertKept(fileView, "interface_test_006.Ns1.Interface1.%%get-iField1:interface_test_006.Ns1.Interface1;i32;");
    AssertKept(fileView, "interface_test_006.Ns1.Interface1.%%set-iField1:interface_test_006.Ns1.Interface1;i32;void;");
    AssertKept(fileView, "interface_test_006.Ns1.Interface1.%%get-iField2:interface_test_006.Ns1.Interface1;i32;");
    AssertKept(fileView, "interface_test_006.Ns1.Interface1.%%set-iField2:interface_test_006.Ns1.Interface1;i32;void;");

    AssertKept(fileView, "interface_test_006.Ns1.Interface2");
    AssertKept(fileView, "interface_test_006.Ns1.Interface2.%%get-iField3:interface_test_006.Ns1.Interface2;i32;");
    AssertKept(fileView, "interface_test_006.Ns1.Interface2.%%set-iField3:interface_test_006.Ns1.Interface2;i32;void;");
    AssertNotKept(fileView, "interface_test_006.Ns1.Interface2.%%get-iField4:interface_test_006.Ns1.Interface2;i32;");
    AssertNotKept(fileView,
                  "interface_test_006.Ns1.Interface2.%%set-iField4:interface_test_006.Ns1.Interface2;i32;void;");
}

/**
 * @tc.name: name_marker_interface_test_007
 * @tc.desc: test name marker keep declare type is interface and not interface
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(NameMarkerTest, name_marker_interface_test_007, TestSize.Level1)
{
    const std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/name_mark/code_sample/interface_test_007.abc";
    const std::string configFilePath =
        GUARD_UNIT_TEST_DIR "ut/name_mark/code_sample/interface_test_007_config.json";
    abckit_wrapper::FileView fileView;
    NameMarkerTestExecute(abcFilePath, configFilePath, fileView);

    AssertModuleNotKept(fileView, "interface_test_007");

    AssertKept(fileView, "interface_test_007.Interface1");
    AssertKept(fileView, "interface_test_007.Interface1.%%get-iField1:interface_test_007.Interface1;i32;");
    AssertKept(fileView, "interface_test_007.Interface1.%%set-iField1:interface_test_007.Interface1;i32;void;");

    AssertKept(fileView, "interface_test_007.Ns1.Interface2");
    AssertKept(fileView, "interface_test_007.Ns1.Interface2.%%get-iField2:interface_test_007.Ns1.Interface2;i32;");
    AssertKept(fileView, "interface_test_007.Ns1.Interface2.%%set-iField2:interface_test_007.Ns1.Interface2;i32;void;");
}

/**
 * @tc.name: name_marker_enum_test_001
 * @tc.desc: test name marker module enum
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(NameMarkerTest, name_marker_enum_test_001, TestSize.Level1)
{
    const std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/name_mark/code_sample/enum_test_001.abc";
    const std::string configFilePath = GUARD_UNIT_TEST_DIR "ut/name_mark/code_sample/enum_test_001_config.json";
    abckit_wrapper::FileView fileView;
    NameMarkerTestExecute(abcFilePath, configFilePath, fileView);

    AssertModuleNotKept(fileView, "enum_test_001");

    AssertKept(fileView, "enum_test_001.Color1");
    AssertKept(fileView, "enum_test_001.Color1.RED");
    AssertKept(fileView, "enum_test_001.Color1.GREEN");

    AssertKept(fileView, "enum_test_001.Color2");
    AssertKept(fileView, "enum_test_001.Color2.YELLOW");
    AssertNotKept(fileView, "enum_test_001.Color2.BLUE");
}

/**
 * @tc.name: name_marker_enum_test_002
 * @tc.desc: test name marker namespace enum
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(NameMarkerTest, name_marker_enum_test_002, TestSize.Level1)
{
    const std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/name_mark/code_sample/enum_test_002.abc";
    const std::string configFilePath = GUARD_UNIT_TEST_DIR "ut/name_mark/code_sample/enum_test_002_config.json";
    abckit_wrapper::FileView fileView;
    NameMarkerTestExecute(abcFilePath, configFilePath, fileView);

    AssertModuleNotKept(fileView, "enum_test_002");

    AssertKept(fileView, "enum_test_002.Ns1.Color1");
    AssertKept(fileView, "enum_test_002.Ns1.Color1.RED");
    AssertKept(fileView, "enum_test_002.Ns1.Color1.GREEN");

    AssertKept(fileView, "enum_test_002.Ns1.Color2");
    AssertKept(fileView, "enum_test_002.Ns1.Color2.YELLOW");
    AssertNotKept(fileView, "enum_test_002.Ns1.Color2.BLUE");
}

/**
 * @tc.name: name_marker_enum_test_003
 * @tc.desc: test name marker keep declare type is enum and not enum
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(NameMarkerTest, name_marker_enum_test_003, TestSize.Level1)
{
    const std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/name_mark/code_sample/enum_test_003.abc";
    const std::string configFilePath = GUARD_UNIT_TEST_DIR "ut/name_mark/code_sample/enum_test_003_config.json";
    abckit_wrapper::FileView fileView;
    NameMarkerTestExecute(abcFilePath, configFilePath, fileView);

    AssertModuleNotKept(fileView, "enum_test_003");

    AssertKept(fileView, "enum_test_003.Color1");
    AssertNotKept(fileView, "enum_test_003.Color1.RED1");
    AssertKept(fileView, "enum_test_003.Color1.GREEN1");

    AssertKept(fileView, "enum_test_003.Ns1.Color2");
    AssertKept(fileView, "enum_test_003.Ns1.Color2.RED2");
    AssertNotKept(fileView, "enum_test_003.Ns1.Color2.GREEN2");
}

/**
 * @tc.name: name_marker_namespace_test_001
 * @tc.desc: test name marker namespace
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(NameMarkerTest, name_marker_namespace_test_001, TestSize.Level1)
{
    const std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/name_mark/code_sample/namespace_test_001.abc";
    const std::string configFilePath =
        GUARD_UNIT_TEST_DIR "ut/name_mark/code_sample/namespace_test_001_config.json";
    abckit_wrapper::FileView fileView;
    NameMarkerTestExecute(abcFilePath, configFilePath, fileView);

    AssertModuleNotKept(fileView, "namespace_test_001");

    AssertKept(fileView, "namespace_test_001.Ns1");
    AssertKept(fileView, "namespace_test_001.Ns1.Ns2");

    AssertNotKept(fileView, "namespace_test_001.Ns3");
}

/**
 * @tc.name: name_marker_namespace_test_002
 * @tc.desc: test name marker namespace method and with annotation
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(NameMarkerTest, name_marker_namespace_test_002, TestSize.Level1)
{
    const std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/name_mark/code_sample/namespace_test_002.abc";
    const std::string configFilePath =
        GUARD_UNIT_TEST_DIR "ut/name_mark/code_sample/namespace_test_002_config.json";
    abckit_wrapper::FileView fileView;
    NameMarkerTestExecute(abcFilePath, configFilePath, fileView);

    AssertModuleNotKept(fileView, "namespace_test_002");

    AssertKept(fileView, "namespace_test_002.Ns1.Ns2.method0:void;");
    AssertKept(fileView, "namespace_test_002.Ns1.method1:void;");
    AssertNotKept(fileView, "namespace_test_002.Ns1.method2:void;");
}

/**
 * @tc.name: name_marker_namespace_test_003
 * @tc.desc: test name marker namespace field with annotation
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(NameMarkerTest, name_marker_namespace_test_003, TestSize.Level1)
{
    const std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/name_mark/code_sample/namespace_test_003.abc";
    const std::string configFilePath =
        GUARD_UNIT_TEST_DIR "ut/name_mark/code_sample/namespace_test_003_config.json";
    abckit_wrapper::FileView fileView;
    NameMarkerTestExecute(abcFilePath, configFilePath, fileView);

    AssertModuleNotKept(fileView, "namespace_test_003");

    AssertKept(fileView, "namespace_test_003.Ns1.Ns2.field1");
    AssertKept(fileView, "namespace_test_003.Ns1.field2");
    AssertNotKept(fileView, "namespace_test_003.Ns1.field3");
}

/**
 * @tc.name: name_marker_namespace_test_004
 * @tc.desc: test name marker namespace and all child
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(NameMarkerTest, name_marker_namespace_test_004, TestSize.Level1)
{
    const std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/name_mark/code_sample/namespace_test_004.abc";
    const std::string configFilePath =
        GUARD_UNIT_TEST_DIR "ut/name_mark/code_sample/namespace_test_004_config.json";
    abckit_wrapper::FileView fileView;
    NameMarkerTestExecute(abcFilePath, configFilePath, fileView);

    AssertModuleNotKept(fileView, "namespace_test_004");

    AssertNotKept(fileView, "namespace_test_004.MyAnno1");

    AssertKept(fileView, "namespace_test_004.Ns1");
    AssertKept(fileView, "namespace_test_004.Ns1.Ns2");

    AssertKept(fileView, "namespace_test_004.Ns1.foo2:i32;i32;void;");
    AssertKept(fileView, "namespace_test_004.Ns1.m2");

    AssertKept(fileView, "namespace_test_004.Ns1.Interface2");
    AssertKept(fileView,
               "namespace_test_004.Ns1.Interface2.%%get-iField2:namespace_test_004.Ns1.Interface2;std.core.String;");
    AssertKept(
        fileView,
        "namespace_test_004.Ns1.Interface2.%%set-iField2:namespace_test_004.Ns1.Interface2;std.core.String;void;");
    AssertKept(fileView, "namespace_test_004.Ns1.Interface2.iMethod2:namespace_test_004.Ns1.Interface2;void;");

    AssertKept(fileView, "namespace_test_004.Ns1.MyAnno2");

    AssertKept(fileView, "namespace_test_004.Ns1.ClassB");
    AssertKept(fileView, "namespace_test_004.Ns1.ClassB.sField2");
    AssertKept(fileView, "namespace_test_004.Ns1.ClassB.field2");
    AssertKept(fileView,
               "namespace_test_004.Ns1.ClassB.%%set-iField2:namespace_test_004.Ns1.ClassB;std.core.String;void;");
    AssertKept(fileView, "namespace_test_004.Ns1.ClassB.%%get-iField2:namespace_test_004.Ns1.ClassB;std.core.String;");
    AssertKept(fileView, "namespace_test_004.Ns1.ClassB.sMethod2:i32;void;");
    AssertKept(fileView, "namespace_test_004.Ns1.ClassB.method2:namespace_test_004.Ns1.ClassB;void;");
    AssertKept(fileView, "namespace_test_004.Ns1.ClassB.iMethod2:namespace_test_004.Ns1.ClassB;void;");

    AssertKept(fileView, "namespace_test_004.Ns1.Color2");
    AssertKept(fileView, "namespace_test_004.Ns1.Color2.YELLOW");
}

/**
 * @tc.name: name_marker_namespace_test_005
 * @tc.desc: test name marker keep declare type is namespace and not namespace
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(NameMarkerTest, name_marker_namespace_test_005, TestSize.Level1)
{
    const std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/name_mark/code_sample/namespace_test_005.abc";
    const std::string configFilePath =
        GUARD_UNIT_TEST_DIR "ut/name_mark/code_sample/namespace_test_005_config.json";
    abckit_wrapper::FileView fileView;
    NameMarkerTestExecute(abcFilePath, configFilePath, fileView);

    AssertModuleNotKept(fileView, "namespace_test_005");

    AssertKept(fileView, "namespace_test_005.Ns1");
    AssertKept(fileView, "namespace_test_005.Ns1.foo1:void;");

    AssertKept(fileView, "namespace_test_005.Ns2");
    AssertKept(fileView, "namespace_test_005.Ns2.foo2:void;");

    AssertNotKept(fileView, "namespace_test_005.Ns3");
    AssertNotKept(fileView, "namespace_test_005.Ns3.foo3:void;");
}

/**
 * @tc.name: name_marker_module_test_001
 * @tc.desc: test name marker module field with annotation
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(NameMarkerTest, name_marker_module_test_001, TestSize.Level1)
{
    const std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/name_mark/code_sample/module_test_001.abc";
    const std::string configFilePath = GUARD_UNIT_TEST_DIR "ut/name_mark/code_sample/module_test_001_config.json";
    abckit_wrapper::FileView fileView;
    NameMarkerTestExecute(abcFilePath, configFilePath, fileView);

    AssertModuleKept(fileView, "module_test_001");

    AssertKept(fileView, "module_test_001.m1");
}

/**
 * @tc.name: name_marker_module_test_002
 * @tc.desc: test name marker module method with annotation
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(NameMarkerTest, name_marker_module_test_002, TestSize.Level1)
{
    const std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/name_mark/code_sample/module_test_002.abc";
    const std::string configFilePath = GUARD_UNIT_TEST_DIR "ut/name_mark/code_sample/module_test_002_config.json";
    abckit_wrapper::FileView fileView;
    NameMarkerTestExecute(abcFilePath, configFilePath, fileView);

    AssertModuleKept(fileView, "module_test_002");

    AssertKept(fileView, "module_test_002.foo1:i32;i32;void;");
}

/**
 * @tc.name: name_marker_module_test_003
 * @tc.desc: test name marker module and all child
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(NameMarkerTest, name_marker_module_test_003, TestSize.Level1)
{
    const std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/name_mark/code_sample/module_test_003.abc";
    const std::string configFilePath = GUARD_UNIT_TEST_DIR "ut/name_mark/code_sample/module_test_003_config.json";
    abckit_wrapper::FileView fileView;
    NameMarkerTestExecute(abcFilePath, configFilePath, fileView);

    AssertModuleKept(fileView, "module_test_003");

    AssertKept(fileView, "module_test_003.foo1:i32;i32;void;");
    AssertKept(fileView, "module_test_003.m1");

    AssertKept(fileView, "module_test_003.Interface1");
    AssertKept(fileView, "module_test_003.Interface1.%%get-iField1:module_test_003.Interface1;i32;");
    AssertKept(fileView, "module_test_003.Interface1.%%set-iField1:module_test_003.Interface1;i32;void;");
    AssertKept(fileView, "module_test_003.Interface1.iMethod1:module_test_003.Interface1;void;");

    AssertKept(fileView, "module_test_003.MyAnno1");

    AssertKept(fileView, "module_test_003.ClassA");
    AssertKept(fileView, "module_test_003.ClassA.sField1");
    AssertKept(fileView, "module_test_003.ClassA.field1");
    AssertKept(fileView, "module_test_003.ClassA.%%set-iField1:module_test_003.ClassA;i32;void;");
    AssertKept(fileView, "module_test_003.ClassA.%%get-iField1:module_test_003.ClassA;i32;");
    AssertKept(fileView, "module_test_003.ClassA.sMethod1:i32;void;");
    AssertKept(fileView, "module_test_003.ClassA.method1:module_test_003.ClassA;void;");
    AssertKept(fileView, "module_test_003.ClassA.iMethod1:module_test_003.ClassA;void;");

    AssertKept(fileView, "module_test_003.Color1");
    AssertKept(fileView, "module_test_003.Color1.RED");

    AssertKept(fileView, "module_test_003.Ns1");
    AssertKept(fileView, "module_test_003.Ns1.Ns2");
    AssertKept(fileView, "module_test_003.Ns1.foo2:i32;i32;void;");
    AssertKept(fileView, "module_test_003.Ns1.m2");
    AssertKept(fileView, "module_test_003.Ns1.Interface2");
    AssertKept(fileView,
               "module_test_003.Ns1.Interface2.%%get-iField2:module_test_003.Ns1.Interface2;std.core.String;");
    AssertKept(fileView,
               "module_test_003.Ns1.Interface2.%%set-iField2:module_test_003.Ns1.Interface2;std.core.String;void;");
    AssertKept(fileView, "module_test_003.Ns1.Interface2.iMethod2:module_test_003.Ns1.Interface2;void;");

    AssertKept(fileView, "module_test_003.Ns1.MyAnno2");

    AssertKept(fileView, "module_test_003.Ns1.ClassB");
    AssertKept(fileView, "module_test_003.Ns1.ClassB.sField2");
    AssertKept(fileView, "module_test_003.Ns1.ClassB.field2");
    AssertKept(fileView, "module_test_003.Ns1.ClassB.%%set-iField2:module_test_003.Ns1.ClassB;std.core.String;void;");
    AssertKept(fileView, "module_test_003.Ns1.ClassB.%%get-iField2:module_test_003.Ns1.ClassB;std.core.String;");
    AssertKept(fileView, "module_test_003.Ns1.ClassB.sMethod2:i32;void;");
    AssertKept(fileView, "module_test_003.Ns1.ClassB.method2:module_test_003.Ns1.ClassB;void;");
    AssertKept(fileView, "module_test_003.Ns1.ClassB.iMethod2:module_test_003.Ns1.ClassB;void;");
    AssertKept(fileView, "module_test_003.Ns1.Color2");
    AssertKept(fileView, "module_test_003.Ns1.Color2.YELLOW");
}

/**
 * @tc.name: name_marker_module_test_004
 * @tc.desc: test name marker keep declare type is package and not package
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(NameMarkerTest, name_marker_module_test_004, TestSize.Level1)
{
    const std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/name_mark/code_sample/module_test_004.abc";
    const std::string configFilePath = GUARD_UNIT_TEST_DIR "ut/name_mark/code_sample/module_test_004_config.json";
    abckit_wrapper::FileView fileView;
    NameMarkerTestExecute(abcFilePath, configFilePath, fileView);

    AssertModuleKept(fileView, "module_test_004");

    AssertKept(fileView, "module_test_004.foo1:i32;i32;void;");
    AssertKept(fileView, "module_test_004.m1");
}

/**
 * @tc.name: name_marker_module_test_005
 * @tc.desc: test name marker keep path with string
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(NameMarkerTest, name_marker_module_test_005, TestSize.Level1)
{
    const std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/name_mark/code_sample/module_test_005.abc";
    const std::string configFilePath = GUARD_UNIT_TEST_DIR "ut/name_mark/code_sample/module_test_005_config.json";
    abckit_wrapper::FileView fileView;
    NameMarkerTestExecute(abcFilePath, configFilePath, fileView);

    AssertModuleKept(fileView, "module_test_005");
    AssertNotKept(fileView, "module_test_005.foo1:i32;i32;void;");
    AssertNotKept(fileView, "module_test_005.m1");
}

/**
 * @tc.name: name_marker_module_test_006
 * @tc.desc: test name marker keep path with universal string
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(NameMarkerTest, name_marker_module_test_006, TestSize.Level1)
{
    const std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/name_mark/code_sample/module_test_006.abc";
    const std::string configFilePath = GUARD_UNIT_TEST_DIR "ut/name_mark/code_sample/module_test_006_config.json";
    abckit_wrapper::FileView fileView;
    NameMarkerTestExecute(abcFilePath, configFilePath, fileView);

    AssertModuleKept(fileView, "module_test_006");
    AssertNotKept(fileView, "module_test_006.foo1:i32;i32;void;");
    AssertNotKept(fileView, "module_test_006.m1");
}

/**
 * @tc.name: name_marker_module_test_007
 * @tc.desc: test name marker module field with union type
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(NameMarkerTest, name_marker_module_test_007, TestSize.Level1)
{
    const std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/name_mark/code_sample/module_test_007.abc";
    const std::string configFilePath = GUARD_UNIT_TEST_DIR "ut/name_mark/code_sample/module_test_007_config.json";
    abckit_wrapper::FileView fileView;
    NameMarkerTestExecute(abcFilePath, configFilePath, fileView);

    AssertModuleKept(fileView, "module_test_007");
    AssertKept(fileView, "module_test_007.v1");
    AssertKept(fileView, "module_test_007.v2");
    AssertKept(fileView, "module_test_007.v3");
    AssertKept(fileView, "module_test_007.v4");
    AssertKept(fileView, "module_test_007.v5");
    AssertKept(fileView, "module_test_007.v6");
    AssertKept(fileView, "module_test_007.v7");
    AssertKept(fileView, "module_test_007.v8");
    AssertKept(fileView, "module_test_007.v9");
    AssertKept(fileView, "module_test_007.v10");
    AssertKept(fileView, "module_test_007.v11");
    AssertKept(fileView, "module_test_007.v12");
    AssertKept(fileView, "module_test_007.v13");
    AssertKept(fileView, "module_test_007.v14");
    AssertKept(fileView, "module_test_007.v15");
    AssertKept(fileView, "module_test_007.v16");
}

/**
 * @tc.name: name_marker_module_test_008
 * @tc.desc: test name marker module method arguments and return type with union type
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(NameMarkerTest, name_marker_module_test_008, TestSize.Level1)
{
    const std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/name_mark/code_sample/module_test_008.abc";
    const std::string configFilePath = GUARD_UNIT_TEST_DIR "ut/name_mark/code_sample/module_test_008_config.json";
    abckit_wrapper::FileView fileView;
    NameMarkerTestExecute(abcFilePath, configFilePath, fileView);

    AssertModuleKept(fileView, "module_test_008");
    AssertKept(fileView, "module_test_008.foo1:{Ustd.core.Int,std.core.String};void;");
    AssertKept(fileView, "module_test_008.foo2:{Ustd.core.Byte,std.core.String};void;");
    AssertKept(fileView, "module_test_008.foo3:{Ustd.core.Short,std.core.String};void;");
    AssertKept(fileView, "module_test_008.foo4:{Ustd.core.Double,std.core.String};void;");
    AssertKept(fileView, "module_test_008.foo5:{Ustd.core.Long,std.core.String};void;");
    AssertKept(fileView, "module_test_008.foo6:{Ustd.core.Float,std.core.String};void;");
    AssertKept(fileView, "module_test_008.foo7:{Ustd.core.Double,std.core.String};void;");
    AssertKept(fileView, "module_test_008.foo8:{Ustd.core.Char,std.core.String};void;");
    AssertKept(fileView, "module_test_008.foo9:{Ustd.core.Boolean,std.core.String};void;");
    AssertKept(fileView, "module_test_008.foo10:{Ustd.core.BigInt,std.core.String};void;");
    AssertKept(fileView, "module_test_008.foo11:std.core.String;void;");
    AssertKept(fileView, "module_test_008.foo12:{Ustd.core.Null,std.core.String};void;");
    AssertKept(fileView, "module_test_008.foo13:std.core.String;void;");
    AssertKept(fileView, "module_test_008.foo14:{Ustd.core.Array,std.core.String};void;");
    AssertKept(fileView, "module_test_008.foo15:std.core.Object;void;");
    AssertKept(fileView, "module_test_008.foo16:std.core.Int;void;");

    AssertKept(fileView, "module_test_008.fooEx1:{Ustd.core.Int,std.core.String};");
    AssertKept(fileView, "module_test_008.fooEx2:{Ustd.core.Byte,std.core.String};");
    AssertKept(fileView, "module_test_008.fooEx3:{Ustd.core.Short,std.core.String};");
    AssertKept(fileView, "module_test_008.fooEx4:{Ustd.core.Double,std.core.String};");
    AssertKept(fileView, "module_test_008.fooEx5:{Ustd.core.Long,std.core.String};");
    AssertKept(fileView, "module_test_008.fooEx6:{Ustd.core.Float,std.core.String};");
    AssertKept(fileView, "module_test_008.fooEx7:{Ustd.core.Double,std.core.String};");
    AssertKept(fileView, "module_test_008.fooEx8:{Ustd.core.Char,std.core.String};");
    AssertKept(fileView, "module_test_008.fooEx9:{Ustd.core.Boolean,std.core.String};");
    AssertKept(fileView, "module_test_008.fooEx10:{Ustd.core.BigInt,std.core.String};");
    AssertKept(fileView, "module_test_008.fooEx11:std.core.String;");
    AssertKept(fileView, "module_test_008.fooEx12:{Ustd.core.Null,std.core.String};");
    AssertKept(fileView, "module_test_008.fooEx13:std.core.String;");
    AssertKept(fileView, "module_test_008.fooEx14:{Ustd.core.Array,std.core.String};");
    AssertKept(fileView, "module_test_008.fooEx15:std.core.Object;");
    AssertKept(fileView, "module_test_008.fooEx16:std.core.Int;");
}

/**
 * @tc.name: name_marker_module_test_009
 * @tc.desc: test name marker enable file name obfuscation, but not config keep file name
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(NameMarkerTest, name_marker_module_test_009, TestSize.Level1)
{
    const std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/name_mark/code_sample/module_test_009.abc";
    const std::string configFilePath = GUARD_UNIT_TEST_DIR "ut/name_mark/code_sample/module_test_009_config.json";
    abckit_wrapper::FileView fileView;
    NameMarkerTestExecute(abcFilePath, configFilePath, fileView);

    AssertModuleNotKept(fileView, "module_test_009");
    AssertNotKept(fileView, "module_test_009.m1");
}

/**
 * @tc.name: name_marker_module_test_010
 * @tc.desc: test name marker keep lambda type
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(NameMarkerTest, name_marker_module_test_010, TestSize.Level1)
{
    const std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/name_mark/code_sample/module_test_010.abc";
    const std::string configFilePath = GUARD_UNIT_TEST_DIR "ut/name_mark/code_sample/module_test_010_config.json";
    abckit_wrapper::FileView fileView;
    NameMarkerTestExecute(abcFilePath, configFilePath, fileView);

    AssertModuleKept(fileView, "module_test_010");
    AssertKept(fileView, "module_test_010.math1");
    AssertKept(fileView, "module_test_010.math2");
}

/**
 * @tc.name: name_marker_module_test_011
 * @tc.desc: test name marker keep tuple type
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(NameMarkerTest, name_marker_module_test_011, TestSize.Level1)
{
    const std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/name_mark/code_sample/module_test_011.abc";
    const std::string configFilePath = GUARD_UNIT_TEST_DIR "ut/name_mark/code_sample/module_test_011_config.json";
    abckit_wrapper::FileView fileView;
    NameMarkerTestExecute(abcFilePath, configFilePath, fileView);

    AssertModuleKept(fileView, "module_test_011");
    AssertKept(fileView, "module_test_011.field1");
    AssertKept(fileView, "module_test_011.field2");
}
