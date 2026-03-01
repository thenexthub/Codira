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

import {TreeSet} from "@ohos.util.TreeSet";
import { BusinessError } from "@ohos.base";

function main(): int {
    const suite = new arktest.ArkTestsuite("TreeSet PopFirst PopLast Iterator API tests")

    suite.addTest("TreeSet popFirst popLast first and last value",
        testPopFirstPopLastValidValues)
    suite.addTest("TreeSet popFirst popLast first and last value from the null treeSet",
        testPopFirstPopLastEmptyTreeSet)
    suite.addTest("TreeSet popFirst popLast first and last value from the TreeSet with a set of data",
        testPopFirstPopLastMultipleValues)
    suite.addTest("TreeSet popFirst popLast first and last value by descending",
        testPopFirstPopLastDescendingOrder)
    suite.addTest("TreeSet popFirst popLast first and last value after clear data",
        testPopFirstPopLastAfterClear)
    suite.addTest("TreeSet popFirst popLast first and last undefined",
        testPopFirstPopLastUndefined)
    suite.addTest("TreeSet $_iterator return iterator",
        testIterator)
    suite.addTest("TreeSet $_iterator return iterator with null treeSet",
        testIteratorEmptyTreeSet)

    return suite.run()
}

function testPopFirstPopLastValidValues() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): double => { return secondValue.compareTo(firstValue) });
    treeSet.add("aa");
    treeSet.add("bb");
    treeSet.add("cc");
    treeSet.add("dd")
    let result_popFirst = treeSet.popFirst();
    let result_has_first = treeSet.has("aa");
    let result_popLast = treeSet.popLast();
    let result_has_last = treeSet.has("dd");
    arktest.assertEQ(result_popFirst, "aa", "The treeSet should pop the first value");
    arktest.assertEQ(result_has_first, false, "The treeSet should not contain the popped first value");
    arktest.assertEQ(result_popLast, "dd", "The treeSet should pop the last value");
    arktest.assertEQ(result_has_last, false, "The treeSet should not contain the popped last value");
    arktest.assertEQ(treeSet.length, 2, "The treeSet length should be 1");
}

function testPopFirstPopLastEmptyTreeSet() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): double => { return secondValue.compareTo(firstValue) });
    try {
        let result_popFirst = treeSet.popFirst();
    } catch (e) {
        if (!(e instanceof BusinessError)) {
            arktest.assertTrue(false);
        }
        arktest.assertEQ(e.message, "Container is empty");
    }
    try {
        let result_popLast = treeSet.popLast();
    } catch (e) {
        if (!(e instanceof BusinessError)) {
            arktest.assertTrue(false);
        }
        arktest.assertEQ(e.message, "Container is empty");
    }
    arktest.assertEQ(treeSet.length, 0, "The treeSet length should be 0")
}

function testPopFirstPopLastMultipleValues() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): double => { return secondValue.compareTo(firstValue) });
    treeSet.add("aa");
    try {
        let result_popFirst = treeSet.popFirst();
    } catch (e) {
        if (!(e instanceof BusinessError)) {
            arktest.assertTrue(false);
        }
        arktest.assertEQ(e.message, "Container is empty");
    }
    try {
        let result_popLast = treeSet.popLast();
    } catch (e) {
        if (!(e instanceof BusinessError)) {
            arktest.assertTrue(false);
        }
        arktest.assertEQ(e.message, "Container is empty");
    }
    arktest.assertEQ(treeSet.length, 0, "The treeSet length should be 0");
}

function testPopFirstPopLastDescendingOrder() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): double => { return firstValue.compareTo(secondValue) });
    treeSet.add("b");
    treeSet.add("c");
    treeSet.add("a");
    treeSet.add("d");
    let result_popFirst = treeSet.popFirst();
    let result_popLast = treeSet.popLast();
    arktest.assertEQ(result_popFirst, "d", "The treeSet should pop the first value");
    arktest.assertEQ(result_popLast, "a", "The treeSet should pop the last value");
    arktest.assertEQ(treeSet.length, 2, "The treeSet length should be 2");
}

function testPopFirstPopLastAfterClear() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): double => { return firstValue.compareTo(secondValue) });
    treeSet.add("squirrel");
    treeSet.add("sparrow");
    treeSet.clear();
    let result1 = treeSet.isEmpty();
    try {
        let result_popFirst = treeSet.popFirst();
    } catch (e) {
        if (!(e instanceof BusinessError)) {
            arktest.assertTrue(false);
        }
        arktest.assertEQ(e.message, "Container is empty");
    }
    try {
        let result_popLast = treeSet.popLast();
    } catch (e) {
        if (!(e instanceof BusinessError)) {
            arktest.assertTrue(false);
        }
        arktest.assertEQ(e.message, "Container is empty");
    }
    let result2 = treeSet.has("squirrel");
    let result3 = treeSet.has("sparrow");
    arktest.assertEQ(result1, true, "The treeSet should be empty");
    arktest.assertEQ(result2, false, "The treeSet should not contain the value that is not set");
    arktest.assertEQ(result3, false, "The treeSet should not contain the value that is not set");
    arktest.assertEQ(treeSet.length, 0, "The treeSet length should be 0");
}

function testPopFirstPopLastUndefined() {
    let treeSet: TreeSet<undefined> = new TreeSet<undefined>();
    treeSet.add(undefined);
    let result1 = treeSet.popFirst();
    arktest.assertEQ(result1, undefined);
    treeSet.add(undefined);
    let result2 = treeSet.popLast();
    arktest.assertEQ(result2, undefined);
}

function testIterator() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): double => { return secondValue.compareTo(firstValue) });
    treeSet.add("aa");
    treeSet.add("bb");
    let it = treeSet.$_iterator();
    arktest.assertEQ(it.next().value, "aa", "The treeSet $_iterator should return the first value");
    arktest.assertEQ(it.next().value, "bb", "The treeSet $_iterator should return the second value");
    arktest.assertTrue(it.next().done, "The treeSet iterator.done should true for no more values");
}

function testIteratorEmptyTreeSet() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): double => { return secondValue.compareTo(firstValue) });
    let it = treeSet.$_iterator();
    arktest.assertTrue(it.next().done, "The treeSet iterator.done should true when treeSet is empty");
}
