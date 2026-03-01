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

#include "StdioTransport.h"
#include "../languageserver/capabilities/shutdown/Shutdown.h"
#include "../languageserver/logger/Logger.h"

namespace ark {
nlohmann::json EncodeError(const MessageErrorDetail &errorInfo)
{
    nlohmann::json root;
    root["message"] = errorInfo.message;
    root["code"] = static_cast<int64_t>(errorInfo.code);
    return root;
}

void StdioTransport::SetIO(std::FILE *in, std::FILE *out)
{
    pFileIn = in;
    pFileOut = out;
}

void StdioTransport::Notify(std::string method, ValueOrError params)
{
    nlohmann::json root;
    root["jsonrpc"] = "2.0";
    root["method"] = method;
    if (params.type == ValueOrErrorCheck::VALUE) {
        root["params"] = std::move(params.jsonValue);
    } else {
        root["error"] = EncodeError(params.errorInfo);
    }
    SendMsg(root);
}

void StdioTransport::Reply(nlohmann::json id, ValueOrError result)
{
    nlohmann::json root;
    root["jsonrpc"] = "2.0";
    root["id"] = std::move(id);
    if (result.type == ValueOrErrorCheck::VALUE) {
        root["result"] = std::move(result.jsonValue);
    } else {
        root["error"] = EncodeError(result.errorInfo);
    }
    SendMsg(root);
}

LSPRet StdioTransport::Loop(MessageHandler &handler)
{
    Logger &logger = Logger::Instance();
    std::stringstream log;
    while (!feof(pFileIn)) {
        auto json = ReadRawMessage();
        if (!json.empty()) {
            CleanAndLog(log, "receive message body:" + json);
            logger.LogMessage(MessageType::MSG_INFO, log.str());
            logger.CollectMessageInfo(logger.LogInfo(MessageType::MSG_INFO, json));
            nlohmann::json doc;
            try {
                doc = nlohmann::json::parse(json);
            } catch (nlohmann::detail::parse_error &errs) {
                CleanAndLog(log, errs.what());
                logger.LogMessage(MessageType::MSG_WARNING, log.str());
                continue;
            }
            LSPRet ret = HandleMessage(std::move(doc), handler);
            if (ret == LSPRet::NORMAL_EXIT || ret == LSPRet::ABNORMAL_EXIT) {
                return ret;
            }
        }
    }
    return LSPRet::ERR_IO;
}

void StdioTransport::SendMsg(const nlohmann::json &message)
{
    std::stringstream log;
    std::ostringstream os;
    // "-1", no indentation format
    os << message.dump(-1, ' ', false, nlohmann::json::error_handler_t::ignore);
    (void)fprintf(pFileOut, "Content-Length: %d%s%s",
                   static_cast<int>(os.str().size()), MessageHeaderEndOfLine::GetEol().c_str(), os.str().c_str());
    (void)fflush(pFileOut);
    CleanAndLog(log, "send message body:" + os.str());
    Logger::Instance().LogMessage(MessageType::MSG_INFO, log.str());
}

std::string StdioTransport::ReadRawMessage()
{
    return ReadStandardMessage();
}

bool ReadLine(std::FILE *in, std::string &out)
{
    static constexpr int bufSize = 1024;
    size_t size = 0;
    out.clear();
    for (;;) {
        out.resize(size + bufSize);

        if (!RetryAfterSignalUnlessShutdown(nullptr, [&] { return std::fgets(&out[size], bufSize, in); })) {
            return false;
        }
        clearerr(in);

        size_t read = std::strlen(&out[size]);
        if (read > 0 && out[size + read - 1] == '\n') {
            out.resize(size + read);
            return true;
        }
        size += read;
    }
}

void Trim(std::string &s)
{
    if (s.empty()) {
        return;
    }
    (void)s.erase(0, s.find_first_not_of(' '));
    (void)s.erase(s.find_last_not_of(' ') + 1);
}

bool isLineEmpty(const std::string& line)
{
    for (char c : line) {
        if (!std::isspace(static_cast<unsigned char>(c))) {
            return false;
        }
    }
    return true;
}

std::string StdioTransport::ReadStandardMessage()
{
    unsigned long long contentLength = 0;
    std::string line;
    Logger &logger = Logger::Instance();
    bool headersEnded = false;
    for (;;) {
        if (feof(pFileIn) || ferror(pFileIn) || !ReadLine(pFileIn, line)) { return ""; }
        if (line.front() == '#') { continue; }
        Trim(line);

        if (isLineEmpty(line)) {
            headersEnded = true;
            break;
        }

        auto found = line.find("Content-Length:");
        if (found != std::string::npos) {
            if (contentLength != 0) {
                logger.LogMessage(MessageType::MSG_WARNING, "Duplicate Content-Length header received.");
            }
            std::stringstream stream;
            stream << line.substr(found + strlen("Content-Length:"));
            stream >> contentLength;
            stream.clear();
        }
    }
    if (!headersEnded) {
        logger.LogMessage(MessageType::MSG_WARNING, "Headers not properly terminated.");
        return "";
    }

    if (contentLength > 1 << MAX_MESSAGE_LENGTH) {
        logger.LogMessage(MessageType::MSG_WARNING, "Refusing to read message with too long Content-Length.");
        return "";
    }
    if (contentLength == 0) {
        logger.LogMessage(MessageType::MSG_WARNING, "Missing Content-Length header, or zero-length message.");
        return "";
    }

    std::string json(contentLength, '\0');
    size_t read;
    size_t pos = 0;
    while (pos < contentLength) {
        read = RetryAfterSignalUnlessShutdown(0, [&json, &pos, &contentLength, this] {
            return std::fread(&json[pos], 1, contentLength - pos, pFileIn);
        });
        if (read == 0) {
            logger.LogMessage(MessageType::MSG_WARNING, "Input was aborted.");
            return "";
        }

        clearerr(pFileIn);
        pos += read;
    }
    return std::move(json);
}

MessageErrorDetail DecodeError(const nlohmann::json &o)
{
    std::string msg = o.contains("message") ? o.value("message", "") : "Unspecified error";
    ErrorCode code = o.contains("code") ? ErrorCode(o.value("code", -1)) : ErrorCode::UNKNOWN_ERROR_CODE;
    return {msg, code};
}

LSPRet StdioTransport::HandleMessage(nlohmann::json message, MessageHandler &handler)
{
    Logger &logger = Logger::Instance();
    // Message must be an object with "jsonrpc":"2.0".
    if (message["jsonrpc"].is_null() || message.value("jsonrpc", "") != "2.0") {
        logger.LogMessage(MessageType::MSG_WARNING, "jsonrpc is null or not 2.0.");
        return LSPRet::ERR_JSON;
    }
    nlohmann::json id = nullptr;
    if (!message["id"].is_null()) {
        id = std::move(message["id"]);
    }

    // call or notify
    if (!message["method"].is_null()) {
        std::string method = message.value("method", "");
        if (method == "exit") {
            if (ShutdownRequested()) {
                return LSPRet::NORMAL_EXIT;
            }
            return LSPRet::ABNORMAL_EXIT;
        }
        nlohmann::json params = nullptr;
        if (message["params"].is_object()) {
            params = std::move(message["params"]);
        }
        if (id.is_null()) {
            return handler.OnNotify(method, std::move(params));
        }
        return handler.OnCall(method, std::move(params), std::move(id));
    }
    if (id.is_null()) {
        std::stringstream log;
        log << "No method and no response ID, message: " << message;
        logger.LogMessage(MessageType::MSG_WARNING, log.str());
        return LSPRet::ERR_JSON;
    }
    // reply
    if (!message["error"].is_null()) {
        return handler.OnReply(std::move(id),
                               ValueOrError(ValueOrErrorCheck::ERR, DecodeError(message["error"])));
    }
    nlohmann::json jsonValue = nullptr;
    if (!message["result"].is_null()) {
        jsonValue = std::move(message["result"]);
    }
    return handler.OnReply(std::move(id), ValueOrError(ValueOrErrorCheck::VALUE, jsonValue));
}
}
