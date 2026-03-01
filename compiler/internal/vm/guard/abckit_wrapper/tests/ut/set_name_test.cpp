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

#include "abckit_wrapper/file_view.h"
#include "test_util/assert.h"
#include "../../library/logger.h"

using namespace testing::ext;

namespace {
constexpr std::string_view ABC_FILE_PATH = ABCKIT_WRAPPER_ABC_FILE_DIR "ut/set_name_test.abc";
constexpr std::string_view UPDATED_ABC_FILE_PATH = ABCKIT_WRAPPER_ABC_FILE_DIR "ut/set_name_test_updated.abc";

bool FileExists(const std::string &filePath)
{
    return std::filesystem::exists(filePath);
}

bool DeleteFile(const std::string &filePath)
{
    if (std::filesystem::remove(filePath)) {
        return true;
    }
    return false;
}
}  // namespace

class TestSetName : public ::testing::Test {
public:
    void SetUp() override
    {
        ASSERT_SUCCESS(fileView.Init(ABC_FILE_PATH));
    }

    void TearDown() override
    {
        fileView.Save(UPDATED_ABC_FILE_PATH);
        ASSERT_TRUE(FileExists(UPDATED_ABC_FILE_PATH.data()));
        ASSERT_TRUE(DeleteFile(UPDATED_ABC_FILE_PATH.data()));
    }

    abckit_wrapper::FileView fileView;
};

/**
 * @tc.name: module_set_name_001
 * @tc.desc: test module set name
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestSetName, module_set_name_001, TestSize.Level0)
{
    const auto module = fileView.GetModule("set_name_test");
    ASSERT_TRUE(module.has_value());

    ASSERT_TRUE(module.value()->SetName("a"));
    ASSERT_EQ(module.value()->GetName(), "a");
}

/**
 * @tc.name: module_field_set_name_001
 * @tc.desc: test module field set name
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestSetName, module_field_set_name_001, TestSize.Level0)
{
    const auto m1 = fileView.Get<abckit_wrapper::Field>("set_name_test.m1");
    ASSERT_TRUE(m1.has_value());

    ASSERT_TRUE(m1.value()->SetName("a"));
}

/**
 * @tc.name: namespace_set_name_001
 * @tc.desc: test namespace set name
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestSetName, namespace_set_name_001, TestSize.Level0)
{
    const auto ns1 = fileView.Get<abckit_wrapper::Namespace>("set_name_test.Ns1");
    ASSERT_TRUE(ns1.has_value());

    ASSERT_TRUE(ns1.value()->SetName("a"));
}

/**
 * @tc.name: namespace_field_set_name_001
 * @tc.desc: test namespace field set name
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestSetName, namespace_field_set_name_001, TestSize.Level0)
{
    const auto nsField1 = fileView.Get<abckit_wrapper::Field>("set_name_test.Ns1.field1");
    ASSERT_TRUE(nsField1.has_value());

    ASSERT_TRUE(nsField1.value()->SetName("a"));
    ASSERT_EQ(nsField1.value()->GetRawName(), "a");

    const auto nsField2 = fileView.Get<abckit_wrapper::Field>("set_name_test.Ns1.Ns2.field2");
    ASSERT_TRUE(nsField2.has_value());

    ASSERT_TRUE(nsField2.value()->SetName("a"));
    ASSERT_EQ(nsField2.value()->GetRawName(), "a");
}

/**
 * @tc.name: function_set_name_001
 * @tc.desc: test function set name
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestSetName, function_set_name_001, TestSize.Level0)
{
    const auto method1 = fileView.Get<abckit_wrapper::Method>("set_name_test.foo1:void;");
    ASSERT_TRUE(method1.has_value());

    ASSERT_TRUE(method1.value()->SetName("a"));
    ASSERT_EQ(method1.value()->GetName(), "a:void;");
    ASSERT_EQ(method1.value()->GetRawName(), "a");
    ASSERT_EQ(method1.value()->GetFullyQualifiedName(), "set_name_test.a:void;");

    const auto method2 = fileView.Get<abckit_wrapper::Method>("set_name_test.Ns1.foo2:void;");
    ASSERT_TRUE(method2.has_value());

    ASSERT_TRUE(method2.value()->SetName("a"));
    ASSERT_EQ(method2.value()->GetName(), "a:void;");
    ASSERT_EQ(method2.value()->GetRawName(), "a");
    ASSERT_EQ(method2.value()->GetFullyQualifiedName(), "set_name_test.Ns1.a:void;");
}

/**
 * @tc.name: function_set_name_002
 * @tc.desc: test async function set name
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestSetName, function_set_name_002, TestSize.Level0)
{
    const auto method1 = fileView.Get<abckit_wrapper::Method>("set_name_test.foo5:std.core.Promise;");
    ASSERT_TRUE(method1.has_value());

    ASSERT_TRUE(method1.value()->SetName("New"));
    ASSERT_EQ(method1.value()->GetName(), "New:std.core.Promise;");
}

/**
 * @tc.name: class_set_name_001
 * @tc.desc: test class set name
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestSetName, class_set_name_001, TestSize.Level0)
{
    const auto class1 = fileView.Get<abckit_wrapper::Class>("set_name_test.C1");
    ASSERT_TRUE(class1.has_value());

    ASSERT_TRUE(class1.value()->SetName("a"));
    ASSERT_EQ(class1.value()->GetName(), "a");
}

/**
 * @tc.name: class_set_name_002
 * @tc.desc: test abstract class set name
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestSetName, class_set_name_002, TestSize.Level0)
{
    const auto class2 = fileView.Get<abckit_wrapper::Class>("set_name_test.C2");
    ASSERT_TRUE(class2.has_value());

    class2.value()->SetName("NewAbstract");
    ASSERT_EQ(class2.value()->GetName(), "NewAbstract");
}

/**
 * @tc.name: class_set_name_003
 * @tc.desc: test class as multiple types set name
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestSetName, class_set_name_003, TestSize.Level0)
{
    const auto classA = fileView.Get<abckit_wrapper::Class>("set_name_test.ClassA");
    ASSERT_TRUE(classA.has_value());
    classA.value()->SetName("a");
    ASSERT_EQ(classA.value()->GetName(), "a");

    const auto classB = fileView.Get<abckit_wrapper::Class>("set_name_test.ClassB");
    ASSERT_TRUE(classA.has_value());
    classB.value()->SetName("b");
    ASSERT_EQ(classB.value()->GetName(), "b");

    const auto classC = fileView.Get<abckit_wrapper::Class>("set_name_test.ClassC");
    ASSERT_TRUE(classC.has_value());
    classC.value()->SetName("c");
    ASSERT_EQ(classC.value()->GetName(), "c");
}

/**
 * @tc.name: class_field_set_name_001
 * @tc.desc: test class field set name
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestSetName, class_field_set_name_001, TestSize.Level0)
{
    const auto field1 = fileView.Get<abckit_wrapper::Field>("set_name_test.C1.field1");
    ASSERT_TRUE(field1.has_value());

    ASSERT_TRUE(field1.value()->SetName("a"));
    ASSERT_EQ(field1.value()->GetName(), "a");
}

/**
 * @tc.name: interface_set_name_001
 * @tc.desc: test interface set name
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestSetName, interface_set_name_001, TestSize.Level0)
{
    const auto interface1 = fileView.Get<abckit_wrapper::Class>("set_name_test.Interface1");
    ASSERT_TRUE(interface1.has_value());

    ASSERT_TRUE(interface1.value()->SetName("a"));
    ASSERT_EQ(interface1.value()->GetName(), "a");
}

/**
 * @tc.name: interface_field_set_name_001
 * @tc.desc: test interface field set name
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestSetName, interface_field_set_name_001, TestSize.Level0)
{
    const auto field = fileView.Get<abckit_wrapper::Field>("set_name_test.Interface4.%%property-field1");
    ASSERT_TRUE(field.has_value());

    ASSERT_TRUE(field.value()->SetName("a"));
    ASSERT_EQ(field.value()->GetName(), "%%property-a");
}

/**
 * @tc.name: enum_set_name_001
 * @tc.desc: test enum set name
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestSetName, enum_set_name_001, TestSize.Level0)
{
    const auto color = fileView.Get<abckit_wrapper::Class>("set_name_test.Color");
    ASSERT_TRUE(color.has_value());

    ASSERT_TRUE(color.value()->SetName("a"));
    ASSERT_EQ(color.value()->GetName(), "a");
}

/**
 * @tc.name: enum_field_set_name_001
 * @tc.desc: test enum field set name
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestSetName, enum_field_set_name_001, TestSize.Level0)
{
    const auto field = fileView.Get<abckit_wrapper::Field>("set_name_test.Color.RED");
    ASSERT_TRUE(field.has_value());

    ASSERT_TRUE(field.value()->SetName("a"));
    ASSERT_EQ(field.value()->GetName(), "a");
}

/**
 * @tc.name: annotationInterface_set_name_001
 * @tc.desc: test annotation interface set name
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestSetName, annotationInterface_set_name_001, TestSize.Level0)
{
    const auto anno1 = fileView.Get<abckit_wrapper::AnnotationInterface>("set_name_test.Anno1");
    ASSERT_TRUE(anno1.has_value());

    ASSERT_TRUE(anno1.value()->SetName("New"));
    ASSERT_EQ(anno1.value()->GetName(), "New");
}