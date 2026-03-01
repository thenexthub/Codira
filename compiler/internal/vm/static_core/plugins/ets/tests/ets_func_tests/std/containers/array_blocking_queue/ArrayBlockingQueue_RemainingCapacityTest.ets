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

const elementsNum: int = 10;
const testNum: int = 20;
const stressNum: int = 100;

function main(): int {
    let ArrayBlockingQueueTestsuite = new arktest.ArkTestsuite("ArrayBlockingQueueRemainingCapacityTest");

    ArrayBlockingQueueTestsuite.addTest("RemainingCapacityTest001", () =>
    {
        let queueInt: containers.ArrayBlockingQueue<int> = new containers.ArrayBlockingQueue<int>(elementsNum);
        for (let i = 0; i < 5; i++) {
            arktest.assertEQ(queueInt.add(i), true);
        }
        arktest.assertEQ(queueInt.isEmpty(), false);
        arktest.assertEQ(queueInt.size, 5);
        arktest.assertEQ(queueInt.remainingCapacity(), 5);
        arktest.assertEQ(queueInt.add(testNum), true);
        arktest.assertEQ(queueInt.size, 6);
        arktest.assertEQ(queueInt.remainingCapacity(), 4);
    });
    ArrayBlockingQueueTestsuite.addTest("RemainingCapacityTest006", () =>
    {
        let queueInt: containers.ArrayBlockingQueue<int> = new containers.ArrayBlockingQueue<int>(stressNum);
        for (let i = 0; i < 10; i++) {
            arktest.assertEQ(queueInt.remainingCapacity(), stressNum - i);
            queueInt.add(i);
        }
        for (let i = 10; i < 20; i++) {
            queueInt.push(i);
        }
        arktest.assertEQ(queueInt.remainingCapacity(), stressNum - 20);
    });
    ArrayBlockingQueueTestsuite.addTest("RemainingCapacityStressTest009", () =>
    {
        let queueInt: containers.ArrayBlockingQueue<int> = new containers.ArrayBlockingQueue<int>(stressNum);
        let j: int = stressNum;
        for (let i = 0; i < stressNum; i++) {
            queueInt.add(i);
            j--;
            arktest.assertEQ(queueInt.remainingCapacity(), j);
        }
        arktest.assertEQ(queueInt.size, stressNum);
    });

    return ArrayBlockingQueueTestsuite.run();
}
