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

let arrKeys: number[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9];
let arrValues: string[] = ["zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine"];

let ConcurrentSetTestOne: containers.ConcurrentSet<number> = new containers.ConcurrentSet<number>();

function main(): int {
    let ConcurrentSetTestsuite = new arktest.ArkTestsuite("ConcurrentSetHasTest");
    // Single Thread Test
    ConcurrentSetTestsuite.addTest("HasTestOne001", () =>
    {
        let ConcurrentSetTest: containers.ConcurrentSet<number> = new containers.ConcurrentSet<number>();
        let res = ConcurrentSetTest.size;
        arktest.assertEQ(res, 0);
        for (let i: int = 0; i < 10 ; i++) {
            ConcurrentSetTest.add(arrKeys[i]);
        }
        res = ConcurrentSetTest.size;
        arktest.assertEQ(res, 10);
        let hasFlag = ConcurrentSetTest.has(1);
        arktest.assertEQ(hasFlag, true);
    });
    ConcurrentSetTestsuite.addTest("HasTestTwo001", () =>
    {
        let ConcurrentSetTest: containers.ConcurrentSet<string> = new containers.ConcurrentSet<string>();
        let res = ConcurrentSetTest.size;
        arktest.assertEQ(res, 0);
        for (let i: int = 0; i < 10 ; i++) {
            ConcurrentSetTest.add(arrValues[i]);
        }
        arktest.assertEQ(ConcurrentSetTest.size, 10);
        let one: string = "one";
        let hasFlag = ConcurrentSetTest.has(one);
        arktest.assertEQ(hasFlag, true);
    });
    ConcurrentSetTestsuite.addTest("HasTestOne002", () =>
    {
        let ConcurrentSetTest: containers.ConcurrentSet<number> = new containers.ConcurrentSet<number>();
        let res = ConcurrentSetTest.size;
        arktest.assertEQ(res, 0);
        for (let i: int = 0; i < 10 ; i++) {
            ConcurrentSetTest.add(arrKeys[i]);
        }
        res = ConcurrentSetTest.size;
        arktest.assertEQ(res, 10);
        let hasFlag = ConcurrentSetTest.has(20);
        arktest.assertEQ(hasFlag, false);
    });
    ConcurrentSetTestsuite.addTest("HasTestTwo002", () =>
    {
        let ConcurrentSetTest: containers.ConcurrentSet<string> = new containers.ConcurrentSet<string>();
        let res = ConcurrentSetTest.size;
        arktest.assertEQ(res, 0);
        for (let i: int = 0; i < 10 ; i++) {
            ConcurrentSetTest.add(arrValues[i]);
        }
        res = ConcurrentSetTest.size;
        arktest.assertEQ(res, 10);
        let hasFlag = ConcurrentSetTest.has("error");
        arktest.assertEQ(hasFlag, false);
    });
    ConcurrentSetTestsuite.addTest("HasTestOne003", () =>
    {
        let ConcurrentSetTest: containers.ConcurrentSet<number> = new containers.ConcurrentSet<number>();
        let res = ConcurrentSetTest.size;
        arktest.assertEQ(res, 0);
        let hasFlag = ConcurrentSetTest.has(10);
        arktest.assertEQ(hasFlag, false);
    });
    ConcurrentSetTestsuite.addTest("HasTestTwo003", () =>
    {
        let ConcurrentSetTest: containers.ConcurrentSet<string> = new containers.ConcurrentSet<string>();
        let res = ConcurrentSetTest.size;
        arktest.assertEQ(res, 0);
        let hasFlag = ConcurrentSetTest.has("two");
        arktest.assertEQ(hasFlag, false);
    });
    // Multiple Threads Test
    ConcurrentSetTestsuite.addTest("Has&ClearTest001", () =>
    {
        ConcurrentSetTestOne.clear();
        arktest.assertEQ(ConcurrentSetTestOne.size, 0);
        addAll();
        arktest.assertEQ(ConcurrentSetTestOne.has(3), true);
        let p1 = launch<void, (i: int) => void>(hasOne, 0);
        let p2 = launch<void, () => void>(clearTest);
        p1.Await();
        p2.Await();
        arktest.assertEQ(ConcurrentSetTestOne.has(0), false);
    });

    return ConcurrentSetTestsuite.run();
}

function addAll() {
    for (let i: int = 0; i < 10 ; i++) {
        ConcurrentSetTestOne.add(arrKeys[i]);
    }
}

function hasOne(i: int) {
    ConcurrentSetTestOne.has(i);
}

function clearTest() {
    ConcurrentSetTestOne.clear();
}
