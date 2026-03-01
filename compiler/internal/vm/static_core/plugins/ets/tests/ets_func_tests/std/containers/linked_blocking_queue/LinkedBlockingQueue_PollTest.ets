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
const stressNum: int = 100;

let sMTQueueInt: containers.LinkedBlockingQueue<int> = new containers.LinkedBlockingQueue<int>(stressNum);

function main(): int {
    let LinkedBlockingQueueTestsuite = new arktest.ArkTestsuite("LinkedBlockingQueuePollTest");

    LinkedBlockingQueueTestsuite.addTest("PollTest001", () =>
    {
        let queueInt: containers.LinkedBlockingQueue<int> = new containers.LinkedBlockingQueue<int>();
        for (let i = 0; i < elementsNum; i++) {
            arktest.assertEQ(queueInt.add(i), true);
        }
        arktest.assertEQ(queueInt.size, elementsNum);
        for (let i = 0; i < 5; i++) {
            arktest.assertEQ(queueInt.poll(), i);
        }
        arktest.assertEQ(queueInt.size, 5);
        for (let i = 5; i < 10; i++) {
            arktest.assertEQ(queueInt.poll(), i);
        }
        arktest.assertEQ(queueInt.size, 0);
        arktest.assertEQ(queueInt.poll(), undefined);
    });
    LinkedBlockingQueueTestsuite.addTest("PollStressTest004", () =>
    {
        let queueInt: containers.LinkedBlockingQueue<int> = new containers.LinkedBlockingQueue<int>(stressNum);
        for (let i = 0; i < stressNum; i++) {
            queueInt.add(i);
        }
        for (let i = 0; i < stressNum; i++) {
            arktest.assertEQ(queueInt.poll(), i);
        }
        arktest.assertEQ(queueInt.size, 0);
    });

    return LinkedBlockingQueueTestsuite.run();
}