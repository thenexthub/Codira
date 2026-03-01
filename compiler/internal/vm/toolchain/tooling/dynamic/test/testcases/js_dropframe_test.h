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

#ifndef ECMASCRIPT_TOOLING_TEST_UTILS_TESTCASES_JS_DROP_FRAME_TEST_H
#define ECMASCRIPT_TOOLING_TEST_UTILS_TESTCASES_JS_DROP_FRAME_TEST_H

#include "test/utils/test_util.h"

namespace panda::ecmascript::tooling::test {
class JsDropFrameTest : public TestEvents {
public:
    JsDropFrameTest()
    {
        channel_ = new JsDropFrameTestChannel();

        breakpoint = [this](const JSPtLocation &location) {
            ASSERT_TRUE(location.GetMethodId().IsValid());
            ++breakpointCounter_;
            debugger_->NotifyPaused(location, PauseReason::INSTRUMENTATION);
            static_cast<JsDropFrameTestChannel *>(channel_)->IncreaseCheckVariableCounter();
            DropFrameIfChecked();
            TestUtil::SuspendUntilContinue(DebugEvent::BREAKPOINT, location);
            return true;
        };

        singleStep = [this](const JSPtLocation &location) {
            if (debugger_->NotifySingleStep(location)) {
                debugger_->NotifyPaused({}, PauseReason::INSTRUMENTATION);
                static_cast<JsDropFrameTestChannel *>(channel_)->IncreaseCheckVariableCounter();
                TestUtil::SuspendUntilContinue(DebugEvent::CHECK_COMPLETE);
                DropFrameIfChecked();
                TestUtil::SuspendUntilContinue(DebugEvent::DROPFRAME);
                return true;
            }
            return false;
        };

        loadModule = [this](std::string_view moduleName) {
            static_cast<JsDropFrameTestChannel *>(channel_)->Initial(vm_, runtime_);
            runtime_->Enable();
            size_t breakpoint[POINTER_SIZE][LINE_COLUMN] =
                {{26, 0}, {42, 0}, {57, 0}, {69, 0}, {87, 0}, {100, 0}, {117, 0},
                 {143, 0}, {151, 0}, {164, 0}, {172, 0}, {182, 0}, {192, 0}, {206, 0},
                 {217, 0}, {224, 0}, {230, 0}}; // 0-based
            SetJSPtLocation(breakpoint[0], POINTER_SIZE, pointerLocations_);
            InitBreakpointOpQueue();
            TestUtil::SuspendUntilContinue(DebugEvent::LOAD_MODULE);
            ASSERT_EQ(moduleName, pandaFile_);
            ASSERT_TRUE(debugger_->NotifyScriptParsed(0, pandaFile_));
            auto condFuncRef = FunctionRef::Undefined(vm_);
            for (auto &iter : pointerLocations_) {
                auto ret = debugInterface_->SetBreakpoint(iter, condFuncRef);
                ASSERT_TRUE(ret);
            }
            return true;
        };

        scenario = [this]() {
            TestUtil::WaitForLoadModule();
            TestUtil::Continue();
            for (size_t index = 0; index < POINTER_SIZE; index++) {
                for (size_t hitCount = 0; hitCount < breakpointHitTimes[index]; hitCount++) {
                    TestUtil::WaitForBreakpoint(pointerLocations_.at(index));
                    if (hitCount == breakpointHitTimes[index] - 1) {
                        auto ret = debugInterface_->RemoveBreakpoint(pointerLocations_.at(index));
                        ASSERT_TRUE(ret);
                    }
                    TestUtil::Continue();
                    while (dropFrameChecked_) {
                        dropFrameChecked_ = false;
                        TestUtil::WaitForCheckComplete();
                        TestUtil::Continue();
                        TestUtil::WaitForDropframe();
                        dropframeCounter_++;
                        TestUtil::Continue();
                    }
                }
            }
            ASSERT_EXITED();
            return true;
        };

        vmDeath = [this]() {
            ASSERT_EQ(breakpointCounter_, 59U);  // a total of 59 times hitting breakpoints
            ASSERT_EQ(dropframeCounter_, 47U); // a total of 47 times dropping frames
            return true;
        };
    }

    std::pair<std::string, std::string> GetEntryPoint() override
    {
        return {pandaFile_, entryPoint_};
    }
    ~JsDropFrameTest()
    {
        delete channel_;
        channel_ = nullptr;
    }

private:
    class JsDropFrameTestChannel : public TestChannel {
    public:
        JsDropFrameTestChannel() = default;
        ~JsDropFrameTestChannel() = default;
        void Initial(const EcmaVM *vm, RuntimeImpl *runtime)
        {
            vm_ = vm;
            runtime_ = runtime;
        }

        void SendNotification(const PtBaseEvents &events) override
        {
            const static std::vector<std::function<bool(const PtBaseEvents &events)>> eventList = {
                [](const PtBaseEvents &events) -> bool {
                    std::string sourceFile = DEBUGGER_JS_DIR "dropframe.js";
                    auto parsed = static_cast<const ScriptParsed *>(&events);
                    ASSERT_EQ(parsed->GetName(), "Debugger.scriptParsed");
                    ASSERT_EQ(parsed->GetUrl(), sourceFile);
                    return true;
                },
                [this](const PtBaseEvents &events) -> bool {
                    std::cout << checkVariableCounter_ << std::endl;
                    auto paused = static_cast<const Paused *>(&events);
                    ASSERT_EQ(paused->GetName(), "Debugger.paused");
                    auto allFrames = paused->GetCallFrames();
                    std::map<std::string, std::string> truthVariableMap = variableMap_.at(checkVariableCounter_);
                    for (size_t index = 0; index < allFrames->size(); index++) {
                        auto frame = paused->GetCallFrames()->at(index).get();
                        auto scopes = frame->GetScopeChain();
                        for (uint32_t i = 0; i < scopes->size(); i++) {
                            auto scope = scopes->at(i).get();
                            std::string scopeType = scope->GetType();
                            if (scopeType != Scope::Type::Local() && scopeType != Scope::Type::Closure()) {
                                continue;
                            }
                            auto localId = scope->GetObject()->GetObjectId();
                            GetPropertiesParams params;
                            params.SetObjectId(localId).SetOwnProperties(true);
                            std::vector<std::unique_ptr<PropertyDescriptor>> outPropertyDesc;
                            runtime_->GetProperties(params, &outPropertyDesc, {}, {}, {});
                            for (const auto &property : outPropertyDesc) {
                                std::cout << "=====================================" << std::endl;
                                std::string name = property->GetName();
                                std::cout << name << std::endl;
                                if (truthVariableMap.count(name)) {
                                    auto value = property->GetValue();
                                    if (value->HasValue()) {
                                        std::string valueStr = value->GetValue()->ToString(vm_)->ToString(vm_);
                                        std::cout << valueStr << std::endl;
                                        ASSERT_EQ(valueStr, truthVariableMap[name]);
                                    }
                                }
                            }
                        }
                    }
                    return true;
                }
            };

            ASSERT_TRUE(eventList[index_](events));
            index_ |= 1;
        }

        void IncreaseCheckVariableCounter()
        {
            checkVariableCounter_++;
        }

    private:
        NO_COPY_SEMANTIC(JsDropFrameTestChannel);
        NO_MOVE_SEMANTIC(JsDropFrameTestChannel);

        const std::vector<std::map<std::string, std::string>> variableMap_ = {
            { { "a", "2" }, { "b", "3" }, { "c", "4" }, {"d", "2"} },
            { { "a", "2" }, { "b", "3" }, { "c", "3" }, {"d", "1"} },
            { { "a", "2" }, { "b", "2" }, { "c", "3" }, {"d", "0"} },
            { { "a", "1" }, { "b", "2" }, { "c", "3" } },
            { { "a", "2" }, { "b", "3" }, { "c", "4" }, {"d", "2"} },
            { { "a", "4" }, { "b", "5" }, { "c", "6" }, {"d", "2"} },
            { { "a", "6" }, { "b", "4" }, { "c", "5" }, {"d", "2"} },
            { { "a", "6" }, { "b", "4" }, { "c", "5" }, {"d", "1"} },
            { { "a", "2" }, { "b", "3" }, { "c", "4" } },
            { { "a", "4" }, { "b", "5" }, { "c", "6" }, {"d", "2"} },
            { { "a", "8" }, { "b", "13" }, { "c", "78" }, {"d", "1"} },
            { { "a", "4" }, { "b", "5" }, { "c", "6" }, {"d", "1"} },
            { { "a", "8" }, { "b", "13" }, { "c", "78" }, {"d", "1"} },
            { { "a", "16" }, { "b", "29" }, { "c", "2262" }, {"d", "182"} },
            { { "a", "8" }, { "b", "13" }, { "c", "78" }, {"d", "182"} },
            { { "a", "4" }, { "b", "5" }, { "c", "6" } },
            { { "a", "8" }, { "b", "13" }, { "c", "78" }, {"d", "1"} },
            { { "a", "32" }, { "b", "61" }, { "c", "4214" }, {"d", "1"} },
            { { "a", "16" }, { "b", "29" }, { "c", "2262" }, {"d", "1"} },
            { { "a", "32" }, { "b", "61" }, { "c", "4214" }, {"d", "1"} },
            { { "a", "64" }, { "b", "125" }, { "c", "12214" }, {"d", "29"} },
            { { "a", "128" }, { "b", "253" }, { "c", "44598" }, {"d", "1769"} },
            { { "a", "256" }, { "b", "509" }, { "c", "174902" }, {"d", "221125"} },
            { { "a", "128" }, { "b", "253" }, { "c", "44598" }, {"d", "221125"} },
            { { "a", "16" }, { "b", "29" }, { "c", "2262" } },
            { { "a", "32" }, { "b", "61" }, { "c", "4214" }, {"d", "1"} },
            { { "a", "256" }, { "b", "1277" }, { "c", "176435" } },
            { { "a", "256" }, { "b", "509" }, { "c", "174902" } },
            { { "a", "256" }, { "b", "509" }, { "c", "174902" } },
            { { "a", "256" }, { "b", "1277" }, { "c", "176435" } },
            { { "a", "256" }, { "b", "2301" }, { "c", "178992" } },
            { { "a", "512" }, { "b", "3837" }, { "c", "183341" } },
            { { "a", "1024" }, { "b", "7933" }, { "c", "192298" } },
            { { "a", "512" }, { "b", "3837" }, { "c", "183341" } },
            { { "a", "1024" }, { "b", "7933" }, { "c", "192298" } },
            { { "a", "1024" }, { "b", "7933" }, { "c", "192298" }, {"s", "2"} },
            { { "a", "1024" }, { "b", "7933" }, { "c", "192298" }, {"s", "2"} },
            { { "a", "1024" }, { "b", "7933" }, { "c", "192298" } },
            { { "a", "1024" }, { "b", "7933" }, { "c", "192298" } },
            { { "a", "1024" }, { "b", "7933" }, { "c", "192298" } },
            { { "a", "1024" }, { "b", "7933" }, { "c", "192298" }, {"s", "2"} },
            { { "a", "1024" }, { "b", "7933" }, { "c", "192298" }, {"s", "3"} },
            { { "a", "1024" }, { "b", "7933" }, { "c", "192298" } },
            { { "a", "1024" }, { "b", "7933" }, { "c", "192298" }, {"s", "3"} },
            { { "a", "10" }, { "b", "7933" }, { "c", "192298" }, {"x", "7"} },
            { { "a", "9" }, { "b", "7933" }, { "c", "192298" }, {"x", "7"} },
            { { "a", "8" }, { "b", "7933" }, { "c", "192298" }, {"x", "7"} },
            { { "a", "10" }, { "b", "7933" }, { "c", "192298" }, {"x", "7"} },
            { { "a", "11" }, { "b", "7933" }, { "c", "192298" }, {"x", "7"} },
            { { "a", "12" }, { "b", "7933" }, { "c", "192298" }, {"x", "7"} },
            { { "a", "7" }, { "b", "7933" }, { "c", "192298" }, {"x", "7"} },
            { { "a", "10" }, { "b", "7933" }, { "c", "192298" }, {"x", "7"} },
            { { "a", "19" }, { "b", "2" }, { "c", "3" }, {"d", "1"} },
            { { "a", "19" }, { "b", "2" }, { "c", "3" }, {"d", "1"} },
            { { "a", "19" }, { "b", "2" }, { "c", "3" }, {"d", "1"} },
            { { "a", "20" }, { "b", "3" }, { "c", "4" }, {"d", "3"} },
            { { "a", "19" }, { "b", "2" }, { "c", "3" }, {"d", "2"} },
            { { "a", "20" }, { "b", "3" }, { "c", "4" }, {"d", "3"} },
            { { "a", "23" }, { "b", "6" }, { "c", "7" }, {"d", "7"} },
            { { "a", "20" }, { "b", "3" }, { "c", "4" }, {"d", "4"} },
            { { "a", "23" }, { "b", "6" }, { "c", "7" }, {"d", "7"} },
            { { "a", "29" }, { "b", "12" }, { "c", "13" }, {"d", "14"} },
            { { "a", "39" }, { "b", "22" }, { "c", "23" }, {"d", "25"} },
            { { "a", "29" }, { "b", "12" }, { "c", "13" }, {"d", "15"} },
            { { "a", "39" }, { "b", "22" }, { "c", "23" }, {"d", "25"} },
            { { "a", "54" }, { "b", "37" }, { "c", "38" }, {"d", "41"} },
            { { "a", "54" }, { "b", "37" }, { "c", "38" }, {"d", "42"} },
            { { "a", "19" }, { "b", "2" }, { "c", "3" } },
            { { "a", "54" }, { "b", "37" }, { "c", "38" }, {"d", "42"} },
            { { "a", "76" }, { "b", "60" }, { "c", "62" }, {"d", "1"}, {"e", "77"} },
            { { "a", "75" }, { "b", "58" }, { "c", "59" }, {"d", "1"}, {"e", "1"} },
            { { "a", "75" }, { "b", "58" }, { "c", "59" }, {"d", "1"} },
            { { "a", "75" }, { "b", "58" }, { "c", "59" } },
            { { "a", "76" }, { "b", "60" }, { "c", "62" }, {"d", "1"}, {"e", "77"} },
            { { "a", "81" }, { "b", "65" }, { "c", "67" }, {"d", "5"} },
            { { "a", "76" }, { "b", "60" }, { "c", "62" }, {"d", "5"} },
            { { "a", "75" }, { "b", "58" }, { "c", "59" } },
            { { "a", "81" }, { "b", "65" }, { "c", "67" }, {"d", "5"} },
            { { "a", "87" }, { "b", "71" }, { "c", "73" }, {"d", "6"} },
            { { "a", "81" }, { "b", "65" }, { "c", "67" }, {"d", "6"} },
            { { "a", "87" }, { "b", "71" }, { "c", "73" }, {"d", "6"} },
            { { "a", "88" }, { "b", "73" }, { "c", "76" }, {"d", "7"}, {"e", "84"} },
            { { "a", "87" }, { "b", "71" }, { "c", "73" }, {"d", "7"}, {"e", "1"} },
            { { "a", "87" }, { "b", "71" }, { "c", "73" }, {"d", "7"} },
            { { "a", "75" }, { "b", "58" }, { "c", "59" } },
            { { "a", "88" }, { "b", "73" }, { "c", "76" }, {"d", "7"}, {"e", "84"} },
            { { "a", "88" }, { "b", "73" }, { "c", "76" }, {"d", "11"} },
            { { "a", "75" }, { "b", "58" }, { "c", "59" } },
            { { "a", "88" }, { "b", "73" }, { "c", "76" }, {"d", "11"} },
            { { "a", "6" }, { "b", "13" }, { "c", "3" }, {"d", "11"}, {"e", "11"} },
            { { "a", "1" }, { "b", "2" }, { "c", "3" }, {"d", "10"}, {"e", "1"} },
            { { "a", "1" }, { "b", "2" }, { "c", "3" }, {"d", "10"} },
            { { "a", "1" }, { "b", "2" }, { "c", "3" }, {"d", "10"} },
            { { "a", "1" }, { "b", "2" }, { "c", "3" } },
            { { "a", "6" }, { "b", "13" }, { "c", "3" }, {"d", "11"}, {"e", "11"} },
            { { "a", "7" }, { "b", "14" }, { "c", "14" }, {"d", "11"} },
            { { "a", "1" }, { "b", "2" }, { "c", "3" }, {"d", "10"} },
            { { "a", "1" }, { "b", "2" }, { "c", "3" } },
            { { "a", "7" }, { "b", "14" }, { "c", "14" }, {"d", "11"} },
            { { "a", "8" }, { "b", "16" }, { "c", "18" }, {"d", "11"} },
            { { "a", "7" }, { "b", "14" }, { "c", "15" }, {"d", "11"} },
            { { "a", "1" }, { "b", "2" }, { "c", "3" } },
            { { "a", "8" }, { "b", "16" }, { "c", "18" }, {"d", "11"} },
            { { "a", "8" }, { "b", "16" }, { "c", "18" }, {"d", "19"} },
            { { "a", "1" }, { "b", "2" }, { "c", "3" } },
            { { "a", "8" }, { "b", "16" }, { "c", "18" }, {"d", "19"} }
        };

        int32_t index_ {0};
        const EcmaVM *vm_ {nullptr};
        RuntimeImpl *runtime_ {nullptr};
        size_t checkVariableCounter_ {0};
    };

    enum breakpointOpType {
        RESUME = 1,
        DROPLASTFRAME = 2
    };

    static constexpr size_t LINE_COLUMN = 2;
    static constexpr size_t POINTER_SIZE = 17;

    std::string pandaFile_ = DEBUGGER_ABC_DIR "dropframe.abc";
    std::string sourceFile_ = DEBUGGER_JS_DIR "dropframe.js";
    std::string entryPoint_ = "dropframe";
    size_t breakpointCounter_ = 0;
    size_t dropframeCounter_ = 0;
    bool dropFrameChecked_ = false;
    std::vector<JSPtLocation> pointerLocations_;
    size_t breakpointHitTimes[POINTER_SIZE] = {2, 2, 4, 6, 6, 4, 5, 10, 2, 2, 4, 2, 2, 2, 2, 2, 2};
    std::queue<std::pair<uint8_t, uint8_t>> breakpointOp;

    void InitBreakpointOpQueue()
    {
        breakpointOp.push({1, breakpointOpType::DROPLASTFRAME});
        breakpointOp.push({1, breakpointOpType::DROPLASTFRAME});
        breakpointOp.push({1, breakpointOpType::DROPLASTFRAME});
        breakpointOp.push({3, breakpointOpType::DROPLASTFRAME});
        breakpointOp.push({3, breakpointOpType::DROPLASTFRAME});
        breakpointOp.push({3, breakpointOpType::DROPLASTFRAME});
        breakpointOp.push({5, breakpointOpType::DROPLASTFRAME});
        breakpointOp.push({7, breakpointOpType::DROPLASTFRAME});
        breakpointOp.push({7, breakpointOpType::DROPLASTFRAME});
        breakpointOp.push({9, breakpointOpType::DROPLASTFRAME});
        breakpointOp.push({13, breakpointOpType::DROPLASTFRAME});
        breakpointOp.push({13, breakpointOpType::DROPLASTFRAME});
        breakpointOp.push({15, breakpointOpType::DROPLASTFRAME});
        breakpointOp.push({15, breakpointOpType::DROPLASTFRAME});
        breakpointOp.push({19, breakpointOpType::DROPLASTFRAME});
        breakpointOp.push({21, breakpointOpType::DROPLASTFRAME});
        breakpointOp.push({21, breakpointOpType::DROPLASTFRAME});
        breakpointOp.push({21, breakpointOpType::DROPLASTFRAME});
        breakpointOp.push({21, breakpointOpType::DROPLASTFRAME});
        breakpointOp.push({23, breakpointOpType::DROPLASTFRAME});
        breakpointOp.push({25, breakpointOpType::DROPLASTFRAME});
        breakpointOp.push({25, breakpointOpType::DROPLASTFRAME});
        breakpointOp.push({28, breakpointOpType::DROPLASTFRAME});
        breakpointOp.push({30, breakpointOpType::DROPLASTFRAME});
        breakpointOp.push({32, breakpointOpType::DROPLASTFRAME});
        breakpointOp.push({34, breakpointOpType::DROPLASTFRAME});
        breakpointOp.push({37, breakpointOpType::DROPLASTFRAME});
        breakpointOp.push({40, breakpointOpType::DROPLASTFRAME});
        breakpointOp.push({42, breakpointOpType::DROPLASTFRAME});
        breakpointOp.push({42, breakpointOpType::DROPLASTFRAME});
        breakpointOp.push({42, breakpointOpType::DROPLASTFRAME});
        breakpointOp.push({44, breakpointOpType::DROPLASTFRAME});
        breakpointOp.push({44, breakpointOpType::DROPLASTFRAME});
        breakpointOp.push({46, breakpointOpType::DROPLASTFRAME});
        breakpointOp.push({48, breakpointOpType::DROPLASTFRAME});
        breakpointOp.push({48, breakpointOpType::DROPLASTFRAME});
        breakpointOp.push({48, breakpointOpType::DROPLASTFRAME});
        breakpointOp.push({50, breakpointOpType::DROPLASTFRAME});
        breakpointOp.push({52, breakpointOpType::DROPLASTFRAME});
        breakpointOp.push({52, breakpointOpType::DROPLASTFRAME});
        breakpointOp.push({52, breakpointOpType::DROPLASTFRAME});
        breakpointOp.push({52, breakpointOpType::DROPLASTFRAME});
        breakpointOp.push({54, breakpointOpType::DROPLASTFRAME});
        breakpointOp.push({54, breakpointOpType::DROPLASTFRAME});
        breakpointOp.push({56, breakpointOpType::DROPLASTFRAME});
        breakpointOp.push({56, breakpointOpType::DROPLASTFRAME});
        breakpointOp.push({58, breakpointOpType::DROPLASTFRAME});
    }

    void DropFrameIfChecked()
    {
        while (!dropFrameChecked_ && !breakpointOp.empty() && breakpointOp.front().first == breakpointCounter_) {
            if (breakpointOp.front().second == breakpointOpType::DROPLASTFRAME) {
                PtJson paramsJson;
                paramsJson.Add("droppedDepth", 1U);
                std::unique_ptr<DropFrameParams> params = DropFrameParams::Create(paramsJson);
                debugger_->DropFrame(*params);
                dropFrameChecked_ = true;
            }
            breakpointOp.pop();
        }
    }

    void SetJSPtLocation(size_t *arr, size_t number, std::vector<JSPtLocation> &locations)
    {
        for (size_t i = 0; i < number; i++) {
            JSPtLocation location = TestUtil::GetLocation(sourceFile_.c_str(), arr[i * LINE_COLUMN],
                                                          arr[i * LINE_COLUMN + 1], pandaFile_.c_str());
            locations.push_back(location);
        }
    };
};

std::unique_ptr<TestEvents> GetJsDropFrameTest()
{
    return std::make_unique<JsDropFrameTest>();
}
}  // namespace panda::ecmascript::tooling::test

#endif  // ECMASCRIPT_TOOLING_TEST_UTILS_TESTCASES_JS_DROP_FRAME_TEST_H
