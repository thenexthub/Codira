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
import * as ArrayTest from "array_test";

loadLibrary("ani_array");

function sumArray() {
    let numbers: int[] = [1, 2, 3, 4, 5];
    let sum = ArrayTest.sumArray(numbers, 100)
    console.log("sum is " + sum)
    arktest.assertEQ(sum, 115)
}

function getArrayValue() {
    let nums: long[] = [35, 45, 55];
    let res = ArrayTest.getArrayValue(nums, 2)
    console.log("index 2 is " + res)
    arktest.assertEQ(res, 55)
}

function toStingArray() {
    let nums: int[] = [35, 45, 55];
    let res = ArrayTest.toStingArray(nums)
    console.log("new array is " + res)
    arktest.assertEQ(res.length, nums.length)
    for (let i = 0; i < res.length; i++) {
        arktest.assertEQ(res[i], `${nums[i]}`)
    }
}

function makeIntArray() {
    let arr = ArrayTest.makeIntArray(5, 3)
    console.log("new array is " + arr)
    arktest.assertEQ(arr.length, 3)
    for (let i = 0; i < arr.length; i++) {
        arktest.assertEQ(arr[i], 5)
    }
}

function makeEnumArray() {
    let arr = ArrayTest.makeEnumArray(ArrayTest.Color.GREEN, 5)
    console.log("new array is " + arr)
    arktest.assertEQ(arr.length, 5)
    for (let i = 0; i < arr.length; i++) {
        arktest.assertEQ(arr[i], ArrayTest.Color.GREEN)
    }
}

function makeRecordArray() {
    let arr = ArrayTest.makeRecordArray("k", 128, 3)
    console.log("new array is " + arr)
    arktest.assertEQ(arr.length, 3)
    for (let i = 0; i < arr.length; i++) {
        arktest.assertEQ(arr[i]["k"], 128)
    }
}

function makeStructArray() {
    let arr = ArrayTest.makeStructArray("a", "b", 5, 3)
    console.log("new array is " + arr)
    arktest.assertEQ(arr.length, 3)
    for (let i = 0; i < arr.length; i++) {
        arktest.assertEQ(arr[i].a, "a")
        arktest.assertEQ(arr[i].b, "b")
        arktest.assertEQ(arr[i].c, 5)
    }
}

function makeIntArray2() {
    let arr = ArrayTest.makeIntArray(5, 3);
    let arr2 = ArrayTest.makeIntArray2(arr, 3);
    arktest.assertEQ(arr2.length, 3);
    for (let i = 0; i < arr2.length; i++) {
        arktest.assertEQ(arr2[i].length, 3)
    }
}

function changeEnumArray() {
    let arr = ArrayTest.makeEnumArray(ArrayTest.Color.GREEN, 5);
    let arr2 = ArrayTest.changeEnumArray(arr, ArrayTest.Color.BLUE);
    arktest.assertEQ(arr.length, arr2.length)
    for (let i = 0; i < arr2.length; i++) {
        arktest.assertEQ(arr2[i], ArrayTest.Color.BLUE)
    }
}

function changeRecordArray() {
    let arr = ArrayTest.makeRecordArray("k", 128, 3)
    let arr2 = ArrayTest.changeRecordArray(arr, "v", 21);
    arktest.assertEQ(arr.length, arr2.length)
    for (let i = 0; i < arr2.length; i++) {
        arktest.assertEQ(arr2[i]["v"], 21)
    }
}

function changeStructArray() {
    let arr = ArrayTest.makeStructArray("a", "b", 5, 3)
    let arr2 = ArrayTest.changeStructArray(arr, "aa", "bb", 3);
    arktest.assertEQ(arr.length, arr2.length)
    for (let i = 0; i < arr2.length; i++) {
        arktest.assertEQ(arr2[i].a, "aa")
        arktest.assertEQ(arr2[i].b, "bb")
        arktest.assertEQ(arr2[i].c, 3)
    }
}

function fetchBinaryData() {
    let promise = new Promise<Array<float>>(
        (resolve, reject) => {ArrayTest.fetchBinaryData(
            5, (error: BusinessError|null, data?: Array<float>) => {
                if (error !== null && error.code !== 0) {
                    reject(error);
                } else {
                    resolve(data as Array<float>);
                }
            })});
    let data: Array<float>|undefined;
    let error: BusinessError|undefined;
    try {
        data = await promise;
    } catch (e) {
        error = e as BusinessError;
    }
    arktest.assertEQ(data?.length, 5);
}

function makeStructArrayArray() {
    let arr = ArrayTest.makeStructArrayArray("a", "b", 5, 2, 3)
    console.log("new array is " + arr)
    arktest.assertEQ(arr.length, 3)
    arktest.assertEQ(arr[0].length, 2)
    for (let i = 0; i < arr.length; i++) {
        for (let j = 0; j < arr[i].length; j++) {
            arktest.assertEQ(arr[i][j].a, "a")
            arktest.assertEQ(arr[i][j].b, "b")
            arktest.assertEQ(arr[i][j].c, 5)
        }
    }
}

function test_ani_array_with_arrayi8() {
    let arrayi8: ArrayTest.ArrayI8 = ArrayTest.getArrayI8();
    let info = arrayi8.arrayI8Value(127 as byte);

    console.log("arrayi8.arrayI8Value is: " + info);
    arktest.assertEQ(info[0], 127 as byte);
}

function test_ani_array_with_arrayi16() {
    let arrayi16: ArrayTest.ArrayI16 = ArrayTest.getArrayI16();
    let info = arrayi16.arrayI16Value(-32768 as short);

    console.log("arrayi16.arrayI16Value is: " + info);
    arktest.assertEQ(info[0], -32768 as short);
}

function test_ani_array_with_arrayi32() {
    let arrayi32: ArrayTest.ArrayI32 = ArrayTest.getArrayI32();
    let info = arrayi32.arrayI32Value(2147483647 as int);

    console.log("arrayi32.arrayI32Value is: " + info);
    arktest.assertEQ(info[0], 2147483647 as int);
}

function test_ani_array_with_arrayi64() {
    let arrayi64: ArrayTest.ArrayI64 = ArrayTest.getArrayI64();
    let info = arrayi64.arrayI64Value(-9223372036854775808 as long);

    console.log("arrayi64.arrayI64Value is: " + info);
    arktest.assertEQ(info[0], -9223372036854775808 as long);
}

function test_ani_array_with_arrayf32() {
    let arrayf32: ArrayTest.ArrayF32 = ArrayTest.getArrayF32();
    let info = arrayf32.arrayF32Value((202510.25).toFloat());

    console.log("arrayf32.arrayF32Value is: " + info);
    arktest.assertEQ(info[0], (202510.25).toFloat());
}

function test_ani_array_with_arrayf64() {
    let arrayf64: ArrayTest.ArrayF64 = ArrayTest.getArrayF64();
    let info = arrayf64.arrayF64Value((20250521.1027).toDouble());

    console.log("arrayf64.arrayF64Value is: " + info);
    arktest.assertEQ(info[0], (20250521.1027).toDouble());
}

function test_ani_array_with_arrayString() {
    let arrayString: ArrayTest.ArrayString = ArrayTest.getArrayString();
    let info = arrayString.arrayStringValue("20250521.1035");

    console.log("arrayString.arrayStringValue is: " + info);
    arktest.assertEQ(info[0], "20250521.1035");
}

function test_ani_array_with_arrayBool() {
    let arrayBool: ArrayTest.ArrayBool = ArrayTest.getArrayBool();
    let info = arrayBool.arrayBoolValue(true);

    console.log("arrayBool.arrayBoolValue is: " + info);
    arktest.assertEQ(info[0], true);
}

function test_ani_array_with_arrayEnum() {
    let arr: ArrayTest.ArrayEnum = ArrayTest.getArrayEnum();
    let info = arr.arrayEnumValue("Color");

    console.log("arr.arrayEnumValue is: " + info);
    arktest.assertEQ(info[0], ArrayTest.Color.RED);
}

function test_ani_array_with_arrayRecord() {
    let arr: ArrayTest.ArrayRecord = ArrayTest.getArrayRecord();
    let info = arr.arrayRecordValue("Color", 255 as long);

    console.log("arr.arrayRecordValue is: " + info);
    arktest.assertEQ(info[0].size, 1);
}

function test_ani_array_with_arrayRecord_stri8() {
    let arr: ArrayTest.ArrayRecordStrI8 = ArrayTest.getArrayRecordStrI8();
    let info = arr.arrayRecordValue("Color", 127 as byte);

    console.log("arr.arrayRecordValue is: " + info);
    arktest.assertEQ(info[0].size, 1);
}

function test_ani_array_with_arrayRecord_strf32() {
    let arr: ArrayTest.ArrayRecordStrF32 = ArrayTest.getArrayRecordStrF32();
    let info = arr.arrayRecordValue("Color", (52115.24).toFloat());

    console.log("arr.arrayRecordValue is: " + info);
    arktest.assertEQ(info[0].size, 1);
}

function test_ani_array_with_arrayRecord_strf64() {
    let arr: ArrayTest.ArrayRecordStrF64 = ArrayTest.getArrayRecordStrF64();
    let info = arr.arrayRecordValue("Color", (202552115.24).toDouble());

    console.log("arr.arrayRecordValue is: " + info);
    arktest.assertEQ(info[0].size, 1);
}

function test_ani_array_with_arrayRecord_strbool() {
    let arr: ArrayTest.ArrayRecordStrBool = ArrayTest.getArrayRecordStrBool();
    let info = arr.arrayRecordValue("Color", true);

    console.log("arr.arrayRecordValue is: " + info);
    arktest.assertEQ(info[0].size, 1);
}

function test_ani_array_with_union() {
    let arr: ArrayTest.ArrayUnion = ArrayTest.getArrayUnion();
    let info = arr.arrayValue("Color");

    console.log("arr.arrayValue is: " + info);
    arktest.assertEQ(info[0], "Color");
}

let arrI8: ArrayTest.ArrayPromiseI8 = ArrayTest.getArrayPromiseI8();
let arrI16: ArrayTest.ArrayPromiseI16 = ArrayTest.getArrayPromiseI16();
let arrI32: ArrayTest.ArrayPromiseI32 = ArrayTest.getArrayPromiseI32();
let arrI64: ArrayTest.ArrayPromiseI64 = ArrayTest.getArrayPromiseI64();
let arrF32: ArrayTest.ArrayPromiseF32 = ArrayTest.getArrayPromiseF32();
let arrF64: ArrayTest.ArrayPromiseF64 = ArrayTest.getArrayPromiseF64();
let arrStr: ArrayTest.ArrayPromiseString = ArrayTest.getArrayPromiseString();
let arrBool: ArrayTest.ArrayPromiseBool = ArrayTest.getArrayPromiseBool();
let arrU8: ArrayTest.ArrayPromiseU8 = ArrayTest.getArrayPromiseU8();
let arrU16: ArrayTest.ArrayPromiseU16 = ArrayTest.getArrayPromiseU16();
function test_ani_array_with_PromiseI8() {
    let info: byte[] = new Array<byte>(0);
    let err: BusinessError = new BusinessError();
    try {
        info = arrI8.fetchDataI8();
    } catch (e) {
        err = e as BusinessError;
    }

    console.log("arr.fetchDataI8 is: " + info);
    arktest.assertEQ(info[0], -128 as byte);
}

function test_ani_array_with_PromiseI8_sync() {
    let promise = new Promise<byte[]>((resolve, reject) => {
        arrI8.fetchData((err: BusinessError|null, data?: byte[]) => {
            if (err !== null && err.code !== 0) {
                reject(err);
            } else {
                resolve(data as byte[]);
            }
        });
    });
    let info: byte[] = new Array<byte>(0);
    let err: BusinessError|undefined;
    try {
        info = await promise;
        console.log("info is: " + info);
    } catch (e) {
        err = e as BusinessError;
    }
    arktest.assertEQ(info[0], -128 as byte);
}

function test_ani_array_with_PromiseI8_promise() {
    let info: byte[] = new Array<byte>(0);
    let err: BusinessError = new BusinessError();
    try {
        info = await arrI8.fetchData();
    } catch (e) {
        err = e as BusinessError;
    }
    arktest.assertEQ(info[0], -128 as byte);
}

function test_ani_array_with_PromiseI16() {
    let info: short[] = new Array<short>(0);
    let err: BusinessError = new BusinessError();
    try {
        info = arrI16.fetchDataI16();
    } catch (e) {
        err = e as BusinessError;
    }

    console.log("arr.fetchDataI16 is: " + info);
    arktest.assertEQ(info[0], -32768 as short);
}

function test_ani_array_with_PromiseI16_sync() {
    let promise = new Promise<short[]>((resolve, reject) => {
        arrI16.fetchData((err: BusinessError|null, data?: short[]) => {
            if (err !== null && err.code !== 0) {
                reject(err);
            } else {
                resolve(data as short[]);
            }
        });
    });
    let info: short[] = new Array<short>(0);
    let err: BusinessError = new BusinessError();
    try {
        info = await promise;
        console.log("info is: " + info);
    } catch (e) {
        err = e as BusinessError;
    }
    arktest.assertEQ(info[0], -32768 as short);
}

function test_ani_array_with_PromiseI16_promise() {
    let info: short[] = new Array<short>(0);
    let err: BusinessError = new BusinessError();
    try {
        info = await arrI16.fetchData();
    } catch (e) {
        err = e as BusinessError;
    }
    arktest.assertEQ(info[0], -32768 as short);
}

function test_ani_array_with_PromiseI32() {
    let info: int[] = new Array<int>(0);
    let err: BusinessError = new BusinessError();
    try {
        info = arrI32.fetchDataI32();
    } catch (e) {
        err = e as BusinessError;
    }

    console.log("arr.fetchDataI32 is: " + info);
    arktest.assertEQ(info[0], -2147483648);
}

function test_ani_array_with_PromiseI32_sync() {
    let promise = new Promise<int[]>((resolve, reject) => {
        arrI32.fetchData((err: BusinessError|null, data?: int[]) => {
            if (err !== null && err.code !== 0) {
                reject(err);
            } else {
                resolve(data as int[]);
            }
        });
    });
    let info: int[] = new Array<int>(0);
    let err: BusinessError = new BusinessError();
    try {
        info = await promise;
        console.log("info is: " + info);
    } catch (e) {
        err = e as BusinessError;
    }
    arktest.assertEQ(info[0], -2147483648);
}

function test_ani_array_with_PromiseI32_promise() {
    let info: int[] = new Array<int>(0);
    let err: BusinessError = new BusinessError();
    try {
        info = await arrI32.fetchData();
    } catch (e) {
        err = e as BusinessError;
    }
    arktest.assertEQ(info[0], -2147483648);
}

function test_ani_array_with_PromiseI64() {
    let info: long[] = new Array<long>(0);
    let err: BusinessError = new BusinessError();
    try {
        info = arrI64.fetchDataI64();
    } catch (e) {
        err = e as BusinessError;
    }

    console.log("arr.fetchDataI64 is: " + info);
    arktest.assertEQ(info[0], -9223372036854775807 as long);
}

function test_ani_array_with_PromiseI64_sync() {
    let promise = new Promise<long[]>((resolve, reject) => {
        arrI64.fetchData((err: BusinessError|null, data?: long[]) => {
            if (err !== null && err.code !== 0) {
                reject(err);
            } else {
                resolve(data as long[]);
            }
        });
    });
    let info: long[] = new Array<long>(0);
    let err: BusinessError = new BusinessError();
    try {
        info = await promise;
        console.log("info is: " + info);
    } catch (e) {
        err = e as BusinessError;
    }
    arktest.assertEQ(info[0], -9223372036854775807 as long);
}

function test_ani_array_with_PromiseI64_promise() {
    let info: long[] = new Array<long>(0);
    let err: BusinessError = new BusinessError();
    try {
        info = await arrI64.fetchData();
    } catch (e) {
        err = e as BusinessError;
    }
    arktest.assertEQ(info[0], -9223372036854775807 as long);
}

function test_ani_array_with_PromiseF32() {
    let info: float[] = new Array<float>(0);
    let err: BusinessError = new BusinessError();
    try {
        info = arrF32.fetchDataF32();
    } catch (e) {
        err = e as BusinessError;
    }

    console.log("arr.fetchDataF32 is: " + info);
    arktest.assertEQ(info[0], (-202505.22).toFloat());
}

function test_ani_array_with_PromiseF32_sync() {
    let promise = new Promise<float[]>((resolve, reject) => {
        arrF32.fetchData((err: BusinessError|null, data?: float[]) => {
            if (err !== null && err.code !== 0) {
                reject(err);
            } else {
                resolve(data as float[]);
            }
        });
    });
    let info: float[] = new Array<float>(0);
    let err: BusinessError = new BusinessError();
    try {
        info = await promise;
        console.log("info is: " + info);
    } catch (e) {
        err = e as BusinessError;
    }
    arktest.assertEQ(info[0], (-202505.22).toFloat());
}

function test_ani_array_with_PromiseF32_promise() {
    let info: float[] = new Array<float>(0);
    let err: BusinessError = new BusinessError();
    try {
        info = await arrF32.fetchData();
    } catch (e) {
        err = e as BusinessError;
    }
    arktest.assertEQ(info[0], (-202505.22).toFloat());
}

function test_ani_array_with_PromiseF64() {
    let info: double[] = new Array<double>(0);
    let err: BusinessError = new BusinessError();
    try {
        info = arrF64.fetchDataF64();
    } catch (e) {
        err = e as BusinessError;
    }

    console.log("arr.fetchDataF64 is: " + info);
    arktest.assertEQ(info[0], (-202505.221539).toDouble());
}

function test_ani_array_with_PromiseF64_sync() {
    let promise = new Promise<double[]>((resolve, reject) => {
        arrF64.fetchData((err: BusinessError|null, data?: double[]) => {
            if (err !== null && err.code !== 0) {
                reject(err);
            } else {
                resolve(data as double[]);
            }
        });
    });
    let info: double[] = new Array<double>(0);
    let err: BusinessError = new BusinessError();
    try {
        info = await promise;
        console.log("info is: " + info);
    } catch (e) {
        err = e as BusinessError;
    }
    arktest.assertEQ(info[0], -202505.221539 as double);
}

function test_ani_array_with_PromiseF64_promise() {
    let info: double[] = new Array<double>(0);
    let err: BusinessError = new BusinessError();
    try {
        info = await arrF64.fetchData();
    } catch (e) {
        err = e as BusinessError;
    }
    arktest.assertEQ(info[0], -202505.221539 as double);
}

function test_ani_array_with_PromiseString() {
    let info: string[] = new Array<string>(0);
    let err: BusinessError = new BusinessError();
    try {
        info = arrStr.fetchDataString();
    } catch (e) {
        err = e as BusinessError;
    }

    console.log("arr.fetchDataString is: " + info);
    arktest.assertEQ(info[0], "String");
}

function test_ani_array_with_PromiseString_sync() {
    let promise = new Promise<string[]>((resolve, reject) => {
        arrStr.fetchData((err: BusinessError|null, data?: string[]) => {
            if (err !== null && err.code !== 0) {
                reject(err);
            } else {
                resolve(data as string[]);
            }
        });
    });
    let info: string[] = new Array<string>(0);
    let err: BusinessError = new BusinessError();
    try {
        info = await promise;
        console.log("info is: " + info);
    } catch (e) {
        err = e as BusinessError;
    }
    arktest.assertEQ(info[0], "String");
}

function test_ani_array_with_PromiseString_promise() {
    let info: string[] = new Array<string>(0);
    let err: BusinessError = new BusinessError();
    try {
        info = await arrStr.fetchData();
    } catch (e) {
        err = e as BusinessError;
    }
    arktest.assertEQ(info[0], "String");
}

function test_ani_array_with_PromiseBool() {
    let info: boolean[] = new Array<boolean>(0);
    let err: BusinessError = new BusinessError();
    try {
        info = arrBool.fetchDataBool();
    } catch (e) {
        err = e as BusinessError;
    }

    console.log("arr.fetchDataBool is: " + info);
    arktest.assertEQ(info[0], true);
}

function test_ani_array_with_PromiseBool_sync() {
    let promise = new Promise<boolean[]>((resolve, reject) => {
        arrBool.fetchData((err: BusinessError|null, data?: boolean[]) => {
            if (err !== null && err.code !== 0) {
                reject(err);
            } else {
                resolve(data as boolean[]);
            }
        });
    });
    let info: boolean[] = new Array<boolean>(0);
    let err: BusinessError = new BusinessError();
    try {
        info = await promise;
        console.log("info is: " + info);
    } catch (e) {
        err = e as BusinessError;
    }
    arktest.assertEQ(info[0], true);
}

function test_ani_array_with_PromiseBool_promise() {
    let info: boolean[] = new Array<boolean>(0);
    let err: BusinessError = new BusinessError();
    try {
        info = await arrBool.fetchData();
    } catch (e) {
        err = e as BusinessError;
    }
    arktest.assertEQ(info[0], true);
}

function test_ani_array_with_PromiseU8() {
    let a = new ArrayBuffer(10);
    let info = new Uint8Array(a);
    let err: BusinessError = new BusinessError();
    try {
        info = arrU8.fetchDataPro();
    } catch (e) {
        err = e as BusinessError;
    }

    console.log("arr.fetchDataPro is: " + info);
    arktest.assertEQ(info[1], 255);
}

function test_ani_array_with_PromiseU8_sync() {
    let promise = new Promise<Uint8Array>((resolve, reject) => {
        arrU8.fetchData((err: BusinessError|null, data?: Uint8Array) => {
            if (err !== null && err.code !== 0) {
                reject(err);
            } else {
                resolve(data as Uint8Array);
            }
        });
    });
    let info: Uint8Array = new Uint8Array(0);
    let err: BusinessError = new BusinessError();
    try {
        info = await promise;
        console.log("info is: " + info);
    } catch (e) {
        err = e as BusinessError;
    }
    arktest.assertEQ(info[1], 255);
}

function test_ani_array_with_PromiseU8_promise() {
    let info: Uint8Array = new Uint8Array(0);
    let err: BusinessError = new BusinessError();
    try {
        info = await arrU8.fetchData();
    } catch (e) {
        err = e as BusinessError;
    }
    arktest.assertEQ(info[1], 255);
}

function test_ani_array_with_PromiseU16() {
    let info: Uint16Array = new Uint16Array(0);
    let err: BusinessError = new BusinessError();
    try {
        info = arrU16.fetchDataPro();
    } catch (e) {
        err = e as BusinessError;
    }

    console.log("arr.fetchDataPro is: " + info);
    arktest.assertEQ(info[1], 65535);
}

function test_ani_array_with_PromiseU16_sync() {
    let promise = new Promise<Uint16Array>((resolve, reject) => {
        arrU16.fetchData((err: BusinessError|null, data?: Uint16Array) => {
            if (err !== null && err.code !== 0) {
                reject(err);
            } else {
                resolve(data as Uint16Array);
            }
        });
    });
    let info: Uint16Array = new Uint16Array(0);
    let err: BusinessError = new BusinessError();
    try {
        info = await promise;
        console.log("info is: " + info);
    } catch (e) {
        err = e as BusinessError;
    }
    arktest.assertEQ(info[1], 65535);
}

function test_ani_array_with_PromiseU16_promise() {
    let info: Uint16Array = new Uint16Array(0);
    let err: BusinessError = new BusinessError();
    try {
        info = await arrU16.fetchData();
    } catch (e) {
        err = e as BusinessError;
    }
    arktest.assertEQ(info[1], 65535);
}

let arrNest: ArrayTest.ArrayNest = ArrayTest.getArrayNest();
function test_ani_array_with_nest_i8() {
    let info = arrNest.arrayNestI8(120 as byte);

    console.log("arrNest.ArrayNestI8 is: " + info);
    arktest.assertEQ(info.length, 4);
}

function test_ani_array_with_nest_i16() {
    let info = arrNest.arrayNestI16(32700 as short);

    console.log("arrNest.ArrayNestI16 is: " + info);
    arktest.assertEQ(info.length, 4);
}

function test_ani_array_with_nest_f32() {
    let info = arrNest.arrayNestF32((202505.28).toFloat());

    console.log("arrNest.ArrayNestF32 is: " + info);
    arktest.assertEQ(info.length, 4);
}

function test_ani_array_with_nest_string() {
    let info = arrNest.arrayNestStrng("0528");

    console.log("arrNest.ArrayNestStrng is: " + info);
    arktest.assertEQ(info.length, 4);
}

function test_ani_array_with_nest_bool() {
    let info = arrNest.arrayNestBool(true);

    console.log("arrNest.ArrayNestBool is: " + info);
    arktest.assertEQ(info.length, 3);
}

function test_ani_array_with_optional() {
    let arrOpt: ArrayTest.ArrayOptional = ArrayTest.getArrayOptional();
    let info = arrOpt.arrayOpt("optional");

    console.log("arrOpt.arrayOpt is: " + info);
    arktest.assertEQ(info?.[0], "optional");
}

function main() {
    console.log("run main ... ArrayTest ...")
    const suite = new arktest.ArkTestsuite("ArrayTest")

    suite.addTest("sumArray", sumArray)
    suite.addTest("getArrayValue", getArrayValue)
    suite.addTest("toStingArray", toStingArray)
    suite.addTest("makeIntArray", makeIntArray)
    suite.addTest("makeIntArray2", makeIntArray2)
    suite.addTest("makeEnumArray", makeEnumArray)
    suite.addTest("changeEnumArray", changeEnumArray)
    suite.addTest("makeRecordArray", makeRecordArray)
    suite.addTest("changeRecordArray", changeRecordArray)
    suite.addTest("makeStructArray", makeStructArray)
    suite.addTest("changeStructArray", changeStructArray)
    suite.addTest("fetchBinaryData", fetchBinaryData)
    suite.addTest("makeStructArrayArray", makeStructArrayArray)

    suite.addTest("test_ani_array_with_arrayi8", test_ani_array_with_arrayi8)
    suite.addTest("test_ani_array_with_arrayi16", test_ani_array_with_arrayi16)
    suite.addTest("test_ani_array_with_arrayi32", test_ani_array_with_arrayi32)
    suite.addTest("test_ani_array_with_arrayi64", test_ani_array_with_arrayi64)
    suite.addTest("test_ani_array_with_arrayf32", test_ani_array_with_arrayf32)
    suite.addTest("test_ani_array_with_arrayf64", test_ani_array_with_arrayf64)
    suite.addTest(
        "test_ani_array_with_arrayString", test_ani_array_with_arrayString)
    suite.addTest(
        "test_ani_array_with_arrayBool", test_ani_array_with_arrayBool)
    suite.addTest(
        "test_ani_array_with_arrayEnum", test_ani_array_with_arrayEnum)
    suite.addTest(
        "test_ani_array_with_arrayRecord", test_ani_array_with_arrayRecord)
    suite.addTest(
        "test_ani_array_with_arrayRecord",
        test_ani_array_with_arrayRecord_stri8)
    suite.addTest(
        "test_ani_array_with_arrayRecord",
        test_ani_array_with_arrayRecord_strf32)
    suite.addTest(
        "test_ani_array_with_arrayRecord",
        test_ani_array_with_arrayRecord_strf64)
    suite.addTest(
        "test_ani_array_with_arrayRecord",
        test_ani_array_with_arrayRecord_strbool)
    suite.addTest("test_ani_array_with_union", test_ani_array_with_union)
    suite.addTest(
        "test_ani_array_with_PromiseI8", test_ani_array_with_PromiseI8)
    suite.addTest(
        "test_ani_array_with_PromiseI8_sync",
        test_ani_array_with_PromiseI8_sync)
    suite.addTest(
        "test_ani_array_with_PromiseI8_promise",
        test_ani_array_with_PromiseI8_promise)
    suite.addTest(
        "test_ani_array_with_PromiseI16", test_ani_array_with_PromiseI16)
    suite.addTest(
        "test_ani_array_with_PromiseI16_sync",
        test_ani_array_with_PromiseI16_sync)
    suite.addTest(
        "test_ani_array_with_PromiseI16_promise",
        test_ani_array_with_PromiseI16_promise)
    suite.addTest(
        "test_ani_array_with_PromiseI32", test_ani_array_with_PromiseI32)
    suite.addTest(
        "test_ani_array_with_PromiseI32_sync",
        test_ani_array_with_PromiseI32_sync)
    suite.addTest(
        "test_ani_array_with_PromiseI32_promise",
        test_ani_array_with_PromiseI32_promise)
    suite.addTest(
        "test_ani_array_with_PromiseI64", test_ani_array_with_PromiseI64)
    suite.addTest(
        "test_ani_array_with_PromiseI64_sync",
        test_ani_array_with_PromiseI64_sync)
    suite.addTest(
        "test_ani_array_with_PromiseI64_promise",
        test_ani_array_with_PromiseI64_promise)
    suite.addTest(
        "test_ani_array_with_PromiseF32", test_ani_array_with_PromiseF32)
    suite.addTest(
        "test_ani_array_with_PromiseF32_sync",
        test_ani_array_with_PromiseF32_sync)
    suite.addTest(
        "test_ani_array_with_PromiseF32_promise",
        test_ani_array_with_PromiseF32_promise)
    suite.addTest(
        "test_ani_array_with_PromiseF64", test_ani_array_with_PromiseF64)
    suite.addTest(
        "test_ani_array_with_PromiseF64_sync",
        test_ani_array_with_PromiseF64_sync)
    suite.addTest(
        "test_ani_array_with_PromiseF64_promise",
        test_ani_array_with_PromiseF64_promise)
    suite.addTest(
        "test_ani_array_with_PromiseString", test_ani_array_with_PromiseString)
    suite.addTest(
        "test_ani_array_with_PromiseString_sync",
        test_ani_array_with_PromiseString_sync)
    suite.addTest(
        "test_ani_array_with_PromiseString_promise",
        test_ani_array_with_PromiseString_promise)
    suite.addTest(
        "test_ani_array_with_PromiseBool", test_ani_array_with_PromiseBool)
    suite.addTest(
        "test_ani_array_with_PromiseBool_sync",
        test_ani_array_with_PromiseBool_sync)
    suite.addTest(
        "test_ani_array_with_PromiseBool_promise",
        test_ani_array_with_PromiseBool_promise)
    suite.addTest(
        "test_ani_array_with_PromiseU8", test_ani_array_with_PromiseU8)
    suite.addTest(
        "test_ani_array_with_PromiseU8_sync",
        test_ani_array_with_PromiseU8_sync)
    suite.addTest(
        "test_ani_array_with_PromiseU8_promise",
        test_ani_array_with_PromiseU8_promise)
    suite.addTest(
        "test_ani_array_with_PromiseU16", test_ani_array_with_PromiseU16)
    suite.addTest(
        "test_ani_array_with_PromiseU16_sync",
        test_ani_array_with_PromiseU16_sync)
    suite.addTest(
        "test_ani_array_with_PromiseU16_promise",
        test_ani_array_with_PromiseU16_promise)

    suite.addTest("test_ani_array_with_nest_i8", test_ani_array_with_nest_i8)
    suite.addTest("test_ani_array_with_nest_i16", test_ani_array_with_nest_i16)
    suite.addTest("test_ani_array_with_nest_f32", test_ani_array_with_nest_f32)
    suite.addTest(
        "test_ani_array_with_nest_string", test_ani_array_with_nest_string)
    suite.addTest(
        "test_ani_array_with_nest_bool", test_ani_array_with_nest_bool)
    suite.addTest("test_ani_array_with_optional", test_ani_array_with_optional)

    exit(suite.run())
}
