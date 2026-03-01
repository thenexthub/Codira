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
import * as record_test from "record_test";
import {Color, Data, ICpu, ICpuInfo, ICpuZero, Pair} from "record_test";

loadLibrary("ani_record");

class A {
    constructor() {}
}

function getCpuSize() {
    // test function getCpuSize(r: Record<string, ICpu>): int
    let cpu0: ICpu = record_test.MakeCpu();
    let cpu1: ICpu = record_test.MakeCpu();
    let cpus: Record<string, ICpu> = {
        "cpu0": cpu0,
        "cpu1": cpu1,
    };
    let ret0: int = record_test.GetCpuSize(cpus);
    console.log(ret0);
    arktest.assertEQ(ret0, 2);
}

function getASize() {
    // test function getASize(r: Record<int, Object>): int
    let a = new A();
    let b = new A();
    let c = new A();
    let As: Record<int, Object> = {
        0: a,
        1: b,
        2: c,
    };
    let ret1: int = record_test.GetASize(As);
    console.log(ret1);
    arktest.assertEQ(ret1, 3);
}

function getStringIntSize() {
    // test function getStringIntSize(r: Record<string, int>): int
    let record: Record<string, int> = {
        "aaa": 0,
        "bbb": 1,
        "ccc": 2,
        "ddd": 3,
    };
    let ret2: int = record_test.GetStringIntSize(record);
    console.log(ret2);
    arktest.assertEQ(ret2, 4);
}

function createStringString() {
    // test function createStringString(a: int): Record<string, string>
    let strRecord: Record<string, string> = record_test.CreateStringString(5);
    console.log(`Key: 0, Value: ` + strRecord["0"]);
    arktest.assertEQ(strRecord["0"], "abc");
}

function getMapfromArray() {
    // test function getMapfromArray(d: (Data[])): Record<string, int>
    let d0: Data = {a: "a", b: 1};
    let d1: Data = {a: "b", b: 2};
    let d2: Data = {a: "c", b: 3};
    let data: Data[] = [d0, d1, d2];
    let dataRecord: Record<string, int> = record_test.GetMapfromArray(data);
    console.log(`Key: b, Value: ` + dataRecord["b"]);
    arktest.assertEQ(dataRecord["b"], 2);
}

function getDatafromMap() {
    // test function getDatafromMap(m: Record<string, Data>, k: string): Data
    let data1: Data = {a: "aaa", b: 1};
    let data2: Data = {a: "bbb", b: 2};
    let data3: Data = {a: "ccc", b: 3};
    let drecord: Record<string, Data> = {
        "aa": data1,
        "bb": data2,
        "cc": data3,
    };
    let d: Data = record_test.GetDatafromMap(drecord, "bb");
    console.log(`Data Value: ` + d.a);
    arktest.assertEQ(d.a, "bbb");
}

function foreachMap() {
    let recordStrForeach: Record<string, string> = {
        "key1": "v1",
        "key2": "v2",
        "key3": "v3",
        "key4": "v4",
    };
    record_test.ForeachMap(recordStrForeach);
}

function main() {
    console.log("##############record ut start#############")
    const suite = new arktest.ArkTestsuite("record ut")
    suite.addTest("get Cpu Size test", getCpuSize)
    suite.addTest("get A Size test", getASize)
    suite.addTest("get String Int Size test", getStringIntSize)
    suite.addTest("create String String test", createStringString)
    suite.addTest("get Map from Array test", getMapfromArray)
    suite.addTest("get Data from Map test", getDatafromMap)
    suite.addTest("foreach Map test", foreachMap)
    suite.addTest("testMapfunc01", testMapfunc01)
    suite.addTest("testMapfunc02", testMapfunc02)
    suite.addTest("testMapfunc03", testMapfunc03)
    suite.addTest("testMapfunc04", testMapfunc04)
    suite.addTest("testMapfunc05", testMapfunc05)
    suite.addTest("testMapfunc06", testMapfunc06)
    suite.addTest("testMapfunc07", testMapfunc07)
    suite.addTest("testMapfunc08", testMapfunc08)
    suite.addTest("testMapfunc09", testMapfunc09)
    suite.addTest("testMapfunc10", testMapfunc10)
    suite.addTest("testMapfunc11", testMapfunc11)
    suite.addTest("testMapfunc12", testMapfunc12)
    suite.addTest("testMapfunc13", testMapfunc13)
    suite.addTest("testMapfunc14", testMapfunc14)
    suite.addTest("testMapfunc15", testMapfunc15)
    suite.addTest("testMapfunc16", testMapfunc16)
    suite.addTest("testMapfunc17", testMapfunc17)
    suite.addTest("testMapfunc18", testMapfunc18)
    suite.addTest("testMapfunc19", testMapfunc19)
    suite.addTest("testMapfunc20", testMapfunc20)
    suite.addTest("testMapfunc21", testMapfunc21)
    suite.addTest("testMapfunc22", testMapfunc22)
    suite.addTest("testMapfunc23", testMapfunc23)
    suite.addTest("testMapfunc24", testMapfunc24)
    suite.addTest("testMapfunc25", testMapfunc25)
    suite.addTest("testMapfunc26", testMapfunc26)
    suite.addTest("testMapfunc27", testMapfunc27)
    suite.addTest("testMapfunc28", testMapfunc28)
    suite.addTest("testMapfunc29", testMapfunc29)
    suite.addTest("testMapfunc30", testMapfunc30)
    suite.addTest("testMapfunc31", testMapfunc31)
    suite.addTest("testMapfunc32", testMapfunc32)
    suite.addTest("testMapfunc33", testMapfunc33)
    suite.addTest("testMapfunc34", testMapfunc34)
    suite.addTest("testMapfunc35", testMapfunc35)
    suite.addTest("testMapfunc36", testMapfunc36)
    suite.addTest("testMapfunc37", testMapfunc37)
    suite.addTest("testMapfunc38", testMapfunc38)
    suite.addTest("testMapfunc39", testMapfunc39)
    suite.addTest("testMapfunc40", testMapfunc40)
    suite.addTest("testMapfunc41", testMapfunc41)
    suite.addTest("testMapfunc42", testMapfunc42)
    suite.addTest("testMapfunc43", testMapfunc43)
    suite.addTest("testMapfunc44", testMapfunc44)
    suite.addTest("testMapfunc45", testMapfunc45)
    suite.addTest("testMapfunc46", testMapfunc46)
    suite.addTest("testMapfunc47", testMapfunc47)
    suite.addTest("testMapfunc48", testMapfunc48)
    suite.addTest("testMapfunc49", testMapfunc49)
    suite.addTest("testMapfunc50", testMapfunc50)
    suite.addTest("testMapfunc51", testMapfunc51)
    exit(suite.run())
    console.log("##############record ut end#############")
}


// function mapfunc01(m: Record<string, boolean>): boolean
function testMapfunc01() {
    let testData: Record<string, boolean> = {"key1": true, "key2": false};
    let result: boolean = record_test.Mapfunc01(testData);
    arktest.assertEQ(result, true);
}

// function mapfunc02(m: Record<string, byte>): boolean
function testMapfunc02() {
    let testData:
        Record<string, byte> = {"key1": (123).toByte(), "key2": (45).toByte()};
    let result: boolean = record_test.Mapfunc02(testData);
    arktest.assertEQ(result, true);
}

// function mapfunc03(m: Record<string, short>): boolean
function testMapfunc03() {
    let testData: Record<string, short> = {
        "key1": (1234).toShort(),
        "key2": (5678).toShort()
    };
    let result: boolean = record_test.Mapfunc03(testData);
    arktest.assertEQ(result, true);
}

// function mapfunc04(m: Record<string, int>): boolean
function testMapfunc04() {
    let testData: Record<string, int> = {
        "key1": (12345).toInt(),
        "key2": (67890).toInt()
    };
    let result: boolean = record_test.Mapfunc04(testData);
    arktest.assertEQ(result, true);
}

// function mapfunc05(m: Record<string, long>): boolean
function testMapfunc05() {
    let testData: Record<string, long> = {
        "key1": (123456).toLong(),
        "key2": (789012).toLong()
    };
    let result: boolean = record_test.Mapfunc05(testData);
    arktest.assertEQ(result, true);
}

// function mapfunc06(m: Record<string, float>): boolean
function testMapfunc06() {
    let testData: Record<string, float> = {"key1": 123.45f, "key2": 67.89f};
    let result: boolean = record_test.Mapfunc06(testData);
    arktest.assertEQ(result, true);
}

// function mapfunc07(m: Record<string, double>): boolean
function testMapfunc07() {
    let testData: Record<string, double> = {
        "key1": (123.456).toDouble(),
        "key2": (789.012).toDouble()
    };
    let result: boolean = record_test.Mapfunc07(testData);
    arktest.assertEQ(result, true);
}

// function mapfunc08(m: Record<string, string>): boolean
function testMapfunc08() {
    let testData: Record<string, string> = {"key1": "value1", "key2": "value2"};
    let result: boolean = record_test.Mapfunc08(testData);
    arktest.assertEQ(result, true);
}

// function mapfunc09(m: Record<string, (byte[])>): boolean
function testMapfunc09() {
    let testData: Record<string, (byte[])> = {
        "key1": [(1).toByte(), (2).toByte(), (3).toByte()],
        "key2": [(4).toByte(), (5).toByte(), (6).toByte()]
    };
    let result: boolean = record_test.Mapfunc09(testData);
    arktest.assertEQ(result, true);
}

// function mapfunc10(m: Record<string, (short[])>): boolean
function testMapfunc10() {
    let testData: Record<string, (short[])> = {
        "key1": [(123).toShort(), (456).toShort()],
        "key2": [(789).toShort(), (1011).toShort()]
    };
    let result: boolean = record_test.Mapfunc10(testData);
    arktest.assertEQ(result, true);
}

// function mapfunc11(m: Record<string, (int[])>): boolean
function testMapfunc11() {
    let testData: Record<string, (int[])> = {
        "key1": [(1234).toInt(), (5678).toInt()],
        "key2": [(9012).toInt(), (3456).toInt()]
    };
    let result: boolean = record_test.Mapfunc11(testData);
    arktest.assertEQ(result, true);
}

// function mapfunc12(m: Record<string, (long[])>): boolean
function testMapfunc12() {
    let testData: Record<string, (long[])> = {
        "key1": [(12345).toLong(), (67890).toLong()],
        "key2": [(11111).toLong(), (22222).toLong()]
    };
    let result: boolean = record_test.Mapfunc12(testData);
    arktest.assertEQ(result, true);
}

// function mapfunc13(m: Record<string, (ArrayBuffer[])>): boolean
function testMapfunc13() {
    let testData: Record<string, (ArrayBuffer[])> = {
        "key1": [new ArrayBuffer(8), new ArrayBuffer(8)],
        "key2": [new ArrayBuffer(8), new ArrayBuffer(8)]
    };
    let result: boolean = record_test.Mapfunc13(testData);
    arktest.assertEQ(result, true);
}

// function mapfunc14(m: Record<string, (boolean[])>): boolean
function testMapfunc14() {
    let testData: Record<string, (boolean[])> = {
        "key1": [true, false],
        "key2": [false, true]
    };
    let result: boolean = record_test.Mapfunc14(testData);
    arktest.assertEQ(result, true);
}

// function mapfunc15(m: Record<string, (string[])>): boolean
function testMapfunc15() {
    let testData: Record<string, (string[])> = {
        "key1": ["value1", "value2"],
        "key2": ["value3", "value4"]
    };
    let result: boolean = record_test.Mapfunc15(testData);
    arktest.assertEQ(result, true);
}

// function mapfunc16(m: Record<string, (int | boolean | string)>): boolean
function testMapfunc16() {
    let testData: Record<string, (int|boolean|string)> = {
        "key1": 123,
        "key2": true,
        "key3": "value"
    };
    let result: boolean = record_test.Mapfunc16(testData);
    arktest.assertEQ(result, true);
}

// function mapfunc17(m: Record<string, Color>): boolean
function testMapfunc17() {
    let testData:
        Record<string, Color> = {"key1": Color.RED, "key2": Color.GREEN};
    let result: boolean = record_test.Mapfunc17(testData);
    arktest.assertEQ(result, true);
}

// function mapfunc18(m: Record<string, Pair>): boolean
function testMapfunc18() {
    let pair1: Pair = {a: "one", b: true};
    let pair2: Pair = {a: "two", b: false};
    let testData: Record<string, Pair> = {"key1": pair1, "key2": pair2};
    let result: boolean = record_test.Mapfunc18(testData);
    arktest.assertEQ(result, true);
}

// function mapfunc19(m: Record<string, ICpuZero>): boolean
function testMapfunc19() {
    let a: ICpuZero = record_test.MakeICpuZero();
    let b: ICpuZero = record_test.MakeICpuZero();
    let testData: Record<string, ICpuZero> = {"key1": a, "key2": b};
    let result: boolean = record_test.Mapfunc19(testData);
    arktest.assertEQ(result, true);
}

// function mapfunc20(m: Record<string, ICpuInfo>): boolean
function testMapfunc20() {
    let a: ICpuInfo = record_test.MakeICpuInfo();
    let b: ICpuInfo = record_test.MakeICpuInfo();
    let testData: Record<string, ICpuInfo> = {"key1": a, "key2": a};
    let result: boolean = record_test.Mapfunc20(testData);
    arktest.assertEQ(result, true);
}

// function mapfunc21(m: Record<string, Object>): boolean
function testMapfunc21() {
    let testData: Record<string, Object> = {"key1": "val1", "key2": "val2"};
    let result: boolean = record_test.Mapfunc21(testData);
    arktest.assertEQ(result, true);
}

// function mapfunc22(m: Record<string, Record<string, boolean>>): boolean
function testMapfunc22() {
    let testData: Record<string, Record<string, boolean>> = {
        "key1": {"subkey1": true, "subkey2": false},
        "key2": {"subkey3": true, "subkey4": false}
    };
    let result: boolean = record_test.Mapfunc22(testData);
    arktest.assertEQ(result, true);
}

// function mapfunc23(m: Record<string, Record<string, int>>): boolean
function testMapfunc23() {
    let testData: Record<string, Record<string, int>> = {
        "key1": {"subkey1": 100, "subkey2": 200},
        "key2": {"subkey3": 300, "subkey4": 400}
    };
    let result: boolean = record_test.Mapfunc23(testData);
    arktest.assertEQ(result, true);
}

// function mapfunc24(m: Record<string, Record<string, (int[])>>): boolean
function testMapfunc24() {
    let testData: Record<string, Record<string, (int[])>> = {
        "key1": {"subkey1": [1, 2, 3], "subkey2": [4, 5, 6]},
        "key2": {"subkey3": [7, 8, 9], "subkey4": [10, 11, 12]}
    };
    let result: boolean = record_test.Mapfunc24(testData);
    arktest.assertEQ(result, true);
}

// function mapfunc25(m: Record<string, Record<string, string>>): boolean
function testMapfunc25() {
    let testData: Record<string, Record<string, string>> = {
        "key1": {"subkey1": "value1", "subkey2": "value2"},
        "key2": {"subkey3": "value3", "subkey4": "value4"}
    };
    let result: boolean = record_test.Mapfunc25(testData);
    arktest.assertEQ(result, true);
}

// function mapfunc26(): Record<string, boolean>
function testMapfunc26() {
    let result: Record<string, boolean> = record_test.Mapfunc26();
    arktest.assertEQ(result.has("key1"), true);
}

// function mapfunc27(): Record<string, byte>
function testMapfunc27() {
    let result: Record<string, byte> = record_test.Mapfunc27();
    arktest.assertEQ(result.has("key1"), true);
}

// function mapfunc28(): Record<string, short>
function testMapfunc28() {
    let result: Record<string, short> = record_test.Mapfunc28();
    arktest.assertEQ(result.has("key1"), true);
}

// function mapfunc29(): Record<string, int>
function testMapfunc29() {
    let result: Record<string, int> = record_test.Mapfunc29();
    arktest.assertEQ(result.has("key1"), true);
}

// function mapfunc30(): Record<string, long>
function testMapfunc30() {
    let result: Record<string, long> = record_test.Mapfunc30();
    arktest.assertEQ(result.has("key1"), true);
}

// function mapfunc31(): Record<string, float>
function testMapfunc31() {
    let result: Record<string, float> = record_test.Mapfunc31();
    arktest.assertEQ(result.has("key1"), true);
}

// function mapfunc32(): Record<string, double>
function testMapfunc32() {
    let result: Record<string, double> = record_test.Mapfunc32();
    arktest.assertEQ(result.has("key1"), true);
}

// function mapfunc33(): Record<string, string>
function testMapfunc33() {
    let result: Record<string, string> = record_test.Mapfunc33();
    arktest.assertEQ(result.has("key1"), true);
}

// function mapfunc34(): Record<string, (byte[])>
function testMapfunc34() {
    let result: Record<string, (byte[])> = record_test.Mapfunc34();
    arktest.assertEQ(result.has("key1"), true);
}

// function mapfunc35(): Record<string, (short[])>
function testMapfunc35() {
    let result: Record<string, (short[])> = record_test.Mapfunc35();
    arktest.assertEQ(result.has("key1"), true);
}

// function mapfunc36(): Record<string, (int[])>
function testMapfunc36() {
    let result: Record<string, (int[])> = record_test.Mapfunc36();
    arktest.assertEQ(result.has("key1"), true);
}

// function mapfunc37(): Record<string, (long[])>
function testMapfunc37() {
    let result: Record<string, (long[])> = record_test.Mapfunc37();
    arktest.assertEQ(result.has("key1"), true);
}

// function mapfunc38(): Record<string, ArrayBuffer>
function testMapfunc38() {
    let result: Record<string, ArrayBuffer> = record_test.Mapfunc38();
    arktest.assertEQ(result.has("key1"), true);
}

// function mapfunc39(): Record<string, (boolean[])>
function testMapfunc39() {
    let result: Record<string, (boolean[])> = record_test.Mapfunc39();
    arktest.assertEQ(result.has("key1"), true);
}

// function mapfunc40(): Record<string, (string[])>
function testMapfunc40() {
    let result: Record<string, (string[])> = record_test.Mapfunc40();
    arktest.assertEQ(result.has("key1"), true);
}

// function mapfunc41(): Record<string, (int | boolean | string)>
function testMapfunc41() {
    let result: Record<string, (int|boolean|string)> = record_test.Mapfunc41();
    arktest.assertEQ(result.has("key1"), true);
}

// function mapfunc42(): Record<string, Color>
function testMapfunc42() {
    let result: Record<string, Color> = record_test.Mapfunc42();
    arktest.assertEQ(result.has("key1"), true);
}

// function mapfunc43(): Record<string, Pair>
function testMapfunc43() {
    let result: Record<string, Pair> = record_test.Mapfunc43();
    arktest.assertEQ(result.has("key1"), true);
}

// function mapfunc44(): Record<string, ICpuZero>
function testMapfunc44() {
    let result: Record<string, ICpuZero> = record_test.Mapfunc44();
    arktest.assertEQ(result.has("key1"), true);
}

// function mapfunc45(): Record<string, ICpuInfo>
function testMapfunc45() {
    let result: Record<string, ICpuInfo> = record_test.Mapfunc45();
    arktest.assertEQ(result.has("key1"), true);
}

// function mapfunc46(): Record<string, Object>
function testMapfunc46() {
    let result: Record<string, Object> = record_test.Mapfunc46();
    arktest.assertEQ(result.has("key1"), true);
}

// function mapfunc47(): Record<string, Record<string, boolean>>
function testMapfunc47() {
    let result: Record<string, Record<string, boolean>> =
        record_test.Mapfunc47();
    arktest.assertEQ(result.has("key1"), true);
}

// function mapfunc48(): Record<string, Record<string, int>>
function testMapfunc48() {
    let result: Record<string, Record<string, int>> = record_test.Mapfunc48();
    arktest.assertEQ(result.has("key1"), true);
}

// function mapfunc49(): Record<string, Record<string, (int[])>>
function testMapfunc49() {
    let result: Record<string, Record<string, (int[])>> =
        record_test.Mapfunc49();
    arktest.assertEQ(result.has("key1"), true);
}

// function mapfunc50(): Record<string, Record<string, string>>
function testMapfunc50() {
    let result: Record<string, Record<string, string>> =
        record_test.Mapfunc50();
    arktest.assertEQ(result.has("key1"), true);
}

// function mapfunc51(op: (Record<string, string> | undefined)): Record<string,
// Record<string, string>>
function testMapfunc51() {
    let testData: Record<string, string> = {"key1": "value1", "key2": "value2"};
    let result: Record<string, Record<string, string>> =
        record_test.Mapfunc51(testData);
    arktest.assertEQ(result.has("key1"), true);
}
