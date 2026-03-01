/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import { Stack } from "@ohos.util.Stack";

class C1 {
    name: string = "";
    age: string = "";
}

function main() {
    const suite = new arktest.ArkTestsuite("Stack API tests");

    suite.addTest("Testing Stack locate operation with existing element", testStackLocateWithExistingElement);
    suite.addTest("Testing Stack locate operation with non-existing element", testStackLocateWithNonExistingElement);
    suite.addTest("Testing Stack locate operation with duplicate elements", testStackLocateWithDuplicateElements);
    suite.addTest("Testing Stack length property after push operations", testStackLengthAfterPush);
    suite.addTest("Testing Stack forEach iteration with single element", testStackForEachWithSingleElement);
    suite.addTest("Testing Stack forEach iteration with value and index validation",
        testStackForEachWithValueAndIndexValidation);

    return suite.run();
}

// Test cases ported from ArkTS 1.0: STACK_LOCATE_01100
function testStackLocateWithExistingElement() {
    let stack = new Stack<double | string | object>();
    let a: C1 = {
        name: "Dylon", age: "13"
    };
    let b = 'a';
    let c = 1;
    stack.push(a);
    stack.push(b);
    stack.push(c);
    let result = stack.locate(a);
    arktest.assertEQ(result, 0);
}

// Test cases ported from ArkTS 1.0: STACK_LOCATE_1200
function testStackLocateWithNonExistingElement() {
    let stack = new Stack<double>();
    stack.push(1);
    stack.push(2);
    stack.push(3);
    let result = stack.locate(4);
    arktest.assertEQ(result, -1);
}

// Test cases ported from ArkTS 1.0: STACK_LOCATE_1300
function testStackLocateWithDuplicateElements() {
    let stack = new Stack<double>();
    stack.push(1);
    stack.push(2);
    stack.push(3);
    stack.push(3);
    let result = stack.locate(3);
    arktest.assertEQ(result, 2);
}

// Test cases ported from ArkTS 1.0: STACK_PUSH_1500
function testStackLengthAfterPush() {
    let stack = new Stack<double>();
    stack.push(1);
    stack.push(2);
    stack.push(3);
    stack.push(3);
    arktest.assertEQ(stack.length, 4);
}

// Test cases ported from ArkTS 1.0: STACK_FOREACH_1600
function testStackForEachWithSingleElement() {
    let stack = new Stack<double>();
    stack.push(1);
    let count = 0;
    stack.forEach((value) => {
        arktest.assertEQ(stack.locate(value), 0);
        count++;
    });
    arktest.assertEQ(count, stack.length);
}

// Test cases ported from ArkTS 1.0: STACK_FOREACH_1700
function testStackForEachWithValueAndIndexValidation() {
    let stack = new Stack<double>();
    stack.push(1);
    let count = 0;
    stack.forEach((value, index, stack) => {
        arktest.assertEQ(stack.locate(value), 0);
        count++;
    });
    arktest.assertEQ(count, stack.length);
}
