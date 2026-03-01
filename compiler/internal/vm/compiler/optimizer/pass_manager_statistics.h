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

#ifndef COMPILER_OPTIMIZER_PASS_MANAGER_STATISTICS_H
#define COMPILER_OPTIMIZER_PASS_MANAGER_STATISTICS_H

#include <chrono>
#include "utils/arena_containers.h"

namespace panda::compiler {
class Graph;
class Pass;

class PassManagerStatistics {
public:
    explicit PassManagerStatistics(Graph *graph);

    NO_MOVE_SEMANTIC(PassManagerStatistics);
    NO_COPY_SEMANTIC(PassManagerStatistics);
    ~PassManagerStatistics() = default;

    void ProcessBeforeRun(const Pass &pass);

    void ProcessAfterRun(size_t local_mem_used);

    void PrintStatistics() const;

    auto AddInlinedMethods(size_t num)
    {
        inlined_methods_ += num;
    }
    auto GetInlinedMethods() const
    {
        return inlined_methods_;
    }

    auto AddPbcInstNum(uint64_t num)
    {
        pbc_inst_num_ += num;
    }
    auto SetPbcInstNum(uint64_t num)
    {
        pbc_inst_num_ = num;
    }
    auto GetPbcInstNum() const
    {
        return pbc_inst_num_;
    }

    auto GetCurrentPassIndex() const
    {
        return pass_run_index_;
    }

    void DumpStatisticsCsv(char sep = ',') const;

private:
    struct GraphStatistic {
        size_t num_of_instructions {0};
        size_t num_of_basicblocks {0};
    };
    struct PassStatistic {
        int run_depth {0};
        const char *pass_name {nullptr};
        GraphStatistic before_pass;
        GraphStatistic after_pass;
        size_t mem_used_ir {0};
        size_t mem_used_local {0};
        size_t time_us {0};
    };

private:
    Graph *graph_;
    // We use list because the elements inside this container are referred by pointers in other places.
    ArenaList<PassStatistic> pass_stat_list_;
    ArenaStack<PassStatistic *> pass_stat_stack_;
    size_t last_allocated_local_ {0};
    size_t last_allocated_ir_ {0};
    std::chrono::time_point<std::chrono::steady_clock> last_timestamp_;

    unsigned pass_run_index_ {0};
    int pass_call_depth_ {0};

    // Count of inlined methods
    size_t inlined_methods_ {0};
    // Number of pbc instructions in main and all successfully inlined methods.
    uint64_t pbc_inst_num_ {0};

    bool enable_ir_stat_ {false};
};
}  // namespace panda::compiler

#endif  // COMPILER_OPTIMIZER_PASS_MANAGER_STATISTICS_H
