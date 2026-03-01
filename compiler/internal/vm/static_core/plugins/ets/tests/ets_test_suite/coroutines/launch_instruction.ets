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

import { launch } from "std/concurrency"

function noarg(): Object | null {
    return null;
}

function onearg(x: int): null {
    arktest.assertEQ(x, 1);
    return null;
}

function twoargs(x: int, y: int): null {
    arktest.assertEQ(x, 1);
    arktest.assertEQ(y, 2);
    return null;
}

function threeargs(x: int, y: int, z: int): null {
    arktest.assertEQ(x, 1);
    arktest.assertEQ(y, 2);
    arktest.assertEQ(z, 3);
    return null;
}

function fourargs(x: int, y: int, z: int, w: int): null {
    arktest.assertEQ(x, 1);
    arktest.assertEQ(y, 2);
    arktest.assertEQ(z, 3);
    arktest.assertEQ(w, 4);
    return null;
}

function fiveargs(x: int, y: int, z: int, w: int, v: int): null {
    arktest.assertEQ(x, 1);
    arktest.assertEQ(y, 2);
    arktest.assertEQ(z, 3);
    arktest.assertEQ(w, 4);
    arktest.assertEQ(v, 5);
    return null;
}

function testLaunchNoArgs(): void {
    launch<Object | null , () => Object | null>(noarg);
    Coroutine.Schedule();
}

function testLaunchOneArgs(): void {
    launch<Object | null , (x: int) => Object | null>(onearg, 1);
    Coroutine.Schedule();
}

function testLaunchTwoArgs(): void {
    launch<Object | null , (x1: int, x2: int) => Object | null>(twoargs, 1, 2);
    Coroutine.Schedule();
}

function testLaunchThreeArgs(): void {
    launch<Object | null , (x1: int, x2: int, x3: int) => Object | null>(threeargs, 1, 2, 3);
    Coroutine.Schedule();
}

function testLaunchFourArgs(): void {
    launch<Object | null , (x1: int, x2: int, x3: int, x4: int) => Object | null>(fourargs, 1, 2, 3, 4);
    Coroutine.Schedule();
}

function testLaunchFiveArgs(): void {
    launch<Object | null , (x1: int, x2: int, x3: int, x4: int, x5: int) => Object | null>(fiveargs, 1, 2, 3, 4, 5);
    Coroutine.Schedule();
}

function main(): int {
    // These tests must be run with exactly one coroutine worker thread due to it's design.
    let testSuite = new arktest.ArkTestsuite('coroutines.launch_instruction');
    testSuite.addTest('testLaunchNoArgs', testLaunchNoArgs);
    testSuite.addTest('testLaunchOneArgs', testLaunchOneArgs);
    testSuite.addTest('testLaunchTwoArgs', testLaunchTwoArgs);
    testSuite.addTest('testLaunchThreeArgs', testLaunchThreeArgs);
    testSuite.addTest('testLaunchFourArgs', testLaunchFourArgs);
    testSuite.addTest('testLaunchFiveArgs', testLaunchFiveArgs);
    let res = testSuite.run();
    return res;
}
