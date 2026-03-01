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
import * as test_overload from "overload";
import * as OverloadTest from "overload_test";

loadLibrary("ani_javalike_overload");


function test_add_numbers() {
    const result = OverloadTest.add(1, 2);
    arktest.assertEQ(result, 3);
}

function test_add_strings() {
    const result = OverloadTest.add("1", "2");
    arktest.assertEQ(result, "12");
}

function test_Foo_bar_number(foo: OverloadTest.Foo) {
    const result = foo.bar(123);
    arktest.assertEQ(result, 123);
}

function test_Foo_bar_string(foo: OverloadTest.Foo) {
    const result = foo.bar("Hello");
    arktest.assertEQ(result, "Hello");
}
// 测试 byte (i8)
function testByte() {
    let instance: test_overload.OverloadInterface =
        test_overload.get_interface();
    let res = instance.overloadFunc((5).toByte(), (5).toByte());
    arktest.assertEQ(res, 5);
}

// 测试 short (i16)
function testShort() {
    let instance: test_overload.OverloadInterface =
        test_overload.get_interface();
    let res = instance.overloadFunc((42).toShort(), (42).toShort());
    arktest.assertEQ(res, 42);
}

// 测试 int (i32)
function testInt() {
    let instance: test_overload.OverloadInterface =
        test_overload.get_interface();
    let res = instance.overloadFunc((1000).toInt(), (1000).toInt());
    arktest.assertEQ(res, 1000);
}

// 测试 long (i64)
function testLong() {
    let instance: test_overload.OverloadInterface =
        test_overload.get_interface();
    let res = instance.overloadFunc((1000000).toLong(), (1000000).toLong());
    arktest.assertEQ(res, 1000000);
}

// 测试 float (f32)
function testFloat() {
    let instance: test_overload.OverloadInterface =
        test_overload.get_interface();
    let res = instance.overloadFunc(3.14f, 3.14f);
    arktest.assertEQ(res, 3.14f);
}

// 测试 double (f64)
function testDouble() {
    let instance: test_overload.OverloadInterface =
        test_overload.get_interface();
    let res = instance.overloadFunc((2.71828).toDouble(), (2.71828).toDouble());
    arktest.assertEQ(res, 2.71828);
}

// 测试 String
function testString() {
    let instance: test_overload.OverloadInterface =
        test_overload.get_interface();
    let res = instance.overloadFunc("123", "123");
    arktest.assertEQ(res, "123");
}

// 测试 i8 与 i16
function testByte_Short() {
    let instance: test_overload.OverloadInterface =
        test_overload.get_interface();
    let res = instance.overloadFunc((5).toByte(), (42).toShort());
    arktest.assertEQ(res, 5);
}
// 测试 i8 与 f32
function testByte_Float() {
    let instance: test_overload.OverloadInterface =
        test_overload.get_interface();
    let res = instance.overloadFunc((5).toByte(), 3.14f);
    arktest.assertEQ(res, 5);
}
// 测试 i8 与 String
function testByte_String() {
    let instance: test_overload.OverloadInterface =
        test_overload.get_interface();
    let res = instance.overloadFunc((5).toByte(), "123");
    arktest.assertEQ(res, 5);
}

// 测试传入 enum 类型
function test_enum() {
    let color = test_overload.Color.red;
    let instance: test_overload.OverloadInterface =
        test_overload.get_interface();
    let res = instance.overloadFunc(color);
    arktest.assertEQ(res, (1).toInt());
}

// 测试自定义的 struct 类型
function test_struct() {
    let mystruct: test_overload.Mystruct = {
        testNum: 2147483647,
        testStr: "path/to/source"
    };
    let instance: test_overload.OverloadInterface =
        test_overload.get_interface();
    let res = instance.overloadFunc(mystruct)
    arktest.assertEQ(res, "path/to/source")
}

// 多参数组合

// 测试 5 param
function test_5param() {
    let instance: test_overload.OverloadInterface =
        test_overload.get_interface();
    instance.overloadFunc(
        (1).toByte(), (1).toShort(), (1).toInt(), 1.1f, (1.123).toDouble());
}
function test_5param2() {
    let instance: test_overload.OverloadInterface =
        test_overload.get_interface();
    instance.overloadFunc(
        true, "hello", (1).toByte(), (1).toShort(), (1).toInt());
}
function test_5param3() {
    let instance: test_overload.OverloadInterface =
        test_overload.get_interface();
    instance.overloadFunc(1.0f, (1).toDouble(), true, "hello", (1).toByte());
}
function test_5param4() {
    let instance: test_overload.OverloadInterface =
        test_overload.get_interface();
    instance.overloadFunc("1 as byte", (1).toShort(), (1).toInt(), 1.1f, true);
}
function test_5param5() {
    let instance: test_overload.OverloadInterface =
        test_overload.get_interface();
    instance.overloadFunc("1 as byte", "1 as short", "1 as int", true, false);
}

function test_5param6() {
    let instance: test_overload.OverloadInterface =
        test_overload.get_interface();
    instance.overloadFunc(
        (10).toShort(), (20).toInt(), 3.0f, (4.0).toDouble(), true);
}

function test_5param7() {
    let instance: test_overload.OverloadInterface =
        test_overload.get_interface();
    instance.overloadFunc("hello", (1).toByte(), (2).toShort(), 3.0f, false);
}

function test_5param8() {
    let instance: test_overload.OverloadInterface =
        test_overload.get_interface();
    instance.overloadFunc(
        true, (5).toByte(), (10).toInt(), (2.5).toDouble(), "world");
}

function test_5param9() {
    let instance: test_overload.OverloadInterface =
        test_overload.get_interface();
    instance.overloadFunc(
        (3.14).toDouble(), false, "test", (100).toShort(), (200).toInt());
}

function test_5param10() {
    let instance: test_overload.OverloadInterface =
        test_overload.get_interface();
    instance.overloadFunc((42).toByte(), 1.5f, true, "example", (1000).toInt());
}


// 测试 10 param
function test_10param() {
    let instance: test_overload.OverloadInterface =
        test_overload.get_interface();
    let arri8: byte[] = [1, 2, 3];
    let arri16: short[] = [1, 2, 3, 4];
    let arri32: int[] = [1, 2, 3, 4, 5];
    instance.overloadFunc(
        (1).toByte(), (1).toShort(), (1).toInt(), 1.1f, (1.123).toDouble(),
        true, "123", arri8, arri16, arri32);
}

// 测试 10 param1
function test_10param1() {
    let instance: test_overload.OverloadInterface =
        test_overload.get_interface();
    let arri8: byte[] = [1, 2, 3];
    let mystruct: test_overload.Mystruct = {
        testNum: 2147483647,
        testStr: "path/to/source"
    };
    let color = test_overload.Color.red;
    instance.overloadFunc(
        (1).toByte(), (1).toShort(), (1).toInt(), 1.1f, (1.123).toDouble(),
        true, "123", arri8, mystruct, color);
}

// 测试 10 param2
function test_10param2() {
    let instance: test_overload.OverloadInterface =
        test_overload.get_interface();
    let arri8: byte[] = [1, 2, 3];
    let arri16: short[] = [1, 2, 3, 4];
    let arri32: int[] = [1, 2, 3, 4, 5];
    let arrbool: boolean[] = [true, false];
    let arrf32: float[] = [1.1f, 2.2f];
    let arrf64: double[] = [1.11111, 2.22222];
    let arrstring: String[] = ["hello", "world"];

    let mystruct: test_overload.Mystruct = {
        testNum: 2147483647,
        testStr: "path/to/source"
    };
    let color = test_overload.Color.red;
    instance.overloadFunc(
        (1).toByte(), mystruct, color, arrf32, arrf64, arrbool, arrstring,
        arri8, arri16, arri32);
}

// 测试 10 param3
function test_10param3() {
    let instance: test_overload.OverloadInterface =
        test_overload.get_interface();
    let arri8: byte[] = [1, 2, 3];
    let mystruct: test_overload.Mystruct = {
        testNum: 2147483647,
        testStr: "path/to/source"
    };
    let color = test_overload.Color.red;

    let numbersU8: byte[] = [1, 2, 3, 4, 5];
    let arrbuf1: ArrayBuffer = new ArrayBuffer(numbersU8.length);
    for (let i = 0; i < numbersU8.length; i++) {
        arrbuf1.set(i, numbersU8[i]);
    }

    instance.overloadFunc(
        (1).toByte(), (1).toShort(), (1).toInt(), 1.1f, (1.123).toDouble(),
        true, "123", arrbuf1, mystruct, color);
}
// 测试 10 param4
function test_10param4() {
    let instance: test_overload.OverloadInterface =
        test_overload.get_interface();
    let arri8: byte[] = [1, 2, 3];
    let mystruct: test_overload.Mystruct = {
        testNum: 2147483647,
        testStr: "path/to/source"
    };
    let color = test_overload.Color.red;

    let numbersU8: byte[] = [1, 2, 3, 4, 5];
    let arrbuf1: ArrayBuffer = new ArrayBuffer(numbersU8.length);
    for (let i = 0; i < numbersU8.length; i++) {
        arrbuf1.set(i, numbersU8[i]);
    }

    instance.overloadFunc(
        (1).toByte(), (1).toShort(), (1).toInt(), 1.1f, (1.123).toDouble(),
        true, "123", arri8, arrbuf1, color);
}



// 测试重载函数传入数组
function test_Array() {
    let instance: test_overload.OverloadInterface =
        test_overload.get_interface()

    let numbers: int[] = [1, 2, 3, 4, 5];
    let ocp = instance.overloadFunc(numbers)
    arktest.assertEQ(ocp, (1).toInt());
}

function test_buffer() {
    let instance: test_overload.OverloadInterface =
        test_overload.get_interface()
    let numbersU8: byte[] = [1, 2, 3, 4, 5];
    let arrbuf1: ArrayBuffer = new ArrayBuffer(numbersU8.length);
    for (let i = 0; i < numbersU8.length; i++) {
        arrbuf1.set(i, numbersU8[i]);
    }
    let res = instance.overloadFunc(arrbuf1)
    arktest.assertEQ(res, 15);
}

function test_enum_record() {
    const studentScores: Record<String, short> = {
        "math": (10).toShort(),
        "english": (85).toShort(),
        "science": (95).toShort(),
    };
    let instance: test_overload.OverloadInterface =
        test_overload.get_interface()
    let color = test_overload.Color.red;
    // instance.overloadFunc((test_overload.Color)color , studentScores)
}

function test_array_record() {
    const studentScores: Record<String, short> = {
        "math": (10).toShort(),
        "english": (85).toShort(),
        "science": (95).toShort(),
    };
    let instance: test_overload.OverloadInterface =
        test_overload.get_interface()
    let numbers: int[] = [1, 2, 3, 4, 5];
    instance.overloadFunc(numbers, studentScores)
}


function main() {
    console.log("##############start#############");
    const suite = new arktest.ArkTestsuite("Overload Test");

    suite.addTest("test add numbers", test_add_numbers);
    suite.addTest("test add strings", test_add_strings);

    let foo = OverloadTest.makeFoo();

    suite.addTest("test Foo bar with number", () => test_Foo_bar_number(foo));
    suite.addTest("test Foo bar with string", () => test_Foo_bar_string(foo));

    // primitive
    suite.addTest("testByte", testByte);
    suite.addTest("testShort", testShort);
    suite.addTest("testInt", testInt);
    suite.addTest("testLong", testLong);
    suite.addTest("testFloat", testFloat);
    suite.addTest("testDouble", testDouble);
    suite.addTest("testString", testString);
    suite.addTest("testByte_Short", testByte_Short);
    suite.addTest("testByte_Float", testByte_Float);
    suite.addTest("testByte_String", testByte_String);

    //
    suite.addTest("test_enum", test_enum);
    suite.addTest("test_struct", test_struct);
    suite.addTest("test_Array", test_Array);
    suite.addTest("test_buffer", test_buffer);
    suite.addTest("test_enum_record", test_enum_record);
    suite.addTest("test_array_record", test_array_record);

    // 多参数组合
    suite.addTest("testByte_5param", test_5param);
    suite.addTest("testByte_5param2", test_5param2);
    suite.addTest("testByte_5param3", test_5param3);
    suite.addTest("testByte_5param4", test_5param4);
    suite.addTest("testByte_5param5", test_5param5);
    suite.addTest("testByte_5param6", test_5param6);
    suite.addTest("testByte_5param7", test_5param7);
    suite.addTest("testByte_5param8", test_5param8);
    suite.addTest("testByte_5param9", test_5param9);
    suite.addTest("testByte_5param10", test_5param10);

    suite.addTest("testByte_10param", test_10param);
    suite.addTest("testByte_10param1", test_10param1);
    suite.addTest("testByte_10param2", test_10param2);
    suite.addTest("testByte_10param3", test_10param3);
    suite.addTest("testByte_10param4", test_10param4);
    exit(suite.run());
}
