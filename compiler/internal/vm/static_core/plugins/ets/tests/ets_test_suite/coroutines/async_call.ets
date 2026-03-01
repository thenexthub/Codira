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

// Test async bridge
// Check all parameters are passed correctly for different architectures

let called: boolean = false;

type L = StdDebug.Logger

function assert_eq(value1: int, value2: int): void {
    if (value1 == value2) {
        return;
    }
    L.log('Values of type int are not equal: ' + value1 + ' != ' + value2);
    throw new Error();
}

function assert_eq(value1: double, value2: double): void {
    if (value1 == value2) {
        return;
    }
    L.log('Values of type double are not equal: ' + value1 + ' != ' + value2);
    throw new Error();
}

function prepare(): void {
    called = false;
}

// check no-args static function call
async function foo(): Promise<Object | null> {
    called = true;
    return null;
}

// check function call when all arguments are in registers on x86-64
async function amd64_small(i1: int, i2: int, i3: int, i4: int, i5: int, f1: double, f2: double, f3: double, f4: double, f5: double, f6: double, f7: double, f8: double): Promise<Object | null> {
    called = true;
    assert_eq(i1, 1);
    assert_eq(i2, 2);
    assert_eq(i3, 3);
    assert_eq(i4, 4);
    assert_eq(i5, 5);
    assert_eq(f1, 1.0);
    assert_eq(f2, 2.0);
    assert_eq(f3, 3.0);
    assert_eq(f4, 4.0);
    assert_eq(f5, 5.0);
    assert_eq(f6, 6.0);
    assert_eq(f7, 7.0);
    assert_eq(f8, 8.0);
    return null;
}

// check function call when some arguments are passed via the stack on x86-64
async function amd64_large(i1: int, i2: int, i3: int, i4: int, i5: int, i6: int, f1: double, f2: double, f3: double, f4: double, f5: double, f6: double, f7: double, f8: double, f9: double): Promise<Object | null> {
    called = true;
    assert_eq(i1, 1);
    assert_eq(i2, 2);
    assert_eq(i3, 3);
    assert_eq(i4, 4);
    assert_eq(i5, 5);
    assert_eq(i6, 6);
    assert_eq(f1, 1.0);
    assert_eq(f2, 2.0);
    assert_eq(f3, 3.0);
    assert_eq(f4, 4.0);
    assert_eq(f5, 5.0);
    assert_eq(f6, 6.0);
    assert_eq(f7, 7.0);
    assert_eq(f8, 8.0);
    assert_eq(f9, 9.0);
    return null;
}

// check function call when all arguments are in registers on arm32
async function arm32_small(i1: int, i2: int, i3: int): Promise<Object | null> {
    called = true;
    assert_eq(i1, 1);
    assert_eq(i2, 2);
    assert_eq(i3, 3);
    return null;
}

// check function call when some arguments are passed via the stack on arm32
async function arm32_large(i1: int, i2: int, i3: int, i4: int, i5: int): Promise<Object | null> {
    called = true;
    assert_eq(i1, 1);
    assert_eq(i2, 2);
    assert_eq(i3, 3);
    assert_eq(i4, 4);
    assert_eq(i5, 5);
    return null;
}

// check function call when all arguments are in registers on arm32hf
async function arm32hf_small(i1: int, i2: int, i3: int, f1: double, f2: double, f3: double, f4: double, f5: double, f6: double, f7: double, f8: double): Promise<Object | null> {
    called = true;
    assert_eq(i1, 1);
    assert_eq(i2, 2);
    assert_eq(i3, 3);
    assert_eq(f1, 1.0);
    assert_eq(f2, 2.0);
    assert_eq(f3, 3.0);
    assert_eq(f4, 4.0);
    assert_eq(f5, 5.0);
    assert_eq(f6, 6.0);
    assert_eq(f7, 7.0);
    assert_eq(f8, 8.0);
    return null;
}

// check function call when some arguments are passed via the stack on arm32hf
async function arm32hf_large(i1: int, i2: int, i3: int, i4: int, f1: double, f2: double, f3: double, f4: double, f5: double, f6: double, f7: double, f8: double, f9: double): Promise<Object | null> {
    called = true;
    assert_eq(i1, 1);
    assert_eq(i2, 2);
    assert_eq(i3, 3);
    assert_eq(i4, 4);
    assert_eq(f1, 1.0);
    assert_eq(f2, 2.0);
    assert_eq(f3, 3.0);
    assert_eq(f4, 4.0);
    assert_eq(f5, 5.0);
    assert_eq(f6, 6.0);
    assert_eq(f7, 7.0);
    assert_eq(f8, 8.0);
    assert_eq(f9, 9.0);
    return null;
}

// check function call when all arguments are in registers on arm64
async function arm64_small(i1: int, i2: int, i3: int, i4: int, i5: int, i6: int, i7: int, f1: double, f2: double, f3: double, f4: double, f5: double, f6: double, f7: double, f8: double): Promise<Object | null> {
    called = true;
    assert_eq(i1, 1);
    assert_eq(i2, 2);
    assert_eq(i3, 3);
    assert_eq(i4, 4);
    assert_eq(i5, 5);
    assert_eq(i6, 6);
    assert_eq(i7, 7);
    assert_eq(f1, 1.0);
    assert_eq(f2, 2.0);
    assert_eq(f3, 3.0);
    assert_eq(f4, 4.0);
    assert_eq(f5, 5.0);
    assert_eq(f6, 6.0);
    assert_eq(f7, 7.0);
    assert_eq(f8, 8.0);
    return null;
}

// check function call when some arguments are passed via the stack on arm64
async function arm64_large(i1: int, i2: int, i3: int, i4: int, i5: int, i6: int, i7: int, i8: int, f1: double, f2: double, f3: double, f4: double, f5: double, f6: double, f7: double, f8: double, f9: double): Promise<Object | null> {
    called = true;
    assert_eq(i1, 1);
    assert_eq(i2, 2);
    assert_eq(i3, 3);
    assert_eq(i4, 4);
    assert_eq(i5, 5);
    assert_eq(i6, 6);
    assert_eq(i7, 7);
    assert_eq(i8, 8);
    assert_eq(f1, 1.0);
    assert_eq(f2, 2.0);
    assert_eq(f3, 3.0);
    assert_eq(f4, 4.0);
    assert_eq(f5, 5.0);
    assert_eq(f6, 6.0);
    assert_eq(f7, 7.0);
    assert_eq(f8, 8.0);
    assert_eq(f9, 9.0);
    return null;
}

async function crasher(): Promise<Int> {
    return await Promise.resolve(1);
}

// check async/await in global context
try {
    let x = crasher();
} catch (e) {
    arktest.assertTrue(e instanceof InvalidCoroutineOperationError);
}

async function testNoArgsStaticFuncCall(): Promise<void> {
    prepare();
    let p = foo();
    arktest.assertTrue(called);
    await p;
}

async function testAmd64SmallFuncCall(): Promise<void> {
    prepare();
    let p = amd64_small(1, 2, 3, 4, 5, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);
    arktest.assertTrue(called);
    await p;
}

async function testAmd64LargeFuncCall(): Promise<void> {
    prepare();
    let p = amd64_large(1, 2, 3, 4, 5, 6, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0);
    arktest.assertTrue(called);
    await p;
}

async function testArm32SmallFuncCall(): Promise<void> {
    prepare();
    let p = arm32_small(1, 2, 3);
    arktest.assertTrue(called);
    await p;
}

async function testArm32LargeFuncCall(): Promise<void> {
    prepare();
    let p = arm32_large(1, 2, 3, 4, 5);
    arktest.assertTrue(called);
    await p;
}

async function testArm32hfSmallFuncCall(): Promise<void> {
    prepare();
    let p = arm32hf_small(1, 2, 3, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);
    arktest.assertTrue(called);
    await p;
}

async function testArm32hfLargeFuncCall(): Promise<void> {
    prepare();
    let p = arm32hf_large(1, 2, 3, 4, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0);
    arktest.assertTrue(called);
    await p;
}

async function testArm64SmallFuncCall(): Promise<void> {
    prepare();
    let p = arm64_small(1, 2, 3, 4, 5, 6, 7, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);
    arktest.assertTrue(called);
    await p;
}

async function testArm64LargeFuncCall(): Promise<void> {
    prepare();
    let p = arm64_large(1, 2, 3, 4, 5, 6, 7, 8, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0);
    arktest.assertTrue(called);
    await p;
}

function main(): int {
    let testSuite = new arktest.ArkTestsuite('coroutines.async_call');
    testSuite.addAsyncTest('testNoArgsStaticFuncCall', testNoArgsStaticFuncCall);
    testSuite.addAsyncTest('testAmd64SmallFuncCall', testAmd64SmallFuncCall);
    testSuite.addAsyncTest('testAmd64LargeFuncCall', testAmd64LargeFuncCall);
    testSuite.addAsyncTest('testArm32SmallFuncCall', testArm32SmallFuncCall);
    testSuite.addAsyncTest('testArm32LargeFuncCall', testArm32LargeFuncCall);
    testSuite.addAsyncTest('testArm32hfSmallFuncCall', testArm32hfSmallFuncCall);
    testSuite.addAsyncTest('testArm32hfLargeFuncCall', testArm32hfLargeFuncCall);
    testSuite.addAsyncTest('testArm64SmallFuncCall', testArm64SmallFuncCall);
    testSuite.addAsyncTest('testArm64LargeFuncCall', testArm64LargeFuncCall);
    let res = testSuite.run();
    return res;
}
