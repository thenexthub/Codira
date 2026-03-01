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

function checkArrayForInt(data: BigInt64Array, elem: int, expected: number): void {
    arktest.assertEQ(data.indexOf(elem), expected, 'data.indexOf(elem: int) must return ' + expected);
}

function checkArray(data: BigInt64Array, elem: BigInt, expected: number, fromIndex?: int): void {
    arktest.assertEQ(data.indexOf(elem, fromIndex), expected,
        'data.indexOf(elem: BigInt, fromIndex?: int) must return ' + expected);
}

function checkArrayForInt(data: BigUint64Array, elem: int, expected: number): void {
    arktest.assertEQ(data.indexOf(elem), expected, 'data.indexOf(elem: int) must return ' + expected);
}

function checkArray(data: BigUint64Array, elem: BigInt, expected: number, fromIndex?: int): void {
    arktest.assertEQ(data.indexOf(elem, fromIndex), expected,
        'data.includes(elem: BigInt, fromIndex?: int) must return ' + expected);
}

function checkArrayForInt(data: Int32Array, elem: int, expected: number): void {
    arktest.assertEQ(data.indexOf(elem), expected, 'data.indexOf(elem: int) must return ' + expected);
}

function checkArray(data: Int32Array, elem: number, expected: number, fromIndex?: int): void {
    arktest.assertEQ(data.indexOf(elem, fromIndex), expected,
        'data.indexOf(elem: number, fromIndex?: int) must return ' + expected);
}

function checkArrayForInt(data: Uint32Array, elem: int, expected: number): void {
    arktest.assertEQ(data.indexOf(elem), expected, 'data.indexOf(elem: int) must return ' + expected);
}

function checkArray(data: Uint32Array, elem: number, expected: number, fromIndex?: int): void {
    arktest.assertEQ(data.indexOf(elem, fromIndex), expected,
        'data.indexOf(elem: number, fromIndex?: int) must return ' + expected);
}

function checkArrayForInt(data: Int16Array, elem: int, expected: number): void {
    arktest.assertEQ(data.indexOf(elem), expected, 'data.indexOf(elem: int) must return ' + expected);
}

function checkArray(data: Int16Array, elem: number, expected: number, fromIndex?: int): void {
    arktest.assertEQ(data.indexOf(elem, fromIndex), expected,
        'data.indexOf(elem: number, fromIndex?: int) must return ' + expected);
}

function checkArrayForInt(data: Uint16Array, elem: int, expected: number): void {
    arktest.assertEQ(data.indexOf(elem), expected, 'data.indexOf(elem: int) must return ' + expected);
}

function checkArray(data: Uint16Array, elem: number, expected: number, fromIndex?: int): void {
    arktest.assertEQ(data.indexOf(elem, fromIndex), expected,
        'data.indexOf(elem: number, fromIndex?: int) must return ' + expected);
}

function checkArrayForInt(data: Int8Array, elem: int, expected: number, fromIndex: int): void {
    arktest.assertEQ(data.indexOf(elem, fromIndex), expected,
        'data.indexOf(elem: int, fromIndex: int) must return ' + expected);
}

function checkArrayForInt(data: Int8Array, elem: int, expected: number): void {
    arktest.assertEQ(data.indexOf(elem), expected, 'data.indexOf(elem: int) must return ' + expected);
}

function checkArray(data: Int8Array, elem: number, expected: number, fromIndex?: int): void {
    arktest.assertEQ(data.indexOf(elem, fromIndex), expected,
        'data.indexOf(elem: number, fromIndex?: int) must return ' + expected);
}

function checkArrayForInt(data: Uint8Array, elem: int, expected: number): void {
    arktest.assertEQ(data.indexOf(elem), expected, 'data.indexOf(elem: int) must return ' + expected);
}

function checkArrayForInt(data: Uint8Array, elem: int, expected: number, fromIndex: int): void {
    arktest.assertEQ(data.indexOf(elem, fromIndex), expected, 'data.indexOf(elem: int, fromIndex: int) must return ' +
        expected);
}

function checkArray(data: Uint8Array, elem: number, expected: number, fromIndex?: int): void {
    arktest.assertEQ(data.indexOf(elem, fromIndex), expected,
        'data.indexOf(elem: number, fromIndex?: int) must return ' + expected);
}

function checkArrayForInt(data: Float32Array, elem: int, expected: number): void {
    arktest.assertEQ(data.indexOf(elem), expected, 'data.indexOf(elem: int) must return ' + expected);
}

function checkArrayForInt(data: Float32Array, elem: int, expected: number, fromIndex: int): void {
    arktest.assertEQ(data.indexOf(elem, fromIndex), expected,
        'data.indexOf(elem: int, fromIndex: int) must return ' + expected);
}

function checkArray(data: Float32Array, elem: number, expected: number, fromIndex?: int): void {
    arktest.assertEQ(data.indexOf((elem.toFloat()).toDouble(), fromIndex), expected,
        'data.indexOf(elem: number, fromIndex?: int) must return ' + expected);
}

function checkArrayExactlyNumber(data: Float32Array, elem: number, expected: number): void {
    arktest.assertEQ(data.indexOf(elem), expected, 'data.indexOf(elem: number) must return ' + expected);
}

function checkArrayForInt(data: Float64Array, elem: int, expected: number, fromIndex: int): void {
    arktest.assertEQ(data.indexOf(elem, fromIndex), expected, 'data.indexOf(elem: int, fromIndex: int) must return ' +
        expected);
}

function checkArrayForInt(data: Float64Array, elem: int, expected: number): void {
    arktest.assertEQ(data.indexOf(elem), expected, 'data.indexOf(elem: int) must return ' + expected);
}

function checkArray(data: Float64Array, elem: number, expected: number, fromIndex?: int): void {
    arktest.assertEQ(data.indexOf(elem, fromIndex), expected,
        'data.indexOf(elem: number, fromIndex?: int) must return ' + expected);
}

function testIU8Array() {
    let data = Int8Array.of(-128, -128, -42, -14, -14, 0, 16, 48, 127);
    checkArray(data, NaN, -1);
    checkArray(data, -100.0, -1);
    checkArrayForInt(data, -100, -1);
    checkArray(data, -128.0, 0);
    checkArrayForInt(data, -128, 0);
    checkArray(data, -42.0, 2);
    checkArrayForInt(data, -42, 2);
    checkArray(data, -14.0, 3);
    checkArrayForInt(data, -14, 3);
    checkArray(data, 0.0, 5);
    checkArrayForInt(data, 0, 5);
    checkArray(data, 1.0, -1);
    checkArrayForInt(data, 1, -1);
    checkArray(data, 16.0, 6);
    checkArrayForInt(data, 16, 6);
    checkArray(data, 48.0, 7);
    checkArrayForInt(data, 48, 7);
    checkArray(data, 127.0, 8);
    checkArrayForInt(data, 127, 8);
    checkArray(data, 128.0, -1);
    checkArrayForInt(data, 128, 0); // 128 as byte in the array == -128

    let udata = Uint8Array.of(0, 16, 48, 127, 16, 160, 255, 160);
    checkArray(udata, NaN, -1);
    checkArray(udata, 1.0, -1);
    checkArrayForInt(udata, 1, -1);
    checkArray(udata, 0.0, 0);
    checkArrayForInt(udata, 0, 0);
    checkArray(udata, 16.0, 1);
    checkArrayForInt(udata, 16, 1);
    checkArray(udata, 48.0, 2);
    checkArrayForInt(udata, 48, 2);
    checkArray(udata, 127.0, 3);
    checkArrayForInt(udata, 127, 3);
    checkArray(udata, 128.0, -1);
    checkArrayForInt(udata, 128, -1);
    checkArray(udata, 160.0, 5);
    checkArrayForInt(udata, 160, 5);
    checkArray(udata, 255.0, 6);
    checkArrayForInt(udata, 255, 6);
    checkArray(udata, 256.0, -1);
    checkArrayForInt(udata, 256, 0);
}

function testIU8ArrayFromIndex() {
    let data = Int8Array.of(-128, -128, -42, -14, -14, 0, 16, 48, 127);
    checkArray(data, NaN, -1, 0);
    checkArray(data, -128.0, 0, 0);
    checkArray(data, -128.0, 0, -data.length);
    checkArrayForInt(data, -128, 0, -data.length);
    checkArray(data, -128.0, 1, -(data.length - 1));
    checkArrayForInt(data, -128, 1, -(data.length - 1));
    checkArray(data, -42.0, 2, 0);
    checkArray(data, -42.0, 2, 2);
    checkArrayForInt(data, -42, 2, 2);
    checkArray(data, -42.0, -1, 3);
    checkArrayForInt(data, -42, -1, 3);
    checkArray(data, -14.0, 3, 0);
    checkArray(data, -14.0, 3, -data.length);
    checkArrayForInt(data, -14, 3, -data.length);
    checkArray(data, -14.0, 3, -(data.length - 3));
    checkArrayForInt(data, -14, 3, -(data.length - 3));
    checkArray(data, -14.0, 4, -(data.length - 4));
    checkArrayForInt(data, -14, 4, -(data.length - 4));
    checkArray(data, 128.0, -1, 0);
    checkArrayForInt(data, 128, 0, 0); // 128 as byte in the array == -128
    checkArrayForInt(data, 128, 1, 1);
    checkArrayForInt(data, 128, -1, 2);

    let udata = Uint8Array.of(0, 16, 48, 127, 16, 160, 255, 160);
    checkArray(udata, NaN, -1, 0);
    checkArray(udata, 0.0, 0, 0);
    checkArrayForInt(udata, 0, 0, 0);
    checkArray(udata, 0.0, -1, 1);
    checkArrayForInt(udata, 0, -1, 1);
    checkArray(udata, 0.0, -1, 1);
    checkArrayForInt(udata, 0, -1, 1);
    checkArray(udata, 0, 0, -udata.length);
    checkArrayForInt(udata, 0, 0, -udata.length);
    checkArray(udata, 0.0, 0, -(udata.length + 1));
    checkArrayForInt(udata, 0, 0, -(udata.length + 1));
    checkArray(udata, 0.0, -1, -(udata.length - 1));
    checkArrayForInt(udata, 0, -1, -(udata.length - 1));
    checkArray(udata, 16.0, 1, -(udata.length - 1));
    checkArrayForInt(udata, 16, 1, -(udata.length - 1));
    checkArray(udata, 16.0, 1, 1);
    checkArrayForInt(udata, 16, 1, 1);
    checkArray(udata, 16.0, 4, -(udata.length - 2));
    checkArrayForInt(udata, 16, 4, -(udata.length - 2));
    checkArray(udata, 160.0, 5, -udata.length);
    checkArrayForInt(udata, 160, 5, -udata.length);
    checkArray(udata, 160.0, 5, -(udata.length - 5));
    checkArray(udata, 160.0, -1, udata.length);
    checkArrayForInt(udata, 160, -1, udata.length);
    checkArray(udata, 160.0, 7, udata.length - 1);
    checkArrayForInt(udata, 160, 7, udata.length - 1);
    checkArray(udata, 160.0, 7, -(udata.length - 6));
    checkArrayForInt(udata, 160, 7, -(udata.length - 6));
    checkArray(udata, 160.0, 7, -(udata.length - 7));
    checkArrayForInt(udata, 160, 7, -(udata.length - 7));
}

function testIU16Array() {
    let data = Int16Array.of(-32768, -2731, -227, -227, 0, 128, 0, 2564, 32767, 0);
    checkArray(data, NaN, -1);
    checkArray(data, -32767.0, -1);
    checkArrayForInt(data, -32767, -1);
    checkArray(data, -32768.0, 0);
    checkArrayForInt(data, -32768, 0);
    checkArray(data, -2731.0, 1);
    checkArrayForInt(data, -2731, 1);
    checkArray(data, -227.0, 2);
    checkArrayForInt(data, -227, 2);
    checkArray(data, 0.0, 4);
    checkArrayForInt(data, 0, 4);
    checkArray(data, 128.0, 5);
    checkArrayForInt(data, 128, 5);
    checkArray(data, 2564.0, 7);
    checkArrayForInt(data, 2564, 7);
    checkArray(data, 32766.0, -1);
    checkArrayForInt(data, 32766, -1);
    checkArray(data, 32767.0, 8);
    checkArrayForInt(data, 32767, 8);
    checkArray(data, 32768.0, -1);
    checkArrayForInt(data, 32768, 0);

    let udata = Uint16Array.of(0, 0, 0, 0, 128, 2564, 32767, 65535, 65535);
    checkArray(udata, NaN, -1);
    checkArray(udata, 0.0, 0);
    checkArrayForInt(udata, 0, 0);
    checkArray(udata, 1.0, -1);
    checkArrayForInt(udata, 1, -1);
    checkArray(udata, 128.0, 4);
    checkArrayForInt(udata, 128, 4);
    checkArray(udata, 2564.0, 5);
    checkArrayForInt(udata, 2564, 5);
    checkArray(udata, 32767.0, 6);
    checkArrayForInt(udata, 32767, 6);
    checkArray(udata, 32768.0, -1);
    checkArrayForInt(udata, 32768, -1);
    checkArray(udata, 65534.0, -1);
    checkArrayForInt(udata, 65534, -1);
    checkArray(udata, 65535.0, 7);
    checkArrayForInt(udata, 65535, 7);
    checkArray(udata, 65536.0, -1);
    checkArrayForInt(udata, 65536, 0);
}

function testIU32Array() {
    let data = Int32Array.of(-2147483648, -1398, 0, 0, 11184, 89478, 536872, 0, 2147483647)
    checkArray(data, NaN, -1);
    checkArray(data, -2147483648.0, 0);
    checkArrayForInt(data, -2147483648, 0);
    checkArray(data, -2147483647.0, -1);
    checkArrayForInt(data, -2147483647, -1);
    checkArray(data, -1398.0, 1);
    checkArrayForInt(data, -1398, 1);
    checkArray(data, 0.0, 2);
    checkArrayForInt(data, 0, 2);
    checkArray(data, 11184.0, 4);
    checkArrayForInt(data, 11184, 4);
    checkArray(data, 89478.0, 5);
    checkArrayForInt(data, 89478, 5);
    checkArray(data, 536872.0, 6);
    checkArrayForInt(data, 536872, 6);
    checkArray(data, 2147483646.0, -1);
    checkArrayForInt(data, 2147483646, -1);
    checkArray(data, 2147483647.0, 8);
    checkArrayForInt(data, 2147483647, 8);
    checkArray(data, 2147483648.0, -1);
    checkArrayForInt(data, Long.toInt(2147483648), 0); // 2147483648 as int == -2147483648
    checkArray(data, Long.toInt(2147483648).toDouble(), 0); // 2147483648 as int == -2147483648
    checkArrayForInt(data, Double.toInt(2147483648), 8); // Double(2147483648) == 2147483647

    let udata = Uint32Array.of(0, 11184, 89478, 89478, 536872, 89478, 2147483647)
    checkArray(udata, NaN, -1);
    checkArray(udata, 0.0, 0);
    checkArrayForInt(udata, 0, 0);
    checkArray(udata, 1.0, -1);
    checkArrayForInt(udata, 1, -1);
    checkArray(udata, 11184.0, 1);
    checkArrayForInt(udata, 11184, 1);
    checkArray(udata, 89478.0, 2);
    checkArrayForInt(udata, 89478, 2);
    checkArray(udata, 536872.0, 4);
    checkArrayForInt(udata, 536872, 4);
    checkArray(udata, 2147483646.0, -1);
    checkArrayForInt(udata, 2147483646, -1);
    checkArray(udata, 2147483647.0, 6);
    checkArrayForInt(udata, 2147483647, 6);
    checkArray(udata, 2147483648.0, -1);
    checkArrayForInt(udata, Long.toInt(2147483648), -1);
}

function testIU64Array() {
    let data = BigInt64Array.of(-9.223e+18, -9.223e+18, -9.223e+18, -8589934588, -2147483648, 0, 1802338304,
        4294967294, 1802338304, 2.3058e+16, 9.223e+18);
    checkArrayForInt(data, Long.toInt(-10000000000), -1);
    checkArray(data, BigInt(-10000000000), -1);
    checkArrayForInt(data, -10000, -1);
    checkArray(data, BigInt(-10000), -1);
    checkArrayForInt(data, Double.toInt(-9.224e+18), 4);
    checkArray(data, BigInt(-9.224e+18), -1);
    checkArrayForInt(data, Double.toInt(-9.223e+18), 4);
    checkArray(data, BigInt(-9.223e+18), 0);
    checkArrayForInt(data, Long.toInt(-8589934588), -1);
    checkArray(data, BigInt(-8589934588), 3);
    checkArrayForInt(data, 0, 5);
    checkArray(data, BigInt(0), 5);
    checkArrayForInt(data, Long.toInt(1802338304), 6);
    checkArray(data, BigInt(1802338304), 6);
    checkArrayForInt(data, Long.toInt(4294967294), -1);
    checkArray(data, BigInt(4294967294), 7);
    checkArrayForInt(data, Double.toInt(2.3058e+16), -1);
    checkArray(data, BigInt(2.3058e+16), 9);
    checkArrayForInt(data, Double.toInt(9.223e+18), -1);
    checkArray(data, BigInt(9.223e+18), 10);
    checkArrayForInt(data, Double.toInt(9.224e+18), -1);
    checkArray(data, BigInt(9.224e+18), -1);

    let udata = BigUint64Array.of(0, 4294967294, 2.3058e+16, 2.3058e+16, 9.223e+18, 2.3058e+16, 18.446e+18, 2.3058e+16)
    checkArrayForInt(udata, Long.toInt(10000000000), -1);
    checkArray(udata, BigInt(10000000000), -1);
    checkArrayForInt(udata, 10000, -1);
    checkArray(udata, BigInt(10000), -1);
    checkArrayForInt(udata, 0, 0);
    checkArray(udata, BigInt(0), 0);
    checkArrayForInt(udata, 1, -1);
    checkArray(udata, BigInt(1), -1);
    checkArrayForInt(udata, Long.toInt(4294967294), -1);
    checkArray(udata, BigInt(4294967294), 1);
    checkArrayForInt(udata, Double.toInt(2.3058e+16), -1);
    checkArray(udata, BigInt(2.3058e+16), 2);
    checkArrayForInt(udata, Double.toInt(9.223e+18), -1);
    checkArray(udata, BigInt(9.223e+18), 4);
    checkArrayForInt(udata, Double.toInt(9.224e+18), -1);
    checkArray(udata, BigInt(9.224e+18), -1);
    checkArrayForInt(udata, Double.toInt(18.446e+18), -1);
    checkArray(udata, BigInt(18.446e+18), 6);
}

function testF64Array() {
    let dataNoInf = Float64Array.of(-1.7e+308, -3.6e+42, -3.6e+42, 0, NaN, -0.6e+16, 0, 0.4e+105, 0, 1.7e+308);
    checkArray(dataNoInf, NaN, -1);
    checkArray(dataNoInf, Infinity, -1);
    checkArray(dataNoInf, -1.701e+308, -1);
    checkArrayForInt(dataNoInf, Double.toInt(-1.701e+308), -1);
    checkArray(dataNoInf, -1.7e+308, 0);
    checkArrayForInt(dataNoInf, Double.toInt(-1.7e+308), -1);
    checkArray(dataNoInf, 1.7e+308, 9);
    checkArrayForInt(dataNoInf, Double.toInt(1.7e+308), -1);
    checkArray(dataNoInf, -1.7e+1, -1);
    checkArrayForInt(dataNoInf, Double.toInt(-1.7e+1), -1);
    checkArray(dataNoInf, 0.0, 3);
    checkArrayForInt(dataNoInf, 0, 3);
    checkArray(dataNoInf, -0.6e+16, 5);
    checkArrayForInt(dataNoInf, Double.toInt(-0.6e+16), -1);
    checkArray(dataNoInf, 0.4e+105, 7);
    checkArrayForInt(dataNoInf, Double.toInt(0.4e+105), -1);

    let dataInf = Float64Array.of(Infinity, -1.7e+308, -3.6e+42, 0, NaN, 1024, -0.6e+16, 0.4e+105, 1.7e+308, Infinity);
    checkArray(dataInf, Infinity, 0);
    checkArray(dataInf, NaN, -1);
    checkArray(dataInf, -1.701e+308, -1);
    checkArrayForInt(dataInf, Double.toInt(-1.701e+308), -1);
    checkArray(dataInf, -1.7e+308, 1);
    checkArrayForInt(dataInf, Double.toInt(-1.7e+308), -1);
    checkArray(dataInf, 1.7e+308, 8);
    checkArrayForInt(dataInf, Double.toInt(1.7e+308), -1);
    checkArray(dataInf, -1.7e+1, -1);
    checkArrayForInt(dataInf, Double.toInt(-1.7e+1), -1);
    checkArray(dataInf, 0.0, 3);
    checkArrayForInt(dataInf, 0, 3);
    checkArray(dataInf, 1024.0, 5);
    checkArrayForInt(dataInf, Double.toInt(1024.5), 5);
    checkArray(dataInf, -0.6e+16, 6);
    checkArrayForInt(dataInf, Double.toInt(-0.6e+16), -1);
    checkArray(dataInf, 0.4e+105, 7);
    checkArrayForInt(dataInf, Double.toInt(0.4e+105), -1);
}

function testF64ArrayFromIndex() {
    let data = Float64Array.of(-1.7e+308, -3.6e+42, -3.6e+42, 0, NaN, -0.6e+16, 0.4e+105, -3.6e+42, 1.7e+308, 1.7e+308);
    checkArray(data, NaN, -1, 0);
    checkArray(data, Infinity, -1, 0);
    checkArray(data, -1.7e+308, 0, 0);
    checkArray(data, -1.7e+308, -1, 1);
    checkArray(data, -1.7e+308, 0, -data.length);
    checkArray(data, -1.7e+308, -1, -(data.length - 1));
    checkArray(data, -1.7e+308, -1, -1);
    checkArray(data, -1.7e+308, 0, -(data.length + 1));
    checkArray(data, 1.7e+308, (data.length - 2), 0);
    checkArray(data, 1.7e+308, (data.length - 2), -2);
    checkArray(data, 1.7e+308, (data.length - 1), -1);
    checkArray(data, 1.7e+308, (data.length - 2), -data.length);
    checkArray(data, 1.7e+308, -1, data.length);
    checkArray(data, 1.7e+308, (data.length - 2), data.length - 2);
    checkArray(data, 1.7e+308, (data.length - 1), data.length - 1);
    checkArray(data, 1.7e+308, (data.length - 2), -(data.length + 1));
    checkArray(data, 1.7e+308, (data.length - 2), -(data.length - 1));
    checkArray(data, -3.6e+42, 1, 1);
    checkArray(data, -3.6e+42, 2, 2);
    checkArray(data, -3.6e+42, 7, 3);
    checkArray(data, -3.6e+42, 7, 7);
    checkArray(data, -3.6e+42, -1, 8);
    checkArray(data, -3.6e+42, 1, -(data.length + 1));
    checkArray(data, -3.6e+42, 1, -(data.length - 1));
    checkArray(data, -3.6e+42, -1, -2);
    checkArray(data, -3.6e+42, 7, -3);
    checkArray(data, -3.6e+42, 1, -data.length);
    checkArray(data, -3.6e+42, -1, data.length);
    checkArray(data, 0.0, 3, 3);
    checkArrayForInt(data, 0, 3, 3);
    checkArray(data, 0.0, -1, 4);
    checkArrayForInt(data, 0, -1, 4);
    checkArray(data, 0.0, -1, 5);
    checkArrayForInt(data, 0, -1, 5);
    checkArray(data, 0.0, -1, data.length);
    checkArrayForInt(data, 0, -1, data.length);
    checkArray(data, 0.0, 3, -data.length);
    checkArrayForInt(data, 0, 3, -data.length);
    checkArray(data, 0.0, 3, -(data.length + 1));
    checkArrayForInt(data, 0, 3, -(data.length + 1));
    checkArray(data, 0.0, 3, -(data.length - 3));
    checkArrayForInt(data, 0, 3, -(data.length - 3));
    checkArray(data, 0.0, -1, -(data.length - 4));
    checkArrayForInt(data, 0, -1, -(data.length - 4));
    checkArray(data, -0.6e+16, 5, 0);
    checkArray(data, -0.6e+16, 5, 5);
    checkArray(data, -0.6e+16, -1, 6);
    checkArray(data, -0.6e+16, -1, -4);
    checkArray(data, -0.6e+16, 5, -5);
    checkArray(data, -0.6e+16, 5, -6);
    checkArray(data, 2147483648, -1, 0);
}

function testF32Array() {
    let data = Float32Array.of(Infinity, -3.4e+38, -3.4e+38, -2.02e+16, 1, 1, 1, 1.5, -1, -3, -3.4e+38, -1.0031,
        -0.1031, -1.061e+09, 1.0601e+09, 1.060001e+09, Infinity, 2.8e+24, Infinity, 2.8e+24, 3.4e+38, Infinity);
    checkArray(data, Infinity, 0);
    checkArrayExactlyNumber(data, Infinity, 0);
    checkArray(data, -3.4e+38, 1);
    checkArrayExactlyNumber(data, -3.4e+38, -1);
    checkArray(data, (-3.4e+38 + 1e32), -1);
    checkArray(data, -2.02e+16, 3);
    checkArrayForInt(data, Double.toInt(-2.02e+16), -1);
    checkArrayExactlyNumber(data, -2.02e+16, -1);
    checkArray(data, -2.020101e+16, -1);
    checkArray(data, -1.061e+09, 13);
    checkArrayExactlyNumber(data, -1.061e+09, 13); // this is exactly c1cf9ec9a0000000 as well as the float
    checkArray(data, -1.0031, 11);
    checkArrayForInt(data, Double.toInt(-1.0031), 8); // this is looking for -1
    checkArrayExactlyNumber(data, -1.0031, -1);
    checkArray(data, -0.1031, 12);
    checkArrayExactlyNumber(data, -0.1031, -1);
    checkArrayForInt(data, Double.toInt(-0.0031), -1); // there is no zero in the array
    checkArray(data, 1.0, 4);
    checkArrayExactlyNumber(data, 1.0, 4);
    checkArrayForInt(data, 1, 4);
    checkArray(data, 1 + 1.19209290e-07, -1);
    checkArray(data, 1.0601e+09, 14);
    checkArrayExactlyNumber(data, 1.0601e+09, -1);
    checkArray(data, 1.060001e+09, 15);
    checkArrayExactlyNumber(data, 1.060001e+09, -1);
    checkArray(data, 2.8e+24, 17);
    checkArrayExactlyNumber(data, 2.8e+24, -1);
    checkArray(data, 3.4e+38, 20);
    checkArrayExactlyNumber(data, 3.4e+38, -1);
    checkArray(data, (3.4e+38 - 1.175e-38), 20);
    checkArrayExactlyNumber(data, (3.4e+38 - 1.175e-38), -1);
    checkArray(data, (3.4e+38 - 1e32), -1);
}

function testF32ArrayFromIndex() {
    let data = Float32Array.of(Infinity, -3.4e+38, -3.4e+38, -2.02e+16, 1, 1, 1, 1.5, -1, -3, -3.4e+38, -1.0031,
        -0.1031, -1.061e+09, 1.0601e+09, 1.060001e+09, Infinity, 2.8e+24, Infinity, 2.8e+24, 3.4e+38, Infinity);
    checkArray(data, Infinity, 0, 0);
    checkArray(data, Infinity, 16, 1);
    checkArray(data, Infinity, 21, -1);
    checkArray(data, Infinity, 21, -2);
    checkArray(data, Infinity, 21, -3);
    checkArray(data, Infinity, 18, -4);
    checkArray(data, Infinity, 0, -(data.length + 1));
    checkArray(data, Infinity, 0, -data.length);
    checkArray(data, Infinity, 16, -(data.length - 1));
    checkArray(data, 1, 4, 0);
    checkArrayForInt(data, 1, 4, 0);
    checkArray(data, 1, 4, 4);
    checkArrayForInt(data, 1, 4, 4);
    checkArray(data, 1, 5, 5);
    checkArrayForInt(data, 1, 5, 5);
    checkArray(data, 1, 6, 6);
    checkArrayForInt(data, 1, 6, 6);
    checkArray(data, 1, -1, 7);
    checkArrayForInt(data, 1, -1, 7);
    checkArray(data, 1, 4, -(data.length - 4));
    checkArrayForInt(data, 1, 4, -(data.length - 4));
    checkArray(data, 1, 5, -(data.length - 5));
    checkArrayForInt(data, 1, 5, -(data.length - 5));
    checkArray(data, -1.0031, 11, 0);
    checkArray(data, -1.0031, 11, 9);
    checkArray(data, -1.0031, 11, 11);
    checkArray(data, -1.0031, -1, 12);
    checkArrayForInt(data, Double.toInt(-1.0031), 8, 0); // this is looking for -1
    checkArrayForInt(data, Double.toInt(-1.0031), 8, 8);
    checkArrayForInt(data, Double.toInt(-1.0031), -1, 9);
}

function main(): int {
    let testSuite = new arktest.ArkTestsuite('typedArray.indexOf');
    testSuite.addTest('Int8,UInt8 Arrays: indexOf for arrays', testIU8Array);
    testSuite.addTest('Int8,UInt8 Arrays: indexOf for arrays', testIU8ArrayFromIndex);
    testSuite.addTest('Int16,UInt16 Arrays: indexOf for arrays', testIU16Array);
    testSuite.addTest('Int32,UInt32 Arrays: indexOf for arrays', testIU32Array);
    testSuite.addTest('BigInt64,BigUint64 Arrays: indexOf for arrays', testIU64Array);
    testSuite.addTest('Float32 Arrays: indexOf for arrays', testF32Array);
    testSuite.addTest('Float32 Arrays: indexOf for arrays with fromIndex', testF32ArrayFromIndex);
    testSuite.addTest('Float64 Arrays: indexOf for arrays', testF64Array);
    testSuite.addTest('Float64 Arrays: indexOf for arrays with fromIndex', testF64ArrayFromIndex);
    return testSuite.run();
}
