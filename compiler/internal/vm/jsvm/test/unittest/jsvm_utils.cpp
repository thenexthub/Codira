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
#include "jsvm_utils.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <string>
#include <vector>

#include "jsvm.h"

void Print(const char* str)
{
    printf("%s\n", str);
}

void PrintException(JSVM_Env env, JSVM_Value exception, const char* call)
{
    bool isObject = false;
    JSVM_Status status = OH_JSVM_IsObject(env, exception, &isObject);
    ASSERT_EQ(status, JSVM_OK);
    ASSERT_TRUE(isObject);

    JSVM_Value stack;
    status = OH_JSVM_GetNamedProperty(env, exception, "stack", &stack);
    ASSERT_EQ(status, JSVM_OK);
    char stackStr[BUF_SIZE];
    OH_JSVM_GetValueStringUtf8(env, stack, stackStr, BUF_SIZE, nullptr);
    printf("[PrintException] exception.stack: %s\n", stackStr);

    status = OH_JSVM_GetNamedProperty(env, exception, "message", &stack);
    ASSERT_EQ(status, JSVM_OK);
    char messageStr[BUF_SIZE];
    OH_JSVM_GetValueStringUtf8(env, stack, messageStr, BUF_SIZE, nullptr);
    printf("[PrintException] exception.message: %s\n", messageStr);
}

void CheckErrorAndException(JSVM_Env env, JSVM_Status returnStatus, const char* call)
{
    const JSVM_ExtendedErrorInfo* errorInfo;
    JSVM_Status status = OH_JSVM_GetLastErrorInfo(env, &errorInfo);
    ASSERT_EQ(status, JSVM_OK);
    ASSERT_EQ(returnStatus, errorInfo->errorCode);
    bool isPending = false;
    status = OH_JSVM_IsExceptionPending(env, &isPending);
    ASSERT_EQ(status, JSVM_OK);
    JSVM_Value exception;
    status = OH_JSVM_GetAndClearLastException(env, &exception);
    ASSERT_EQ(status, JSVM_OK);
    bool isExceptionUndefined = false;
    status = OH_JSVM_IsUndefined(env, exception, &isExceptionUndefined);
    ASSERT_EQ(status, JSVM_OK);
    bool hasException = !isExceptionUndefined;
    if (hasException) {
        ASSERT_TRUE(isPending);
        ASSERT_NE(returnStatus, JSVM_OK);
        PrintException(env, exception, call);
        ASSERT_TRUE(false);
    } else {
        // no exception
        ASSERT_FALSE(isPending);
    }
    ASSERT_EQ(returnStatus, JSVM_OK);
}

// jsvm utils
namespace jsvm {

JSVM_Value Str(const char* s)
{
    JSVM_Value str;
    JSVMTEST_CALL(OH_JSVM_CreateStringUtf8(jsvm_env, s, JSVM_AUTO_LENGTH, &str));
    return str;
}

JSVM_Value Str(const std::string& stdString)
{
    return Str(stdString.c_str());
}

JSVM_Script Compile(JSVM_Value js_str, const uint8_t* cache, size_t size)
{
    JSVM_Script script;
    JSVMTEST_CALL(OH_JSVM_CompileScript(jsvm_env, js_str, cache, size, false, nullptr, &script));
    return script;
}

JSVM_Script CompileWithName(JSVM_Value js_str, const char* name)
{
    JSVM_Script script;
    JSVM_ScriptOrigin origin = {
        .sourceMapUrl = nullptr,
        .resourceName = name,
    };
    JSVMTEST_CALL(
        OH_JSVM_CompileScriptWithOrigin(jsvm_env, js_str, NULL, JSVM_AUTO_LENGTH, false, NULL, &origin, &script));
    return script;
}

JSVM_Script Compile(const char* s, const uint8_t* cache, size_t size)
{
    return Compile(Str(s), cache, size);
}

JSVM_Script CompileWithName(const char* s, const char* name)
{
    return CompileWithName(Str(s), name);
}

JSVM_Value Run(JSVM_Script script)
{
    JSVM_Value result;
    JSVMTEST_CALL(OH_JSVM_RunScript(jsvm_env, script, &result));
    return result;
}

JSVM_Value Run(const char* s)
{
    return Run(Compile(s));
}

bool ToBoolean(JSVM_Value val)
{
    JSVM_Value js_boolean;
    JSVMTEST_CALL(OH_JSVM_CoerceToBool(jsvm_env, val, &js_boolean));
    bool res;
    JSVMTEST_CALL(OH_JSVM_GetValueBool(jsvm_env, js_boolean, &res));
    return res;
}

double ToNumber(JSVM_Value val)
{
    JSVM_Value js_number;
    JSVMTEST_CALL(OH_JSVM_CoerceToNumber(jsvm_env, val, &js_number));
    double res;
    JSVMTEST_CALL(OH_JSVM_GetValueDouble(jsvm_env, js_number, &res));
    return res;
}

std::string ToString(JSVM_Value val)
{
    JSVM_Value js_string;
    JSVMTEST_CALL(OH_JSVM_CoerceToString(jsvm_env, val, &js_string));
    size_t length = 0;
    JSVMTEST_CALL(OH_JSVM_GetValueStringUtf8(jsvm_env, js_string, NULL, 0, &length));
    size_t capacity = length + 1;
    char* buffer = new char[capacity];
    size_t copyLength = 0;
    JSVMTEST_CALL(OH_JSVM_GetValueStringUtf8(jsvm_env, js_string, buffer, capacity, &copyLength));
    std::string str(buffer);
    delete[] buffer;
    return str;
}

JSVM_Value True()
{
    JSVM_Value js_true;
    JSVMTEST_CALL(OH_JSVM_GetBoolean(jsvm_env, true, &js_true));
    return js_true;
}

JSVM_Value False()
{
    JSVM_Value js_false;
    JSVMTEST_CALL(OH_JSVM_GetBoolean(jsvm_env, false, &js_false));
    return js_false;
}

JSVM_Value Undefined()
{
    JSVM_Value js_undefined;
    JSVMTEST_CALL(OH_JSVM_GetUndefined(jsvm_env, &js_undefined));
    return js_undefined;
}

JSVM_Value Null()
{
    JSVM_Value js_null;
    JSVMTEST_CALL(OH_JSVM_GetNull(jsvm_env, &js_null));
    return js_null;
}

JSVM_Value Int32(int32_t v)
{
    JSVM_Value result;
    JSVMTEST_CALL(OH_JSVM_CreateInt32(jsvm_env, v, &result));
    return result;
}

JSVM_Value Int64(int64_t v)
{
    JSVM_Value result;
    JSVMTEST_CALL(OH_JSVM_CreateInt64(jsvm_env, v, &result));
    return result;
}

JSVM_Value Uint32(uint32_t v)
{
    JSVM_Value result;
    JSVMTEST_CALL(OH_JSVM_CreateUint32(jsvm_env, v, &result));
    return result;
}

JSVM_Value Double(double v)
{
    JSVM_Value result;
    JSVMTEST_CALL(OH_JSVM_CreateDouble(jsvm_env, v, &result));
    return result;
}

JSVM_Value Object()
{
    JSVM_Value js_object;
    JSVMTEST_CALL(OH_JSVM_CreateObject(jsvm_env, &js_object));
    return js_object;
}

JSVM_Value Global()
{
    JSVM_Value js_global;
    JSVMTEST_CALL(OH_JSVM_GetGlobal(jsvm_env, &js_global));
    return js_global;
}

// object property
JSVM_Value GetProperty(JSVM_Value object, JSVM_Value key)
{
    JSVM_Value result;
    JSVMTEST_CALL(OH_JSVM_GetProperty(jsvm_env, object, key, &result));
    return result;
}

JSVM_Value GetProperty(JSVM_Value object, const char* name)
{
    return GetProperty(object, Str(name));
}

void SetProperty(JSVM_Value object, JSVM_Value key, JSVM_Value value)
{
    JSVMTEST_CALL(OH_JSVM_SetProperty(jsvm_env, object, key, value));
}

void SetProperty(JSVM_Value object, const char* name, JSVM_Value value)
{
    JSVMTEST_CALL(OH_JSVM_SetProperty(jsvm_env, object, Str(name), value));
}

// Get named property with name `name` from globalThis
JSVM_Value Global(const char* name)
{
    return GetProperty(Global(), Str(name));
}

// Get property with key `key` from globalThis
JSVM_Value Global(JSVM_Value key)
{
    return GetProperty(Global(), key);
}

// Call function with normal function with empty argument list
JSVM_Value Call(JSVM_Value func)
{
    JSVM_Value result;
    JSVMTEST_CALL(OH_JSVM_CallFunction(jsvm_env, Undefined(), func, 0, NULL, &result));
    return result;
}

// Call function with normal function with specified argument list
JSVM_Value Call(JSVM_Value func, JSVM_Value thisArg, std::initializer_list<JSVM_Value> args)
{
    JSVM_Value result;
    int argc = args.size();
    if (argc == 0) {
        JSVMTEST_CALL(OH_JSVM_CallFunction(jsvm_env, thisArg, func, 0, NULL, &result));
        return result;
    }
    JSVM_Value argv[argc];
    size_t i = 0;
    for (JSVM_Value a : args) {
        argv[i++] = a;
    }
    JSVMTEST_CALL(OH_JSVM_CallFunction(jsvm_env, thisArg, func, argc, argv, &result));
    return result;
}

// Call function as constructor with empty argument list
JSVM_Value New(JSVM_Value constructor)
{
    JSVM_Value result;
    JSVMTEST_CALL(OH_JSVM_NewInstance(jsvm_env, constructor, 0, NULL, &result));
    return result;
}

bool InstanceOf(JSVM_Value value, JSVM_Value constructor)
{
    bool result;
    JSVMTEST_CALL(OH_JSVM_Instanceof(jsvm_env, value, constructor, &result));
    return result;
}

bool IsNull(JSVM_Value value)
{
    bool result;
    JSVMTEST_CALL(OH_JSVM_IsNull(jsvm_env, value, &result));
    return result;
}

bool IsUndefined(JSVM_Value value)
{
    bool result;
    JSVMTEST_CALL(OH_JSVM_IsUndefined(jsvm_env, value, &result));
    return result;
}

bool IsNullOrUndefined(JSVM_Value value)
{
    bool result;
    JSVMTEST_CALL(OH_JSVM_IsNullOrUndefined(jsvm_env, value, &result));
    return result;
}

bool IsBoolean(JSVM_Value value)
{
    bool result;
    JSVMTEST_CALL(OH_JSVM_IsBoolean(jsvm_env, value, &result));
    return result;
}

bool IsTrue(JSVM_Value value)
{
    return IsBoolean(value) && ToBoolean(value) == true;
}

bool IsFalse(JSVM_Value value)
{
    return IsBoolean(value) && ToBoolean(value) == false;
}

bool IsNumber(JSVM_Value value)
{
    bool result;
    JSVMTEST_CALL(OH_JSVM_IsNumber(jsvm_env, value, &result));
    return result;
}

bool IsString(JSVM_Value value)
{
    bool result;
    JSVMTEST_CALL(OH_JSVM_IsString(jsvm_env, value, &result));
    return result;
}

bool IsObject(JSVM_Value value)
{
    bool result;
    JSVMTEST_CALL(OH_JSVM_IsObject(jsvm_env, value, &result));
    return result;
}

bool IsBigInt(JSVM_Value value)
{
    bool result;
    JSVMTEST_CALL(OH_JSVM_IsBigInt(jsvm_env, value, &result));
    return result;
}

bool IsSymbol(JSVM_Value value)
{
    bool result;
    JSVMTEST_CALL(OH_JSVM_IsSymbol(jsvm_env, value, &result));
    return result;
}

bool IsFunction(JSVM_Value value)
{
    bool result;
    JSVMTEST_CALL(OH_JSVM_IsFunction(jsvm_env, value, &result));
    return result;
}

bool IsArray(JSVM_Value value)
{
    bool result;
    JSVMTEST_CALL(OH_JSVM_IsArray(jsvm_env, value, &result));
    return result;
}

bool IsArraybuffer(JSVM_Value value)
{
    bool result;
    JSVMTEST_CALL(OH_JSVM_IsArraybuffer(jsvm_env, value, &result));
    return result;
}

bool IsPromise(JSVM_Value value)
{
    bool result;
    JSVMTEST_CALL(OH_JSVM_IsPromise(jsvm_env, value, &result));
    return result;
}

bool IsWasmModuleObject(JSVM_Value value)
{
    bool result;
#ifdef TEST_WASM
    JSVMTEST_CALL(OH_JSVM_IsWasmModuleObject(jsvm_env, value, &result));
#else
    auto Module = jsvm::GetProperty(jsvm::Global("WebAssembly"), "Module");
    result = jsvm::InstanceOf(value, Module);
#endif
    return result;
}

bool IsWebAssemblyInstance(JSVM_Value value)
{
    auto Instance = jsvm::GetProperty(jsvm::Global("WebAssembly"), "Instance");
    return jsvm::InstanceOf(value, Instance);
}

bool IsWebAssemblyMemory(JSVM_Value value)
{
    auto Memory = jsvm::GetProperty(jsvm::Global("WebAssembly"), "Memory");
    return jsvm::InstanceOf(value, Memory);
}

bool IsWebAssemblyTable(JSVM_Value value)
{
    auto Table = jsvm::GetProperty(jsvm::Global("WebAssembly"), "Table");
    return jsvm::InstanceOf(value, Table);
}

JSVM_Value ArrayAt(JSVM_Value array, uint32_t index)
{
    JSVM_Value element;
    JSVMTEST_CALL(OH_JSVM_GetElement(jsvm_env, array, index, &element));
    return element;
}

JSVM_Value JsonParse(JSVM_Value str)
{
    JSVM_Value obj;
    JSVMTEST_CALL(OH_JSVM_JsonParse(jsvm_env, str, &obj));
    return obj;
}

JSVM_Value JsonStringify(JSVM_Value obj)
{
    JSVM_Value str;
    JSVMTEST_CALL(OH_JSVM_JsonStringify(jsvm_env, obj, &str));
    return str;
}

bool Equals(JSVM_Value lhs, JSVM_Value rhs)
{
    bool result;
    JSVMTEST_CALL(OH_JSVM_Equals(jsvm_env, lhs, rhs, &result));
    return result;
}

bool StrictEquals(JSVM_Value lhs, JSVM_Value rhs)
{
    bool result;
    JSVMTEST_CALL(OH_JSVM_StrictEquals(jsvm_env, lhs, rhs, &result));
    return result;
}

// This is a simple log function
JSVM_Value MyConsoleLog(JSVM_Env env, JSVM_CallbackInfo info)
{
    size_t argc = 1;
    JSVM_Value argv[1];
    JSVMTEST_CALL(OH_JSVM_GetCbInfo(env, info, &argc, argv, NULL, NULL));
    JSVM_Value x = argv[0];
    std::string str = jsvm::ToString(x);
    Print(str.c_str());
    return jsvm::Undefined();
}

void InstallMyConsoleLog(JSVM_Env env)
{
    static JSVM_CallbackStruct cb = { MyConsoleLog, NULL };
    JSVM_Value log;
    JSVMTEST_CALL(OH_JSVM_CreateFunction(env, "log", JSVM_AUTO_LENGTH, &cb, &log));
    ASSERT_TRUE(jsvm::IsFunction(log));
    auto console = jsvm::Global("console");
    if (!jsvm::IsObject(console)) {
        JSVMTEST_CALL(OH_JSVM_CreateObject(env, &console));
        jsvm::SetProperty(jsvm::Global(), "console", console);
    }
    jsvm::SetProperty(console, "log", log);
}

void TryTriggerOOM()
{
    static std::vector<JSVM_Value> arrayVec;
    int loopCount = 10000;
    for (int i = 0; i < loopCount; i++) {
        JSVM_Value array;
        JSVMTEST_CALL(OH_JSVM_CreateArrayWithLength(jsvm_env, 0xffffff, &array));
        arrayVec.push_back(array);
    }
}

void TryTriggerFatalError(JSVM_VM vm)
{
    OH_JSVM_DestroyVM(vm);
}

void TryTriggerGC()
{
    JSVMTEST_CALL(OH_JSVM_MemoryPressureNotification(jsvm_env, JSVM_MEMORY_PRESSURE_LEVEL_CRITICAL));
}

void TryLowMemoryGC()
{
    JSVMTEST_CALL(OH_JSVM_MemoryPressureNotification(jsvm_env, JSVM_MEMORY_PRESSURE_LEVEL_LOW_MEMORY));
}
} // namespace jsvm
