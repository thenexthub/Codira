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

import Deque from "@ohos.util.Deque";

function main() {
    const suite = new arktest.ArkTestsuite("Deque API tests");

    suite.addTest("Test popLast and verify the new last element", testPopLastAndVerifyNewLast);
    suite.addTest("Test popLast after expansion and reset of rear to zero", testPopLastAfterExpansionAndReset);

    return suite.run();
}

// Test cases ported from ArkTS 1.0: DEQUE_1400
function testPopLastAndVerifyNewLast() {
    let deque = new Deque<double>();
    deque.insertFront(1);
    deque.insertFront(2);
    deque.insertEnd(4);
    deque.insertFront(3);
    deque.popLast();
    let result = deque.getLast();
    arktest.assertEQ(result, 1, "The result should be 1");
}

function testPopLastAfterExpansionAndReset() {
    let deque = new Deque<double>();
    deque.insertFront(1);
    deque.insertFront(2);
    deque.insertEnd(8);
    deque.insertFront(3);
    deque.insertFront(4);
    deque.insertFront(5);
    deque.insertEnd(9);
    deque.insertFront(6);
    deque.insertFront(7);
    deque.popLast();
    deque.popLast();
    let result = deque.getLast();
    arktest.assertEQ(result, 1, "The result should be 1");
}
