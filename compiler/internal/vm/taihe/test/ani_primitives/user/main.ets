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
import * as test_primitives from "primitives_test";
import * as test_void_ns from "void_func";

loadLibrary("ani_primitives");

let primitivesTest: test_primitives.PrimitivesVoid =
    test_primitives.get_interface();
let primitivesBoolTest: test_primitives.PrimitivesBoolean =
    test_primitives.get_interface_bool();
let primitivesIntergerTest: test_primitives.PrimitivesInteger =
    test_primitives.get_interface_interger();

function testMultiply() {
    let res = test_primitives.Multiply(2, 4)
    arktest.assertEQ(res, 8)
}


function testFunC1() {
    let res = test_primitives.BaseCFunc(10);
    console.log("BaseCFunc pass:", res);
    arktest.assertEQ(res, true)
}

function testFunC2() {
    let res = test_primitives.BaseCFunc(5);
    console.log("BaseCFunc fail", res);
    arktest.assertEQ(res, false)
}

function testFunA1() {
    test_primitives.BaseAFunc(true)
}

function testFunA2() {
    test_primitives.BaseAFunc(false)
}

function testFunB1() {
    let res = test_primitives.BaseBFunc(true)
    arktest.assertEQ(res, false)
}

function testFunB2() {
    let res = test_primitives.BaseBFunc(false)
    arktest.assertEQ(res, true)
}

function testFunD1() {
    let res = test_primitives.BaseDFunc("test123");
    console.log("BaseDFunc pass:", res);
    arktest.assertEQ(res, true)
}

function testFunD2() {
    let res = test_primitives.BaseDFunc("test1231");
    console.log("BaseDFunc fail:", res);
    arktest.assertEQ(res, false)
}

function testFunE() {
    const option:
        test_primitives.Foo = {testNum: 2147483647, testStr: "path/to/source"};
    try {
        let res = test_primitives.BaseEFunc(option)
        arktest.assertEQ(res, "success")
    } catch (error) {
        console.error("Failed to parse res:", error);
    }
}

function testFunH1() {
    let res = test_primitives.BaseHFunc(2147483647, 9223372036854775807)
    arktest.assertEQ(res, "-9223372034707292162")
}

function testFunH2() {
    let res = test_primitives.BaseHFunc(-2147483648, -9223372036854775808)
    arktest.assertEQ(res, "9223372034707292160")
}

function baseFunc1() {
    let res = test_primitives.BaseFunc1(3.0f)
    arktest.assertEQ(res, 4.0)
}

function baseFunc2() {
    test_primitives.BaseFunc2(3.0f)
}


function baseFunc3() {
    let res = test_primitives.BaseFunc3(3.0f, (3.14159265).toDouble())
    console.log("tests baseFunc3" + res)
}

function baseFunc4() {
    let res = test_primitives.BaseFunc4((3.0).toDouble())
    console.log("tests baseFunc3" + res)
}

function baseFunc5() {
    test_primitives.BaseFunc5((3.14159).toDouble())
}

function baseFunc6() {
    test_primitives.BaseFunc6("testbaseFunc6")
}

function baseFunc6_1() {
    test_primitives.BaseFunc6("testbaseFunc66")
}

function baseFunc7() {
    let res = test_primitives.BaseFunc7("TestbaseFunc7")
    arktest.assertEQ(res, "TestbaseFunc7")
}

function baseFunc7_1() {
    let res = test_primitives.BaseFunc7("testbaseFunc77")
    arktest.assertEQ(res, "testbaseFunc77false")
}

function baseFunc8() {
    let res = test_primitives.BaseFunc8("TestBaseFunc8", 32)
    arktest.assertEQ(res, "TestBaseFunc832")
}

function baseFunc8_1() {
    let res = test_primitives.BaseFunc8("testbaseFunc88", 32)
    arktest.assertEQ(res, "testbaseFunc88")
}

function baseFunc9() {
    test_primitives.BaseFunc9(
        "testbaseFunc9", 32, 9223372036854775807, true, 3.1f)
}

function testFunI1() {
    let res = test_primitives.BaseIFunc((3.1415926535897931).toDouble(), 4.3f)
    console.log("testFunI1 ", res)
    arktest.assertEQ(res, "7.441593")
}

// function testFunI2(){
//     let res =
//     test_primitives.BaseIFunc((-1234567890.1234567).toDouble(),-123.4f)
//     console.log("testFunI2 ",res)
//     arktest.assertEQ(res,"-1234568013.523457")
// }


function baseFunc10() {
    test_primitives.BaseFunc10()
}

function baseFunc11() {
    test_primitives.BaseFunc11(32, true)
}

function baseFunc11_1() {
    test_primitives.BaseFunc11(32, false)
}

function baseFunc12() {
    test_primitives.BaseFunc12(32, 92233720368547758)
}

function baseFunc13() {
    test_primitives.BaseFunc13(32, "testbaseFunc13")
}

function baseFunc14() {
    test_primitives.BaseFunc14(92233720368547758, true)
}


function baseFunc14_1() {
    test_primitives.BaseFunc14(92233720368547758, false)
}

function baseFunc15() {
    test_primitives.BaseFunc15(32, 3.15f)
}

function baseFunc16() {
    test_primitives.BaseFunc16(32, 3.141592653589793)
}

function baseFunc16_1() {
    test_primitives.BaseFunc16(32, -3.141592653589793)
}


function baseFunc17() {
    test_primitives.BaseFunc17(123.456f, true)
}

function baseFunc17_1() {
    test_primitives.BaseFunc17(3.14159f, false)
}

function baseFunc18() {
    test_primitives.BaseFunc18(123.456f, "testbaseFunc18")
}

function baseFunc18_1() {
    test_primitives.BaseFunc18(3.14159f, "testbaseFunc18")
}

function baseFunc19() {
    test_primitives.BaseFunc19(3.141592653589793, "testbaseFunc19")
}

function baseFunc20() {
    test_primitives.BaseFunc20(3.141592653589793, true)
}

function baseFunc20_1() {
    test_primitives.BaseFunc20(3.141592653589793, false)
}

function baseFunc21() {
    test_primitives.BaseFunc21((32).toInt(), (65535).toLong())
}

function baseFunc21_1() {
    test_primitives.BaseFunc21(undefined, (65535).toLong())
}

function baseFunc21_2() {
    test_primitives.BaseFunc21(undefined, undefined)
}

function baseFunc21_3() {
    test_primitives.BaseFunc21((32).toInt(), undefined)
}

function baseFunc22() {
    test_primitives.BaseFunc22(3.14159f, 3.141592653589793)
}

function baseFunc22_1() {
    test_primitives.BaseFunc22(undefined, 3.141592653589793)
}

function baseFunc22_2() {
    test_primitives.BaseFunc22(undefined, undefined)
}

function baseFunc22_3() {
    test_primitives.BaseFunc22(3.14159f, undefined)
}

function baseFunc23() {
    test_primitives.BaseFunc23("testBaseFunc23", true)
}

function baseFunc23_1() {
    test_primitives.BaseFunc23(undefined, false)
}

function baseFunc23_2() {
    test_primitives.BaseFunc23(undefined, undefined)
}

function baseFunc23_3() {
    test_primitives.BaseFunc23("testBaseFunc23", undefined)
}

function baseFunc24() {
    test_primitives.BaseFunc24((32767).toShort(), 92233720368547758)
}

function baseFunc24_1() {
    test_primitives.BaseFunc24(undefined, 92233720368547758)
}

function baseFunc24_2() {
    test_primitives.BaseFunc24(undefined, undefined)
}

function baseFunc24_3() {
    test_primitives.BaseFunc24((32767).toShort(), undefined)
}

function testCaseBaseFunc1() {
    primitivesTest.TestBaseFunc1()
}

function testCaseBaseFunc2() {
    primitivesTest.TestBaseFunc2(32, true)
}

function testCaseBaseFunc2_1() {
    primitivesTest.TestBaseFunc2(32, false)
}

function testCaseBaseFunc3() {
    primitivesTest.TestBaseFunc3(-2147483648, 92233720368547758)
}

function testCaseBaseFunc4() {
    primitivesTest.TestBaseFunc4(2147483647, "testBaseFunc4")
}

function testCaseBaseFunc5() {
    primitivesTest.TestBaseFunc5(-92233720368547758, true)
}


function testCaseBaseFunc5_1() {
    primitivesTest.TestBaseFunc5(-92233720368547758, false)
}

function testCaseBaseFunc6() {
    primitivesTest.TestBaseFunc6(32, 3.15f)
}

function testCaseBaseFunc7() {
    primitivesTest.TestBaseFunc7(9223372036854775807, 3.141592653589793)
}

function testCaseBaseFunc7_1() {
    primitivesTest.TestBaseFunc7(-9223372036854775808, -3.141592653589793)
}


function testCaseBaseFunc8() {
    primitivesTest.TestBaseFunc8(123.456789f, true)
}

function testCaseBaseFunc8_1() {
    primitivesTest.TestBaseFunc8(3.1415978f, false)
}

function testCaseBaseFunc9() {
    primitivesTest.TestBaseFunc9(123.456789f, "testBaseFunc9")
}

function testCaseBaseFunc9_1() {
    primitivesTest.TestBaseFunc9(3.1415978f, "testBaseFunc9")
}

function testCaseBaseFunc10() {
    primitivesTest.TestBaseFunc10(-3.141592653589793, "testBaseFunc10")
}

function testCaseBaseFunc11() {
    primitivesTest.TestBaseFunc11(-3.141592653589793, true)
}

function testCaseBaseFunc11_1() {
    primitivesTest.TestBaseFunc11(-3.141592653589793, false)
}

function testCaseBaseFunc12() {
    primitivesTest.TestBaseFunc12(256, (6553565536).toLong())
}

function testCaseBaseFunc12_1() {
    primitivesTest.TestBaseFunc12(undefined, (65535).toLong())
}

function testCaseBaseFunc12_2() {
    primitivesTest.TestBaseFunc12(undefined, undefined)
}

function testCaseBaseFunc12_3() {
    primitivesTest.TestBaseFunc12(-65535, undefined)
}

function testCaseBaseFunc13() {
    primitivesTest.TestBaseFunc13(-3.1415952f, 3.141592653589793)
}

function testCaseBaseFunc13_1() {
    primitivesTest.TestBaseFunc13(undefined, 3.141592653589793)
}

function testCaseBaseFunc13_2() {
    primitivesTest.TestBaseFunc13(undefined, undefined)
}

function testCaseBaseFunc13_3() {
    primitivesTest.TestBaseFunc13(-3.1415952f, undefined)
}

function testCaseBaseFunc14() {
    primitivesTest.TestBaseFunc14("testBaseFunc23", true)
}

function testCaseBaseFunc14_1() {
    primitivesTest.TestBaseFunc14(undefined, false)
}

function testCaseBaseFunc14_2() {
    primitivesTest.TestBaseFunc14(undefined, undefined)
}

function testCaseBaseFunc14_3() {
    primitivesTest.TestBaseFunc14("testBaseFunc14", undefined)
}

function testCaseBaseFunc15() {
    primitivesTest.TestBaseFunc15((32767).toShort(), 92233720368547758)
}

function testCaseBaseFunc15_1() {
    primitivesTest.TestBaseFunc15(undefined, 92233720368547758)
}

function testCaseBaseFunc15_2() {
    primitivesTest.TestBaseFunc15(undefined, undefined)
}

function testCaseBaseFunc15_3() {
    primitivesTest.TestBaseFunc15((-32768).toShort(), undefined)
}

function testCaseBaseFunc16() {
    primitivesTest.TestBaseFunc16((127).toByte(), (32765).toShort())
}

function testCaseBaseFunc16_1() {
    primitivesTest.TestBaseFunc16((-128).toByte(), (-32765).toShort())
}


function testCaseBaseFunc17() {
    let intArray: int[] = [1, 2, 3, 4, 5];
    let byteArray: byte[] = [100, 101, 102];
    primitivesTest.TestBaseFunc17(intArray, byteArray)
}

function testCaseBaseFunc18() {
    let shortArray: short[] = [32767, 32767, -32768, 4, 5];
    let longArray: long[] =
        [10086, 10010, 10000, 9223372036854775807, -9223372036854775808];
    primitivesTest.TestBaseFunc18(shortArray, longArray)
}

function testCaseBaseFunc19() {
    let floatArray: float[] = [3.14159f, 123.0f, -45678.5454f, 0.052f, 0.01f];
    let doubleArray: double[] = [
        10086.123456, -156.45678945, 3.141592345454545, 9223372036.854775807,
        -922337203.6854775808
    ];
    console.log("testCaseBaseFunc19 floatArray ", floatArray)
    console.log("testCaseBaseFunc19 doubleArray ", doubleArray)
    primitivesTest.TestBaseFunc19(floatArray, doubleArray)
}

function testCaseBaseFunc20() {
    let booleanArray: boolean[] = [true, false, false, true, true];
    let strArray: string[] =
        ["test1", "test111", "12345~!<>?|{}[]", "test333", "@#$$%^&&*123"];
    primitivesTest.TestBaseFunc20(booleanArray, strArray)
}

function testCaseBaseIntegerFunc1() {
    try {
        let temp = (255).toByte();
        console.log("test error 1 ", temp)
        let res = primitivesIntergerTest.TestBaseIntegerFunc1(temp)
    } catch (e) {
        arktest.assertEQ(
            e.toString(),
            "Error: out of range The i8 maximum value is 127 and minnum values is -128")
    }
}

function testCaseBaseIntegerFunc1_1() {
    let res = primitivesIntergerTest.TestBaseIntegerFunc1((100).toByte())
    arktest.assertEQ(res, 101)
}

function testCaseBaseIntegerFunc2() {
    try {
        let temp = (255).toByte();
        console.log("test error 2 ", temp)
        let res =
            primitivesIntergerTest.TestBaseIntegerFunc2(temp, (100).toShort())
    } catch (e) {
        arktest.assertEQ(
            e.toString(),
            "Error: out of range The i8 maximum value is 127 and minnum values is -128")
    }
}

function testCaseBaseIntegerFunc2_1() {
    let res = primitivesIntergerTest.TestBaseIntegerFunc2(
        (100).toByte(), (27).toShort())
    arktest.assertEQ(res, 127)
}

function testCaseBaseIntegerFunc2_2() {
    try {
        let temp = (-1).toShort();
        console.log("test error 2_2 ", temp)
        let res =
            primitivesIntergerTest.TestBaseIntegerFunc2((100).toByte(), temp)
    } catch (e) {
        console.log("testCaseBaseIntegerFunc2_2 error ", e.toString())
        arktest.assertEQ(
            e.toString(),
            "Error: out of range The i16 maximum value is 32767 and minnum values is -32768")
    }
}


function testCaseBaseIntegerFunc2_3() {
    let res = primitivesIntergerTest.TestBaseIntegerFunc2(
        (100).toByte(), (32667).toShort())
    arktest.assertEQ(res, -1)
}

function testCaseBaseIntegerFunc3() {
    primitivesIntergerTest.TestBaseIntegerFunc3((100).toByte(), (27).toShort())
}

function testCaseBaseIntegerFunc4() {
    let res = primitivesIntergerTest.TestBaseIntegerFunc4(
        (100).toByte(), (32667).toShort())
    arktest.assertEQ(res, 32767)
}

function testCaseBaseIntegerFunc4_1() {
    let res = primitivesIntergerTest.TestBaseIntegerFunc4(
        (127).toByte(), (-32768).toShort())
    arktest.assertEQ(res, -32641)
}

function testCaseBaseIntegerFunc4_2() {
    let res =
        primitivesIntergerTest.TestBaseIntegerFunc4((0).toByte(), (0).toShort())
    arktest.assertEQ(res, 0)
}

function testCaseBaseIntegerFunc5() {
    let res =
        primitivesIntergerTest.TestBaseIntegerFunc5((100).toByte(), 2147483547)
    arktest.assertEQ(res, -1)
}

function testCaseBaseIntegerFunc5_1() {
    let res = primitivesIntergerTest.TestBaseIntegerFunc5(
        (-127).toByte(), -2147483648)
    arktest.assertEQ(res, -127)
}

function testCaseBaseIntegerFunc5_2() {
    let res = primitivesIntergerTest.TestBaseIntegerFunc5((0).toByte(), 100)
    arktest.assertEQ(res, 100)
}

function testCaseBaseIntegerFunc6() {
    let res =
        primitivesIntergerTest.TestBaseIntegerFunc6((100).toByte(), 2147483547)
    arktest.assertEQ(res, 2147483647)
}

function testCaseBaseIntegerFunc6_1() {
    let res = primitivesIntergerTest.TestBaseIntegerFunc6(
        (-127).toByte(), -2147483648)
    arktest.assertEQ(res, 2147483521)
}

function testCaseBaseIntegerFunc6_2() {
    let res = primitivesIntergerTest.TestBaseIntegerFunc6((0).toByte(), 0)
    arktest.assertEQ(res, 0)
}

function testCaseBaseIntegerFunc7() {
    primitivesIntergerTest.TestBaseIntegerFunc7((100).toByte(), 2147483647)
}

function testCaseBaseIntegerFunc7_1() {
    try {
        primitivesIntergerTest.TestBaseIntegerFunc7((100).toByte(), -1)
    } catch (e) {
        console.log("testCaseBaseIntegerFunc7_1 error ", e.toString())
        arktest.assertEQ(
            e.toString(),
            "Error: out of range The i32 maximum value is 2147483647 and minnum values is -2147483648")
    }
}

function testCaseBaseIntegerFunc8() {
    let res = primitivesIntergerTest.TestBaseIntegerFunc8(
        (100).toByte(), 9223372036854775807)
    arktest.assertEQ(res, 9223372036854775707)
}

function testCaseBaseIntegerFunc8_1() {
    try {
        primitivesIntergerTest.TestBaseIntegerFunc8((100).toByte(), -1)
    } catch (e) {
        console.log("testCaseBaseIntegerFunc8_1 error ", e.toString())
        arktest.assertEQ(
            e.toString(),
            "Error: out of range The i64 maximum value is 9223372036854775807 and minnum values is -9223372036854775808")
    }
}

function testCaseBaseIntegerFunc8_2() {
    let res = primitivesIntergerTest.TestBaseIntegerFunc8(
        (100).toByte(), -9223372036854775808)
    arktest.assertEQ(res, 9223372036854775708)
}

function testCaseBaseIntegerFunc9() {
    let res = primitivesIntergerTest.TestBaseIntegerFunc9((100).toByte(), 227)
    arktest.assertEQ(res, 127)
}

function testCaseBaseIntegerFunc9_1() {
    let res = primitivesIntergerTest.TestBaseIntegerFunc9((-7).toByte(), -120)
    arktest.assertEQ(res, -113)
}

function testCaseBaseIntegerFunc10() {
    let res =
        primitivesIntergerTest.TestBaseIntegerFunc10((100).toByte(), 3.14f)
    arktest.assertEQ(res, 103.14f)
}

function testCaseBaseIntegerFunc10_1() {
    let res =
        primitivesIntergerTest.TestBaseIntegerFunc10((-7).toByte(), -0.0005f)
    arktest.assertEQ(res, -7.0005f)
}

function testCaseBaseIntegerFunc11() {
    let res =
        primitivesIntergerTest.TestBaseIntegerFunc11((100).toByte(), 3.14f)
    arktest.assertEQ(res, 103)
}

function testCaseBaseIntegerFunc11_1() {
    let res =
        primitivesIntergerTest.TestBaseIntegerFunc11((-7).toByte(), -0.0005f)
    arktest.assertEQ(res, -7)
}

function testCaseBaseIntegerFunc12() {
    let res =
        primitivesIntergerTest.TestBaseIntegerFunc12((100).toByte(), 3.1415926)
    arktest.assertEQ(res, 103.1415926)
}

function testCaseBaseIntegerFunc12_1() {
    let res =
        primitivesIntergerTest.TestBaseIntegerFunc12((-7).toByte(), -0.0005)
    arktest.assertEQ(res, -7.0005)
}

function testCaseBaseIntegerFunc13() {
    let res = primitivesIntergerTest.TestBaseIntegerFunc13((100).toByte(), 27)
    arktest.assertEQ(res, 127)
}

function testCaseBaseIntegerFunc13_1() {
    let res = primitivesIntergerTest.TestBaseIntegerFunc13((-7).toByte(), -121)
    arktest.assertEQ(res, -128)
}

function testCaseBaseIntegerFunc14() {
    let res = primitivesIntergerTest.TestBaseIntegerFunc14(
        (100).toByte(), "TestBaseIntegerFunc14")
    arktest.assertEQ(res, "TestBaseIntegerFunc14100")
}

function testCaseBaseIntegerFunc14_1() {
    let res = primitivesIntergerTest.TestBaseIntegerFunc14(
        (-7).toByte(), "testBaseIntegerFunc14_1")
    arktest.assertEQ(res, "testBaseIntegerFunc14_1")
}

function testCaseBaseIntegerFunc15() {
    let res = primitivesIntergerTest.TestBaseIntegerFunc15(
        (100).toByte(), "TestBaseIntegerFunc15")
    arktest.assertEQ(res, 110)
}

function testCaseBaseIntegerFunc15_1() {
    let res = primitivesIntergerTest.TestBaseIntegerFunc15(
        (-7).toByte(), "testBaseIntegerFunc15_1")
    arktest.assertEQ(res, -7)
}

function testCaseBaseIntegerFunc16() {
    let res = primitivesIntergerTest.TestBaseIntegerFunc16((100).toByte(), true)
    arktest.assertEQ(res, true)
}

function testCaseBaseIntegerFunc16_1() {
    let res = primitivesIntergerTest.TestBaseIntegerFunc16((-7).toByte(), false)
    arktest.assertEQ(res, false)
}

function testCaseBaseIntegerFunc17() {
    let res = primitivesIntergerTest.TestBaseIntegerFunc17((126).toByte(), true)
    arktest.assertEQ(res, 127)
}

function testCaseBaseIntegerFunc17_1() {
    let res =
        primitivesIntergerTest.TestBaseIntegerFunc17((-126).toByte(), false)
    arktest.assertEQ(res, -126)
}

function testCaseBaseIntegerFunc18() {
    try {
        let res =
            primitivesIntergerTest.TestBaseIntegerFunc18((32767).toShort())
    } catch (e) {
        arktest.assertEQ(
            e.toString(),
            "Error: TestBaseIntegerFunc18: result exceeds int16_t range")
    }
}

function testCaseBaseIntegerFunc18_1() {
    let res = primitivesIntergerTest.TestBaseIntegerFunc18((327).toShort())
    arktest.assertEQ(res, 3270)
}

function testCaseBaseIntegerFunc19() {
    primitivesIntergerTest.TestBaseIntegerFunc19((32767).toShort())
}

function testCaseBaseIntegerFunc19_1() {
    primitivesIntergerTest.TestBaseIntegerFunc19((-32768).toShort())
}

function testCaseBaseIntegerFunc20() {
    try {
        let res =
            primitivesIntergerTest.TestBaseIntegerFunc20((32767).toShort(), 100)
    } catch (e) {
        arktest.assertEQ(
            e.toString(),
            "Error: TestBaseIntegerFunc20: result exceeds int16_t range")
    }
}

function testCaseBaseIntegerFunc20_1() {
    let res =
        primitivesIntergerTest.TestBaseIntegerFunc20((30000).toShort(), 2767)
    arktest.assertEQ(res, 32767)
}

function testCaseBaseIntegerFunc21() {
    try {
        let res = primitivesIntergerTest.TestBaseIntegerFunc21(
            (100).toShort(), 9223372036854775807)
    } catch (e) {
        arktest.assertEQ(
            e.toString(),
            "Error: TestBaseIntegerFunc21: result exceeds int16_t range")
    }
}

function testCaseBaseIntegerFunc21_1() {
    let res =
        primitivesIntergerTest.TestBaseIntegerFunc21((-30000).toShort(), -2768)
    arktest.assertEQ(res, -32768)
}

function testCaseBaseIntegerFunc22() {
    try {
        let res = primitivesIntergerTest.TestBaseIntegerFunc22(2147483647)
    } catch (e) {
        arktest.assertEQ(
            e.toString(),
            "Error: TestBaseIntegerFunc22: result exceeds int32_t range")
    }
}

function testCaseBaseIntegerFunc22_1() {
    let res = primitivesIntergerTest.TestBaseIntegerFunc22(-21474836)
    arktest.assertEQ(res, -2147483600)
}

function testCaseBaseIntegerFunc23() {
    primitivesIntergerTest.TestBaseIntegerFunc23(2147483647)
}

function testCaseBaseIntegerFunc23_1() {
    try {
        primitivesIntergerTest.TestBaseIntegerFunc23(922337203)
    } catch (e) {
        arktest.assertEQ(
            e.toString(),
            "Error: TestBaseIntegerFunc22: result exceeds int32_t range")
    }
}

function testCaseBaseIntegerFunc24() {
    let res = primitivesIntergerTest.TestBaseIntegerFunc24(2147483646, 1)
    arktest.assertEQ(res, 2147483647)
}

function testCaseBaseIntegerFunc24_1() {
    try {
        primitivesIntergerTest.TestBaseIntegerFunc24(32767, 9223372036854775807)
    } catch (e) {
        arktest.assertEQ(
            e.toString(),
            "Error: TestBaseIntegerFunc24: result exceeds int32_t range")
    }
}

function testCaseBaseIntegerFunc25() {
    let res =
        primitivesIntergerTest.TestBaseIntegerFunc25(-2147483646, (-2).toByte())
    arktest.assertEQ(res, -2147483648)
}

function testCaseBaseIntegerFunc25_1() {
    try {
        primitivesIntergerTest.TestBaseIntegerFunc25(2147483647, (127).toByte())
    } catch (e) {
        arktest.assertEQ(
            e.toString(),
            "Error: TestBaseIntegerFunc24: result exceeds int32_t range")
    }
}

function testCaseBaseIntegerFunc26() {
    let res = primitivesIntergerTest.TestBaseIntegerFunc26(9223372036854775)
    arktest.assertEQ(res, 922337203685477500)
}

function testCaseBaseIntegerFunc26_1() {
    try {
        let res =
            primitivesIntergerTest.TestBaseIntegerFunc26(9223372036854775807)
    } catch (e) {
        arktest.assertEQ(
            e.toString(),
            "Error: TestBaseIntegerFunc24: result exceeds int64_t range")
    }
}

function testCaseBaseIntegerFunc27() {
    primitivesIntergerTest.TestBaseIntegerFunc27(-9223372036854775808)
}

function testCaseBaseIntegerFunc28() {
    let res = primitivesIntergerTest.TestBaseIntegerFunc28(
        9223372036854775, "TestBaseIntegerFunc28")
    arktest.assertEQ(res, "TestBaseIntegerFunc289223372036854775")
}

function testCaseBaseIntegerFunc28_1() {
    let res = primitivesIntergerTest.TestBaseIntegerFunc28(
        9223372036854775, "testCaseBaseIntegerFunc28_1")
    arktest.assertEQ(res, "testCaseBaseIntegerFunc28_1")
}


function testCaseBaseIntegerFunc29() {
    let res = primitivesIntergerTest.TestBaseIntegerFunc29(
        9223372036854, "TestBaseIntegerFunc29")
    arktest.assertEQ(res, 92233720368540)
}

function testCaseBaseIntegerFunc29_1() {
    try {
        let res = primitivesIntergerTest.TestBaseIntegerFunc29(
            9223372036854775807, "testBaseIntegerFunc29_1")
    } catch (e) {
        arktest.assertEQ(
            e.toString(),
            "Error: TestBaseIntegerFunc24: result exceeds int64_t range")
    }
}

function testCaseBaseIntegerFunc30() {
    let res: float = primitivesIntergerTest.TestBaseIntegerFunc30(3.1415f)
    arktest.assertEQ(res, 4.1415f)
}

function testCaseBaseIntegerFunc30_1() {
    let res: float = primitivesIntergerTest.TestBaseIntegerFunc30(-0.015f)
    arktest.assertEQ(res, 0.985f)
}

function testCaseBaseIntegerFunc31() {
    primitivesIntergerTest.TestBaseIntegerFunc31(-0.0151456f)
}

function testCaseBaseIntegerFunc31_1() {
    primitivesIntergerTest.TestBaseIntegerFunc31(1.000123456789f)
}

function testCaseBaseIntegerFunc32() {
    let res: float = primitivesIntergerTest.TestBaseIntegerFunc32(-0.015f, 1.0)
    arktest.assertEQ(res, 0.985f)
}

function testCaseBaseIntegerFunc32_1() {
    let res: float =
        primitivesIntergerTest.TestBaseIntegerFunc32(0.0001f, 100.12)
    arktest.assertEQ(res, 100.1201f)
}

function testCaseBaseIntegerFunc33() {
    let res: double =
        primitivesIntergerTest.TestBaseIntegerFunc33(123.456f, 1000.11101)
    console.log("testCaseBaseIntegerFunc33 res ", res)
    let temp: double = Math.abs(res - 1123.56701);
    console.log("testCaseBaseIntegerFunc33 temp ", temp)
    const epsilon = Math.pow(10, -5);
    console.log("testCaseBaseIntegerFunc33 epsilon ", epsilon)
    let dif: boolean = temp < epsilon
    arktest.assertEQ(dif, true)
}

function testCaseBaseIntegerFunc33_1() {
    let res: double =
        primitivesIntergerTest.TestBaseIntegerFunc33(-100.1111111f, 100)
    console.log("testCaseBaseIntegerFunc33_1 res ", res)
    let temp: double = Math.abs(res + 0.1111111);
    console.log("testCaseBaseIntegerFunc33_1 temp ", temp)
    const epsilon = Math.pow(10, -5);
    console.log("testCaseBaseIntegerFunc33_1 epsilon ", epsilon)
    let dif: boolean = temp < epsilon
    arktest.assertEQ(dif, true)
}

function testCaseBaseIntegerFunc34() {
    let res: double =
        primitivesIntergerTest.TestBaseIntegerFunc34(3.141592653589793)
    arktest.assertEQ(res, 4.141592653589793)
}

function testCaseBaseIntegerFunc34_1() {
    let res: double =
        primitivesIntergerTest.TestBaseIntegerFunc34(-0.000123456789)
    arktest.assertEQ(res, 0.999876543211)
}

function testCaseBaseIntegerFunc35() {
    primitivesIntergerTest.TestBaseIntegerFunc35(123456789.123456)
}

function testCaseBaseIntegerFunc35_1() {
    primitivesIntergerTest.TestBaseIntegerFunc35(123456789.123456789123456789)
}


function testCaseBaseIntegerFunc36() {
    let res: byte = primitivesIntergerTest.i8testattribute;
    arktest.assertEQ(res, 127);
}

function testCaseBaseIntegerFunc37() {
    let res: short = primitivesIntergerTest.i16testattribute;
    arktest.assertEQ(res, -32768);
}

function testCaseBaseIntegerFunc38() {
    let res: int = primitivesIntergerTest.i32testattribute;
    arktest.assertEQ(res, 2147483647);
}

function testCaseBaseIntegerFunc39() {
    let res: long = primitivesIntergerTest.i64testattribute;
    arktest.assertEQ(res, 9223372036854775807);
}

function testCaseBaseIntegerFunc40() {
    let res: float = primitivesIntergerTest.f32testattribute;
    arktest.assertEQ(res, 3.14f);
}

function testCaseBaseIntegerFunc41() {
    let res: double = primitivesIntergerTest.f64testattribute;
    arktest.assertEQ(res, 123.45678);
}


function testCasetestBaseBoolFunc1() {
    primitivesBoolTest.TestBaseBoolFunc1(true)
}

function testCasetestBaseBoolFunc1_1() {
    primitivesBoolTest.TestBaseBoolFunc1(false)
}

function testCasetestBaseBoolFunc2() {
    let res = primitivesBoolTest.TestBaseBoolFunc2(true)
    arktest.assertEQ(res, 65535)
}

function testCasetestBaseBoolFunc2_1() {
    let res = primitivesBoolTest.TestBaseBoolFunc2(false)
    arktest.assertEQ(res, 0)
}

function testCasetestBaseBoolFunc3() {
    let res = primitivesBoolTest.TestBaseBoolFunc3(true)
    arktest.assertEQ(res, false)
}

function testCasetestBaseBoolFunc3_1() {
    let res = primitivesBoolTest.TestBaseBoolFunc3(false)
    arktest.assertEQ(res, true)
}

function testCasetestBaseBoolFunc4() {
    let res = primitivesBoolTest.TestBaseBoolFunc4(true)
    arktest.assertEQ(res, false)
}

function testCasetestBaseBoolFunc4_1() {
    let res = primitivesBoolTest.TestBaseBoolFunc4(false)
    arktest.assertEQ(res, false)
}

function testCasetestBaseBoolFunc4_2() {
    let res = primitivesBoolTest.TestBaseBoolFunc4(undefined)
    arktest.assertEQ(res, true)
}

function testCasetestBaseBoolFunc5() {
    let p = new Promise<boolean>((resolve, reject) => {
        primitivesBoolTest.TestBaseBoolFunc5Async(
            true, (err: BusinessError|null, data?: boolean) => {
                if (err !== null && err.code !== 0) {
                    reject(err);
                } else {
                    resolve(data!);
                }
            });
    });
    let res: boolean;
    let err: BusinessError = new BusinessError();
    try {
        res = await p;
    } catch (e) {
        err = e as BusinessError;
    }
    arktest.assertEQ(res, true);
}

function testCasetestBaseBoolFunc5_1() {
    let p = new Promise<boolean>((resolve, reject) => {
        primitivesBoolTest.TestBaseBoolFunc5Async(
            false, (err: BusinessError|null, data?: boolean) => {
                if (err !== null && err.code !== 0) {
                    reject(err);
                } else {
                    resolve(data!);
                }
            });
    });
    let res: boolean;
    let err: BusinessError = new BusinessError();
    try {
        res = await p;
    } catch (e) {
        err = e as BusinessError;
    }
    arktest.assertEQ(res, false);
}

function testCasetestBaseBoolFunc5_2() {
    try {
        let result: boolean =
            await primitivesBoolTest.TestBaseBoolFunc5Promise(true);
        arktest.assertEQ(result, true);
        console.log("testCasetestBaseBoolFunc5_2 result: " + result);
    } catch (error) {
        console.error("testCasetestBaseBoolFunc5_2 failed: " + error);
    }
}

function testCasetestBaseBoolFunc5_3() {
    let result: boolean = primitivesBoolTest.TestBaseBoolFunc5(true);
    arktest.assertEQ(result, true);
}

function testCasetestBaseBoolFunc6() {
    let result: boolean =
        test_primitives.PrimitivesBooleanClass.TestBaseBoolFunc6();
    arktest.assertEQ(result, false);
}

function testCasetestBaseBoolFunc7() {
    let result: boolean = primitivesBoolTest.boolTest;
    arktest.assertEQ(result, true);
}

function testCasetestBaseBoolFunc8() {
    let testRecord: Record<string, boolean> = {
        "test1": false,
        "test2": false,
        "test3": false,
        "test4": true,
    };
    let result: boolean = primitivesBoolTest.TestBaseBoolFunc7(testRecord);
    arktest.assertEQ(result, false);
}

function testCasetestBaseBoolFunc8_1() {
    let testRecord: Record<string, boolean> = {
        "test1": true,
        "test2": false,
        "test3": false,
        "test4": true,
    };
    let result: boolean = primitivesBoolTest.TestBaseBoolFunc7(testRecord);
    arktest.assertEQ(result, false);
}

function testCasetestBaseBoolFunc8_2() {
    let testRecord: Record<string, boolean> = {
        "test1": false,
        "test2": false,
        "test3": false,
        "test4": false,
    };
    let result: boolean = primitivesBoolTest.TestBaseBoolFunc7(testRecord);
    arktest.assertEQ(result, true);
}

function testNsMyfunc1() {
    test_void_ns.mytest.Myfunc1();
}

function testNsMyfunc2() {
    test_void_ns.mytest.Myfunc2((120).toByte(), (255).toShort());
}

function testNsMyfunc3() {
    test_void_ns.mytest.Myfunc3(10000, true);
}

function testNsMyfunc3_1() {
    test_void_ns.mytest.Myfunc3(10000, false);
}

function testNsMyfunc4() {
    test_void_ns.mytest.Myfunc4(25555, 9223372036854775807);
}

function testNsMyfunc5() {
    test_void_ns.mytest.Myfunc5(2147483647, "testNsMyfunc5");
}

function testNsMyfunc6() {
    test_void_ns.mytest.Myfunc6(-9223372036854775808, true);
}

function testNsMyfunc6_1() {
    test_void_ns.mytest.Myfunc6(2147483647, false);
}

function testNsMyfunc7() {
    test_void_ns.mytest.Myfunc7(-2147483647, 3.12f);
}

function testNsMyfunc8() {
    test_void_ns.mytest.Myfunc8(9223372036854775807, 1.123456789);
}


function testNsMyfunc9() {
    test_void_ns.mytest.Myfunc9(0.000123f, true);
}

function testNsMyfunc10() {
    test_void_ns.mytest.Myfunc10(0.000123f, "testNsMyfunc10");
}

function testNsMyfunc11() {
    test_void_ns.mytest.Myfunc11(-0.000123, "testNsMyfunc11");
}

function testNsMyfunc12() {
    test_void_ns.mytest.Myfunc12(2147483647, 9223372036854775807);
}

function testNsMyfunc12_1() {
    test_void_ns.mytest.Myfunc12(undefined, -9223372036854775808);
}

function testNsMyfunc12_2() {
    test_void_ns.mytest.Myfunc12(undefined, undefined);
}

function testNsMyfunc13() {
    test_void_ns.mytest.Myfunc13(3.11111f, 1.1245645);
}

function testNsMyfunc13_1() {
    test_void_ns.mytest.Myfunc13(-3.11111f, undefined);
}

function testNsMyfunc13_2() {
    test_void_ns.mytest.Myfunc13(undefined, undefined);
}

function testNsMyfunc14() {
    test_void_ns.mytest.Myfunc14("testNsMyfunc14", true);
}

function testNsMyfunc14_1() {
    test_void_ns.mytest.Myfunc14(undefined, undefined);
}

function testNsMyfunc14_2() {
    test_void_ns.mytest.Myfunc14(undefined, false);
}

function testNsMyfunc15() {
    test_void_ns.mytest.Myfunc15((32767).toShort(), 92233720368547758);
}

function testNsMyfunc15_1() {
    test_void_ns.mytest.Myfunc15((-214748364).toShort(), undefined);
}

function testNsMyfunc15_2() {
    test_void_ns.mytest.Myfunc15(undefined, undefined);
}

function testNsMyfunc16() {
    test_void_ns.mytest.Myfunc16((255).toShort(), test_void_ns.mytest.BarTest.B)
}

function testNsMyfunc16_1() {
    test_void_ns.mytest.Myfunc16(
        (-32768).toShort(), test_void_ns.mytest.BarTest.A)
}

function testNsMyfunc16_2() {
    test_void_ns.mytest.Myfunc16((0).toShort(), test_void_ns.mytest.BarTest.C)
}

function testCaseConstant1() {
    let res: boolean = test_primitives.CONSTANT_BOOL_TRUE;
    arktest.assertEQ(res, true)
}

function testCaseConstant2() {
    let res: boolean = test_primitives.CONSTANT_BOOL_FALSE;
    arktest.assertEQ(res, false)
}

function testCaseConstant3() {
    let res: int = test_primitives.CONSTANT_I32_001;
    arktest.assertEQ(res, 10000)
}

function testCaseConstant4() {
    let res: int = test_primitives.CONSTANT_I32_002;
    arktest.assertEQ(res, 10001)
}


function main() {
    console.log("run main ... test_primitives ...")
    const suite = new arktest.ArkTestsuite("test function primitives")

    suite.addTest("check funcMultiply", testMultiply);
    suite.addTest("check testFunC1", testFunC1);
    suite.addTest("check testFunC2", testFunC2);
    suite.addTest("check testFunA1", testFunA1);
    suite.addTest("check testFunA2", testFunA2);
    suite.addTest("check testFunB1", testFunB1);
    suite.addTest("check testFunB2", testFunB2);
    suite.addTest("check testFunD1", testFunD1);
    suite.addTest("check testFunD2", testFunD2);
    suite.addTest("check testFunE", testFunE);
    suite.addTest("check testFunH1", testFunH1);
    suite.addTest("check testFunH2", testFunH2);
    suite.addTest("check testFunI1", testFunI1);
    // suite.addTest("check testFunI2", testFunI2);
    suite.addTest("check testbaseFunc1", baseFunc1);
    suite.addTest("check testbaseFunc2", baseFunc2);
    suite.addTest("check testbaseFunc3", baseFunc3);
    suite.addTest("check testbaseFunc4", baseFunc4);
    suite.addTest("check testbaseFunc5", baseFunc5);
    suite.addTest("check testbaseFunc6", baseFunc6);
    suite.addTest("check testbaseFunc7", baseFunc7);
    suite.addTest("check testbaseFunc8", baseFunc8);
    suite.addTest("check testbaseFunc6_1", baseFunc6_1);
    suite.addTest("check testbaseFunc7_1", baseFunc7_1);
    suite.addTest("check testbaseFunc8_1", baseFunc8_1);
    suite.addTest("check testbaseFunc9", baseFunc9);
    suite.addTest("check testbaseFunc10", baseFunc10);
    suite.addTest("check testbaseFunc11", baseFunc11);
    suite.addTest("check testbaseFunc11_1", baseFunc11_1);
    suite.addTest("check testbaseFunc12", baseFunc12);
    suite.addTest("check testbaseFunc13", baseFunc13);
    suite.addTest("check testbaseFunc14", baseFunc14);
    suite.addTest("check testbaseFunc14_1", baseFunc14_1);
    suite.addTest("check testbaseFunc15", baseFunc15);
    suite.addTest("check testbaseFunc16", baseFunc16);
    suite.addTest("check testbaseFunc16_1", baseFunc16_1);
    suite.addTest("check testbaseFunc17", baseFunc17);
    suite.addTest("check testbaseFunc17_1", baseFunc17_1);
    suite.addTest("check testbaseFunc18", baseFunc18);
    suite.addTest("check testbaseFunc18_1", baseFunc18_1);
    suite.addTest("check testbaseFunc19", baseFunc19);
    suite.addTest("check testbaseFunc20", baseFunc20);
    suite.addTest("check testbaseFunc20_1", baseFunc20_1);
    suite.addTest("check testbaseFunc21", baseFunc21);
    suite.addTest("check testbaseFunc21_1", baseFunc21_1);
    suite.addTest("check testbaseFunc21_2", baseFunc21_2);
    suite.addTest("check testbaseFunc21_3", baseFunc21_3);
    suite.addTest("check testbaseFunc22", baseFunc22);
    suite.addTest("check testbaseFunc22_1", baseFunc22_1);
    suite.addTest("check testbaseFunc22_2", baseFunc22_2);
    suite.addTest("check testbaseFunc22_3", baseFunc22_3);
    suite.addTest("check testbaseFunc23", baseFunc23);
    suite.addTest("check testbaseFunc23_1", baseFunc23_1);
    suite.addTest("check testbaseFunc23_2", baseFunc23_2);
    suite.addTest("check testbaseFunc23_3", baseFunc23_3);
    suite.addTest("check testbaseFunc24", baseFunc24);
    suite.addTest("check testbaseFunc24_1", baseFunc24_1);
    suite.addTest("check testbaseFunc24_2", baseFunc24_2);
    suite.addTest("check testbaseFunc24_3", baseFunc24_3);
    suite.addTest("check testCaseBaseFunc1", testCaseBaseFunc1);
    suite.addTest("check testCaseBaseFunc2", testCaseBaseFunc2);
    suite.addTest("check testCaseBaseFunc2_1", testCaseBaseFunc2_1);
    suite.addTest("check testCaseBaseFunc3", testCaseBaseFunc3);
    suite.addTest("check testCaseBaseFunc4", testCaseBaseFunc4);
    suite.addTest("check testCaseBaseFunc5", testCaseBaseFunc5);
    suite.addTest("check testCaseBaseFunc5_1", testCaseBaseFunc5_1);
    suite.addTest("check testCaseBaseFunc6", testCaseBaseFunc6);
    suite.addTest("check testCaseBaseFunc7", testCaseBaseFunc7);
    suite.addTest("check testCaseBaseFunc7_1", testCaseBaseFunc7_1);
    suite.addTest("check testCaseBaseFunc8", testCaseBaseFunc8);
    suite.addTest("check testCaseBaseFunc8_1", testCaseBaseFunc8_1);
    suite.addTest("check testCaseBaseFunc9", testCaseBaseFunc9);
    suite.addTest("check testCaseBaseFunc9_1", testCaseBaseFunc9_1);
    suite.addTest("check testCaseBaseFunc10", testCaseBaseFunc10);
    suite.addTest("check testCaseBaseFunc11", testCaseBaseFunc11);
    suite.addTest("check testCaseBaseFunc11_1", testCaseBaseFunc11_1);
    suite.addTest("check testCaseBaseFunc12", testCaseBaseFunc12);
    suite.addTest("check testCaseBaseFunc12_1", testCaseBaseFunc12_1);
    suite.addTest("check testCaseBaseFunc12_2", testCaseBaseFunc12_2);
    suite.addTest("check testCaseBaseFunc12_3", testCaseBaseFunc12_3);
    suite.addTest("check testCaseBaseFunc13", testCaseBaseFunc13);
    suite.addTest("check testCaseBaseFunc13_1", testCaseBaseFunc13_1);
    suite.addTest("check testCaseBaseFunc13_2", testCaseBaseFunc13_2);
    suite.addTest("check testCaseBaseFunc13_3", testCaseBaseFunc13_3);
    suite.addTest("check testCaseBaseFunc14", testCaseBaseFunc14);
    suite.addTest("check testCaseBaseFunc14_1", testCaseBaseFunc14_1);
    suite.addTest("check testCaseBaseFunc14_2", testCaseBaseFunc14_2);
    suite.addTest("check testCaseBaseFunc14_3", testCaseBaseFunc14_3);
    suite.addTest("check testCaseBaseFunc15", testCaseBaseFunc15);
    suite.addTest("check testCaseBaseFunc15_1", testCaseBaseFunc15_1);
    suite.addTest("check testCaseBaseFunc15_2", testCaseBaseFunc15_2);
    suite.addTest("check testCaseBaseFunc15_3", testCaseBaseFunc15_3);
    suite.addTest("check testCaseBaseFunc16", testCaseBaseFunc16);
    suite.addTest("check testCaseBaseFunc16_1", testCaseBaseFunc16_1);
    suite.addTest("check testCaseBaseFunc17", testCaseBaseFunc17);
    suite.addTest("check testCaseBaseFunc18", testCaseBaseFunc18);
    suite.addTest("check testCaseBaseFunc19", testCaseBaseFunc19);
    suite.addTest("check testCaseBaseFunc20", testCaseBaseFunc20);
    suite.addTest("check testCaseBaseIntegerFunc1", testCaseBaseIntegerFunc1);
    suite.addTest(
        "check testCaseBaseIntegerFunc1_1", testCaseBaseIntegerFunc1_1);
    suite.addTest("check testCaseBaseIntegerFunc2", testCaseBaseIntegerFunc2);
    suite.addTest(
        "check testCaseBaseIntegerFunc2_1", testCaseBaseIntegerFunc2_1);
    suite.addTest(
        "check testCaseBaseIntegerFunc2_2", testCaseBaseIntegerFunc2_2);
    suite.addTest(
        "check testCaseBaseIntegerFunc2_3", testCaseBaseIntegerFunc2_3);
    suite.addTest("check testCaseBaseIntegerFunc3", testCaseBaseIntegerFunc3);
    suite.addTest("check testCaseBaseIntegerFunc4", testCaseBaseIntegerFunc4);
    suite.addTest(
        "check testCaseBaseIntegerFunc4_1", testCaseBaseIntegerFunc4_1);
    suite.addTest(
        "check testCaseBaseIntegerFunc4_2", testCaseBaseIntegerFunc4_2);
    suite.addTest("check testCaseBaseIntegerFunc5", testCaseBaseIntegerFunc5);
    suite.addTest(
        "check testCaseBaseIntegerFunc5_1", testCaseBaseIntegerFunc5_1);
    suite.addTest(
        "check testCaseBaseIntegerFunc5_2", testCaseBaseIntegerFunc5_2);
    suite.addTest("check testCaseBaseIntegerFunc6", testCaseBaseIntegerFunc6);
    suite.addTest(
        "check testCaseBaseIntegerFunc6_1", testCaseBaseIntegerFunc6_1);
    suite.addTest(
        "check testCaseBaseIntegerFunc6_2", testCaseBaseIntegerFunc6_2);
    suite.addTest("check testCaseBaseIntegerFunc7", testCaseBaseIntegerFunc7);
    suite.addTest(
        "check testCaseBaseIntegerFunc7_1", testCaseBaseIntegerFunc7_1);
    suite.addTest("check testCaseBaseIntegerFunc8", testCaseBaseIntegerFunc8);
    suite.addTest(
        "check testCaseBaseIntegerFunc8_1", testCaseBaseIntegerFunc8_1);
    suite.addTest(
        "check testCaseBaseIntegerFunc8_2", testCaseBaseIntegerFunc8_2);
    suite.addTest("check testCaseBaseIntegerFunc9", testCaseBaseIntegerFunc9);
    suite.addTest(
        "check testCaseBaseIntegerFunc9_1", testCaseBaseIntegerFunc9_1);
    suite.addTest("check testCaseBaseIntegerFunc10", testCaseBaseIntegerFunc10);
    suite.addTest(
        "check testCaseBaseIntegerFunc10_1", testCaseBaseIntegerFunc10_1);
    suite.addTest("check testCaseBaseIntegerFunc11", testCaseBaseIntegerFunc11);
    suite.addTest(
        "check testCaseBaseIntegerFunc11_1", testCaseBaseIntegerFunc11_1);
    suite.addTest("check testCaseBaseIntegerFunc12", testCaseBaseIntegerFunc12);
    suite.addTest(
        "check testCaseBaseIntegerFunc12_1", testCaseBaseIntegerFunc12_1);
    suite.addTest("check testCaseBaseIntegerFunc13", testCaseBaseIntegerFunc13);
    suite.addTest(
        "check testCaseBaseIntegerFunc13_1", testCaseBaseIntegerFunc13_1);
    suite.addTest("check testCaseBaseIntegerFunc14", testCaseBaseIntegerFunc14);
    suite.addTest(
        "check testCaseBaseIntegerFunc14_1", testCaseBaseIntegerFunc14_1);
    suite.addTest("check testCaseBaseIntegerFunc15", testCaseBaseIntegerFunc15);
    suite.addTest(
        "check testCaseBaseIntegerFunc15_1", testCaseBaseIntegerFunc15_1);
    suite.addTest("check testCaseBaseIntegerFunc16", testCaseBaseIntegerFunc16);
    suite.addTest(
        "check testCaseBaseIntegerFunc16_1", testCaseBaseIntegerFunc16_1);
    suite.addTest("check testCaseBaseIntegerFunc17", testCaseBaseIntegerFunc17);
    suite.addTest(
        "check testCaseBaseIntegerFunc17_1", testCaseBaseIntegerFunc17_1);
    suite.addTest("check testCaseBaseIntegerFunc18", testCaseBaseIntegerFunc18);
    suite.addTest(
        "check testCaseBaseIntegerFunc18_1", testCaseBaseIntegerFunc18_1);
    suite.addTest("check testCaseBaseIntegerFunc19", testCaseBaseIntegerFunc19);
    suite.addTest(
        "check testCaseBaseIntegerFunc19_1", testCaseBaseIntegerFunc19_1);
    suite.addTest("check testCaseBaseIntegerFunc20", testCaseBaseIntegerFunc20);
    suite.addTest(
        "check testCaseBaseIntegerFunc20_1", testCaseBaseIntegerFunc20_1);
    suite.addTest("check testCaseBaseIntegerFunc21", testCaseBaseIntegerFunc21);
    suite.addTest(
        "check testCaseBaseIntegerFunc21_1", testCaseBaseIntegerFunc21_1);
    suite.addTest("check testCaseBaseIntegerFunc22", testCaseBaseIntegerFunc22);
    suite.addTest(
        "check testCaseBaseIntegerFunc22_1", testCaseBaseIntegerFunc22_1);
    suite.addTest("check testCaseBaseIntegerFunc23", testCaseBaseIntegerFunc23);
    suite.addTest(
        "check testCaseBaseIntegerFunc23_1", testCaseBaseIntegerFunc23_1);
    suite.addTest("check testCaseBaseIntegerFunc24", testCaseBaseIntegerFunc24);
    suite.addTest(
        "check testCaseBaseIntegerFunc24_1", testCaseBaseIntegerFunc24_1);
    suite.addTest("check testCaseBaseIntegerFunc25", testCaseBaseIntegerFunc25);
    suite.addTest(
        "check testCaseBaseIntegerFunc25_1", testCaseBaseIntegerFunc25_1);
    suite.addTest("check testCaseBaseIntegerFunc26", testCaseBaseIntegerFunc26);
    suite.addTest(
        "check testCaseBaseIntegerFunc26_1", testCaseBaseIntegerFunc26_1);
    suite.addTest("check testCaseBaseIntegerFunc27", testCaseBaseIntegerFunc27);
    suite.addTest("check testCaseBaseIntegerFunc28", testCaseBaseIntegerFunc28);
    suite.addTest(
        "check testCaseBaseIntegerFunc28_1", testCaseBaseIntegerFunc28_1);
    suite.addTest("check testCaseBaseIntegerFunc29", testCaseBaseIntegerFunc29);
    suite.addTest(
        "check testCaseBaseIntegerFunc29_1", testCaseBaseIntegerFunc29_1);
    suite.addTest("check testCaseBaseIntegerFunc30", testCaseBaseIntegerFunc30);
    suite.addTest(
        "check testCaseBaseIntegerFunc30_1", testCaseBaseIntegerFunc30_1);
    suite.addTest("check testCaseBaseIntegerFunc31", testCaseBaseIntegerFunc31);
    suite.addTest(
        "check testCaseBaseIntegerFunc31_1", testCaseBaseIntegerFunc31_1);
    suite.addTest("check testCaseBaseIntegerFunc32", testCaseBaseIntegerFunc32);
    suite.addTest(
        "check testCaseBaseIntegerFunc32_1", testCaseBaseIntegerFunc32_1);
    suite.addTest("check testCaseBaseIntegerFunc33", testCaseBaseIntegerFunc33);
    suite.addTest(
        "check testCaseBaseIntegerFunc33_1", testCaseBaseIntegerFunc33_1);
    suite.addTest("check testCaseBaseIntegerFunc34", testCaseBaseIntegerFunc34);
    suite.addTest(
        "check testCaseBaseIntegerFunc34_1", testCaseBaseIntegerFunc34_1);
    suite.addTest("check testCaseBaseIntegerFunc35", testCaseBaseIntegerFunc35);
    suite.addTest(
        "check testCaseBaseIntegerFunc35_1", testCaseBaseIntegerFunc35_1);
    suite.addTest("check testCaseBaseIntegerFunc36", testCaseBaseIntegerFunc36);
    suite.addTest("check testCaseBaseIntegerFunc37", testCaseBaseIntegerFunc37);
    suite.addTest("check testCaseBaseIntegerFunc38", testCaseBaseIntegerFunc38);
    suite.addTest("check testCaseBaseIntegerFunc39", testCaseBaseIntegerFunc39);
    suite.addTest("check testCaseBaseIntegerFunc40", testCaseBaseIntegerFunc40);
    suite.addTest("check testCaseBaseIntegerFunc41", testCaseBaseIntegerFunc41);
    suite.addTest("check testCasetestBaseBoolFunc1", testCasetestBaseBoolFunc1);
    suite.addTest(
        "check testCasetestBaseBoolFunc1_1", testCasetestBaseBoolFunc1_1);
    suite.addTest("check testCasetestBaseBoolFunc2", testCasetestBaseBoolFunc2);
    suite.addTest(
        "check testCasetestBaseBoolFunc2_1", testCasetestBaseBoolFunc2_1);
    suite.addTest("check testCasetestBaseBoolFunc3", testCasetestBaseBoolFunc3);
    suite.addTest(
        "check testCasetestBaseBoolFunc3_1", testCasetestBaseBoolFunc3_1);
    suite.addTest("check testCasetestBaseBoolFunc4", testCasetestBaseBoolFunc4);
    suite.addTest(
        "check testCasetestBaseBoolFunc4_1", testCasetestBaseBoolFunc4_1);
    suite.addTest(
        "check testCasetestBaseBoolFunc4_2", testCasetestBaseBoolFunc4_2);
    suite.addTest("check testCasetestBaseBoolFunc5", testCasetestBaseBoolFunc5);
    suite.addTest(
        "check testCasetestBaseBoolFunc5_1", testCasetestBaseBoolFunc5_1);
    suite.addTest(
        "check testCasetestBaseBoolFunc5_2", testCasetestBaseBoolFunc5_2);
    suite.addTest(
        "check testCasetestBaseBoolFunc5_3", testCasetestBaseBoolFunc5_3);
    suite.addTest("check testCasetestBaseBoolFunc6", testCasetestBaseBoolFunc6);
    suite.addTest("check testCasetestBaseBoolFunc7", testCasetestBaseBoolFunc7);
    suite.addTest("check testCasetestBaseBoolFunc8", testCasetestBaseBoolFunc8);
    suite.addTest(
        "check testCasetestBaseBoolFunc8_1", testCasetestBaseBoolFunc8_1);
    suite.addTest(
        "check testCasetestBaseBoolFunc8_2", testCasetestBaseBoolFunc8_2);
    // namspacevoid test
    suite.addTest("check testNsMyfunc1", testNsMyfunc1);
    suite.addTest("check testNsMyfunc2", testNsMyfunc2);
    suite.addTest("check testNsMyfunc3", testNsMyfunc3);
    suite.addTest("check testNsMyfunc3_1", testNsMyfunc3_1);
    suite.addTest("check testNsMyfunc4", testNsMyfunc4);
    suite.addTest("check testNsMyfunc5", testNsMyfunc5);
    suite.addTest("check testNsMyfunc6", testNsMyfunc6);
    suite.addTest("check testNsMyfunc6_1", testNsMyfunc6_1);
    suite.addTest("check testNsMyfunc7", testNsMyfunc7);
    suite.addTest("check testNsMyfunc8", testNsMyfunc8);
    suite.addTest("check testNsMyfunc9", testNsMyfunc9);
    suite.addTest("check testNsMyfunc10", testNsMyfunc10);
    suite.addTest("check testNsMyfunc11", testNsMyfunc11);
    suite.addTest("check testNsMyfunc12", testNsMyfunc12);
    suite.addTest("check testNsMyfunc12_1", testNsMyfunc12_1);
    suite.addTest("check testNsMyfunc12_2", testNsMyfunc12_2);
    suite.addTest("check testNsMyfunc13", testNsMyfunc13);
    suite.addTest("check testNsMyfunc13_1", testNsMyfunc13_1);
    suite.addTest("check testNsMyfunc13_2", testNsMyfunc13_2);
    suite.addTest("check testNsMyfunc14", testNsMyfunc14);
    suite.addTest("check testNsMyfunc14_1", testNsMyfunc14_1);
    suite.addTest("check testNsMyfunc14_2", testNsMyfunc14_2);
    suite.addTest("check testNsMyfunc15", testNsMyfunc15);
    suite.addTest("check testNsMyfunc15_1", testNsMyfunc15_1);
    suite.addTest("check testNsMyfunc15_2", testNsMyfunc15_2);
    suite.addTest("check testNsMyfunc16", testNsMyfunc16);
    suite.addTest("check testNsMyfunc16_1", testNsMyfunc16_1);
    suite.addTest("check testNsMyfunc16_2", testNsMyfunc16_2);
    suite.addTest("check testCaseConstant1", testCaseConstant1);
    suite.addTest("check testCaseConstant2", testCaseConstant2);
    suite.addTest("check testCaseConstant3", testCaseConstant3);
    suite.addTest("check testCaseConstant4", testCaseConstant4);
    exit(suite.run())
}
