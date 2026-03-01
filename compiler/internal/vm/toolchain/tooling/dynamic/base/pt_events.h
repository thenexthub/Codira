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

#ifndef ECMASCRIPT_TOOLING_BASE_PT_EVENTS_H
#define ECMASCRIPT_TOOLING_BASE_PT_EVENTS_H

#include <memory>
#include <optional>

#include "ecmascript/dfx/tracing/tracing.h"
#include "tooling/dynamic/base/pt_script.h"
#include "tooling/dynamic/base/pt_types.h"
#include "dispatcher.h"

#include "libpandabase/macros.h"

namespace panda::ecmascript::tooling {
class PtBaseEvents : public PtBaseTypes {
public:
    PtBaseEvents() = default;
    ~PtBaseEvents() override = default;
    virtual std::string GetName() const = 0;

private:
    NO_COPY_SEMANTIC(PtBaseEvents);
    NO_MOVE_SEMANTIC(PtBaseEvents);
};

class BreakpointResolved final : public PtBaseEvents {
public:
    BreakpointResolved() = default;
    ~BreakpointResolved() override = default;
    std::unique_ptr<PtJson> ToJson() const override;

    std::string GetName() const override
    {
        return "Debugger.breakpointResolved";
    }

    BreakpointId GetBreakpointId() const
    {
        return breakpointId_;
    }

    BreakpointResolved &SetBreakpointId(const BreakpointId &breakpointId)
    {
        breakpointId_ = breakpointId;
        return *this;
    }

    Location *GetLocation() const
    {
        return location_.get();
    }

    BreakpointResolved &SetLocation(std::unique_ptr<Location> location)
    {
        location_ = std::move(location);
        return *this;
    }

private:
    NO_COPY_SEMANTIC(BreakpointResolved);
    NO_MOVE_SEMANTIC(BreakpointResolved);

    BreakpointId breakpointId_ {};
    std::unique_ptr<Location> location_ {nullptr};
};

class Paused final : public PtBaseEvents {
public:
    Paused() = default;
    ~Paused() override = default;
    std::unique_ptr<PtJson> ToJson() const override;
    std::unique_ptr<PtJson> ToJson(StackFrame stackFrame) const;
    std::unique_ptr<PtJson> ToJson(AsyncStack asyncStack, int32_t asyncCallChainDepth) const;

    std::string GetName() const override
    {
        return "Debugger.paused";
    }

    const std::vector<std::unique_ptr<CallFrame>> *GetCallFrames() const
    {
        return &callFrames_;
    }

    const std::shared_ptr<AsyncStack> *GetAsyncStack() const
    {
        return &asyncStack_;
    }

    Paused &SetAysncStack(std::shared_ptr<AsyncStack> AsyncStack)
    {
        asyncStack_ = std::move(AsyncStack);
        return *this;
    }

    Paused &SetCallFrames(std::vector<std::unique_ptr<CallFrame>> callFrames)
    {
        callFrames_ = std::move(callFrames);
        return *this;
    }

    const std::string &GetReason() const
    {
        return reason_;
    }

    Paused &SetReason(PauseReason reason)
    {
        reason_ = GetReasonString(reason);
        return *this;
    }

    static std::string GetReasonString(PauseReason reason)
    {
        switch (reason) {
            case AMBIGUOUS: {
                return "ambiguous";
            }
            case ASSERT: {
                return "assert";
            }
            case DEBUGCOMMAND: {
                return "debugCommand";
            }
            case DOM: {
                return "DOM";
            }
            case EVENTLISTENER: {
                return "EventListener";
            }
            case EXCEPTION: {
                return "exception";
            }
            case INSTRUMENTATION: {
                return "instrumentation";
            }
            case OOM: {
                return "OOM";
            }
            case OTHER: {
                return "other";
            }
            case PROMISEREJECTION: {
                return "promiseRejection";
            }
            case XHR: {
                return "XHR";
            }
            case BREAK_ON_START: {
                return "Break on start";
            }
            case DEBUGGERSTMT: {
                return "Debugger statement";
            }
            case STEP: {
                return "Step";
            }
            case NATIVE_OUT: {
                return "Native out";
            }
            case SYMBOL: {
                return "Symbol";
            }
            default: {
                LOG_DEBUGGER(ERROR) << "Unknown paused reason: " << reason;
            }
        }
        return "";
    }

    RemoteObject *GetData() const
    {
        if (data_) {
            return data_->get();
        }
        return nullptr;
    }

    Paused &SetData(std::unique_ptr<RemoteObject> data)
    {
        data_ = std::move(data);
        return *this;
    }

    bool HasData() const
    {
        return data_.has_value();
    }

    Paused &SetAsyncCallChainDepth(int32_t asyncCallChainDepth)
    {
        asyncCallChainDepth_ = asyncCallChainDepth;
        return *this;
    }

    int32_t GetAsyncCallChainDepth() const
    {
        return asyncCallChainDepth_;
    }

    std::vector<BreakpointId> GetHitBreakpoints() const
    {
        return hitBreakpoints_.value_or(std::vector<BreakpointId>());
    }

    Paused &SetHitBreakpoints(std::vector<BreakpointId> hitBreakpoints)
    {
        hitBreakpoints_ = std::move(hitBreakpoints);
        return *this;
    }

    bool HasHitBreakpoints() const
    {
        return hitBreakpoints_.has_value();
    }

private:
    NO_COPY_SEMANTIC(Paused);
    NO_MOVE_SEMANTIC(Paused);

    std::vector<std::unique_ptr<CallFrame>> callFrames_ {};
    std::shared_ptr<AsyncStack> asyncStack_ {};
    std::string reason_ {};
    std::optional<std::unique_ptr<RemoteObject>> data_ {};
    std::optional<std::vector<BreakpointId>> hitBreakpoints_ {};
    int32_t asyncCallChainDepth_ {0};
};

class Resumed final : public PtBaseEvents {
public:
    Resumed() = default;
    ~Resumed() override = default;
    std::unique_ptr<PtJson> ToJson() const override;

    std::string GetName() const override
    {
        return "Debugger.resumed";
    }

private:
    NO_COPY_SEMANTIC(Resumed);
    NO_MOVE_SEMANTIC(Resumed);
};

class NativeCalling final : public PtBaseEvents {
public:
    NativeCalling() = default;
    ~NativeCalling() override = default;
    std::unique_ptr<PtJson> ToJson() const override;

    std::string GetName() const override
    {
        return "Debugger.nativeCalling";
    }

    const void *GetNativeAddress() const
    {
        return nativeAddress_;
    }

    bool GetIntoStatus() const
    {
        return isStepInto_.value_or(false);
    }

    NativeCalling &SetNativeAddress(const void *nativeAddress)
    {
        nativeAddress_ = nativeAddress;
        return *this;
    }

    NativeCalling &SetIntoStatus(bool isStepInto)
    {
        isStepInto_ = isStepInto;
        return *this;
    }

private:
    NO_COPY_SEMANTIC(NativeCalling);
    NO_MOVE_SEMANTIC(NativeCalling);

    const void *nativeAddress_ {nullptr};
    std::optional<bool> isStepInto_ {};
};

class MixedStack final : public PtBaseEvents {
public:
    MixedStack() = default;
    ~MixedStack() override = default;
    std::unique_ptr<PtJson> ToJson() const override;

    std::string GetName() const override
    {
        return "Debugger.mixedStack";
    }

    const std::vector<std::unique_ptr<CallFrame>> *GetCallFrames() const
    {
        return &callFrames_;
    }

    const std::vector<void *> *GetNativePointers() const
    {
        return &nativePointer_;
    }

    MixedStack &SetCallFrames(std::vector<std::unique_ptr<CallFrame>> callFrames)
    {
        callFrames_ = std::move(callFrames);
        return *this;
    }

    MixedStack &SetNativePointers(std::vector<void *> nativePointer)
    {
        nativePointer_ = std::move(nativePointer);
        return *this;
    }

private:
    NO_COPY_SEMANTIC(MixedStack);
    NO_MOVE_SEMANTIC(MixedStack);

    std::vector<void *> nativePointer_ {};
    std::vector<std::unique_ptr<CallFrame>> callFrames_ {};
};

class ScriptFailedToParse final : public PtBaseEvents {
public:
    ScriptFailedToParse() = default;
    ~ScriptFailedToParse() override = default;
    std::unique_ptr<PtJson> ToJson() const override;

    std::string GetName() const override
    {
        return "Debugger.scriptFailedToParse";
    }

    ScriptId GetScriptId() const
    {
        return scriptId_;
    }

    ScriptFailedToParse &SetScriptId(ScriptId scriptId)
    {
        scriptId_ = scriptId;
        return *this;
    }

    const std::string &GetUrl() const
    {
        return url_;
    }

    ScriptFailedToParse &SetUrl(const std::string &url)
    {
        url_ = url;
        return *this;
    }

    int32_t GetStartLine() const
    {
        return startLine_;
    }

    ScriptFailedToParse &SetStartLine(int32_t startLine)
    {
        startLine_ = startLine;
        return *this;
    }

    int32_t GetStartColumn() const
    {
        return startColumn_;
    }

    ScriptFailedToParse &SetStartColumn(int32_t startColumn)
    {
        startColumn_ = startColumn;
        return *this;
    }

    int32_t GetEndLine() const
    {
        return endLine_;
    }

    ScriptFailedToParse &SetEndLine(int32_t endLine)
    {
        endLine_ = endLine;
        return *this;
    }

    int32_t GetEndColumn() const
    {
        return endColumn_;
    }

    ScriptFailedToParse &SetEndColumn(int32_t endColumn)
    {
        endColumn_ = endColumn;
        return *this;
    }

    int32_t GetExecutionContextId() const
    {
        return executionContextId_;
    }

    ScriptFailedToParse &SetExecutionContextId(int32_t executionContextId)
    {
        executionContextId_ = executionContextId;
        return *this;
    }

    const std::string &GetHash() const
    {
        return hash_;
    }

    ScriptFailedToParse &SetHash(const std::string &hash)
    {
        hash_ = hash;
        return *this;
    }

    Local<ObjectRef> GetExecutionContextAuxData() const
    {
        return execContextAuxData_.value_or(Local<ObjectRef>());
    }

    ScriptFailedToParse &SetExecutionContextAuxData(Local<ObjectRef> execContextAuxData)
    {
        execContextAuxData_ = execContextAuxData;
        return *this;
    }

    bool HasExecutionContextAuxData() const
    {
        return execContextAuxData_.has_value();
    }

    const std::string &GetSourceMapURL() const
    {
        ASSERT(HasSourceMapUrl());
        return sourceMapUrl_.value();
    }

    ScriptFailedToParse &SetSourceMapURL(const std::string &sourceMapUrl)
    {
        sourceMapUrl_ = sourceMapUrl;
        return *this;
    }

    bool HasSourceMapUrl() const
    {
        return sourceMapUrl_.has_value();
    }

    bool GetHasSourceURL() const
    {
        return hasSourceUrl_.value_or(false);
    }

    ScriptFailedToParse &SetHasSourceURL(bool hasSourceUrl)
    {
        hasSourceUrl_ = hasSourceUrl;
        return *this;
    }

    bool HasHasSourceURL() const
    {
        return hasSourceUrl_.has_value();
    }

    bool GetIsModule() const
    {
        return isModule_.value_or(false);
    }

    ScriptFailedToParse &SetIsModule(bool isModule)
    {
        isModule_ = isModule;
        return *this;
    }

    bool HasIsModule() const
    {
        return isModule_.has_value();
    }

    int32_t GetLength() const
    {
        return length_.value_or(0);
    }

    ScriptFailedToParse &SetLength(int32_t length)
    {
        length_ = length;
        return *this;
    }

    bool HasLength() const
    {
        return length_.has_value();
    }

    int32_t GetCodeOffset() const
    {
        return codeOffset_.value_or(0);
    }

    ScriptFailedToParse &SetCodeOffset(int32_t codeOffset)
    {
        codeOffset_ = codeOffset;
        return *this;
    }

    bool HasCodeOffset() const
    {
        return codeOffset_.has_value();
    }

    const std::string &GetScriptLanguage() const
    {
        ASSERT(HasScriptLanguage());
        return scriptLanguage_.value();
    }

    ScriptFailedToParse &SetScriptLanguage(const std::string &scriptLanguage)
    {
        scriptLanguage_ = scriptLanguage;
        return *this;
    }

    bool HasScriptLanguage() const
    {
        return scriptLanguage_.has_value();
    }

    const std::string &GetEmbedderName() const
    {
        ASSERT(HasEmbedderName());
        return embedderName_.value();
    }

    ScriptFailedToParse &SetEmbedderName(const std::string &embedderName)
    {
        embedderName_ = embedderName;
        return *this;
    }

    bool HasEmbedderName() const
    {
        return embedderName_.has_value();
    }

private:
    NO_COPY_SEMANTIC(ScriptFailedToParse);
    NO_MOVE_SEMANTIC(ScriptFailedToParse);

    ScriptId scriptId_ {0};
    std::string url_ {};
    int32_t startLine_ {0};
    int32_t startColumn_ {0};
    int32_t endLine_ {0};
    int32_t endColumn_ {0};
    ExecutionContextId executionContextId_ {0};
    std::string hash_ {};
    std::optional<Local<ObjectRef>> execContextAuxData_ {};
    std::optional<std::string> sourceMapUrl_ {};
    std::optional<bool> hasSourceUrl_ {};
    std::optional<bool> isModule_ {};
    std::optional<int32_t> length_ {};
    std::optional<int32_t> codeOffset_ {};
    std::optional<std::string> scriptLanguage_ {};
    std::optional<std::string> embedderName_ {};
};

class ScriptParsed final : public PtBaseEvents {
public:
    ScriptParsed() = default;
    ~ScriptParsed() override = default;
    std::unique_ptr<PtJson> ToJson() const override;

    std::string GetName() const override
    {
        return "Debugger.scriptParsed";
    }

    ScriptId GetScriptId() const
    {
        return scriptId_;
    }

    ScriptParsed &SetScriptId(ScriptId scriptId)
    {
        scriptId_ = scriptId;
        return *this;
    }

    const std::string &GetUrl() const
    {
        return url_;
    }

    ScriptParsed &SetUrl(const std::string &url)
    {
        url_ = url;
        return *this;
    }

    int32_t GetStartLine() const
    {
        return startLine_;
    }

    ScriptParsed &SetStartLine(int32_t startLine)
    {
        startLine_ = startLine;
        return *this;
    }

    int32_t GetStartColumn() const
    {
        return startColumn_;
    }

    ScriptParsed &SetStartColumn(int32_t startColumn)
    {
        startColumn_ = startColumn;
        return *this;
    }

    int32_t GetEndLine() const
    {
        return endLine_;
    }

    ScriptParsed &SetEndLine(int32_t endLine)
    {
        endLine_ = endLine;
        return *this;
    }

    int32_t GetEndColumn() const
    {
        return endColumn_;
    }

    ScriptParsed &SetEndColumn(int32_t endColumn)
    {
        endColumn_ = endColumn;
        return *this;
    }

    int32_t GetExecutionContextId() const
    {
        return executionContextId_;
    }

    ScriptParsed &SetExecutionContextId(int32_t executionContextId)
    {
        executionContextId_ = executionContextId;
        return *this;
    }

    const std::string &GetHash() const
    {
        return hash_;
    }

    ScriptParsed &SetHash(const std::string &hash)
    {
        hash_ = hash;
        return *this;
    }

    bool GetIsLiveEdit() const
    {
        return isLiveEdit_.value_or(false);
    }

    ScriptParsed &SetIsLiveEdit(bool isLiveEdit)
    {
        isLiveEdit_ = isLiveEdit;
        return *this;
    }

    bool HasIsLiveEdit() const
    {
        return isLiveEdit_.has_value();
    }

    Local<ObjectRef> GetExecutionContextAuxData() const
    {
        return execContextAuxData_.value_or(Local<ObjectRef>());
    }

    ScriptParsed &SetExecutionContextAuxData(Local<ObjectRef> execContextAuxData)
    {
        execContextAuxData_ = execContextAuxData;
        return *this;
    }

    bool HasExecutionContextAuxData() const
    {
        return execContextAuxData_.has_value();
    }

    const std::string &GetSourceMapURL() const
    {
        ASSERT(HasSourceMapUrl());
        return sourceMapUrl_.value();
    }

    ScriptParsed &SetSourceMapURL(const std::string &sourceMapUrl)
    {
        sourceMapUrl_ = sourceMapUrl;
        return *this;
    }

    bool HasSourceMapUrl() const
    {
        return sourceMapUrl_.has_value();
    }

    bool GetHasSourceURL() const
    {
        return hasSourceUrl_.value_or(false);
    }

    ScriptParsed &SetHasSourceURL(bool hasSourceUrl)
    {
        hasSourceUrl_ = hasSourceUrl;
        return *this;
    }

    bool HasHasSourceURL() const
    {
        return hasSourceUrl_.has_value();
    }

    bool GetIsModule() const
    {
        return isModule_.value_or(false);
    }

    ScriptParsed &SetIsModule(bool isModule)
    {
        isModule_ = isModule;
        return *this;
    }

    bool HasIsModule() const
    {
        return isModule_.has_value();
    }

    int32_t GetLength() const
    {
        return length_.value_or(0);
    }

    ScriptParsed &SetLength(int32_t length)
    {
        length_ = length;
        return *this;
    }

    bool HasLength() const
    {
        return length_.has_value();
    }

    int32_t GetCodeOffset() const
    {
        return codeOffset_.value_or(0);
    }

    ScriptParsed &SetCodeOffset(int32_t codeOffset)
    {
        codeOffset_ = codeOffset;
        return *this;
    }

    bool HasCodeOffset() const
    {
        return codeOffset_.has_value();
    }

    const std::string &GetScriptLanguage() const
    {
        ASSERT(HasScriptLanguage());
        return scriptLanguage_.value();
    }

    ScriptParsed &SetScriptLanguage(const std::string &scriptLanguage)
    {
        scriptLanguage_ = scriptLanguage;
        return *this;
    }

    bool HasScriptLanguage() const
    {
        return scriptLanguage_.has_value();
    }

    const std::string &GetEmbedderName() const
    {
        ASSERT(HasEmbedderName());
        return embedderName_.value();
    }

    ScriptParsed &SetEmbedderName(const std::string &embedderName)
    {
        embedderName_ = embedderName;
        return *this;
    }

    bool HasEmbedderName() const
    {
        return embedderName_.has_value();
    }

    ScriptParsed &SetLocations(std::vector<std::shared_ptr<BreakpointReturnInfo>> locations)
    {
        locations_ = locations;
        return *this;
    }

private:
    NO_COPY_SEMANTIC(ScriptParsed);
    NO_MOVE_SEMANTIC(ScriptParsed);

    ScriptId scriptId_ {0};
    std::string url_ {};
    int32_t startLine_ {0};
    int32_t startColumn_ {0};
    int32_t endLine_ {0};
    int32_t endColumn_ {0};
    ExecutionContextId executionContextId_ {0};
    std::string hash_ {};
    std::optional<Local<ObjectRef>> execContextAuxData_ {};
    std::optional<bool> isLiveEdit_ {};
    std::optional<std::string> sourceMapUrl_ {};
    std::optional<bool> hasSourceUrl_ {};
    std::optional<bool> isModule_ {};
    std::optional<int32_t> length_ {};
    std::optional<int32_t> codeOffset_ {};
    std::optional<std::string> scriptLanguage_ {};
    std::optional<std::string> embedderName_ {};
    std::vector<std::shared_ptr<BreakpointReturnInfo>> locations_ {};
};

class AddHeapSnapshotChunk final : public PtBaseEvents {
public:
    AddHeapSnapshotChunk() = default;
    ~AddHeapSnapshotChunk() override = default;
    std::unique_ptr<PtJson> ToJson() const override;

    std::string GetName() const override
    {
        return "HeapProfiler.addHeapSnapshotChunk";
    }

    AddHeapSnapshotChunk &SetChunk(const std::string &chunk)
    {
        chunk_ = chunk;
        return *this;
    }
    
    std::string &GetChunk()
    {
        return chunk_;
    }

private:
    NO_COPY_SEMANTIC(AddHeapSnapshotChunk);
    NO_MOVE_SEMANTIC(AddHeapSnapshotChunk);

    std::string chunk_ {};
};

class ConsoleProfileFinished final : public PtBaseEvents {
public:
    ConsoleProfileFinished() = default;
    ~ConsoleProfileFinished() override = default;
    std::unique_ptr<PtJson> ToJson() const override;
    std::string GetName() const override
    {
        return "Profile.ConsoleProfileFinished";
    }

    const std::string &GetId() const
    {
        return id_;
    }

    ConsoleProfileFinished &SetId(const std::string &id)
    {
        id_ = id;
        return *this;
    }

    Location *GetLocation() const
    {
        return location_.get();
    }

    ConsoleProfileFinished &SetLocation(std::unique_ptr<Location> location)
    {
        location_ = std::move(location);
        return *this;
    }

    Profile *GetProfile() const
    {
        return profile_.get();
    }

    ConsoleProfileFinished &SetProfile(std::unique_ptr<Profile> profile)
    {
        profile_ = std::move(profile);
        return *this;
    }

    const std::string &GetTitle() const
    {
        ASSERT(HasTitle());
        return title_.value();
    }

    ConsoleProfileFinished &SetTitle(const std::string &title)
    {
        title_ = title;
        return *this;
    }

    bool HasTitle() const
    {
        return title_.has_value();
    }

private:
    NO_COPY_SEMANTIC(ConsoleProfileFinished);
    NO_MOVE_SEMANTIC(ConsoleProfileFinished);

    std::string id_ {};
    std::unique_ptr<Location> location_ {nullptr};
    std::unique_ptr<Profile> profile_ {nullptr};
    std::optional<std::string> title_ {};
};

class ConsoleProfileStarted final : public PtBaseEvents {
public:
    ConsoleProfileStarted() = default;
    ~ConsoleProfileStarted() override = default;
    std::unique_ptr<PtJson> ToJson() const override;
    std::string GetName() const override
    {
        return "Profile.ConsoleProfileStarted";
    }

    const std::string &GetId() const
    {
        return id_;
    }

    ConsoleProfileStarted &SetId(const std::string &id)
    {
        id_ = id;
        return *this;
    }

    Location *GetLocation() const
    {
        return location_.get();
    }

    ConsoleProfileStarted &SetLocation(std::unique_ptr<Location> location)
    {
        location_ = std::move(location);
        return *this;
    }

    const std::string &GetTitle() const
    {
        ASSERT(HasTitle());
        return title_.value();
    }

    ConsoleProfileStarted &SetTitle(const std::string &title)
    {
        title_ = title;
        return *this;
    }

    bool HasTitle() const
    {
        return title_.has_value();
    }

private:
    NO_COPY_SEMANTIC(ConsoleProfileStarted);
    NO_MOVE_SEMANTIC(ConsoleProfileStarted);

    std::string id_ {};
    std::unique_ptr<Location> location_ {nullptr};
    std::optional<std::string> title_ {};
};

class PreciseCoverageDeltaUpdate final : public PtBaseEvents {
public:
    PreciseCoverageDeltaUpdate() = default;
    ~PreciseCoverageDeltaUpdate() override = default;
    std::unique_ptr<PtJson> ToJson() const override;
    std::string GetName() const override
    {
        return "Profile.PreciseCoverageDeltaUpdate";
    }

    int64_t GetTimestamp() const
    {
        return timestamp_;
    }

    PreciseCoverageDeltaUpdate &SetTimestamp(int64_t timestamp)
    {
        timestamp_ = timestamp;
        return *this;
    }

    const std::string &GetOccasion() const
    {
        return occasion_;
    }

    PreciseCoverageDeltaUpdate &SetOccasion(const std::string &occasion)
    {
        occasion_ = occasion;
        return *this;
    }

    const std::vector<std::unique_ptr<ScriptCoverage>> *GetResult() const
    {
        return &result_;
    }

    PreciseCoverageDeltaUpdate &SetResult(std::vector<std::unique_ptr<ScriptCoverage>> result)
    {
        result_ = std::move(result);
        return *this;
    }

private:
    NO_COPY_SEMANTIC(PreciseCoverageDeltaUpdate);
    NO_MOVE_SEMANTIC(PreciseCoverageDeltaUpdate);

    int64_t timestamp_ {0};
    std::string occasion_ {};
    std::vector<std::unique_ptr<ScriptCoverage>> result_ {};
};

class HeapStatsUpdate final : public PtBaseEvents {
public:
    HeapStatsUpdate() = default;
    ~HeapStatsUpdate() override = default;
    std::unique_ptr<PtJson> ToJson() const override;

    std::string GetName() const override
    {
        return "HeapProfiler.heapStatsUpdate";
    }

    const std::vector<int32_t> *GetStatsUpdate() const
    {
        return &statsUpdate_;
    }

    HeapStatsUpdate &SetStatsUpdate(std::vector<int32_t> statsUpdate)
    {
        statsUpdate_ = std::move(statsUpdate);
        return *this;
    }

private:
    NO_COPY_SEMANTIC(HeapStatsUpdate);
    NO_MOVE_SEMANTIC(HeapStatsUpdate);

    std::vector<int32_t> statsUpdate_ {};
};

class LastSeenObjectId final : public PtBaseEvents {
public:
    LastSeenObjectId() = default;
    ~LastSeenObjectId() override = default;
    std::unique_ptr<PtJson> ToJson() const override;

    std::string GetName() const override
    {
        return "HeapProfiler.lastSeenObjectId";
    }

    int32_t GetLastSeenObjectId() const
    {
        return lastSeenObjectId_;
    }

    LastSeenObjectId &SetLastSeenObjectId(int32_t lastSeenObjectId)
    {
        lastSeenObjectId_ = lastSeenObjectId;
        return *this;
    }

    double GetTimestamp() const
    {
        return timestamp_;
    }

    LastSeenObjectId &SetTimestamp(double timestamp)
    {
        timestamp_ = timestamp;
        return *this;
    }

private:
    NO_COPY_SEMANTIC(LastSeenObjectId);
    NO_MOVE_SEMANTIC(LastSeenObjectId);

    int32_t lastSeenObjectId_ {};
    double timestamp_ {};
};

class ReportHeapSnapshotProgress final : public PtBaseEvents {
public:
    ReportHeapSnapshotProgress() = default;
    ~ReportHeapSnapshotProgress() override = default;
    std::unique_ptr<PtJson> ToJson() const override;

    std::string GetName() const override
    {
        return "HeapProfiler.reportHeapSnapshotProgress";
    }

    int32_t GetDone() const
    {
        return done_;
    }

    ReportHeapSnapshotProgress &SetDone(int32_t done)
    {
        done_ = done;
        return *this;
    }

    int32_t GetTotal() const
    {
        return total_;
    }

    ReportHeapSnapshotProgress &SetTotal(int32_t total)
    {
        total_ = total;
        return *this;
    }

    bool GetFinished() const
    {
        return finished_.value_or(false);
    }

    ReportHeapSnapshotProgress &SetFinished(bool finished)
    {
        finished_ = finished;
        return *this;
    }

private:
    NO_COPY_SEMANTIC(ReportHeapSnapshotProgress);
    NO_MOVE_SEMANTIC(ReportHeapSnapshotProgress);

    int32_t done_ {};
    int32_t total_ {};
    std::optional<bool> finished_ {};
};

class BufferUsage final : public PtBaseEvents {
public:
    BufferUsage() = default;
    ~BufferUsage() override = default;
    std::unique_ptr<PtJson> ToJson() const override;

    std::string GetName() const override
    {
        return "Tracing.bufferUsage";
    }

    double GetPercentFull() const
    {
        return percentFull_.value();
    }

    BufferUsage &SetPercentFull(double percentFull)
    {
        percentFull_ = percentFull;
        return *this;
    }

    bool HasPercentFull() const
    {
        return percentFull_.has_value();
    }

    int32_t GetEventCount() const
    {
        return eventCount_.value();
    }

    BufferUsage &SetEventCount(int32_t eventCount)
    {
        eventCount_ = eventCount;
        return *this;
    }

    bool HasEventCount() const
    {
        return eventCount_.has_value();
    }

    double GetValue() const
    {
        return value_.value();
    }

    BufferUsage &SetValue(double value)
    {
        value_ = value;
        return *this;
    }

    bool HasValue() const
    {
        return value_.has_value();
    }

private:
    NO_COPY_SEMANTIC(BufferUsage);
    NO_MOVE_SEMANTIC(BufferUsage);

    std::optional<double> percentFull_ {0};
    std::optional<int32_t> eventCount_ {0};
    std::optional<double> value_ {0};
};

class DataCollected final : public PtBaseEvents {
public:
    DataCollected() = default;
    ~DataCollected() override = default;
    std::unique_ptr<PtJson> ToJson() const override;
    std::unique_ptr<PtJson> TraceEventToJson(TraceEvent &traceEvent) const;

    std::string GetName() const override
    {
        return "Tracing.dataCollected";
    }

    DataCollected &SetTraceEvents(std::unique_ptr<std::vector<TraceEvent>> traceEvents)
    {
        traceEvents_ = std::move(traceEvents);
        return *this;
    }

private:
    NO_COPY_SEMANTIC(DataCollected);
    NO_MOVE_SEMANTIC(DataCollected);

    std::unique_ptr<std::vector<TraceEvent>> traceEvents_;
};

class TracingComplete final : public PtBaseEvents {
public:
    TracingComplete() = default;
    ~TracingComplete() override = default;
    std::unique_ptr<PtJson> ToJson() const override;

    std::string GetName() const override
    {
        return "Tracing.tracingComplete";
    }

    bool GetDataLossOccurred() const
    {
        return dataLossOccurred_;
    }

    TracingComplete &SetDataLossOccurred(bool dataLossOccurred)
    {
        dataLossOccurred_ = dataLossOccurred;
        return *this;
    }

    StreamFormat *GetTraceFormat() const
    {
        if (traceFormat_) {
            return traceFormat_->get();
        }
        return nullptr;
    }

    TracingComplete &SetTraceFormat(std::unique_ptr<StreamFormat> traceFormat)
    {
        traceFormat_ = std::move(traceFormat);
        return *this;
    }

    bool HasTraceFormat() const
    {
        return traceFormat_.has_value();
    }

    StreamCompression *GetStreamCompression() const
    {
        if (streamCompression_) {
            return streamCompression_->get();
        }
        return nullptr;
    }

    TracingComplete &SetStreamCompression(std::unique_ptr<StreamCompression> streamCompression)
    {
        streamCompression_ = std::move(streamCompression);
        return *this;
    }

    bool HasStreamCompression() const
    {
        return streamCompression_.has_value();
    }

private:
    NO_COPY_SEMANTIC(TracingComplete);
    NO_MOVE_SEMANTIC(TracingComplete);

    bool dataLossOccurred_ {};
    /*
     * { TracingComplete.stream }   IO is currently not supported;
     */
    std::optional<std::unique_ptr<StreamFormat>> traceFormat_ {};
    std::optional<std::unique_ptr<StreamCompression>> streamCompression_ {};
};
}  // namespace panda::ecmascript::tooling
#endif
