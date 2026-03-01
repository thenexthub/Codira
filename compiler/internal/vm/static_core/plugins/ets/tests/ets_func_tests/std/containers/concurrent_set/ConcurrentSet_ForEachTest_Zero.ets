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

let flag: int = 0;
let arrKeys: number[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9];
let arrValues: string[] = ["zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine"];

let ConcurrentSetTestOne: containers.ConcurrentSet<number> = new containers.ConcurrentSet<number>();

function main(): int {
    let ConcurrentSetTestsuite = new arktest.ArkTestsuite("ConcurrentSetForEachTest");

    ConcurrentSetTestsuite.addTest("TestForEach001", () =>
    {
        let ConcurrentSetTest: containers.ConcurrentSet<string> = new containers.ConcurrentSet<string>();
        ConcurrentSetTest.add(arrValues[0]);
        ConcurrentSetTest.add(arrValues[1]);
        flag = 0;
        ConcurrentSetTest.forEach((): void => {
            flag++;
        });
        arktest.assertEQ(flag, 2);
    });
    ConcurrentSetTestsuite.addTest("ForEach&AddTest001", () =>
    {
        ConcurrentSetTestOne.clear();
        arktest.assertEQ(ConcurrentSetTestOne.size, 0);
        flag = 0;
        let p1 = launch<void, () => void>(forEachZero);
        let p2 = launch<void, () => void>(addAll);
        p1.Await();
        p2.Await();
        arktest.assertTrue(ConcurrentSetTestOne.size >= 0);
    });
    ConcurrentSetTestsuite.addTest("ForEach&AddTest002", () =>
    {
        ConcurrentSetTestOne.clear();
        arktest.assertEQ(ConcurrentSetTestOne.size, 0);
        flag = 0;
        forEachZero();
        addAll();
        arktest.assertEQ(ConcurrentSetTestOne.size, 10);
    });
    ConcurrentSetTestsuite.addTest("ForEach&DeteleTest001", () =>
    {
        ConcurrentSetTestOne.clear();
        arktest.assertEQ(ConcurrentSetTestOne.size, 0);
        flag = 0;
        addAll();
        let p1 = launch<void, () => void>(forEachZero);
        let p2 = launch<void, () => void>(deleteAllBack);
        p1.Await();
        p2.Await();
        arktest.assertTrue(ConcurrentSetTestOne.size < 11);
    });
    ConcurrentSetTestsuite.addTest("ForEach&HasTest001", () =>
    {
        ConcurrentSetTestOne.clear();
        arktest.assertEQ(ConcurrentSetTestOne.size, 0);
        flag = 0;
        addAll();
        let p1 = launch<void, () => void>(forEachZero);
        let p2 = launch<void, () => void>(hasAll);
        p1.Await();
        p2.Await();
        arktest.assertEQ(ConcurrentSetTestOne.size, 10);
    });
    ConcurrentSetTestsuite.addTest("ForEach&ClearTest001", () =>
    {
        ConcurrentSetTestOne.clear();
        arktest.assertEQ(ConcurrentSetTestOne.size, 0); 
        flag = 0;
        addAll();
        let p1 = launch<void, () => void>(forEachZero);
        let p2 = launch<void, () => void>(clearTest);
        p1.Await();
        p2.Await();
        arktest.assertTrue(ConcurrentSetTestOne.size < 11);
    });

    return ConcurrentSetTestsuite.run();
}

function forEachZero() {
    ConcurrentSetTestOne.forEach(() => {
        flag++;
    });
}

function clearTest() {
    ConcurrentSetTestOne.clear();
}

function deleteAllBack() {
    for (let i: int = 9; i >= 0; i--) {
        ConcurrentSetTestOne.delete(i);
    }
}

function hasAll() {
    for (let i: int = 0; i < 10 ; i++) {
        ConcurrentSetTestOne.has(i);
    }
}

function addAll() {
    for (let i: int = 0; i < 10 ; i++) {
        ConcurrentSetTestOne.add(arrKeys[i]);
    }
}
