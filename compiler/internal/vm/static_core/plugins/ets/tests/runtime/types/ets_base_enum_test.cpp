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

#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_panda_file_items.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/types/ets_base_enum.h"
#include "plugins/ets/tests/runtime/types/ets_test_mirror_classes.h"

namespace ark::ets::test {

class EtsBaseEnumMembers : public testing::Test {
public:
    EtsBaseEnumMembers()
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(true);
        options.SetShouldInitializeIntrinsics(true);
        options.SetCompilerEnableJit(false);
        options.SetGcType("epsilon");
        options.SetLoadRuntimes({"ets"});

        auto stdlib = std::getenv("PANDA_STD_LIB");
        if (stdlib == nullptr) {
            std::cerr << "PANDA_STD_LIB env variable should be set and point to mock_stdlib.abc" << std::endl;
            std::abort();
        }
        options.SetBootPandaFiles({stdlib});

        Runtime::Create(options);
        EtsCoroutine *coroutine = EtsCoroutine::GetCurrent();
        vm_ = coroutine->GetPandaVM();
    }

    ~EtsBaseEnumMembers() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(EtsBaseEnumMembers);
    NO_MOVE_SEMANTIC(EtsBaseEnumMembers);

    static std::vector<MirrorFieldInfo> GetMembers()
    {
        return std::vector<MirrorFieldInfo> {MIRROR_FIELD_INFO(EtsBaseEnum, value_, "value")};
    }

    EtsClass *GetClass(const char *name)
    {
        os::memory::WriteLockHolder lock(*vm_->GetMutatorLock());
        return vm_->GetClassLinker()->GetClass(name);
    }

private:
    PandaEtsVM *vm_ = nullptr;
};

TEST_F(EtsBaseEnumMembers, MemoryLayout)
{
    auto *klass = GetClass(panda_file_items::class_descriptors::BASE_ENUM.data());
    MirrorFieldInfo::CompareMemberOffsets(klass, GetMembers());
}
}  // namespace ark::ets::test
