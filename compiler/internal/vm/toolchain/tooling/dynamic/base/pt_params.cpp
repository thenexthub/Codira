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

#include "tooling/dynamic/base/pt_params.h"

namespace panda::ecmascript::tooling {
std::unique_ptr<EnableParams> EnableParams::Create(const PtJson &params)
{
    auto paramsObject = std::make_unique<EnableParams>();
    std::string error;
    Result ret;

    double maxScriptsCacheSize;
    ret = params.GetDouble("maxScriptsCacheSize", &maxScriptsCacheSize);
    if (ret == Result::SUCCESS) {
        paramsObject->maxScriptsCacheSize_ = maxScriptsCacheSize;
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'maxScriptsCacheSize';";
    }

    std::unique_ptr<PtJson> options;
    std::vector<std::string> enableOptionsList;
    ret = params.GetArray("options", &options);
    if (ret == Result::SUCCESS) {
        int32_t length = options->GetSize();
        for (int32_t i = 0; i < length; i++) {
            auto option = options->Get(i);
            if (option != nullptr && option->IsString()) {
                enableOptionsList.emplace_back(option->GetString());
            }
        }
        paramsObject->enableOptionList_ = enableOptionsList;
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'options';";
    }

    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "EnableParams::Create " << error;
        return nullptr;
    }

    return paramsObject;
}

std::unique_ptr<EvaluateOnCallFrameParams> EvaluateOnCallFrameParams::Create(const PtJson &params)
{
    auto paramsObject = std::make_unique<EvaluateOnCallFrameParams>();
    std::string error;
    Result ret;

    std::string callFrameId;
    ret = params.GetString("callFrameId", &callFrameId);
    if (ret == Result::SUCCESS) {
        if (!ToolchainUtils::StrToInt32(callFrameId, paramsObject->callFrameId_)) {
            error += "Failed to convert 'callFrameId' from string to int;";
        }
    } else {
        error += "Unknown or wrong type of 'callFrameId';";
    }
    std::string expression;
    ret = params.GetString("expression", &expression);
    if (ret == Result::SUCCESS) {
        paramsObject->expression_ = std::move(expression);
    } else {
        error += "Unknown or wrong type of 'expression';";
    }
    std::string objectGroup;
    ret = params.GetString("objectGroup", &objectGroup);
    if (ret == Result::SUCCESS) {
        paramsObject->objectGroup_ = std::move(objectGroup);
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'objectGroup';";
    }
    bool includeCommandLineAPI = false;
    ret = params.GetBool("includeCommandLineAPI", &includeCommandLineAPI);
    if (ret == Result::SUCCESS) {
        paramsObject->includeCommandLineAPI_ = includeCommandLineAPI;
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'includeCommandLineAPI';";
    }
    bool silent = false;
    ret = params.GetBool("silent", &silent);
    if (ret == Result::SUCCESS) {
        paramsObject->silent_ = silent;
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'silent';";
    }
    bool returnByValue = false;
    ret = params.GetBool("returnByValue", &returnByValue);
    if (ret == Result::SUCCESS) {
        paramsObject->returnByValue_ = returnByValue;
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'returnByValue';";
    }
    bool generatePreview = false;
    ret = params.GetBool("generatePreview", &generatePreview);
    if (ret == Result::SUCCESS) {
        paramsObject->generatePreview_ = generatePreview;
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'generatePreview';";
    }
    bool throwOnSideEffect = false;
    ret = params.GetBool("throwOnSideEffect", &throwOnSideEffect);
    if (ret == Result::SUCCESS) {
        paramsObject->throwOnSideEffect_ = throwOnSideEffect;
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'throwOnSideEffect';";
    }

    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "EvaluateOnCallFrameParams::Create " << error;
        return nullptr;
    }
    return paramsObject;
}

std::unique_ptr<ContinueToLocationParams> ContinueToLocationParams::Create(const PtJson &params)
{
    auto paramsObject = std::make_unique<ContinueToLocationParams>();
    std::string error;
    Result ret;

    std::unique_ptr<PtJson> position;
    ret = params.GetObject("location", &position);
    if (ret == Result::SUCCESS) {
        std::unique_ptr<Location> location = Location::Create(*position);
        if (location == nullptr) {
            error += "'location' is invalid;";
        } else {
            paramsObject->location_= std::move(location);
        }
    } else {
        error += "Unknown or wrong type of 'location';";
    }

    std::string targetCallFrames;
    ret = params.GetString("targetCallFrames", &targetCallFrames);
    if (ret == Result::SUCCESS) {
        paramsObject->targetCallFrames_ = std::move(targetCallFrames);
    } else {
        error += "Unknown 'targetCallFrames';";
    }

    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "ContinueToLocationParams::Create " << error;
        return nullptr;
    }

    return paramsObject;
}


std::unique_ptr<GetPossibleBreakpointsParams> GetPossibleBreakpointsParams::Create(const PtJson &params)
{
    auto paramsObject = std::make_unique<GetPossibleBreakpointsParams>();
    std::string error;
    Result ret;

    std::unique_ptr<PtJson> start;
    ret = params.GetObject("start", &start);
    if (ret == Result::SUCCESS) {
        std::unique_ptr<Location> location = Location::Create(*start);
        if (location == nullptr) {
            error += "'start' is invalid;";
        } else {
            paramsObject->start_ = std::move(location);
        }
    } else {
        error += "Unknown or wrong type of 'start';";
    }
    std::unique_ptr<PtJson> end;
    ret = params.GetObject("end", &end);
    if (ret == Result::SUCCESS) {
        std::unique_ptr<Location> location = Location::Create(*end);
        if (location == nullptr) {
            error += "'end' is invalid;";
        } else {
            paramsObject->end_ = std::move(location);
        }
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'end';";
    }
    bool restrictToFunction = false;
    ret = params.GetBool("restrictToFunction", &restrictToFunction);
    if (ret == Result::SUCCESS) {
        paramsObject->restrictToFunction_ = restrictToFunction;
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'restrictToFunction';";
    }

    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "GetPossibleBreakpointsParams::Create " << error;
        return nullptr;
    }

    return paramsObject;
}

std::unique_ptr<GetScriptSourceParams> GetScriptSourceParams::Create(const PtJson &params)
{
    auto paramsObject = std::make_unique<GetScriptSourceParams>();
    std::string error;
    Result ret;

    std::string scriptId;
    ret = params.GetString("scriptId", &scriptId);
    if (ret == Result::SUCCESS) {
        if (!ToolchainUtils::StrToInt32(scriptId, paramsObject->scriptId_)) {
            error += "Failed to convert 'scriptId' from string to int;";
        }
    } else {
        error += "Unknown or wrong type of'scriptId';";
    }

    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "GetScriptSourceParams::Create " << error;
        return nullptr;
    }

    return paramsObject;
}

std::unique_ptr<RemoveBreakpointParams> RemoveBreakpointParams::Create(const PtJson &params)
{
    auto paramsObject = std::make_unique<RemoveBreakpointParams>();
    std::string error;
    Result ret;

    std::string breakpointId;
    ret = params.GetString("breakpointId", &breakpointId);
    if (ret == Result::SUCCESS) {
        paramsObject->breakpointId_ = std::move(breakpointId);
    } else {
        error += "Unknown or wrong type of 'breakpointId';";
    }

    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "RemoveBreakpointParams::Create " << error;
        return nullptr;
    }

    return paramsObject;
}

std::unique_ptr<RemoveBreakpointsByUrlParams> RemoveBreakpointsByUrlParams::Create(const PtJson &params)
{
    auto paramsObject = std::make_unique<RemoveBreakpointsByUrlParams>();
    std::string error;
    Result ret;

    std::string url;
    ret = params.GetString("url", &url);
    if (ret == Result::SUCCESS) {
        paramsObject->url_ = std::move(url);
    } else {
        error += "Wrong or unknown type of 'url'";
    }

    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "RemoveBreakpointByUrlParams::Create " << error;
        return nullptr;
    }

    return paramsObject;
}

std::unique_ptr<ResumeParams> ResumeParams::Create(const PtJson &params)
{
    auto paramsObject = std::make_unique<ResumeParams>();
    std::string error;
    Result ret;

    bool terminateOnResume = false;
    ret = params.GetBool("terminateOnResume", &terminateOnResume);
    if (ret == Result::SUCCESS) {
        paramsObject->terminateOnResume_ = terminateOnResume;
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'terminateOnResume';";
    }

    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "ResumeParams::Create " << error;
        return nullptr;
    }

    return paramsObject;
}

std::unique_ptr<SetAsyncCallStackDepthParams> SetAsyncCallStackDepthParams::Create(const PtJson &params)
{
    auto paramsObject = std::make_unique<SetAsyncCallStackDepthParams>();
    std::string error;
    Result ret;

    int32_t maxDepth;
    ret = params.GetInt("maxDepth", &maxDepth);
    if (ret == Result::SUCCESS) {
        paramsObject->maxDepth_ = maxDepth;
    } else {
        error += "Unknown or wrong type of 'maxDepth';";
    }

    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "SetAsyncCallStackDepthParams::Create " << error;
        return nullptr;
    }

    return paramsObject;
}

std::unique_ptr<SetBlackboxPatternsParams> SetBlackboxPatternsParams::Create(const PtJson &params)
{
    auto paramsObject = std::make_unique<SetBlackboxPatternsParams>();
    std::string error;
    Result ret;

    std::unique_ptr<PtJson> patterns;
    ret = params.GetArray("patterns", &patterns);
    if (ret == Result::SUCCESS) {
        int32_t len = patterns->GetSize();
        for (int32_t i = 0; i < len; ++i) {
            std::unique_ptr<PtJson> item = patterns->Get(i);
            if (item->IsString()) {
                paramsObject->patterns_.emplace_back(item->GetString());
            } else {
                error += "'patterns' items should be a String;";
            }
        }
    } else {
        error += "Unknown or wrong type of 'patterns';";
    }

    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "SetBlackboxPatternsParams::Create " << error;
        return nullptr;
    }

    return paramsObject;
}

std::unique_ptr<SetBreakpointByUrlParams> SetBreakpointByUrlParams::Create(const PtJson &params)
{
    auto paramsObject = std::make_unique<SetBreakpointByUrlParams>();
    std::string error;
    Result ret;

    int32_t lineNumber;
    ret = params.GetInt("lineNumber", &lineNumber);
    if (ret == Result::SUCCESS) {
        paramsObject->lineNumber_ = lineNumber;
    } else {
        error += "Unknown or wrong type of 'lineNumber';";
    }
    std::string url;
    ret = params.GetString("url", &url);
    if (ret == Result::SUCCESS) {
        paramsObject->url_ = std::move(url);
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'url';";
    }
    std::string urlRegex;
    ret = params.GetString("urlRegex", &urlRegex);
    if (ret == Result::SUCCESS) {
        paramsObject->urlRegex_ = std::move(urlRegex);
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'urlRegex';";
    }
    std::string scriptHash;
    ret = params.GetString("scriptHash", &scriptHash);
    if (ret == Result::SUCCESS) {
        paramsObject->scriptHash_ = std::move(scriptHash);
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'scriptHash';";
    }
    int32_t columnNumber;
    ret = params.GetInt("columnNumber", &columnNumber);
    if (ret == Result::SUCCESS) {
        paramsObject->columnNumber_ = columnNumber;
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'columnNumber';";
    }
    std::string condition;
    ret = params.GetString("condition", &condition);
    if (ret == Result::SUCCESS) {
        paramsObject->condition_ = std::move(condition);
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'condition';";
    }
    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "SetBreakpointByUrlParams::Create " << error;
        return nullptr;
    }

    return paramsObject;
}

std::unique_ptr<SetBreakpointsActiveParams> SetBreakpointsActiveParams::Create(const PtJson &params)
{
    auto paramsObject = std::make_unique<SetBreakpointsActiveParams>();
    std::string error;
    Result ret;

    bool breakpointsState;
    ret = params.GetBool("active", &breakpointsState);
    if (ret == Result::SUCCESS) {
        paramsObject->breakpointsState_ = std::move(breakpointsState);
    } else {
        error += "Unknown or wrong type of 'active';";
    }
    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "SetBreakpointsActiveParams::Create " << error;
        return nullptr;
    }

    return paramsObject;
}

std::unique_ptr<SetSkipAllPausesParams> SetSkipAllPausesParams::Create(const PtJson &params)
{
    auto paramsObject = std::make_unique<SetSkipAllPausesParams>();
    std::string error;
    Result ret;

    bool skipAllPausesState;
    ret = params.GetBool("skip", &skipAllPausesState);
    if (ret == Result::SUCCESS) {
        paramsObject->skipAllPausesState_ = std::move(skipAllPausesState);
    } else {
        error += "Unknown or wrong type of 'skip';";
    }
    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "SetSkipAllPausesParams::Create " << error;
        return nullptr;
    }

    return paramsObject;
}

std::unique_ptr<GetPossibleAndSetBreakpointParams> GetPossibleAndSetBreakpointParams::Create(const PtJson &params)
{
    auto paramsObject = std::make_unique<GetPossibleAndSetBreakpointParams>();
    std::string error;
    Result ret;

    std::unique_ptr<PtJson> breakpoints;
    ret = params.GetArray("locations", &breakpoints);
    if (ret == Result::SUCCESS) {
        int32_t length = breakpoints->GetSize();
        std::vector<std::unique_ptr<BreakpointInfo>> breakpointList;
        for (int32_t i = 0; i < length; i++) {
            std::unique_ptr<BreakpointInfo> info = BreakpointInfo::Create(*breakpoints->Get(i));
            if (info == nullptr) {
                error += "'breakpoints' items BreakpointInfo is invaild;";
                break;
            } else {
                breakpointList.emplace_back(std::move(info));
            }
        }
        if (!breakpointList.empty()) {
            paramsObject->breakpointsList_ = std::move(breakpointList);
        }
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'breakpoints';";
    }

    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "GetPossibleAndSetBreakpointParams::Create " << error;
        return nullptr;
    }

    return paramsObject;
}

std::unique_ptr<SetPauseOnExceptionsParams> SetPauseOnExceptionsParams::Create(const PtJson &params)
{
    auto paramsObject = std::make_unique<SetPauseOnExceptionsParams>();
    std::string error;
    Result ret;

    std::string state;
    ret = params.GetString("state", &state);
    if (ret == Result::SUCCESS) {
        paramsObject->StoreState(state);
    } else {
        error += "Unknown or wrong type of'state';";
    }

    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "SetPauseOnExceptionsParams::Create " << error;
        return nullptr;
    }

    return paramsObject;
}

std::unique_ptr<StepIntoParams> StepIntoParams::Create(const PtJson &params)
{
    auto paramsObject = std::make_unique<StepIntoParams>();
    std::string error;
    Result ret;

    bool breakOnAsyncCall = false;
    ret = params.GetBool("breakOnAsyncCall", &breakOnAsyncCall);
    if (ret == Result::SUCCESS) {
        paramsObject->breakOnAsyncCall_ = breakOnAsyncCall;
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'breakOnAsyncCall';";
    }
    std::unique_ptr<PtJson> skipList;
    ret = params.GetArray("skipList", &skipList);
    if (ret == Result::SUCCESS) {
        int32_t len = skipList->GetSize();
        std::list<std::unique_ptr<LocationRange>> listLocation;
        for (int32_t i = 0; i < len; ++i) {
            std::unique_ptr<LocationRange> obj = LocationRange::Create(*skipList->Get(i));
            if (obj == nullptr) {
                error += "'skipList' items LocationRange is invalid;";
                break;
            } else {
                listLocation.emplace_back(std::move(obj));
            }
        }
        if (listLocation.size()) {
            paramsObject->skipList_ = std::move(listLocation);
        }
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'skipList';";
    }

    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "StepIntoParams::Create " << error;
        return nullptr;
    }

    return paramsObject;
}

std::unique_ptr<SmartStepIntoParams> SmartStepIntoParams::Create(const PtJson &params)
{
    auto paramsObject = std::make_unique<SmartStepIntoParams>();
    auto sbpObject = std::make_unique<SetBreakpointByUrlParams>();
    PtJson ptJson(params.GetJson());
    AddRequireParams(ptJson);
    if (!(sbpObject = SetBreakpointByUrlParams::Create(ptJson))) {
        return nullptr;
    }
    paramsObject->sbpParams_ = std::move(sbpObject);
    return paramsObject;
}

void SmartStepIntoParams::AddRequireParams(PtJson &params)
{
    if (!params.Contains("urlRegex")) {
        params.Add("urlRegex", "");
    }
    if (!params.Contains("scriptHash")) {
        params.Add("scriptHash", "");
    }
    if (!params.Contains("columnNumber")) {
        params.Add("columnNumber", 0);
    }
    if (!params.Contains("condition")) {
        params.Add("condition", "");
    }
}

std::unique_ptr<StepOverParams> StepOverParams::Create(const PtJson &params)
{
    auto paramsObject = std::make_unique<StepOverParams>();
    std::string error;
    Result ret;

    std::unique_ptr<PtJson> skipList;
    ret = params.GetArray("skipList", &skipList);
    if (ret == Result::SUCCESS) {
        int32_t len = skipList->GetSize();
        std::list<std::unique_ptr<LocationRange>> listLocation;
        for (int32_t i = 0; i < len; ++i) {
            std::unique_ptr<LocationRange> obj = LocationRange::Create(*skipList->Get(i));
            if (obj == nullptr) {
                error += "'skipList' items LocationRange is invalid;";
                break;
            } else {
                listLocation.emplace_back(std::move(obj));
            }
        }
        if (listLocation.size()) {
            paramsObject->skipList_ = std::move(listLocation);
        }
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'skipList';";
    }

    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "StepOverParams::Create " << error;
        return nullptr;
    }

    return paramsObject;
}

std::unique_ptr<DropFrameParams> DropFrameParams::Create(const PtJson &params)
{
    auto paramsObject = std::make_unique<DropFrameParams>();
    std::string error;
    Result ret;

    uint32_t droppedDepth = 0;
    ret = params.GetUInt("droppedDepth", &droppedDepth);
    if (ret == Result::SUCCESS) {
        paramsObject->droppedDepth_ = droppedDepth;
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'droppedDepth';";
    }

    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "DropFrameParams::Create " << error;
        return nullptr;
    }

    return paramsObject;
}

std::unique_ptr<SetNativeRangeParams> SetNativeRangeParams::Create(const PtJson &params)
{
    auto paramsObject = std::make_unique<SetNativeRangeParams>();
    std::string error;
    Result ret;

    std::unique_ptr<PtJson> nativeRange;
    ret = params.GetArray("nativeRange", &nativeRange);
    if (ret == Result::SUCCESS) {
        int32_t len = nativeRange->GetSize();
        std::vector<NativeRange> vectorNativeRange;
        for (int32_t i = 0; i < len; ++i) {
            std::unique_ptr<NativeRange> obj = NativeRange::Create(*nativeRange->Get(i));
            if (obj == nullptr) {
                error += "'nativeRange' is invalid;";
                break;
            } else {
                vectorNativeRange.emplace_back(std::move(*obj));
            }
        }
        if (vectorNativeRange.size()) {
            paramsObject->nativeRange_ = std::move(vectorNativeRange);
        }
    } else if (ret == Result::TYPE_ERROR) {
        error += "Unknown 'nativeRange';";
    }

    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "SetNativeRangeParams::Create " << error;
        return nullptr;
    }

    return paramsObject;
}

std::unique_ptr<SetMixedDebugParams> SetMixedDebugParams::Create(const PtJson &params)
{
    auto paramsObject = std::make_unique<SetMixedDebugParams>();
    std::string error;
    Result ret;

    bool enabled = false;
    ret = params.GetBool("enabled", &enabled);
    if (ret == Result::SUCCESS) {
        paramsObject->enabled_ = enabled;
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'enabled';";
    }

    bool mixedStackEnabled = false;
    ret = params.GetBool("mixedStackEnabled", &mixedStackEnabled);
    if (ret == Result::SUCCESS) {
        paramsObject->mixedStackEnabled_ = mixedStackEnabled;
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'enabled';";
    }

    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "SetMixedDebugParams::Create " << error;
        return nullptr;
    }

    return paramsObject;
}

std::unique_ptr<ResetSingleStepperParams> ResetSingleStepperParams::Create(const PtJson &params)
{
    auto paramsObject = std::make_unique<ResetSingleStepperParams>();
    std::string error;
    Result ret;

    bool resetSingleStepper = false;
    ret = params.GetBool("resetSingleStepper", &resetSingleStepper);
    if (ret == Result::SUCCESS) {
        paramsObject->resetSingleStepper_ = resetSingleStepper;
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'resetSingleStepper';";
    }

    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "ResetSingleStepperParams::Create " << error;
        return nullptr;
    }

    return paramsObject;
}

std::unique_ptr<ReplyNativeCallingParams> ReplyNativeCallingParams::Create(const PtJson &params)
{
    auto paramsObject = std::make_unique<ReplyNativeCallingParams>();
    std::string error;
    Result ret;

    bool userCode = false;
    ret = params.GetBool("userCode", &userCode);
    if (ret == Result::SUCCESS) {
        paramsObject->userCode_ = userCode;
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'userCode';";
    }

    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "ReplyNativeCallingParams::Create " << error;
        return nullptr;
    }

    return paramsObject;
}

std::unique_ptr<GetPropertiesParams> GetPropertiesParams::Create(const PtJson &params)
{
    auto paramsObject = std::make_unique<GetPropertiesParams>();
    std::string error;
    Result ret;

    std::string objectId;
    ret = params.GetString("objectId", &objectId);
    if (ret == Result::SUCCESS) {
        if (!ToolchainUtils::StrToInt32(objectId, paramsObject->objectId_)) {
            error += "Failed to convert 'objectId' from string to int;";
        }
    } else {
        error += "Unknown or wrong type of 'objectId';";
    }
    bool ownProperties = false;
    ret = params.GetBool("ownProperties", &ownProperties);
    if (ret == Result::SUCCESS) {
        paramsObject->ownProperties_ = ownProperties;
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'ownProperties';";
    }
    bool accessorPropertiesOnly = false;
    ret = params.GetBool("accessorPropertiesOnly", &accessorPropertiesOnly);
    if (ret == Result::SUCCESS) {
        paramsObject->accessorPropertiesOnly_ = accessorPropertiesOnly;
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'accessorPropertiesOnly';";
    }
    bool generatePreview = false;
    ret = params.GetBool("generatePreview", &generatePreview);
    if (ret == Result::SUCCESS) {
        paramsObject->generatePreview_ = generatePreview;
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'generatePreview';";
    }
    // Try to get "start" and "count" from request parameter
    int32_t startIndex = INVALID_START_OR_COUNT;
    ret = params.GetInt("start", &startIndex);
    if (ret == Result::SUCCESS) {
        paramsObject->startIndex_ = startIndex;
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'start';";
    }
    int32_t groupCount = INVALID_START_OR_COUNT;
    ret = params.GetInt("count", &groupCount);
    if (ret == Result::SUCCESS) {
        paramsObject->groupCount_ = groupCount;
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'count';";
    }
    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "GetPropertiesParams::Create " << error;
        return nullptr;
    }

    return paramsObject;
}

std::unique_ptr<CallFunctionOnParams> CallFunctionOnParams::Create(const PtJson &params)
{
    auto paramsObject = std::make_unique<CallFunctionOnParams>();
    std::string error;
    Result ret;

    // paramsObject->callFrameId_
    std::string callFrameId;
    ret = params.GetString("callFrameId", &callFrameId);
    if (ret == Result::SUCCESS) {
        if (!ToolchainUtils::StrToInt32(callFrameId, paramsObject->callFrameId_)) {
            error += "Failed to convert 'callFrameId' from string to int;";
        }
    } else {
        error += "Unknown or wrong type of 'callFrameId';";
    }
    // paramsObject->functionDeclaration_
    std::string functionDeclaration;
    ret = params.GetString("functionDeclaration", &functionDeclaration);
    if (ret == Result::SUCCESS) {
        paramsObject->functionDeclaration_ = std::move(functionDeclaration);
    } else {
        error += "Unknown or wrong type of 'functionDeclaration';";
    }
    // paramsObject->objectId_
    std::string objectId;
    ret = params.GetString("objectId", &objectId);
    if (ret == Result::SUCCESS) {
        if (!ToolchainUtils::StrToInt32(objectId, paramsObject->objectId_)) {
            error += "Failed to convert 'objectId' from string to int;";
        }
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'objectId';";
    }
    // paramsObject->arguments_
    std::unique_ptr<PtJson> arguments;
    ret = params.GetArray("arguments", &arguments);
    if (ret == Result::SUCCESS) {
        int32_t len = arguments->GetSize();
        std::vector<std::unique_ptr<CallArgument>> callArgument;
        for (int32_t i = 0; i < len; ++i) {
            std::unique_ptr<CallArgument> obj = CallArgument::Create(*arguments->Get(i));
            if (obj == nullptr) {
                error += "'arguments' items CallArgument is invaild;";
                break;
            } else {
                callArgument.emplace_back(std::move(obj));
            }
        }
        if (callArgument.size()) {
            paramsObject->arguments_ = std::move(callArgument);
        }
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'arguments';";
    }
    // paramsObject->silent_
    bool silent = false;
    ret = params.GetBool("silent", &silent);
    if (ret == Result::SUCCESS) {
        paramsObject->silent_ = silent;
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'silent';";
    }
    // paramsObject->returnByValue_
    bool returnByValue = false;
    ret = params.GetBool("returnByValue", &returnByValue);
    if (ret == Result::SUCCESS) {
        paramsObject->returnByValue_ = returnByValue;
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'returnByValue';";
    }
    // paramsObject->generatePreview_
    bool generatePreview = false;
    ret = params.GetBool("generatePreview", &generatePreview);
    if (ret == Result::SUCCESS) {
        paramsObject->generatePreview_ = generatePreview;
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'generatePreview';";
    }
    // paramsObject->userGesture_
    bool userGesture = false;
    ret = params.GetBool("userGesture", &userGesture);
    if (ret == Result::SUCCESS) {
        paramsObject->userGesture_ = userGesture;
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'userGesture';";
    }
    // paramsObject->awaitPromise_
    bool awaitPromise = false;
    ret = params.GetBool("awaitPromise", &awaitPromise);
    if (ret == Result::SUCCESS) {
        paramsObject->awaitPromise_ = awaitPromise;
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'awaitPromise';";
    }
    // paramsObject->executionContextId_
    int32_t executionContextId;
    ret = params.GetInt("executionContextId", &executionContextId);
    if (ret == Result::SUCCESS) {
        paramsObject->executionContextId_ = executionContextId;
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'executionContextId';";
    }
    // paramsObject->objectGroup_
    std::string objectGroup;
    ret = params.GetString("objectGroup", &objectGroup);
    if (ret == Result::SUCCESS) {
        paramsObject->objectGroup_ = std::move(objectGroup);
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'objectGroup';";
    }
    // paramsObject->throwOnSideEffect_
    bool throwOnSideEffect = false;
    ret = params.GetBool("throwOnSideEffect", &throwOnSideEffect);
    if (ret == Result::SUCCESS) {
        paramsObject->throwOnSideEffect_ = throwOnSideEffect;
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'throwOnSideEffect';";
    }

    // Check whether the error is empty.
    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "CallFunctionOnParams::Create " << error;
        return nullptr;
    }

    return paramsObject;
}

std::unique_ptr<StartSamplingParams> StartSamplingParams::Create(const PtJson &params)
{
    auto paramsObject = std::make_unique<StartSamplingParams>();
    std::string error;
    Result ret;

    double samplingInterval;
    ret = params.GetDouble("samplingInterval", &samplingInterval);
    if (ret == Result::SUCCESS) {
        if (samplingInterval <= 0) {
            error += "Invalid SamplingInterval";
        } else {
            paramsObject->samplingInterval_ = samplingInterval;
        }
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'samplingInterval';";
    }

    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "StartSamplingParams::Create " << error;
        return nullptr;
    }
    return paramsObject;
}

std::unique_ptr<StartTrackingHeapObjectsParams> StartTrackingHeapObjectsParams::Create(const PtJson &params)
{
    auto paramsObject = std::make_unique<StartTrackingHeapObjectsParams>();
    std::string error;
    Result ret;

    bool trackAllocations = false;
    ret = params.GetBool("trackAllocations", &trackAllocations);
    if (ret == Result::SUCCESS) {
        paramsObject->trackAllocations_ = trackAllocations;
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'trackAllocations';";
    }

    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "StartTrackingHeapObjectsParams::Create " << error;
        return nullptr;
    }
    return paramsObject;
}

std::unique_ptr<StopTrackingHeapObjectsParams> StopTrackingHeapObjectsParams::Create(const PtJson &params)
{
    auto paramsObject = std::make_unique<StopTrackingHeapObjectsParams>();
    std::string error;
    Result ret;

    bool reportProgress = false;
    ret = params.GetBool("reportProgress", &reportProgress);
    if (ret == Result::SUCCESS) {
        paramsObject->reportProgress_ = reportProgress;
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'reportProgress';";
    }

    bool treatGlobalObjectsAsRoots = false;
    ret = params.GetBool("treatGlobalObjectsAsRoots", &treatGlobalObjectsAsRoots);
    if (ret == Result::SUCCESS) {
        paramsObject->treatGlobalObjectsAsRoots_ = treatGlobalObjectsAsRoots;
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'treatGlobalObjectsAsRoots';";
    }

    bool captureNumericValue = false;
    ret = params.GetBool("captureNumericValue", &captureNumericValue);
    if (ret == Result::SUCCESS) {
        paramsObject->captureNumericValue_ = captureNumericValue;
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'captureNumericValue';";
    }

    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "StopTrackingHeapObjectsParams::Create " << error;
        return nullptr;
    }
    return paramsObject;
}

std::unique_ptr<AddInspectedHeapObjectParams> AddInspectedHeapObjectParams::Create(const PtJson &params)
{
    auto paramsObject = std::make_unique<AddInspectedHeapObjectParams>();
    std::string error;
    Result ret;

    std::string heapObjectId;
    ret = params.GetString("heapObjectId", &heapObjectId);
    if (ret == Result::SUCCESS) {
        if (!ToolchainUtils::StrToInt32(heapObjectId, paramsObject->heapObjectId_)) {
            error += "Failed to convert 'heapObjectId' from string to int;";
        }
    } else {
        error += "Unknown or wrong type of 'heapObjectId';";
    }

    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "AddInspectedHeapObjectParams::Create " << error;
        return nullptr;
    }
    return paramsObject;
}

std::unique_ptr<GetHeapObjectIdParams> GetHeapObjectIdParams::Create(const PtJson &params)
{
    auto paramsObject = std::make_unique<GetHeapObjectIdParams>();
    std::string error;
    Result ret;

    std::string objectId;
    ret = params.GetString("objectId", &objectId);
    if (ret == Result::SUCCESS) {
        if (!ToolchainUtils::StrToInt32(objectId, paramsObject->objectId_)) {
            error += "Failed to convert 'objectId' from string to int;";
        }
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'objectId';";
    }

    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "GetHeapObjectIdParams::Create " << error;
        return nullptr;
    }
    return paramsObject;
}

std::unique_ptr<GetObjectByHeapObjectIdParams> GetObjectByHeapObjectIdParams::Create(const PtJson &params)
{
    auto paramsObject = std::make_unique<GetObjectByHeapObjectIdParams>();
    std::string error;
    Result ret;

    std::string objectId;
    ret = params.GetString("objectId", &objectId);
    if (ret == Result::SUCCESS) {
        if (!ToolchainUtils::StrToInt32(objectId, paramsObject->objectId_)) {
            error += "Failed to convert 'objectId' from string to int;";
        }
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'objectId';";
    }

    std::string objectGroup;
    ret = params.GetString("objectGroup", &objectGroup);
    if (ret == Result::SUCCESS) {
        paramsObject->objectGroup_ = std::move(objectGroup);
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'objectGroup';";
    }

    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "GetObjectByHeapObjectIdParams::Create " << error;
        return nullptr;
    }
    return paramsObject;
}

std::unique_ptr<StartPreciseCoverageParams> StartPreciseCoverageParams::Create(const PtJson &params)
{
    auto paramsObject = std::make_unique<StartPreciseCoverageParams>();
    std::string error;
    Result ret;

    bool callCount = false;
    ret = params.GetBool("callCount", &callCount);
    if (ret == Result::SUCCESS) {
        paramsObject->callCount_ = callCount;
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'callCount';";
    }

    bool detailed = false;
    ret = params.GetBool("detailed", &detailed);
    if (ret == Result::SUCCESS) {
        paramsObject->detailed_ = detailed;
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'detailed';";
    }

    bool allowTriggeredUpdates = false;
    ret = params.GetBool("allowTriggeredUpdates", &allowTriggeredUpdates);
    if (ret == Result::SUCCESS) {
        paramsObject->allowTriggeredUpdates_ = allowTriggeredUpdates;
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'allowTriggeredUpdates';";
    }

    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "StartPreciseCoverageParams::Create " << error;
        return nullptr;
    }
    return paramsObject;
}

std::unique_ptr<SetSamplingIntervalParams> SetSamplingIntervalParams::Create(const PtJson &params)
{
    auto paramsObject = std::make_unique<SetSamplingIntervalParams>();
    std::string error;
    Result ret;

    int32_t interval = 0;
    ret = params.GetInt("interval", &interval);
    if (ret == Result::SUCCESS) {
        paramsObject->interval_ = interval;
    } else {
        error += "Unknown or wrong type of 'interval';";
    }

    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "SetSamplingIntervalParams::Create " << error;
        return nullptr;
    }
    return paramsObject;
}

std::unique_ptr<RecordClockSyncMarkerParams> RecordClockSyncMarkerParams::Create(const PtJson &params)
{
    std::string error;
    auto recordClockSyncMarkerParams = std::make_unique<RecordClockSyncMarkerParams>();
    Result ret;
    
    std::string syncId;
    ret = params.GetString("syncId", &syncId);
    if (ret == Result::SUCCESS) {
        recordClockSyncMarkerParams->syncId_ = syncId;
    } else {
        error += "Unknown or wrong type of 'syncId';";
    }
    
    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "RecordClockSyncMarkerParams::Create " << error;
        return nullptr;
    }

    return recordClockSyncMarkerParams;
}

std::unique_ptr<RequestMemoryDumpParams> RequestMemoryDumpParams::Create(const PtJson &params)
{
    std::string error;
    auto requestMemoryDumpParams = std::make_unique<RequestMemoryDumpParams>();
    Result ret;
    
    bool deterministic = false;
    ret = params.GetBool("deterministic", &deterministic);
    if (ret == Result::SUCCESS) {
        requestMemoryDumpParams->deterministic_ = deterministic;
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'deterministic';";
    }

    std::string levelOfDetail;
    ret = params.GetString("levelOfDetail", &levelOfDetail);
    if (ret == Result::SUCCESS) {
        if (MemoryDumpLevelOfDetailValues::Valid(levelOfDetail)) {
            requestMemoryDumpParams->levelOfDetail_ = std::move(levelOfDetail);
        } else {
            error += "'levelOfDetail' is invalid;";
        }
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'levelOfDetail';";
    }
    
    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "RequestMemoryDumpParams::Create " << error;
        return nullptr;
    }

    return requestMemoryDumpParams;
}

std::unique_ptr<StartParams> StartParams::Create(const PtJson &params)
{
    std::string error;
    auto startParams = std::make_unique<StartParams>();
    Result ret;
    
    std::string categories;
    ret = params.GetString("categories", &categories);
    if (ret == Result::SUCCESS) {
        startParams->categories_ = std::move(categories);
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'categories';";
    }

    std::string options;
    ret = params.GetString("options", &options);
    if (ret == Result::SUCCESS) {
        startParams->options_ = std::move(options);
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'options';";
    }

    int32_t bufferUsageReportingInterval = 0;
    ret = params.GetInt("bufferUsageReportingInterval", &bufferUsageReportingInterval);
    if (ret == Result::SUCCESS) {
        startParams->bufferUsageReportingInterval_ = bufferUsageReportingInterval;
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'bufferUsageReportingInterval';";
    }

    std::string transferMode;
    ret = params.GetString("transferMode", &transferMode);
    if (ret == Result::SUCCESS) {
        if (StartParams::TransferModeValues::Valid(transferMode)) {
            startParams->transferMode_ = std::move(transferMode);
        } else {
            error += "'transferMode' is invalid;";
        }
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'transferMode';";
    }

    std::string streamFormat;
    ret = params.GetString("streamFormat", &streamFormat);
    if (ret == Result::SUCCESS) {
        if (StreamFormatValues::Valid(streamFormat)) {
            startParams->streamFormat_ = std::move(streamFormat);
        } else {
            error += "'streamFormat' is invalid;";
        }
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'streamFormat';";
    }

    std::string streamCompression;
    ret = params.GetString("streamCompression", &streamCompression);
    if (ret == Result::SUCCESS) {
        if (StreamCompressionValues::Valid(streamCompression)) {
            startParams->streamCompression_ = std::move(streamCompression);
        } else {
            error += "'streamCompression' is invalid;";
        }
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'streamCompression';";
    }

    std::unique_ptr<PtJson> traceConfig;
    ret = params.GetObject("traceConfig", &traceConfig);
    if (ret == Result::SUCCESS) {
        std::unique_ptr<TraceConfig> pTraceConfig = TraceConfig::Create(*traceConfig);
        if (pTraceConfig == nullptr) {
            error += "'traceConfig' format is invalid;";
        } else {
            startParams->traceConfig_ = std::move(pTraceConfig);
        }
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'traceConfig';";
    }

    std::string perfettoConfig;
    ret = params.GetString("perfettoConfig", &perfettoConfig);
    if (ret == Result::SUCCESS) {
        startParams->perfettoConfig_ = std::move(perfettoConfig);
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'perfettoConfig';";
    }

    std::string tracingBackend;
    ret = params.GetString("tracingBackend", &tracingBackend);
    if (ret == Result::SUCCESS) {
        if (TracingBackendValues::Valid(tracingBackend)) {
            startParams->tracingBackend_ = std::move(tracingBackend);
        } else {
            error += "'tracingBackend' is invalid;";
        }
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'tracingBackend';";
    }

    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "StartParams::Create " << error;
        return nullptr;
    }

    return startParams;
}

std::unique_ptr<SeriliazationTimeoutCheckEnableParams> SeriliazationTimeoutCheckEnableParams::Create
    (const PtJson &params)
{
    auto paramsObject = std::make_unique<SeriliazationTimeoutCheckEnableParams>();
    std::string error;
    Result ret;

    int32_t threshold = 0;
    ret = params.GetInt("threshold", &threshold);
    if (ret == Result::SUCCESS) {
        paramsObject->threshold_ = threshold;
    } else {
        error += "Unknown or wrong type of 'threshold';";
    }

    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "SeriliazationTimeoutCheckEnableParams::Create " << error;
        return nullptr;
    }
    return paramsObject;
}

std::unique_ptr<SaveAllPossibleBreakpointsParams> SaveAllPossibleBreakpointsParams::Create(const PtJson &params)
{
    auto paramsObject = std::make_unique<SaveAllPossibleBreakpointsParams>();
    std::unordered_map<std::string, std::vector<std::shared_ptr<BreakpointInfo>>> breakpointMap {};
    std::string error;
    Result ret;

    std::unique_ptr<PtJson> locationList;
    ret = params.GetObject("locations", &locationList);
    if (ret != Result::SUCCESS) {
        LOG_DEBUGGER(ERROR) << "SaveAllPossibleBreakpointsParams::Get Breakpoints location: " << error;
        return nullptr;
    }

    auto keys = locationList->GetKeysArray();
    std::unique_ptr<PtJson> breakpoints;
    for (const auto &key : keys) {
        ret = locationList->GetArray(key.c_str(), &breakpoints);
        if (ret == Result::SUCCESS) {
            int32_t length = breakpoints->GetSize();
            std::vector<std::shared_ptr<BreakpointInfo>> breakpointList;
            for (int32_t i = 0; i < length; i++) {
                auto json = *breakpoints->Get(i);
                json.Add("url", key.c_str());
                std::shared_ptr<BreakpointInfo> info = BreakpointInfo::Create(json);
                if (info == nullptr) {
                    error += "'breakpoints' items BreakpointInfo is invalid;";
                    break;
                }
                breakpointList.emplace_back(std::move(info));
            }
            breakpointMap[key] = breakpointList;
        } else if (ret == Result::TYPE_ERROR) {
            error += "Wrong type of 'breakpoints'";
        }
    }
    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "SaveAllPossibleBreakpointsParams::Create " << error;
        return nullptr;
    }
    paramsObject->breakpointsMap_ = breakpointMap;

    return paramsObject;
}

std::unique_ptr<SetSymbolicBreakpointsParams> SetSymbolicBreakpointsParams::Create(const PtJson &params)
{
    auto paramsObject = std::make_unique<SetSymbolicBreakpointsParams>();
    std::unordered_set<std::string> functionNamesSet {};
    std::string error;
    Result ret;

    std::unique_ptr<PtJson> symbolicBreakpoints;
    ret = params.GetArray("symbolicBreakpoints", &symbolicBreakpoints);
    if (ret != Result::SUCCESS) {
        LOG_DEBUGGER(ERROR) << "SetSymbolicBreakpointsParams::Get SymbolicBreakpoints: " << error;
        return nullptr;
    }

    int32_t length = symbolicBreakpoints->GetSize();
    for (int32_t i = 0; i < length; i++) {
        auto json = symbolicBreakpoints->Get(i);
        std::string functionName;
        ret = json->GetString("functionName", &functionName);
        if (ret == Result::SUCCESS) {
            functionNamesSet.insert(std::move(functionName));
        } else if (ret == Result::TYPE_ERROR) {
            error += "Wrong type of 'functionName';";
        } else if (ret == Result::NOT_EXIST) {
            error += "'functionName' no exists;";
        }
    }

    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "SetSymbolicBreakpointsParams::Create " << error;
        return nullptr;
    }
    paramsObject->functionNamesSet_ = functionNamesSet;

    return paramsObject;
}

std::unique_ptr<RemoveSymbolicBreakpointsParams> RemoveSymbolicBreakpointsParams::Create(const PtJson &params)
{
    auto paramsObject = std::make_unique<RemoveSymbolicBreakpointsParams>();
    std::unordered_set<std::string> functionNamesSet {};
    std::string error;
    Result ret;

    std::unique_ptr<PtJson> symbolicBreakpoints;
    ret = params.GetArray("symbolicBreakpoints", &symbolicBreakpoints);
    if (ret != Result::SUCCESS) {
        LOG_DEBUGGER(ERROR) << "RemoveSymbolicBreakpointsParams::Get SymbolicBreakpoints: " << error;
        return nullptr;
    }

    int32_t length = symbolicBreakpoints->GetSize();
    for (int32_t i = 0; i < length; i++) {
        auto json = symbolicBreakpoints->Get(i);
        std::string functionName;
        ret = json->GetString("functionName", &functionName);
        if (ret == Result::SUCCESS) {
            functionNamesSet.insert(std::move(functionName));
        } else if (ret == Result::TYPE_ERROR) {
            error += "Wrong type of 'functionName';";
        } else if (ret == Result::NOT_EXIST) {
            error += "'functionName' no exists;";
        }
    }

    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "RemoveSymbolicBreakpointsParams::Create " << error;
        return nullptr;
    }
    paramsObject->functionNamesSet_ = functionNamesSet;

    return paramsObject;
}

}  // namespace panda::ecmascript::tooling
