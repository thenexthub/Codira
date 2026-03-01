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

#include "optimizer/ir/basicblock.h"
#include "monitor_analysis.h"
#include "optimizer/ir/graph.h"
#include "compiler_logger.h"

namespace ark::compiler {
void MonitorAnalysis::MarkedMonitorRec(BasicBlock *bb, int32_t numMonitors)
{
    ASSERT(numMonitors >= 0);
    if (numMonitors > 0) {
        bb->SetMonitorBlock(true);
        if (bb->IsEndBlock()) {
            COMPILER_LOG(DEBUG, MONITOR_ANALYSIS) << "There is MonitorEntry without MonitorExit";
            incorrectMonitors_ = true;
            return;
        }
    }
    for (auto inst : bb->Insts()) {
        if (inst->GetOpcode() == Opcode::Throw) {
            // The Monitor.Exit is removed from the compiled code after explicit Throw instruction
            // in the syncronized block because the execution is switching to the interpreting mode
            numMonitors = 0;
        }
        if (inst->GetOpcode() == Opcode::Monitor) {
            bb->SetMonitorBlock(true);
            if (inst->CastToMonitor()->IsEntry()) {
                bb->SetMonitorEntryBlock(true);
                ++numMonitors;
                continue;
            }
            ASSERT(inst->CastToMonitor()->IsExit());
            if (numMonitors <= 0) {
                COMPILER_LOG(DEBUG, MONITOR_ANALYSIS) << "There is MonitorExit without MonitorEntry";
                incorrectMonitors_ = true;
                return;
            }
            bb->SetMonitorExitBlock(true);
            --numMonitors;
        }
    }
    enteredMonitorsCount_->at(bb->GetId()) = static_cast<uint32_t>(numMonitors);
    if (numMonitors == 0) {
        return;
    }
    for (auto succ : bb->GetSuccsBlocks()) {
        if (checkNonCatchOnly_ && succ->IsCatch()) {
            continue;
        }
        if (succ->SetMarker(marker_)) {
            continue;
        }
        MarkedMonitorRec(succ, numMonitors);
    }
}

bool MonitorAnalysis::RunImpl()
{
    auto allocator = GetGraph()->GetLocalAllocator();
    incorrectMonitors_ = false;
    enteredMonitorsCount_ = allocator->New<ArenaVector<uint32_t>>(allocator->Adapter());
    ASSERT(enteredMonitorsCount_ != nullptr);
    enteredMonitorsCount_->resize(GetGraph()->GetVectorBlocks().size());
    marker_ = GetGraph()->NewMarker();
    for (auto bb : GetGraph()->GetBlocksRPO()) {
        if (checkNonCatchOnly_ && bb->IsCatch()) {
            continue;
        }
        if (bb->SetMarker(marker_)) {
            continue;
        }
        MarkedMonitorRec(bb, 0);
        if (incorrectMonitors_) {
            return false;
        }
    }
    for (auto bb : GetGraph()->GetBlocksRPO()) {
        if (checkNonCatchOnly_ && bb->IsCatch()) {
            continue;
        }
        const uint32_t uninitialized = 0xFFFFFFFF;
        uint32_t count = uninitialized;
        for (auto prev : bb->GetPredsBlocks()) {
            if (checkNonCatchOnly_ && prev->IsCatch()) {
                continue;
            }
            if (count == uninitialized) {
                count = enteredMonitorsCount_->at(prev->GetId());
            } else if (count != enteredMonitorsCount_->at(prev->GetId())) {
                COMPILER_LOG(DEBUG, MONITOR_ANALYSIS)
                    << "There is an inconsistent MonitorEntry counters in parent basic blocks";
                return false;
            }
        }
    }
    GetGraph()->EraseMarker(marker_);
    return true;
}
}  // namespace ark::compiler
