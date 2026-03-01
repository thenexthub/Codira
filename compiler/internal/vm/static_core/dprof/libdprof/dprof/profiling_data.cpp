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

#include "profiling_data.h"
#include "libarkbase/utils/logger.h"
#include "ipc/ipc_message.h"
#include "ipc/ipc_unix_socket.h"
#include "ipc/ipc_message_protocol.h"
#include "libarkbase/serializer/serializer.h"

namespace ark::dprof {
bool ProfilingData::SetFeatureDate(const std::string &featureName, std::vector<uint8_t> &&data)
{
    auto it = featuresDataMap_.find(featureName);
    if (it != featuresDataMap_.end()) {
        LOG(ERROR, DPROF) << "Feature already exists, featureName=" << featureName;
        return false;
    }

    featuresDataMap_.emplace(std::pair(featureName, std::move(data)));
    return false;
}

bool ProfilingData::DumpAndResetFeatures()
{
    os::unique_fd::UniqueFd sock(ipc::CreateUnixClientSocket());
    if (!sock.IsValid()) {
        LOG(ERROR, DPROF) << "Cannot create client socket";
        return false;
    }

    ipc::protocol::Version tmp {ipc::protocol::VERSION};

    std::vector<uint8_t> versionData;
    serializer::StructToBuffer<ipc::protocol::VERSION_FCOUNT>(tmp, versionData);

    ipc::Message msgVersion(ipc::Message::Id::VERSION, std::move(versionData));
    if (!SendMessage(sock.Get(), msgVersion)) {
        LOG(ERROR, DPROF) << "Cannot send version";
        return false;
    }

    ipc::protocol::AppInfo tmp2 {appName_, hash_, pid_};
    std::vector<uint8_t> appInfoData;
    serializer::StructToBuffer<ipc::protocol::APP_INFO_FCOUNT>(tmp2, appInfoData);

    ipc::Message msgAppInfo(ipc::Message::Id::APP_INFO, std::move(appInfoData));
    if (!SendMessage(sock.Get(), msgAppInfo)) {
        LOG(ERROR, DPROF) << "Cannot send app info";
        return false;
    }

    // Send features data
    for (auto &kv : featuresDataMap_) {
        ipc::protocol::FeatureData tmpData;
        tmpData.name = kv.first;
        tmpData.data = std::move(kv.second);

        std::vector<uint8_t> featureData;
        serializer::StructToBuffer<ipc::protocol::FEATURE_DATA_FCOUNT>(tmpData, featureData);

        ipc::Message msgFeatureData(ipc::Message::Id::FEATURE_DATA, std::move(featureData));
        if (!SendMessage(sock.Get(), msgFeatureData)) {
            LOG(ERROR, DPROF) << "Cannot send feature data, featureName=" << tmpData.name;
            return false;
        }
    }

    featuresDataMap_.clear();
    return true;
}
}  // namespace ark::dprof
