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

function main(): int {
    const suite = new arktest.ArkTestsuite("RegExpMatchArray pop() Tests");

    suite.addTest("pop() returns the last element", testPopReturnsLast);
    suite.addTest("pop() reduces length", testPopReducesLength);
    suite.addTest("Multiple pop() calls remove elements in reverse order", testPopMultipleTimes);
    suite.addTest("pop() on empty array returns undefined", testPopEmptyReturnsUndefined);
    suite.addTest("$_get", testGet);
    suite.addTest("$_set", testSet);

    return suite.run();
}

/**
 * Test: pop() should return the last element in result array.
 */
function testPopReturnsLast(): void {
    let match = new RegExpMatchArray(0, "abc", ["a", "b", "c"]);
    let popped = match.pop();
    arktest.assertEQ(popped, "c", "testPopReturnsLast");
}

/**
 * Test: pop() should reduce result array length.
 */
function testPopReducesLength(): void {
    let match = new RegExpMatchArray(0, "abc", ["a", "b", "c"]);
    match.pop();
    arktest.assertEQ(match.result.length, 2, "testPopReducesLength");
}

/**
 * Test: multiple pop() calls should remove elements in reverse order.
 */
function testPopMultipleTimes(): void {
    let match = new RegExpMatchArray(0, "abc", ["x", "y", "z"]);
    arktest.assertEQ(match.pop(), "z", "testPopMultipleTimes step 1");
    arktest.assertEQ(match.pop(), "y", "testPopMultipleTimes step 2");
    arktest.assertEQ(match.pop(), "x", "testPopMultipleTimes step 3");
}

/**
 * Test: pop() on empty result should return undefined.
 */
function testPopEmptyReturnsUndefined(): void {
    let match = new RegExpMatchArray(0, "abc", []);
    let popped = match.pop();
    arktest.assertEQ(popped, undefined, "testPopEmptyReturnsUndefined");
}

/**
 * Test: $_get test.
 */
function testGet(): void {
    let match = new RegExpMatchArray(0, "abc", ["x", "y", "z"]);

    arktest.assertEQ(match[0], "x", "testGet, int index, step 1");

    arktest.assertEQ(match[1], "y", "testGet, int index, step 2");

    arktest.assertEQ(match[2], "z", "testGet, int index, step 3");

    arktest.expectError((): void => { match[-1] }, new RangeError("Out of bounds"))
    arktest.expectError((): void => { match[match.length] }, new RangeError("Out of bounds"))
}

/**
 * Test: $_set test.
 */
function testSet(): void {
    let match = new RegExpMatchArray(0, "abc", ["x", "y", "z"]);

    match[0] = "x1"
    arktest.assertEQ(match[0], "x1", "testSet, int index, step 1");

    match[1] = "y1"
    arktest.assertEQ(match[1], "y1", "testSet, int index, step 2");

    match[2] = "z1"
    arktest.assertEQ(match[2], "z1", "testSet, int index, step 3");

    arktest.expectError((): void => { match[-1] = "a" }, new RangeError("Out of bounds"))
    arktest.expectError((): void => { match[match.length] = "b" }, new RangeError("Out of bounds"))
}
