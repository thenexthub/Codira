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

function numberToStringTest1(): void {
    let n: Number = Number(233);
    let radix: Number = 16;
    arktest.assertEQ(n.toString(radix), 'e9', 'Number(233).toString(16) should be equal to e9');
}

function numberToStringTest2(): void {
    let n: Number = Number(233);
    let radix: Number = 16.99;
    arktest.assertEQ(n.toString(radix), 'e9', 'Number(233).toString(16.99) should be equal to e9');
}

function numberToStringTest3(): void {
    let n: Number = Number(42);
    arktest.assertEQ(n.toString(), '42', 'Number(42).toString() should be equal to 42');
}

function numberToStringTest4(): void {
    let n: Number = Number(10);
    let radix: Number = 2;
    arktest.assertEQ(n.toString(radix), '1010', 'Number(10).toString(2) should be equal to 1010');
}

function numberToStringTest5(): void {
    let n: Number = Number(255);
    let radix: Number = 16;
    arktest.assertEQ(n.toString(radix), 'ff', 'Number(255).toString(16) should be equal to ff');
}

function testNumberToExponential(): void {
    let n: number | undefined = 1
    let m: number = 1
    arktest.assertEQ(m.toExponential(n), '1.0e+0')
}

function testNumberToExponentialUndefined(): void {
    let m: number = 1
    arktest.assertEQ(m.toExponential(undefined), '1e+0')
}

function testNumberForParseFloat1(): void {
    let num1: double = Number.parseFloat('1e-9000000000000000')
    let num2: double = Number.parseFloat('-1e-9000000000000000')
    arktest.assertDoubleEQ(num1, 0.0, 0.000001)
    arktest.assertDoubleEQ(num2, 0.0, 0.000001)
}

function testNumberForParseFloat2(): void {
    let num1: double = Number.parseFloat('1e-100000')
    let num2: double = Number.parseFloat('-1e-100000')
    arktest.assertDoubleEQ(num1, 0.0, 0.000001)
    arktest.assertDoubleEQ(num2, 0.0, 0.000001)
}

function testNumberForParseFloat3(): void {
    let num1: double = Number.parseFloat('1e-5')
    let num2: double = Number.parseFloat('-1e-5')
    arktest.assertDoubleEQ(num1, 0.00001, 0.000001)
    arktest.assertDoubleEQ(num2, -0.00001, 0.000001)
}

function testNumberForParseFloat4(): void {
    let num1: double = Number.parseFloat('1e-0000000')
    let num2: double = Number.parseFloat('-1e-0000000')
    arktest.assertDoubleEQ(num1, 1.0, 0.000001)
    arktest.assertDoubleEQ(num2, -1.0, 0.000001)
}

function testNumberForParseFloat5(): void {
    let res = Double.parseFloat('9999999999999999999999n');
    arktest.assertDoubleEQ(res, 1e+22, 0.000001);

    res = Double.parseFloat('9999999999999999999999.');
    arktest.assertDoubleEQ(res, 1e+22, 0.000001);

    res = Double.parseFloat('9999999999999999999999');
    arktest.assertDoubleEQ(res, 1e+22, 0.000001);

    res = Double.parseFloat('999999999999999999999999999n');
    arktest.assertDoubleEQ(res, 1e+27, 0.000001);

    res = Double.parseFloat('1000000000000000000000sd');
    arktest.assertDoubleEQ(res, 1e+21, 0.000001);
}

function testNumberForParseFloat6(): void {
    let res = Double.parseFloat('1.7976931348623159e+308');
    arktest.assertEQ(res.toString(), 'Infinity');

    res = Double.parseFloat('-1.7976931348623159e+308');
    arktest.assertEQ(res.toString(), '-Infinity');

    res = Double.parseFloat('1.8976931348623159e+308');
    arktest.assertEQ(res.toString(), 'Infinity');

    res = Double.parseFloat('-1.8976931348623159e+308');
    arktest.assertEQ(res.toString(), '-Infinity');

    res = Double.parseFloat('2e+308');
    arktest.assertEQ(res.toString(), 'Infinity');

    res = Double.parseFloat('-2e+308');
    arktest.assertEQ(res.toString(), '-Infinity');
}

function main() {
    let myTestsuite = new arktest.ArkTestsuite('myTestsuite');
    myTestsuite.addTest('NumberToStringTest1', numberToStringTest1);
    myTestsuite.addTest('NumberToStringTest2', numberToStringTest2);
    myTestsuite.addTest('NumberToStringTest3', numberToStringTest3);
    myTestsuite.addTest('NumberToStringTest4', numberToStringTest4);
    myTestsuite.addTest('NumberToStringTest5', numberToStringTest5);
    myTestsuite.addTest('testNumberToExponential', testNumberToExponential);
    myTestsuite.addTest('testNumberToExponentialUndefined', testNumberToExponentialUndefined);
    myTestsuite.addTest('testNumberForParseFloat exceeding int16_t', testNumberForParseFloat1);
    myTestsuite.addTest('testNumberForParseFloat not exceeding int16_t', testNumberForParseFloat2);
    myTestsuite.addTest('testNumberForParseFloat normal number', testNumberForParseFloat3);
    myTestsuite.addTest('testNumberForParseFloat special', testNumberForParseFloat4);
    myTestsuite.addTest('testNumberForParseFloat big number', testNumberForParseFloat5);
    myTestsuite.addTest('testNumberForParseFloat Infinity', testNumberForParseFloat6);
    myTestsuite.run();
}
