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
import {BusinessError} from "@ohos.base";
import * as OptTest from "opt";
import * as optionalTest from "optional";

loadLibrary("ani_optional");

function showOptionalInt() {
    OptTest.showOptionalInt((100).toInt())
}

function showOptionalInt_1() {
    OptTest.showOptionalInt()
}

function makeOptionalInt() {
    let res = OptTest.makeOptionalInt(true);
    console.log(res)
    arktest.assertTrue(res != undefined);
}

function makeOptionalInt_1() {
    let res = OptTest.makeOptionalInt(false);
    console.log(res)
    arktest.assertTrue(res == undefined);
}

function makeOptionalArray() {
    let res = OptTest.makeOptionalArray(true, 5, 3);
    console.log(res)
    arktest.assertTrue(res != undefined);
}

function makeOptionalArray_1() {
    let res = OptTest.makeOptionalArray(false, 5, 3);
    console.log(res)
    arktest.assertTrue(res == undefined);
}

function sendReturnResult() {
    let result: OptTest.ReturnResult = {results: ["ab", "cd", "ef"]};
    let res = OptTest.sendReturnResult(result);
    console.log(res)
    arktest.assertTrue(res != undefined);
}

function sendReturnResult_1() {
    let result: OptTest.ReturnResult = {};
    let res = OptTest.sendReturnResult(result);
    console.log(res)
    arktest.assertTrue(res == undefined);
}

// 数值类型测试函数
function func_primitive_i8() {
    let instance: optionalTest.ExampleInterface = optionalTest.getInterface();
    instance.funcPrimitiveI8((5).toByte())  // byte 对应 i8
}

function func_primitive_i16() {
    let instance: optionalTest.ExampleInterface = optionalTest.getInterface();
    instance.funcPrimitiveI16((42).toShort())  // short 对应 i16
}

function func_primitive_i32() {
    let instance: optionalTest.ExampleInterface = optionalTest.getInterface();
    instance.funcPrimitiveI32((1000).toInt())  // int 对应 i32
}

function func_primitive_i64() {
    let instance: optionalTest.ExampleInterface = optionalTest.getInterface();
    instance.funcPrimitiveI64((1000000).toLong())  // long 对应 i64
}

// 浮点类型测试函数
function func_primitive_f32() {
    let instance: optionalTest.ExampleInterface = optionalTest.getInterface();
    instance.funcPrimitiveF32(3.14f)  // float 对应 f32
}

function func_primitive_f64() {
    let instance: optionalTest.ExampleInterface = optionalTest.getInterface();
    instance.funcPrimitiveF64((2.71828).toDouble())  // double 对应 f64
}

// 布尔类型测试函数
function func_primitive_bool() {
    let instance: optionalTest.ExampleInterface = optionalTest.getInterface();
    instance.funcPrimitiveBool(true)
}

// 字符串类型测试函数
function func_primitive_string() {
    let instance: optionalTest.ExampleInterface = optionalTest.getInterface();
    instance.funcPrimitiveString("test_string")
}


function func_Array() {
    let instance: optionalTest.ExampleInterface = optionalTest.getInterface();

    let numbers: int[] = [1, 2, 3, 4, 5];
    instance.funcArray(numbers)
}

function func_buffer() {
    let instance: optionalTest.ExampleInterface = optionalTest.getInterface();

    let numbersU8: byte[] = [1, 2, 3, 4, 5];
    let arrbuf1: ArrayBuffer = new ArrayBuffer(numbersU8.length);
    for (let i = 0; i < numbersU8.length; i++) {
        arrbuf1.set(i, numbersU8[i]);
    }
    instance.funcBuffer(arrbuf1)
}

function func_Union() {
    let instance: optionalTest.ExampleInterface = optionalTest.getInterface();
    instance.funcUnion("hello")
    instance.funcUnion((1024).toInt())
}

function func_Map() {
    let instance: optionalTest.ExampleInterface = optionalTest.getInterface();
    let cpus: Record<string, int> = {
        "cpu0": 10,
        "cpu1": 20,
    };
    instance.funcMap(cpus)
}

function func_InterfaceName() {
    let instance: optionalTest.ExampleInterface = optionalTest.getInterface();
    optionalTest.printTestInterfaceName(instance)
}
function func_Interfacei8() {
    let instance: optionalTest.ExampleInterface = optionalTest.getInterface();
    optionalTest.printTestInterfaceNumberi8(instance)
}
function func_PrintTestInterfacei16() {
    let instance: optionalTest.ExampleInterface = optionalTest.getInterface();
    optionalTest.printTestInterfaceNumberi16(instance);
}

function func_PrintTestInterfacei32() {
    let instance: optionalTest.ExampleInterface = optionalTest.getInterface();
    optionalTest.printTestInterfaceNumberi32(instance);
}

function func_PrintTestInterfacei64() {
    let instance: optionalTest.ExampleInterface = optionalTest.getInterface();
    optionalTest.printTestInterfaceNumberi64(instance);
}

function func_PrintTestInterfacef32() {
    let instance: optionalTest.ExampleInterface = optionalTest.getInterface();
    optionalTest.printTestInterfaceNumberf32(instance);
}

function func_PrintTestInterfacef64() {
    let instance: optionalTest.ExampleInterface = optionalTest.getInterface();
    optionalTest.printTestInterfaceNumberf64(instance);
}

function func_PrintTestInterfacebool() {
    let instance: optionalTest.ExampleInterface = optionalTest.getInterface();
    optionalTest.printTestInterfacebool(instance);
}

function func_PrintTestInterfaceArraybuffer() {
    let instance: optionalTest.ExampleInterface = optionalTest.getInterface();
    optionalTest.printTestInterfaceArraybuffer(instance);
}

function func_showOptionalString() {
    let instance: OptTest.Test = OptTest.getTest();
    let str = "this is string data";
    let result = instance.showOptionalString(str);
    arktest.assertEQ(result, "this is string data");
}

function func_showOptionalString_1() {
    let instance: OptTest.Test = OptTest.getTest();
    let result = instance.showOptionalString(undefined);
    arktest.assertTrue(result == undefined);
}

function func_getTest_setIntData() {
    let instance: OptTest.Test = OptTest.getTest();
    instance.setIntData("this is str");
}

function func_getTest_property() {
    let instance: OptTest.Test = OptTest.getTest();
    instance.memberStr = "this is string value";
    let res = instance.memberStr;
    arktest.assertEQ(res, "this is string value");
}

function func_getTest_property_1() {
    let instance: OptTest.Test = OptTest.getTest();
    instance.memberStr = undefined;
    let res = instance.memberStr;
    arktest.assertEQ(res, undefined);
}

function func_showOptionalInt32() {
    let instance: OptTest.Test = OptTest.getTest();
    let intValue = 12;
    let res = instance.showOptionalInt32(intValue);
    arktest.assertEQ(res, 12);
}

function func_showOptionalInt32_1() {
    let instance: OptTest.Test = OptTest.getTest();
    let res = instance.showOptionalInt32(undefined);
    arktest.assertEQ(res, undefined);
}

function func_showOptionalBool() {
    let instance: OptTest.Test = OptTest.getTest();
    let res = instance.showOptionalBool(true);
    arktest.assertEQ(res, true);
}

function func_showOptionalBool_1() {
    let instance: OptTest.Test = OptTest.getTest();
    let res = instance.showOptionalBool(undefined);
    arktest.assertEQ(res, undefined);
}

function func_showOptionalRecord() {
    let instance: OptTest.Test = OptTest.getTest();
    let data: Record<string, boolean> = {"key1": false};
    let res = instance.showOptionalRecord(data) as Record<string, boolean>;
    arktest.assertEQ(res.toString(), data.toString());
}

function func_showOptionalRecord_1() {
    let instance: OptTest.Test = OptTest.getTest();
    let res = instance.showOptionalRecord(undefined);
    arktest.assertEQ(res, undefined);
}

function func_showOptionalStruct() {
    let instance: OptTest.Test = OptTest.getTest();
    let data: OptTest.MyStruct = {param1: 12, param2: "string data"};
    let res = instance.showOptionalStruct(data) as OptTest.MyStruct;
    arktest.assertEQ(res.toString(), data.toString());
}

function func_showOptionalStruct_1() {
    let instance: OptTest.Test = OptTest.getTest();
    let res = instance.showOptionalStruct(undefined);
    arktest.assertEQ(res, undefined);
}

function test_callCallback() {
    OptTest.callCallback(true, (a: string, b?: string) => {
        console.log(`a should be "Hello": ${a}`);
        console.log(`b should be "World": ${b}`);
        arktest.assertEQ(a, "Hello");
        arktest.assertEQ(b, "World");
    });
    OptTest.callCallback(false, (a: string, b?: string) => {
        console.log(`a should be "Hello": ${a}`);
        console.log(`b should be undefined: ${b}`);
        arktest.assertEQ(a, "Hello");
        arktest.assertTrue(b === undefined);
    });
}

function main() {
    console.log("run main ... OptTest ...")

    const suite = new arktest.ArkTestsuite("OptTest")
    suite.addTest("showOptionalInt", showOptionalInt)
    suite.addTest("showOptionalInt_1", showOptionalInt_1)
    suite.addTest("makeOptionalInt", makeOptionalInt)
    suite.addTest("makeOptionalInt_1", makeOptionalInt_1)
    suite.addTest("makeOptionalArray", makeOptionalArray)
    suite.addTest("makeOptionalArray_1", makeOptionalArray_1)
    suite.addTest("sendReturnResult", sendReturnResult)
    suite.addTest("sendReturnResult_1", sendReturnResult_1)
    suite.addTest("i8 parameter test", func_primitive_i8)
    suite.addTest("i16 parameter test", func_primitive_i16)
    suite.addTest("i32 parameter test", func_primitive_i32)
    suite.addTest("i64 parameter test", func_primitive_i64)
    suite.addTest("f32 parameter test", func_primitive_f32)
    suite.addTest("f64 parameter test", func_primitive_f64)
    suite.addTest("bool parameter test", func_primitive_bool)
    suite.addTest("bool parameter test", func_primitive_string)
    suite.addTest("Array parameter test", func_Array)
    suite.addTest("buffer parameter test", func_buffer)
    suite.addTest("Union parameter test", func_Union)
    suite.addTest("Map parameter test", func_Map)

    suite.addTest("func_InterfaceName test", func_InterfaceName)
    suite.addTest("func_Interfacei8 test", func_Interfacei8)
    suite.addTest(
        "func_PrintTestInterfacei16 test", func_PrintTestInterfacei16);
    suite.addTest(
        "func_PrintTestInterfacei32 test", func_PrintTestInterfacei32);
    suite.addTest(
        "func_PrintTestInterfacei64 test", func_PrintTestInterfacei64);
    suite.addTest(
        "func_PrintTestInterfacef32 test", func_PrintTestInterfacef32);
    suite.addTest(
        "func_PrintTestInterfacef64 test", func_PrintTestInterfacef64);
    suite.addTest(
        "func_PrintTestInterfacebool test", func_PrintTestInterfacebool);
    suite.addTest(
        "func_PrintTestInterfaceArraybuffer test",
        func_PrintTestInterfaceArraybuffer);

    suite.addTest("func_showOptionalString", func_showOptionalString)
    suite.addTest("func_showOptionalString_1", func_showOptionalString_1)
    suite.addTest("func_getTest_setIntData", func_getTest_setIntData)
    suite.addTest("func_getTest_property", func_getTest_property)
    suite.addTest("func_getTest_property_1", func_getTest_property_1)
    suite.addTest("func_showOptionalInt32", func_showOptionalInt32)
    suite.addTest("func_showOptionalInt32_1", func_showOptionalInt32_1)
    suite.addTest("func_showOptionalBool", func_showOptionalBool)
    suite.addTest("func_showOptionalBool_1", func_showOptionalBool_1)
    suite.addTest("func_showOptionalRecord", func_showOptionalRecord)
    suite.addTest("func_showOptionalRecord_1", func_showOptionalRecord_1)
    suite.addTest("func_showOptionalStruct", func_showOptionalStruct)
    suite.addTest("func_showOptionalStruct_1", func_showOptionalStruct_1)

    suite.addTest("test_callCallback", test_callCallback)

    exit(suite.run());
}
