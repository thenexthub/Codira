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

function getTestName(arrayType: string): string {
    return arrayType + " meta $_set/$_get";
}

function testInt8ArrayMetaSetGet(): void {
    const arr = new Int8Array(4);
    arr.$_set(0, 12);
    arr.$_set(3, -34);
    arktest.assertEQ(arr.$_get(0).toDouble(), 12);
    arktest.assertEQ(arr.$_get(3).toDouble(), -34);
}

function testInt16ArrayMetaSetGet(): void {
    const arr = new Int16Array(4);
    arr.$_set(1, 1234);
    arr.$_set(2, -5678);
    arktest.assertEQ(arr.$_get(1).toDouble(), 1234);
    arktest.assertEQ(arr.$_get(2).toDouble(), -5678);
}

function testInt32ArrayMetaSetGet(): void {
    const arr = new Int32Array(3);
    arr.$_set(0, 1_000_000);
    arr.$_set(2, -2_000_000);
    arktest.assertEQ(arr.$_get(0).toDouble(), 1000000);
    arktest.assertEQ(arr.$_get(2).toDouble(), -2000000);
}

function testBigInt64ArrayMetaSetGet(): void {
    const arr = new BigInt64Array(2);
    arr.$_set(0, 9007199254740991n);
    arr.$_set(1, -9007199254740991n);
    arktest.assertEQ(9007199254740991n, arr.$_get(0));
    arktest.assertEQ(-9007199254740991n, arr.$_get(1));
}

function testFloat32ArrayMetaSetGet(): void {
    const arr = new Float32Array(2);
    arr.$_set(0, 3.14);
    arr.$_set(1, -2.718);
    const a0 = arr.$_get(0).toDouble();
    const a1 = arr.$_get(1).toDouble();
    const epsilon = 1e-6;
    arktest.assertTrue(Math.abs(a0 - 3.14) < epsilon);
    arktest.assertTrue(Math.abs(a1 + 2.718) < epsilon);
}

function testFloat64ArrayMetaSetGet(): void {
    const arr = new Float64Array(2);
    arr.$_set(0, 1.23456789);
    arr.$_set(1, -9.87654321);
    arktest.assertEQ(arr.$_get(0).toDouble(), 1.23456789);
    arktest.assertEQ(arr.$_get(1).toDouble(), -9.87654321);
}

function main(): int {
    const suite = new arktest.ArkTestsuite("TypedArrayMetaSetGet");
    suite.addTest(getTestName("Int8Array"), testInt8ArrayMetaSetGet);
    suite.addTest(getTestName("Int16Array"), testInt16ArrayMetaSetGet);
    suite.addTest(getTestName("Int32Array"), testInt32ArrayMetaSetGet);
    suite.addTest(getTestName("BigInt64Array"), testBigInt64ArrayMetaSetGet);
    suite.addTest(getTestName("Float32Array"), testFloat32ArrayMetaSetGet);
    suite.addTest(getTestName("Float64Array"), testFloat64ArrayMetaSetGet);
    return suite.run();
}