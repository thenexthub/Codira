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
import * as arraybuffer from "arraybuffer";

loadLibrary("ani_arraybuffer");

function arraybuffer_input() {
    // case 1 : test sumArrayu8，The input parameter is ArrayBuffer, and the
    // output parameter is u8
    let numbersU8: byte[] = [1, 2, 3, 4, 5];
    let arrbuf1: ArrayBuffer = new ArrayBuffer(numbersU8.length);
    for (let i = 0; i < numbersU8.length; i++) {
        arrbuf1.set(i, numbersU8[i]);
    }
    let sumArrayu8 = arraybuffer.SumArrayU8(arrbuf1)
    console.log("sumArrayu8 is " + sumArrayu8)
    arktest.assertEQ(sumArrayu8, 15)
}

function arraybuffer_return() {
    // case 2 : test getArrayBuffer，The input parameter is u8, and the output
    // parameter is ArrayBuffer
    let getArrayBuffer = arraybuffer.GetArrayBuffer(4 as byte)
    console.log("getArrayBuffer length: " + getArrayBuffer.getByteLength());
    arktest.assertEQ(getArrayBuffer.getByteLength(), 4)
    for (let i = 0; i < getArrayBuffer.getByteLength(); i++) {
        console.log(
            `getArrayBuffer Byte at index ${i}: ${getArrayBuffer.at(i)}`);
        arktest.assertEQ(getArrayBuffer.at(i), 4)
    }
}

function arraybuffer_inout() {
    // case 3 : test doubleArrayBuffer，The input parameter is ArrayBuffer, and
    // the output parameter is ArrayBuffer
    let numbers2: byte[] = [3, 4, 5, 6, 7];
    let arrbuf3: ArrayBuffer = new ArrayBuffer(numbers2.length);
    for (let i = 0; i < numbers2.length; i++) {
        arrbuf3.set(i, numbers2[i]);
    }
    let doubleArrayBuffer = arraybuffer.DoubleArrayBuffer(arrbuf3)
    console.log(
        "doubleArrayBuffer length: " + doubleArrayBuffer.getByteLength());
    arktest.assertEQ(doubleArrayBuffer.getByteLength(), 5)

    for (let i = 0; i < doubleArrayBuffer.getByteLength(); i++) {
        console.log(
            `doubleArrayBuffer Byte at index ${i}: ${doubleArrayBuffer.at(i)}`);
        arktest.assertEQ(doubleArrayBuffer.at(i), numbers2[i] * 2)
    }
}

function arraybuffer_uint8toarraybuffer() {
    // case 4 : test Uint8Array to ArrayBuffer conversion
    let uint8Array = new Uint8Array([2, 3, 4, 5, 6]);
    let arrbuf4: ArrayBuffer =
        uint8Array.buffer as ArrayBuffer;  // Convert Uint8Array to ArrayBuffer
    // Verify ArrayBuffer length
    console.log("arrbuf4 length: " + arrbuf4.getByteLength());
    arktest.assertEQ(arrbuf4.getByteLength(), uint8Array.length);
    // Verify ArrayBuffer content
    for (let i = 0; i < arrbuf4.getByteLength(); i++) {
        console.log(`arrbuf4 Byte at index ${i}: ${arrbuf4.at(i)}`);
        arktest.assertEQ(arrbuf4.at(i), uint8Array[i]);
    }
    // Optionally, pass the ArrayBuffer to a function for further validation
    let sumArrayu8FromUint8Array = arraybuffer.SumArrayU8(arrbuf4);
    console.log("sumArrayu8FromUint8Array is " + sumArrayu8FromUint8Array);
    arktest.assertEQ(sumArrayu8FromUint8Array, 20);  // 2 + 3 + 4 + 5 + 6 = 20
}

function arraybuffer_dataview() {
    // case 6 : test Uint8Array to ArrayBuffer conversion, use DataView
    let uint8ArrayData: Uint8Array = new Uint8Array([8, 9, 10]);
    let arrayBufferFromUint8Array: ArrayBuffer =
        new ArrayBuffer(uint8ArrayData.length);
    let view = new DataView(arrayBufferFromUint8Array);
    for (let i = 0; i < uint8ArrayData.length; i++) {
        view.setUint8(i, uint8ArrayData[i]);
    }

    for (let i = 0; i < uint8ArrayData.length; i++) {
        let value = view.getUint8(i);
        console.log(`Value at index ${i} in ArrayBuffer: ${value}`);
        arktest.assertEQ(value, uint8ArrayData[i]);
    }
    // Optionally, pass the ArrayBuffer to a function for further validation
    let sumArrayBufferFromUint8Array =
        arraybuffer.SumArrayU8(arrayBufferFromUint8Array);
    console.log(
        "sumArrayBufferFromUint8Array is " + sumArrayBufferFromUint8Array);
    arktest.assertEQ(sumArrayBufferFromUint8Array, 27);  // 8 + 9 + 10 = 27
}

function arrayi8_input() {
    let int8Array: byte[] = [1, 2, 3, 4, 5];
    let sumArrayi8 = arraybuffer.SumArrayI8(int8Array)
    console.log("sumArrayi8 is " + sumArrayi8)
    arktest.assertEQ(sumArrayi8, 15)
}

function arrayi8_return() {
    let arrayi8 = arraybuffer.GetArrayI8(4 as byte)
    console.log("getArrayi8 length: " + arrayi8.length);
    arktest.assertEQ(arrayi8.length, 4)
    for (let i = 0; i < arrayi8.length; i++) {
        console.log(`getArrayi8 Byte at index ${i}: ${arrayi8[i]}`);
        arktest.assertEQ(arrayi8[i], 4)
    }
}

function arrayi8_inout() {
    let numbers2: byte[] = [3, 4, 5, 6, 7];
    let doubleArrayBuffer = arraybuffer.DoubleI8(numbers2)
    console.log("arrayi8_inout length: " + doubleArrayBuffer.length);
    arktest.assertEQ(doubleArrayBuffer.length, 5)

    for (let i = 0; i < doubleArrayBuffer.length; i++) {
        console.log(
            `arrayi8_inout Byte at index ${i}: ${doubleArrayBuffer[i]}`);
        arktest.assertEQ(doubleArrayBuffer[i], numbers2[i] * 2)
    }
}

function arrayi16_input() {
    let int16Array: short[] = [1, 2, 3, 4, 5];
    let sumArrayi16 = arraybuffer.SumArrayI16(int16Array)
    console.log("sumArrayi16 is " + sumArrayi16)
    arktest.assertEQ(sumArrayi16, 15)
}

function arrayi16_return() {
    let arrayi16 = arraybuffer.GetArrayI16(4 as short)
    console.log("arrayi16 length: " + arrayi16.length);
    arktest.assertEQ(arrayi16.length, 4)
    for (let i = 0; i < arrayi16.length; i++) {
        console.log(`arrayi16 Short at index ${i}: ${arrayi16[i]}`);
        arktest.assertEQ(arrayi16[i], 4)
    }
}

function arrayi16_inout() {
    let numbers2: short[] = [3, 4, 5, 6, 7];
    let doubleArrayBuffer = arraybuffer.DoubleI16(numbers2)
    console.log("arrayi16_inout length: " + doubleArrayBuffer.length);
    arktest.assertEQ(doubleArrayBuffer.length, 5)

    for (let i = 0; i < doubleArrayBuffer.length; i++) {
        console.log(
            `arrayi16_inout Short at index ${i}: ${doubleArrayBuffer[i]}`);
        arktest.assertEQ(doubleArrayBuffer[i], numbers2[i] * 2)
    }
}

function arrayi32_input() {
    let int32Array: int[] = [1, 2, 3, 4, 5];
    let sumArrayi32 = arraybuffer.SumArrayI32(int32Array)
    console.log("sumArrayi32 is " + sumArrayi32)
    arktest.assertEQ(sumArrayi32, 15)
}

function arrayi32_return() {
    let arrayi32 = arraybuffer.GetArrayI32(4)
    console.log("arrayi32 length: " + arrayi32.length);
    arktest.assertEQ(arrayi32.length, 4)
    for (let i = 0; i < arrayi32.length; i++) {
        console.log(`arrayi32 int at index ${i}: ${arrayi32[i]}`);
        arktest.assertEQ(arrayi32[i], 4)
    }
}

function arrayi32_inout() {
    let numbers2: int[] = [3, 4, 5, 6, 7];
    let doubleArrayBuffer = arraybuffer.DoubleI32(numbers2)
    console.log("arrayi16_inout length: " + doubleArrayBuffer.length);
    arktest.assertEQ(doubleArrayBuffer.length, 5)

    for (let i = 0; i < doubleArrayBuffer.length; i++) {
        console.log(
            `arrayi16_inout int at index ${i}: ${doubleArrayBuffer[i]}`);
        arktest.assertEQ(doubleArrayBuffer[i], numbers2[i] * 2)
    }
}

function arrayi64_input() {
    let int64Array: long[] = [10, 20, 30, 40, 50];
    let sumArrayi64 = arraybuffer.SumArrayI64(int64Array)
    console.log("sumArrayi64 is " + sumArrayi64)
    arktest.assertEQ(sumArrayi64, 150)
}

function arrayi64_return() {
    let arrayi64 = arraybuffer.GetArrayI64(4)
    console.log("arrayi64 length: " + arrayi64.length);
    arktest.assertEQ(arrayi64.length, 4)
    for (let i = 0; i < arrayi64.length; i++) {
        console.log(`arrayi64 int at index ${i}: ${arrayi64[i]}`);
        arktest.assertEQ(arrayi64[i], 4)
    }
}

function arrayi64_inout() {
    let numbers2: long[] = [30, 40, 50, 60, 70];
    let doubleArrayBuffer = arraybuffer.DoubleI64(numbers2)
    console.log("arrayi16_inout length: " + doubleArrayBuffer.length);
    arktest.assertEQ(doubleArrayBuffer.length, 5)

    for (let i = 0; i < doubleArrayBuffer.length; i++) {
        console.log(
            `arrayi16_inout long at index ${i}: ${doubleArrayBuffer[i]}`);
        arktest.assertEQ(doubleArrayBuffer[i], numbers2[i] * 2)
    }
}

function arrayf32_input() {
    let f32Array: float[] = [10, 20, 30, 40, 50];
    let sumArrayf32 = arraybuffer.SumArrayF32(f32Array)
    console.log("sumArrayf32 is " + sumArrayf32)
    arktest.assertEQ(sumArrayf32, 150)
}

function arrayf32_return() {
    let arrayf32 = arraybuffer.GetArrayF32(4)
    console.log("arrayf32 length: " + arrayf32.length);
    arktest.assertEQ(arrayf32.length, 4)
    for (let i = 0; i < arrayf32.length; i++) {
        console.log(`arrayf32 float at index ${i}: ${arrayf32[i]}`);
        arktest.assertEQ(arrayf32[i], 4)
    }
}

function arrayf32_inout() {
    let numbers2: float[] = [30, 40, 50, 60, 70];
    let doubleArrayBuffer = arraybuffer.DoubleF32(numbers2)
    console.log("arrayi16_inout length: " + doubleArrayBuffer.length);
    arktest.assertEQ(doubleArrayBuffer.length, 5)

    for (let i = 0; i < doubleArrayBuffer.length; i++) {
        console.log(
            `arrayi16_inout long at index ${i}: ${doubleArrayBuffer[i]}`);
        arktest.assertEQ(doubleArrayBuffer[i], numbers2[i] * 2)
    }
}

function arrayf64_input() {
    let f64Array: double[] = [10, 20, 30, 40, 50];
    let sumArrayf64 = arraybuffer.SumArrayF64(f64Array)
    console.log("sumArrayf64 is " + sumArrayf64)
    arktest.assertEQ(sumArrayf64, 150)
}

function arrayf64_return() {
    let arrayf64 = arraybuffer.GetArrayF64(4)
    console.log("arrayf64 length: " + arrayf64.length);
    arktest.assertEQ(arrayf64.length, 4)
    for (let i = 0; i < arrayf64.length; i++) {
        console.log(`arrayf64 double at index ${i}: ${arrayf64[i]}`);
        arktest.assertEQ(arrayf64[i], 4)
    }
}

function arrayf64_inout() {
    let numbers2: double[] = [30, 40, 50, 60, 70];
    let doubleArrayBuffer = arraybuffer.DoubleF64(numbers2)
    console.log("arrayi16_inout length: " + doubleArrayBuffer.length);
    arktest.assertEQ(doubleArrayBuffer.length, 5)

    for (let i = 0; i < doubleArrayBuffer.length; i++) {
        console.log(
            `arrayi16_inout double at index ${i}: ${doubleArrayBuffer[i]}`);
        arktest.assertEQ(doubleArrayBuffer[i], numbers2[i] * 2)
    }
}

function int8Array_inout() {
    let buf: Int8Array = new Int8Array([10, 20, 30, 60]);
    let ret = arraybuffer.doubleInt8Array(buf);
    for (let i = 0; i < ret.length; i++) {
        console.log(`Int8Array_inout->  index ${i}: ${ret[i]}`);
        arktest.assertEQ(ret[i], buf[i] * 2)
    }
}

function int16Array_inout() {
    let buf: Int16Array = new Int16Array([1000, 2000, 3000, 4000]);
    let ret = arraybuffer.doubleInt16Array(buf);
    for (let i = 0; i < ret.length; i++) {
        console.log(`int16Array_inout->  index ${i}: ${ret[i]}`);
        arktest.assertEQ(ret[i], buf[i] * 2)
    }
}

function int32Array_inout() {
    let buf: Int32Array = new Int32Array([100000, 200000, 300000, 400000]);
    let ret = arraybuffer.doubleInt32Array(buf);
    for (let i = 0; i < ret.length; i++) {
        console.log(`Int32Array_inout->  index ${i}: ${ret[i]}`);
        arktest.assertEQ(ret[i], buf[i] * 2)
    }
}

function uint8Array_inout() {
    let buf: Uint8Array = new Uint8Array([10, 20, 30, 40]);
    let ret = arraybuffer.doubleUint8Array(buf);
    for (let i = 0; i < ret.length; i++) {
        console.log(`uint8Array_inout->  index ${i}: ${ret[i]}`);
        arktest.assertEQ(ret[i], buf[i] * 2)
    }
}

function uint16Array_inout() {
    let buf: Uint16Array = new Uint16Array([1000, 2000, 3000, 4000]);
    let ret = arraybuffer.doubleUint16Array(buf);
    for (let i = 0; i < ret.length; i++) {
        console.log(`uint16Array_inout->  index ${i}: ${ret[i]}`);
        arktest.assertEQ(ret[i], buf[i] * 2)
    }
}

function uint32Array_inout() {
    let buf: Uint32Array = new Uint32Array([100000, 200000, 300000, 400000]);
    let ret = arraybuffer.doubleUint32Array(buf);
    for (let i = 0; i < ret.length; i++) {
        console.log(`uint32Array_inout->  index ${i}: ${ret[i]}`);
        arktest.assertEQ(ret[i], buf[i] * 2)
    }
}

function main() {
    const suite = new arktest.ArkTestsuite("arraybuffer testcase")

    suite.addTest("arraybuffer_input", arraybuffer_input)
    suite.addTest("arraybuffer_return", arraybuffer_return)
    suite.addTest("arraybuffer_inout", arraybuffer_inout)
    suite.addTest(
        "arraybuffer_uint8toarraybuffer", arraybuffer_uint8toarraybuffer)
    suite.addTest("arraybuffer_dataview", arraybuffer_dataview)

    suite.addTest("arrayi8_input", arrayi8_input)
    suite.addTest("arrayi8_return", arrayi8_return)
    suite.addTest("arrayi8_return", arrayi8_inout)

    suite.addTest("arrayi16_input", arrayi16_input)
    suite.addTest("arrayi16_return", arrayi16_return)
    suite.addTest("arrayi16_inout", arrayi16_inout)

    suite.addTest("arrayi32_input", arrayi32_input)
    suite.addTest("arrayi32_return", arrayi32_return)
    suite.addTest("arrayi32_inout", arrayi32_inout)

    suite.addTest("arrayi64_input", arrayi64_input)
    suite.addTest("arrayi64_return", arrayi64_return)
    suite.addTest("arrayi64_inout", arrayi64_inout)

    suite.addTest("arrayf32_input", arrayf32_input)
    suite.addTest("arrayf32_return", arrayf32_return)
    suite.addTest("arrayf32_inout", arrayf32_inout)

    suite.addTest("arrayf64_input", arrayf64_input)
    suite.addTest("arrayf64_return", arrayf64_return)
    suite.addTest("arrayf64_inout", arrayf64_inout)

    suite.addTest("int8Array_inout", int8Array_inout)
    suite.addTest("int16Array_inout", int16Array_inout)
    suite.addTest("int32Array_inout", int32Array_inout)

    suite.addTest("uint8Array_inout", uint8Array_inout)
    suite.addTest("uint16Array_inout", uint16Array_inout)
    suite.addTest("uint32Array_inout", uint32Array_inout)
    exit(suite.run())
}
