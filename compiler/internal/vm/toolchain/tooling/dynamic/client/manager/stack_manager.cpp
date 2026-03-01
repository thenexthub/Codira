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

#include "tooling/dynamic/client/manager/stack_manager.h"

namespace OHOS::ArkCompiler::Toolchain {
void StackManager::SetCallFrames(std::map<int32_t, std::unique_ptr<CallFrame>> callFrames)
{
    for (auto &callFrame : callFrames) {
        callFrames_[callFrame.first] = std::move(callFrame.second);
    }
}

void StackManager::ShowCallFrames()
{
    std::cout << std::endl;
    for (const auto &callFrame : callFrames_) {
        if (callFrame.second->GetFunctionName().empty()) {
            callFrame.second->SetFunctionName("<anonymous function>");
        }
        std::cout << callFrame.first << ". " << callFrame.second->GetFunctionName() << "(), "
                  << callFrame.second->GetUrl() << ": " << callFrame.second->GetLocation()->GetLine() << std::endl;
    }
}

std::map<int32_t, std::map<int32_t, std::string>> StackManager::GetScopeChainInfo()
{
    std::map<int32_t, std::map<int32_t, std::string>> scopeInfos;
    for (const auto &callFram : callFrames_) {
        int32_t callFramId = callFram.second->GetCallFrameId();
        for (const auto &scope : *(callFram.second->GetScopeChain())) {
            scopeInfos[callFramId][scope->GetObject()->GetObjectId()] = scope->GetType();
        }
    }
    return scopeInfos;
}

void StackManager::ClearCallFrame()
{
    callFrames_.clear();
}

void StackManager::PrintScopeChainInfo(const std::map<int32_t, std::map<int32_t, std::string>>& scopeInfos)
{
    for (const auto& [callFrameId, scopes] : scopeInfos) {
        std::cout << "CallFrame ID: " << callFrameId << std::endl;
        for (const auto& [objectId, type] : scopes) {
            std::cout << "  Object ID: " << objectId << ", Type: " << type << std::endl;
        }
        std::cout << "-----------------------" << std::endl;
    }
}
}