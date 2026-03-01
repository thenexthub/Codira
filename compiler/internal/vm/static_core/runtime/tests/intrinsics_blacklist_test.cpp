/**
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

#include <gtest/gtest.h>
#include "assembly-parser.h"
#include "assembly-emitter.h"
#include "runtime/include/runtime.h"

namespace ark::test {

inline std::string Separator()
{
#ifdef _WIN32
    return "\\";
#else
    return "/";
#endif
}

class IntrinsicsBlacklistTest : public testing::Test {
public:
    IntrinsicsBlacklistTest() = default;
    ~IntrinsicsBlacklistTest() override
    {
        if (runtimeCreated_) {
            Runtime::Destroy();
        }
    }

    NO_COPY_SEMANTIC(IntrinsicsBlacklistTest);
    NO_MOVE_SEMANTIC(IntrinsicsBlacklistTest);

    void CreateRuntime(RuntimeOptions &options)
    {
        ASSERT_FALSE(runtimeCreated_);
        Runtime::Create(options);
        runtimeCreated_ = true;
    }

private:
    bool runtimeCreated_ {false};
};

TEST_F(IntrinsicsBlacklistTest, DisableIntrinsic)
{
    auto source = R"(
.record Math <external>
.function i32 Math.absI32(i32 a0) <external>

.function i32 main() {
    movi v0, 42
    call.short Math.absI32, v0, v0
    return
}
)";
    RuntimeOptions options;
    options.SetLoadRuntimes({"core"});
    options.SetIntrinsicsBlacklist({"Math::absI32"});
    auto execPath = ark::os::file::File::GetExecutablePath();
    std::string pandaStdLib =
        execPath.Value() + Separator() + ".." + Separator() + "pandastdlib" + Separator() + "arkstdlib.abc";
    options.SetBootPandaFiles({pandaStdLib});
    CreateRuntime(options);
    pandasm::Parser parser;
    auto res = parser.Parse(source);
    ASSERT_TRUE(res);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());
    Runtime::GetCurrent()->GetClassLinker()->AddPandaFile(std::move(pf));

    // There are no implementations of Math.absI32 other than intrinsic, so execution should be aborted
    ASSERT_DEATH(Runtime::GetCurrent()->Execute("_GLOBAL::main", {}), "");
}
}  // namespace ark::test
