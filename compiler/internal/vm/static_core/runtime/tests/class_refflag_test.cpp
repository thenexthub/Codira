/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include <memory>
#include "assembler/assembly-parser.h"
#include "runtime/core/core_class_linker_extension.h"
#include "runtime/include/runtime.h"

namespace ark::test {

class ClassRefFlagTest : public testing::Test {
public:
    ClassRefFlagTest()
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);
        options.SetGcType("epsilon");
        // NOLINTNEXTLINE(readability-magic-numbers)
        options.SetHeapSizeLimit(64_MB);
        Runtime::Create(options);
        thread_ = ark::ManagedThread::GetCurrent();
        thread_->ManagedCodeBegin();
    }

    ~ClassRefFlagTest() override
    {
        thread_->ManagedCodeEnd();
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(ClassRefFlagTest);
    NO_MOVE_SEMANTIC(ClassRefFlagTest);

protected:
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    ark::ManagedThread *thread_;

    std::unique_ptr<ClassLinker> CreateClassLinker(ManagedThread *thread)
    {
        std::vector<std::unique_ptr<ClassLinkerExtension>> extensions;
        extensions.push_back(std::make_unique<CoreClassLinkerExtension>());
        auto allocator = thread->GetVM()->GetHeapManager()->GetInternalAllocator();
        auto classLinker = std::make_unique<ClassLinker>(allocator, std::move(extensions));
        if (!classLinker->Initialize()) {
            return nullptr;
        }

        return classLinker;
    }
};

TEST_F(ClassRefFlagTest, TEST_REFFLAGS)
{
    auto source = R"(
        .record REF {}
        .record A1 {}
        .record A2 <extends=A1> { i32 a }
        .record A3 <extends=A2> { f32 a}
        .record A4 <extends=A3> {}
        .record A5 <extends=A4> { REF a }
        .record A6 <extends=A5> {}
        .record A7 <extends=A6> { i32 a }
        .record B1 { REF b}
        .record B2  <extends=B1> { }
    )";

    auto res = pandasm::Parser {}.Parse(source);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());

    auto classLinker = CreateClassLinker(thread_);
    ASSERT_NE(classLinker, nullptr);

    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    auto *ext = classLinker->GetExtension(ctx);

    classLinker->AddPandaFile(std::move(pf));

    auto getClass = [&ext](const char *className) {
        PandaString descriptor;
        auto klass = ext->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8(className), &descriptor));
        if (klass == nullptr) {
            UNREACHABLE();
        }
        return klass;
    };

    ASSERT_TRUE(getClass("REF")->HaveNoRefsInParents());
    ASSERT_TRUE(getClass("REF")->GetBase()->HaveNoRefsInParents());  // Object class always MUST has this flag
    ASSERT_TRUE(getClass("A1")->HaveNoRefsInParents());
    ASSERT_TRUE(getClass("A2")->HaveNoRefsInParents());
    ASSERT_TRUE(getClass("A3")->HaveNoRefsInParents());
    ASSERT_TRUE(getClass("A4")->HaveNoRefsInParents());
    ASSERT_TRUE(getClass("A5")->HaveNoRefsInParents());
    ASSERT_FALSE(getClass("A6")->HaveNoRefsInParents());
    ASSERT_FALSE(getClass("A7")->HaveNoRefsInParents());
    ASSERT_TRUE(getClass("B1")->HaveNoRefsInParents());
    ASSERT_FALSE(getClass("B2")->HaveNoRefsInParents());
}

}  // namespace ark::test
