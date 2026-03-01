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

#ifndef SRC_JSVM_HOST_PORT_H_
#define SRC_JSVM_HOST_PORT_H_

#include "jsvm_dfx.h"
#include "jsvm_mutex.h"
#include "jsvm_util.h"

namespace jsvm {

class HostPort {
public:
    HostPort(const std::string& hostName, int port, int pid = -1) : hostName(hostName), port(port), pid(pid) {}
    HostPort(const HostPort&) = default;
    HostPort& operator=(const HostPort&) = default;
    HostPort(HostPort&&) = default;
    HostPort& operator=(HostPort&&) = default;

    void SetHost(const std::string& host)
    {
        hostName = host;
    }

    void SetPort(int portParam)
    {
        port = portParam;
    }

    const std::string& GetHost() const
    {
        return hostName;
    }

    int GetPort() const
    {
        CHECK_GE(port, 0);
        return port;
    }

    int GetPid() const
    {
        return pid;
    }

    void Update(const HostPort& other)
    {
        if (!other.hostName.empty()) {
            hostName = other.hostName;
        }
        if (other.port >= 0) {
            port = other.port;
        }
    }

private:
    std::string hostName;
    int port;
    int pid;
};

struct InspectPublishUid {
    bool console;
    bool http;
};

} // namespace jsvm

#endif // SRC_JSVM_HOST_PORT_H_
