/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>

#include "runtime/include/class-inl.h"
#include "runtime/include/coretypes/array.h"
#include "runtime/include/runtime.h"

namespace ark::coretypes::test {

class ArrayTest : public testing::Test {
public:
    ArrayTest()
    {
        // Logger::InitializeStdLogging(Logger::Level::DEBUG, Logger::Component::ALL);
        // We need to create a runtime instance to be able to create strings.
        options_.SetShouldLoadBootPandaFiles(false);
        options_.SetShouldInitializeIntrinsics(false);
        Runtime::Create(options_);
        thread_ = ark::MTManagedThread::GetCurrent();
        thread_->ManagedCodeBegin();
    }

    ~ArrayTest() override
    {
        thread_->ManagedCodeEnd();
        Runtime::Destroy();
        // Logger::Destroy();
    }

    NO_COPY_SEMANTIC(ArrayTest);
    NO_MOVE_SEMANTIC(ArrayTest);

private:
    ark::MTManagedThread *thread_;
    RuntimeOptions options_;
};

static size_t GetArrayObjectSize(ark::Class *klass, size_t n)
{
    return sizeof(Array) + klass->GetComponentSize() * n;
}

static void TestArrayObjectSize(ClassRoot classRoot, uint32_t n)
{
    std::string msg = "Test with class_root ";
    msg += std::to_string(static_cast<int>(classRoot));

    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    auto *klass = Runtime::GetCurrent()->GetClassLinker()->GetExtension(ctx)->GetClassRoot(classRoot);

    Array *array = Array::Create(klass, n);
    ASSERT_NE(array, nullptr) << msg;

    ASSERT_EQ(array->ObjectSize(klass->GetComponentSize()), GetArrayObjectSize(klass, n)) << msg;
}

TEST_F(ArrayTest, ObjectSize)
{
    // NOLINTBEGIN(readability-magic-numbers)
    TestArrayObjectSize(ClassRoot::ARRAY_U1, 10U);
    TestArrayObjectSize(ClassRoot::ARRAY_I8, 10U);
    TestArrayObjectSize(ClassRoot::ARRAY_U8, 10U);
    TestArrayObjectSize(ClassRoot::ARRAY_I16, 10U);
    TestArrayObjectSize(ClassRoot::ARRAY_U16, 10U);
    TestArrayObjectSize(ClassRoot::ARRAY_I32, 10U);
    TestArrayObjectSize(ClassRoot::ARRAY_U32, 10U);
    TestArrayObjectSize(ClassRoot::ARRAY_I64, 10U);
    TestArrayObjectSize(ClassRoot::ARRAY_U64, 10U);
    TestArrayObjectSize(ClassRoot::ARRAY_F32, 10U);
    TestArrayObjectSize(ClassRoot::ARRAY_F64, 10U);
    // NOLINTEND(readability-magic-numbers)
}

}  // namespace ark::coretypes::test
