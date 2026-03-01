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
    const suite = new arktest.ArkTestsuite("RegExpExecArray Array-like Tests");

    // Basic access tests
    suite.addTest("$_get", testGet);
    suite.addTest("$_set", testSet);
    suite.addTest("length property", testLength);

    // Array manipulation methods
    suite.addTest("push() adds elements", testPush);
    suite.addTest("pop() returns the last element", testPopReturnsLast);
    suite.addTest("pop() reduces length", testPopReducesLength);
    suite.addTest("Multiple pop() calls remove elements in reverse order", testPopMultipleTimes);
    suite.addTest("pop() on empty array returns undefined", testPopEmptyReturnsUndefined);
    suite.addTest("shift() removes first element", testShift);
    suite.addTest("unshift() adds elements to beginning", testUnshift);

    // Array methods
    suite.addTest("slice() creates a copy", testSlice);
    suite.addTest("splice() modifies array", testSplice);
    suite.addTest("concat() combines arrays", testConcat);
    suite.addTest("join() creates string", testJoin);
    suite.addTest("reverse() reverses elements", testReverse);
    suite.addTest("sort() sorts elements", testSort);

    // Search methods
    suite.addTest("indexOf() finds element", testIndexOf);
    suite.addTest("lastIndexOf() finds last occurrence", testLastIndexOf);
    suite.addTest("includes() checks presence", testIncludes);

    // Higher-order methods
    suite.addTest("filter() filters elements", testFilter);
    suite.addTest("map() transforms elements", testMap);
    suite.addTest("find() finds element", testFind);
    suite.addTest("findIndex() finds element index", testFindIndex);
    suite.addTest("every() tests all elements", testEvery);
    suite.addTest("some() tests any element", testSome);
    suite.addTest("reduce() accumulates values", testReduce);
    suite.addTest("forEach() iterates elements", testForEach);

    // Modern array methods
    suite.addTest("at() accesses by index", testAt);
    suite.addTest("copyWithin() copies elements", testCopyWithin);
    suite.addTest("fill() fills with value", testFill);
    suite.addTest("toSorted() creates sorted copy", testToSorted);
    suite.addTest("toReversed() creates reversed copy", testToReversed);
    suite.addTest("with() creates copy with replaced element", testWith);

    // Iterator methods
    suite.addTest("values() iterator", testValues);
    suite.addTest("keys() iterator", testKeys);
    suite.addTest("entries() iterator", testEntries);

    return suite.run();
}

// Basic access tests

/**
 * Test: $_get test.
 */
function testGet(): void {
    let match = new RegExpExecArray(0, "abc", ["x", "y", "z"]);

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
    let match = new RegExpExecArray(0, "abc", ["x", "y", "z"]);

    match[0] = "x1"
    arktest.assertEQ(match[0], "x1", "testSet, int index, step 1");

    match[1] = "y1"
    arktest.assertEQ(match[1], "y1", "testSet, int index, step 2");

    match[2] = "z1"
    arktest.assertEQ(match[2], "z1", "testSet, int index, step 3");

    arktest.expectError((): void => { match[-1] = "a" }, new RangeError("Out of bounds"))
    arktest.expectError((): void => { match[match.length] = "b" }, new RangeError("Out of bounds"))
}

/**
 * Test: length property.
 */
function testLength(): void {
    let match1 = new RegExpExecArray(0, "abc", []);
    arktest.assertEQ(match1.length, 0, "empty array length");

    let match2 = new RegExpExecArray(0, "abc", ["a"]);
    arktest.assertEQ(match2.length, 1, "single element length");

    let match3 = new RegExpExecArray(0, "abc", ["a", "b", "c", "d", "e"]);
    arktest.assertEQ(match3.length, 5, "multiple elements length");
}

// Array manipulation methods

/**
 * Test: push() adds elements.
 */
function testPush(): void {
    let match = new RegExpExecArray(0, "abc", ["a", "b"]);
    let newLength = match.push("c");

    arktest.assertEQ(newLength, 3, "push returns new length");
    arktest.assertEQ(match.length, 3, "length updated after push");
    arktest.assertEQ(match[2], "c", "new element added");

    match.push("d", "e");
    arktest.assertEQ(match.length, 5, "multiple push");
    arktest.assertEQ(match[3], "d", "first multiple element");
    arktest.assertEQ(match[4], "e", "second multiple element");
}

/**
 * Test: pop() should return the last element in result array.
 */
function testPopReturnsLast(): void {
    let match = new RegExpExecArray(0, "abc", ["a", "b", "c"]);
    let popped = match.pop();
    arktest.assertEQ(popped, "c", "testPopReturnsLast");
}

/**
 * Test: pop() should reduce result array length.
 */
function testPopReducesLength(): void {
    let match = new RegExpExecArray(0, "abc", ["a", "b", "c"]);
    match.pop();
    arktest.assertEQ(match.length, 2, "testPopReducesLength");
}

/**
 * Test: multiple pop() calls should remove elements in reverse order.
 */
function testPopMultipleTimes(): void {
    let match = new RegExpExecArray(0, "abc", ["x", "y", "z"]);
    arktest.assertEQ(match.pop(), "z", "testPopMultipleTimes step 1");
    arktest.assertEQ(match.pop(), "y", "testPopMultipleTimes step 2");
    arktest.assertEQ(match.pop(), "x", "testPopMultipleTimes step 3");
}

/**
 * Test: pop() on empty result should return undefined.
 */
function testPopEmptyReturnsUndefined(): void {
    let match = new RegExpExecArray(0, "abc", []);
    let popped = match.pop();
    arktest.assertEQ(popped, undefined, "testPopEmptyReturnsUndefined");
}

/**
 * Test: shift() removes first element.
 */
function testShift(): void {
    let match = new RegExpExecArray(0, "abc", ["a", "b", "c"]);
    let shifted = match.shift();

    arktest.assertEQ(shifted, "a", "shift returns first element");
    arktest.assertEQ(match.length, 2, "length reduced after shift");
    arktest.assertEQ(match[0], "b", "remaining elements shifted");
    arktest.assertEQ(match[1], "c", "remaining elements shifted");

    let emptyMatch = new RegExpExecArray(0, "abc", []);
    let emptyShift = emptyMatch.shift();
    arktest.assertEQ(emptyShift, undefined, "shift on empty returns undefined");
}


/**
 * Test: unshift() adds elements to beginning.
 */
function testUnshift(): void {
    let match = new RegExpExecArray(0, "abc", ["b", "c"]);
    let newLength = match.unshift("a");

    arktest.assertEQ(newLength, 3, "unshift returns new length");
    arktest.assertEQ(match.length, 3, "length updated after unshift");
    arktest.assertEQ(match[0], "a", "new element at beginning");
    arktest.assertEQ(match[1], "b", "existing elements shifted");
    arktest.assertEQ(match[2], "c", "existing elements shifted");

    match[1] = "y1"
    arktest.assertEQ(match[1], "y1", "testSet, int index, step 2");

    match[2] = "z1"
    arktest.assertEQ(match[2], "z1", "testSet, int index, step 3");

    arktest.expectError((): void => { match[-1] = "a" }, new RangeError("Out of bounds"))
    arktest.expectError((): void => { match[match.length] = "b" }, new RangeError("Out of bounds"))
    match.unshift("x", "y");
    arktest.assertEQ(match.length, 5, "multiple unshift");
    arktest.assertEQ(match[0], "x", "first multiple element");
    arktest.assertEQ(match[1], "y", "second multiple element");
}

// Array methods

/**
 * Test: slice() creates a copy.
 */
function testSlice(): void {
    let match = new RegExpExecArray(0, "abc", ["a", "b", "c", "d", "e"]);

    let slice1 = match.slice(1, 3);
    arktest.assertEQ(slice1.length, 2, "slice length");
    arktest.assertEQ(slice1[0], "b", "slice first element");
    arktest.assertEQ(slice1[1], "c", "slice second element");

    let slice2 = match.slice(2);
    arktest.assertEQ(slice2.length, 3, "slice from index");
    arktest.assertEQ(slice2[0], "c", "slice from index first");

    // Original should be unchanged
    arktest.assertEQ(match.length, 5, "original unchanged");
}

/**
 * Test: splice() modifies array.
 */

function testSplice(): void {
    let match = new RegExpExecArray(0, "abc", ["a", "b", "c", "d", "e"]);
    // TypeError: Reference to splice is ambiguous, will be fixed later
    // let removed = match.splice(1, 2, "x", "y");
}

/**
 * Test: concat() combines arrays.
 */
function testConcat(): void {
    let match = new RegExpExecArray(0, "abc", ["a", "b"]);
    let result = match.concat(["c", "d"] as Array<String | undefined>);

    arktest.assertEQ(result.length, 4, "concat length");
    arktest.assertEQ(result[0], "a", "original elements");
    arktest.assertEQ(result[1], "b", "original elements");
    arktest.assertEQ(result[2], "c", "concatenated elements");
    arktest.assertEQ(result[3], "d", "concatenated elements");

    // Original should be unchanged
    arktest.assertEQ(match.length, 2, "original unchanged");
}

/**
 * Test: join() creates string.
 */
function testJoin(): void {
    let match = new RegExpExecArray(0, "abc", ["a", "b", "c"]);

    let joined1 = match.join(",");
    arktest.assertEQ(joined1, "a,b,c", "join with separator");

    let joined2 = match.join();
    arktest.assertTrue(joined2.includes("a"), "join without separator contains elements");
}

/**
 * Test: reverse() reverses elements.
 */
function testReverse(): void {
    let match = new RegExpExecArray(0, "abc", ["a", "b", "c"]);
    let result = match.reverse();

    arktest.assertEQ(result, match, "reverse returns same object");
    arktest.assertEQ(match[0], "c", "first element after reverse");
    arktest.assertEQ(match[1], "b", "middle element after reverse");
    arktest.assertEQ(match[2], "a", "last element after reverse");
}

/**
 * Test: sort() sorts elements.
 */
function testSort(): void {
    let match = new RegExpExecArray(0, "abc", ["c", "a", "b"]);
    let result = match.sort();

    arktest.assertEQ(result, match, "sort returns same object");
    arktest.assertEQ(match[0], "a", "first element after sort");
    arktest.assertEQ(match[1], "b", "second element after sort");
    arktest.assertEQ(match[2], "c", "third element after sort");
}

// Search methods

/**
 * Test: indexOf() finds element.
 */
function testIndexOf(): void {
    let match = new RegExpExecArray(0, "abc", ["a", "b", "c", "b"]);

    arktest.assertEQ(match.indexOf("b"), 1, "indexOf finds first occurrence");
    arktest.assertEQ(match.indexOf("d"), -1, "indexOf returns -1 for not found");
    arktest.assertEQ(match.indexOf("b", 2), 3, "indexOf with start index");
}

/**
 * Test: lastIndexOf() finds last occurrence.
 */
function testLastIndexOf(): void {
    let match = new RegExpExecArray(0, "abc", ["a", "b", "c", "b"]);

    arktest.assertEQ(match.lastIndexOf("b"), 3, "lastIndexOf finds last occurrence");
    arktest.assertEQ(match.lastIndexOf("d"), -1, "lastIndexOf returns -1 for not found");
    arktest.assertEQ(match.lastIndexOf("b", 2), 1, "lastIndexOf with end index");
}

/**
 * Test: includes() checks presence.
 */
function testIncludes(): void {
    let match = new RegExpExecArray(0, "abc", ["a", "b", "c"]);

    arktest.assertTrue(match.includes("b"), "includes finds element");
    arktest.assertFalse(match.includes("d"), "includes returns false for not found");
    arktest.assertTrue(match.includes("c", 2), "includes with start index");
    arktest.assertFalse(match.includes("a", 2), "includes with start index not found");
}

// Higher-order methods

/**
 * Test: filter() filters elements.
 */
function testFilter(): void {
    let match = new RegExpExecArray(0, "abc", ["apple", "banana", "apricot", "cherry"]);

    let filtered = match.filter((value: String | undefined, index: int, array: Array<String | undefined>) => {
        return value!.startsWith("a");
    });

    arktest.assertEQ(filtered.length, 2, "filter result length");
    arktest.assertEQ(filtered[0], "apple", "filtered first element");
    arktest.assertEQ(filtered[1], "apricot", "filtered second element");
}

/**
 * Test: map() transforms elements.
 */
function testMap(): void {
    let match = new RegExpExecArray(0, "abc", ["a", "b", "c"]);

    let mapped = match.map((value: String | undefined, index: int, array: Array<String | undefined>) => {
        return value!.toUpperCase();
    });

    arktest.assertEQ(mapped.length, 3, "map result length");
    arktest.assertEQ(mapped[0], "A", "mapped first element");
    arktest.assertEQ(mapped[1], "B", "mapped second element");
    arktest.assertEQ(mapped[2], "C", "mapped third element");
}

/**
 * Test: find() finds element.
 */
function testFind(): void {
    let match = new RegExpExecArray(0, "abc", ["apple", "banana", "apricot"]);

    let found = match.find((value: String | undefined, index: int, array: Array<String | undefined>) => {
        return value!.startsWith("b");
    });

    arktest.assertEQ(found, "banana", "find returns element");

    let notFound = match.find((value: String | undefined, index: int, array: Array<String | undefined>) => {
        return value!.startsWith("z");
    });

    arktest.assertEQ(notFound, undefined, "find returns undefined for not found");
}

/**
 * Test: findIndex() finds element index.
 */
function testFindIndex(): void {
    let match = new RegExpExecArray(0, "abc", ["apple", "banana", "apricot"]);

    let foundIndex = match.findIndex((value: String | undefined, index: int, array: Array<String | undefined>) => {
        return value!.startsWith("b");
    });

    arktest.assertEQ(foundIndex, 1, "findIndex returns index");

    let notFoundIndex = match.findIndex((value: String | undefined, index: int, array: Array<String | undefined>) => {
        return value!.startsWith("z");
    });

    arktest.assertEQ(notFoundIndex, -1, "findIndex returns -1 for not found");
}

/**
 * Test: every() tests all elements.
 */
function testEvery(): void {
    let match = new RegExpExecArray(0, "abc", ["apple", "apricot", "avocado"]);

    let allStartWithA = match.every((value: String | undefined, index: int, array: Array<String | undefined>) => {
        return value!.startsWith("a");
    });

    arktest.assertTrue(allStartWithA, "every returns true when all match");

    let allStartWithB = match.every((value: String | undefined, index: int, array: Array<String | undefined>) => {
        return value!.startsWith("b");
    });

    arktest.assertFalse(allStartWithB, "every returns false when not all match");
}

/**
 * Test: some() tests any element.
 */
function testSome(): void {
    let match = new RegExpExecArray(0, "abc", ["apple", "banana", "cherry"]);

    let someStartWithB = match.some((value: String | undefined, index: int, array: Array<String | undefined>) => {
        return value!.startsWith("b");
    });

    arktest.assertTrue(someStartWithB, "some returns true when any match");

    let someStartWithZ = match.some((value: String | undefined, index: int, array: Array<String | undefined>) => {
        return value!.startsWith("z");
    });

    arktest.assertFalse(someStartWithZ, "some returns false when none match");
}

/**
 * Test: reduce() accumulates values.
 */
function testReduce(): void {
    let match = new RegExpExecArray(0, "abc", ["a", "b", "c"]);

    let concatenated = match.reduce((prev: String | undefined, curr: String | undefined, index: int, array: Array<String | undefined>) => {
        return prev! + curr!;
    });

    arktest.assertEQ(concatenated, "abc", "reduce concatenates");

    let withInitial = match.reduce((prev: String | undefined, curr: String | undefined, index: int, array: Array<String | undefined>) => {
        return prev! + curr!;
    }, "start:");

    arktest.assertEQ(withInitial, "start:abc", "reduce with initial value");
}

/**
 * Test: forEach() iterates elements.
 */
function testForEach(): void {
    let match = new RegExpExecArray(0, "abc", ["a", "b", "c"]);
    let result = "";

    match.forEach((value: String | undefined, index: int, array: Array<String | undefined>) => {
        result += value! + index.toString();
    });

    arktest.assertEQ(result, "a0b1c2", "forEach iterates correctly");
}

// Modern array methods

/**
 * Test: at() accesses by index.
 */
function testAt(): void {
    let match = new RegExpExecArray(0, "abc", ["a", "b", "c"]);

    arktest.assertEQ(match.at(0), "a", "at positive index");
    arktest.assertEQ(match.at(-1), "c", "at negative index");
}

/**
 * Test: copyWithin() copies elements.
 */
function testCopyWithin(): void {
    let match = new RegExpExecArray(0, "abc", ["a", "b", "c", "d", "e"]);
    let result = match.copyWithin(0, 3, 5);

    arktest.assertEQ(result, match, "copyWithin returns same object");
    arktest.assertEQ(match[0], "d", "copied element");
    arktest.assertEQ(match[1], "e", "copied element");
    arktest.assertEQ(match[2], "c", "unchanged element");
}

/**
 * Test: fill() fills with value.
 */
function testFill(): void {
    let match = new RegExpExecArray(0, "abc", ["a", "b", "c", "d", "e"]);
    let result = match.fill("x", 1, 3);

    arktest.assertEQ(result, match, "fill returns same object");
    arktest.assertEQ(match[0], "a", "unchanged element");
    arktest.assertEQ(match[1], "x", "filled element");
    arktest.assertEQ(match[2], "x", "filled element");
    arktest.assertEQ(match[3], "d", "unchanged element");
}

/**
 * Test: toSorted() creates sorted copy.
 */
function testToSorted(): void {
    let match = new RegExpExecArray(0, "abc", ["c", "a", "b"]);
    let sorted = match.toSorted();

    arktest.assertEQ(sorted.length, 3, "toSorted length");
    arktest.assertEQ(sorted[0], "a", "sorted first element");
    arktest.assertEQ(sorted[1], "b", "sorted second element");
    arktest.assertEQ(sorted[2], "c", "sorted third element");

    // Original should be unchanged
    arktest.assertEQ(match[0], "c", "original unchanged");
}

/**
 * Test: toReversed() creates reversed copy.
 */
function testToReversed(): void {
    let match = new RegExpExecArray(0, "abc", ["a", "b", "c"]);
    let reversed = match.toReversed();

    arktest.assertEQ(reversed.length, 3, "toReversed length");
    arktest.assertEQ(reversed[0], "c", "reversed first element");
    arktest.assertEQ(reversed[1], "b", "reversed second element");
    arktest.assertEQ(reversed[2], "a", "reversed third element");

    // Original should be unchanged
    arktest.assertEQ(match[0], "a", "original unchanged");
}

/**
 * Test: with() creates copy with replaced element.
 */
function testWith(): void {
    let match = new RegExpExecArray(0, "abc", ["a", "b", "c"]);
    let withReplaced = match.with(1, "x");

    arktest.assertEQ(withReplaced.length, 3, "with length");
    arktest.assertEQ(withReplaced[0], "a", "unchanged element");
    arktest.assertEQ(withReplaced[1], "x", "replaced element");
    arktest.assertEQ(withReplaced[2], "c", "unchanged element");

    // Original should be unchanged
    arktest.assertEQ(match[1], "b", "original unchanged");
}

// Iterator methods

/**
 * Test: values() iterator.
 */
function testValues(): void {
    let match = new RegExpExecArray(0, "abc", ["a", "b", "c"]);
    let iterator = match.values();

    let result1 = iterator.next();
    arktest.assertFalse(result1.done, "first iteration not done");
    arktest.assertEQ(result1.value, "a", "first value");

    let result2 = iterator.next();
    arktest.assertFalse(result2.done, "second iteration not done");
    arktest.assertEQ(result2.value, "b", "second value");

    let result3 = iterator.next();
    arktest.assertFalse(result3.done, "third iteration not done");
    arktest.assertEQ(result3.value, "c", "third value");

    let result4 = iterator.next();
    arktest.assertTrue(result4.done, "fourth iteration done");
}

/**
 * Test: keys() iterator.
 */
function testKeys(): void {
    let match = new RegExpExecArray(0, "abc", ["a", "b", "c"]);
    let iterator = match.keys();

    let result1 = iterator.next();
    arktest.assertFalse(result1.done, "first key iteration not done");
    arktest.assertEQ(result1.value, 0, "first key");

    let result2 = iterator.next();
    arktest.assertFalse(result2.done, "second key iteration not done");
    arktest.assertEQ(result2.value, 1, "second key");
}

/**
 * Test: entries() iterator.
 */
function testEntries(): void {
    let match = new RegExpExecArray(0, "abc", ["a", "b"]);
    let iterator = match.entries();

    let result1 = iterator.next();
    arktest.assertFalse(result1.done, "first entry iteration not done");
    arktest.assertEQ(result1!.value![0], 0, "first entry key");
    arktest.assertEQ(result1!.value![1], "a", "first entry value");

    let result2 = iterator.next();
    arktest.assertFalse(result2.done, "second entry iteration not done");
    arktest.assertEQ(result2!.value![0], 1, "second entry key");
    arktest.assertEQ(result2!.value![1], "b", "second entry value");
}
