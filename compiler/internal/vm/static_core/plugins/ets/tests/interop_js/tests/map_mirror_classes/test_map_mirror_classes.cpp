/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#include "ets_interop_js_gtest.h"
#include "plugins/ets/runtime/interop_js/js_value.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "runtime/include/thread_scopes.h"

namespace ark::ets::interop::js::testing {

class EtsInteropJsClassLinkerTest : public EtsInteropTest {};

struct MemberInfo {
    uint32_t offset;
    const char *name;
};

static void CheckOffsetOfFields(const char *className, const std::vector<MemberInfo> &membersList)
{
    ScopedManagedCodeThread scoped(ManagedThread::GetCurrent());

    EtsClassLinker *etsClassLinker = PandaEtsVM::GetCurrent()->GetClassLinker();
    EtsClass *klass = etsClassLinker->GetClass(className);
    ASSERT_NE(klass, nullptr);

    auto fields = klass->GetRuntimeClass()->GetInstanceFields();
    ASSERT_EQ(fields.size(), membersList.size());

    for (size_t i = 0; i < fields.size(); i++) {
        ASSERT_NE(fields[i].GetName().data, nullptr);
        EXPECT_STREQ(utf::Mutf8AsCString(fields[i].GetName().data), membersList[i].name);
    }
}

class JSValueOffsets {
public:
    static std::vector<MemberInfo> GetMembersInfo()
    {
        return std::vector<MemberInfo> {
            MemberInfo {MEMBER_OFFSET(JSValue, type_), "__internal_field_0"},  // long
            MemberInfo {MEMBER_OFFSET(JSValue, data_), "__internal_field_1"},  // long
        };
    }
};

TEST_F(EtsInteropJsClassLinkerTest, Filed_std_interop_js_JSValue)
{
    CheckOffsetOfFields("Lstd/interop/js/JSValue;", JSValueOffsets::GetMembersInfo());
}

class ESValueOffsets {
public:
    static std::vector<MemberInfo> GetMembersInfo()
    {
        return std::vector<MemberInfo> {
            MemberInfo {MEMBER_OFFSET(ESValue, eo_), "ev"},
            MemberInfo {MEMBER_OFFSET(ESValue, isStatic_), "isStatic"},
        };
    }
};

TEST_F(EtsInteropJsClassLinkerTest, Filed_std_interop_ESValue)
{
    // #IC8LH8: change class name after ESObject is finally removed
    CheckOffsetOfFields("Lstd/interop/ESValue;", ESValueOffsets::GetMembersInfo());
}

}  // namespace ark::ets::interop::js::testing
