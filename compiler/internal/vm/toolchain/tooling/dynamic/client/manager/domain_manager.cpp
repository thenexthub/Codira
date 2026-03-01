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

#include "domain_manager.h"

#include "tooling/dynamic/client/session/session.h"

using PtJson = panda::ecmascript::tooling::PtJson;
using Result = panda::ecmascript::tooling::Result;
namespace OHOS::ArkCompiler::Toolchain {
DomainManager::DomainManager(uint32_t sessionId)
    : sessionId_(sessionId), heapProfilerClient_(sessionId), profilerClient_(sessionId),
      debuggerClient_(sessionId), runtimeClient_(sessionId), testClient_(sessionId)
{
}

void DomainManager::DispatcherReply(char* msg)
{
    std::string decMessage = std::string(msg);
    std::unique_ptr<PtJson> json = PtJson::Parse(decMessage);
    if (json == nullptr) {
        LOGE("json parse error");
        return;
    }

    if (!json->IsObject()) {
        LOGE("json parse format error");
        json->ReleaseRoot();
        return;
    }

    std::string domain;
    Result ret;
    int32_t id;
    ret = json->GetInt("id", &id);
    if (ret == Result::SUCCESS) {
        domain = GetDomainById(id);
        RemoveDomainById(id);
    }

    std::string wholeMethod;
    ret = json->GetString("method", &wholeMethod);
    if (ret == Result::SUCCESS) {
        std::string::size_type length = wholeMethod.length();
        if (length == 0) {
            return;
        }
        std::string::size_type indexPoint = 0;
        indexPoint = wholeMethod.find_first_of('.', 0);
        if (indexPoint == std::string::npos || indexPoint == 0 || indexPoint == length - 1) {
            return;
        }
        domain = wholeMethod.substr(0, indexPoint);
    }

    if (domain == "HeapProfiler") {
        heapProfilerClient_.RecvReply(std::move(json));
    } else if (domain == "Profiler") {
        profilerClient_.RecvProfilerResult(std::move(json));
    } else if (domain == "Runtime") {
        runtimeClient_.RecvReply(std::move(json));
    } else if (domain == "Debugger") {
        debuggerClient_.RecvReply(std::move(json));
    }
}
}