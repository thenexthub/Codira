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

let flag: int = 0;
const elementsNum: int = 10;
const stressNum: int = 100;
const arr: Number[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9];

function main(): int {
    let LinkedBlockingQueueTestsuite = new arktest.ArkTestsuite("LinkedBlockingQueueConstructorTest");

    LinkedBlockingQueueTestsuite.addTest("ConstructorTest001", () =>
    {
        let queueInt: containers.LinkedBlockingQueue<int> = new containers.LinkedBlockingQueue<int>();
        arktest.assertEQ(queueInt.isEmpty(), true);
        arktest.assertEQ(queueInt.size, 0);
    });
    LinkedBlockingQueueTestsuite.addTest("ConstructorTest002", () =>
    {
        let queueInt: containers.LinkedBlockingQueue<int> = new containers.LinkedBlockingQueue<int>(elementsNum);
        arktest.assertEQ(queueInt.isEmpty(), true);
        arktest.assertEQ(queueInt.size, 0);
        arktest.assertEQ(queueInt.capacity, elementsNum);
    });
    LinkedBlockingQueueTestsuite.addTest("ConstructorTest003", () =>
    {
        let array = Array.from<number>(arr);
        let ConcurrentSetTest: containers.ConcurrentSet<number> = new containers.ConcurrentSet<number>(array);
        arktest.assertEQ(ConcurrentSetTest.size, 10);
        flag = 0;
        ConcurrentSetTest.forEach((val: number) => {
            arktest.assertEQ(val, flag++);
        });
        let iter = ConcurrentSetTest.keys();
        let queueInt: containers.LinkedBlockingQueue<number> = new containers.LinkedBlockingQueue<number>(iter);
        for (let i = 0; i < 10; i++) {
            arktest.assertEQ(queueInt.pop(), i);
        }
        arktest.assertEQ(queueInt.capacity, Int.MAX_VALUE);
    });
    LinkedBlockingQueueTestsuite.addTest("ConstructorTest004", () =>
    {
        let array = Array.from<number>(arr);
        let ConcurrentSetTest: containers.ConcurrentSet<number> = new containers.ConcurrentSet<number>(array);
        arktest.assertEQ(ConcurrentSetTest.size, 10);
        flag = 0;
        ConcurrentSetTest.forEach((val: number) => {
            arktest.assertEQ(val, flag++);
        });
        let iter = ConcurrentSetTest.keys();
        let queueInt: containers.LinkedBlockingQueue<number> = new containers.LinkedBlockingQueue<number>(10, iter);
        for (let i = 0; i < 10; i++) {
            arktest.assertEQ(queueInt.pop(), i);
        }
        arktest.assertEQ(queueInt.capacity, 10);
    });

    return LinkedBlockingQueueTestsuite.run();
}