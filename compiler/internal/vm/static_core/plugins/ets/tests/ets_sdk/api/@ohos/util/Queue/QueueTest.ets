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

import { Queue } from "@ohos.util.Queue";

// Test cases ported from ArkTS 1.0: container_queue.js
function main() {
    const suite = new arktest.ArkTestsuite("Queue API tests");

    suite.addTest("Testing Queue basic operations", testQueueBasicOperations);
    suite.addTest("Testing Queue pop and iteration", testQueuePopAndIteration);

    return suite.run();
}

function testQueueBasicOperations() {
    let queue = new Queue<double>();
    let testArray = new Array<double>();
    for (let i: int = 0; i < 10; i++) {
        arktest.assertTrue(queue.add(i), `Adding element ${i} to the queue should return true`);
        testArray.push(i);
    }

    queue.forEach((i, d) => {
        arktest.assertEQ(d, testArray[i.toInt()]);
    });

    let j = 0;
    for (const data of queue) {
        arktest.assertEQ(data, testArray[j]);
        j++;
    }
    arktest.assertEQ(j, queue.length);

    arktest.assertEQ(queue.getFirst(), 0);
    arktest.assertEQ(queue.pop(), 0);
}

function testQueuePopAndIteration() {
    let myTest = new Queue<double>();
    let arr1 = new Array<double>();
    for (let i: int = 0; i < 10; i++) {
        myTest.add(i);
    }
    for (let i: int = 0; i < 5; i++) {
        myTest.pop();
    }
    myTest.forEach((item: double, index: int, arr: Queue<double>) => {
        arr1.push(item);
    });
    for (let j = 5; j < 10; j++) {
        arktest.assertEQ(arr1[j - 5], j);
    }

    let test1 = new Queue<double>();
    for (let k = 0; k < 10; k++) {
        test1.add(k);
    }
    let keyName = "";
    let key: double = 0;
    for (key of test1) {
        keyName += key;
    }
    arktest.assertEQ(keyName, "0123456789");
}
