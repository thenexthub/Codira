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

#ifndef PANDA_TOOLING_INSPECTOR_TYPES_DEBUGGER_CALL_FUNCTION_ON_REQUEST_H
#define PANDA_TOOLING_INSPECTOR_TYPES_DEBUGGER_CALL_FUNCTION_ON_REQUEST_H

#include "libarkbase/macros.h"
#include "libarkbase/utils/expected.h"

namespace ark {
class JsonObject;
}  // namespace ark

namespace ark::tooling::inspector {

/**
 * @brief Request parameters of `Debugger.callFunctionOn` method.
 * Instances of this class must not outlive the source `JsonObject`.
 * Parameters:
 * - `callFrameId_`: size_t - The identifier of the call frame where the function will be executed. (Required)
 * - `functionDeclaration_`: std::string - The function declaration string to be executed. (Required)
 * - `objectId_`: size_t - The identifier of the object on which the function will be called. (Optional)
 * - `arguments_`: std::vector<std::unique_ptr<CallArgument>> - A list of arguments to pass to the function. (Optional)
 * - `silent_`: bool - Whether to suppress execution side effects. (Optional)
 * - `returnByValue_`: bool - Whether to return the result as a value or by reference. (Optional)
 * - `generatePreview_`: bool - Whether to generate a preview of the result. (Optional)
 * - `userGesture_`: bool - Whether the function call is triggered by a user gesture. (Optional)
 * - `awaitPromise_`: bool - Whether to await the promise returned by the function. (Optional)
 * - `executionContextId_`: size_t - The identifier of the execution context where the function will be executed.
 * (Optional)
 * - `objectGroup_`: std::string - The group name for the result object. (Optional)
 * - `throwOnSideEffect_`: bool - Whether to throw an exception if the function has side effects. (Optional)
 */
class DebuggerCallFunctionOnRequest final {
public:
    static Expected<DebuggerCallFunctionOnRequest, std::string> FromJson(const JsonObject &object);

    DEFAULT_COPY_SEMANTIC(DebuggerCallFunctionOnRequest);
    DEFAULT_MOVE_SEMANTIC(DebuggerCallFunctionOnRequest);

    ~DebuggerCallFunctionOnRequest() = default;

    size_t GetCallFrameId() const
    {
        return callFrameId_;
    }

    const std::string &GetFunctionDeclaration() const
    {
        return *functionDeclaration_;
    }

private:
    DebuggerCallFunctionOnRequest() = default;

private:
    size_t callFrameId_ {0};
    const std::string *functionDeclaration_ {nullptr};
    bool silent_ {false};
    bool returnByValue_ {false};
    bool generatePreview_ {false};
};

}  // namespace ark::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_TYPES_DEBUGGER_CALL_FUNCTION_ON_REQUEST_H
