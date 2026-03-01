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

using namespace testing::ext;

namespace {
constexpr std::string_view ABC_FILE_PATH = ABCKIT_WRAPPER_ABC_FILE_DIR "ut/method_param_test.abc";
abckit_wrapper::FileView fileView;
}  // namespace

class TestBaseMethodParamVisitor : public abckit_wrapper::ParameterVisitor {
public:
    virtual void InitTargetParameterTypeNames() = 0;

    void AddTargetParameterTypeName(const std::string &name)
    {
        this->targetParameterTypeNames_.emplace_back(name);
    }

    bool Visit(abckit_wrapper::Parameter *parameter) override
    {
        this->parameters_.emplace_back(parameter);
        return true;
    }

    void Validate()
    {
        this->InitTargetParameterTypeNames();
        for (size_t i = 0; i < std::min(this->targetParameterTypeNames_.size(), this->parameters_.size()); ++i) {
            ASSERT_EQ(this->targetParameterTypeNames_[i], this->parameters_[i]->GetTypeName());
        }
    }

private:
    std::vector<abckit_wrapper::Parameter *> parameters_;
    std::vector<std::string> targetParameterTypeNames_;
};

class TestMethodParam : public ::testing::Test {
public:
    static void SetUpTestSuite()
    {
        ASSERT_SUCCESS(fileView.Init(ABC_FILE_PATH));
    }

    void TearDown() override
    {
        ASSERT_TRUE(visitor_ != nullptr);
        visitor_->Validate();
    }

protected:
    std::unique_ptr<TestBaseMethodParamVisitor> visitor_ = nullptr;
};

class TestClassMethodParamVisitor final : public TestBaseMethodParamVisitor {
public:
    void InitTargetParameterTypeNames() override
    {
        AddTargetParameterTypeName("method_param_test.ClassA");
        AddTargetParameterTypeName("i32");
        AddTargetParameterTypeName("std.core.String");
        AddTargetParameterTypeName("std.core.Array");
    }
};

/**
 * @tc.name: method_param_001
 * @tc.desc: test visit class method params
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestMethodParam, method_param_001, TestSize.Level0)
{
    const auto method1 = fileView.Get<abckit_wrapper::Method>(
        "method_param_test.ClassA.method1:method_param_test.ClassA;i32;std.core.String;std.core.Array;void;");
    ASSERT_TRUE(method1.has_value());
    visitor_ = std::make_unique<TestClassMethodParamVisitor>();
    method1.value()->ParametersAccept(*visitor_);
}

class TestFunctionParamVisitor final : public TestBaseMethodParamVisitor {
public:
    void InitTargetParameterTypeNames() override
    {
        AddTargetParameterTypeName("i32");
        AddTargetParameterTypeName("method_param_test.ClassA");
        AddTargetParameterTypeName("std.core.String");
        AddTargetParameterTypeName("std.core.Array");
    }
};

/**
 * @tc.name: method_param_002
 * @tc.desc: test visit function params
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestMethodParam, method_param_002, TestSize.Level0)
{
    const auto foo1 = fileView.Get<abckit_wrapper::Method>(
        "method_param_test.foo1:i32;method_param_test.ClassA;std.core.String;std.core.Array;void;");
    ASSERT_TRUE(foo1.has_value());
    visitor_ = std::make_unique<TestFunctionParamVisitor>();
    foo1.value()->ParametersAccept(*visitor_);
}