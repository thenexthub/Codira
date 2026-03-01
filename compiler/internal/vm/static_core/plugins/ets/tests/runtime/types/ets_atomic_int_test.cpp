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

#include "types/ets_atomic_int.h"
#include "types/ets_class.h"
#include "plugins/ets/tests/runtime/types/ets_test_mirror_classes.h"

namespace ark::ets::test {

class EtsAtomicIntMembers : public EtsMirrorClassTestBase {
public:
    static std::vector<MirrorFieldInfo> GetMembers()
    {
        return std::vector<MirrorFieldInfo> {MIRROR_FIELD_INFO(EtsAtomicInt, value_, "value")};
    }
};

// Note: Need to add concurrent tests by ANI #28032
TEST_F(EtsAtomicIntMembers, MemoryLayout)
{
    auto *klass = GetClassLinker()->GetClass("Lstd/containers/AtomicInt;");
    ASSERT(klass != nullptr);
    MirrorFieldInfo::CompareMemberOffsets(klass, GetMembers());
}

TEST_F(EtsAtomicIntMembers, MemoryLayout2)
{
    auto *klass = GetClassLinker()->GetClass("Lstd/concurrency/AtomicInt;");
    ASSERT(klass != nullptr);
    MirrorFieldInfo::CompareMemberOffsets(klass, GetMembers());
}
}  // namespace ark::ets::test
