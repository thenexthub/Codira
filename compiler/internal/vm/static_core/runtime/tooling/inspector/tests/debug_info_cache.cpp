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

#include "debugger/debug_info_cache.h"

#include "gtest/gtest.h"

#include "assembly-emitter.h"
#include "assembly-parser.h"
#include "runtime.h"
#include "runtime_options.h"
#include "thread_scopes.h"

#include "test_frame.h"

// NOLINTBEGIN

namespace ark::tooling::inspector::test {

static constexpr const char *g_source = R"(
    .record Test {}

    .function i32 Test.foo(u64 a0, u64 a1) {
        mov v0, v1         # line 2, offset 0, 1
        mov v100, v101     # line 3, offset 2, 3, 4
        movi v0, 4         # line 4, offset 5, 6
        ldai 222           # line 5, offset 7, 8, 9
        return             # line 6, offset 10
    }
)";

class DebugInfoCacheTest : public testing::Test {
protected:
    static void SetUpTestSuite()
    {
        pandasm::Parser p;

        auto res = p.Parse(g_source, SOURCE_FILE_NAME.data());
        ASSERT_TRUE(res.HasValue());
        ASSERT_TRUE(pandasm::AsmEmitter::Emit(ASM_FILE_NAME.data(), res.Value()));
        auto pf = panda_file::OpenPandaFile(ASM_FILE_NAME);
        ASSERT_NE(pf, nullptr);

        cache.AddPandaFile(*pf, true);

        RuntimeOptions options;
        options.SetShouldInitializeIntrinsics(false);
        options.SetShouldLoadBootPandaFiles(false);
        Runtime::Create(options);

        thread_ = ManagedThread::GetCurrent();
        {
            ScopedManagedCodeThread s(thread_);

            ClassLinker *classLinker = Runtime::GetCurrent()->GetClassLinker();
            classLinker->AddPandaFile(std::move(pf));

            PandaString descriptorHolder;
            const auto *descriptor = ClassHelper::GetDescriptor(utf::CStringAsMutf8("Test"), &descriptorHolder);
            auto *ext = classLinker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY);
            Class *klass = ext->GetClass(descriptor, true, ext->GetBootContext());
            ASSERT_NE(klass, nullptr);

            auto methods = klass->GetMethods();
            ASSERT_EQ(methods.size(), 1);
            methodFoo = &methods[0];
        }
    }

    static void TearDownTestSuite()
    {
        Runtime::Destroy();
    }

    static constexpr std::string_view ASM_FILE_NAME = "source.abc";
    // This test intentionally sets empty source file name to ensure that disassembly is used for debug info
    static constexpr std::string_view SOURCE_FILE_NAME = "";
    static DebugInfoCache cache;
    static ManagedThread *thread_;
    static Method *methodFoo;
};

DebugInfoCache DebugInfoCacheTest::cache {};
ManagedThread *DebugInfoCacheTest::thread_ = nullptr;
Method *DebugInfoCacheTest::methodFoo = nullptr;

TEST_F(DebugInfoCacheTest, GetCurrentLineLocations)
{
    auto fr0 = TestFrame(methodFoo, 2U);   // offset 2, line 3 of function
    auto fr1 = TestFrame(methodFoo, 6U);   // offset 6, line 4 of function
    auto fr2 = TestFrame(methodFoo, 10U);  // offset 10, line 6 of function

    auto curr = cache.GetCurrentLineLocations(fr0);
    ASSERT_EQ(curr.size(), 3U);
    ASSERT_NE(curr.find(PtLocation(ASM_FILE_NAME.data(), methodFoo->GetFileId(), 2U)), curr.end());
    ASSERT_NE(curr.find(PtLocation(ASM_FILE_NAME.data(), methodFoo->GetFileId(), 3U)), curr.end());
    ASSERT_NE(curr.find(PtLocation(ASM_FILE_NAME.data(), methodFoo->GetFileId(), 4U)), curr.end());

    curr = cache.GetCurrentLineLocations(fr1);
    ASSERT_EQ(curr.size(), 2U);
    ASSERT_NE(curr.find(PtLocation(ASM_FILE_NAME.data(), methodFoo->GetFileId(), 5U)), curr.end());
    ASSERT_NE(curr.find(PtLocation(ASM_FILE_NAME.data(), methodFoo->GetFileId(), 6U)), curr.end());

    curr = cache.GetCurrentLineLocations(fr2);
    ASSERT_EQ(curr.size(), 1);
    ASSERT_NE(curr.find(PtLocation(ASM_FILE_NAME.data(), methodFoo->GetFileId(), 10U)), curr.end());
}

TEST_F(DebugInfoCacheTest, GetLocals)
{
    static constexpr size_t ARGUMENTS_COUNT = 2U;
    static constexpr size_t LOCALS_COUNT = 103U;

    auto fr0 = TestFrame(methodFoo, 2U);  // offset 2, line 3 of function

    for (size_t i = 0; i < ARGUMENTS_COUNT; i++) {
        fr0.SetArgument(i, i + 1);
        fr0.SetArgumentKind(i, PtFrame::RegisterKind::PRIMITIVE);
    }
    for (size_t i = 0; i < LOCALS_COUNT; i++) {
        fr0.SetVReg(i, i);
        fr0.SetVRegKind(i, PtFrame::RegisterKind::PRIMITIVE);
    }
    auto mapLocals = cache.GetLocals(fr0);
    ASSERT_EQ(ARGUMENTS_COUNT + LOCALS_COUNT, mapLocals.size());

    EXPECT_NO_THROW(mapLocals.at("a0"));
    EXPECT_NO_THROW(mapLocals.at("a1"));
    EXPECT_NO_THROW(mapLocals.at("v101"));

    ASSERT_EQ(mapLocals.at("a0").GetAsU64(), 1U);
    ASSERT_EQ(mapLocals.at("a1").GetAsU64(), 2U);
    ASSERT_EQ(mapLocals.at("v101").GetAsU64(), 101U);
}

TEST_F(DebugInfoCacheTest, GetSourceLocation)
{
    auto fr0 = TestFrame(methodFoo, 2U);  // offset 2, line 3 of function
    auto fr1 = TestFrame(methodFoo, 6U);  // offset 6, line 4 of function

    std::string_view disasm_file;
    std::string_view method_name;
    size_t line_number = 0;

    cache.GetSourceLocation(fr0, disasm_file, method_name, line_number);
    ASSERT_NE(disasm_file.find(ASM_FILE_NAME.data()), std::string::npos);
    ASSERT_EQ(method_name, "foo");
    ASSERT_EQ(line_number, 3U);

    cache.GetSourceLocation(fr1, disasm_file, method_name, line_number);
    ASSERT_NE(disasm_file.find(ASM_FILE_NAME.data()), std::string::npos);
    ASSERT_EQ(method_name, "foo");
    ASSERT_EQ(line_number, 4U);

    auto set_locs = cache.GetContinueToLocations(disasm_file, 4U);
    ASSERT_EQ(set_locs.size(), 2U);
    ASSERT_NE(set_locs.find(PtLocation(ASM_FILE_NAME.data(), methodFoo->GetFileId(), 6U)), set_locs.end());
    ASSERT_NE(set_locs.find(PtLocation(ASM_FILE_NAME.data(), methodFoo->GetFileId(), 5U)), set_locs.end());

    set_locs = cache.GetContinueToLocations(disasm_file, 6U);
    ASSERT_EQ(set_locs.size(), 1);
    ASSERT_NE(set_locs.find(PtLocation(ASM_FILE_NAME.data(), methodFoo->GetFileId(), 10U)), set_locs.end());

    set_locs = cache.GetContinueToLocations(disasm_file, 1);
    ASSERT_EQ(set_locs.size(), 0);

    auto valid_locs = cache.GetValidLineNumbers(disasm_file, 0U, 100U, false);
    ASSERT_EQ(valid_locs.size(), 5U);

    ASSERT_NE(valid_locs.find(2U), valid_locs.end());
    ASSERT_NE(valid_locs.find(3U), valid_locs.end());
    ASSERT_NE(valid_locs.find(4U), valid_locs.end());
    ASSERT_NE(valid_locs.find(5U), valid_locs.end());
    ASSERT_NE(valid_locs.find(6U), valid_locs.end());

    auto s = cache.GetSourceCode(disasm_file);
    ASSERT_NE(s.find(".function i32 Test.foo(u64 a0, u64 a1)"), std::string::npos);

    s = cache.GetSourceCode("source.pa");
    ASSERT_TRUE(s.empty());

    std::set<std::string_view> sets;
    auto breaks = cache.GetBreakpointLocations([](auto) { return true; }, 4U, sets);
    ASSERT_EQ(breaks.size(), 1);
    ASSERT_EQ(sets.size(), 1);
    ASSERT_EQ(*sets.begin(), disasm_file);

    ASSERT_NE(std::find(breaks.begin(), breaks.end(), PtLocation(ASM_FILE_NAME.data(), methodFoo->GetFileId(), 5U)),
              breaks.end());
}

}  // namespace ark::tooling::inspector::test

// NOLINTEND
