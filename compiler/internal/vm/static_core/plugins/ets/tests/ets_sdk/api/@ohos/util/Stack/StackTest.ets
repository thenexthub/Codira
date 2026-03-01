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

import { Stack } from "@ohos.util.Stack";

// Test cases ported from ArkTS 1.0: container_stack.js
function main() {
    const suite = new arktest.ArkTestsuite("Stack API tests");

    suite.addTest("Testing Stack basic operations including push, peek, and locate", testStackBasicOperations);
    suite.addTest("Testing Stack pop operation and length validation", testStackPopAndLengthValidation);
    suite.addTest("Testing Stack key iteration and key concatenation", testStackKeyIteration);

    return suite.run();
}

function testStackBasicOperations() {
    let stack = new Stack<double>();
    let testArray = new Array<double>();

    arktest.assertTrue(stack.isEmpty(), "The stack should be empty initially");

    for (let i: int = 0; i < 10; i++) {
        stack.push(i);
        testArray.push(i);
    }

    arktest.assertFalse(stack.isEmpty(), "The stack should not be empty after pushing elements");

    let count = 0;
    stack.forEach((i, d) => {
        arktest.assertEQ(d, testArray[i.toInt()]);
        count++;
    });
    arktest.assertEQ(count, stack.length);

    let j = 0;
    for (const data of stack) {
        arktest.assertEQ(data, testArray[j]);
        j++;
    }
    arktest.assertEQ(j, stack.length);

    arktest.assertEQ(stack.peek(), 9);
    arktest.assertEQ(stack.locate(5), 5);
}

function testStackPopAndLengthValidation() {
    let popStack = new Stack<double>();
    for (let i: int = 0; i < 10; i++) {
        popStack.push(i);
    }
    for (let i: int = 9; i >= 0; i--) {
        arktest.assertEQ(popStack.pop(), i);
        arktest.assertEQ(popStack.length, i);
    }
    arktest.assertTrue(popStack.isEmpty(), "The stack should be empty after popping all elements");
}

function testStackKeyIteration() {
    let test1 = new Stack<double>();
    for (let k = 0; k < 10; k++) {
        test1.push(k);
    }
    let keyName = "";
    let key: double = 0;
    for (key of test1) {
        keyName += key;
    }
    arktest.assertEQ(keyName, "0123456789");
}
