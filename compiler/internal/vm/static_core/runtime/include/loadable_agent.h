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

#ifndef PANDA_RUNTIME_INCLUDE_LOADABLE_AGENT_H
#define PANDA_RUNTIME_INCLUDE_LOADABLE_AGENT_H

#include "libarkbase/os/library_loader.h"
#include "libarkbase/os/mutex.h"
#include "runtime/include/mem/panda_smart_pointers.h"
#include "runtime/include/mem/panda_string.h"

namespace ark {
class LoadableAgent {
public:
    LoadableAgent() = default;
    virtual ~LoadableAgent() = default;

    virtual bool Load() = 0;
    virtual bool Unload() = 0;

private:
    NO_COPY_SEMANTIC(LoadableAgent);
    NO_MOVE_SEMANTIC(LoadableAgent);
};

using LoadableAgentHandle = std::shared_ptr<LoadableAgent>;

class LibraryAgent : public LoadableAgent {
public:
    PANDA_PUBLIC_API LibraryAgent(os::memory::Mutex &mutex, PandaString libraryPath, PandaString loadCallbackName,
                                  PandaString unloadCallbackName);

    PANDA_PUBLIC_API bool Load() override;
    PANDA_PUBLIC_API bool Unload() override;

private:
    virtual bool CallLoadCallback(void *resolvedFunction) = 0;
    virtual bool CallUnloadCallback(void *resolvedFunction) = 0;

    os::memory::LockHolder<os::memory::Mutex> lock_;

    PandaString libraryPath_;
    PandaString loadCallbackName_;
    PandaString unloadCallbackName_;

    os::library_loader::LibraryHandle handle_ {nullptr};
    void *unloadCallback_ {};
};

template <typename AgentType, bool REUSABLE>
class LibraryAgentLoader {
public:
    template <typename... Args>
    static LoadableAgentHandle LoadInstance(Args &&...args)
    {
        // This mutex is needed to be sure that getting / creation of an instance is made atomically
        // (e.g. there won't be two threads that found no instance and tried to create a new one).
        static os::memory::Mutex creationMutex;
        os::memory::LockHolder creationMutexLock(creationMutex);

        static std::weak_ptr<LoadableAgent> instance;

        auto inst = instance.lock();
        if (inst) {
            if constexpr (REUSABLE) {  // NOLINT(readability-braces-around-statements)
                return inst;
            } else {  // NOLINT(readability-misleading-indentation)
                LOG(ERROR, RUNTIME) << "Non-reusable library is already used";
                return {};
            }
        }

        // This mutex is needed to be sure that there is only one library at a given point of time
        // (e.g. a new instance initialization won't be performed earlier than deinitialization of the old one).
        static os::memory::Mutex uniquenessMutex;

        auto lib = MakePandaUnique<AgentType>(uniquenessMutex, std::forward<Args>(args)...);
        ASSERT(lib != nullptr);
        if (!lib->Load()) {
            LOG(ERROR, RUNTIME) << "Could not load library";
            return {};
        }

        inst = std::shared_ptr<AgentType>(lib.release(), [](AgentType *ptr) {
            ptr->Unload();
            mem::InternalAllocator<>::GetInternalAllocatorFromRuntime()->Delete(ptr);
        });

        instance = inst;

        return inst;
    }
};
}  // namespace ark

#endif  // PANDA_RUNTIME_INCLUDE_LOADABLE_AGENT_H
