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

#include "ets_coroutine.h"
#include "ets_vm.h"

#include "plugins/ets/runtime/types/ets_typeapi_field.h"
#include "plugins/ets/runtime/types/ets_typeapi_method.h"
#include "plugins/ets/runtime/types/ets_typeapi_parameter.h"
#include "plugins/ets/runtime/types/ets_typeapi_type.h"
#include "plugins/ets/tests/runtime/types/ets_test_mirror_classes.h"

namespace ark::ets::test {

class EtsTypeAPITest : public testing::Test {
public:
    EtsTypeAPITest()
    {
        options_.SetShouldLoadBootPandaFiles(true);
        options_.SetShouldInitializeIntrinsics(true);
        options_.SetCompilerEnableJit(false);
        options_.SetGcType("g1-gc");
        options_.SetLoadRuntimes({"ets"});

        auto stdlib = std::getenv("PANDA_STD_LIB");
        if (stdlib == nullptr) {
            std::cerr << "PANDA_STD_LIB env variable should be set and point to mock_stdlib.abc" << std::endl;
            std::abort();
        }
        options_.SetBootPandaFiles({stdlib});

        Runtime::Create(options_);
    }

    ~EtsTypeAPITest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(EtsTypeAPITest);
    NO_MOVE_SEMANTIC(EtsTypeAPITest);

    void SetUp() override
    {
        coroutine_ = EtsCoroutine::GetCurrent();
        vm_ = coroutine_->GetPandaVM();
        coroutine_->ManagedCodeBegin();
    }

    void TearDown() override
    {
        coroutine_->ManagedCodeEnd();
    }

    static std::vector<MirrorFieldInfo> GetTypeAPITypeClassMembers()
    {
        return std::vector<MirrorFieldInfo> {MIRROR_FIELD_INFO(EtsTypeAPIType, td_, "td"),
                                             MIRROR_FIELD_INFO(EtsTypeAPIType, contextLinker_, "contextLinker")};
    }

    static std::vector<MirrorFieldInfo> GetTypeAPIMethodClassMembers()
    {
        return std::vector<MirrorFieldInfo> {MIRROR_FIELD_INFO(EtsTypeAPIMethod, methodType_, "methodType"),
                                             MIRROR_FIELD_INFO(EtsTypeAPIMethod, ownerType_, "ownerType"),
                                             MIRROR_FIELD_INFO(EtsTypeAPIMethod, name_, "name"),
                                             MIRROR_FIELD_INFO(EtsTypeAPIMethod, attr_, "attributes"),
                                             MIRROR_FIELD_INFO(EtsTypeAPIMethod, accessMod_, "accessMod")};
    }

    static std::vector<MirrorFieldInfo> GetTypeAPIFieldClassMembers()
    {
        return std::vector<MirrorFieldInfo> {MIRROR_FIELD_INFO(EtsTypeAPIField, fieldType_, "fieldType"),
                                             MIRROR_FIELD_INFO(EtsTypeAPIField, ownerType_, "ownerType"),
                                             MIRROR_FIELD_INFO(EtsTypeAPIField, name_, "name"),
                                             MIRROR_FIELD_INFO(EtsTypeAPIField, attr_, "attributes"),
                                             MIRROR_FIELD_INFO(EtsTypeAPIField, accessMod_, "accessMod")};
    }

    static std::vector<MirrorFieldInfo> GetTypeAPIParameterClassMembers()
    {
        return std::vector<MirrorFieldInfo> {MIRROR_FIELD_INFO(EtsTypeAPIParameter, paramType_, "paramType"),
                                             MIRROR_FIELD_INFO(EtsTypeAPIParameter, name_, "name"),
                                             MIRROR_FIELD_INFO(EtsTypeAPIParameter, attr_, "attributes")};
    }

protected:
    PandaEtsVM *vm_ = nullptr;  // NOLINT(misc-non-private-member-variables-in-classes)

private:
    RuntimeOptions options_;
    EtsCoroutine *coroutine_ = nullptr;
};

TEST_F(EtsTypeAPITest, TypeAPITypeMemoryLayout)
{
    auto *typeAPITypeClass = vm_->GetClassLinker()->GetClass("Lstd/core/Type;");
    ASSERT(typeAPITypeClass != nullptr);
    MirrorFieldInfo::CompareMemberOffsets(typeAPITypeClass, GetTypeAPITypeClassMembers());
}

TEST_F(EtsTypeAPITest, TypeAPIMethodMemoryLayout)
{
    auto *typeAPIMethodClass = PlatformTypes(vm_)->coreMethod;
    MirrorFieldInfo::CompareMemberOffsets(typeAPIMethodClass, GetTypeAPIMethodClassMembers());
}

TEST_F(EtsTypeAPITest, TypeAPIFieldMemoryLayout)
{
    auto *typeAPIFieldClass = PlatformTypes(vm_)->coreField;
    MirrorFieldInfo::CompareMemberOffsets(typeAPIFieldClass, GetTypeAPIFieldClassMembers());
}

TEST_F(EtsTypeAPITest, TypeAPIParameterMemoryLayout)
{
    auto *typeAPIParameterClass = PlatformTypes(vm_)->coreTypeAPIParameter;
    MirrorFieldInfo::CompareMemberOffsets(typeAPIParameterClass, GetTypeAPIParameterClassMembers());
}

}  // namespace ark::ets::test
