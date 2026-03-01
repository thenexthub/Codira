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
import { Job } from "std/core"

let ConcurrentSetTestOne: containers.ConcurrentSet<number> = new containers.ConcurrentSet<number>();

function main(): int {
    let ConcurrentSetTestsuite = new arktest.ArkTestsuite("ConcurrentSetPressureTest");

    ConcurrentSetTestsuite.addTest("setPressureTest001", () =>
    {
        ConcurrentSetTestOne.clear();
        arktest.assertEQ(ConcurrentSetTestOne.size, 0);
        let jobs = new Array<Job<void>>(100);

        for (let i: int = 0; i < 100; i++) {
            jobs[i] = launch<void, (i: int) => void>(addMore, i);
        }
        for (let i: int = 0; i < 100; i++) {
            jobs[i].Await();
        }
        arktest.assertEQ(ConcurrentSetTestOne.size, 100);
    });

    return ConcurrentSetTestsuite.run();
}

function addMore(i: int) {
    ConcurrentSetTestOne.add(i);
}
