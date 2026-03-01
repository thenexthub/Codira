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

#include <chrono>
#include <csetjmp>
#include <csignal>
#include <cstring>
#include <deque>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "jsvm.h"
#include "jsvm_types.h"
#include "jsvm_utils.h"

#define JSVM_PARENT_CLASS_DES_COUNT 2
#define JSVM_OBJECT_INTERFAIELD_COUNT 3
#define JSVM_DEFINE_CLASS_OPTIONS_COUNT 2

using namespace std;
using namespace testing;
using namespace testing::ext;

JSVM_Env jsvm_env = nullptr;

static string srcProf = R"JS(
function sleep(delay) {
    var start = (new Date()).getTime();
    while ((new Date()).getTime() - start < delay) {
        continue;
    }
}
function work9() {
    sleep(100);
}
function work8() {
    work9();
    sleep(100);
}
function work7() {
    work8();
    sleep(100);
}
function work6() {
    work7();
    sleep(100);
}
function work5() {
    work6();
    sleep(100);
}
function work4() {
    work5();
    sleep(100);
}

function work3() {
    work4();
    sleep(100);
}

function work2() {
    work3();
    sleep(100);
}

function work1() {
    work2();
    sleep(100);
}

work1();
)JS";

static JSVM_Value hello_fn(JSVM_Env env, JSVM_CallbackInfo info)
{
    JSVM_Value output;
    void* data = nullptr;
    OH_JSVM_GetCbInfo(env, info, nullptr, nullptr, nullptr, &data);
    OH_JSVM_CreateStringUtf8(env, (char*)data, strlen((char*)data), &output);
    return output;
}

class Task {
public:
    virtual ~Task() = default;
    virtual void Run() = 0;
};

static deque<Task*> task_queue;

static JSVM_Value read_fn(JSVM_Env env, JSVM_CallbackInfo info)
{
    JSVM_Value promise;
    JSVM_Deferred deferred;
    OH_JSVM_CreatePromise(env, &deferred, &promise);
    class ReadTask : public Task {
    public:
        ReadTask(JSVM_Env env, JSVM_Deferred deferred) : env_(env), deferred_(deferred) {}
        void Run() override
        {
            string str;
            getline(cin, str);
            JSVM_Value result;
            OH_JSVM_CreateStringUtf8(env_, str.c_str(), str.size(), &result);
            OH_JSVM_ResolveDeferred(env_, deferred_, result);
        }

    private:
        JSVM_Env env_;
        JSVM_Deferred deferred_;
    };
    task_queue.push_back(new ReadTask(env, deferred));
    return promise;
}

#define BUF_SIZE1 102400
static JSVM_Value print_fn(JSVM_Env env, JSVM_CallbackInfo info)
{
    size_t argc = 1;
    JSVM_Value argv[1];
    OH_JSVM_GetCbInfo(env, info, &argc, argv, nullptr, nullptr);
    if (argc > 0) {
        char buf[BUF_SIZE1];
        OH_JSVM_GetValueStringUtf8(env, argv[0], buf, BUF_SIZE1, nullptr);
        std::cout << buf << std::endl;
    }
    return nullptr;
}

static JSVM_CallbackStruct hello_cb = { hello_fn, (void*)"Hello" };
static JSVM_CallbackStruct read_cb = { read_fn, nullptr };
static JSVM_CallbackStruct print_cb = { print_fn, nullptr };

static JSVM_PropertyDescriptor property_descriptors[] = {
    { "hello", NULL, &hello_cb, NULL, NULL, NULL, JSVM_DEFAULT },
    { "read", NULL, &read_cb, NULL, NULL, NULL, JSVM_DEFAULT },
    { "print", NULL, &print_cb, NULL, NULL, NULL, JSVM_DEFAULT },
};

class JSVMTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        GTEST_LOG_(INFO) << "JSVMTest SetUpTestCase";
        JSVM_InitOptions init_options {};
        OH_JSVM_Init(&init_options);
    }

    static void TearDownTestCase() {}

    void SetUp() override
    {
        OH_JSVM_CreateVM(nullptr, &vm);
        // propertyCount is 3
        OH_JSVM_CreateEnv(vm, 3, property_descriptors, &env);
        OH_JSVM_OpenVMScope(vm, &vm_scope);
        OH_JSVM_OpenEnvScope(env, &env_scope);
        OH_JSVM_OpenHandleScope(env, &handleScope);
        jsvm_env = env;
    }
    void TearDown() override
    {
        GTEST_LOG_(INFO) << "JSVMTest TearDown";
        OH_JSVM_CloseHandleScope(env, handleScope);
        OH_JSVM_CloseEnvScope(env, env_scope);
        OH_JSVM_CloseVMScope(vm, vm_scope);
        OH_JSVM_DestroyEnv(env);
        OH_JSVM_DestroyVM(vm);
    }

protected:
    JSVM_Env env = nullptr;
    JSVM_VM vm = nullptr;
    JSVM_EnvScope env_scope = nullptr;
    JSVM_VMScope vm_scope = nullptr;
    JSVM_HandleScope handleScope;
};

HWTEST_F(JSVMTest, JSVMGetVersion001, TestSize.Level1)
{
    uint32_t versionId = 0;
    JSVMTEST_CALL(OH_JSVM_GetVersion(env, &versionId));
    ASSERT_EQ(versionId, 9);
}

HWTEST_F(JSVMTest, JSVMEquals001, TestSize.Level1)
{
    JSVM_Value lhs = nullptr;
    bool x = true;
    JSVMTEST_CALL(OH_JSVM_GetBoolean(env, x, &lhs));
    JSVM_Value rhs = nullptr;
    bool y = true;
    JSVMTEST_CALL(OH_JSVM_GetBoolean(env, y, &rhs));
    bool isEquals = false;
    JSVMTEST_CALL(OH_JSVM_Equals(env, lhs, rhs, &isEquals));
    ASSERT_TRUE(isEquals);
}

HWTEST_F(JSVMTest, JSVMCreateCodeCache001, TestSize.Level1)
{
    JSVM_Value jsSrc;
    JSVMTEST_CALL(OH_JSVM_CreateStringUtf8(env, srcProf.c_str(), srcProf.size(), &jsSrc));

    const uint8_t x1 = 34;
    const uint8_t* x2 = &x1;
    const uint8_t** dataPtr = &x2;
    size_t x3 = 1;
    size_t* lengthPtr = &x3;
    JSVM_Script script = nullptr;
    JSVMTEST_CALL(OH_JSVM_CompileScript(env, jsSrc, nullptr, 0, true, nullptr, &script));
    JSVMTEST_CALL(OH_JSVM_CreateCodeCache(env, script, dataPtr, lengthPtr));
}

HWTEST_F(JSVMTest, JSVMAcquire001, TestSize.Level1)
{
    JSVMTEST_CALL(OH_JSVM_AcquireLock(env));
}

HWTEST_F(JSVMTest, JSVMIsObject001, TestSize.Level1)
{
    JSVM_Value obj;
    JSVMTEST_CALL(OH_JSVM_CreateMap(env, &obj));
    bool result = false;
    JSVMTEST_CALL(OH_JSVM_IsObject(env, obj, &result));
    ASSERT_TRUE(result);

    JSVMTEST_CALL(OH_JSVM_CreateSymbol(env, nullptr, &obj));
    result = false;
    JSVMTEST_CALL(OH_JSVM_IsObject(env, obj, &result));
    ASSERT_FALSE(result);
}

HWTEST_F(JSVMTest, JSVMIsBoolean001, TestSize.Level1)
{
    JSVM_Value obj;
    JSVMTEST_CALL(OH_JSVM_CreateArray(env, &obj));
    bool result = false;
    JSVMTEST_CALL(OH_JSVM_IsBoolean(env, obj, &result));
    ASSERT_FALSE(result);

    bool boolvalue = true;
    JSVMTEST_CALL(OH_JSVM_GetBoolean(env, boolvalue, &obj));
    result = false;
    JSVMTEST_CALL(OH_JSVM_IsBoolean(env, obj, &result));
    ASSERT_TRUE(result);
}

HWTEST_F(JSVMTest, JSVMIsString001, TestSize.Level1)
{
    JSVM_Value createString;
    char str[12] = "hello world";
    JSVMTEST_CALL(OH_JSVM_CreateStringUtf8(env, str, 12, &createString));
    bool result = false;
    JSVMTEST_CALL(OH_JSVM_IsString(env, createString, &result));
    ASSERT_TRUE(result);

    JSVM_Value obj = nullptr;
    JSVMTEST_CALL(OH_JSVM_CreateSet(env, &obj));
    result = false;
    JSVMTEST_CALL(OH_JSVM_IsString(env, obj, &result));
    ASSERT_FALSE(result);
}

HWTEST_F(JSVMTest, JSVMIsFunction001, TestSize.Level1)
{
    JSVM_Value function;
    JSVM_CallbackStruct param;
    param.data = nullptr;
    param.callback = nullptr;
    JSVMTEST_CALL(OH_JSVM_CreateFunction(env, "func", JSVM_AUTO_LENGTH, &param, &function));
    bool result = false;
    JSVMTEST_CALL(OH_JSVM_IsFunction(env, function, &result));
    ASSERT_TRUE(result);

    JSVM_Value obj;
    JSVMTEST_CALL(OH_JSVM_CreateObject(env, &obj));
    result = false;
    JSVMTEST_CALL(OH_JSVM_IsFunction(env, obj, &result));
    ASSERT_FALSE(result);
}

HWTEST_F(JSVMTest, JSVMIsSymbol001, TestSize.Level1)
{
    JSVM_Value obj;
    JSVMTEST_CALL(OH_JSVM_CreateSymbol(env, nullptr, &obj));
    bool result = false;
    JSVMTEST_CALL(OH_JSVM_IsSymbol(env, obj, &result));
    ASSERT_TRUE(result);

    JSVMTEST_CALL(OH_JSVM_CreateObject(env, &obj));
    result = false;
    JSVMTEST_CALL(OH_JSVM_IsSymbol(env, obj, &result));
    ASSERT_FALSE(result);
}

HWTEST_F(JSVMTest, JSVMIsNumber001, TestSize.Level1)
{
    JSVM_Value obj;
    JSVMTEST_CALL(OH_JSVM_CreateObject(env, &obj));
    bool result = false;
    JSVMTEST_CALL(OH_JSVM_IsNumber(env, obj, &result));
    ASSERT_FALSE(result);

    JSVM_Value value;
    int intValue = 2;
    JSVMTEST_CALL(OH_JSVM_CreateInt32(env, intValue, &value));
    result = false;
    JSVMTEST_CALL(OH_JSVM_IsNumber(env, value, &result));
    ASSERT_TRUE(result);
}

HWTEST_F(JSVMTest, JSVMIsBigInt001, TestSize.Level1)
{
    JSVM_Value obj;
    JSVMTEST_CALL(OH_JSVM_CreateObject(env, &obj));
    bool result = false;
    JSVMTEST_CALL(OH_JSVM_IsBigInt(env, obj, &result));
    ASSERT_FALSE(result);

    JSVM_Value bigint;
    int intValue = 2;
    JSVMTEST_CALL(OH_JSVM_CreateBigintInt64(env, intValue, &bigint));
    result = false;
    JSVMTEST_CALL(OH_JSVM_IsBigInt(env, bigint, &result));
    ASSERT_TRUE(result);
}

HWTEST_F(JSVMTest, JSVMIsNull001, TestSize.Level1)
{
    JSVM_Value input;
    bool result = false;
    JSVMTEST_CALL(OH_JSVM_CreateArray(env, &input));
    JSVMTEST_CALL(OH_JSVM_IsNull(env, input, &result));
    ASSERT_FALSE(result);

    JSVM_Value input2;
    bool result2;
    JSVMTEST_CALL(OH_JSVM_GetNull(env, &input2));
    JSVMTEST_CALL(OH_JSVM_IsNull(env, input2, &result2));
    ASSERT_TRUE(result2);
}

HWTEST_F(JSVMTest, JSVMIsUndefined001, TestSize.Level1)
{
    JSVM_Value input;
    bool result = false;
    JSVMTEST_CALL(OH_JSVM_CreateArray(env, &input));
    JSVMTEST_CALL(OH_JSVM_IsUndefined(env, input, &result));
    ASSERT_FALSE(result);

    JSVM_Value input2;
    bool result2;
    JSVMTEST_CALL(OH_JSVM_GetUndefined(env, &input2));
    JSVMTEST_CALL(OH_JSVM_IsUndefined(env, input2, &result2));
    ASSERT_TRUE(result2);
}

HWTEST_F(JSVMTest, OH_JSVM_IsNullOrUndefined001, TestSize.Level1)
{
    JSVM_Value input;
    bool result = false;
    JSVMTEST_CALL(OH_JSVM_CreateArray(env, &input));
    JSVMTEST_CALL(OH_JSVM_IsNullOrUndefined(env, input, &result));
    ASSERT_FALSE(result);

    JSVM_Value input2;
    bool result2 = false;
    JSVMTEST_CALL(OH_JSVM_GetNull(env, &input2));
    JSVMTEST_CALL(OH_JSVM_IsNullOrUndefined(env, input2, &result2));
    ASSERT_TRUE(result2);

    result2 = false;
    JSVMTEST_CALL(OH_JSVM_GetUndefined(env, &input2));
    JSVMTEST_CALL(OH_JSVM_IsNullOrUndefined(env, input2, &result2));
    ASSERT_TRUE(result2);
}

HWTEST_F(JSVMTest, JSVMIsLocked001, TestSize.Level1)
{
    bool isLocked = false;
    JSVMTEST_CALL(OH_JSVM_IsLocked(env, &isLocked));
}

HWTEST_F(JSVMTest, JSVMReleaseLock001, TestSize.Level1)
{
    bool isLocked = false;
    JSVMTEST_CALL(OH_JSVM_IsLocked(env, &isLocked));
    JSVMTEST_CALL(OH_JSVM_ReleaseLock(env));
}

HWTEST_F(JSVMTest, JSVMCompileScriptWithOrigin001, TestSize.Level1)
{
    JSVM_Value jsSrc;
    JSVMTEST_CALL(OH_JSVM_CreateStringUtf8(env, srcProf.c_str(), srcProf.size(), &jsSrc));
    JSVM_Script script;
    JSVM_ScriptOrigin origin {
        .sourceMapUrl = "/data/local/tmp/workload/index.js.map",
        // 源文件名字
        .resourceName = "index.js",
        // scirpt 在源文件中的起始行列号
        .resourceLineOffset = 0,
        .resourceColumnOffset = 0,
    };
    JSVMTEST_CALL(OH_JSVM_CompileScriptWithOrigin(env, jsSrc, nullptr, 0, true, nullptr, &origin, &script));
}

HWTEST_F(JSVMTest, JSVMCompileScriptWithOrigin002, TestSize.Level1)
{
    JSVM_Value jsSrc = nullptr;
    bool x = true;
    JSVMTEST_CALL(OH_JSVM_GetBoolean(env, x, &jsSrc));
    JSVM_Script script;
    JSVM_ScriptOrigin origin {
        .sourceMapUrl = "/data/local/tmp/workload/index.js.map",
        // 源文件名字
        .resourceName = "index.js",
        // scirpt 在源文件中的起始行列号
        .resourceLineOffset = 0,
        .resourceColumnOffset = 0,
    };
    JSVM_Status status = OH_JSVM_CompileScriptWithOrigin(env, jsSrc, nullptr, 0, true, nullptr, &origin, &script);
    ASSERT_EQ(status, 3);
}

HWTEST_F(JSVMTest, JSVMCompileScriptWithOrigin003, TestSize.Level1)
{
    JSVM_Value jsSrc = nullptr;
    bool x = true;
    JSVMTEST_CALL(OH_JSVM_GetBoolean(env, x, &jsSrc));
    JSVM_ScriptOrigin origin {
        .sourceMapUrl = "/data/local/tmp/workload/index.js.map",
        // 源文件名字
        .resourceName = "index.js",
        // scirpt 在源文件中的起始行列号
        .resourceLineOffset = 0,
        .resourceColumnOffset = 0,
    };
    JSVM_Status status = OH_JSVM_CompileScriptWithOrigin(env, jsSrc, nullptr, 0, true, nullptr, &origin, nullptr);
    ASSERT_EQ(status, 1);
}

static JSVM_PropertyHandlerConfigurationStruct propertyCfg { nullptr, nullptr, nullptr, nullptr, nullptr,
                                                             nullptr, nullptr, nullptr, nullptr, nullptr };

HWTEST_F(JSVMTest, JSVMDefineClassWithPropertyHandler001, TestSize.Level1)
{
    JSVM_CallbackStruct param;
    param.callback = [](JSVM_Env env, JSVM_CallbackInfo info) -> JSVM_Value {
        JSVM_Value thisVar = nullptr;
        JSVMTEST_CALL(OH_JSVM_GetCbInfo(env, info, nullptr, nullptr, &thisVar, nullptr));
        return thisVar;
    };
    param.data = nullptr;
    JSVM_Value testWrapClass = nullptr;
    JSVMTEST_CALL(OH_JSVM_DefineClassWithPropertyHandler(env, "Test2", 5, &param, 0, nullptr, &propertyCfg, nullptr,
                                                         &testWrapClass));
}

HWTEST_F(JSVMTest, JSVMDefineClassWithPropertyHandler002, TestSize.Level1)
{
    JSVM_CallbackStruct param;
    param.callback = [](JSVM_Env env, JSVM_CallbackInfo info) -> JSVM_Value {
        JSVM_Value thisVar = nullptr;
        JSVMTEST_CALL(OH_JSVM_GetCbInfo(env, info, nullptr, nullptr, &thisVar, nullptr));
        return thisVar;
    };
    param.data = nullptr;
    JSVM_Value testWrapClass = nullptr;
    JSVMTEST_CALL(OH_JSVM_DefineClassWithPropertyHandler(env, "Test2", JSVM_AUTO_LENGTH, &param, 0, nullptr,
                                                         &propertyCfg, nullptr, &testWrapClass));
}

HWTEST_F(JSVMTest, JSVMDefineClassWithPropertyHandler003, TestSize.Level1)
{
    JSVM_CallbackStruct param;
    param.callback = [](JSVM_Env env, JSVM_CallbackInfo info) -> JSVM_Value {
        JSVM_Value thisVar = nullptr;
        JSVMTEST_CALL(OH_JSVM_GetCbInfo(env, info, nullptr, nullptr, &thisVar, nullptr));
        return thisVar;
    };
    param.data = nullptr;
    JSVM_Value testWrapClass = nullptr;
    JSVMTEST_CALL(OH_JSVM_DefineClassWithPropertyHandler(env, "Test2", 4, &param, 0, nullptr, &propertyCfg, nullptr,
                                                         &testWrapClass));
}

HWTEST_F(JSVMTest, JSVMDefineClassWithPropertyHandler004, TestSize.Level1)
{
    JSVM_CallbackStruct param;
    param.callback = [](JSVM_Env env, JSVM_CallbackInfo info) -> JSVM_Value {
        JSVM_Value thisVar = nullptr;
        JSVMTEST_CALL(OH_JSVM_GetCbInfo(env, info, nullptr, nullptr, &thisVar, nullptr));
        return thisVar;
    };
    param.data = nullptr;
    JSVM_Value testWrapClass = nullptr;
    JSVMTEST_CALL(OH_JSVM_DefineClassWithPropertyHandler(env, "Test2", 6, &param, 0, nullptr, &propertyCfg, nullptr,
                                                         &testWrapClass));
}

HWTEST_F(JSVMTest, JSVMDefineClassWithPropertyHandler005, TestSize.Level1)
{
    JSVM_CallbackStruct param;
    param.callback = [](JSVM_Env env, JSVM_CallbackInfo info) -> JSVM_Value {
        JSVM_Value thisVar = nullptr;
        JSVMTEST_CALL(OH_JSVM_GetCbInfo(env, info, nullptr, nullptr, &thisVar, nullptr));
        return thisVar;
    };
    param.data = nullptr;
    JSVM_Status status = OH_JSVM_DefineClassWithPropertyHandler(env, "Test2", JSVM_AUTO_LENGTH, &param, 0, nullptr,
                                                                &propertyCfg, nullptr, nullptr);
    ASSERT_EQ(status, JSVM_INVALID_ARG);
}

HWTEST_F(JSVMTest, JSVMDefineClassWithPropertyHandler006, TestSize.Level1)
{
    JSVM_Value testWrapClass = nullptr;
    JSVM_Status status = OH_JSVM_DefineClassWithPropertyHandler(env, "Test2", JSVM_AUTO_LENGTH, nullptr, 0, nullptr,
                                                                &propertyCfg, nullptr, &testWrapClass);
    ASSERT_EQ(status, JSVM_INVALID_ARG);
}

HWTEST_F(JSVMTest, JSVMDefineClassWithPropertyHandler007, TestSize.Level1)
{
    JSVM_CallbackStruct param;
    param.callback = [](JSVM_Env env, JSVM_CallbackInfo info) -> JSVM_Value {
        JSVM_Value thisVar = nullptr;
        JSVMTEST_CALL(OH_JSVM_GetCbInfo(env, info, nullptr, nullptr, &thisVar, nullptr));
        return thisVar;
    };
    param.data = nullptr;
    JSVM_Value testWrapClass = nullptr;
    JSVM_Status status = OH_JSVM_DefineClassWithPropertyHandler(env, "Test2", JSVM_AUTO_LENGTH, &param, 0, nullptr,
                                                                nullptr, nullptr, &testWrapClass);
    ASSERT_EQ(status, JSVM_INVALID_ARG);
}

HWTEST_F(JSVMTest, JSVMCreateSnapshot001, TestSize.Level1)
{
    const char* blobData = nullptr;
    size_t blobSize = 0;
    JSVM_Env envs[1] = { env };
    JSVM_Status status = OH_JSVM_CreateSnapshot(vm, 1, envs, &blobData, &blobSize);
    ASSERT_EQ(status, JSVM_GENERIC_FAILURE);
}

HWTEST_F(JSVMTest, JSVMCreateEnvFromSnapshot001, TestSize.Level1)
{
    JSVM_Env env2 = nullptr;
    JSVM_Status status = OH_JSVM_CreateEnvFromSnapshot(vm, 0, &env2);
    ASSERT_EQ(status, JSVM_GENERIC_FAILURE);
}

HWTEST_F(JSVMTest, JSVMTraceStart001, TestSize.Level1)
{
    JSVM_Status status = OH_JSVM_TraceStart(0, nullptr, "default", 0);
    ASSERT_EQ(status, JSVM_OK);
}

HWTEST_F(JSVMTest, JSVMTraceStart002, TestSize.Level1)
{
    std::vector<JSVM_TraceCategory> category = { JSVM_TRACE_WASM };
    JSVM_Status status = OH_JSVM_TraceStart(category.size(), category.data(), "custom", 100);
    ASSERT_EQ(status, JSVM_OK);
}

HWTEST_F(JSVMTest, JSVMTraceStart003, TestSize.Level1)
{
    std::vector<JSVM_TraceCategory> category(1);
    *(int*)&category[0] = 100;
    JSVM_Status status = OH_JSVM_TraceStart(category.size(), category.data(), "invalid", 0);
    ASSERT_EQ(status, JSVM_INVALID_ARG);
}

HWTEST_F(JSVMTest, JSVMTraceStart004, TestSize.Level1)
{
    std::vector<JSVM_TraceCategory> category = { JSVM_TRACE_WASM };
    JSVM_Status status = OH_JSVM_TraceStart(0, category.data(), "invalid", 0);
    ASSERT_EQ(status, JSVM_INVALID_ARG);
}

HWTEST_F(JSVMTest, JSVMTraceStart005, TestSize.Level1)
{
    JSVM_Status status = OH_JSVM_TraceStart(1, nullptr, "invalid", 0);
    ASSERT_EQ(status, JSVM_INVALID_ARG);
}

bool OutputStream(const char* data, int size, void* streamData)
{
    std::string value(data, size);
    *(std::string*)streamData = std::string(data, size);
    return true;
}

HWTEST_F(JSVMTest, JSVMTraceStop001, TestSize.Level1)
{
    JSVM_Status status = OH_JSVM_TraceStart(0, nullptr, "default", 0);
    ASSERT_EQ(status, JSVM_OK);
    std::string data;
    status = OH_JSVM_TraceStop(OutputStream, (void*)&data);
    ASSERT_EQ(status, JSVM_OK);
}

HWTEST_F(JSVMTest, JSVMTraceStop002, TestSize.Level1)
{
    std::string data;
    JSVM_Status status = OH_JSVM_TraceStop(OutputStream, (void*)&data);
    ASSERT_EQ(status, JSVM_INVALID_ARG);
}

HWTEST_F(JSVMTest, JSVMTraceStop003, TestSize.Level1)
{
    JSVM_Status status = OH_JSVM_TraceStart(0, nullptr, "default", 0);
    ASSERT_EQ(status, JSVM_OK);
    status = OH_JSVM_TraceStop(OutputStream, nullptr);
    ASSERT_EQ(status, JSVM_INVALID_ARG);
    std::string data;
    status = OH_JSVM_TraceStop(OutputStream, (void*)&data);
    ASSERT_EQ(status, JSVM_OK);
}

HWTEST_F(JSVMTest, JSVMTraceStop004, TestSize.Level1)
{
    JSVM_Status status = OH_JSVM_TraceStart(0, nullptr, "default", 0);
    ASSERT_EQ(status, JSVM_OK);
    std::string data;
    status = OH_JSVM_TraceStop(nullptr, (void*)&data);
    ASSERT_EQ(status, JSVM_INVALID_ARG);
    status = OH_JSVM_TraceStop(OutputStream, (void*)&data);
    ASSERT_EQ(status, JSVM_OK);
}

HWTEST_F(JSVMTest, JSVMIsNumberObject001, TestSize.Level1)
{
    JSVM_Value result = jsvm::Run("new Number(42)");
    bool isNumberObject = false;
    JSVMTEST_CALL(OH_JSVM_IsNumberObject(env, result, &isNumberObject));
    ASSERT_TRUE(isNumberObject);
}

HWTEST_F(JSVMTest, JSVMIsBooleanObject001, TestSize.Level1)
{
    JSVM_Value result = jsvm::Run("new Boolean(true)");
    bool isBooleanObject = false;
    JSVMTEST_CALL(OH_JSVM_IsBooleanObject(env, result, &isBooleanObject));
    ASSERT_TRUE(isBooleanObject);
}

HWTEST_F(JSVMTest, JSVMIsBigIntObject001, TestSize.Level1)
{
    JSVM_Value result = jsvm::Run("new Object(42n)");
    bool isBigIntObject = false;
    JSVMTEST_CALL(OH_JSVM_IsBigIntObject(env, result, &isBigIntObject));
    ASSERT_TRUE(isBigIntObject);
}

HWTEST_F(JSVMTest, JSVMIsStringObject001, TestSize.Level1)
{
    JSVM_Value result = jsvm::Run("new String(\"test\")");
    bool isStringObject = false;
    JSVMTEST_CALL(OH_JSVM_IsStringObject(env, result, &isStringObject));
    ASSERT_TRUE(isStringObject);
}

HWTEST_F(JSVMTest, JSVMIsSymbolObject001, TestSize.Level1)
{
    JSVM_Value result = jsvm::Run("Object(Symbol('foo'))");
    bool isSymbolObject = false;
    JSVMTEST_CALL(OH_JSVM_IsSymbolObject(env, result, &isSymbolObject));
    ASSERT_TRUE(isSymbolObject);
}

HWTEST_F(JSVMTest, JSVMToStringTag001, TestSize.Level1)
{
    JSVM_Value result;
    JSVMTEST_CALL(OH_JSVM_GetSymbolToStringTag(env, &result));
    ASSERT_TRUE(jsvm::StrictEquals(result, jsvm::Run("Symbol.toStringTag")));
}

HWTEST_F(JSVMTest, JSVMToPrimitive001, TestSize.Level1)
{
    JSVM_Value result;
    JSVMTEST_CALL(OH_JSVM_GetSymbolToPrimitive(env, &result));
    ASSERT_TRUE(jsvm::StrictEquals(result, jsvm::Run("Symbol.toPrimitive")));
}

HWTEST_F(JSVMTest, JSVMSplit001, TestSize.Level1)
{
    JSVM_Value result;
    JSVMTEST_CALL(OH_JSVM_GetSymbolSplit(env, &result));
    ASSERT_TRUE(jsvm::StrictEquals(result, jsvm::Run("Symbol.split")));
}

HWTEST_F(JSVMTest, JSVMSearch001, TestSize.Level1)
{
    JSVM_Value result;
    JSVMTEST_CALL(OH_JSVM_GetSymbolSearch(env, &result));
    ASSERT_TRUE(jsvm::StrictEquals(result, jsvm::Run("Symbol.search")));
}

HWTEST_F(JSVMTest, JSVMReplace001, TestSize.Level1)
{
    JSVM_Value result;
    JSVMTEST_CALL(OH_JSVM_GetSymbolReplace(env, &result));
    ASSERT_TRUE(jsvm::StrictEquals(result, jsvm::Run("Symbol.replace")));
}

HWTEST_F(JSVMTest, JSVMMatch001, TestSize.Level1)
{
    JSVM_Value result;
    JSVMTEST_CALL(OH_JSVM_GetSymbolMatch(env, &result));
    ASSERT_TRUE(jsvm::StrictEquals(result, jsvm::Run("Symbol.match")));
}

HWTEST_F(JSVMTest, JSVMIsConcatSpreadable001, TestSize.Level1)
{
    JSVM_Value result;
    JSVMTEST_CALL(OH_JSVM_GetSymbolIsConcatSpreadable(env, &result));
    ASSERT_TRUE(jsvm::StrictEquals(result, jsvm::Run("Symbol.isConcatSpreadable")));
}

HWTEST_F(JSVMTest, CreateProxy, TestSize.Level1)
{
    std::string registerObject = R"JS(
      target = {
        message1: "hello",
        message2: "everyone",
      };

      handler1 = {};
      handler2 = {
        get(target, prop, receiver) {
          return "world";
        },
      };
    )JS";

    jsvm::Run(registerObject.c_str());
    auto obj = jsvm::GetProperty(jsvm::Global(), "target");
    auto handler1 = jsvm::GetProperty(jsvm::Global(), "handler1");
    auto handler2 = jsvm::GetProperty(jsvm::Global(), "handler2");

    JSVM_Value proxy1 = nullptr;
    JSVMTEST_CALL(OH_JSVM_CreateProxy(env, obj, handler1, &proxy1));

    JSVM_Value proxy2 = nullptr;
    JSVMTEST_CALL(OH_JSVM_CreateProxy(env, obj, handler2, &proxy2));

    jsvm::SetProperty(jsvm::Global(), "proxy1", proxy1);
    jsvm::SetProperty(jsvm::Global(), "proxy2", proxy2);

    std::string checkProxy1 = R"JS(
      proxy1.message1 == target.message1 && proxy1.message2 == target.message2;
    )JS";

    auto result = jsvm::Run(checkProxy1.c_str());
    ASSERT_TRUE(jsvm::IsTrue(result));

    std::string checkProxy2 = R"JS(
      proxy2.message1 == "world" && proxy2.message2 == "world" &&
      proxy2.anyProperty == "world";
    )JS";

    result = jsvm::Run(checkProxy2.c_str());
    ASSERT_TRUE(jsvm::IsTrue(result));
}

HWTEST_F(JSVMTest, IsProxy, TestSize.Level1)
{
    jsvm::Run(R"JS(
      target = {
        message1: "hello",
        message2: "everyone",
      };

      handler1 = {};
      proxy1 = new Proxy(target, handler1);
    )JS");

    bool isProxy = false;

    auto value = jsvm::GetProperty(jsvm::Global(), "target");
    JSVMTEST_CALL(OH_JSVM_IsProxy(env, value, &isProxy));
    ASSERT_TRUE(!isProxy && "target is not JS Proxy");

    value = jsvm::GetProperty(jsvm::Global(), "handler1");
    JSVMTEST_CALL(OH_JSVM_IsProxy(env, value, &isProxy));
    ASSERT_TRUE(!isProxy && "handler1 is not JS Proxy");

    value = jsvm::GetProperty(jsvm::Global(), "proxy1");
    JSVMTEST_CALL(OH_JSVM_IsProxy(env, value, &isProxy));
    ASSERT_TRUE(isProxy && "proxy1 is JS Proxy");
}

HWTEST_F(JSVMTest, GetProxyTarget, TestSize.Level1)
{
    jsvm::Run(R"JS(
      target = {
        message1: "hello",
        message2: "everyone",
      };

      handler1 = {};
      proxy1 = new Proxy(target, handler1);
    )JS");

    auto target = jsvm::GetProperty(jsvm::Global(), "target");
    auto proxy1 = jsvm::GetProperty(jsvm::Global(), "proxy1");

    JSVM_Value result = nullptr;
    JSVMTEST_CALL(OH_JSVM_ProxyGetTarget(env, proxy1, &result));

    ASSERT_TRUE(jsvm::StrictEquals(target, result));
}

static std::string g_scriptEvalMicrotask = R"JS(
    evaluateMicrotask = false;
    Promise.resolve().then(()=>{
        evaluateMicrotask = true;
    });
    evaluateMicrotask
)JS";

// Default microtask policy is JSVM_MICROTASK_AUTO
HWTEST_F(JSVMTest, MicrotaskPolicyDefault, TestSize.Level1)
{
    auto result = jsvm::Run(g_scriptEvalMicrotask.c_str());
    ASSERT_TRUE(jsvm::IsFalse(result));
    result = jsvm::GetProperty(jsvm::Global(), "evaluateMicrotask");
    ASSERT_TRUE(jsvm::IsTrue(result));
}

// Test Set Microtask Policy to explicit
HWTEST_F(JSVMTest, MicrotaskPolicyExplict, TestSize.Level1)
{
    OH_JSVM_SetMicrotaskPolicy(vm, JSVM_MicrotaskPolicy::JSVM_MICROTASK_EXPLICIT);
    auto result = jsvm::Run(g_scriptEvalMicrotask.c_str());
    ASSERT_TRUE(jsvm::IsFalse(result));
    result = jsvm::GetProperty(jsvm::Global(), "evaluateMicrotask");
    ASSERT_TRUE(jsvm::IsFalse(result));
    OH_JSVM_PerformMicrotaskCheckpoint(vm);
    result = jsvm::GetProperty(jsvm::Global(), "evaluateMicrotask");
    ASSERT_TRUE(jsvm::IsTrue(result));
}

// Test Set Microtask Policy to auto
HWTEST_F(JSVMTest, MicrotaskPolicyAuto, TestSize.Level1)
{
    OH_JSVM_SetMicrotaskPolicy(vm, JSVM_MicrotaskPolicy::JSVM_MICROTASK_EXPLICIT);
    OH_JSVM_SetMicrotaskPolicy(vm, JSVM_MicrotaskPolicy::JSVM_MICROTASK_AUTO);
    auto result = jsvm::Run(g_scriptEvalMicrotask.c_str());
    ASSERT_TRUE(jsvm::IsFalse(result));
    result = jsvm::GetProperty(jsvm::Global(), "evaluateMicrotask");
    ASSERT_TRUE(jsvm::IsTrue(result));
}

HWTEST_F(JSVMTest, MicrotaskInvalidPolicy, TestSize.Level1)
{
    JSVM_MicrotaskPolicy invalidPolicy = (JSVM_MicrotaskPolicy)2;
    auto status = OH_JSVM_SetMicrotaskPolicy(vm, invalidPolicy);
    ASSERT_TRUE(status != JSVM_OK);
}

static std::string g_defineFunction = R"JS(
    var x1 = 0;
    var x2 = 0;
    function f1(x) {
        x1 = x;
        return x + 1;
    }
    function f2(x) {
        x2 = x;
        return x + 1;
    }
)JS";

static std::string g_init = R"JS(
    x1 = 0;
    x2 = 0;
)JS";

HWTEST_F(JSVMTest, PromiseThen, TestSize.Level1)
{
    JSVM_Value promise = nullptr;
    JSVM_Deferred deffered;

    JSVMTEST_CALL(OH_JSVM_CreatePromise(env, &deffered, &promise));
    jsvm::Run(g_defineFunction.c_str());

    JSVM_Value f1 = jsvm::GetProperty(jsvm::Global(), "f1");

    JSVMTEST_CALL(OH_JSVM_PromiseRegisterHandler(env, promise, f1, nullptr, nullptr));

    auto x1 = jsvm::GetProperty(jsvm::Global(), "x1");
    auto x2 = jsvm::GetProperty(jsvm::Global(), "x2");
    ASSERT_TRUE(jsvm::Equals(x1, jsvm::Int32(0)));
    ASSERT_TRUE(jsvm::Equals(x2, jsvm::Int32(0)));

    OH_JSVM_ResolveDeferred(env, deffered, jsvm::Int32(2));
    deffered = nullptr;

    x1 = jsvm::GetProperty(jsvm::Global(), "x1");
    x2 = jsvm::GetProperty(jsvm::Global(), "x2");
    ASSERT_TRUE(jsvm::Equals(x1, jsvm::Int32(2)));
    ASSERT_TRUE(jsvm::Equals(x2, jsvm::Int32(0)));
}

HWTEST_F(JSVMTest, PromiseThen2, TestSize.Level1)
{
    jsvm::Run(g_defineFunction.c_str());

    JSVM_Value f1 = jsvm::GetProperty(jsvm::Global(), "f1");
    auto f2 = jsvm::GetProperty(jsvm::Global(), "f2");

    JSVM_Value promise;
    JSVM_Deferred deffered;

    // Resolve
    JSVMTEST_CALL(OH_JSVM_CreatePromise(env, &deffered, &promise));
    JSVMTEST_CALL(OH_JSVM_PromiseRegisterHandler(env, promise, f1, f2, nullptr));

    auto x1 = jsvm::GetProperty(jsvm::Global(), "x1");
    auto x2 = jsvm::GetProperty(jsvm::Global(), "x2");
    ASSERT_TRUE(jsvm::Equals(x1, jsvm::Int32(0)));
    ASSERT_TRUE(jsvm::Equals(x2, jsvm::Int32(0)));

    OH_JSVM_ResolveDeferred(env, deffered, jsvm::Int32(2));
    deffered = nullptr;

    x1 = jsvm::GetProperty(jsvm::Global(), "x1");
    x2 = jsvm::GetProperty(jsvm::Global(), "x2");
    ASSERT_TRUE(jsvm::Equals(x1, jsvm::Int32(2)));
    ASSERT_TRUE(jsvm::Equals(x2, jsvm::Int32(0)));

    jsvm::Run(g_init.c_str());
    // Reject
    JSVMTEST_CALL(OH_JSVM_CreatePromise(env, &deffered, &promise));
    JSVMTEST_CALL(OH_JSVM_PromiseRegisterHandler(env, promise, f1, f2, nullptr));

    x1 = jsvm::GetProperty(jsvm::Global(), "x1");
    x2 = jsvm::GetProperty(jsvm::Global(), "x2");
    ASSERT_TRUE(jsvm::Equals(x1, jsvm::Int32(0)));
    ASSERT_TRUE(jsvm::Equals(x2, jsvm::Int32(0)));

    OH_JSVM_RejectDeferred(env, deffered, jsvm::Int32(3));
    deffered = nullptr;

    x1 = jsvm::GetProperty(jsvm::Global(), "x1");
    x2 = jsvm::GetProperty(jsvm::Global(), "x2");
    ASSERT_TRUE(jsvm::Equals(x1, jsvm::Int32(0)));
    ASSERT_TRUE(jsvm::Equals(x2, jsvm::Int32(3)));
}

HWTEST_F(JSVMTest, PromiseThenChain, TestSize.Level1)
{
    jsvm::Run(g_defineFunction.c_str());

    JSVM_Value f1 = jsvm::GetProperty(jsvm::Global(), "f1");
    auto f2 = jsvm::GetProperty(jsvm::Global(), "f2");

    JSVM_Value promise, promise1, x1, x2;
    JSVM_Deferred deffered = nullptr;

    // Resolve
    JSVMTEST_CALL(OH_JSVM_CreatePromise(env, &deffered, &promise));
    JSVMTEST_CALL(OH_JSVM_PromiseRegisterHandler(env, promise, f1, nullptr, &promise1));
    JSVMTEST_CALL(OH_JSVM_PromiseRegisterHandler(env, promise1, f2, nullptr, nullptr));

    x1 = jsvm::GetProperty(jsvm::Global(), "x1");
    x2 = jsvm::GetProperty(jsvm::Global(), "x2");
    ASSERT_TRUE(jsvm::Equals(x1, jsvm::Int32(0)));
    ASSERT_TRUE(jsvm::Equals(x2, jsvm::Int32(0)));

    OH_JSVM_ResolveDeferred(env, deffered, jsvm::Int32(2));
    deffered = nullptr;

    x1 = jsvm::GetProperty(jsvm::Global(), "x1");
    x2 = jsvm::GetProperty(jsvm::Global(), "x2");
    ASSERT_TRUE(jsvm::Equals(x1, jsvm::Int32(2)));
    ASSERT_TRUE(jsvm::Equals(x2, jsvm::Int32(3)));
}

HWTEST_F(JSVMTest, PromiseCatch, TestSize.Level1)
{
    jsvm::Run(g_defineFunction.c_str());

    JSVM_Value f1 = jsvm::GetProperty(jsvm::Global(), "f1");

    JSVM_Value promise;
    JSVM_Value promise1;
    JSVM_Deferred deffered = nullptr;

    // Resolve
    JSVMTEST_CALL(OH_JSVM_CreatePromise(env, &deffered, &promise));
    JSVMTEST_CALL(OH_JSVM_PromiseRegisterHandler(env, promise, nullptr, f1, &promise1));

    auto x1 = jsvm::GetProperty(jsvm::Global(), "x1");
    auto x2 = jsvm::GetProperty(jsvm::Global(), "x2");
    ASSERT_TRUE(jsvm::Equals(x1, jsvm::Int32(0)));
    ASSERT_TRUE(jsvm::Equals(x2, jsvm::Int32(0)));

    JSVMTEST_CALL(OH_JSVM_RejectDeferred(env, deffered, jsvm::Int32(2)));
    deffered = nullptr;

    x1 = jsvm::GetProperty(jsvm::Global(), "x1");
    x2 = jsvm::GetProperty(jsvm::Global(), "x2");
    ASSERT_TRUE(jsvm::Equals(x1, jsvm::Int32(2)));
    ASSERT_TRUE(jsvm::Equals(x2, jsvm::Int32(0)));
}

HWTEST_F(JSVMTest, JSVMHasInstance001, TestSize.Level1)
{
    JSVM_Value result;
    JSVMTEST_CALL(OH_JSVM_GetSymbolHasInstance(env, &result));
    ASSERT_TRUE(jsvm::StrictEquals(result, jsvm::Run("Symbol.hasInstance")));
}

HWTEST_F(JSVMTest, JSVMUnscopables001, TestSize.Level1)
{
    JSVM_Value result;
    JSVMTEST_CALL(OH_JSVM_GetSymbolUnscopables(env, &result));
    ASSERT_TRUE(jsvm::StrictEquals(result, jsvm::Run("Symbol.unscopables")));
}

HWTEST_F(JSVMTest, JSVMAsyncIterator001, TestSize.Level1)
{
    JSVM_Value result;
    JSVMTEST_CALL(OH_JSVM_GetSymbolAsyncIterator(env, &result));
    ASSERT_TRUE(jsvm::StrictEquals(result, jsvm::Run("Symbol.asyncIterator")));
}

HWTEST_F(JSVMTest, JSVMIterator001, TestSize.Level1)
{
    JSVM_Value result;
    JSVMTEST_CALL(OH_JSVM_GetSymbolIterator(env, &result));
    ASSERT_TRUE(jsvm::StrictEquals(result, jsvm::Run("Symbol.iterator")));
}

static jmp_buf g_buf;
static bool g_beforeFlag1 = false;
static bool g_beforeFlag2 = false;
static bool g_beforeFlag3 = false;
static bool g_afterFlag1 = false;
static bool g_afterFlag2 = false;
static bool g_afterFlag3 = false;
static int g_nativeValue = 2024;

void OnBeforeGC(JSVM_VM vm, JSVM_GCType gcType, JSVM_GCCallbackFlags flags, void* data)
{
    g_beforeFlag1 = true;
}

void OnBeforeGC2(JSVM_VM vm, JSVM_GCType gcType, JSVM_GCCallbackFlags flags, void* data)
{
    if (*(int*)data == g_nativeValue) {
        g_beforeFlag2 = true;
    }
}

void OnBeforeGC3(JSVM_VM vm, JSVM_GCType gcType, JSVM_GCCallbackFlags flags, void* data)
{
    g_beforeFlag3 = true;
}

void OnAfterGC(JSVM_VM vm, JSVM_GCType gcType, JSVM_GCCallbackFlags flags, void* data)
{
    g_afterFlag1 = true;
}

void OnAfterGC2(JSVM_VM vm, JSVM_GCType gcType, JSVM_GCCallbackFlags flags, void* data)
{
    g_afterFlag2 = true;
}

void OnAfterGC3(JSVM_VM vm, JSVM_GCType gcType, JSVM_GCCallbackFlags flags, void* data)
{
    g_afterFlag3 = true;
}

HWTEST_F(JSVMTest, JSVMForwardUsageApplicationScenariosOfGCCB, TestSize.Level1)
{
    g_beforeFlag1 = false;
    g_beforeFlag2 = false;
    g_beforeFlag3 = false;
    g_afterFlag1 = false;
    g_afterFlag2 = false;
    int data = g_nativeValue;
    JSVMTEST_CALL(OH_JSVM_AddHandlerForGC(vm, JSVM_CB_TRIGGER_BEFORE_GC, OnBeforeGC, JSVM_GC_TYPE_ALL, NULL));
    JSVMTEST_CALL(
        OH_JSVM_AddHandlerForGC(vm, JSVM_CB_TRIGGER_BEFORE_GC, OnBeforeGC2, JSVM_GC_TYPE_ALL, (void*)(&data)));
    JSVMTEST_CALL(OH_JSVM_AddHandlerForGC(vm, JSVM_CB_TRIGGER_BEFORE_GC, OnBeforeGC3, JSVM_GC_TYPE_SCAVENGE, NULL));
    JSVMTEST_CALL(OH_JSVM_AddHandlerForGC(vm, JSVM_CB_TRIGGER_AFTER_GC, OnAfterGC, JSVM_GC_TYPE_ALL, NULL));
    JSVMTEST_CALL(OH_JSVM_AddHandlerForGC(vm, JSVM_CB_TRIGGER_AFTER_GC, OnAfterGC2, JSVM_GC_TYPE_ALL, NULL));

    JSVMTEST_CALL(OH_JSVM_RemoveHandlerForGC(vm, JSVM_CB_TRIGGER_AFTER_GC, OnAfterGC2, NULL));

    // can not remove handler that has not been added.
    ASSERT_TRUE(OH_JSVM_RemoveHandlerForGC(vm, JSVM_CB_TRIGGER_AFTER_GC, OnAfterGC2, new int(12)) == JSVM_INVALID_ARG);

    jsvm::TryTriggerGC();
    ASSERT_TRUE(g_beforeFlag1);
    ASSERT_TRUE(g_beforeFlag2);
    ASSERT_FALSE(g_beforeFlag3);
    ASSERT_TRUE(g_afterFlag1);
    ASSERT_FALSE(g_afterFlag2);
}

HWTEST_F(JSVMTest, JSVMLowMemoryTriggerGC, TestSize.Level1)
{
    g_afterFlag3 = false;
    JSVMTEST_CALL(OH_JSVM_AddHandlerForGC(vm, JSVM_CB_TRIGGER_AFTER_GC, OnAfterGC3, JSVM_GC_TYPE_ALL, NULL));
    jsvm::TryLowMemoryGC();
    ASSERT_TRUE(g_afterFlag3);
}

HWTEST_F(JSVMTest, JSVMNegativeApplicationScenariosOfGCCB, TestSize.Level1)
{
    JSVMTEST_CALL(OH_JSVM_AddHandlerForGC(vm, JSVM_CB_TRIGGER_BEFORE_GC, OnBeforeGC, JSVM_GC_TYPE_ALL, NULL));
    // 1. Repeatedly register the same handler and native-data.
    ASSERT_TRUE(OH_JSVM_AddHandlerForGC(vm, JSVM_CB_TRIGGER_BEFORE_GC, OnBeforeGC, JSVM_GC_TYPE_ALL, NULL) ==
                JSVM_INVALID_ARG);
    // 2. Register an empty handler.
    ASSERT_TRUE(OH_JSVM_AddHandlerForGC(vm, JSVM_CB_TRIGGER_BEFORE_GC, NULL, JSVM_GC_TYPE_ALL, NULL) ==
                JSVM_INVALID_ARG);
    // 3. Remove unregistered handlers.
    ASSERT_TRUE(OH_JSVM_RemoveHandlerForGC(vm, JSVM_CB_TRIGGER_BEFORE_GC, OnBeforeGC2, NULL) == JSVM_INVALID_ARG);
    // 4. Remove the same handler and native-data repeatedly.
    ASSERT_TRUE(OH_JSVM_RemoveHandlerForGC(vm, JSVM_CB_TRIGGER_BEFORE_GC, OnBeforeGC, NULL) == JSVM_OK);
    ASSERT_TRUE(OH_JSVM_RemoveHandlerForGC(vm, JSVM_CB_TRIGGER_BEFORE_GC, OnBeforeGC, NULL) == JSVM_INVALID_ARG);
}

static bool g_oomHandlerFinished = false;
static bool g_fatalHandlerFinished = false;
static bool g_fatalHandlerFinished2 = false;
static bool g_promiseRejectWithNoHandlerFinished = false;
static bool g_promiseAddHandlerAfterRejectedFinished = false;
static bool g_promiseRejectAfterResolvedFinished = false;
static bool g_promiseResolveAfterResolvedFinished = false;

void OnOOMError(const char* location, const char* detail, bool isHeapOOM)
{
    g_oomHandlerFinished = true;
    longjmp(g_buf, 1);
}

void OnFatalError(const char* location, const char* message)
{
    g_fatalHandlerFinished = true;
    longjmp(g_buf, 1);
}

void OnFatalError2(const char* location, const char* message)
{
    g_fatalHandlerFinished2 = true;
    longjmp(g_buf, 1);
}

void OnPromiseReject(JSVM_Env env, JSVM_PromiseRejectEvent rejectEvent, JSVM_Value rejectInfo)
{
    ASSERT_TRUE(jsvm::IsObject(rejectInfo));
    auto promise = jsvm::GetProperty(rejectInfo, "promise");
    ASSERT_TRUE(jsvm::IsPromise(promise));
    if (rejectEvent == JSVM_PROMISE_ADD_HANDLER_AFTER_REJECTED) {
        auto value = jsvm::GetProperty(rejectInfo, "value");
        ASSERT_TRUE(jsvm::IsUndefined(value));
    } else {
        auto value = jsvm::GetProperty(rejectInfo, "value");
        ASSERT_TRUE(jsvm::IsNumber(value));
        const int32_t expectedNumber = 42;
        int32_t number;
        OH_JSVM_GetValueInt32(env, value, &number);
        ASSERT_EQ(number, expectedNumber);
    }

    switch (rejectEvent) {
        case JSVM_PROMISE_REJECT_WITH_NO_HANDLER:
            g_promiseRejectWithNoHandlerFinished = true;
            break;
        case JSVM_PROMISE_ADD_HANDLER_AFTER_REJECTED:
            g_promiseAddHandlerAfterRejectedFinished = true;
            break;
        case JSVM_PROMISE_REJECT_AFTER_RESOLVED:
            g_promiseRejectAfterResolvedFinished = true;
            break;
        case JSVM_PROMISE_RESOLVE_AFTER_RESOLVED:
            g_promiseResolveAfterResolvedFinished = true;
            break;
        default:
            break;
    }
}

HWTEST_F(JSVMTest, JSVMOOM, TestSize.Level1)
{
    g_oomHandlerFinished = false;
    JSVMTEST_CALL(OH_JSVM_SetHandlerForOOMError(vm, OnOOMError));
    static bool oomed = false;
    setjmp(g_buf);
    if (!oomed) {
        oomed = true;
        jsvm::TryTriggerOOM();
    }
    ASSERT_TRUE(g_oomHandlerFinished);
}

HWTEST_F(JSVMTest, JSVMFatalError, TestSize.Level1)
{
    g_fatalHandlerFinished = false;
    JSVMTEST_CALL(OH_JSVM_SetHandlerForFatalError(vm, OnFatalError));
    static bool fataled = false;
    setjmp(g_buf);
    if (!fataled) {
        fataled = true;
        jsvm::TryTriggerFatalError(vm);
    }
    ASSERT_TRUE(jsvm::ToNumber(jsvm::Run("42")) == 42);
    ASSERT_TRUE(g_fatalHandlerFinished);
}

HWTEST_F(JSVMTest, JSVMPromiseReject, TestSize.Level1)
{
    JSVMTEST_CALL(OH_JSVM_SetHandlerForPromiseReject(vm, OnPromiseReject));
    jsvm::Run("let p = new Promise((resolve, reject) => { reject(42); }); p.catch()");
    jsvm::Run("new Promise((resolve, reject) => { resolve(41); reject(42); })");
    jsvm::Run("new Promise((resolve, reject) => { resolve(41); resolve(42); })");
}

HWTEST_F(JSVMTest, JSVMCheckHandler, TestSize.Level1)
{
    ASSERT_TRUE(g_oomHandlerFinished);
    ASSERT_TRUE(g_fatalHandlerFinished);
    ASSERT_TRUE(g_promiseRejectWithNoHandlerFinished);
    ASSERT_TRUE(g_promiseAddHandlerAfterRejectedFinished);
    ASSERT_TRUE(g_promiseRejectAfterResolvedFinished);
    ASSERT_TRUE(g_promiseResolveAfterResolvedFinished);
}

static bool g_callAsFunctionFlag = false;
static bool g_setNamedPropertyFlag = false;
static bool g_callAsConstructorFlag = false;
static bool g_propertiesFlag = false;

static JSVM_Value SetNamedPropertyCbInfo2(JSVM_Env env,
                                          JSVM_Value name,
                                          JSVM_Value property,
                                          JSVM_Value thisArg,
                                          JSVM_Value data)
{
    g_setNamedPropertyFlag = true;
    return property;
}

static JSVM_Value Add(JSVM_Env env, JSVM_CallbackInfo info)
{
    g_propertiesFlag = true;
    size_t argc = 2;
    JSVM_Value args[2];
    OH_JSVM_GetCbInfo(env, info, &argc, args, NULL, NULL);
    double num1 = 0.0;
    double num2 = 0.0;
    OH_JSVM_GetValueDouble(env, args[0], &num1);
    OH_JSVM_GetValueDouble(env, args[1], &num2);
    JSVM_Value sum = nullptr;
    OH_JSVM_CreateDouble(env, num1 + num2, &sum);
    return sum;
}

JSVM_Value GenerateParentClass(JSVM_Env env)
{
    JSVM_Value parentClass = nullptr;
    JSVM_CallbackStruct* parentClassConstructor = new JSVM_CallbackStruct;
    parentClassConstructor->data = nullptr;
    parentClassConstructor->callback = [](JSVM_Env env, JSVM_CallbackInfo info) -> JSVM_Value {
        JSVM_Value thisVar = nullptr;
        OH_JSVM_GetCbInfo(env, info, nullptr, nullptr, &thisVar, nullptr);
        return thisVar;
    };
    JSVM_Value fooVal = jsvm::Str("bar");
    JSVM_PropertyDescriptor des[2];
    des[0] = {
        .utf8name = "foo",
        .value = fooVal,
    };
    JSVM_CallbackStruct* addMethod = new JSVM_CallbackStruct;
    addMethod->data = nullptr;
    addMethod->callback = Add;
    des[1] = {
        .utf8name = "add",
        .method = addMethod,
    };
    JSVM_DefineClassOptions options[1];
    options[0].id = JSVM_DEFINE_CLASS_WITH_COUNT;
    options[0].content.num = JSVM_OBJECT_INTERFAIELD_COUNT;
    JSVMTEST_CALL(OH_JSVM_DefineClassWithOptions(env, "parentClass", JSVM_AUTO_LENGTH, parentClassConstructor,
                                                 JSVM_PARENT_CLASS_DES_COUNT, des, nullptr, 1, options, &parentClass));
    return parentClass;
}

JSVM_Value GenerateSubClass(JSVM_Env env, JSVM_Value parentClass)
{
    JSVM_Value subClass = nullptr;
    JSVM_CallbackStruct* subClassConstructor = new JSVM_CallbackStruct;
    subClassConstructor->data = nullptr;
    subClassConstructor->callback = [](JSVM_Env env, JSVM_CallbackInfo info) -> JSVM_Value {
        JSVM_Value thisVar = nullptr;
        g_callAsConstructorFlag = true;
        OH_JSVM_GetCbInfo(env, info, nullptr, nullptr, &thisVar, nullptr);
        return thisVar;
    };
    JSVM_DefineClassOptions subOptions[2];
    JSVM_CallbackStruct* callAsFuncParam = new JSVM_CallbackStruct;
    callAsFuncParam->data = nullptr;
    callAsFuncParam->callback = [](JSVM_Env env, JSVM_CallbackInfo info) -> JSVM_Value {
        JSVM_Value thisVar = nullptr;
        g_callAsFunctionFlag = true;
        OH_JSVM_GetCbInfo(env, info, nullptr, nullptr, &thisVar, nullptr);
        return thisVar;
    };
    propertyCfg.genericNamedPropertySetterCallback = SetNamedPropertyCbInfo2;
    JSVM_PropertyHandler propertyHandler = {
        .propertyHandlerCfg = &propertyCfg,
        .callAsFunctionCallback = callAsFuncParam,
    };
    subOptions[0].id = JSVM_DEFINE_CLASS_WITH_COUNT;
    subOptions[0].content.num = JSVM_OBJECT_INTERFAIELD_COUNT;
    subOptions[1].id = JSVM_DEFINE_CLASS_WITH_PROPERTY_HANDLER;
    subOptions[1].content.ptr = &propertyHandler;
    JSVMTEST_CALL(OH_JSVM_DefineClassWithOptions(env, "subClass", JSVM_AUTO_LENGTH, subClassConstructor, 0, nullptr,
                                                 parentClass, JSVM_DEFINE_CLASS_OPTIONS_COUNT, subOptions, &subClass));
    return subClass;
}

/**
 * @brief Verify the validity of the following parameters in the OH_JSVM_DefineClassWithOptions interface:
 * 'consturctor' | 'properties' | 'parentClass' | 'options'
 */
HWTEST_F(JSVMTest, JSVMTestParentClassWithCount, TestSize.Level1)
{
    g_callAsFunctionFlag = false;
    g_setNamedPropertyFlag = false;
    g_callAsConstructorFlag = false;
    g_propertiesFlag = false;
    // 1. Define parent-class.
    JSVM_Value parentClass = GenerateParentClass(env);
    // 2. Define sub-class.
    JSVM_Value subClass = GenerateSubClass(env, parentClass);
    // 3. Verify the validity of 'constructor'.
    JSVM_Value subInstance;
    ASSERT_FALSE(g_callAsConstructorFlag);
    JSVMTEST_CALL(OH_JSVM_NewInstance(env, subClass, 0, nullptr, &subInstance));
    ASSERT_TRUE(g_callAsConstructorFlag);

    JSVM_Value globalVal;
    OH_JSVM_GetGlobal(env, &globalVal);
    OH_JSVM_SetNamedProperty(env, globalVal, "obj", subInstance);

    // 4. Verify the validity of 'parentClass'.
    JSVM_Value subRes = nullptr;
    JSVMTEST_CALL(OH_JSVM_GetNamedProperty(env, subInstance, "foo", &subRes));
    ASSERT_TRUE(jsvm::ToString(subRes).compare("bar") == 0);
    // 5. Verify the validity of 'properties'.
    ASSERT_FALSE(g_propertiesFlag);
    jsvm::Run("obj.add(3, 4);");
    ASSERT_TRUE(g_propertiesFlag);
    // 6. Verify the validity of 'options'.
    ASSERT_FALSE(g_callAsFunctionFlag);
    jsvm::Run("obj()");
    ASSERT_TRUE(g_callAsFunctionFlag);
    ASSERT_FALSE(g_setNamedPropertyFlag);
    jsvm::Run("obj.x = 123;");
    ASSERT_TRUE(g_setNamedPropertyFlag);
    propertyCfg.genericNamedPropertySetterCallback = nullptr;
}

/**
 * @brief If parentClass is not an APIFunction, the status code is JSVM_INVALID_ARG.
 */
HWTEST_F(JSVMTest, NonAPIFunction, TestSize.Level1)
{
    JSVM_Value script = jsvm::Str("return 2 + 3;");
    JSVM_Value nonAPIFunction = nullptr;
    JSVMTEST_CALL(OH_JSVM_CreateFunctionWithScript(env, nullptr, 0, 0, nullptr, script, &nonAPIFunction));

    JSVM_Value subClass = nullptr;
    JSVM_CallbackStruct constructor;
    constructor.data = nullptr;
    constructor.callback = [](JSVM_Env env, JSVM_CallbackInfo info) -> JSVM_Value {
        JSVM_Value thisVar = nullptr;
        OH_JSVM_GetCbInfo(env, info, nullptr, nullptr, &thisVar, nullptr);
        return thisVar;
    };
    JSVM_Status status = OH_JSVM_DefineClassWithOptions(env, "subClass", JSVM_AUTO_LENGTH, &constructor, 0, nullptr,
                                                        nonAPIFunction, 0, nullptr, &subClass);
    ASSERT_TRUE(status == JSVM_INVALID_ARG);
}

static JSVM_Value LogFunc(JSVM_Env env, JSVM_CallbackInfo info)
{
    size_t argc = 1;
    JSVM_Value argv[1];
    JSVMTEST_CALL(OH_JSVM_GetCbInfo(env, info, &argc, argv, NULL, NULL));
    return jsvm::Str("log");
}

HWTEST_F(JSVMTest, test_function_private, TestSize.Level1)
{
    JSVM_Data key;
    JSVMTEST_CALL(OH_JSVM_CreatePrivate(env, nullptr, &key));
    auto obj = jsvm::Object();
    JSVMTEST_CALL(OH_JSVM_SetPrivate(env, obj, key, jsvm::Str("a")));
    static JSVM_CallbackStruct cb = { LogFunc, NULL };
    JSVM_Value log;
    JSVMTEST_CALL(OH_JSVM_CreateFunction(env, "log", JSVM_AUTO_LENGTH, &cb, &log));
    JSVMTEST_CALL(OH_JSVM_SetPrivate(env, obj, key, log));
    JSVM_Value result;
    JSVMTEST_CALL(OH_JSVM_GetPrivate(env, obj, key, &result));
    auto ret = jsvm::Call(result);
    ASSERT_TRUE(jsvm::Equals(ret, jsvm::Str("log")));
}

HWTEST_F(JSVMTest, test_str_private, TestSize.Level1)
{
    JSVM_Data key;
    JSVMTEST_CALL(OH_JSVM_CreatePrivate(env, nullptr, &key));
    auto obj = jsvm::Object();
    JSVMTEST_CALL(OH_JSVM_SetPrivate(env, obj, key, jsvm::Str("a")));
    JSVMTEST_CALL(OH_JSVM_SetPrivate(env, obj, key, jsvm::Str("log")));
    JSVM_Value result;
    JSVMTEST_CALL(OH_JSVM_GetPrivate(env, obj, key, &result));
    ASSERT_TRUE(jsvm::Equals(result, jsvm::Str("log")));
}

HWTEST_F(JSVMTest, test_different_private_with_same_key, TestSize.Level1)
{
    JSVM_Data key;
    JSVMTEST_CALL(OH_JSVM_CreatePrivate(env, jsvm::Str("a"), &key));
    JSVM_Data key1;
    JSVMTEST_CALL(OH_JSVM_CreatePrivate(env, jsvm::Str("a"), &key1));
    auto obj = jsvm::Object();
    JSVMTEST_CALL(OH_JSVM_SetPrivate(env, obj, key, jsvm::Str("a")));
    JSVMTEST_CALL(OH_JSVM_SetPrivate(env, obj, key1, jsvm::Str("b")));
    JSVM_Value result;
    JSVMTEST_CALL(OH_JSVM_GetPrivate(env, obj, key, &result));
    JSVM_Value result1;
    JSVMTEST_CALL(OH_JSVM_GetPrivate(env, obj, key1, &result1));
    ASSERT_TRUE(!jsvm::Equals(result, result1));
}

HWTEST_F(JSVMTest, test_set_private_with_nullptr_key, TestSize.Level1)
{
    JSVM_Data key = nullptr;
    auto obj = jsvm::Object();
    auto status = OH_JSVM_SetPrivate(env, obj, key, jsvm::Str("a"));
    ASSERT_TRUE(status == JSVM_INVALID_ARG);
}

HWTEST_F(JSVMTest, test_set_private_with_nullptr_value, TestSize.Level1)
{
    JSVM_Data key = nullptr;
    auto obj = jsvm::Object();
    JSVMTEST_CALL(OH_JSVM_CreatePrivate(env, jsvm::Str("a"), &key));
    auto status = OH_JSVM_SetPrivate(env, obj, key, nullptr);
    ASSERT_TRUE(status == JSVM_INVALID_ARG);
}

HWTEST_F(JSVMTest, test_set_private_with_non_object, TestSize.Level1)
{
    JSVM_Data key = nullptr;
    auto obj = jsvm::True();
    JSVMTEST_CALL(OH_JSVM_CreatePrivate(env, jsvm::Str("a"), &key));
    auto status = OH_JSVM_SetPrivate(env, obj, key, jsvm::Str("a"));
    ASSERT_TRUE(status == JSVM_OBJECT_EXPECTED);
}

HWTEST_F(JSVMTest, test_set_private_with_non_private, TestSize.Level1)
{
    JSVM_Data key = nullptr;
    auto obj = jsvm::Object();
    JSVMTEST_CALL(OH_JSVM_CreateSymbol(env, jsvm::Str("a"), (JSVM_Value*)&key));
    auto status = OH_JSVM_SetPrivate(env, obj, key, jsvm::Str("a"));
    ASSERT_TRUE(status == JSVM_INVALID_ARG);
}

HWTEST_F(JSVMTest, test_get_non_exist_private, TestSize.Level1)
{
    JSVM_Data key;
    JSVMTEST_CALL(OH_JSVM_CreatePrivate(env, nullptr, &key));
    auto obj = jsvm::Object();
    JSVM_Value result;
    JSVMTEST_CALL(OH_JSVM_GetPrivate(env, obj, key, &result));
    ASSERT_TRUE(jsvm::Equals(result, jsvm::Undefined()));
}

HWTEST_F(JSVMTest, test_get_private_with_nullptr_key, TestSize.Level1)
{
    JSVM_Data key = nullptr;
    auto obj = jsvm::Object();
    JSVM_Value value;
    auto status = OH_JSVM_GetPrivate(env, obj, key, &value);
    ASSERT_TRUE(status == JSVM_INVALID_ARG);
}

HWTEST_F(JSVMTest, test_get_private_with_nullptr_value, TestSize.Level1)
{
    JSVM_Data key = nullptr;
    auto obj = jsvm::Object();
    JSVMTEST_CALL(OH_JSVM_CreatePrivate(env, jsvm::Str("a"), &key));
    auto status = OH_JSVM_GetPrivate(env, obj, key, nullptr);
    ASSERT_TRUE(status == JSVM_INVALID_ARG);
}

HWTEST_F(JSVMTest, test_get_private_with_non_object, TestSize.Level1)
{
    JSVM_Data key = nullptr;
    auto obj = jsvm::True();
    JSVMTEST_CALL(OH_JSVM_CreatePrivate(env, jsvm::Str("a"), &key));
    JSVM_Value value;
    auto status = OH_JSVM_GetPrivate(env, obj, key, &value);
    ASSERT_TRUE(status == JSVM_OBJECT_EXPECTED);
}

HWTEST_F(JSVMTest, test_get_private_with_non_private, TestSize.Level1)
{
    JSVM_Data key = nullptr;
    auto obj = jsvm::Object();
    JSVMTEST_CALL(OH_JSVM_CreateSymbol(env, jsvm::Str("a"), (JSVM_Value*)&key));
    JSVM_Value value;
    auto status = OH_JSVM_GetPrivate(env, obj, key, &value);
    ASSERT_TRUE(status == JSVM_INVALID_ARG);
}

HWTEST_F(JSVMTest, test_delete_non_exist_private, TestSize.Level1)
{
    JSVM_Data key;
    JSVMTEST_CALL(OH_JSVM_CreatePrivate(env, nullptr, &key));
    auto obj = jsvm::Object();
    JSVMTEST_CALL(OH_JSVM_DeletePrivate(env, obj, key));
}

HWTEST_F(JSVMTest, test_delete_nullptr_key, TestSize.Level1)
{
    JSVM_Data key = nullptr;
    auto obj = jsvm::Object();
    auto status = OH_JSVM_DeletePrivate(env, obj, key);
    ASSERT_TRUE(status == JSVM_INVALID_ARG);
}

HWTEST_F(JSVMTest, test_delete_with_non_object, TestSize.Level1)
{
    JSVM_Data key = nullptr;
    auto obj = jsvm::True();
    JSVMTEST_CALL(OH_JSVM_CreatePrivate(env, jsvm::Str("a"), &key));
    auto status = OH_JSVM_DeletePrivate(env, obj, key);
    ASSERT_TRUE(status == JSVM_OBJECT_EXPECTED);
}

HWTEST_F(JSVMTest, test_delete_private_with_non_private, TestSize.Level1)
{
    JSVM_Data key = nullptr;
    auto obj = jsvm::Object();
    JSVMTEST_CALL(OH_JSVM_CreateSymbol(env, jsvm::Str("a"), (JSVM_Value*)&key));
    auto status = OH_JSVM_DeletePrivate(env, obj, key);
    ASSERT_TRUE(status == JSVM_INVALID_ARG);
}

HWTEST_F(JSVMTest, test_set_debug_option1, TestSize.Level1)
{
    JSVM_HandleScope handleScope1, handleScope2, handleScope3;
    int num = 100;
    bool boolValue = false;
    JSVM_Value array1[num], array2[num], array3[num];
    auto status1 = OH_JSVM_SetDebugOption(env, JSVM_SCOPE_CHECK, true);
    ASSERT_TRUE(status1 == JSVM_OK);
    OH_JSVM_OpenHandleScope(env, &handleScope1);
    for (int i = 0; i < num; i++) {
        OH_JSVM_GetBoolean(env, false, &array1[i]);
        OH_JSVM_IsBoolean(env, array1[i], &boolValue);
    }
    OH_JSVM_OpenHandleScope(env, &handleScope2);
    for (int i = 0; i < num; i++) {
        OH_JSVM_GetBoolean(env, false, &array2[i]);
        OH_JSVM_IsBoolean(env, array2[i], &boolValue);
    }
    OH_JSVM_OpenHandleScope(env, &handleScope3);
    for (int i = 0; i < num; i++) {
        OH_JSVM_GetBoolean(env, false, &array3[i]);
        OH_JSVM_IsBoolean(env, array3[i], &boolValue);
        OH_JSVM_IsBoolean(env, array2[i], &boolValue);
    }
    OH_JSVM_CloseHandleScope(env, handleScope3);
    OH_JSVM_CloseHandleScope(env, handleScope2);
    OH_JSVM_CloseHandleScope(env, handleScope1);
    auto status2 = OH_JSVM_SetDebugOption(env, JSVM_SCOPE_CHECK, false);
    ASSERT_TRUE(status2 == JSVM_OK);
}

static bool g_fatalErrorFinished = false;

static void HandleAbort(int sig)
{
    g_fatalErrorFinished = true;
    longjmp(g_buf, 1);
}

HWTEST_F(JSVMTest, test_set_debug_option2, TestSize.Level1)
{
    JSVM_HandleScope handleScope;
    JSVM_Value result;
    bool boolValue = false;
    auto status1 = OH_JSVM_SetDebugOption(env, JSVM_SCOPE_CHECK, true);
    ASSERT_TRUE(status1 == JSVM_OK);
    OH_JSVM_OpenHandleScope(env, &handleScope);
    OH_JSVM_GetBoolean(env, true, &result);
    OH_JSVM_CloseHandleScope(env, handleScope);
    signal(SIGABRT, HandleAbort);
    static bool fataled = false;
    setjmp(g_buf);
    if (!fataled) {
        fataled = true;
        OH_JSVM_IsBoolean(env, result, &boolValue);
    }
    auto status2 = OH_JSVM_SetDebugOption(env, JSVM_SCOPE_CHECK, false);
    ASSERT_TRUE(status2 == JSVM_OK);
    ASSERT_TRUE(g_fatalErrorFinished);
}

HWTEST_F(JSVMTest, test_set_debug_option3, TestSize.Level1)
{
    JSVM_HandleScope handleScope;
    JSVM_Value val, escapeVal;
    bool boolValue = false;
    auto status1 = OH_JSVM_SetDebugOption(env, JSVM_SCOPE_CHECK, true);
    ASSERT_TRUE(status1 == JSVM_OK);
    OH_JSVM_OpenHandleScope(env, &handleScope);
    JSVM_EscapableHandleScope scope = nullptr;
    OH_JSVM_OpenEscapableHandleScope(env, &scope);
    OH_JSVM_GetBoolean(env, true, &val);
    OH_JSVM_EscapeHandle(env, scope, val, &escapeVal);
    OH_JSVM_CloseEscapableHandleScope(env, scope);
    OH_JSVM_IsBoolean(env, escapeVal, &boolValue);
    OH_JSVM_CloseHandleScope(env, handleScope);
    auto status2 = OH_JSVM_SetDebugOption(env, JSVM_SCOPE_CHECK, false);
    ASSERT_TRUE(status2 == JSVM_OK);
}

HWTEST_F(JSVMTest, JSVMCloseHandleScopeUAF, TestSize.Level1)
{
    JSVM_HandleScope handle = nullptr;
    JSVMTEST_CALL(OH_JSVM_OpenHandleScope(env, &handle));

    JSVM_Value jsSrc = nullptr;
    JSVMTEST_CALL(OH_JSVM_CreateStringUtf8(env, srcProf.c_str(), srcProf.size(), &jsSrc));

    bool cacheRejected = true;
    JSVM_Script script = nullptr;
    JSVMTEST_CALL(OH_JSVM_CompileScript(env, jsSrc, nullptr, 0, true, &cacheRejected, &script));

    JSVMTEST_CALL(OH_JSVM_RetainScript(env, script));
    JSVMTEST_CALL(OH_JSVM_ReleaseScript(env, script));

    const int length = 32;
    char* data = new((char *)script) char[length];
    memset(data, 0, length);

    JSVMTEST_CALL(OH_JSVM_CloseHandleScope(env, handle));
}