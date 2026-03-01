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
import * as bar from "bar";

loadLibrary("ani_typedarray");

function checkUint8Array() {
    let arr = bar.newUint8Array(2, (0x01).toByte());
    arktest.assertEQ(arr.length, 2);
    arktest.assertEQ(arr.at(0), 0x01);
    arktest.assertEQ(arr.at(1), 0x01);
    let res = bar.sumUint8Array(arr);
    arktest.assertEQ(res, 0x01 * 2);
}

function checkUint16Array() {
    let arr = bar.newUint16Array(2, (0x0123).toShort());
    arktest.assertEQ(arr.at(0), 0x0123);
    arktest.assertEQ(arr.at(1), 0x0123);
    let res = bar.sumUint16Array(arr);
    arktest.assertEQ(res, 0x0123 * 2);
}

function checkUint32Array() {
    let arr = bar.newUint32Array(2, (0x01234567).toInt());
    arktest.assertEQ(arr.at(0), 0x01234567);
    arktest.assertEQ(arr.at(1), 0x01234567);
    let res = bar.sumUint32Array(arr);
    arktest.assertEQ(res, 0x01234567 * 2);
}

function checkBigUint64Array() {
    let arr = bar.newBigUint64Array(2, (21).toLong());
    arktest.assertEQ(arr.at(0), 21n);
    arktest.assertEQ(arr.at(1), 21n);
    let res = bar.sumBigUint64Array(arr);
    arktest.assertEQ(res, 42);
}

function checkInt8Array() {
    let arr = bar.newInt8Array(2, (0x01).toByte());
    arktest.assertEQ(arr.at(0), 0x01);
    arktest.assertEQ(arr.at(1), 0x01);
    let res = bar.sumInt8Array(arr);
    arktest.assertEQ(res, 0x01 * 2);
}

function checkInt16Array() {
    let arr = bar.newInt16Array(2, (0x0123).toShort());
    arktest.assertEQ(arr.at(0), 0x0123);
    arktest.assertEQ(arr.at(1), 0x0123);
    let res = bar.sumInt16Array(arr);
    arktest.assertEQ(res, 0x0123 * 2);
}

function checkInt32Array() {
    let arr = bar.newInt32Array(2, (0x01234567).toInt());
    arktest.assertEQ(arr.at(0), 0x01234567);
    arktest.assertEQ(arr.at(1), 0x01234567);
    let res = bar.sumInt32Array(arr);
    arktest.assertEQ(res, 0x01234567 * 2);
}

function checkBigInt64Array() {
    let arr = bar.newBigInt64Array(2, (21).toLong());
    arktest.assertEQ(arr.at(0), 21n);
    arktest.assertEQ(arr.at(1), 21n);
    let res = bar.sumBigInt64Array(arr);
    arktest.assertEQ(res, 42);
}

function checkFloat32Array() {
    let arr = bar.newFloat32Array(2, 0.1f);
    let tolerance = 0.00001f;
    arktest.assertEQ(Math.abs(arr.at(0)! - 0.1) < tolerance, true);
    arktest.assertEQ(Math.abs(arr.at(1)! - 0.1) < tolerance, true);
    let res = bar.sumFloat32Array(arr);
    arktest.assertEQ(Math.abs(res - 0.1 * 2) < tolerance, true);
}

function checkFloat64Array() {
    let arr = bar.newFloat64Array(2, (0.1).toDouble());
    let tolerance = (0.00001).toDouble();
    arktest.assertEQ(Math.abs(arr.at(0)! - 0.1) < tolerance, true);
    arktest.assertEQ(Math.abs(arr.at(1)! - 0.1) < tolerance, true);
    let res = bar.sumFloat64Array(arr);
    arktest.assertEQ(Math.abs(res - 0.1 * 2) < tolerance, true);
}

// bar.d.ts
let typedArrInfo: bar.TypedArrInfo = bar.getTypedArrInfo();

function test_bar_with_createAndPrintUint8Array() {
    let arr: Uint8Array = new Uint8Array([1, 2, 3, 4, 5]);
    typedArrInfo.createUint8Array(arr);
    let res = typedArrInfo.printUint8Array(arr);
    let arraysAreEqual = arr.length === res.length &&
        arr.every((value, index) => value === res[index.toInt()]);
    arktest.assertEQ(arraysAreEqual, true);
}

function test_bar_with_setAndgetUint8Array() {
    typedArrInfo.Uint8Array = new Uint8Array([1, 2, 3, 4, 5]);
    let res = typedArrInfo.Uint8Array;
    let arr = new Uint8Array([1, 2, 3, 4, 5]);
    let arraysAreEqual = arr.length === res.length &&
        arr.every((value, index) => value === res[index.toInt()]);
    arktest.assertEQ(arraysAreEqual, true);
}

function test_bar_with_createAndPrintInt8Array() {
    let arr: Int8Array = new Int8Array([1, -2, 3, -4, 5]);
    typedArrInfo.createInt8Array(arr);
    let res = typedArrInfo.printInt8Array(arr);
    let arraysAreEqual = arr.length === res.length &&
        arr.every((value, index) => value === res[index.toInt()]);
    arktest.assertEQ(arraysAreEqual, true);
}

function test_bar_with_setAndgetInt8Array() {
    typedArrInfo.Int8Array = new Int8Array([1, -2, 3, -4, 5]);
    let res = typedArrInfo.Int8Array;
    let arr = new Int8Array([1, -2, 3, -4, 5]);
    let arraysAreEqual = arr.length === res.length &&
        arr.every((value, index) => value === res[index.toInt()]);
    arktest.assertEQ(arraysAreEqual, true);
}

function test_bar_with_createAndPrintUint16Array() {
    let arr: Uint16Array = new Uint16Array([10, 20, 30, 40, 50]);
    typedArrInfo.createUint16Array(arr);
    let res = typedArrInfo.printUint16Array(arr);
    let arraysAreEqual = arr.length === res.length &&
        arr.every((value, index) => value === res[index.toInt()]);
    arktest.assertEQ(arraysAreEqual, true);
}

function test_bar_with_setAndgetUint16Array() {
    typedArrInfo.Uint16Array = new Uint16Array([10, 20, 30, 40, 50]);
    let res = typedArrInfo.Uint16Array;
    let arr = new Uint16Array([10, 20, 30, 40, 50]);
    let arraysAreEqual = arr.length === res.length &&
        arr.every((value, index) => value === res[index.toInt()]);
    arktest.assertEQ(arraysAreEqual, true);
}

function test_bar_with_createAndPrintInt16Array() {
    let arr: Int16Array = new Int16Array([10, -20, 30, -40, 50]);
    typedArrInfo.createInt16Array(arr);
    let res = typedArrInfo.printInt16Array(arr);
    let arraysAreEqual = arr.length === res.length &&
        arr.every((value, index) => value === res[index.toInt()]);
    arktest.assertEQ(arraysAreEqual, true);
}

function test_bar_with_setAndgetInt16Array() {
    typedArrInfo.Int16Array = new Int16Array([10, -20, 30, -40, 50]);
    let res = typedArrInfo.Int16Array;
    let arr = new Int16Array([10, -20, 30, -40, 50]);
    let arraysAreEqual = arr.length === res.length &&
        arr.every((value, index) => value === res[index.toInt()]);
    arktest.assertEQ(arraysAreEqual, true);
}

function test_bar_with_createAndPrintUint32Array() {
    let arr: Uint32Array = new Uint32Array([100, 200, 300, 400, 500]);
    typedArrInfo.createUint32Array(arr);
    let res = typedArrInfo.printUint32Array(arr);
    let arraysAreEqual = arr.length === res.length &&
        arr.every((value, index) => value === res[index.toInt()]);
    arktest.assertEQ(arraysAreEqual, true);
}

function test_bar_with_setAndgetUint32Array() {
    typedArrInfo.Uint32Array = new Uint32Array([100, 200, 300, 400, 500]);
    let res = typedArrInfo.Uint32Array;
    let arr = new Uint32Array([100, 200, 300, 400, 500]);
    let arraysAreEqual = arr.length === res.length &&
        arr.every((value, index) => value === res[index.toInt()]);
    arktest.assertEQ(arraysAreEqual, true);
}

function test_bar_with_createAndPrintInt32Array() {
    let arr: Int32Array = new Int32Array([100, -200, 300, -400, 500]);
    typedArrInfo.createInt32Array(arr);
    let res = typedArrInfo.printInt32Array(arr);
    let arraysAreEqual = arr.length === res.length &&
        arr.every((value, index) => value === res[index.toInt()]);
    arktest.assertEQ(arraysAreEqual, true);
}

function test_bar_with_setAndgetInt32Array() {
    typedArrInfo.Int32Array = new Int32Array([100, -200, 300, -400, 500]);
    let res = typedArrInfo.Int32Array;
    let arr = new Int32Array([100, -200, 300, -400, 500]);
    let arraysAreEqual = arr.length === res.length &&
        arr.every((value, index) => value === res[index.toInt()]);
    arktest.assertEQ(arraysAreEqual, true);
}

function test_bar_with_createAndPrintUint64Array() {
    let arr: BigUint64Array =
        new BigUint64Array([1000n, 2000n, 3000n, 4000n, 5000n]);
    typedArrInfo.createUint64Array(arr);
    let res = typedArrInfo.printUint64Array(arr);
    let arraysAreEqual = arr.length === res.length &&
        arr.every((value, index) => value === res[index.toInt()]);
    arktest.assertEQ(arraysAreEqual, true);
}

function test_bar_with_setAndgetUint64Array() {
    typedArrInfo.Uint64Array =
        new BigUint64Array([1000n, 2000n, 3000n, 4000n, 5000n]);
    let res = typedArrInfo.Uint64Array;
    let arr = new BigUint64Array([1000n, 2000n, 3000n, 4000n, 5000n]);
    let arraysAreEqual = arr.length === res.length &&
        arr.every((value, index) => value === res[index.toInt()]);
    arktest.assertEQ(arraysAreEqual, true);
}

function test_bar_with_createAndPrintInt64Array() {
    let arr: BigInt64Array =
        new BigInt64Array([1000n, -2000n, 3000n, -4000n, 5000n]);
    typedArrInfo.createInt64Array(arr);
    let res = typedArrInfo.printInt64Array(arr);
    let arraysAreEqual = arr.length === res.length &&
        arr.every((value, index) => value === res[index.toInt()]);
    arktest.assertEQ(arraysAreEqual, true);
}

function test_bar_with_setAndgetInt64Array() {
    typedArrInfo.Int64Array =
        new BigInt64Array([1000n, -2000n, 3000n, -4000n, 5000n]);
    let res = typedArrInfo.Int64Array;
    let arr = new BigInt64Array([1000n, -2000n, 3000n, -4000n, 5000n]);
    let arraysAreEqual = arr.length === res.length &&
        arr.every((value, index) => value === res[index.toInt()]);
    arktest.assertEQ(arraysAreEqual, true);
}

function test_bar_with_createAndPrintFloat32Array() {
    let arr: Float32Array = new Float32Array([1.0, 2.0, 3.0, 4.0, 5.0]);
    typedArrInfo.createFloat32Array(arr);
    let res = typedArrInfo.printFloat32Array(arr);
    let arraysAreEqual = arr.length === res.length &&
        arr.every(
            (value, index) => Math.abs(value - res[index.toInt()]) < 1e-6);
    arktest.assertEQ(arraysAreEqual, true);
}

function test_bar_with_setAndgetFloat32Array() {
    typedArrInfo.Float32Array = new Float32Array([1.0, 2.0, 3.0, 4.0, 5.0]);
    let res = typedArrInfo.Float32Array;
    let arr = new Float32Array([1.0, 2.0, 3.0, 4.0, 5.0]);
    let arraysAreEqual = arr.length === res.length &&
        arr.every(
            (value, index) => Math.abs(value - res[index.toInt()]) < 1e-6);
    arktest.assertEQ(arraysAreEqual, true);
}

function test_bar_with_createAndPrintFloat64Array() {
    let arr: Float64Array = new Float64Array([1.0, 2.0, 3.0, 4.0, 5.0]);
    typedArrInfo.createFloat64Array(arr);
    let res = typedArrInfo.printFloat64Array(arr);
    let arraysAreEqual = arr.length === res.length &&
        arr.every(
            (value, index) => Math.abs(value - res[index.toInt()]) < 1e-15);
    arktest.assertEQ(arraysAreEqual, true);
}

function test_bar_with_setAndgetFloat64Array() {
    typedArrInfo.Float64Array = new Float64Array([1.0, 2.0, 3.0, 4.0, 5.0]);
    let res = typedArrInfo.Float64Array;
    let arr = new Float64Array([1.0, 2.0, 3.0, 4.0, 5.0]);
    let arraysAreEqual = arr.length === res.length &&
        arr.every(
            (value, index) => Math.abs(value - res[index.toInt()]) < 1e-15);
    arktest.assertEQ(arraysAreEqual, true);
}

function test_bar_with_convertToInt64Array() {
    let uint8Arr: Uint8Array = new Uint8Array([1, 2, 3, 4, 5]);
    let uint16Arr: Uint16Array = new Uint16Array([10, 20, 30, 40, 50]);
    let uint32Arr: Uint32Array = new Uint32Array([100, 200, 300, 400, 500]);
    let uint64Arr: BigUint64Array =
        new BigUint64Array([1000n, 2000n, 3000n, 4000n, 5000n]);
    let int8Arr: Int8Array = new Int8Array([1, -2, 3, -4, 5]);
    let res = typedArrInfo.convertToInt64Array(
        uint8Arr, uint16Arr, uint32Arr, uint64Arr, int8Arr);
    let exceptArr: BigInt64Array =
        new BigInt64Array([1112n, 2220n, 3336n, 4440n, 5560n]);
    let arraysAreEqual = res.length === exceptArr.length &&
        res.every((value, index) => value === exceptArr[index.toInt()]);
    arktest.assertEQ(arraysAreEqual, true);
}

function test_bar_with_convertToFloat64Array() {
    let int16Arr: Int16Array = new Int16Array([10, -20, 30, -40, 50]);
    let int32Arr: Int32Array = new Int32Array([100, -200, 300, -400, 500]);
    let int64Arr: BigInt64Array =
        new BigInt64Array([1000n, -2000n, 3000n, -4000n, 5000n]);
    let float32Arr: Float32Array = new Float32Array([1.0, 2.0, 3.0, 4.0, 5.0]);
    let float64Arr: Float64Array = new Float64Array([1.0, 2.0, 3.0, 4.0, 5.0]);
    let res = typedArrInfo.convertToFloat64Array(
        int16Arr, int32Arr, int64Arr, float32Arr, float64Arr);
    let exceptArr: Float64Array =
        new Float64Array([1112.0, -2216.0, 3336.0, -4432.0, 5560.0]);
    let arraysAreEqual = res.length === exceptArr.length &&
        res.every(
            (value, index) =>
                Math.abs(value - exceptArr[index.toInt()]) < 1e-6);
    arktest.assertEQ(arraysAreEqual, true);
}

function test_bar_with_getUint8ArrayOptional_1() {
    let arr: Uint8Array = new Uint8Array([1, 2, 3, 4, 5]);
    let res = typedArrInfo.getUint8ArrayOptional(arr);
    console.log("test_bar_with_getUint8ArrayOptional_1 have Optional: ", res);
}

function test_bar_with_getUint8ArrayOptional_2() {
    let res = typedArrInfo.getUint8ArrayOptional(undefined);
    console.log(
        "test_bar_with_getUint8ArrayOptional_2 have no Optional: ", res);
}

function test_bar_with_getInt8ArrayOptional_1() {
    let arr: Int8Array = new Int8Array([1, -2, 3, -4, 5]);
    let res = typedArrInfo.getInt8ArrayOptional(arr);
    console.log("test_bar_with_getInt8ArrayOptional_1 have Optional: ", res);
}

function test_bar_with_getInt8ArrayOptional_2() {
    let res = typedArrInfo.getInt8ArrayOptional(undefined);
    console.log("test_bar_with_getInt8ArrayOptional_2 have no Optional: ", res);
}

function test_bar_with_getUint16ArrayOptional_1() {
    let arr: Uint16Array = new Uint16Array([10, 20, 30, 40, 50]);
    let res = typedArrInfo.getUint16ArrayOptional(arr);
    console.log("test_bar_with_getUint16ArrayOptional_1 have Optional: ", res);
}

function test_bar_with_getUint16ArrayOptional_2() {
    let res = typedArrInfo.getUint16ArrayOptional(undefined);
    console.log(
        "test_bar_with_getUint16ArrayOptional_2 have no Optional: ", res);
}

function test_bar_with_getInt16ArrayOptional_1() {
    let arr: Int16Array = new Int16Array([10, -20, 30, -40, 50]);
    let res = typedArrInfo.getInt16ArrayOptional(arr);
    console.log("test_bar_with_getInt16ArrayOptional_1 have Optional: ", res);
}

function test_bar_with_getInt16ArrayOptional_2() {
    let res = typedArrInfo.getInt16ArrayOptional(undefined);
    console.log(
        "test_bar_with_getInt16ArrayOptional_2 have no Optional: ", res);
}

function test_bar_with_getUint32ArrayOptional_1() {
    let arr: Uint32Array = new Uint32Array([100, 200, 300, 400, 500]);
    let res = typedArrInfo.getUint32ArrayOptional(arr);
    console.log("test_bar_with_getUint32ArrayOptional_1 have Optional: ", res);
}

function test_bar_with_getUint32ArrayOptional_2() {
    let res = typedArrInfo.getUint32ArrayOptional(undefined);
    console.log(
        "test_bar_with_getUint32ArrayOptional_2 have no Optional: ", res);
}

function test_bar_with_getInt32ArrayOptional_1() {
    let arr: Int32Array = new Int32Array([100, -200, 300, -400, 500]);
    let res = typedArrInfo.getInt32ArrayOptional(arr);
    console.log("test_bar_with_getInt32ArrayOptional_1 have Optional: ", res);
}

function test_bar_with_getInt32ArrayOptional_2() {
    let res = typedArrInfo.getInt32ArrayOptional(undefined);
    console.log(
        "test_bar_with_getInt32ArrayOptional_2 have no Optional: ", res);
}

function test_bar_with_getUint64ArrayOptional_1() {
    let arr: BigUint64Array =
        new BigUint64Array([1000n, 2000n, 3000n, 4000n, 5000n]);
    let res = typedArrInfo.getUint64ArrayOptional(arr);
    console.log("test_bar_with_getUint64ArrayOptional_1 have Optional: ", res);
}

function test_bar_with_getUint64ArrayOptional_2() {
    let res = typedArrInfo.getUint64ArrayOptional(undefined);
    console.log(
        "test_bar_with_getUint64ArrayOptional_2 have no Optional: ", res);
}

function test_bar_with_getInt64ArrayOptional_1() {
    let arr: BigInt64Array =
        new BigInt64Array([1000n, -2000n, 3000n, -4000n, 5000n]);
    let res = typedArrInfo.getInt64ArrayOptional(arr);
    console.log("test_bar_with_getInt64ArrayOptional_1 have Optional: ", res);
}

function test_bar_with_getInt64ArrayOptional_2() {
    let res = typedArrInfo.getInt64ArrayOptional(undefined);
    console.log(
        "test_bar_with_getInt64ArrayOptional_2 have no Optional: ", res);
}

function test_bar_with_getFloat32ArrayOptional_1() {
    let arr: Float32Array = new Float32Array([1.0, 2.0, 3.0, 4.0, 5.0]);
    let res = typedArrInfo.getFloat32ArrayOptional(arr);
    console.log("test_bar_with_getFloat32ArrayOptional_1 have Optional: ", res);
}

function test_bar_with_getFloat32ArrayOptional_2() {
    let res = typedArrInfo.getFloat32ArrayOptional(undefined);
    console.log(
        "test_bar_with_getFloat32ArrayOptional_2 have no Optional: ", res);
}

function test_bar_with_getFloat64ArrayOptional_1() {
    let arr: Float64Array = new Float64Array([1.0, 2.0, 3.0, 4.0, 5.0]);
    let res = typedArrInfo.getFloat64ArrayOptional(arr);
    console.log("test_bar_with_getFloat64ArrayOptional_1 have Optional: ", res);
}

function test_bar_with_getFloat64ArrayOptional_2() {
    let res = typedArrInfo.getFloat64ArrayOptional(undefined);
    console.log(
        "test_bar_with_getFloat64ArrayOptional_2 have no Optional: ", res);
}


function test_bar_with_mapUint8Arrays() {
    let data: Record<string, Uint8Array> = {
        "uint8Key": new Uint8Array([1, 2, 3, 4, 5])
    };
    let res = typedArrInfo.mapUint8Arrays(data);
    console.log("test_bar_with_mapUint8Arrays result: ", res);
}

function test_bar_with_mapInt8Arrays() {
    let data: Record<string, Int8Array> = {
        "int8Key": new Int8Array([1, -2, 3, -4, 5])
    };
    let res = typedArrInfo.mapInt8Arrays(data);
    console.log("test_bar_with_mapInt8Arrays result: ", res);
}

function test_bar_with_mapUint16Arrays() {
    let data: Record<string, Uint16Array> = {
        "uint16Key": new Uint16Array([10, 20, 30, 40, 50])
    };
    let res = typedArrInfo.mapUint16Arrays(data);
    console.log("test_bar_with_mapUint16Arrays result: ", res);
}

function test_bar_with_mapInt16Arrays() {
    let data: Record<string, Int16Array> = {
        "int16Key": new Int16Array([10, -20, 30, -40, 50])
    };
    let res = typedArrInfo.mapInt16Arrays(data);
    console.log("test_bar_with_mapInt16Arrays result: ", res);
}

function test_bar_with_mapUint32Arrays() {
    let data: Record<string, Uint32Array> = {
        "uint32Key": new Uint32Array([100, 200, 300, 400, 500])
    };
    let res = typedArrInfo.mapUint32Arrays(data);
    console.log("test_bar_with_mapUint32Arrays result: ", res);
}

function test_bar_with_mapInt32Arrays() {
    let data: Record<string, Int32Array> = {
        "int32Key": new Int32Array([100, -200, 300, -400, 500])
    };
    let res = typedArrInfo.mapInt32Arrays(data);
    console.log("test_bar_with_mapInt32Arrays result: ", res);
}

function test_bar_with_mapUint64Arrays() {
    let data: Record<string, BigUint64Array> = {
        "uint64Key": new BigUint64Array([1000n, 2000n, 3000n, 4000n, 5000n])
    };
    let res = typedArrInfo.mapUint64Arrays(data);
    console.log("test_bar_with_mapUint64Arrays result: ", res);
}

function test_bar_with_mapInt64Arrays() {
    let data: Record<string, BigInt64Array> = {
        "int64Key": new BigInt64Array([1000n, -2000n, 3000n, -4000n, 5000n])
    };
    let res = typedArrInfo.mapInt64Arrays(data);
    console.log("test_bar_with_mapInt64Arrays result: ", res);
}

function test_bar_with_mapFloat32Arrays() {
    let data: Record<string, Float32Array> = {
        "float32Key": new Float32Array([1.0, 2.0, 3.0, 4.0, 5.0])
    };
    let res = typedArrInfo.mapFloat32Arrays(data);
    console.log("test_bar_with_mapFloat32Arrays result: ", res);
}

function test_bar_with_mapFloat64Arrays() {
    let data: Record<string, Float64Array> = {
        "float64Key": new Float64Array([1.0, 2.0, 3.0, 4.0, 5.0])
    };
    let res = typedArrInfo.mapFloat64Arrays(data);
    console.log("test_bar_with_mapFloat64Arrays result: ", res);
}

function test_bar_with_setupStructAndPrint() {
    let a: bar.TypedArray1 = {
        a: new Uint8Array([1, 2, 3]),
        b: new Int8Array([1, -2, 3]),
        c: new Uint16Array([10, 20, 30]),
        d: new Int16Array([10, -20, 30]),
        e: new Uint32Array([100, 200, 300]),
        f: new Int32Array([100, -200, 300]),
        g: new BigUint64Array([1000n, 2000n, 3000n]),
        h: new BigInt64Array([1000n, -2000n, 3000n]),
        i: new Float32Array([1.0, 2.0, 3.0]),
        j: new Float64Array([1.0, 2.0, 3.0])
    };
    bar.setupStructAndPrint(a);
}

function test_bar_with_union() {
    // union null
    bar.showMyUnion(null);
    let union0 = bar.makeMyUnion(0);
    arktest.assertEQ(union0, null);

    // union u8
    let u8Arr: Uint8Array = new Uint8Array([1, 2, 3, 4, 5]);
    bar.showMyUnion(u8Arr);
    let union1 = bar.makeMyUnion(1);
    let u8Expected: Uint8Array = union1 as Uint8Array;
    arktest.assertEQ(u8Arr.toString(), u8Expected.toString());

    // union i8
    let i8Arr: Int8Array = new Int8Array([1, -2, 3, -4, 5]);
    bar.showMyUnion(i8Arr);
    let union2 = bar.makeMyUnion(2);
    let i8Expected: Int8Array = union2 as Int8Array;
    arktest.assertEQ(i8Arr.toString(), i8Expected.toString());

    // union u16
    let u16Arr: Uint16Array = new Uint16Array([10, 20, 30, 40, 50]);
    bar.showMyUnion(u16Arr);
    let union3 = bar.makeMyUnion(3);
    let u16Expected: Uint16Array = union3 as Uint16Array;
    arktest.assertEQ(u16Arr.toString(), u16Expected.toString());

    // union i16
    let i16Arr: Int16Array = new Int16Array([10, -20, 30, -40, 50]);
    bar.showMyUnion(i16Arr);
    let union4 = bar.makeMyUnion(4);
    let i16Expected: Int16Array = union4 as Int16Array;
    arktest.assertEQ(i16Arr.toString(), i16Expected.toString());

    // union u32
    let u32Arr: Uint32Array = new Uint32Array([100, 200, 300, 400, 500]);
    bar.showMyUnion(u32Arr);
    let union5 = bar.makeMyUnion(5);
    let u32Expected: Uint32Array = union5 as Uint32Array;
    arktest.assertEQ(u32Arr.toString(), u32Expected.toString());

    // union i32
    let i32Arr: Int32Array = new Int32Array([100, -200, 300, -400, 500]);
    bar.showMyUnion(i32Arr);
    let union6 = bar.makeMyUnion(6);
    let i32Expected: Int32Array = union6 as Int32Array;
    arktest.assertEQ(i32Arr.toString(), i32Expected.toString());

    // union u64
    let u64Arr: BigUint64Array =
        new BigUint64Array([1000n, 2000n, 3000n, 4000n, 5000n]);
    bar.showMyUnion(u64Arr);
    let union7 = bar.makeMyUnion(7);
    let u64Expected: BigUint64Array = union7 as BigUint64Array;
    arktest.assertEQ(u64Arr.toString(), u64Expected.toString());

    // union i64
    let i64Arr: BigInt64Array =
        new BigInt64Array([1000n, -2000n, 3000n, -4000n, 5000n]);
    bar.showMyUnion(i64Arr);
    let union8 = bar.makeMyUnion(8);
    let i64Expected: BigInt64Array = union8 as BigInt64Array;
    arktest.assertEQ(i64Arr.toString(), i64Expected.toString());

    // union f32
    let f32Arr: Float32Array = new Float32Array([1.0, 2.0, 3.0, 4.0, 5.0]);
    bar.showMyUnion(f32Arr);
    let union9 = bar.makeMyUnion(9);
    let f32Expected: Float32Array = union9 as Float32Array;
    arktest.assertEQ(f32Arr.toString(), f32Expected.toString());

    // union f64
    let f64Arr: Float64Array = new Float64Array([1.0, 2.0, 3.0, 4.0, 5.0]);
    bar.showMyUnion(f64Arr);
    let union10 = bar.makeMyUnion(10);
    let f64Expected: Float64Array = union10 as Float64Array;
    arktest.assertEQ(f64Arr.toString(), f64Expected.toString());
}

function main() {
    const suite = new arktest.ArkTestsuite("ani_typedarray ut")
    suite.addTest("checkUint8Array", checkUint8Array);
    suite.addTest("checkUint16Array", checkUint16Array);
    suite.addTest("checkUint32Array", checkUint32Array);
    suite.addTest("checkBigUint64Array", checkBigUint64Array);
    suite.addTest("checkInt8Array", checkInt8Array);
    suite.addTest("checkInt16Array", checkInt16Array);
    suite.addTest("checkInt32Array", checkInt32Array);
    suite.addTest("checkBigInt64Array", checkBigInt64Array);
    suite.addTest("checkFloat32Array", checkFloat32Array);
    suite.addTest("checkFloat64Array", checkFloat64Array);

    suite.addTest(
        "test_bar_with_setAndgetUint8Array", test_bar_with_setAndgetUint8Array);
    suite.addTest(
        "test_bar_with_createAndPrintInt8Array",
        test_bar_with_createAndPrintInt8Array);
    suite.addTest(
        "test_bar_with_setAndgetInt8Array", test_bar_with_setAndgetInt8Array);
    suite.addTest(
        "test_bar_with_createAndPrintUint16Array",
        test_bar_with_createAndPrintUint16Array);
    suite.addTest(
        "test_bar_with_setAndgetUint16Array",
        test_bar_with_setAndgetUint16Array);
    suite.addTest(
        "test_bar_with_createAndPrintInt16Array",
        test_bar_with_createAndPrintInt16Array);
    suite.addTest(
        "test_bar_with_setAndgetInt16Array", test_bar_with_setAndgetInt16Array);
    suite.addTest(
        "test_bar_with_createAndPrintUint32Array",
        test_bar_with_createAndPrintUint32Array);
    suite.addTest(
        "test_bar_with_setAndgetUint32Array",
        test_bar_with_setAndgetUint32Array);
    suite.addTest(
        "test_bar_with_createAndPrintInt32Array",
        test_bar_with_createAndPrintInt32Array);
    suite.addTest(
        "test_bar_with_setAndgetInt32Array", test_bar_with_setAndgetInt32Array);
    suite.addTest(
        "test_bar_with_createAndPrintUint64Array",
        test_bar_with_createAndPrintUint64Array);
    suite.addTest(
        "test_bar_with_setAndgetUint64Array",
        test_bar_with_setAndgetUint64Array);
    suite.addTest(
        "test_bar_with_createAndPrintInt64Array",
        test_bar_with_createAndPrintInt64Array);
    suite.addTest(
        "test_bar_with_setAndgetInt64Array", test_bar_with_setAndgetInt64Array);
    suite.addTest(
        "test_bar_with_createAndPrintFloat32Array",
        test_bar_with_createAndPrintFloat32Array);
    suite.addTest(
        "test_bar_with_setAndgetFloat32Array",
        test_bar_with_setAndgetFloat32Array);
    suite.addTest(
        "test_bar_with_createAndPrintFloat64Array",
        test_bar_with_createAndPrintFloat64Array);
    suite.addTest(
        "test_bar_with_setAndgetFloat64Array",
        test_bar_with_setAndgetFloat64Array);
    suite.addTest(
        "test_bar_with_convertToInt64Array", test_bar_with_convertToInt64Array);
    suite.addTest(
        "test_bar_with_convertToFloat64Array",
        test_bar_with_convertToFloat64Array);
    suite.addTest(
        "test_bar_with_getUint8ArrayOptional_1",
        test_bar_with_getUint8ArrayOptional_1);
    suite.addTest(
        "test_bar_with_getUint8ArrayOptional_2",
        test_bar_with_getUint8ArrayOptional_2);
    suite.addTest(
        "test_bar_with_getInt8ArrayOptional_1",
        test_bar_with_getInt8ArrayOptional_1);
    suite.addTest(
        "test_bar_with_getInt8ArrayOptional_2",
        test_bar_with_getInt8ArrayOptional_2);
    suite.addTest(
        "test_bar_with_getUint16ArrayOptional_1",
        test_bar_with_getUint16ArrayOptional_1);
    suite.addTest(
        "test_bar_with_getUint16ArrayOptional_2",
        test_bar_with_getUint16ArrayOptional_2);
    suite.addTest(
        "test_bar_with_getInt16ArrayOptional_1",
        test_bar_with_getInt16ArrayOptional_1);
    suite.addTest(
        "test_bar_with_getInt16ArrayOptional_2",
        test_bar_with_getInt16ArrayOptional_2);
    suite.addTest(
        "test_bar_with_getUint32ArrayOptional_1",
        test_bar_with_getUint32ArrayOptional_1);
    suite.addTest(
        "test_bar_with_getUint32ArrayOptional_2",
        test_bar_with_getUint32ArrayOptional_2);
    suite.addTest(
        "test_bar_with_getInt32ArrayOptional_1",
        test_bar_with_getInt32ArrayOptional_1);
    suite.addTest(
        "test_bar_with_getInt32ArrayOptional_2",
        test_bar_with_getInt32ArrayOptional_2);
    suite.addTest(
        "test_bar_with_getUint64ArrayOptional_1",
        test_bar_with_getUint64ArrayOptional_1);
    suite.addTest(
        "test_bar_with_getUint64ArrayOptional_2",
        test_bar_with_getUint64ArrayOptional_2);
    suite.addTest(
        "test_bar_with_getInt64ArrayOptional_1",
        test_bar_with_getInt64ArrayOptional_1);
    suite.addTest(
        "test_bar_with_getInt64ArrayOptional_2",
        test_bar_with_getInt64ArrayOptional_2);
    suite.addTest(
        "test_bar_with_getFloat32ArrayOptional_1",
        test_bar_with_getFloat32ArrayOptional_1);
    suite.addTest(
        "test_bar_with_getFloat32ArrayOptional_2",
        test_bar_with_getFloat32ArrayOptional_2);
    suite.addTest(
        "test_bar_with_getFloat64ArrayOptional_1",
        test_bar_with_getFloat64ArrayOptional_1);
    suite.addTest(
        "test_bar_with_getFloat64ArrayOptional_2",
        test_bar_with_getFloat64ArrayOptional_2);

    suite.addTest("test_bar_with_mapUint8Arrays", test_bar_with_mapUint8Arrays);
    suite.addTest("test_bar_with_mapInt8Arrays", test_bar_with_mapInt8Arrays);
    suite.addTest(
        "test_bar_with_mapUint16Arrays", test_bar_with_mapUint16Arrays);
    suite.addTest("test_bar_with_mapInt16Arrays", test_bar_with_mapInt16Arrays);
    suite.addTest(
        "test_bar_with_mapUint32Arrays", test_bar_with_mapUint32Arrays);
    suite.addTest("test_bar_with_mapInt32Arrays", test_bar_with_mapInt32Arrays);
    suite.addTest(
        "test_bar_with_mapUint64Arrays", test_bar_with_mapUint64Arrays);
    suite.addTest("test_bar_with_mapInt64Arrays", test_bar_with_mapInt64Arrays);
    suite.addTest(
        "test_bar_with_mapFloat32Arrays", test_bar_with_mapFloat32Arrays);
    suite.addTest(
        "test_bar_with_mapFloat64Arrays", test_bar_with_mapFloat64Arrays);

    suite.addTest(
        "test_bar_with_setupStructAndPrint", test_bar_with_setupStructAndPrint);
    suite.addTest("test_bar_with_union", test_bar_with_union);

    exit(suite.run());
}