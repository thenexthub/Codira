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
#ifndef ECMASCRIPT_TOOLING_CLIENT_MANAGER_SOURCE_MANAGER_H
#define ECMASCRIPT_TOOLING_CLIENT_MANAGER_SOURCE_MANAGER_H

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "tooling/dynamic/base/pt_json.h"
#include "tooling/dynamic/base/pt_types.h"

namespace OHOS::ArkCompiler::Toolchain {
using PtJson = panda::ecmascript::tooling::PtJson;
using Result = panda::ecmascript::tooling::Result;
class SourceManager {
public:
    SourceManager(int32_t sessionId) : sessionId_(sessionId) {}

    void SendRequeSource(int scriptId);
    void EnableReply(const std::unique_ptr<PtJson> json);
    void GetFileName();
    void SetFileName(int scriptId, const std::string& fileName);
    void SetFileSource(int scriptIdIndex, const std::string& fileSource);
    std::vector<std::string> GetFileSource(int scriptId);
    void GetDebugSources(const std::unique_ptr<PtJson> json);
    void ListSourceCodeWithParameters(int startLine, int endLine);
    void ListSource(int startLine, int endLine);
    void GetListSource(std::string startLine, std::string endLine);

private:
    [[maybe_unused]] int32_t sessionId_;
    int32_t scriptId_ = 0;
    int32_t debugLineNum_ = 0;
    std::unordered_map<int, int> scriptIdMap_ {};
    std::unordered_map<int, std::pair<std::string, std::vector<std::string>>> fileSource_ {};
    SourceManager(const SourceManager&) = delete;
    SourceManager& operator=(const SourceManager&) = delete;
};
} // OHOS::ArkCompiler::Toolchain
#endif