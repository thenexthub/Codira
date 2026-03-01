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
#include "abckit_wrapper/file_view.h"
#include "test_util/assert.h"
#include "../../library/logger.h"

using namespace testing::ext;

namespace {
constexpr std::string_view ABC_FILE_PATH = ABCKIT_WRAPPER_ABC_FILE_DIR "ut/module_static.abc";
abckit_wrapper::FileView fileView;
constexpr std::string_view MODULE_NAME = "module_static";
}  // namespace

static std::string GetFullName(const std::string &objectRawName)
{
    return std::string(MODULE_NAME) + "." + objectRawName;
}

template <typename T>
static void AssertSignatureEqual(const std::string &objectRawName, const std::string &rawName,
                                 const std::string &descriptor)
{
    const auto object = fileView.Get<T>(GetFullName(objectRawName));
    ASSERT_TRUE(object.has_value());
    ASSERT_EQ(object.value()->GetRawName(), rawName);
    ASSERT_EQ(object.value()->GetDescriptor(), descriptor);
    ASSERT_EQ(object.value()->GetSignature(), rawName + descriptor);
}

class TestFileView : public ::testing::Test {
public:
    static void SetUpTestSuite()
    {
        ASSERT_SUCCESS(fileView.Init(ABC_FILE_PATH));
    }

    template <typename T>
    static void ValidateModuleObject(const std::string &objectRawName)
    {
        // 1. get module
        const auto &module = fileView.GetModule(MODULE_NAME.data());
        ASSERT_TRUE(module.has_value());

        // 2. build object full name
        const auto objectName = GetFullName(objectRawName);
        LOG_I << "objectName:" << objectName;

        // 3. validate exists object
        ASSERT_TRUE(fileView.GetObject(objectName).has_value());
        ASSERT_TRUE(fileView.Get<T>(objectName).has_value());
        ASSERT_TRUE(module.value()->Get<T>(objectName).has_value());

        // 4. validate not exists object
        ASSERT_FALSE(fileView.Get<T>("aaa.bbb").has_value());
        ASSERT_FALSE(module.value()->Get<T>("bbb.bcc").has_value());
    }
};

/**
 * @tc.name: fileView_namespace_001
 * @tc.desc: test fileView get namespace of module and namespace
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestFileView, fileView_namespace_001, TestSize.Level0)
{
    ValidateModuleObject<abckit_wrapper::Namespace>("Ns1");
    ValidateModuleObject<abckit_wrapper::Namespace>("Ns1.Ns2");
}

/**
 * @tc.name: fileView_function_001
 * @tc.desc: test fileView get function of module and namespace
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestFileView, fileView_function_001, TestSize.Level0)
{
    ValidateModuleObject<abckit_wrapper::Method>("foo1:i32;i32;void;");
    ValidateModuleObject<abckit_wrapper::Method>("Ns1.foo2:i32;i32;void;");
}

/**
 * @tc.name: fileView_function_002
 * @tc.desc: test fileView get function signature
 * @tc.require:
 */
HWTEST_F(TestFileView, fileView_function_002, TestSize.Level0)
{
    AssertSignatureEqual<abckit_wrapper::Method>("foo1:i32;i32;void;", "foo1", "(i32, i32)void");
    AssertSignatureEqual<abckit_wrapper::Method>("Ns1.foo2:i32;i32;void;", "foo2", "(i32, i32)void");
}

/**
 * @tc.name: fileView_module_field_001
 * @tc.desc: test fileView get module field of module and namespace
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestFileView, fileView_module_field_001, TestSize.Level0)
{
    ValidateModuleObject<abckit_wrapper::Field>("m1");
    ValidateModuleObject<abckit_wrapper::Field>("Ns1.m2");
}

/**
 * @tc.name: fileView_module_field_002
 * @tc.desc: test fileView get module field signature
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestFileView, fileView_module_field_002, TestSize.Level0)
{
    AssertSignatureEqual<abckit_wrapper::Field>("m1", "m1", "");
    AssertSignatureEqual<abckit_wrapper::Field>("Ns1.m2", "m2", "");
}

/**
 * @tc.name: fileView_class_001
 * @tc.desc: test fileView get class of module and namespace
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestFileView, fileView_class_001, TestSize.Level0)
{
    ValidateModuleObject<abckit_wrapper::Class>("ClassA");
    ValidateModuleObject<abckit_wrapper::Class>("Ns1.ClassB");
}

/**
 * @tc.name: fileView_class_002
 * @tc.desc: test fileView get class methods of module and namespace
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestFileView, fileView_class_002, TestSize.Level0)
{
    ValidateModuleObject<abckit_wrapper::Method>("ClassA.method1:module_static.ClassA;void;");
    ValidateModuleObject<abckit_wrapper::Method>("ClassA.%%get-iField1:module_static.ClassA;i32;");
    ValidateModuleObject<abckit_wrapper::Method>("ClassA.%%set-iField1:module_static.ClassA;i32;void;");

    ValidateModuleObject<abckit_wrapper::Method>("Ns1.ClassB.method2:module_static.Ns1.ClassB;void;");
    ValidateModuleObject<abckit_wrapper::Method>("Ns1.ClassB.%%get-iField2:module_static.Ns1.ClassB;std.core.String;");
    ValidateModuleObject<abckit_wrapper::Method>(
        "Ns1.ClassB.%%set-iField2:module_static.Ns1.ClassB;std.core.String;void;");
}

/**
 * @tc.name: fileView_class_003
 * @tc.desc: test fileView get class fields of module and namespace
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestFileView, fileView_class_003, TestSize.Level0)
{
    ValidateModuleObject<abckit_wrapper::Field>("ClassA.sField1");
    ValidateModuleObject<abckit_wrapper::Field>("ClassA.field1");
    ValidateModuleObject<abckit_wrapper::Field>("ClassA.%%property-iField1");

    ValidateModuleObject<abckit_wrapper::Field>("Ns1.ClassB.sField2");
    ValidateModuleObject<abckit_wrapper::Field>("Ns1.ClassB.field2");
    ValidateModuleObject<abckit_wrapper::Field>("Ns1.ClassB.%%property-iField2");
}

/**
 * @tc.name: fileView_class_004
 * @tc.desc: test fileView get class methods signature
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestFileView, fileView_class_004, TestSize.Level0)
{
    AssertSignatureEqual<abckit_wrapper::Method>("ClassA.sMethod1:i32;void;", "sMethod1", "(i32)void");
    AssertSignatureEqual<abckit_wrapper::Method>("ClassA.method1:module_static.ClassA;void;", "method1", "()void");
    AssertSignatureEqual<abckit_wrapper::Method>("ClassA.%%get-iField1:module_static.ClassA;i32;", "iField1", "");
    AssertSignatureEqual<abckit_wrapper::Method>("ClassA.%%set-iField1:module_static.ClassA;i32;void;", "iField1", "");

    AssertSignatureEqual<abckit_wrapper::Method>("Ns1.ClassB.sMethod2:i32;void;", "sMethod2", "(i32)void");
    AssertSignatureEqual<abckit_wrapper::Method>("Ns1.ClassB.method2:module_static.Ns1.ClassB;void;", "method2",
                                                 "()void");
    AssertSignatureEqual<abckit_wrapper::Method>("Ns1.ClassB.%%get-iField2:module_static.Ns1.ClassB;std.core.String;",
                                                 "iField2", "");
    AssertSignatureEqual<abckit_wrapper::Method>(
        "Ns1.ClassB.%%set-iField2:module_static.Ns1.ClassB;std.core.String;void;", "iField2", "");
}

/**
 * @tc.name: fileView_class_005
 * @tc.desc: test fileView get class fields signature
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestFileView, fileView_class_005, TestSize.Level0)
{
    AssertSignatureEqual<abckit_wrapper::Field>("ClassA.sField1", "sField1", "");
    AssertSignatureEqual<abckit_wrapper::Field>("ClassA.field1", "field1", "");
    AssertSignatureEqual<abckit_wrapper::Field>("ClassA.%%property-iField1", "iField1", "");

    AssertSignatureEqual<abckit_wrapper::Field>("Ns1.ClassB.sField2", "sField2", "");
    AssertSignatureEqual<abckit_wrapper::Field>("Ns1.ClassB.field2", "field2", "");
    AssertSignatureEqual<abckit_wrapper::Field>("Ns1.ClassB.%%property-iField2", "iField2", "");
}

/**
 * @tc.name: fileView_interface_001
 * @tc.desc: test fileView get interface of module and namespace
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestFileView, fileView_interface_001, TestSize.Level0)
{
    ValidateModuleObject<abckit_wrapper::Class>("Interface1");
    ValidateModuleObject<abckit_wrapper::Class>("Ns1.Interface2");
}

/**
 * @tc.name: fileView_interface_002
 * @tc.desc: test fileView get interface methods of module and namespace
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestFileView, fileView_interface_002, TestSize.Level0)
{
    ValidateModuleObject<abckit_wrapper::Method>("Interface1.iMethod1:module_static.Interface1;void;");
    ValidateModuleObject<abckit_wrapper::Method>("Ns1.Interface2.iMethod2:module_static.Ns1.Interface2;void;");
}

/**
 * @tc.name: fileView_interface_003
 * @tc.desc: test fileView get interface fields of module and namespace
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestFileView, fileView_interface_003, TestSize.Level0)
{
    ValidateModuleObject<abckit_wrapper::Field>("Interface1.%%property-iField1");
    ValidateModuleObject<abckit_wrapper::Field>("Ns1.Interface2.%%property-iField2");
}

/**
 * @tc.name: fileView_enum_001
 * @tc.desc: test fileView get enum of module and namespace
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestFileView, fileView_enum_001, TestSize.Level0)
{
    ValidateModuleObject<abckit_wrapper::Class>("Color1");
    ValidateModuleObject<abckit_wrapper::Class>("Ns1.Color2");
}

/**
 * @tc.name: fileView_enum_002
 * @tc.desc: test fileView get enum fields of module and namespace
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestFileView, fileView_enum_002, TestSize.Level0)
{
    ValidateModuleObject<abckit_wrapper::Field>("Color1.RED");
    ValidateModuleObject<abckit_wrapper::Field>("Ns1.Color2.YELLOW");
}

/**
 * @tc.name: fileView_annotation_interface_001
 * @tc.desc: test fileView get annotationInterface of module and namespace
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestFileView, fileView_annotation_interface_001, TestSize.Level0)
{
    ValidateModuleObject<abckit_wrapper::AnnotationInterface>("MyAnno1");
    ValidateModuleObject<abckit_wrapper::AnnotationInterface>("Ns1.MyAnno2");
}