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

#include "assembly-emitter.h"
#include "assembly-parser.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "verification/type/type_system.h"
#include "libarkbase/utils/utf.h"

namespace ark::ets::test {

class EtsUnionTest : public testing::Test {
public:
    EtsUnionTest()
    {
        options_.SetShouldLoadBootPandaFiles(true);
        options_.SetShouldInitializeIntrinsics(true);
        options_.SetCompilerEnableJit(false);
        options_.SetGcType("epsilon");
        options_.SetLoadRuntimes({"ets"});

        auto stdlib = std::getenv("PANDA_STD_LIB");
        if (stdlib == nullptr) {
            std::cerr << "PANDA_STD_LIB env variable should be set and point to mock_stdlib.abc" << std::endl;
            std::abort();
        }
        options_.SetBootPandaFiles({stdlib});

        Runtime::Create(options_);
    }

    ~EtsUnionTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(EtsUnionTest);
    NO_MOVE_SEMANTIC(EtsUnionTest);

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

    bool CheckConstituentClasses(EtsClass *unionClass, const std::vector<PandaString> &consClassesList)
    {
        size_t idx = 0;
        bool res = true;
        unionClass->EnumerateConstituentClasses([&](EtsClass *consClass) {
            res &= consClass->GetRuntimeClass()->IsLoaded();
            if (PandaString(consClass->GetDescriptor()) != consClassesList[idx++]) {
                res = false;
                return true;
            }
            if (PandaString(consClass->GetDescriptor()) == "[{ULB;LC;}") {
                res &= consClass->IsArrayClass() && consClass->GetComponentType()->IsUnionClass();
                res &= CheckConstituentClasses(consClass->GetComponentType(), {"LB;", "LC;"});
            }
            return false;
        });
        return res;
    }

protected:
    PandaEtsVM *vm_ = nullptr;  // NOLINT(misc-non-private-member-variables-in-classes)

private:
    RuntimeOptions options_;
    EtsCoroutine *coroutine_ = nullptr;
};

TEST_F(EtsUnionTest, CreateUnionClass)
{
    pandasm::Parser p;

    const char *source =
        ".language eTS\n"
        ".record std.core.Object <external>\n"
        ".record A <ets.extends=std.core.Object, access.record=public> {}\n"
        ".record B <ets.extends=A, access.record=public> {}\n"
        ".record C <ets.extends=A, access.record=public> {}\n"
        ".record D <ets.extends=std.core.Object, access.record=public> {}\n"

        ".record {UA,B} <external>\n"
        ".record {UB,C} <external>\n"
        ".record {UA,B,C} <external>\n"
        ".record {UA,D} <external>\n"
        ".record {UA,{UB,C}[],D} <external>";

    auto res = p.Parse(source);
    ASSERT_EQ(p.ShowError().err, pandasm::Error::ErrorType::ERR_NONE);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());

    auto classLinker = Runtime::GetCurrent()->GetClassLinker();
    ASSERT(classLinker);
    classLinker->AddPandaFile(std::move(pf));

    std::map<PandaString, std::tuple<bool, PandaString, std::vector<PandaString>>> descMap {
        {"LA;", {false, "", {}}},
        {"{ULA;LB;}", {true, "LA;", {"LA;", "LB;"}}},
        {"{ULB;LC;}", {true, "LA;", {"LB;", "LC;"}}},
        {"{ULA;LB;LC;}", {true, "LA;", {"LA;", "LB;", "LC;"}}},
        {"{ULA;LB;[LC;}", {true, "Lstd/core/Object;", {"LA;", "LB;", "[LC;"}}},
        {"{ULA;Lstd/core/String;[LC;}", {true, "Lstd/core/Object;", {"LA;", "Lstd/core/String;", "[LC;"}}},
        {"{ULA;LD;[{ULB;LC;}}", {true, "Lstd/core/Object;", {"LA;", "LD;", "[{ULB;LC;}"}}}};

    for (const auto &[desc, infoList] : descMap) {
        EtsClass *klass = vm_->GetClassLinker()->GetClass(desc.c_str());
        ASSERT_NE(klass, nullptr);
        ASSERT_TRUE(klass->GetRuntimeClass()->IsLoaded());

        auto isUnion = std::get<bool>(infoList);
        auto classLUB = std::get<PandaString>(infoList);
        const auto &consClassesList = std::get<std::vector<PandaString>>(infoList);
        EXPECT_EQ(klass->IsUnionClass(), isUnion);
        EXPECT_EQ(PandaString(klass->GetDescriptor()), desc);
        if (!isUnion) {
            continue;
        }
        ASSERT_TRUE(klass->IsInitialized());

        ASSERT_TRUE(CheckConstituentClasses(klass, consClassesList));
    }
}

}  // namespace ark::ets::test
