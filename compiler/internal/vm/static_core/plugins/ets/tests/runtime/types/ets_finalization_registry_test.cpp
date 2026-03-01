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
#include <array>

#include "libarkbase/os/mutex.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/include/gc_task.h"
#include "runtime/tests/test_utils.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_handle.h"
#include "plugins/ets/runtime/ets_handle_scope.h"
#include "plugins/ets/runtime/types/ets_class.h"
#include "plugins/ets/runtime/types/ets_finreg_node.h"
#include "plugins/ets/runtime/types/ets_finalization_registry.h"
#include "plugins/ets/tests/runtime/types/ets_test_mirror_classes.h"

namespace ark::ets::test {

class EtsFinalizationRegistryLayoutTest : public testing::Test {
public:
    EtsFinalizationRegistryLayoutTest()
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(true);
        options.SetShouldInitializeIntrinsics(true);
        options.SetCompilerEnableJit(false);
        options.SetGcType("epsilon");
        options.SetLoadRuntimes({"ets"});
        options.SetUseTlabForAllocations(false);
        options.SetGcTriggerType("debug");
        options.SetGcDebugTriggerStart(std::numeric_limits<int>::max());
        options.SetExplicitConcurrentGcEnabled(false);

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

    ~EtsFinalizationRegistryLayoutTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(EtsFinalizationRegistryLayoutTest);
    NO_MOVE_SEMANTIC(EtsFinalizationRegistryLayoutTest);

    static std::vector<MirrorFieldInfo> GetFinalizationRegistryMembers()
    {
        return std::vector<MirrorFieldInfo> {
            MIRROR_FIELD_INFO(EtsFinalizationRegistry, callback_, "callback"),
            MIRROR_FIELD_INFO(EtsFinalizationRegistry, mutex_, "mutex"),
            MIRROR_FIELD_INFO(EtsFinalizationRegistry, buckets_, "buckets"),
            MIRROR_FIELD_INFO(EtsFinalizationRegistry, nonUnregistrableList_, "nonUnregistrableList"),
            MIRROR_FIELD_INFO(EtsFinalizationRegistry, nextFinReg_, "nextFinReg"),
            MIRROR_FIELD_INFO(EtsFinalizationRegistry, finalizationQueueHead_, "finalizationQueueHead"),
            MIRROR_FIELD_INFO(EtsFinalizationRegistry, finalizationQueueTail_, "finalizationQueueTail"),
            MIRROR_FIELD_INFO(EtsFinalizationRegistry, workerId_, "workerId"),
            MIRROR_FIELD_INFO(EtsFinalizationRegistry, workerDomain_, "workerDomain"),
            MIRROR_FIELD_INFO(EtsFinalizationRegistry, mapSize_, "mapSize"),
            MIRROR_FIELD_INFO(EtsFinalizationRegistry, listSize_, "listSize")};
    }

    static std::vector<MirrorFieldInfo> GetFinRegNodeMembers()
    {
        return std::vector<MirrorFieldInfo> {MIRROR_FIELD_INFO(EtsFinRegNode, token_, "token"),
                                             MIRROR_FIELD_INFO(EtsFinRegNode, arg_, "arg"),
                                             MIRROR_FIELD_INFO(EtsFinRegNode, finReg_, "finReg"),
                                             MIRROR_FIELD_INFO(EtsFinRegNode, prev_, "prev"),
                                             MIRROR_FIELD_INFO(EtsFinRegNode, next_, "next"),
                                             MIRROR_FIELD_INFO(EtsFinRegNode, bucketIdx_, "bucketIdx")};
    }

protected:
    PandaEtsVM *vm_ = nullptr;  // NOLINT(misc-non-private-member-variables-in-classes)
};

TEST_F(EtsFinalizationRegistryLayoutTest, FinalizationRegistryMemoryLayout)
{
    EtsClass *finRegClass = PlatformTypes(vm_)->coreFinalizationRegistry;
    MirrorFieldInfo::CompareMemberOffsets(finRegClass, GetFinalizationRegistryMembers());
}

TEST_F(EtsFinalizationRegistryLayoutTest, FinRegNodeMemoryLayout)
{
    // Check the parent is BaseWeakRef because it affects memory layout of fields
    EtsClass *baseWeakRefClass = PlatformTypes(vm_)->coreBaseWeakRef;
    ASSERT_NE(nullptr, baseWeakRefClass);
    EtsClass *finRegNodeClass = PlatformTypes(vm_)->coreFinRegNode;
    ASSERT_EQ(baseWeakRefClass, finRegNodeClass->GetBase());
    MirrorFieldInfo::CompareMemberOffsets(finRegNodeClass, GetFinRegNodeMembers());
}
}  // namespace ark::ets::test
