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

class CompilerThreadPoolTest : public testing::Test {
public:
    static const size_t METHOD_COUNT = 32;
    CompilerThreadPoolTest()
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);
        Runtime::Create(options);
        thread_ = ark::MTManagedThread::GetCurrent();
        thread_->ManagedCodeBegin();
    }

    void SetUp() override
    {
#ifndef PANDA_COMPILER_ENABLE
        GTEST_SKIP();
#endif
    }

    ~CompilerThreadPoolTest() override
    {
        thread_->ManagedCodeEnd();
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(CompilerThreadPoolTest);
    NO_MOVE_SEMANTIC(CompilerThreadPoolTest);

private:
    ark::MTManagedThread *thread_;
};

Method *GetMethod(Class *klass, size_t num)
{
    PandaStringStream ss;
    ss << "f" << num;
    Method *method = klass->GetDirectMethod(utf::CStringAsMutf8(ss.str().c_str()));
    return method;
}

Class *GetClass()
{
    pandasm::Parser p;

    PandaStringStream ss;

    for (size_t i = 0; i < CompilerThreadPoolTest::METHOD_COUNT; i++) {
        ss << ".function void f" << i << "() {" << std::endl;
        ss << "    return.void" << std::endl;
        ss << "}" << std::endl;
    }

    auto source = ss.str();
    // NOLINTNEXTLINE(readability-redundant-string-cstr)
    auto res = p.Parse(source.c_str());
    auto pf = pandasm::AsmEmitter::Emit(res.Value());

    ClassLinker *classLinker = Runtime::GetCurrent()->GetClassLinker();
    classLinker->AddPandaFile(std::move(pf));

    PandaString descriptor;

    return classLinker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY)
        ->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("_GLOBAL"), &descriptor));
}

void CompileMethods(int initialNumberOfThreads, size_t scaledNumberOfThreads)
{
    auto *klass = GetClass();
    ASSERT_NE(klass, nullptr);

    auto *compiler = static_cast<Compiler *>(PandaVM::GetCurrent()->GetCompiler());

    compiler->ScaleThreadPool(initialNumberOfThreads);

    for (size_t i = 0; i < CompilerThreadPoolTest::METHOD_COUNT; i++) {
        Method *method = GetMethod(klass, i);
        ASSERT_NE(method, nullptr);
        compiler->CompileMethod(method, i, false, TaggedValue::Hole());
    }

    compiler->ScaleThreadPool(scaledNumberOfThreads);

    for (;;) {
        bool isCompleted = true;
        for (size_t i = 0; i < CompilerThreadPoolTest::METHOD_COUNT; i++) {
            Method *method = GetMethod(klass, i);
            if (method->GetCompilationStatus() == Method::NOT_COMPILED) {
                // In case queue was full.
                compiler->CompileMethod(method, i, false, TaggedValue::Hole());
            }
            if (method->GetCompilationStatus() != Method::COMPILED) {
                isCompleted = false;
            }
        }
        if (isCompleted) {
            break;
        }
    }
}

TEST_F(CompilerThreadPoolTest, SeveralThreads)
{
    constexpr size_t NUMBER_OF_THREADS = 8;
    CompileMethods(NUMBER_OF_THREADS, NUMBER_OF_THREADS);
}

TEST_F(CompilerThreadPoolTest, ReduceThreads)
{
    constexpr size_t NUMBER_OF_THREADS = 8;
    constexpr size_t NUMBER_OF_THREADS_SCALED = 4;
    CompileMethods(NUMBER_OF_THREADS, NUMBER_OF_THREADS_SCALED);
}

TEST_F(CompilerThreadPoolTest, IncreaseThreads)
{
    constexpr size_t NUMBER_OF_THREADS = 4;
    constexpr size_t NUMBER_OF_THREADS_SCALED = 8;
    CompileMethods(NUMBER_OF_THREADS, NUMBER_OF_THREADS_SCALED);
}

}  // namespace ark::test
