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
#include "ani.h"
#include "tools/sampler/aspt_converter.h"

namespace ark::ets::test {

class SampleAniTest : public testing::Test {
public:
    std::string BuildOptionString()
    {
        const char *stdlib = std::getenv("ARK_ETS_STDLIB_PATH");
        if (stdlib == nullptr) {
            return "";
        }

        const char *abcPath = std::getenv("MANAGED_GTEST_ABC_PATH");
        std::string bootFileString = "--ext:boot-panda-files=" + std::string(stdlib);
        if (abcPath != nullptr) {
            bootFileString += ":" + std::string(abcPath);
        }
        return bootFileString;
    }

    void RunMain(ani_options optionsPtr, ani_vm *vm)
    {
        ASSERT_EQ(ANI_CreateVM(&optionsPtr, ANI_VERSION_1, &vm), ANI_OK);
        ani_env *env;
        // Get ANI API
        ASSERT_TRUE(vm->GetEnv(ANI_VERSION_1, &env) == ANI_OK) << "Cannot get ani env";
        uint32_t aniVersion;
        ASSERT_TRUE(env->GetVersion(&aniVersion) == ANI_OK) << "Cannot get ani version";
        ASSERT_TRUE(aniVersion == ANI_VERSION_1) << "Incorrect ani version";

        ani_module module {};
        ASSERT_EQ(env->FindModule("create_vm_sampler", &module), ANI_OK) << "Cannot find module create_vm_sampler";
        ASSERT_NE(module, nullptr);

        ani_function fn {};
        ASSERT_EQ(env->Module_FindFunction(module, "main", ":i", &fn), ANI_OK);
        ASSERT_NE(fn, nullptr);
        ani_int res {};
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        ASSERT_EQ(env->Function_Call_Int(fn, &res), ANI_OK);
        ASSERT_EQ(res, 0);

        ASSERT_TRUE(vm->DestroyVM() == ANI_OK) << "Cannot destroy ANI VM";
    }

    std::vector<ani_option> GetBaseOptions()
    {
        std::vector<ani_option> options;
        bootFileString_ = BuildOptionString();
        ani_option bootFileOption = {bootFileString_.c_str(), nullptr};
        ani_option samplerCreate = {"--ext:sampling-profiler-create", nullptr};
        ani_option samplerEnable = {"--ext:sampling-profiler-startup-run", nullptr};
        ani_option workersType = {"--ext:workers-type=threadpool", nullptr};
        options.push_back(bootFileOption);
        options.push_back(samplerCreate);
        options.push_back(samplerEnable);
        options.push_back(workersType);

        return options;
    }

    void CheckSamplerTrace(const std::string &name)
    {
        tooling::sampler::AsptConverter conv((name + ".aspt").c_str());
        ASSERT_TRUE(conv.RunDumpTracesInCsvMode(name + ".csv"));
        ASSERT_TRUE(conv.DumpModulesToFile(name + "modules.csv"));
    }

private:
    std::string bootFileString_;
};

TEST_F(SampleAniTest, simpleTest)
{
    ani_vm *vm = nullptr;

    auto options = GetBaseOptions();
    ani_option samplerOutput = {"--ext:sampling-profiler-output-file=proftest.aspt", nullptr};
    options.push_back(samplerOutput);

    ani_options optionsPtr = {options.size(), options.data()};
    RunMain(optionsPtr, vm);

    CheckSamplerTrace("proftest");
}

TEST_F(SampleAniTest, noJitTest)
{
    ani_vm *vm = nullptr;

    auto options = GetBaseOptions();
    ani_option samplerOutput = {"--ext:sampling-profiler-output-file=nojitproftest.aspt", nullptr};
    options.push_back(samplerOutput);
    ani_option noJit = {"--ext:compiler-enable-jit=false", nullptr};
    options.push_back(noJit);

    ani_options optionsPtr = {options.size(), options.data()};
    RunMain(optionsPtr, vm);

    CheckSamplerTrace("nojitproftest");
}

}  // namespace ark::ets::test
