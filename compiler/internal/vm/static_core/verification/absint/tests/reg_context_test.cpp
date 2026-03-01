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

#include "absint/reg_context.h"

#include "public_internal.h"
#include "jobs/service.h"
#include "type/type_system.h"
#include "value/abstract_typed_value.h"

#include "util/tests/verifier_test.h"

#include <gtest/gtest.h>

#include <functional>

namespace ark::verifier::test {

TEST_F(VerifierTest, AbsIntRegContext)
{
    using Builtin = Type::Builtin;
    auto *config = NewConfig();
    RuntimeOptions runtimeOpts;
    Runtime::Create(runtimeOpts);
    auto *service = CreateService(config, Runtime::GetCurrent()->GetInternalAllocator(),
                                  Runtime::GetCurrent()->GetClassLinker(), "");
    TypeSystem typeSystem(service->verifierService);
    Variables variables;

    auto i16 = Type {Builtin::I16};
    auto i32 = Type {Builtin::I32};

    auto u16 = Type {Builtin::U16};

    auto nv = [&variables] { return variables.NewVar(); };

    AbstractTypedValue av1 {i16, nv()};
    AbstractTypedValue av2 {i32, nv()};
    AbstractTypedValue av3 {u16, nv()};

    RegContext ctx1;
    RegContext ctx2;

    ctx1[-1] = av1;
    ctx2[0] = av2;

    auto ctx3 = RcUnion(&ctx1, &ctx2, &typeSystem);

    ctx3.RemoveInconsistentRegs();
    EXPECT_EQ(ctx3.Size(), 0);

    ctx1[0] = av1;

    ctx3 = RcUnion(&ctx1, &ctx2, &typeSystem);

    ctx3.RemoveInconsistentRegs();
    EXPECT_EQ(ctx3.Size(), 1);

    EXPECT_EQ(ctx3[0].GetAbstractType(), i32);

    DestroyService(service, false);
    DestroyConfig(config);
}

}  // namespace ark::verifier::test
