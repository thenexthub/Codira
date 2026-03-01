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

#ifndef ECMASCRIPT_TOOLING_DISPATCHER_H
#define ECMASCRIPT_TOOLING_DISPATCHER_H

#include <map>
#include <memory>
#include <set>

#include "tooling/dynamic/base/pt_returns.h"

#include "ecmascript/debugger/js_debugger_interface.h"
#include "ecmascript/napi/include/jsnapi.h"
#include "libpandabase/macros.h"

namespace panda::ecmascript::tooling {
class ProtocolChannel;
class PtBaseReturns;
class PtBaseEvents;

enum class RequestCode : uint8_t {
    OK = 0,
    NOK,

    // Json parse errors
    JSON_PARSE_ERROR,
    PARSE_ID_ERROR,
    ID_FORMAT_ERROR,
    PARSE_METHOD_ERROR,
    METHOD_FORMAT_ERROR,
    PARSE_PARAMS_ERROR,
    PARAMS_FORMAT_ERROR
};

enum class ResponseCode : uint8_t { OK, NOK };

class DispatchRequest {
public:
    explicit DispatchRequest(const std::string &message);
    ~DispatchRequest();

    bool IsValid() const
    {
        return code_ == RequestCode::OK;
    }
    int32_t GetCallId() const
    {
        return callId_;
    }
    const PtJson &GetParams() const
    {
        return *params_;
    }
    const std::string &GetDomain() const
    {
        return domain_;
    }
    const std::string &GetMethod() const
    {
        return method_;
    }

private:
    int32_t callId_ = -1;
    std::string domain_ {};
    std::string method_ {};
    std::unique_ptr<PtJson> params_ = std::make_unique<PtJson>();
    RequestCode code_ {RequestCode::OK};
    std::string errorMsg_ {};
    void JsonParseError()
    {
        code_ = RequestCode::JSON_PARSE_ERROR;
        LOG_DEBUGGER(ERROR) << "json parse error";
    }
    void JsonFormatError(std::unique_ptr<PtJson>& json)
    {
        code_ = RequestCode::PARAMS_FORMAT_ERROR;
        LOG_DEBUGGER(ERROR) << "json parse format error";
        json->ReleaseRoot();
    }
};

class DispatchResponse {
public:
    bool IsOk() const
    {
        return code_ == ResponseCode::OK;
    }

    ResponseCode GetError() const
    {
        return code_;
    }

    const std::string &GetMessage() const
    {
        return errorMsg_;
    }

    static DispatchResponse Create(ResponseCode code, const std::string &msg = "");
    static DispatchResponse Create(std::optional<std::string> error);
    static DispatchResponse Ok();
    static DispatchResponse Fail(const std::string &message);

    ~DispatchResponse() = default;

private:
    DispatchResponse() = default;

    ResponseCode code_ {ResponseCode::OK};
    std::string errorMsg_ {};
};

class DispatcherBase {
public:
    explicit DispatcherBase(ProtocolChannel *channel) : channel_(channel) {}
    virtual ~DispatcherBase()
    {
        channel_ = nullptr;
    };
    virtual std::optional<std::string> Dispatch(const DispatchRequest &request, bool crossLanguageDebug = false) = 0;

protected:
    void SendResponse(const DispatchRequest &request, const DispatchResponse &response,
                      const PtBaseReturns &result = PtBaseReturns());
    std::string ReturnsValueToString(const int32_t callId, const DispatchResponse& response,
        std::unique_ptr<PtBaseReturns> result);
    std::unique_ptr<PtJson> DispatchResponseToJson(const DispatchResponse &response) const;

private:
    ProtocolChannel *channel_ {nullptr};

    NO_COPY_SEMANTIC(DispatcherBase);
    NO_MOVE_SEMANTIC(DispatcherBase);
};

class Dispatcher {
public:
    explicit Dispatcher(const EcmaVM *vm, ProtocolChannel *channel);
    ~Dispatcher() = default;
    std::optional<std::string> Dispatch(const DispatchRequest &request, bool crossLanguageDebug = false) const;
    std::string GetJsFrames() const;
    std::string OperateDebugMessage(const char* message) const;

private:
    std::unordered_map<std::string, std::unique_ptr<DispatcherBase>> dispatchers_ {};

    NO_COPY_SEMANTIC(Dispatcher);
    NO_MOVE_SEMANTIC(Dispatcher);
};
}  // namespace panda::ecmascript::tooling
#endif
