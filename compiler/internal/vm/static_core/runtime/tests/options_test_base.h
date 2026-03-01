/**
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#ifndef PANDA_RUNTIME_TESTS_OPTIONS_TEST_BASE_H
#define PANDA_RUNTIME_TESTS_OPTIONS_TEST_BASE_H

#include <gtest/gtest.h>
#include <vector>

#include "runtime/include/runtime_options.h"
#include "libarkbase/utils/pandargs.h"

namespace ark::test {
class RuntimeOptionsTestBase : public testing::Test {
public:
    NO_COPY_SEMANTIC(RuntimeOptionsTestBase);
    NO_MOVE_SEMANTIC(RuntimeOptionsTestBase);

    RuntimeOptionsTestBase() : runtimeOptions_("AAA") {}
    ~RuntimeOptionsTestBase() override = default;

    void SetUp() override
    {
        runtimeOptions_.AddOptions(&paParser_);
        LoadCorrectOptionsList();
    }

    void TearDown() override {}

    ark::PandArgParser *GetParser()
    {
        return &paParser_;
    }

    const std::vector<std::string> &GetCorrectOptionsList() const
    {
        return correctOptionsList_;
    }

    RuntimeOptions *GetRuntimeOptions()
    {
        return &runtimeOptions_;
    }

protected:
    void AddTestingOption(const std::string &opt, const std::string &value)
    {
        correctOptionsList_.push_back("--" + opt + "=" + value);
    }

private:
    virtual void LoadCorrectOptionsList() = 0;

    RuntimeOptions runtimeOptions_;
    ark::PandArgParser paParser_;
    std::vector<std::string> correctOptionsList_;
};
}  // namespace ark::test

#endif  // PANDA_RUNTIME_TESTS_OPTIONS_TEST_BASE_H
