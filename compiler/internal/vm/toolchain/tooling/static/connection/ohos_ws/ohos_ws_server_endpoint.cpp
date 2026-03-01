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

#include "connection/ohos_ws/ohos_ws_server_endpoint.h"

namespace ark::tooling::inspector {
OhosWsServerEndpoint::OhosWsServerEndpoint() noexcept
{
    if (endpoint_ != nullptr) {
        endpoint_->SetValidateConnectionCallback([this](auto) {
            // a new connection will be accepted only after the prior is finished,
            // which is ensured by websocket implementation
            onValidate_();
            return true;
        });

        endpoint_->SetOpenConnectionCallback([this] { onOpen_(); });
        endpoint_->SetFailConnectionCallback([this] { onFail_(); });
    }
}
}  // namespace ark::tooling::inspector
