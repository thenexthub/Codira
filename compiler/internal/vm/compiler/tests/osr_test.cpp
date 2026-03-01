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

#include <fstream>
#include "unit_test.h"
#include "code_info/code_info_builder.h"
#include "codegen.h"
#include "panda_runner.h"
#include "events/events.h"

using panda::panda_file::File;

namespace panda::test {
class OsrTest : public testing::Test {
public:
    OsrTest()
        : default_compiler_non_optimizing_(compiler::options.IsCompilerNonOptimizing()),
          default_compiler_inlining_(compiler::options.IsCompilerInlining()),
          default_compiler_inlining_blacklist_(compiler::options.GetCompilerInliningBlacklist()),
          default_compiler_regex_(compiler::options.GetCompilerRegex())
    {
    }

    ~OsrTest()
    {
        compiler::options.SetCompilerNonOptimizing(default_compiler_non_optimizing_);
        compiler::options.SetCompilerInlining(default_compiler_inlining_);
        compiler::options.SetCompilerInliningBlacklist(default_compiler_inlining_blacklist_);
        compiler::options.SetCompilerRegex(default_compiler_regex_);
    }
    void SetUp()
    {
#ifndef PANDA_EVENTS_ENABLED
        GTEST_SKIP();
#endif
        if constexpr (!ArchTraits<RUNTIME_ARCH>::SUPPORT_OSR) {
            GTEST_SKIP();
        }
    }

protected:
    static constexpr size_t HOTNESS_THRESHOLD = 4;

private:
    bool default_compiler_non_optimizing_;
    bool default_compiler_inlining_;
    arg_list_t default_compiler_inlining_blacklist_;
    std::string default_compiler_regex_;
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

TEST_F(OsrTest, OsrInTopFrameNonOptimizing)
{
    PandaRunner runner;
    runner.GetRuntimeOptions().SetCompilerHotnessThreshold(HOTNESS_THRESHOLD);
    runner.GetRuntimeOptions().SetCompilerEnableJit(true);
    runner.GetCompilerOptions().SetCompilerNonOptimizing(true);

    ScopeEvents scope_events;

    runner.Run(OSR_IN_TOP_FRAME_SOURCE, 435);
    auto events = Events::CastTo<Events::MEMORY>();
    auto osr_events = events->Select<events::EventsMemory::OsrEntryEvent>();
    ASSERT_EQ(osr_events.size(), 1);
}

TEST_F(OsrTest, OsrInTopFrameOptimizing)
{
    PandaRunner runner;
    runner.GetRuntimeOptions().SetCompilerHotnessThreshold(HOTNESS_THRESHOLD);
    runner.GetRuntimeOptions().SetCompilerEnableJit(true);
    runner.GetCompilerOptions().SetCompilerNonOptimizing(false);

    ScopeEvents scope_events;

    runner.Run(OSR_IN_TOP_FRAME_SOURCE, 435);
    auto events = Events::CastTo<Events::MEMORY>();
    auto osr_events = events->Select<events::EventsMemory::OsrEntryEvent>();
    ASSERT_EQ(osr_events.size(), 1);
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

    ScopeEvents scope_events;
    runner.Run(OSR_AFTER_IFRAME_SOURCE, 435);
    auto events = Events::CastTo<Events::MEMORY>();
    auto osr_events = events->Select<events::EventsMemory::OsrEntryEvent>();
    ASSERT_EQ(osr_events.size(), 1);
}

TEST_F(OsrTest, OsrAfterIFrameOptimizing)
{
    PandaRunner runner;
    runner.GetRuntimeOptions().SetCompilerHotnessThreshold(HOTNESS_THRESHOLD);
    runner.GetRuntimeOptions().SetCompilerEnableJit(true);
    runner.GetCompilerOptions().SetCompilerNonOptimizing(false);

    ScopeEvents scope_events;
    runner.Run(OSR_AFTER_IFRAME_SOURCE, 435);
    auto events = Events::CastTo<Events::MEMORY>();
    auto osr_events = events->Select<events::EventsMemory::OsrEntryEvent>();
    ASSERT_EQ(osr_events.size(), 1);
}

static constexpr auto OSR_AFTER_IFRAME_RESTORE_ACC_AFTER_VOID = R"(
    .function i32 main() {
        call f1
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

    ScopeEvents scope_events;
    runner.Run(OSR_AFTER_IFRAME_RESTORE_ACC_AFTER_VOID, static_cast<ssize_t>0);
    auto events = Events::CastTo<Events::MEMORY>();
    auto osr_events = events->Select<events::EventsMemory::OsrEntryEvent>();
    ASSERT_EQ(osr_events.size(), 1);
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

    ScopeEvents scope_events;
    runner.Run(OSR_AFTER_CFRAME_SOURCE, 435);
    auto events = Events::CastTo<Events::MEMORY>();
    auto osr_events = events->Select<events::EventsMemory::OsrEntryEvent>();
    ASSERT_EQ(osr_events.size(), 1);
    ASSERT_EQ(osr_events[0]->kind, events::OsrEntryKind::AFTER_CFRAME);
    ASSERT_EQ(osr_events[0]->result, events::OsrEntryResult::SUCCESS);
}

TEST_F(OsrTest, OsrAfterCFrameOptimizing)
{
    PandaRunner runner;
    runner.GetRuntimeOptions().SetCompilerHotnessThreshold(HOTNESS_THRESHOLD);
    runner.GetRuntimeOptions().SetCompilerEnableJit(true);
    runner.GetCompilerOptions().SetCompilerNonOptimizing(false);
    runner.GetCompilerOptions().SetCompilerInlining(false);

    ScopeEvents scope_events;
    runner.Run(OSR_AFTER_CFRAME_SOURCE, 435);
    auto events = Events::CastTo<Events::MEMORY>();
    auto osr_events = events->Select<events::EventsMemory::OsrEntryEvent>();
    ASSERT_EQ(osr_events.size(), 1);
    ASSERT_EQ(osr_events[0]->kind, events::OsrEntryKind::AFTER_CFRAME);
    ASSERT_EQ(osr_events[0]->result, events::OsrEntryResult::SUCCESS);
}

TEST_F(OsrTest, OsrAfterCFrameOptimizingWithInlining)
{
    PandaRunner runner;
    runner.GetRuntimeOptions().SetCompilerHotnessThreshold(HOTNESS_THRESHOLD);
    runner.GetRuntimeOptions().SetCompilerEnableJit(true);
    runner.GetCompilerOptions().SetCompilerNonOptimizing(false);
    runner.GetCompilerOptions().SetCompilerInlining(true);
    runner.GetCompilerOptions().SetCompilerInliningBlacklist({"_GLOBAL::f2"});

    ScopeEvents scope_events;
    runner.Run(OSR_AFTER_CFRAME_SOURCE, 435);
    auto events = Events::CastTo<Events::MEMORY>();
    auto osr_events = events->Select<events::EventsMemory::OsrEntryEvent>();
    auto found = events->Find<events::EventsMemory::InlineEvent>(
        [](const auto &event) { return event.caller == "_GLOBAL::main" && event.callee == "_GLOBAL::f1"; });
    ASSERT_TRUE(found);
    ASSERT_EQ(osr_events.size(), 1);
    ASSERT_EQ(osr_events[0]->kind, events::OsrEntryKind::AFTER_CFRAME);
    ASSERT_EQ(osr_events[0]->result, events::OsrEntryResult::SUCCESS);
}

TEST_F(OsrTest, MainOsrCatchThrow)
{
    static constexpr auto source = R"(
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

    ScopeEvents scope_events;
    runner.Run(source, 123);
    auto events = Events::CastTo<Events::MEMORY>();
    auto osr_events = events->Select<events::EventsMemory::OsrEntryEvent>();
    ASSERT_EQ(osr_events.size(), 1);
    ASSERT_EQ(osr_events[0]->kind, events::OsrEntryKind::TOP_FRAME);
    ASSERT_EQ(osr_events[0]->result, events::OsrEntryResult::SUCCESS);
    auto deopt_events = events->Select<events::EventsMemory::DeoptimizationEvent>();
    ASSERT_EQ(deopt_events.size(), 1);
    ASSERT_EQ(deopt_events[0]->after, events::DeoptimizationAfter::TOP);
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
    PandaRunner runner;
    runner.GetRuntimeOptions().SetCompilerHotnessThreshold(HOTNESS_THRESHOLD);
    runner.GetRuntimeOptions().SetCompilerEnableJit(true);
    runner.GetCompilerOptions().SetCompilerNonOptimizing(false);
    runner.GetCompilerOptions().SetCompilerInlining(false);
    runner.GetCompilerOptions().SetCompilerRegex("(?!_GLOBAL::f1).*");

    ScopeEvents scope_events;
    runner.Run(MAIN_OSR_CATCH_F1_THROW_SOURCE, 123);
    auto events = Events::CastTo<Events::MEMORY>();
    auto osr_events = events->Select<events::EventsMemory::OsrEntryEvent>();
    ASSERT_EQ(osr_events.size(), 1);
    ASSERT_EQ(osr_events[0]->kind, events::OsrEntryKind::TOP_FRAME);
    ASSERT_EQ(osr_events[0]->result, events::OsrEntryResult::SUCCESS);
    auto deopt_events = events->Select<events::EventsMemory::DeoptimizationEvent>();
    ASSERT_EQ(deopt_events.size(), 1);
    ASSERT_EQ(deopt_events[0]->after, events::DeoptimizationAfter::TOP);
}

TEST_F(OsrTest, MainOsrCatchF1ThrowCompiled)
{
    PandaRunner runner;
    runner.GetRuntimeOptions().SetCompilerHotnessThreshold(HOTNESS_THRESHOLD);
    runner.GetRuntimeOptions().SetCompilerEnableJit(true);
    runner.GetCompilerOptions().SetCompilerNonOptimizing(false);
    runner.GetCompilerOptions().SetCompilerInlining(false);

    ScopeEvents scope_events;
    runner.Run(MAIN_OSR_CATCH_F1_THROW_SOURCE, 123);
    auto events = Events::CastTo<Events::MEMORY>();
    auto osr_events = events->Select<events::EventsMemory::OsrEntryEvent>();
    ASSERT_EQ(osr_events.size(), 1);
    ASSERT_EQ(osr_events[0]->kind, events::OsrEntryKind::TOP_FRAME);
    ASSERT_EQ(osr_events[0]->result, events::OsrEntryResult::SUCCESS);
    auto exception_events = events->Select<events::EventsMemory::ExceptionEvent>();
    ASSERT_EQ(exception_events.size(), 1);
    ASSERT_EQ(exception_events[0]->type, events::ExceptionType::BOUND_CHECK);
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
    PandaRunner runner;
    runner.GetRuntimeOptions().SetCompilerHotnessThreshold(HOTNESS_THRESHOLD);
    runner.GetRuntimeOptions().SetCompilerEnableJit(true);
    runner.GetCompilerOptions().SetCompilerNonOptimizing(false);
    runner.GetCompilerOptions().SetCompilerInlining(false);

    ScopeEvents scope_events;
    runner.Run(MAIN_CATCH_F1_OSR_THROW_SOURCE, 123);
    auto events = Events::CastTo<Events::MEMORY>();
    auto osr_events = events->Select<events::EventsMemory::OsrEntryEvent>();
    ASSERT_EQ(osr_events.size(), 1);
    ASSERT_EQ(osr_events[0]->kind, events::OsrEntryKind::AFTER_IFRAME);
    ASSERT_EQ(osr_events[0]->result, events::OsrEntryResult::SUCCESS);
    auto exception_events = events->Select<events::EventsMemory::ExceptionEvent>();
    ASSERT_EQ(exception_events.size(), 1);
    ASSERT_EQ(exception_events[0]->type, events::ExceptionType::BOUND_CHECK);
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
    PandaRunner runner;
    runner.GetRuntimeOptions().SetCompilerHotnessThreshold(HOTNESS_THRESHOLD);
    runner.GetRuntimeOptions().SetCompilerEnableJit(true);
    runner.GetCompilerOptions().SetCompilerNonOptimizing(false);
    runner.GetCompilerOptions().SetCompilerInlining(false);
    runner.GetCompilerOptions().SetCompilerRegex("(?!_GLOBAL::f2).*");

    ScopeEvents scope_events;
    runner.Run(MAIN_CATCH_F1_OSR_F2_THROW_SOURCE, 123);
    auto events = Events::CastTo<Events::MEMORY>();
    auto osr_events = events->Select<events::EventsMemory::OsrEntryEvent>();
    ASSERT_EQ(osr_events.size(), 1);
    ASSERT_EQ(osr_events[0]->kind, events::OsrEntryKind::AFTER_IFRAME);
    ASSERT_EQ(osr_events[0]->result, events::OsrEntryResult::SUCCESS);
    auto deopt_events = events->Select<events::EventsMemory::DeoptimizationEvent>();
    // Since f1 hasn't catch handler, it shouldn't be deoptimized
    ASSERT_EQ(deopt_events.size(), 0);
    auto exception_events = events->Select<events::EventsMemory::ExceptionEvent>();
    ASSERT_EQ(exception_events.size(), 0);
}

TEST_F(OsrTest, MainCatchF1OsrF2ThrowCompiled)
{
    PandaRunner runner;
    runner.GetRuntimeOptions().SetCompilerHotnessThreshold(HOTNESS_THRESHOLD);
    runner.GetRuntimeOptions().SetCompilerEnableJit(true);
    runner.GetCompilerOptions().SetCompilerNonOptimizing(false);
    runner.GetCompilerOptions().SetCompilerInlining(false);

    ScopeEvents scope_events;
    runner.Run(MAIN_CATCH_F1_OSR_F2_THROW_SOURCE, 123);
    auto events = Events::CastTo<Events::MEMORY>();
    auto osr_events = events->Select<events::EventsMemory::OsrEntryEvent>();
    ASSERT_EQ(osr_events.size(), 1);
    ASSERT_EQ(osr_events[0]->kind, events::OsrEntryKind::AFTER_IFRAME);
    ASSERT_EQ(osr_events[0]->result, events::OsrEntryResult::SUCCESS);
    auto exception_events = events->Select<events::EventsMemory::ExceptionEvent>();
    ASSERT_EQ(exception_events.size(), 1);
    ASSERT_EQ(exception_events[0]->type, events::ExceptionType::BOUND_CHECK);
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
    PandaRunner runner;
    runner.GetRuntimeOptions().SetCompilerHotnessThreshold(HOTNESS_THRESHOLD);
    runner.GetRuntimeOptions().SetCompilerEnableJit(true);
    runner.GetCompilerOptions().SetCompilerNonOptimizing(false);
    runner.GetCompilerOptions().SetCompilerInlining(false);
    runner.GetCompilerOptions().SetCompilerRegex("(?!_GLOBAL::f2).*");

    ScopeEvents scope_events;
    runner.Run(MAIN_F1_OSR_CATCH_F2_THROW_SOURCE, 123);
    auto events = Events::CastTo<Events::MEMORY>();
    auto osr_events = events->Select<events::EventsMemory::OsrEntryEvent>();
    ASSERT_EQ(osr_events.size(), 1);
    ASSERT_EQ(osr_events[0]->method_name, "_GLOBAL::f1");
    ASSERT_EQ(osr_events[0]->kind, events::OsrEntryKind::AFTER_IFRAME);
    ASSERT_EQ(osr_events[0]->result, events::OsrEntryResult::SUCCESS);
    auto deopt_events = events->Select<events::EventsMemory::DeoptimizationEvent>();
    ASSERT_EQ(deopt_events.size(), 1);
    ASSERT_EQ(deopt_events[0]->method_name, "_GLOBAL::f1");
    ASSERT_EQ(deopt_events[0]->after, events::DeoptimizationAfter::IFRAME);
}

TEST_F(OsrTest, MainF1OsrCatchF2ThrowCompiled)
{
    PandaRunner runner;
    runner.GetRuntimeOptions().SetCompilerHotnessThreshold(HOTNESS_THRESHOLD);
    runner.GetRuntimeOptions().SetCompilerEnableJit(true);
    runner.GetCompilerOptions().SetCompilerNonOptimizing(false);
    runner.GetCompilerOptions().SetCompilerInlining(false);

    ScopeEvents scope_events;
    runner.Run(MAIN_F1_OSR_CATCH_F2_THROW_SOURCE, 123);
    auto events = Events::CastTo<Events::MEMORY>();
    auto osr_events = events->Select<events::EventsMemory::OsrEntryEvent>();
    ASSERT_EQ(osr_events.size(), 1);
    ASSERT_EQ(osr_events[0]->method_name, "_GLOBAL::f1");
    ASSERT_EQ(osr_events[0]->kind, events::OsrEntryKind::AFTER_IFRAME);
    ASSERT_EQ(osr_events[0]->result, events::OsrEntryResult::SUCCESS);
    auto deopt_events = events->Select<events::EventsMemory::DeoptimizationEvent>();
    auto exception_events = events->Select<events::EventsMemory::ExceptionEvent>();
    ASSERT_EQ(exception_events.size(), 1);
    ASSERT_EQ(exception_events[0]->method_name, "_GLOBAL::f2");
    ASSERT_EQ(exception_events[0]->type, events::ExceptionType::BOUND_CHECK);
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

    uint64_t insts_per_byte = 32;
    uint64_t max_bits_in_inst = GetInstructionSizeBits(RUNTIME_ARCH);
    uint64_t inst_count = runner.GetCompilerOptions().GetCompilerMaxGenCodeSize() / (insts_per_byte * max_bits_in_inst);

    for (uint64_t i = 0; i < inst_count; ++i) {
        text << "       addi 1\n";
    }

    std::string BoundTest = std::string(start) + std::string(text.str()) + std::string(end);

    ScopeEvents scope_events;

    runner.Run(BoundTest, 123);
    auto events = Events::CastTo<Events::MEMORY>();
    auto osr_events = events->Select<events::EventsMemory::OsrEntryEvent>();
    ASSERT_EQ(osr_events.size(), 1);
}
#endif
}  // namespace panda::test
