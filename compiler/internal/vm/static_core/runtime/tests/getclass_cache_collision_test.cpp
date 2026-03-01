/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>
#include <iostream>
#include <ostream>
#include <string>

#include "runtime/include/runtime.h"
#include "assembly-parser.h"
#include "compiler/tests/unit_test.h"
#include "libarkbase/os/exec.h"
#include "libarkbase/os/file.h"
#include "libarkbase/panda_gen_options/generated/logger_options.h"

namespace ark::test {

class CacheCollisionTest : public testing::Test {
public:
    CacheCollisionTest()
    {
        execPath_ = ark::os::file::File::GetExecutablePath().Value();
        paocPath_ = execPath_.substr(0U, execPath_.rfind('/')) + "/bin/ark_aot";
        std::string pandaStdLib = execPath_ + "/../pandastdlib/arkstdlib.abc";
        options_.SetBootPandaFiles({pandaStdLib});
        options_.SetLoadRuntimes({"core"});
        options_.SetCompilerQueueType("simple");
        options_.SetRunGcInPlace(true);
        options_.SetGcType("g1-gc");

        ark::Logger::InitializeStdLogging(ark::Logger::Level::DEBUG,
                                          ark::Logger::ComponentMask().set(ark::Logger::Component::CLASS_LINKER));
    }

    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    void SetPandaFile(const ark::arg_list_t abcPaths)
    {
        options_.SetPandaFiles(abcPaths);
    }

    void SetAotFile(const char *aotPath)
    {
        options_.SetAotFile(aotPath);
    }

    std::pair<int, std::string> ExecutePanda(const char *file, const char *entryPoint)
    {
        Runtime::Create(options_);
        auto *runtime = ark::Runtime::GetCurrent();

        std::stringstream strBuf;
        auto old = std::cerr.rdbuf(strBuf.rdbuf());
        auto reset = [&old](auto *cout) { cout->rdbuf(old); };
        auto guard = std::unique_ptr<std::ostream, decltype(reset)>(&std::cerr, reset);

        auto res = runtime->ExecutePandaFile(file, entryPoint, {});
        if (!res) {
            return {1, "error " + std::to_string((int)res.Error())};
        }
        return {res.Value(), strBuf.str()};
    }

    const char *GetPaocFile() const
    {
        return paocPath_.c_str();
    }

    ~CacheCollisionTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(CacheCollisionTest);
    NO_MOVE_SEMANTIC(CacheCollisionTest);

private:
    RuntimeOptions options_;
    std::string execPath_;
    std::string paocPath_;
};

static constexpr auto MAIN_FILE_SOURCE = R"(
.record cache_collision_test {
}

.record pad1 { i32 v }
.record pad2 { i32 v }
.record pad3 { i32 v }
.record pad4 { i32 v }
.record pad5 { i32 v }
.record pad6 { i32 v }
.record pad7 { i32 v }
.record pad8 { i32 v }
.record pad9 { i32 v }
.record pad10 { i32 v }
.record pad11 { i32 v }

.record ext.C <external>

.record ext.T <external>

.function i32 cache_collision_test.main() <static> {
    initobj.short ext.C._ctor_:(ext.C)
    call.acc.short ext.C.get:(ext.C), v0, 0x0
    addi 0x0
    sta v0

    call.short ext.C.ext_call
    add2 v0
    return
}

.function i32 ext.C.ext_call() <external, static>

.function void ext.C._ctor_(ext.C a0) <ctor, external>

.function i32 ext.C.get(ext.C a0) <external>

.function void ext.T._ctor_(ext.T a0) <ctor, external>

.function i32 ext.T.get(ext.T a0) <external>
)";

static constexpr auto EXTERNAL_FILE_SOURCE = R"(
.record ext {
}

.record ext.C {
}

.record ext.T {
    i32 v_
}

.function i32 ext.C.ext_call() <static> {
    initobj.short ext.T._ctor_:(ext.T)
    call.acc.short ext.T.get:(ext.T), v0, 0x0
    return
}

.function void ext.C._ctor_(ext.C a0) <ctor> {
    return.void
}

.function i32 ext.C.get(ext.C a0) {
    ldai 0x7
    return
}

.function void ext.T._ctor_(ext.T a0) <ctor> {
    ldai 0x5
    stobj a0, ext.T.v_
    return.void
}

.function i32 ext.T.get(ext.T a0) {
    ldobj a0, ext.T.v_
    return
}
)";

// This test demonstrates a possible cache collision scenario if class resolution writes
// a resolved class into the caller's panda_file cache instead of the target file cache.
// Two different panda files can legitimately have classes with the same EntityId offset.
// Caches are per-file; mixing them can cause wrong-class returns on subsequent lookups.
TEST_F(CacheCollisionTest, CallerPandaCacheMisuseCausesCollision)
{
    compiler::TmpFile mainFile("test_collision.abc");
    compiler::TmpFile externalFile("external_test.abc");
    compiler::TmpFile aotFile("test_collision.an");

    // Main file
    {
        pandasm::Parser p;
        auto res = p.Parse(MAIN_FILE_SOURCE);
        auto err = p.ShowError();
        ASSERT_EQ(err.err, pandasm::Error::ErrorType::ERR_NONE) << err.message;
        ASSERT_TRUE(pandasm::AsmEmitter::Emit(mainFile.GetFileName(), res.Value()))
            << pandasm::AsmEmitter::GetLastError();
    }

    // External file
    {
        pandasm::Parser p;
        auto res = p.Parse(EXTERNAL_FILE_SOURCE);
        auto err = p.ShowError();
        ASSERT_EQ(err.err, pandasm::Error::ErrorType::ERR_NONE) << err.message;
        ASSERT_TRUE(pandasm::AsmEmitter::Emit(externalFile.GetFileName(), res.Value()))
            << pandasm::AsmEmitter::GetLastError();
    }

    // Compile AOT
    {
        auto res = os::exec::Exec(GetPaocFile(), "--compiler-inline-external-methods-aot=true", "--paoc-panda-files",
                                  mainFile.GetFileName(), "--panda-files", externalFile.GetFileName(), "--paoc-output",
                                  aotFile.GetFileName(), "--gc-type=g1-gc");
        ASSERT_TRUE(res) << "paoc failed with error: " << res.Error().ToString();
        ASSERT_EQ(res.Value(), 0U);
    }

    SetPandaFile({externalFile.GetFileName(), mainFile.GetFileName()});
    SetAotFile(aotFile.GetFileName());

    testing::internal::CaptureStderr();
    auto ret = ExecutePanda(mainFile.GetFileName(), "cache_collision_test::main");
    std::string log = testing::internal::GetCapturedStderr();
    std::string expectedLog = "D/classlinker: Initializing class ext.T";
    ASSERT_NE(log.find(expectedLog), std::string::npos) << "Expected:\n" << expectedLog << "\nLog:\n" << log;
}

}  // namespace ark::test
