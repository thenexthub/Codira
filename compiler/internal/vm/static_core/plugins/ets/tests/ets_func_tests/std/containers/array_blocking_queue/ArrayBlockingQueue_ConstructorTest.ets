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
const arr: Number[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9];

function main() {
    let ArrayBlockingQueueTestsuite = new arktest.ArkTestsuite("ArrayBlockingQueueConstructorTest");

    ArrayBlockingQueueTestsuite.addTest("ConstructorTestInt001", () =>
    {
        let queueIntA: containers.ArrayBlockingQueue<int> = new containers.ArrayBlockingQueue<int>(200);
        arktest.assertEQ(queueIntA.isEmpty(), true);
        arktest.assertEQ(queueIntA.size, 0);
    });
    ArrayBlockingQueueTestsuite.addTest("ConstructorTestInt002", () =>
    {
        let array = Array.from<number>(arr);
        let ConcurrentSetTest: containers.ConcurrentSet<number> = new containers.ConcurrentSet<number>(array);
        arktest.assertEQ(ConcurrentSetTest.size, 10);
        flag = 0;
        ConcurrentSetTest.forEach((val: number) => {
            arktest.assertEQ(val, flag++);
        });
        let iter = ConcurrentSetTest.keys();
        let queueIntA: containers.ArrayBlockingQueue<number> = new containers.ArrayBlockingQueue<number>(200, iter);
        for (let i = 0; i < 10; i++) {
            arktest.assertEQ(queueIntA.pop(), i);
        }
    });

    ArrayBlockingQueueTestsuite.run();
}