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

class OsrCodeTest : public testing::Test {
public:
    static const size_t METHOD_COUNT = 32;
    OsrCodeTest()
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
    void CompileMethods();

    ~OsrCodeTest() override
    {
        thread_->ManagedCodeEnd();
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(OsrCodeTest);
    NO_MOVE_SEMANTIC(OsrCodeTest);

private:
    ark::MTManagedThread *thread_;
};

Method *OsrCodeTest::GetMethod(Class *klass, size_t num)
{
    PandaStringStream ss;
    ss << "f" << num;
    Method *method = klass->GetDirectMethod(utf::CStringAsMutf8(ss.str().c_str()));
    return method;
}

Class *OsrCodeTest::GetClass()
{
    pandasm::Parser p;

    PandaStringStream ss;

    for (size_t i = 0; i < OsrCodeTest::METHOD_COUNT; i++) {
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
    auto *extension = classLinker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY);

    PandaString descriptor;

    return extension->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("_GLOBAL"), &descriptor));
}

void OsrCodeTest::CompileMethods()
{
    auto *klass = GetClass();
    ASSERT_NE(klass, nullptr);

    auto *compiler = static_cast<Compiler *>(PandaVM::GetCurrent()->GetCompiler());

    for (size_t i = 0; i < OsrCodeTest::METHOD_COUNT; i++) {
        Method *method = GetMethod(klass, i);
        ASSERT_NE(method, nullptr);
        compiler->SetOsrCode(method, static_cast<void *>(method));
    }

    for (size_t i = 0; i < OsrCodeTest::METHOD_COUNT; i++) {
        Method *method = GetMethod(klass, i);
        ASSERT_NE(method, nullptr);
        ASSERT_EQ(compiler->GetOsrCode(method), static_cast<const void *>(method));
    }
}

TEST_F(OsrCodeTest, GetOsrCode)
{
    CompileMethods();
}

}  // namespace ark::test
