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

let pressureFlag: number = 0;

let concurrentHashMapOne: containers.ConcurrentHashMap<number, string> = new containers.ConcurrentHashMap<number, string>();

function main(): int {

    let concurrentHashMapTestsuite = new arktest.ArkTestsuite("concurrentHashMapPressureTest");

    concurrentHashMapTestsuite.addTest("replacePressureTest_002", () =>
    {
        concurrentHashMapOne.clear();
        arktest.assertTrue(concurrentHashMapOne.isEmpty());
        for (let i: int = 0; i < 100; i++) {
            let j: string = `${i}`;
            concurrentHashMapOne.set(i, j);
        }
        let jobs = new Array<Job<void>>(100);
        pressureFlag = 0;
        for (let i: int = 0; i < 100; i++) {
            jobs[i] = launch<void, (i: number) => void>(replaceForPressure, pressureFlag);
            pressureFlag += 1;
        }
        for (let i: int = 0; i < 100; i++) {
            jobs[i].Await();
        }
        arktest.assertEQ(concurrentHashMapOne.size, 100);
        for (let i: int = 0; i < 100; i++) {
            let j: string = `${i}` + "-re";
            arktest.assertEQ(concurrentHashMapOne.get(i), j);
        }
    });

    return concurrentHashMapTestsuite.run();
}

function replaceForPressure(pressureFlag: number) {
    let num = pressureFlag;
    for (let i: number = num; i < num + 1; i++) {
        let j: string = `${i}` + "-re";
        concurrentHashMapOne.replace(i, j);
    }
}
