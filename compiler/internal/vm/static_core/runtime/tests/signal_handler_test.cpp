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

#include <gtest/gtest.h>
#include <csignal>
#include <filesystem>
#include <string>
#include <vector>

#include "libarkfile/file_items.h"
#include "libarkfile/value.h"
#include "runtime/include/runtime.h"
#include "runtime/include/thread.h"
#include "libarkbase/macros.h"
#include "libarkbase/utils/utils.h"
#include "libarkbase/os/thread.h"
#include "runtime/include/managed_thread.h"
#include <sys/syscall.h>
#include "assembly-parser.h"
#include "assembly-emitter.h"
#include "runtime/signal_handler.h"

namespace ark::test {

inline std::string Separator()
{
#ifdef _WIN32
    return "\\";
#else
    return "/";
#endif
}

static int g_counter = 0;

void SigProfSamplingProfilerHandler([[maybe_unused]] int signum)
{
    g_counter++;
    std::array<volatile int, 100UL> a {};  // NOLINT(readability-magic-numbers)
    for (int i = 0; i < 100_I; i++) {      // NOLINT(readability-magic-numbers)
        a[i] = i;
    }
}

class Sampler final {
public:
    Sampler() = default;
    ~Sampler() = default;

    bool Start()
    {
        managedThread_ = os::thread::GetCurrentThreadId();
        // Atomic with release order reason: To ensure start store correctly
        isActive_.store(true, std::memory_order_release);

        // All prepairing actions should be done before this thread is started
        samplerThread_ = std::make_unique<std::thread>(&Sampler::SamplerThreadEntry, this);
        return true;
    }

    void Stop()
    {
        if (!samplerThread_->joinable()) {
            LOG(FATAL, PROFILER) << "Sampling profiler thread unexpectedly disappeared";
            UNREACHABLE();
        }

        // Atomic with release order reason: To ensure stop store correctly
        isActive_.store(false, std::memory_order_release);

        samplerThread_->join();
    }

private:
    void SamplerThreadEntry()
    {
        struct sigaction action {};
        action.sa_handler = SigProfSamplingProfilerHandler;
        action.sa_flags = 0;
        // Clear signal set
        sigemptyset(&action.sa_mask);
        // Ignore incoming sigprof if handler isn't completed
        sigaddset(&action.sa_mask, SIGPROF);

        struct sigaction oldAction {};

        if (sigaction(SIGPROF, &action, &oldAction) == -1) {
            LOG(FATAL, PROFILER) << "Sigaction failed, can't start profiling";
            UNREACHABLE();
        }

        auto pid = getpid();
        // Atomic with acquire order reason: To ensure start/stop load correctly
        while (isActive_.load(std::memory_order_acquire)) {
            {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
                if (syscall(SYS_tgkill, pid, managedThread_, SIGPROF) != 0) {
                    LOG(ERROR, PROFILER) << "Can't send signal to thread";
                }
            }
            os::thread::NativeSleepUS(sampleInterval_);
        }
    }

    std::unique_ptr<std::thread> samplerThread_ {nullptr};

    std::atomic<bool> isActive_ {false};

    std::chrono::microseconds sampleInterval_ {200};  // NOLINT(readability-magic-numbers)

    os::thread::ThreadId managedThread_ = 0;

    NO_COPY_SEMANTIC(Sampler);
    NO_MOVE_SEMANTIC(Sampler);
};

class SignalHandlerTest : public testing::Test {
public:
    SignalHandlerTest()
    {
        RuntimeOptions options;
        options.SetLoadRuntimes({"core"});
        auto execPath = ark::os::file::File::GetExecutablePath();
        std::string pandaStdLib =
            execPath.Value() + Separator() + ".." + Separator() + "pandastdlib" + Separator() + "arkstdlib.abc";
        options.SetBootPandaFiles({pandaStdLib});
        options.SetGcType("epsilon");
        options.SetCompilerEnableJit(false);
        options.SetCompilerEnableOsr(false);
        Runtime::Create(options);

        thread_ = MTManagedThread::GetCurrent();
    }

    ~SignalHandlerTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(SignalHandlerTest);
    NO_MOVE_SEMANTIC(SignalHandlerTest);

private:
    MTManagedThread *thread_;
};

static auto g_callTestSource = R"(
    .record Math <external>
    .function f64 Math.pow (f64 a0, f64 a1) <external>
    .function f64 Math.absF64 (f64 a0) <external>

    .function void main_short() {
        movi v0, 100000           #loop_counter
        call.short main, v0
        return.void
    }
    .function void main_long() {
        movi v0, 10000000          #loop_counter
        call.short main, v0
        return.void
    }
    .function i32 main(i32 a0) {
    movi v5, 0
    loop:
        lda v5
        jeq a0, loop_exit
        fmovi.64 v0, 2.5
        fmovi.64 v1, 24.7052942201
        fmovi.64 v2, 1e-6
        fmovi.64 v3, 3.5
        call.short Math.pow, v0, v3
        fsub2.64 v1
        sta.64 v1
        call.short Math.absF64, v1, v1
        fcmpl.64 v2
        jgez exit_failure
        inci v5, 1
        jmp loop
    loop_exit:
        ldai 0
        return
    exit_failure:
        ldai 1
        return
    }
    )";

TEST_F(SignalHandlerTest, CallTest)
{
    auto source = g_callTestSource;

    pandasm::Parser parser;
    auto res = parser.Parse(source);
    ASSERT_TRUE(res);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());
    Runtime::GetCurrent()->GetClassLinker()->AddPandaFile(std::move(pf));

    Sampler sp;
    ASSERT_EQ(sp.Start(), true);
#ifdef PANDA_TARGET_ARM32
    Runtime::GetCurrent()->Execute("_GLOBAL::main_short", {});
#else
    Runtime::GetCurrent()->Execute("_GLOBAL::main_long", {});
#endif

    sp.Stop();
    ASSERT_NE(g_counter, 0);
}

class AotEscapeSignalHandlerTest : public testing::Test {
public:
    AotEscapeSignalHandlerTest()
    {
        execPath_ = ark::os::file::File::GetExecutablePath().Value();
        std::string pandaStdLib =
            execPath_ + Separator() + ".." + Separator() + "pandastdlib" + Separator() + "arkstdlib.abc";
        options_.SetLoadRuntimes({"core"});
        options_.SetBootPandaFiles({pandaStdLib});
        options_.SetGcType("g1-gc");
        options_.SetCompilerEnableJit(false);
        options_.SetCompilerEnableOsr(false);
        options_.SetArkAot(true);
    }

    ~AotEscapeSignalHandlerTest() override = default;

    NO_COPY_SEMANTIC(AotEscapeSignalHandlerTest);
    NO_MOVE_SEMANTIC(AotEscapeSignalHandlerTest);

    bool CompileAbc(std::string &abcPath)
    {
        std::string srcPath = "AotEscapeSignalHandlerTestSrc.ets";
        std::ofstream file(srcPath);
        if (file.is_open()) {
            file << TEST_SOURCE;
            file.close();
        }
        std::string es2panda = execPath_ + "/../bin/es2panda";
        std::string runArkAot = es2panda + " " + srcPath + " --output " + abcPath;
        // NOLINTNEXTLINE(cert-env33-c)
        int ret = std::system(runArkAot.c_str());
        return ret == 0;
    }

    std::string FindEtsStdlibRecursive(const std::filesystem::path &rootDir)
    {
        auto iter = std::filesystem::recursive_directory_iterator(
            rootDir, std::filesystem::directory_options::skip_permission_denied);
        for (const auto &entry : iter) {
            if (entry.is_regular_file() && entry.path().filename() == "etsstdlib.abc") {
                return std::filesystem::absolute(entry.path()).string();
            }
        }
        return "";
    }

    bool CompileInvalidAot(const std::string &abcPath, const std::string &anPath)
    {
        std::string pandaStdLib = FindEtsStdlibRecursive(execPath_ + Separator() + ".." + Separator());
        if (pandaStdLib.length() == 0) {
            return false;
        }
        const char *stdlib = pandaStdLib.c_str();
        std::string aotArgs = "--load-runtimes=ets";
        aotArgs += " --boot-panda-files=" + std::string(stdlib);
        aotArgs += " --paoc-panda-files=" + abcPath;
        aotArgs += " --paoc-output=" + anPath;
        std::string arkAot = execPath_ + "/../bin/ark_aot";
        std::string runArkAot = arkAot + " " + aotArgs;
        // NOLINTNEXTLINE(cert-env33-c)
        int ret = std::system(runArkAot.c_str());
        return ret == 0;
    }

    RuntimeOptions &GetOptions()
    {
        return options_;
    }

    void SetPandaFile(std::string &abcPath)
    {
        options_.SetPandaFiles({abcPath});
    }

    void SetAotFile(std::string &aotPath)
    {
        options_.SetAotFile(aotPath);
    }

private:
    std::string execPath_;
    RuntimeOptions options_;
    // NOLINTNEXTLINE
    static constexpr char TEST_SOURCE[] =
        "\
    class B {\n\
    }\n\
    function test_func(): B {\n\
        let a = new B();\n\
        return a;\n\
    }\n\
    function main() {\n\
        test_func();\n\
    }\n";
};

TEST_F(AotEscapeSignalHandlerTest, AotEscapeTestSetDirectly)
{
    std::string abcPath = "AotEscapeTest1Tmp.abc";
    std::string anPath = "AotEscapeTest1Tmp.an";
    bool hasAbc = CompileAbc(abcPath);
    bool hasAn = CompileInvalidAot(abcPath, anPath);

    if (hasAbc) {
        SetPandaFile(abcPath);
    }
    if (hasAn) {
        SetAotFile(anPath);
    }
    AotEscapeSignalHandler::SetEscaped(false);
    std::string aotEscapePath = "AotEscapeTestSetDirectlyPath.txt";
    AotEscapeSignalHandler::SetEscapedFlagFilePath(aotEscapePath);
    Runtime::Create(GetOptions());
    auto hasAot = Runtime::GetCurrent()->GetClassLinker()->GetAotManager()->HasAotFiles();
    ASSERT_EQ(hasAot, hasAn);
    Runtime::Destroy();
    // Then set escaped, resart vm, will not load an file
    AotEscapeSignalHandler::SetEscaped(true);
    Runtime::Create(GetOptions());
    hasAot = Runtime::GetCurrent()->GetClassLinker()->GetAotManager()->HasAotFiles();
    ASSERT_EQ(hasAot, false);
    Runtime::Destroy();
}

#if defined(PANDA_TARGET_AMD64)
TEST_F(AotEscapeSignalHandlerTest, AotEscapeTestCallAction)
{
    std::string abcPath = "AotEscapeTest2Tmp.abc";
    std::string anPath = "AotEscapeTest2Tmp.an";
    bool hasAbc = CompileAbc(abcPath);
    bool hasAn = CompileInvalidAot(abcPath, anPath);

    if (hasAbc) {
        SetPandaFile(abcPath);
    }
    if (hasAn) {
        SetAotFile(anPath);
    }
    AotEscapeSignalHandler::SetEscaped(false);
    Runtime::Create(GetOptions());
    auto hasAot = Runtime::GetCurrent()->GetClassLinker()->GetAotManager()->HasAotFiles();
    ASSERT_EQ(hasAot, hasAn);
    std::string aotEscapePath = "AotEscapeTest2EscapeFlagPath.txt";
    AotEscapeSignalHandler::SetEscapedFlagFilePath(aotEscapePath);
    ucontext_t uc;
    mcontext_t ucMcontext = {};
    uc.uc_mcontext = ucMcontext;
    AotEscapeSignalHandler::HandleAction(SIGSEGV, nullptr, &uc);
    AotEscapeSignalHandler::HandleAction(SIGABRT, nullptr, &uc);
    AotEscapeSignalHandler::HandleAction(SIGBUS, nullptr, &uc);
    Runtime::Destroy();

    Runtime::Create(GetOptions());
    hasAot = Runtime::GetCurrent()->GetClassLinker()->GetAotManager()->HasAotFiles();
    ASSERT_EQ(hasAot, false);
    Runtime::Destroy();
}
#endif

}  // namespace ark::test
