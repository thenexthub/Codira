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

#include "libarkfile/bytecode_instruction-inl.h"
#include "libarkfile/file_items.h"
#include "libarkbase/macros.h"
#include "include/runtime.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/include/method-inl.h"

#include "libarkbase/utils/logger.h"

#include "util/str.h"

#include "cflow_info.h"

#include <iomanip>

#include "cflow_iterate_inl.h"

#include "verification/jobs/job.h"
#include "verification/cflow/cflow_common.h"
#include "verification/public_internal.h"
#include "verification/verification_status.h"

#include "verifier_messages.h"

namespace ark::verifier {

VerificationStatus CflowMethodInfo::FillCodeMaps(Method const *method)
{
    auto status = IterateOverInstructions(
        addrStart_, addrStart_, addrEnd_,
        [this, method]([[maybe_unused]] auto typ, uint8_t const *pc, size_t sz, bool exceptionSource,
                       [[maybe_unused]] auto tgt) -> std::optional<VerificationStatus> {
            SetFlag(pc, INSTRUCTION);
            if (exceptionSource) {
                SetFlag(pc, EXCEPTION_SOURCE);
            }
            if (tgt != nullptr) {  // a jump
                if (!IsAddrValid(tgt)) {
                    LOG_VERIFIER_CFLOW_INVALID_JUMP_OUTSIDE_METHOD_BODY(
                        method->GetFullName(), OffsetAsHexStr(addrStart_, tgt), OffsetAsHexStr(addrStart_, pc));
                    return VerificationStatus::ERROR;
                }
                SetFlag(tgt, JUMP_TARGET);
            }
            uint8_t const *nextInstPc = &pc[sz];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            if (nextInstPc == addrEnd_) {
                return VerificationStatus::OK;
            }
            if (nextInstPc > addrEnd_) {
                LOG_VERIFIER_CFLOW_INVALID_INSTRUCTION(OffsetAsHexStr(addrStart_, pc));
                return VerificationStatus::ERROR;
            }
            return std::nullopt;
        });
    return status;
}

VerificationStatus CflowMethodInfo::ProcessCatchBlocks(Method const *method)
{
    using CatchBlock = panda_file::CodeDataAccessor::CatchBlock;

    LOG(DEBUG, VERIFIER) << "Tracing exception handlers.";

    auto status = VerificationStatus::OK;
    method->EnumerateCatchBlocks([&]([[maybe_unused]] uint8_t const *tryStartPc,
                                     [[maybe_unused]] uint8_t const *tryEndPc, CatchBlock const &catchBlock) {
        auto catchBlockStart = reinterpret_cast<uint8_t const *>(reinterpret_cast<uintptr_t>(addrStart_) +
                                                                 static_cast<uintptr_t>(catchBlock.GetHandlerPc()));
        auto catchBlockEnd =
            &catchBlockStart[catchBlock.GetCodeSize()];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        if (catchBlockStart > catchBlockEnd || catchBlockStart >= addrEnd_ || catchBlockEnd < addrStart_) {
            LOG_VERIFIER_CFLOW_BAD_CATCH_BLOCK_BOUNDARIES(OffsetAsHexStr(addrStart_, catchBlockStart),
                                                          OffsetAsHexStr(addrStart_, catchBlockEnd));
            status = VerificationStatus::ERROR;
            return false;
        }
        if (catchBlockEnd == catchBlockStart) {
            // special case, no need to iterate over instructions.
            return true;
        }

        handlerStartAddresses_.push_back(catchBlockStart);

        auto result = IterateOverInstructions(
            catchBlockStart, addrStart_, addrEnd_,
            [this, catchBlockEnd]([[maybe_unused]] auto typ, uint8_t const *pc, size_t sz,
                                  [[maybe_unused]] bool exceptionSource,
                                  [[maybe_unused]] auto tgt) -> std::optional<VerificationStatus> {
                SetFlag(pc, EXCEPTION_HANDLER);
                uint8_t const *nextInstPc = &pc[sz];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                if (nextInstPc == catchBlockEnd) {
                    return VerificationStatus::OK;
                }
                if (nextInstPc > catchBlockEnd) {
                    LOG_VERIFIER_CFLOW_INVALID_INSTRUCTION(OffsetAsHexStr(addrStart_, pc));
                    return VerificationStatus::ERROR;
                }
                return std::nullopt;
            });
        status = std::max(status, result);
        return true;
    });

    // Serves as a barrier
    auto endCode = reinterpret_cast<uint8_t const *>(reinterpret_cast<uintptr_t>(method->GetInstructions()) +
                                                     method->GetCodeSize());
    handlerStartAddresses_.push_back(endCode);
    std::sort(handlerStartAddresses_.begin(), handlerStartAddresses_.end());

    return status;
}

PandaUniquePtr<CflowMethodInfo> GetCflowMethodInfo(Method const *method)
{
    const uint8_t *methodPcStartPtr = method->GetInstructions();
    size_t codeSize = method->GetCodeSize();

    auto cflowInfo = MakePandaUnique<CflowMethodInfo>(methodPcStartPtr, codeSize);

    LOG(DEBUG, VERIFIER) << "Build control flow info for method " << method->GetFullName();

    // 1. fill instructions map
    LOG(DEBUG, VERIFIER) << "Build instructions map.";
    if (cflowInfo == nullptr) {
        LOG(ERROR, VERIFIER) << "cflowInfo is nullptr ";
        return {};
    }
    if (cflowInfo->FillCodeMaps(method) == VerificationStatus::ERROR) {
        return {};
    }

    // 2. Mark exception handlers.
    if (cflowInfo->ProcessCatchBlocks(method) == VerificationStatus::ERROR) {
        return {};
    }

    return cflowInfo;
}

}  // namespace ark::verifier
