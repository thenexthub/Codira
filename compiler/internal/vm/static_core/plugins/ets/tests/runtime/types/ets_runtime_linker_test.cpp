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

#include "assembly-emitter.h"
#include "assembly-parser.h"
#include "types/ets_runtime_linker.h"
#include "plugins/ets/tests/runtime/types/ets_test_mirror_classes.h"

#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "runtime/include/class_linker.h"
#include "libarkbase/utils/utf.h"

#include "ets_coroutine.h"
#include "ets_vm.h"

namespace ark::ets::test {

class EtsRuntimeLinkerTest : public testing::Test {
public:
    EtsRuntimeLinkerTest()
    {
        options_.SetShouldLoadBootPandaFiles(true);
        options_.SetShouldInitializeIntrinsics(true);
        options_.SetCompilerEnableJit(false);
        options_.SetGcType("epsilon");
        options_.SetLoadRuntimes({"ets"});

        auto stdlib = std::getenv("PANDA_STD_LIB");
        if (stdlib == nullptr) {
            std::cerr << "PANDA_STD_LIB env variable should be set and point to mock_stdlib.abc" << std::endl;
            std::abort();
        }
        options_.SetBootPandaFiles({stdlib});

        Runtime::Create(options_);
    }

    ~EtsRuntimeLinkerTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(EtsRuntimeLinkerTest);
    NO_MOVE_SEMANTIC(EtsRuntimeLinkerTest);

    void SetUp() override
    {
        coroutine_ = EtsCoroutine::GetCurrent();
        vm_ = coroutine_->GetPandaVM();
        coroutine_->ManagedCodeBegin();
    }

    void TearDown() override
    {
        coroutine_->ManagedCodeEnd();
    }

    static std::vector<MirrorFieldInfo> GetClassMembers()
    {
        return std::vector<MirrorFieldInfo> {
            MIRROR_FIELD_INFO(EtsRuntimeLinker, classLinkerCtxPtr_, "classLinkerCtxPtr")};
    }

protected:
    PandaEtsVM *vm_ = nullptr;  // NOLINT(misc-non-private-member-variables-in-classes)

private:
    RuntimeOptions options_;
    EtsCoroutine *coroutine_ = nullptr;
};

TEST_F(EtsRuntimeLinkerTest, RuntimeLinkerMemoryLayout)
{
    EtsClass *runtimeLinkerClass = PlatformTypes(vm_)->coreRuntimeLinker;
    MirrorFieldInfo::CompareMemberOffsets(runtimeLinkerClass, GetClassMembers());
}

class SuppressErrorHandler : public ark::ClassLinkerErrorHandler {
    void OnError([[maybe_unused]] ark::ClassLinker::Error error, const ark::PandaString &message) override
    {
        message_ = message;
    }

public:
    const ark::PandaString &GetMessage() const
    {
        return message_;
    }

private:
    ark::PandaString message_;
};

TEST_F(EtsRuntimeLinkerTest, GetClassReturnsNullWhenErrorSuppressed)
{
    auto *ext = static_cast<ark::ets::EtsClassLinkerExtension *>(
        ark::Runtime::GetCurrent()->GetClassLinker()->GetExtension(ark::panda_file::SourceLang::ETS));

    const auto *fakeDescriptor = reinterpret_cast<const uint8_t *>("Lark/test/Fake;");
    SuppressErrorHandler handler;

    Class *klass = ext->GetClass(fakeDescriptor, true, nullptr, &handler);
    ASSERT_EQ(klass, nullptr);
}

TEST_F(EtsRuntimeLinkerTest, CreateUnionClassNotRedecl)
{
    pandasm::Parser p;

    const char *source =
        ".language eTS\n"

        ".record A <ets.extends=std.core.Object, access.record=public> {}\n"
        ".record B <ets.extends=std.core.Object, access.record=public> {}\n"
        ".record C <ets.extends=B, access.record=public> {}\n"
        ".record D <ets.extends=std.core.Object, access.record=public> {}\n"
        ".record std.core.Object <external>\n"
        ".record {UA,B} <external>\n"
        ".record {UA,C} <external>\n"

        ".function void D.foo(D a0, {UA,B} a1) <access.function=public> { return.void }\n"
        ".function void D.foo(D a0, {UA,C} a1) <access.function=public> { return.void }";

    auto res = p.Parse(source);
    ASSERT_EQ(p.ShowError().err, pandasm::Error::ErrorType::ERR_NONE);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());

    auto classLinker = Runtime::GetCurrent()->GetClassLinker();
    ASSERT(classLinker);
    classLinker->AddPandaFile(std::move(pf));

    auto *ext = static_cast<ark::ets::EtsClassLinkerExtension *>(
        ark::Runtime::GetCurrent()->GetClassLinker()->GetExtension(ark::panda_file::SourceLang::ETS));
    SuppressErrorHandler handler;
    const auto *classWithError = reinterpret_cast<const uint8_t *>("LD;");
    Class *klass = ext->GetClass(classWithError, true, nullptr, &handler);
    ASSERT_NE(klass, nullptr);
    ASSERT_EQ(handler.GetMessage(), "");
}

TEST_F(EtsRuntimeLinkerTest, CreateUnionClassMultiOverriding1)
{
    pandasm::Parser p;

    const char *source =
        ".language eTS\n"

        ".record A <ets.extends=AA, access.record=public> {}\n"
        ".record AA <ets.extends=std.core.Object, access.record=public> {}\n"
        ".record B <ets.extends=BB, access.record=public> {}\n"
        ".record BB <ets.extends=std.core.Object, access.record=public> {}\n"
        ".record C <ets.extends=CC, access.record=public> {}\n"
        ".record CC <ets.extends=BB, access.record=public> {}\n"
        ".record D <ets.extends=std.core.Object, access.record=public> {}\n"
        ".record E <ets.extends=D, access.record=public> {}\n"
        ".record std.core.Object <external>\n"
        ".record {UA,C} <external>\n"
        ".record {UAA,BB} <external>\n"
        ".record {UB,C} <external>\n"
        ".function void D.foo(D a0, {UA,C} a1) <access.function=public> { return.void }\n"
        ".function void D.foo(D a0, {UB,C} a1) <access.function=public> { return.void }\n"
        ".function void E.foo(E a0, {UBB,AA} a1) <access.function=public> { return.void }\n"
        ".function void E.foo(E a0, {UB,C} a1) <access.function=public> { return.void }";

    auto res = p.Parse(source);
    ASSERT_EQ(p.ShowError().err, pandasm::Error::ErrorType::ERR_NONE);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());

    auto classLinker = Runtime::GetCurrent()->GetClassLinker();
    ASSERT(classLinker);
    classLinker->AddPandaFile(std::move(pf));

    auto *ext = static_cast<ark::ets::EtsClassLinkerExtension *>(
        ark::Runtime::GetCurrent()->GetClassLinker()->GetExtension(ark::panda_file::SourceLang::ETS));
    SuppressErrorHandler handler;
    const auto *classWithError = reinterpret_cast<const uint8_t *>("LE;");
    Class *klass = ext->GetClass(classWithError, true, nullptr, &handler);
    ASSERT_EQ(klass, nullptr);
    ASSERT_EQ(handler.GetMessage(), "Multiple override LE;foo LD;foo");
}

TEST_F(EtsRuntimeLinkerTest, CreateUnionClassMultiOverriding2)
{
    pandasm::Parser p;

    const char *source =
        ".language eTS\n"

        ".record Base <ets.extends=std.core.Object, access.record=public> {}\n"
        ".record I <ets.abstract, ets.interface, ets.extends=std.core.Object, access.record=public> {}\n"
        ".record A <ets.implements=I, ets.extends=Base, access.record=public> {}\n"
        ".record D <ets.extends=std.core.Object, access.record=public> {}\n"
        ".record E <ets.extends=D, access.record=public> {}\n"
        ".record std.core.Object <external>\n"
        ".function void D.foo(D a0, A a1) <access.function=public> { return.void }\n"
        ".function void E.foo(E a0, {UBase,I,D} a1) <access.function=public> { return.void }\n"
        ".function void E.foo(E a0, I a1) <access.function=public> { return.void }\n";
    auto res = p.Parse(source);
    ASSERT_EQ(p.ShowError().err, pandasm::Error::ErrorType::ERR_NONE);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());

    auto classLinker = Runtime::GetCurrent()->GetClassLinker();
    ASSERT(classLinker);
    classLinker->AddPandaFile(std::move(pf));

    auto *ext = static_cast<ark::ets::EtsClassLinkerExtension *>(
        ark::Runtime::GetCurrent()->GetClassLinker()->GetExtension(ark::panda_file::SourceLang::ETS));
    SuppressErrorHandler handler;
    const auto *classWithError = reinterpret_cast<const uint8_t *>("LE;");
    Class *klass = ext->GetClass(classWithError, true, nullptr, &handler);
    ASSERT_EQ(klass, nullptr);
    ASSERT_EQ(handler.GetMessage(), "Multiple override LE;foo LD;foo");
}

TEST_F(EtsRuntimeLinkerTest, CreateUnionClassMultiOverriding3)
{
    pandasm::Parser p;

    const char *source =
        ".language eTS\n"

        ".record Base <ets.extends=std.core.Object, access.record=public> {}\n"
        ".record I <ets.abstract, ets.interface, ets.extends=std.core.Object, access.record=public> {}\n"
        ".record A <ets.implements=I, ets.extends=Base, access.record=public> {}\n"
        ".record D <ets.extends=std.core.Object, access.record=public> {}\n"
        ".record E <ets.extends=D, access.record=public> {}\n"
        ".record std.core.Object <external>\n"
        ".function void D.foo(D a0, A a1) <access.function=public> { return.void }\n"
        ".function void E.foo(E a0, {UBase,I} a1) <access.function=public> { return.void }\n"
        ".function void E.foo(E a0, I a1) <access.function=public> { return.void }\n";
    auto res = p.Parse(source);
    ASSERT_EQ(p.ShowError().err, pandasm::Error::ErrorType::ERR_NONE);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());

    auto classLinker = Runtime::GetCurrent()->GetClassLinker();
    ASSERT(classLinker);
    classLinker->AddPandaFile(std::move(pf));

    auto *ext = static_cast<ark::ets::EtsClassLinkerExtension *>(
        ark::Runtime::GetCurrent()->GetClassLinker()->GetExtension(ark::panda_file::SourceLang::ETS));
    SuppressErrorHandler handler;
    const auto *classWithError = reinterpret_cast<const uint8_t *>("LE;");
    Class *klass = ext->GetClass(classWithError, true, nullptr, &handler);
    ASSERT_EQ(klass, nullptr);
    ASSERT_EQ(handler.GetMessage(), "Multiple override LE;foo LD;foo");
}

TEST_F(EtsRuntimeLinkerTest, CreateUnionClassMultiOverriding4)
{
    pandasm::Parser p;

    const char *source =
        ".language eTS\n"

        ".record Base <ets.extends=std.core.Object, access.record=public> {}\n"
        ".record Derv1 <ets.extends=Base, access.record=public> {}\n"
        ".record Derv2 <ets.extends=Derv1, access.record=public> {}\n"
        ".record Derv3 <ets.extends=Derv2, access.record=public> {}\n"
        ".record Derv4 <ets.extends=Derv3, access.record=public> {}\n"
        ".record Derv11 <ets.extends=Derv1, access.record=public> {}\n"
        ".record D <ets.extends=std.core.Object, access.record=public> {}\n"
        ".record E <ets.extends=D, access.record=public> {}\n"
        ".record std.core.Object <external>\n"
        ".function void D.foo(D a0, {UDerv11,Derv4} a1) <access.function=public> { return.void }\n"
        ".function void E.foo(E a0, {UDerv11,Derv3} a1) <access.function=public> { return.void }\n"
        ".function void E.foo(E a0, Derv1 a1) <access.function=public> { return.void }\n";
    auto res = p.Parse(source);
    ASSERT_EQ(p.ShowError().err, pandasm::Error::ErrorType::ERR_NONE);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());

    auto classLinker = Runtime::GetCurrent()->GetClassLinker();
    ASSERT(classLinker);
    classLinker->AddPandaFile(std::move(pf));

    auto *ext = static_cast<ark::ets::EtsClassLinkerExtension *>(
        ark::Runtime::GetCurrent()->GetClassLinker()->GetExtension(ark::panda_file::SourceLang::ETS));
    SuppressErrorHandler handler;
    const auto *classWithError = reinterpret_cast<const uint8_t *>("LE;");
    Class *klass = ext->GetClass(classWithError, true, nullptr, &handler);
    ASSERT_EQ(klass, nullptr);
    ASSERT_EQ(handler.GetMessage(), "Multiple override LE;foo LD;foo");
}

TEST_F(EtsRuntimeLinkerTest, CreateUnionClassMultiOverriding5)
{
    pandasm::Parser p;

    const char *source =
        ".language eTS\n"

        ".record Base <ets.extends=std.core.Object, access.record=public> {}\n"
        ".record I <ets.abstract, ets.interface, ets.extends=std.core.Object, access.record=public> {}\n"
        ".record A <ets.implements=I, ets.extends=Base, access.record=public> {}\n"
        ".record BB <ets.extends=std.core.Object, access.record=public> {}\n"
        ".record B <ets.extends=BB, access.record=public> {}\n"
        ".record D <ets.extends=std.core.Object, access.record=public> {}\n"
        ".record E <ets.extends=D, access.record=public> {}\n"
        ".record std.core.Object <external>\n"
        ".function void D.foo(D a0, A a1) <access.function=public> { return.void }\n"
        ".function void E.foo(E a0, {UBase,I,B} a1) <access.function=public> { return.void }\n"
        ".function void E.foo(E a0, I a1) <access.function=public> { return.void }\n";
    auto res = p.Parse(source);
    ASSERT_EQ(p.ShowError().err, pandasm::Error::ErrorType::ERR_NONE);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());

    auto classLinker = Runtime::GetCurrent()->GetClassLinker();
    ASSERT(classLinker);
    classLinker->AddPandaFile(std::move(pf));

    auto *ext = static_cast<ark::ets::EtsClassLinkerExtension *>(
        ark::Runtime::GetCurrent()->GetClassLinker()->GetExtension(ark::panda_file::SourceLang::ETS));
    SuppressErrorHandler handler;
    const auto *classWithError = reinterpret_cast<const uint8_t *>("LE;");
    Class *klass = ext->GetClass(classWithError, true, nullptr, &handler);
    ASSERT_EQ(klass, nullptr);
    ASSERT_EQ(handler.GetMessage(), "Multiple override LE;foo LD;foo");
}

TEST_F(EtsRuntimeLinkerTest, CreateUnionClassOverload)
{
    pandasm::Parser p;

    const char *source =
        ".language eTS\n"

        ".record Base <ets.extends=std.core.Object, access.record=public> {}\n"
        ".record I <ets.abstract, ets.interface, ets.extends=std.core.Object, access.record=public> {}\n"
        ".record A <ets.implements=I, ets.extends=Base, access.record=public> {}\n"
        ".record BI <ets.implements=I, ets.extends=BB, access.record=public> {}\n"
        ".record BB <ets.extends=std.core.Object, access.record=public> {}\n"
        ".record B <ets.extends=BB, access.record=public> {}\n"
        ".record D <ets.extends=std.core.Object, access.record=public> {}\n"
        ".record E <ets.extends=D, access.record=public> {}\n"
        ".record std.core.Object <external>\n"
        ".function void D.foo(D a0, A a1) <access.function=public> { return.void }\n"
        ".function void E.foo(E a0, {UD,I,B} a1) <access.function=public> { return.void }\n"
        ".function void E.foo(E a0, BI a1) <access.function=public> { return.void }\n";
    auto res = p.Parse(source);
    ASSERT_EQ(p.ShowError().err, pandasm::Error::ErrorType::ERR_NONE);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());

    auto classLinker = Runtime::GetCurrent()->GetClassLinker();
    ASSERT(classLinker);
    classLinker->AddPandaFile(std::move(pf));

    auto *ext = static_cast<ark::ets::EtsClassLinkerExtension *>(
        ark::Runtime::GetCurrent()->GetClassLinker()->GetExtension(ark::panda_file::SourceLang::ETS));
    SuppressErrorHandler handler;
    const auto *classWithError = reinterpret_cast<const uint8_t *>("LE;");
    Class *klass = ext->GetClass(classWithError, true, nullptr, &handler);
    ASSERT_NE(klass, nullptr);
    ASSERT_EQ(handler.GetMessage(), "");
}

TEST_F(EtsRuntimeLinkerTest, CreateUnionClassOverriding)
{
    pandasm::Parser p;

    const char *source =
        ".language eTS\n"

        ".record Base <ets.extends=std.core.Object, access.record=public> {}\n"
        ".record Derv1 <ets.extends=Base, access.record=public> {}\n"
        ".record Derv2 <ets.extends=Derv1, access.record=public> {}\n"
        ".record Derv3 <ets.extends=Derv2, access.record=public> {}\n"
        ".record Derv4 <ets.extends=Derv3, access.record=public> {}\n"
        ".record Derv11 <ets.extends=Derv1, access.record=public> {}\n"
        ".record D <ets.extends=std.core.Object, access.record=public> {}\n"
        ".record E <ets.extends=D, access.record=public> {}\n"
        ".record std.core.Object <external>\n"
        ".function void D.foo(D a0, {UDerv11,Derv4} a1) <access.function=public> { return.void }\n"
        ".function void D.foo(D a0, {UDerv11,Derv3} a1) <access.function=public> { return.void }\n"
        ".function void E.foo(E a0, Derv1 a1) <access.function=public> { return.void }\n";
    auto res = p.Parse(source);
    ASSERT_EQ(p.ShowError().err, pandasm::Error::ErrorType::ERR_NONE);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());

    auto classLinker = Runtime::GetCurrent()->GetClassLinker();
    ASSERT(classLinker);
    classLinker->AddPandaFile(std::move(pf));

    auto *ext = static_cast<ark::ets::EtsClassLinkerExtension *>(
        ark::Runtime::GetCurrent()->GetClassLinker()->GetExtension(ark::panda_file::SourceLang::ETS));
    SuppressErrorHandler handler;
    const auto *classWithError = reinterpret_cast<const uint8_t *>("LE;");
    Class *klass = ext->GetClass(classWithError, true, nullptr, &handler);
    ASSERT_NE(klass, nullptr);
    ASSERT_EQ(handler.GetMessage(), "");
}

}  // namespace ark::ets::test
