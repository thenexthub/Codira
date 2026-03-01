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

/**
 * @file
 *
 * This file implements the Job Class.
 */

#include "Job.h"

#include "Codira/Basic/Print.h"
#include "Codira/Driver/Tool.h"
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
#include "Codira/Driver/Backend/CODENATIVEBackend.h"
#endif
#include "Codira/Driver/TempFileManager.h"
#include "Codira/Driver/Toolchains/ToolChain.h"
#include "Codira/Utils/FileUtil.h"
#include "Codira/Utils/ProfileRecorder.h"
#include "Codira/Utils/Semaphore.h"

namespace {
bool CheckExecuteResult(std::map<std::string, std::unique_ptr<ToolFuture>>& checklist,
    bool returnIfAnyToolFinished = false)
{
    auto printError = [](std::string cmd) {
        if (!TempFileManager::Instance().IsDeleted()) {
            Errorln(cmd, ": command failed (use -V to see invocation)");
        }
    };
    bool success = true;
    while (!checklist.empty()) {
        size_t totalTasks = checklist.size();
        for (auto it = checklist.cbegin(); it != checklist.cend();) {
            auto state = it->second->GetState();
            if (state == ToolFuture::State::FAILED) {
                Utils::Semaphore::Get().Release();
                printError(it->first);
                checklist.erase(it++);
                success = false;
            } else if (state == ToolFuture::State::SUCCESS) {
                Utils::Semaphore::Get().Release();
                checklist.erase(it++);
            } else {
                ++it;
            }
        }
        if (returnIfAnyToolFinished && totalTasks != checklist.size()) {
            // Some tasks are finished and removed from the list.
            return success;
        }
        std::this_thread::sleep_for(std::chrono::microseconds(200)); // Check running tasks every 200 ms.
    }
    return success;
}
} // namespace

using namespace Codira;

bool Job::Assemble(const DriverOptions& driverOptions, const Driver& driver)
{
    switch (driverOptions.backend) {
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
        case Triple::BackendType::CODENATIVE:
            backend = std::make_unique<CODENATIVEBackend>(*this, driverOptions, driver);
            break;
#endif
        case Triple::BackendType::UNKNOWN:
        default:
            Errorln("Toolchain: Unsupported backend");
            return false;
    }

    if (!backend->Generate()) {
        return false;
    }

    verbose = driverOptions.enableVerbose;

    return true;
}

bool Job::Execute() const
{
    const std::vector<ToolBatch>& commandList = backend->GetBackendCmds();
    for (const ToolBatch& cmdBatch : commandList) {
        if (cmdBatch.empty()) {
            continue;
        }
        std::map<std::string, std::unique_ptr<ToolFuture>> childWorkers{};
        Utils::ProfileRecorder recorder("Main Stage", "Execute " + FileUtil::GetFileName(cmdBatch[0]->GetName()), "");
        for (auto& cmd : cmdBatch) {
            // NOTE: `CheckExecuteResult` acquires semaphore without condition. We must ensure that there is still
            // available slot in semaphore before executing next command. If there is no more slot available, wait
            // any of previous created threads finish.
            while (Utils::Semaphore::Get().GetCount() == 0) {
                if (!CheckExecuteResult(childWorkers, true)) {
                    return false;
                }
            }
            auto future = cmd->Execute(verbose);
            if (!future) {
                return false;
            }
            childWorkers.emplace(cmd->GetCommandString(), std::move(future));
        }
        if (!CheckExecuteResult(childWorkers)) {
            return false;
        }
    }
    return true;
}
