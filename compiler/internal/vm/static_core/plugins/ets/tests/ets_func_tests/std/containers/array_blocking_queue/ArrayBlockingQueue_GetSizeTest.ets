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

import { launch } from "std/concurrency"

const elementsNum: int = 100;
let sizeNum: int = 0;

let queueInt: containers.ArrayBlockingQueue<int> = new containers.ArrayBlockingQueue<int>(200);

function main(): int {
    let ArrayBlockingQueueTestsuite = new arktest.ArkTestsuite("ArrayBlockingQueueGetSizeTest");

    ArrayBlockingQueueTestsuite.addTest("GetSizeTestInt003", () =>
    {
        while (!queueInt.isEmpty()) {
            queueInt.pop();
        }
        for (let i: int = 0; i < elementsNum; ++i) {
            queueInt.push(i);
        }
        let p1 = launch<void, () => void>(queueIntSizedata);
        let p2 = launch<void, () => void>(queueIntSizedata);
        let p3 = launch<void, () => void>(queueIntSizedata);
        let p4 = launch<void, () => void>(queueIntSizedata);
        p1.Await(); 
        p2.Await(); 
        p3.Await(); 
        p4.Await(); 
        arktest.assertEQ(queueInt.size, elementsNum);
    });

    return ArrayBlockingQueueTestsuite.run();
}

function queueIntSizedata() {
    sizeNum = queueInt.size;
}