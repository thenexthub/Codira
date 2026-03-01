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

#ifndef ECMASCRIPT_TOOLING_CLIENT_MANAGER_DOMAIN_MANAGER_H
#define ECMASCRIPT_TOOLING_CLIENT_MANAGER_DOMAIN_MANAGER_H

#include <iostream>

#include "tooling/dynamic/client/domain/debugger_client.h"
#include "tooling/dynamic/client/domain/heapprofiler_client.h"
#include "tooling/dynamic/client/domain/profiler_client.h"
#include "tooling/dynamic/client/domain/runtime_client.h"
#include "tooling/dynamic/client/domain/test_client.h"

namespace OHOS::ArkCompiler::Toolchain {
class DomainManager {
public:
    explicit DomainManager(uint32_t sessionId);
    ~DomainManager() = default;

    void DispatcherReply(char* msg);

    std::string GetDomainById(uint32_t id)
    {
        auto iter = idDomainMap_.find(id);
        if (iter == idDomainMap_.end()) {
            return "";
        }
        return iter->second;
    }

    void SetDomainById(uint32_t id, std::string domain)
    {
        idDomainMap_.emplace(id, domain);
    }

    void RemoveDomainById(uint32_t id)
    {
        auto it = idDomainMap_.find(id);
        if (it != idDomainMap_.end()) {
            idDomainMap_.erase(it);
        }
    }

    HeapProfilerClient &GetHeapProfilerClient()
    {
        return heapProfilerClient_;
    }

    ProfilerClient &GetProfilerClient()
    {
        return profilerClient_;
    }

    DebuggerClient &GetDebuggerClient()
    {
        return debuggerClient_;
    }

    RuntimeClient &GetRuntimeClient()
    {
        return runtimeClient_;
    }

    TestClient &GetTestClient()
    {
        return testClient_;
    }

private:
    [[maybe_unused]] uint32_t sessionId_;
    HeapProfilerClient heapProfilerClient_;
    ProfilerClient profilerClient_;
    DebuggerClient debuggerClient_;
    RuntimeClient runtimeClient_;
    TestClient testClient_;
    std::map<uint32_t, std::string> idDomainMap_ {};
};
} // OHOS::ArkCompiler::Toolchain
#endif