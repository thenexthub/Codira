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

import {Queue} from "@ohos.util.Queue";
import { BusinessError } from "@ohos.base";

class C1 {
    name: string = "";
    age: string = "";
}

function main() {
    const suite = new arktest.ArkTestsuite("Queue API tests");

    suite.addTest("Testing Queue add with double index", testQueueAddWithNumberIndex);
    suite.addTest("Testing Queue add with string index", testQueueAddWithStringIndex);
    suite.addTest("Testing Queue add with object index", testQueueAddWithObjectIndex);
    suite.addTest("Testing Queue add with mixed types (object, double, string)", testQueueAddWithMixedTypes);
    suite.addTest("Testing Queue pop operation with elements", testQueuePopWithElements);
    suite.addTest("Testing Queue pop operation without elements", testQueuePopWithoutElements);
    suite.addTest("Testing Queue getFirst operation with elements", testQueueGetFirstWithElements);
    suite.addTest("Testing Queue getFirst operation without elements", testQueueGetFirstWithoutElements);
    suite.addTest("Testing Queue get and pop operation with undefined elements", testQueueGetPopWithUndefinedElements);

    return suite.run();
}

// Test cases ported from ArkTS 1.0: QUEUE_ADD_0100
function testQueueAddWithNumberIndex() {
    let queue = new Queue<double>();
    let result = queue.add(1);
    arktest.assertTrue(result, "Adding a double to the queue should return true");
}

// Test cases ported from ArkTS 1.0: QUEUE_ADD_0200
function testQueueAddWithStringIndex() {
    let queue = new Queue<string>();
    let result = queue.add("a");
    arktest.assertTrue(result, "Adding a string to the queue should return true");
}

// Test cases ported from ArkTS 1.0: QUEUE_ADD_0300
function testQueueAddWithObjectIndex() {
    let queue = new Queue<object>();
    let a: C1 = {
        name: "Dylon", age: "13"
    };
    let result = queue.add(a);
    arktest.assertTrue(result, "Adding an object to the queue should return true");
}

// Test cases ported from ArkTS 1.0: QUEUE_ADD_0400
function testQueueAddWithMixedTypes() {
    let queue = new Queue<double | string | object>();
    let a: C1 = {
        name: "Dylon", age: "13"
    };
    let b = 'a';
    let c = 1;
    let result1 = queue.add(a);
    let result2 = queue.add(b);
    let result3 = queue.add(c);
    arktest.assertTrue(result1, "Adding an object to the queue should return true");
    arktest.assertTrue(result2, "Adding a string to the queue should return true");
    arktest.assertTrue(result3, "Adding a double to the queue should return true");
}

// Test cases ported from ArkTS 1.0: QUEUE_POP_0600
function testQueuePopWithElements() {
    let queue = new Queue<double | string | object>();
    let a: C1 = {
        name: "Dylon", age: "13"
    };
    let b = 'a';
    let c = 1;
    queue.add(a);
    queue.add(b);
    queue.add(c);
    let result = queue.pop();
    arktest.assertEQ(result, a);
}

// Test cases ported from ArkTS 1.0: QUEUE_POP_0700
function testQueuePopWithoutElements() {
    let queue = new Queue<double | string | object>();
    try {
        let result = queue.pop();
    } catch (e) {
        if (!(e instanceof BusinessError)) {
            arktest.assertTrue(false);
        }
        arktest.assertEQ(e.message, "Container is empty");
    }
}

// Test cases ported from ArkTS 1.0: QUEUE_GETFIRST_0800
function testQueueGetFirstWithElements() {
    let queue = new Queue<double | string | object>();
    let a: C1 = {
        name: "Dylon", age: "13"
    };
    let b = 'a';
    let c = 1;
    queue.add(a);
    queue.add(b);
    queue.add(c);
    let result = queue.getFirst();
    arktest.assertEQ(result, a);
}

// Test cases ported from ArkTS 1.0: QUEUE_GETFIRST_0900
function testQueueGetFirstWithoutElements() {
    let queue = new Queue<double | string | object>();
    try {
        let result = queue.getFirst();
    } catch (e) {
        if (!(e instanceof BusinessError)) {
            arktest.assertTrue(false);
        }
        arktest.assertEQ(e.message, "Container is empty");
    }
}

function testQueueGetPopWithUndefinedElements () {
    let queue = new Queue<undefined>();
    queue.add(undefined);
    let result = queue.getFirst();
    arktest.assertEQ(result, undefined);
    let result1 = queue.pop();
    arktest.assertEQ(result1, undefined);
}
