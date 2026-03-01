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

#ifndef PANDA_PASS_STATISTICS_H
#define PANDA_PASS_STATISTICS_H

#include <chrono>
#include "libarkbase/utils/arena_containers.h"

namespace ark::compiler {
class Graph;
class Pass;

class PassManagerStatistics {
public:
    explicit PassManagerStatistics(Graph *graph);

    NO_MOVE_SEMANTIC(PassManagerStatistics);
    NO_COPY_SEMANTIC(PassManagerStatistics);
    ~PassManagerStatistics() = default;

    void ProcessBeforeRun(const Pass &pass);

    void ProcessAfterRun(size_t localMemUsed);

    void PrintStatistics() const;

    auto AddInlinedMethods(size_t num)
    {
        inlinedMethods_ += num;
    }
    auto GetInlinedMethods() const
    {
        return inlinedMethods_;
    }

    auto AddPbcInstNum(uint64_t num)
    {
        pbcInstNum_ += num;
    }
    auto SetPbcInstNum(uint64_t num)
    {
        pbcInstNum_ = num;
    }
    auto GetPbcInstNum() const
    {
        return pbcInstNum_;
    }

    auto GetCurrentPassIndex() const
    {
        return passRunIndex_;
    }

    auto GetCurrentPassCallDepth() const
    {
        return passCallDepth_;
    }

    void DumpStatisticsCsv(char sep = ',') const;

private:
    struct GraphStatistic {
        size_t numOfInstructions {0};
        size_t numOfBasicblocks {0};
    };
    struct PassStatistic {
        int runDepth {0};
        const char *passName {nullptr};
        GraphStatistic beforePass;
        GraphStatistic afterPass;
        size_t memUsedIr {0};
        size_t memUsedLocal {0};
        size_t timeUs {0};
    };

private:
    Graph *graph_;
    // We use list because the elements inside this container are referred by pointers in other places.
    ArenaList<PassStatistic> passStatList_;
    ArenaStack<PassStatistic *> passStatStack_;
    size_t lastAllocatedLocal_ {0};
    size_t lastAllocatedIr_ {0};
    std::chrono::time_point<std::chrono::steady_clock> lastTimestamp_;

    unsigned passRunIndex_ {0};
    int passCallDepth_ {0};

    // Count of inlined methods
    size_t inlinedMethods_ {0};
    // Number of pbc instructions in main and all successfully inlined methods.
    uint64_t pbcInstNum_ {0};

    bool enableIrStat_ {false};
};
}  // namespace ark::compiler

#endif  // PANDA_PASS_STATISTICS_H
