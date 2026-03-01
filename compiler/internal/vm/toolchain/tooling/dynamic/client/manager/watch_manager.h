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
#ifndef ECMASCRIPT_TOOLING_CLIENT_MANAGER_WATCH_MANAGER_H
#define ECMASCRIPT_TOOLING_CLIENT_MANAGER_WATCH_MANAGER_H

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "tooling/dynamic/base/pt_json.h"
#include "tooling/dynamic/base/pt_types.h"
#include "tooling/dynamic/client/domain/runtime_client.h"

namespace OHOS::ArkCompiler::Toolchain {
using PtJson = panda::ecmascript::tooling::PtJson;
using Result = panda::ecmascript::tooling::Result;
class WatchManager {
public:
    explicit WatchManager(uint32_t sessionId);
    ~WatchManager() = default;

    void SendRequestWatch(const int32_t &watchInfoIndex, const std::string &callFrameId);
    void GetPropertiesCommand(const int32_t &watchInfoIndex, const std::string &objectId);
    void RequestWatchInfo(const std::unique_ptr<PtJson> &json);
    void AddWatchInfo(const std::string& watchInfo);

    bool GetDebugState();
    void DebugFalseState();
    void DebugTrueState();
    bool HandleWatchResult(const std::unique_ptr<PtJson> &json, int32_t id);
    bool ShowWatchResult(const std::unique_ptr<PtJson> &result, int32_t id);
    void OutputWatchResult(const std::unique_ptr<PtJson> &watchResult);
    bool ShowWatchResult2(const int &id, const std::unique_ptr<PtJson> &result);
    std::string GetCallFrameId();
    int GetWatchInfoSize();
    void SetWatchInfoMap(const int &id, const int &index);

private:
    [[maybe_unused]] int32_t sessionId_;
    std::vector<std::string> watchInfoList_;
    std::unordered_map<int, int> watchInfoMap_;
    bool IsDebug_ = false;
    std::string callFrameId_;
    RuntimeClient runtimeClient_;
    int inputRowFlag_ = 0;
    bool isShowWatchInfo_ = true;
};
} // OHOS::ArkCompiler::Toolchain
#endif