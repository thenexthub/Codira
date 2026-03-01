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
#include <string>

#include "assembly-emitter.h"
#include "assembly-parser.h"
#include "include/method.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "public.h"
#include "verification/ets_plugin.h"
#include "verification/type/type_system.h"
#include "verification/public_internal.h"
#include "verification/jobs/service.h"
#include "libarkbase/utils/utf.h"

namespace ark::ets::test {

class CheckAccessModifiersTest : public testing::Test {
public:
    CheckAccessModifiersTest()
    {
        options_.SetShouldLoadBootPandaFiles(true);
        options_.SetShouldInitializeIntrinsics(true);
        options_.SetCompilerEnableJit(false);
        options_.SetLoadRuntimes({"ets"});

        auto stdlib = std::getenv("PANDA_STD_LIB");
        if (stdlib == nullptr) {
            std::cerr << "PANDA_STD_LIB env variable should be set and point to etsstdlib.abc" << std::endl;
            std::abort();
        }
        options_.SetBootPandaFiles({stdlib});
        Runtime::Create(options_);

        config = verifier::NewConfig();
        service = CreateService(config, Runtime::GetCurrent()->GetInternalAllocator(),
                                ark::Runtime::GetCurrent()->GetClassLinker(), "");
    }

    ~CheckAccessModifiersTest() override
    {
        DestroyService(service, false);
        DestroyConfig(config);
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(CheckAccessModifiersTest);
    NO_MOVE_SEMANTIC(CheckAccessModifiersTest);

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

protected:
    PandaEtsVM *vm_ = nullptr;                    // NOLINT(misc-non-private-member-variables-in-classes)
    ark::verifier::plugin::EtsPlugin plugin_ {};  // NOLINT(misc-non-private-member-variables-in-classes)
    verifier::Service *service;                   // NOLINT(misc-non-private-member-variables-in-classes)
    verifier::Config *config;                     // NOLINT(misc-non-private-member-variables-in-classes)

private:
    RuntimeOptions options_;
    EtsCoroutine *coroutine_ = nullptr;
};

TEST_F(CheckAccessModifiersTest, CorrectMethodAccess)
{
    verifier::TypeSystem typeSystem(service->verifierService, ark::panda_file::SourceLang::ETS);
    pandasm::Parser p;

    auto source = R"(
        .language eTS
    
        .record R {}
        .record B <ets.extends=R> {}

        .function void R.foo(R a0) <access.function=public> {
            return.void
        }
        .function void R.bar() <access.function=private> {
            return.void
        }
        .function void R.buz() <access.function=protected> {
            return.void
        }
        .function void B.bar() <access.function=public> {
            return.void
        }
    )";

    auto parseRes = p.Parse(source);
    auto pf = pandasm::AsmEmitter::Emit(parseRes.Value());
    ASSERT_EQ(p.ShowError().err, pandasm::Error::ErrorType::ERR_NONE);
    ASSERT_NE(pf, nullptr);

    ClassLinker *classLinker = Runtime::GetCurrent()->GetClassLinker();
    classLinker->AddPandaFile(std::move(pf));
    auto *ext = static_cast<ark::ets::EtsClassLinkerExtension *>(
        ark::Runtime::GetCurrent()->GetClassLinker()->GetExtension(ark::panda_file::SourceLang::ETS));
    PandaString descriptor;

    Class *klass = ext->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("R"), &descriptor));
    Method *from = klass->GetDirectMethod(utf::CStringAsMutf8("foo"));
    Method *method = klass->GetDirectMethod(utf::CStringAsMutf8("bar"));
    auto res = plugin_.CheckMethodAccessViolation(method, from, &typeSystem);
    ASSERT_TRUE(res.IsOk());

    Class *klassB = ext->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("B"), &descriptor));
    from = klassB->GetDirectMethod(utf::CStringAsMutf8("bar"));
    method = klass->GetDirectMethod(utf::CStringAsMutf8("buz"));
    res = plugin_.CheckMethodAccessViolation(method, from, &typeSystem);
    ASSERT_TRUE(res.IsOk());
}

TEST_F(CheckAccessModifiersTest, IncorrectMethodAccess)
{
    verifier::TypeSystem typeSystem(service->verifierService, ark::panda_file::SourceLang::ETS);
    pandasm::Parser p;

    auto source = R"(
        .language eTS
    
        .record R {}
        .record Foo {}

        .function void R.foo(R a0) <access.function=private> {
            return.void
        }
        .function void Foo.foo() <access.function=private> {
            return.void
        }
    )";

    auto parseRes = p.Parse(source);
    auto pf = pandasm::AsmEmitter::Emit(parseRes.Value());
    ASSERT_EQ(p.ShowError().err, pandasm::Error::ErrorType::ERR_NONE);
    ASSERT_NE(pf, nullptr);

    ClassLinker *classLinker = Runtime::GetCurrent()->GetClassLinker();
    classLinker->AddPandaFile(std::move(pf));
    auto *ext = static_cast<ark::ets::EtsClassLinkerExtension *>(
        ark::Runtime::GetCurrent()->GetClassLinker()->GetExtension(ark::panda_file::SourceLang::ETS));
    PandaString descriptor;

    Class *klassR = ext->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("R"), &descriptor));
    Class *klassFoo = ext->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("Foo"), &descriptor));

    Method *from = klassR->GetDirectMethod(utf::CStringAsMutf8("foo"));
    Method *method = klassFoo->GetDirectMethod(utf::CStringAsMutf8("foo"));

    const auto res = plugin_.CheckMethodAccessViolation(method, from, &typeSystem);
    ASSERT_TRUE(res.IsError());
    ASSERT_TRUE(std::string(res.msg) == std::string(verifier::CheckResult::private_method.msg));
}

TEST_F(CheckAccessModifiersTest, CorrectFieldAccess)
{
    verifier::TypeSystem typeSystem(service->verifierService, ark::panda_file::SourceLang::ETS);
    pandasm::Parser p;

    auto source = R"(
        .language eTS
    
        .record R {
            i32 num <access.field=protected>
            i32 num1 <access.field=private>
        }
        .function void R.foo(R a0) <access.function=public> {
            return.void
        }

        .record B <ets.extends=R> {}
        .function void B.bar() <access.function=public> {
            return.void
        }
    )";

    auto parseRes = p.Parse(source);
    auto pf = pandasm::AsmEmitter::Emit(parseRes.Value());
    ASSERT_EQ(p.ShowError().err, pandasm::Error::ErrorType::ERR_NONE);
    ASSERT_NE(pf, nullptr);

    ClassLinker *classLinker = Runtime::GetCurrent()->GetClassLinker();
    classLinker->AddPandaFile(std::move(pf));
    auto *ext = static_cast<ark::ets::EtsClassLinkerExtension *>(
        ark::Runtime::GetCurrent()->GetClassLinker()->GetExtension(ark::panda_file::SourceLang::ETS));
    PandaString descriptor;

    Class *klass = ext->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("R"), &descriptor));
    Method *from = klass->GetDirectMethod(utf::CStringAsMutf8("foo"));
    Field *field = klass->GetInstanceFieldByName(utf::CStringAsMutf8("num1"));
    auto res = plugin_.CheckFieldAccessViolation(field, from, &typeSystem);
    ASSERT_TRUE(res.IsOk());

    Class *klassB = ext->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("B"), &descriptor));
    from = klassB->GetDirectMethod(utf::CStringAsMutf8("bar"));
    field = klass->GetInstanceFieldByName(utf::CStringAsMutf8("num"));
    res = plugin_.CheckFieldAccessViolation(field, from, &typeSystem);
    ASSERT_TRUE(res.IsOk());
}

TEST_F(CheckAccessModifiersTest, IncorrectFieldAccess)
{
    verifier::TypeSystem typeSystem(service->verifierService, ark::panda_file::SourceLang::ETS);
    pandasm::Parser p;

    auto source = R"(
        .language eTS
        .record R <access.record=public> {
            i32 num <access.field=private>
        }

        .record Foo {}
        .function void Foo.foo() <access.function=public> {
            return.void
        }
    )";

    auto parseRes = p.Parse(source);
    auto pf = pandasm::AsmEmitter::Emit(parseRes.Value());
    ASSERT_EQ(p.ShowError().err, pandasm::Error::ErrorType::ERR_NONE);
    ASSERT_NE(pf, nullptr);

    ClassLinker *classLinker = Runtime::GetCurrent()->GetClassLinker();
    classLinker->AddPandaFile(std::move(pf));
    auto *ext = static_cast<ark::ets::EtsClassLinkerExtension *>(
        ark::Runtime::GetCurrent()->GetClassLinker()->GetExtension(ark::panda_file::SourceLang::ETS));
    PandaString descriptor;

    Class *klassR = ext->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("R"), &descriptor));
    Class *klassFoo = ext->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("Foo"), &descriptor));

    Method *from = klassFoo->GetDirectMethod(utf::CStringAsMutf8("foo"));
    Field *field = klassR->GetInstanceFieldByName(utf::CStringAsMutf8("num"));
    const auto res = plugin_.CheckFieldAccessViolation(field, from, &typeSystem);
    ASSERT_TRUE(res.IsError());
    ASSERT_TRUE(std::string(res.msg) == std::string(verifier::CheckResult::private_field.msg));
}

}  // namespace ark::ets::test