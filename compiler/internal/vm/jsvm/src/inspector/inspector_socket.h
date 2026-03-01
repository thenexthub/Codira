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

#ifndef SRC_INSPECTOR_SOCKET_H_
#define SRC_INSPECTOR_SOCKET_H_

#include <string>
#include <vector>

#include "inspector_utils.h"
#include "uv.h"

namespace jsvm {
namespace inspector {

class ProtocolHandler;

// HTTP Wrapper around a uv_tcp_t
class InspectorSocket {
private:
    InspectorSocket() = default;
public:
    using Pointer = std::unique_ptr<InspectorSocket>;

    InspectorSocket(const InspectorSocket&) = delete;

    InspectorSocket& operator=(const InspectorSocket&) = delete;

    ~InspectorSocket();

    class Delegate {
    public:
        virtual void OnWsFrame(const std::vector<char>& frame) = 0;
        virtual ~Delegate() = default;
        virtual void OnHttpGet(const std::string& host, const std::string& path) = 0;
        virtual void OnSocketUpgrade(const std::string& host,
                                     const std::string& path,
                                     const std::string& acceptKey) = 0;
    };

    using DelegatePointer = std::unique_ptr<Delegate>;

    static Pointer Accept(uv_stream_t* server, DelegatePointer delegate);

    void AcceptUpgrade(const std::string& acceptKey);

    void SwitchProtocol(ProtocolHandler* handler);

    void Write(const char* data, size_t len);

    void CancelHandshake();

    std::string GetHost();

private:
    static void Shutdown(ProtocolHandler* handler);

    DeleteFnPtr<ProtocolHandler, Shutdown> protocolHandler;
};

} // namespace inspector
} // namespace jsvm

#endif // SRC_INSPECTOR_SOCKET_H_
