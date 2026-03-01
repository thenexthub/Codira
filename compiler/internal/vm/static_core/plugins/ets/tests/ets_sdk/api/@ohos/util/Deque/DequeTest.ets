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

import Deque from "@ohos.util.Deque";
import { BusinessError } from "@ohos.base";

const TypeErrorCodeId: int = 401;

// Test cases ported from ArkTS 1.0: container_deque.js
function main() {
    const suite = new arktest.ArkTestsuite("Deque API tests");

    suite.addTest("Testing basic operations of Deque", testBasicOperations);
    suite.addTest("Testing popFirst and iteration in Deque", testPopFirstAndIteration);
    suite.addTest("Testing key iteration in Deque", testKeyIteration);

    return suite.run();
}

function testBasicOperations() {
    let deque = new Deque<double>();
    let testArray = new Array<double>();
    for (let i: int = 0; i < 10; i++) {
        testArray.push(i);
    }
    for (let i: int = 9; i >= 0; --i) {
        deque.insertFront(i);
    }

    for (let i: int = 0; i < testArray.length; i++) {
        arktest.assertEQ(deque[i], testArray[i]);
    }

    for (let i: int = 10; i < 20; i++) {
        deque.insertEnd(i);
        testArray.push(i);
    }

    for (let i: int = 0; i < testArray.length; i++) {
        arktest.assertEQ(deque[i], testArray[i]);
    }

    deque.forEach((i, d) => {
        arktest.assertEQ(d, testArray[i.toInt()]);
    });

    let j = 0;
    for (const data of deque) {
        arktest.assertEQ(data, testArray[j]);
        j++;
    }

    let itr = deque.$_iterator();
    let tmp = itr.next().value;
    let testArray1 = new Array<double>();
    do {
        testArray1.push(tmp!);
        tmp = itr.next().value;
    } while (tmp != undefined);

    for (let k = 0; k < deque.length; k++) {
        arktest.assertEQ(testArray1[k], testArray[k]);
    }

    arktest.assertTrue(deque.has(7), "Deque should contain the element 7");
    arktest.assertEQ(deque.getFirst(), 0);
    arktest.assertEQ(deque.getLast(), 19);
    arktest.assertEQ(deque.popFirst(), 0);
    arktest.assertEQ(deque.popLast(), 19);
}

function testPopFirstAndIteration() {
    let arr1 = new Array<double>();
    let myTest = new Deque<double>();
    for (let i: int = 0; i < 10; i++) {
        myTest.insertEnd(i);
    }
    for (let i: int = 0; i < 5; i++) {
        myTest.popFirst();
    }
    myTest.forEach((item: double, index: int, arr: Deque<double>) => {
        arr1.push(item);
    });
    arktest.assertEQ(myTest.length, 5);
    for (let j = 5; j < 10; j++) {
        arktest.assertEQ(arr1[j - 5], j);
    }
}

function testKeyIteration() {
    let test1 = new Deque<double>();
    for (let k = 0; k < 10; k++) {
        test1.insertEnd(k);
    }
    let keyName = "";
    let key: double = 0;
    for (key of test1) {
        keyName += key;
    }
    arktest.assertEQ(keyName, "0123456789");
}
