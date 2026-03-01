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
    let LinkedBlockingQueueTestsuite = new arktest.ArkTestsuite("LinkedBlockingQueueCapacityTest");

    LinkedBlockingQueueTestsuite.addTest("CapacityTest001", () =>
    {
        let queueInt: containers.LinkedBlockingQueue<int> = new containers.LinkedBlockingQueue<int>(elementsNum);
        for (let i = 0; i < 5; i++) {
            arktest.assertEQ(queueInt.add(i), true);
        }
        arktest.assertEQ(queueInt.isEmpty(), false);
        arktest.assertEQ(queueInt.size, 5);
        arktest.assertEQ(queueInt.capacity, elementsNum);
        arktest.assertEQ(queueInt.add(testNum), true);
        arktest.assertEQ(queueInt.size, 6);
        arktest.assertEQ(queueInt.capacity, elementsNum);
    });
    LinkedBlockingQueueTestsuite.addTest("CapacityTest007", () =>
    {
        let queueInt: containers.LinkedBlockingQueue<int> = new containers.LinkedBlockingQueue<int>(stressNum);
        for (let i = 0; i < 10; i++) {
            arktest.assertEQ(queueInt.capacity, stressNum);
            queueInt.add(i);
        }
        for (let i = 10; i < 20; i++) {
            queueInt.push(i);
        }
        arktest.assertEQ(queueInt.capacity, stressNum);
    });
    LinkedBlockingQueueTestsuite.addTest("GetCapacityStressTest008", () =>
    {
        let queueInt: containers.LinkedBlockingQueue<int> = new containers.LinkedBlockingQueue<int>(stressNum);
        for (let i = 0; i < stressNum; i++) {
            queueInt.add(i);
            arktest.assertEQ(queueInt.capacity, stressNum);
        }
        arktest.assertEQ(queueInt.size, stressNum);
    });

    return LinkedBlockingQueueTestsuite.run();
}
