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

#include <gtest/gtest.h>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <string>
#include "include/mem/panda_containers.h"
#include "include/method.h"
#include "include/runtime.h"
#include "jit/libprofile/aot_profiling_data.h"
#include "jit/profiling_loader.h"
#include "jit/profiling_saver.h"
#include "libarkbase/macros.h"
#include "unit_test.h"
#include "panda_runner.h"
#include "runtime/jit/profiling_data.h"
namespace ark::test {

static constexpr auto SOURCE = R"(
.function void main() <static> {
    movi v0, 0x2
    movi v1, 0x0
    jump_label_1: lda v1
    jge v0, jump_label_0
    call.short foo
    inci v1, 0x1
    jmp jump_label_1
    jump_label_0: return.void
}

.function i32 foo() <static> {
    movi v0, 0x64
    movi v1, 0x0
    mov v2, v1
    jump_label_3: lda v2
    jge v0, jump_label_0
    lda v2
    modi 0x3
    jnez jump_label_1
    lda v1
    addi 0x2
    sta v3
    mov v1, v3
    jmp jump_label_2
    jump_label_1: lda v1
    addi 0x3
    sta v3
    mov v1, v3
    jump_label_2: inci v2, 0x1
    jmp jump_label_3
    jump_label_0: lda v1
    return
}
)";

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

class ProfilingMergerTest : public ::testing::Test {
public:
    void CollectProfile()
    {
        runner_.GetRuntimeOptions().SetIncrementalProfilesaverEnabled(false);
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

    void LoadProfile(ProfilingLoader &profilingLoader)
    {
        auto profileCtxOrError = profilingLoader.LoadProfile(PandaString(pgoFilePath_));
        if (!profileCtxOrError) {
            std::cerr << profileCtxOrError.Error();
        }
        ASSERT_TRUE(profileCtxOrError.HasValue());
        ASSERT_EQ(*profileCtxOrError, PandaString(classCtxStr_));
    }

    ark::Class *ClassResolver([[maybe_unused]] uint32_t classIdx, [[maybe_unused]] size_t fileIdx)
    {
        return FindClass("_GLOBAL");
    }

    void InitPGOFilePath(const std::string &path)
    {
        pgoFilePath_ = path;
        if (std::filesystem::exists(path)) {
            std::filesystem::remove(path);
        }
    }

    void SetCompilerProfilingThreshold(uint32_t value)
    {
        runner_.GetRuntimeOptions().SetCompilerProfilingThreshold(value);
    }

    void DestoryRuntime()
    {
        Runtime::Destroy();
    }

private:
    PandaRunner runner_;
    std::map<std::string, ark::Class *> classMap_;
    std::string pgoFilePath_;
    std::string classCtxStr_;
};

TEST_F(ProfilingMergerTest, ProfileAddAndOverwriteTest)
{
    InitPGOFilePath("/tmp/profileaddandoverwrite.ap");

    // first run
    const uint32_t highThreshold = 10U;
    SetCompilerProfilingThreshold(highThreshold);
    CollectProfile();
    SaveProfile();
    Runtime::Destroy();

    // second run
    SetCompilerProfilingThreshold(0U);
    CollectProfile();
    {
        ProfilingLoader profilingLoader0;
        LoadProfile(profilingLoader0);
        auto &methodsProfile0 = profilingLoader0.GetAotProfilingData().GetAllMethods().begin()->second;
        EXPECT_EQ(methodsProfile0.size(), 1U);
        SaveProfile();
        ProfilingLoader profilingLoader1;
        LoadProfile(profilingLoader1);
        auto &methodsProfile1 = profilingLoader1.GetAotProfilingData().GetAllMethods().begin()->second;
        EXPECT_EQ(methodsProfile1.size(), 2U);

        // add profile data of main (new profiled)
        // overwrite profile data of foo
        auto globalCls = FindClass("_GLOBAL");
        auto mainMtd = globalCls->GetClassMethod(utf::CStringAsMutf8("main"));
        auto fooMtd = globalCls->GetClassMethod(utf::CStringAsMutf8("foo"));
        EXPECT_FALSE(mainMtd->GetProfilingData()->IsUpdateSinceLastSave());
        EXPECT_FALSE(fooMtd->GetProfilingData()->IsUpdateSinceLastSave());

        for (auto &[methodId, methodProfile] : methodsProfile1) {
            EXPECT_EQ(methodId, methodProfile.GetMethodIdx());
            if (methodId == mainMtd->GetFileId().GetOffset()) {
                // check data of main, same as second profiled
                auto profilingData = profilingLoader1.CreateProfilingData(
                    methodProfile, Runtime::GetCurrent()->GetInternalAllocator(),
                    [this](uint32_t classIdx, size_t fileIdx) { return ClassResolver(classIdx, fileIdx); });
                EXPECT_TRUE(Equals(*mainMtd->GetProfilingData(), *profilingData));
                Runtime::GetCurrent()->GetInternalAllocator()->Free(profilingData);
            } else if (methodId == fooMtd->GetFileId().GetOffset()) {
                // check data of foo, same as second profiled
                auto profilingData = profilingLoader1.CreateProfilingData(
                    methodProfile, Runtime::GetCurrent()->GetInternalAllocator(),
                    [this](uint32_t classIdx, size_t fileIdx) { return ClassResolver(classIdx, fileIdx); });
                auto methodProfile0 = methodsProfile0.begin()->second;
                EXPECT_TRUE(Equals(*fooMtd->GetProfilingData(), *profilingData));
                Runtime::GetCurrent()->GetInternalAllocator()->Free(profilingData);
            }
        }
    }
    Runtime::Destroy();
}

TEST_F(ProfilingMergerTest, ProfileKeepAndOverwriteTest)
{
    InitPGOFilePath("/tmp/profilekeepandoverwrite.ap");

    // first run
    SetCompilerProfilingThreshold(0U);
    CollectProfile();
    SaveProfile();
    Runtime::Destroy();

    // second run
    const uint32_t highThreshold = 10U;
    SetCompilerProfilingThreshold(highThreshold);
    CollectProfile();
    {
        ProfilingLoader profilingLoader0;
        LoadProfile(profilingLoader0);
        auto &methodsProfile0 = profilingLoader0.GetAotProfilingData().GetAllMethods().begin()->second;
        EXPECT_EQ(methodsProfile0.size(), 2U);
        SaveProfile();
        ProfilingLoader profilingLoader1;
        LoadProfile(profilingLoader1);
        auto &methodsProfile1 = profilingLoader1.GetAotProfilingData().GetAllMethods().begin()->second;
        EXPECT_EQ(methodsProfile1.size(), 2U);

        // keep profile data of main (old profield)
        // overwrite profile data of foo
        auto globalCls = FindClass("_GLOBAL");
        auto mainMtd = globalCls->GetClassMethod(utf::CStringAsMutf8("main"));
        auto fooMtd = globalCls->GetClassMethod(utf::CStringAsMutf8("foo"));
        EXPECT_FALSE(fooMtd->GetProfilingData()->IsUpdateSinceLastSave());

        for (auto &[methodId, methodProfile] : methodsProfile1) {
            EXPECT_EQ(methodId, methodProfile.GetMethodIdx());
            if (methodId == mainMtd->GetFileId().GetOffset()) {
                // check data of main, same as first profiled
                auto profilingData = profilingLoader1.CreateProfilingData(
                    methodProfile, Runtime::GetCurrent()->GetInternalAllocator(),
                    [this](uint32_t classIdx, size_t fileIdx) { return ClassResolver(classIdx, fileIdx); });
                auto methodProfile0 = methodsProfile0.begin()->first == mainMtd->GetFileId().GetOffset()
                                          ? methodsProfile0.begin()->second
                                          : methodsProfile0.rbegin()->second;
                auto profilingData0 = profilingLoader0.CreateProfilingData(
                    methodProfile0, Runtime::GetCurrent()->GetInternalAllocator(),
                    [this](uint32_t classIdx, size_t fileIdx) { return ClassResolver(classIdx, fileIdx); });
                EXPECT_TRUE(Equals(*profilingData0, *profilingData));
                Runtime::GetCurrent()->GetInternalAllocator()->Free(profilingData);
                Runtime::GetCurrent()->GetInternalAllocator()->Free(profilingData0);
            } else if (methodId == fooMtd->GetFileId().GetOffset()) {
                // check data of foo, same as second profiled
                auto profilingData = profilingLoader1.CreateProfilingData(
                    methodProfile, Runtime::GetCurrent()->GetInternalAllocator(),
                    [this](uint32_t classIdx, size_t fileIdx) { return ClassResolver(classIdx, fileIdx); });
                EXPECT_TRUE(Equals(*fooMtd->GetProfilingData(), *profilingData));
                Runtime::GetCurrent()->GetInternalAllocator()->Free(profilingData);
            }
        }
    }
    Runtime::Destroy();
}
}  // namespace ark::test
