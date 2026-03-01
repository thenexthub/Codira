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
#ifndef PANDA_TOOLING_INSPECTOR_EXCEPTION_DETAILS_H
#define PANDA_TOOLING_INSPECTOR_EXCEPTION_DETAILS_H

#include "json_serialization/serializable.h"

#include <optional>
#include <string>

#include "libarkbase/macros.h"
#include "types/numeric_id.h"
#include "types/remote_object.h"

namespace ark {
class JsonObjectBuilder;
}  // namespace ark

namespace ark::tooling::inspector {
class ExceptionDetails final : public JsonSerializable {
public:
    ExceptionDetails(size_t exceptionId, std::string text, int32_t lineNumber, int32_t columnNumber)
        : exceptionId_(exceptionId), text_(std::move(text)), lineNumber_(lineNumber), columnNumber_(columnNumber)
    {
    }

    void Serialize(JsonObjectBuilder &builder) const override;

    size_t GetExceptionId() const
    {
        return exceptionId_;
    }

    const std::string &GetText() const
    {
        return text_;
    }

    int32_t GetLine() const
    {
        return lineNumber_;
    }

    int32_t GetColumn() const
    {
        return columnNumber_;
    }

    ScriptId GetScriptId() const
    {
        return scriptId_.value_or(0);
    }

    ExceptionDetails &SetScriptId(ScriptId scriptId)
    {
        scriptId_ = scriptId;
        return *this;
    }

    bool HasScriptId() const
    {
        return scriptId_.has_value();
    }

    const std::string &GetUrl() const
    {
        ASSERT(HasUrl());
        return url_.value();
    }

    ExceptionDetails &SetUrl(std::string_view url)
    {
        url_ = url;
        return *this;
    }

    bool HasUrl() const
    {
        return url_.has_value();
    }

    const std::optional<RemoteObject> &GetExceptionObject() const
    {
        return exception_;
    }

    ExceptionDetails &SetExceptionObject(RemoteObject &&exception)
    {
        exception_ = std::move(exception);
        return *this;
    }

    bool HasExceptionObject() const
    {
        return exception_.has_value();
    }

    ExecutionContextId GetExecutionContextId() const
    {
        return executionContextId_.value_or(-1);
    }

    ExceptionDetails &SetExecutionContextId(ExecutionContextId executionContextId)
    {
        executionContextId_ = executionContextId;
        return *this;
    }

    bool HasExecutionContextId() const
    {
        return executionContextId_.has_value();
    }

private:
    size_t exceptionId_ {0};
    std::string text_;
    int32_t lineNumber_ {0};
    int32_t columnNumber_ {0};
    std::optional<ScriptId> scriptId_;
    std::optional<std::string> url_;
    std::optional<RemoteObject> exception_;
    std::optional<ExecutionContextId> executionContextId_;
};
}  // namespace ark::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_EXCEPTION_DETAILS_H
