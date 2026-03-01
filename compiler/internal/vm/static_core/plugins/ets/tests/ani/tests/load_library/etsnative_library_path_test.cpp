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

#include "ets_vm.h"
#include "include/mem/panda_string.h"

#include <cstdlib>
#include <gtest/gtest-param-test.h>
#include <gtest/gtest.h>
#include <ios>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

namespace ark::ets::test {

struct TestParams {
    bool expected;
    std::string libraryPath;
    std::string libraryFile;
    bool permissionCheck;
};

inline std::ostream &operator<<(std::ostream &out, const TestParams &val)
{
    std::ios_base::fmtflags f = out.flags();
    out << std::boolalpha                                          //-
        << "libraryPath is '" << val.libraryPath << "'"            //-
        << ", libraryFile is '" << val.libraryFile << "'"          //-
        << ", permissionCheck is '" << val.permissionCheck << "'"  //-
        << ", expected is " << std::boolalpha << val.expected;
    out.setf(f);
    return out;
}

class EtsNativeLibraryPathTest : public testing::TestWithParam<TestParams> {
protected:
    void SetUp() override
    {
        std::string bootPandaFilesOption = std::string("--ext:boot-panda-files=") + std::getenv("ARK_ETS_STDLIB_PATH");
        std::string nativeLibraryPathOption = "--ext:native-library-path=" + GetParam().libraryPath;
        std::vector<ani_option> options = {
            {bootPandaFilesOption.c_str(), nullptr},
            {nativeLibraryPathOption.c_str(), nullptr},
        };
        ani_options opts {options.size(), options.data()};

        ASSERT_EQ(ANI_CreateVM(&opts, ANI_VERSION_1, &vm_), ANI_OK) << "Cannot create ETS VM";
        ASSERT_EQ(vm_->GetEnv(ANI_VERSION_1, &env_), ANI_OK) << "Cannot get ani env";
    }

    void TearDown() override
    {
        ASSERT_EQ(vm_->DestroyVM(), ANI_OK) << "Cannot destroy ETS VM";
    }

    inline ani_env *GetEnv()
    {
        return env_;
    }

    inline ani_vm *GetVm()
    {
        return vm_;
    }

private:
    ani_env *env_ {nullptr};
    ani_vm *vm_ {nullptr};
};

TEST_P(EtsNativeLibraryPathTest, EtsNativeTestLibraryPath)
{
    auto pandaVm = EtsCoroutine::GetCurrent()->GetPandaVM();
    ASSERT_EQ(pandaVm, GetVm());
    ASSERT_EQ(GetParam().expected, pandaVm->LoadNativeLibrary(GetEnv(), ConvertToString(GetParam().libraryFile),
                                                              GetParam().permissionCheck, ""))
        << GetParam();
}

constexpr auto TEST_LIBRARY_FILE = "libnativelib.so";

static std::string GetLibraryPath()
{
    const auto path = std::getenv("ARK_ETS_LIBRARY_PATH");
    return {path != nullptr ? path : ""};
}

INSTANTIATE_TEST_SUITE_P(  //-
    LibraryPathTests, EtsNativeLibraryPathTest,
    ::testing::Values(
        TestParams {
            true,
            GetLibraryPath(),
            TEST_LIBRARY_FILE,
            false,
        },
        TestParams {
            true,
            []() {
                std::stringstream ss {};
                ss << GetLibraryPath() << "/not_exists_dir"
                   << ":"  //-
                   << GetLibraryPath();
                return ss.str();
            }(),
            TEST_LIBRARY_FILE,
            false,
        },
        TestParams {
            false,
            {},
            TEST_LIBRARY_FILE,
            false,
        },
        TestParams {
            true,
            GetLibraryPath(),
            TEST_LIBRARY_FILE,
            true,
        }));

}  // namespace ark::ets::test
