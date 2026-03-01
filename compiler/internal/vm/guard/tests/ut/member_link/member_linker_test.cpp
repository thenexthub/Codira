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

#include "abckit_wrapper/hierarchy_dumper.h"

#include "error.h"
#include "logger.h"
#include "obfuscator/member_linker.h"

#include "test_util/assert.h"

using namespace testing::ext;

template <typename T>
static void ValidateLastMember(abckit_wrapper::FileView &fileView, const std::string &memberName,
                               abckit_wrapper::Member *targetLastMember)
{
    auto member = fileView.Get<T>(memberName);
    ASSERT_TRUE(member.has_value()) << "memberName:" << memberName << " does not exist";

    auto lastMember = ark::guard::MemberLinker::LastMember(member.value());
    ASSERT_EQ(lastMember, targetLastMember) << "expected:" << lastMember->GetFullyQualifiedName()
                                            << ", but found:" << targetLastMember->GetFullyQualifiedName();
}

class TestMemberVisitor final : public abckit_wrapper::MemberVisitor {
public:
    bool Visit(abckit_wrapper::Member *member) override
    {
        LOG_I << "[Test] Member:" << member->GetFullyQualifiedName();
        LOG_I << "[Test] descriptor:" << member->GetDescriptor();

        size_t i = 1;

        auto nextMember = ark::guard::MemberLinker::NextMember(member);
        while (nextMember) {
            LOG_I << "[Test] member[" << i++ << "]:" << nextMember->GetFullyQualifiedName() << " "
                  << member->GetDescriptor();

            nextMember = ark::guard::MemberLinker::NextMember(nextMember);
        }

        return true;
    }
};

/**
 * @tc.name: member_linker_001
 * @tc.desc: test link class associated method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(MemberLinkerTest, member_linker_test_001, TestSize.Level1)
{
    abckit_wrapper::FileView fileView;
    constexpr std::string_view abcFilePath =
        GUARD_ABC_FILE_DIR "ut/member_link/module_member_linker_method_test.abc";
    ASSERT_ABCKIT_WRAPPER_SUCCESS(fileView.Init(abcFilePath));
    ark::guard::MemberLinker memberLinker(fileView);
    ASSERT_GUARD_SUCCESS(memberLinker.Link());

    // print next members
    TestMemberVisitor visitor;
    ASSERT_TRUE(fileView.ModulesAccept(visitor.Wrap<abckit_wrapper::ClassMemberVisitor>()
                                           .Wrap<abckit_wrapper::ModuleClassVisitor>()
                                           .Wrap<abckit_wrapper::LocalModuleFilter>()));

    // validate last member
    const auto method1OfClassA = fileView.Get<abckit_wrapper::Method>(
        "module_member_linker_method_test.ClassA.method1:module_member_linker_method_test.ClassA;void;");
    ASSERT_TRUE(method1OfClassA.has_value());
    auto method1OfClassB = fileView.Get<abckit_wrapper::Method>(
        "module_member_linker_method_test.ClassB.method1:module_member_linker_method_test.ClassB;void;");
    ASSERT_TRUE(method1OfClassB.has_value());

    // Interface1 method1
    ValidateLastMember<abckit_wrapper::Method>(
        fileView,
        "module_member_linker_method_test.Interface1.method1:module_member_linker_method_test.Interface1;void;",
        method1OfClassA.value());

    // Interface2 method1
    ValidateLastMember<abckit_wrapper::Method>(
        fileView,
        "module_member_linker_method_test.Interface2.method1:module_member_linker_method_test.Interface2;void;",
        method1OfClassB.value());
}

/**
 * @tc.name: member_linker_002
 * @tc.desc: test link class associated field
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(MemberLinkerTest, member_linker_test_002, TestSize.Level1)
{
    abckit_wrapper::FileView fileView;
    constexpr std::string_view abcFilePath =
        GUARD_ABC_FILE_DIR "ut/member_link/module_member_linker_field_test.abc";
    ASSERT_ABCKIT_WRAPPER_SUCCESS(fileView.Init(abcFilePath));
    ark::guard::MemberLinker memberLinker(fileView);
    ASSERT_GUARD_SUCCESS(memberLinker.Link());

    // print next members
    TestMemberVisitor visitor;
    ASSERT_TRUE(fileView.ModulesAccept(visitor.Wrap<abckit_wrapper::ClassMemberVisitor>()
                                           .Wrap<abckit_wrapper::ModuleClassVisitor>()
                                           .Wrap<abckit_wrapper::LocalModuleFilter>()));

    // validate last member
    const auto field1SetterOfClassA = fileView.Get<abckit_wrapper::Method>(
        "module_member_linker_field_test.ClassA.%%set-field1:module_member_linker_field_test.ClassA;i32;void;");
    ASSERT_TRUE(field1SetterOfClassA.has_value());

    // ClassA get field1
    ValidateLastMember<abckit_wrapper::Method>(
        fileView, "module_member_linker_field_test.ClassA.%%get-field1:module_member_linker_field_test.ClassA;i32;",
        field1SetterOfClassA.value());
    // ClassA field1
    ValidateLastMember<abckit_wrapper::Field>(fileView, "module_member_linker_field_test.ClassA.%%property-field1",
                                              field1SetterOfClassA.value());
    // Interface1 get field1
    ValidateLastMember<abckit_wrapper::Method>(
        fileView,
        "module_member_linker_field_test.Interface1.%%get-field1:module_member_linker_field_test.Interface1;i32;",
        field1SetterOfClassA.value());
    // Interface1 set field1
    ValidateLastMember<abckit_wrapper::Method>(
        fileView,
        "module_member_linker_field_test.Interface1.%%set-field1:module_member_linker_field_test.Interface1;i32;void;",
        field1SetterOfClassA.value());
    // Interface1 field1
    ValidateLastMember<abckit_wrapper::Field>(fileView, "module_member_linker_field_test.Interface1.%%property-field1",
                                              field1SetterOfClassA.value());

    const auto field1OfClassC = fileView.Get<abckit_wrapper::Field>("module_member_linker_field_test.ClassC.field1");
    ASSERT_TRUE(field1OfClassC.has_value());

    // ClassB field1
    ValidateLastMember<abckit_wrapper::Field>(fileView, "module_member_linker_field_test.ClassB.%%property-field1",
                                              field1OfClassC.value());
    // ClassB set field1
    ValidateLastMember<abckit_wrapper::Method>(
        fileView,
        "module_member_linker_field_test.ClassB.%%set-field1:module_member_linker_field_test.ClassB;i32;void;",
        field1OfClassC.value());
    // ClassB get field1
    ValidateLastMember<abckit_wrapper::Method>(
        fileView, "module_member_linker_field_test.ClassB.%%get-field1:module_member_linker_field_test.ClassB;i32;",
        field1OfClassC.value());
    // Interface2 field1
    ValidateLastMember<abckit_wrapper::Field>(fileView, "module_member_linker_field_test.Interface2.%%property-field1",
                                              field1OfClassC.value());
    // Interface2 set field1
    ValidateLastMember<abckit_wrapper::Method>(
        fileView,
        "module_member_linker_field_test.Interface2.%%set-field1:module_member_linker_field_test.Interface2;i32;void;",
        field1OfClassC.value());
    // Interface2 get field1
    ValidateLastMember<abckit_wrapper::Method>(
        fileView,
        "module_member_linker_field_test.Interface2.%%get-field1:module_member_linker_field_test.Interface2;i32;",
        field1OfClassC.value());
}

/**
 * @tc.name: member_linker_003
 * @tc.desc: test namespace link class associated method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(MemberLinkerTest, member_linker_test_003, TestSize.Level1)
{
    abckit_wrapper::FileView fileView;
    constexpr std::string_view abcFilePath = GUARD_ABC_FILE_DIR "ut/member_link/ns_member_linker_method_test.abc";
    ASSERT_ABCKIT_WRAPPER_SUCCESS(fileView.Init(abcFilePath));
    ark::guard::MemberLinker memberLinker(fileView);
    ASSERT_GUARD_SUCCESS(memberLinker.Link());

    // print next members
    TestMemberVisitor visitor;
    ASSERT_TRUE(fileView.ModulesAccept(visitor.Wrap<abckit_wrapper::ClassMemberVisitor>()
                                           .Wrap<abckit_wrapper::ModuleClassVisitor>()
                                           .Wrap<abckit_wrapper::LocalModuleFilter>()));

    // validate last member
    const auto method1OfClassA = fileView.Get<abckit_wrapper::Method>(
        "ns_member_linker_method_test.Ns1.ClassA.method1:ns_member_linker_method_test.Ns1.ClassA;void;");
    ASSERT_TRUE(method1OfClassA.has_value());
    auto method1OfClassB = fileView.Get<abckit_wrapper::Method>(
        "ns_member_linker_method_test.Ns1.ClassB.method1:ns_member_linker_method_test.Ns1.ClassB;void;");
    ASSERT_TRUE(method1OfClassB.has_value());

    // Interface1 method1
    ValidateLastMember<abckit_wrapper::Method>(
        fileView,
        "ns_member_linker_method_test.Ns1.Interface1.method1:ns_member_linker_method_test.Ns1.Interface1;void;",
        method1OfClassA.value());

    // Interface2 method1
    ValidateLastMember<abckit_wrapper::Method>(
        fileView,
        "ns_member_linker_method_test.Ns1.Interface2.method1:ns_member_linker_method_test.Ns1.Interface2;void;",
        method1OfClassB.value());
}

/**
 * @tc.name: member_linker_004
 * @tc.desc: test namespace link class associated field
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(MemberLinkerTest, member_linker_test_004, TestSize.Level1)
{
    abckit_wrapper::FileView fileView;
    constexpr std::string_view abcFilePath = GUARD_ABC_FILE_DIR "ut/member_link/ns_member_linker_field_test.abc";
    ASSERT_ABCKIT_WRAPPER_SUCCESS(fileView.Init(abcFilePath));
    ark::guard::MemberLinker memberLinker(fileView);
    ASSERT_GUARD_SUCCESS(memberLinker.Link());

    // print next members
    TestMemberVisitor visitor;
    ASSERT_TRUE(fileView.ModulesAccept(visitor.Wrap<abckit_wrapper::ClassMemberVisitor>()
                                           .Wrap<abckit_wrapper::ModuleClassVisitor>()
                                           .Wrap<abckit_wrapper::LocalModuleFilter>()));

    // validate last member
    const auto field1SetterOfClassA = fileView.Get<abckit_wrapper::Method>(
        "ns_member_linker_field_test.Ns1.ClassA.%%set-field1:ns_member_linker_field_test.Ns1.ClassA;i32;void;");
    ASSERT_TRUE(field1SetterOfClassA.has_value());

    // ClassA get field1
    ValidateLastMember<abckit_wrapper::Method>(
        fileView, "ns_member_linker_field_test.Ns1.ClassA.%%get-field1:ns_member_linker_field_test.Ns1.ClassA;i32;",
        field1SetterOfClassA.value());
    // ClassA field1
    ValidateLastMember<abckit_wrapper::Field>(fileView, "ns_member_linker_field_test.Ns1.ClassA.%%property-field1",
                                              field1SetterOfClassA.value());
    // Interface1 get field1
    ValidateLastMember<abckit_wrapper::Method>(
        fileView,
        "ns_member_linker_field_test.Ns1.Interface1.%%get-field1:ns_member_linker_field_test.Ns1.Interface1;i32;",
        field1SetterOfClassA.value());
    // Interface1 set field1
    ValidateLastMember<abckit_wrapper::Method>(
        fileView,
        "ns_member_linker_field_test.Ns1.Interface1.%%set-field1:ns_member_linker_field_test.Ns1.Interface1;i32;void;",
        field1SetterOfClassA.value());
    // Interface1 field1
    ValidateLastMember<abckit_wrapper::Field>(fileView, "ns_member_linker_field_test.Ns1.Interface1.%%property-field1",
                                              field1SetterOfClassA.value());

    const auto field1OfClassC = fileView.Get<abckit_wrapper::Field>("ns_member_linker_field_test.Ns1.ClassC.field1");
    ASSERT_TRUE(field1OfClassC.has_value());

    // ClassB field1
    ValidateLastMember<abckit_wrapper::Field>(fileView, "ns_member_linker_field_test.Ns1.ClassB.%%property-field1",
                                              field1OfClassC.value());
    // ClassB set field1
    ValidateLastMember<abckit_wrapper::Method>(
        fileView,
        "ns_member_linker_field_test.Ns1.ClassB.%%set-field1:ns_member_linker_field_test.Ns1.ClassB;i32;void;",
        field1OfClassC.value());
    // ClassB get field1
    ValidateLastMember<abckit_wrapper::Method>(
        fileView, "ns_member_linker_field_test.Ns1.ClassB.%%get-field1:ns_member_linker_field_test.Ns1.ClassB;i32;",
        field1OfClassC.value());
    // Interface2 field1
    ValidateLastMember<abckit_wrapper::Field>(fileView, "ns_member_linker_field_test.Ns1.Interface2.%%property-field1",
                                              field1OfClassC.value());
    // Interface2 set field1
    ValidateLastMember<abckit_wrapper::Method>(
        fileView,
        "ns_member_linker_field_test.Ns1.Interface2.%%set-field1:ns_member_linker_field_test.Ns1.Interface2;i32;void;",
        field1OfClassC.value());
    // Interface2 get field1
    ValidateLastMember<abckit_wrapper::Method>(
        fileView,
        "ns_member_linker_field_test.Ns1.Interface2.%%get-field1:ns_member_linker_field_test.Ns1.Interface2;i32;",
        field1OfClassC.value());
}