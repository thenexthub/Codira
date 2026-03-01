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

#include "ets_mirror_class_test_base.h"

#include "types/ets_atomic_flag.h"
#include "types/ets_class.h"
#include "plugins/ets/tests/runtime/types/ets_test_mirror_classes.h"

namespace ark::ets::test {

class EtsAtomicFlagTest : public EtsMirrorClassTestBase {
public:
    static std::vector<MirrorFieldInfo> GetMembers()
    {
        return std::vector<MirrorFieldInfo> {MIRROR_FIELD_INFO(EtsAtomicFlag, flag_, "flag")};
    }
};

TEST_F(EtsAtomicFlagTest, MemoryLayout)
{
    auto *klass = GetClassLinker()->GetClass("Lstd/debug/concurrency/AtomicFlag;");
    ASSERT(klass != nullptr);
    MirrorFieldInfo::CompareMemberOffsets(klass, GetMembers());
}
}  // namespace ark::ets::test
