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
    let ConcurrentSetTestsuite = new arktest.ArkTestsuite("ConcurrentSetDeteleTest");
    // Single Thread Test
    ConcurrentSetTestsuite.addTest("DeteleTestOne001", () =>
    {
        let ConcurrentSetTest: containers.ConcurrentSet<number> = new containers.ConcurrentSet<number>();
        let res = ConcurrentSetTest.size;
        arktest.assertEQ(res, 0);
        ConcurrentSetTest.add(arrKeys[0]);
        res = ConcurrentSetTest.size;
        arktest.assertEQ(res, 1);
        let del = ConcurrentSetTest.delete(arrKeys[0]);
        arktest.assertEQ(del, true);
        res = ConcurrentSetTest.size;
        arktest.assertEQ(res, 0);
    });
    ConcurrentSetTestsuite.addTest("DeteleTestTwo001", () =>
    {
        let ConcurrentSetTest: containers.ConcurrentSet<string> = new containers.ConcurrentSet<string>();
        let res = ConcurrentSetTest.size;
        arktest.assertEQ(res, 0);
        ConcurrentSetTest.add(arrValues[0]);
        res = ConcurrentSetTest.size;
        arktest.assertEQ(res, 1);
        let del = ConcurrentSetTest.delete(arrValues[0]);
        arktest.assertEQ(del, true);
        res = ConcurrentSetTest.size;
        arktest.assertEQ(res, 0);
    });
    ConcurrentSetTestsuite.addTest("DeteleTestOne002", () =>
    {
        let ConcurrentSetTest: containers.ConcurrentSet<number> = new containers.ConcurrentSet<number>();
        let res = ConcurrentSetTest.size;
        arktest.assertEQ(res, 0);
        ConcurrentSetTest.add(arrKeys[0]);
        res = ConcurrentSetTest.size;
        arktest.assertEQ(res, 1);
        let del = ConcurrentSetTest.delete(arrKeys[1]);
        arktest.assertEQ(del, false);
        res = ConcurrentSetTest.size;
        arktest.assertEQ(res, 1);
    });
    ConcurrentSetTestsuite.addTest("DeteleTestTwo002", () =>
    {
        let ConcurrentSetTest: containers.ConcurrentSet<string> = new containers.ConcurrentSet<string>();
        let res = ConcurrentSetTest.size;
        arktest.assertEQ(res, 0);
        ConcurrentSetTest.add(arrValues[0]);
        res = ConcurrentSetTest.size;
        arktest.assertEQ(res, 1);
        let del = ConcurrentSetTest.delete(arrValues[1]);
        arktest.assertEQ(del, false);
        res = ConcurrentSetTest.size;
        arktest.assertEQ(res, 1);
    });
    // Multiple Threads Test
    ConcurrentSetTestsuite.addTest("Detele&HasSizeTest001", () =>
    {
        ConcurrentSetTestOne.clear();
        arktest.assertEQ(ConcurrentSetTestOne.size, 0);
        addAll();
        let p1 = launch<void, (i: int) => void>(deleteOne, 0);
        let p2 = launch<void, (i: int) => void>(deleteOne, 1);
        let p3 = launch<void, (i: int) => void>(deleteOne, 2);
        let p4 = launch<void, () => void>(getSize);
        p1.Await();
        p2.Await();
        p3.Await();
        p4.Await();
        arktest.assertTrue(ConcurrentSetTestOne.size > 6 && ConcurrentSetTestOne.size < 11);
    });
    ConcurrentSetTestsuite.addTest("Detele&HasTest001", () =>
    {
        ConcurrentSetTestOne.clear();
        arktest.assertEQ(ConcurrentSetTestOne.size, 0);
        addAll();
        let p1 = launch<void, (i: int) => void>(deleteOne, 0);
        let p2 = launch<void, (i: int) => void>(deleteOne, 1);
        let p3 = launch<void, (i: int) => void>(hasOne, 3);
        p1.Await();
        p2.Await();
        p3.Await();
        arktest.assertTrue(ConcurrentSetTestOne.size > 7 && ConcurrentSetTestOne.size < 11);
    });
    ConcurrentSetTestsuite.addTest("Detele&ClearTest001", () =>
    {
        ConcurrentSetTestOne.clear();
        arktest.assertEQ(ConcurrentSetTestOne.size, 0);
        addAll();
        let p1 = launch<void, (i: int) => void>(deleteOne, 0);
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

function getSize() {
    ConcurrentSetTestOne.size;
}

function deleteOne(i: int) {
    ConcurrentSetTestOne.delete(arrKeys[i]);
}

function clearTest() {
    ConcurrentSetTestOne.clear();
}