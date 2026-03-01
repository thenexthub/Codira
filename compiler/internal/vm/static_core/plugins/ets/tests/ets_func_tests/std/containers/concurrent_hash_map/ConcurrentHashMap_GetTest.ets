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

let buf = new ArrayBuffer(4);
let flag: Int8Array = new Int8Array(buf, 0, 4);

let arrKeys: number[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9];
let arrValues: string[] = ["zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine"];
let arrValuesTest: string[] = ["ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten"];
let arrValuesTest1: string[] = ["ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten"];

let concurrentHashMapOne: containers.ConcurrentHashMap<number, string> = new containers.ConcurrentHashMap<number, string>();

function main(): int {
    let map: Map<number, string> = new Map<number, string>();
    for (let i: int = 0; i < 10 ; i++) {
        map.set(arrKeys[i], arrValues[i]);
    }

    let concurrentHashMapTestsuite = new arktest.ArkTestsuite("concurrentHashMapGetTest");

    concurrentHashMapTestsuite.addTest("getTest_001", () =>
    {
        let concurrentHashMap: containers.ConcurrentHashMap<number, string> = new containers.ConcurrentHashMap<number, string>();
        arktest.assertEQ(concurrentHashMap.get(1), undefined);
        arktest.assertEQ(concurrentHashMap.get(2), undefined);
        arktest.assertEQ(concurrentHashMap.get(3), undefined);
    });
    concurrentHashMapTestsuite.addTest("getTest_002", () =>
    {
        let concurrentHashMap: containers.ConcurrentHashMap<number, string> = new containers.ConcurrentHashMap<number, string>(map);
        arktest.assertEQ(concurrentHashMap.get(0), "zero");
        arktest.assertEQ(concurrentHashMap.get(1), "one");
        arktest.assertEQ(concurrentHashMap.get(2), "two");
    });
    concurrentHashMapTestsuite.addTest("getTest_003", () =>
    {
        let concurrentHashMap: containers.ConcurrentHashMap<number, string> = new containers.ConcurrentHashMap<number, string>(map);
        arktest.assertEQ(concurrentHashMap.get(0), "zero");
        arktest.assertEQ(concurrentHashMap.get(10), undefined);
        arktest.assertEQ(concurrentHashMap.get(11), undefined);
    });
    concurrentHashMapTestsuite.addTest("getTest_004", () =>
    {
        let concurrentHashMap: containers.ConcurrentHashMap<string, number> = new containers.ConcurrentHashMap<string, number>();
        for (let i: int = 0; i < 10; i++) {
            concurrentHashMap.set(arrValues[i], arrKeys[i]);
        }
        arktest.assertEQ(concurrentHashMap.size, 10);
        for (let i: int = 0; i < 10; i++) {
            arktest.assertEQ(concurrentHashMap.get(arrValues[i]), arrKeys[i]);
        }
    });
    concurrentHashMapTestsuite.addTest("twiceTest_002", () =>
    {
        arrValuesTest = ["ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten"];
        arrValuesTest1 = ["ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten"];
        concurrentHashMapOne.clear();
        arktest.assertTrue(concurrentHashMapOne.isEmpty());
        setAll();
        let p1 = launch<void, () => void>(getAll);
        let p2 = launch<void, () => void>(getAll1);
        p1.Await();
        p2.Await();
        let flagCounter: int = 0;
        for (let i = 0; i < 10; i++) {
            if ((arrValuesTest[i] == concurrentHashMapOne.get(i)) &&
                (arrValuesTest1[i] == concurrentHashMapOne.get(i)))
            {
                flagCounter++;
            }
        }
        arktest.assertEQ(flagCounter, 10);
    });
    concurrentHashMapTestsuite.addTest("getAndTest_001", () =>
    {
        arrValuesTest = ["ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten"];
        concurrentHashMapOne.clear();
        arktest.assertTrue(concurrentHashMapOne.isEmpty());
        setAll();
        let p1 = launch<void, () => void>(getAll);
        let p2 = launch<void, () => void>(hasAll);
        p1.Await();
        p2.Await();
        let flagCounter: int = 0;
        for (let i = 0; i < 10; i++) {
            if (arrValues[i] == arrValuesTest[i]) {
                flagCounter++;
            }
        }
        arktest.assertEQ(flagCounter, 10);
    });
    concurrentHashMapTestsuite.addTest("getAndTest_002", () =>
    {
        Atomics.store(flag, 0, 0);
        arrValuesTest = ["ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten"];
        concurrentHashMapOne.clear();
        arktest.assertTrue(concurrentHashMapOne.isEmpty());
        setAll();
        let p1 = launch<void, () => void>(getAll);
        let p2 = launch<void, () => void>(deleteAllBack);
        p1.Await();
        p2.Await();
        for (let i = 0; i < 10; i++) {
            if (arrValues[i] == arrValuesTest[i]) {
                Atomics.add(flag, 0, 1);
            }
        }
        arktest.assertTrue(Atomics.load(flag, 0) >= 10);
    });
    concurrentHashMapTestsuite.addTest("getAndTest_003", () =>
    {
        arrValuesTest = ["ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten"];
        concurrentHashMapOne.clear();
        arktest.assertTrue(concurrentHashMapOne.isEmpty());
        setAll();
        let p1 = launch<void, () => void>(replaceAll);
        let p2 = launch<void, () => void>(getAllBack);
        p1.Await();
        p2.Await();
        let flagCounter: int = 0;
        for (let i = 0; i < 10; i++) {
            if (arrValuesTest[i] == arrValues[i] + "-re") {
                flagCounter++;
            }
        }
        arktest.assertTrue(flagCounter >= 0);
    });
    concurrentHashMapTestsuite.addTest("getAndTest_004", () =>
    {
        arrValuesTest = ["ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten"];
        concurrentHashMapOne.clear();
        arktest.assertTrue(concurrentHashMapOne.isEmpty());
        setAll();
        let p1 = launch<void, () => void>(getAll);
        let p2 = launch<void, () => void>(isEmptyFun);
        p1.Await();
        p2.Await();
        let flagCounter: int = 0;
        for (let i = 0; i < 10; i++) {
            if (arrValuesTest[i] == arrValues[i]) {
                flagCounter++;
            }
        }
        arktest.assertEQ(flagCounter, 10);
    });
    concurrentHashMapTestsuite.addTest("getAndTest_005", () =>
    {
        arrValuesTest = ["ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten"];
        concurrentHashMapOne.clear();
        arktest.assertTrue(concurrentHashMapOne.isEmpty());
        setAll();
        let p1 = launch<void, () => void>(getAll);
        let p2 = launch<void, () => void>(clearAll);
        p1.Await();
        p2.Await();
        let flagCounter: int = 0;
        for (let i = 0; i < 10; i++) {
            if (arrValuesTest[i] == arrValues[i]) {
                flagCounter++;
            }
        }
        arktest.assertTrue(flagCounter >= 0);
    });
    concurrentHashMapTestsuite.addTest("getAndTest_006", () =>
    {
        arrValuesTest = ["ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten"];
        arrValuesTest1 = ["ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten"];
        concurrentHashMapOne.clear();
        arktest.assertTrue(concurrentHashMapOne.isEmpty());
        setAll();
        let p1 = launch<void, () => void>(getAll1);
        let p2 = launch<void, () => void>(removeAll);
        p1.Await();
        p2.Await();
        let flagCounter: int = 0;
        for (let i = 0; i < 10; i++) {
            if (arrValuesTest[i] == arrValuesTest1[i]) {
                flagCounter++;
            }
        }
        arktest.assertTrue(flagCounter >= 0);
    });

    return concurrentHashMapTestsuite.run();
}

function setAll() {
    for (let i: int = 0; i < 10; i++) {
        concurrentHashMapOne.set(arrKeys[i], arrValues[i]);
    }
}

function replaceAll() {
    for (let i: int = 0; i < 10; i++) {
        let newVal = arrValues[i] + "-re";
        if (concurrentHashMapOne.replace(i, newVal)) {
            Atomics.add(flag, 0, 1);
        }
    }
}

function hasAll() {
    for (let i: int = 0; i < 10; i++) {
        if (concurrentHashMapOne.has(i)) {
            Atomics.add(flag, 0, 1);
        }
    }
}

function getAll() {
    for (let i: int = 0; i < 10; i++) {
        if (concurrentHashMapOne.has(i)) {
            let getVal = concurrentHashMapOne.get(i);
            if (getVal !== undefined) {
                arrValuesTest[i] = getVal as string;
            }
        }
    }
}

function getAll1() {
    for (let i: int = 0; i < 10; i++) {
        if (concurrentHashMapOne.has(i)) {
            let getVal = concurrentHashMapOne.get(i);
            if (getVal !== undefined) {
                arrValuesTest1[i] = getVal as string;
            }
        }
    }
}

function getAllBack() {
    for (let i: int = 9; i >= 0; i--) {
        if (concurrentHashMapOne.has(i)) {
            let getVal = concurrentHashMapOne.get(i);
            if (getVal !== undefined) {
                arrValuesTest[i] = getVal as string;
            }
        }
    }
}

function deleteAllBack() {
    for (let i: int = 9; i >= 0; i--) {
        if (concurrentHashMapOne.delete(i)) {
            Atomics.add(flag, 0, 1);
        }
    }
}

function removeAll() {
    for (let i = 0; i < 10; i++) {
        if (!concurrentHashMapOne.isEmpty()) {
            let val = concurrentHashMapOne.remove(i);
            if (val !== undefined) {
                arrValuesTest[i] = val as string;
            }
        }
    }
}

function isEmptyFun() {
    let ie = concurrentHashMapOne.isEmpty();
}

function clearAll() {
    concurrentHashMapOne.clear();
}