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

#include <compiler/compiler_options.h>
#include <gtest/gtest.h>

#include <bitset>
#include <vector>

#include "assembly-emitter.h"
#include "assembly-parser.h"
#include "code_info/code_info_builder.h"
#include "libarkbase/utils/cframe_layout.h"
#include "libarkbase/utils/utils.h"
#include "runtime/bridge/bridge.h"
#include "runtime/include/runtime.h"
#include "runtime/include/stack_walker-inl.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/mem/refstorage/global_object_storage.h"
#include "runtime/tests/test_utils.h"
#include "compiler/tests/panda_runner.h"

namespace ark::test {

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace compiler;

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define HOOK_ASSERT(cond, action)                                                     \
    do {                                                                              \
        if (!(cond)) {                                                                \
            std::cerr << "ASSERT FAILED(" << __LINE__ << "): " << #cond << std::endl; \
            action;                                                                   \
        }                                                                             \
    } while (0)

class StackWalkerTest : public testing::Test {
public:
    using Callback = int (*)(uintptr_t, uintptr_t);

    StackWalkerTest()
        : defaultCompilerNonOptimizing_(g_options.IsCompilerNonOptimizing()),
          defaultCompilerRegex_(g_options.GetCompilerRegex())
    {
    }

    ~StackWalkerTest() override
    {
        g_options.SetCompilerNonOptimizing(defaultCompilerNonOptimizing_);
        g_options.SetCompilerRegex(defaultCompilerRegex_);
    }

    NO_COPY_SEMANTIC(StackWalkerTest);
    NO_MOVE_SEMANTIC(StackWalkerTest);

    void SetUp() override
    {
#ifndef PANDA_COMPILER_ENABLE
        GTEST_SKIP();
#endif
    }

    void TestModifyManyVregs(bool isCompiled);

    static Method *GetMethod(std::string_view methodName)
    {
        PandaString descriptor;
        auto *thread = MTManagedThread::GetCurrent();
        thread->ManagedCodeBegin();
        auto *extension = Runtime::GetCurrent()->GetClassLinker()->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY);
        auto cls = extension->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("_GLOBAL"), &descriptor));
        thread->ManagedCodeEnd();
        ASSERT(cls);
        return cls->GetDirectMethod(utf::CStringAsMutf8(methodName.data()));
    }

    static mem::Reference *globalObj_;

private:
    bool defaultCompilerNonOptimizing_;
    std::string defaultCompilerRegex_;
};

mem::Reference *StackWalkerTest::globalObj_;

template <typename T>
uint64_t ConvertToU64(T val)
{
    if constexpr (sizeof(T) == sizeof(uint32_t)) {
        return bit_cast<uint32_t>(val);
    } else if constexpr (sizeof(T) == sizeof(uint64_t)) {
        return bit_cast<uint64_t>(val);
    } else {
        return static_cast<uint64_t>(val);
    }
}

template <Arch ARCH>
int32_t ToCalleeRegister(size_t reg)
{
    return reg + GetFirstCalleeReg(ARCH, false);
}

template <Arch ARCH>
int32_t ToCalleeFpRegister(size_t reg)
{
    return reg + GetFirstCalleeReg(ARCH, true);
}

static auto g_modifyVregSource = R"(
    .function i32 main() {
        movi.64 v0, 27
    try_begin:
        call.short testa, v0
        jmp try_end
    try_end:
        ldai 0
        return
    .catchall try_begin, try_end, try_end
    }
    .function i32 testa(i64 a0) {
        call.short testb, a0
        return
    }
    .function i32 testb(i64 a0) {
        call.short testc, a0
        return
    }
    .function i32 testc(i64 a0) {
        lda.64 a0
    try_begin:
        call.short hook  # change vregs in all frames
        jnez exit
        call.short hook  # verify vregs in all frames
        jmp exit
    exit:
        ldai 0
        return
    .catchall try_begin, exit, exit
    }
    .function i32 hook() {
        ldai 1
        return
    }
)";

[[maybe_unused]] static constexpr std::array<uint64_t, 3> FRAME_VALUES = {0x123456789abcdef, 0xaaaabbbbccccdddd,
                                                                          0xabcdef20};

int WalkIfZeroRun(StackWalker &walker)
{
    bool success = false;
    bool wasSet = false;
    HOOK_ASSERT(!walker.IsCFrame(), return 1);
    success = walker.IterateVRegsWithInfo([&wasSet, &walker](const auto &regInfo, const auto &reg) {
        if (!regInfo.IsAccumulator()) {
            HOOK_ASSERT(reg.GetLong() == 27L, return false);
            walker.SetVRegValue(regInfo, FRAME_VALUES[0]);
            wasSet = true;
        }
        return true;
    });
    HOOK_ASSERT(success, return 1);
    HOOK_ASSERT(wasSet, return 1);

    walker.NextFrame();
    HOOK_ASSERT(walker.IsCFrame(), return 1);
    success = walker.IterateVRegsWithInfo([&walker](const auto &regInfo, const auto &reg) {
        if (!regInfo.IsAccumulator()) {
            HOOK_ASSERT(reg.GetLong() == 27L, return false);
            walker.SetVRegValue(regInfo, FRAME_VALUES[1]);
        }
        return true;
    });
    HOOK_ASSERT(success, return 1);

    walker.NextFrame();
    HOOK_ASSERT(walker.IsCFrame(), return 1);
    success = walker.IterateVRegsWithInfo([&walker](const auto &regInfo, const auto &reg) {
        if (!regInfo.IsAccumulator()) {
            HOOK_ASSERT(reg.GetLong() == 27L, return true);
            walker.SetVRegValue(regInfo, FRAME_VALUES[2U]);
        }
        return true;
    });
    HOOK_ASSERT(success, return 1);

    return 0;
}

int WalkIfOneRun(StackWalker &walker)
{
    bool success = false;
    HOOK_ASSERT(!walker.IsCFrame(), return 1);
    success = walker.IterateVRegsWithInfo([](const auto &regInfo, const auto &reg) {
        if (!regInfo.IsAccumulator()) {
            HOOK_ASSERT(reg.GetLong() == bit_cast<int64_t>(FRAME_VALUES[0]), return true);
        }
        return true;
    });
    HOOK_ASSERT(success, return 1);

    walker.NextFrame();
    HOOK_ASSERT(walker.IsCFrame(), return 1);
    success = walker.IterateVRegsWithInfo([](const auto &regInfo, const auto &reg) {
        if (!regInfo.IsAccumulator()) {
            HOOK_ASSERT(reg.GetLong() == bit_cast<int64_t>(FRAME_VALUES[1]), return true);
        }
        return true;
    });
    HOOK_ASSERT(success, return 1);

    walker.NextFrame();
    HOOK_ASSERT(walker.IsCFrame(), return 1);
    success = walker.IterateVRegsWithInfo([](const auto &regInfo, const auto &reg) {
        if (!regInfo.IsAccumulator()) {
            HOOK_ASSERT(reg.GetLong() == bit_cast<int64_t>(FRAME_VALUES[2U]), return true);
        }
        return true;
    });
    HOOK_ASSERT(success, return 1);

    return 0;
}

TEST_F(StackWalkerTest, ModifyVreg)
{
    auto source = g_modifyVregSource;

    PandaRunner runner;
    runner.GetRuntimeOptions().SetCompilerHotnessThreshold(0);
    runner.GetCompilerOptions().SetCompilerNonOptimizing(true);
    runner.GetCompilerOptions().SetCompilerRematConst(false);
    runner.GetCompilerOptions().SetCompilerRegex("(?!_GLOBAL::testb|_GLOBAL::hook).*");

    static int runCount = 0;
    runner.Run(source, [](uintptr_t lr, [[maybe_unused]] uintptr_t fp) -> int {
        StackWalker walker(reinterpret_cast<void *>(fp), true, lr);
        walker.NextFrame();
        if (runCount == 0) {
            if (WalkIfZeroRun(walker) != 0) {
                return 1;
            }
        } else if (runCount == 1) {
            if (WalkIfOneRun(walker) != 0) {
                return 1;
            }
        } else {
            return 1;
        }
        runCount++;
        return 0;
    });
    ASSERT_EQ(runCount, 2_I);
}

static auto g_testModifyManyVregsSource = R"(
    .function i32 main() {
        movi.64 v0, 5
    try_begin:
        call.short test
        jmp try_end
    try_end:
        ldai 0
        return

    .catchall try_begin, try_end, try_end
    }

    .function i32 test() {
        movi.64 v1, 1
        movi.64 v2, 2
        movi.64 v3, 3
        movi.64 v4, 4
        movi.64 v5, 5
        movi.64 v6, 6
        movi.64 v7, 7
        movi.64 v8, 8
        movi.64 v9, 9
        movi.64 v10, 10
        movi.64 v11, 11
        movi.64 v12, 12
        movi.64 v13, 13
        movi.64 v14, 14
        movi.64 v15, 15
        movi.64 v16, 16
        movi.64 v17, 17
        movi.64 v18, 18
        movi.64 v19, 19
        movi.64 v20, 20
        movi.64 v21, 21
        movi.64 v22, 22
        movi.64 v23, 23
        movi.64 v24, 24
        movi.64 v25, 25
        movi.64 v26, 26
        movi.64 v27, 27
        movi.64 v28, 28
        movi.64 v29, 29
        movi.64 v30, 30
        movi v31, 31
    try_begin:
        mov v0, v31
        newarr v0, v0, i32[]
        call.short stub
        jmp try_end
    try_end:
        ldai 0
        return

    .catchall try_begin, try_end, try_end
    }

    .function i32 stub() {
    try_begin:
        call.short hook  # change vregs in all frames
        jnez exit
        call.short hook  # verify vregs in all frames
        jmp exit
    exit:
        ldai 0
        return

    .catchall try_begin, exit, exit
    }

    .function i32 hook() {
        ldai 1
        return
    }
)";

template <typename VRegRef>
static bool FirstRunModifyVregs(int *regIndex, StackWalker *walker, ObjectHeader *obj, const VRegInfo *regInfo,
                                const VRegRef &reg)
{
    if (!regInfo->IsAccumulator()) {
        if (reg.HasObject()) {
            HOOK_ASSERT(reg.GetReference() != nullptr, return false);
            walker->SetVRegValue(*regInfo, obj);
        } else if (regInfo->GetLocation() != VRegInfo::Location::CONSTANT) {
            // frame_size is more than nregs_ now for inst `V4_V4_ID16` and `V4_V4_V4_V4_ID16`
            auto regIdx = regInfo->GetIndex();
            if (regIdx < walker->GetMethod()->GetNumVregs()) {
                HOOK_ASSERT(regIdx == reg.GetLong(), return false);
            }
            // NOLINTNEXTLINE(readability-magic-numbers)
            walker->SetVRegValue(*regInfo, regIdx + 100000000000L);
        }
        (*regIndex)++;
    }
    return true;
}

template <typename VRegRef>
static bool CheckVregs(int *regIndex, ObjectHeader *obj, const VRegInfo &regInfo, const VRegRef &reg)
{
    if (!regInfo.IsAccumulator()) {
        if (reg.HasObject()) {
#if defined(PANDA_TARGET_64) && !defined(PANDA_32_BIT_MANAGED_POINTER)
            HOOK_ASSERT(reg.GetReference() == obj, return false);
#else
            HOOK_ASSERT((reg.GetReference() == reinterpret_cast<ObjectHeader *>(Low32Bits(obj))), return false);
#endif
        } else {
            if (regInfo.GetLocation() != VRegInfo::Location::CONSTANT) {
                HOOK_ASSERT(reg.GetLong() == (regInfo.GetIndex() + 100000000000L), return false);
            }
            (*regIndex)++;
        }
    }
    return true;
}

void StackWalkerTest::TestModifyManyVregs(bool isCompiled)
{
    auto source = g_testModifyManyVregsSource;
    static bool firstRun;
    static bool compiled;

    PandaRunner runner;
    runner.GetRuntimeOptions().SetCompilerHotnessThreshold(0);
    runner.GetCompilerOptions().SetCompilerNonOptimizing(true);

    if (!isCompiled) {
        runner.GetCompilerOptions().SetCompilerRegex("(?!_GLOBAL::main)(?!_GLOBAL::test)(?!_GLOBAL::hook).*");
    }

    firstRun = true;
    compiled = isCompiled;
    runner.Run(source, [](uintptr_t lr, [[maybe_unused]] uintptr_t fp) -> int {
        StackWalker walker(reinterpret_cast<void *>(fp), true, lr);

        HOOK_ASSERT(walker.GetMethod()->GetFullName() == "_GLOBAL::stub", return 1);
        walker.NextFrame();
        HOOK_ASSERT(walker.GetMethod()->GetFullName() == "_GLOBAL::test", return 1);
        HOOK_ASSERT(walker.IsCFrame() == compiled, return 1);

        int regIndex = 1;
        bool success = false;
        if (firstRun) {
            auto storage = Runtime::GetCurrent()->GetPandaVM()->GetGlobalObjectStorage();
            StackWalkerTest::globalObj_ =
                storage->Add(ark::mem::AllocateNullifiedPayloadString(1), mem::Reference::ObjectType::GLOBAL);
        }
        auto obj = Runtime::GetCurrent()->GetPandaVM()->GetGlobalObjectStorage()->Get(StackWalkerTest::globalObj_);
        if (firstRun) {
            success = walker.IterateVRegsWithInfo([&regIndex, &walker, &obj](const auto &regInfo, const auto &reg) {
                return FirstRunModifyVregs(&regIndex, &walker, obj, &regInfo, reg);
            });
            HOOK_ASSERT(success, return 1);
            HOOK_ASSERT(regIndex >= 32_I, return 1);
            firstRun = false;
        } else {
            success = walker.IterateVRegsWithInfo([&regIndex, &obj](const auto &regInfo, const auto &reg) {
                return CheckVregs(&regIndex, obj, regInfo, reg);
            });
            HOOK_ASSERT(success, return 1);
            HOOK_ASSERT(regIndex >= 32_I, return 1);
        }

        HOOK_ASSERT(success, return 1);
        return 0;
    });
}

TEST_F(StackWalkerTest, ModifyMultipleVregs)
{
    if constexpr (ArchTraits<RUNTIME_ARCH>::SUPPORT_DEOPTIMIZATION) {
        TestModifyManyVregs(true);
        TestModifyManyVregs(false);
    }
}

static auto g_throwExceptionThroughMultipleFramesSource = R"(
        .record E {}

        .function u1 f4() {
            newobj v0, E
            throw v0
            return
        }

        .function u1 f3() {
            call f4
            return
        }

        .function u1 f2() {
            call f3
            return
        }

        .function u1 f1() {
            call f2
            return
        }

        .function u1 main() {
        try_begin:
            movi v0, 123
            call f1
            ldai 1
            return
        try_end:

        catch_block1_begin:
            movi v1, 123
            lda v0
            jne v1, exit_1
            ldai 0
            return
            exit_1:
            ldai 1
            return

        .catch E, try_begin, try_end, catch_block1_begin
        }
    )";

TEST_F(StackWalkerTest, ThrowExceptionThroughMultipleFrames)
{
    auto source = g_throwExceptionThroughMultipleFramesSource;

    PandaRunner runner;
    runner.GetRuntimeOptions().SetCompilerHotnessThreshold(0);
    runner.GetCompilerOptions().SetCompilerNonOptimizing(true);

    runner.GetCompilerOptions().SetCompilerRegex("(?!_GLOBAL::main|_GLOBAL::f4|_GLOBAL::f1|_GLOBAL::f2|_GLOBAL::f3).*");
    runner.Run(source, nullptr);

    runner.GetCompilerOptions().SetCompilerRegex("(?!_GLOBAL::main|_GLOBAL::f4|_GLOBAL::f1|_GLOBAL::f2).*");
    runner.Run(source, nullptr);

    runner.GetCompilerOptions().SetCompilerRegex("(?!_GLOBAL::main|_GLOBAL::f4|_GLOBAL::f2).*");
    runner.Run(source, nullptr);

    runner.GetCompilerOptions().SetCompilerRegex("(?!_GLOBAL::main|_GLOBAL::f4).*");
    runner.Run(source, nullptr);

    runner.GetCompilerOptions().SetCompilerRegex("(?!_GLOBAL::main|_GLOBAL::f1|_GLOBAL::f2|_GLOBAL::f3).*");
    runner.Run(source, nullptr);

    runner.GetCompilerOptions().SetCompilerRegex("(?!_GLOBAL::main).*");
    runner.Run(source, nullptr);

    runner.GetCompilerOptions().SetCompilerRegex("(?!_GLOBAL::f4|_GLOBAL::f1|_GLOBAL::f2|_GLOBAL::f3).*");
    runner.Run(source, nullptr);

    runner.GetCompilerOptions().SetCompilerRegex("(?!_GLOBAL::f4|_GLOBAL::f1|_GLOBAL::f2).*");
    runner.Run(source, nullptr);

    runner.GetCompilerOptions().SetCompilerRegex("(?!_GLOBAL::f4|_GLOBAL::f2).*");
    runner.Run(source, nullptr);

    runner.GetCompilerOptions().SetCompilerRegex("(?!_GLOBAL::f4).*");
    runner.Run(source, nullptr);

    runner.GetCompilerOptions().SetCompilerRegex(".*");
    runner.Run(source, nullptr);

    runner.GetCompilerOptions().SetCompilerRegex("(?!_GLOBAL::f1|_GLOBAL::f2|_GLOBAL::f3).*");
    runner.Run(source, nullptr);

    runner.GetCompilerOptions().SetCompilerRegex("(?!_GLOBAL::f1|_GLOBAL::f2).*");
    runner.Run(source, nullptr);

    runner.GetCompilerOptions().SetCompilerRegex("(?!_GLOBAL::f2).*");
    runner.Run(source, nullptr);

    runner.GetCompilerOptions().SetCompilerRegex(".*");
    runner.Run(source, nullptr);
}

TEST_F(StackWalkerTest, CatchInCompiledCode)
{
    auto source = R"(
        .record panda.String <external>
        .record panda.ArrayIndexOutOfBoundsException <external>

        .function i32 f2() {
            movi v0, 4
            newarr v1, v0, i32[]
            ldai 42
            ldarr v1 # BoundsException
            return
        }

        .function i32 f1() {
        try_begin:
            call f2
            ldai 1
            return
        try_end:

        catch_block1_begin:
            ldai 123
            return

        .catch panda.ArrayIndexOutOfBoundsException, try_begin, try_end, catch_block1_begin
        }

        .function u1 main() {
            call f1
            movi v0, 123
            jne v0, exit_1
            ldai 0
            return
            exit_1:
            ldai 1
            return
        }
    )";

    PandaRunner runner;
    runner.GetRuntimeOptions().SetCompilerHotnessThreshold(0);
    runner.GetCompilerOptions().SetCompilerNonOptimizing(true);

    runner.GetCompilerOptions().SetCompilerRegex("(?!_GLOBAL::main|_GLOBAL::f2).*");
    runner.Run(source, nullptr);

    runner.GetCompilerOptions().SetCompilerRegex("(?!_GLOBAL::main|_GLOBAL::f1).*");
    runner.Run(source, nullptr);

    runner.GetCompilerOptions().SetCompilerRegex("(?!_GLOBAL::main).*");
    runner.Run(source, nullptr);

    runner.GetCompilerOptions().SetCompilerRegex(".*");
    runner.Run(source, nullptr);
}

}  // namespace ark::test
