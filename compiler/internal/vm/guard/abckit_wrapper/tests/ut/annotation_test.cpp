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
constexpr std::string_view ABC_FILE_PATH = ABCKIT_WRAPPER_ABC_FILE_DIR "ut/annotation_test.abc";
abckit_wrapper::FileView fileView;
}  // namespace

class TestBaseAnnotationVisitor : public abckit_wrapper::AnnotationVisitor {
public:
    void AddAnnotationName(const std::string &annotationName)
    {
        this->targetAnnotationNames_.insert(annotationName);
    }

    virtual void InitTargetAnnotationNames() = 0;

    bool Visit(abckit_wrapper::Annotation *annotation) override
    {
        LOG_I << "AnnotationName:" << annotation->GetFullyQualifiedName();
        this->annotations_.emplace_back(annotation);
        return true;
    }

    void Validate()
    {
        this->InitTargetAnnotationNames();
        for (const auto &annotation : this->annotations_) {
            ASSERT_TRUE(this->targetAnnotationNames_.find(annotation->GetName()) != this->targetAnnotationNames_.end());
        }
    }

private:
    std::vector<abckit_wrapper::Annotation *> annotations_;
    std::set<std::string> targetAnnotationNames_;
};

class TestAnnotation : public ::testing::Test {
public:
    static void SetUpTestSuite()
    {
        ASSERT_SUCCESS(fileView.Init(ABC_FILE_PATH));
    }

    void TearDown() override
    {
        ASSERT_TRUE(this->visitor_ != nullptr);
        this->visitor_->Validate();
    }

protected:
    std::unique_ptr<TestBaseAnnotationVisitor> visitor_ = nullptr;
};

class TestClassAnnotationVisitor final : public TestBaseAnnotationVisitor {
public:
    void InitTargetAnnotationNames() override
    {
        AddAnnotationName("MyAnno1");
        AddAnnotationName("MyAnno2");
    }
};

/**
 * @tc.name: annotation_001
 * @tc.desc: test visit class annotation
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestAnnotation, annotation_001, TestSize.Level0)
{
    const auto clazz = fileView.Get<abckit_wrapper::Class>("annotation_test.ClassA");
    ASSERT_TRUE(clazz.has_value());

    visitor_ = std::make_unique<TestClassAnnotationVisitor>();
    clazz.value()->AnnotationsAccept(*visitor_);
}

class TestInterfaceAnnotationVisitor final : public TestBaseAnnotationVisitor {
public:
    void InitTargetAnnotationNames() override
    {
        AddAnnotationName("MyAnno1");
        AddAnnotationName("MyAnno2");
    }
};

/**
 * @tc.name: annotation_002
 * @tc.desc: test visit interface annotation
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestAnnotation, annotation_002, TestSize.Level0)
{
    const auto interface1 = fileView.Get<abckit_wrapper::Class>("annotation_test.Interface1");
    ASSERT_TRUE(interface1.has_value());

    visitor_ = std::make_unique<TestInterfaceAnnotationVisitor>();
    interface1.value()->AnnotationsAccept(*visitor_);
}

class TestClassFieldAnnotationVisitor final : public TestBaseAnnotationVisitor {
public:
    void InitTargetAnnotationNames() override
    {
        AddAnnotationName("MyAnno1");
    }
};

/**
 * @tc.name: annotation_003
 * @tc.desc: test visit class field annotation
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestAnnotation, annotation_003, TestSize.Level0)
{
    const auto field1 = fileView.Get<abckit_wrapper::Field>("annotation_test.ClassA.field1");
    ASSERT_TRUE(field1.has_value());

    visitor_ = std::make_unique<TestClassFieldAnnotationVisitor>();
    field1.value()->AnnotationsAccept(*visitor_);
}

class TestClassMethodAnnotationVisitor final : public TestBaseAnnotationVisitor {
public:
    void InitTargetAnnotationNames() override
    {
        AddAnnotationName("MyAnno1");
    }
};

/**
 * @tc.name: annotation_004
 * @tc.desc: test visit class method annotation
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestAnnotation, annotation_004, TestSize.Level0)
{
    const auto method1 =
        fileView.Get<abckit_wrapper::Method>("annotation_test.ClassA.method1:annotation_test.ClassA;void;");
    ASSERT_TRUE(method1.has_value());

    visitor_ = std::make_unique<TestClassMethodAnnotationVisitor>();
    method1.value()->AnnotationsAccept(*visitor_);
}

class TestInterfaceFieldAnnotationVisitor final : public TestBaseAnnotationVisitor {
public:
    void InitTargetAnnotationNames() override
    {
        AddAnnotationName("MyAnno1");
    }
};

/**
 * @tc.name: annotation_005
 * @tc.desc: test visit interface field annotation
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestAnnotation, annotation_005, TestSize.Level0)
{
    const auto field1 = fileView.Get<abckit_wrapper::Field>("annotation_test.Interface1.%%property-iField1");
    ASSERT_TRUE(field1.has_value());

    visitor_ = std::make_unique<TestInterfaceFieldAnnotationVisitor>();
    field1.value()->AnnotationsAccept(*visitor_);
}

class TestInterfaceMethodAnnotationVisitor final : public TestBaseAnnotationVisitor {
public:
    void InitTargetAnnotationNames() override
    {
        AddAnnotationName("MyAnno1");
    }
};

/**
 * @tc.name: annotation_006
 * @tc.desc: test visit interface method annotation
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestAnnotation, annotation_006, TestSize.Level0)
{
    const auto method1 =
        fileView.Get<abckit_wrapper::Method>("annotation_test.Interface1.iMethod1:annotation_test.Interface1;void;");
    ASSERT_TRUE(method1.has_value());

    visitor_ = std::make_unique<TestInterfaceMethodAnnotationVisitor>();
    method1.value()->AnnotationsAccept(*visitor_);
}