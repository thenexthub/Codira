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

import {Stack} from "@ohos.util.Stack";

function main() {
    const suite = new arktest.ArkTestsuite("Stack API tests");

    suite.addTest("Stack locate with unique element", testLocateWithUniqueElement);
    suite.addTest("Stack locate with repeated element", testLocateWithRepeatedElement);
    suite.addTest("Stack locate with inexistent element", testLocateWithInexistentElement);
    suite.addTest("Stack forEach with no parameter function", testForEachNoParameter);
    suite.addTest("Stack forEach with function with value", testForEachWithValue);
    suite.addTest("Stack forEach with function with value and index", testForEachWithValueAndIndex);
    suite.addTest("Stack forEach with function with value, index and stack", testForEachWithValueIndexAndStack);
    suite.addTest("Stack forEach with changing the elements", testForEachWithChangingElements);
    suite.addTest("Stack isEmpty with empty container", testIsEmptyWithEmpty);
    suite.addTest("Stack isEmpty with non-empty container", testIsEmptyWithNonEmpty);
    suite.addTest("Stack iterate with empty stack", testIteratorWithEmptyStack);
    suite.addTest("Stack iterate", testIterator);
    suite.addTest("Stack interface composition testing", testInterfaceComposition);

    return suite.run();
}

class TestData {
    static readonly testNumber1: int = 1;
    static readonly testNumber2: int = 2;
    static readonly testNumber3: int = 3;
    static readonly testNumber4: int = 4;
    static readonly testNumber5: int = 5;
    static readonly testNumber6: int = 6;
    static readonly testNumber8: int = 8;
    static readonly forNumber: Number[] = [17, 3, 22, 8, 29, 11, 5, 19, 2, 26, 14, 7, 28, 10, 1, 24, 16, 13,
        21, 6, 27, 18, 4, 25, 12, 23, 9, 15, 20, 30];
}

function testLocateWithUniqueElement() {
    let stack = new Stack<double>();
    stack.push(TestData.testNumber1);
    stack.push(TestData.testNumber2);
    stack.push(TestData.testNumber3);
    stack.push(TestData.testNumber2);
    stack.push(TestData.testNumber4);
    arktest.assertEQ(stack.locate(TestData.testNumber1), 0, "Locate should return the correct index of the element");
    arktest.assertEQ(stack.locate(TestData.testNumber4), stack.length - TestData.testNumber1,
        "Locate should return the correct index of the element");
}

function testLocateWithRepeatedElement() {
    let stack = new Stack<double>();
    stack.push(TestData.testNumber1);
    stack.push(TestData.testNumber2);
    stack.push(TestData.testNumber3);
    stack.push(TestData.testNumber2);
    stack.push(TestData.testNumber4);
    arktest.assertEQ(stack.locate(TestData.testNumber2), TestData.testNumber1,
        "Locate should return the correct index of the element");

}

function testLocateWithInexistentElement() {
    let stack = new Stack<double>();
    stack.push(TestData.testNumber1);
    stack.push(TestData.testNumber2);
    stack.push(TestData.testNumber3);
    stack.push(TestData.testNumber2);
    stack.push(TestData.testNumber4);
    arktest.assertEQ(stack.locate(TestData.testNumber5), -TestData.testNumber1, "Locate of inexistent element should be -1");
}

function testForEachNoParameter() {
    let stack = new Stack<double>();
    initializeStack(stack, TestData.forNumber);
    let forEachCount: int = 0;
    stack.forEach(() => {
        forEachCount++;
    });
    arktest.assertEQ(forEachCount, stack.length, "ForEach count should match stack length");
}

function testForEachWithValue() {
    let stack = new Stack<double>();
    initializeStack(stack, TestData.forNumber);
    let forEachCount: int = 0;
    stack.forEach((value: double) => {
        arktest.assertEQ(value, TestData.forNumber[forEachCount], "ForEach should iterate elements in correct order");
        forEachCount++;
    });
    arktest.assertEQ(forEachCount, TestData.forNumber.length, "ForEach count should match stack length");
}

function testForEachWithValueAndIndex() {
    let stack = new Stack<double>();
    initializeStack(stack, TestData.forNumber);
    let forEachCount: int = 0;
    stack.forEach((value: double, index: int) => {
        arktest.assertEQ(value, TestData.forNumber[forEachCount], "ForEach should iterate elements in correct order");
        arktest.assertEQ(index, forEachCount, "ForEach should iterate elements in correct order");
        forEachCount++;
    });
    arktest.assertEQ(forEachCount, TestData.forNumber.length, "ForEach count should match stack length");
}

function testForEachWithValueIndexAndStack() {
    let stack = new Stack<double>();
    initializeStack(stack, TestData.forNumber);
    let forEachCount: int = 0;
    stack.forEach((value: double, index: int, tempStack: Stack<double>) => {
        arktest.assertEQ(stack, tempStack, "TempStack should be this");
        arktest.assertEQ(value, TestData.forNumber[forEachCount], "ForEach should iterate elements in correct order");
        arktest.assertEQ(index, forEachCount, "ForEach should iterate elements in correct order");
        arktest.assertEQ(stack.locate(value), tempStack.locate(value), "Locate should return the correct index of the element");

        forEachCount++;
    });
    arktest.assertEQ(forEachCount, TestData.forNumber.length, "ForEach count should match stack length");
}

function testForEachWithChangingElements() {
    let stack = new Stack<double>();
    initializeStack(stack, TestData.forNumber);

    stack.forEach((value: double) => {
        value = 0;
    });

    let forEachCount: int = 0;
    stack.forEach((value: double) => {
        arktest.assertEQ(value, TestData.forNumber[forEachCount], "ForEach should iterate elements in correct order");
        forEachCount++;
    });
}

function testIsEmptyWithEmpty() {
    let stack = new Stack<double>();
    arktest.assertTrue(stack.isEmpty(), "The container should be empty");
}

function testIsEmptyWithNonEmpty() {
    let stack = new Stack<double>();
    initializeStack(stack, TestData.forNumber);
    arktest.assertFalse(stack.isEmpty(), "The container should not be empty");
}

function testIteratorWithEmptyStack() {
    let stack = new Stack<double>();
    let elem: double = 0;
    for (let item of stack) {
        elem = TestData.testNumber1;
    }
    let iter = stack.$_iterator();
    let temp: IteratorResult<double> = iter.next();
    while (!temp.done) {
        elem = TestData.testNumber1;
    }
    arktest.assertEQ(elem, 0, "Iterator over empty stack should not iterate any elements");
}

function testIterator() {
    let stack = new Stack<double>();
    for (let i: int = TestData.testNumber1; i < TestData.testNumber6; i++) {
        stack.push(i);
    }
    let elem1: int = TestData.testNumber1;
    for (let item of stack) {
        arktest.assertEQ(item, elem1++, "Iterator should iterate elements in correct order");
    }
    arktest.assertEQ(elem1, TestData.testNumber6, "Iterator should iterate all elements correctly");
    let iter = stack.$_iterator();
    let temp: IteratorResult<double> = iter.next();
    let elem2: int = TestData.testNumber1;
    while (!temp.done) {
        arktest.assertEQ(temp.value, elem2++, "Iterator should iterate elements in correct order");
        temp = iter.next();
    }
    arktest.assertEQ(elem2, TestData.testNumber6, "Iterator should iterate all elements correctly");
}

function testInterfaceComposition() {
    let stack = new Stack<double>();
    arktest.assertTrue(stack.isEmpty(), "Stack is empty");

    initializeStack(stack, TestData.forNumber);
    arktest.assertFalse(stack.isEmpty(), "Stack is not empty after adding elements");

    arktest.assertEQ(stack.peek(), stack.pop(), "Pop first from stack should return first element");
    arktest.assertEQ(stack.locate(TestData.testNumber2), TestData.testNumber8,
        "Locate should return the correct index of the element");

    let forEachCount: int = 0;
    stack.forEach(() => {
        forEachCount++;
    });
    arktest.assertEQ(forEachCount, stack.length, "ForEach count should match stack length");
}

function initializeStack<T>(stack: Stack<T>, sourceArray: T[]) {
    for (let i: int = 0; i < sourceArray.length; i++) {
        arktest.assertEQ(stack.push(sourceArray[i]), sourceArray[i], "The result should be the added element");
    }
}
