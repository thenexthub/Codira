/*
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
#ifndef COMPILER_OPTIMIZER_ANALYSIS_MONITOR_ANALYSIS_H
#define COMPILER_OPTIMIZER_ANALYSIS_MONITOR_ANALYSIS_H

#include "optimizer/ir/graph.h"
#include "optimizer/ir/marker.h"
#include "optimizer/pass.h"

namespace ark::compiler {
/*
 * The analysis checks MonitorEntry and MonitorExit in the Graph and set properties for blocks:
 *     1. BlockMonitorEntry for blocks with MonitorEntry
 *     2. BlockMonitorExit for blocks with MonitorExit
 *     3. BlockMonitor for blocks between BlockMonitorEntry and BlockMonitorExit
 * The analysis returns false if there is way with MonitorEntry and without MonitorExit or Vice versa
 * For this case the analysis return false:
 *    bb 1
 *    |    \
 *    bb 2   bb 3
 *    |    MonitorEntry
 *    |    /
 *    bb 4   - Conditional is equal to bb 1
 *    |    \
 *    bb 5   bb 6
 *    |    MonitorExit
 *    |    /
 *    bb 7
 */
class MonitorAnalysis final : public Analysis {
public:
    using Analysis::Analysis;

    const char *GetPassName() const override
    {
        return "MonitorAnalysis";
    }

    bool RunImpl() override;

    void SetCheckNonCatchOnly(bool checkNonCatchOnly)
    {
        checkNonCatchOnly_ = checkNonCatchOnly;
    }

private:
    void MarkedMonitorRec(BasicBlock *bb, int32_t numMonitors);

    Marker marker_ {UNDEF_MARKER};
    bool incorrectMonitors_ {false};
    ArenaVector<uint32_t> *enteredMonitorsCount_ {nullptr};
    bool checkNonCatchOnly_ {false};
};
}  // namespace ark::compiler
#endif  // COMPILER_OPTIMIZER_ANALYSIS_MONITOR_ANALYSIS_H
