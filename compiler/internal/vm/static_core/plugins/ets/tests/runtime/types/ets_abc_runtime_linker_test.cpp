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

#include "types/ets_abc_file.h"
#include "types/ets_abc_runtime_linker.h"
#include "plugins/ets/tests/runtime/types/ets_test_mirror_classes.h"

namespace ark::ets::test {

class EtsAbcRuntimeLinkerTest : public EtsMirrorClassTestBase {
public:
    static std::vector<MirrorFieldInfo> GetEtsAbcClassMembers()
    {
        return std::vector<MirrorFieldInfo> {MIRROR_FIELD_INFO(EtsAbcFile, fileHandlePtr_, "fileHandlePtr")};
    }

    static std::vector<MirrorFieldInfo> GetEtsAbcRuntimeLinkerClassMembers()
    {
        return std::vector<MirrorFieldInfo> {
            MIRROR_FIELD_INFO(EtsAbcRuntimeLinker, parentLinker_, "parentLinker"),
            MIRROR_FIELD_INFO(EtsAbcRuntimeLinker, abcFiles_, "abcFiles"),
            MIRROR_FIELD_INFO(EtsAbcRuntimeLinker, sharedRuntimeLinkers_, "sharedLibraryLoaders"),
        };
    }
};

TEST_F(EtsAbcRuntimeLinkerTest, AbcFileMemoryLayout)
{
    EtsClass *abcFileClass = GetPlatformTypes()->coreAbcFile;
    MirrorFieldInfo::CompareMemberOffsets(abcFileClass, GetEtsAbcClassMembers());
}

TEST_F(EtsAbcRuntimeLinkerTest, AbcRuntimeLinkerMemoryLayout)
{
    EtsClass *abcRuntimeLinkerClass = GetPlatformTypes()->coreAbcRuntimeLinker;
    MirrorFieldInfo::CompareMemberOffsets(abcRuntimeLinkerClass, GetEtsAbcRuntimeLinkerClassMembers());
}

}  // namespace ark::ets::test
