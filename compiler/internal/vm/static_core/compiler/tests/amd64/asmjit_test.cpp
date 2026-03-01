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

#include "gtest/gtest.h"

#include "asmjit/x86.h"

#include "libarkbase/mem/pool_manager.h"

namespace ark::compiler {
using namespace asmjit;  // NOLINT(*-build-using-namespace)

class AsmJitTest : public ::testing::Test {
public:
    AsmJitTest()
    {
        // NOLINTNEXTLINE(readability-magic-numbers)
        ark::mem::MemConfig::Initialize(64_MB, 64_MB, 64_MB, 32_MB, 0, 0);
        PoolManager::Initialize();
        allocator_ = new ArenaAllocator(SpaceType::SPACE_TYPE_COMPILER);
    }

    ~AsmJitTest() override
    {
        delete allocator_;
        PoolManager::Finalize();
        ark::mem::MemConfig::Finalize();
    }

    NO_MOVE_SEMANTIC(AsmJitTest);
    NO_COPY_SEMANTIC(AsmJitTest);

    ArenaAllocator *GetAllocator()
    {
        return allocator_;
    }

private:
    ArenaAllocator *allocator_ {nullptr};
};

TEST_F(AsmJitTest, HelloWorld)
{
    // Runtime designed for JIT code execution.
    JitRuntime rt;

    // Holds code and relocation information.
    CodeHolder code(GetAllocator());
    code.init(rt.environment());

    x86::Assembler a(&code);
    a.mov(x86::rax, 1);
    a.ret();

    // Signature of the generated function.
    using Func = int (*)();
    Func fn {nullptr};

    // Add the generated code to the runtime.
    Error err = rt.add(&fn, &code);
    ASSERT_FALSE(err);

    int result {fn()};
    ASSERT_EQ(1, result);
}

TEST_F(AsmJitTest, Add)
{
    // Runtime designed for JIT code execution.
    JitRuntime rt;

    // Holds code and relocation information.
    CodeHolder code(GetAllocator());
    code.init(rt.environment());

    // Generating code:
    x86::Assembler a(&code);
    x86::Gp lhs = a.zax();
    x86::Gp rhs = a.zcx();

    FuncDetail func;
    func.init(FuncSignatureT<size_t, size_t, size_t>(CallConv::kIdHost), code.environment());

    FuncFrame frame;
    frame.init(func);
    frame.addDirtyRegs(lhs, rhs);

    FuncArgsAssignment args(&func);
    args.assignAll(lhs, rhs);
    args.updateFuncFrame(frame);
    frame.finalize();

    a.emitProlog(frame);
    a.emitArgsAssignment(frame, args);
    a.add(lhs, rhs);
    a.emitEpilog(frame);

    // Signature of the generated function.
    using Func = size_t (*)(size_t, size_t);
    Func fn {nullptr};

    // Add the generated code to the runtime.
    Error err = rt.add(&fn, &code);
    ASSERT_FALSE(err);

    size_t result {fn(size_t(2U), size_t(3U))};
    ASSERT_EQ(size_t(5U), result);
}

TEST_F(AsmJitTest, Add2)
{
    JitRuntime rt;

    CodeHolder code(GetAllocator());
    code.init(rt.environment());

    // Generating code:
    x86::Assembler a(&code);
    a.add(x86::rdi, x86::rsi);
    a.mov(x86::rax, x86::rdi);
    a.ret();

    // Signature of the generated function.
    using Func = size_t (*)(size_t, size_t);
    Func fn {nullptr};

    // Add the generated code to the runtime.
    Error err = rt.add(&fn, &code);
    ASSERT_FALSE(err);

    size_t result {fn(size_t(2U), size_t(3U))};
    ASSERT_EQ(size_t(5U), result);
}

TEST_F(AsmJitTest, AddDouble)
{
    JitRuntime rt;

    CodeHolder code(GetAllocator());
    code.init(rt.environment());

    // Generating code:
    x86::Assembler a(&code);
    a.addsd(x86::xmm0, x86::xmm1);
    a.ret();

    // Signature of the generated function.
    using Func = double (*)(double, double);
    Func fn {nullptr};

    // Add the generated code to the runtime.
    Error err = rt.add(&fn, &code);
    ASSERT_FALSE(err);

    double result {fn(2.0F, 3.0F)};  // NOLINT(readability-magic-numbers)
    ASSERT_EQ(5.0F, result);
}

TEST_F(AsmJitTest, AddExplicit)
{
    Environment env = hostEnvironment();
    JitAllocator allocator;

    CodeHolder code(GetAllocator());
    code.init(env);

    // Generating code:
    x86::Assembler a(&code);
    x86::Gp lhs = a.zax();
    x86::Gp rhs = a.zcx();

    FuncDetail func;
    func.init(FuncSignatureT<size_t, size_t, size_t>(CallConv::kIdHost), code.environment());

    FuncFrame frame;
    frame.init(func);
    frame.addDirtyRegs(lhs, rhs);

    FuncArgsAssignment args(&func);
    args.assignAll(lhs, rhs);
    args.updateFuncFrame(frame);
    frame.finalize();

    a.emitProlog(frame);
    a.emitArgsAssignment(frame, args);
    a.add(lhs, rhs);
    a.emitEpilog(frame);

    code.flatten();
    code.resolveUnresolvedLinks();
    size_t estimatedSize = code.codeSize();

    // Allocate memory for the function and relocate it there.
    void *roPtr;
    void *rwPtr;
    Error err = allocator.alloc(&roPtr, &rwPtr, estimatedSize);
    ASSERT_FALSE(err);

    // Relocate to the base-address of the allocated memory.
    code.relocateToBase(reinterpret_cast<uintptr_t>(rwPtr));
    size_t codeSize = code.codeSize();

    code.copyFlattenedData(rwPtr, codeSize, CodeHolder::kCopyPadSectionBuffer);

    // Execute the function and test whether it works.
    using Func = size_t (*)(size_t lhs, size_t rhs);
    Func fn = reinterpret_cast<Func>(roPtr);

    size_t result {fn(size_t(2U), size_t(3U))};
    ASSERT_EQ(size_t(5U), result);

    err = allocator.release(roPtr);
    ASSERT_FALSE(err);
}
}  // namespace ark::compiler
