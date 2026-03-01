/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_CODE_SCOPES_H
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_CODE_SCOPES_H

#include "plugins/ets/runtime/interop_js/interop_context.h"

namespace ark::ets::interop::js {

/// @brief RAII-style class for registering interop call stack information
class ScopedInteropCallStackRecord {
public:
    explicit ScopedInteropCallStackRecord(EtsCoroutine *coro, char const *descr = nullptr);
    ~ScopedInteropCallStackRecord();

private:
    EtsCoroutine *coro_;

    NO_COPY_SEMANTIC(ScopedInteropCallStackRecord);
    NO_MOVE_SEMANTIC(ScopedInteropCallStackRecord);
};

/// @brief RAII-style class for switching from ETS state to JS
class InteropETSToJSCodeScope {
public:
    explicit InteropETSToJSCodeScope(EtsCoroutine *coro, char const *descr = nullptr);
    ~InteropETSToJSCodeScope();

private:
    EtsCoroutine *coro_;

    NO_COPY_SEMANTIC(InteropETSToJSCodeScope);
    NO_MOVE_SEMANTIC(InteropETSToJSCodeScope);
};

/// @brief RAII-style class for switching from JS state to ETS
class InteropJSToETSCodeScope {
public:
    explicit InteropJSToETSCodeScope(EtsCoroutine *coro, char const *descr = nullptr);
    ~InteropJSToETSCodeScope();

private:
    EtsCoroutine *coro_;

    NO_COPY_SEMANTIC(InteropJSToETSCodeScope);
    NO_MOVE_SEMANTIC(InteropJSToETSCodeScope);
};

// CC-OFFNXT(G.PRE.02-CPP) for readability and ease of use
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define INTEROP_CODE_SCOPE_JS_TO_ETS(coro) InteropJSToETSCodeScope codeScope(coro, __PRETTY_FUNCTION__)
// CC-OFFNXT(G.PRE.02-CPP) for readability and ease of use
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define INTEROP_CODE_SCOPE_ETS_TO_JS(coro) InteropETSToJSCodeScope codeScope(coro, __PRETTY_FUNCTION__)

}  // namespace ark::ets::interop::js

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_CODE_SCOPES_H
