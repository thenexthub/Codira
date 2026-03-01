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

#include "tooling/dynamic/base/pt_types.h"
#include "tooling/dynamic/base/pt_params.h"
#include "dispatcher.h"

#include "ecmascript/js_array.h"
#include "ecmascript/js_object-inl.h"
#include "ecmascript/js_tagged_value-inl.h"
#include "ecmascript/object_factory.h"
#include "ecmascript/tests/test_helper.h"

using namespace panda::ecmascript;
using namespace panda::ecmascript::tooling;

namespace panda::test {
// Duplicate name of panda::ecmascript::PropertyDescriptor in js_object-inl.h
using panda::ecmascript::tooling::PropertyDescriptor;

using ObjectType = RemoteObject::TypeName;
using ObjectSubType = RemoteObject::SubTypeName;
using ObjectClassName = RemoteObject::ClassName;

class DebuggerParamsTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        GTEST_LOG_(INFO) << "SetUpTestCase";
        Logger::InitializeStdLogging(Logger::Level::FATAL, LoggerComponentMaskAll);
    }

    static void TearDownTestCase()
    {
        Logger::InitializeStdLogging(Logger::Level::ERROR, LoggerComponentMaskAll);
        GTEST_LOG_(INFO) << "TearDownCase";
    }

    void SetUp() override
    {
        TestHelper::CreateEcmaVMWithScope(ecmaVm, thread, scope);
        // Main logic is JSON parser, so not need trigger GC to decrease execute time
        ecmaVm->SetEnableForceGC(false);
    }

    void TearDown() override
    {
        TestHelper::DestroyEcmaVMWithScope(ecmaVm, scope);
    }

protected:
    EcmaVM *ecmaVm {nullptr};
    EcmaHandleScope *scope {nullptr};
    JSThread *thread {nullptr};
};

HWTEST_F_L0(DebuggerParamsTest, ContinueToLocationParamsCreateTest)
{
    std::string msg;
    std::unique_ptr<ContinueToLocationParams> objectData;

    // abnormal params of null msg
    msg = std::string() + R"({})";
    objectData = ContinueToLocationParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    // abnormal params of unexist key params
    msg = std::string() + R"({"id":0,"method":"Debugger.Test"})";
    objectData = ContinueToLocationParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    // abnormal params of null params.sub-key
    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{}})";
    objectData = ContinueToLocationParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"location":10,
        "targetCallFrames":"testTargetCallFrames"}})";
    objectData = ContinueToLocationParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"location":{"scriptId":"2", "lineNumber":3,
        "columnNumber":20}, "targetCallFrames":"testTargetCallFrames"}})";
    objectData = ContinueToLocationParams::Create(DispatchRequest(msg).GetParams());
    ASSERT_NE(objectData, nullptr);
    Location *location = objectData->GetLocation();
    EXPECT_EQ(location->GetScriptId(), 2);
    EXPECT_EQ(location->GetLine(), 3);
    EXPECT_EQ(location->GetColumn(), 20);
    EXPECT_EQ(objectData->GetTargetCallFrames(), "testTargetCallFrames");
}

HWTEST_F_L0(DebuggerParamsTest, SetBreakpointsActiveParamsCreateTest)
{
    std::string msg;
    std::unique_ptr<SetBreakpointsActiveParams> objectData;

    // abnormal params of null msg
    msg = std::string() + R"({})";
    objectData = SetBreakpointsActiveParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    // abnormal params of unexist key params
    msg = std::string() + R"({"id":0,"method":"Debugger.Test"})";
    objectData = SetBreakpointsActiveParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    // abnormal params of null params.sub-key
    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{}})";
    objectData = SetBreakpointsActiveParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);


    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"active":true}})";
    objectData = SetBreakpointsActiveParams::Create(DispatchRequest(msg).GetParams());
    ASSERT_NE(objectData, nullptr);
    EXPECT_TRUE(objectData->GetBreakpointsState());
}

HWTEST_F_L0(DebuggerParamsTest, SetSkipAllPausesParamsCreateTest)
{
    std::string msg;
    std::unique_ptr<SetSkipAllPausesParams> objectData;

    // abnormal params of null msg
    msg = std::string() + R"({})";
    objectData = SetSkipAllPausesParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    // abnormal params of unexist key params
    msg = std::string() + R"({"id":0,"method":"Debugger.Test"})";
    objectData = SetSkipAllPausesParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    // abnormal params of null params.sub-key
    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{}})";
    objectData = SetSkipAllPausesParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"skip":true}})";
    objectData = SetSkipAllPausesParams::Create(DispatchRequest(msg).GetParams());
    ASSERT_NE(objectData, nullptr);
    EXPECT_TRUE(objectData->GetSkipAllPausesState());
}

HWTEST_F_L0(DebuggerParamsTest, SetMixedDebugParamsCreateTest)
{
    std::string msg;
    std::unique_ptr<SetMixedDebugParams> objectData;

    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"enabled":"test", "mixedStackEnabled":true}})";
    objectData = SetMixedDebugParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"enabled":true, "mixedStackEnabled":true}})";
    objectData = SetMixedDebugParams::Create(DispatchRequest(msg).GetParams());
    ASSERT_NE(objectData, nullptr);
    EXPECT_TRUE(objectData->GetEnabled());
    EXPECT_TRUE(objectData->GetMixedStackEnabled());
}

HWTEST_F_L0(DebuggerParamsTest, GetPossibleAndSetBreakpointParamsCreateTest)
{
    std::string msg;
    std::unique_ptr<GetPossibleAndSetBreakpointParams> objectData;

    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"locations":10}})";
    objectData = GetPossibleAndSetBreakpointParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"locations":[{"lineNumber":3,
        "columnNumber":20, "url":"Index.ets"}]}})";
    objectData = GetPossibleAndSetBreakpointParams::Create(DispatchRequest(msg).GetParams());
    ASSERT_NE(objectData, nullptr);
    auto breakpointsList = objectData->GetBreakpointsList();
    EXPECT_EQ((*breakpointsList).size(), 1);
    EXPECT_EQ((*breakpointsList)[0]->GetLineNumber(), 3);
    EXPECT_EQ((*breakpointsList)[0]->GetColumnNumber(), 20);
    EXPECT_EQ((*breakpointsList)[0]->GetUrl(), "Index.ets");
}

HWTEST_F_L0(DebuggerParamsTest, DropFrameParamsCreateTest)
{
    std::string msg;
    std::unique_ptr<DropFrameParams> objectData;

    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"droppedDepth":true}})";
    objectData = DropFrameParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"droppedDepth":3}})";
    objectData = DropFrameParams::Create(DispatchRequest(msg).GetParams());
    ASSERT_NE(objectData, nullptr);
    EXPECT_EQ(objectData->GetDroppedDepth(), 3);
}

HWTEST_F_L0(DebuggerParamsTest, EnableParamsCreateTest)
{
    std::string msg;
    std::unique_ptr<EnableParams> enableParams;

    // abnormal
    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{}})";
    enableParams = EnableParams::Create(DispatchRequest(msg).GetParams());
    ASSERT_NE(enableParams, nullptr);
    EXPECT_FALSE(enableParams->HasMaxScriptsCacheSize());

    // normal
    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"maxScriptsCacheSize":100}})";
    enableParams = EnableParams::Create(DispatchRequest(msg).GetParams());
    ASSERT_NE(enableParams, nullptr);
    EXPECT_EQ(enableParams->GetMaxScriptsCacheSize(), 100);
}

HWTEST_F_L0(DebuggerParamsTest, EnableParamsAccelerateLaunchModeCreateTest)
{
    std::string msg;
    std::unique_ptr<EnableParams> enableParams;

    // abnormal
    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{}})";
    enableParams = EnableParams::Create(DispatchRequest(msg).GetParams());
    ASSERT_NE(enableParams, nullptr);
    EXPECT_FALSE(enableParams->HasMaxScriptsCacheSize());

    // normal
    msg = std::string() + R"({"id":0,"method":"Debugger.Test",
        "params":{"options":["enableLaunchAccelerate"], "maxScriptsCacheSize":100}})";
    enableParams = EnableParams::Create(DispatchRequest(msg).GetParams());
    ASSERT_NE(enableParams, nullptr);
    EXPECT_EQ(enableParams->GetMaxScriptsCacheSize(), 100);
    EXPECT_EQ(enableParams->GetEnableOptionsList().size(), 1);
}

HWTEST_F_L0(DebuggerParamsTest, SaveAllPossibleBreakpointsParamsCreateTest)
{
    std::string msg;
    std::unique_ptr<SaveAllPossibleBreakpointsParams> params;

    // abnormal
    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{}})";
    params = SaveAllPossibleBreakpointsParams::Create(DispatchRequest(msg).GetParams());
    ASSERT_EQ(params, nullptr);

    // normal
    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{
            "locations":{"actualUrl/actualFileName.ts":[{"lineNumber": 7, "columnNumber": 8}]}}})";
    params = SaveAllPossibleBreakpointsParams::Create(DispatchRequest(msg).GetParams());
    ASSERT_NE(params, nullptr);
    EXPECT_TRUE(params->HasBreakpointsMap());
    EXPECT_EQ(params->GetBreakpointsMap()->size(), 1);
}

HWTEST_F_L0(DebuggerParamsTest, SetSymbolicBreakpointsParamsCreateTest)
{
    std::string msg;
    std::unique_ptr<SetSymbolicBreakpointsParams> params;

    // abnormal
    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{}})";
    params = SetSymbolicBreakpointsParams::Create(DispatchRequest(msg).GetParams());
    ASSERT_EQ(params, nullptr);

    // abnormal
    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"symbolicBreakpoints":[
        {"condition":"testDebug"}]}})";
    params = SetSymbolicBreakpointsParams::Create(DispatchRequest(msg).GetParams());
    ASSERT_EQ(params, nullptr);

    // normal
    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"symbolicBreakpoints":[
        {"functionName":"testDebug1"}, {"functionName":"testDebug2"}]}})";
    params = SetSymbolicBreakpointsParams::Create(DispatchRequest(msg).GetParams());
    ASSERT_NE(params, nullptr);
    EXPECT_TRUE(params->HasSymbolicBreakpoints());
    EXPECT_EQ(params->GetFunctionNamesSet()->size(), 2);
}

HWTEST_F_L0(DebuggerParamsTest, StartSamplingParamsCreateTest)
{
    std::string msg;
    std::unique_ptr<StartSamplingParams> startSamplingData;

    // abnormal params of null params.sub-key
    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{}})";
    startSamplingData = StartSamplingParams::Create(DispatchRequest(msg).GetParams());
    ASSERT_NE(startSamplingData, nullptr);
    EXPECT_EQ(startSamplingData->GetSamplingInterval(), 32768);

    // abnormal params of unknown params.sub-key
    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"unknownKey":100}})";
    startSamplingData = StartSamplingParams::Create(DispatchRequest(msg).GetParams());
    ASSERT_NE(startSamplingData, nullptr);
    EXPECT_EQ(startSamplingData->GetSamplingInterval(), 32768);

    // abnormal params of params.sub-key=["samplingInterval":true]
    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"samplingInterval":true}})";
    startSamplingData = StartSamplingParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(startSamplingData, nullptr);

    // abnormal params of params.sub-key=["samplingInterval":true]
    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"samplingInterval":"Test"}})";
    startSamplingData = StartSamplingParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(startSamplingData, nullptr);

    // abnormal params of params.sub-key = [ "size"=100,"nodeId"=1,"ordinal"=10]
    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"samplingInterval":1000}})";
    startSamplingData = StartSamplingParams::Create(DispatchRequest(msg).GetParams());
    ASSERT_NE(startSamplingData, nullptr);
    EXPECT_EQ(startSamplingData->GetSamplingInterval(), 1000);
}

HWTEST_F_L0(DebuggerParamsTest, StartTrackingHeapObjectsParamsCreateTest)
{
    std::string msg;
    std::unique_ptr<StartTrackingHeapObjectsParams> objectData;

    // abnormal params of null params.sub-key
    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{}})";
    objectData = StartTrackingHeapObjectsParams::Create(DispatchRequest(msg).GetParams());
    ASSERT_NE(objectData, nullptr);
    ASSERT_FALSE(objectData->GetTrackAllocations());

    // abnormal params of unknown params.sub-key
    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"unknownKey":100}})";
    objectData = StartTrackingHeapObjectsParams::Create(DispatchRequest(msg).GetParams());
    ASSERT_NE(objectData, nullptr);
    ASSERT_FALSE(objectData->GetTrackAllocations());

    // abnormal params of params.sub-key=["trackAllocations":10]
    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"trackAllocations":10}})";
    objectData = StartTrackingHeapObjectsParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    // abnormal params of params.sub-key=["trackAllocations":"Test"]
    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"trackAllocations":"Test"}})";
    objectData = StartTrackingHeapObjectsParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    // abnormal params of params.sub-key=["trackAllocations":true]
    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"trackAllocations":true}})";
    objectData = StartTrackingHeapObjectsParams::Create(DispatchRequest(msg).GetParams());
    ASSERT_NE(objectData, nullptr);
    ASSERT_TRUE(objectData->GetTrackAllocations());
}

HWTEST_F_L0(DebuggerParamsTest, StopTrackingHeapObjectsParamsCreateTest)
{
    std::string msg;
    std::unique_ptr<StopTrackingHeapObjectsParams> objectData;

    // abnormal params of null params.sub-key
    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{}})";
    objectData = StopTrackingHeapObjectsParams::Create(DispatchRequest(msg).GetParams());
    ASSERT_NE(objectData, nullptr);
    ASSERT_FALSE(objectData->GetReportProgress());
    ASSERT_FALSE(objectData->GetTreatGlobalObjectsAsRoots());
    ASSERT_FALSE(objectData->GetCaptureNumericValue());

    // abnormal params of unknown params.sub-key
    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"unknownKey":100}})";
    objectData = StopTrackingHeapObjectsParams::Create(DispatchRequest(msg).GetParams());
    ASSERT_NE(objectData, nullptr);
    ASSERT_FALSE(objectData->GetReportProgress());
    ASSERT_FALSE(objectData->GetTreatGlobalObjectsAsRoots());
    ASSERT_FALSE(objectData->GetCaptureNumericValue());

    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{
            "reportProgress":10,
            "treatGlobalObjectsAsRoots":10,
            "captureNumericValue":10}})";
    objectData = StopTrackingHeapObjectsParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{
            "reportProgress":"Test",
            "treatGlobalObjectsAsRoots":"Test",
            "captureNumericValue":"Test"}})";
    objectData = StopTrackingHeapObjectsParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{
            "reportProgress":true,
            "treatGlobalObjectsAsRoots":true,
            "captureNumericValue":true}})";
    objectData = StopTrackingHeapObjectsParams::Create(DispatchRequest(msg).GetParams());
    ASSERT_NE(objectData, nullptr);
    ASSERT_TRUE(objectData->GetReportProgress());
    ASSERT_TRUE(objectData->GetTreatGlobalObjectsAsRoots());
    ASSERT_TRUE(objectData->GetCaptureNumericValue());
}

HWTEST_F_L0(DebuggerParamsTest, AddInspectedHeapObjectParamsCreateTest)
{
    std::string msg;
    std::unique_ptr<AddInspectedHeapObjectParams> objectData;

    //  abnormal params of null msg
    msg = std::string() + R"({})";
    objectData = AddInspectedHeapObjectParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    // abnormal params of unexist key params
    msg = std::string() + R"({"id":0,"method":"Debugger.Test"})";
    objectData = AddInspectedHeapObjectParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    // abnormal params of null params.sub-key
    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{}})";
    objectData = AddInspectedHeapObjectParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    // abnormal params of unknown params.sub-key
    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"unknownKey":100}})";
    objectData = AddInspectedHeapObjectParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    // abnormal params of params.sub-key=["heapObjectId":10]
    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"heapObjectId":10}})";
    objectData = AddInspectedHeapObjectParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    // abnormal params of params.sub-key=["heapObjectId":true]
    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"heapObjectId":true}})";
    objectData = AddInspectedHeapObjectParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    // abnormal params of params.sub-key=["heapObjectId":“10”]
    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"heapObjectId":"10"}})";
    objectData = AddInspectedHeapObjectParams::Create(DispatchRequest(msg).GetParams());
    ASSERT_NE(objectData, nullptr);
    EXPECT_EQ(objectData->GetHeapObjectId(), 10);
}

HWTEST_F_L0(DebuggerParamsTest, GetHeapObjectIdParamsCreateTest)
{
    std::string msg;
    std::unique_ptr<GetHeapObjectIdParams> objectData;

    // abnormal params of params.sub-key=["objectId":10]
    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"objectId":10}})";
    objectData = GetHeapObjectIdParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    // abnormal params of params.sub-key=["objectId":true]
    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"objectId":true}})";
    objectData = GetHeapObjectIdParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    // abnormal params of params.sub-key=["objectId":“10”]
    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"objectId":"10"}})";
    objectData = GetHeapObjectIdParams::Create(DispatchRequest(msg).GetParams());
    ASSERT_NE(objectData, nullptr);
    EXPECT_EQ((int)objectData->GetObjectId(), 10);
}

HWTEST_F_L0(DebuggerParamsTest, GetObjectByHeapObjectIdParamsCreateTest)
{
    std::string msg;
    std::unique_ptr<GetObjectByHeapObjectIdParams> objectData;

    // abnormal params of params.sub-key=["objectId":10]
    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"objectId":10}})";
    objectData = GetObjectByHeapObjectIdParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    // abnormal params of params.sub-key=["objectId":true]
    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"objectId":"10", "objectGroup":10}})";
    objectData = GetObjectByHeapObjectIdParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    // abnormal params of params.sub-key=["objectId":“10”]
    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"objectId":"10"}})";
    objectData = GetObjectByHeapObjectIdParams::Create(DispatchRequest(msg).GetParams());
    ASSERT_NE(objectData, nullptr);
    EXPECT_EQ((int)objectData->GetObjectId(), 10);
    ASSERT_FALSE(objectData->HasObjectGroup());

    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"objectId":"10", "objectGroup":"groupname"}})";
    objectData = GetObjectByHeapObjectIdParams::Create(DispatchRequest(msg).GetParams());
    ASSERT_NE(objectData, nullptr);
    EXPECT_EQ((int)objectData->GetObjectId(), 10);
    EXPECT_EQ(objectData->GetObjectGroup(), "groupname");
}

HWTEST_F_L0(DebuggerParamsTest, StartPreciseCoverageParamCreateTest)
{
    std::string msg;
    std::unique_ptr<StartPreciseCoverageParams> objectData;

    // abnormal params of null params.sub-key
    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{}})";
    objectData = StartPreciseCoverageParams::Create(DispatchRequest(msg).GetParams());
    ASSERT_NE(objectData, nullptr);

    // abnormal params of unknown params.sub-key
    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"unknownKey":100}})";
    objectData = StartPreciseCoverageParams::Create(DispatchRequest(msg).GetParams());
    ASSERT_NE(objectData, nullptr);

    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{
            "callCount":8,
            "detailed":8,
            "allowTriggeredUpdates":8}})";
    objectData = StartPreciseCoverageParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{
            "callCount":"Test",
            "detailed":"Test",
            "allowTriggeredUpdates":"Test"}})";
    objectData = StartPreciseCoverageParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{
            "callCount":true,
            "detailed":true,
            "allowTriggeredUpdates":true}})";
    objectData = StartPreciseCoverageParams::Create(DispatchRequest(msg).GetParams());
    ASSERT_NE(objectData, nullptr);
    ASSERT_TRUE(objectData->GetCallCount());
    ASSERT_TRUE(objectData->GetDetailed());
    ASSERT_TRUE(objectData->GetAllowTriggeredUpdates());
}

HWTEST_F_L0(DebuggerParamsTest, SetSamplingIntervalParamsCreateTest)
{
    std::string msg;
    std::unique_ptr<SetSamplingIntervalParams> objectData;

    //  abnormal params of null msg
    msg = std::string() + R"({})";
    objectData = SetSamplingIntervalParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    // abnormal params of unexist key params
    msg = std::string() + R"({"id":0,"method":"Debugger.Test"})";
    objectData = SetSamplingIntervalParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    // abnormal params of null params.sub-key
    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{}})";
    objectData = SetSamplingIntervalParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    // abnormal params of unknown params.sub-key
    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"unknownKey":100}})";
    objectData = SetSamplingIntervalParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{
            "interval":"500"}})";
    objectData = SetSamplingIntervalParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"interval":500}})";
    objectData = SetSamplingIntervalParams::Create(DispatchRequest(msg).GetParams());
    ASSERT_NE(objectData, nullptr);
    EXPECT_EQ(objectData->GetInterval(), 500);
}

HWTEST_F_L0(DebuggerParamsTest, RecordClockSyncMarkerParamsCreateTest)
{
    std::string msg;
    std::unique_ptr<RecordClockSyncMarkerParams> objectData;

    //  abnormal params of null msg
    msg = std::string() + R"({})";
    objectData = RecordClockSyncMarkerParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    // abnormal params of unexist key params
    msg = std::string() + R"({"id":0,"method":"Debugger.Test"})";
    objectData = RecordClockSyncMarkerParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    // abnormal params of null params.sub-key
    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{}})";
    objectData = RecordClockSyncMarkerParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    // abnormal params of unknown params.sub-key
    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"unknownKey":100}})";
    objectData = RecordClockSyncMarkerParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"syncId":"101"}})";
    objectData = RecordClockSyncMarkerParams::Create(DispatchRequest(msg).GetParams());
    ASSERT_NE(objectData, nullptr);
    EXPECT_EQ(objectData->GetSyncId(), "101");
}

HWTEST_F_L0(DebuggerParamsTest, RequestMemoryDumpParamsCreateTest)
{
    std::string msg;
    std::unique_ptr<RequestMemoryDumpParams> objectData;

    //  abnormal params of null msg
    msg = std::string() + R"({})";
    objectData = RequestMemoryDumpParams::Create(DispatchRequest(msg).GetParams());
    ASSERT_NE(objectData, nullptr);

    // abnormal params of unexist key params
    msg = std::string() + R"({"id":0,"method":"Debugger.Test"})";
    objectData = RequestMemoryDumpParams::Create(DispatchRequest(msg).GetParams());
    ASSERT_NE(objectData, nullptr);

    // abnormal params of null params.sub-key
    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{}})";
    objectData = RequestMemoryDumpParams::Create(DispatchRequest(msg).GetParams());
    ASSERT_NE(objectData, nullptr);

    // abnormal params of unknown params.sub-key
    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"unknownKey":100}})";
    objectData = RequestMemoryDumpParams::Create(DispatchRequest(msg).GetParams());
    ASSERT_NE(objectData, nullptr);

    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"deterministic":true,
            "levelOfDetail":"background"}})";
    objectData = RequestMemoryDumpParams::Create(DispatchRequest(msg).GetParams());
    ASSERT_NE(objectData, nullptr);
    ASSERT_TRUE(objectData->GetDeterministic());
    EXPECT_EQ(objectData->GetLevelOfDetail(), "background");
}

HWTEST_F_L0(DebuggerParamsTest, StartParamsCreateTest)
{
    std::string msg;
    std::unique_ptr<StartParams> objectData;

    //  abnormal params of null msg
    msg = std::string() + R"({})";
    objectData = StartParams::Create(DispatchRequest(msg).GetParams());
    ASSERT_NE(objectData, nullptr);

    // abnormal params of unexist key params
    msg = std::string() + R"({"id":0,"method":"Debugger.Test"})";
    objectData = StartParams::Create(DispatchRequest(msg).GetParams());
    ASSERT_NE(objectData, nullptr);

    // abnormal params of null params.sub-key
    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{}})";
    objectData = StartParams::Create(DispatchRequest(msg).GetParams());
    ASSERT_NE(objectData, nullptr);

    // abnormal params of unknown params.sub-key
    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"unknownKey":100}})";
    objectData = StartParams::Create(DispatchRequest(msg).GetParams());
    ASSERT_NE(objectData, nullptr);

    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"categories":"filter1",
            "options":"1", "bufferUsageReportingInterval":11, "transferMode":"ReportEvents", "streamFormat":"json",
            "streamCompression":"none", "traceConfig": {"recordMode":"recordUntilFull"}, "perfettoConfig":"categories",
            "tracingBackend":"auto"}})";
    objectData = StartParams::Create(DispatchRequest(msg).GetParams());
    ASSERT_NE(objectData, nullptr);
    EXPECT_EQ(objectData->GetCategories(), "filter1");
    EXPECT_EQ(objectData->GetOptions(), "1");
    EXPECT_EQ(objectData->GetBufferUsageReportingInterval(), 11);
    EXPECT_EQ(objectData->GetTransferMode(), "ReportEvents");
    EXPECT_EQ(objectData->GetStreamFormat(), "json");
    EXPECT_EQ(objectData->GetStreamCompression(), "none");
    TraceConfig *traceConfig = objectData->GetTraceConfig();
    ASSERT_NE(traceConfig, nullptr);
    EXPECT_EQ(traceConfig->GetRecordMode(), "recordUntilFull");
    EXPECT_EQ(objectData->GetPerfettoConfig(), "categories");
    EXPECT_EQ(objectData->GetTracingBackend(), "auto");
}

HWTEST_F_L0(DebuggerParamsTest, EvaluateOnCallFrameParamsCreateTest)
{
    std::string msg;
    std::unique_ptr<EvaluateOnCallFrameParams> objectData;

    // abnormal params of null msg
    msg = std::string() + R"({})";
    objectData = EvaluateOnCallFrameParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    // abnormal params of unexist key params
    msg = std::string() + R"({"id":0,"method":"Debugger.Test"})";
    objectData = EvaluateOnCallFrameParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    // abnormal params of null params.sub-key
    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{}})";
    objectData = EvaluateOnCallFrameParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"callFrameId":1, "expression":true,
        "objectGroup":1, "includeCommandLineAPI":"true", "silent":"true", "returnByValue":"true",
        "generatePreview":"true", "throwOnSideEffect":"true", "timeout":"10"}})";
    EXPECT_EQ(objectData, nullptr);

    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"callFrameId":"1", "expression":"test",
        "objectGroup":"testGroup", "includeCommandLineAPI":true, "silent":true, "returnByValue":true,
        "generatePreview":true, "throwOnSideEffect":true, "timeout":10}})";
    objectData = EvaluateOnCallFrameParams::Create(DispatchRequest(msg).GetParams());
    ASSERT_NE(objectData, nullptr);
    EXPECT_EQ(objectData->GetCallFrameId(), 1);
    EXPECT_EQ(objectData->GetExpression(), "test");
}

HWTEST_F_L0(DebuggerParamsTest, GetPossibleBreakpointsParamsCreateTest)
{
    std::string msg;
    std::unique_ptr<GetPossibleBreakpointsParams> objectData;

    // abnormal params of null msg
    msg = std::string() + R"({})";
    objectData = GetPossibleBreakpointsParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    // abnormal params of unexist key params
    msg = std::string() + R"({"id":0,"method":"Debugger.Test"})";
    objectData = GetPossibleBreakpointsParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    // abnormal params of null params.sub-key
    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{}})";
    objectData = GetPossibleBreakpointsParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"start":10, "end":10,
        "restrictToFunction":"true"}})";
    EXPECT_EQ(objectData, nullptr);

    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"start":{"scriptId":"1", "lineNumber":1,
        "columnNumber":10}, "end":{"scriptId":"1", "lineNumber":10, "columnNumber":10}, "restrictToFunction":true}})";
    objectData = GetPossibleBreakpointsParams::Create(DispatchRequest(msg).GetParams());
    ASSERT_NE(objectData, nullptr);
    Location *start = objectData->GetStart();
    Location *end = objectData->GetEnd();
    EXPECT_EQ(start->GetScriptId(), 1);
    EXPECT_EQ(start->GetLine(), 1);
    EXPECT_EQ(start->GetColumn(), 10);
    EXPECT_EQ(end->GetScriptId(), 1);
    EXPECT_EQ(end->GetLine(), 10);
    EXPECT_EQ(end->GetColumn(), 10);
    ASSERT_TRUE(objectData->GetRestrictToFunction());
}

HWTEST_F_L0(DebuggerParamsTest, GetScriptSourceParamsCreateTest)
{
    std::string msg;
    std::unique_ptr<GetScriptSourceParams> objectData;

    // abnormal params of null msg
    msg = std::string() + R"({})";
    objectData = GetScriptSourceParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    // abnormal params of unexist key params
    msg = std::string() + R"({"id":0,"method":"Debugger.Test"})";
    objectData = GetScriptSourceParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    // abnormal params of null params.sub-key
    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{}})";
    objectData = GetScriptSourceParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"scriptId":10}})";
    objectData = GetScriptSourceParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"scriptId":"10"}})";
    objectData = GetScriptSourceParams::Create(DispatchRequest(msg).GetParams());
    ASSERT_NE(objectData, nullptr);
    EXPECT_EQ(objectData->GetScriptId(), 10);
}

HWTEST_F_L0(DebuggerParamsTest, RemoveBreakpointParamsCreateTest)
{
    std::string msg;
    std::unique_ptr<RemoveBreakpointParams> objectData;

    // abnormal params of null msg
    msg = std::string() + R"({})";
    objectData = RemoveBreakpointParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    // abnormal params of unexist key params
    msg = std::string() + R"({"id":0,"method":"Debugger.Test"})";
    objectData = RemoveBreakpointParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    // abnormal params of null params.sub-key
    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{}})";
    objectData = RemoveBreakpointParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"breakpointId":10}})";
    objectData = RemoveBreakpointParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"breakpointId":"10"}})";
    objectData = RemoveBreakpointParams::Create(DispatchRequest(msg).GetParams());
    ASSERT_NE(objectData, nullptr);
    EXPECT_EQ(objectData->GetBreakpointId(), "10");
}

HWTEST_F_L0(DebuggerParamsTest, RemoveBreakpointsByUrlParamsCreateTest)
{
    std::string msg;
    std::unique_ptr<RemoveBreakpointsByUrlParams> objectData;

    // abnormal params of null msg
    msg = std::string() + R"({})";
    objectData = RemoveBreakpointsByUrlParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    // abnormal params of unexist key params
    msg = std::string() + R"({"id":0,"method":"Debugger.Test"})";
    objectData = RemoveBreakpointsByUrlParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    // abnormal params of null params.sub-key
    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{}})";
    objectData = RemoveBreakpointsByUrlParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"url":10}})";
    objectData = RemoveBreakpointsByUrlParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"url":"10"}})";
    objectData = RemoveBreakpointsByUrlParams::Create(DispatchRequest(msg).GetParams());
    ASSERT_NE(objectData, nullptr);
    EXPECT_EQ(objectData->GetUrl(), "10");
}

HWTEST_F_L0(DebuggerParamsTest, ResumeParamsCreateTest)
{
    std::string msg;
    std::unique_ptr<ResumeParams> objectData;

    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"terminateOnResume":10}})";
    objectData = ResumeParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"terminateOnResume":true}})";
    objectData = ResumeParams::Create(DispatchRequest(msg).GetParams());
    ASSERT_NE(objectData, nullptr);
    ASSERT_TRUE(objectData->GetTerminateOnResume());
}

HWTEST_F_L0(DebuggerParamsTest, SetAsyncCallStackDepthParamsCreateTest)
{
    std::string msg;
    std::unique_ptr<SetAsyncCallStackDepthParams> objectData;

    // abnormal params of null msg
    msg = std::string() + R"({})";
    objectData = SetAsyncCallStackDepthParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    // abnormal params of unexist key params
    msg = std::string() + R"({"id":0,"method":"Debugger.Test"})";
    objectData = SetAsyncCallStackDepthParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    // abnormal params of null params.sub-key
    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{}})";
    objectData = SetAsyncCallStackDepthParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"maxDepth":true}})";
    objectData = SetAsyncCallStackDepthParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"maxDepth":5}})";
    objectData = SetAsyncCallStackDepthParams::Create(DispatchRequest(msg).GetParams());
    ASSERT_NE(objectData, nullptr);
    EXPECT_EQ(objectData->GetMaxDepth(), 5);
}

HWTEST_F_L0(DebuggerParamsTest, SetBlackboxPatternsParamsCreateTest)
{
    std::string msg;
    std::unique_ptr<SetBlackboxPatternsParams> objectData;

    // abnormal params of null msg
    msg = std::string() + R"({})";
    objectData = SetBlackboxPatternsParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    // abnormal params of unexist key params
    msg = std::string() + R"({"id":0,"method":"Debugger.Test"})";
    objectData = SetBlackboxPatternsParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    // abnormal params of null params.sub-key
    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{}})";
    objectData = SetBlackboxPatternsParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"patterns":"testpattern"}})";
    objectData = SetBlackboxPatternsParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"patterns":["testpattern1", "testpattern2"]}})";
    objectData = SetBlackboxPatternsParams::Create(DispatchRequest(msg).GetParams());
    ASSERT_NE(objectData, nullptr);
    std::list<std::string> patterns = objectData->GetPatterns();
    EXPECT_EQ(patterns.size(), 2U);
}

HWTEST_F_L0(DebuggerParamsTest, SetBreakpointByUrlParamsCreateTest)
{
    std::string msg;
    std::unique_ptr<SetBreakpointByUrlParams> objectData;

    // abnormal params of null msg
    msg = std::string() + R"({})";
    objectData = SetBreakpointByUrlParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    // abnormal params of unexist key params
    msg = std::string() + R"({"id":0,"method":"Debugger.Test"})";
    objectData = SetBreakpointByUrlParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    // abnormal params of null params.sub-key
    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{}})";
    objectData = SetBreakpointByUrlParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"lineNumber":"10", "url":10,
        "urlRegex":true, "scriptHash":true, "columnNumber":"10", "condition":10}})";
    objectData = SetBreakpointByUrlParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"lineNumber":10, "url":"testURL",
        "urlRegex":"testURLRegex", "scriptHash":"testScriptHash", "columnNumber":10, "condition":"testCondition"}})";
    objectData = SetBreakpointByUrlParams::Create(DispatchRequest(msg).GetParams());
    ASSERT_NE(objectData, nullptr);
    EXPECT_EQ(objectData->GetLine(), 10);
    EXPECT_EQ(objectData->GetUrl(), "testURL");
    EXPECT_EQ(objectData->GetUrlRegex(), "testURLRegex");
    EXPECT_EQ(objectData->GetScriptHash(), "testScriptHash");
    EXPECT_EQ(objectData->GetColumn(), 10);
    EXPECT_EQ(objectData->GetCondition(), "testCondition");
}

HWTEST_F_L0(DebuggerParamsTest, SetPauseOnExceptionsParamsCreateTest)
{
    std::string msg;
    std::unique_ptr<SetPauseOnExceptionsParams> objectData;

    // abnormal params of null msg
    msg = std::string() + R"({})";
    objectData = SetPauseOnExceptionsParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    // abnormal params of unexist key params
    msg = std::string() + R"({"id":0,"method":"Debugger.Test"})";
    objectData = SetPauseOnExceptionsParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    // abnormal params of null params.sub-key
    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{}})";
    objectData = SetPauseOnExceptionsParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"state":10}})";
    objectData = SetPauseOnExceptionsParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"state":"none"}})";
    objectData = SetPauseOnExceptionsParams::Create(DispatchRequest(msg).GetParams());
    ASSERT_NE(objectData, nullptr);
    EXPECT_EQ(objectData->GetState(), PauseOnExceptionsState::NONE);
}

HWTEST_F_L0(DebuggerParamsTest, StepIntoParamsCreateTest)
{
    std::string msg;
    std::unique_ptr<StepIntoParams> objectData;

    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"breakOnAsyncCall":10, "skipList":true}})";
    objectData = StepIntoParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"breakOnAsyncCall":true,
        "skipList":[{"scriptId":"1", "start":{"lineNumber":1, "columnNumber":10}, "end":{"lineNumber":2,
        "columnNumber":20}}, {"scriptId":"2", "start":{"lineNumber":3, "columnNumber":30}, "end":{"lineNumber":4,
        "columnNumber":40}}]}})";
    objectData = StepIntoParams::Create(DispatchRequest(msg).GetParams());
    ASSERT_NE(objectData, nullptr);
    ASSERT_TRUE(objectData->GetBreakOnAsyncCall());
    const std::list<std::unique_ptr<LocationRange>> *locationRangeList = objectData->GetSkipList();
    EXPECT_EQ(locationRangeList->size(), 2U);
}


HWTEST_F_L0(DebuggerParamsTest, StepOverParamsCreateTest)
{
    std::string msg;
    std::unique_ptr<StepOverParams> objectData;

    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"skipList":true}})";
    objectData = StepOverParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"skipList":[{"scriptId":"1",
        "start":{"lineNumber":1, "columnNumber":10}, "end":{"lineNumber":2, "columnNumber":20}}, {"scriptId":"2",
        "start":{"lineNumber":3, "columnNumber":30}, "end":{"lineNumber":4, "columnNumber":40}}]}})";
    objectData = StepOverParams::Create(DispatchRequest(msg).GetParams());
    ASSERT_NE(objectData, nullptr);
    const std::list<std::unique_ptr<LocationRange>> *locationRangeList = objectData->GetSkipList();
    EXPECT_EQ(locationRangeList->size(), 2U);
}

HWTEST_F_L0(DebuggerParamsTest, ReplyNativeCallingParamsCreateTest)
{
    std::string msg;
    std::unique_ptr<ReplyNativeCallingParams> objectData;

    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"userCode":"test"}})";
    objectData = ReplyNativeCallingParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"userCode":true}})";
    objectData = ReplyNativeCallingParams::Create(DispatchRequest(msg).GetParams());
    ASSERT_NE(objectData, nullptr);
    ASSERT_TRUE(objectData->GetUserCode());
}

HWTEST_F_L0(DebuggerParamsTest, GetPropertiesParamsCreateTest)
{
    std::string msg;
    std::unique_ptr<GetPropertiesParams> objectData;

    // abnormal params of null msg
    msg = std::string() + R"({})";
    objectData = GetPropertiesParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    // abnormal params of unexist key params
    msg = std::string() + R"({"id":0,"method":"Debugger.Test"})";
    objectData = GetPropertiesParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    // abnormal params of null params.sub-key
    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{}})";
    objectData = GetPropertiesParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"objectId":10, "ownProperties":1,
        "accessorPropertiesOnly":2, "generatePreview":3}})";
    objectData = GetPropertiesParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"objectId":"10", "ownProperties":true,
        "accessorPropertiesOnly":true, "generatePreview":true}})";
    objectData = GetPropertiesParams::Create(DispatchRequest(msg).GetParams());
    ASSERT_NE(objectData, nullptr);
    EXPECT_EQ(objectData->GetObjectId(), 10);
    ASSERT_TRUE(objectData->GetOwnProperties());
    ASSERT_TRUE(objectData->GetAccessPropertiesOnly());
    ASSERT_TRUE(objectData->GetGeneratePreview());
}

HWTEST_F_L0(DebuggerParamsTest, CallFunctionOnParamsCreateTest)
{
    std::string msg;
    std::unique_ptr<CallFunctionOnParams> objectData;

    // abnormal params of null msg
    msg = std::string() + R"({})";
    objectData = CallFunctionOnParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    // abnormal params of unexist key params
    msg = std::string() + R"({"id":0,"method":"Debugger.Test"})";
    objectData = CallFunctionOnParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    // abnormal params of null params.sub-key
    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{}})";
    objectData = CallFunctionOnParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    msg = std::string() + R"({"id":0, "method":"Debugger.Test", "params":{"callFrameId":0, "functionDeclaration":true,
        "objectId":1, "arguments":[{"unserializableValue":1, "objectId":10}, {"unserializableValue":2, "objectId":20}],
        "silent":"true", "returnByValue":"true", "generatePreview":"true", "userGesture":"true",
        "awaitPromise":"true", "executionContextId":"1", "objectGroup":2, "throwOnSideEffect":"true"}})";
    objectData = CallFunctionOnParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    msg = std::string() + R"({"id":0, "method":"Debugger.Test", "params":{"callFrameId":"0",
        "functionDeclaration":"testFunction", "objectId":"1",
        "arguments":[{"unserializableValue":"testValue1", "objectId":"10"},
        {"unserializableValue":"testValue2", "objectId":"20"}], "silent":true, "returnByValue":true,
        "generatePreview":true, "userGesture":true, "awaitPromise":true, "executionContextId":1,
        "objectGroup":"testGrp", "throwOnSideEffect":true}})";
    objectData = CallFunctionOnParams::Create(DispatchRequest(msg).GetParams());
    ASSERT_NE(objectData, nullptr);
    EXPECT_EQ(objectData->GetCallFrameId(), 0);
    EXPECT_EQ(objectData->GetFunctionDeclaration(), "testFunction");
    EXPECT_EQ(objectData->GetObjectId(), 1);
    const std::vector<std::unique_ptr<CallArgument>> *callArgument = objectData->GetArguments();
    EXPECT_EQ(callArgument->size(), 2U);
    ASSERT_TRUE(objectData->GetSilent());
    ASSERT_TRUE(objectData->GetReturnByValue());
    ASSERT_TRUE(objectData->GetGeneratePreview());
    ASSERT_TRUE(objectData->GetUserGesture());
    ASSERT_TRUE(objectData->GetAwaitPromise());
    EXPECT_EQ(objectData->GetExecutionContextId(), 1);
    EXPECT_EQ(objectData->GetObjectGroup(), "testGrp");
    ASSERT_TRUE(objectData->GetThrowOnSideEffect());
}

HWTEST_F_L0(DebuggerParamsTest, ResetSingleStepperParamsCreateTest)
{
    std::string msg;
    std::unique_ptr<ResetSingleStepperParams> objectData;

    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"resetSingleStepper":"test"}})";
    objectData = ResetSingleStepperParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(objectData, nullptr);

    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"resetSingleStepper":true}})";
    objectData = ResetSingleStepperParams::Create(DispatchRequest(msg).GetParams());
    ASSERT_NE(objectData, nullptr);
    ASSERT_TRUE(objectData->GetResetSingleStepper());
}
}  // namespace panda::test