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

#include "tooling/dynamic/base/pt_events.h"

namespace panda::ecmascript::tooling {
std::unique_ptr<PtJson> BreakpointResolved::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();
    result->Add("breakpointId", breakpointId_.c_str());
    ASSERT(location_ != nullptr);
    result->Add("location", location_->ToJson());

    std::unique_ptr<PtJson> object = PtJson::CreateObject();
    object->Add("method", GetName().c_str());
    object->Add("params", result);

    return object;
}

std::unique_ptr<PtJson> Paused::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    std::unique_ptr<PtJson> array = PtJson::CreateArray();
    size_t len = callFrames_.size();
    for (size_t i = 0; i < len; i++) {
        ASSERT(callFrames_[i] != nullptr);
        array->Push(callFrames_[i]->ToJson());
    }
    result->Add("callFrames", array);
    result->Add("reason", reason_.c_str());
    if (data_) {
        ASSERT(data_.value() != nullptr);
        result->Add("data", data_.value()->ToJson());
    }
    if (hitBreakpoints_) {
        std::unique_ptr<PtJson> breakpoints = PtJson::CreateArray();
        len = hitBreakpoints_->size();
        for (size_t i = 0; i < len; i++) {
            breakpoints->Push(hitBreakpoints_.value()[i].c_str());
        }
        result->Add("hitBreakpoints", breakpoints);
    }

    if (asyncStack_ && asyncCallChainDepth_) {
        result->Add("asyncStackTrace", ToJson(*asyncStack_, asyncCallChainDepth_ - 1));
    }

    std::unique_ptr<PtJson> object = PtJson::CreateObject();
    object->Add("method", GetName().c_str());
    object->Add("params", result);
    return object;
}

std::unique_ptr<PtJson> Paused::ToJson(StackFrame stackFrame) const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    std::string functionName = stackFrame.GetFunctionName();
    std::string url = stackFrame.GetUrl();
    int32_t scriptId = stackFrame.GetScriptId();
    int32_t lineNumber = stackFrame.GetLineNumber();
    int32_t columnNumber = stackFrame.GetColumnNumber();
    result->Add("functionName", functionName.c_str());
    result->Add("scriptId", scriptId);
    result->Add("url", url.c_str());
    result->Add("lineNumber", lineNumber);
    result->Add("columnNumber", columnNumber);

    return result;
}

std::unique_ptr<PtJson> Paused::ToJson(AsyncStack asyncStack, int32_t asyncCallChainDepth) const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    std::unique_ptr<PtJson> array = PtJson::CreateArray();
    std::vector<std::shared_ptr<StackFrame>> callFrames = asyncStack.GetFrames();
    size_t len = callFrames.size();
    for (size_t i = 0; i < len; i++) {
        array->Push(ToJson(*callFrames[i]));
    }
    result->Add("callFrames", array);
    std::string description = asyncStack.GetDescription();
    result->Add("description", description.c_str());

    std::weak_ptr<AsyncStack> weakAsyncStack = asyncStack.GetAsyncParent();
    auto sharedAsyncStack = weakAsyncStack.lock();
    if (sharedAsyncStack && asyncCallChainDepth) {
        asyncCallChainDepth--;
        result->Add("parent", ToJson(*sharedAsyncStack, asyncCallChainDepth));
    }

    return result;
}

std::unique_ptr<PtJson> Resumed::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    std::unique_ptr<PtJson> object = PtJson::CreateObject();
    object->Add("method", GetName().c_str());
    object->Add("params", result);

    return object;
}

std::unique_ptr<PtJson> NativeCalling::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    result->Add("nativeAddress", reinterpret_cast<int64_t>(GetNativeAddress()));
    result->Add("isStepInto", GetIntoStatus());

    std::unique_ptr<PtJson> object = PtJson::CreateObject();
    object->Add("method", GetName().c_str());
    object->Add("params", result);

    return object;
}

std::unique_ptr<PtJson> MixedStack::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    std::unique_ptr<PtJson> nativePointerArray = PtJson::CreateArray();
    size_t nativePointerLength = nativePointer_.size();
    for (size_t i = 0; i < nativePointerLength; ++i) {
        nativePointerArray->Push(reinterpret_cast<int64_t>(nativePointer_[i]));
    }
    result->Add("nativePointer", nativePointerArray);

    std::unique_ptr<PtJson> callFrameArray = PtJson::CreateArray();
    size_t callFrameLength = callFrames_.size();
    for (size_t i = 0; i < callFrameLength; ++i) {
        ASSERT(callFrames_[i] != nullptr);
        callFrameArray->Push(callFrames_[i]->ToJson());
    }
    result->Add("callFrames", callFrameArray);

    std::unique_ptr<PtJson> object = PtJson::CreateObject();
    object->Add("method", GetName().c_str());
    object->Add("params", result);

    return object;
}

std::unique_ptr<PtJson> ScriptFailedToParse::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    result->Add("scriptId", std::to_string(scriptId_).c_str());
    result->Add("url", url_.c_str());
    result->Add("startLine", startLine_);
    result->Add("startColumn", startColumn_);
    result->Add("endLine", endLine_);
    result->Add("endColumn", endColumn_);
    result->Add("executionContextId", executionContextId_);
    result->Add("hash", hash_.c_str());
    if (sourceMapUrl_) {
        result->Add("sourceMapURL", sourceMapUrl_->c_str());
    }
    if (hasSourceUrl_) {
        result->Add("hasSourceURL", hasSourceUrl_.value());
    }
    if (isModule_) {
        result->Add("isModule", isModule_.value());
    }
    if (length_) {
        result->Add("length", length_.value());
    }
    if (codeOffset_) {
        result->Add("codeOffset", codeOffset_.value());
    }
    if (scriptLanguage_) {
        result->Add("scriptLanguage", scriptLanguage_->c_str());
    }
    if (embedderName_) {
        result->Add("embedderName", embedderName_->c_str());
    }

    std::unique_ptr<PtJson> object = PtJson::CreateObject();
    object->Add("method", GetName().c_str());
    object->Add("params", result);

    return object;
}

std::unique_ptr<PtJson> ScriptParsed::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    result->Add("scriptId", std::to_string(scriptId_).c_str());
    result->Add("url", url_.c_str());
    result->Add("startLine", startLine_);
    result->Add("startColumn", startColumn_);
    result->Add("endLine", endLine_);
    result->Add("endColumn", endColumn_);
    result->Add("executionContextId", executionContextId_);
    result->Add("hash", hash_.c_str());
    if (isLiveEdit_) {
        result->Add("isLiveEdit", isLiveEdit_.value());
    }
    if (sourceMapUrl_) {
        result->Add("sourceMapURL", sourceMapUrl_->c_str());
    }
    if (hasSourceUrl_) {
        result->Add("hasSourceURL", hasSourceUrl_.value());
    }
    if (isModule_) {
        result->Add("isModule", isModule_.value());
    }
    if (length_) {
        result->Add("length", length_.value());
    }
    if (codeOffset_) {
        result->Add("codeOffset", codeOffset_.value());
    }
    if (scriptLanguage_) {
        result->Add("scriptLanguage", scriptLanguage_->c_str());
    }
    if (embedderName_) {
        result->Add("embedderName", embedderName_->c_str());
    }

    std::unique_ptr<PtJson> array = PtJson::CreateArray();
    size_t len = locations_.size();
    for (size_t i = 0; i < len; i++) {
        ASSERT(locations_[i] != nullptr);
        std::unique_ptr<PtJson> location = locations_[i]->ToJson();
        array->Push(location);
    }
    result->Add("locations", array);

    std::unique_ptr<PtJson> object = PtJson::CreateObject();
    object->Add("method", GetName().c_str());
    object->Add("params", result);

    return object;
}

std::unique_ptr<PtJson> AddHeapSnapshotChunk::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    result->Add("chunk", chunk_.c_str());

    std::unique_ptr<PtJson> object = PtJson::CreateObject();
    object->Add("method", GetName().c_str());
    object->Add("params", result);

    return object;
}

std::unique_ptr<PtJson> ConsoleProfileFinished::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    result->Add("id", id_.c_str());
    ASSERT(location_ != nullptr);
    result->Add("location", location_->ToJson());
    ASSERT(profile_ != nullptr);
    result->Add("profile", profile_->ToJson());
    if (title_) {
        result->Add("title", title_->c_str());
    }

    std::unique_ptr<PtJson> object = PtJson::CreateObject();
    object->Add("method", GetName().c_str());
    object->Add("params", result);

    return object;
}

std::unique_ptr<PtJson> ConsoleProfileStarted::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    result->Add("id", id_.c_str());
    ASSERT(location_ != nullptr);
    result->Add("location", location_->ToJson());
    if (title_) {
        result->Add("title", title_->c_str());
    }

    std::unique_ptr<PtJson> object = PtJson::CreateObject();
    object->Add("method", GetName().c_str());
    object->Add("params", result);

    return object;
}

std::unique_ptr<PtJson> PreciseCoverageDeltaUpdate::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    result->Add("timestamp", timestamp_);
    result->Add("occasion", occasion_.c_str());
    std::unique_ptr<PtJson> array = PtJson::CreateArray();
    size_t len = result_.size();
    for (size_t i = 0; i < len; i++) {
        ASSERT(result_[i] != nullptr);
        std::unique_ptr<PtJson> res = result_[i]->ToJson();
        array->Push(res);
    }
    result->Add("result", array);

    std::unique_ptr<PtJson> object = PtJson::CreateObject();
    object->Add("method", GetName().c_str());
    object->Add("params", result);

    return object;
}

std::unique_ptr<PtJson> HeapStatsUpdate::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    std::unique_ptr<PtJson> array = PtJson::CreateArray();
    size_t len = statsUpdate_.size();
    for (size_t i = 0; i < len; i++) {
        array->Push(statsUpdate_[i]);
    }
    result->Add("statsUpdate", array);

    std::unique_ptr<PtJson> object = PtJson::CreateObject();
    object->Add("method", GetName().c_str());
    object->Add("params", result);

    return object;
}

std::unique_ptr<PtJson> LastSeenObjectId::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    result->Add("lastSeenObjectId", lastSeenObjectId_);
    result->Add("timestamp", timestamp_);

    std::unique_ptr<PtJson> object = PtJson::CreateObject();
    object->Add("method", GetName().c_str());
    object->Add("params", result);

    return object;
}

std::unique_ptr<PtJson> ReportHeapSnapshotProgress::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    result->Add("done", done_);
    result->Add("total", total_);
    if (finished_) {
        result->Add("finished", finished_.value());
    }

    std::unique_ptr<PtJson> object = PtJson::CreateObject();
    object->Add("method", GetName().c_str());
    object->Add("params", result);

    return object;
}

std::unique_ptr<PtJson> BufferUsage::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    if (percentFull_) {
        result->Add("percentFull", percentFull_.value());
    }
    if (eventCount_) {
        result->Add("eventCount", eventCount_.value());
    }
    if (value_) {
        result->Add("value", value_.value());
    }

    std::unique_ptr<PtJson> object = PtJson::CreateObject();
    object->Add("method", GetName().c_str());
    object->Add("params", result);

    return object;
}

std::unique_ptr<PtJson> DataCollected::TraceEventToJson(TraceEvent &traceEvent) const
{
    std::unique_ptr<PtJson> event = PtJson::CreateObject();
    std::unique_ptr<PtJson> args = PtJson::Parse(traceEvent.args_);
    event->Add("args", args);
    event->Add("cat", traceEvent.cat_.c_str());

    if (traceEvent.dur_ != 0) {
        event->Add("dur", traceEvent.dur_);
    }

    if (!traceEvent.id_.empty()) {
        event->Add("id", traceEvent.id_.c_str());
    }

    event->Add("name", traceEvent.name_.c_str());
    event->Add("ph", traceEvent.ph_.c_str());
    event->Add("pid", traceEvent.pid_);

    if (!traceEvent.s_.empty()) {
        event->Add("s", traceEvent.s_.c_str());
    }

    if (traceEvent.tdur_ != 0) {
        event->Add("tdur", traceEvent.tdur_);
    }

    event->Add("tid", traceEvent.tid_);
    event->Add("ts", traceEvent.ts_);

    if (traceEvent.tts_ != 0) {
        event->Add("tts", traceEvent.tts_);
    }

    return event;
}

std::unique_ptr<PtJson> DataCollected::ToJson() const
{
    std::unique_ptr<PtJson> traceEvents = PtJson::CreateArray();
    for (auto &traceEvent : *traceEvents_) {
        traceEvents->Push(TraceEventToJson(traceEvent));
    }

    std::unique_ptr<PtJson> value = PtJson::CreateObject();
    value->Add("value", traceEvents);

    std::unique_ptr<PtJson> object = PtJson::CreateObject();
    object->Add("method", GetName().c_str());
    object->Add("params", value);

    return object;
}

std::unique_ptr<PtJson> TracingComplete::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    result->Add("dataLossOccurred", dataLossOccurred_);
    if (traceFormat_) {
        result->Add("traceFormat", traceFormat_.value()->c_str());
    }
    if (streamCompression_) {
        result->Add("streamCompression", streamCompression_.value()->c_str());
    }

    std::unique_ptr<PtJson> object = PtJson::CreateObject();
    object->Add("method", GetName().c_str());
    object->Add("params", result);

    return object;
}
}  // namespace panda::ecmascript::tooling
