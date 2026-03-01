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

#include "debugger_service.h"

#include "protocol_handler.h"

#include "ecmascript/debugger/js_debugger_manager.h"

namespace panda::ecmascript::tooling {
void InitializeDebugger(::panda::ecmascript::EcmaVM *vm,
                        const std::function<void(const void *, const std::string &)> &onResponse)
{
    if (vm == nullptr || vm->GetJsDebuggerManager() == nullptr) {
        LOG_DEBUGGER(DEBUG) << "VM has already been destroyed";
        return;
    }
    ProtocolHandler *handler = vm->GetJsDebuggerManager()->GetDebuggerHandler();
    if (handler != nullptr) {
        LOG_DEBUGGER(ERROR) << "JS debugger was initialized";
        return;
    }
    vm->GetJsDebuggerManager()->SetDebuggerHandler(new ProtocolHandler(onResponse, vm));
}

void SetDebugApp(::panda::ecmascript::EcmaVM *vm)
{
    if (vm == nullptr || vm->GetJsDebuggerManager() == nullptr) {
        LOG_DEBUGGER(DEBUG) << "VM has already been destroyed";
        return;
    }
    vm->GetJsDebuggerManager()->SetIsDebugApp(true);
}

void UninitializeDebugger(::panda::ecmascript::EcmaVM *vm)
{
    if (vm == nullptr || vm->GetJsDebuggerManager() == nullptr) {
        LOG_DEBUGGER(DEBUG) << "VM has already been destroyed";
        return;
    }
    ProtocolHandler *handler = vm->GetJsDebuggerManager()->GetDebuggerHandler();
    delete handler;
    vm->GetJsDebuggerManager()->SetDebuggerHandler(nullptr);
}

void WaitForDebugger(const ::panda::ecmascript::EcmaVM *vm)
{
    if (vm == nullptr || vm->GetJsDebuggerManager() == nullptr) {
        LOG_DEBUGGER(DEBUG) << "VM has already been destroyed";
        return;
    }
    ProtocolHandler *handler = vm->GetJsDebuggerManager()->GetDebuggerHandler();
    if (LIKELY(handler != nullptr)) {
        handler->WaitForDebugger();
    }
}

void OnMessage(const ::panda::ecmascript::EcmaVM *vm, std::string &&message)
{
    if (vm == nullptr || vm->GetJsDebuggerManager() == nullptr) {
        LOG_DEBUGGER(DEBUG) << "VM has already been destroyed";
        return;
    }
    ProtocolHandler *handler = vm->GetJsDebuggerManager()->GetDebuggerHandler();
    if (LIKELY(handler != nullptr)) {
        handler->DispatchCommand(std::move(message));
    }
}

void ProcessMessage(const ::panda::ecmascript::EcmaVM *vm)
{
    if (vm == nullptr || vm->GetJsDebuggerManager() == nullptr) {
        LOG_DEBUGGER(DEBUG) << "VM has already been destroyed";
        return;
    }
    ProtocolHandler *handler = vm->GetJsDebuggerManager()->GetDebuggerHandler();
    if (LIKELY(handler != nullptr)) {
        handler->ProcessCommand();
    }
}

int32_t GetDispatchStatus(const ::panda::ecmascript::EcmaVM *vm)
{
    if (vm == nullptr || vm->GetJsDebuggerManager() == nullptr) {
        LOG_DEBUGGER(DEBUG) << "VM has already been destroyed";
        return ProtocolHandler::DispatchStatus::UNKNOWN;
    }
    ProtocolHandler *handler = vm->GetJsDebuggerManager()->GetDebuggerHandler();
    if (LIKELY(handler != nullptr)) {
        return handler->GetDispatchStatus();
    }
    return ProtocolHandler::DispatchStatus::UNKNOWN;
}

DebugResponse GetCallFrames(const ::panda::ecmascript::EcmaVM *vm)
{
    if (vm == nullptr || vm->GetJsDebuggerManager() == nullptr) {
        LOG_DEBUGGER(ERROR) << "VM has already been destroyed";
        return {0, nullptr};
    }
    ProtocolHandler *handler = vm->GetJsDebuggerManager()->GetDebuggerHandler();
    if (handler == nullptr) {
        LOG_DEBUGGER(ERROR) << "GetCallFrames: Debugger handler is null";
        return {0, nullptr};
    }
    auto dispatcher = handler->GetDispatcher();
    if (dispatcher == nullptr) {
        LOG_DEBUGGER(ERROR) << "GetCallFrames: Dispatcher is null";
        return {0, nullptr};
    }
    std::string info = dispatcher->GetJsFrames();
    auto size = info.size();
    char* response = new char[size + 1]();
    int result = memcpy_s(response, size + 1, info.c_str(), size);
    if (result != 0) {
        LOG_DEBUGGER(ERROR) << "GetCallFrames: Memory copy failed";
        delete[] response;
        return {0, nullptr};
    }
    return {size, response};
}

DebugResponse OperateDebugMessage(const ::panda::ecmascript::EcmaVM *vm, const char* message)
{
    if (message == nullptr) {
        LOG_DEBUGGER(ERROR) << "OperateDebugMessage: message is null";
        return {0, nullptr};
    }
    if (vm == nullptr || vm->GetJsDebuggerManager() == nullptr) {
        LOG_DEBUGGER(ERROR) << "VM has already been destroyed";
        return {0, nullptr};
    }
    ProtocolHandler *handler = vm->GetJsDebuggerManager()->GetDebuggerHandler();
    if (handler == nullptr) {
        LOG_DEBUGGER(ERROR) << "OperateDebugMessage: Debugger handler is null";
        return {0, nullptr};
    }
    auto dispatcher = handler->GetDispatcher();
    if (dispatcher == nullptr) {
        LOG_DEBUGGER(ERROR) << "OperateDebugMessage: Dispatcher is null";
        return {0, nullptr};
    }
    DispatchRequest request(message);
    auto info = dispatcher->Dispatch(request, true);
    if (!info.has_value()) {
        return {0, nullptr};
    }
    auto size = info.value().size();
    char* response = new char[size + 1]();
    int result = memcpy_s(response, size + 1, info.value().c_str(), size);
    if (result != 0) {
        LOG_DEBUGGER(ERROR) << "OperateDebugMessage: Memory copy failed";
        delete[] response;
        return {0, nullptr};
    }
    return {size, response};
}
}  // namespace panda::ecmascript::tooling