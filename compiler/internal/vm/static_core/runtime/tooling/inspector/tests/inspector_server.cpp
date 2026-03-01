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

#include "inspector_server.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "assembly-emitter.h"
#include "assembly-parser.h"
#include "runtime.h"
#include "types/location.h"
#include "libarkbase/utils/json_builder.h"

#include "connection/server.h"

#include "common.h"
#include "json_object_matcher.h"

// NOLINTBEGIN

using namespace std::placeholders;

namespace ark::tooling::inspector::test {

class TestServer : public Server {
public:
    void OnValidate([[maybe_unused]] std::function<void()> &&handler) override {};
    void OnOpen([[maybe_unused]] std::function<void()> &&handler) override {};
    void OnFail([[maybe_unused]] std::function<void()> &&handler) override {};

    void Call(const std::string &session, const char *method_call,
              std::function<void(JsonObjectBuilder &)> &&parameters) override
    {
        std::string tmp(method_call);
        CallMock(session, tmp, std::move(parameters));
    }

    MOCK_METHOD(void, CallMock,
                (const std::string &session, const std::string &method_call,
                 std::function<void(JsonObjectBuilder &)> &&parameters));

    MOCK_METHOD(void, OnCallMock, (const std::string &method_call, Handler &&handler));

    bool RunOne() override
    {
        return true;
    };

private:
    void OnCallImpl(const char *method_call, Handler &&handler) override
    {
        std::string tmp(method_call);
        OnCallMock(tmp, std::move(handler));
    }
};

static PtThread g_mthread = PtThread(PtThread::NONE);
static const std::string g_sessionId;
static const std::string g_sourceFile = "source";
static bool g_handlerCalled = false;

class ServerTest : public testing::Test {
public:
    void SetUp() override
    {
        RuntimeOptions options;
        options.SetShouldInitializeIntrinsics(false);
        options.SetShouldLoadBootPandaFiles(false);
        Runtime::Create(options);
        g_mthread = PtThread(ManagedThread::GetCurrent());
        g_handlerCalled = false;
    }
    void TearDown() override
    {
        Runtime::Destroy();
    }
    TestServer server;
    InspectorServer inspectorServer {server};
};

TEST_F(ServerTest, CallDebuggerResumed)
{
    inspectorServer.CallTargetAttachedToTarget(g_mthread);
    EXPECT_CALL(server, CallMock(g_sessionId, "Debugger.resumed", testing::_)).Times(1);

    inspectorServer.CallDebuggerResumed(g_mthread);
}

TEST_F(ServerTest, CallDebuggerScriptParsed)
{
    inspectorServer.CallTargetAttachedToTarget(g_mthread);

    size_t scriptId = 4;
    EXPECT_CALL(server, CallMock(g_sessionId, "Debugger.scriptParsed", testing::_))
        .WillOnce([&](testing::Unused, testing::Unused, auto s) {
            ASSERT_THAT(ToObject(std::move(s)),
                        JsonProperties(JsonProperty<JsonObject::NumT> {"executionContextId", 0},
                                       JsonProperty<JsonObject::StringT> {"scriptId", std::to_string(scriptId)},
                                       JsonProperty<JsonObject::NumT> {"startLine", 0},
                                       JsonProperty<JsonObject::NumT> {"startColumn", 0},
                                       JsonProperty<JsonObject::NumT> {"endLine", std::numeric_limits<int>::max()},
                                       JsonProperty<JsonObject::NumT> {"endColumn", std::numeric_limits<int>::max()},
                                       JsonProperty<JsonObject::StringT> {"hash", ""},
                                       JsonProperty<JsonObject::StringT> {"url", g_sourceFile.c_str()}));
        });
    inspectorServer.CallDebuggerScriptParsed(ScriptId(scriptId), g_sourceFile);
}

using ResultHolder = std::optional<JsonObject>;

static void GetResult(Expected<std::unique_ptr<JsonSerializable>, JRPCError> &&returned, ResultHolder &result)
{
    ASSERT_TRUE(returned.HasValue());
    if (returned.Value() != nullptr) {
        JsonObjectBuilder builder;
        returned.Value()->Serialize(builder);
        result.emplace(std::move(builder).Build());
    } else {
        result.emplace("{}");
    }
}

TEST_F(ServerTest, DebuggerEnable)
{
    TestServer server1;
    EXPECT_CALL(server1, OnCallMock("Target.attachToTarget", testing::_)).WillOnce([&](testing::Unused, auto handler) {
        JsonObjectBuilder params;
        params.AddProperty("targetId", g_sessionId);
        auto res = handler(g_sessionId, JsonObject(std::move(params).Build()));
        ASSERT_FALSE(res.HasValue());
    });
    EXPECT_CALL(server1, OnCallMock("Debugger.enable", testing::_)).WillOnce([&](testing::Unused, auto handler) {
        JsonObject empty;
        auto res = handler(g_sessionId, empty);
        ResultHolder result;
        GetResult(std::move(res), result);
        std::vector<testing::Matcher<JsonObject::JsonObjPointer>> protocols;
        ASSERT_THAT(*result,
                    JsonProperties(JsonProperty<JsonObject::NumT> {"debuggerId", 0},
                                   JsonProperty<JsonObject::ArrayT> {"protocols", JsonElementsAreArray(protocols)}));
    });
    InspectorServer inspectorServer1(server1);
    inspectorServer1.OnCallDebuggerEnable([] {});
}

TEST_F(ServerTest, OnCallAttachToTarget)
{
    inspectorServer.CallTargetAttachedToTarget(g_mthread);

    EXPECT_CALL(server, OnCallMock("Target.attachToTarget", testing::_)).WillOnce([&](testing::Unused, auto handler) {
        JsonObjectBuilder params;
        params.AddProperty("targetId", g_sessionId);
        auto res = handler(g_sessionId, JsonObject(std::move(params).Build()));
        ResultHolder result;
        GetResult(std::move(res), result);
        ASSERT_TRUE(result);
        ASSERT_THAT(*result, JsonProperties(JsonProperty<JsonObject::StringT> {"sessionId", g_sessionId}));
    });
    inspectorServer.OnCallTargetAttachToTarget();
}

static auto g_simpleHandler = []([[maybe_unused]] auto unused, auto handler) {
    JsonObject empty;
    auto res = handler(g_sessionId, empty);
    ResultHolder result;
    GetResult(std::move(res), result);
    ASSERT_THAT(*result, JsonProperties());
    ASSERT_TRUE(g_handlerCalled);
};

TEST_F(ServerTest, OnCallDebuggerPause)
{
    inspectorServer.CallTargetAttachedToTarget(g_mthread);

    EXPECT_CALL(server, OnCallMock("Debugger.pause", testing::_)).WillOnce(g_simpleHandler);
    inspectorServer.OnCallDebuggerPause([](PtThread thread) {
        ASSERT_EQ(thread.GetId(), g_mthread.GetId());
        g_handlerCalled = true;
    });
}

TEST_F(ServerTest, OnCallDebuggerRemoveBreakpoint)
{
    size_t breakId = 14;

    inspectorServer.CallTargetAttachedToTarget(g_mthread);

    EXPECT_CALL(server, OnCallMock("Debugger.removeBreakpoint", testing::_))
        .WillOnce([&](testing::Unused, auto handler) {
            JsonObject empty;
            auto res = handler(g_sessionId, empty);
            ASSERT_FALSE(res.HasValue());
            ASSERT_FALSE(g_handlerCalled);
        });

    auto breaks = [breakId](PtThread thread, BreakpointId bid) {
        ASSERT_EQ(bid, BreakpointId(breakId));
        ASSERT_EQ(thread.GetId(), g_mthread.GetId());
        g_handlerCalled = true;
    };
    inspectorServer.OnCallDebuggerRemoveBreakpoint(std::move(breaks));

    EXPECT_CALL(server, OnCallMock("Debugger.removeBreakpoint", testing::_))
        .WillOnce([&](testing::Unused, auto handler) {
            JsonObjectBuilder params;
            params.AddProperty("breakpointId", std::to_string(breakId));
            auto res = handler(g_sessionId, JsonObject(std::move(params).Build()));
            ResultHolder result;
            GetResult(std::move(res), result);
            ASSERT_THAT(*result, JsonProperties());
            ASSERT_TRUE(g_handlerCalled);
        });

    inspectorServer.OnCallDebuggerRemoveBreakpoint(std::move(breaks));
}

TEST_F(ServerTest, OnCallDebuggerRestartFrame)
{
    size_t fid = 5;

    inspectorServer.CallTargetAttachedToTarget(g_mthread);

    EXPECT_CALL(server, OnCallMock("Debugger.restartFrame", testing::_)).WillOnce([&](testing::Unused, auto handler) {
        std::vector<testing::Matcher<JsonObject::JsonObjPointer>> callFrames;
        JsonObjectBuilder params;
        auto res = handler(g_sessionId, JsonObject(std::move(params).Build()));
        ASSERT_FALSE(res.HasValue());
        ASSERT_FALSE(g_handlerCalled);
    });

    inspectorServer.OnCallDebuggerRestartFrame([&](auto, auto) { g_handlerCalled = true; });

    EXPECT_CALL(server, OnCallMock("Debugger.restartFrame", testing::_)).WillOnce([&](testing::Unused, auto handler) {
        std::vector<testing::Matcher<JsonObject::JsonObjPointer>> callFrames;
        JsonObjectBuilder params;
        params.AddProperty("callFrameId", std::to_string(fid));
        auto res = handler(g_sessionId, JsonObject(std::move(params).Build()));
        ResultHolder result;
        GetResult(std::move(res), result);
        ASSERT_THAT(*result,
                    JsonProperties(JsonProperty<JsonObject::ArrayT> {"callFrames", JsonElementsAreArray(callFrames)}));
        ASSERT_TRUE(g_handlerCalled);
    });

    inspectorServer.OnCallDebuggerRestartFrame([&](PtThread thread, FrameId id) {
        ASSERT_EQ(id, FrameId(fid));
        ASSERT_EQ(thread.GetId(), g_mthread.GetId());
        g_handlerCalled = true;
    });
}

TEST_F(ServerTest, OnCallDebuggerResume)
{
    inspectorServer.CallTargetAttachedToTarget(g_mthread);

    EXPECT_CALL(server, OnCallMock("Debugger.resume", testing::_)).WillOnce(g_simpleHandler);
    inspectorServer.OnCallDebuggerResume([](PtThread thread) {
        ASSERT_EQ(thread.GetId(), g_mthread.GetId());
        g_handlerCalled = true;
    });
}

static JsonObject CreatePossibleBreakpointsRequest(ScriptId startScriptId, size_t start, ScriptId endScriptId,
                                                   size_t end, bool restrictToFunction)
{
    JsonObjectBuilder params;
    params.AddProperty("start", Location(startScriptId, start));
    params.AddProperty("end", Location(endScriptId, end));
    params.AddProperty("restrictToFunction", restrictToFunction);
    return JsonObject(std::move(params).Build());
}

static auto g_getPossibleBreakpointsHandler = [](ScriptId scriptId, size_t start, size_t end, bool restrictToFunction,
                                                 testing::Unused, auto handler) {
    auto res =
        handler(g_sessionId, CreatePossibleBreakpointsRequest(scriptId, start, scriptId, end, restrictToFunction));
    ResultHolder result;
    GetResult(std::move(res), result);
    std::vector<testing::Matcher<JsonObject::JsonObjPointer>> locations;
    for (auto i = start; i < end; i++) {
        locations.push_back(
            testing::Pointee(JsonProperties(JsonProperty<JsonObject::StringT> {"scriptId", std::to_string(scriptId)},
                                            JsonProperty<JsonObject::NumT> {"lineNumber", i})));
    }
    ASSERT_THAT(*result,
                JsonProperties(JsonProperty<JsonObject::ArrayT> {"locations", JsonElementsAreArray(locations)}));
};

static void DefaultFrameEnumerator(const InspectorServer::FrameInfoHandler &handler)
{
    std::optional<RemoteObject> objThis;
    auto scope_chain = std::vector {Scope(Scope::Type::LOCAL, RemoteObject::Number(72))};
    handler(FrameId(0), std::to_string(0), g_sourceFile, 0, scope_chain, objThis);
}

TEST_F(ServerTest, OnCallDebuggerGetPossibleBreakpoints)
{
    auto scriptId = 0;
    size_t start = 5;
    size_t end = 5;

    inspectorServer.CallTargetAttachedToTarget(g_mthread);
    inspectorServer.CallDebuggerPaused(g_mthread, {}, {}, PauseReason::OTHER, DefaultFrameEnumerator);

    EXPECT_CALL(server, OnCallMock("Debugger.getPossibleBreakpoints", testing::_))
        .WillOnce(std::bind(g_getPossibleBreakpointsHandler, scriptId, start, end,  // NOLINT(modernize-avoid-bind)
                            true, _1, _2));
    auto getLinesTrue = [](std::string_view source, size_t startLine, size_t endLine, bool restrictToFunction) {
        std::set<size_t> result;
        if ((source == g_sourceFile) && restrictToFunction) {
            for (auto i = startLine; i < endLine; i++) {
                result.insert(i);
            }
        }
        return result;
    };
    inspectorServer.OnCallDebuggerGetPossibleBreakpoints(getLinesTrue);

    EXPECT_CALL(server, OnCallMock("Debugger.getPossibleBreakpoints", testing::_))
        .WillOnce(std::bind(g_getPossibleBreakpointsHandler, scriptId, start, end,  // NOLINT(modernize-avoid-bind)
                            false, _1, _2));
    auto getLinesFalse = [](std::string_view source, size_t startLine, size_t endLine, bool restrictToFunction) {
        std::set<size_t> result;
        if ((source == g_sourceFile) && !restrictToFunction) {
            for (auto i = startLine; i < endLine; i++) {
                result.insert(i);
            }
        }
        return result;
    };
    inspectorServer.OnCallDebuggerGetPossibleBreakpoints(getLinesFalse);

    EXPECT_CALL(server, OnCallMock("Debugger.getPossibleBreakpoints", testing::_))
        .WillOnce([&](testing::Unused, auto handler) {
            auto res =
                handler(g_sessionId, CreatePossibleBreakpointsRequest(scriptId, start, scriptId + 1, end, false));
            ASSERT_FALSE(res.HasValue());
        });
    inspectorServer.OnCallDebuggerGetPossibleBreakpoints(getLinesFalse);
}

TEST_F(ServerTest, OnCallDebuggerGetScriptSource)
{
    auto scriptId = 0;

    EXPECT_CALL(server, CallMock(g_sessionId, "Debugger.paused", testing::_))
        .WillOnce([&](testing::Unused, testing::Unused, auto s) { ToObject(std::move(s)); });

    inspectorServer.CallTargetAttachedToTarget(g_mthread);
    inspectorServer.CallDebuggerPaused(g_mthread, {}, {}, PauseReason::OTHER, DefaultFrameEnumerator);

    EXPECT_CALL(server, OnCallMock("Debugger.getScriptSource", testing::_))
        .WillOnce([&](testing::Unused, auto handler) {
            JsonObjectBuilder params;
            params.AddProperty("scriptId", std::to_string(scriptId));
            auto res = handler(g_sessionId, JsonObject(std::move(params).Build()));
            ResultHolder result;
            GetResult(std::move(res), result);
            ASSERT_THAT(*result, JsonProperties(JsonProperty<JsonObject::StringT> {"scriptSource", g_sourceFile}));
        });
    inspectorServer.OnCallDebuggerGetScriptSource([](auto source) {
        std::string s(source);
        return s;
    });

    EXPECT_CALL(server, OnCallMock("Debugger.getScriptSource", testing::_))
        .WillOnce([&](testing::Unused, auto handler) {
            JsonObject empty;
            auto res = handler(g_sessionId, empty);
            ASSERT_FALSE(res.HasValue());
        });
    inspectorServer.OnCallDebuggerGetScriptSource([](auto) { return "a"; });
}

TEST_F(ServerTest, OnCallDebuggerStepOut)
{
    inspectorServer.CallTargetAttachedToTarget(g_mthread);

    EXPECT_CALL(server, OnCallMock("Debugger.stepOut", testing::_)).WillOnce(g_simpleHandler);

    inspectorServer.OnCallDebuggerStepOut([](PtThread thread) {
        ASSERT_EQ(thread.GetId(), g_mthread.GetId());
        g_handlerCalled = true;
    });
}

TEST_F(ServerTest, OnCallDebuggerStepInto)
{
    inspectorServer.CallTargetAttachedToTarget(g_mthread);

    EXPECT_CALL(server, OnCallMock("Debugger.stepInto", testing::_)).WillOnce(g_simpleHandler);

    inspectorServer.OnCallDebuggerStepInto([](PtThread thread) {
        ASSERT_EQ(thread.GetId(), g_mthread.GetId());
        g_handlerCalled = true;
    });
}

TEST_F(ServerTest, OnCallDebuggerStepOver)
{
    inspectorServer.CallTargetAttachedToTarget(g_mthread);

    EXPECT_CALL(server, OnCallMock("Debugger.stepOver", testing::_)).WillOnce(g_simpleHandler);

    inspectorServer.OnCallDebuggerStepOver([](PtThread thread) {
        ASSERT_EQ(thread.GetId(), g_mthread.GetId());
        g_handlerCalled = true;
    });
}

std::optional<BreakpointId> handlerForSetBreak([[maybe_unused]] PtThread thread,
                                               [[maybe_unused]] const std::function<bool(std::string_view)> &comp,
                                               size_t line, [[maybe_unused]] std::set<std::string_view> &sources,
                                               [[maybe_unused]] const std::string *condition)
{
    sources.insert("source");
    return BreakpointId(line);
}

std::optional<BreakpointId> handlerForSetBreakEmpty([[maybe_unused]] PtThread thread,
                                                    [[maybe_unused]] const std::function<bool(std::string_view)> &comp,
                                                    [[maybe_unused]] size_t line,
                                                    [[maybe_unused]] std::set<std::string_view> &sources,
                                                    [[maybe_unused]] const std::string *condition)
{
    sources.insert("source");
    return {};
}

TEST_F(ServerTest, OnCallDebuggerSetBreakpoint)
{
    auto scriptId = 0;
    size_t start = 5;

    inspectorServer.CallTargetAttachedToTarget(g_mthread);
    inspectorServer.CallDebuggerPaused(g_mthread, {}, {}, PauseReason::OTHER, DefaultFrameEnumerator);

    EXPECT_CALL(server, OnCallMock("Debugger.setBreakpoint", testing::_)).WillOnce([&](testing::Unused, auto handler) {
        JsonObjectBuilder params;
        params.AddProperty("location", Location(scriptId, start));
        auto res = handler(g_sessionId, JsonObject(std::move(params).Build()));
        ResultHolder result;
        GetResult(std::move(res), result);
        ASSERT_THAT(*result,
                    JsonProperties(JsonProperty<JsonObject::StringT> {"breakpointId", std::to_string(start)},
                                   JsonProperty<JsonObject::JsonObjPointer> {
                                       "actualLocation",
                                       testing::Pointee(JsonProperties(
                                           JsonProperty<JsonObject::StringT> {"scriptId", std::to_string(scriptId)},
                                           JsonProperty<JsonObject::NumT> {"lineNumber", start - 1}))}));
    });

    inspectorServer.OnCallDebuggerSetBreakpoint(handlerForSetBreak);

    EXPECT_CALL(server, OnCallMock("Debugger.setBreakpoint", testing::_)).WillOnce([&](testing::Unused, auto handler) {
        JsonObject empty;
        auto res = handler(g_sessionId, empty);
        ASSERT_FALSE(res.HasValue());
    });

    inspectorServer.OnCallDebuggerSetBreakpoint(handlerForSetBreak);

    EXPECT_CALL(server, OnCallMock("Debugger.setBreakpoint", testing::_)).WillOnce([&](testing::Unused, auto handler) {
        JsonObjectBuilder params;
        params.AddProperty("location", Location(scriptId, start));
        auto res = handler(g_sessionId, JsonObject(std::move(params).Build()));
        ASSERT_FALSE(res.HasValue());
    });

    inspectorServer.OnCallDebuggerSetBreakpoint(handlerForSetBreakEmpty);
}

TEST_F(ServerTest, OnCallDebuggerSetBreakpointByUrl)
{
    auto scriptId = 0;
    size_t start = 5;

    inspectorServer.CallTargetAttachedToTarget(g_mthread);

    EXPECT_CALL(server, OnCallMock("Debugger.setBreakpointByUrl", testing::_))
        .WillOnce([&](testing::Unused, auto handler) {
            JsonObjectBuilder params;
            params.AddProperty("lineNumber", start);
            auto res = handler(g_sessionId, JsonObject(std::move(params).Build()));
            ASSERT_FALSE(res.HasValue());
        });

    inspectorServer.OnCallDebuggerSetBreakpointByUrl(handlerForSetBreak);

    EXPECT_CALL(server, OnCallMock("Debugger.setBreakpointByUrl", testing::_))
        .WillOnce([&](testing::Unused, auto handler) {
            JsonObjectBuilder params;
            params.AddProperty("lineNumber", start);
            params.AddProperty("url", "file://source");
            auto res = handler(g_sessionId, JsonObject(std::move(params).Build()));
            ResultHolder result;
            GetResult(std::move(res), result);

            std::vector<testing::Matcher<JsonObject::JsonObjPointer>> locations;
            locations.push_back(testing::Pointee(
                JsonProperties(JsonProperty<JsonObject::StringT> {"scriptId", std::to_string(scriptId)},
                               JsonProperty<JsonObject::NumT> {"lineNumber", start})));
            auto expected =
                JsonProperties(JsonProperty<JsonObject::StringT> {"breakpointId", std::to_string(start + 1)},
                               JsonProperty<JsonObject::ArrayT> {"locations", JsonElementsAreArray(locations)});

            ASSERT_THAT(*result, expected);
        });

    inspectorServer.OnCallDebuggerSetBreakpointByUrl(handlerForSetBreak);
}

TEST_F(ServerTest, OnCallDebuggerSetBreakpointsActive)
{
    inspectorServer.CallTargetAttachedToTarget(g_mthread);

    EXPECT_CALL(server, OnCallMock("Debugger.setBreakpointsActive", testing::_))
        .WillOnce([&](testing::Unused, auto handler) {
            JsonObject empty;
            auto res = handler(g_sessionId, empty);
            ASSERT_FALSE(res.HasValue());
            ASSERT_FALSE(g_handlerCalled);
        });

    inspectorServer.OnCallDebuggerSetBreakpointsActive([](auto thread, auto value) {
        ASSERT_EQ(thread.GetId(), g_mthread.GetId());
        ASSERT_FALSE(value);
        g_handlerCalled = true;
    });

    EXPECT_CALL(server, OnCallMock("Debugger.setBreakpointsActive", testing::_))
        .WillOnce([&](testing::Unused, auto handler) {
            JsonObjectBuilder params;
            params.AddProperty("active", true);
            auto res = handler(g_sessionId, JsonObject(std::move(params).Build()));
            ResultHolder result;
            GetResult(std::move(res), result);
            ASSERT_THAT(*result, JsonProperties());
            ASSERT_TRUE(g_handlerCalled);
            g_handlerCalled = false;
        });

    inspectorServer.OnCallDebuggerSetBreakpointsActive([](auto thread, auto value) {
        ASSERT_EQ(thread.GetId(), g_mthread.GetId());
        ASSERT_TRUE(value);
        g_handlerCalled = true;
    });

    EXPECT_CALL(server, OnCallMock("Debugger.setBreakpointsActive", testing::_))
        .WillOnce([&](testing::Unused, auto handler) {
            JsonObjectBuilder params;
            params.AddProperty("active", false);
            auto res = handler(g_sessionId, JsonObject(std::move(params).Build()));
            ResultHolder result;
            GetResult(std::move(res), result);
            ASSERT_THAT(*result, JsonProperties());
            ASSERT_TRUE(g_handlerCalled);
        });

    inspectorServer.OnCallDebuggerSetBreakpointsActive([](auto thread, auto value) {
        ASSERT_EQ(thread.GetId(), g_mthread.GetId());
        ASSERT_FALSE(value);
        g_handlerCalled = true;
    });
}

TEST_F(ServerTest, OnCallDebuggerSetPauseOnExceptions)
{
    inspectorServer.CallTargetAttachedToTarget(g_mthread);

    EXPECT_CALL(server, OnCallMock("Debugger.setPauseOnExceptions", testing::_))
        .WillOnce([&](testing::Unused, auto handler) {
            JsonObjectBuilder params;
            params.AddProperty("state", "none");
            auto res = handler(g_sessionId, JsonObject(std::move(params).Build()));
            ResultHolder result;
            GetResult(std::move(res), result);
            ASSERT_THAT(*result, JsonProperties());
            ASSERT_TRUE(g_handlerCalled);
        });

    inspectorServer.OnCallDebuggerSetPauseOnExceptions([](PtThread thread, PauseOnExceptionsState state) {
        ASSERT_EQ(thread.GetId(), g_mthread.GetId());
        ASSERT_EQ(PauseOnExceptionsState::NONE, state);
        g_handlerCalled = true;
    });
}

TEST_F(ServerTest, OnCallDebuggerDisable)
{
    inspectorServer.CallTargetAttachedToTarget(g_mthread);

    EXPECT_CALL(server, OnCallMock("Debugger.disable", testing::_)).WillOnce(g_simpleHandler);

    inspectorServer.OnCallDebuggerDisable([](PtThread thread) {
        ASSERT_EQ(thread.GetId(), g_mthread.GetId());
        g_handlerCalled = true;
    });
}

TEST_F(ServerTest, OnCallDebuggerClientDisconnect)
{
    inspectorServer.CallTargetAttachedToTarget(g_mthread);

    EXPECT_CALL(server, OnCallMock("Debugger.clientDisconnect", testing::_)).WillOnce(g_simpleHandler);

    inspectorServer.OnCallDebuggerClientDisconnect([](PtThread thread) {
        ASSERT_EQ(thread.GetId(), g_mthread.GetId());
        g_handlerCalled = true;
    });
}

TEST_F(ServerTest, OnCallDebuggerSetAsyncCallStackDepth)
{
    inspectorServer.CallTargetAttachedToTarget(g_mthread);

    EXPECT_CALL(server, OnCallMock("Debugger.setAsyncCallStackDepth", testing::_)).WillOnce(g_simpleHandler);

    inspectorServer.OnCallDebuggerSetAsyncCallStackDepth([](PtThread thread) {
        ASSERT_EQ(thread.GetId(), g_mthread.GetId());
        g_handlerCalled = true;
    });
}

TEST_F(ServerTest, OnCallDebuggerSetBlackboxPatterns)
{
    inspectorServer.CallTargetAttachedToTarget(g_mthread);

    EXPECT_CALL(server, OnCallMock("Debugger.setBlackboxPatterns", testing::_)).WillOnce(g_simpleHandler);

    inspectorServer.OnCallDebuggerSetBlackboxPatterns([](PtThread thread) {
        ASSERT_EQ(thread.GetId(), g_mthread.GetId());
        g_handlerCalled = true;
    });
}

TEST_F(ServerTest, OnCallDebuggerSmartStepInto)
{
    inspectorServer.CallTargetAttachedToTarget(g_mthread);

    EXPECT_CALL(server, OnCallMock("Debugger.smartStepInto", testing::_)).WillOnce(g_simpleHandler);

    inspectorServer.OnCallDebuggerSmartStepInto([](PtThread thread) {
        ASSERT_EQ(thread.GetId(), g_mthread.GetId());
        g_handlerCalled = true;
    });
}

TEST_F(ServerTest, OnCallDebuggerDropFrame)
{
    inspectorServer.CallTargetAttachedToTarget(g_mthread);

    EXPECT_CALL(server, OnCallMock("Debugger.dropFrame", testing::_)).WillOnce(g_simpleHandler);

    inspectorServer.OnCallDebuggerDropFrame([](PtThread thread) {
        ASSERT_EQ(thread.GetId(), g_mthread.GetId());
        g_handlerCalled = true;
    });
}

TEST_F(ServerTest, OnCallDebuggerSetNativeRange)
{
    inspectorServer.CallTargetAttachedToTarget(g_mthread);

    EXPECT_CALL(server, OnCallMock("Debugger.setNativeRange", testing::_)).WillOnce(g_simpleHandler);

    inspectorServer.OnCallDebuggerSetNativeRange([](PtThread thread) {
        ASSERT_EQ(thread.GetId(), g_mthread.GetId());
        g_handlerCalled = true;
    });
}

TEST_F(ServerTest, OnCallDebuggerReplyNativeCalling)
{
    inspectorServer.CallTargetAttachedToTarget(g_mthread);

    EXPECT_CALL(server, OnCallMock("Debugger.replyNativeCalling", testing::_)).WillOnce(g_simpleHandler);

    inspectorServer.OnCallDebuggerReplyNativeCalling([](PtThread thread) {
        ASSERT_EQ(thread.GetId(), g_mthread.GetId());
        g_handlerCalled = true;
    });
}

TEST_F(ServerTest, OnCallDebuggerContinueToLocation)
{
    auto scriptId = 0;
    size_t start = 5;

    inspectorServer.CallTargetAttachedToTarget(g_mthread);

    EXPECT_CALL(server, OnCallMock("Debugger.continueToLocation", testing::_))
        .WillOnce([&](testing::Unused, auto handler) {
            JsonObjectBuilder params;
            params.AddProperty("location", Location(scriptId, start));
            handler(g_sessionId, JsonObject(std::move(params).Build()));
        });

    inspectorServer.OnCallDebuggerContinueToLocation([](PtThread thread, std::string_view, size_t) {
        ASSERT_EQ(thread.GetId(), g_mthread.GetId());
        g_handlerCalled = true;
    });
}

TEST_F(ServerTest, OnCallDebuggerSetSkipAllPauses)
{
    inspectorServer.CallTargetAttachedToTarget(g_mthread);

    EXPECT_CALL(server, OnCallMock("Debugger.setSkipAllPauses", testing::_))
        .WillOnce([&](testing::Unused, auto handler) {
            JsonObjectBuilder params;
            params.AddProperty("skip", true);
            handler(g_sessionId, JsonObject(std::move(params).Build()));
        });

    inspectorServer.OnCallDebuggerSetSkipAllPauses([](PtThread thread, bool) {
        ASSERT_EQ(thread.GetId(), g_mthread.GetId());
        g_handlerCalled = true;
    });
}

Expected<EvaluationResult, std::string> handlerForEvaluateFailed([[maybe_unused]] PtThread thread,
                                                                 [[maybe_unused]] const std::string &bytecodeBase64,
                                                                 [[maybe_unused]] size_t frameNumber)
{
    return Unexpected(std::string("evaluate failed"));
}

Expected<EvaluationResult, std::string> handlerForEvaluate([[maybe_unused]] PtThread thread,
                                                           [[maybe_unused]] const std::string &bytecodeBase64,
                                                           [[maybe_unused]] size_t frameNumber)
{
    return Unexpected(std::string("evaluation failed"));
}

TEST_F(ServerTest, OnCallDebuggerEvaluateOnCallFrame)
{
    inspectorServer.CallTargetAttachedToTarget(g_mthread);

    EXPECT_CALL(server, OnCallMock("Debugger.evaluateOnCallFrame", testing::_))
        .WillOnce([&](testing::Unused, auto handler) {
            JsonObjectBuilder params;
            params.AddProperty("callFrameId", -1);
            params.AddProperty("expression", "any expression");

            auto res = handler(g_sessionId, JsonObject(std::move(params).Build()));
            ASSERT_FALSE(res.HasValue());
        });

    inspectorServer.OnCallDebuggerEvaluateOnCallFrame(handlerForEvaluateFailed);
}

TEST_F(ServerTest, OnCallDebuggerCallFunctionOn)
{
    inspectorServer.CallTargetAttachedToTarget(g_mthread);

    EXPECT_CALL(server, OnCallMock("Debugger.callFunctionOn", testing::_)).WillOnce([&](testing::Unused, auto handler) {
        JsonObjectBuilder params;
        params.AddProperty("callFrameId", -1);
        params.AddProperty("functionDeclaration", "any functionDeclaration");

        auto res = handler(g_sessionId, JsonObject(std::move(params).Build()));
        ASSERT_FALSE(res.HasValue());
    });

    inspectorServer.OnCallDebuggerCallFunctionOn(handlerForEvaluateFailed);
}

TEST_F(ServerTest, OnCallDebuggerGetPossibleAndSetBreakpointByUrl)
{
    auto scriptId = 0;
    size_t start1 = 5;
    size_t start2 = 6;

    inspectorServer.CallTargetAttachedToTarget(g_mthread);

    EXPECT_CALL(server, OnCallMock("Debugger.getPossibleAndSetBreakpointByUrl", testing::_))
        .WillOnce([&](testing::Unused, auto handler) {
            class RequestLocation : public JsonSerializable {
            public:
                explicit RequestLocation(std::string url, size_t lineNumber) : url_(url), lineNumber_(lineNumber) {}

                void Serialize(JsonObjectBuilder &builder) const override
                {
                    builder.AddProperty("url", url_);
                    builder.AddProperty("lineNumber", lineNumber_);
                }

            private:
                std::string url_;
                size_t lineNumber_;
            };
            std::vector<RequestLocation> requestLocations = {RequestLocation("file://source", start1),
                                                             RequestLocation("file://source", start2)};
            JsonObjectBuilder builder;
            builder.AddProperty("locations", [&](JsonArrayBuilder &locations) {
                for (const auto &loc : requestLocations) {
                    locations.Add(loc);
                }
            });

            auto res = handler(g_sessionId, JsonObject(std::move(builder).Build()));
            ResultHolder result;
            GetResult(std::move(res), result);

            std::vector<testing::Matcher<JsonObject::JsonObjPointer>> locations;
            locations.push_back(testing::Pointee(JsonProperties(JsonProperty<JsonObject::NumT> {"scriptId", scriptId},
                                                                JsonProperty<JsonObject::NumT> {"lineNumber", start1},
                                                                JsonProperty<JsonObject::NumT> {"columnNumber", 0},
                                                                JsonProperty<JsonObject::StringT> {"id", "6"})));
            locations.push_back(testing::Pointee(JsonProperties(JsonProperty<JsonObject::NumT> {"scriptId", scriptId},
                                                                JsonProperty<JsonObject::NumT> {"lineNumber", start2},
                                                                JsonProperty<JsonObject::NumT> {"columnNumber", 0},
                                                                JsonProperty<JsonObject::StringT> {"id", "7"})));
            auto expected =
                JsonProperties(JsonProperty<JsonObject::ArrayT> {"locations", JsonElementsAreArray(locations)});

            ASSERT_THAT(*result, expected);
        });

    inspectorServer.OnCallDebuggerGetPossibleAndSetBreakpointByUrl(handlerForSetBreak);
}

TEST_F(ServerTest, OnCallRuntimeEnable)
{
    inspectorServer.CallTargetAttachedToTarget(g_mthread);

    EXPECT_CALL(server, OnCallMock("Runtime.enable", testing::_)).WillOnce([&](testing::Unused, auto handler) {
        JsonObject empty;
        auto res = handler(g_sessionId, empty);
        ResultHolder result;
        GetResult(std::move(res), result);
        ASSERT_THAT(*result, JsonProperties());
        ASSERT_TRUE(g_handlerCalled);
    });
    inspectorServer.OnCallRuntimeEnable([](PtThread thread) {
        ASSERT_EQ(thread.GetId(), g_mthread.GetId());
        g_handlerCalled = true;
    });
}

TEST_F(ServerTest, OnCallRuntimeGetProperties)
{
    auto object_id = 6;
    auto preview = true;

    inspectorServer.CallTargetAttachedToTarget(g_mthread);

    EXPECT_CALL(server, OnCallMock("Runtime.getProperties", testing::_)).WillOnce([&](testing::Unused, auto handler) {
        JsonObjectBuilder params;
        params.AddProperty("objectId", std::to_string(object_id));
        params.AddProperty("generatePreview", preview);
        auto res = handler(g_sessionId, JsonObject(std::move(params).Build()));
        ResultHolder result;
        GetResult(std::move(res), result);

        std::vector<testing::Matcher<JsonObject::JsonObjPointer>> expected;
        expected.push_back(testing::Pointee(JsonProperties(
            JsonProperty<JsonObject::StringT> {"name", "object"},
            JsonProperty<JsonObject::JsonObjPointer> {
                "value", testing::Pointee(JsonProperties(JsonProperty<JsonObject::NumT> {"value", object_id},
                                                         JsonProperty<JsonObject::StringT> {"type", "number"}))},
            JsonProperty<JsonObject::BoolT> {"writable", testing::_},
            JsonProperty<JsonObject::BoolT> {"configurable", testing::_},
            JsonProperty<JsonObject::BoolT> {"enumerable", testing::_})));
        expected.push_back(testing::Pointee(JsonProperties(
            JsonProperty<JsonObject::StringT> {"name", "preview"},
            JsonProperty<JsonObject::JsonObjPointer> {
                "value", testing::Pointee(JsonProperties(JsonProperty<JsonObject::BoolT> {"value", preview},
                                                         JsonProperty<JsonObject::StringT> {"type", "boolean"}))},
            JsonProperty<JsonObject::BoolT> {"writable", testing::_},
            JsonProperty<JsonObject::BoolT> {"configurable", testing::_},
            JsonProperty<JsonObject::BoolT> {"enumerable", testing::_})));
        expected.push_back(testing::Pointee(JsonProperties(
            JsonProperty<JsonObject::StringT> {"name", "threadId"},
            JsonProperty<JsonObject::JsonObjPointer> {
                "value", testing::Pointee(JsonProperties(JsonProperty<JsonObject::NumT> {"value", g_mthread.GetId()},
                                                         JsonProperty<JsonObject::StringT> {"type", "number"}))},
            JsonProperty<JsonObject::BoolT> {"writable", testing::_},
            JsonProperty<JsonObject::BoolT> {"configurable", testing::_},
            JsonProperty<JsonObject::BoolT> {"enumerable", testing::_})));

        ASSERT_THAT(*result,
                    JsonProperties(JsonProperty<JsonObject::ArrayT> {"result", JsonElementsAreArray(expected)}));
    });

    auto getProperties = [](PtThread thread, RemoteObjectId id, bool need_preview) {
        std::vector<PropertyDescriptor> res;
        res.push_back(PropertyDescriptor("object", RemoteObject::Number(id)));
        res.push_back(PropertyDescriptor("preview", RemoteObject::Boolean(need_preview)));
        res.push_back(PropertyDescriptor("threadId", RemoteObject::Number(thread.GetId())));
        return res;
    };

    inspectorServer.OnCallRuntimeGetProperties(getProperties);
}

TEST_F(ServerTest, OnCallRuntimeRunIfWaitingForDebugger)
{
    inspectorServer.CallTargetAttachedToTarget(g_mthread);

    EXPECT_CALL(server, OnCallMock("Runtime.runIfWaitingForDebugger", testing::_))
        .WillOnce([&](testing::Unused, auto handler) {
            JsonObject empty;
            auto res = handler(g_sessionId, empty);
            ResultHolder result;
            GetResult(std::move(res), result);
            ASSERT_THAT(*result, JsonProperties());
            ASSERT_TRUE(g_handlerCalled);
        });

    inspectorServer.OnCallRuntimeRunIfWaitingForDebugger([](PtThread thread) {
        ASSERT_EQ(thread.GetId(), g_mthread.GetId());
        g_handlerCalled = true;
    });
}

}  // namespace ark::tooling::inspector::test

// NOLINTEND
