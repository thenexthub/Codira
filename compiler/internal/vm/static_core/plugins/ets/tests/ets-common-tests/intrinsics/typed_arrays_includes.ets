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

function checkArray(data: Float64Array, elem: number, expected: boolean, fromIndex?: int): void {
    arktest.assertEQ(data.includes(elem, fromIndex), expected,
        'data.includes(elem: number, fromIndex?: int) must return ' + expected);
}

function checkArray(data: Float32Array, elem: number, expected: boolean, fromIndex?: int): void {
    arktest.assertEQ(data.includes((elem.toFloat()).toDouble(), fromIndex), expected,
        'data.includes(elem: number, fromIndex?: int) must return ' + expected);
}

function checkArrayForFloat(data: Float32Array, elem: float): void {
    arktest.assertEQ(data.includes(elem), data.indexOf(elem.toInt()) > 0, 'data.includes(elem: float) must check ' +
        'whether data.indexOf(elem.toInt()) > -1, so whether the truncated float exists within the array.');
}

function testF64ArrayNoNaN() {
    let dataNoInf = Float64Array.of(-1.7e+308, -3.6e+42, 0, -0.6e+16, 0.4e+105, 1.7e+308);
    checkArray(dataNoInf, Infinity, false);
    checkArray(dataNoInf, -1.701e+308, false);
    checkArray(dataNoInf, -1.7e+308, true);
    checkArray(dataNoInf, 1.7e+308, true);
    checkArray(dataNoInf, -1.7e+1, false);
    checkArray(dataNoInf, 0, true);
}

function testF64ArrayNoNaNFromIndex() {
    let dataNoInf = Float64Array.of(-1.7e+308, -3.6e+42, 0, -0.6e+16, 0.4e+105, 1.7e+308);
    checkArray(dataNoInf, -1.7e+308, true);
    checkArray(dataNoInf, -1.7e+308, true, 0);
    checkArray(dataNoInf, -1.7e+308, false, 1);
    checkArray(dataNoInf, -1.7e+308, true, -dataNoInf.length);
    checkArray(dataNoInf, -1.7e+308, false, -(dataNoInf.length - 1));
    checkArray(dataNoInf, -1.7e+308, false, -1);
    checkArray(dataNoInf, -1.7e+308, true, -(dataNoInf.length + 1));
    checkArray(dataNoInf, 1.7e+308, true);
    checkArray(dataNoInf, 1.7e+308, true, 0);
    checkArray(dataNoInf, 1.7e+308, true, -1);
    checkArray(dataNoInf, 1.7e+308, true, -dataNoInf.length);
    checkArray(dataNoInf, 1.7e+308, false, dataNoInf.length);
    checkArray(dataNoInf, 1.7e+308, true, dataNoInf.length - 1);
    checkArray(dataNoInf, 1.7e+308, true, -(dataNoInf.length + 1));
    checkArray(dataNoInf, 1.7e+308, true, -(dataNoInf.length - 1));
    checkArray(dataNoInf, -3.6e+42, true);
    checkArray(dataNoInf, -3.6e+42, true, 1);
    checkArray(dataNoInf, -3.6e+42, false, 2);
    checkArray(dataNoInf, -3.6e+42, true, -(dataNoInf.length + 1));
    checkArray(dataNoInf, -3.6e+42, true, -(dataNoInf.length - 1));
    checkArray(dataNoInf, -3.6e+42, false, -(dataNoInf.length - 2));
    checkArray(dataNoInf, -3.6e+42, true, -dataNoInf.length);
    checkArray(dataNoInf, -3.6e+42, false, dataNoInf.length);
    checkArray(dataNoInf, 0, true);
    checkArray(dataNoInf, 0, true, 2);
    checkArray(dataNoInf, 0, false, 3);
    checkArray(dataNoInf, 0, false, 4);
    checkArray(dataNoInf, 0, false, dataNoInf.length);
    checkArray(dataNoInf, 0, true, -dataNoInf.length);
    checkArray(dataNoInf, 0, true, -(dataNoInf.length + 1));
    checkArray(dataNoInf, 0, true, -(dataNoInf.length - 2));
    checkArray(dataNoInf, 0, false, -(dataNoInf.length - 3));
}

function testF32ArrayNoNaN() {
    let dataNoInf = Float32Array.of(-3.4e+38, -2.02e+16, 1, 1.5, -1, -3, -1.0031, -.1031, -1.061e+09, 1.0601e+09,
        1.060001e+09, 2.8e+24, 3.4e+38);
    checkArray(dataNoInf, Infinity, false);
    checkArray(dataNoInf, -3.4e+38, true);
    checkArrayForFloat(dataNoInf, -3.4e+38f); // -3 exists, indexOf(-3) returns smth > -1
    checkArray(dataNoInf, (-3.4e+38 + 1e32), false);
    checkArrayForFloat(dataNoInf, (-3.4e+38f + 1e32f));
    checkArray(dataNoInf, -2.02e+16, true);
    checkArrayForFloat(dataNoInf, -2.02e+16f); // -2 doesn't exist, indexOf(-2) returns -1
    checkArray(dataNoInf, -2.020101e+16, false);
    checkArrayForFloat(dataNoInf, -2.020101e+16f);
    checkArray(dataNoInf, -1.061e+09, true);
    checkArray(dataNoInf, -1.0031, true);
    checkArrayForFloat(dataNoInf, -1.0031f);
    checkArray(dataNoInf, -0.1031, true);
    checkArrayForFloat(dataNoInf, -0.1031f);
    checkArray(dataNoInf, -1.061e+09, true);
    checkArrayForFloat(dataNoInf, -1.061e+09f);
    checkArray(dataNoInf, 1.0, true);
    checkArrayForFloat(dataNoInf, 1.0f);
    checkArray(dataNoInf, 1 + 1.19209290e-07, false);
    checkArrayForFloat(dataNoInf, 1.0f + 1.19209290e-07f);
    checkArray(dataNoInf, 1.0601e+09, true);
    checkArrayForFloat(dataNoInf, 1.0601e+09f);
    checkArray(dataNoInf, 1.060001e+09, true);
    checkArrayForFloat(dataNoInf, 1.060001e+09f);
    checkArray(dataNoInf, 2.8e+24, true);
    checkArrayForFloat(dataNoInf, 2.8e+24f);
    checkArray(dataNoInf, 3.4e+38, true);
    checkArrayForFloat(dataNoInf, 3.4e+38f);
    checkArray(dataNoInf, (3.4e+38 - 1.175e-38), true);
    checkArrayForFloat(dataNoInf, (3.4e+38f - 1.175e-38f));
    checkArray(dataNoInf, (3.4e+38 - 1e32), false);
    checkArrayForFloat(dataNoInf, (3.4e+38f - 1e32f));
}
function testF32ArrayNoNaNFromIndex() {
    let dataNoInf = Float32Array.of(-3.4e+38, -2.02e+16, 1, 1.5, -1, -3, -1.0031, -.1031, -1.061e+09, 1.0601e+09,
        1.060001e+09, 2.8e+24, 3.4e+38);
    checkArray(dataNoInf, Infinity, false);
    checkArray(dataNoInf, -3.4e+38, true);
    checkArray(dataNoInf, -3.4e+38, true, 0);
    checkArray(dataNoInf, -3.4e+38, false, 1);
    checkArray(dataNoInf, -3.4e+38, true, -dataNoInf.length);
    checkArray(dataNoInf, -3.4e+38, false, -(dataNoInf.length - 1));
    checkArray(dataNoInf, -3.4e+38, false, -1);
    checkArray(dataNoInf, -3.4e+38, true, -(dataNoInf.length + 1));
    checkArray(dataNoInf, -2.02e+16, true);
    checkArray(dataNoInf, -2.02e+16, true, 1);
    checkArray(dataNoInf, -2.02e+16, false, 2);
    checkArray(dataNoInf, -2.02e+16, true, -(dataNoInf.length + 1));
    checkArray(dataNoInf, -2.02e+16, true, -(dataNoInf.length - 1));
    checkArray(dataNoInf, -2.02e+16, false, -(dataNoInf.length - 2));
    checkArray(dataNoInf, -2.02e+16, true, -dataNoInf.length);
    checkArray(dataNoInf, -2.02e+16, false, dataNoInf.length);
    checkArray(dataNoInf, -2.020101e+16, false);
    checkArray(dataNoInf, -1.061e+09, true);
    checkArray(dataNoInf, -1.0031, true);
    checkArray(dataNoInf, -0.1031, true);
    checkArray(dataNoInf, -1.061e+09, true);
    checkArray(dataNoInf, 1.0, true);
    checkArray(dataNoInf, 1, true, 2);
    checkArray(dataNoInf, 1, false, 3);
    checkArray(dataNoInf, 1, false, 4);
    checkArray(dataNoInf, 1, false, dataNoInf.length);
    checkArray(dataNoInf, 1, true, -dataNoInf.length);
    checkArray(dataNoInf, 1, true, -(dataNoInf.length + 1));
    checkArray(dataNoInf, 1, true, -(dataNoInf.length - 2));
    checkArray(dataNoInf, 1, false, -(dataNoInf.length - 3));
    checkArray(dataNoInf, 1 + 1.19209290e-08, true);
    checkArray(dataNoInf, 1 + 1.19209290e-07, false);
    checkArray(dataNoInf, 1.0601e+09, true);
    checkArray(dataNoInf, 1.060001e+09, true);
    checkArray(dataNoInf, 2.8e+24, true);
    checkArray(dataNoInf, 3.4e+38, true);
    checkArray(dataNoInf, 3.4e+38, true);
    checkArray(dataNoInf, 3.4e+38, true, 0);
    checkArray(dataNoInf, 3.4e+38, true, -1);
    checkArray(dataNoInf, 3.4e+38, true, -dataNoInf.length);
    checkArray(dataNoInf, 3.4e+38, false, dataNoInf.length);
    checkArray(dataNoInf, 3.4e+38, true, dataNoInf.length - 1);
    checkArray(dataNoInf, 3.4e+38, true, -(dataNoInf.length + 1));
    checkArray(dataNoInf, 3.4e+38, true, -(dataNoInf.length - 1));
    checkArray(dataNoInf, (3.4e+38 - 1.175e-38), true);
    checkArray(dataNoInf, (3.4e+38 - 1e32), false);
}

function testF64ArrayInf() {
    const checkEveryElement = (arr: Float64Array) => {
        checkArray(arr, -1.7e+308, true);
        checkArray(arr, -3.6e+42, true);
        checkArray(arr, -0.6e+16, true);
        checkArray(arr, 0.4e+105, true);
        checkArray(arr, 1.7e+308, true);
        checkArray(arr, Infinity, true);
        checkArray(arr, NaN, false);
        checkArray(arr, 0, false);
        checkArray(arr, 1024.0, false);
    }
    let dataInf0 = Float64Array.of(Infinity, -1.7e+308, -3.6e+42, -0.6e+16, 0.4e+105, 1.7e+308);
    checkEveryElement(dataInf0);
    let dataInf1 = Float64Array.of(-1.7e+308, Infinity, -3.6e+42, -0.6e+16, 0.4e+105, 1.7e+308);
    checkEveryElement(dataInf1);
    let dataInf2 = Float64Array.of(-1.7e+308, -3.6e+42, Infinity, -0.6e+16, 0.4e+105, 1.7e+308);
    checkEveryElement(dataInf2);
    let dataInf3 = Float64Array.of(-1.7e+308, -3.6e+42, -0.6e+16, Infinity, 0.4e+105, 1.7e+308);
    checkEveryElement(dataInf3);
    let dataInf4 = Float64Array.of(-1.7e+308, -3.6e+42, -0.6e+16, 0.4e+105, Infinity, 1.7e+308);
    checkEveryElement(dataInf4);
    let dataInf5 = Float64Array.of(-1.7e+308, -3.6e+42, -0.6e+16, 0.4e+105, 1.7e+308, Infinity);
    checkEveryElement(dataInf5);
}

function testF32ArrayInf() {
    const checkEveryElement = (arr: Float32Array) => {
        checkArray(arr, -3.4e+38, true);
        checkArrayForFloat(arr, -3.4e+38f);
        checkArray(arr, -2.02e+16, true);
        checkArrayForFloat(arr, -2.02e+16f);
        checkArray(arr, 0, true);
        checkArrayForFloat(arr, 0.0f);
        checkArray(arr, 1.06e+12, true);
        checkArrayForFloat(arr, 1.06e+12f);
        checkArray(arr, 2.8e+24, true);
        checkArrayForFloat(arr, 2.8e+24f);
        checkArray(arr, 3.4e+38, true);
        checkArrayForFloat(arr, 3.4e+38f);
        checkArray(arr, Infinity, true);
        checkArrayForFloat(arr, Infinity.toFloat());
        checkArray(arr, NaN, false);
        checkArrayForFloat(arr, NaN.toFloat());
        checkArray(arr, (3.4e+38 - 1e+32), false);
        checkArrayForFloat(arr, 3.4e+38f - 1e32f);
        checkArray(arr, 0.0000001, false);
        checkArrayForFloat(arr, 0.0000001f);
        checkArray(arr, 1024.0, false);
        checkArrayForFloat(arr, 1024.0f);
    }
    let dataInf0 = Float32Array.of(Infinity, -3.4e+38, -2.02e+16, 0, 1.06e+12, 2.8e+24, 3.4e+38);
    checkEveryElement(dataInf0);
    let dataInf1 = Float32Array.of(-3.4e+38, Infinity, -2.02e+16, 0, 1.06e+12, 2.8e+24, 3.4e+38);
    checkEveryElement(dataInf1);
    let dataInf2 = Float32Array.of(-3.4e+38, -2.02e+16, Infinity, 0, 1.06e+12, 2.8e+24, 3.4e+38);
    checkEveryElement(dataInf2);
    let dataInf3 = Float32Array.of(-3.4e+38, -2.02e+16, 0, Infinity, 1.06e+12, 2.8e+24, 3.4e+38);
    checkEveryElement(dataInf3);
    let dataInf4 = Float32Array.of(-3.4e+38, -2.02e+16, 0, 1.06e+12, Infinity, 2.8e+24, 3.4e+38);
    checkEveryElement(dataInf4);
    let dataInf5 = Float32Array.of(-3.4e+38, -2.02e+16, 0, 1.06e+12, 2.8e+24, Infinity, 3.4e+38);
    checkEveryElement(dataInf5);
    let dataInf6 = Float32Array.of(-3.4e+38, -2.02e+16, 0, 1.06e+12, 2.8e+24, 3.4e+38, Infinity);
    checkEveryElement(dataInf6);
}

function testF64ArrayNaN() {
    const checkNaN = (arr: Float64Array, positionOfNaN: int) => {
        arktest.assertLT(1, arr.length);
        arktest.assertLT(positionOfNaN, arr.length);
        checkArray(arr, NaN, true);
        checkArray(arr, NaN, true, 0);
        checkArray(arr, NaN, true, positionOfNaN);
        checkArray(arr, NaN, true, -(arr.length + positionOfNaN));
        checkArray(arr, NaN, false, positionOfNaN + 1);
        checkArray(arr, NaN, true, -arr.length);
        checkArray(arr, NaN, false, arr.length);
        checkArray(arr, NaN, positionOfNaN == arr.length - 1, arr.length - 1);
        checkArray(arr, NaN, true, -(arr.length + 1));
        checkArray(arr, NaN, true, -(arr.length - positionOfNaN));
    }
    const checkNoNaN = (arr: Float64Array, positionOfNaN: int) => {
        arktest.assertLT(1, arr.length);
        const positionOfVal = positionOfNaN < 4 ? 4 : 3;
        arktest.assertLT(positionOfVal, arr.length);
        const value: number = arr[positionOfVal];
        arktest.assertFalse(isNaN(value));
        checkArray(arr, value, true);
        checkArray(arr, value, true, positionOfVal);
        checkArray(arr, value, false, arr.length + positionOfVal);
        checkArray(arr, value, false, arr.length);
        checkArray(arr, value, true, -arr.length);
        for (let p = 0; p <= positionOfVal; p++) {
            checkArray(arr, value, true, p);
            checkArray(arr, value, true, -(arr.length - p));
        }
        checkArray(arr, value, false, -(arr.length - positionOfVal - 1));
        checkArray(arr, value, true, arr.length - (arr.length - positionOfVal));
        checkArray(arr, Infinity, true);
        checkArray(arr, -100, false);
    }
    let dataNaN0 = Float64Array.of(NaN, Infinity, -1.7e+308, -3.6e+42, -0.6e+16, 0.4e+105, 1.7e+308);
    checkNaN(dataNaN0, 0);
    checkNoNaN(dataNaN0, 0);
    let dataNaN1 = Float64Array.of(Infinity, NaN, -1.7e+308, -3.6e+42, -0.6e+16, 0.4e+105, 1.7e+308);
    checkNaN(dataNaN1, 1);
    checkNoNaN(dataNaN1, 1);
    let dataNaN2 = Float64Array.of(Infinity, -1.7e+308, NaN, -3.6e+42, -0.6e+16, 0.4e+105, 1.7e+308);
    checkNaN(dataNaN2, 2);
    checkNoNaN(dataNaN2, 2);
    let dataNaN3 = Float64Array.of(Infinity, -1.7e+308, -3.6e+42, NaN, -0.6e+16, 0.4e+105, 1.7e+308);
    checkNaN(dataNaN3, 3);
    checkNoNaN(dataNaN3, 3);
    let dataNaN4 = Float64Array.of(Infinity, -1.7e+308, -3.6e+42, -0.6e+16, NaN, 0.4e+105, 1.7e+308);
    checkNaN(dataNaN4, 4);
    checkNoNaN(dataNaN4, 4);
    let dataNaN5 = Float64Array.of(Infinity, -1.7e+308, -3.6e+42, -0.6e+16, 0.4e+105, NaN, 1.7e+308);
    checkNaN(dataNaN5, 5);
    checkNoNaN(dataNaN5, 5);
    let dataNaN6 = Float64Array.of(Infinity, -1.7e+308, -3.6e+42, -0.6e+16, 0.4e+105, 1.7e+308, NaN);
    checkNaN(dataNaN6, 6);
    checkNoNaN(dataNaN6, 6);
}

function testF32ArrayNaN() {
    const checkNaN = (arr: Float32Array, positionOfNaN: int) => {
        arktest.assertLT(1, arr.length);
        arktest.assertLT(positionOfNaN, arr.length);
        checkArray(arr, NaN, true);
        checkArray(arr, NaN, true, 0);
        checkArray(arr, NaN, true, positionOfNaN);
        checkArray(arr, NaN, true, -(arr.length + positionOfNaN));
        checkArray(arr, NaN, false, positionOfNaN + 1);
        checkArray(arr, NaN, true, -arr.length);
        checkArray(arr, NaN, false, arr.length);
        checkArray(arr, NaN, positionOfNaN == arr.length - 1, arr.length - 1);
        checkArray(arr, NaN, true, -(arr.length + 1));
        checkArray(arr, NaN, true, -(arr.length - positionOfNaN));
        checkArrayForFloat(arr, NaN.toFloat()); // No special case for NaN in Float32Array.includes(float)
    }
    const checkNoNaN = (arr: Float32Array, positionOfNaN: int) => {
        arktest.assertLT(1, arr.length);
        const positionOfVal = positionOfNaN < 5 ? 5 : 4;
        arktest.assertLT(positionOfVal, arr.length);
        const value: number = arr[positionOfVal];
        arktest.assertFalse(isNaN(value));
        checkArray(arr, 0, true);
        checkArray(arr, Infinity, true);
        checkArray(arr, value, true);
        checkArray(arr, value, true, positionOfVal);
        checkArray(arr, value, false, arr.length + positionOfVal);
        checkArray(arr, value, false, arr.length);
        checkArray(arr, value, true, -arr.length);
        for (let p = 0; p <= positionOfVal; p++) {
            checkArray(arr, value, true, p);
            checkArray(arr, value, true, -(arr.length - p));
        }
        checkArray(arr, value, false, -(arr.length - positionOfVal - 1));
        checkArray(arr, value, true, arr.length - (arr.length - positionOfVal));
        checkArrayForFloat(arr, 0.0f);
        checkArrayForFloat(arr, -100.01f);
    }
    let dataNaN0 = Float32Array.of(NaN, Infinity, -3.4e+38, -2.02e+16, 0, 1.06e+12, 2.8e+24, 3.4e+38);
    checkNaN(dataNaN0, 0);
    checkNoNaN(dataNaN0, 0);
    let dataNaN1 = Float32Array.of(Infinity, NaN, -3.4e+38, -2.02e+16, 0, 1.06e+12, 2.8e+24, 3.4e+38);
    checkNaN(dataNaN1, 1);
    checkNoNaN(dataNaN1, 1);
    let dataNaN2 = Float32Array.of(Infinity, -3.4e+38, NaN, -2.02e+16, 0, 1.06e+12, 2.8e+24, 3.4e+38);
    checkNaN(dataNaN2, 2);
    checkNoNaN(dataNaN2, 2);
    let dataNaN3 = Float32Array.of(Infinity, -3.4e+38, -2.02e+16, NaN, 0, 1.06e+12, 2.8e+24, 3.4e+38);
    checkNaN(dataNaN3, 3);
    checkNoNaN(dataNaN3, 3);
    let dataNaN4 = Float32Array.of(Infinity, -3.4e+38, -2.02e+16, 0, NaN, 1.06e+12, 2.8e+24, 3.4e+38);
    checkNaN(dataNaN4, 4);
    checkNoNaN(dataNaN4, 4);
    let dataNaN5 = Float32Array.of(Infinity, -3.4e+38, -2.02e+16, 0, 1.06e+12, NaN, 2.8e+24, 3.4e+38);
    checkNaN(dataNaN5, 5);
    checkNoNaN(dataNaN5, 5);
    let dataNaN6 = Float32Array.of(Infinity, -3.4e+38, -2.02e+16, 0, 1.06e+12, 2.8e+24, NaN, 3.4e+38);
    checkNaN(dataNaN6, 6);
    checkNoNaN(dataNaN6, 6);
    let dataNaN7 = Float32Array.of(Infinity, -3.4e+38, -2.02e+16, 0, 1.06e+12, 2.8e+24, 3.4e+38, NaN);
    checkNaN(dataNaN7, 7);
    checkNoNaN(dataNaN7, 7);
}

function testF64ArrayNaNOnly() {
    const checkNaNOnly = (arr: Float64Array) => {
        checkArray(arr, NaN, true);
        checkArray(arr, NaN, false, arr.length);
        checkArray(arr, NaN, true, -arr.length);
        checkArray(arr, NaN, true, -(arr.length + 1));
        checkArray(arr, 0, false);
    }
    checkNaNOnly(Float64Array.of(NaN));
    checkNaNOnly(Float64Array.of(NaN, NaN));
    checkNaNOnly(Float64Array.of(NaN, NaN, NaN));
}

function testF32ArrayNaNOnly() {
    const checkNaNOnly = (arr: Float32Array) => {
        checkArray(arr, NaN, true);
        checkArray(arr, NaN, false, arr.length);
        checkArray(arr, NaN, true, -arr.length);
        checkArray(arr, NaN, true, -(arr.length + 1));
        checkArrayForFloat(arr, NaN.toFloat());
        checkArray(arr, 0, false);
        checkArrayForFloat(arr, 0.0f);
    }
    checkNaNOnly(Float32Array.of(NaN));
    checkNaNOnly(Float32Array.of(NaN, NaN));
    checkNaNOnly(Float32Array.of(NaN, NaN, NaN));
}

function main(): int {
    let testSuite = new arktest.ArkTestsuite('typedArray.includes');
    testSuite.addTest('Float64 Arrays: includes for arrays w/o NaN', testF64ArrayNoNaN);
    testSuite.addTest('Float64 Arrays: includes for arrays w/o NaN with fromIndex', testF64ArrayNoNaNFromIndex);
    testSuite.addTest('Float64 Arrays: includes for arrays with Infinity', testF64ArrayInf);
    testSuite.addTest('Float64 Arrays: includes for arrays with NaN', testF64ArrayNaN);
    testSuite.addTest('Float64 Arrays: includes for arrays of NaN only', testF64ArrayNaNOnly);
    testSuite.addTest('Float32 Arrays: includes for arrays w/o NaN', testF32ArrayNoNaN);
    testSuite.addTest('Float32 Arrays: includes for arrays w/o NaN with fromIndex', testF32ArrayNoNaNFromIndex);
    testSuite.addTest('Float32 Arrays: includes for arrays with Infinity', testF32ArrayInf);
    testSuite.addTest('Float32 Arrays: includes for arrays with NaN', testF32ArrayNaN);
    testSuite.addTest('Float32 Arrays: includes for arrays of NaN only', testF32ArrayNaNOnly);
    return testSuite.run();
}
