/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

import {Stack} from "@ohos.util.Stack";
import {BusinessError} from "@ohos.base";

function main() {
    const suite = new arktest.ArkTestsuite("Stack API tests");

    suite.addTest("Stack create an instance that stores single-type data", testCreateSingleTypeInstance);
    suite.addTest("Stack create an instance that stores multiple types of data", testCreateMultiTypeInstance);
    suite.addTest("Stack length", testStackLength);
    suite.addTest("Stack length with mixed types", testStackLengthMixedTypes);
    suite.addTest("Stack length with large double of elements", testStackLengthLargeNumber);
    suite.addTest("Stack push", testStackPush);
    suite.addTest("Stack pop first element from an empty stack", testPopFromEmptyStack);
    suite.addTest("Stack pop first element", testPopFromNonEmptyStack);
    suite.addTest("Stack peek with an empty stack", testPeekEmptyStack);
    suite.addTest("Stack peek", testPeekNonEmptyStack);

    return suite.run();
}

class Temp {
    name: string = "";
    age: double = 0;
}

class TestData {
    static readonly testNumber1: int = 1;
    static readonly testNumber2: int = 2;
    static readonly testNumber3: int = 3;
    static readonly testNumber4: int = 4;
    static readonly testNumber5: int = 5;
    static readonly testNumber6: int = 6;
    static readonly testNumber1w: int = 10000;
    static readonly testObject1: Temp = { name: "Dylon", age: 13 };
    static readonly testString1: string = "string";
    static readonly forNumber: Number[] = [17, 3, 22, 8, 29, 11, 5, 19, 2, 26, 14, 7, 28, 10, 1, 24, 16, 13,
        21, 6, 27, 18, 4, 25, 12, 23, 9, 15, 20, 30];
}

function testCreateSingleTypeInstance() {
    let stack: Stack<double>;
    arktest.expectNoThrow(() => {stack = new Stack<double>()});
}

function testCreateMultiTypeInstance() {
    let stack: Stack<double | string | object | boolean>;
    arktest.expectNoThrow(() => {stack = new Stack<double | string | object | boolean>()});
}

function testStackLength() {
    let stack = new Stack<double>();
    arktest.assertEQ(stack.length, 0, "Initial stack length should be 0");
    stack.push(TestData.testNumber3);
    stack.push(TestData.testNumber1);
    stack.push(TestData.testNumber2);
    arktest.assertEQ(stack.length, TestData.testNumber3, "Stack length should be 3 after adding three elements");
    stack.pop();
    stack.pop();
    arktest.assertEQ(stack.length, TestData.testNumber1, "Stack length should be 1 after popping two elements");
}

function testStackLengthMixedTypes() {
    let stack = new Stack<double | string | boolean | undefined>();
    arktest.assertEQ(stack.length, 0, "Initial stack length should be 0");
    stack.push(TestData.testNumber3);
    stack.push(TestData.testString1);
    stack.push(true);
    stack.push(undefined);
    arktest.assertEQ(stack.length, TestData.testNumber4, "Stack length should be 4 after adding four elements of mixed types");
    stack.pop();
    stack.pop();
    arktest.assertEQ(stack.length, TestData.testNumber2, "Stack length should be 2 after popping two elements");
}

function testStackLengthLargeNumber() {
    let stack = new Stack<double>();
    for (let i: int = 0; i < TestData.testNumber1w; i++) {
        stack.push(TestData.testNumber1);
    }
    arktest.assertEQ(stack.length, TestData.testNumber1w, "Stack length should be 10000 after adding elements");
}

function testStackPush() {
    let stack = new Stack<double>();
    initializeStack(stack, TestData.forNumber);
    let countForEach = 0;
    stack.forEach((value: double) => {
        arktest.assertEQ(value, TestData.forNumber[countForEach++], "Element should match");
    });
    arktest.assertEQ(countForEach, stack.length, "Stack length same as foreach");
    arktest.assertEQ(stack.length, TestData.forNumber.length, "Stack length same as TestData.forNumber");
}

function testPopFromEmptyStack() {
    let stack = new Stack<double>();
    arktest.assertEQ(stack.length, 0, "Initial stack length should be 0");
    try {
        let first = stack.pop();
    } catch (e) {
        if (!(e instanceof BusinessError)) {
            arktest.assertTrue(false);
        }
        arktest.assertEQ(e.message, "Container is empty");
    }
}

function testPopFromNonEmptyStack() {
    let stack = new Stack<double>();
    for (let i: int = TestData.testNumber1; i < TestData.testNumber6; i++) {
        stack.push(TestData.testNumber3);
    }
    stack.push(TestData.testNumber1);
    arktest.assertEQ(stack.length, TestData.testNumber6, "Stack length should be 6 after inserting six elements");
    arktest.assertEQ(stack.pop(), TestData.testNumber1, "Pop from non-empty stack should return last element");
    arktest.assertEQ(stack.length, TestData.testNumber5, "Pop from non-empty stack should update length");
    let countForEach = 0;
    stack.forEach((value: double) => {
        arktest.assertEQ(value, TestData.testNumber3, "Element should match");
        countForEach++;
    });
    arktest.assertEQ(countForEach, stack.length, "Stack length same as foreach");
}

function testPeekEmptyStack() {
    let stack = new Stack<double>();
    arktest.assertEQ(stack.length, 0, "Initial stack length should be 0");
    try {
        let first = stack.peek();
    } catch (e) {
        if (!(e instanceof BusinessError)) {
            arktest.assertTrue(false);
        }
        arktest.assertEQ(e.message, "Container is empty");
    }
}

function testPeekNonEmptyStack() {
    let stack = new Stack<double>();
    stack.push(TestData.testNumber1);
    stack.push(TestData.testNumber2);
    arktest.assertEQ(stack.peek(), TestData.testNumber2, "Peek should return the new top element");
    arktest.assertEQ(stack.length, TestData.testNumber2, "Stack length should be 2 after adding two elements");
}

function initializeStack<T>(stack: Stack<T>, sourceArray: T[]) {
    for (let i: int = 0; i < sourceArray.length; i++) {
        arktest.assertEQ(stack.push(sourceArray[i]), sourceArray[i], "The result should be the added element");
    }
}
