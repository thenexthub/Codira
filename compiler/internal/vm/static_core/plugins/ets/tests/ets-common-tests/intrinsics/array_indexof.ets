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

function checkArray<T extends Number | BigInt>(data: Array<T>, elem: T, expected: int, fromIndex?: int): void {
    arktest.assertEQ(data.indexOf(elem, fromIndex), expected,
        'data.indexOf(elem: T, fromIndex?: number) must return ' + expected);
}

function checkArray<T extends Number | BigInt>(data: Array<T>, elem: T, expected: number): void {
    arktest.assertEQ(data.indexOf(elem), expected, 'data.indexOf(elem: T) must return ' + expected);
}

function testArrayOfBigInt() {
    let data = Array.of(-1n, BigInt(-9.223e+18), BigInt(-9.223e+18), BigInt(-9.223e+18), -8589934588n, -2147483648n, 0n,
        1802338304n, 4294967294n, 1802338304n, BigInt(2.3058e+16), BigInt(9.223e+18));
    checkArray(data, -1n, 0);
    checkArray(data, -10000000000n, -1);
    checkArray(data, -10000n, -1);
    checkArray(data, BigInt(-9.224e+18), -1);
    checkArray(data, BigInt(-9.223e+18), 1);
    checkArray(data, -8589934588n, 4);
    checkArray(data, 0n, 6);
    checkArray(data, 1802338304n, 7);
    checkArray(data, 4294967294n, 8);
    checkArray(data, BigInt(2.3058e+16), 10);
    checkArray(data, BigInt(9.223e+18), 11);
    checkArray(data, BigInt(9.224e+18), -1);
}

function testArrayOfBigIntFromIndex() {
    let data = Array.of(BigInt(-1.7e+308), BigInt(-3.6e+42), BigInt(-3.6e+42), 0n, BigInt(-0.6e+16), BigInt(0.4e+105),
        BigInt(-3.6e+42), BigInt(1.7e+308), BigInt(1.7e+308), 4294967294n, 1802338304n, -10000000000n,
        9007199254740991n * 9007199254740991n);
    checkArray(data, BigInt(-1.7e+308), 0, 0);
    checkArray(data, BigInt(-1.7e+308), -1, 1);
    checkArray(data, BigInt(-1.7e+308), 0, -data.length);
    checkArray(data, BigInt(-1.7e+308), -1, -(data.length - 1));
    checkArray(data, BigInt(-1.7e+308), -1, -1);
    checkArray(data, BigInt(-1.7e+308), 0, -(data.length + 1));
    checkArray(data, BigInt(1.7e+308), 7, 0);
    checkArray(data, BigInt(1.7e+308), -1, -2);
    checkArray(data, BigInt(1.7e+308), 7, -6);
    checkArray(data, BigInt(1.7e+308), 8, -5);
    checkArray(data, BigInt(1.7e+308), -1, -1);
    checkArray(data, BigInt(1.7e+308), 7, -data.length);
    checkArray(data, BigInt(1.7e+308), -1, data.length);
    checkArray(data, BigInt(1.7e+308), 7, 7);
    checkArray(data, BigInt(1.7e+308), 8, 8);
    checkArray(data, BigInt(1.7e+308), 7, -(data.length + 1));
    checkArray(data, BigInt(1.7e+308), 7, -(data.length - 1));
    checkArray(data, BigInt(-3.6e+42), 1, 1);
    checkArray(data, BigInt(-3.6e+42), 2, 2);
    checkArray(data, BigInt(-3.6e+42), 6, 3);
    checkArray(data, BigInt(-3.6e+42), -1, 7);
    checkArray(data, BigInt(-3.6e+42), 6, 6);
    checkArray(data, BigInt(-3.6e+42), 1, -(data.length + 1));
    checkArray(data, BigInt(-3.6e+42), 1, -(data.length - 1));
    checkArray(data, BigInt(-3.6e+42), 1, -data.length);
    checkArray(data, BigInt(-3.6e+42), -1, data.length);
    checkArray(data, BigInt(-3.6e+42), -1, -6);
    checkArray(data, BigInt(-3.6e+42), 6, -7);
    checkArray(data, 0n, 3, 3);
    checkArray(data, 0n, -1, 4);
    checkArray(data, 0n, -1, 5);
    checkArray(data, 0n, -1, data.length);
    checkArray(data, 0n, 3, -data.length);
    checkArray(data, 0n, 3, -(data.length + 1));
    checkArray(data, 0n, 3, -(data.length - 3));
    checkArray(data, 0n, -1, -(data.length - 4));
    checkArray(data, BigInt(-0.6e+16), 4, 0);
    checkArray(data, BigInt(-0.6e+16), 4, 4);
    checkArray(data, BigInt(-0.6e+16), -1, 5);
    checkArray(data, BigInt(-0.6e+16), -1, -8);
    checkArray(data, BigInt(-0.6e+16), 4, -9);
    checkArray(data, BigInt(-0.6e+16), 4, -10);
    checkArray(data, 2147483648n, -1, 0);
    checkArray(data, 4294967294n, 9, 0);
    checkArray(data, 1802338304n, 10, 0);
    checkArray(data, -10000000000n, 11, 0);
    checkArray(data, -10000000000n, -1, data.length);
    checkArray(data, -10000000000n, -1, data.length - 1);
    checkArray(data, -10000000000n, 11, data.length - 2);
    checkArray(data, -10000000001n, -1, 0);
    checkArray(data, 9007199254740991n * 9007199254740991n, 12, 0);
    checkArray(data, 9007199254740991n * 9007199254740991n, 12, 1);
    checkArray(data, 9007199254740991n * 9007199254740991n, 12, -data.length);
    checkArray(data, 9007199254740991n * 9007199254740991n, -1, data.length);
    checkArray(data, 9007199254740991n * 9007199254740991n, 12, data.length - 1);
}

function testArrayOfNumber() {
    let data = Array.of(-1.0, -9.223e+18, -9.223e+18, -9.223e+18, -8589934588.00001, -2147483648.0002, 0, 1802338304,
        4294967294.0, 1802338304.1, 2.3058e+16, 9.223e+18);
    checkArray(data, -1, 0);
    checkArray(data, -10000000000.0, -1);
    checkArray(data, -10000, -1);
    checkArray(data, -9.224e+18, -1);
    checkArray(data, -9.223e+18, 1);
    checkArray(data, -8589934588.00001, 4);
    checkArray(data, -2147483648.0002, 5);
    checkArray(data, 0, 6);
    checkArray(data, 1802338304, 7);
    checkArray(data, 4294967294.0, 8);
    checkArray(data, 1802338304.1, 9);
    checkArray(data, 1802338304.11, -1);
    checkArray(data, 2.3058e+16, 10);
    checkArray(data, 9.223e+18, 11);
    checkArray(data, 9.224e+18, -1);
}

function testArrayOfNumberFromIndex() {
    let data = Array.of(-1.7e+308, -3.6e+42, -3.6e+42, 0, -0.6e+16, 0.4e+105, -3.6e+42, 1.7e+308, 1.7e+308, 4294967294,
        1802338304.00001, -10000000000.99999);
    checkArray(data, -1.7e+308, 0, 0);
    checkArray(data, -1.7e+308, -1, 1);
    checkArray(data, -1.7e+308, 0, -data.length);
    checkArray(data, -1.7e+308, -1, -(data.length - 1));
    checkArray(data, -1.7e+308, -1, -1);
    checkArray(data, -1.7e+308, 0, -(data.length + 1));
    checkArray(data, 1.7e+308, 7, 0);
    checkArray(data, 1.7e+308, 7, -5);
    checkArray(data, 1.7e+308, 8, -4);
    checkArray(data, 1.7e+308, -1, -3);
    checkArray(data, 1.7e+308, 7, -data.length);
    checkArray(data, 1.7e+308, -1, data.length);
    checkArray(data, 1.7e+308, 7, data.length - 5);
    checkArray(data, 1.7e+308, 8, data.length - 4);
    checkArray(data, 1.7e+308, 7, -(data.length + 1));
    checkArray(data, 1.7e+308, 7, -(data.length - 1));
    checkArray(data, -3.6e+42, 1, 1);
    checkArray(data, -3.6e+42, 2, 2);
    checkArray(data, -3.6e+42, 6, 3);
    checkArray(data, -3.6e+42, 6, 6);
    checkArray(data, -3.6e+42, -1, 8);
    checkArray(data, -3.6e+42, 1, -(data.length + 1));
    checkArray(data, -3.6e+42, 1, -(data.length - 1));
    checkArray(data, -3.6e+42, -1, -2);
    checkArray(data, -3.6e+42, 6, -6);
    checkArray(data, -3.6e+42, -1, -5);
    checkArray(data, -3.6e+42, 1, -data.length);
    checkArray(data, -3.6e+42, -1, data.length);
    checkArray(data, 0.0, 3, 3);
    checkArray(data, 0.0, -1, 4);
    checkArray(data, 0.0, -1, 5);
    checkArray(data, 0.0, -1, data.length);
    checkArray(data, 0.0, 3, -data.length);
    checkArray(data, 0.0, 3, -(data.length + 1));
    checkArray(data, 0.0, 3, -(data.length - 3));
    checkArray(data, 0.0, -1, -(data.length - 4));
    checkArray(data, -0.6e+16, 4, 0);
    checkArray(data, -0.6e+16, 4, 4);
    checkArray(data, -0.6e+16, -1, 5);
    checkArray(data, -0.6e+16, -1, -7);
    checkArray(data, -0.6e+16, 4, -8);
    checkArray(data, -0.6e+16, 4, -9);
    checkArray(data, 2147483648, -1, 0);
    checkArray(data, 4294967294, 9, 0);
    checkArray(data, 1802338304.00001, 10, 0);
    checkArray(data, 1802338304.00001, -1, data.length);
    checkArray(data, 1802338304.00001, -1, data.length - 1);
    checkArray(data, 1802338304.00001, 10, data.length - 2);
    checkArray(data, 1802338304, -1, 0);
    checkArray(data, -10000000000.99999, 11, 0);
    checkArray(data, -10000000000.99999, -1, data.length);
    checkArray(data, -10000000000.99999, 11, data.length - 1);
    checkArray(data, -10000000001, -1, 0);
}

function main(): int {
    let testSuite = new arktest.ArkTestsuite('Array.indexOf');
    testSuite.addTest('Arrays of BigInt64: indexOf for arrays', testArrayOfBigInt);
    testSuite.addTest('Arrays of BigInt64: indexOf for arrays with fromIndex', testArrayOfBigIntFromIndex);
    testSuite.addTest('Arrays of Number: indexOf for arrays', testArrayOfNumber);
    testSuite.addTest('Arrays of Number: indexOf for arrays with fromIndex', testArrayOfNumberFromIndex);
    return testSuite.run();
}
