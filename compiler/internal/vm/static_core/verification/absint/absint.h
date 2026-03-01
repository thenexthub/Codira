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

#ifndef PANDA_VERIFIER_ABSINT_HPP_
#define PANDA_VERIFIER_ABSINT_HPP_

#include "verification_context.h"
#include "verification/type/type_system.h"

#include "verification/verification_status.h"

#include "cflow/cflow_info.h"

#include "runtime/include/method.h"

#include <optional>

namespace ark::verifier {
VerificationContext PrepareVerificationContext(TypeSystem *typeSystem, Job const *job);

VerificationStatus AbstractInterpret(VerificationContext &verifCtx, const uint8_t *pc);

VerificationStatus VerifyMethod(VerificationContext &verifCtx);
}  // namespace ark::verifier

#endif  // !PANDA_VERIFIER_ABSINT_HPP_
