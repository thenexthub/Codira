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
import * as namespace2Test from "my_ns";
import * as namespaceTest from "ns_alltest";

loadLibrary("ani_namespace");

let nameInterfaceTest: namespaceTest.functiontest.TestNameSpace =
    namespaceTest.functiontest.get_interface_NameSpace();
let testInterfacePerformance1:
    namespaceTest.functiontest.TestInterfacePerformance1 =
    namespaceTest.functiontest.get_interface_performance1();
let testInterfacePerformance2:
    namespaceTest.functiontest.TestInterfacePerformance2 =
    namespaceTest.functiontest.get_interface_performance2();
let testInterfacePerformance3:
    namespaceTest.functiontest.TestInterfacePerformance3 =
    namespaceTest.functiontest.get_interface_performance3();
let testInterfacePerformance4:
    namespaceTest.functiontest.TestInterfacePerformance4 =
    namespaceTest.functiontest.get_interface_performance4();
let testInterfacePerformance5:
    namespaceTest.functiontest.TestInterfacePerformance5 =
    namespaceTest.functiontest.get_interface_performance5();
let testInterfacePerformance6:
    namespaceTest.functiontest.TestInterfacePerformance6 =
    namespaceTest.functiontest.get_interface_performance6();
let testInterfacePerformance7:
    namespaceTest.functiontest.TestInterfacePerformance7 =
    namespaceTest.functiontest.get_interface_performance7();
let testInterfacePerformance8:
    namespaceTest.functiontest.TestInterfacePerformance8 =
    namespaceTest.functiontest.get_interface_performance8();
let testInterfacePerformance9:
    namespaceTest.functiontest.TestInterfacePerformance9 =
    namespaceTest.functiontest.get_interface_performance9();
let testInterfacePerformance10:
    namespaceTest.functiontest.TestInterfacePerformance10 =
    namespaceTest.functiontest.get_interface_performance10();
let testInterfacePerformance11:
    namespaceTest.functiontest.TestInterfacePerformance11 =
    namespaceTest.functiontest.get_interface_performance11();
let testInterfacePerformance12:
    namespaceTest.functiontest.TestInterfacePerformance12 =
    namespaceTest.functiontest.get_interface_performance12();
let testInterfacePerformance13:
    namespaceTest.functiontest.TestInterfacePerformance13 =
    namespaceTest.functiontest.get_interface_performance13();
let testInterfacePerformance14:
    namespaceTest.functiontest.TestInterfacePerformance14 =
    namespaceTest.functiontest.get_interface_performance14();
let testInterfacePerformance15:
    namespaceTest.functiontest.TestInterfacePerformance15 =
    namespaceTest.functiontest.get_interface_performance15();
let testInterfacePerformance16:
    namespaceTest.functiontest.TestInterfacePerformance16 =
    namespaceTest.functiontest.get_interface_performance16();
let testInterfacePerformance17:
    namespaceTest.functiontest.TestInterfacePerformance17 =
    namespaceTest.functiontest.get_interface_performance17();
let testInterfacePerformance18:
    namespaceTest.functiontest.TestInterfacePerformance18 =
    namespaceTest.functiontest.get_interface_performance18();
let testInterfacePerformance19:
    namespaceTest.functiontest.TestInterfacePerformance19 =
    namespaceTest.functiontest.get_interface_performance19();
let testInterfacePerformance20:
    namespaceTest.functiontest.TestInterfacePerformance20 =
    namespaceTest.functiontest.get_interface_performance20();
let testInterfacePerformance21:
    namespaceTest.functiontest.TestInterfacePerformance21 =
    namespaceTest.functiontest.get_interface_performance21();

let testIbaseInterface2: namespaceTest.functiontest.IbaseInterface2 =
    namespaceTest.functiontest.get_interface_IbaseInterface2();

let testIbaseInterface3: namespaceTest.functiontest.IbaseInterface3 =
    namespaceTest.functiontest.get_interface_IbaseInterface3();

function testCasebaseFunctionTest1() {
    namespaceTest.functiontest.BaseFunctionTest1()
}

function testCasebaseFunctionTest2() {
    namespaceTest.functiontest.BaseFunctionTest2((127).toByte())
}

function testCasebaseFunctionTest3() {
    namespaceTest.functiontest.BaseFunctionTest3((255).toShort())
}

function testCasebaseFunctionTest4() {
    namespaceTest.functiontest.BaseFunctionTest4(2147483647)
}

function testCasebaseFunctionTest5() {
    namespaceTest.functiontest.BaseFunctionTest5(9223372036854775807)
}

function testCasebaseFunctionTest6() {
    namespaceTest.functiontest.BaseFunctionTest6((3.1415).toFloat())
}

function testCasebaseFunctionTest7() {
    namespaceTest.functiontest.BaseFunctionTest7(3.1415125)
}

function testCasebaseFunctionTest8() {
    namespaceTest.functiontest.BaseFunctionTest8("baseFunctionTest8!@#$%%^&*()")
}

function testCasebaseFunctionTest9() {
    namespaceTest.functiontest.BaseFunctionTest9(false)
}

function testCasebaseFunctionTest10() {
    let byteArray: byte[] = [1, 127, -120, 0, 100];
    namespaceTest.functiontest.BaseFunctionTest10(byteArray)
}

function testCasebaseFunctionTest11() {
    let shortArray: short[] = [32767, -32768, 122, 50, 10086];
    namespaceTest.functiontest.BaseFunctionTest11(shortArray)
}

function testCasebaseFunctionTest12() {
    namespaceTest.functiontest.BaseFunctionTest12((255).toShort())
}

function testCasebaseFunctionTest12_1() {
    namespaceTest.functiontest.BaseFunctionTest12(undefined)
}

function testCasebaseFunctionTest13() {
    namespaceTest.functiontest.BaseFunctionTest13(9223372036854775807)
}

function testCasebaseFunctionTest13_1() {
    namespaceTest.functiontest.BaseFunctionTest13(undefined)
}

function testCasebaseFunctionTest14() {
    let numbersU8: byte[] = [1, 2, 3, 4, 5];
    let arrbuf1: ArrayBuffer = new ArrayBuffer(numbersU8.length);
    for (let i = 0; i < numbersU8.length; i++) {
        arrbuf1.set(i, numbersU8[i]);
    }
    namespaceTest.functiontest.BaseFunctionTest14(arrbuf1)
}

function testCasebaseFunctionTest15() {
    let record: Record<string, int> = {
        "test1": 0,
        "test2": 1,
        "test3": 2,
        "test4": 3,
    };
    namespaceTest.functiontest.BaseFunctionTest15(record)
}

function testCasebaseFunctionTest16() {
    let res = namespaceTest.functiontest.BaseFunctionTest16((110).toByte());
    arktest.assertEQ(res, 120)
}

function testCasebaseFunctionTest16_1() {
    try {
        let res = namespaceTest.functiontest.BaseFunctionTest16((127).toByte());
    } catch (e) {
        arktest.assertEQ(
            e.toString(),
            "Error: BaseFunctionTest16: result exceeds int8_t range")
    }
}

function testCasebaseFunctionTest17() {
    let res = namespaceTest.functiontest.BaseFunctionTest17((3270).toShort());
    arktest.assertEQ(res, 32700)
}

function testCasebaseFunctionTest17_1() {
    try {
        let res =
            namespaceTest.functiontest.BaseFunctionTest17((32767).toShort());
    } catch (e) {
        arktest.assertEQ(
            e.toString(),
            "Error: BaseFunctionTest17: result exceeds int16_t range")
    }
}

function testCasebaseFunctionTest18() {
    let res = namespaceTest.functiontest.BaseFunctionTest18(21474836);
    arktest.assertEQ(res, 2147483600)
}

function testCasebaseFunctionTest18_1() {
    try {
        let res = namespaceTest.functiontest.BaseFunctionTest18(21474837);
    } catch (e) {
        console.log("testCasebaseFunctionTest18_1 error ", e.toString())
        arktest.assertEQ(
            e.toString(),
            "Error: BaseFunctionTest18: result exceeds int32_t range")
    }
}

function testCasebaseFunctionTest19() {
    let res = namespaceTest.functiontest.BaseFunctionTest19(92233720368547758);
    arktest.assertEQ(res, 922337203685477580)
}

function testCasebaseFunctionTest20() {
    let res = namespaceTest.functiontest.BaseFunctionTest20((3.14).toFloat());
    arktest.assertEQ(res, (103.14).toFloat())
}

function testCasebaseFunctionTest21() {
    let res = namespaceTest.functiontest.BaseFunctionTest21(1);
    arktest.assertEQ(res, 2.01)
}

function testCasebaseFunctionTest22() {
    let res =
        namespaceTest.functiontest.BaseFunctionTest22("BaseFunctionTest22");
    arktest.assertEQ(res, "BaseFunctionTest22hello ani")
}

function testCasebaseFunctionTest22_1() {
    let res =
        namespaceTest.functiontest.BaseFunctionTest22("baseFunctionTest22——1");
    arktest.assertEQ(res, "baseFunctionTest22——1")
}

function testCasebaseFunctionTest23() {
    let res = namespaceTest.functiontest.BaseFunctionTest23(true);
    arktest.assertEQ(res, false)
}

function testCasebaseFunctionTest23_1() {
    let res = namespaceTest.functiontest.BaseFunctionTest23(false);
    arktest.assertEQ(res, true)
}

function testCasebaseFunctionTest24() {
    let byteArray: byte[] = [1, 2, 3, 4, 5];
    let res: byte[] = namespaceTest.functiontest.BaseFunctionTest24(byteArray);
    let temp: byte[] = [2, 4, 6, 8, 10];
    for (let i = 0; i < res.length; i++) {
        arktest.assertEQ(res[i], temp[i])
    }
}

function testCasebaseFunctionTest25() {
    let shortArray: short[] = [1, 2, 3, 4, 5];
    let res = namespaceTest.functiontest.BaseFunctionTest25(shortArray);
    let temp: byte[] = [3, 4, 5, 6, 7];
    for (let i = 0; i < res.length; i++) {
        arktest.assertEQ(res[i], temp[i])
    }
}

function testCasebaseFunctionTest26() {
    let res = namespaceTest.functiontest.BaseFunctionTest26((3276).toShort());
    arktest.assertEQ(res, 3286)
}

function testCasebaseFunctionTest26_1() {
    let res = namespaceTest.functiontest.BaseFunctionTest26(undefined);
    arktest.assertEQ(res, undefined)
}

function testCasebaseFunctionTest27() {
    let res = namespaceTest.functiontest.BaseFunctionTest27(922337203685477580);
    arktest.assertEQ(res, 922337203685477590)
}

function testCasebaseFunctionTest27_1() {
    let res = namespaceTest.functiontest.BaseFunctionTest27(undefined);
    arktest.assertEQ(res, undefined)
}

function testCasebaseFunctionTest28() {
    let numbersU8: byte[] = [11, 2, 3, 4, 5];
    let arrbuf1: ArrayBuffer = new ArrayBuffer(numbersU8.length);
    for (let i = 0; i < numbersU8.length; i++) {
        arrbuf1.set(i, numbersU8[i]);
    }
    let res = namespaceTest.functiontest.BaseFunctionTest28(arrbuf1);
    console.log("testCasebaseFunctionTest28 length: " + res.getByteLength());
    console.log("testCasebaseFunctionTest28 : " + res);
    arktest.assertEQ(res.getByteLength(), 5)
    for (let i = 0; i < res.getByteLength(); i++) {
        console.log(
            `testCasebaseFunctionTest28 Byte at index ${i}: ${res.at(i)}`);
        arktest.assertEQ(res.at(i), numbersU8[i] * 10)
    }
}

function testCasebaseFunctionTest29() {
    const myRecord: Record<string, int> = {
        "key1": 10,
        "key2": 20,
        "key3": 30,
    };

    const resultRecord: Record<string, int> = {
        "test0": 10,
        "test1": 11,
        "test2": 12,
    };
    let res = namespaceTest.functiontest.BaseFunctionTest29(myRecord);
    console.log("testCasebaseFunctionTest29 ", res);
    console.log("testCasebaseFunctionTest29 test0 ", res["test0"]);
    arktest.assertEQ(res["test0"], resultRecord["test0"]);
    arktest.assertEQ(res["test1"], resultRecord["test1"]);
    arktest.assertEQ(res["test2"], resultRecord["test2"]);
}

function testCasebaseFunctionTest30() {
    let color = namespaceTest.functiontest.Color.BLUE;
    let res = namespaceTest.functiontest.BaseFunctionTest30(color);
    console.log("testCasebaseFunctionTest30 ", res)
    arktest.assertEQ(res, namespaceTest.functiontest.Color.RED)
}

function testCasebaseFunctionTest31() {
    let color = namespaceTest.functiontest.Color.RED;
    namespaceTest.functiontest.BaseFunctionTest31(color);
}

function testCasebaseFunctionTest32() {
    let res = namespaceTest.functiontest.BaseFunctionTest32(
        (127).toByte(), (100).toShort(), 256, 10000, true);
    arktest.assertEQ(res, 100)
}

function testCasebaseFunctionTest32_1() {
    let res = namespaceTest.functiontest.BaseFunctionTest32(
        (127).toByte(), (100).toShort(), 256, 10000, false);
    arktest.assertEQ(res, 127)
}

function testCasebaseFunctionTest33() {
    let res = namespaceTest.functiontest.BaseFunctionTest33(
        (127).toByte(), (100).toShort(), 256, 10000, true);
    arktest.assertEQ(res, 256)
}

function testCasebaseFunctionTest33_1() {
    let res = namespaceTest.functiontest.BaseFunctionTest33(
        (127).toByte(), (100).toShort(), 256, 10000, false);
    arktest.assertEQ(res, 100)
}

function testCasebaseFunctionTest34() {
    let res = namespaceTest.functiontest.BaseFunctionTest34(
        (127).toByte(), (100).toShort(), 256, 10000, true);
    arktest.assertEQ(res, 10000)
}

function testCasebaseFunctionTest34_1() {
    let res = namespaceTest.functiontest.BaseFunctionTest34(
        (127).toByte(), (100).toShort(), 256, 10000, false);
    arktest.assertEQ(res, 100)
}

function testCasebaseFunctionTest35() {
    let res = namespaceTest.functiontest.BaseFunctionTest35(
        (127).toByte(), (100).toShort(), 256, 10000, true);
    arktest.assertEQ(res, 127)
}

function testCasebaseFunctionTest35_1() {
    let res = namespaceTest.functiontest.BaseFunctionTest35(
        (127).toByte(), (100).toShort(), 256, 10000, false);
    arktest.assertEQ(res, 100)
}

function testCasebaseFunctionTest36() {
    let int8Array: byte[] = [1, 2, 3, 4, 5];
    let int64Array: long[] =
        [9223372036854775807, -100, 121, -2147483648, -9223372036854775808];
    let res = namespaceTest.functiontest.BaseFunctionTest36(
        (10).toByte(), (11).toShort(), true, int8Array, int64Array);
    arktest.assertEQ(res.length, int8Array.length)
    let tempResult: int[] = [22, 23, 24, 25, 26];
    for (let i: int = 0; i < res.length; i++) {
        arktest.assertEQ(res[i], tempResult[i])
    }
}

function testCasebaseFunctionTest36_1() {
    let int8Array: byte[] = [1, 2, 3, 4, 5];
    let int64Array: long[] = [100, -100, 200, -300, -400];
    let res = namespaceTest.functiontest.BaseFunctionTest36(
        (127).toByte(), (100).toShort(), false, int8Array, int64Array);
    arktest.assertEQ(res.length, int64Array.length)
    for (let i: int = 0; i < res.length; i++) {
        arktest.assertEQ(res[i], int64Array[i])
    }
}

function testCasebaseFunctionTest37() {
    let intArray: int[] = [922337, -100, 121, -2147483648, -92233720];
    let int64Array: long[] = [100, -100, 121, -200, 0];
    let res = namespaceTest.functiontest.BaseFunctionTest37(
        (10).toByte(), (11).toShort(), 12, true, int64Array);
    arktest.assertEQ(res.length, int64Array.length)
    let tempResult: int[] = [133, -67, 154, -167, 33];
    for (let i: int = 0; i < res.length; i++) {
        arktest.assertEQ(res[i], tempResult[i])
    }
}

function testCasebaseFunctionTest37_1() {
    let intArray: int[] = [922337, -100, 121, -2147483648, -92233720];
    let int64Array: long[] = [100, -100, 200, -300, -400];
    let res = namespaceTest.functiontest.BaseFunctionTest37(
        (127).toByte(), (100).toShort(), 256, false, int64Array);
    arktest.assertEQ(res.length, int64Array.length)
    for (let i: int = 0; i < res.length; i++) {
        arktest.assertEQ(res[i], int64Array[i] * 10)
    }
}

function testCasebaseFunctionTest38() {
    let byteArray: byte[] = [1, 2, 3, 4, 5];
    let int16Array: short[] = [100, -100, 121, -200, -500];
    let res = namespaceTest.functiontest.BaseFunctionTest38(
        (10).toByte(), "test1", true, byteArray, int16Array);
    arktest.assertEQ(res, "test110")
}

function testCasebaseFunctionTest38_1() {
    let byteArray: byte[] = [1, 2, 3, 4, 5];
    let int16Array: short[] = [100, -100, 121, -200, -500];
    let res = namespaceTest.functiontest.BaseFunctionTest38(
        (127).toByte(), "test1", false, byteArray, int16Array);
    arktest.assertEQ(res, "test11100")
}

function testCasebaseFunctionTest39() {
    let boolArray: boolean[] = [true, false, true, false, true];
    let intArray: int[] = [922337, -100, 121, -2147483648, -92233720];
    let res = namespaceTest.functiontest.BaseFunctionTest39(
        (11).toShort(), "test1", true, boolArray, intArray);
    arktest.assertEQ(res, false)
}

function testCasebaseFunctionTest39_1() {
    let boolArray: boolean[] = [true, false, true, false, true];
    let intArray: int[] = [922337, -100, 121, -2147483648, -92233720];
    let res = namespaceTest.functiontest.BaseFunctionTest39(
        (100).toShort(), "test1", false, boolArray, intArray);
    arktest.assertEQ(res, false)
}

function testCasebaseFunctionTest40() {
    let byteArray: byte[] = [1, 2, 3, 4, 5];
    let numbersU8: byte[] = [10, 20, 30, 40, 50];
    let arrbuf1: ArrayBuffer = new ArrayBuffer(numbersU8.length);
    for (let i = 0; i < numbersU8.length; i++) {
        arrbuf1.set(i, numbersU8[i])
    }
    let res = namespaceTest.functiontest.BaseFunctionTest40(
        -2.0002, "test1", true, byteArray, arrbuf1);
    arktest.assertEQ(res.length, byteArray.length)
    for (let i: int = 0; i < res.length; i++) {
        arktest.assertEQ(res[i], byteArray[i] * 2)
    }
}

function testCasebaseFunctionTest40_1() {
    let byteArray: byte[] = [1, 2, 3, 4, 5];
    let numbersU8: byte[] = [10, 20, 30, 40, 50];
    let arrbuf1: ArrayBuffer = new ArrayBuffer(numbersU8.length);
    for (let i = 0; i < numbersU8.length; i++) {
        arrbuf1.set(i, numbersU8[i])
    }
    let res = namespaceTest.functiontest.BaseFunctionTest40(
        -2.0002, "test1", false, byteArray, arrbuf1);
    arktest.assertEQ(res.length, byteArray.length)
    for (let i: int = 0; i < res.length; i++) {
        arktest.assertEQ(res[i], byteArray[i] + 10)
    }
}

function testCasebaseFunctionTest41() {
    namespaceTest.functiontest.BaseFunctionTest41(
        (10).toByte(), (11).toShort(), 12, 10000, (3.14).toFloat());
}
function testCasebaseFunctionTest42() {
    let res =
        namespaceTest.functiontest.BaseFunctionTest42(10000, (256).toShort());
    arktest.assertEQ(res, 10256)
}

function testCasebaseFunctionTest42_1() {
    try {
        let res = namespaceTest.functiontest.BaseFunctionTest42(
            2147483647, (256).toShort());
    } catch (e) {
        console.log("testCasebaseFunctionTest42_1 e ", e.toString())
        arktest.assertEQ(
            e.toString(),
            "Error: BaseFunctionTest42_int: result exceeds int32_t range")
    }
}

function testCasebaseFunctionTest43() {
    let record: Record<string, int> = {
        "test1": 0,
        "test2": 1,
        "test3": 2,
    };
    let res = namespaceTest.functiontest.BaseFunctionTest42(10000, record);
    arktest.assertEQ(res, 10010)
}

function testCasebaseFunctionTest43_1() {
    let record: Record<string, int> = {
        "test1": 0,
        "test2": 1,
        "test3": 2,
    };
    let res = namespaceTest.functiontest.BaseFunctionTest42(undefined, record);
    arktest.assertEQ(res, -2147483648)
}

function testCasebaseFunctionTest44() {
    let res = namespaceTest.functiontest.BaseFunctionTest42();
    arktest.assertEQ(res, 2147483647)
}

function testCasebaseFunctionTest45() {
    const testData: namespaceTest.functiontest
        .Data = {data1: "baseFunctionTest42", data2: true, data3: 2147483647};
    namespaceTest.functiontest.BaseFunctionTest42(testData);
}


function testCasebaseFunctionTest46() {
    nameInterfaceTest.BaseFunctionTest43();
}

function testCasebaseFunctionTest47() {
    let res = nameInterfaceTest.BaseFunctionTest44((-10).toByte());
    arktest.assertEQ(res, 90)
}

function testCasebaseFunctionTest48() {
    let res = nameInterfaceTest.BaseFunctionTest45((320).toShort());
    arktest.assertEQ(res, 32000)
}

function testCasebaseFunctionTest49() {
    let res = nameInterfaceTest.BaseFunctionTest46(2147483);
    arktest.assertEQ(res, 214748300)
}

function testCasebaseFunctionTest50() {
    let res = nameInterfaceTest.BaseFunctionTest47(9223372036854775);
    arktest.assertEQ(res, 922337203685477500)
}

function testCasebaseFunctionTest51() {
    let res = nameInterfaceTest.BaseFunctionTest48((100.123).toFloat());
    arktest.assertEQ(res, (90.123).toFloat())
}

function testCasebaseFunctionTest52() {
    let res = nameInterfaceTest.BaseFunctionTest49(200.66);
    arktest.assertEQ(res, 100.66)
}

function testCasebaseFunctionTest53() {
    let res =
        nameInterfaceTest.BaseFunctionTest50("testCasebaseFunctionTest53");
    arktest.assertEQ(res, "testCasebaseFunctionTest53BaseFunctionTest50")
}

function testCasebaseFunctionTest54() {
    let res = nameInterfaceTest.BaseFunctionTest51(true);
    arktest.assertEQ(res, false)
}

function testCasebaseFunctionTest54_1() {
    let res = nameInterfaceTest.BaseFunctionTest51(false);
    arktest.assertEQ(res, true)
}

function testCasebaseFunctionTest55() {
    let numbersU8: byte[] = [100, 101, 102, 103, 104];
    let arrbuf1: ArrayBuffer = new ArrayBuffer(numbersU8.length);
    for (let i = 0; i < numbersU8.length; i++) {
        arrbuf1.set(i, numbersU8[i]);
    }
    let res = nameInterfaceTest.BaseFunctionTest52(arrbuf1);
    arktest.assertEQ(res.getByteLength(), 5)
    for (let i = 0; i < res.getByteLength(); i++) {
        console.log(
            `testCasebaseFunctionTest55 Byte at index ${i}: ${res.at(i)}`);
        arktest.assertEQ(res.at(i), numbersU8[i] + 10)
    }
}

function testCasebaseFunctionTest56() {
    let res = nameInterfaceTest.BaseFunctionTest53((101).toByte());
    arktest.assertEQ(res, 1)
}

function testCasebaseFunctionTest56_1() {
    let res = nameInterfaceTest.BaseFunctionTest53(undefined);
    arktest.assertEQ(res, undefined)
}

function testCasebaseFunctionTest57() {
    let res = nameInterfaceTest.BaseFunctionTest54(2147483647);
    arktest.assertEQ(res, 2147483547)
}

function testCasebaseFunctionTest57_1() {
    let res = nameInterfaceTest.BaseFunctionTest54(undefined);
    arktest.assertEQ(res, undefined)
}

// function testCasebaseFunctionTest58(){
//     const testData1 = new namespaceTest.functiontest.Data(
//         "testCasebaseFunctionTest58",
//         true,
//         100
//     );
//     const testData2= new namespaceTest.functiontest.Data(
//         "testCasebaseFunctionTest58_1",
//         false,
//         200
//     );
//     const testData3= new namespaceTest.functiontest.Data(
//         "testCasebaseFunctionTest58_2",
//         false,
//         300
//     );
//     const myRecord: Record<short, namespaceTest.functiontest.Data> = {
//     100: testData1,
//     101: testData2,
//     102: testData3,
//     };
//     let res = nameInterfaceTest.BaseFunctionTest55(myRecord);
//     console.log("testCasebaseFunctionTest58 ",res)
//     arktest.assertEQ(res,2147483547)
// }

function testCasebaseFunctionTest59() {
    let res = nameInterfaceTest.BaseFunctionTest56(100);
    arktest.assertEQ(res, 10000)
}

function testCasebaseFunctionTest60() {
    let res = nameInterfaceTest.BaseFunctionTest57((127).toByte());
    arktest.assertEQ(res, 12700)
}

function testCasebaseFunctionTest61() {
    let res = nameInterfaceTest.testStr;
    arktest.assertEQ(res, "testNameSpace")
}

function testCasebaseFunctionTest62() {
    let res = nameInterfaceTest.BaseFunctionTest58(true);
    arktest.assertEQ(res.success, true)
    arktest.assertEQ(res.errorCode, 10000)
    arktest.assertEQ(res.errorMessage, "test58")
}

function testCasebaseFunctionTest62_1() {
    let res = nameInterfaceTest.BaseFunctionTest58(false);
    arktest.assertEQ(res.success, false)
    arktest.assertEQ(res.errorCode, 100)
    arktest.assertEQ(res.errorMessage, "test581")
}

function testCasebaseFunctionTest63() {
    let record: Record<string, string> = {
        "test1": "test123!@#$%",
        "test2": "test123^&*()",
        "test3": "test123_+{}[]",
        "test4": "test123;':<>?`",
    };
    namespaceTest.functiontest.BaseFunctionTest59(record);
}


function testCasebaseFunctionTest64() {
    let record: Record<long, boolean> = {
        9223372036854775807: false,
        10: true,
        20: false,
        30: true,
    };
    let res = namespaceTest.functiontest.BaseFunctionTest60(record);
    arktest.assertEQ(res, true)
}

// function testCasebaseFunctionTest64_1() {
//     let record: Record<long, boolean> = {
//         [-100]: false,
//         10: true,
//         20: false,
//         30: true,
//     };
//     let res = namespaceTest.functiontest.BaseFunctionTest60(record);
//     arktest.assertEQ(res, false)
// }


function testCasebaseFunctionTest64_2() {
    let record: Record<long, boolean> = {
        50: false,
        10: true,
        20: false,
        0: true,
    };
    let res = namespaceTest.functiontest.BaseFunctionTest60(record);
    arktest.assertEQ(res, false)
}


function testInter1BasePerformanceFunctionTest1() {
    testInterfacePerformance1.BasePerformanceFunctionTest1();
}

function testInter1BasePerformanceFunctionTest2() {
    let res =
        testInterfacePerformance1.BasePerformanceFunctionTest2((95).toByte());
    arktest.assertEQ(res, 10)
}

function testInter2BasePerformanceFunctionTest1() {
    testInterfacePerformance2.BasePerformanceFunctionTest1();
}

function testInter2BasePerformanceFunctionTest2() {
    let res =
        testInterfacePerformance2.BasePerformanceFunctionTest2((95).toByte());
    arktest.assertEQ(res, 10)
}

function testInter3BasePerformanceFunctionTest1() {
    testInterfacePerformance3.BasePerformanceFunctionTest1();
}

function testInter3BasePerformanceFunctionTest2() {
    let res =
        testInterfacePerformance3.BasePerformanceFunctionTest2((95).toByte());
    arktest.assertEQ(res, 10)
}

function testInter4BasePerformanceFunctionTest1() {
    testInterfacePerformance4.BasePerformanceFunctionTest1();
}

function testInter4BasePerformanceFunctionTest2() {
    let res =
        testInterfacePerformance4.BasePerformanceFunctionTest2((95).toByte());
    arktest.assertEQ(res, 10)
}
function testInter5BasePerformanceFunctionTest1() {
    testInterfacePerformance5.BasePerformanceFunctionTest1();
}

function testInter5BasePerformanceFunctionTest2() {
    let res =
        testInterfacePerformance5.BasePerformanceFunctionTest2((95).toByte());
    arktest.assertEQ(res, 10)
}

function testInter6BasePerformanceFunctionTest1() {
    testInterfacePerformance6.BasePerformanceFunctionTest1();
}

function testInter6BasePerformanceFunctionTest2() {
    let res =
        testInterfacePerformance6.BasePerformanceFunctionTest2((95).toByte());
    arktest.assertEQ(res, 10)
}
function testInter7BasePerformanceFunctionTest1() {
    testInterfacePerformance7.BasePerformanceFunctionTest1();
}

function testInter7BasePerformanceFunctionTest2() {
    let res =
        testInterfacePerformance7.BasePerformanceFunctionTest2((95).toByte());
    arktest.assertEQ(res, 10)
}

function testInter8BasePerformanceFunctionTest1() {
    testInterfacePerformance8.BasePerformanceFunctionTest1();
}

function testInter8BasePerformanceFunctionTest2() {
    let res =
        testInterfacePerformance8.BasePerformanceFunctionTest2((95).toByte());
    arktest.assertEQ(res, 10)
}
function testInter9BasePerformanceFunctionTest1() {
    testInterfacePerformance9.BasePerformanceFunctionTest1();
}

function testInter9BasePerformanceFunctionTest2() {
    let res =
        testInterfacePerformance9.BasePerformanceFunctionTest2((95).toByte());
    arktest.assertEQ(res, 10)
}
function testInter10BasePerformanceFunctionTest1() {
    testInterfacePerformance10.BasePerformanceFunctionTest1();
}

function testInter10BasePerformanceFunctionTest2() {
    let res =
        testInterfacePerformance10.BasePerformanceFunctionTest2((95).toByte());
    arktest.assertEQ(res, 10)
}
function testInter11BasePerformanceFunctionTest1() {
    testInterfacePerformance11.BasePerformanceFunctionTest1();
}

function testInter11BasePerformanceFunctionTest2() {
    let res =
        testInterfacePerformance11.BasePerformanceFunctionTest2((95).toByte());
    arktest.assertEQ(res, 10)
}
function testInter12BasePerformanceFunctionTest1() {
    testInterfacePerformance12.BasePerformanceFunctionTest1();
}

function testInter12BasePerformanceFunctionTest2() {
    let res =
        testInterfacePerformance12.BasePerformanceFunctionTest2((95).toByte());
    arktest.assertEQ(res, 10)
}
function testInter13BasePerformanceFunctionTest1() {
    testInterfacePerformance13.BasePerformanceFunctionTest1();
}

function testInter13BasePerformanceFunctionTest2() {
    let res =
        testInterfacePerformance13.BasePerformanceFunctionTest2((95).toByte());
    arktest.assertEQ(res, 10)
}

function testInter14BasePerformanceFunctionTest1() {
    testInterfacePerformance14.BasePerformanceFunctionTest1();
}

function testInter14BasePerformanceFunctionTest2() {
    let res =
        testInterfacePerformance14.BasePerformanceFunctionTest2((95).toByte());
    arktest.assertEQ(res, 10)
}

function testInter15BasePerformanceFunctionTest1() {
    testInterfacePerformance15.BasePerformanceFunctionTest1();
}

function testInter15BasePerformanceFunctionTest2() {
    let res =
        testInterfacePerformance15.BasePerformanceFunctionTest2((95).toByte());
    arktest.assertEQ(res, 10)
}

function testInter16BasePerformanceFunctionTest1() {
    testInterfacePerformance16.BasePerformanceFunctionTest1();
}

function testInter16BasePerformanceFunctionTest2() {
    let res =
        testInterfacePerformance16.BasePerformanceFunctionTest2((95).toByte());
    arktest.assertEQ(res, 10)
}
function testInter17BasePerformanceFunctionTest1() {
    testInterfacePerformance17.BasePerformanceFunctionTest1();
}

function testInter17BasePerformanceFunctionTest2() {
    let res =
        testInterfacePerformance17.BasePerformanceFunctionTest2((95).toByte());
    arktest.assertEQ(res, 10)
}
function testInter18BasePerformanceFunctionTest1() {
    testInterfacePerformance18.BasePerformanceFunctionTest1();
}

function testInter18BasePerformanceFunctionTest2() {
    let res =
        testInterfacePerformance18.BasePerformanceFunctionTest2((95).toByte());
    arktest.assertEQ(res, 10)
}

function testInter19BasePerformanceFunctionTest1() {
    testInterfacePerformance19.BasePerformanceFunctionTest1();
}

function testInter19BasePerformanceFunctionTest2() {
    let res =
        testInterfacePerformance19.BasePerformanceFunctionTest2((95).toByte());
    arktest.assertEQ(res, 10)
}

function testInter20BasePerformanceFunctionTest1() {
    testInterfacePerformance20.BasePerformanceFunctionTest1();
}

function testInter20BasePerformanceFunctionTest2() {
    let res =
        testInterfacePerformance20.BasePerformanceFunctionTest2((95).toByte());
    arktest.assertEQ(res, 10)
}
function testInter21BasePerformanceFunctionTest1() {
    let res = testInterfacePerformance21.i32_test;
    arktest.assertEQ(res, 2147483647)
}
function testInter21BasePerformanceFunctionTest2() {
    let res = testInterfacePerformance21.struct_test;
    arktest.assertEQ(res.data1, "testStruct")
    arktest.assertEQ(res.data2, false)
    arktest.assertEQ(res.data3, -2147483648)
}
function testInter21BasePerformanceFunctionTest3() {
    let res =
        testInterfacePerformance21.BasePerformanceFunctionTest2((10).toByte());
    arktest.assertEQ(res, (110).toByte())
}

function testInter21BasePerformanceFunctionTest4() {
    let res: byte = testInterfacePerformance21.i8_test_attribute;
    arktest.assertEQ(res, 127)
}

function testInter21BasePerformanceFunctionTest5() {
    let res: short = testInterfacePerformance21.i16_test_attribute;
    arktest.assertEQ(res, 32767)
}

function testInter21BasePerformanceFunctionTest6() {
    let res: int = testInterfacePerformance21.i32_test_attribute;
    arktest.assertEQ(res, 2147483647)
}

function testInter21BasePerformanceFunctionTest7() {
    let res: long = testInterfacePerformance21.i64_test_attribute;
    arktest.assertEQ(res, 9223372036854775807)
}

function testInter21BasePerformanceFunctionTest8() {
    let res: float = testInterfacePerformance21.f32_test_attribute;
    arktest.assertEQ(res, (3.14).toFloat())
}
function testInter21BasePerformanceFunctionTest9() {
    let res: double = testInterfacePerformance21.f64_test_attribute;
    arktest.assertEQ(res, -1.23)
}

function testInter21BasePerformanceFunctionTest10() {
    let res: string = testInterfacePerformance21.str_test_attribute;
    arktest.assertEQ(res, "TESTSTR")
}

function testInter21BasePerformanceFunctionTest11() {
    let res: boolean = testInterfacePerformance21.bool_test_attribute;
    arktest.assertEQ(res, false)
}

function testInter21BasePerformanceFunctionTest12() {
    let res: ArrayBuffer =
        testInterfacePerformance21._ArrayBuffer_test_attribute;
    arktest.assertEQ(res.getByteLength(), 10)
    for (let i = 0; i < res.getByteLength(); i++) {
        arktest.assertEQ(res.at(i), 10)
    }
}
function testInter21BasePerformanceFunctionTest13() {
    let res: int|undefined = testInterfacePerformance21.optional_test_attribute;
    arktest.assertEQ(res, 1010)
}

function testInter21BasePerformanceFunctionTest14() {
    let res: Record<int, namespaceTest.functiontest.Data> =
        testInterfacePerformance21.record_test_attribute;
    console.log("testInter21BasePerformanceFunctionTest14 ", res)
    const item = res[100];
    arktest.assertNE(item, undefined);
    arktest.assertEQ(item?.data1, "one");
    arktest.assertEQ(item?.data2, true);
    arktest.assertEQ(item?.data3, 100);
    const item1 = res[101];
    arktest.assertNE(item1, undefined);
    arktest.assertEQ(item1?.data1, "two");
    arktest.assertEQ(item1?.data2, false);
    arktest.assertEQ(item1?.data3, 101);
}

function testInter21BasePerformanceFunctionTest15() {
    let res: namespaceTest.functiontest.TestUnionName =
        testInterfacePerformance21.union_test_attribute;
    console.log("testInter21BasePerformanceFunctionTest15 ", res)
    arktest.assertEQ(res, 100)
}

function testInter21BasePerformanceFunctionTest16() {
    let res: boolean[] = testInterfacePerformance21.array_test_attribute;
    console.log("testInter21BasePerformanceFunctionTest16 ", res)
    arktest.assertEQ(res.length, 1)
    arktest.assertEQ(res[0], true)
}

function testInter21BasePerformanceFunctionTest17() {
    let color = namespaceTest.functiontest.Color.GREEN;
    let res: namespaceTest.functiontest.Color =
        testInterfacePerformance21.enum_test_attribute;
    arktest.assertEQ(res, color)
}

function testCaseIbaseInterface1() {
    let res = testIbaseInterface2.BaseTest();
    arktest.assertEQ(res, -2147483648)
}


function testCaseIbaseInterface2() {
    let res = testIbaseInterface2.BaseTest1();
    arktest.assertEQ(res, "TestInterface")
}
function testCaseIbaseInterface3() {
    let res = testIbaseInterface3.BaseTest();
    arktest.assertEQ(res, 2147483647)
}


function testCaseIbaseInterface4() {
    let res = testIbaseInterface3.BaseTest2();
    arktest.assertEQ(res, true)
}

function testCaseConstant1() {
    const res = namespaceTest.functiontest.CONSTANT_I8_001;
    arktest.assertEQ(res, 127)
}

function testCaseConstant2() {
    const res = namespaceTest.functiontest.CONSTANT_I8_002;
    arktest.assertEQ(res, -128)
}

function testCaseConstant3() {
    const res = namespaceTest.functiontest.CONSTANT_I32_001;
    arktest.assertEQ(res, 2147483647)
}
// namespace2
function primitives_noreturn() {
    namespace2Test.inner.Primitives_noreturn(
        (1).toInt(), (2).toDouble(), true, "hello", (3).toByte())
}
function primitives_return() {
    let res = namespace2Test.inner.Primitives_return(
        (1).toInt(), (2).toDouble(), true, "hello", (3).toByte())
    console.log(res);
}

function containers_noreturn() {
    let arri8: byte[] = [1, 2, 3];
    let arri16: short[] = [1, 2, 3, 4];
    let arri32: int[] = [1, 2, 3, 4, 5];
    let arrf32: float[] = [1.1f, 1.2f, 1.3f];
    let arrf64: double[] = [1.123, 2.123, 3.123];

    namespace2Test.inner.Containers_noreturn(
        arri8, arri16, arrf32, arrf64, "union_string")
    namespace2Test.inner.Containers_noreturn(
        arri8, arri16, arrf32, arrf64, (1).toByte())
}
function containers_return() {
    let arri8: byte[] = [1, 2, 3];
    let arri16: short[] = [1, 2, 3, 4];
    let arri32: int[] = [1, 2, 3, 4, 5];
    let arrf32: float[] = [1.1f, 1.2f, 1.3f];
    let arrf64: double[] = [1.123, 2.123, 3.123];

    let res1 = namespace2Test.inner.Containers_return(
        arri8, arri16, arrf32, arrf64, "union_string")
    let res2 = namespace2Test.inner.Containers_return(
        arri8, arri16, arrf32, arrf64, (1).toByte())
    console.log(res1 + " " + res2)
}
function enum_noreturn() {
    let color1 = namespace2Test.inner.Color.red;
    let color2 = namespace2Test.inner.Color.yellow;
    let color3 = namespace2Test.inner.Color.blue;
    let color4 = namespace2Test.inner.Color.green;
    let color5 = namespace2Test.inner.Color.black;

    namespace2Test.inner.Enum_noreturn(color1, color2, color3, color4, color5)
}
function enum_return() {
    let color1 = namespace2Test.inner.Color.red;
    let color2 = namespace2Test.inner.Color.yellow;
    let color3 = namespace2Test.inner.Color.blue;
    let color4 = namespace2Test.inner.Color.green;
    let color5 = namespace2Test.inner.Color.black;

    let res =
        namespace2Test.inner.Enum_return(color1, color2, color3, color4, color5)
    console.log(res)
}

let instance: namespace2Test.inner.TestInterface =
    namespace2Test.inner.get_interface();

function interface_noparam_noreturn() {
    instance.Noparam_noreturn()
}

function interface_primitives_noreturn() {
    instance.Primitives_noreturn((1).toByte())
}
function interface_primitives_noreturn1() {
    instance.Primitives_noreturn1((12).toShort())
}
function interface_primitives_noreturn2() {
    instance.Primitives_noreturn2((123).toInt())
}
function interface_primitives_noreturn3() {
    instance.Primitives_noreturn3((1.1).toFloat())
}
function interface_primitives_noreturn4() {
    instance.Primitives_noreturn4((1.1234).toDouble())
}
function interface_primitives_noreturn5() {
    instance.Primitives_noreturn5(true)
}
function interface_primitives_noreturn6() {
    instance.Primitives_noreturn6("hello")
}
function interface_primitives_noreturn7() {
    instance.Primitives_noreturn7(1234)
}

function interface_primitives_return() {
    instance.Primitives_return((123).toInt())
}
function interface_containers_noreturn1() {
    let arri8: byte[] = [1, 2, 3, 4, 5];
    instance.Containers_noreturn1(arri8)
}
function interface_containers_noreturn2() {
    instance.Containers_noreturn2("union string")
    instance.Containers_noreturn2((1).toByte())
}
function interface_containers_noreturn3() {
    let numbersU8: byte[] = [1, 2, 3, 4, 5];
    let arrbuf1: ArrayBuffer = new ArrayBuffer(numbersU8.length);
    instance.Containers_noreturn3(arrbuf1)
}
function interface_containers_noreturn4() {
    let color1 = namespace2Test.inner.Color.red;
    instance.Containers_noreturn4(color1)
}
function interface_containers_noreturn5() {
    let record: Record<string, int> = {
        "test1": 0,
        "test2": 1,
        "test3": 2,
        "test4": 3,
    };
    instance.Containers_noreturn5(record)
}



function interface_containers_return() {
    let res = instance.Containers_return((123).toByte())
    console.log(res)
}

function interface_ErrorResponse() {
    let _struct: namespace2Test.inner.ErrorResponse =
        instance.Func_ErrorResponse()
}

function overload1() {
    instance.OverloadFunc((1).toByte(), (2).toByte())
}
function overload2() {
    let arri8: byte[] = [1, 2, 3, 4, 5];
    let numbersU8: byte[] = [1, 2, 3, 4, 5];
    let arrbuf1: ArrayBuffer = new ArrayBuffer(numbersU8.length);
    let s = instance.OverloadFunc(arri8, arrbuf1)
    console.log(s)
}
function overload3() {
    instance.OverloadFunc()
}
function overload4() {
    let mystruct:
        namespace2Test.inner.Mystruct = {_string: "hello", _i8: (1).toByte()};
    instance.OverloadFunc(mystruct)
}

function testFuntest() {
    namespace2Test.ns1.ns2.ns3.ns4.ns5.Funtest({a: 0, b: 0.0});
}

function test20interface() {
    let instance_1: namespace2Test.inner.Test1 =
        namespace2Test.inner.get_interface_1();
    let instance_20: namespace2Test.inner.Test20 =
        namespace2Test.inner.get_interface_20();
    instance_1.Fun1()
    instance_20.Fun2()
}

// attribute
function testattributeString() {
    let name = namespace2Test.inner.PrintTestInterfaceName(instance)
    console.log("instance.name is " + name)
    arktest.assertEQ(name, "String")
}
function testattributei8() {
    let i8 = namespace2Test.inner.PrintTestInterfaceNumberi8(instance)
    console.log("instance.i8 is " + i8)
    arktest.assertEQ(i8, -128)
}
function testattributei16() {
    let i16 = namespace2Test.inner.PrintTestInterfaceNumberi16(instance)
    console.log("instance.i16 is " + i16)
    arktest.assertEQ(i16, -32768)
}
function testattributei32() {
    let i32 = namespace2Test.inner.PrintTestInterfaceNumberi32(instance)
    console.log("instance.i32 is " + i32)
    arktest.assertEQ(i32, -2147483648)
}
function testattributei64() {
    let i64 = namespace2Test.inner.PrintTestInterfaceNumberi64(instance)
    console.log("instance.i64 is " + i64)
    arktest.assertEQ(i64, (1000).toLong())
}
function testattributef32() {
    let f32 = namespace2Test.inner.PrintTestInterfaceNumberf32(instance)
    console.log("instance.f32 is " + f32)
    arktest.assertEQ(f32, (3.1415927).toFloat())
}
function testattributef64() {
    let f64 = namespace2Test.inner.PrintTestInterfaceNumberf64(instance)
    console.log("instance.f64 is " + f64)
    arktest.assertEQ(f64, 3.141592653589793)
}
function testattributebool() {
    let b = namespace2Test.inner.PrintTestInterfacebool(instance)
    arktest.assertEQ(b, true)
}
function testattributeArrayBuffer() {
    namespace2Test.inner.PrintTestInterfaceArraybuffer(instance)
}
function testattributeArray() {
    namespace2Test.inner.PrintTestInterfaceArray(instance)
}
function testattributeUnion() {
    namespace2Test.inner.PrintTestInterfaceUnion(instance)
}
function testattributeRecord() {
    namespace2Test.inner.PrintTestInterfaceRecord(instance)
}
function testattributeEnum() {
    namespace2Test.inner.PrintTestInterfaceEnum(instance)
}

function testinheritance() {
    let instancetestA: namespace2Test.inner.TestA =
        namespace2Test.inner.get_interface_A()
    instancetestA.Fun1()
    let instancetestB: namespace2Test.inner.TestB =
        namespace2Test.inner.get_interface_B()
    instancetestB.Fun2()
    let instancetestC: namespace2Test.inner.TestC =
        namespace2Test.inner.get_interface_C()
    instancetestC.Fun3()
}

function main() {
    console.log("run main ... ani_namespace_test begin ...")
    const suite = new arktest.ArkTestsuite("test function namespace_test")
    // namespace2
    suite.addTest("primitives_noreturn", primitives_noreturn)
    suite.addTest("primitives_return", primitives_return)
    suite.addTest("containers_noreturn", containers_noreturn)
    suite.addTest("containers_return", containers_return)
    suite.addTest("enum_noreturn", enum_noreturn)
    suite.addTest("enum_return", enum_return)

    suite.addTest("interface_primitives_return", interface_primitives_return)
    suite.addTest(
        "interface_containers_noreturn1", interface_containers_noreturn1)
    suite.addTest(
        "interface_containers_noreturn2", interface_containers_noreturn2)
    suite.addTest(
        "interface_containers_noreturn3", interface_containers_noreturn3)
    suite.addTest(
        "interface_containers_noreturn3", interface_containers_noreturn4)
    suite.addTest(
        "interface_containers_noreturn3", interface_containers_noreturn5)
    suite.addTest("interface_containers_return", interface_containers_return)

    suite.addTest("interface_ErrorResponse", interface_ErrorResponse)

    suite.addTest("overload1", overload1)
    suite.addTest("overload2", overload2)
    suite.addTest("overload3", overload3)
    suite.addTest("overload4", overload4)

    suite.addTest("interface_noparam_noreturn", interface_noparam_noreturn)
    suite.addTest(
        "interface_primitives_noreturn", interface_primitives_noreturn)
    suite.addTest(
        "interface_primitives_noreturn1", interface_primitives_noreturn1)
    suite.addTest(
        "interface_primitives_noreturn2", interface_primitives_noreturn2)
    suite.addTest(
        "interface_primitives_noreturn3", interface_primitives_noreturn3)
    suite.addTest(
        "interface_primitives_noreturn4", interface_primitives_noreturn4)
    suite.addTest(
        "interface_primitives_noreturn5", interface_primitives_noreturn5)
    suite.addTest(
        "interface_primitives_noreturn6", interface_primitives_noreturn6)
    suite.addTest(
        "interface_primitives_noreturn7", interface_primitives_noreturn7)

    suite.addTest("test20interface", test20interface);
    suite.addTest("testFuntest", testFuntest);

    suite.addTest("testattributeString", testattributeString);
    suite.addTest("testattributei8", testattributei8);
    suite.addTest("testattributei16", testattributei16);
    suite.addTest("testattributei32", testattributei32);
    suite.addTest("testattributei64", testattributei64);
    suite.addTest("testattributef32", testattributef32);
    suite.addTest("testattributef64", testattributef64);
    suite.addTest("testattributebool", testattributebool);
    suite.addTest("testattributeArrayBuffer", testattributeArrayBuffer);
    suite.addTest("testattributeArray", testattributeArray);
    suite.addTest("testattributeUnion", testattributeUnion);
    suite.addTest("testattributeRecord", testattributeRecord);
    suite.addTest("testattributeEnum", testattributeEnum);
    suite.addTest("testinheritance", testinheritance);

    // namespace1
    suite.addTest("check testCasebaseFunctionTest1", testCasebaseFunctionTest1);
    suite.addTest("check testCasebaseFunctionTest2", testCasebaseFunctionTest2);
    suite.addTest("check testCasebaseFunctionTest3", testCasebaseFunctionTest3);
    suite.addTest("check testCasebaseFunctionTest4", testCasebaseFunctionTest4);
    suite.addTest("check testCasebaseFunctionTest5", testCasebaseFunctionTest5);
    suite.addTest("check testCasebaseFunctionTest6", testCasebaseFunctionTest6);
    suite.addTest("check testCasebaseFunctionTest7", testCasebaseFunctionTest7);
    suite.addTest("check testCasebaseFunctionTest8", testCasebaseFunctionTest8);
    suite.addTest("check testCasebaseFunctionTest9", testCasebaseFunctionTest9);
    suite.addTest(
        "check testCasebaseFunctionTest10", testCasebaseFunctionTest10);
    suite.addTest(
        "check testCasebaseFunctionTest11", testCasebaseFunctionTest11);
    suite.addTest(
        "check testCasebaseFunctionTest12", testCasebaseFunctionTest12);
    suite.addTest(
        "check testCasebaseFunctionTest12_1", testCasebaseFunctionTest12_1);
    suite.addTest(
        "check testCasebaseFunctionTest13", testCasebaseFunctionTest13);
    suite.addTest(
        "check testCasebaseFunctionTest13_1", testCasebaseFunctionTest13_1);
    suite.addTest(
        "check testCasebaseFunctionTest14", testCasebaseFunctionTest14);
    suite.addTest(
        "check testCasebaseFunctionTest15", testCasebaseFunctionTest15);
    suite.addTest(
        "check testCasebaseFunctionTest16", testCasebaseFunctionTest16);
    suite.addTest(
        "check testCasebaseFunctionTest16_1", testCasebaseFunctionTest16_1);
    suite.addTest(
        "check testCasebaseFunctionTest17", testCasebaseFunctionTest17);
    suite.addTest(
        "check testCasebaseFunctionTest17_1", testCasebaseFunctionTest17_1);
    suite.addTest(
        "check testCasebaseFunctionTest18", testCasebaseFunctionTest18);
    suite.addTest(
        "check testCasebaseFunctionTest18_1", testCasebaseFunctionTest18_1);
    suite.addTest(
        "check testCasebaseFunctionTest19", testCasebaseFunctionTest19);
    suite.addTest(
        "check testCasebaseFunctionTest20", testCasebaseFunctionTest20);
    suite.addTest(
        "check testCasebaseFunctionTest21", testCasebaseFunctionTest21);
    suite.addTest(
        "check testCasebaseFunctionTest22", testCasebaseFunctionTest22);
    suite.addTest(
        "check testCasebaseFunctionTest22_1", testCasebaseFunctionTest22_1);
    suite.addTest(
        "check testCasebaseFunctionTest23", testCasebaseFunctionTest23);
    suite.addTest(
        "check testCasebaseFunctionTest23_1", testCasebaseFunctionTest23_1);
    suite.addTest(
        "check testCasebaseFunctionTest24", testCasebaseFunctionTest24);
    suite.addTest(
        "check testCasebaseFunctionTest25", testCasebaseFunctionTest25);
    suite.addTest(
        "check testCasebaseFunctionTest26", testCasebaseFunctionTest26);
    suite.addTest(
        "check testCasebaseFunctionTest26_1", testCasebaseFunctionTest26_1);
    suite.addTest(
        "check testCasebaseFunctionTest27", testCasebaseFunctionTest27);
    suite.addTest(
        "check testCasebaseFunctionTest27_1", testCasebaseFunctionTest27_1);
    suite.addTest(
        "check testCasebaseFunctionTest28", testCasebaseFunctionTest28);
    suite.addTest(
        "check testCasebaseFunctionTest29", testCasebaseFunctionTest29);
    suite.addTest(
        "check testCasebaseFunctionTest30", testCasebaseFunctionTest30);
    suite.addTest(
        "check testCasebaseFunctionTest31", testCasebaseFunctionTest31);
    suite.addTest(
        "check testCasebaseFunctionTest32", testCasebaseFunctionTest32);
    suite.addTest(
        "check testCasebaseFunctionTest32_1", testCasebaseFunctionTest32_1);
    suite.addTest(
        "check testCasebaseFunctionTest33", testCasebaseFunctionTest33);
    suite.addTest(
        "check testCasebaseFunctionTest33_1", testCasebaseFunctionTest33_1);
    suite.addTest(
        "check testCasebaseFunctionTest34", testCasebaseFunctionTest34);
    suite.addTest(
        "check testCasebaseFunctionTest34_1", testCasebaseFunctionTest34_1);
    suite.addTest(
        "check testCasebaseFunctionTest35", testCasebaseFunctionTest35);
    suite.addTest(
        "check testCasebaseFunctionTest35_1", testCasebaseFunctionTest35_1);
    suite.addTest(
        "check testCasebaseFunctionTest36", testCasebaseFunctionTest36);
    suite.addTest(
        "check testCasebaseFunctionTest36_1", testCasebaseFunctionTest36_1);
    suite.addTest(
        "check testCasebaseFunctionTest37", testCasebaseFunctionTest37);
    suite.addTest(
        "check testCasebaseFunctionTest37_1", testCasebaseFunctionTest37_1);
    suite.addTest(
        "check testCasebaseFunctionTest38", testCasebaseFunctionTest38);
    suite.addTest(
        "check testCasebaseFunctionTest38_1", testCasebaseFunctionTest38_1);
    suite.addTest(
        "check testCasebaseFunctionTest39", testCasebaseFunctionTest39);
    suite.addTest(
        "check testCasebaseFunctionTest39_1", testCasebaseFunctionTest39_1);
    suite.addTest(
        "check testCasebaseFunctionTest40", testCasebaseFunctionTest40);
    suite.addTest(
        "check testCasebaseFunctionTest40_1", testCasebaseFunctionTest40_1);
    suite.addTest(
        "check testCasebaseFunctionTest41", testCasebaseFunctionTest41);
    suite.addTest(
        "check testCasebaseFunctionTest42", testCasebaseFunctionTest42);
    suite.addTest(
        "check testCasebaseFunctionTest42_1", testCasebaseFunctionTest42_1);
    suite.addTest(
        "check testCasebaseFunctionTest43", testCasebaseFunctionTest43);
    suite.addTest(
        "check testCasebaseFunctionTest43_1", testCasebaseFunctionTest43_1);
    suite.addTest(
        "check testCasebaseFunctionTest44", testCasebaseFunctionTest44);
    suite.addTest(
        "check testCasebaseFunctionTest45", testCasebaseFunctionTest45);
    suite.addTest(
        "check testCasebaseFunctionTest46", testCasebaseFunctionTest46);
    suite.addTest(
        "check testCasebaseFunctionTest47", testCasebaseFunctionTest47);
    suite.addTest(
        "check testCasebaseFunctionTest48", testCasebaseFunctionTest48);
    suite.addTest(
        "check testCasebaseFunctionTest49", testCasebaseFunctionTest49);
    suite.addTest(
        "check testCasebaseFunctionTest50", testCasebaseFunctionTest50);
    suite.addTest(
        "check testCasebaseFunctionTest51", testCasebaseFunctionTest51);
    suite.addTest(
        "check testCasebaseFunctionTest52", testCasebaseFunctionTest52);
    suite.addTest(
        "check testCasebaseFunctionTest53", testCasebaseFunctionTest53);
    suite.addTest(
        "check testCasebaseFunctionTest54", testCasebaseFunctionTest54);
    suite.addTest(
        "check testCasebaseFunctionTest54_1", testCasebaseFunctionTest54_1);
    suite.addTest(
        "check testCasebaseFunctionTest55", testCasebaseFunctionTest55);
    suite.addTest(
        "check testCasebaseFunctionTest56", testCasebaseFunctionTest56);
    suite.addTest(
        "check testCasebaseFunctionTest56_1", testCasebaseFunctionTest56_1);
    suite.addTest(
        "check testCasebaseFunctionTest57", testCasebaseFunctionTest57);
    suite.addTest(
        "check testCasebaseFunctionTest57_1", testCasebaseFunctionTest57_1);
    // suite.addTest("check testCasebaseFunctionTest58",
    // testCasebaseFunctionTest58);
    suite.addTest(
        "check testCasebaseFunctionTest59", testCasebaseFunctionTest59);
    suite.addTest(
        "check testCasebaseFunctionTest60", testCasebaseFunctionTest60);
    suite.addTest(
        "check testCasebaseFunctionTest61", testCasebaseFunctionTest61);
    suite.addTest(
        "check testCasebaseFunctionTest62", testCasebaseFunctionTest62);
    suite.addTest(
        "check testCasebaseFunctionTest62_1", testCasebaseFunctionTest62_1);
    suite.addTest(
        "check testCasebaseFunctionTest63", testCasebaseFunctionTest63);
    suite.addTest(
        "check testCasebaseFunctionTest64", testCasebaseFunctionTest64);
    // suite.addTest(
    //     "check testCasebaseFunctionTest64_1", testCasebaseFunctionTest64_1);
    suite.addTest(
        "check testCasebaseFunctionTest64_2", testCasebaseFunctionTest64_2);
    // performance
    suite.addTest(
        "check testInter1BasePerformanceFunctionTest1",
        testInter1BasePerformanceFunctionTest1);
    suite.addTest(
        "check testInter1BasePerformanceFunctionTest2",
        testInter1BasePerformanceFunctionTest2);
    suite.addTest(
        "check testInter2BasePerformanceFunctionTest1",
        testInter2BasePerformanceFunctionTest1);
    suite.addTest(
        "check testInter2BasePerformanceFunctionTest2",
        testInter2BasePerformanceFunctionTest2);
    suite.addTest(
        "check testInter3BasePerformanceFunctionTest1",
        testInter3BasePerformanceFunctionTest1);
    suite.addTest(
        "check testInter3BasePerformanceFunctionTest2",
        testInter3BasePerformanceFunctionTest2);
    suite.addTest(
        "check testInter4BasePerformanceFunctionTest1",
        testInter4BasePerformanceFunctionTest1);
    suite.addTest(
        "check testInter4BasePerformanceFunctionTest2",
        testInter4BasePerformanceFunctionTest2);
    suite.addTest(
        "check testInter5BasePerformanceFunctionTest1",
        testInter5BasePerformanceFunctionTest1);
    suite.addTest(
        "check testInter5BasePerformanceFunctionTest2",
        testInter5BasePerformanceFunctionTest2);
    suite.addTest(
        "check testInter6BasePerformanceFunctionTest1",
        testInter6BasePerformanceFunctionTest1);
    suite.addTest(
        "check testInter6BasePerformanceFunctionTest2",
        testInter6BasePerformanceFunctionTest2);
    suite.addTest(
        "check testInter7BasePerformanceFunctionTest1",
        testInter7BasePerformanceFunctionTest1);
    suite.addTest(
        "check testInter7BasePerformanceFunctionTest2",
        testInter7BasePerformanceFunctionTest2);
    suite.addTest(
        "check testInter8BasePerformanceFunctionTest1",
        testInter8BasePerformanceFunctionTest1);
    suite.addTest(
        "check testInter8BasePerformanceFunctionTest2",
        testInter8BasePerformanceFunctionTest2);
    suite.addTest(
        "check testInter9BasePerformanceFunctionTest1",
        testInter9BasePerformanceFunctionTest1);
    suite.addTest(
        "check testInter9BasePerformanceFunctionTest2",
        testInter9BasePerformanceFunctionTest2);
    suite.addTest(
        "check testInter10BasePerformanceFunctionTest1",
        testInter10BasePerformanceFunctionTest1);
    suite.addTest(
        "check testInter10BasePerformanceFunctionTest2",
        testInter10BasePerformanceFunctionTest2);
    suite.addTest(
        "check testInter11BasePerformanceFunctionTest1",
        testInter11BasePerformanceFunctionTest1);
    suite.addTest(
        "check testInter11BasePerformanceFunctionTest2",
        testInter11BasePerformanceFunctionTest2);
    suite.addTest(
        "check testInter12BasePerformanceFunctionTest1",
        testInter12BasePerformanceFunctionTest1);
    suite.addTest(
        "check testInter12BasePerformanceFunctionTest2",
        testInter12BasePerformanceFunctionTest2);
    suite.addTest(
        "check testInter13BasePerformanceFunctionTest1",
        testInter13BasePerformanceFunctionTest1);
    suite.addTest(
        "check testInter13BasePerformanceFunctionTest2",
        testInter13BasePerformanceFunctionTest2);
    suite.addTest(
        "check testInter14BasePerformanceFunctionTest1",
        testInter14BasePerformanceFunctionTest1);
    suite.addTest(
        "check testInter14BasePerformanceFunctionTest2",
        testInter14BasePerformanceFunctionTest2);
    suite.addTest(
        "check testInter15BasePerformanceFunctionTest1",
        testInter15BasePerformanceFunctionTest1);
    suite.addTest(
        "check testInter15BasePerformanceFunctionTest2",
        testInter15BasePerformanceFunctionTest2);
    suite.addTest(
        "check testInter16BasePerformanceFunctionTest1",
        testInter16BasePerformanceFunctionTest1);
    suite.addTest(
        "check testInter16BasePerformanceFunctionTest2",
        testInter16BasePerformanceFunctionTest2);
    suite.addTest(
        "check testInter17BasePerformanceFunctionTest1",
        testInter17BasePerformanceFunctionTest1);
    suite.addTest(
        "check testInter17BasePerformanceFunctionTest2",
        testInter17BasePerformanceFunctionTest2);
    suite.addTest(
        "check testInter18BasePerformanceFunctionTest1",
        testInter18BasePerformanceFunctionTest1);
    suite.addTest(
        "check testInter18BasePerformanceFunctionTest2",
        testInter18BasePerformanceFunctionTest2);
    suite.addTest(
        "check testInter19BasePerformanceFunctionTest1",
        testInter19BasePerformanceFunctionTest1);
    suite.addTest(
        "check testInter19BasePerformanceFunctionTest2",
        testInter19BasePerformanceFunctionTest2);
    suite.addTest(
        "check testInter20BasePerformanceFunctionTest1",
        testInter20BasePerformanceFunctionTest1);
    suite.addTest(
        "check testInter20BasePerformanceFunctionTest2",
        testInter20BasePerformanceFunctionTest2);
    suite.addTest(
        "check testInter21BasePerformanceFunctionTest1",
        testInter21BasePerformanceFunctionTest1);
    suite.addTest(
        "check testInter21BasePerformanceFunctionTest2",
        testInter21BasePerformanceFunctionTest2);
    suite.addTest(
        "check testInter21BasePerformanceFunctionTest3",
        testInter21BasePerformanceFunctionTest3);
    suite.addTest(
        "check testInter21BasePerformanceFunctionTest4",
        testInter21BasePerformanceFunctionTest4);
    suite.addTest(
        "check testInter21BasePerformanceFunctionTest5",
        testInter21BasePerformanceFunctionTest5);

    suite.addTest(
        "check testInter21BasePerformanceFunctionTest6",
        testInter21BasePerformanceFunctionTest6);
    suite.addTest(
        "check testInter21BasePerformanceFunctionTest7",
        testInter21BasePerformanceFunctionTest7);
    suite.addTest(
        "check testInter21BasePerformanceFunctionTest8",
        testInter21BasePerformanceFunctionTest8);
    suite.addTest(
        "check testInter21BasePerformanceFunctionTest9",
        testInter21BasePerformanceFunctionTest9);

    suite.addTest(
        "check testInter21BasePerformanceFunctionTest10",
        testInter21BasePerformanceFunctionTest10);

    suite.addTest(
        "check testInter21BasePerformanceFunctionTest11",
        testInter21BasePerformanceFunctionTest11);
    suite.addTest(
        "check testInter21BasePerformanceFunctionTest12",
        testInter21BasePerformanceFunctionTest12);
    suite.addTest(
        "check testInter21BasePerformanceFunctionTest13",
        testInter21BasePerformanceFunctionTest13);

    suite.addTest(
        "check testInter21BasePerformanceFunctionTest14",
        testInter21BasePerformanceFunctionTest14);
    suite.addTest(
        "check testInter21BasePerformanceFunctionTest15",
        testInter21BasePerformanceFunctionTest15);
    suite.addTest(
        "check testInter21BasePerformanceFunctionTest16",
        testInter21BasePerformanceFunctionTest16);
    suite.addTest(
        "check testInter21BasePerformanceFunctionTest17",
        testInter21BasePerformanceFunctionTest17);
    suite.addTest("check testCaseIbaseInterface1", testCaseIbaseInterface1);
    suite.addTest("check testCaseIbaseInterface2", testCaseIbaseInterface2);
    suite.addTest("check testCaseIbaseInterface3", testCaseIbaseInterface3);
    suite.addTest("check testCaseIbaseInterface4", testCaseIbaseInterface4);
    suite.addTest("check testCaseConstant1", testCaseConstant1);
    suite.addTest("check testCaseConstant2", testCaseConstant2);
    suite.addTest("check testCaseConstant3", testCaseConstant3);
    exit(suite.run());
}
