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

#include "libarkbase/macros.h"
#include "unit_test.h"
#include "code_info/code_info_builder.h"
#include "codegen.h"
#include "panda_runner.h"
#include "libarkbase/events/events.h"
#include <fstream>
#include "libarkbase/utils/utils.h"

namespace ark::test {
class OsrTest : public testing::Test {
public:
    OsrTest()
        : defaultCompilerNonOptimizing_(compiler::g_options.IsCompilerNonOptimizing()),
          defaultCompilerInlining_(compiler::g_options.IsCompilerInlining()),
          defaultCompilerInliningBlacklist_(compiler::g_options.GetCompilerInliningBlacklist()),
          defaultCompilerRegex_(compiler::g_options.GetCompilerRegex())
    {
    }

    ~OsrTest() override
    {
        compiler::g_options.SetCompilerNonOptimizing(defaultCompilerNonOptimizing_);
        compiler::g_options.SetCompilerInlining(defaultCompilerInlining_);
        compiler::g_options.SetCompilerInliningBlacklist(defaultCompilerInliningBlacklist_);
        compiler::g_options.SetCompilerRegex(defaultCompilerRegex_);
    }

    NO_COPY_SEMANTIC(OsrTest);
    NO_MOVE_SEMANTIC(OsrTest);

    void SetUp() override
    {
#ifndef PANDA_EVENTS_ENABLED
        GTEST_SKIP();
#endif
        if constexpr (!ArchTraits<RUNTIME_ARCH>::SUPPORT_OSR) {
            GTEST_SKIP();
        }
    }

protected:
    static constexpr size_t HOTNESS_THRESHOLD = 4U;

private:
    bool defaultCompilerNonOptimizing_;
    bool defaultCompilerInlining_;
    arg_list_t defaultCompilerInliningBlacklist_;
    std::string defaultCompilerRegex_;
};

struct ScopeEvents {
    ScopeEvents()
    {
        Events::Create<Events::MEMORY>();
    }
    ~ScopeEvents()
    {
        Events::Destroy();
    }

    NO_COPY_SEMANTIC(ScopeEvents);
    NO_MOVE_SEMANTIC(ScopeEvents);
};

static constexpr auto OSR_IN_TOP_FRAME_SOURCE = R"(
    .function i32 main() {
        movi v0, 0
        movi v1, 30
        movi v2, 0
    loop:
        lda v0
        jeq v1, exit
        add2 v2
        sta v2
        inci v0, 1
        jmp loop
    exit:
        lda v2
        return
    }
)";

// NOLINTBEGIN(readability-magic-numbers)
TEST_F(OsrTest, OsrInTopFrameNonOptimizing)
{
    PandaRunner runner;
    runner.GetRuntimeOptions().SetCompilerHotnessThreshold(HOTNESS_THRESHOLD);
    runner.GetRuntimeOptions().SetCompilerEnableJit(true);
    runner.GetCompilerOptions().SetCompilerNonOptimizing(true);

    ScopeEvents scopeEvents;

    runner.Run(OSR_IN_TOP_FRAME_SOURCE, 435U);
    auto events = Events::CastTo<Events::MEMORY>();
    auto osrEvents = events->Select<events::EventsMemory::OsrEntryEvent>();
    ASSERT_EQ(osrEvents.size(), 1U);
}

TEST_F(OsrTest, OsrInTopFrameOptimizing)
{
    PandaRunner runner;
    runner.GetRuntimeOptions().SetCompilerHotnessThreshold(HOTNESS_THRESHOLD);
    runner.GetRuntimeOptions().SetCompilerEnableJit(true);
    runner.GetCompilerOptions().SetCompilerNonOptimizing(false);

    ScopeEvents scopeEvents;

    runner.Run(OSR_IN_TOP_FRAME_SOURCE, 435U);
    auto events = Events::CastTo<Events::MEMORY>();
    auto osrEvents = events->Select<events::EventsMemory::OsrEntryEvent>();
    ASSERT_EQ(osrEvents.size(), 1U);
}

static constexpr auto OSR_AFTER_IFRAME_SOURCE = R"(
    .function i32 main() {
        call f1
        return
    }

    .function i32 f1() {
        movi v0, 0
        movi v1, 30
        movi v2, 0
    loop:
        lda v0
        jeq v1, exit
        add2 v2
        sta v2
        inci v0, 1
        jmp loop
    exit:
        lda v2
        return
    }
)";

TEST_F(OsrTest, OsrAfterIFrameNonOptimizing)
{
    PandaRunner runner;
    runner.GetRuntimeOptions().SetCompilerHotnessThreshold(HOTNESS_THRESHOLD);
    runner.GetRuntimeOptions().SetCompilerEnableJit(true);
    runner.GetCompilerOptions().SetCompilerNonOptimizing(true);

    ScopeEvents scopeEvents;
    runner.Run(OSR_AFTER_IFRAME_SOURCE, 435U);
    auto events = Events::CastTo<Events::MEMORY>();
    auto osrEvents = events->Select<events::EventsMemory::OsrEntryEvent>();
    ASSERT_EQ(osrEvents.size(), 1U);
}

TEST_F(OsrTest, OsrAfterIFrameOptimizing)
{
    PandaRunner runner;
    runner.GetRuntimeOptions().SetCompilerHotnessThreshold(HOTNESS_THRESHOLD);
    runner.GetRuntimeOptions().SetCompilerEnableJit(true);
    runner.GetCompilerOptions().SetCompilerNonOptimizing(false);

    ScopeEvents scopeEvents;
    runner.Run(OSR_AFTER_IFRAME_SOURCE, 435U);
    auto events = Events::CastTo<Events::MEMORY>();
    auto osrEvents = events->Select<events::EventsMemory::OsrEntryEvent>();
    ASSERT_EQ(osrEvents.size(), 1U);
}

static constexpr auto OSR_AFTER_IFRAME_RESTORE_ACC_AFTER_VOID = R"(
    .function i32 main() {
        call f1
        ldai 0x0
        return
    }

    .function void f1() {
        movi v0, 0
        movi v1, 30
        movi v2, 0
    loop:
        lda v0
        jeq v1, exit
        add2 v2
        sta v2
        inci v0, 1
        jmp loop
    exit:
        lda v2
        return.void
    }
)";

TEST_F(OsrTest, OsrAfterIFrameRestoreAccAfterVoid)
{
    PandaRunner runner;
    runner.GetRuntimeOptions().SetCompilerHotnessThreshold(HOTNESS_THRESHOLD);
    runner.GetRuntimeOptions().SetCompilerEnableJit(true);
    runner.GetCompilerOptions().SetCompilerNonOptimizing(true);

    ScopeEvents scopeEvents;
    runner.Run(OSR_AFTER_IFRAME_RESTORE_ACC_AFTER_VOID, static_cast<ssize_t>(0U));
    auto events = Events::CastTo<Events::MEMORY>();
    auto osrEvents = events->Select<events::EventsMemory::OsrEntryEvent>();
    ASSERT_EQ(osrEvents.size(), 1U);
}

static constexpr auto OSR_AFTER_CFRAME_SOURCE = R"(
    .function i32 main() {
        movi v0, 0
        movi v1, 11
    loop:
        lda v1
        jeq v0, exit
        inci v0, 1
        movi v2, 0
        call f1, v2
        jmp loop
    exit:
        movi v2, 1
        call f1, v2
        return
    }

    .function i32 f1(i32 a0) {
        ldai 0
        jeq a0, exit
        call f2
    exit:
        return
    }

    .function i32 f2() {
        movi v0, 0
        movi v1, 30
        movi v2, 0
    loop:
        lda v0
        jeq v1, exit
        add2 v2
        sta v2
        inci v0, 1
        jmp loop
    exit:
        lda v2
        return
    }
)";

TEST_F(OsrTest, OsrAfterCFrameNonOptimizing)
{
    PandaRunner runner;
    runner.GetRuntimeOptions().SetCompilerHotnessThreshold(HOTNESS_THRESHOLD);
    runner.GetRuntimeOptions().SetCompilerEnableJit(true);
    runner.GetCompilerOptions().SetCompilerNonOptimizing(true);

    ScopeEvents scopeEvents;
    runner.Run(OSR_AFTER_CFRAME_SOURCE, 435U);
    auto events = Events::CastTo<Events::MEMORY>();
    auto osrEvents = events->Select<events::EventsMemory::OsrEntryEvent>();
    ASSERT_EQ(osrEvents.size(), 1U);
    ASSERT_EQ(osrEvents[0U]->kind, events::OsrEntryKind::AFTER_CFRAME);
    ASSERT_EQ(osrEvents[0U]->result, events::OsrEntryResult::SUCCESS);
}

TEST_F(OsrTest, OsrAfterCFrameOptimizing)
{
    PandaRunner runner;
    runner.GetRuntimeOptions().SetCompilerHotnessThreshold(HOTNESS_THRESHOLD);
    runner.GetRuntimeOptions().SetCompilerEnableJit(true);
    runner.GetCompilerOptions().SetCompilerNonOptimizing(false);
    runner.GetCompilerOptions().SetCompilerInlining(false);

    ScopeEvents scopeEvents;
    runner.Run(OSR_AFTER_CFRAME_SOURCE, 435U);
    auto events = Events::CastTo<Events::MEMORY>();
    auto osrEvents = events->Select<events::EventsMemory::OsrEntryEvent>();
    ASSERT_EQ(osrEvents.size(), 1U);
    ASSERT_EQ(osrEvents[0U]->kind, events::OsrEntryKind::AFTER_CFRAME);
    ASSERT_EQ(osrEvents[0U]->result, events::OsrEntryResult::SUCCESS);
}

TEST_F(OsrTest, OsrAfterCFrameOptimizingWithInlining)
{
    PandaRunner runner;
    runner.GetRuntimeOptions().SetCompilerHotnessThreshold(HOTNESS_THRESHOLD);
    runner.GetRuntimeOptions().SetCompilerEnableJit(true);
    runner.GetCompilerOptions().SetCompilerNonOptimizing(false);
    runner.GetCompilerOptions().SetCompilerInlining(true);
    runner.GetCompilerOptions().SetCompilerInliningBlacklist({"_GLOBAL::f2"});

    ScopeEvents scopeEvents;
    runner.Run(OSR_AFTER_CFRAME_SOURCE, 435U);
    auto events = Events::CastTo<Events::MEMORY>();
    auto osrEvents = events->Select<events::EventsMemory::OsrEntryEvent>();
    auto found = events->Find<events::EventsMemory::InlineEvent>(
        [](const auto &event) { return event.caller == "_GLOBAL::main" && event.callee == "_GLOBAL::f1"; });
    ASSERT_TRUE(found);
    ASSERT_EQ(osrEvents.size(), 1U);
    ASSERT_EQ(osrEvents[0U]->kind, events::OsrEntryKind::AFTER_CFRAME);
    ASSERT_EQ(osrEvents[0U]->result, events::OsrEntryResult::SUCCESS);
}

TEST_F(OsrTest, MainOsrCatchThrow)
{
    GTEST_SKIP() << "Should be skipped until try-catch processing re-designed in OSR";
    static constexpr auto SOURCE = R"(
        .record panda.ArrayIndexOutOfBoundsException <external>

        .function i32 main() {
            movi v0, 0
            movi v1, 15
            newarr v1, v1, i32[]
            movi v2, 20
        loop:
        try_begin:
            lda v2
            jeq v0, exit
            inci v0, 1
            lda v0
            starr v1, v0
            jmp loop
        try_end:
        exit:
            ldai 1
            return

        catch_block1_begin:
            ldai 123
            return

        .catch panda.ArrayIndexOutOfBoundsException, try_begin, try_end, catch_block1_begin
        }
    )";

    PandaRunner runner;
    runner.GetRuntimeOptions().SetCompilerHotnessThreshold(HOTNESS_THRESHOLD);
    runner.GetRuntimeOptions().SetCompilerEnableJit(true);
    runner.GetCompilerOptions().SetCompilerNonOptimizing(true);

    ScopeEvents scopeEvents;
    runner.Run(SOURCE, 123U);
    auto events = Events::CastTo<Events::MEMORY>();
    auto osrEvents = events->Select<events::EventsMemory::OsrEntryEvent>();
    ASSERT_EQ(osrEvents.size(), 1U);
    ASSERT_EQ(osrEvents[0U]->kind, events::OsrEntryKind::TOP_FRAME);
    ASSERT_EQ(osrEvents[0U]->result, events::OsrEntryResult::SUCCESS);
    auto deoptEvents = events->Select<events::EventsMemory::DeoptimizationEvent>();
    ASSERT_EQ(deoptEvents.size(), 1U);
    ASSERT_EQ(deoptEvents[0U]->after, events::DeoptimizationAfter::TOP);
}

static constexpr auto MAIN_OSR_CATCH_F1_THROW_SOURCE = R"(
    .record panda.ArrayIndexOutOfBoundsException <external>

    .function i32 main() {
        movi v0, 0
        movi v1, 15
        newarr v1, v1, i32[]
        movi v2, 20
    loop:
    try_begin:
        lda v2
        jeq v0, exit
        inci v0, 1
        call f1, v1, v0
        jmp loop
    try_end:
    exit:
        ldai 1
        return

    catch_block1_begin:
        ldai 123
        return

    .catch panda.ArrayIndexOutOfBoundsException, try_begin, try_end, catch_block1_begin
    }

    .function void f1(i32[] a0, i32 a1) {
        lda a1
        starr a0, a1
        return.void
    }
)";

TEST_F(OsrTest, MainOsrCatchF1Throw)
{
    GTEST_SKIP() << "Should be skipped until try-catch processing re-designed in OSR";
    PandaRunner runner;
    runner.GetRuntimeOptions().SetCompilerHotnessThreshold(HOTNESS_THRESHOLD);
    runner.GetRuntimeOptions().SetCompilerEnableJit(true);
    runner.GetCompilerOptions().SetCompilerNonOptimizing(false);
    runner.GetCompilerOptions().SetCompilerInlining(false);
    runner.GetCompilerOptions().SetCompilerRegex("(?!_GLOBAL::f1).*");

    ScopeEvents scopeEvents;
    runner.Run(MAIN_OSR_CATCH_F1_THROW_SOURCE, 123U);
    auto events = Events::CastTo<Events::MEMORY>();
    auto osrEvents = events->Select<events::EventsMemory::OsrEntryEvent>();
    ASSERT_EQ(osrEvents.size(), 1);
    ASSERT_EQ(osrEvents[0]->kind, events::OsrEntryKind::TOP_FRAME);
    ASSERT_EQ(osrEvents[0]->result, events::OsrEntryResult::SUCCESS);
    auto deoptEvents = events->Select<events::EventsMemory::DeoptimizationEvent>();
    ASSERT_EQ(deoptEvents.size(), 1);
    ASSERT_EQ(deoptEvents[0]->after, events::DeoptimizationAfter::TOP);
}

TEST_F(OsrTest, MainOsrCatchF1ThrowCompiled)
{
    GTEST_SKIP() << "Should be skipped until try-catch processing re-designed in OSR";
    PandaRunner runner;
    runner.GetRuntimeOptions().SetCompilerHotnessThreshold(HOTNESS_THRESHOLD);
    runner.GetRuntimeOptions().SetCompilerEnableJit(true);
    runner.GetCompilerOptions().SetCompilerNonOptimizing(false);
    runner.GetCompilerOptions().SetCompilerInlining(false);

    ScopeEvents scopeEvents;
    runner.Run(MAIN_OSR_CATCH_F1_THROW_SOURCE, 123U);
    auto events = Events::CastTo<Events::MEMORY>();
    auto osrEvents = events->Select<events::EventsMemory::OsrEntryEvent>();
    ASSERT_EQ(osrEvents.size(), 1);
    ASSERT_EQ(osrEvents[0]->kind, events::OsrEntryKind::TOP_FRAME);
    ASSERT_EQ(osrEvents[0]->result, events::OsrEntryResult::SUCCESS);
    auto exceptionEvents = events->Select<events::EventsMemory::ExceptionEvent>();
    ASSERT_EQ(exceptionEvents.size(), 1);
    ASSERT_EQ(exceptionEvents[0]->type, events::ExceptionType::BOUND_CHECK);
}

static constexpr auto MAIN_CATCH_F1_OSR_THROW_SOURCE = R"(
    .record panda.ArrayIndexOutOfBoundsException <external>

    .function i32 main() {
        movi v1, 15
        newarr v1, v1, i32[]
    try_begin:
        call f1, v1
    try_end:
        ldai 1
        return

    catch_block1_begin:
        ldai 123
        return

    .catch panda.ArrayIndexOutOfBoundsException, try_begin, try_end, catch_block1_begin
    }

    .function void f1(i32[] a0) {
        movi v0, 0
        movi v2, 20
    loop:
        lda v2
        jeq v0, exit
        inci v0, 1
        starr a0, v0
        jmp loop
    exit:
        return.void
    }
)";

TEST_F(OsrTest, MainCatchF1OsrThrow)
{
    GTEST_SKIP() << "Should be skipped until try-catch processing re-designed in OSR";
    PandaRunner runner;
    runner.GetRuntimeOptions().SetCompilerHotnessThreshold(HOTNESS_THRESHOLD);
    runner.GetRuntimeOptions().SetCompilerEnableJit(true);
    runner.GetCompilerOptions().SetCompilerNonOptimizing(false);
    runner.GetCompilerOptions().SetCompilerInlining(false);

    ScopeEvents scopeEvents;
    runner.Run(MAIN_CATCH_F1_OSR_THROW_SOURCE, 123U);
    auto events = Events::CastTo<Events::MEMORY>();
    auto osrEvents = events->Select<events::EventsMemory::OsrEntryEvent>();
    ASSERT_EQ(osrEvents.size(), 1U);
    ASSERT_EQ(osrEvents[0U]->kind, events::OsrEntryKind::AFTER_IFRAME);
    ASSERT_EQ(osrEvents[0U]->result, events::OsrEntryResult::SUCCESS);
    auto exceptionEvents = events->Select<events::EventsMemory::ExceptionEvent>();
    ASSERT_EQ(exceptionEvents.size(), 1U);
    ASSERT_EQ(exceptionEvents[0U]->type, events::ExceptionType::BOUND_CHECK);
}

static constexpr auto MAIN_CATCH_F1_OSR_F2_THROW_SOURCE = R"(
    .record panda.ArrayIndexOutOfBoundsException <external>

    .function i32 main() {
        movi v1, 15
        newarr v1, v1, i32[]
    try_begin:
        call f1, v1
    try_end:
        ldai 1
        return

    catch_block1_begin:
        ldai 123
        return

    .catch panda.ArrayIndexOutOfBoundsException, try_begin, try_end, catch_block1_begin
    }

    .function void f1(i32[] a0) {
        movi v0, 0
        movi v2, 20
    loop:
        lda v2
        jeq v0, exit
        inci v0, 1
        call f2, a0, v0
        jmp loop
    exit:
        return.void
    }

    .function void f2(i32[] a0, i32 a1) {
        lda a1
        starr a0, a1
        return.void
    }
)";

TEST_F(OsrTest, MainCatchF1OsrF2Throw)
{
    GTEST_SKIP() << "Should be skipped until try-catch processing re-designed in OSR";
    PandaRunner runner;
    runner.GetRuntimeOptions().SetCompilerHotnessThreshold(HOTNESS_THRESHOLD);
    runner.GetRuntimeOptions().SetCompilerEnableJit(true);
    runner.GetCompilerOptions().SetCompilerNonOptimizing(false);
    runner.GetCompilerOptions().SetCompilerInlining(false);
    runner.GetCompilerOptions().SetCompilerRegex("(?!_GLOBAL::f2).*");

    ScopeEvents scopeEvents;
    runner.Run(MAIN_CATCH_F1_OSR_F2_THROW_SOURCE, 123U);
    auto events = Events::CastTo<Events::MEMORY>();
    auto osrEvents = events->Select<events::EventsMemory::OsrEntryEvent>();
    ASSERT_EQ(osrEvents.size(), 1);
    ASSERT_EQ(osrEvents[0]->kind, events::OsrEntryKind::AFTER_IFRAME);
    ASSERT_EQ(osrEvents[0]->result, events::OsrEntryResult::SUCCESS);
    auto deoptEvents = events->Select<events::EventsMemory::DeoptimizationEvent>();
    // Since f1 hasn't catch handler, it shouldn't be deoptimized
    ASSERT_EQ(deoptEvents.size(), 0);
    auto exceptionEvents = events->Select<events::EventsMemory::ExceptionEvent>();
    ASSERT_EQ(exceptionEvents.size(), 0);
}

TEST_F(OsrTest, MainCatchF1OsrF2ThrowCompiled)
{
    GTEST_SKIP() << "Should be skipped until try-catch processing re-designed in OSR";
    PandaRunner runner;
    runner.GetRuntimeOptions().SetCompilerHotnessThreshold(HOTNESS_THRESHOLD);
    runner.GetRuntimeOptions().SetCompilerEnableJit(true);
    runner.GetCompilerOptions().SetCompilerNonOptimizing(false);
    runner.GetCompilerOptions().SetCompilerInlining(false);

    ScopeEvents scopeEvents;
    runner.Run(MAIN_CATCH_F1_OSR_F2_THROW_SOURCE, 123U);
    auto events = Events::CastTo<Events::MEMORY>();
    auto osrEvents = events->Select<events::EventsMemory::OsrEntryEvent>();
    ASSERT_EQ(osrEvents.size(), 1);
    ASSERT_EQ(osrEvents[0]->kind, events::OsrEntryKind::AFTER_IFRAME);
    ASSERT_EQ(osrEvents[0]->result, events::OsrEntryResult::SUCCESS);
    auto exceptionEvents = events->Select<events::EventsMemory::ExceptionEvent>();
    ASSERT_EQ(exceptionEvents.size(), 1);
    ASSERT_EQ(exceptionEvents[0]->type, events::ExceptionType::BOUND_CHECK);
}

static constexpr auto MAIN_F1_OSR_CATCH_F2_THROW_SOURCE = R"(
    .record panda.ArrayIndexOutOfBoundsException <external>

    .function i32 main() {
        movi v1, 15
        newarr v1, v1, i32[]
        call f1, v1
        return
    }

    .function i32 f1(i32[] a0) {
        movi v0, 0
        movi v2, 20
    loop:
    try_begin:
        lda v2
        jeq v0, exit
        inci v0, 1
        call f2, a0, v0
        jmp loop
    try_end:

    exit:
        ldai 1
        return

    catch_block1_begin:
        ldai 123
        return

    .catch panda.ArrayIndexOutOfBoundsException, try_begin, try_end, catch_block1_begin
    }

    .function void f2(i32[] a0, i32 a1) {
        lda a1
        starr a0, a1
        return.void
    }
)";

TEST_F(OsrTest, MainF1OsrCatchF2Throw)
{
    GTEST_SKIP() << "Should be skipped until try-catch processing re-designed in OSR";
    PandaRunner runner;
    runner.GetRuntimeOptions().SetCompilerHotnessThreshold(HOTNESS_THRESHOLD);
    runner.GetRuntimeOptions().SetCompilerEnableJit(true);
    runner.GetCompilerOptions().SetCompilerNonOptimizing(false);
    runner.GetCompilerOptions().SetCompilerInlining(false);
    runner.GetCompilerOptions().SetCompilerRegex("(?!_GLOBAL::f2).*");

    ScopeEvents scopeEvents;
    runner.Run(MAIN_F1_OSR_CATCH_F2_THROW_SOURCE, 123U);
    auto events = Events::CastTo<Events::MEMORY>();
    auto osrEvents = events->Select<events::EventsMemory::OsrEntryEvent>();
    ASSERT_EQ(osrEvents.size(), 1);
    ASSERT_EQ(osrEvents[0]->methodName, "_GLOBAL::f1");
    ASSERT_EQ(osrEvents[0]->kind, events::OsrEntryKind::AFTER_IFRAME);
    ASSERT_EQ(osrEvents[0]->result, events::OsrEntryResult::SUCCESS);
    auto deoptEvents = events->Select<events::EventsMemory::DeoptimizationEvent>();
    ASSERT_EQ(deoptEvents.size(), 1);
    ASSERT_EQ(deoptEvents[0]->methodName, "_GLOBAL::f1");
    ASSERT_EQ(deoptEvents[0]->after, events::DeoptimizationAfter::IFRAME);
}

TEST_F(OsrTest, MainF1OsrCatchF2ThrowCompiled)
{
    GTEST_SKIP() << "Should be skipped until try-catch processing re-designed in OSR";
    PandaRunner runner;
    runner.GetRuntimeOptions().SetCompilerHotnessThreshold(HOTNESS_THRESHOLD);
    runner.GetRuntimeOptions().SetCompilerEnableJit(true);
    runner.GetCompilerOptions().SetCompilerNonOptimizing(false);
    runner.GetCompilerOptions().SetCompilerInlining(false);

    ScopeEvents scopeEvents;
    runner.Run(MAIN_F1_OSR_CATCH_F2_THROW_SOURCE, 123U);
    auto events = Events::CastTo<Events::MEMORY>();
    auto osrEvents = events->Select<events::EventsMemory::OsrEntryEvent>();
    ASSERT_EQ(osrEvents.size(), 1);
    ASSERT_EQ(osrEvents[0]->methodName, "_GLOBAL::f1");
    ASSERT_EQ(osrEvents[0]->kind, events::OsrEntryKind::AFTER_IFRAME);
    ASSERT_EQ(osrEvents[0]->result, events::OsrEntryResult::SUCCESS);
    auto deoptEvents = events->Select<events::EventsMemory::DeoptimizationEvent>();
    auto exceptionEvents = events->Select<events::EventsMemory::ExceptionEvent>();
    ASSERT_EQ(exceptionEvents.size(), 1);
    ASSERT_EQ(exceptionEvents[0]->methodName, "_GLOBAL::f2");
    ASSERT_EQ(exceptionEvents[0]->type, events::ExceptionType::BOUND_CHECK);
}

#if !defined(USE_ADDRESS_SANITIZER) && defined(PANDA_TARGET_AMD64)
TEST_F(OsrTest, BoundTest)
{
    PandaRunner runner;
    runner.GetRuntimeOptions().SetCompilerHotnessThreshold(0);
    runner.GetRuntimeOptions().SetCompilerEnableJit(true);
    runner.GetCompilerOptions().SetCompilerNonOptimizing(true);

    PandaString start =
        "    .function i64 main(i64 a0) {"
        "        lda a0";

    PandaString end =
        "        return "
        "        }"
        "    ";

    std::stringstream text("");

    uint64_t instsPerByte = 32;
    uint64_t maxBitsInInst = GetInstructionSizeBits(RUNTIME_ARCH);
    uint64_t instCount = runner.GetCompilerOptions().GetCompilerMaxGenCodeSize() / (instsPerByte * maxBitsInInst);

    for (uint64_t i = 0; i < instCount; ++i) {
        text << "       addi 1\n";
    }

    std::string boundTest = std::string(start) + std::string(text.str()) + std::string(end);

    ScopeEvents scopeEvents;

    runner.Run(boundTest, 123_I);
    auto events = Events::CastTo<Events::MEMORY>();
    auto osrEvents = events->Select<events::EventsMemory::OsrEntryEvent>();
    ASSERT_EQ(osrEvents.size(), 1);
}
#endif
// NOLINTEND(readability-magic-numbers)

}  // namespace ark::test
