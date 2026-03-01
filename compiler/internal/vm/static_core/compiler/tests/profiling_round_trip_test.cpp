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

#include <algorithm>
#include <filesystem>
#include <utility>
#include "include/class.h"
#include "jit/profiling_data.h"
#include "gmock/gmock.h"
#include "panda_runner.h"
#include "libarkbase/utils/string_helpers.h"
#include "runtime/jit/profiling_saver.h"
#include "runtime/jit/profiling_loader.h"

namespace ark::test {
// NOLINTBEGIN(readability-magic-numbers)

namespace {

constexpr auto SOURCE = R"(
.record E {}
.record A {}
.record B <extends=A> {}
.record C <extends=A> {}

.function i32 A.bar(A a0, i32 a1) {
	ldai 0x10
	jge a1, jump_label_0
	lda a1
	return
jump_label_0:
	newobj v0, E
    throw v0
}

.function i32 A.foo(A a0) {
	ldai 0x1
	return
}

.function i32 B.foo(B a0) {
	ldai 0x2
	return
}

.function i32 C.foo(C a0) {
    ldai 0x3
    return
}

.function void test() <static> {
    movi v0, 0x20
	movi v1, 0x0
	newobj v4, A
    newobj v5, B
    newobj v6, C
jump_label_4:
	lda v1
	jge v0, jump_label_3
	call.virt A.foo, v4
	call.virt B.foo, v5
	call.virt C.foo, v6
try_begin:
	call.short A.bar, v4, v1
try_end:
	jmp handler_begin_label_0_0
handler_begin_label_0_0:
	inci v1, 0x1
	jmp jump_label_4
jump_label_3:
	return.void

.catchall try_begin, try_end, handler_begin_label_0_0
}

.function void main() <static> {
    call.short test
    return.void
}
)";

std::string CreateTmpFileName()
{
    uint32_t tid = os::thread::GetCurrentThreadId();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    std::string pgoFileName = helpers::string::Format("tmpfile2_%04x.ap", tid);
    // CC-OFFNXT(G.NAM.03-CPP) project code style
    static const std::string LOCATION = "/tmp";
    return LOCATION + "/" + pgoFileName;
}

Class *FindClass(const std::string &name)
{
    PandaString descriptor;
    auto *thread = MTManagedThread::GetCurrent();
    thread->ManagedCodeBegin();
    auto cls = Runtime::GetCurrent()
                   ->GetClassLinker()
                   ->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY)
                   ->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8(name.data()), &descriptor));
    thread->ManagedCodeEnd();
    return cls;
}

std::map<std::string, ark::Class *> CreateClassMap(const std::vector<std::string> &classNames)
{
    std::map<std::string, ark::Class *> classMap;
    for (auto &name : classNames) {
        classMap[name] = FindClass(name);
    }
    return classMap;
}

auto ToTuple(const BranchData &data)
{
    return std::make_tuple(data.GetPc(), data.GetTakenCounter(), data.GetNotTakenCounter());
}

auto ToTuple(const ThrowData &data)
{
    return std::make_tuple(data.GetPc(), data.GetTakenCounter());
}

auto ToTuple(const CallSiteInlineCache &data)
{
    auto classIdsVec = data.GetClassesCopy();
    std::set<ark::Class *> classIds(classIdsVec.begin(), classIdsVec.end());
    return std::make_tuple(data.GetBytecodePc(), classIds);
}

bool Equals(const BranchData &lhs, const BranchData &rhs)
{
    return ToTuple(lhs) == ToTuple(rhs);
}

bool Equals(const ThrowData &lhs, const ThrowData &rhs)
{
    return ToTuple(lhs) == ToTuple(rhs);
}

bool Equals(const CallSiteInlineCache &lhs, const CallSiteInlineCache &rhs)
{
    return ToTuple(lhs) == ToTuple(rhs);
}

template <typename T>
bool Equals(const Span<T> &lhs, const Span<T> &rhs)
{
    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(),
                      [](const T &l, const T &r) { return Equals(l, r); });
}

bool Equals(const ProfilingData &lhs, const ProfilingData &rhs)
{
    return Equals(lhs.GetBranchData(), rhs.GetBranchData()) && Equals(lhs.GetThrowData(), rhs.GetThrowData()) &&
           Equals(lhs.GetInlineCaches(), rhs.GetInlineCaches());
}

class ProfilingRoundTripTest : public ::testing::Test {
public:
    PandaRunner &CreateRunner()
    {
        runner_.GetRuntimeOptions().SetIncrementalProfilesaverEnabled(false);
        runner_.GetRuntimeOptions().SetShouldLoadBootPandaFiles(false);
        runner_.GetRuntimeOptions().SetShouldInitializeIntrinsics(false);
        runner_.GetRuntimeOptions().SetCompilerProfilingThreshold(0U);
        return runner_;
    }

    void CollectProfile()
    {
        auto runtime = runner_.CreateRuntime();
        runner_.Run(runtime, SOURCE, std::vector<std::string> {});
    }

    void SaveProfile()
    {
        auto *runtime = Runtime::GetCurrent();
        if (!runtime->GetClassLinker()->GetAotManager()->HasProfiledMethods()) {
            return;
        }
        ProfilingSaver profileSaver;
        classCtxStr_ = runtime->GetClassLinker()->GetAotManager()->GetBootClassContext() + ":" +
                       runtime->GetClassLinker()->GetAotManager()->GetAppClassContext();
        auto &writtenMethods = runtime->GetClassLinker()->GetAotManager()->GetProfiledMethods();
        auto writtenMethodsFinal = runtime->GetClassLinker()->GetAotManager()->GetProfiledMethodsFinal();
        auto profiledPandaFiles = runtime->GetClassLinker()->GetAotManager()->GetProfiledPandaFiles();
        profileSaver.SaveProfile(PandaString(pgoFilePath_), PandaString(classCtxStr_), writtenMethods,
                                 writtenMethodsFinal, profiledPandaFiles);
    }

    void LoadProfileFail(ProfilingLoader &profilingLoader)
    {
        auto profileCtxOrError = profilingLoader.LoadProfile(PandaString(pgoFilePath_));
        ASSERT_FALSE(profileCtxOrError.HasValue());
    }

    void LoadProfile(ProfilingLoader &profilingLoader)
    {
        auto profileCtxOrError = profilingLoader.LoadProfile(PandaString(pgoFilePath_));
        if (!profileCtxOrError) {
            std::cerr << profileCtxOrError.Error();
        }
        ASSERT_TRUE(profileCtxOrError.HasValue());
        ASSERT_EQ(*profileCtxOrError, PandaString(classCtxStr_));
    }

    ark::Class *ClassResolver(uint32_t classIdx, [[maybe_unused]] size_t fileIdx)
    {
        for (auto &entry : classMap_) {
            if (classIdx == entry.second->GetFileId().GetOffset()) {
                return entry.second;
            }
        }
        return nullptr;
    }

    void CheckProfile(ProfilingLoader &profilingLoader)
    {
        using ::testing::_;
        using ::testing::ElementsAre;
        using ::testing::Pair;

        auto *runtime = Runtime::GetCurrent();

        classMap_ = CreateClassMap({"_GLOBAL", "A", "B", "C"});
        ark::Class *globalCls = classMap_["_GLOBAL"];
        ASSERT_NE(globalCls, nullptr);
        ark::Method *testMtd = globalCls->GetClassMethod(utf::CStringAsMutf8("test"));
        ASSERT_NE(testMtd, nullptr);
        ark::Class *aCls = classMap_["A"];
        ASSERT_NE(aCls, nullptr);
        ark::Method *barMtd = aCls->GetClassMethod(utf::CStringAsMutf8("bar"));
        ASSERT_NE(barMtd, nullptr);

        EXPECT_THAT(profilingLoader.GetAotProfilingData().GetPandaFileMap(), ElementsAre(Pair("", _)));
        auto &methodsProfile = profilingLoader.GetAotProfilingData().GetAllMethods().begin()->second;
        EXPECT_EQ(methodsProfile.size(), 2U);

        for (auto &[methodId, methodProfile] : methodsProfile) {
            EXPECT_EQ(methodId, methodProfile.GetMethodIdx());
            auto *cls = ClassResolver(methodProfile.GetClassIdx(), 0);
            ASSERT_NE(cls, nullptr);
            auto profilingData = profilingLoader.CreateProfilingData(
                methodProfile, runtime->GetInternalAllocator(),
                [this](uint32_t classIdx, size_t fileIdx) { return ClassResolver(classIdx, fileIdx); });
            ASSERT_NE(profilingData, nullptr);

            if (cls == globalCls) {
                // this is test
                EXPECT_EQ(methodId, testMtd->GetFileId().GetOffset());
                EXPECT_TRUE(Equals(*testMtd->GetProfilingData(), *profilingData));
            } else if (cls == aCls) {
                // this is A.bar
                EXPECT_EQ(methodId, barMtd->GetFileId().GetOffset());
                EXPECT_TRUE(Equals(*barMtd->GetProfilingData(), *profilingData));
            }

            runtime->GetInternalAllocator()->Free(profilingData);
        }
    }

    void InitPGOFilePath()
    {
        pgoFilePath_ = CreateTmpFileName();
        if (std::filesystem::exists(pgoFilePath_)) {
            std::filesystem::remove(pgoFilePath_);
        }
    }

private:
    PandaRunner runner_;
    std::map<std::string, ark::Class *> classMap_;
    std::string pgoFilePath_;
    std::string classCtxStr_;
};

}  // namespace

TEST_F(ProfilingRoundTripTest, ProfilingFileSaveCpp)
{
    InitPGOFilePath();
    CreateRunner().GetRuntimeOptions().SetInterpreterType("cpp");
    CollectProfile();
    SaveProfile();

    {
        ProfilingLoader profilingLoader;
        LoadProfile(profilingLoader);
        CheckProfile(profilingLoader);
    }

    Runtime::Destroy();
}

TEST_F(ProfilingRoundTripTest, ProfilingFileSave)
{
    InitPGOFilePath();
    CreateRunner();
    CollectProfile();
    SaveProfile();

    {
        ProfilingLoader profilingLoader;
        LoadProfile(profilingLoader);
        CheckProfile(profilingLoader);
    }

    Runtime::Destroy();
}

TEST_F(ProfilingRoundTripTest, ProfilingFileSaveCppWithProfiler)
{
    InitPGOFilePath();
    auto &runner = CreateRunner();
    runner.GetRuntimeOptions().SetInterpreterType("cpp");
    runner.GetRuntimeOptions().SetProfilerEnabled(true);
    runner.GetRuntimeOptions().SetCompilerEnableJit(false);
    CollectProfile();
    SaveProfile();

    {
        ProfilingLoader profilingLoader;
        LoadProfile(profilingLoader);
        CheckProfile(profilingLoader);
    }

    Runtime::Destroy();
}

TEST_F(ProfilingRoundTripTest, ProfilingFileSaveWithProfiler)
{
    InitPGOFilePath();
    auto &runner = CreateRunner();
    runner.GetRuntimeOptions().SetProfilerEnabled(true);
    runner.GetRuntimeOptions().SetCompilerEnableJit(false);
    CollectProfile();
    SaveProfile();

    {
        ProfilingLoader profilingLoader;
        LoadProfile(profilingLoader);
        CheckProfile(profilingLoader);
    }

    Runtime::Destroy();
}

TEST_F(ProfilingRoundTripTest, ProfilingFileSaveCppWithJit)
{
    InitPGOFilePath();
    auto &runner = CreateRunner();
    runner.GetRuntimeOptions().SetInterpreterType("cpp");
    runner.GetRuntimeOptions().SetProfilerEnabled(false);
    runner.GetRuntimeOptions().SetCompilerEnableJit(true);
    CollectProfile();
    SaveProfile();

    {
        ProfilingLoader profilingLoader;
        LoadProfile(profilingLoader);
        CheckProfile(profilingLoader);
    }

    Runtime::Destroy();
}

TEST_F(ProfilingRoundTripTest, ProfilingFileSaveWithJit)
{
    InitPGOFilePath();
    auto &runner = CreateRunner();
    runner.GetRuntimeOptions().SetProfilerEnabled(false);
    runner.GetRuntimeOptions().SetCompilerEnableJit(true);
    CollectProfile();
    SaveProfile();

    {
        ProfilingLoader profilingLoader;
        LoadProfile(profilingLoader);
        CheckProfile(profilingLoader);
    }

    Runtime::Destroy();
}

TEST_F(ProfilingRoundTripTest, ProfilingFileSaveCppWithoutJitAndProfiler)
{
    InitPGOFilePath();
    auto &runner = CreateRunner();
    runner.GetRuntimeOptions().SetInterpreterType("cpp");
    runner.GetRuntimeOptions().SetProfilerEnabled(false);
    runner.GetRuntimeOptions().SetCompilerEnableJit(false);
    CollectProfile();
    SaveProfile();

    {
        ProfilingLoader profilingLoader;
        LoadProfileFail(profilingLoader);
    }

    Runtime::Destroy();
}

TEST_F(ProfilingRoundTripTest, ProfilingFileSaveWithoutJitAndProfiler)
{
    InitPGOFilePath();
    auto &runner = CreateRunner();
    runner.GetRuntimeOptions().SetProfilerEnabled(false);
    runner.GetRuntimeOptions().SetCompilerEnableJit(false);
    CollectProfile();
    SaveProfile();

    {
        ProfilingLoader profilingLoader;
        LoadProfileFail(profilingLoader);
    }

    Runtime::Destroy();
}

// NOLINTEND(readability-magic-numbers)
}  // namespace ark::test
