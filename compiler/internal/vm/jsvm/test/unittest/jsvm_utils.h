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
#ifndef JSVM_UTILS_H
#define JSVM_UTILS_H

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>

#include "gtest/gtest.h"
#include "jsvm.h"

extern JSVM_Env jsvm_env;

#define BUF_SIZE 256

void PrintException(JSVM_Env env, JSVM_Value exception, const char* call);
void CheckErrorAndException(JSVM_Env env, JSVM_Status returnStatus, const char* call);

#define JSVMTEST_CALL(the_call)                              \
    do {                                                     \
        JSVM_Status status = (the_call);                     \
        CheckErrorAndException(jsvm_env, status, #the_call); \
    } while (0)

// jsvm utils
namespace jsvm {

class HandleScope {
public:
    explicit HandleScope(JSVM_Env jsvmEnv) : env(jsvmEnv)
    {
        OH_JSVM_OpenHandleScope(env, &scope);
    }
    ~HandleScope()
    {
        OH_JSVM_CloseHandleScope(env, scope);
    }

private:
    JSVM_Env env;
    JSVM_HandleScope scope;
};

class Value {
public:
    explicit Value(JSVM_Value value) : value_(value) {}

    operator JSVM_Value() const
    {
        return value_;
    }

protected:
    JSVM_Value value_;
};

class Object : Value {
public:
    explicit Object(JSVM_Value jsvm_value) : Value(jsvm_value)
    {
        bool isObject = false;
        OH_JSVM_IsObject(jsvm_env, this->value_, &isObject);
        if (!isObject) {
            GTEST_LOG_(ERROR) << "Reset SIGPIPE failed.";
        }
    }

    JSVM_Value Get(const char* propName)
    {
        JSVM_Value result;
        OH_JSVM_GetNamedProperty(jsvm_env, this->value_, propName, &result);
        return result;
    }

    void Set(const char* propName, JSVM_Value propValue)
    {
        OH_JSVM_SetNamedProperty(jsvm_env, this->value_, propName, propValue);
    }
};

// global functions
JSVM_Value Str(const char* s);
JSVM_Value Str(const std::string& stdString);
JSVM_Script Compile(JSVM_Value js_str, const uint8_t* cache = nullptr, size_t size = JSVM_AUTO_LENGTH);
JSVM_Script CompileWithName(JSVM_Value js_str, const char* name);
JSVM_Script Compile(const char* s, const uint8_t* cache = nullptr, size_t size = JSVM_AUTO_LENGTH);
JSVM_Script CompileWithName(const char* s, const char* name);
JSVM_Value Run(JSVM_Script script);
JSVM_Value Run(const char* s);
bool ToBoolean(JSVM_Value val);
double ToNumber(JSVM_Value val);
std::string ToString(JSVM_Value val);
JSVM_Value True();
JSVM_Value False();
JSVM_Value Undefined();
JSVM_Value Null();
JSVM_Value Int32(int32_t v);
JSVM_Value Int64(int64_t v);
JSVM_Value Uint32(uint32_t v);
JSVM_Value Double(double v);
JSVM_Value Object();
JSVM_Value Global();
// object property
JSVM_Value GetProperty(JSVM_Value object, JSVM_Value key);
JSVM_Value GetProperty(JSVM_Value object, const char* name);
void SetProperty(JSVM_Value object, JSVM_Value key, JSVM_Value value);
void SetProperty(JSVM_Value object, const char* name, JSVM_Value value);
// Get named property with name `name` from globalThis
JSVM_Value Global(const char* name);
// Get property with key `key` from globalThis
JSVM_Value Global(JSVM_Value key);
// Call function with normal function with empty argument list
JSVM_Value Call(JSVM_Value func);
// Call function with normal function with specified argument list
JSVM_Value Call(JSVM_Value func, JSVM_Value thisArg, std::initializer_list<JSVM_Value> args);
// Call function as constructor with empty argument list
JSVM_Value New(JSVM_Value constructor);
bool InstanceOf(JSVM_Value value, JSVM_Value constructor);
bool IsNull(JSVM_Value value);
bool IsUndefined(JSVM_Value value);
bool IsNullOrUndefined(JSVM_Value value);
bool IsBoolean(JSVM_Value value);
bool IsTrue(JSVM_Value value);
bool IsFalse(JSVM_Value value);
bool IsNumber(JSVM_Value value);
bool IsString(JSVM_Value value);
bool IsObject(JSVM_Value value);
bool IsBigInt(JSVM_Value value);
bool IsSymbol(JSVM_Value value);
bool IsFunction(JSVM_Value value);
bool IsArray(JSVM_Value value);
bool IsArraybuffer(JSVM_Value value);
bool IsPromise(JSVM_Value value);
bool IsWasmModuleObject(JSVM_Value value);
bool IsWebAssemblyInstance(JSVM_Value value);
bool IsWebAssemblyMemory(JSVM_Value value);
bool IsWebAssemblyTable(JSVM_Value value);
size_t StringLength(JSVM_Value str);
uint32_t ArrayLength(JSVM_Value array);
JSVM_Value ArrayAt(JSVM_Value array, uint32_t index);
JSVM_Value JsonParse(JSVM_Value str);
JSVM_Value JsonStringify(JSVM_Value obj);
bool Equals(JSVM_Value lhs, JSVM_Value rhs);
bool StrictEquals(JSVM_Value lhs, JSVM_Value rhs);
// This is a simple log function
JSVM_Value MyConsoleLog(JSVM_Env env, JSVM_CallbackInfo info);
void InstallMyConsoleLog(JSVM_Env env);
void TryTriggerOOM();
void TryTriggerFatalError(JSVM_VM vm);
void TryTriggerGC();
void TryLowMemoryGC();
} // namespace jsvm

#endif // JSVM_UTILS_H
