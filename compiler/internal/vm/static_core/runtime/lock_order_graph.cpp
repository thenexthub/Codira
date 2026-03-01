/**
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "runtime/include/panda_vm.h"

#include "runtime/lock_order_graph.h"

namespace ark {
void UpdateMonitorsForThread(PandaMap<ManagedThread::ThreadId, Monitor::MonitorId> &enteringMonitors,
                             PandaMap<Monitor::MonitorId, PandaSet<ManagedThread::ThreadId>> &enteredMonitors,
                             MTManagedThread *thread)
{
    auto threadId = thread->GetId();
    auto enteringMonitor = thread->GetEnteringMonitor();
    if (enteringMonitor != nullptr) {
        enteringMonitors[threadId] = enteringMonitor->GetId();
    }
    for (auto enteredMonitorId : thread->GetVM()->GetMonitorPool()->GetEnteredMonitorsIds(thread)) {
        enteredMonitors[enteredMonitorId].insert(threadId);
    }
}

bool LockOrderGraph::CheckForTerminationLoops(const PandaList<MTManagedThread *> &threads,
                                              const PandaList<MTManagedThread *> &daemonThreads,
                                              MTManagedThread *current)
{
    PandaMap<ThreadId, bool> nodes;
    PandaMap<ThreadId, ThreadId> edges;
    PandaMap<ThreadId, MonitorId> enteringMonitors;
    PandaMap<MonitorId, PandaSet<ThreadId>> enteredMonitors;
    for (auto thread : threads) {
        if (thread == current) {
            continue;
        }

        auto threadId = thread->GetId();
        auto status = thread->GetStatus();
        if (status == ThreadStatus::NATIVE) {
            nodes[threadId] = true;
        } else {
            if (status != ThreadStatus::IS_BLOCKED) {
                LOG(DEBUG, RUNTIME) << "Thread " << threadId << " has changed its status during graph construction";
                return false;
            }
            nodes[threadId] = false;
        }
        LOG(DEBUG, RUNTIME) << "LockOrderGraph node: " << threadId << ", is NATIVE = " << nodes[threadId];
        UpdateMonitorsForThread(enteringMonitors, enteredMonitors, thread);
    }
    for (auto thread : daemonThreads) {
        auto threadId = thread->GetId();
        nodes[threadId] = true;
        LOG(DEBUG, RUNTIME) << "LockOrderGraph node: " << threadId << ", in termination loop";
        UpdateMonitorsForThread(enteringMonitors, enteredMonitors, thread);
    }

    for (const auto &[from_thread_id, entering_monitor_id] : enteringMonitors) {
        for (const auto toThreadId : enteredMonitors[entering_monitor_id]) {
            // We can only wait for a single monitor here.
            if (edges.count(from_thread_id) != 0) {
                LOG(DEBUG, RUNTIME) << "Graph has been changed during its construction. Previous edge "
                                    << from_thread_id << " -> " << edges[from_thread_id]
                                    << " cannot be overwritten with " << from_thread_id << " -> " << toThreadId;
                return false;
            }
            edges[from_thread_id] = toThreadId;
            LOG(DEBUG, RUNTIME) << "LockOrderGraph edge: " << from_thread_id << " -> " << toThreadId;
        }
    }
    return LockOrderGraph(nodes, edges).CheckForTerminationLoops();
}

void LockOrderGraph::CheckNodeForTerminationLoops(ThreadId node, PandaSet<ThreadId> &nodesInDeadlocks) const
{
    if (nodesInDeadlocks.count(node) != 0) {
        // If this node belongs to some previously found loop, we ignore it.
        return;
    }
    if (nodes_.at(node)) {
        // This node is terminating, ignore it.
        nodesInDeadlocks.insert(node);
        return;
    }

    // explored_nodes contains nodes reachable from the node chosen in the outer loop.
    PandaSet<ThreadId> exploredNodes = {node};
    // front contains nodes which have not been explored yet.
    PandaList<ThreadId> front = {node};
    // On each iteration of the loop we take next unexplored node from the front and find all reachable nodes from
    // it. If we find already explored node then there is a loop and we save it in nodes_in_deadlocks. Also we
    // detect paths leading to nodes_in_deadlocks and to termination nodes.
    while (!front.empty()) {
        auto i = front.begin();
        while (i != front.end()) {
            ThreadId currentNode = *i;
            i = front.erase(i);
            if (edges_.count(currentNode) == 0) {
                // No transitions from this node.
                continue;
            }
            auto nextNode = edges_.at(currentNode);
            // There is a rare case, in which a monitor may be entered recursively in a
            // daemon thread. If a runtime calls DeregisterSuspendedThreads exactly when
            // the daemon thread sets SetEnteringMonitor, then we create an edge from a thread
            // to itself, i.e. a self-loop and, thus, falsely flag this situation as a deadlock.
            // So here we ignore this self-loop as a false loop.
            if (nextNode == currentNode) {
                continue;
            }
            if (exploredNodes.count(nextNode) != 0 || nodesInDeadlocks.count(nextNode) != 0 || nodes_.at(nextNode)) {
                // Loop or path to another loop or to terminating node was found
                nodesInDeadlocks.merge(exploredNodes);
                front.clear();
                break;
            }
            exploredNodes.insert(nextNode);
            front.push_back(nextNode);
        }
    }
}

bool LockOrderGraph::CheckForTerminationLoops() const
{
    // This function returns true, if the following conditions are satisfied for each node:
    // the node belongs to a loop (i.e., there is a deadlock with corresponding threads), or
    // this is a terminating node (thread with NATIVE status), or
    // there is a path to a loop or to a terminating node.
    PandaSet<ThreadId> nodesInDeadlocks = {};
    for (auto const nodeElem : nodes_) {
        CheckNodeForTerminationLoops(nodeElem.first, nodesInDeadlocks);
    }
    return nodesInDeadlocks.size() == nodes_.size();
}
}  // namespace ark
