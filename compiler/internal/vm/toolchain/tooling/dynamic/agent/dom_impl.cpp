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

#include "agent/dom_impl.h"

namespace panda::ecmascript::tooling {
std::optional<std::string> DomImpl::DispatcherImpl::Dispatch(const DispatchRequest &request,
    [[maybe_unused]] bool crossLanguageDebug)
{
    Method method = GetMethodEnum(request.GetMethod());
    LOG_DEBUGGER(INFO) << "dispatch [" << request.GetMethod() << "] to DomImpl";
    switch (method) {
        case Method::DISABLE:
            Disable(request);
            break;
        default:
            SendResponse(request, DispatchResponse::Fail("Unknown method: " + request.GetMethod()));
            break;
    }
    return std::nullopt;
}

DomImpl::DispatcherImpl::Method DomImpl::DispatcherImpl::GetMethodEnum(const std::string& method)
{
    if (method == "disable") {
        return Method::DISABLE;
    } else {
        return Method::UNKNOWN;
    }
}

void DomImpl::DispatcherImpl::Disable(const DispatchRequest &request)
{
    DispatchResponse response = DispatchResponse::Ok();
    SendResponse(request, response);
}
}  // namespace panda::ecmascript::tooling