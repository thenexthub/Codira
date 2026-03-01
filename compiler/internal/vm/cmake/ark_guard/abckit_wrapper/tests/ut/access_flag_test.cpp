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
constexpr std::string_view ABC_FILE_PATH = ABCKIT_WRAPPER_ABC_FILE_DIR "ut/access_flag_test.abc";
abckit_wrapper::FileView fileView;
}  // namespace

static void AssertHasAccessFlags(const uint32_t accessFlags, const abckit_wrapper::AccessFlags targetAccessFlags)
{
    LOG_I << "accessFlags:" << accessFlags;
    LOG_I << "targetAccessFlags:" << targetAccessFlags;
    ASSERT_TRUE(abckit_wrapper::AccessFlagsTarget::HasAccessFlag(accessFlags, targetAccessFlags));
}

class TestAccessFlag : public ::testing::Test {
public:
    static void SetUpTestSuite()
    {
        ASSERT_SUCCESS(fileView.Init(ABC_FILE_PATH));
    }
};

/**
 * @tc.name: method_access_flags_001
 * @tc.desc: test method access flags PUBLIC PROTECTED PRIVATE INTERNAL
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestAccessFlag, method_access_flags_001, TestSize.Level4)
{
    // public
    const auto method1 =
        fileView.Get<abckit_wrapper::Method>("access_flag_test.ClassA.method1:access_flag_test.ClassA;void;");
    ASSERT_TRUE(method1.has_value());
    AssertHasAccessFlags(method1.value()->GetAccessFlags(), abckit_wrapper::AccessFlags::PUBLIC);
    // protected
    const auto method2 =
        fileView.Get<abckit_wrapper::Method>("access_flag_test.ClassA.method2:access_flag_test.ClassA;void;");
    ASSERT_TRUE(method2.has_value());
    AssertHasAccessFlags(method2.value()->GetAccessFlags(), abckit_wrapper::AccessFlags::PROTECTED);
    // private
    const auto method3 =
        fileView.Get<abckit_wrapper::Method>("access_flag_test.ClassA.method3:access_flag_test.ClassA;void;");
    ASSERT_TRUE(method3.has_value());
    AssertHasAccessFlags(method3.value()->GetAccessFlags(), abckit_wrapper::AccessFlags::PRIVATE);
    // internal
    const auto method4 =
        fileView.Get<abckit_wrapper::Method>("access_flag_test.ClassA.method4:access_flag_test.ClassA;void;");
    ASSERT_TRUE(method4.has_value());
    AssertHasAccessFlags(method4.value()->GetAccessFlags(), abckit_wrapper::AccessFlags::INTERNAL);
}

/**
 * @tc.name: method_access_flags_002
 * @tc.desc: test method access flags FINAL ABSTRACT ASYNC NATIVE
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestAccessFlag, method_access_flags_002, TestSize.Level4)
{
    // final
    const auto method0 =
        fileView.Get<abckit_wrapper::Method>("access_flag_test.ClassB.method0:access_flag_test.ClassB;void;");
    ASSERT_TRUE(method0.has_value());
    AssertHasAccessFlags(method0.value()->GetAccessFlags(), abckit_wrapper::AccessFlags::FINAL);
    // abstract
    const auto method1 =
        fileView.Get<abckit_wrapper::Method>("access_flag_test.ClassB.method1:access_flag_test.ClassB;void;");
    ASSERT_TRUE(method1.has_value());
    AssertHasAccessFlags(method1.value()->GetAccessFlags(), abckit_wrapper::AccessFlags::ABSTRACT);
    // async
    const auto method2 = fileView.Get<abckit_wrapper::Method>(
        "access_flag_test.ClassB.method2:access_flag_test.ClassB;std.core.Promise;");
    ASSERT_TRUE(method2.has_value());
    AssertHasAccessFlags(method2.value()->GetAccessFlags(), abckit_wrapper::AccessFlags::ASYNC);
    // native
    const auto method3 =
        fileView.Get<abckit_wrapper::Method>("access_flag_test.ClassB.method3:access_flag_test.ClassB;void;");
    ASSERT_TRUE(method3.has_value());
    AssertHasAccessFlags(method3.value()->GetAccessFlags(), abckit_wrapper::AccessFlags::NATIVE);
}

class TestMemberAccessFlagVisitor final : public abckit_wrapper::MemberVisitor {
public:
    bool Visit(abckit_wrapper::Member *member) override
    {
        LOG_I << "memberName:" << member->GetFullyQualifiedName();
        this->members_.emplace_back(member);
        return true;
    }

    std::vector<abckit_wrapper::Member *> members_;
};

/**
 * @tc.name: method_access_flags_003
 * @tc.desc: test member access filter the public method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestAccessFlag, method_access_flags_003, TestSize.Level4)
{
    const auto classA = fileView.Get<abckit_wrapper::Class>("access_flag_test.ClassA");
    const auto method1 =
        fileView.Get<abckit_wrapper::Method>("access_flag_test.ClassA.method1:access_flag_test.ClassA;void;");
    ASSERT_TRUE(classA.has_value());
    ASSERT_TRUE(method1.has_value());
    TestMemberAccessFlagVisitor visitor;
    // ignore static and ctor method
    ASSERT_TRUE(classA.value()->MembersAccept(visitor.Wrap<abckit_wrapper::MemberAccessFlagFilter>(
        abckit_wrapper::AccessFlags::PUBLIC, abckit_wrapper::AccessFlags::STATIC | abckit_wrapper::AccessFlags::CTOR)));
    ASSERT_TRUE(visitor.members_.size() == 1);
    ASSERT_EQ(visitor.members_[0], method1.value());
}

/**
 * @tc.name: method_access_flags_004
 * @tc.desc: test member access filter the protected method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestAccessFlag, method_access_flags_004, TestSize.Level4)
{
    const auto classA = fileView.Get<abckit_wrapper::Class>("access_flag_test.ClassA");
    const auto method2 =
        fileView.Get<abckit_wrapper::Method>("access_flag_test.ClassA.method2:access_flag_test.ClassA;void;");
    ASSERT_TRUE(classA.has_value());
    ASSERT_TRUE(method2.has_value());
    TestMemberAccessFlagVisitor visitor;
    ASSERT_TRUE(classA.value()->MembersAccept(
        visitor.Wrap<abckit_wrapper::MemberAccessFlagFilter>(abckit_wrapper::AccessFlags::PROTECTED)));
    ASSERT_TRUE(visitor.members_.size() == 1);
    ASSERT_EQ(visitor.members_[0], method2.value());
}

/**
 * @tc.name: method_access_flags_005
 * @tc.desc: test member access filter the private method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestAccessFlag, method_access_flags_005, TestSize.Level4)
{
    const auto classA = fileView.Get<abckit_wrapper::Class>("access_flag_test.ClassA");
    const auto method3 =
        fileView.Get<abckit_wrapper::Method>("access_flag_test.ClassA.method3:access_flag_test.ClassA;void;");
    ASSERT_TRUE(classA.has_value());
    ASSERT_TRUE(method3.has_value());
    TestMemberAccessFlagVisitor visitor;
    ASSERT_TRUE(classA.value()->MembersAccept(
        visitor.Wrap<abckit_wrapper::MemberAccessFlagFilter>(abckit_wrapper::AccessFlags::PRIVATE)));
    ASSERT_TRUE(visitor.members_.size() == 1);
    ASSERT_EQ(visitor.members_[0], method3.value());
}

/**
 * @tc.name: method_access_flags_006
 * @tc.desc: test member access filter the internal method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestAccessFlag, method_access_flags_006, TestSize.Level4)
{
    const auto classA = fileView.Get<abckit_wrapper::Class>("access_flag_test.ClassA");
    const auto method4 =
        fileView.Get<abckit_wrapper::Method>("access_flag_test.ClassA.method4:access_flag_test.ClassA;void;");
    ASSERT_TRUE(classA.has_value());
    ASSERT_TRUE(method4.has_value());
    TestMemberAccessFlagVisitor visitor;
    ASSERT_TRUE(classA.value()->MembersAccept(
        visitor.Wrap<abckit_wrapper::MemberAccessFlagFilter>(abckit_wrapper::AccessFlags::INTERNAL)));
    ASSERT_TRUE(visitor.members_.size() == 1);
    ASSERT_EQ(visitor.members_[0], method4.value());
}

/**
 * @tc.name: method_access_flags_007
 * @tc.desc: test method access flags CTOR(instance constructor)
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestAccessFlag, method_access_flags_007, TestSize.Level4)
{
    // ctor
    const auto _ctor_ =
        fileView.Get<abckit_wrapper::Method>("access_flag_test.ClassA._ctor_:access_flag_test.ClassA;void;");
    ASSERT_TRUE(_ctor_.has_value());
    AssertHasAccessFlags(_ctor_.value()->GetAccessFlags(), abckit_wrapper::AccessFlags::CTOR);
}

/**
 * @tc.name: method_access_flags_008
 * @tc.desc: test member access filter the _ctor_ member
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestAccessFlag, method_access_flags_008, TestSize.Level4)
{
    const auto classA = fileView.Get<abckit_wrapper::Class>("access_flag_test.ClassA");
    const auto _ctor_ =
        fileView.Get<abckit_wrapper::Method>("access_flag_test.ClassA._ctor_:access_flag_test.ClassA;void;");
    ASSERT_TRUE(classA.has_value());
    ASSERT_TRUE(_ctor_.has_value());
    TestMemberAccessFlagVisitor visitor;
    ASSERT_TRUE(classA.value()->MembersAccept(
        visitor.Wrap<abckit_wrapper::MemberAccessFlagFilter>(abckit_wrapper::AccessFlags::CTOR)));
    ASSERT_TRUE(visitor.members_.size() == 1);
    ASSERT_EQ(visitor.members_[0], _ctor_.value());
}

/**
 * @tc.name: method_access_flags_009
 * @tc.desc: test method access flags STATIC
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestAccessFlag, method_access_flags_009, TestSize.Level4)
{
    // static method
    const auto method0 = fileView.Get<abckit_wrapper::Method>("access_flag_test.ClassA.method0:void;");
    ASSERT_TRUE(method0.has_value());
    AssertHasAccessFlags(method0.value()->GetAccessFlags(), abckit_wrapper::AccessFlags::STATIC);
}

/**
 * @tc.name: method_access_flags_010
 * @tc.desc: test member access filter the static method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestAccessFlag, method_access_flags_010, TestSize.Level4)
{
    const auto classA = fileView.Get<abckit_wrapper::Class>("access_flag_test.ClassA");
    const auto method0 = fileView.Get<abckit_wrapper::Method>("access_flag_test.ClassA.method0:void;");
    ASSERT_TRUE(classA.has_value());
    ASSERT_TRUE(method0.has_value());
    TestMemberAccessFlagVisitor visitor;
    ASSERT_TRUE(classA.value()->MembersAccept(
        visitor.Wrap<abckit_wrapper::MemberAccessFlagFilter>(abckit_wrapper::AccessFlags::STATIC)));
    ASSERT_TRUE(visitor.members_.size() == 1);
    ASSERT_EQ(visitor.members_[0], method0.value());
}

/**
 * @tc.name: method_access_flags_011
 * @tc.desc: test member access filter the final method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestAccessFlag, method_access_flags_011, TestSize.Level4)
{
    const auto classB = fileView.Get<abckit_wrapper::Class>("access_flag_test.ClassB");
    const auto method0 =
        fileView.Get<abckit_wrapper::Method>("access_flag_test.ClassB.method0:access_flag_test.ClassB;void;");
    ASSERT_TRUE(classB.has_value());
    ASSERT_TRUE(method0.has_value());
    TestMemberAccessFlagVisitor visitor;
    ASSERT_TRUE(classB.value()->MembersAccept(
        visitor.Wrap<abckit_wrapper::MemberAccessFlagFilter>(abckit_wrapper::AccessFlags::FINAL)));
    ASSERT_TRUE(visitor.members_.size() == 1);
    ASSERT_EQ(visitor.members_[0], method0.value());
}

/**
 * @tc.name: method_access_flags_012
 * @tc.desc: test member access filter the abstract method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestAccessFlag, method_access_flags_012, TestSize.Level4)
{
    const auto classB = fileView.Get<abckit_wrapper::Class>("access_flag_test.ClassB");
    const auto method1 =
        fileView.Get<abckit_wrapper::Method>("access_flag_test.ClassB.method1:access_flag_test.ClassB;void;");
    ASSERT_TRUE(classB.has_value());
    ASSERT_TRUE(method1.has_value());
    TestMemberAccessFlagVisitor visitor;
    ASSERT_TRUE(classB.value()->MembersAccept(
        visitor.Wrap<abckit_wrapper::MemberAccessFlagFilter>(abckit_wrapper::AccessFlags::ABSTRACT)));
    ASSERT_TRUE(visitor.members_.size() == 1);
    ASSERT_EQ(visitor.members_[0], method1.value());
}

/**
 * @tc.name: method_access_flags_013
 * @tc.desc: test member access filter the final method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestAccessFlag, method_access_flags_013, TestSize.Level4)
{
    const auto classB = fileView.Get<abckit_wrapper::Class>("access_flag_test.ClassB");
    const auto method2 = fileView.Get<abckit_wrapper::Method>(
        "access_flag_test.ClassB.method2:access_flag_test.ClassB;std.core.Promise;");
    ASSERT_TRUE(classB.has_value());
    ASSERT_TRUE(method2.has_value());
    TestMemberAccessFlagVisitor visitor;
    ASSERT_TRUE(classB.value()->MembersAccept(
        visitor.Wrap<abckit_wrapper::MemberAccessFlagFilter>(abckit_wrapper::AccessFlags::ASYNC)));
    ASSERT_TRUE(visitor.members_.size() == 1);
    ASSERT_EQ(visitor.members_[0], method2.value());
}

/**
 * @tc.name: method_access_flags_014
 * @tc.desc: test member access filter the final method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestAccessFlag, method_access_flags_014, TestSize.Level4)
{
    const auto classB = fileView.Get<abckit_wrapper::Class>("access_flag_test.ClassB");
    const auto method3 =
        fileView.Get<abckit_wrapper::Method>("access_flag_test.ClassB.method3:access_flag_test.ClassB;void;");
    ASSERT_TRUE(classB.has_value());
    ASSERT_TRUE(method3.has_value());
    TestMemberAccessFlagVisitor visitor;
    ASSERT_TRUE(classB.value()->MembersAccept(
        visitor.Wrap<abckit_wrapper::MemberAccessFlagFilter>(abckit_wrapper::AccessFlags::NATIVE)));
    ASSERT_TRUE(visitor.members_.size() == 1);
    ASSERT_EQ(visitor.members_[0], method3.value());
}

/**
 * @tc.name: method_access_flags_015
 * @tc.desc: test method access flags CCTOR(static constructor)
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestAccessFlag, method_access_flags_015, TestSize.Level4)
{
    // cctor
    const auto _ctor_ = fileView.Get<abckit_wrapper::Method>("access_flag_test.ClassC._cctor_:void;");
    ASSERT_TRUE(_ctor_.has_value());
    AssertHasAccessFlags(_ctor_.value()->GetAccessFlags(), abckit_wrapper::AccessFlags::CCTOR);
}

/**
 * @tc.name: method_access_flags_016
 * @tc.desc: test member access filter the _cctor_ member
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestAccessFlag, method_access_flags_016, TestSize.Level4)
{
    const auto classC = fileView.Get<abckit_wrapper::Class>("access_flag_test.ClassC");
    const auto _cctor_ = fileView.Get<abckit_wrapper::Method>("access_flag_test.ClassC._cctor_:void;");
    ASSERT_TRUE(classC.has_value());
    ASSERT_TRUE(_cctor_.has_value());
    TestMemberAccessFlagVisitor visitor;
    ASSERT_TRUE(classC.value()->MembersAccept(
        visitor.Wrap<abckit_wrapper::MemberAccessFlagFilter>(abckit_wrapper::AccessFlags::CCTOR)));
    ASSERT_TRUE(visitor.members_.size() == 1);
    ASSERT_EQ(visitor.members_[0], _cctor_.value());
}

/**
 * @tc.name: field_access_flags_001
 * @tc.desc: test field access flags PUBLIC PROTECTED PRIVATE INTERNAL
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestAccessFlag, field_access_flags_001, TestSize.Level4)
{
    // public
    const auto field1 = fileView.Get<abckit_wrapper::Field>("access_flag_test.ClassC.field1");
    ASSERT_TRUE(field1.has_value());
    AssertHasAccessFlags(field1.value()->GetAccessFlags(), abckit_wrapper::AccessFlags::PUBLIC);
    // protected
    const auto field2 = fileView.Get<abckit_wrapper::Field>("access_flag_test.ClassC.field2");
    ASSERT_TRUE(field2.has_value());
    AssertHasAccessFlags(field2.value()->GetAccessFlags(), abckit_wrapper::AccessFlags::PROTECTED);
    // private
    const auto field3 = fileView.Get<abckit_wrapper::Field>("access_flag_test.ClassC.field3");
    ASSERT_TRUE(field3.has_value());
    AssertHasAccessFlags(field3.value()->GetAccessFlags(), abckit_wrapper::AccessFlags::PRIVATE);
    // internal
    const auto field4 = fileView.Get<abckit_wrapper::Field>("access_flag_test.ClassC.field4");
    ASSERT_TRUE(field4.has_value());
    AssertHasAccessFlags(field4.value()->GetAccessFlags(), abckit_wrapper::AccessFlags::INTERNAL);
}

/**
 * @tc.name: field_access_flags_002
 * @tc.desc: test method access flags STATIC
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestAccessFlag, field_access_flags_002, TestSize.Level4)
{
    // static field
    const auto field0 = fileView.Get<abckit_wrapper::Field>("access_flag_test.ClassC.field0");
    ASSERT_TRUE(field0.has_value());
    AssertHasAccessFlags(field0.value()->GetAccessFlags(), abckit_wrapper::AccessFlags::STATIC);
}

/**
 * @tc.name: field_access_flags_003
 * @tc.desc: test member access filter the public field
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestAccessFlag, field_access_flags_003, TestSize.Level4)
{
    const auto classC = fileView.Get<abckit_wrapper::Class>("access_flag_test.ClassC");
    const auto field1 = fileView.Get<abckit_wrapper::Field>("access_flag_test.ClassC.field1");
    ASSERT_TRUE(classC.has_value());
    ASSERT_TRUE(field1.has_value());
    TestMemberAccessFlagVisitor visitor;
    // ignore static field and ctor method
    ASSERT_TRUE(classC.value()->MembersAccept(visitor.Wrap<abckit_wrapper::MemberAccessFlagFilter>(
        abckit_wrapper::AccessFlags::PUBLIC, abckit_wrapper::AccessFlags::CTOR | abckit_wrapper::AccessFlags::STATIC)));
    ASSERT_TRUE(visitor.members_.size() == 1);
    ASSERT_EQ(visitor.members_[0], field1.value());
}

/**
 * @tc.name: field_access_flags_004
 * @tc.desc: test member access filter the protected field
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestAccessFlag, field_access_flags_004, TestSize.Level4)
{
    const auto classC = fileView.Get<abckit_wrapper::Class>("access_flag_test.ClassC");
    const auto field2 = fileView.Get<abckit_wrapper::Field>("access_flag_test.ClassC.field2");
    ASSERT_TRUE(classC.has_value());
    ASSERT_TRUE(field2.has_value());
    TestMemberAccessFlagVisitor visitor;
    ASSERT_TRUE(classC.value()->MembersAccept(
        visitor.Wrap<abckit_wrapper::MemberAccessFlagFilter>(abckit_wrapper::AccessFlags::PROTECTED)));
    ASSERT_TRUE(visitor.members_.size() == 1);
    ASSERT_EQ(visitor.members_[0], field2.value());
}

/**
 * @tc.name: field_access_flags_005
 * @tc.desc: test member access filter the private field
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestAccessFlag, field_access_flags_005, TestSize.Level4)
{
    const auto classC = fileView.Get<abckit_wrapper::Class>("access_flag_test.ClassC");
    const auto field3 = fileView.Get<abckit_wrapper::Field>("access_flag_test.ClassC.field3");
    ASSERT_TRUE(classC.has_value());
    ASSERT_TRUE(field3.has_value());
    TestMemberAccessFlagVisitor visitor;
    ASSERT_TRUE(classC.value()->MembersAccept(
        visitor.Wrap<abckit_wrapper::MemberAccessFlagFilter>(abckit_wrapper::AccessFlags::PRIVATE)));
    ASSERT_TRUE(visitor.members_.size() == 1);
    ASSERT_EQ(visitor.members_[0], field3.value());
}

/**
 * @tc.name: field_access_flags_006
 * @tc.desc: test member access filter the internal field
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestAccessFlag, field_access_flags_006, TestSize.Level4)
{
    const auto classC = fileView.Get<abckit_wrapper::Class>("access_flag_test.ClassC");
    const auto field4 = fileView.Get<abckit_wrapper::Field>("access_flag_test.ClassC.field4");
    ASSERT_TRUE(classC.has_value());
    ASSERT_TRUE(field4.has_value());
    TestMemberAccessFlagVisitor visitor;
    // ignore cctor
    ASSERT_TRUE(classC.value()->MembersAccept(visitor.Wrap<abckit_wrapper::MemberAccessFlagFilter>(
        abckit_wrapper::AccessFlags::INTERNAL, abckit_wrapper::AccessFlags::CCTOR)));
    ASSERT_TRUE(visitor.members_.size() == 1);
    ASSERT_EQ(visitor.members_[0], field4.value());
}

/**
 * @tc.name: field_access_flags_007
 * @tc.desc: test member access filter the static field
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestAccessFlag, field_access_flags_007, TestSize.Level4)
{
    const auto classC = fileView.Get<abckit_wrapper::Class>("access_flag_test.ClassC");
    const auto field0 = fileView.Get<abckit_wrapper::Field>("access_flag_test.ClassC.field0");
    ASSERT_TRUE(classC.has_value());
    ASSERT_TRUE(field0.has_value());
    TestMemberAccessFlagVisitor visitor;
    // ignore cctor
    ASSERT_TRUE(classC.value()->MembersAccept(visitor.Wrap<abckit_wrapper::MemberAccessFlagFilter>(
        abckit_wrapper::AccessFlags::STATIC, abckit_wrapper::AccessFlags::CCTOR)));
    ASSERT_TRUE(visitor.members_.size() == 1);
    ASSERT_EQ(visitor.members_[0], field0.value());
}

/**
 * @tc.name: class_access_flags_001
 * @tc.desc: test class access flags FINAL
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestAccessFlag, class_access_flags_001, TestSize.Level4)
{
    const auto finalClass = fileView.Get<abckit_wrapper::Class>("access_flag_test.FinalClass");
    ASSERT_TRUE(finalClass.has_value());
    AssertHasAccessFlags(finalClass.value()->GetAccessFlags(), abckit_wrapper::AccessFlags::FINAL);
}

/**
 * @tc.name: class_access_flags_002
 * @tc.desc: test class access flags ABSTRACT
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestAccessFlag, class_access_flags_002, TestSize.Level4)
{
    const auto abstractClass = fileView.Get<abckit_wrapper::Class>("access_flag_test.AbstractClass");
    ASSERT_TRUE(abstractClass.has_value());
    AssertHasAccessFlags(abstractClass.value()->GetAccessFlags(), abckit_wrapper::AccessFlags::ABSTRACT);
}