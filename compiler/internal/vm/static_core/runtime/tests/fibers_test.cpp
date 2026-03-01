/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include <securec.h>

#include <cstdlib>
#include <memory>
#include <vector>

#include "gtest/gtest.h"

#include "libarkbase/macros.h"
#include "libarkbase/utils/utils.h"
#include "runtime/fibers/fiber_context.h"

namespace ark::fibers::test {

/// The fixture
class FibersTest : public testing::Test {
public:
    size_t GetEntryExecCounter()
    {
        return entryExecCounter_;
    }

    void IncEntryExecCounter()
    {
        ++entryExecCounter_;
    }

private:
    size_t entryExecCounter_ = 0;
};

/// A fiber instance: provides stack, registers the EP
class Fiber final {
public:
    NO_COPY_SEMANTIC(Fiber);
    NO_MOVE_SEMANTIC(Fiber);

    static constexpr size_t STACK_SIZE = 4096 * 32;
    static constexpr uint32_t MAGIC = 0xC001BEAF;

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    explicit Fiber(FibersTest &fixture, Fiber *parent = nullptr, fibers::FiberEntry entry = nullptr)
        : fixture_(fixture), parent_(parent), magic_(MAGIC)
    {
        stack_ = new uint8_t[STACK_SIZE];
        fibers::GetCurrentContext(&ctx_);
        if (entry != nullptr) {
            fibers::UpdateContext(&ctx_, entry, this, stack_, STACK_SIZE);
        }
    }

    ~Fiber()
    {
        delete[] stack_;
    }

    fibers::FiberContext *GetContextPtr()
    {
        return &ctx_;
    }

    Fiber *GetParent()
    {
        return parent_;
    }

    uint32_t GetMagic()
    {
        return magic_;
    }

    void RegisterEntryExecution()
    {
        fixture_.IncEntryExecCounter();
    }

private:
    FibersTest &fixture_;
    fibers::FiberContext ctx_;
    uint8_t *stack_ = nullptr;
    Fiber *parent_ = nullptr;

    volatile uint32_t magic_;
};

/// Regular fiber EP: switches to parent fiber on return
extern "C" void Entry(void *currentFiber)
{
    ASSERT_TRUE(currentFiber != nullptr);
    auto *fCur = reinterpret_cast<Fiber *>(currentFiber);
    ASSERT_EQ(fCur->GetMagic(), Fiber::MAGIC);
    // increment the EP execution counter
    fCur->RegisterEntryExecution();
    // EOT: switch to parent (otherwise fibers lib will call abort())
    ASSERT_TRUE(fCur->GetParent() != nullptr);
    fibers::SwitchContext(fCur->GetContextPtr(), fCur->GetParent()->GetContextPtr());
}

/// Empty fiber EP: checks what happens on a simple return from a fiber
extern "C" void EmptyEntry([[maybe_unused]] void *currentFiber)
{
    // NOLINTNEXTLINE(readability-redundant-control-flow)
    return;
}

/// The EP that switches back to parent in a loop
extern "C" void LoopedSwitchEntry(void *currentFiber)
{
    ASSERT_TRUE(currentFiber != nullptr);
    auto *fCur = reinterpret_cast<Fiber *>(currentFiber);
    ASSERT_EQ(fCur->GetMagic(), Fiber::MAGIC);

    // some non-optimized counters...
    volatile size_t counterInt = 0;
    volatile double counterDbl = 0;
    while (true) {
        // ...and their modification...
        ++counterInt;
        counterDbl = static_cast<double>(counterInt);

        fCur->RegisterEntryExecution();
        ASSERT_TRUE(fCur->GetParent() != nullptr);
        fibers::SwitchContext(fCur->GetContextPtr(), fCur->GetParent()->GetContextPtr());

        // ...and the check for the counters to stay correct after the switch
        ASSERT_DOUBLE_EQ(counterDbl, static_cast<double>(counterInt));
    }
}

/* Tests*/

/// Create fiber, switch to it, execute till its end, switch back
TEST_F(FibersTest, SwitchExecuteSwitchBack)
{
    Fiber fInit(*this);
    Fiber f1(*this, &fInit, Entry);
    fibers::SwitchContext(fInit.GetContextPtr(), f1.GetContextPtr());

    ASSERT_EQ(GetEntryExecCounter(), 1);
}

/**
 * Create several fibers, organizing them in a chain using the "parent" field.
 * Switch to the last one, wait till the whole chain is executed
 */
TEST_F(FibersTest, ChainSwitch)
{
    Fiber fInit(*this);
    Fiber f1(*this, &fInit, Entry);
    Fiber f2(*this, &f1, Entry);
    Fiber f3(*this, &f2, Entry);
    fibers::SwitchContext(fInit.GetContextPtr(), f3.GetContextPtr());

    ASSERT_EQ(GetEntryExecCounter(), 3U);
}

/// Create the child fiber, then switch context back and forth several times in a loop
TEST_F(FibersTest, LoopedSwitch)
{
    constexpr size_t SWITCHES = 10;

    Fiber fInit(*this);
    Fiber fTarget(*this, &fInit, LoopedSwitchEntry);

    // some unoptimized counters
    volatile size_t counterInt = 0;
    volatile double counterDbl = 0;
    for (size_t i = 0; i < SWITCHES; ++i) {
        counterInt = i;
        counterDbl = static_cast<double>(i);

        // do something with the context before the next switch
        double n1 = 0;
        double n2 = 0;
        // NOLINTNEXTLINE(cert-err34-c, cppcoreguidelines-pro-type-vararg)
        [[maybe_unused]] auto res = sscanf_s("1.23 4.56", "%lf %lf", &n1, &n2);
        ASSERT(res != -1);

        fibers::SwitchContext(fInit.GetContextPtr(), fTarget.GetContextPtr());

        // check that no corruption occurred
        ASSERT_DOUBLE_EQ(n1, 1.23_D);
        ASSERT_DOUBLE_EQ(n2, 4.56_D);

        // counters should not be corrupted after a switch
        ASSERT_EQ(counterInt, i);
        ASSERT_DOUBLE_EQ(counterDbl, static_cast<double>(i));
    }

    ASSERT_EQ(GetEntryExecCounter(), SWITCHES);
}

using FibersDeathTest = FibersTest;
/**
 * Death test. Creates an orphaned fiber that will silently return from its entry function.
 * Should cause the program to be abort()-ed
 */
TEST_F(FibersDeathTest, DISABLED_AbortOnFiberReturn)
{
    // Death test under qemu_arm32 is not compatible with 'threadsafe' death test style flag
#if defined(PANDA_TARGET_ARM32) && defined(PANDA_QEMU_BUILD)
    testing::FLAGS_gtest_death_test_style = "fast";
#endif
    Fiber fInit(*this);
    Fiber fAborts(*this, nullptr, EmptyEntry);
    EXPECT_EXIT(fibers::SwitchContext(fInit.GetContextPtr(), fAborts.GetContextPtr()), testing::KilledBySignal(SIGABRT),
                ".*");
}

}  // namespace ark::fibers::test
