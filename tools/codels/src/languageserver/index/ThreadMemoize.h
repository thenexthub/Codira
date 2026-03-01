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

#ifndef LSPSERVER_INDEX_THREAD_MEMOIZE_H
#define LSPSERVER_INDEX_THREAD_MEMOIZE_H

#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <thread>

#include "../logger/Logger.h"

namespace ark {
namespace lsp {
const unsigned int EXTRA_THREAD_COUNT = 3; // main thread and message thread
const unsigned int HARDWARE_CONCURRENCY_COUNT = std::thread::hardware_concurrency();
const unsigned int MAX_THREAD_COUNT = HARDWARE_CONCURRENCY_COUNT > EXTRA_THREAD_COUNT ?
                                      HARDWARE_CONCURRENCY_COUNT - EXTRA_THREAD_COUNT : 1;

bool ShutdownRequested();

/**
 * Checks whether notifyShutdown() or requestShutdown() was called.
 * This function is threadsafe and signal-safe.
 */
bool ShutdownPending();

/**
 * Variation of Memoize which maintains a set of thread local caches.
 */
template <typename container> class ThreadMemoize {
public:
    ThreadMemoize() : capacity(MAX_THREAD_COUNT) {}

    template <typename F> auto &Get(F &&compute)
    {
        auto tid = GetThreadID();
        std::lock_guard<std::mutex> lock(threadsMu);
        return Put(tid, std::forward<F>(compute));
    }

    void EraseThreadCache()
    {
        auto tid = GetThreadID();
        std::lock_guard<std::mutex> lock(threadsMu);
        auto it = threadsCache.find(tid);
        if (it == threadsCache.end()) {
            return;
        }
        threadList.erase(it->second);
        threadsCache.erase(it);
    }

private:
    static std::thread::id GetThreadID()
    {
        static thread_local auto threadID = std::this_thread::get_id();
        return threadID;
    }

    template <typename F>
    container& Put(const std::thread::id& tid, F&& compute)
    {
        auto it = threadsCache.find(tid);
        if (it != threadsCache.end()) {
            threadList.splice(threadList.begin(), threadList, it->second);
            return it->second->second;
        }

        if (threadList.size() >= capacity) {
            auto last = threadList.end();
            --last;
            threadsCache.erase(last->first);
            threadList.pop_back();
        }

        threadList.emplace_front(tid, compute());
        threadsCache[tid] = threadList.begin();
        return threadList.begin()->second;
    }

    size_t capacity;

    std::mutex threadsMu;
    std::list<std::pair<std::thread::id, container>> threadList;
    std::unordered_map<std::thread::id,
        typename std::list<std::pair<std::thread::id, container>>::iterator> threadsCache;
};

} // namespace lsp
} // namespace ark
#endif // LSPSERVER_INDEX_THREAD_MEMOIZE_H
