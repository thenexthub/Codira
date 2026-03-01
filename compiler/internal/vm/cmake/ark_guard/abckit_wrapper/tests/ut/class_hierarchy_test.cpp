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
constexpr std::string_view ABC_FILE_PATH = ABCKIT_WRAPPER_ABC_FILE_DIR "ut/class_hierarchy.abc";
abckit_wrapper::FileView fileView;
}  // namespace

/**
 * @brief Assert whether classA extends classB
 * @param classA classA
 * @param classB classB
 * @return `true` classA extends classB
 */
static void AssertIsSuperClass(const abckit_wrapper::Class *classA, const abckit_wrapper::Class *classB)
{
    ASSERT_TRUE(classA->superClass_.has_value());
    ASSERT_EQ(classA->superClass_.value(), classB);
    ASSERT_TRUE(classB->subClasses_.find(classA->GetFullyQualifiedName()) != classB->subClasses_.end());
}

/**
 * @brief Assert whether class implement the given interfaces
 * @param clazz Class
 * @param interface interface
 */
static void AssertIsImplemented(const abckit_wrapper::Class *clazz, const abckit_wrapper::Class *interface)
{
    ASSERT_TRUE(clazz->interfaces_.find(interface->GetFullyQualifiedName()) != clazz->interfaces_.end());
    ASSERT_TRUE(interface->subClasses_.find(clazz->GetFullyQualifiedName()) != interface->subClasses_.end());
}

class TestClassHierarchyVisitor : public abckit_wrapper::ClassVisitor {
public:
    virtual void InitTargetClassNames() = 0;

    void AddTargetClassName(const std::string &className)
    {
        this->targetClassNames_.insert(className);
    }

    bool Visit(abckit_wrapper::Class *clazz) override
    {
        this->classes_.emplace_back(clazz);
        return true;
    }

    void Validate()
    {
        this->InitTargetClassNames();
        for (const auto &clazz : this->classes_) {
            ASSERT_TRUE(this->targetClassNames_.find(clazz->GetFullyQualifiedName()) != this->targetClassNames_.end());
        }
    }

private:
    std::set<std::string> targetClassNames_;
    std::vector<abckit_wrapper::Class *> classes_;
};

class TestClassHierarchy : public ::testing::Test {
public:
    static void SetUpTestSuite()
    {
        ASSERT_SUCCESS(fileView.Init(ABC_FILE_PATH));
    }

    void TearDown() override
    {
        if (this->visitor_ != nullptr) {
        }
    }

protected:
    std::unique_ptr<TestClassHierarchyVisitor> visitor_ = nullptr;
};

/**
 * @tc.name: class_hierarchy_001
 * @tc.desc: test class hierarchy for super class
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestClassHierarchy, class_hierarchy_001, TestSize.Level0)
{
    // ClassA
    const auto classA = fileView.Get<abckit_wrapper::Class>("class_hierarchy.ClassA");
    ASSERT_TRUE(classA.has_value());
    // ClassB
    const auto classB = fileView.Get<abckit_wrapper::Class>("class_hierarchy.ClassB");
    ASSERT_TRUE(classB.has_value());
    // ClassC
    const auto classC = fileView.Get<abckit_wrapper::Class>("class_hierarchy.ClassC");
    ASSERT_TRUE(classC.has_value());

    AssertIsSuperClass(classB.value(), classA.value());
    AssertIsSuperClass(classC.value(), classB.value());
}

/**
 * @tc.name: class_hierarchy_002
 * @tc.desc: test class hierarchy for interfaces
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestClassHierarchy, class_hierarchy_002, TestSize.Level0)
{
    // ClassA
    const auto classA = fileView.Get<abckit_wrapper::Class>("class_hierarchy.ClassA");
    ASSERT_TRUE(classA.has_value());
    // ClassB
    const auto classB = fileView.Get<abckit_wrapper::Class>("class_hierarchy.ClassB");
    ASSERT_TRUE(classB.has_value());
    // ClassC
    const auto classC = fileView.Get<abckit_wrapper::Class>("class_hierarchy.ClassC");
    ASSERT_TRUE(classC.has_value());

    // Interface1
    const auto interface1 = fileView.Get<abckit_wrapper::Class>("class_hierarchy.Interface1");
    ASSERT_TRUE(interface1.has_value());
    // Interface2
    const auto interface2 = fileView.Get<abckit_wrapper::Class>("class_hierarchy.Interface2");
    ASSERT_TRUE(interface2.has_value());
    // ClassC
    const auto interface3 = fileView.Get<abckit_wrapper::Class>("class_hierarchy.Interface3");
    ASSERT_TRUE(interface3.has_value());

    AssertIsImplemented(classA.value(), interface1.value());
    AssertIsImplemented(classB.value(), interface2.value());
    AssertIsImplemented(classC.value(), interface3.value());
    AssertIsImplemented(interface3.value(), interface1.value());
}

class TestClassHierarchyAllVisitor final : public TestClassHierarchyVisitor {
public:
    void InitTargetClassNames() override
    {
        AddTargetClassName("class_hierarchy.ClassA");
        AddTargetClassName("class_hierarchy.ClassB");
        AddTargetClassName("class_hierarchy.ClassC");
        AddTargetClassName("class_hierarchy.Interface1");
        AddTargetClassName("class_hierarchy.Interface2");
        AddTargetClassName("class_hierarchy.Interface3");
    }
};

/**
 * @tc.name: class_hierarchy_003
 * @tc.desc: test class hierarchy visit all class
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestClassHierarchy, class_hierarchy_003, TestSize.Level0)
{
    // ClassB
    const auto clazz = fileView.Get<abckit_wrapper::Class>("class_hierarchy.ClassB");
    ASSERT_TRUE(clazz.has_value());

    visitor_ = std::make_unique<TestClassHierarchyAllVisitor>();
    clazz.value()->HierarchyAccept(*visitor_, true, true, true, true);
}

class TestClassHierarchySuperClassVisitor final : public TestClassHierarchyVisitor {
public:
    void InitTargetClassNames() override
    {
        AddTargetClassName("class_hierarchy.ClassA");
        AddTargetClassName("class_hierarchy.ClassB");
    }
};

/**
 * @tc.name: class_hierarchy_004
 * @tc.desc: test class hierarchy visit super class
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestClassHierarchy, class_hierarchy_004, TestSize.Level0)
{
    // ClassC
    const auto clazz = fileView.Get<abckit_wrapper::Class>("class_hierarchy.ClassC");
    ASSERT_TRUE(clazz.has_value());

    visitor_ = std::make_unique<TestClassHierarchySuperClassVisitor>();
    clazz.value()->HierarchyAccept(*visitor_, false, true, false, false);
}

class TestClassHierarchyInterfacesVisitor final : public TestClassHierarchyVisitor {
public:
    void InitTargetClassNames() override
    {
        AddTargetClassName("class_hierarchy.Interface1");
        AddTargetClassName("class_hierarchy.Interface2");
        AddTargetClassName("class_hierarchy.Interface3");
    }
};

/**
 * @tc.name: class_hierarchy_005
 * @tc.desc: test class hierarchy visit interfaces
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestClassHierarchy, class_hierarchy_005, TestSize.Level0)
{
    // ClassC
    const auto clazz = fileView.Get<abckit_wrapper::Class>("class_hierarchy.ClassC");
    ASSERT_TRUE(clazz.has_value());

    visitor_ = std::make_unique<TestClassHierarchyInterfacesVisitor>();
    clazz.value()->HierarchyAccept(*visitor_, false, false, true, false);
}

class TestClassHierarchySubClassesVisitor final : public TestClassHierarchyVisitor {
public:
    void InitTargetClassNames() override
    {
        AddTargetClassName("class_hierarchy.ClassB");
        AddTargetClassName("class_hierarchy.ClassC");
    }
};

/**
 * @tc.name: class_hierarchy_006
 * @tc.desc: test class hierarchy visit subClasses
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestClassHierarchy, class_hierarchy_006, TestSize.Level0)
{
    // ClassC
    const auto clazz = fileView.Get<abckit_wrapper::Class>("class_hierarchy.ClassA");
    ASSERT_TRUE(clazz.has_value());

    visitor_ = std::make_unique<TestClassHierarchySubClassesVisitor>();
    clazz.value()->HierarchyAccept(*visitor_, false, false, false, true);
}