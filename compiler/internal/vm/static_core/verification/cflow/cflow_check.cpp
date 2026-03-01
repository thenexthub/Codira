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

#include "cflow_check.h"
#include "cflow_common.h"
#include "cflow_iterate_inl.h"
#include "runtime/include/method-inl.h"
#include "libarkbase/utils/logger.h"
#include "verification/jobs/job.h"
#include "verification/util/str.h"
#include "verifier_messages.h"

#include <iomanip>
#include <optional>

namespace ark::verifier {

static VerificationStatus CheckValidFlagInstructionException(Method const *&method, CflowMethodInfo const *&cflowInfo,
                                                             const uint8_t *&target, uint8_t const *&methodStart,
                                                             uint8_t const *&pc)
{
    if (!cflowInfo->IsAddrValid(target)) {
        LOG_VERIFIER_CFLOW_INVALID_JUMP_OUTSIDE_METHOD_BODY(method->GetFullName(), OffsetAsHexStr(methodStart, target),
                                                            OffsetAsHexStr(methodStart, pc));
        return VerificationStatus::ERROR;
    }
    if (!cflowInfo->IsFlagSet(target, CflowMethodInfo::INSTRUCTION)) {
        LOG_VERIFIER_CFLOW_INVALID_JUMP_INTO_MIDDLE_OF_INSTRUCTION(
            method->GetFullName(), OffsetAsHexStr(methodStart, target), OffsetAsHexStr(methodStart, pc));
        return VerificationStatus::ERROR;
    }
    if (cflowInfo->IsFlagSet(target, CflowMethodInfo::EXCEPTION_HANDLER) &&
        !cflowInfo->IsFlagSet(pc, CflowMethodInfo::EXCEPTION_HANDLER)) {
        // - jumps into body of exception handler from code is prohibited by Panda compiler.
        LOG_VERIFIER_CFLOW_INVALID_JUMP_INTO_EXC_HANDLER(method->GetFullName(),
                                                         (OffsetAsHexStr(method->GetInstructions(), pc)));
        return VerificationStatus::ERROR;
    }

    return VerificationStatus::OK;
}

static VerificationStatus CheckCode(Method const *method, CflowMethodInfo const *cflowInfo)
{
    uint8_t const *methodStart = method->GetInstructions();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic
    uint8_t const *methodEnd = methodStart + method->GetCodeSize();
    auto handlerStarts = cflowInfo->GetHandlerStartAddresses();
    size_t nandlerIndex = (*handlerStarts)[0] == methodStart ? 0 : -1;
    return IterateOverInstructions(
        methodStart, methodStart, methodEnd,
        [&](auto type, uint8_t const *pc, size_t size, [[maybe_unused]] bool exceptionSource,
            auto target) -> std::optional<VerificationStatus> {
            if (target != nullptr) {  // a jump
                if (CheckValidFlagInstructionException(method, cflowInfo, target, methodStart, pc) ==
                    VerificationStatus::ERROR) {
                    return VerificationStatus::ERROR;
                }

                if (cflowInfo->IsFlagSet(target, CflowMethodInfo::EXCEPTION_HANDLER) &&
                    (target < (*handlerStarts)[nandlerIndex] || target >= (*handlerStarts)[nandlerIndex + 1])) {
                    // Jump from handler to handler; need to make sure it's the same one.
                    LOG_VERIFIER_CFLOW_INVALID_JUMP_INTO_EXC_HANDLER(method->GetFullName(),
                                                                     (OffsetAsHexStr(method->GetInstructions(), pc)));
                    return VerificationStatus::ERROR;
                }
            }
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            if (&pc[size] == methodEnd) {
                if (type != InstructionType::THROW && type != InstructionType::JUMP &&
                    type != InstructionType::RETURN) {
                    LOG_VERIFIER_CFLOW_INVALID_LAST_INSTRUCTION(method->GetFullName());
                    return VerificationStatus::ERROR;
                }
                return VerificationStatus::OK;
            }
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            if (&pc[size] == (*handlerStarts)[nandlerIndex + 1]) {
                if (type != InstructionType::JUMP && type != InstructionType::RETURN &&
                    type != InstructionType::THROW) {
                    // - fallthrough on beginning of exception handler is prohibited by Panda compiler
                    LOG_VERIFIER_CFLOW_BODY_FALL_INTO_EXC_HANDLER(method->GetFullName(),
                                                                  (OffsetAsHexStr(method->GetInstructions(), pc)));
                    return VerificationStatus::ERROR;
                }
                nandlerIndex++;
            }
            return std::nullopt;
        });
}

PandaUniquePtr<CflowMethodInfo> CheckCflow(Method const *method)
{
    auto cflowInfo = GetCflowMethodInfo(method);
    if (!cflowInfo) {
        return {};
    }

    if (CheckCode(method, cflowInfo.get()) == VerificationStatus::ERROR) {
        return {};
    }

    return cflowInfo;
}

}  // namespace ark::verifier
