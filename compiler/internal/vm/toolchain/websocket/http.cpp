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

#include "common/log_wrapper.h"
#include "http.h"

namespace OHOS::ArkCompiler::Toolchain {

const std::regex HttpRequest::localUrlRegex(
    R"(^(https?:\/\/)(localhost|127\.0\.0\.1|\[::1\])(:\d+)?$)", std::regex_constants::icase
);
const std::regex HttpRequest::httpPrefixRegex(R"(^(https?:\/\/).*)");

bool HttpRequest::ValidateHttpRequestOrigin(const std::string& httpRequestOrigin)
{
    // An empty Origin header is considered as safe
    if (httpRequestOrigin.empty()) {
        return true;
    }

    // If the Origin header does not start with "http://" or "https://", it is considered as safe
    if (!std::regex_match(httpRequestOrigin, httpPrefixRegex)) {
        return true;
    }

    // If it start with "http://" or "https://", it must be followed by the local IP to be considered secure
    return std::regex_match(httpRequestOrigin, localUrlRegex);
}

/* static */
std::string HttpBase::DecodeHeader(const std::string& headersText, std::string_view headerName)
{
    auto startPos = headersText.find(headerName);
    if (startPos != std::string::npos) {
        auto endOfLinePos = headersText.find(EOL, startPos);
        startPos += headerName.size();
        if (startPos < headersText.size() && startPos < endOfLinePos) {
            return headersText.substr(startPos, endOfLinePos - startPos);
        }
    }
    return "";
}

/* static */
std::string HttpRequest::DecodeVersion(const std::string& request, std::string::size_type methodStartPos)
{
    if (methodStartPos >= request.size()) {
        return "";
    }

    auto endOfLinePos = request.find(EOL, methodStartPos);
    // the typical header is "GET /chat HTTP/1.1", where protocol version is located after the second space symbol
    methodStartPos = request.find(' ', methodStartPos);
    if (methodStartPos != std::string::npos) {
        methodStartPos = request.find(' ', methodStartPos + 1);
    }
    if (methodStartPos != std::string::npos &&
        methodStartPos + 1 < request.size() &&
        methodStartPos + 1 < endOfLinePos) {
        return request.substr(methodStartPos + 1, endOfLinePos - (methodStartPos + 1));
    }
    return "";
}

// request example can be found at https://www.rfc-editor.org/rfc/rfc6455#section-1.3
/* static */
bool HttpRequest::Decode(const std::string& request, HttpRequest& parsed)
{
    auto pos = request.find(GET);
    if (pos == std::string::npos) {
        LOGW("Handshake failed: lack of necessary info");
        return false;
    }

    parsed.version = DecodeVersion(request, pos);
    parsed.connection = DecodeHeader(request, CONNECTION);
    parsed.upgrade = DecodeHeader(request, UPGRADE);
    parsed.secWebSocketKey = DecodeHeader(request, SEC_WEBSOCKET_KEY);
    parsed.origin = DecodeHeader(request, ORIGIN);

    // Validate Origin field to prevent cross-domain attacks
    return ValidateHttpRequestOrigin(parsed.origin);
}

/* static */
std::string HttpResponse::DecodeVersion(const std::string& response, std::string::size_type versionStartPos)
{
    // status-line example: "HTTP/1.1 404 Not Found"
    if (versionStartPos < response.size()) {
        auto versionEndPos = response.find(' ', versionStartPos);
        if (versionEndPos != std::string::npos) {
            return response.substr(versionStartPos, versionEndPos - versionStartPos);
        }
    }
    return "";
}

/* static */
std::string HttpResponse::DecodeStatus(const std::string& response, std::string::size_type versionEndPos)
{
    // status-line example: "HTTP/1.1 404 Not Found"
    if (versionEndPos < response.size() && response[versionEndPos] == ' ') {
        auto statusStartPos = response.find_first_not_of(' ', versionEndPos);
        if (statusStartPos != std::string::npos) {
            auto statusEndPos = response.find(' ', statusStartPos);
            statusEndPos = (statusEndPos == std::string::npos) ? response.find(EOL, statusStartPos) : statusEndPos;
            if (statusEndPos != std::string::npos) {
                return response.substr(statusStartPos, statusEndPos - statusStartPos);
            }
        }
    }
    return "";
}

// request example can be found at https://www.rfc-editor.org/rfc/rfc6455#section-1.2
/* static */
bool HttpResponse::Decode(const std::string& response, HttpResponse& parsed)
{
    // find start of status-line
    auto versionStartPos = response.find("HTTP");
    if (versionStartPos == std::string::npos) {
        LOGW("Handshake failed: lack of necessary info, no status-line found");
        return false;
    }

    parsed.version = DecodeVersion(response, versionStartPos);
    parsed.status = DecodeStatus(response, versionStartPos + parsed.version.size());
    parsed.connection = DecodeHeader(response, CONNECTION);
    parsed.upgrade = DecodeHeader(response, UPGRADE);
    parsed.secWebSocketAccept = DecodeHeader(response, SEC_WEBSOCKET_ACCEPT);

    return true;
}
} // namespace OHOS::ArkCompiler::Toolchain
