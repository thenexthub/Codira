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

case RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_MEMMOVE_UNCHECKED_REF: {
    auto *preheader = loop_.GetPreHeader();
    auto *ssd = preheader->FindSaveStateDeoptimize();
    // We should create mmove if and only if the preheader contains a SaveStateDeoptimize instruction
    // The switch in ark::compiler::ArrayMoveParser::CreateIntrinsic() must check this fact in the case
    // for DataType::REF.
    ASSERT(ssd);
    auto *preheadSaveState = ssd->CastToSaveStateDeoptimize();
    auto *saveState = CopySaveState(GetGraph(), preheadSaveState);
    saveState->SetOpcode(Opcode::SaveState); // SaveStateDeoptimize -> SaveState
    // mmove's inputs, src and dst arrays, must be added to the SaveState.
    saveState->AppendBridge(load_.inst->GetArray());
    saveState->AppendBridge(store_.inst->GetArray());

    mmove->InsertBefore(saveState);
    mmove->AppendInput(saveState);
    mmove->AddInputType(DataType::NO_TYPE);
    mmove->ClearFlag(inst_flags::Flags::CAN_THROW);
    break;
}
