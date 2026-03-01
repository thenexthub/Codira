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
#include "runtime/include/runtime.h"
#include "value/abstract_typed_value.h"
#include "public_internal.h"
#include "jobs/service.h"
#include "type/type_system.h"

#include "util/tests/verifier_test.h"

#include <gtest/gtest.h>

#include <functional>

namespace ark::verifier::test {

TEST_F(VerifierTest, AbstractTypedValue)
{
    using Builtin = Type::Builtin;
    auto *config = NewConfig();
    RuntimeOptions runtimeOpts;
    Runtime::Create(runtimeOpts);
    auto *service = CreateService(config, Runtime::GetCurrent()->GetInternalAllocator(),
                                  Runtime::GetCurrent()->GetClassLinker(), "");
    TypeSystem typeSystem(service->verifierService);
    Variables variables;

    auto top = Type::Top();

    auto i16 = Type {Builtin::I16};
    auto i32 = Type {Builtin::I32};
    auto u16 = Type {Builtin::U16};

    auto reference = Type {Builtin::REFERENCE};

    auto nv = [&variables] { return variables.NewVar(); };

    AbstractTypedValue av1 {i16, nv()};
    AbstractTypedValue av2 {i32, nv()};

    auto av3 = AtvJoin(&av1, &av2, &typeSystem);
    auto t3 = av3.GetAbstractType();

    EXPECT_EQ(t3, i32);

    AbstractTypedValue av4 {u16, nv()};

    auto av5 = AtvJoin(&av2, &av4, &typeSystem);
    auto t5 = av5.GetAbstractType();

    EXPECT_EQ(t5, i32);

    AbstractTypedValue av6 {reference, nv()};

    auto av7 = AtvJoin(&av1, &av6, &typeSystem);
    auto t7 = av7.GetAbstractType();

    EXPECT_EQ(t7, top);

    DestroyService(service, false);
    DestroyConfig(config);
}

}  // namespace ark::verifier::test
