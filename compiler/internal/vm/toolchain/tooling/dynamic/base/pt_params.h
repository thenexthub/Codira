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

#ifndef ECMASCRIPT_TOOLING_BASE_PT_PARAMS_H
#define ECMASCRIPT_TOOLING_BASE_PT_PARAMS_H

#include "tooling/dynamic/base/pt_types.h"

namespace panda::ecmascript::tooling {
class PtBaseParams : public PtBaseTypes {
public:
    PtBaseParams() = default;
    ~PtBaseParams() override = default;
    std::unique_ptr<PtJson> ToJson() const override
    {
        UNREACHABLE();
    }

private:
    NO_COPY_SEMANTIC(PtBaseParams);
    NO_MOVE_SEMANTIC(PtBaseParams);
};

class ContinueToLocationParams : public PtBaseParams {
public:
    ContinueToLocationParams() = default;
    ~ContinueToLocationParams() override = default;

    static std::unique_ptr<ContinueToLocationParams> Create(const PtJson &params);
    Location *GetLocation() const
    {
        return location_.get();
    }
    
    const std::string &GetTargetCallFrames() const
    {
        return targetCallFrames_;
    }

private:
    NO_COPY_SEMANTIC(ContinueToLocationParams);
    NO_MOVE_SEMANTIC(ContinueToLocationParams);

    std::unique_ptr<Location> location_ {nullptr};
    std::string targetCallFrames_ {};
};

class EnableParams : public PtBaseParams {
public:
    EnableParams() = default;
    ~EnableParams() override = default;

    static std::unique_ptr<EnableParams> Create(const PtJson &params);

    double GetMaxScriptsCacheSize() const
    {
        return maxScriptsCacheSize_.value_or(0);
    }

    bool HasMaxScriptsCacheSize() const
    {
        return maxScriptsCacheSize_.has_value();
    }

    bool HasEnableOptionsList() const
    {
        return enableOptionList_.has_value();
    }

    std::vector<std::string> GetEnableOptionsList() const
    {
        ASSERT(HasEnableOptionsList());
        return enableOptionList_.value();
    }

private:
    NO_COPY_SEMANTIC(EnableParams);
    NO_MOVE_SEMANTIC(EnableParams);

    std::optional<double> maxScriptsCacheSize_ {};
    std::optional<std::vector<std::string>> enableOptionList_ {};
};

class EvaluateOnCallFrameParams : public PtBaseParams {
public:
    EvaluateOnCallFrameParams() = default;
    ~EvaluateOnCallFrameParams() override = default;

    static std::unique_ptr<EvaluateOnCallFrameParams> Create(const PtJson &params);

    CallFrameId GetCallFrameId() const
    {
        return callFrameId_;
    }

    const std::string &GetExpression() const
    {
        return expression_;
    }

private:
    NO_COPY_SEMANTIC(EvaluateOnCallFrameParams);
    NO_MOVE_SEMANTIC(EvaluateOnCallFrameParams);

    CallFrameId callFrameId_ {};
    std::string expression_ {};
    std::optional<std::string> objectGroup_ {};
    std::optional<bool> includeCommandLineAPI_ {};
    std::optional<bool> silent_ {};
    std::optional<bool> returnByValue_ {};
    std::optional<bool> generatePreview_ {};
    std::optional<bool> throwOnSideEffect_ {};
};

class GetPossibleBreakpointsParams : public PtBaseParams {
public:
    GetPossibleBreakpointsParams() = default;
    ~GetPossibleBreakpointsParams() override = default;

    static std::unique_ptr<GetPossibleBreakpointsParams> Create(const PtJson &params);

    Location *GetStart() const
    {
        return start_.get();
    }

    Location *GetEnd() const
    {
        if (end_) {
            return end_->get();
        }
        return nullptr;
    }

    bool HasEnd() const
    {
        return end_.has_value();
    }

    bool GetRestrictToFunction() const
    {
        return restrictToFunction_.value_or(false);
    }

    bool HasRestrictToFunction() const
    {
        return restrictToFunction_.has_value();
    }

private:
    NO_COPY_SEMANTIC(GetPossibleBreakpointsParams);
    NO_MOVE_SEMANTIC(GetPossibleBreakpointsParams);

    std::unique_ptr<Location> start_ {nullptr};
    std::optional<std::unique_ptr<Location>> end_ {};
    std::optional<bool> restrictToFunction_ {};
};

class GetScriptSourceParams : public PtBaseParams {
public:
    GetScriptSourceParams() = default;
    ~GetScriptSourceParams() override = default;

    static std::unique_ptr<GetScriptSourceParams> Create(const PtJson &params);

    ScriptId GetScriptId() const
    {
        return scriptId_;
    }

private:
    NO_COPY_SEMANTIC(GetScriptSourceParams);
    NO_MOVE_SEMANTIC(GetScriptSourceParams);

    ScriptId scriptId_ {0};
};

class RemoveBreakpointParams : public PtBaseParams {
public:
    RemoveBreakpointParams() = default;
    ~RemoveBreakpointParams() override = default;

    static std::unique_ptr<RemoveBreakpointParams> Create(const PtJson &params);

    BreakpointId GetBreakpointId() const
    {
        return breakpointId_;
    }

private:
    NO_COPY_SEMANTIC(RemoveBreakpointParams);
    NO_MOVE_SEMANTIC(RemoveBreakpointParams);

    BreakpointId breakpointId_ {};
};

class RemoveBreakpointsByUrlParams : public PtBaseParams {
public:
    RemoveBreakpointsByUrlParams() = default;
    ~RemoveBreakpointsByUrlParams() override = default;

    static std::unique_ptr<RemoveBreakpointsByUrlParams> Create(const PtJson &params);

    const std::string &GetUrl() const
    {
        ASSERT(HasUrl());
        return url_.value();
    }

    bool HasUrl() const
    {
        return url_.has_value();
    }
private:
    NO_COPY_SEMANTIC(RemoveBreakpointsByUrlParams);
    NO_MOVE_SEMANTIC(RemoveBreakpointsByUrlParams);

    std::optional<std::string> url_ {};
};

class ResumeParams : public PtBaseParams {
public:
    ResumeParams() = default;
    ~ResumeParams() override = default;

    static std::unique_ptr<ResumeParams> Create(const PtJson &params);

    bool GetTerminateOnResume() const
    {
        return terminateOnResume_.value_or(false);
    }

    bool HasTerminateOnResume() const
    {
        return terminateOnResume_.has_value();
    }

private:
    NO_COPY_SEMANTIC(ResumeParams);
    NO_MOVE_SEMANTIC(ResumeParams);

    std::optional<bool> terminateOnResume_ {};
};

class SetAsyncCallStackDepthParams : public PtBaseParams {
public:
    SetAsyncCallStackDepthParams() = default;
    ~SetAsyncCallStackDepthParams() override = default;

    static std::unique_ptr<SetAsyncCallStackDepthParams> Create(const PtJson &params);

    int32_t GetMaxDepth() const
    {
        return maxDepth_;
    }

private:
    NO_COPY_SEMANTIC(SetAsyncCallStackDepthParams);
    NO_MOVE_SEMANTIC(SetAsyncCallStackDepthParams);

    int32_t maxDepth_ {0};
};

class SetBlackboxPatternsParams : public PtBaseParams {
public:
    SetBlackboxPatternsParams() = default;
    ~SetBlackboxPatternsParams() override = default;
    static std::unique_ptr<SetBlackboxPatternsParams> Create(const PtJson &params);

    std::list<std::string> GetPatterns() const
    {
        return patterns_;
    }

private:
    NO_COPY_SEMANTIC(SetBlackboxPatternsParams);
    NO_MOVE_SEMANTIC(SetBlackboxPatternsParams);

    std::list<std::string> patterns_ {};
};

class SetBreakpointByUrlParams : public PtBaseParams {
public:
    SetBreakpointByUrlParams() = default;
    ~SetBreakpointByUrlParams() override = default;

    static std::unique_ptr<SetBreakpointByUrlParams> Create(const PtJson &params);

    int32_t GetLine() const
    {
        return lineNumber_;
    }

    const std::string &GetUrl() const
    {
        ASSERT(HasUrl());
        return url_.value();
    }

    bool HasUrl() const
    {
        return url_.has_value();
    }

    const std::string &GetUrlRegex() const
    {
        ASSERT(HasUrlRegex());
        return urlRegex_.value();
    }

    bool HasUrlRegex() const
    {
        return urlRegex_.has_value();
    }

    const std::string &GetScriptHash() const
    {
        ASSERT(HasScriptHash());
        return scriptHash_.value();
    }

    bool HasScriptHash() const
    {
        return scriptHash_.has_value();
    }

    int32_t GetColumn() const
    {
        return columnNumber_.value_or(0);
    }

    bool HasColumn() const
    {
        return columnNumber_.has_value();
    }

    const std::string &GetCondition() const
    {
        ASSERT(HasCondition());
        return condition_.value();
    }

    bool HasCondition() const
    {
        return condition_.has_value();
    }

private:
    NO_COPY_SEMANTIC(SetBreakpointByUrlParams);
    NO_MOVE_SEMANTIC(SetBreakpointByUrlParams);

    int32_t lineNumber_ {0};
    std::optional<std::string> url_ {};
    std::optional<std::string> urlRegex_ {};
    std::optional<std::string> scriptHash_ {};
    std::optional<int32_t> columnNumber_ {0};
    std::optional<std::string> condition_ {};
};


class SetBreakpointsActiveParams : public PtBaseParams {
public:
    SetBreakpointsActiveParams() = default;
    ~SetBreakpointsActiveParams() override = default;

    static std::unique_ptr<SetBreakpointsActiveParams> Create(const PtJson &params);

    bool GetBreakpointsState() const
    {
        return breakpointsState_.value_or(false);
    }
private:
    NO_COPY_SEMANTIC(SetBreakpointsActiveParams);
    NO_MOVE_SEMANTIC(SetBreakpointsActiveParams);

    std::optional<bool> breakpointsState_ {};
};

class SetSkipAllPausesParams : public PtBaseParams {
public:
    SetSkipAllPausesParams() = default;
    ~SetSkipAllPausesParams() override = default;

    static std::unique_ptr<SetSkipAllPausesParams> Create(const PtJson &params);

    bool GetSkipAllPausesState() const
    {
        return skipAllPausesState_.value_or(false);
    }
private:
    NO_COPY_SEMANTIC(SetSkipAllPausesParams);
    NO_MOVE_SEMANTIC(SetSkipAllPausesParams);

    std::optional<bool> skipAllPausesState_ {};
};

class GetPossibleAndSetBreakpointParams : public PtBaseParams {
public:
    GetPossibleAndSetBreakpointParams() = default;
    ~GetPossibleAndSetBreakpointParams() = default;
    static std::unique_ptr<GetPossibleAndSetBreakpointParams> Create(const PtJson &params);

    const std::vector<std::unique_ptr<BreakpointInfo>> *GetBreakpointsList() const
    {
        if (!breakpointsList_) {
            return nullptr;
        }
        return &(breakpointsList_.value());
    }

    bool HasBreakpointsList() const
    {
        return breakpointsList_.has_value();
    }

private:
    NO_COPY_SEMANTIC(GetPossibleAndSetBreakpointParams);
    NO_MOVE_SEMANTIC(GetPossibleAndSetBreakpointParams);

    std::optional<std::vector<std::unique_ptr<BreakpointInfo>>> breakpointsList_ {};
};

enum class PauseOnExceptionsState : uint8_t { NONE, UNCAUGHT, ALL };

class SetPauseOnExceptionsParams : public PtBaseParams {
public:
    SetPauseOnExceptionsParams() = default;
    ~SetPauseOnExceptionsParams() override = default;
    static std::unique_ptr<SetPauseOnExceptionsParams> Create(const PtJson &params);

    PauseOnExceptionsState GetState() const
    {
        return state_;
    }

    bool StoreState(const std::string &state)
    {
        if (state == "none") {
            state_ = PauseOnExceptionsState::NONE;
            return true;
        }
        if (state == "uncaught") {
            state_ = PauseOnExceptionsState::UNCAUGHT;
            return true;
        }
        if (state == "all") {
            state_ = PauseOnExceptionsState::ALL;
            return true;
        }
        return false;
    }

private:
    NO_COPY_SEMANTIC(SetPauseOnExceptionsParams);
    NO_MOVE_SEMANTIC(SetPauseOnExceptionsParams);

    PauseOnExceptionsState state_ {PauseOnExceptionsState::ALL};
};

class StepIntoParams : public PtBaseParams {
public:
    StepIntoParams() = default;
    ~StepIntoParams() override = default;

    static std::unique_ptr<StepIntoParams> Create(const PtJson &params);

    bool GetBreakOnAsyncCall() const
    {
        return breakOnAsyncCall_.value_or(false);
    }

    bool HasBreakOnAsyncCall() const
    {
        return breakOnAsyncCall_.has_value();
    }

    const std::list<std::unique_ptr<LocationRange>> *GetSkipList() const
    {
        if (!skipList_) {
            return nullptr;
        }
        return &(skipList_.value());
    }

    bool HasSkipList() const
    {
        return skipList_.has_value();
    }

private:
    NO_COPY_SEMANTIC(StepIntoParams);
    NO_MOVE_SEMANTIC(StepIntoParams);

    std::optional<bool> breakOnAsyncCall_ {};
    std::optional<std::list<std::unique_ptr<LocationRange>>> skipList_ {};
};

class SmartStepIntoParams : public PtBaseParams {
public:
    SmartStepIntoParams() = default;
    ~SmartStepIntoParams() override = default;

    static std::unique_ptr<SmartStepIntoParams> Create(const PtJson &params);

    SetBreakpointByUrlParams *GetSetBreakpointByUrlParams() const
    {
        return sbpParams_.get();
    }

private:
    NO_COPY_SEMANTIC(SmartStepIntoParams);
    NO_MOVE_SEMANTIC(SmartStepIntoParams);

    static void AddRequireParams(PtJson &params);

    std::unique_ptr<SetBreakpointByUrlParams> sbpParams_ {nullptr};
};

class StepOverParams : public PtBaseParams {
public:
    StepOverParams() = default;
    ~StepOverParams() override = default;

    static std::unique_ptr<StepOverParams> Create(const PtJson &params);

    const std::list<std::unique_ptr<LocationRange>> *GetSkipList() const
    {
        if (!skipList_) {
            return nullptr;
        }
        return &(skipList_.value());
    }

    bool HasSkipList() const
    {
        return skipList_.has_value();
    }

private:
    NO_COPY_SEMANTIC(StepOverParams);
    NO_MOVE_SEMANTIC(StepOverParams);

    std::optional<std::list<std::unique_ptr<LocationRange>>> skipList_ {};
};

class DropFrameParams : public PtBaseParams {
public:
    DropFrameParams() = default;
    ~DropFrameParams() override = default;
    static std::unique_ptr<DropFrameParams> Create(const PtJson &params);

    uint32_t GetDroppedDepth() const
    {
        return droppedDepth_.value_or(0);
    }

    bool HasDroppedDepth() const
    {
        return droppedDepth_.has_value();
    }

private:
    NO_COPY_SEMANTIC(DropFrameParams);
    NO_MOVE_SEMANTIC(DropFrameParams);

    std::optional<uint32_t> droppedDepth_ {};
};

class SetNativeRangeParams {
public:
    SetNativeRangeParams() = default;
    ~SetNativeRangeParams() = default;
    static std::unique_ptr<SetNativeRangeParams> Create(const PtJson &params);

    std::vector<NativeRange> GetNativeRange() const
    {
        return nativeRange_;
    }
private:

    std::vector<NativeRange> nativeRange_ {};
};

class ResetSingleStepperParams : public PtBaseParams {
public:
    ResetSingleStepperParams() = default;
    ~ResetSingleStepperParams() = default;
    static std::unique_ptr<ResetSingleStepperParams> Create(const PtJson &params);

    bool GetResetSingleStepper() const
    {
        return resetSingleStepper_;
    }
private:
    NO_COPY_SEMANTIC(ResetSingleStepperParams);
    NO_MOVE_SEMANTIC(ResetSingleStepperParams);

    bool resetSingleStepper_ {false};
};

class SetMixedDebugParams : public PtBaseParams {
public:
    SetMixedDebugParams() = default;
    ~SetMixedDebugParams() override = default;
    static std::unique_ptr<SetMixedDebugParams> Create(const PtJson &params);

    bool GetEnabled() const
    {
        return enabled_;
    }

    bool GetMixedStackEnabled() const
    {
        return mixedStackEnabled_;
    }

private:
    NO_COPY_SEMANTIC(SetMixedDebugParams);
    NO_MOVE_SEMANTIC(SetMixedDebugParams);

    bool enabled_ {false};
    bool mixedStackEnabled_ {false};
};

class ReplyNativeCallingParams : public PtBaseParams {
public:
    ReplyNativeCallingParams() = default;
    ~ReplyNativeCallingParams() override = default;
    static std::unique_ptr<ReplyNativeCallingParams> Create(const PtJson &params);

    bool GetUserCode() const
    {
        return userCode_;
    }

private:
    NO_COPY_SEMANTIC(ReplyNativeCallingParams);
    NO_MOVE_SEMANTIC(ReplyNativeCallingParams);

    bool userCode_ {false};
};

class GetPropertiesParams : public PtBaseParams {
public:
    GetPropertiesParams() = default;
    ~GetPropertiesParams() override = default;

    static std::unique_ptr<GetPropertiesParams> Create(const PtJson &params);

    RemoteObjectId GetObjectId() const
    {
        return objectId_;
    }

    GetPropertiesParams &SetObjectId(RemoteObjectId id)
    {
        objectId_ = id;
        return *this;
    }

    bool GetOwnProperties() const
    {
        return ownProperties_.value_or(false);
    }

    GetPropertiesParams &SetOwnProperties(bool ownProperties)
    {
        ownProperties_ = ownProperties;
        return *this;
    }

    bool HasOwnProperties() const
    {
        return ownProperties_.has_value();
    }

    bool GetAccessPropertiesOnly() const
    {
        return accessorPropertiesOnly_.value_or(false);
    }

    GetPropertiesParams &SetAccessPropertiesOnly(bool accessorPropertiesOnly)
    {
        accessorPropertiesOnly_ = accessorPropertiesOnly;
        return *this;
    }

    bool HasAccessPropertiesOnly() const
    {
        return accessorPropertiesOnly_.has_value();
    }

    bool GetGeneratePreview() const
    {
        return generatePreview_.value_or(false);
    }

    bool HasGeneratePreview() const
    {
        return generatePreview_.has_value();
    }

    bool HasStartIndex() const
    {
        return startIndex_.has_value();
    }
 
    int32_t GetStartIndex() const
    {
        return startIndex_.value_or(INVALID_START_OR_COUNT);
    }
 
    bool HasGroupCount() const
    {
        return groupCount_.has_value();
    }
 
    int32_t GetGroupCount() const
    {
        return groupCount_.value_or(INVALID_START_OR_COUNT);
    }
 
    bool IsValidRequestUsingRange() const
    {
        return HasStartIndex() && HasGroupCount() &&
            GetStartIndex() >= 0 && GetGroupCount() > 0;
    }

private:
    NO_COPY_SEMANTIC(GetPropertiesParams);
    NO_MOVE_SEMANTIC(GetPropertiesParams);

    RemoteObjectId objectId_ {};
    std::optional<bool> ownProperties_ {};
    std::optional<bool> accessorPropertiesOnly_ {};
    std::optional<bool> generatePreview_ {};

    // Params for Arrays and containers
    // retrieving properties with certain range
    std::optional<int32_t> startIndex_ {};
    std::optional<int32_t> groupCount_ {};
    static constexpr int32_t INVALID_START_OR_COUNT = -1;
};

class CallFunctionOnParams : public PtBaseParams {
public:
    CallFunctionOnParams() = default;
    ~CallFunctionOnParams() override = default;

    static std::unique_ptr<CallFunctionOnParams> Create(const PtJson &params);

    CallFrameId GetCallFrameId() const
    {
        return callFrameId_;
    }

    const std::string &GetFunctionDeclaration() const
    {
        return functionDeclaration_;
    }

    RemoteObjectId GetObjectId() const
    {
        return objectId_.value_or(-1);
    }

    CallFunctionOnParams &SetObjectId(RemoteObjectId objectId)
    {
        objectId_ = objectId;
        return *this;
    }

    bool HasObjectId() const
    {
        return objectId_.has_value();
    }

    const std::vector<std::unique_ptr<CallArgument>> *GetArguments() const
    {
        if (!arguments_) {
            return nullptr;
        }
        return &(arguments_.value());
    }

    bool HasArguments() const
    {
        return arguments_.has_value();
    }

    bool GetSilent() const
    {
        return silent_.value_or(false);
    }

    bool HasSilent() const
    {
        return silent_.has_value();
    }

    bool GetReturnByValue() const
    {
        return returnByValue_.value_or(false);
    }

    bool HasReturnByValue() const
    {
        return returnByValue_.has_value();
    }

    bool GetGeneratePreview() const
    {
        return generatePreview_.value_or(false);
    }

    bool HasGeneratePreview() const
    {
        return generatePreview_.has_value();
    }

    bool GetUserGesture() const
    {
        return userGesture_.value_or(false);
    }

    bool HasUserGesture() const
    {
        return userGesture_.has_value();
    }

    bool GetAwaitPromise() const
    {
        return awaitPromise_.value_or(false);
    }

    bool HasAwaitPromise() const
    {
        return awaitPromise_.has_value();
    }

    ExecutionContextId GetExecutionContextId() const
    {
        return executionContextId_.value_or(-1);
    }

    CallFunctionOnParams &SetExecutionContextId(ExecutionContextId executionContextId)
    {
        executionContextId_ = executionContextId;
        return *this;
    }

    bool HasExecutionContextId() const
    {
        return executionContextId_.has_value();
    }

    const std::string &GetObjectGroup() const
    {
        ASSERT(HasObjectGroup());
        return objectGroup_.value();
    }

    bool HasObjectGroup() const
    {
        return objectGroup_.has_value();
    }

    bool GetThrowOnSideEffect() const
    {
        return throwOnSideEffect_.value_or(false);
    }

    bool HasThrowOnSideEffect() const
    {
        return throwOnSideEffect_.has_value();
    }

private:
    NO_COPY_SEMANTIC(CallFunctionOnParams);
    NO_MOVE_SEMANTIC(CallFunctionOnParams);

    CallFrameId callFrameId_ {};
    std::string functionDeclaration_ {};
    std::optional<RemoteObjectId> objectId_ {};
    std::optional<std::vector<std::unique_ptr<CallArgument>>> arguments_ {};
    std::optional<bool> silent_ {};
    std::optional<bool> returnByValue_ {};
    std::optional<bool> generatePreview_ {};
    std::optional<bool> userGesture_ {};
    std::optional<bool> awaitPromise_ {};
    std::optional<ExecutionContextId> executionContextId_ {};
    std::optional<std::string> objectGroup_ {};
    std::optional<bool> throwOnSideEffect_ {};
};

class StartSamplingParams : public PtBaseParams {
public:
    StartSamplingParams() = default;
    ~StartSamplingParams() override = default;

    static std::unique_ptr<StartSamplingParams> Create(const PtJson &params);

    double GetSamplingInterval() const
    {
        return samplingInterval_.value_or(32768); // 32768: default interval
    }

private:
    NO_COPY_SEMANTIC(StartSamplingParams);
    NO_MOVE_SEMANTIC(StartSamplingParams);

    std::optional<double> samplingInterval_ {32768};
};

class StartTrackingHeapObjectsParams : public PtBaseParams {
public:
    StartTrackingHeapObjectsParams() = default;
    ~StartTrackingHeapObjectsParams() override = default;

    static std::unique_ptr<StartTrackingHeapObjectsParams> Create(const PtJson &params);

    bool GetTrackAllocations() const
    {
        return trackAllocations_.value_or(false);
    }

    bool HasTrackAllocations() const
    {
        return trackAllocations_.has_value();
    }

private:
    NO_COPY_SEMANTIC(StartTrackingHeapObjectsParams);
    NO_MOVE_SEMANTIC(StartTrackingHeapObjectsParams);

    std::optional<bool> trackAllocations_;
};

class StopTrackingHeapObjectsParams : public PtBaseParams {
public:
    StopTrackingHeapObjectsParams() = default;
    ~StopTrackingHeapObjectsParams() override = default;

    static std::unique_ptr<StopTrackingHeapObjectsParams> Create(const PtJson &params);

    bool GetReportProgress() const
    {
        return reportProgress_.value_or(false);
    }

    bool HasReportProgress() const
    {
        return reportProgress_.has_value();
    }

    bool GetTreatGlobalObjectsAsRoots() const
    {
        return treatGlobalObjectsAsRoots_.value_or(false);
    }

    bool HasTreatGlobalObjectsAsRoots() const
    {
        return treatGlobalObjectsAsRoots_.has_value();
    }

    bool GetCaptureNumericValue() const
    {
        return captureNumericValue_.value_or(false);
    }

    bool HasCaptureNumericValue() const
    {
        return captureNumericValue_.has_value();
    }

private:
    NO_COPY_SEMANTIC(StopTrackingHeapObjectsParams);
    NO_MOVE_SEMANTIC(StopTrackingHeapObjectsParams);

    std::optional<bool> reportProgress_ {};
    std::optional<bool> treatGlobalObjectsAsRoots_ {};
    std::optional<bool> captureNumericValue_ {};
};

class AddInspectedHeapObjectParams : public PtBaseParams {
public:
    AddInspectedHeapObjectParams() = default;
    ~AddInspectedHeapObjectParams() override = default;

    static std::unique_ptr<AddInspectedHeapObjectParams> Create(const PtJson &params);

    HeapSnapshotObjectId GetHeapObjectId() const
    {
        return heapObjectId_;
    }

private:
    NO_COPY_SEMANTIC(AddInspectedHeapObjectParams);
    NO_MOVE_SEMANTIC(AddInspectedHeapObjectParams);

    HeapSnapshotObjectId heapObjectId_ {};
};

class GetHeapObjectIdParams : public PtBaseParams {
public:
    GetHeapObjectIdParams() = default;
    ~GetHeapObjectIdParams() override = default;

    static std::unique_ptr<GetHeapObjectIdParams> Create(const PtJson &params);

    RemoteObjectId GetObjectId() const
    {
        return objectId_;
    }

private:
    NO_COPY_SEMANTIC(GetHeapObjectIdParams);
    NO_MOVE_SEMANTIC(GetHeapObjectIdParams);

    RemoteObjectId objectId_ {};
};

class GetObjectByHeapObjectIdParams : public PtBaseParams {
public:
    GetObjectByHeapObjectIdParams() = default;
    ~GetObjectByHeapObjectIdParams() override = default;

    static std::unique_ptr<GetObjectByHeapObjectIdParams> Create(const PtJson &params);

    HeapSnapshotObjectId GetObjectId() const
    {
        return objectId_;
    }

    const std::string &GetObjectGroup() const
    {
        ASSERT(HasObjectGroup());
        return objectGroup_.value();
    }

    bool HasObjectGroup() const
    {
        return objectGroup_.has_value();
    }

private:
    NO_COPY_SEMANTIC(GetObjectByHeapObjectIdParams);
    NO_MOVE_SEMANTIC(GetObjectByHeapObjectIdParams);

    HeapSnapshotObjectId objectId_ {};
    std::optional<std::string> objectGroup_ {};
};

class StartPreciseCoverageParams : public PtBaseParams {
public:
    StartPreciseCoverageParams() = default;
    ~StartPreciseCoverageParams() override = default;

    static std::unique_ptr<StartPreciseCoverageParams> Create(const PtJson &params);

    bool GetCallCount() const
    {
        return callCount_.value_or(false);
    }

    bool HasCallCount() const
    {
        return callCount_.has_value();
    }

    bool GetDetailed() const
    {
        return detailed_.value_or(false);
    }

    bool HasDetailed() const
    {
        return detailed_.has_value();
    }

    bool GetAllowTriggeredUpdates() const
    {
        return allowTriggeredUpdates_.value_or(false);
    }

    bool HasAllowTriggeredUpdates() const
    {
        return allowTriggeredUpdates_.has_value();
    }

private:
    NO_COPY_SEMANTIC(StartPreciseCoverageParams);
    NO_MOVE_SEMANTIC(StartPreciseCoverageParams);

    std::optional<bool> callCount_ {};
    std::optional<bool> detailed_ {};
    std::optional<bool> allowTriggeredUpdates_ {};
};

class SetSamplingIntervalParams : public PtBaseParams {
public:
    SetSamplingIntervalParams() = default;
    ~SetSamplingIntervalParams() override = default;

    static std::unique_ptr<SetSamplingIntervalParams> Create(const PtJson &params);

    int32_t GetInterval() const
    {
        return interval_;
    }

    SetSamplingIntervalParams &SetInterval(int32_t interval)
    {
        interval_ = interval;
        return *this;
    }

private:
    NO_COPY_SEMANTIC(SetSamplingIntervalParams);
    NO_MOVE_SEMANTIC(SetSamplingIntervalParams);

    int32_t interval_ {0};
};

class RecordClockSyncMarkerParams : public PtBaseParams {
public:
    RecordClockSyncMarkerParams() = default;
    ~RecordClockSyncMarkerParams() override = default;

    static std::unique_ptr<RecordClockSyncMarkerParams> Create(const PtJson &params);

    std::string GetSyncId() const
    {
        return syncId_;
    }

    RecordClockSyncMarkerParams &SetSyncId(std::string syncId)
    {
        syncId_ = syncId;
        return *this;
    }

private:
    NO_COPY_SEMANTIC(RecordClockSyncMarkerParams);
    NO_MOVE_SEMANTIC(RecordClockSyncMarkerParams);

    std::string syncId_ {};
};

class RequestMemoryDumpParams : public PtBaseParams {
public:
    RequestMemoryDumpParams() = default;
    ~RequestMemoryDumpParams() override = default;

    static std::unique_ptr<RequestMemoryDumpParams> Create(const PtJson &params);

    bool GetDeterministic() const
    {
        return deterministic_.value_or(false);
    }

    RequestMemoryDumpParams &SetDeterministic(bool deterministic)
    {
        deterministic_ = deterministic;
        return *this;
    }

    bool HasDeterministic() const
    {
        return deterministic_.has_value();
    }

    MemoryDumpLevelOfDetail GetLevelOfDetail() const
    {
        return levelOfDetail_.value_or("");
    }

    RequestMemoryDumpParams &SetLevelOfDetail(const MemoryDumpLevelOfDetail &levelOfDetail)
    {
        levelOfDetail_ = levelOfDetail;
        return *this;
    }

    bool HasLevelOfDetail() const
    {
        return levelOfDetail_.has_value();
    }

private:
    NO_COPY_SEMANTIC(RequestMemoryDumpParams);
    NO_MOVE_SEMANTIC(RequestMemoryDumpParams);

    std::optional<bool> deterministic_ {};
    std::optional<MemoryDumpLevelOfDetail> levelOfDetail_ {};
};

class StartParams : public PtBaseParams {
public:
    StartParams() = default;
    ~StartParams() override = default;

    static std::unique_ptr<StartParams> Create(const PtJson &params);

    std::string GetCategories() const
    {
        return categories_.value_or("");
    }

    StartParams &SetCategories(std::string categories)
    {
        categories_ = categories;
        return *this;
    }

    bool HasCategories() const
    {
        return categories_.has_value();
    }

    std::string GetOptions() const
    {
        return options_.value_or("");
    }

    StartParams &SetOptions(std::string options)
    {
        options_ = options;
        return *this;
    }

    bool HasOptions() const
    {
        return options_.has_value();
    }

    int32_t GetBufferUsageReportingInterval() const
    {
        return bufferUsageReportingInterval_.value_or(0);
    }

    StartParams &SetBufferUsageReportingInterval(int32_t bufferUsageReportingInterval)
    {
        bufferUsageReportingInterval_ = bufferUsageReportingInterval;
        return *this;
    }

    bool HasBufferUsageReportingInterval() const
    {
        return bufferUsageReportingInterval_.has_value();
    }

    std::string GetTransferMode() const
    {
        return transferMode_.value_or("");
    }

    StartParams &SetTransferMode(std::string transferMode)
    {
        transferMode_ = transferMode;
        return *this;
    }

    bool HasTransferMode() const
    {
        return transferMode_.has_value();
    }

    struct TransferModeValues {
        static bool Valid(const std::string &values)
        {
            return values == ReportEvents() || values == ReturnAsStream();
        }
        static std::string ReportEvents()
        {
            return "ReportEvents";
        }
        static std::string ReturnAsStream()
        {
            return "ReturnAsStream";
        }
    };

    StreamFormat GetStreamFormat() const
    {
        return streamFormat_.value_or("");
    }

    StartParams &SetStreamFormat(const StreamFormat &streamFormat)
    {
        streamFormat_ = streamFormat;
        return *this;
    }

    bool HasStreamFormat() const
    {
        return streamFormat_.has_value();
    }

    StreamCompression GetStreamCompression() const
    {
        return streamCompression_.value_or("");
    }

    StartParams &SetStreamCompression(const StreamCompression &streamCompression)
    {
        streamCompression_ = streamCompression;
        return *this;
    }

    bool HasStreamCompression() const
    {
        return streamCompression_.has_value();
    }

    TraceConfig *GetTraceConfig() const
    {
        if (traceConfig_) {
            return traceConfig_->get();
        }
        return nullptr;
    }

    StartParams &SetTraceConfig(std::unique_ptr<TraceConfig> &traceConfig)
    {
        traceConfig_ = std::move(traceConfig);
        return *this;
    }

    bool HasTraceConfig() const
    {
        return traceConfig_.has_value();
    }

    std::string GetPerfettoConfig() const
    {
        return perfettoConfig_.value_or("");
    }

    StartParams &SetPerfettoConfig(std::string perfettoConfig)
    {
        perfettoConfig_ = perfettoConfig;
        return *this;
    }

    bool HasPerfettoConfig() const
    {
        return perfettoConfig_.has_value();
    }

    TracingBackend GetTracingBackend() const
    {
        return tracingBackend_.value_or("");
    }

    StartParams &SetTracingBackend(const TracingBackend &tracingBackend)
    {
        tracingBackend_ = tracingBackend;
        return *this;
    }

    bool HasTracingBackend() const
    {
        return tracingBackend_.has_value();
    }

private:
    NO_COPY_SEMANTIC(StartParams);
    NO_MOVE_SEMANTIC(StartParams);

    std::optional<std::string> categories_ {};
    std::optional<std::string> options_ {};
    std::optional<int32_t> bufferUsageReportingInterval_ {};
    std::optional<std::string> transferMode_ {};
    std::optional<StreamFormat> streamFormat_ {};
    std::optional<StreamCompression> streamCompression_ {};
    std::optional<std::unique_ptr<TraceConfig>> traceConfig_ {};
    std::optional<std::string> perfettoConfig_ {};
    std::optional<TracingBackend> tracingBackend_ {};
};

class SeriliazationTimeoutCheckEnableParams : public PtBaseParams {
public:
    SeriliazationTimeoutCheckEnableParams() = default;
    ~SeriliazationTimeoutCheckEnableParams() override = default;

    static std::unique_ptr<SeriliazationTimeoutCheckEnableParams> Create(const PtJson &params);

    int32_t GetThreshold() const
    {
        return threshold_;
    }

    SeriliazationTimeoutCheckEnableParams &SetThreshold(int32_t threshold)
    {
        threshold_ = threshold;
        return *this;
    }

private:
    NO_COPY_SEMANTIC(SeriliazationTimeoutCheckEnableParams);
    NO_MOVE_SEMANTIC(SeriliazationTimeoutCheckEnableParams);
    static constexpr int32_t DEFAULT_THRESHOLD = 8;
    int32_t threshold_ { DEFAULT_THRESHOLD };
};

class SaveAllPossibleBreakpointsParams : public PtBaseParams {
public:
    SaveAllPossibleBreakpointsParams() = default;
    ~SaveAllPossibleBreakpointsParams() = default;

    static std::unique_ptr<SaveAllPossibleBreakpointsParams> Create(const PtJson &params);

    const std::unordered_map<std::string, std::vector<std::shared_ptr<BreakpointInfo>>> *GetBreakpointsMap() const
    {
        if (!HasBreakpointsMap()) {
            return nullptr;
        }
        return &(breakpointsMap_.value());
    }

    bool HasBreakpointsMap() const
    {
        return breakpointsMap_.has_value();
    }

private:
    NO_COPY_SEMANTIC(SaveAllPossibleBreakpointsParams);
    NO_MOVE_SEMANTIC(SaveAllPossibleBreakpointsParams);

    std::optional<std::unordered_map<std::string, std::vector<std::shared_ptr<BreakpointInfo>>>>
        breakpointsMap_ {};
};

class SetSymbolicBreakpointsParams : public PtBaseParams {
public:
    SetSymbolicBreakpointsParams() = default;
    ~SetSymbolicBreakpointsParams() = default;

    static std::unique_ptr<SetSymbolicBreakpointsParams> Create(const PtJson &params);

    const std::unordered_set<std::string> *GetFunctionNamesSet() const
    {
        if (!HasSymbolicBreakpoints()) {
            return nullptr;
        }
        return &(functionNamesSet_.value());
    }

    bool HasSymbolicBreakpoints() const
    {
        return functionNamesSet_.has_value();
    }

private:
    NO_COPY_SEMANTIC(SetSymbolicBreakpointsParams);
    NO_MOVE_SEMANTIC(SetSymbolicBreakpointsParams);

    std::optional<std::unordered_set<std::string>> functionNamesSet_ {};
};

class RemoveSymbolicBreakpointsParams : public PtBaseParams {
public:
    RemoveSymbolicBreakpointsParams() = default;
    ~RemoveSymbolicBreakpointsParams() = default;

    static std::unique_ptr<RemoveSymbolicBreakpointsParams> Create(const PtJson &params);

    const std::unordered_set<std::string> *GetFunctionNamesSet() const
    {
        if (!HasSymbolicBreakpoints()) {
            return nullptr;
        }
        return &(functionNamesSet_.value());
    }

    bool HasSymbolicBreakpoints() const
    {
        return functionNamesSet_.has_value();
    }

private:
    NO_COPY_SEMANTIC(RemoveSymbolicBreakpointsParams);
    NO_MOVE_SEMANTIC(RemoveSymbolicBreakpointsParams);

    std::optional<std::unordered_set<std::string>> functionNamesSet_ {};
};

}  // namespace panda::ecmascript::tooling
#endif