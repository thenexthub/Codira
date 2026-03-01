/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "gtest/gtest.h"

#include "asmjit/x86.h"

namespace ark::compiler {
using namespace asmjit;

class AsmJitTest : public ::testing::Test {};

TEST_F(AsmJitTest, HelloWorld)
{
    // Runtime designed for JIT code execution.
    JitRuntime rt;

    // Holds code and relocation information.
    CodeHolder code;
    code.init(rt.environment());

    x86::Assembler a(&code);
    a.mov(x86::eax, 1);
    a.ret();

    // Signature of the generated function.
    typedef int (*Func)(void);
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
    CodeHolder code;
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
    typedef size_t (*Func)(size_t, size_t);
    Func fn {nullptr};

    // Add the generated code to the runtime.
    Error err = rt.add(&fn, &code);
    ASSERT_FALSE(err);

    size_t result {fn(size_t(2), size_t(3))};
    ASSERT_EQ(size_t(5U), result);
}

TEST_F(AsmJitTest, AddExplicit)
{
    Environment env = hostEnvironment();
    JitAllocator allocator;

    CodeHolder code;
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
    size_t estimated_size = code.codeSize();

    // Allocate memory for the function and relocate it there.
    void *ro_ptr;
    void *rw_ptr;
    Error err = allocator.alloc(&ro_ptr, &rw_ptr, estimated_size);
    ASSERT_FALSE(err);

    // Relocate to the base-address of the allocated memory.
    code.relocateToBase(reinterpret_cast<uintptr_t>(rw_ptr));
    size_t code_size = code.codeSize();

    code.copyFlattenedData(rw_ptr, code_size, CodeHolder::kCopyPadSectionBuffer);

    // Execute the function and test whether it works.
    typedef size_t (*Func)(size_t lhs, size_t rhs);
    Func fn = (Func)ro_ptr;

    size_t result {fn(size_t(2), size_t(3))};
    ASSERT_EQ(size_t(5U), result);

    err = allocator.release(ro_ptr);
    ASSERT_FALSE(err);
}
}  // namespace ark::compiler
