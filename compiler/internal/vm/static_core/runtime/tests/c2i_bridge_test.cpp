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
#include <vector>

#include "assembly-parser.h"
#include "libarkbase/utils/utils.h"
#include "libarkfile/bytecode_emitter.h"
#include "libarkfile/bytecode_instruction.h"
#include "libarkfile/file_items.h"
#include "libarkfile/shorty_iterator.h"
#include "libarkfile/value.h"
#include "runtime/arch/helpers.h"
#include "runtime/bridge/bridge.h"
#include "runtime/entrypoints/entrypoints.h"
#include "runtime/include/method.h"
#include "runtime/include/runtime.h"
#include "runtime/include/thread.h"
#include "runtime/tests/interpreter/test_runtime_interface.h"
#include "invokation_helper.h"

using TypeId = ark::panda_file::Type::TypeId;
using BytecodeEmitter = ark::BytecodeEmitter;

namespace ark::test {

int32_t CmpDynImpl(Method * /*unused*/, coretypes::TaggedValue v1, coretypes::TaggedValue v2)
{
    return v1 == v2 ? 0 : 1;
}

coretypes::TaggedValue LdUndefinedImpl(Method *method)
{
    return Runtime::GetCurrent()->GetLanguageContext(*method).GetInitialTaggedValue();
}

class CompiledCodeToInterpreterBridgeTest : public testing::Test {
public:
    CompiledCodeToInterpreterBridgeTest() = default;
    NO_COPY_SEMANTIC(CompiledCodeToInterpreterBridgeTest);
    NO_MOVE_SEMANTIC(CompiledCodeToInterpreterBridgeTest);

    void SetUp() override
    {
        // See comment in InvokeEntryPoint function
        if constexpr (RUNTIME_ARCH == Arch::AARCH32) {
            GTEST_SKIP();
        }

        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);
        options.SetGcType("epsilon");
        // NOTE: Can be testes with SetNoAsyncJit(true)

        Runtime::Create(options);
        thread_ = MTManagedThread::GetCurrent();
        thread_->ManagedCodeBegin();
        SetUpHelperFunctions("PandaAssembly");
    }

    ~CompiledCodeToInterpreterBridgeTest() override
    {
        if constexpr (RUNTIME_ARCH == Arch::AARCH32) {
            return;
        }
        thread_->ManagedCodeEnd();
        Runtime::Destroy();
    }

    void SetUpHelperFunctions(const std::string &language)
    {
        Runtime *runtime = Runtime::GetCurrent();
        ClassLinker *classLinker = runtime->GetClassLinker();

        std::string source = ".language " + language + "\n" + R"(
            .record TestUtils {}
            .function i32 TestUtils.cmpDyn(any a0, any a1) <native>
            .function any TestUtils.ldundefined() <native>
        )";
        pandasm::Parser p;
        auto res = p.Parse(source);
        ASSERT_TRUE(res.HasValue());
        std::unique_ptr<const panda_file::File> pf = pandasm::AsmEmitter::Emit(res.Value());
        classLinker->AddPandaFile(std::move(pf));

        auto descriptor = std::make_unique<PandaString>();
        std::optional<ark::panda_file::SourceLang> langLocal = panda_file::LanguageFromString(language);
        if (!langLocal) {
            UNREACHABLE();
        }
        auto *extension = classLinker->GetExtension(langLocal.value_or(panda_file::SourceLang::PANDA_ASSEMBLY));
        Class *klass =
            extension->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("TestUtils"), descriptor.get()));

        Method *cmpDyn = klass->GetDirectMethod(utf::CStringAsMutf8("cmpDyn"));
        ASSERT_NE(cmpDyn, nullptr);
        cmpDyn->SetCompiledEntryPoint(reinterpret_cast<const void *>(CmpDynImpl));

        Method *ldundefined = klass->GetDirectMethod(utf::CStringAsMutf8("ldundefined"));
        ASSERT_NE(ldundefined, nullptr);
        ldundefined->SetCompiledEntryPoint(reinterpret_cast<const void *>(LdUndefinedImpl));
    }

    Method *MakeNoArgsMethod(TypeId retType, int64_t ret)
    {
        Runtime *runtime = Runtime::GetCurrent();
        ClassLinker *classLinker = runtime->GetClassLinker();
        LanguageContext ctx = runtime->GetLanguageContext(lang_);

        std::ostringstream out;
        out << ".language " << ctx << '\n';
        if (retType == TypeId::REFERENCE) {
            // 'operator <<' for TypeId::REFERENCE returns 'reference'. So create a class to handle this situation.
            out << ".record reference {}\n";
        }
        out << ".function " << panda_file::Type(retType) << " main() {\n";
        if (TypeId::F32 <= retType && retType <= TypeId::F64) {
            out << "fldai.64 " << bit_cast<double>(ret) << '\n';
            out << "return.64\n";
        } else if (TypeId::I64 <= retType && retType <= TypeId::U64) {
            out << "ldai.64 " << ret << '\n';
            out << "return.64\n";
        } else if (retType == TypeId::REFERENCE) {
            out << "lda.null\n";
            out << "return.obj\n";
        } else {
            out << "ldai " << ret << '\n';
            out << "return\n";
        }
        out << "}";

        pandasm::Parser p;
        auto res = p.Parse(out.str());
        std::unique_ptr<const panda_file::File> pf = pandasm::AsmEmitter::Emit(res.Value());
        classLinker->AddPandaFile(std::move(pf));

        auto descriptor = std::make_unique<PandaString>();
        auto *extension = classLinker->GetExtension(ctx);
        Class *klass =
            extension->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("_GLOBAL"), descriptor.get()));

        Method *main = klass->GetDirectMethod(utf::CStringAsMutf8("main"));
        main->SetInterpreterEntryPoint();
        return main;
    }

    Method *MakeCheckArgsMethod(const std::initializer_list<TypeId> &shorty, const std::initializer_list<int64_t> args,
                                bool isInstance = false)
    {
        Runtime *runtime = Runtime::GetCurrent();
        ClassLinker *classLinker = runtime->GetClassLinker();
        LanguageContext ctx = runtime->GetLanguageContext(lang_);

        std::ostringstream out;
        std::ostringstream signature;
        std::ostringstream body;
        uint32_t argNum = 0;

        if (isInstance) {
            signature << "Test a0";
            body << "lda.null" << '\n' << "jne.obj a0, fail\n";
            ++argNum;
        }
        auto shortyIt = shorty.begin();
        TypeId retType = *shortyIt++;
        auto argsIt = args.begin();
        while (shortyIt != shorty.end()) {
            if (argsIt == args.end()) {
                // only dynamic methods could be called with less arguments than declared
                ASSERT(*shortyIt == TypeId::TAGGED);
            }
            if (argNum > 0) {
                signature << ", ";
            }
            if ((TypeId::F32 <= *shortyIt && *shortyIt <= TypeId::U64) || *shortyIt == TypeId::REFERENCE ||
                *shortyIt == TypeId::TAGGED) {
                signature << panda_file::Type(*shortyIt) << " a" << argNum;
                body = GetBodyPrologue(shortyIt, argsIt, args, argNum);
            } else {
                signature << "i32 a" << argNum;
                body << "ldai " << *argsIt << '\n' << "jne a" << argNum << ", fail\n";
            }
            ++shortyIt;
            if (argsIt != args.end()) {
                ++argsIt;
            }
            ++argNum;
        }

        FillBodyEpilogue(retType, body);

        out = GetFileWithInfo(ctx, retType, signature, body);

        pandasm::Parser p;
        auto res = p.Parse(out.str());
        std::unique_ptr<const panda_file::File> pf = pandasm::AsmEmitter::Emit(res.Value());
        classLinker->AddPandaFile(std::move(pf));

        auto descriptor = std::make_unique<PandaString>();
        auto *extension = classLinker->GetExtension(ctx);
        Class *klass = extension->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("Test"), descriptor.get()));

        Method *main = klass->GetDirectMethod(utf::CStringAsMutf8("main"));
        main->SetInterpreterEntryPoint();
        return main;
    }

private:
    std::ostringstream GetBodyPrologue(std::initializer_list<TypeId>::const_iterator &shortyIt,
                                       std::initializer_list<int64_t>::const_iterator &argsIt,
                                       const std::initializer_list<int64_t> &args, uint32_t argNum)
    {
        std::ostringstream body;
        if (TypeId::F32 <= *shortyIt && *shortyIt <= TypeId::F64) {
            body << "fldai.64 " << bit_cast<double>(*argsIt) << '\n';
            body << "fcmpg.64 a" << argNum << '\n';
            body << "jnez fail\n";
        } else if (TypeId::I64 <= *shortyIt && *shortyIt <= TypeId::U64) {
            body << "ldai.64 " << *argsIt << '\n';
            body << "cmp.64 a" << argNum << '\n';
            body << "jnez fail\n";
        } else if (*shortyIt == TypeId::TAGGED) {
            if (argsIt == args.end()) {
                body << "call.short TestUtils.ldundefined\n";
            } else {
                body << "ldai.dyn " << *argsIt << '\n';
            }
            body << "sta.dyn v0\n";
            body << "call.short TestUtils.cmpDyn, v0, a" << argNum << '\n';
            body << "jnez fail\n";
        } else {
            body << "lda.null\n";
            body << "jne.obj a" << argNum << ", fail\n";
        }
        return body;
    }

    void FillBodyEpilogue(const TypeId &retType, std::ostringstream &body)
    {
        if (retType == TypeId::TAGGED) {
            body << "ldai.dyn 1\n";
            body << "return.dyn\n";
            body << "fail:\n";
            body << "ldai.dyn 0\n";
            body << "return.dyn\n";
        } else {
            body << "ldai 1\n";
            body << "return\n";
            body << "fail:\n";
            body << "ldai 0\n";
            body << "return\n";
        }
    }

    std::ostringstream GetFileWithInfo(LanguageContext &ctx, TypeId retType, const std::ostringstream &signature,
                                       const std::ostringstream &body)
    {
        std::ostringstream out;
        out << ".language " << ctx << '\n';
        out << ".record TestUtils <external>\n";
        out << ".function i32 TestUtils.cmpDyn(any a0, any a1) <external>\n";
        out << ".function any TestUtils.ldundefined() <external>\n";
        out << ".record reference {}\n";
        out << ".record Test {}\n";
        out << ".function " << panda_file::Type(retType) << " Test.main(" << signature.str() << ") {\n";
        out << body.str();
        out << "}";
        return out;
    }

    MTManagedThread *thread_ {nullptr};
    panda_file::SourceLang lang_ {panda_file::SourceLang::PANDA_ASSEMBLY};
};

TEST_F(CompiledCodeToInterpreterBridgeTest, InvokeVoidNoArg)
{
    auto method = MakeNoArgsMethod(TypeId::VOID, 0);
    InvokeEntryPoint<void>(method);
}

TEST_F(CompiledCodeToInterpreterBridgeTest, InvokeIntNoArg)
{
    auto method = MakeNoArgsMethod(TypeId::I32, 5L);
    auto res = InvokeEntryPoint<int32_t>(method);
    ASSERT_EQ(res, 5L);
}

TEST_F(CompiledCodeToInterpreterBridgeTest, InvokeLongNoArg)
{
    auto method = MakeNoArgsMethod(TypeId::I64, 7L);

    auto res = InvokeEntryPoint<int64_t>(method);
    ASSERT_EQ(res, 7L);
}

TEST_F(CompiledCodeToInterpreterBridgeTest, InvokeDoubleNoArg)
{
    // NOLINTNEXTLINE(readability-magic-numbers)
    auto method = MakeNoArgsMethod(TypeId::F64, bit_cast<int64_t>(3.0_D));

    auto res = InvokeEntryPoint<double>(method);
    ASSERT_EQ(res, 3.0_D);
}

TEST_F(CompiledCodeToInterpreterBridgeTest, InvokeObjNoArg)
{
    auto method = MakeNoArgsMethod(TypeId::REFERENCE, 0);

    auto *res = InvokeEntryPoint<ObjectHeader *>(method);
    ASSERT_EQ(res, nullptr);
}

/// Arguments tests
TEST_F(CompiledCodeToInterpreterBridgeTest, InvokeInt)
{
    auto method = MakeCheckArgsMethod({TypeId::I32, TypeId::I32}, {5L});

    auto res = InvokeEntryPoint<int32_t>(method, 5L);
    ASSERT_EQ(res, 1);
}

TEST_F(CompiledCodeToInterpreterBridgeTest, InvokeInstanceInt)
{
    auto method = MakeCheckArgsMethod({TypeId::I32}, {0, 5L}, true);

    auto res = InvokeEntryPoint<int32_t>(method, nullptr, 5L);
    ASSERT_EQ(res, 1);
}

TEST_F(CompiledCodeToInterpreterBridgeTest, Invoke3Int)
{
    auto method = MakeCheckArgsMethod({TypeId::I32, TypeId::I32, TypeId::I32, TypeId::I32}, {3L, 2L, 1L});

    auto res = InvokeEntryPoint<int32_t>(method, 3L, 2L, 1L);
    ASSERT_EQ(res, 1);
}

TEST_F(CompiledCodeToInterpreterBridgeTest, InvokeLong)
{
    auto method = MakeCheckArgsMethod({TypeId::I32, TypeId::I64}, {7L});

    auto res = InvokeEntryPoint<int32_t>(method, 7L);
    ASSERT_EQ(res, 1);
}

TEST_F(CompiledCodeToInterpreterBridgeTest, InvokeDouble)
{
    // NOLINTNEXTLINE(readability-magic-numbers)
    auto method = MakeCheckArgsMethod({TypeId::I32, TypeId::F64}, {bit_cast<int64_t>(2.0_D)});

    // NOLINTNEXTLINE(readability-magic-numbers)
    auto res = InvokeEntryPoint<int32_t>(method, 2.0_D);
    ASSERT_EQ(res, 1);
}

TEST_F(CompiledCodeToInterpreterBridgeTest, Invoke4Int)
{
    auto method =
        MakeCheckArgsMethod({TypeId::I32, TypeId::I32, TypeId::I32, TypeId::I32, TypeId::I32}, {4L, 3L, 2L, 1L});

    auto res = InvokeEntryPoint<int32_t>(method, 4L, 3L, 2L, 1L);
    ASSERT_EQ(res, 1);
}

TEST_F(CompiledCodeToInterpreterBridgeTest, Invoke2Long)
{
    auto method = MakeCheckArgsMethod({TypeId::I32, TypeId::I64, TypeId::I64}, {7L, 8L});

    auto res = InvokeEntryPoint<int32_t>(method, 7L, 8L);
    ASSERT_EQ(res, 1);
}

// NOLINTBEGIN(readability-magic-numbers)

TEST_F(CompiledCodeToInterpreterBridgeTest, Invoke4IntDouble)
{
    auto method = MakeCheckArgsMethod({TypeId::I32, TypeId::I32, TypeId::I32, TypeId::I32, TypeId::I32, TypeId::F64},
                                      {4L, 3L, 2L, 1L, bit_cast<int64_t>(8.0_D)});

    auto res = InvokeEntryPoint<int32_t>(method, 4L, 3L, 2L, 1L, 8.0_D);
    ASSERT_EQ(res, 1);
}

TEST_F(CompiledCodeToInterpreterBridgeTest, Invoke7Int)
{
    auto method = MakeCheckArgsMethod(
        {TypeId::I32, TypeId::I32, TypeId::I32, TypeId::I32, TypeId::I32, TypeId::I32, TypeId::I32, TypeId::I32},
        {7L, 6L, 5L, 4L, 3L, 2L, 1L});

    auto res = InvokeEntryPoint<int32_t>(method, 7L, 6L, 5L, 4L, 3L, 2L, 1L);
    ASSERT_EQ(res, 1);
}

TEST_F(CompiledCodeToInterpreterBridgeTest, Invoke7Int8Double)
{
    auto method = MakeCheckArgsMethod(
        {TypeId::I32, TypeId::I32, TypeId::I32, TypeId::I32, TypeId::I32, TypeId::I32, TypeId::I32, TypeId::I32,
         TypeId::F64, TypeId::F64, TypeId::F64, TypeId::F64, TypeId::F64, TypeId::F64, TypeId::F64, TypeId::F64},
        {7L, 6L, 5L, 4L, 3L, 2L, 1L, bit_cast<int64_t>(10.0_D), bit_cast<int64_t>(11.0_D), bit_cast<int64_t>(12.0_D),
         bit_cast<int64_t>(13.0_D), bit_cast<int64_t>(14.0_D), bit_cast<int64_t>(15.0_D), bit_cast<int64_t>(16.0_D),
         bit_cast<int64_t>(17.0_D)});

    auto res = InvokeEntryPoint<int32_t>(method, 7L, 6L, 5L, 4L, 3L, 2L, 1L, 10.0_D, 11.0_D, 12.0_D, 13.0_D, 14.0_D,
                                         15.0_D, 16.0_D, 17.0_D);
    ASSERT_EQ(res, 1);
}

TEST_F(CompiledCodeToInterpreterBridgeTest, Invoke8Int)
{
    auto method = MakeCheckArgsMethod({TypeId::I32, TypeId::I32, TypeId::I32, TypeId::I32, TypeId::I32, TypeId::I32,
                                       TypeId::I32, TypeId::I32, TypeId::I32},
                                      {8L, 7L, 6L, 5L, 4L, 3L, 2L, 1L});

    auto res = InvokeEntryPoint<int32_t>(method, 8L, 7L, 6L, 5L, 4L, 3L, 2L, 1L);
    ASSERT_EQ(res, 1);
}

TEST_F(CompiledCodeToInterpreterBridgeTest, Invoke8Int9Double)
{
    auto method = MakeCheckArgsMethod({TypeId::I32, TypeId::I32, TypeId::I32, TypeId::I32, TypeId::I32, TypeId::I32,
                                       TypeId::I32, TypeId::I32, TypeId::I32, TypeId::F64, TypeId::F64, TypeId::F64,
                                       TypeId::F64, TypeId::F64, TypeId::F64, TypeId::F64, TypeId::F64, TypeId::F64},
                                      {8L, 7L, 6L, 5L, 4L, 3L, 2L, 1L, bit_cast<int64_t>(10.0_D),
                                       bit_cast<int64_t>(11.0_D), bit_cast<int64_t>(12.0_D), bit_cast<int64_t>(13.0_D),
                                       bit_cast<int64_t>(14.0_D), bit_cast<int64_t>(15.0_D), bit_cast<int64_t>(16.0_D),
                                       bit_cast<int64_t>(17.0_D), bit_cast<int64_t>(18.0_D)});

    auto res = InvokeEntryPoint<int32_t>(method, 8L, 7L, 6L, 5L, 4L, 3L, 2L, 1L, 10.0_D, 11.0_D, 12.0_D, 13.0_D, 14.0_D,
                                         15.0_D, 16.0_D, 17.0_D, 18.0_D);
    ASSERT_EQ(res, 1);
}

// Dynamic functions

// NOLINTEND(readability-magic-numbers)

}  // namespace ark::test
