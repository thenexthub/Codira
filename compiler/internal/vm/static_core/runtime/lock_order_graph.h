/**
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
#ifndef PANDA_RUNTIME_LOCK_ORDER_GRAPH_H_
#define PANDA_RUNTIME_LOCK_ORDER_GRAPH_H_

#include <bitset>

#include "libarkbase/os/mutex.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/thread.h"
#include "runtime/include/thread_status.h"

namespace ark::mem::test {
class LockOrderGraphTest;
}  // namespace ark::mem::test

namespace ark {
// This class is required to find a loop in lock order graph to detect termination loops.
class LockOrderGraph {
public:
    using ThreadId = ManagedThread::ThreadId;
    using MonitorId = Monitor::MonitorId;
    static bool CheckForTerminationLoops(const PandaList<MTManagedThread *> &threads,
                                         const PandaList<MTManagedThread *> &daemonThreads, MTManagedThread *current);

    bool CheckForTerminationLoops() const;

    DEFAULT_MOVE_SEMANTIC(LockOrderGraph);
    DEFAULT_COPY_SEMANTIC(LockOrderGraph);

    ~LockOrderGraph() = default;

private:
    LockOrderGraph(PandaMap<ThreadId, bool> nodes, PandaMap<ThreadId, ThreadId> edges)
    {
        nodes_ = std::move(nodes);
        edges_ = std::move(edges);
        for (auto const &edge : edges_) {
            if (nodes_[edge.second]) {
                LOG(DEBUG, RUNTIME) << "Edge in LockOrderGraph: " << edge.first << " -> " << edge.second << " [T]";
            } else {
                LOG(DEBUG, RUNTIME) << "Edge in LockOrderGraph: " << edge.first << " -> " << edge.second;
            }
        }
    }
    PandaMap<ThreadId, bool> nodes_;
    PandaMap<ThreadId, ThreadId> edges_;

    void CheckNodeForTerminationLoops(ThreadId node, PandaSet<ThreadId> &nodesInDeadlocks) const;

    friend class ark::mem::test::LockOrderGraphTest;
};
}  // namespace ark

#endif  // PANDA_RUNTIME_THREAD_MANAGER_H_
