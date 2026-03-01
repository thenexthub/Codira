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

#include <vector>

#include "assembly-parser.h"
#include "mangling.h"
#include "libarkfile/value.h"
#include "libarkbase/utils/utils.h"
#include "runtime/entrypoints/entrypoints.h"
#include "runtime/include/compiler_interface.h"
#include "runtime/include/method-inl.h"
#include "runtime/include/runtime.h"

namespace ark::test {

class MethodTest : public testing::Test {
public:
    MethodTest()
    {
        RuntimeOptions options;
        // NOLINTNEXTLINE(readability-magic-numbers)
        options.SetHeapSizeLimit(128_MB);
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);
        options.SetGcType("epsilon");
        Runtime::Create(options);
        thread_ = ark::MTManagedThread::GetCurrent();
        thread_->ManagedCodeBegin();
    }

    ~MethodTest() override
    {
        thread_->ManagedCodeEnd();
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(MethodTest);
    NO_MOVE_SEMANTIC(MethodTest);

    void VerifyLineNumber(const std::vector<int> &lines)
    {
        pandasm::Parser p;
        auto source = R"(
            .function i32 foo() {
                movi v0, 0x64               # offset 0x0, size 3
                mov v1, v0                  # offset 0x3, size 2
                mod v0, v1                  # offset 0x5, size 2
                sta v0                      # offset 0x7, size 2
                mov v2, v0                  # offset 0x9, size 2
                mov v0, v1                  # offset 0xb, size 2
                sta v0                      # offset 0xd, size 2
                mov v2, v0                  # offset 0xf, size 2
                mov v0, v1                  # offset 0x11, size 2
                lda v0                      # offset 0x13, size 2
                return                      # offset 0x15, size 1
                movi v0, 0x1                # offset 0x16, size 2
                lda v0                      # offset 0x18, size 2
                return                      # offset 0x20, size 1
            }
        )";
        // when verifying line numbers, we do not care about the exact instructions.
        // we only care about the sequence of line numbers.
        const std::vector<int> offsets {0x0, 0x3, 0x5, 0x7, 0x9, 0xb, 0xd, 0xf, 0x11, 0x13, 0x15, 0x16, 0x18, 0x20};
        auto res = p.Parse(source);
        auto &prog = res.Value();
        const std::string name = pandasm::GetFunctionSignatureFromName("foo", {});
        ASSERT_NE(prog.functionStaticTable.find(name), prog.functionStaticTable.end());
        auto &insVec = prog.functionStaticTable.find(name)->second.ins;
        const int insNum = insVec.size();
        ASSERT_EQ(lines.size(), insNum);

        for (int i = 0; i < insNum; i++) {
            insVec[i].insDebug.SetLineNumber(lines[i]);
        }

        auto pf = pandasm::AsmEmitter::Emit(res.Value());
        ASSERT_NE(pf, nullptr);

        ClassLinker *classLinker = Runtime::GetCurrent()->GetClassLinker();
        classLinker->AddPandaFile(std::move(pf));
        auto *extension = classLinker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY);

        PandaString descriptor;

        Class *klass = extension->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("_GLOBAL"), &descriptor));
        ASSERT_NE(klass, nullptr);

        Method *method = klass->GetDirectMethod(utf::CStringAsMutf8("foo"));
        ASSERT_NE(method, nullptr);

        for (int i = 0; i < insNum; i++) {
            ASSERT_EQ(method->GetLineNumFromBytecodeOffset(offsets[i]), lines[i]) << "do not match on i = " << i;
        }
    }

private:
    ark::MTManagedThread *thread_;
};

template <bool IS_DYNAMIC = false>
static Frame *CreateFrame(size_t nregs, Method *method, Frame *prev)
{
    return ark::CreateFrameWithSize(Frame::GetActualSize<IS_DYNAMIC>(nregs), nregs, method, prev);
}

TEST_F(MethodTest, SetIntrinsic)
{
    Method method(nullptr, nullptr, panda_file::File::EntityId(), panda_file::File::EntityId(), 0, 0, nullptr);
    ASSERT_FALSE(method.IsIntrinsic());

    auto intrinsic = intrinsics::Intrinsic::MATH_COS_F64;
    method.SetIntrinsic(intrinsic);
    ASSERT_TRUE(method.IsIntrinsic());

    ASSERT_EQ(method.GetIntrinsic(), intrinsic);
}

static int32_t EntryPoint(Method * /* unused */)
{
    return 0;
}

static auto g_invokeSource = R"(
        .function i32 g() {
            ldai 0
            return
        }

        .function i32 f() {
            ldai 0
            return
        }

        .function void main() {
            call f
            return.void
        }
    )";

TEST_F(MethodTest, Invoke)
{
    pandasm::Parser p;

    auto source = g_invokeSource;

    auto res = p.Parse(source);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());
    ASSERT_NE(pf, nullptr);

    ClassLinker *classLinker = Runtime::GetCurrent()->GetClassLinker();
    classLinker->AddPandaFile(std::move(pf));
    auto *extension = classLinker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY);

    PandaString descriptor;

    Class *klass = extension->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("_GLOBAL"), &descriptor));
    ASSERT_NE(klass, nullptr);

    Method *mainMethod = klass->GetDirectMethod(utf::CStringAsMutf8("main"));
    ASSERT_NE(mainMethod, nullptr);

    Method *fMethod = klass->GetDirectMethod(utf::CStringAsMutf8("f"));
    ASSERT_NE(fMethod, nullptr);

    Method *gMethod = klass->GetDirectMethod(utf::CStringAsMutf8("g"));
    ASSERT_NE(gMethod, nullptr);

    gMethod->SetCompiledEntryPoint(reinterpret_cast<const void *>(EntryPoint));

    EXPECT_EQ(fMethod->GetHotnessCounter(),
              Runtime::GetCurrent()->IsProfilerEnabled() ? 1500U : std::numeric_limits<int16_t>::max());

    auto frameDeleter = [](Frame *frame) { FreeFrame(frame); };
    std::unique_ptr<Frame, decltype(frameDeleter)> frame(CreateFrame(0, mainMethod, nullptr), frameDeleter);

    ManagedThread *thread = ManagedThread::GetCurrent();
    thread->SetCurrentFrame(frame.get());

    // Invoke f calls interpreter

    std::vector<Value> args;
    Value v = fMethod->Invoke(ManagedThread::GetCurrent(), args.data());
    EXPECT_EQ(v.GetAs<int64_t>(), 0);
    EXPECT_EQ(fMethod->GetHotnessCounter(),
              Runtime::GetCurrent()->IsProfilerEnabled() ? 1499U : std::numeric_limits<int16_t>::max() - 1);
    EXPECT_EQ(ManagedThread::GetCurrent(), thread);

    // Invoke f called compiled code

    fMethod->SetCompiledEntryPoint(reinterpret_cast<const void *>(EntryPoint));

    v = fMethod->Invoke(ManagedThread::GetCurrent(), args.data());
    EXPECT_EQ(v.GetAs<int64_t>(), 0);
    EXPECT_EQ(fMethod->GetHotnessCounter(),
              Runtime::GetCurrent()->IsProfilerEnabled() ? 1498U : std::numeric_limits<int16_t>::max() - 2U);
    EXPECT_EQ(ManagedThread::GetCurrent(), thread);
}

TEST_F(MethodTest, VirtualMethod)
{
    pandasm::Parser p;

    auto source = R"(
        .record R {}

        .function void R.foo(R a0, i32 a1) {
            return
        }
    )";

    auto res = p.Parse(source);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());
    ASSERT_NE(pf, nullptr);

    ClassLinker *classLinker = Runtime::GetCurrent()->GetClassLinker();
    classLinker->AddPandaFile(std::move(pf));
    auto *extension = classLinker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY);

    PandaString descriptor;

    Class *klass = extension->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("R"), &descriptor));
    ASSERT_NE(klass, nullptr);

    Method *method = klass->GetDirectMethod(utf::CStringAsMutf8("foo"));
    ASSERT_NE(method, nullptr);

    ASSERT_FALSE(method->IsStatic());
    ASSERT_EQ(method->GetNumArgs(), 2U);
    ASSERT_EQ(method->GetArgType(0).GetId(), panda_file::Type::TypeId::REFERENCE);
    ASSERT_EQ(method->GetArgType(1).GetId(), panda_file::Type::TypeId::I32);
}

TEST_F(MethodTest, GetLineNumFromBytecodeOffset1)
{
    pandasm::Parser p;

    auto source = R"(          # line 1
        .function void foo() { # line 2
            mov v0, v1         # line 3, offset 0, size 2
            mov v100, v200     # line 4, offset 2, size 3
            movi v0, 4         # line 5, offset 5, size 2
            movi v0, 100       # line 6, offset 7, size 3
            movi v0, 300       # line 7, offset 10, size 4
            return.void        # line 8, offset 14, size 1
        }
    )";

    auto res = p.Parse(source);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());
    ASSERT_NE(pf, nullptr);

    ClassLinker *classLinker = Runtime::GetCurrent()->GetClassLinker();
    classLinker->AddPandaFile(std::move(pf));
    auto *extension = classLinker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY);

    PandaString descriptor;

    Class *klass = extension->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("_GLOBAL"), &descriptor));
    ASSERT_NE(klass, nullptr);

    Method *method = klass->GetDirectMethod(utf::CStringAsMutf8("foo"));
    ASSERT_NE(method, nullptr);

    ASSERT_EQ(method->GetLineNumFromBytecodeOffset(0), 3_I);
    ASSERT_EQ(method->GetLineNumFromBytecodeOffset(2U), 4_I);
    ASSERT_EQ(method->GetLineNumFromBytecodeOffset(5U), 5_I);
    ASSERT_EQ(method->GetLineNumFromBytecodeOffset(7U), 6_I);
    ASSERT_EQ(method->GetLineNumFromBytecodeOffset(10U), 7_I);
    ASSERT_EQ(method->GetLineNumFromBytecodeOffset(14U), 8_I);

    ASSERT_EQ(method->GetLineNumFromBytecodeOffset(20U), 8_I);
}

// NOLINTBEGIN(readability-magic-numbers)

TEST_F(MethodTest, GetLineNumFromBytecodeOffset2)
{
    VerifyLineNumber({4, 4, 4, 4, 4, 6, 6, 6, 6, 6, 6, 8, 8, 8});
}

TEST_F(MethodTest, GetLineNumFromBytecodeOffset3)
{
    VerifyLineNumber({4, 4, 4, 4, 4, 7, 5, 5, 6, 6, 6, 8, 8, 8});
}

TEST_F(MethodTest, GetLineNumFromBytecodeOffset4)
{
    VerifyLineNumber({3, 3, 4, 4, 6, 6, 10, 5, 8, 9, 9, 4, 4, 12});
}

TEST_F(MethodTest, GetLineNumFromBytecodeOffset5)
{
    VerifyLineNumber({4, 4, 4, 4, 6, 6, 7, 8, 8, 8, 9, 4, 4, 12});
}

TEST_F(MethodTest, GetLineNumFromBytecodeOffset6)
{
    VerifyLineNumber({4, 17, 5, 7, 7, 13, 19, 19, 11, 10, 2, 7, 8, 18});
}

TEST_F(MethodTest, GetLineNumFromBytecodeOffset7)
{
    VerifyLineNumber({4, 5, 7, 9, 10, 11, 13, 14, 15, 16, 6, 1, 3, 2});
}

TEST_F(MethodTest, GetLineNumFromBytecodeOffset8)
{
    VerifyLineNumber({3, 4, 4, 5, 6, 6, 7, 9, 10, 11, 12, 13, 14, 14});
}

TEST_F(MethodTest, GetLineNumFromBytecodeOffset9)
{
    VerifyLineNumber({3, 4, 5, 6, 6, 7, 9, 10, 16, 12, 13, 14, 15, 11});
}

TEST_F(MethodTest, GetLineNumFromBytecodeOffset10)
{
    pandasm::Parser p;

    auto source = R"(          # line 1
        .function void foo() { # line 2
            mov v0, v1         # line 3, offset 0, size 2
            mov v100, v200     # line 4, offset 2, size 3
            movi v0, 4         # line 5, offset 5, size 2
            movi v0, 100       # line 6, offset 7, size 3
            movi v0, 300       # line 7, offset 10, size 4
            return.void        # line 8, offset 14, size 1
        }
    )";

    auto res = p.Parse(source);
    auto &prog = res.Value();
    auto &function = prog.functionStaticTable.at(pandasm::GetFunctionSignatureFromName("foo", {}));

    pandasm::debuginfo::LocalVariable lv;
    lv.name = "a";
    lv.signature = "I";
    lv.reg = 0;
    lv.start = 0;
    lv.length = 5U;

    function.localVariableDebug.push_back(lv);

    lv.name = "b";
    lv.start = 5U;
    lv.length = 10U;

    function.localVariableDebug.push_back(lv);

    auto pf = pandasm::AsmEmitter::Emit(res.Value());
    ASSERT_NE(pf, nullptr);

    ClassLinker *classLinker = Runtime::GetCurrent()->GetClassLinker();
    classLinker->AddPandaFile(std::move(pf));
    auto *extension = classLinker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY);

    PandaString descriptor;

    Class *klass = extension->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("_GLOBAL"), &descriptor));
    ASSERT_NE(klass, nullptr);

    Method *method = klass->GetDirectMethod(utf::CStringAsMutf8("foo"));
    ASSERT_NE(method, nullptr);

    ASSERT_EQ(method->GetLineNumFromBytecodeOffset(0U), 3_I);
    ASSERT_EQ(method->GetLineNumFromBytecodeOffset(2U), 4_I);
    ASSERT_EQ(method->GetLineNumFromBytecodeOffset(5U), 5_I);
    ASSERT_EQ(method->GetLineNumFromBytecodeOffset(7U), 6_I);
    ASSERT_EQ(method->GetLineNumFromBytecodeOffset(10U), 7_I);
    ASSERT_EQ(method->GetLineNumFromBytecodeOffset(14U), 8_I);

    ASSERT_EQ(method->GetLineNumFromBytecodeOffset(20U), 8_I);
}

TEST_F(MethodTest, GetClassSourceFile)
{
    pandasm::Parser p;

    auto source = R"(
        .record R {}

        .function void R.foo() {
            return.void
        }

        .function void foo() {
            return.void
        }
    )";

    std::string sourceFilename = "source.pa";
    auto res = p.Parse(source, sourceFilename);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());
    ASSERT_NE(pf, nullptr);

    ClassLinker *classLinker = Runtime::GetCurrent()->GetClassLinker();
    auto *extension = classLinker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY);
    classLinker->AddPandaFile(std::move(pf));

    PandaString descriptor;

    {
        Class *klass = extension->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("_GLOBAL"), &descriptor));
        ASSERT_NE(klass, nullptr);

        Method *method = klass->GetDirectMethod(utf::CStringAsMutf8("foo"));
        ASSERT_NE(method, nullptr);

        auto result = method->GetClassSourceFile();
        ASSERT_EQ(result.data, nullptr);
    }

    {
        Class *klass = extension->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("R"), &descriptor));
        ASSERT_NE(klass, nullptr);

        Method *method = klass->GetDirectMethod(utf::CStringAsMutf8("foo"));
        ASSERT_NE(method, nullptr);

        auto result = method->GetClassSourceFile();
        ASSERT_TRUE(utf::IsEqual(result.data, utf::CStringAsMutf8(sourceFilename.data())));
    }
}

static int32_t StackTraceEntryPoint(Method * /* unused */)
{
    auto thread = ManagedThread::GetCurrent();

    struct StackTraceData {
        // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
        std::string funcName;
        // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
        int32_t lineNum;

        bool operator==(const StackTraceData &st) const
        {
            return funcName == st.funcName && lineNum == st.lineNum;
        }
    };

    std::vector<StackTraceData> expected {{"f3", 31},   {"f2", 26}, {".cctor", 14},
                                          {".ctor", 9}, {"f1", 20}, {"main", 41}};
    std::vector<StackTraceData> trace;
    for (auto stack = StackWalker::Create(thread); stack.HasFrame(); stack.NextFrame()) {
        auto pc = stack.GetBytecodePc();
        auto *methodFromFrame = stack.GetMethod();
        auto lineNum = static_cast<int32_t>(methodFromFrame->GetLineNumFromBytecodeOffset(pc));
        std::string funcName(utf::Mutf8AsCString(methodFromFrame->GetName().data));
        trace.push_back({funcName, lineNum});
    }

    if (trace == expected) {
        return 0;
    }

    return 1;
}

// NOLINTEND(readability-magic-numbers)
static auto g_stackTraceSource = R"(             # 1
        .record R1 {}                           # 2
                                                # 3
        .record R2 {                            # 4
            i32 f1 <static>                     # 5
        }                                       # 6
        .function void R1.ctor(R1 a0) <ctor> {  # 7
            ldai 0                              # 8
            ldstatic R2.f1                      # 9
            return.void                         # 10
        }                                       # 11
                                                # 12
        .function void R2.cctor() <cctor> {     # 13
            call f2                             # 14
            ststatic R2.f1                      # 15
            return.void                         # 16
        }                                       # 17
                                                # 18
        .function i32 f1() {                    # 19
            initobj R1.ctor                     # 20
            ldstatic R2.f1                      # 21
            return                              # 22
        }                                       # 23
                                                # 24
        .function i32 f2() {                    # 25
            call f3                             # 26
            return                              # 27
        }                                       # 28
                                                # 29
        .function i32 f3() {                    # 30
            call f4                             # 31
            return                              # 32
        }                                       # 33
                                                # 34
        .function i32 f4() {                    # 35
            ldai 0                              # 36
            return                              # 37
        }                                       # 38
                                                # 39
        .function i32 main() {                  # 40
            call f1                             # 41
            return                              # 42
        }                                       # 43
)";

TEST_F(MethodTest, StackTrace)
{
    pandasm::Parser p;

    auto source = g_stackTraceSource;

    auto res = p.Parse(source);
    ASSERT_TRUE(res) << res.Error().message;

    auto pf = pandasm::AsmEmitter::Emit(res.Value());
    ASSERT_NE(pf, nullptr) << pandasm::AsmEmitter::GetLastError();

    ClassLinker *classLinker = Runtime::GetCurrent()->GetClassLinker();
    classLinker->AddPandaFile(std::move(pf));
    auto *extension = classLinker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY);

    PandaString descriptor;

    Class *klass = extension->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("_GLOBAL"), &descriptor));
    ASSERT_NE(klass, nullptr);

    Method *mainMethod = klass->GetDirectMethod(utf::CStringAsMutf8("main"));
    ASSERT_NE(mainMethod, nullptr);

    Method *f4Method = klass->GetDirectMethod(utf::CStringAsMutf8("f4"));
    ASSERT_NE(f4Method, nullptr);

    f4Method->SetCompiledEntryPoint(reinterpret_cast<const void *>(StackTraceEntryPoint));

    ManagedThread *thread = ManagedThread::GetCurrent();
    thread->SetCurrentFrame(nullptr);

    std::vector<Value> args;
    Value v = mainMethod->Invoke(thread, args.data());
    EXPECT_EQ(v.GetAs<int32_t>(), 0);
}

TEST_F(MethodTest, GetFullName)
{
    pandasm::Parser p;

    auto source = R"(
        .record Foo {}
        .record R {}

        .function void R.void_fun(R a0) {
            return.void
        }
        .function void R.static_void_fun() <static> {
            return.void
        }
        .function Foo R.multiple_args(R a0, i32 a1, Foo a2, i64[] a3, Foo[] a4) {
            lda.obj a2
            return.obj
        }
    )";

    auto res = p.Parse(source);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());
    ASSERT_NE(pf, nullptr);

    ClassLinker *classLinker = Runtime::GetCurrent()->GetClassLinker();
    classLinker->AddPandaFile(std::move(pf));
    auto *extension = classLinker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY);
    PandaString descriptor;

    Class *klass = extension->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("R"), &descriptor));
    ASSERT_NE(klass, nullptr);

    Method *method1 = klass->GetDirectMethod(utf::CStringAsMutf8("void_fun"));
    ASSERT_NE(method1, nullptr);
    ASSERT_EQ(PandaStringToStd(method1->GetFullName()), "R::void_fun");
    ASSERT_EQ(PandaStringToStd(method1->GetFullName(true)), "void R::void_fun(R)");

    Method *method2 = klass->GetDirectMethod(utf::CStringAsMutf8("static_void_fun"));
    ASSERT_NE(method2, nullptr);
    ASSERT_EQ(PandaStringToStd(method2->GetFullName()), "R::static_void_fun");
    ASSERT_EQ(PandaStringToStd(method2->GetFullName(true)), "void R::static_void_fun()");

    Method *method3 = klass->GetDirectMethod(utf::CStringAsMutf8("multiple_args"));
    ASSERT_NE(method3, nullptr);
    ASSERT_EQ(PandaStringToStd(method3->GetFullName()), "R::multiple_args");
    ASSERT_EQ(PandaStringToStd(method3->GetFullName(true)), "Foo R::multiple_args(R, i32, Foo, [J, [LFoo;)");
}

static auto g_testSource = R"(
    .record Foo {}
    .record R {}
    .function void R.void_fun(R a0) {
        return.void
    }
    .function u1 R.u1() {
        ldai 0
        return
    }
    .function i8 R.i8() {
        ldai 0
        return
    }
    .function u8 R.u8() {
        ldai 0
        return
    }
    .function i16 R.i16() {
        ldai 0
        return
    }
    .function u16 R.u16() {
        ldai 0
        return
    }
    .function i32 R.i32() {
        ldai 0
        return
    }
    .function u32 R.u32() {
        ldai 0
        return
    }
    .function f32 R.f32() {
        ldai 0
        return
    }
    .function i64 R.i64() {
        ldai 0
        return
    }
    .function u64 R.u64() {
        ldai 0
        return
    }
    .function f64 R.f64() {
        ldai 0
        return
    }
    .function R R.tagged() {
        ldai 0
        return
    }
)";

void TestMethodType(Class *klass, const std::string &s, const std::string &type)
{
    Method *method = klass->GetDirectMethod(utf::CStringAsMutf8(s.c_str()));
    Method::Proto proto = method->GetProto();
    const char *descriptor = proto.GetReturnTypeDescriptor().data();
    ASSERT_EQ(descriptor, std::string_view(type));
}

TEST_F(MethodTest, GetReturnTypeDescriptor)
{
    pandasm::Parser p;
    auto res = p.Parse(g_testSource);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());
    ASSERT_NE(pf, nullptr);

    ClassLinker *classLinker = Runtime::GetCurrent()->GetClassLinker();
    classLinker->AddPandaFile(std::move(pf));
    auto *extension = classLinker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY);
    PandaString cdescriptor;

    Class *klass = extension->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("R"), &cdescriptor));
    ASSERT_NE(klass, nullptr);

    TestMethodType(klass, "void_fun", "V");
    TestMethodType(klass, "u1", "Z");
    TestMethodType(klass, "i8", "B");
    TestMethodType(klass, "u8", "H");
    TestMethodType(klass, "i16", "S");
    TestMethodType(klass, "u16", "C");
    TestMethodType(klass, "i32", "I");
    TestMethodType(klass, "u32", "U");
    TestMethodType(klass, "f32", "F");
    TestMethodType(klass, "i64", "J");
    TestMethodType(klass, "u64", "Q");
    TestMethodType(klass, "f64", "D");
}
}  // namespace ark::test
