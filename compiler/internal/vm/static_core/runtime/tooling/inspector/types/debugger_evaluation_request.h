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

#ifndef PANDA_TOOLING_INSPECTOR_TYPES_DEBUGGER_EVALUATION_REQUEST_H
#define PANDA_TOOLING_INSPECTOR_TYPES_DEBUGGER_EVALUATION_REQUEST_H

#include "libarkbase/macros.h"
#include "libarkbase/utils/expected.h"

namespace ark {
class JsonObject;
}  // namespace ark

namespace ark::tooling::inspector {

/**
 * @brief Request parameters of `Debugger.evaluateOnCallFrame` method.
 * Instances of this class must not outlive the source `JsonObject`.
 */
class DebuggerEvaluationRequest final {
public:
    static Expected<DebuggerEvaluationRequest, std::string> FromJson(const JsonObject &object);

    DEFAULT_COPY_SEMANTIC(DebuggerEvaluationRequest);
    DEFAULT_MOVE_SEMANTIC(DebuggerEvaluationRequest);

    ~DebuggerEvaluationRequest() = default;

    size_t GetCallFrameId() const
    {
        return callFrameId_;
    }

    const std::string &GetExpression() const
    {
        return *expression_;
    }

private:
    DebuggerEvaluationRequest() = default;

private:
    size_t callFrameId_ {0};
    const std::string *expression_ {nullptr};
    bool silent_ {false};
    bool returnByValue_ {false};
    bool generatePreview_ {false};
};

}  // namespace ark::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_TYPES_DEBUGGER_EVALUATION_REQUEST_H
