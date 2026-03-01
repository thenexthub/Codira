/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "agent/debugger_impl.h"

#include "backend/debugger_executor.h"

#include "ecmascript/jspandafile/js_pandafile_manager.h"
#include "ecmascript/napi/jsnapi_helper.h"

#include "protocol_handler.h"

#include "tooling/dynamic/base/pt_base64.h"

namespace panda::ecmascript::tooling {
using namespace std::placeholders;

using ObjectType = RemoteObject::TypeName;
using ObjectSubType = RemoteObject::SubTypeName;
using ObjectClassName = RemoteObject::ClassName;
using StepperType = SingleStepper::Type;

#ifdef OHOS_UNIT_TEST
const std::string DATA_APP_PATH = "/";
#else
const std::string DATA_APP_PATH = "/data/";
#endif

static std::atomic<uint32_t> g_scriptId {0};

DebuggerImpl::DebuggerImpl(const EcmaVM *vm, ProtocolChannel *channel, RuntimeImpl *runtime)
    : vm_(vm), frontend_(channel), runtime_(runtime)
{
    hooks_ = std::make_unique<JSPtHooks>(this);

    jsDebugger_ = DebuggerApi::CreateJSDebugger(vm_);
    DebuggerApi::RegisterHooks(jsDebugger_, hooks_.get());

    updaterFunc_ = std::bind(&DebuggerImpl::UpdateScopeObject, this, _1, _2, _3, _4);
    stepperFunc_ = std::bind(&DebuggerImpl::ClearSingleStepper, this);
    returnNative_ = std::bind(&DebuggerImpl::NotifyReturnNative, this);
    vm_->GetJsDebuggerManager()->SetLocalScopeUpdater(&updaterFunc_);
    vm_->GetJsDebuggerManager()->SetStepperFunc(&stepperFunc_);
    vm_->GetJsDebuggerManager()->SetJSReturnNativeFunc(&returnNative_);
}

DebuggerImpl::~DebuggerImpl()
{
    // in worker thread, it will ~DebuggerImpl before release worker thread
    // after ~DebuggerImpl, it maybe call these methods
    vm_->GetJsDebuggerManager()->SetLocalScopeUpdater(nullptr);
    vm_->GetJsDebuggerManager()->SetStepperFunc(nullptr);
    vm_->GetJsDebuggerManager()->SetJSReturnNativeFunc(nullptr);
    DebuggerApi::DestroyJSDebugger(jsDebugger_);
}

bool DebuggerImpl::NotifyScriptParsed(const std::string &fileName, std::string_view entryPoint)
{
    if (!CheckScriptParsed(fileName)) {
        return false;
    }

    const JSPandaFile *jsPandaFile = JSPandaFileManager::GetInstance()->FindJSPandaFile(fileName.c_str()).get();
    if (jsPandaFile == nullptr) {
        LOG_DEBUGGER(ERROR) << "NotifyScriptParsed: unknown file: " << fileName;
        return false;
    }

    DebugInfoExtractor *extractor = GetExtractor(jsPandaFile);
    if (extractor == nullptr) {
        LOG_DEBUGGER(ERROR) << "NotifyScriptParsed: Unsupported file: " << fileName;
        return false;
    }

    if (!vm_->GetJsDebuggerManager()->GetFaApp() && DebuggerApi::JSPandaFileIsBundlePack(jsPandaFile)) {
        LOG_DEBUGGER(DEBUG) << "NotifyScriptParsed: Unmerge file: " << fileName;
        return false;
    }

    const char *recordName = entryPoint.data();
    auto mainMethodIndex = panda_file::File::EntityId(
        DebuggerApi::GetJSPandaFileMainMethodIndex(jsPandaFile, recordName));
    const std::string &source = extractor->GetSourceCode(mainMethodIndex);
    const std::string &url = extractor->GetSourceFile(mainMethodIndex);
    // if load module, it needs to check whether clear singlestepper_
    ClearSingleStepper();
    if (MatchUrlAndFileName(url, fileName)) {
        LOG_DEBUGGER(WARN) << "DebuggerImpl::NotifyScriptParsed: Script already been parsed: "
            << "url: " << url << " fileName: " << fileName;
        return false;
    }

    SaveParsedScriptsAndUrl(fileName, url, recordName, source);
    return true;
}

std::vector<std::shared_ptr<BreakpointReturnInfo>> DebuggerImpl::SetBreakpointsWhenParsingScript(const std::string &url)
{
    std::vector<std::shared_ptr<BreakpointReturnInfo>> outLocations {};
    for (const auto &breakpoint : breakpointPendingMap_[url]) {
        if (!ProcessSingleBreakpoint(*breakpoint, outLocations)) {
            std::string invalidBpId = "invalid";
            std::shared_ptr<BreakpointReturnInfo> bpInfo = std::make_shared<BreakpointReturnInfo>();
            bpInfo->SetId(invalidBpId)
                .SetLineNumber(breakpoint->GetLineNumber())
                .SetColumnNumber(breakpoint->GetColumnNumber());
            outLocations.emplace_back(bpInfo);
        }
    }
    return outLocations;
}

bool DebuggerImpl::NeedToSetBreakpointsWhenParsingScript(const std::string &url)
{
    if (breakpointPendingMap_.find(url) != breakpointPendingMap_.end()) {
        return !breakpointPendingMap_[url].empty();
    }
    return false;
}

bool DebuggerImpl::CheckScriptParsed([[maybe_unused]] const std::string &fileName)
{
#if !defined(PANDA_TARGET_WINDOWS) && !defined(PANDA_TARGET_MACOS) \
    && !defined(PANDA_TARGET_ANDROID) && !defined(PANDA_TARGET_IOS) \
    && !defined(PANDA_TARGET_LINUX)
    if (!IsApplicationFile(fileName)) {
        LOG_DEBUGGER(DEBUG) << "CheckScriptParsed: unsupport file: " << fileName;
        return false;
    }
#endif

    // check if Debugable flag is true in module.json
    if (!vm_->GetJsDebuggerManager()->IsDebugApp()) {
        LOG_DEBUGGER(WARN) << "DebugApp is false";
        return false;
    }

    return true;
}

bool DebuggerImpl::IsApplicationFile(const std::string &fileName)
{
    if (fileName.empty() || fileName.substr(0, DATA_APP_PATH.length()) != DATA_APP_PATH) {
        return false;
    }
    return true;
}

void DebuggerImpl::SaveParsedScriptsAndUrl(const std::string &fileName, const std::string &url,
    const std::string &recordName, const std::string &source)
{
    // Save recordName to its corresponding url
    recordNames_[url].insert(recordName);
    recordNameSet_.insert(recordName);
    // Save parsed fileName to its corresponding url
    urlFileNameMap_[url].insert(fileName);
    // Create and save script
    std::shared_ptr<PtScript> script = std::make_shared<PtScript>(g_scriptId++, fileName, url, source);
    scripts_[script->GetScriptId()] = script;
    // Check if is launch accelerate mode & has pending bps to set
    if (IsLaunchAccelerateMode() && NeedToSetBreakpointsWhenParsingScript(url)) {
        script->SetLocations(SetBreakpointsWhenParsingScript(url));
    }
    // Notify frontend ScriptParsed event
    frontend_.ScriptParsed(vm_, *script);
}

bool DebuggerImpl::NotifyScriptParsedBySendable(JSHandle<Method> method)
{
    JSThread *thread = vm_->GetJSThread();
    // Find extractor and retrieve infos
    const JSPandaFile *jsPandaFile = method->GetJSPandaFile(thread);
    if (jsPandaFile == nullptr) {
        LOG_DEBUGGER(ERROR) << "JSPandaFile is nullptr";
        return false;
    }
    DebugInfoExtractor *extractor = JSPandaFileManager::GetInstance()->GetJSPtExtractor(jsPandaFile);
    if (extractor == nullptr) {
        LOG_DEBUGGER(ERROR) << "extractor is nullptr";
        return false;
    }
    auto methodId = method->GetMethodId();
    const std::string &url = extractor->GetSourceFile(methodId);
    const std::string &fileName = std::string(DebuggerApi::GetJSPandaFileDesc(jsPandaFile));
    // Check url path & is debugable in module.json
    if (!CheckScriptParsed(fileName)) {
        return false;
    }
    // Clear SingleStepper before notify
    ClearSingleStepper();
    // Check if this (url, fileName) pair has already been parsed
    if (MatchUrlAndFileName(url, fileName)) {
        LOG_DEBUGGER(WARN) << "DebuggerImpl::NotifyScriptParsedBySendable: Script already been parsed: "
            << "url: " << url << " fileName: " << fileName;
        return false;
    }
    // Parse and save this file
    const std::string &source = extractor->GetSourceCode(methodId);
    const std::string &recordName = std::string(method->GetRecordNameStr(thread));
    SaveParsedScriptsAndUrl(fileName, url, recordName, source);
    return true;
}

bool DebuggerImpl::MatchUrlAndFileName(const std::string &url, const std::string &fileName)
{
    auto urlFileNameIter = urlFileNameMap_.find(url);
    if (urlFileNameIter != urlFileNameMap_.end()) {
        if (urlFileNameIter->second.find(fileName) != urlFileNameIter->second.end()) {
            return true;
        }
    }
    return false;
}

bool DebuggerImpl::NotifyNativeOut()
{
    if (nativeOutPause_) {
        nativeOutPause_ = false;
        return true;
    }
    return false;
}

bool DebuggerImpl::NotifySingleStep(const JSPtLocation &location)
{
    if (UNLIKELY(pauseOnNextByteCode_)) {
        if (IsSkipLine(location)) {
            return false;
        }
        pauseOnNextByteCode_ = false;
        LOG_DEBUGGER(INFO) << "StepComplete: pause on next bytecode";
        return true;
    }

    if (LIKELY(singleStepper_ == nullptr)) {
        return false;
    }

    // step not complete
    if (!singleStepper_->StepComplete(location.GetBytecodeOffset())) {
        return false;
    }

    // skip unknown file or special line -1
    if (IsSkipLine(location)) {
        return false;
    }

    singleStepper_.reset();
    LOG_DEBUGGER(INFO) << "StepComplete: pause on current byte_code";
    if (!DebuggerApi::GetSingleStepStatus(jsDebugger_)) {
        DebuggerApi::SetSingleStepStatus(jsDebugger_, true);
    }
    return true;
}

bool DebuggerImpl::IsSkipLine(const JSPtLocation &location)
{
    DebugInfoExtractor *extractor = nullptr;
    const auto *jsPandaFile = location.GetJsPandaFile();
    auto scriptFunc = [this, &extractor, jsPandaFile](PtScript *) -> bool {
        extractor = GetExtractor(jsPandaFile);
        return true;
    };

    // In hot reload scenario, use the base js panda file instead
    const auto &fileName = DebuggerApi::GetJSPandaFileDesc(DebuggerApi::GetBaseJSPandaFile(vm_, jsPandaFile));
    if (!MatchScripts(scriptFunc, fileName.c_str(), ScriptMatchType::FILE_NAME) || extractor == nullptr) {
        LOG_DEBUGGER(INFO) << "StepComplete: skip unknown file " << fileName.c_str();
        return true;
    }

    auto callbackFunc = [](int32_t line) -> bool {
        return line == DebugInfoExtractor::SPECIAL_LINE_MARK;
    };
    panda_file::File::EntityId methodId = location.GetMethodId();
    uint32_t offset = location.GetBytecodeOffset();
    if (extractor->MatchLineWithOffset(callbackFunc, methodId, offset)) {
        LOG_DEBUGGER(INFO) << "StepComplete: skip -1";
        return true;
    }

    return false;
}

bool DebuggerImpl::CheckPauseOnException()
{
    if (pauseOnException_ == PauseOnExceptionsState::NONE) {
        return false;
    }
    if (pauseOnException_ == PauseOnExceptionsState::UNCAUGHT) {
        if (DebuggerApi::IsExceptionCaught(vm_)) {
            return false;
        }
    }
    return true;
}

void DebuggerImpl::NotifyPaused(std::optional<JSPtLocation> location, PauseReason reason)
{
    if (skipAllPausess_) {
        return;
    }

    if (location.has_value() && !breakpointsState_) {
        return;
    }

    if (reason == EXCEPTION && !CheckPauseOnException()) {
        return;
    }

    Local<JSValueRef> exception = DebuggerApi::GetAndClearException(vm_);

    std::vector<std::string> hitBreakpoints;
    if (location.has_value()) {
        BreakpointDetails detail;
        DebugInfoExtractor *extractor = nullptr;
        auto scriptFunc = [this, &location, &detail, &extractor](PtScript *script) -> bool {
            detail.url_ = script->GetUrl();
            extractor = GetExtractor(location->GetJsPandaFile());
            return true;
        };
        auto callbackLineFunc = [&detail](int32_t line) -> bool {
            detail.line_ = line;
            return true;
        };
        auto callbackColumnFunc = [&detail](int32_t column) -> bool {
            detail.column_ = column;
            return true;
        };
        panda_file::File::EntityId methodId = location->GetMethodId();
        uint32_t offset = location->GetBytecodeOffset();
        // In merge abc scenario, need to use the source file to match to get right url
        if (!MatchScripts(scriptFunc, location->GetSourceFile(), ScriptMatchType::URL) ||
            extractor == nullptr || !extractor->MatchLineWithOffset(callbackLineFunc, methodId, offset) ||
            !extractor->MatchColumnWithOffset(callbackColumnFunc, methodId, offset)) {
            LOG_DEBUGGER(ERROR) << "NotifyPaused: unknown file " << location->GetSourceFile();
            return;
        }
        hitBreakpoints.emplace_back(BreakpointDetails::ToString(detail));
    }

    // Do something cleaning on paused
    CleanUpOnPaused();
    GeneratePausedInfo(reason, hitBreakpoints, exception);
}

void DebuggerImpl::GeneratePausedInfo(PauseReason reason,
                                      std::vector<std::string> &hitBreakpoints,
                                      const Local<JSValueRef> &exception)
{
    // Notify paused event
    std::vector<std::unique_ptr<CallFrame>> callFrames;
    if (!GenerateCallFrames(&callFrames, true)) {
        LOG_DEBUGGER(ERROR) << "NotifyPaused: GenerateCallFrames failed";
        return;
    }
    tooling::Paused paused;
    if (reason == DEBUGGERSTMT) {
        BreakpointDetails detail;
        hitBreakpoints.emplace_back(BreakpointDetails::ToString(detail));
        paused.SetCallFrames(std::move(callFrames))
            .SetReason(PauseReason::OTHER)
            .SetHitBreakpoints(std::move(hitBreakpoints));
    } else {
        paused.SetCallFrames(std::move(callFrames)).SetReason(reason).SetHitBreakpoints(std::move(hitBreakpoints));
    }
    if (reason == EXCEPTION && exception->IsError(vm_)) {
        std::unique_ptr<RemoteObject> tmpException = RemoteObject::FromTagged(vm_, exception);
        paused.SetData(std::move(tmpException));
    }
    if (vm_->GetJsDebuggerManager()->IsAsyncStackTrace()) {
        paused.SetAsyncCallChainDepth(maxAsyncCallChainDepth_);
        paused.SetAysncStack(DebuggerApi::GetCurrentAsyncParent(vm_));
    }
    frontend_.Paused(vm_, paused);
    if (reason != BREAK_ON_START && reason != NATIVE_OUT) {
        singleStepper_.reset();
    }
    nativeOutPause_ = false;
    debuggerState_ = DebuggerState::PAUSED;
    frontend_.WaitForDebugger(vm_);
    DebuggerApi::SetException(vm_, exception);
}

bool DebuggerImpl::IsUserCode(const void *nativeAddress)
{
    uint64_t nativeEntry =  reinterpret_cast<uint64_t>(nativeAddress);
    for (const auto &nativeRange : nativeRanges_) {
        if (nativeEntry >= nativeRange.GetStart() && nativeEntry <= nativeRange.GetEnd()) {
            return true;
        }
    }
    return false;
}

void DebuggerImpl::NotifyNativeCalling(const void *nativeAddress)
{
    // native calling only after step into should be reported
    if (singleStepper_ != nullptr &&
        singleStepper_->GetStepperType() == StepperType::STEP_INTO) {
        tooling::NativeCalling nativeCalling;
        nativeCalling.SetIntoStatus(true);
        nativeCalling.SetNativeAddress(nativeAddress);
        frontend_.NativeCalling(vm_, nativeCalling);
        frontend_.WaitForDebugger(vm_);
    }

    if (mixStackEnabled_ && IsUserCode(nativeAddress)) {
        tooling::MixedStack mixedStack;
        nativePointer_ = DebuggerApi::GetNativePointer(vm_);
        mixedStack.SetNativePointers(nativePointer_);
        std::vector<std::unique_ptr<CallFrame>> callFrames;
        if (GenerateCallFrames(&callFrames, false)) {
            mixedStack.SetCallFrames(std::move(callFrames));
        }
        frontend_.MixedStack(vm_, mixedStack);
    }
}

void DebuggerImpl::NotifyNativeReturn(const void *nativeAddress)
{
    if (mixStackEnabled_ && IsUserCode(nativeAddress)) {
        nativeOutPause_ = true;
    }
}

void DebuggerImpl::NotifyReturnNative()
{
    if (mixStackEnabled_) {
        tooling::MixedStack mixedStack;
        nativePointer_ = DebuggerApi::GetNativePointer(vm_);
        if (nativePointer_.empty()) {
            return;
        }
        mixedStack.SetNativePointers(nativePointer_);
        std::vector<std::unique_ptr<CallFrame>> callFrames;
        if (GenerateCallFrames(&callFrames, false)) {
            mixedStack.SetCallFrames(std::move(callFrames));
        }
        frontend_.MixedStack(vm_, mixedStack);
    }
}

// only use for test case
void DebuggerImpl::SetDebuggerState(DebuggerState debuggerState)
{
    debuggerState_ = debuggerState;
}

// only use for test case
void DebuggerImpl::SetNativeOutPause(bool nativeOutPause)
{
    nativeOutPause_ = nativeOutPause;
}

void DebuggerImpl::NotifyHandleProtocolCommand()
{
    auto *handler = vm_->GetJsDebuggerManager()->GetDebuggerHandler();
    handler->ProcessCommand();
}

// Whenever adding a new protocol which is not a standard CDP protocol,
// must add its methodName to the debuggerProtocolsList
void DebuggerImpl::InitializeExtendedProtocolsList()
{
    std::vector<std::string> debuggerProtocolList {
        "removeBreakpointsByUrl",
        "setMixedDebugEnabled",
        "replyNativeCalling",
        "getPossibleAndSetBreakpointByUrl",
        "dropFrame",
        "setNativeRange",
        "resetSingleStepper",
        "callFunctionOn",
        "smartStepInto",
        "saveAllPossibleBreakpoints",
        "setSymbolicBreakpoints",
        "removeSymbolicBreakpoints"
    };
    debuggerExtendedProtocols_ = std::move(debuggerProtocolList);
}

std::string DebuggerImpl::DispatcherImpl::GetJsFrames()
{
    std::vector<void *> nativePointers = debugger_->GetNativeAddr();
    if (nativePointers.empty()) {
        return "";
    }
    tooling::MixedStack mixedStack;
    mixedStack.SetNativePointers(nativePointers);
    std::vector<std::unique_ptr<CallFrame>> callFrames;
    if (debugger_->GenerateCallFrames(&callFrames, false)) {
        mixedStack.SetCallFrames(std::move(callFrames));
    }
    return mixedStack.ToJson()->Stringify();
}

std::vector<void *> DebuggerImpl::GetNativeAddr()
{
    return DebuggerApi::GetNativePointer(vm_);
}
std::optional<std::string> DebuggerImpl::DispatcherImpl::Dispatch(
    const DispatchRequest &request, [[maybe_unused]] bool crossLanguageDebug)
{
    Method method = GetMethodEnum(request.GetMethod());
    LOG_DEBUGGER(DEBUG) << "dispatch [" << request.GetMethod() << "] to DebuggerImpl";
    if (method == Method::CLIENT_DISCONNECT) {
        ClientDisconnect(request);
        return std::nullopt;
    }
    DispatchResponse response = DispatchResponse::Fail("unknown method: " + request.GetMethod());
    std::unique_ptr<PtBaseReturns> result = nullptr;
    switch (method) {
        case Method::CONTINUE_TO_LOCATION:
            response = ContinueToLocation(request);
            break;
        case Method::ENABLE:
            response = Enable(request, result);
            break;
        case Method::DISABLE:
            response = Disable(request);
            break;
        case Method::EVALUATE_ON_CALL_FRAME:
            response = EvaluateOnCallFrame(request, result);
            break;
        case Method::GET_POSSIBLE_BREAKPOINTS:
            response = GetPossibleBreakpoints(request, result);
            break;
        case Method::GET_SCRIPT_SOURCE:
            response = GetScriptSource(request, result);
            break;
        case Method::PAUSE:
            response = Pause(request);
            break;
        case Method::REMOVE_BREAKPOINT:
            response = RemoveBreakpoint(request);
            break;
        case Method::REMOVE_BREAKPOINTS_BY_URL:
            response = RemoveBreakpointsByUrl(request);
            break;
        case Method::RESUME:
            response = Resume(request);
            break;
        case Method::SET_ASYNC_CALL_STACK_DEPTH:
            response = SetAsyncCallStackDepth(request);
            break;
        case Method::SET_BREAKPOINT_BY_URL:
            response = SetBreakpointByUrl(request, result);
            break;
        case Method::SET_BREAKPOINTS_ACTIVE:
            response = SetBreakpointsActive(request);
            break;
        case Method::SET_PAUSE_ON_EXCEPTIONS:
            response = SetPauseOnExceptions(request);
            break;
        case Method::SET_SKIP_ALL_PAUSES:
            response = SetSkipAllPauses(request);
            break;
        case Method::STEP_INTO:
            response = StepInto(request);
            break;
        case Method::SMART_STEP_INTO:
            response = SmartStepInto(request);
            break;
        case Method::STEP_OUT:
            response = StepOut(request);
            break;
        case Method::STEP_OVER:
            response = StepOver(request);
            break;
        case Method::SET_MIXED_DEBUG_ENABLED:
            response = SetMixedDebugEnabled(request);
            break;
        case Method::SET_BLACKBOX_PATTERNS:
            response = SetBlackboxPatterns(request);
            break;
        case Method::REPLY_NATIVE_CALLING:
            response = ReplyNativeCalling(request);
            break;
        case Method::GET_POSSIBLE_AND_SET_BREAKPOINT_BY_URL:
            response = GetPossibleAndSetBreakpointByUrl(request, result);
            break;
        case Method::DROP_FRAME:
            response = DropFrame(request);
            break;
        case Method::SET_NATIVE_RANGE:
            response = SetNativeRange(request);
            break;
        case Method::RESET_SINGLE_STEPPER:
            response = ResetSingleStepper(request);
            break;
        case Method::CALL_FUNCTION_ON:
            response = CallFunctionOn(request, result);
            break;
        case Method::SAVE_ALL_POSSIBLE_BREAKPOINTS:
            response = SaveAllPossibleBreakpoints(request);
            break;
        case Method::SET_SYMBOLIC_BREAKPOINTS:
            response = SetSymbolicBreakpoints(request);
            break;
        case Method::REMOVE_SYMBOLIC_BREAKPOINTS:
            response = RemoveSymbolicBreakpoints(request);
            break;
        default:
            response = DispatchResponse::Fail("Unknown method: " + request.GetMethod());
            break;
    }
    if (crossLanguageDebug) {
        return ReturnsValueToString(request.GetCallId(), response, std::move(result));
    }
    if (result != nullptr) {
        SendResponse(request, response, *result);
    } else {
        SendResponse(request, response);
    }
    return std::nullopt;
}

DebuggerImpl::DispatcherImpl::Method DebuggerImpl::DispatcherImpl::GetMethodEnum(const std::string& method)
{
    if (method == "continueToLocation") {
        return Method::CONTINUE_TO_LOCATION;
    } else if (method == "enable") {
        return Method::ENABLE;
    } else if (method == "disable") {
        return Method::DISABLE;
    } else if (method == "evaluateOnCallFrame") {
        return Method::EVALUATE_ON_CALL_FRAME;
    } else if (method == "getPossibleBreakpoints") {
        return Method::GET_POSSIBLE_BREAKPOINTS;
    } else if (method == "getScriptSource") {
        return Method::GET_SCRIPT_SOURCE;
    } else if (method == "pause") {
        return Method::PAUSE;
    } else if (method == "removeBreakpoint") {
        return Method::REMOVE_BREAKPOINT;
    } else if (method == "removeBreakpointsByUrl") {
        return Method::REMOVE_BREAKPOINTS_BY_URL;
    } else if (method == "resume") {
        return Method::RESUME;
    } else if (method == "setAsyncCallStackDepth") {
        return Method::SET_ASYNC_CALL_STACK_DEPTH;
    } else if (method == "setBreakpointByUrl") {
        return Method::SET_BREAKPOINT_BY_URL;
    } else if (method == "setBreakpointsActive") {
        return Method::SET_BREAKPOINTS_ACTIVE;
    } else if (method == "setPauseOnExceptions") {
        return Method::SET_PAUSE_ON_EXCEPTIONS;
    } else if (method == "setSkipAllPauses") {
        return Method::SET_SKIP_ALL_PAUSES;
    } else if (method == "stepInto") {
        return Method::STEP_INTO;
    } else if (method == "smartStepInto") {
        return Method::SMART_STEP_INTO;
    } else if (method == "stepOut") {
        return Method::STEP_OUT;
    } else if (method == "stepOver") {
        return Method::STEP_OVER;
    } else if (method == "setMixedDebugEnabled") {
        return Method::SET_MIXED_DEBUG_ENABLED;
    } else if (method == "setBlackboxPatterns") {
        return Method::SET_BLACKBOX_PATTERNS;
    } else if (method == "replyNativeCalling") {
        return Method::REPLY_NATIVE_CALLING;
    } else if (method == "getPossibleAndSetBreakpointByUrl") {
        return Method::GET_POSSIBLE_AND_SET_BREAKPOINT_BY_URL;
    } else if (method == "dropFrame") {
        return Method::DROP_FRAME;
    } else if (method == "setNativeRange") {
        return Method::SET_NATIVE_RANGE;
    } else if (method == "resetSingleStepper") {
        return Method::RESET_SINGLE_STEPPER;
    } else if (method == "clientDisconnect") {
        return Method::CLIENT_DISCONNECT;
    } else if (method == "callFunctionOn") {
        return Method::CALL_FUNCTION_ON;
    } else if (method == "saveAllPossibleBreakpoints") {
        return Method::SAVE_ALL_POSSIBLE_BREAKPOINTS;
    } else if (method == "setSymbolicBreakpoints") {
        return Method::SET_SYMBOLIC_BREAKPOINTS;
    } else if (method == "removeSymbolicBreakpoints") {
        return Method::REMOVE_SYMBOLIC_BREAKPOINTS;
    } else {
        return Method::UNKNOWN;
    }
}

DispatchResponse DebuggerImpl::DispatcherImpl::ContinueToLocation(const DispatchRequest &request)
{
    std::unique_ptr<ContinueToLocationParams> params = ContinueToLocationParams::Create(request.GetParams());
    if (params == nullptr) {
        return DispatchResponse::Fail("wrong params");
    }
    DispatchResponse response = debugger_->ContinueToLocation(*params);
    return response;
}

DispatchResponse DebuggerImpl::DispatcherImpl::Enable(const DispatchRequest &request,
    std::unique_ptr<PtBaseReturns> &result)
{
    std::unique_ptr<EnableParams> params = EnableParams::Create(request.GetParams());
    if (params == nullptr) {
        return DispatchResponse::Fail("wrong params");
    }

    UniqueDebuggerId id;
    DispatchResponse response = debugger_->Enable(*params, &id);

    debugger_->InitializeExtendedProtocolsList();
    result = std::make_unique<DebuggerEnableReturns>(id, debugger_->debuggerExtendedProtocols_);
    return response;
}

DispatchResponse DebuggerImpl::DispatcherImpl::Disable(const DispatchRequest &request)
{
    DispatchResponse response = debugger_->Disable();
    return response;
}

DispatchResponse DebuggerImpl::DispatcherImpl::EvaluateOnCallFrame(const DispatchRequest &request,
    std::unique_ptr<PtBaseReturns> &result)
{
    std::unique_ptr<EvaluateOnCallFrameParams> params = EvaluateOnCallFrameParams::Create(request.GetParams());
    if (params == nullptr) {
        return DispatchResponse::Fail("wrong params");
    }
    std::unique_ptr<RemoteObject> result1;
    DispatchResponse response = debugger_->EvaluateOnCallFrame(*params, &result1);
    if (result1 == nullptr) {
        return response;
    }

    result = std::make_unique<EvaluateOnCallFrameReturns>(std::move(result1));
    return response;
}

DispatchResponse DebuggerImpl::DispatcherImpl::GetPossibleBreakpoints(const DispatchRequest &request,
    std::unique_ptr<PtBaseReturns> &result)
{
    std::unique_ptr<GetPossibleBreakpointsParams> params = GetPossibleBreakpointsParams::Create(request.GetParams());
    if (params == nullptr) {
        return DispatchResponse::Fail("wrong params");
    }
    std::vector<std::unique_ptr<BreakLocation>> locations;
    DispatchResponse response = debugger_->GetPossibleBreakpoints(*params, &locations);
    result = std::make_unique<GetPossibleBreakpointsReturns>(std::move(locations));
    return response;
}

DispatchResponse DebuggerImpl::DispatcherImpl::GetScriptSource(const DispatchRequest &request,
    std::unique_ptr<PtBaseReturns> &result)
{
    std::unique_ptr<GetScriptSourceParams> params = GetScriptSourceParams::Create(request.GetParams());
    if (params == nullptr) {
        return DispatchResponse::Fail("wrong params");
    }
    std::string source;
    DispatchResponse response = debugger_->GetScriptSource(*params, &source);
    result = std::make_unique<GetScriptSourceReturns>(source);
    return response;
}

DispatchResponse DebuggerImpl::DispatcherImpl::Pause(const DispatchRequest &request)
{
    DispatchResponse response = debugger_->Pause();
    return response;
}

DispatchResponse DebuggerImpl::DispatcherImpl::RemoveBreakpoint(const DispatchRequest &request)
{
    std::unique_ptr<RemoveBreakpointParams> params = RemoveBreakpointParams::Create(request.GetParams());
    if (params == nullptr) {
        return DispatchResponse::Fail("wrong params");
    }
    DispatchResponse response = debugger_->RemoveBreakpoint(*params);
    return response;
}

DispatchResponse DebuggerImpl::DispatcherImpl::RemoveBreakpointsByUrl(const DispatchRequest &request)
{
    std::unique_ptr<RemoveBreakpointsByUrlParams> params = RemoveBreakpointsByUrlParams::Create(request.GetParams());
    if (params == nullptr) {
        return DispatchResponse::Fail("wrong params");
    }
    DispatchResponse response = debugger_->RemoveBreakpointsByUrl(*params);
    return response;
}

DispatchResponse DebuggerImpl::DispatcherImpl::Resume(const DispatchRequest &request)
{
    std::unique_ptr<ResumeParams> params = ResumeParams::Create(request.GetParams());
    if (params == nullptr) {
        return DispatchResponse::Fail("wrong params");
    }
    DispatchResponse response = debugger_->Resume(*params);
    return response;
}

DispatchResponse DebuggerImpl::DispatcherImpl::SetBreakpointByUrl(const DispatchRequest &request,
    std::unique_ptr<PtBaseReturns> &result)
{
    std::unique_ptr<SetBreakpointByUrlParams> params = SetBreakpointByUrlParams::Create(request.GetParams());
    if (params == nullptr) {
        return DispatchResponse::Fail("wrong params");
    }

    std::string outId;
    std::vector<std::unique_ptr<Location>> outLocations;
    DispatchResponse response = debugger_->SetBreakpointByUrl(*params, &outId, &outLocations);
    result = std::make_unique<SetBreakpointByUrlReturns>(outId, std::move(outLocations));
    return response;
}

DispatchResponse DebuggerImpl::DispatcherImpl::SetBreakpointsActive(const DispatchRequest &request)
{
    std::unique_ptr<SetBreakpointsActiveParams> params = SetBreakpointsActiveParams::Create(request.GetParams());
    if (params == nullptr) {
        return DispatchResponse::Fail("wrong params");
    }

    DispatchResponse response = debugger_->SetBreakpointsActive(*params);
    return response;
}

DispatchResponse DebuggerImpl::DispatcherImpl::GetPossibleAndSetBreakpointByUrl(const DispatchRequest &request,
    std::unique_ptr<PtBaseReturns> &result)
{
    std::unique_ptr<GetPossibleAndSetBreakpointParams> params =
        GetPossibleAndSetBreakpointParams::Create(request.GetParams());
    if (params == nullptr) {
        return DispatchResponse::Fail("wrong params");
    }

    std::vector<std::shared_ptr<BreakpointReturnInfo>> outLocation;
    DispatchResponse response = debugger_->GetPossibleAndSetBreakpointByUrl(*params, outLocation);
    result = std::make_unique<GetPossibleAndSetBreakpointByUrlReturns>(std::move(outLocation));
    return response;
}

DispatchResponse DebuggerImpl::DispatcherImpl::SaveAllPossibleBreakpoints(const DispatchRequest &request)
{
    std::unique_ptr<SaveAllPossibleBreakpointsParams> params =
        SaveAllPossibleBreakpointsParams::Create(request.GetParams());
    if (params == nullptr) {
        return DispatchResponse::Fail("wrong params");
    }
    DispatchResponse response = debugger_->SaveAllPossibleBreakpoints(*params);
    return response;
}

DispatchResponse DebuggerImpl::DispatcherImpl::SetSymbolicBreakpoints(const DispatchRequest &request)
{
    std::unique_ptr<SetSymbolicBreakpointsParams> params =
        SetSymbolicBreakpointsParams::Create(request.GetParams());
    if (params == nullptr) {
        return DispatchResponse::Fail("wrong params");
    }
    DispatchResponse response = debugger_->SetSymbolicBreakpoints(*params);
    return response;
}

DispatchResponse DebuggerImpl::DispatcherImpl::RemoveSymbolicBreakpoints(const DispatchRequest &request)
{
    std::unique_ptr<RemoveSymbolicBreakpointsParams> params =
        RemoveSymbolicBreakpointsParams::Create(request.GetParams());
    if (params == nullptr) {
        return DispatchResponse::Fail("wrong params");
    }
    DispatchResponse response = debugger_->RemoveSymbolicBreakpoints(*params);
    return response;
}

DispatchResponse DebuggerImpl::DispatcherImpl::SetAsyncCallStackDepth(const DispatchRequest &request)
{
    std::unique_ptr<SetAsyncCallStackDepthParams> params =
        SetAsyncCallStackDepthParams::Create(request.GetParams());
    if (params == nullptr) {
        return DispatchResponse::Fail("wrong params");
    }
    DispatchResponse response = debugger_->SetAsyncCallStackDepth(*params);
    return response;
}

DispatchResponse DebuggerImpl::DispatcherImpl::SetPauseOnExceptions(const DispatchRequest &request)
{
    std::unique_ptr<SetPauseOnExceptionsParams> params = SetPauseOnExceptionsParams::Create(request.GetParams());
    if (params == nullptr) {
        return DispatchResponse::Fail("wrong params");
    }

    DispatchResponse response = debugger_->SetPauseOnExceptions(*params);
    return response;
}

DispatchResponse DebuggerImpl::DispatcherImpl::SetSkipAllPauses(const DispatchRequest &request)
{
    std::unique_ptr<SetSkipAllPausesParams> params = SetSkipAllPausesParams::Create(request.GetParams());
    if (params == nullptr) {
        return DispatchResponse::Fail("wrong params");
    }

    DispatchResponse response = debugger_->SetSkipAllPauses(*params);
    return response;
}

DispatchResponse DebuggerImpl::DispatcherImpl::SetNativeRange(const DispatchRequest &request)
{
    std::unique_ptr<SetNativeRangeParams> params = SetNativeRangeParams::Create(request.GetParams());
    if (params == nullptr) {
        return DispatchResponse::Fail("wrong params");
    }
    DispatchResponse response = debugger_->SetNativeRange(*params);
    return response;
}

DispatchResponse DebuggerImpl::DispatcherImpl::ResetSingleStepper(const DispatchRequest &request)
{
    std::unique_ptr<ResetSingleStepperParams> params = ResetSingleStepperParams::Create(request.GetParams());
    if (params == nullptr) {
        return DispatchResponse::Fail("wrong params");
    }
    DispatchResponse response = debugger_->ResetSingleStepper(*params);
    return response;
}

DispatchResponse DebuggerImpl::DispatcherImpl::StepInto(const DispatchRequest &request)
{
    std::unique_ptr<StepIntoParams> params = StepIntoParams::Create(request.GetParams());
    if (params == nullptr) {
        return DispatchResponse::Fail("wrong params");
    }
    DispatchResponse response = debugger_->StepInto(*params);
    return response;
}

DispatchResponse DebuggerImpl::DispatcherImpl::SmartStepInto(const DispatchRequest &request)
{
    std::unique_ptr<SmartStepIntoParams> params = SmartStepIntoParams::Create(request.GetParams());
    if (params == nullptr) {
        return DispatchResponse::Fail("wrong params");
    }
    DispatchResponse response = debugger_->SmartStepInto(*params);
    return response;
}

DispatchResponse DebuggerImpl::DispatcherImpl::StepOut(const DispatchRequest &request)
{
    DispatchResponse response = debugger_->StepOut();
    return response;
}

DispatchResponse DebuggerImpl::DispatcherImpl::StepOver(const DispatchRequest &request)
{
    std::unique_ptr<StepOverParams> params = StepOverParams::Create(request.GetParams());
    if (params == nullptr) {
        return DispatchResponse::Fail("wrong params");
    }
    DispatchResponse response = debugger_->StepOver(*params);
    return response;
}

DispatchResponse DebuggerImpl::DispatcherImpl::SetMixedDebugEnabled(const DispatchRequest &request)
{
    std::unique_ptr<SetMixedDebugParams> params = SetMixedDebugParams::Create(request.GetParams());
    if (params == nullptr) {
        return DispatchResponse::Fail("wrong params");
    }
    DispatchResponse response = debugger_->SetMixedDebugEnabled(*params);
    return response;
}

DispatchResponse DebuggerImpl::DispatcherImpl::ReplyNativeCalling(const DispatchRequest &request)
{
    std::unique_ptr<ReplyNativeCallingParams> params = ReplyNativeCallingParams::Create(request.GetParams());
    if (params == nullptr) {
        return DispatchResponse::Fail("wrong params");
    }
    DispatchResponse response = debugger_->ReplyNativeCalling(*params);
    return response;
}

DispatchResponse DebuggerImpl::DispatcherImpl::SetBlackboxPatterns(const DispatchRequest &request)
{
    DispatchResponse response = debugger_->SetBlackboxPatterns();
    return response;
}

DispatchResponse DebuggerImpl::DispatcherImpl::DropFrame(const DispatchRequest &request)
{
    std::unique_ptr<DropFrameParams> params = DropFrameParams::Create(request.GetParams());
    if (params == nullptr) {
        return DispatchResponse::Fail("wrong params");
    }
    DispatchResponse response = debugger_->DropFrame(*params);
    return response;
}

// inner message, not SendResponse to outer
void DebuggerImpl::DispatcherImpl::ClientDisconnect([[maybe_unused]] const DispatchRequest &request)
{
    if (GetJsDebuggerManager()->IsDebugMode() && GetJsDebuggerManager()->IsDebugApp()) {
        debugger_->Disable();
    }
}

DispatchResponse DebuggerImpl::DispatcherImpl::CallFunctionOn(const DispatchRequest &request,
    std::unique_ptr<PtBaseReturns> &result)
{
    std::unique_ptr<CallFunctionOnParams> params = CallFunctionOnParams::Create(request.GetParams());
    if (params == nullptr) {
        return DispatchResponse::Fail("wrong params");
    }

    std::unique_ptr<RemoteObject> outRemoteObject;
    std::optional<std::unique_ptr<ExceptionDetails>> outExceptionDetails;
    DispatchResponse response = debugger_->CallFunctionOn(*params, &outRemoteObject, &outExceptionDetails);
    if (outExceptionDetails) {
        ASSERT(outExceptionDetails.value() != nullptr);
        LOG_DEBUGGER(WARN) << "CallFunctionOn thrown an exception";
    }
    if (outRemoteObject == nullptr) {
        return response;
    }

    result = std::make_unique<CallFunctionOnReturns>(std::move(outRemoteObject), std::move(outExceptionDetails));
    return response;
}

bool DebuggerImpl::Frontend::AllowNotify(const EcmaVM *vm) const
{
    return vm->GetJsDebuggerManager()->IsDebugMode() && channel_ != nullptr;
}

void DebuggerImpl::Frontend::BreakpointResolved(const EcmaVM *vm)
{
    if (!AllowNotify(vm)) {
        return;
    }

    tooling::BreakpointResolved breakpointResolved;
    channel_->SendNotification(breakpointResolved);
}

void DebuggerImpl::Frontend::Paused(const EcmaVM *vm, const tooling::Paused &paused)
{
    if (!AllowNotify(vm)) {
        return;
    }

    channel_->SendNotification(paused);
}

void DebuggerImpl::Frontend::NativeCalling(const EcmaVM *vm, const tooling::NativeCalling &nativeCalling)
{
    if (!AllowNotify(vm)) {
        return;
    }

    channel_->SendNotification(nativeCalling);
}

void DebuggerImpl::Frontend::MixedStack(const EcmaVM *vm, const tooling::MixedStack &mixedStack)
{
    if (!AllowNotify(vm)) {
        return;
    }

    channel_->SendNotification(mixedStack);
}

void DebuggerImpl::Frontend::Resumed(const EcmaVM *vm)
{
    if (!AllowNotify(vm)) {
        return;
    }

    channel_->RunIfWaitingForDebugger();
    tooling::Resumed resumed;
    channel_->SendNotification(resumed);
}

void DebuggerImpl::Frontend::ScriptFailedToParse(const EcmaVM *vm)
{
    if (!AllowNotify(vm)) {
        return;
    }

    tooling::ScriptFailedToParse scriptFailedToParse;
    channel_->SendNotification(scriptFailedToParse);
}

void DebuggerImpl::Frontend::ScriptParsed(const EcmaVM *vm, const PtScript &script)
{
    if (!AllowNotify(vm)) {
        return;
    }

    tooling::ScriptParsed scriptParsed;
    scriptParsed.SetScriptId(script.GetScriptId())
        .SetUrl(script.GetUrl())
        .SetStartLine(0)
        .SetStartColumn(0)
        .SetEndLine(script.GetEndLine())
        .SetEndColumn(0)
        .SetExecutionContextId(0)
        .SetHash(script.GetHash())
        .SetLocations(script.GetLocations());

    channel_->SendNotification(scriptParsed);
}

void DebuggerImpl::Frontend::WaitForDebugger(const EcmaVM *vm)
{
    if (!AllowNotify(vm)) {
        return;
    }

    channel_->WaitForDebugger();
}

void DebuggerImpl::Frontend::RunIfWaitingForDebugger([[maybe_unused]] const EcmaVM *vm)
{
    // Because release hap can WaitForDebugger, need RunIfWaitingForDebugger to run continue.
    // But release hap debugMode is false, so not check debugMode.
    if (channel_ == nullptr) {
        return;
    }

    channel_->RunIfWaitingForDebugger();
}

DispatchResponse DebuggerImpl::ContinueToLocation(const ContinueToLocationParams &params)
{
    location_ = *params.GetLocation();
    return DispatchResponse::Ok();
}

DispatchResponse DebuggerImpl::Enable([[maybe_unused]] const EnableParams &params, UniqueDebuggerId *id)
{
    DebuggerExecutor::Initialize(vm_);
    ASSERT(id != nullptr);
    *id = 0;
    vm_->GetJsDebuggerManager()->SetDebugMode(true);
    // Enable corresponding features requested by IDE
    EnableDebuggerFeatures(params);
    for (auto &script : scripts_) {
        frontend_.ScriptParsed(vm_, *script.second);
    }
    debuggerState_ = DebuggerState::ENABLED;
    return DispatchResponse::Ok();
}

void DebuggerImpl::EnableDebuggerFeatures(const EnableParams &params)
{
    if (!params.HasEnableOptionsList()) {
        return;
    }
    auto enableOptionsList = params.GetEnableOptionsList();
    if (enableOptionsList.empty()) {
        return;
    }
    for (auto &option : enableOptionsList) {
        LOG_DEBUGGER(INFO) << "Debugger feature " << option << " is enabled";
        EnableFeature(GetDebuggerFeatureEnum(option));
    }
}

DebuggerFeature DebuggerImpl::GetDebuggerFeatureEnum(std::string &option)
{
    if (option == "enableLaunchAccelerate") {
        return DebuggerFeature::LAUNCH_ACCELERATE;
    }
    // Future features could be added here to parse as DebuggerFeatureEnum
    return DebuggerFeature::UNKNOWN;
}

void DebuggerImpl::EnableFeature(DebuggerFeature feature)
{
    switch (feature) {
        case DebuggerFeature::LAUNCH_ACCELERATE:
            EnableLaunchAccelerateMode();
            DebuggerApi::DisableFirstTimeFlag(jsDebugger_);
            break;
        default:
            break;
    }
}

DispatchResponse DebuggerImpl::Disable()
{
    DebuggerApi::RemoveAllBreakpoints(jsDebugger_);
    frontend_.RunIfWaitingForDebugger(vm_);
    frontend_.Resumed(vm_);
    vm_->GetJsDebuggerManager()->SetDebugMode(false);
    debuggerState_ = DebuggerState::DISABLED;
    return DispatchResponse::Ok();
}

DispatchResponse DebuggerImpl::EvaluateOnCallFrame(const EvaluateOnCallFrameParams &params,
                                                   std::unique_ptr<RemoteObject> *result)
{
    CallFrameId callFrameId = params.GetCallFrameId();
    const std::string &expression = params.GetExpression();
    if (callFrameId < 0 || callFrameId >= static_cast<CallFrameId>(callFrameHandlers_.size())) {
        return DispatchResponse::Fail("Invalid callFrameId.");
    }

    std::vector<uint8_t> dest;
    if (!DecodeAndCheckBase64(expression, dest)) {
        LOG_DEBUGGER(ERROR) << "EvaluateValue: base64 decode failed";
        auto ret = CmptEvaluateValue(callFrameId, expression, result);
        if (ret.has_value()) {
            LOG_DEBUGGER(ERROR) << "Evaluate fail, expression: " << expression;
        }
        return DispatchResponse::Create(ret);
    }

    Local<JSValueRef> currentContext = DebuggerApi::GetCurrentGlobalEnv(vm_);
    Local<JSValueRef> originContext = JSNApi::GetCurrentContext(vm_);
    JSNApi::SwitchContext(vm_, currentContext);

    auto funcRef = DebuggerApi::GenerateFuncFromBuffer(vm_, dest.data(), dest.size(),
        JSPandaFile::ENTRY_FUNCTION_NAME);
    auto res = DebuggerApi::EvaluateViaFuncCall(const_cast<EcmaVM *>(vm_), funcRef,
        callFrameHandlers_[callFrameId]);

    JSNApi::SwitchContext(vm_, originContext);

    if (vm_->GetJSThread()->HasPendingException()) {
        LOG_DEBUGGER(ERROR) << "EvaluateValue: has pending exception";
        std::string msg;
        DebuggerApi::HandleUncaughtException(vm_, msg);
        *result = RemoteObject::FromTagged(vm_,
            Exception::EvalError(vm_, StringRef::NewFromUtf8(vm_, msg.data())));
        return DispatchResponse::Fail(msg);
    }

    *result = RemoteObject::FromTagged(vm_, res);
    runtime_->CacheObjectIfNeeded(res, (*result).get());
    return DispatchResponse::Ok();
}

DispatchResponse DebuggerImpl::GetPossibleBreakpoints(const GetPossibleBreakpointsParams &params,
                                                      std::vector<std::unique_ptr<BreakLocation>> *locations)
{
    Location *start = params.GetStart();
    auto iter = scripts_.find(start->GetScriptId());
    if (iter == scripts_.end()) {
        return DispatchResponse::Fail("Unknown file name.");
    }
    const std::string &url = iter->second->GetUrl();
    std::vector<DebugInfoExtractor *> extractors = GetExtractors(url);
    for (auto extractor : extractors) {
        if (extractor == nullptr) {
            LOG_DEBUGGER(DEBUG) << "GetPossibleBreakpoints: extractor is null";
            continue;
        }

        int32_t line = start->GetLine();
        int32_t column = start->GetColumn();
        auto callbackFunc = [](const JSPtLocation &) -> bool {
            return true;
        };
        if (extractor->MatchWithLocation(callbackFunc, line, column, url, GetRecordName(url))) {
            std::unique_ptr<BreakLocation> location = std::make_unique<BreakLocation>();
            location->SetScriptId(start->GetScriptId()).SetLine(line).SetColumn(column);
            locations->emplace_back(std::move(location));
            break;
        }
    }
    return DispatchResponse::Ok();
}

DispatchResponse DebuggerImpl::GetScriptSource(const GetScriptSourceParams &params, std::string *source)
{
    ScriptId scriptId = params.GetScriptId();
    auto iter = scripts_.find(scriptId);
    if (iter == scripts_.end()) {
        *source = "";
        return DispatchResponse::Fail("unknown script id: " + std::to_string(scriptId));
    }
    *source = iter->second->GetScriptSource();

    return DispatchResponse::Ok();
}

DispatchResponse DebuggerImpl::Pause()
{
    if (debuggerState_ == DebuggerState::PAUSED) {
        return DispatchResponse::Fail("Can only perform operation while running");
    }
    pauseOnNextByteCode_ = true;
    return DispatchResponse::Ok();
}

DispatchResponse DebuggerImpl::RemoveBreakpoint(const RemoveBreakpointParams &params)
{
    std::string id = params.GetBreakpointId();
    LOG_DEBUGGER(INFO) << "RemoveBreakpoint: " << id;
    BreakpointDetails metaData{};
    if (!BreakpointDetails::ParseBreakpointId(id, &metaData)) {
        return DispatchResponse::Fail("Parse breakpoint id failed");
    }

    auto scriptFunc = [](PtScript *) -> bool {
        return true;
    };
    if (!MatchScripts(scriptFunc, metaData.url_, ScriptMatchType::URL)) {
        LOG_DEBUGGER(ERROR) << "RemoveBreakpoint: Unknown url: " << metaData.url_;
        return DispatchResponse::Fail("Unknown file name.");
    }

    std::vector<DebugInfoExtractor *> extractors = GetExtractors(metaData.url_);
    for (auto extractor : extractors) {
        if (extractor == nullptr) {
            LOG_DEBUGGER(DEBUG) << "RemoveBreakpoint: extractor is null";
            continue;
        }

        auto callbackFunc = [this](const JSPtLocation &location) -> bool {
            LOG_DEBUGGER(INFO) << "remove breakpoint location: " << location.ToString();
            return DebuggerApi::RemoveBreakpoint(jsDebugger_, location);
        };
        if (!extractor->MatchWithLocation(callbackFunc, metaData.line_, metaData.column_,
            metaData.url_, GetRecordName(metaData.url_))) {
            LOG_DEBUGGER(ERROR) << "failed to remove breakpoint location number: "
                << metaData.line_ << ":" << metaData.column_;
        }
    }

    LOG_DEBUGGER(INFO) << "remove breakpoint line number:" << metaData.line_;
    return DispatchResponse::Ok();
}

DispatchResponse DebuggerImpl::RemoveBreakpointsByUrl(const RemoveBreakpointsByUrlParams &params)
{
    std::string url = params.GetUrl();
    auto scriptMatchCallback = [](PtScript *) -> bool {
        return true;
    };
    if (!MatchScripts(scriptMatchCallback, url, ScriptMatchType::URL)) {
        if (IsLaunchAccelerateMode() && breakpointPendingMap_.erase(url)) {
            LOG_DEBUGGER(INFO) << "All breakpoints on " << url << " are removed";
            return DispatchResponse::Ok();
        }
        LOG_DEBUGGER(ERROR) << "RemoveBreakpointByUrl: Unknown url: " << url;
        return DispatchResponse::Fail("Unknown url");
    }
    if (!DebuggerApi::RemoveBreakpointsByUrl(jsDebugger_, url)) {
        return DispatchResponse::Fail("RemoveBreakpointByUrl failed");
    }

    LOG_DEBUGGER(INFO) << "All breakpoints on " << url << " are removed";
    if (IsLaunchAccelerateMode()) {
        breakpointPendingMap_.erase(url);
    }
    return DispatchResponse::Ok();
}

DispatchResponse DebuggerImpl::Resume([[maybe_unused]] const ResumeParams &params)
{
    if (debuggerState_ != DebuggerState::PAUSED) {
        return DispatchResponse::Fail("Can only perform operation while paused");
    }
    frontend_.Resumed(vm_);
    debuggerState_ = DebuggerState::ENABLED;
    return DispatchResponse::Ok();
}

void DebuggerImpl::AddBreakpointDetail(const std::string &url,
                                       int32_t lineNumber,
                                       std::string *outId,
                                       std::vector<std::unique_ptr<Location>> *outLocations)
{
    std::vector<PtScript *> ptScripts = MatchAllScripts(url);
    for (auto ptScript : ptScripts) {
        ScriptId scriptId = ptScript->GetScriptId();
        std::unique_ptr<Location> location = std::make_unique<Location>();
        location->SetScriptId(scriptId).SetLine(lineNumber).SetColumn(0);
        outLocations->emplace_back(std::move(location));
    }
    BreakpointDetails metaData{lineNumber, 0, url};
    *outId = BreakpointDetails::ToString(metaData);
}


DispatchResponse DebuggerImpl::SetBreakpointByUrl(const SetBreakpointByUrlParams &params,
                                                  std::string *outId,
                                                  std::vector<std::unique_ptr<Location>> *outLocations,
                                                  bool isSmartBreakpoint)
{
    if (!vm_->GetJsDebuggerManager()->IsDebugMode()) {
        return DispatchResponse::Fail("SetBreakpointByUrl: debugger agent is not enabled");
    }
    const std::string &url = params.GetUrl();
    int32_t lineNumber = params.GetLine();
    // it is not support column breakpoint now, so columnNumber is not useful
    int32_t columnNumber = -1;
    auto condition = params.HasCondition() ? params.GetCondition() : std::optional<std::string> {};
    *outLocations = std::vector<std::unique_ptr<Location>>();

    auto scriptFunc = [](PtScript *) -> bool {
        return true;
    };
    if (!MatchScripts(scriptFunc, url, ScriptMatchType::URL)) {
        LOG_DEBUGGER(ERROR) << "SetBreakpointByUrl: Unknown url: " << url;
        return DispatchResponse::Fail("Unknown file name.");
    }

    std::vector<DebugInfoExtractor *> extractors = GetExtractors(url);
    for (auto extractor : extractors) {
        if (extractor == nullptr) {
            LOG_DEBUGGER(DEBUG) << "SetBreakpointByUrl: extractor is null";
            continue;
        }

        auto callbackFunc = [this, &condition, &isSmartBreakpoint](const JSPtLocation &location) -> bool {
            LOG_DEBUGGER(INFO) << "set breakpoint location: " << location.ToString();
            Local<FunctionRef> condFuncRef = FunctionRef::Undefined(vm_);
            if (condition.has_value() && !condition.value().empty()) {
                condFuncRef = CheckAndGenerateCondFunc(condition);
                if (condFuncRef->IsUndefined()) {
                    LOG_DEBUGGER(ERROR) << "SetBreakpointByUrl: generate function failed";
                    return false;
                }
            }
            return DebuggerApi::SetBreakpoint(jsDebugger_, location, condFuncRef, isSmartBreakpoint);
        };
        if (!extractor->MatchWithLocation(callbackFunc, lineNumber, columnNumber, url, GetRecordName(url))) {
            LOG_DEBUGGER(ERROR) << "failed to set breakpoint location number: "
                << lineNumber << ":" << columnNumber;
            return DispatchResponse::Fail("Breakpoint not found.");
        }
    }
    AddBreakpointDetail(url, lineNumber, outId, outLocations);
    return DispatchResponse::Ok();
}

DispatchResponse DebuggerImpl::SetBreakpointsActive(const SetBreakpointsActiveParams &params)
{
    breakpointsState_ = params.GetBreakpointsState();
    return DispatchResponse::Ok();
}

DispatchResponse DebuggerImpl::GetPossibleAndSetBreakpointByUrl(const GetPossibleAndSetBreakpointParams &params,
    std::vector<std::shared_ptr<BreakpointReturnInfo>> &outLocations)
{
    if (!vm_->GetJsDebuggerManager()->IsDebugMode()) {
        return DispatchResponse::Fail("GetPossibleAndSetBreakpointByUrl: debugger agent is not enabled");
    }
    if (!params.HasBreakpointsList()) {
        return DispatchResponse::Fail("GetPossibleAndSetBreakpointByUrl: no pennding breakpoint exists");
    }
    auto breakpointList = params.GetBreakpointsList();
    for (const auto &breakpoint : *breakpointList) {
        if (!ProcessSingleBreakpoint(*breakpoint, outLocations)) {
            std::string invalidBpId = "invalid";
            std::shared_ptr<BreakpointReturnInfo> bpInfo = std::make_shared<BreakpointReturnInfo>();
            bpInfo->SetId(invalidBpId)
                .SetLineNumber(breakpoint->GetLineNumber())
                .SetColumnNumber(breakpoint->GetColumnNumber());
            outLocations.emplace_back(bpInfo);
        }
        // Insert this bp into bp pending map
        if (IsLaunchAccelerateMode()) {
            InsertIntoPendingBreakpoints(*breakpoint);
        }
    }
    return DispatchResponse::Ok();
}

bool DebuggerImpl::InsertIntoPendingBreakpoints(const BreakpointInfo &breakpoint)
{
    auto condition = breakpoint.HasCondition() ? breakpoint.GetCondition() : std::optional<std::string> {};
    auto bpShared = BreakpointInfo::CreateAsSharedPtr(breakpoint.GetLineNumber(), breakpoint.GetColumnNumber(),
        breakpoint.GetUrl(), (condition.has_value() ? condition.value() : ""));
    if (breakpointPendingMap_.empty() ||
            breakpointPendingMap_.find(breakpoint.GetUrl()) == breakpointPendingMap_.end()) {
        CUnorderedSet<std::shared_ptr<BreakpointInfo>, HashBreakpointInfo> set {};
        set.insert(bpShared);
        breakpointPendingMap_[breakpoint.GetUrl()] = set;
        return true;
    }
    return (breakpointPendingMap_[breakpoint.GetUrl()].insert(bpShared)).second;
}

DispatchResponse DebuggerImpl::SaveAllPossibleBreakpoints(const SaveAllPossibleBreakpointsParams &params)
{
    if (!vm_->GetJsDebuggerManager()->IsDebugMode()) {
        return DispatchResponse::Fail("SaveAllPossibleBreakpoints: debugger agent is not enabled");
    }
    if (!IsLaunchAccelerateMode()) {
        return DispatchResponse::Fail("SaveAllPossibleBreakpoints: protocol is not enabled");
    }
    if (!params.HasBreakpointsMap()) {
        return DispatchResponse::Fail("SaveAllPossibleBreakpoints: no pending breakpoint exists");
    }
    SavePendingBreakpoints(params);
    return DispatchResponse::Ok();
}

DispatchResponse DebuggerImpl::SetSymbolicBreakpoints(const SetSymbolicBreakpointsParams &params)
{
    if (!vm_->GetJsDebuggerManager()->IsDebugMode()) {
        return DispatchResponse::Fail("SetSymbolicBreakpoints: debugger agent is not enabled");
    }
    if (!params.HasSymbolicBreakpoints()) {
        return DispatchResponse::Fail("SetSymbolicBreakpoints: no symbolicBreakpoints exists");
    }
    // Symbolic breakpoints support only function names
    DebuggerApi::SetSymbolicBreakpoint(jsDebugger_, *(params.GetFunctionNamesSet()));

    return DispatchResponse::Ok();
}

DispatchResponse DebuggerImpl::RemoveSymbolicBreakpoints(const RemoveSymbolicBreakpointsParams &params)
{
    if (!vm_->GetJsDebuggerManager()->IsDebugMode()) {
        return DispatchResponse::Fail("RemoveSymbolicBreakpoints: debugger agent is not enabled");
    }
    if (!params.HasSymbolicBreakpoints()) {
        return DispatchResponse::Fail("RemoveSymbolicBreakpoints: no symbolBreakpoints removed");
    }
    // Symbolic breakpoints support only function names
    for (const auto& symbolicBreakpoint : *(params.GetFunctionNamesSet())) {
        DebuggerApi::RemoveSymbolicBreakpoint(jsDebugger_, symbolicBreakpoint);
    }
    return DispatchResponse::Ok();
}

DispatchResponse DebuggerImpl::SetAsyncCallStackDepth(const SetAsyncCallStackDepthParams &params)
{
    maxAsyncCallChainDepth_ = params.GetMaxDepth();
    if (maxAsyncCallChainDepth_) {
        // enable async stack trace
        vm_->GetJsDebuggerManager()->SetAsyncStackTrace(true);
    }
    return DispatchResponse::Ok();
}

void DebuggerImpl::SavePendingBreakpoints(const SaveAllPossibleBreakpointsParams &params)
{
    for (const auto &entry : *(params.GetBreakpointsMap())) {
        if (breakpointPendingMap_.find(entry.first) == breakpointPendingMap_.end()) {
            CUnorderedSet<std::shared_ptr<BreakpointInfo>, HashBreakpointInfo> set {};
            for (auto &info : entry.second) {
                set.insert(info);
            }
            breakpointPendingMap_[entry.first] = set;
        } else {
            for (auto &info : entry.second) {
                breakpointPendingMap_[entry.first].insert(info);
            }
        }
    }
}

bool DebuggerImpl::ProcessSingleBreakpoint(const BreakpointInfo &breakpoint,
                                           std::vector<std::shared_ptr<BreakpointReturnInfo>> &outLocations)
{
    const std::string &url = breakpoint.GetUrl();
    int32_t lineNumber = breakpoint.GetLineNumber();
    // it is not support column breakpoint now, so columnNumber is not useful
    int32_t columnNumber = -1;
    auto condition = breakpoint.HasCondition() ? breakpoint.GetCondition() : std::optional<std::string> {};

    ScriptId scriptId;
    auto scriptFunc = [&scriptId](PtScript *script) -> bool {
        scriptId = script->GetScriptId();
        return true;
    };
    if (!MatchScripts(scriptFunc, url, ScriptMatchType::URL)) {
        LOG_DEBUGGER(ERROR) << "GetPossibleAndSetBreakpointByUrl: Unknown url: " << url;
        return false;
    }

    std::vector<DebugInfoExtractor *> extractors = GetExtractors(url);
    for (auto extractor : extractors) {
        if (extractor == nullptr) {
            LOG_DEBUGGER(DEBUG) << "GetPossibleAndSetBreakpointByUrl: extractor is null";
            continue;
        }
        // decode and convert condition to function before doing matchWithLocation
        Local<FunctionRef> funcRef = FunctionRef::Undefined(vm_);
        if (condition.has_value() && !condition.value().empty()) {
            funcRef = CheckAndGenerateCondFunc(condition);
            if (funcRef->IsUndefined()) {
                LOG_DEBUGGER(ERROR) << "GetPossibleAndSetBreakpointByUrl: generate function failed";
                return false;
            }
        }
        auto matchLocationCbFunc = [this, &funcRef](const JSPtLocation &location) -> bool {
            return DebuggerApi::SetBreakpoint(jsDebugger_, location, funcRef);
        };
        if (!extractor->MatchWithLocation(matchLocationCbFunc, lineNumber, columnNumber, url, GetRecordName(url))) {
            LOG_DEBUGGER(ERROR) << "failed to set breakpoint location number: " << lineNumber << ":" << columnNumber;
            return false;
        }
    }
    
    BreakpointDetails bpMetaData {lineNumber, 0, url};
    std::string outId = BreakpointDetails::ToString(bpMetaData);
    std::shared_ptr<BreakpointReturnInfo> bpInfo = std::make_unique<BreakpointReturnInfo>();
    bpInfo->SetScriptId(scriptId).SetLineNumber(lineNumber).SetColumnNumber(0).SetId(outId);
    outLocations.emplace_back(bpInfo);

    return true;
}

DispatchResponse DebuggerImpl::SetNativeRange(const SetNativeRangeParams &params)
{
    nativeRanges_ = params.GetNativeRange();
    return DispatchResponse::Ok();
}

DispatchResponse DebuggerImpl::ResetSingleStepper(const ResetSingleStepperParams &params)
{
    // if JS to C++ and C++ has breakpoint; it need to clear singleStepper_
    if (params.GetResetSingleStepper()) {
        singleStepper_.reset();
    }
    return DispatchResponse::Ok();
}

DispatchResponse DebuggerImpl::SetPauseOnExceptions(const SetPauseOnExceptionsParams &params)
{
    pauseOnException_ = params.GetState();
    return DispatchResponse::Ok();
}

DispatchResponse DebuggerImpl::SetSkipAllPauses(const SetSkipAllPausesParams &params)
{
    skipAllPausess_ = params.GetSkipAllPausesState();
    return DispatchResponse::Ok();
}

DispatchResponse DebuggerImpl::StepInto([[maybe_unused]] const StepIntoParams &params)
{
    if (debuggerState_ != DebuggerState::PAUSED) {
        return DispatchResponse::Fail("Can only perform operation while paused");
    }
    singleStepper_ = SingleStepper::GetStepIntoStepper(vm_);
    if (singleStepper_ == nullptr) {
        LOG_DEBUGGER(ERROR) << "StepInto: singleStepper is null";
        return DispatchResponse::Fail("Failed to StepInto");
    }
    frontend_.Resumed(vm_);
    debuggerState_ = DebuggerState::ENABLED;
    return DispatchResponse::Ok();
}

DispatchResponse DebuggerImpl::SmartStepInto(const SmartStepIntoParams &params)
{
    if (debuggerState_ != DebuggerState::PAUSED) {
        return DispatchResponse::Fail("Can only perform operation while paused");
    }
    [[maybe_unused]] std::string outId;
    [[maybe_unused]] std::vector<std::unique_ptr<Location>> outLocations;
    return SetBreakpointByUrl(*(params.GetSetBreakpointByUrlParams()), &outId, &outLocations, true);
}

DispatchResponse DebuggerImpl::StepOut()
{
    if (debuggerState_ != DebuggerState::PAUSED) {
        return DispatchResponse::Fail("Can only perform operation while paused");
    }
    singleStepper_ = SingleStepper::GetStepOutStepper(vm_);
    if (singleStepper_ == nullptr) {
        LOG_DEBUGGER(ERROR) << "StepOut: singleStepper is null";
        return DispatchResponse::Fail("Failed to StepOut");
    }
    frontend_.Resumed(vm_);
    debuggerState_ = DebuggerState::ENABLED;
    return DispatchResponse::Ok();
}

DispatchResponse DebuggerImpl::StepOver([[maybe_unused]] const StepOverParams &params)
{
    if (debuggerState_ != DebuggerState::PAUSED) {
        return DispatchResponse::Fail("Can only perform operation while paused");
    }
    singleStepper_ = SingleStepper::GetStepOverStepper(vm_);
    if (singleStepper_ == nullptr) {
        LOG_DEBUGGER(ERROR) << "StepOver: singleStepper is null";
        return DispatchResponse::Fail("Failed to StepOver");
    }
    frontend_.Resumed(vm_);
    debuggerState_ = DebuggerState::ENABLED;
    return DispatchResponse::Ok();
}

DispatchResponse DebuggerImpl::SetBlackboxPatterns()
{
    return DispatchResponse::Fail("SetBlackboxPatterns not support now");
}

DispatchResponse DebuggerImpl::SetMixedDebugEnabled([[maybe_unused]] const SetMixedDebugParams &params)
{
    vm_->GetJsDebuggerManager()->SetMixedDebugEnabled(params.GetEnabled());
    vm_->GetJsDebuggerManager()->SetMixedStackEnabled(params.GetMixedStackEnabled());
    mixStackEnabled_ = params.GetMixedStackEnabled();
    return DispatchResponse::Ok();
}

DispatchResponse DebuggerImpl::ReplyNativeCalling([[maybe_unused]] const ReplyNativeCallingParams &params)
{
    frontend_.Resumed(vm_);
    if (params.GetUserCode()) {
        singleStepper_.reset();
    }
    return DispatchResponse::Ok();
}

DispatchResponse DebuggerImpl::DropFrame(const DropFrameParams &params)
{
    if (debuggerState_ != DebuggerState::PAUSED) {
        return DispatchResponse::Fail("Can only perform operation while paused");
    }
    uint32_t droppedDepth = 1;
    if (params.HasDroppedDepth()) {
        droppedDepth = params.GetDroppedDepth();
        if (droppedDepth == 0) {
            return DispatchResponse::Ok();
        }
        if (droppedDepth > 1) {
            return DispatchResponse::Fail("Not yet support dropping multiple frames");
        }
    }
    uint32_t stackDepth = DebuggerApi::GetStackDepth(vm_);
    if (droppedDepth > stackDepth) {
        return DispatchResponse::Fail("The input depth exceeds stackDepth");
    }
    if (droppedDepth == stackDepth) {
        return DispatchResponse::Fail("The bottom frame cannot be dropped");
    }
    uint32_t stackDepthOverBuiltin = DebuggerApi::GetStackDepthOverBuiltin(vm_);
    if (droppedDepth >= stackDepthOverBuiltin) {
        return DispatchResponse::Fail("Frames to be dropped contain builtin frame");
    }
    if (DebuggerApi::CheckIsSendableMethod(vm_)) {
        return DispatchResponse::Fail("Not yet support sendable method");
    }
    if (!DebuggerApi::CheckPromiseQueueSize(vm_)) {
        return DispatchResponse::Fail("Detect promise enqueued in current frame");
    }
    for (uint32_t i = 0; i < droppedDepth; i++) {
        DebuggerApi::DropLastFrame(vm_);
    }
    pauseOnNextByteCode_ = true;
    frontend_.RunIfWaitingForDebugger(vm_);
    debuggerState_ = DebuggerState::ENABLED;
    return DispatchResponse::Ok();
}

DispatchResponse DebuggerImpl::CallFunctionOn([[maybe_unused]] const CallFunctionOnParams &params,
    std::unique_ptr<RemoteObject> *outRemoteObject,
    [[maybe_unused]] std::optional<std::unique_ptr<ExceptionDetails>> *outExceptionDetails)
{
    // get callFrameId
    CallFrameId callFrameId = params.GetCallFrameId();
    if (callFrameId < 0 || callFrameId >= static_cast<CallFrameId>(callFrameHandlers_.size())) {
        return DispatchResponse::Fail("Invalid callFrameId.");
    }
    // get function declaration
    std::string functionDeclaration = params.GetFunctionDeclaration();
    std::vector<uint8_t> dest;
    if (!DecodeAndCheckBase64(functionDeclaration, dest)) {
        LOG_DEBUGGER(ERROR) << "CallFunctionOn: base64 decode failed";
        return DispatchResponse::Fail("base64 decode failed, functionDeclaration: " +
            functionDeclaration);
    }
    auto funcRef = DebuggerApi::GenerateFuncFromBuffer(vm_, dest.data(), dest.size(),
        JSPandaFile::ENTRY_FUNCTION_NAME);
    // call function
    auto res = DebuggerApi::CallFunctionOnCall(const_cast<EcmaVM *>(vm_), funcRef,
        callFrameHandlers_[callFrameId]);
    if (vm_->GetJSThread()->HasPendingException()) {
        LOG_DEBUGGER(ERROR) << "CallFunctionOn: has pending exception";
        std::string msg;
        DebuggerApi::HandleUncaughtException(vm_, msg);
        *outRemoteObject = RemoteObject::FromTagged(vm_,
            Exception::EvalError(vm_, StringRef::NewFromUtf8(vm_, msg.data())));
        return DispatchResponse::Fail(msg);
    }

    *outRemoteObject = RemoteObject::FromTagged(vm_, res);
    runtime_->CacheObjectIfNeeded(res, (*outRemoteObject).get());
    return DispatchResponse::Ok();
}

void DebuggerImpl::CleanUpOnPaused()
{
    CleanUpRuntimeProperties();
    callFrameHandlers_.clear();
    scopeObjects_.clear();
}

void DebuggerImpl::CleanUpRuntimeProperties()
{
    LOG_DEBUGGER(INFO) << "CleanUpRuntimeProperties OnPaused";
    if (runtime_->properties_.empty()) {
        return;
    }
    RemoteObjectId validObjId = runtime_->curObjectId_ - 1;
    for (; validObjId >= 0; validObjId--) {
        runtime_->properties_[validObjId].FreeGlobalHandleAddr();
    }
    runtime_->curObjectId_ = 0;
    runtime_->properties_.clear();
}

std::string DebuggerImpl::Trim(const std::string &str)
{
    std::string ret = str;
    // If ret has only ' ', remove all charactors.
    ret.erase(ret.find_last_not_of(' ') + 1);
    // If ret has only ' ', remove all charactors.
    ret.erase(0, ret.find_first_not_of(' '));
    return ret;
}

DebugInfoExtractor *DebuggerImpl::GetExtractor(const JSPandaFile *jsPandaFile)
{
    return JSPandaFileManager::GetInstance()->GetJSPtExtractor(jsPandaFile);
}

// mainly used for breakpoints to match location
std::vector<DebugInfoExtractor *> DebuggerImpl::GetExtractors(const std::string &url)
{
    std::vector<DebugInfoExtractor *> extractors;
    // match patch file first if it contains diff for the url, and currently only support the file
    // specified by the url change as a whole
    extractors = DebuggerApi::GetPatchExtractors(vm_, url);
    if (!extractors.empty()) {
        return extractors;
    }

    std::vector<PtScript *> ptScripts = MatchAllScripts(url);
    for (auto ptScript : ptScripts) {
        std::string fileName = ptScript->GetFileName();
        const JSPandaFile *jsPandaFile = JSPandaFileManager::GetInstance()->FindJSPandaFile(fileName.c_str()).get();
        if (jsPandaFile == nullptr) {
            continue;
        }
        DebugInfoExtractor *extractor = JSPandaFileManager::GetInstance()->GetJSPtExtractor(jsPandaFile);
        if (extractor == nullptr) {
            LOG_DEBUGGER(DEBUG) << "GetPossibleBreakpoints: extractor is null";
            continue;
        }
        extractors.emplace_back(extractor);
    }
    return extractors;
}

bool DebuggerImpl::GenerateCallFrames(std::vector<std::unique_ptr<CallFrame>> *callFrames, bool getScope)
{
    CallFrameId callFrameId = 0;
    auto walkerFunc = [this, &callFrameId, &callFrames, &getScope](const FrameHandler *frameHandler) -> StackState {
        if (DebuggerApi::IsNativeMethod(frameHandler)) {
            LOG_DEBUGGER(INFO) << "GenerateCallFrames: Skip CFrame and Native method";
            return StackState::CONTINUE;
        }
        std::unique_ptr<CallFrame> callFrame = std::make_unique<CallFrame>();
        if (!GenerateCallFrame(callFrame.get(), frameHandler, callFrameId, getScope)) {
            if (callFrameId == 0) {
                return StackState::FAILED;
            }
        } else {
            SaveCallFrameHandler(frameHandler);
            callFrames->emplace_back(std::move(callFrame));
            callFrameId++;
        }
        return StackState::CONTINUE;
    };
    return DebuggerApi::StackWalker(vm_, walkerFunc);
}

bool DebuggerImpl::GenerateAsyncFrames(std::shared_ptr<AsyncStack> asyncStack, bool skipTopFrame)
{
    std::vector<std::shared_ptr<StackFrame>> asyncFrames;
    bool topFrame = true;
    int32_t stackSize = MAX_CALL_STACK_SIZE_TO_CAPTURE;
    auto walkerFunc = [this, &asyncFrames, &topFrame, &stackSize](const FrameHandler *frameHandler) -> StackState {
        if (DebuggerApi::IsNativeMethod(frameHandler) || !stackSize) {
            return StackState::CONTINUE;
        }
        std::shared_ptr<StackFrame> stackFrame = std::make_shared<StackFrame>();
        if (!GenerateAsyncFrame(stackFrame.get(), frameHandler)) {
            if (topFrame) {
                return StackState::FAILED;
            }
        } else {
            topFrame = false;
            stackSize--;
            asyncFrames.emplace_back(std::move(stackFrame));
        }
        return StackState::CONTINUE;
    };
    bool result = DebuggerApi::StackWalker(vm_, walkerFunc);
    // await need skip top frame
    if (skipTopFrame && !asyncFrames.empty()) {
        asyncFrames.erase(asyncFrames.begin());
    }
    asyncStack->SetFrames(asyncFrames);
    return result;
}

bool DebuggerImpl::GenerateAsyncFrame(StackFrame *stackFrame, const FrameHandler *frameHandler)
{
    if (!frameHandler->HasFrame()) {
        return false;
    }
    Method *method = DebuggerApi::GetMethod(frameHandler);
    auto methodId = method->GetMethodId();
    JSThread *thread = vm_->GetJSThread();
    const JSPandaFile *jsPandaFile = method->GetJSPandaFile(thread);
    DebugInfoExtractor *extractor = GetExtractor(jsPandaFile);
    if (extractor == nullptr) {
        LOG_DEBUGGER(DEBUG) << "GenerateAsyncFrame: extractor is null";
        return false;
    }

    // functionName
    std::string functionName = method->ParseFunctionName(thread);

    std::string url = extractor->GetSourceFile(methodId);
    int32_t scriptId = -1;
    int32_t lineNumber = -1;
    int32_t columnNumber = -1;
    auto scriptFunc = [&scriptId](PtScript *script) -> bool {
        scriptId = script->GetScriptId();
        return true;
    };
    if (!MatchScripts(scriptFunc, url, ScriptMatchType::URL)) {
        LOG_DEBUGGER(ERROR) << "GenerateAsyncFrame: Unknown url: " << url;
        return false;
    }
    auto callbackLineFunc = [&lineNumber](int32_t line) -> bool {
        lineNumber = line;
        return true;
    };
    auto callbackColumnFunc = [&columnNumber](int32_t column) -> bool {
        columnNumber = column;
        return true;
    };
    if (!extractor->MatchLineWithOffset(callbackLineFunc, methodId, DebuggerApi::GetBytecodeOffset(frameHandler)) ||
        !extractor->MatchColumnWithOffset(callbackColumnFunc, methodId, DebuggerApi::GetBytecodeOffset(frameHandler))) {
        LOG_DEBUGGER(ERROR) << "GenerateAsyncFrame: unknown offset: " << DebuggerApi::GetBytecodeOffset(frameHandler);
        return false;
    }
    stackFrame->SetFunctionName(functionName);
    stackFrame->SetScriptId(scriptId);
    stackFrame->SetUrl(url);
    stackFrame->SetLineNumber(lineNumber);
    stackFrame->SetColumnNumber(columnNumber);
    return true;
}

void DebuggerImpl::SetPauseOnNextByteCode(bool pauseOnNextByteCode)
{
    pauseOnNextByteCode_ = pauseOnNextByteCode;
}

void DebuggerImpl::SaveCallFrameHandler(const FrameHandler *frameHandler)
{
    auto handlerPtr = DebuggerApi::NewFrameHandler(vm_);
    *handlerPtr = *frameHandler;
    callFrameHandlers_.emplace_back(handlerPtr);
}

bool DebuggerImpl::GenerateCallFrame(CallFrame *callFrame, const FrameHandler *frameHandler,
                                     CallFrameId callFrameId, bool getScope)
{
    if (!frameHandler->HasFrame()) {
        return false;
    }
    Method *method = DebuggerApi::GetMethod(frameHandler);
    auto methodId = method->GetMethodId();
    JSThread *thread = vm_->GetJSThread();
    const JSPandaFile *jsPandaFile = method->GetJSPandaFile(thread);
    DebugInfoExtractor *extractor = GetExtractor(jsPandaFile);
    if (extractor == nullptr) {
        LOG_DEBUGGER(DEBUG) << "GenerateCallFrame: extractor is null";
        return false;
    }

    // functionName
    std::string functionName = method->ParseFunctionName(thread);

    // location
    std::unique_ptr<Location> location = std::make_unique<Location>();
    std::string url = extractor->GetSourceFile(methodId);
    auto scriptFunc = [&location](PtScript *script) -> bool {
        location->SetScriptId(script->GetScriptId());
        return true;
    };
    if (!MatchScripts(scriptFunc, url, ScriptMatchType::URL)) {
        LOG_DEBUGGER(ERROR) << "GenerateCallFrame: Unknown url: " << url;
        return false;
    }
    auto callbackLineFunc = [&location](int32_t line) -> bool {
        location->SetLine(line);
        return true;
    };
    auto callbackColumnFunc = [&location](int32_t column) -> bool {
        location->SetColumn(column);
        return true;
    };
    if (!extractor->MatchLineWithOffset(callbackLineFunc, methodId, DebuggerApi::GetBytecodeOffset(frameHandler)) ||
        !extractor->MatchColumnWithOffset(callbackColumnFunc, methodId, DebuggerApi::GetBytecodeOffset(frameHandler))) {
        LOG_DEBUGGER(ERROR) << "GenerateCallFrame: unknown offset: " << DebuggerApi::GetBytecodeOffset(frameHandler);
        return false;
    }
    std::unique_ptr<RemoteObject> thisObj = std::make_unique<RemoteObject>();
    std::vector<std::unique_ptr<Scope>> scopeChain;
    DebuggerImpl::GenerateScopeChains(getScope, frameHandler, jsPandaFile, scopeChain, thisObj);
    callFrame->SetCallFrameId(callFrameId)
        .SetFunctionName(functionName)
        .SetLocation(std::move(location))
        .SetUrl(url)
        .SetScopeChain(std::move(scopeChain))
        .SetThis(std::move(thisObj));
    return true;
}
    
void DebuggerImpl::GenerateScopeChains(bool getScope,
                                       const FrameHandler *frameHandler,
                                       const JSPandaFile *jsPandaFile,
                                       std::vector<std::unique_ptr<Scope>> &scopeChain,
                                       std::unique_ptr<RemoteObject> &thisObj)
{
    // scopeChain & this
    
    thisObj->SetType(ObjectType::Undefined);
    JSThread *thread = vm_->GetJSThread();
    if (getScope) {
        scopeChain.emplace_back(GetLocalScopeChain(frameHandler, &thisObj));
        // generate closure scopes
        auto closureScopeChains = GetClosureScopeChains(frameHandler, &thisObj);
        for (auto &scope : closureScopeChains) {
            scopeChain.emplace_back(std::move(scope));
        }
        if (jsPandaFile != nullptr && !DebuggerApi::JSPandaFileIsBundlePack(jsPandaFile) &&
            DebuggerApi::JSPandaFileIsNewVersion(jsPandaFile)) {
            JSHandle<JSTaggedValue> currentModule(thread, DebuggerApi::GetCurrentModule(vm_));
            if (currentModule->IsSourceTextModule()) { // CODES module is string
                scopeChain.emplace_back(GetModuleScopeChain(frameHandler));
            }
        }
        scopeChain.emplace_back(GetGlobalScopeChain(frameHandler));
    }
}

std::unique_ptr<Scope> DebuggerImpl::GetLocalScopeChain(const FrameHandler *frameHandler,
    std::unique_ptr<RemoteObject> *thisObj)
{
    auto localScope = std::make_unique<Scope>();

    Method *method = DebuggerApi::GetMethod(frameHandler);
    auto methodId = method->GetMethodId();
    JSThread *thread = vm_->GetJSThread();
    const JSPandaFile *jsPandaFile = method->GetJSPandaFile(thread);
    DebugInfoExtractor *extractor = GetExtractor(jsPandaFile);
    if (extractor == nullptr) {
        LOG_DEBUGGER(ERROR) << "GetScopeChain: extractor is null";
        return localScope;
    }

    std::unique_ptr<RemoteObject> local = std::make_unique<RemoteObject>();
    Local<ObjectRef> localObj = ObjectRef::New(vm_);
    local->SetType(ObjectType::Object)
        .SetObjectId(runtime_->curObjectId_)
        .SetClassName(ObjectClassName::Object)
        .SetDescription(RemoteObject::ObjectDescription);
    auto *sp = DebuggerApi::GetSp(frameHandler);
    scopeObjects_[sp][Scope::Type::Local()].push_back(runtime_->curObjectId_);
    DebuggerApi::AddInternalProperties(vm_, localObj, ArkInternalValueType::Scope,  runtime_->internalObjects_);
    runtime_->properties_[runtime_->curObjectId_++] = Global<JSValueRef>(vm_, localObj);

    Local<JSValueRef> thisVal = JSNApiHelper::ToLocal<JSValueRef>(
        JSHandle<JSTaggedValue>(vm_->GetJSThread(), JSTaggedValue::Hole()));
    GetLocalVariables(frameHandler, methodId, jsPandaFile, thisVal, localObj);
    *thisObj = RemoteObject::FromTagged(vm_, thisVal);
    runtime_->CacheObjectIfNeeded(thisVal, (*thisObj).get());

    const LineNumberTable &lines = extractor->GetLineNumberTable(methodId);
    std::unique_ptr<Location> startLoc = std::make_unique<Location>();
    std::unique_ptr<Location> endLoc = std::make_unique<Location>();
    auto scriptFunc = [&startLoc, &endLoc, lines](PtScript *script) -> bool {
        startLoc->SetScriptId(script->GetScriptId())
            .SetLine(lines.front().line)
            .SetColumn(0);
        endLoc->SetScriptId(script->GetScriptId())
            .SetLine(lines.back().line + 1)
            .SetColumn(0);
        return true;
    };
    if (MatchScripts(scriptFunc, extractor->GetSourceFile(methodId), ScriptMatchType::URL)) {
        localScope->SetType(Scope::Type::Local())
            .SetObject(std::move(local))
            .SetStartLocation(std::move(startLoc))
            .SetEndLocation(std::move(endLoc));
    }

    return localScope;
}

std::vector<std::unique_ptr<Scope>> DebuggerImpl::GetClosureScopeChains(const FrameHandler *frameHandler,
    std::unique_ptr<RemoteObject> *thisObj)
{
    std::vector<std::unique_ptr<Scope>> closureScopes;
    Method *method = DebuggerApi::GetMethod(frameHandler);
    EntityId methodId = method->GetMethodId();
    JSThread *thread = vm_->GetJSThread();
    const JSPandaFile *jsPandaFile = method->GetJSPandaFile(thread);
    DebugInfoExtractor *extractor = GetExtractor(jsPandaFile);

    if (extractor == nullptr) {
        LOG_DEBUGGER(ERROR) << "GetClosureScopeChains: extractor is null";
        return closureScopes;
    }

    JSMutableHandle<JSTaggedValue> envHandle = JSMutableHandle<JSTaggedValue>(
        thread, DebuggerApi::GetEnv(frameHandler));
    JSMutableHandle<JSTaggedValue> valueHandle = JSMutableHandle<JSTaggedValue>(thread, JSTaggedValue::Hole());
    JSTaggedValue currentEnv = envHandle.GetTaggedValue();
    if (!currentEnv.IsLexicalEnv()) {
        LOG_DEBUGGER(DEBUG) << "GetClosureScopeChains: currentEnv is invalid";
        return closureScopes;
    }
    // check if GetLocalScopeChain has already found and set 'this' value
    bool thisFound = (*thisObj)->HasValue();
    bool closureVarFound = false;
    // currentEnv = currentEnv->parent until currentEnv is not lexicalEnv
    for (; currentEnv.IsLexicalEnv();
         currentEnv = LexicalEnv::Cast(currentEnv.GetTaggedObject())->GetParentEnv(thread)) {
        LexicalEnv *lexicalEnv = LexicalEnv::Cast(currentEnv.GetTaggedObject());
        envHandle.Update(currentEnv);
        if (lexicalEnv->GetScopeInfo(thread).IsHole()) {
            continue;
        }
        auto closureScope = std::make_unique<Scope>();
        auto result = JSNativePointer::Cast(lexicalEnv->GetScopeInfo(thread).GetTaggedObject())->GetExternalPointer();
        ScopeDebugInfo *scopeDebugInfo = reinterpret_cast<ScopeDebugInfo *>(result);
        std::unique_ptr<RemoteObject> closure = std::make_unique<RemoteObject>();
        Local<ObjectRef> closureScopeObj = ObjectRef::New(vm_);

        std::unordered_map<CString, int> nameCount;
        for (const auto &[name, slot] : scopeDebugInfo->scopeInfo) {
            if (IsVariableSkipped(name.c_str())) {
                continue;
            }
            currentEnv = envHandle.GetTaggedValue();
            lexicalEnv = LexicalEnv::Cast(currentEnv.GetTaggedObject());
            valueHandle.Update(lexicalEnv->GetProperties(thread, slot));
            Local<JSValueRef> value = JSNApiHelper::ToLocal<JSValueRef>(valueHandle);
            Local<JSValueRef> varName = StringRef::NewFromUtf8(vm_, name.c_str());
            nameCount[name]++;
            if (nameCount[name] > 1) {
                CString fullName = name + "$" + ToCString(slot);
                varName = StringRef::NewFromUtf8(vm_, fullName.c_str());
            }
            // found 'this' and 'this' is not set in GetLocalScopechain
            if (!thisFound && name == "this") {
                *thisObj = RemoteObject::FromTagged(vm_, value);
                // cache 'this' object
                runtime_->CacheObjectIfNeeded(value, (*thisObj).get());
                thisFound = true;
                continue;
            }
            // found closure variable in current lexenv
            closureVarFound = true;
            // if value is hole, should manually set it to undefined
            // otherwise after DefineProperty, corresponding varName
            // will become undefined
            if (value->IsHole()) {
                valueHandle.Update(JSTaggedValue::Undefined());
                value = JSNApiHelper::ToLocal<JSValueRef>(valueHandle);
            }
            PropertyAttribute descriptor(value, true, true, true);
            closureScopeObj->DefineProperty(vm_, varName, descriptor);
        }
        // at least one closure variable has been found
        if (closureVarFound) {
            closure->SetType(ObjectType::Object)
                .SetObjectId(runtime_->curObjectId_)
                .SetClassName(ObjectClassName::Object)
                .SetDescription(RemoteObject::ObjectDescription);

            auto scriptFunc = []([[maybe_unused]] PtScript *script) -> bool {
                return true;
            };
            if (MatchScripts(scriptFunc, extractor->GetSourceFile(methodId), ScriptMatchType::URL)) {
                closureScope->SetType(Scope::Type::Closure()).SetObject(std::move(closure));
                DebuggerApi::AddInternalProperties(
                    vm_, closureScopeObj, ArkInternalValueType::Scope,  runtime_->internalObjects_);
                auto *sp = DebuggerApi::GetSp(frameHandler);
                scopeObjects_[sp][Scope::Type::Closure()].push_back(runtime_->curObjectId_);
                runtime_->properties_[runtime_->curObjectId_++] = Global<JSValueRef>(vm_, closureScopeObj);
                closureScopes.emplace_back(std::move(closureScope));
            }
        }
        currentEnv = envHandle.GetTaggedValue();
        closureVarFound = false;
    }
    return closureScopes;
}

std::unique_ptr<Scope> DebuggerImpl::GetModuleScopeChain(const FrameHandler *frameHandler)
{
    auto moduleScope = std::make_unique<Scope>();

    std::unique_ptr<RemoteObject> module = std::make_unique<RemoteObject>();
    Local<ObjectRef> moduleObj = ObjectRef::New(vm_);
    module->SetType(ObjectType::Object)
        .SetObjectId(runtime_->curObjectId_)
        .SetClassName(ObjectClassName::Object)
        .SetDescription(RemoteObject::ObjectDescription);
    moduleScope->SetType(Scope::Type::Module()).SetObject(std::move(module));
    DebuggerApi::AddInternalProperties(vm_, moduleObj, ArkInternalValueType::Scope,  runtime_->internalObjects_);
    auto *sp = DebuggerApi::GetSp(frameHandler);
    scopeObjects_[sp][Scope::Type::Module()].push_back(runtime_->curObjectId_);
    runtime_->properties_[runtime_->curObjectId_++] = Global<JSValueRef>(vm_, moduleObj);
    JSThread *thread = vm_->GetJSThread();
    JSHandle<JSTaggedValue> currentModule(thread, DebuggerApi::GetCurrentModule(vm_));
    DebuggerApi::GetLocalExportVariables(vm_, moduleObj, currentModule, false);
    DebuggerApi::GetIndirectExportVariables(vm_, moduleObj, currentModule);
    DebuggerApi::GetImportVariables(vm_, moduleObj, currentModule);
    return moduleScope;
}

void DebuggerImpl::GetLocalVariables(const FrameHandler *frameHandler, panda_file::File::EntityId methodId,
    const JSPandaFile *jsPandaFile, Local<JSValueRef> &thisVal, Local<ObjectRef> &localObj)
{
    auto *extractor = GetExtractor(jsPandaFile);
    Local<JSValueRef> value = JSValueRef::Undefined(vm_);
    // in case of arrow function, which doesn't have this in local variable table
    for (const auto &localVariableInfo : extractor->GetLocalVariableTable(methodId)) {
        std::string varName = localVariableInfo.name;
        int32_t regIndex = localVariableInfo.regNumber;
        uint32_t bcOffset = DebuggerApi::GetBytecodeOffset(frameHandler);
        // if the bytecodeOffset is not in the range of the variable's scope,
        // which is indicated as [start_offset, end_offset), ignore it.
        if (!IsWithinVariableScope(localVariableInfo, bcOffset)) {
            continue;
        }

        if (IsVariableSkipped(varName)) {
            continue;
        }

        value = DebuggerApi::GetVRegValue(vm_, frameHandler, regIndex);
        if (varName == "this") {
            LOG_DEBUGGER(INFO) << "find 'this' in local variable table";
            thisVal = value;
            continue;
        }
        Local<JSValueRef> name = JSValueRef::Undefined(vm_);
        if (varName == "4funcObj") {
            if (value->IsFunction(vm_)) {
                auto funcName = Local<FunctionRef>(value)->GetName(vm_)->ToString(vm_);
                name = StringRef::NewFromUtf8(vm_, funcName.c_str());
            } else {
                continue;
            }
        } else {
            name = StringRef::NewFromUtf8(vm_, varName.c_str());
        }
        PropertyAttribute descriptor(value, true, true, true);
        localObj->DefineProperty(vm_, name, descriptor);
    }
}

bool DebuggerImpl::IsWithinVariableScope(const LocalVariableInfo &localVariableInfo, uint32_t bcOffset)
{
    return bcOffset >= localVariableInfo.startOffset && bcOffset < localVariableInfo.endOffset;
}

bool DebuggerImpl::IsVariableSkipped(const std::string &varName)
{
    return varName == "4newTarget" || varName == "0this" || varName == "0newTarget" || varName == "0funcObj";
}

std::unique_ptr<Scope> DebuggerImpl::GetGlobalScopeChain(const FrameHandler *frameHandler)
{
    auto globalScope = std::make_unique<Scope>();

    std::unique_ptr<RemoteObject> global = std::make_unique<RemoteObject>();
    Local<ObjectRef> globalObj = ObjectRef::New(vm_);
    global->SetType(ObjectType::Object)
        .SetObjectId(runtime_->curObjectId_)
        .SetClassName(ObjectClassName::Global)
        .SetDescription(RemoteObject::GlobalDescription);
    globalScope->SetType(Scope::Type::Global()).SetObject(std::move(global));
    globalObj = JSNApi::GetGlobalObject(vm_, DebuggerApi::GetCurrentGlobalEnv(vm_, frameHandler));
    DebuggerApi::AddInternalProperties(vm_, globalObj, ArkInternalValueType::Scope,  runtime_->internalObjects_);
    auto *sp = DebuggerApi::GetSp(frameHandler);
    scopeObjects_[sp][Scope::Type::Global()].push_back(runtime_->curObjectId_);
    runtime_->properties_[runtime_->curObjectId_++] = Global<JSValueRef>(vm_, globalObj);
    return globalScope;
}

void DebuggerImpl::UpdateScopeObject(const FrameHandler *frameHandler,
    std::string_view varName, Local<JSValueRef> newVal, const std::string& scope)
{
    auto *sp = DebuggerApi::GetSp(frameHandler);
    auto iter = scopeObjects_.find(sp);
    if (iter == scopeObjects_.end()) {
        LOG_DEBUGGER(ERROR) << "UpdateScopeObject: object not found";
        return;
    }

    for (auto objectId : scopeObjects_[sp][scope]) {
        Local<ObjectRef> localObj = runtime_->properties_[objectId].ToLocal(vm_);
        Local<JSValueRef> name = StringRef::NewFromUtf8(vm_, varName.data());
        if (localObj->Has(vm_, name)) {
            LOG_DEBUGGER(DEBUG) << "UpdateScopeObject: set new value";
            PropertyAttribute descriptor(newVal, true, true, true);
            localObj->DefineProperty(vm_, name, descriptor);
            return;
        }
    }
    LOG_DEBUGGER(ERROR) << "UpdateScopeObject: not found " << varName;
}

void DebuggerImpl::ClearSingleStepper()
{
    // if current depth is 0, then it is safe to reset
    if (singleStepper_ != nullptr && DebuggerApi::GetStackDepth(vm_) == 0) {
        singleStepper_.reset();
    }
}

std::optional<std::string> DebuggerImpl::CmptEvaluateValue(CallFrameId callFrameId, const std::string &expression,
    std::unique_ptr<RemoteObject> *result)
{
    if (DebuggerApi::IsNativeMethod(vm_)) {
        *result = RemoteObject::FromTagged(vm_,
            Exception::EvalError(vm_, StringRef::NewFromUtf8(vm_, "Native Frame not support.")));
        return "Native Frame not support.";
    }
    DebugInfoExtractor *extractor = GetExtractor(DebuggerApi::GetJSPandaFile(vm_));
    if (extractor == nullptr) {
        *result = RemoteObject::FromTagged(vm_,
            Exception::EvalError(vm_, StringRef::NewFromUtf8(vm_, "Internal error.")));
        return "Internal error.";
    }
    std::string varName = expression;
    std::string varValue;
    std::string::size_type indexEqual = expression.find_first_of('=', 0);
    if (indexEqual != std::string::npos) {
        varName = Trim(expression.substr(0, indexEqual));
        varValue = Trim(expression.substr(indexEqual + 1, expression.length()));
    }

    Local<StringRef> name = StringRef::NewFromUtf8(vm_, varName.c_str());
    FrameHandler *frameHandler = callFrameHandlers_[callFrameId].get();
    if (varValue.empty()) {
        Local<JSValueRef> ret = DebuggerExecutor::GetValue(vm_, frameHandler, name);
        if (!ret.IsEmpty()) {
            *result = RemoteObject::FromTagged(vm_, ret);
            runtime_->CacheObjectIfNeeded(ret, (*result).get());
            return {};
        }
    } else {
        Local<JSValueRef> value = ConvertToLocal(varValue);
        if (value.IsEmpty()) {
            return "Unsupported expression.";
        }
        JsDebuggerManager *mgr = vm_->GetJsDebuggerManager();
        mgr->SetEvalFrameHandler(callFrameHandlers_[callFrameId]);
        bool ret = DebuggerExecutor::SetValue(vm_, frameHandler, name, value);
        mgr->SetEvalFrameHandler(nullptr);
        if (ret) {
            *result = RemoteObject::FromTagged(vm_, value);
            return {};
        }
    }

    *result = RemoteObject::FromTagged(vm_,
        Exception::EvalError(vm_, StringRef::NewFromUtf8(vm_, "Unsupported expression.")));
    return "Unsupported expression.";
}

Local<JSValueRef> DebuggerImpl::ConvertToLocal(const std::string &varValue)
{
    Local<JSValueRef> taggedValue;
    if (varValue.empty()) {
        taggedValue = NumberRef::New(vm_, 0);
    } else if (varValue == "false") {
        taggedValue = JSValueRef::False(vm_);
    } else if (varValue == "true") {
        taggedValue = JSValueRef::True(vm_);
    } else if (varValue == "undefined") {
        taggedValue = JSValueRef::Undefined(vm_);
    } else if (varValue[0] == '\"' && varValue[varValue.length() - 1] == '\"') {
        // 2 : 2 means length
        taggedValue = StringRef::NewFromUtf8(vm_, varValue.substr(1, varValue.length() - 2).c_str());
    } else {
        auto begin = reinterpret_cast<const uint8_t *>((varValue.c_str()));
        auto end = begin + varValue.length();  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        double d = DebuggerApi::StringToDouble(begin, end, 0);
        if (!std::isnan(d)) {
            taggedValue = NumberRef::New(vm_, d);
        }
    }
    return taggedValue;
}

bool DebuggerImpl::DecodeAndCheckBase64(const std::string &src, std::vector<uint8_t> &dest)
{
    dest.resize(PtBase64::DecodedSize(src.size()));
    auto [numOctets, done] = PtBase64::Decode(dest.data(), src.data(), src.size());
    dest.resize(numOctets);
    if ((done && numOctets > panda_file::File::MAGIC_SIZE) &&
        memcmp(dest.data(), panda_file::File::MAGIC.data(), panda_file::File::MAGIC_SIZE) == 0) {
        return true;
    }
    return false;
}

Local<FunctionRef> DebuggerImpl::CheckAndGenerateCondFunc(const std::optional<std::string> &condition)
{
    std::vector<uint8_t> dest;
    if (DecodeAndCheckBase64(condition.value(), dest)) {
        Local<FunctionRef> funcRef =
            DebuggerApi::GenerateFuncFromBuffer(vm_, dest.data(), dest.size(), JSPandaFile::ENTRY_FUNCTION_NAME);
        if (!funcRef->IsUndefined()) {
            return funcRef;
        }
    }
    return FunctionRef::Undefined(vm_);
}

void DebuggerImpl::SetDebuggerAccessor(const JSHandle<GlobalEnv> &globalEnv)
{
    Local<JSValueRef> global = JSNApiHelper::ToLocal<JSValueRef>(JSHandle<JSTaggedValue>(globalEnv));
    DebuggerExecutor::SetDebuggerAccessor(vm_, global);
}
}  // namespace panda::ecmascript::tooling
