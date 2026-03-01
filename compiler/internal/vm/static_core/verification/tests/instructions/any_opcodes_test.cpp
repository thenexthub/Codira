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
#include "include/class_linker_extension.h"
#include "include/language_context.h"
#include "include/method.h"
#include "public.h"
#include "verification/type/type_system.h"
#include "verification/public_internal.h"
#include "verification/jobs/service.h"
#include "libarkbase/utils/utf.h"
#include "util/tests/verifier_test.h"
#include "include/runtime.h"

namespace ark::verifier::test {

class VerifyAnyOpcodes : public VerifierTest {
public:
    VerifyAnyOpcodes()
    {
        config = verifier::NewConfig();
        service = CreateService(config, Runtime::GetCurrent()->GetInternalAllocator(),
                                ark::Runtime::GetCurrent()->GetClassLinker(), "");
    }

    ~VerifyAnyOpcodes() override
    {
        DestroyService(service, false);
        DestroyConfig(config);
    }

    NO_COPY_SEMANTIC(VerifyAnyOpcodes);
    NO_MOVE_SEMANTIC(VerifyAnyOpcodes);

    std::unique_ptr<const panda_file::File> EmitPandasm(const char *source)
    {
        pandasm::Parser p;
        auto res = p.Parse(source);
        return pandasm::AsmEmitter::Emit(res.Value());
    }

    ClassLinkerExtension *GetLinkerExtention(std::unique_ptr<const panda_file::File> pf)
    {
        ClassLinker *classLinker = Runtime::GetCurrent()->GetClassLinker();
        classLinker->AddPandaFile(std::move(pf));
        return classLinker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY);
    }

    Method *GetDirectMethodFromClass(ClassLinkerExtension *ext, std::string className, std::string methodName)
    {
        PandaString descriptor;
        Class *klass = ext->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8(className.c_str()), &descriptor));
        return klass->GetDirectMethod(utf::CStringAsMutf8(methodName.c_str()));
    }

protected:
    verifier::Service *service;  // NOLINT(misc-non-private-member-variables-in-classes)
    verifier::Config *config;    // NOLINT(misc-non-private-member-variables-in-classes)
};

TEST_F(VerifyAnyOpcodes, AnyIsInstance)
{
    auto source = R"(
        .record R {}
        .function void test(R a0, R a1) <static, access.function=public> {
            lda.obj a0
            any.isinstance a1
            return.void
        }
    )";

    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);

    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    auto result = test->Verify();
    ASSERT_TRUE(result);
}

TEST_F(VerifyAnyOpcodes, AnyCallShort)
{
    auto source = R"(
        .record R {}
        .function void test(R a0, R a1) <static, access.function=public> {
            lda.obj a1
            sta.obj v1
            any.call.short a0, v1
            return.void
        }
    )";

    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);

    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    auto result = test->Verify();
    ASSERT_TRUE(result);
}

TEST_F(VerifyAnyOpcodes, NegAnyCallShort)
{
    auto source = R"(
        .record R {}
        .function void test(R a0, R a1) <static, access.function=public> {
            ldai 1
            sta v1
            any.call.short a0, v1
            return.void
        }
    )";

    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);

    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    auto result = test->Verify();
    ASSERT_TRUE(!result);
}

TEST_F(VerifyAnyOpcodes, AnyCallRange)
{
    auto source = R"(
        .record R {}
        .function void test(R a0, R a1, R a2) <static, access.function=public> {
            lda.obj a1
            sta.obj v1
            lda.obj a2
            sta.obj v2
            any.call.range a0, v1, 0x2
            return.void
        }
    )";

    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);

    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    auto result = test->Verify();
    ASSERT_TRUE(result);
}

TEST_F(VerifyAnyOpcodes, AnyCallThisShort)
{
    auto source = R"(
        .record R {}
        .function void test(R a0, R a1, R a2) <static, access.function=public> {
            lda.obj a1
            sta.obj v1
            any.call.this.short "bar", a0, v1
            return.void
        }
    )";

    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);

    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    auto result = test->Verify();
    ASSERT_TRUE(result);
}

TEST_F(VerifyAnyOpcodes, AnyCallThisRange)
{
    auto source = R"(
        .record R {}
        .function void test(R a0, R a1, R a2) <static, access.function=public> {
            lda.obj a1
            sta.obj v1
            lda.obj a2
            sta.obj v2
            any.call.this.range "foo", a0, v1, 0x2
            return.void
        }
    )";

    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);

    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    auto result = test->Verify();
    ASSERT_TRUE(result);
}

TEST_F(VerifyAnyOpcodes, AnyCall0)
{
    auto source = R"(
        .record R {}
        .function void test(R a0) <static, access.function=public> {
            any.call.0 a0
            return.void
        }
    )";

    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);

    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    auto result = test->Verify();
    ASSERT_TRUE(result);
}

TEST_F(VerifyAnyOpcodes, AnyNew0)
{
    auto source = R"(
        .record R {}
        .function void test(R a0) <static, access.function=public> {
            any.call.new.0 a0
            return.void
        }
    )";

    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);

    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    auto result = test->Verify();
    ASSERT_TRUE(result);
}

TEST_F(VerifyAnyOpcodes, AnyNewRange)
{
    auto source = R"(
        .record R {}
        .function void test(R a0, R a1, R a2) <static, access.function=public> {
            lda.obj a1
            sta.obj v1
            lda.obj a2
            sta.obj v2
            any.call.new.range a0, v1, 0x2
            return.void
        }
    )";

    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);

    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    auto result = test->Verify();
    ASSERT_TRUE(result);
}

TEST_F(VerifyAnyOpcodes, AnyNewShort)
{
    auto source = R"(
        .record R {}
        .function void test(R a0, R a1) <static, access.function=public> {
            lda.obj a1
            sta.obj v1
            any.call.new.short a0, v1
            return.void
        }
    )";

    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);

    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    auto result = test->Verify();
    ASSERT_TRUE(result);
}

TEST_F(VerifyAnyOpcodes, AnyCallThis0)
{
    auto source = R"(
        .record R {}
        .function void test(R a0) <static, access.function=public> {
            any.call.this.0 "baz", a0
            return.void
        }
    )";

    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);

    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    auto result = test->Verify();
    ASSERT_TRUE(result);
}

TEST_F(VerifyAnyOpcodes, AnyLdbyval)
{
    auto source = R"(
        .record R {}
        .function void test(R a0, R a1, R a2) <static, access.function=public> {
            any.ldbyval a0, a1
            return.void
        }
    )";

    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);

    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    auto result = test->Verify();
    ASSERT_TRUE(result);
}

TEST_F(VerifyAnyOpcodes, AnyLdbyname)
{
    auto source = R"(
        .record R {}
        .function void test(R a0, R a1, R a2) <static, access.function=public> {
            any.ldbyname a0, "a"
            return.void
        }
    )";

    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);

    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    auto result = test->Verify();
    ASSERT_TRUE(result);
}

TEST_F(VerifyAnyOpcodes, AnyLdbynamev)
{
    auto source = R"(
        .record R {}
        .function void test(R a0, R a1, R a2) <static, access.function=public> {
            any.ldbyname.v v1, a0, "a"
            return.void
        }
    )";

    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);

    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    auto result = test->Verify();
    ASSERT_TRUE(result);
}

TEST_F(VerifyAnyOpcodes, AnyLdbyidx)
{
    auto source = R"(
        .record R {}
        .function void test(R a0, R a1, R a2) <static, access.function=public> {
            fldai.64 0x3ff0000000000000
            any.ldbyidx a0
            return.void
        }
    )";

    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);

    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    auto result = test->Verify();
    ASSERT_TRUE(result);
}

TEST_F(VerifyAnyOpcodes, AnyStbyval)
{
    auto source = R"(
        .record R {}
        .function void test(R a0, R a1, R a2) <static, access.function=public> {
            lda.obj a2
            any.stbyval a0, a1
            return.void
        }
    )";

    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);

    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    auto result = test->Verify();
    ASSERT_TRUE(result);
}

TEST_F(VerifyAnyOpcodes, AnyStbyvalWithNull)
{
    auto source = R"(
        .record R {}
        .function void test(R a0, R a1) <static, access.function=public> {
            lda.null
            any.stbyval a0, a1
            return.void
        }
    )";

    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);

    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    auto result = test->Verify();
    ASSERT_TRUE(result);
}

TEST_F(VerifyAnyOpcodes, AnyStbyname)
{
    auto source = R"(
        .record R {}
        .function void test(R a0, R a1, R a2) <static, access.function=public> {
            lda.obj a1
            any.stbyname a0, "a"
            return.void
        }
    )";

    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);

    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    auto result = test->Verify();
    ASSERT_TRUE(result);
}

TEST_F(VerifyAnyOpcodes, AnyStbynameWithNull)
{
    auto source = R"(
        .record R {}
        .function void test(R a0, R a1, R a2) <static, access.function=public> {
            lda.null
            sta.obj v1
            any.stbyname v1, "a"
            return.void
        }
    )";

    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);

    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    auto result = test->Verify();
    ASSERT_TRUE(result);
}

TEST_F(VerifyAnyOpcodes, AnyStbynamev)
{
    auto source = R"(
        .record R {}
        .function void test(R a0, R a1, R a2) <static, access.function=public> {
            any.stbyname.v a1, a0, "a"
            return.void
        }
    )";

    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);

    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    auto result = test->Verify();
    ASSERT_TRUE(result);
}

TEST_F(VerifyAnyOpcodes, AnyStbynamevWithNull)
{
    auto source = R"(
        .record R {}
        .function void test(R a0) <static, access.function=public> {
            lda.null
            sta.obj v1
            any.stbyname.v v1, a0, "a"
            return.void
        }
    )";

    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);

    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    auto result = test->Verify();
    ASSERT_TRUE(result);
}

TEST_F(VerifyAnyOpcodes, AnyStvalidx)
{
    auto source = R"(
        .record R {}
        .function void test(R a0, R a1, R a2) <static, access.function=public> {
            fldai.64 0x3ff0000000000000
            sta.64 v0
            lda.obj a1
            any.stbyidx a0, v0
            return.void
        }
    )";

    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);

    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    auto result = test->Verify();
    ASSERT_TRUE(result);
}

TEST_F(VerifyAnyOpcodes, AnyStvalidxWithNull)
{
    auto source = R"(
        .record R {}
        .function void test(R a0, R a1, R a2) <static, access.function=public> {
            fldai.64 0x3ff0000000000000
            sta.64 v0
            lda.null
            any.stbyidx a0, v0
            return.void
        }
    )";

    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);

    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    auto result = test->Verify();
    ASSERT_TRUE(result);
}

TEST_F(VerifyAnyOpcodes, NegAnyStvalidx)
{
    auto source = R"(
        .record R {}
        .function void test(R a0, R a1, R a2) <static, access.function=public> {
            fldai 0x3ff0000000000000
            sta v0
            lda.obj a1
            any.stbyidx a0, v0
            return.void
        }
    )";

    auto pf = EmitPandasm(source);
    ASSERT_NE(pf, nullptr);

    auto *ext = GetLinkerExtention(std::move(pf));
    Method *test = GetDirectMethodFromClass(ext, "_GLOBAL", "test");
    auto result = test->Verify();
    ASSERT_TRUE(!result);
}

}  // namespace ark::verifier::test