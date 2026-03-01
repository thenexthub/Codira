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

#include "configuration/configuration_constants.h"
#include "configuration/keep_option_parser.h"

using namespace testing::ext;

/**
 * @tc.name: keep_option_parser_test_001
 * @tc.desc: verify whether the class_specification with wildcard member(*) is successfully parsed.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(KeepOptionParserTest, keep_option_parser_test_001, TestSize.Level0)
{
    static const std::string keepStr = R"(-keep @AnnotationA !final class classA extends @AnnotationB classB {
        *;
    })";

    ark::guard::KeepOptionParser parser(keepStr);
    auto info = parser.Parse();
    ASSERT_TRUE(info.has_value());
    const auto &specification = info.value();
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
    ASSERT_EQ(specification.fieldSpecifications_[0].annotationName_, "");
    ASSERT_EQ(specification.fieldSpecifications_[0].setAccessFlags_, 0);
    ASSERT_EQ(specification.fieldSpecifications_[0].unSetAccessFlags_, 0);
    ASSERT_EQ(specification.fieldSpecifications_[0].memberName_, "");
    ASSERT_EQ(specification.fieldSpecifications_[0].memberType_, "");
    ASSERT_EQ(specification.fieldSpecifications_[0].memberValue_, "");

    ASSERT_EQ(specification.methodSpecifications_.size(), 1);
    ASSERT_EQ(specification.methodSpecifications_[0].keepMember_, true);
    ASSERT_EQ(specification.methodSpecifications_[0].annotationName_, "");
    ASSERT_EQ(specification.methodSpecifications_[0].setAccessFlags_, 0);
    ASSERT_EQ(specification.methodSpecifications_[0].unSetAccessFlags_, 0);
    ASSERT_EQ(specification.methodSpecifications_[0].memberName_, "");
    ASSERT_EQ(specification.methodSpecifications_[0].memberType_, "");
    ASSERT_EQ(specification.methodSpecifications_[0].memberValue_, "");
}

/**
 * @tc.name: keep_option_parser_test_002
 * @tc.desc: verify whether the class_specification with wildcard member(<fields> | <methods>) is successfully parsed.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(KeepOptionParserTest, keep_option_parser_test_002, TestSize.Level0)
{
    static const std::string keepStr = R"(-keep @AnnotationA !final class classA extends @AnnotationB classB {
        @AnnotationC
        <fields>;

        @AnnotationD
        <methods>;
    })";

    ark::guard::KeepOptionParser parser(keepStr);
    auto info = parser.Parse();
    ASSERT_TRUE(info.has_value());
    const auto &specification = info.value();
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
    ASSERT_EQ(specification.fieldSpecifications_[0].annotationName_, "AnnotationC");
    ASSERT_EQ(specification.fieldSpecifications_[0].setAccessFlags_, 0);
    ASSERT_EQ(specification.fieldSpecifications_[0].unSetAccessFlags_, 0);
    ASSERT_EQ(specification.fieldSpecifications_[0].memberName_, "");
    ASSERT_EQ(specification.fieldSpecifications_[0].memberType_, "");
    ASSERT_EQ(specification.fieldSpecifications_[0].memberValue_, "");

    ASSERT_EQ(specification.methodSpecifications_.size(), 1);
    ASSERT_EQ(specification.methodSpecifications_[0].keepMember_, true);
    ASSERT_EQ(specification.methodSpecifications_[0].annotationName_, "AnnotationD");
    ASSERT_EQ(specification.methodSpecifications_[0].setAccessFlags_, 0);
    ASSERT_EQ(specification.methodSpecifications_[0].unSetAccessFlags_, 0);
    ASSERT_EQ(specification.methodSpecifications_[0].memberName_, "");
    ASSERT_EQ(specification.methodSpecifications_[0].memberType_, "");
    ASSERT_EQ(specification.methodSpecifications_[0].memberValue_, "");
}

/**
 * @tc.name: keep_option_parser_test_003
 * @tc.desc: verify whether the class_specification with different type declaration and access flags is successfully
 * parsed.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(KeepOptionParserTest, keep_option_parser_test_003, TestSize.Level0)
{
    static const std::vector<std::string> keepOptionsStrList = {R"(-keep interface xxx {})", R"(-keep class xxx {})",
                                                                R"(-keep enum xxx {})",      R"(-keep type xxx {})",
                                                                R"(-keep namespace xxx {})", R"(-keep package xxx {})"};

    static const std::vector<std::string> negatedKeepOptionsStrList = {
        R"(-keep !interface xxx {})", R"(-keep !class xxx {})",     R"(-keep !enum xxx {})",
        R"(-keep !type xxx {})",      R"(-keep !namespace xxx {})", R"(-keep !package xxx {})"};

    static const std::vector expectTypeDeclarationsList = {
        abckit_wrapper::TypeDeclarations::INTERFACE, abckit_wrapper::TypeDeclarations::CLASS,
        abckit_wrapper::TypeDeclarations::ENUM,      abckit_wrapper::TypeDeclarations::TYPE,
        abckit_wrapper::TypeDeclarations::NAMESPACE, abckit_wrapper::TypeDeclarations::PACKAGE};

    for (size_t i = 0; i < keepOptionsStrList.size(); i++) {
        ark::guard::KeepOptionParser parser(keepOptionsStrList[i]);
        auto info = parser.Parse();
        ASSERT_TRUE(info.has_value());
        const auto &specification = info.value();
        ASSERT_EQ(specification.setTypeDeclarations_, expectTypeDeclarationsList[i]);
        ASSERT_EQ(specification.unSetTypeDeclarations_, 0);
    }

    for (size_t i = 0; i < negatedKeepOptionsStrList.size(); i++) {
        ark::guard::KeepOptionParser parser(negatedKeepOptionsStrList[i]);
        auto info = parser.Parse();
        ASSERT_TRUE(info.has_value());
        const auto &specification = info.value();
        ASSERT_EQ(specification.setTypeDeclarations_, 0);
        ASSERT_EQ(specification.unSetTypeDeclarations_, expectTypeDeclarationsList[i]);
    }
}

/**
 * @tc.name: keep_option_parser_test_004
 * @tc.desc: verify whether the class_specification with multiple extension name is successfully parsed.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(KeepOptionParserTest, keep_option_parser_test_004, TestSize.Level0)
{
    static const std::vector<std::string> keepOptionsStrList = {R"(-keep class xxx1 extends xxx2, xxx3)",
                                                                R"(-keep class xxx1 implements xxx2, xxx3)"};

    static const std::vector expectExtensionTypeList = {abckit_wrapper::ExtensionType::EXTENDS,
                                                        abckit_wrapper::ExtensionType::IMPLEMENTS};

    for (size_t i = 0; i < keepOptionsStrList.size(); i++) {
        ark::guard::KeepOptionParser parser(keepOptionsStrList[i]);
        auto info = parser.Parse();
        ASSERT_TRUE(info.has_value());
        const auto &specification = info.value();
        ASSERT_EQ(specification.setTypeDeclarations_, abckit_wrapper::TypeDeclarations::CLASS);
        ASSERT_EQ(specification.unSetTypeDeclarations_, 0);
        ASSERT_EQ(specification.className_, "xxx1");
        ASSERT_EQ(specification.extensionType_, expectExtensionTypeList[i]);
        ASSERT_EQ(specification.extensionClassName_, "xxx2,xxx3");
    }
}

/**
 * @tc.name: keep_option_parser_test_005
 * @tc.desc: verify whether the class_specification with regular field is successfully parsed.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(KeepOptionParserTest, keep_option_parser_test_005, TestSize.Level0)
{
    static const std::string keepStr = R"(-keep class classA {
        @Annotation1
        public static field1: std.core.String = 'test';

        protected field2 = 1;
    })";

    ark::guard::KeepOptionParser parser(keepStr);
    auto info = parser.Parse();
    ASSERT_TRUE(info.has_value());
    const auto &specification = info.value();
    ASSERT_EQ(specification.annotationName_, "");
    ASSERT_EQ(specification.setAccessFlags_, 0);
    ASSERT_EQ(specification.unSetAccessFlags_, 0);
    ASSERT_EQ(specification.setTypeDeclarations_, abckit_wrapper::TypeDeclarations::CLASS);
    ASSERT_EQ(specification.unSetTypeDeclarations_, 0);
    ASSERT_EQ(specification.className_, "classA");
    ASSERT_EQ(specification.extensionType_, 0);
    ASSERT_EQ(specification.extensionAnnotationName_, "");
    ASSERT_EQ(specification.extensionClassName_, "");

    ASSERT_EQ(specification.fieldSpecifications_.size(), 2);
    ASSERT_EQ(specification.fieldSpecifications_[0].keepMember_, false);
    ASSERT_EQ(specification.fieldSpecifications_[0].annotationName_, "Annotation1");
    ASSERT_EQ(specification.fieldSpecifications_[0].setAccessFlags_,
              abckit_wrapper::AccessFlags::PUBLIC | abckit_wrapper::AccessFlags::STATIC);
    ASSERT_EQ(specification.fieldSpecifications_[0].unSetAccessFlags_, 0);
    ASSERT_EQ(specification.fieldSpecifications_[0].memberName_, "field1");
    ASSERT_EQ(specification.fieldSpecifications_[0].memberType_, "std\\.core\\.String");
    ASSERT_EQ(specification.fieldSpecifications_[0].memberValue_, "test");

    ASSERT_EQ(specification.fieldSpecifications_[1].keepMember_, false);
    ASSERT_EQ(specification.fieldSpecifications_[1].annotationName_, "");
    ASSERT_EQ(specification.fieldSpecifications_[1].setAccessFlags_, abckit_wrapper::AccessFlags::PROTECTED);
    ASSERT_EQ(specification.fieldSpecifications_[1].unSetAccessFlags_, 0);
    ASSERT_EQ(specification.fieldSpecifications_[1].memberName_, "field2");
    ASSERT_EQ(specification.fieldSpecifications_[1].memberType_, "");
    ASSERT_EQ(specification.fieldSpecifications_[1].memberValue_, "1");

    ASSERT_EQ(specification.methodSpecifications_.size(), 0);
}

/**
 * @tc.name: keep_option_parser_test_006
 * @tc.desc: verify whether the class_specification with regular method is successfully parsed.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(KeepOptionParserTest, keep_option_parser_test_006, TestSize.Level0)
{
    static const std::string keepStr = R"(-keep class classA {
        public async doAction(std.core.String, f64): void;
    })";

    ark::guard::KeepOptionParser parser(keepStr);
    auto info = parser.Parse();
    ASSERT_TRUE(info.has_value());
    const auto &specification = info.value();
    ASSERT_EQ(specification.annotationName_, "");
    ASSERT_EQ(specification.setAccessFlags_, 0);
    ASSERT_EQ(specification.unSetAccessFlags_, 0);
    ASSERT_EQ(specification.setTypeDeclarations_, abckit_wrapper::TypeDeclarations::CLASS);
    ASSERT_EQ(specification.unSetTypeDeclarations_, 0);
    ASSERT_EQ(specification.className_, "classA");
    ASSERT_EQ(specification.extensionType_, 0);
    ASSERT_EQ(specification.extensionAnnotationName_, "");
    ASSERT_EQ(specification.extensionClassName_, "");

    ASSERT_EQ(specification.fieldSpecifications_.size(), 0);

    ASSERT_EQ(specification.methodSpecifications_.size(), 1);
    ASSERT_EQ(specification.methodSpecifications_[0].keepMember_, false);
    ASSERT_EQ(specification.methodSpecifications_[0].annotationName_, "");
    ASSERT_EQ(specification.methodSpecifications_[0].setAccessFlags_,
              abckit_wrapper::AccessFlags::PUBLIC | abckit_wrapper::AccessFlags::ASYNC);
    ASSERT_EQ(specification.methodSpecifications_[0].unSetAccessFlags_, 0);
    ASSERT_EQ(specification.methodSpecifications_[0].memberName_, "doAction");
    ASSERT_EQ(specification.methodSpecifications_[0].memberType_, "std\\.core\\.String,f64");
    ASSERT_EQ(specification.methodSpecifications_[0].memberValue_, "void");
}

/**
 * @tc.name: keep_option_parser_test_007
 * @tc.desc: verify whether the class_specification is empty is parsed failed.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(KeepOptionParserTest, keep_option_parser_test_007, TestSize.Level0)
{
    ark::guard::KeepOptionParser parser("");
    auto info = parser.Parse();
    ASSERT_FALSE(info.has_value());
}

/**
 * @tc.name: keep_option_parser_test_008
 * @tc.desc: verify whether the wildcard is parsed to regex.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(KeepOptionParserTest, keep_option_parser_test_008, TestSize.Level0)
{
    static const std::vector<std::string> keepStrList = {R"(-keep class Class?)", R"(-keep class ark.*)",
                                                         R"(-keep class **)", R"(-keep class ark.*Foo<2>)",
                                                         R"(-keep class *)"};
    static const std::vector<std::string> classNameList = {"Class[^/\\.]", "ark\\.[^/\\.]*", ".*", "ark\\.[^/\\.]*Foo",
                                                           "[^/\\.]*"};
    for (size_t i = 0; i < keepStrList.size(); ++i) {
        ark::guard::KeepOptionParser parser(keepStrList[i]);
        auto info = parser.Parse();
        ASSERT_TRUE(info.has_value());
        const auto &specification = info.value();
        ASSERT_EQ(specification.annotationName_, "");
        ASSERT_EQ(specification.setAccessFlags_, 0);
        ASSERT_EQ(specification.unSetAccessFlags_, 0);
        ASSERT_EQ(specification.setTypeDeclarations_, abckit_wrapper::TypeDeclarations::CLASS);
        ASSERT_EQ(specification.unSetTypeDeclarations_, 0);
        ASSERT_EQ(specification.className_, classNameList[i]);
        ASSERT_EQ(specification.extensionType_, 0);
        ASSERT_EQ(specification.extensionAnnotationName_, "");
        ASSERT_EQ(specification.extensionClassName_, "");
    }
}

/**
 * @tc.name: keep_option_parser_test_009
 * @tc.desc: verify whether the type is transform right
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(KeepOptionParserTest, keep_option_parser_test_009, TestSize.Level0)
{
    static const std::vector<std::string> keepStrList = {
        R"(-keep class ClassA {
                public field: i32;
                public foo(i32, i32): i32;
            })",
        R"(-keep class ClassA {
                public field: i8;
                public foo(i8, i8): i8;
            })",
        R"(-keep class ClassA {
                public field: i16;
                public foo(i16, i16): i16;
            })",
        R"(-keep class ClassA {
                public field: i64;
                public foo(i64, i64): i64;
            })",
        R"(-keep class ClassA {
                public field: f64;
                public foo(f64, f64): f64;
            })",
        R"(-keep class ClassA {
                public field: f32;
                public foo(f32, f32): f32;
            })",
        R"(-keep class ClassA {
                public field: f64;
                public foo(f64, f64): f64;
            })",
        R"(-keep class ClassA {
                public field: u16;
                public foo(u16, u16): u16;
            })",
        R"(-keep class ClassA {
                public field: u1;
                public foo(u1, u1): u1;
            })",
        R"(-keep class ClassA {
                public field: std.core.String;
                public foo(std.core.String, std.core.String): std.core.String;
            })",
        R"(-keep class ClassA {
                public field: std.core.BigInt;
                public foo(std.core.BigInt, std.core.BigInt): std.core.BigInt;
            })",
        R"(-keep class ClassA {
                public field: std.core.Null;
                public foo(std.core.Null, std.core.Null): std.core.Null;
            })",
        R"(-keep class ClassA {
                public field: std.core.Object;
                public foo(std.core.Object, std.core.Object): std.core.Object;
            })"};

    static const std::vector<std::string> expectType = {"i32",
                                                        "i8",
                                                        "i16",
                                                        "i64",
                                                        "f64",
                                                        "f32",
                                                        "f64",
                                                        "u16",
                                                        "u1",
                                                        "std\\.core\\.String",
                                                        "std\\.core\\.BigInt",
                                                        "std\\.core\\.Null",
                                                        "std\\.core\\.Object"};

    for (size_t i = 0; i < keepStrList.size(); ++i) {
        ark::guard::KeepOptionParser parser(keepStrList[i]);
        auto info = parser.Parse();
        ASSERT_TRUE(info.has_value());

        const auto &specification = info.value();
        ASSERT_EQ(specification.className_, "ClassA");

        ASSERT_EQ(specification.fieldSpecifications_.size(), 1);
        ASSERT_EQ(specification.fieldSpecifications_[0].memberType_, expectType[i]);

        ASSERT_EQ(specification.methodSpecifications_.size(), 1);
        auto expectParam = std::string(expectType[i]) + "," + std::string(expectType[i]);
        ASSERT_EQ(specification.methodSpecifications_[0].memberType_, expectParam);
        ASSERT_EQ(specification.methodSpecifications_[0].memberValue_, expectType[i]);
    }
}

/**
 * @tc.name: keep_option_parser_test_010
 * @tc.desc: verify whether the keep enum field
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(KeepOptionParserTest, keep_option_parser_test_010, TestSize.Level0)
{
    std::string keepStr = R"(-keep enum Color {
                                RED;
                            })";

    ark::guard::KeepOptionParser parser(keepStr);
    auto info = parser.Parse();
    ASSERT_TRUE(info.has_value());
    const auto &specification = info.value();
    ASSERT_EQ(specification.className_, "Color");

    ASSERT_EQ(specification.fieldSpecifications_.size(), 1);
    ASSERT_EQ(specification.fieldSpecifications_[0].keepMember_, false);
    ASSERT_EQ(specification.fieldSpecifications_[0].annotationName_, "");
    ASSERT_EQ(specification.fieldSpecifications_[0].setAccessFlags_, 0);
    ASSERT_EQ(specification.fieldSpecifications_[0].unSetAccessFlags_, 0);
    ASSERT_EQ(specification.fieldSpecifications_[0].memberName_, "RED");
    ASSERT_EQ(specification.fieldSpecifications_[0].memberType_, "");
    ASSERT_EQ(specification.fieldSpecifications_[0].memberValue_, "");

    ASSERT_TRUE(specification.methodSpecifications_.empty());
}

/**
 * @tc.name: keep_option_parser_test_011
 * @tc.desc: verify whether the keep union field
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(KeepOptionParserTest, keep_option_parser_test_011, TestSize.Level0)
{
    std::string keepStr = R"(-keep class ClassB {
                                public field: {UClassA, std.core.String};
                            })";

    ark::guard::KeepOptionParser parser(keepStr);
    auto info = parser.Parse();
    ASSERT_TRUE(info.has_value());
    const auto &specification = info.value();
    ASSERT_EQ(specification.className_, "ClassB");

    ASSERT_EQ(specification.fieldSpecifications_.size(), 1);
    ASSERT_EQ(specification.fieldSpecifications_[0].memberName_, "field");
    ASSERT_EQ(specification.fieldSpecifications_[0].memberType_, "{UClassA,std\\.core\\.String}");
}

/**
 * @tc.name: keep_option_parser_test_012
 * @tc.desc: verify whether the keep union method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(KeepOptionParserTest, keep_option_parser_test_012, TestSize.Level0)
{
    std::string keepStr = R"(-keep class ClassC {
                                public method1(i32, ClassA):ClassB;
                                public method2(i32, {UClassA, std.core.String}):{UClassB, std.core.String};
                                public method3(i32, std.core.Array):ClassB;
                            })";

    ark::guard::KeepOptionParser parser(keepStr);
    auto info = parser.Parse();
    ASSERT_TRUE(info.has_value());
    const auto &specification = info.value();
    ASSERT_EQ(specification.className_, "ClassC");

    ASSERT_EQ(specification.methodSpecifications_.size(), 3);
    for (const auto &spec : specification.methodSpecifications_) {
        if (spec.memberName_ == "method1") {
            ASSERT_EQ(spec.memberType_, "i32,ClassA");
            ASSERT_EQ(spec.memberValue_, "ClassB");
        } else if (spec.memberName_ == "method2") {
            ASSERT_EQ(spec.memberType_, "i32,{UClassA,std\\.core\\.String}");
            ASSERT_EQ(spec.memberValue_, "{UClassB,std\\.core\\.String}");
        } else if (spec.memberName_ == "method3") {
            ASSERT_EQ(spec.memberType_, "i32,std\\.core\\.Array");
            ASSERT_EQ(spec.memberValue_, "ClassB");
        }
    }
}

/**
 * @tc.name: keep_option_parser_test_013
 * @tc.desc: verify whether the class_specification with final field is successfully parsed.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(KeepOptionParserTest, keep_option_parser_test_013, TestSize.Level0)
{
    static const std::string keepStr = R"(-keep class classA {
        @Annotation1
        public final field1: std.core.String = 'test';
    })";

    ark::guard::KeepOptionParser parser(keepStr);
    auto info = parser.Parse();
    ASSERT_TRUE(info.has_value());
    const auto &specification = info.value();
    ASSERT_EQ(specification.annotationName_, "");
    ASSERT_EQ(specification.setAccessFlags_, 0);
    ASSERT_EQ(specification.unSetAccessFlags_, 0);
    ASSERT_EQ(specification.setTypeDeclarations_, abckit_wrapper::TypeDeclarations::CLASS);
    ASSERT_EQ(specification.unSetTypeDeclarations_, 0);
    ASSERT_EQ(specification.className_, "classA");
    ASSERT_EQ(specification.extensionType_, 0);
    ASSERT_EQ(specification.extensionAnnotationName_, "");
    ASSERT_EQ(specification.extensionClassName_, "");

    ASSERT_EQ(specification.fieldSpecifications_.size(), 1);
    ASSERT_EQ(specification.fieldSpecifications_[0].keepMember_, false);
    ASSERT_EQ(specification.fieldSpecifications_[0].annotationName_, "Annotation1");
    ASSERT_EQ(specification.fieldSpecifications_[0].setAccessFlags_,
              abckit_wrapper::AccessFlags::PUBLIC | abckit_wrapper::AccessFlags::FINAL);
    ASSERT_EQ(specification.fieldSpecifications_[0].unSetAccessFlags_, 0);
    ASSERT_EQ(specification.fieldSpecifications_[0].memberName_, "field1");
    ASSERT_EQ(specification.fieldSpecifications_[0].memberType_, "std\\.core\\.String");
    ASSERT_EQ(specification.fieldSpecifications_[0].memberValue_, "test");

    ASSERT_EQ(specification.methodSpecifications_.size(), 0);
}

/**
 * @tc.name: keep_option_parser_test_014
 * @tc.desc: verify whether the class_specification with lambda field and tuple field is successfully parsed.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(KeepOptionParserTest, keep_option_parser_test_014, TestSize.Level0)
{
    static const std::string keepStr = R"(-keep class classA {
        public field1: std.core.Function1;
        public field2: std.core.FunctionR2;
        public field3: std.core.Tuple3;
    })";

    ark::guard::KeepOptionParser parser(keepStr);
    auto info = parser.Parse();
    ASSERT_TRUE(info.has_value());
    const auto &specification = info.value();
    ASSERT_EQ(specification.annotationName_, "");
    ASSERT_EQ(specification.setAccessFlags_, 0);
    ASSERT_EQ(specification.unSetAccessFlags_, 0);
    ASSERT_EQ(specification.setTypeDeclarations_, abckit_wrapper::TypeDeclarations::CLASS);
    ASSERT_EQ(specification.unSetTypeDeclarations_, 0);
    ASSERT_EQ(specification.className_, "classA");
    ASSERT_EQ(specification.extensionType_, 0);
    ASSERT_EQ(specification.extensionAnnotationName_, "");
    ASSERT_EQ(specification.extensionClassName_, "");

    ASSERT_EQ(specification.fieldSpecifications_.size(), 3);
    ASSERT_EQ(specification.fieldSpecifications_[0].keepMember_, false);
    ASSERT_EQ(specification.fieldSpecifications_[0].annotationName_, "");
    ASSERT_EQ(specification.fieldSpecifications_[0].setAccessFlags_, abckit_wrapper::AccessFlags::PUBLIC);
    ASSERT_EQ(specification.fieldSpecifications_[0].unSetAccessFlags_, 0);
    ASSERT_EQ(specification.fieldSpecifications_[0].memberName_, "field1");
    ASSERT_EQ(specification.fieldSpecifications_[0].memberType_, "std\\.core\\.Function1");
    ASSERT_EQ(specification.fieldSpecifications_[1].keepMember_, false);
    ASSERT_EQ(specification.fieldSpecifications_[1].annotationName_, "");
    ASSERT_EQ(specification.fieldSpecifications_[1].setAccessFlags_, abckit_wrapper::AccessFlags::PUBLIC);
    ASSERT_EQ(specification.fieldSpecifications_[1].unSetAccessFlags_, 0);
    ASSERT_EQ(specification.fieldSpecifications_[1].memberName_, "field2");
    ASSERT_EQ(specification.fieldSpecifications_[1].memberType_, "std\\.core\\.FunctionR2");
    ASSERT_EQ(specification.fieldSpecifications_[2].keepMember_, false);
    ASSERT_EQ(specification.fieldSpecifications_[2].annotationName_, "");
    ASSERT_EQ(specification.fieldSpecifications_[2].setAccessFlags_, abckit_wrapper::AccessFlags::PUBLIC);
    ASSERT_EQ(specification.fieldSpecifications_[2].unSetAccessFlags_, 0);
    ASSERT_EQ(specification.fieldSpecifications_[2].memberName_, "field3");
    ASSERT_EQ(specification.fieldSpecifications_[2].memberType_, "std\\.core\\.Tuple3");

    ASSERT_EQ(specification.methodSpecifications_.size(), 0);
}

/**
 * @tc.name: keep_option_parser_test_015
 * @tc.desc: verify whether the class_specification className field with '@' character
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(KeepOptionParserTest, keep_option_parser_test_015, TestSize.Level0)
{
    std::vector<std::string> keepStrs = {
        R"(-keep class entry.src.main.AAA extends entry.src.main.BBB { *; })",
        R"(-keep class entry.src.main.AAA extends @AnnotationTest entry.src.main.BBB { *; })",
        R"(-keep class entry.src.main.AAA extends @entry.src.main.BBB { *; })",
        R"(-keep class entry.src.main.AAA extends @AnnotationTest @entry.src.main.BBB { *; })",
        R"(-keep class @entry.src.main.AAA extends entry.src.main.BBB { *; })",
        R"(-keep class @entry.src.main.AAA extends @AnnotationTest entry.src.main.BBB { *; })",
        R"(-keep class @entry.src.main.AAA extends @entry.src.main.BBB { *; })",
        R"(-keep class @entry.src.main.AAA extends @AnnotationTest @entry.src.main.BBB { *; })"};
    std::vector<std::string> expectClassNames = {R"(entry\.src\.main\.AAA)",  R"(entry\.src\.main\.AAA)",
                                                 R"(entry\.src\.main\.AAA)",  R"(entry\.src\.main\.AAA)",
                                                 R"(@entry\.src\.main\.AAA)", R"(@entry\.src\.main\.AAA)",
                                                 R"(@entry\.src\.main\.AAA)", R"(@entry\.src\.main\.AAA)"};
    std::vector<std::string> extensionAnnotations = {"", "AnnotationTest", "", "AnnotationTest",
                                                     "", "AnnotationTest", "", "AnnotationTest"};
    std::vector<std::string> extensionClassName = {R"(entry\.src\.main\.BBB)",  R"(entry\.src\.main\.BBB)",
                                                   R"(@entry\.src\.main\.BBB)", R"(@entry\.src\.main\.BBB)",
                                                   R"(entry\.src\.main\.BBB)",  R"(entry\.src\.main\.BBB)",
                                                   R"(@entry\.src\.main\.BBB)", R"(@entry\.src\.main\.BBB)"};

    ASSERT_EQ(keepStrs.size(), extensionAnnotations.size());
    ASSERT_EQ(keepStrs.size(), extensionClassName.size());
    for (size_t i = 0; i < keepStrs.size(); ++i) {
        ark::guard::KeepOptionParser parser(keepStrs[i]);
        auto info = parser.Parse();
        ASSERT_TRUE(info.has_value());
        const auto &specification = info.value();

        ASSERT_EQ(specification.className_, expectClassNames[i]);
        ASSERT_EQ(specification.extensionAnnotationName_, extensionAnnotations[i]);
        ASSERT_EQ(specification.extensionClassName_, extensionClassName[i]);
    }
}