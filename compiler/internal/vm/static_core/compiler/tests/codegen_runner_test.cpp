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

namespace ark::test {
class CodegenRunnerTest : public testing::Test {
public:
    CodegenRunnerTest()
        : defaultCompilerNonOptimizing_(compiler::g_options.IsCompilerNonOptimizing()),
          defaultCompilerInlining_(compiler::g_options.IsCompilerInlining()),
          defaultCompilerRegallocMask_(compiler::g_options.GetCompilerRegallocRegMask())
    {
    }

    ~CodegenRunnerTest() override
    {
        compiler::g_options.SetCompilerNonOptimizing(defaultCompilerNonOptimizing_);
        compiler::g_options.SetCompilerInlining(defaultCompilerInlining_);
        compiler::g_options.SetCompilerRegallocRegMask(defaultCompilerRegallocMask_);
    }

    NO_COPY_SEMANTIC(CodegenRunnerTest);
    NO_MOVE_SEMANTIC(CodegenRunnerTest);

    void SetUp() override
    {
#ifdef USE_ADDRESS_SANITIZER
        GTEST_SKIP();
#endif
        if constexpr (RUNTIME_ARCH == Arch::AARCH32) {
            GTEST_SKIP();
        }
    }

private:
    bool defaultCompilerNonOptimizing_;
    bool defaultCompilerInlining_;
    uint64_t defaultCompilerRegallocMask_;
};

static constexpr auto CODEGEN_OBJECT_PARAMS_SOURCE = R"(
.record Value{
    i32 a
    i64 b
    i16 c
}

.function i32 hook() {
    ldai 1
    return
}

.function i32 get_Value_a(Value a0) {
    ldobj a0, Value.a
    return
}

.function i64 get_Value_b(Value a0) {
    ldobj.64 a0, Value.b
    return.64
}

.function i32 get_Value_c(Value a0) {
    ldobj a0, Value.c
    return
}

#16:
#                    +0x800
#                    +0x200
#                     +0x80
#                     +0x20
#                      +0x8
#                      +0x2
# Result              0xAAA
#32:
#                +0x8000000
#                +0x2000000
#                 +0x800000
#                 +0x200000
#                  +0x80000
#                  +0x20000
# Result          0xAAA0000
#64:
#        +0x800000000000000
#        +0x200000000000000
#         +0x80000000000000
#         +0x20000000000000
#          +0x8000000000000
#          +0x2000000000000
# Result  0xAAA000000000000
# Result  0xAAA00000AAA0AAA

.function i64 foo(Value a0, Value a1, Value a2, Value a3, Value a4, Value a5) {
    ldai.64 0
    sta.64 v17

    # Some math for spill parameters to stack
    ldai.64 0x1234567890123456
    sta.64 v8

    ldai.64 0x1000000000000000
    sta.64 v9
    ldai.64 0x0200000000000000
    sta.64 v10
    ldai.64 0x0030000000000000
    sta.64 v11
    ldai.64 0x0004000000000000
    sta.64 v12
    ldai.64 0x0000500000000000
    sta.64 v13
    ldai.64 0x0000060000000000
    sta.64 v14
    ldai.64 0x0000007000000000
    sta.64 v15
    ldai.64 0x0000000800000000
    sta.64 v16
    ldai.64 0x0000000090000000
    sta.64 v18
    ldai.64 0x0000000000100000
    sta.64 v19
    ldai.64 0x0000000000020000
    sta.64 v20
    ldai.64 0x0000000000003000
    sta.64 v21
    ldai.64 0x0000000000000400
    sta.64 v22
    ldai.64 0x0000000000000050
    sta.64 v23
    ldai.64 0x0000000000000006
    sta.64 v24

    lda.64 v8
    sub2.64 v9
    sub2.64 v10
    sub2.64 v11
    sub2.64 v12
    sub2.64 v13
    sub2.64 v14
    sub2.64 v15
    sub2.64 v16
    sub2.64 v18
    sub2.64 v19
    sub2.64 v20
    sub2.64 v21
    sub2.64 v22
    sub2.64 v23
    sub2.64 v24

    sta.64 v25
    cmp.64 v17
    jeqz success_label
    lda.64 v25
    add2.64 v8
    add2.64 v9
    add2.64 v10
    add2.64 v11
    add2.64 v12
    add2.64 v13
    add2.64 v14
    add2.64 v15
    add2.64 v16

    return.64

success_label:

    ## here params lay on stack - need check somehow (emit in cg)
    ldai 0
    sta v0
    call.range get_Value_c, a2
    # 0x800
    add2 v0
    sta v0
    call.range get_Value_a, a5
    # 0x200000
    add2 v0
    sta v0
    call.range get_Value_c, a4
    # 0x800000
    add2 v0
    sta v0
    call.range get_Value_a, a1
    # 0x20000000
    add2 v0
    sta v0
    call.range get_Value_c, a0
    # 0x8000
    add2 v0
    sta v0
    call.range get_Value_a, a3
    # 0x2000000
    add2 v0
    sta v0
    call.range get_Value_c, a5
    # 0x20
    add2 v0
    sta v0
    call.range get_Value_a, a2
    # 0x8000000
    add2 v0
    sta v0
    call.range get_Value_c, a1
    # 0x2000
    add2 v0
    sta v0
    call.range get_Value_a, a4
    # 0x80
    add2 v0
    sta v0
    call.range get_Value_c, a3
    # 0x200
    add2 v0
    sta v0
    call.range get_Value_a, a0
    # 0x80000000
    add2 v0
    i32toi64
    sta.64 v1

    call.range get_Value_b, a5
    add2.64 v1
    sta.64 v1
    call.range get_Value_b, a0
    add2.64 v1
    sta.64 v1
    call.range get_Value_b, a4
    add2.64 v1
    sta.64 v1
    call.range get_Value_b, a1
    add2.64 v1
    sta.64 v1
    call.range get_Value_b, a3
    add2.64 v1
    sta.64 v1
    call.range get_Value_b, a2
    add2.64 v1

    return.64
}

.function i64 main() {
    newobj v0, Value
    newobj v1, Value
    newobj v2, Value
    newobj v3, Value
    newobj v4, Value
    newobj v5, Value
    ## c - i16
    ldai 0x800
    stobj v0, Value.c
    ldai 0x200
    stobj v1, Value.c
    ldai 0x80
    stobj v2, Value.c
    ldai 0x20
    stobj v3, Value.c
    ldai 0x8
    stobj v4, Value.c
    ldai 0x2
    stobj v5, Value.c
    ## a - i32
    ldai 0x8000000
    stobj v0, Value.a
    ldai 0x2000000
    stobj v1, Value.a
    ldai 0x800000
    stobj v2, Value.a
    ldai 0x200000
    stobj v3, Value.a
    ldai 0x80000
    stobj v4, Value.a
    ldai 0x20000
    stobj v5, Value.a
    ## b - i64
    ldai.64 0x800000000000000
    stobj.64 v0, Value.b
    ldai.64 0x200000000000000
    stobj.64 v1, Value.b
    ldai.64 0x80000000000000
    stobj.64 v2, Value.b
    ldai.64 0x20000000000000
    stobj.64 v3, Value.b
    ldai.64 0x8000000000000
    stobj.64 v4, Value.b
    ldai.64 0x2000000000000
    stobj.64 v5, Value.b

    ldai.64 0xaaa00000aaa0aaa
    sta.64 v7
    # Corrupt stack for call.foo
    call.short hook

    call.range  foo, v0
    sta.64 v8
    cmp.64 v7
    jeqz success_label
    lda.64 v8
    return.64

success_label:
    ldai.64 123
    return.64
}

)";

constexpr uint64_t CORRUPT_SIZE = 0xFFF;
// Allocated on stack check
[[maybe_unused]] constexpr uint64_t HOOK_OFFSET = 0x10000;
constexpr uint64_t CORRUPT_DATA = 0xABCDEF0123456789;

NO_OPTIMIZE int Callback([[maybe_unused]] uintptr_t lr, [[maybe_unused]] uintptr_t fp)
{
    std::array<uint64_t, CORRUPT_SIZE> tmp {};
    [[maybe_unused]] auto delta = bit_cast<uintptr_t>(fp) - bit_cast<uintptr_t>(&tmp);
    ASSERT(delta < HOOK_OFFSET);
    delta = std::distance(tmp.begin(), tmp.end()) * sizeof(uint64_t);
    ASSERT(delta == CORRUPT_SIZE * sizeof(uint64_t));
    for (uint64_t &i : tmp) {
        i = CORRUPT_DATA;
    }
    return 0U;
}

// NOLINTBEGIN(readability-magic-numbers)
TEST_F(CodegenRunnerTest, ObjectParams)
{
    // hi-part is 1 - to do not rewrite fp, lr in arm64
    std::array<uint64_t, 4U> regMasks {
        0U, 0xFFFFFFFFFFFFFFF0U, 0xFFFFFFFFF0FFF000U, 0xFFFFFFFFFFFFDFD6U,
        // NOTE (igorban): enable next variants:
        // 0xFFFFFFFFFF000FFF,
        // 0xFFFFFFFFF000000F,
        // 0xFFFFFFFFF000202A,
        // 0xFFFFFFFFFFFFF00F,
    };

    for (auto &hookOn : {false, true}) {
        for (auto &mask : regMasks) {
            ark::test::PandaRunner runner;
            runner.GetCompilerOptions().SetCompilerNonOptimizing(true);
            runner.GetCompilerOptions().SetCompilerInlining(false);
            runner.GetRuntimeOptions().SetCompilerHotnessThreshold(0U);
            runner.GetRuntimeOptions().SetShouldLoadBootPandaFiles(true);
            runner.GetRuntimeOptions().SetShouldInitializeIntrinsics(false);
            runner.GetCompilerOptions().SetCompilerRegallocRegMask(mask);

            if (hookOn) {
                runner.SetHook(Callback);
            }

            runner.Run(CODEGEN_OBJECT_PARAMS_SOURCE, 123U);
        }
    }
}
// NOLINTEND(readability-magic-numbers)

}  // namespace ark::test
