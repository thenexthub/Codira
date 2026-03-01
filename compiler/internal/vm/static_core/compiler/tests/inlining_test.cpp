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

#include "optimizer/optimizations/inlining.h"
#include "libarkbase/macros.h"
#include "unit_test.h"
#include "libarkbase/utils/logger.h"
#include "compiler_logger.h"
#include "libarkbase/events/events.h"

namespace ark::compiler {
class InliningTest : public AsmTest {
public:
    InliningTest()
    {
        g_options.SetCompilerInlining(true);
    }

    InlineCachesInterface *GetInlineCaches() override
    {
        return ic_;
    }

    void SetInlineCaches(InlineCachesInterface *ic)
    {
        ic_ = ic;
    }

private:
    InlineCachesInterface *ic_ {nullptr};
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

class ScopedDontSkipThrow {
public:
    ScopedDontSkipThrow()
        : skipThrow_(g_options.IsCompilerInliningSkipThrowBlocks()),
          skipThrowMethods_(g_options.IsCompilerInliningSkipAlwaysThrowMethods())
    {
        g_options.SetCompilerInliningSkipThrowBlocks(false);
        g_options.SetCompilerInliningSkipAlwaysThrowMethods(false);
    }
    NO_COPY_SEMANTIC(ScopedDontSkipThrow);
    NO_MOVE_SEMANTIC(ScopedDontSkipThrow);

    ~ScopedDontSkipThrow()
    {
        g_options.SetCompilerInliningSkipThrowBlocks(skipThrow_);
        g_options.SetCompilerInliningSkipAlwaysThrowMethods(skipThrowMethods_);
    }

private:
    bool skipThrow_;
    bool skipThrowMethods_;
};

auto PopulateInstructionMap(Graph *graph)
{
    ArenaUnorderedMap<Opcode, ArenaVector<Inst *>> res(graph->GetLocalAllocator()->Adapter());
    for (auto bb : *graph) {
        for (auto inst : bb->Insts()) {
            auto it = res.try_emplace(inst->GetOpcode(), graph->GetLocalAllocator()->Adapter());
            it.first->second.push_back(inst);
        }
    }
    return res;
}

TEST_F(InliningTest, Simple)
{
    auto source = R"(
.function u1 main(u32 a0, u32 a1) {
    movi v0, 1
    movi v1, 2
    or a0, v1
    lda a0
    add a1, v0
    lda a1
    call.short inline, a0, a1
    return
}

.function u32 inline(u32 a0, u32 a1) {
    movi v2, 2023
    movi v3, 2
    add a0, v2
    sta v4
    sub a1, v3
    mul2 v4
    return
}
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    ASSERT_TRUE(GetGraph()->RunPass<Inlining>());
    ASSERT_EQ(GetGraph()->GetPassManager()->GetStatistics()->GetInlinedMethods(), 1U);
}

TEST_F(InliningTest, TwoMethods)
{
    auto source = R"(
.function u1 main(u32 a0, u32 a1) {
    movi v0, 1
    movi v1, 2
    or a0, v1
    lda a0
    add a1, v0
    lda a1
    call.short inline1, a0, a1
    sta v2
    call.short inline2, v2, a1
    return
}

.function u32 inline1(u32 a0, u32 a1) {
    movi v2, 2023
    movi v3, 2
    add a0, v2
    sta v4
    sub a1, v3
    mul2 v4
    return
}

.function u32 inline2(u32 a0, u32 a1) {
    movi v2, 2023
    movi v3, 2
    add a0, v2
    sta v4
    sub a1, v3
    mul2 v4
    return
}
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    ASSERT_TRUE(GetGraph()->RunPass<Inlining>());
    ASSERT_EQ(GetGraph()->GetPassManager()->GetStatistics()->GetInlinedMethods(), 2U);
}

TEST_F(InliningTest, MultipleReturns)
{
    auto source = R"(
.function f64 main(f64 a0, f64 a1) {
    call.short inline, a0, a1
    return.64
}

.function f64 inline(f64 a0) {
    fldai.64 0.0
    fcmpl.64 a0
    jlez label
    lda.64 a0
    fneg.64
    return.64
label:
    lda.64 a0
    return.64
})";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    ASSERT_TRUE(GetGraph()->RunPass<Inlining>());
    ASSERT_EQ(GetGraph()->GetPassManager()->GetStatistics()->GetInlinedMethods(), 1U);
}

TEST_F(InliningTest, WithCalls)
{
    auto source = R"(
.function u32 main(u32 a0, u32 a1) {
    sub a0, a1
    sta a0
    call.short inline, a0, a0
    addi 1
    return
}
.function u32 inline(u32 a0) {
    lda a0
    not
    jlez label
    sta a0
    call.short fn, a0, a0
    return
label:
    lda a0
    return
}
.function u32 fn(u32 a0) {
    ldai 14
    sub2 a0
    return
}
)";
    g_options.SetCompilerInlineSimpleOnly(false);
    ASSERT_TRUE(ParseToGraph(source, "main"));
    ASSERT_TRUE(GetGraph()->RunPass<Inlining>());
    ASSERT_EQ(GetGraph()->GetPassManager()->GetStatistics()->GetInlinedMethods(), 2U);
    auto instMap = PopulateInstructionMap(GetGraph());
    ASSERT_EQ(instMap.at(Opcode::ReturnInlined).size(), 1U);
    auto &callInsts = instMap.at(Opcode::CallStatic);
    ASSERT_EQ(callInsts.size(), 1U);
    ASSERT_TRUE(static_cast<CallInst *>(callInsts[0U])->IsInlined());
    ASSERT_TRUE(instMap.at(Opcode::ReturnInlined)[0U]->GetInput(0U).GetInst()->GetOpcode() == Opcode::SaveState);
}

TEST_F(InliningTest, WithLoop)
{
    auto source = R"(
.function u32 main(u32 a0, u32 a1) {
    sub a0, a1
    sta a0
    call.short inline, a0, a0
    addi 1
    return
}
.function u32 inline(u32 a0) {
    lda a0
    not
loop:
    jlez exit
    subi 1
    jmp loop
exit:
    return
}
)";
    g_options.SetCompilerInlineSimpleOnly(false);
    auto useSafepointGlobal = g_options.IsCompilerUseSafepoint();
    g_options.SetCompilerUseSafepoint(true);
    ASSERT_TRUE(ParseToGraph(source, "main"));
    ASSERT_FALSE(GetGraph()->HasLoop());
    ASSERT_TRUE(GetGraph()->RunPass<Inlining>());
    ASSERT_EQ(GetGraph()->GetPassManager()->GetStatistics()->GetInlinedMethods(), 1U);
    auto instMap = PopulateInstructionMap(GetGraph());
    ASSERT_EQ(instMap.at(Opcode::ReturnInlined).size(), 1U);
    ASSERT_EQ(instMap.at(Opcode::CallStatic).size(), 1U);
    ASSERT_TRUE(GetGraph()->RunPass<LoopAnalyzer>());
    ASSERT_TRUE(GetGraph()->HasLoop());
    g_options.SetCompilerUseSafepoint(useSafepointGlobal);
}

TEST_F(InliningTest, WithChecks)
{
    auto source = R"(
.function u32 main(u32 a0, u32 a1) {
    sub a0, a1
    sta a0
    call.short inline, a0, a0
    addi 1
    return
}
.function u32 inline(u32 a0) {
    ldai 123
    div2 a0
    return
}
)";
    g_options.SetCompilerInlineSimpleOnly(false);
    ASSERT_TRUE(ParseToGraph(source, "main"));
    ASSERT_TRUE(GetGraph()->RunPass<Inlining>());
    ASSERT_EQ(GetGraph()->GetPassManager()->GetStatistics()->GetInlinedMethods(), 1U);
    auto instMap = PopulateInstructionMap(GetGraph());
    ASSERT_EQ(instMap.at(Opcode::ReturnInlined).size(), 1U);
    ASSERT_EQ(instMap.at(Opcode::CallStatic).size(), 1U);
    ASSERT_TRUE(static_cast<CallInst *>(instMap.at(Opcode::CallStatic)[0U])->IsInlined());
    ASSERT_TRUE(instMap.at(Opcode::ReturnInlined)[0U]->GetInput(0U).GetInst()->GetOpcode() == Opcode::SaveState);
}

TEST_F(InliningTest, InliningMethodWithDivu2)
{
    auto source = R"(.function u1 main() {
    ldai.64 0
    movi.64 v0, 0x8000000000000000
    divu2.64 v0
    movi.64 v0, 0
    ucmp.64 v0
#   check positive
    movi v0, 0 # expected result
    jne v0, exit_failure
    ldai 0 # passed
    return
exit_failure:
    ldai 1 # failed
    return
}
#
.function u1 main_exitcode_wrapper() {
    call main
    jeqz wrapper_exit_positive
    ldai 81
    return
wrapper_exit_positive:
    ldai 80
    return
}
    )";
    ASSERT_TRUE(ParseToGraph(source, "main_exitcode_wrapper"));
    ASSERT_TRUE(GetGraph()->RunPass<Inlining>());
    auto instMap = PopulateInstructionMap(GetGraph());
    ASSERT_EQ(instMap.at(Opcode::ReturnInlined).size(), 1U);
    auto &callInsts = instMap.at(Opcode::CallStatic);
    ASSERT_EQ(callInsts.size(), 1U);
    ASSERT_TRUE(static_cast<CallInst *>(callInsts[0U])->IsInlined());
}

TEST_F(InliningTest, InlineVoidFunc)
{
    auto source = R"(.function u1 main() {
        initobj.short Test.ctor
        sta.obj v0
        movi v1, 1
        movi v2, 2
        call.range run, v0
        return.void
}
.record Test {
        i32 res
}

.function void run(Test a0, i32 a1, i32 a2) {
        mov v0, a1
        lda v0
        jltz label_0
        mov.obj v0, a0
        mov.obj v1, v0
        ldobj v1, Test.res
        sta v1
        mov v2, a1
        mov v3, a2
        add v3, v2
        sta v2
        add v2, v1
        sta v1
        lda v1
        stobj v0, Test.res
        return.void
        label_0: return.void
}

.function void Test.ctor(Test a0) <ctor> {
        mov.obj v0, a0
        movi v1, 5
        lda v1
        stobj v0, Test.res
        return.void
}
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    ASSERT_TRUE(GetGraph()->RunPass<Inlining>());
    auto instMap = PopulateInstructionMap(GetGraph());
    ASSERT_EQ(instMap.at(Opcode::ReturnInlined).size(), 2U);
    auto &callInsts = instMap.at(Opcode::CallStatic);
    ASSERT_EQ(callInsts.size(), 2U);
    ASSERT_TRUE(static_cast<CallInst *>(callInsts[0U])->IsInlined());
    ASSERT_TRUE(static_cast<CallInst *>(callInsts[1U])->IsInlined());
}

TEST_F(InliningTest, VirtualProvenByNewobj)
{
#ifndef PANDA_EVENTS_ENABLED
    return;
#endif
    auto source = R"(
    .record Obj {}

    .function i32 Obj.foo(Obj a0) {
        ldai 42
        return
    }

    .function u1 main() {
        newobj v0, Obj
        call.virt Obj.foo, v0
        return
    }

    )";
    ScopeEvents scopeEvents;

    ASSERT_TRUE(ParseToGraph(source, "main"));
    ASSERT_TRUE(GetGraph()->RunPass<Inlining>());
    auto events = Events::CastTo<Events::MEMORY>();
    auto inlineEvents = events->Select<events::EventsMemory::InlineEvent>();
    ASSERT_EQ(inlineEvents.size(), 1U);
    ASSERT_EQ(inlineEvents[0U]->result, events::InlineResult::SUCCESS);
    ASSERT_EQ(inlineEvents[0U]->caller, "_GLOBAL::main");
    ASSERT_EQ(inlineEvents[0U]->callee, "Obj::foo");
}

TEST_F(InliningTest, VirtualProvenByNewArray)
{
#ifndef PANDA_EVENTS_ENABLED
    return;
#endif
    auto source = R"(
    .record Obj {}

    .function i32 Obj.foo(Obj a0) {
        ldai 42
        return
    }

    .function u1 main() {
        movi v0, 4
        newarr v0, v0, Obj[]
        ldai 1
        ldarr.obj v0
        sta.obj v1
        call.virt Obj.foo, v1
        return
    }
    )";
    ScopeEvents scopeEvents;

    g_options.SetCompilerInlining(true);
    CompilerLogger::Init({"inlining"});

    ASSERT_TRUE(ParseToGraph(source, "main"));
    ASSERT_TRUE(GetGraph()->RunPass<Inlining>());
    auto events = Events::CastTo<Events::MEMORY>();
    auto inlineEvents = events->Select<events::EventsMemory::InlineEvent>();
    ASSERT_EQ(inlineEvents.size(), 1U);
    ASSERT_EQ(inlineEvents[0U]->result, events::InlineResult::SUCCESS);
    ASSERT_EQ(inlineEvents[0U]->caller, "_GLOBAL::main");
    ASSERT_EQ(inlineEvents[0U]->callee, "Obj::foo");
}

TEST_F(InliningTest, VirtualProvenByInitobj)
{
#ifndef PANDA_EVENTS_ENABLED
    return;
#endif
    auto source = R"(
    .record Obj {}

    .function void Obj.ctor(Obj a0) <ctor> {
        return.void
    }

    .function i32 Obj.foo(Obj a0) {
        ldai 42
        return
    }

    .function u1 main() {
        initobj.short Obj.ctor
        sta.obj v0
        call.virt Obj.foo, v0
        return
    }

    )";
    ScopeEvents scopeEvents;

    ASSERT_TRUE(ParseToGraph(source, "main"));
    ASSERT_TRUE(GetGraph()->RunPass<Inlining>());
    auto events = Events::CastTo<Events::MEMORY>();
    auto inlineEvents = events->Select<events::EventsMemory::InlineEvent>();
    ASSERT_EQ(inlineEvents.size(), 2U);
    ASSERT_EQ(inlineEvents[0U]->result, events::InlineResult::SUCCESS);
    ASSERT_EQ(inlineEvents[0U]->caller, "_GLOBAL::main");
    ASSERT_EQ(inlineEvents[0U]->callee, "Obj::.ctor");
    ASSERT_EQ(inlineEvents[1U]->result, events::InlineResult::SUCCESS);
    ASSERT_EQ(inlineEvents[1U]->caller, "_GLOBAL::main");
    ASSERT_EQ(inlineEvents[1U]->callee, "Obj::foo");
}

TEST_F(InliningTest, InliningToInfLoop)
{
#ifndef PANDA_EVENTS_ENABLED
    return;
#endif
    auto source = R"(
    .function u32 foo_inf(i32 a0) {
    loop:
        inci a0, 1
        call foo_throw
        jmp loop
    }

    .function u32 foo_throw() {
        ldai 42
        return
    }
    )";
    ScopeEvents scopeEvents;

    ASSERT_TRUE(ParseToGraph(source, "foo_inf"));
    ASSERT_TRUE(GetGraph()->RunPass<Inlining>());
    auto events = Events::CastTo<Events::MEMORY>();
    auto inlineEvents = events->Select<events::EventsMemory::InlineEvent>();
    ASSERT_EQ(inlineEvents.size(), 1U);
    ASSERT_EQ(inlineEvents[0U]->result, events::InlineResult::SUCCESS);
    ASSERT_EQ(inlineEvents[0U]->caller, "_GLOBAL::foo_inf");
    ASSERT_EQ(inlineEvents[0U]->callee, "_GLOBAL::foo_throw");
}

TEST_F(InliningTest, DontInlineAlwaysThrow)
{
#ifndef PANDA_EVENTS_ENABLED
    return;
#endif
    auto source = R"(
    .record E {}

    .function u32 foo(i32 a0) {
        call foo_throw
        return
    }

    .function u32 foo_throw() {
        newobj v0, E
        throw v0
    }
    )";
    ScopeEvents scopeEvents;

    ASSERT_TRUE(ParseToGraph(source, "foo"));
    ASSERT_FALSE(GetGraph()->RunPass<Inlining>());
    auto events = Events::CastTo<Events::MEMORY>();
    auto inlineEvents = events->Select<events::EventsMemory::InlineEvent>();
    ASSERT_EQ(inlineEvents.size(), 1U);
    ASSERT_EQ(inlineEvents[0U]->result, events::InlineResult::UNSUITABLE);
    ASSERT_EQ(inlineEvents[0U]->caller, "_GLOBAL::foo");
    ASSERT_EQ(inlineEvents[0U]->callee, "_GLOBAL::foo_throw");
}

TEST_F(InliningTest, DontInlineIntoThrowBlock)
{
#ifndef PANDA_EVENTS_ENABLED
    return;
#endif
    auto source = R"(
    .record E {}

    .function u32 foo(i32 a0) {
        lda a0
        jeqz nothrow
        call foo_get_obj
        sta.obj v0
        throw v0
    nothrow:
        call foo_get_int
        return
    }

    .function E foo_get_obj() {
        newobj v0, E
        lda.obj v0
        return.obj
    }

    .function i32 foo_get_int() {
        ldai 42
        return
    }
    )";
    ScopeEvents scopeEvents;

    ASSERT_TRUE(ParseToGraph(source, "foo"));
    ASSERT_TRUE(GetGraph()->RunPass<Inlining>());
    auto events = Events::CastTo<Events::MEMORY>();
    auto inlineEvents = events->Select<events::EventsMemory::InlineEvent>();
    ASSERT_EQ(inlineEvents.size(), 1U);
    ASSERT_EQ(inlineEvents[0U]->result, events::InlineResult::SUCCESS);
    ASSERT_EQ(inlineEvents[0U]->caller, "_GLOBAL::foo");
    ASSERT_EQ(inlineEvents[0U]->callee, "_GLOBAL::foo_get_int");
}

TEST_F(InliningTest, InlineThrow)
{
    auto source = R"(
        .record E1 {}
        .record E2 <extends=E1> {}

        .function u1 inline() {
            newobj v0, E2

        try_begin:
            throw v0
            ldai 2
            return
        try_end:
        }

        .function u1 main() {
        try_begin:
            call inline
            return
        try_end:

        catch_block1_begin:
            ldai 1
            return

        catch_block2_begin:
            ldai 2
            return

        .catch E1, try_begin, try_end, catch_block1_begin
        .catch E2, try_begin, try_end, catch_block2_begin
        }
    )";

    [[maybe_unused]] ScopedDontSkipThrow dontSkipThrow {};

    ASSERT_TRUE(ParseToGraph(source, "main"));
    ASSERT_TRUE(GetGraph()->RunPass<Inlining>());
    ASSERT_EQ(GetGraph()->GetPassManager()->GetStatistics()->GetInlinedMethods(), 1U);
}

class InlineCachesMock : public InlineCachesInterface {
public:
    InlineCachesMock(CallKind kind, std::initializer_list<RuntimeInterface::ClassPtr> classes)
        : kind_ {kind}, classes_ {classes}
    {
    }

    CallKind GetClasses([[maybe_unused]] RuntimeInterface::MethodPtr method, [[maybe_unused]] uintptr_t unused,
                        [[maybe_unused]] ArenaVector<RuntimeInterface::ClassPtr> *classes) override
    {
        for (auto klass : classes_) {
            classes->push_back(klass);
        }
        return kind_;
    }

private:
    CallKind kind_;
    std::vector<RuntimeInterface::ClassPtr> classes_;
};

const char *CreateSourcePolymorphicInlineWithThrow()
{
    auto source = R"(
        .record Base {}
        .record A <extends=Base> {}
        .record B <extends=Base> {}
        .record E {}

        .function Base Base.process(Base a0, Base a1) <noimpl>
        .function Base A.process(A a0, Base a1) {
            lda.obj a1
            mov.null v0
            jeq.obj v0, throw_npe
            lda.obj a0
            return
        throw_npe:
            newobj v2, E
            throw v2
        }
        .function Base B.process(B a0, Base a1) {
            lda.obj a1
            mov.null v0
            jeq.obj v0, throw_npe
            lda.obj a0
            return
        throw_npe:
            newobj v2, E
            throw v2
        }

        .function Base run(Base a0, Base a1) {
            call.virt Base.process, a0, a1
            return
        }
    )";
    return source;
}

static void CheckOneReturnInlined(BasicBlock *bb)
{
    auto retInlWithinBlock = 0;
    for (auto inst : bb->AllInsts()) {
        if (inst->GetOpcode() == Opcode::ReturnInlined) {
            retInlWithinBlock++;
        }
    }
    EXPECT_EQ(retInlWithinBlock, 1U) << "BB " << bb->GetId() << "should contain 1 ReturnInlined, but contains "
                                     << retInlWithinBlock;
}

static InlineCachesMock PolymorphicInlineWithThrowICM(ClassLinkerExtension *ext)
{
    return {InlineCachesInterface::CallKind::POLYMORPHIC,
            {
                reinterpret_cast<RuntimeInterface::ClassPtr>(
                    ext->GetClass(reinterpret_cast<const uint8_t *>("LA;"), false)),
                reinterpret_cast<RuntimeInterface::ClassPtr>(
                    ext->GetClass(reinterpret_cast<const uint8_t *>("LB;"), false)),
            }};
}

TEST_F(InliningTest, PolymorphicInlineWithThrow)
{
    auto source = CreateSourcePolymorphicInlineWithThrow();
    [[maybe_unused]] ScopedDontSkipThrow dontSkipThrow {};

    ASSERT_TRUE(ParseToGraph(source, "run"));
    auto cl = GetClassLinker();
    auto ext = cl->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY);

    // Cannot use ScopedManagedCodeThread here
    MTManagedThread::GetCurrent()->ManagedCodeBegin();
    auto icm = PolymorphicInlineWithThrowICM(ext);
    SetInlineCaches(&icm);
    MTManagedThread::GetCurrent()->ManagedCodeEnd();

    ASSERT_TRUE(GetGraph()->RunPass<Inlining>());
    EXPECT_EQ(GetGraph()->GetPassManager()->GetStatistics()->GetInlinedMethods(), 2U);

    auto throwBlocks = 0;
    for (auto bb : GetGraph()->GetVectorBlocks()) {
        auto *last = bb->GetLastInst();
        if (last == nullptr || last->GetOpcode() != Opcode::Throw) {
            continue;
        }
        throwBlocks++;
        CheckOneReturnInlined(bb);
    }
    EXPECT_EQ(throwBlocks, 2U) << "There should be two throw blocks, from A::process and from B::process";
}

}  // namespace ark::compiler
