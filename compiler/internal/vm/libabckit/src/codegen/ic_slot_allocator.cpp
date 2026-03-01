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

#include "ic_slot_allocator.h"

#include "generated/ic_info.h"

namespace libabckit {

// CC-OFFNXT(WordsTool.95) sensitive word conflict
// NOLINTNEXTLINE(google-build-using-namespace)
using namespace ark;

bool ICSlotAllocator::RunImpl()
{
    if (!Allocate8BitIcSlots()) {
        return false;
    }

    if (!Allocate8Or16BitIcSlots()) {
        return false;
    }

    if (!Allocate16BitIcSlots()) {
        return false;
    }

    return true;
}

template <typename Callback>
void ICSlotAllocator::VisitInstructions(const Callback &callback)
{
    for (auto bb : GetGraph()->GetBlocksRPO()) {
        for (auto inst : bb->AllInsts()) {
            callback(inst);
        }
    }
}

bool ICSlotAllocator::Allocate8BitIcSlots()
{
    VisitInstructions([this](compiler::Inst *inst) {
        if (inst->GetOpcode() != compiler::Opcode::Intrinsic) {
            return;
        }

        auto intrinsicInst = inst->CastToIntrinsic();
        auto intrinsicId = intrinsicInst->GetIntrinsicId();
        auto icFlags = GetIcFlags(intrinsicId);
        if ((icFlags & IcFlags::EIGHT_BIT_IC) == 0) {
            return;
        }

        if ((icFlags & IcFlags::ONE_SLOT) != 0 && nextFree8BitSlot_ < INVALID_8_BIT_SLOT) {
            intrinsicInst->SetImm(0, nextFree8BitSlot_);
            nextFree8BitSlot_ += 1U;
            (*icSlotNumber_)++;
        } else if ((icFlags & IcFlags::TWO_SLOT) != 0 && nextFree8BitSlot_ < INVALID_8_BIT_SLOT - 1U) {
            intrinsicInst->SetImm(0, nextFree8BitSlot_);
            nextFree8BitSlot_ += 2U;
            (*icSlotNumber_) += 2U;
        } else {
            intrinsicInst->SetImm(0, INVALID_8_BIT_SLOT);
        }
    });

    return true;
}

bool ICSlotAllocator::Allocate8Or16BitIcSlots()
{
    VisitInstructions([this](compiler::Inst *inst) {
        if (inst->GetOpcode() != compiler::Opcode::Intrinsic) {
            return;
        }

        auto intrinsicInst = inst->CastToIntrinsic();
        auto intrinsicId = intrinsicInst->GetIntrinsicId();
        auto icFlags = GetIcFlags(intrinsicId);
        if ((icFlags & IcFlags::EIGHT_SIXTEEN_BIT_IC) == 0) {
            return;
        }

        if ((icFlags & IcFlags::ONE_SLOT) != 0 && nextFree8BitSlot_ < INVALID_8_BIT_SLOT) {
            if (!HasInstWith8Or16BitSignature8BitIcSlot(intrinsicId)) {
                intrinsicInst->SetIntrinsicId(GetIdWithInvertedIcImm(intrinsicId));
            }
            intrinsicInst->SetImm(0, nextFree8BitSlot_);
            nextFree8BitSlot_ += 1U;
            (*icSlotNumber_)++;
        } else if ((icFlags & IcFlags::TWO_SLOT) != 0 && nextFree8BitSlot_ < INVALID_8_BIT_SLOT - 1U) {
            if (!HasInstWith8Or16BitSignature8BitIcSlot(intrinsicId)) {
                intrinsicInst->SetIntrinsicId(GetIdWithInvertedIcImm(intrinsicId));
            }
            intrinsicInst->SetImm(0, nextFree8BitSlot_);
            nextFree8BitSlot_ += 2U;
            (*icSlotNumber_) += 2U;
        } else if ((icFlags & IcFlags::ONE_SLOT) != 0 && nextFree16BitSlot_ < INVALID_16_BIT_SLOT) {
            if (HasInstWith8Or16BitSignature8BitIcSlot(intrinsicId)) {
                intrinsicInst->SetIntrinsicId(GetIdWithInvertedIcImm(intrinsicId));
            }
            intrinsicInst->SetImm(0, nextFree16BitSlot_);
            nextFree16BitSlot_ += 1U;
            (*icSlotNumber_)++;
        } else if ((icFlags & IcFlags::TWO_SLOT) != 0 && nextFree16BitSlot_ < INVALID_16_BIT_SLOT - 1U) {
            if (HasInstWith8Or16BitSignature8BitIcSlot(intrinsicId)) {
                intrinsicInst->SetIntrinsicId(GetIdWithInvertedIcImm(intrinsicId));
            }
            intrinsicInst->SetImm(0, nextFree16BitSlot_);
            nextFree16BitSlot_ += 2U;
            (*icSlotNumber_) += 2U;
        } else {
            if (!HasInstWith8Or16BitSignature8BitIcSlot(intrinsicId)) {
                intrinsicInst->SetIntrinsicId(GetIdWithInvertedIcImm(intrinsicId));
            }
            intrinsicInst->SetImm(0, INVALID_8_BIT_SLOT);
        }
    });

    return true;
}

bool ICSlotAllocator::Allocate16BitIcSlots()
{
    VisitInstructions([this](compiler::Inst *inst) {
        if (inst->GetOpcode() != compiler::Opcode::Intrinsic) {
            return;
        }

        auto intrinsicInst = inst->CastToIntrinsic();
        auto intrinsicId = intrinsicInst->GetIntrinsicId();
        auto icFlags = GetIcFlags(intrinsicId);
        if ((icFlags & IcFlags::SIXTEEN_BIT_IC) == 0) {
            return;
        }

        if ((icFlags & IcFlags::ONE_SLOT) != 0 && nextFree16BitSlot_ < INVALID_16_BIT_SLOT) {
            intrinsicInst->SetImm(0, nextFree16BitSlot_);
            nextFree16BitSlot_ += 1U;
            (*icSlotNumber_)++;
        } else if ((icFlags & IcFlags::TWO_SLOT) != 0 && nextFree16BitSlot_ < INVALID_16_BIT_SLOT - 1U) {
            intrinsicInst->SetImm(0, nextFree16BitSlot_);
            nextFree16BitSlot_ += 2U;
            (*icSlotNumber_) += 2U;
        } else {
            intrinsicInst->SetImm(0, INVALID_16_BIT_SLOT);
        }
    });

    return true;
}
}  // namespace libabckit
