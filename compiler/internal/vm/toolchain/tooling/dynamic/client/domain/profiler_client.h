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

#ifndef ECMASCRIPT_TOOLING_CLIENT_DOMAIN_PROFILER_CLIENT_H
#define ECMASCRIPT_TOOLING_CLIENT_DOMAIN_PROFILER_CLIENT_H

#include <iostream>
#include <map>

#include <vector>

#include "tooling/dynamic/base/pt_json.h"

namespace OHOS::ArkCompiler::Toolchain {
using PtJson = panda::ecmascript::tooling::PtJson;
class ProfilerSingleton {
public:
    ProfilerSingleton() = default;

    void AddCpuName(const std::string &data)
    {
        cpulist_.emplace_back(data);
    }

    void ShowCpuFile()
    {
        size_t size = cpulist_.size();
        for (size_t i = 0; i < size; i++) {
            std::cout << cpulist_[i] << std::endl;
        }
    }

    void SetAddress(std::string address)
    {
        address_ = address;
    }

    const std::string& GetAddress() const
    {
        return address_;
    }

private:
    std::vector<std::string> cpulist_;
    std::string address_ = "/data/";
    ProfilerSingleton(const ProfilerSingleton&) = delete;
    ProfilerSingleton& operator=(const ProfilerSingleton&) = delete;
};

class ProfilerClient final {
public:
    ProfilerClient(uint32_t sessionId) : sessionId_(sessionId) {}
    ~ProfilerClient() = default;

    bool DispatcherCmd(const std::string &cmd);
    int CpuprofileCommand();
    int CpuprofileStopCommand();
    int SetSamplingIntervalCommand();
    int CpuprofileEnableCommand();
    int CpuprofileDisableCommand();
    bool WriteCpuProfileForFile(const std::string &fileName, const std::string &data);
    void RecvProfilerResult(std::unique_ptr<PtJson> json);
    void SetSamplingInterval(int interval);

private:
    int32_t interval_ = 0;
    int32_t sessionId_;
    std::map<uint32_t, std::string> idEventMap_ {};
};
} // OHOS::ArkCompiler::Toolchain
#endif
