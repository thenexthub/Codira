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
import { BusinessError } from "@ohos.base";

class C1 {
    name: string = "";
    age: string = "";
}

function main() {
    const suite = new arktest.ArkTestsuite("Stack API tests");

    suite.addTest("Testing Stack push operation with double", testStackPushWithNumber);
    suite.addTest("Testing Stack push operation with string", testStackPushWithString);
    suite.addTest("Testing Stack push operation with object", testStackPushWithObject);
    suite.addTest("Testing Stack push operation with types (object, string, double)", testStackPushWithMixedTypes);
    suite.addTest("Testing Stack pop operation with multiple elements", testStackPopWithMultipleElements);
    suite.addTest("Testing Stack pop operation without elements", testStackPopWithoutElements);
    suite.addTest("Testing Stack peek operation with elements", testStackPeekWithElements);
    suite.addTest("Testing Stack peek operation without elements", testStackPeekWithoutElements);

    return suite.run();
}

// Test cases ported from ArkTS 1.0: STACK_PUSH_0100
function testStackPushWithNumber() {
    let stack = new Stack<double>();
    let result = stack.push(1);
    arktest.assertEQ(result, 1);
}

// Test cases ported from ArkTS 1.0: STACK_PUSH_0200
function testStackPushWithString() {
    let stack = new Stack<string>();
    let result = stack.push("a");
    arktest.assertEQ(result, "a");
}

// Test cases ported from ArkTS 1.0: STACK_PUSH_0300
function testStackPushWithObject() {
    let stack = new Stack<object>();
    let a: C1 = {
        name: "Dylon", age: "13"
    };
    let result = stack.push(a);
    arktest.assertEQ(result, a);
}

// Test cases ported from ArkTS 1.0: STACK_PUSH_0400
function testStackPushWithMixedTypes() {
    let stack = new Stack<double | string | object>();
    let a: C1 = {
        name: "Dylon", age: "13"
    };
    let b = 'a';
    let c = 1;
    let result1 = stack.push(a);
    let result2 = stack.push(b);
    let result3 = stack.push(c);
    arktest.assertEQ(result1, a);
    arktest.assertEQ(result2, b);
    arktest.assertEQ(result3, c);
}

// Test cases ported from ArkTS 1.0: STACK_POP_0600
function testStackPopWithMultipleElements() {
    let stack = new Stack<double | string | object>();
    let a: C1 = {
        name: "Dylon", age: "13"
    };
    let b = 'a';
    let c = 1;
    stack.push(a);
    stack.push(b);
    stack.push(c);
    arktest.assertEQ(stack.pop(), c);
    arktest.assertEQ(stack.pop(), b);
    arktest.assertEQ(stack.pop(), a);
    arktest.assertEQ(stack.length, 0);
}

// Test cases ported from ArkTS 1.0: STACK_POP_0700
function testStackPopWithoutElements() {
    let stack = new Stack<double | string | object>();
    try {
        let first = stack.pop();
    } catch (e) {
        if (!(e instanceof BusinessError)) {
            arktest.assertTrue(false);
        }
        arktest.assertEQ(e.message, "Container is empty");
    }
}

// Test cases ported from ArkTS 1.0: STACK_PEEK_0900
function testStackPeekWithElements() {
    let stack = new Stack<double | string | object>();
    let a: C1 = {
        name: "Dylon", age: "13"
    };
    let b = 'a';
    let c = 1;
    stack.push(a);
    stack.push(b);
    stack.push(c);
    let result = stack.peek();
    arktest.assertEQ(result, c);
    arktest.assertEQ(stack.length, 3);
}

// Test cases ported from ArkTS 1.0: STACK_PEEK_0100
function testStackPeekWithoutElements() {
    let stack = new Stack<double | string | object>();
    try {
        let first = stack.peek();
    } catch (e) {
        if (!(e instanceof BusinessError)) {
            arktest.assertTrue(false);
        }
        arktest.assertEQ(e.message, "Container is empty");
    }
}
