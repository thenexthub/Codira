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

#include <vector>

#include "assembly-parser.h"
#include "runtime/include/method.h"
#include "runtime/include/runtime.h"

#include "runtime/compiler.h"

namespace ark::test {

class CompilationStatusTest : public testing::Test {
public:
    static const size_t METHOD_COUNT = 1;
    CompilationStatusTest()
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);
        Runtime::Create(options);
        thread_ = ark::MTManagedThread::GetCurrent();
        thread_->ManagedCodeBegin();
    }

    Method *GetMethod(Class *klass, size_t num);
    Class *GetClass();

    ~CompilationStatusTest() override
    {
        thread_->ManagedCodeEnd();
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(CompilationStatusTest);
    NO_MOVE_SEMANTIC(CompilationStatusTest);

private:
    ark::MTManagedThread *thread_;
};

Method *CompilationStatusTest::GetMethod(Class *klass, size_t num)
{
    PandaStringStream ss;
    ss << "CompilationStatusTest" << num;
    Method *method = klass->GetDirectMethod(utf::CStringAsMutf8(ss.str().c_str()));
    return method;
}

Class *CompilationStatusTest::GetClass()
{
    pandasm::Parser p;

    PandaStringStream ss;

    for (size_t i = 0; i < CompilationStatusTest::METHOD_COUNT; i++) {
        ss << ".function void CompilationStatusTest" << i << "() {" << std::endl;
        ss << "    return.void" << std::endl;
        ss << "}" << std::endl;
    }

    auto source = ss.str();
    // NOLINTNEXTLINE(readability-redundant-string-cstr)
    auto res = p.Parse(source.c_str());
    auto pf = pandasm::AsmEmitter::Emit(res.Value());

    ClassLinker *classLinker = Runtime::GetCurrent()->GetClassLinker();
    classLinker->AddPandaFile(std::move(pf));
    auto *extension = classLinker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY);

    PandaString descriptor;

    return extension->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("_GLOBAL"), &descriptor));
}

TEST_F(CompilationStatusTest, Status)
{
    auto *klass = GetClass();
    ASSERT_NE(klass, nullptr);
    Method *method = GetMethod(klass, 0);
    auto status = method->GetCompilationStatus();
    ASSERT_EQ(status, Method::NOT_COMPILED);

    ASSERT_EQ(method->AtomicSetCompilationStatus(status, Method::WAITING), true);
    status = method->GetCompilationStatus();
    ASSERT_EQ(status, Method::WAITING);

    ASSERT_EQ(method->AtomicSetCompilationStatus(status, Method::COMPILATION), true);
    status = method->GetCompilationStatus();
    ASSERT_EQ(status, Method::COMPILATION);

    ASSERT_EQ(method->AtomicSetCompilationStatus(status, Method::COMPILED), true);
    status = method->GetCompilationStatus();
    ASSERT_EQ(status, Method::COMPILED);

    ASSERT_EQ(method->AtomicSetCompilationStatus(status, Method::FAILED), true);
    status = method->GetCompilationStatus();
    ASSERT_EQ(status, Method::FAILED);
}

}  // namespace ark::test
