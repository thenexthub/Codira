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

TEST_F(CheckAccessModifiersTest, EtsCallNameShort)
{
    verifier::TypeSystem typeSystem(service->verifierService, ark::panda_file::SourceLang::ETS);
    pandasm::Parser p;

    auto source = R"(
        .language eTS

        .record panda.Object <external>
        .record A1 {}
        .record ETSGLOBAL <extends=panda.Object, access.record=public> {}
        .record ETSGLOBAL.union_prop<ets.abstract, access.record=public> {}
        .function f64 ETSGLOBAL.union_prop.foo(ETSGLOBAL.union_prop a0, f64 a1) <ets.abstract, noimpl, access.function=public>

        .function void ETSGLOBAL.f1(A1 a0) <static, access.function=public> {
            fldai.64 0x3ff0000000000000
            sta.64 v1
            ets.call.name.short ETSGLOBAL.union_prop.foo:(ETSGLOBAL.union_prop,f64), a0, v1
            return.void
        }
    )";

    ASSERT_EQ(p.ShowError().err, pandasm::Error::ErrorType::ERR_NONE);
    auto parseRes = p.Parse(source);
    auto pf = pandasm::AsmEmitter::Emit(parseRes.Value());
    ASSERT_NE(pf, nullptr);

    ClassLinker *classLinker = Runtime::GetCurrent()->GetClassLinker();
    classLinker->AddPandaFile(std::move(pf));
    auto *ext = static_cast<ark::ets::EtsClassLinkerExtension *>(
        ark::Runtime::GetCurrent()->GetClassLinker()->GetExtension(ark::panda_file::SourceLang::ETS));
    PandaString descriptor;

    Class *klassEtsglobal = ext->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("ETSGLOBAL"), &descriptor));

    Method *func = klassEtsglobal->GetDirectMethod(utf::CStringAsMutf8("f1"));

    auto res = func->Verify();
    ASSERT_TRUE(res);
}

TEST_F(CheckAccessModifiersTest, EtsCallNameShortNegative)
{
    verifier::TypeSystem typeSystem(service->verifierService, ark::panda_file::SourceLang::ETS);
    pandasm::Parser p;

    auto source = R"(
        .language eTS

        .record panda.Object <external>
        .record A1 {}
        .record ETSGLOBAL <extends=panda.Object, access.record=public> {}
        .record ETSGLOBAL.union_prop<ets.abstract, access.record=public> {}
        .function f64 ETSGLOBAL.union_prop.foo(ETSGLOBAL.union_prop a0, f64 a1) <ets.abstract, noimpl, access.function=public>

        .function void ETSGLOBAL.f1(f64 a0) <static, access.function=public> {
            fldai.64 0x3ff0000000000000
            sta.64 v1
            ets.call.name.short ETSGLOBAL.union_prop.foo:(ETSGLOBAL.union_prop,f64), a0, v1
            return.void
        }
    )";

    ASSERT_EQ(p.ShowError().err, pandasm::Error::ErrorType::ERR_NONE);
    auto parseRes = p.Parse(source);
    auto pf = pandasm::AsmEmitter::Emit(parseRes.Value());
    ASSERT_NE(pf, nullptr);

    ClassLinker *classLinker = Runtime::GetCurrent()->GetClassLinker();
    classLinker->AddPandaFile(std::move(pf));
    auto *ext = static_cast<ark::ets::EtsClassLinkerExtension *>(
        ark::Runtime::GetCurrent()->GetClassLinker()->GetExtension(ark::panda_file::SourceLang::ETS));
    PandaString descriptor;

    Class *klassEtsglobal = ext->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("ETSGLOBAL"), &descriptor));

    Method *func = klassEtsglobal->GetDirectMethod(utf::CStringAsMutf8("f1"));

    auto res = func->Verify();
    ASSERT_TRUE(!res);
}

TEST_F(CheckAccessModifiersTest, EtsCallName)
{
    verifier::TypeSystem typeSystem(service->verifierService, ark::panda_file::SourceLang::ETS);
    pandasm::Parser p;

    auto source = R"(
        .language eTS

        .record panda.Object <external>
        .record A1 {}
        .record ETSGLOBAL <extends=panda.Object, access.record=public> {}
        .record ETSGLOBAL.union_prop<ets.abstract, access.record=public> {}
        .function f64 ETSGLOBAL.union_prop.foo(ETSGLOBAL.union_prop a0, f64 a1, f64 a2) <ets.abstract, noimpl, access.function=public>

        .function void ETSGLOBAL.f1(A1 a0) <static, access.function=public> {
            fldai.64 0x3ff0000000000000
            sta.64 v1
            fldai.64 0x3ff0000000000000
            sta.64 v2
            ets.call.name ETSGLOBAL.union_prop.foo:(ETSGLOBAL.union_prop,f64,f64), a0, v1, v2
            return.void
        }
    )";

    ASSERT_EQ(p.ShowError().err, pandasm::Error::ErrorType::ERR_NONE);
    auto parseRes = p.Parse(source);
    auto pf = pandasm::AsmEmitter::Emit(parseRes.Value());
    ASSERT_NE(pf, nullptr);

    ClassLinker *classLinker = Runtime::GetCurrent()->GetClassLinker();
    classLinker->AddPandaFile(std::move(pf));
    auto *ext = static_cast<ark::ets::EtsClassLinkerExtension *>(
        ark::Runtime::GetCurrent()->GetClassLinker()->GetExtension(ark::panda_file::SourceLang::ETS));
    PandaString descriptor;

    Class *klassEtsglobal = ext->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("ETSGLOBAL"), &descriptor));

    Method *func = klassEtsglobal->GetDirectMethod(utf::CStringAsMutf8("f1"));

    auto res = func->Verify();
    ASSERT_TRUE(res);
}

TEST_F(CheckAccessModifiersTest, EtsCallNameRange)
{
    verifier::TypeSystem typeSystem(service->verifierService, ark::panda_file::SourceLang::ETS);
    pandasm::Parser p;

    auto source = R"(
        .language eTS

        .record panda.Object <external>
        .record A1 {}
        .record ETSGLOBAL <extends=panda.Object, access.record=public> {}
        .record ETSGLOBAL.union_prop<ets.abstract, access.record=public> {}
        .function f64 ETSGLOBAL.union_prop.foo(ETSGLOBAL.union_prop a0, f64 a1, f64 a2, f64 a3, f64 a4) <ets.abstract, noimpl, access.function=public>

        .function void ETSGLOBAL.f1(A1 a0) <static, access.function=public> {
            lda.obj a0
            sta.obj v2
            fldai.64 0x3ff0000000000000
            sta.64 v3
            fldai.64 0x3ff0000000000000
            sta.64 v4
            fldai.64 0x3ff0000000000000
            sta.64 v5
            fldai.64 0x3ff0000000000000
            sta.64 v6
            ets.call.name.range ETSGLOBAL.union_prop.foo:(ETSGLOBAL.union_prop,f64,f64,f64,f64), v2
            return.void
        }
    )";

    ASSERT_EQ(p.ShowError().err, pandasm::Error::ErrorType::ERR_NONE);
    auto parseRes = p.Parse(source);
    auto pf = pandasm::AsmEmitter::Emit(parseRes.Value());
    ASSERT_NE(pf, nullptr);

    ClassLinker *classLinker = Runtime::GetCurrent()->GetClassLinker();
    classLinker->AddPandaFile(std::move(pf));
    auto *ext = static_cast<ark::ets::EtsClassLinkerExtension *>(
        ark::Runtime::GetCurrent()->GetClassLinker()->GetExtension(ark::panda_file::SourceLang::ETS));
    PandaString descriptor;

    Class *klassEtsglobal = ext->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("ETSGLOBAL"), &descriptor));

    Method *func = klassEtsglobal->GetDirectMethod(utf::CStringAsMutf8("f1"));

    auto res = func->Verify();
    ASSERT_TRUE(res);
}

}  // namespace ark::ets::test