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

let concurrentHashMapOne: containers.ConcurrentHashMap<number, string> = new containers.ConcurrentHashMap<number, string>();

function main(): int {
    let map: Map<number, string> = new Map<number, string>();
    for (let i: int = 0; i < 10 ; i++) {
        map.set(arrKeys[i], arrValues[i]);
    }
    let concurrentHashMapHasTestsuite = new arktest.ArkTestsuite("concurrentHashMapHasTest");

    concurrentHashMapHasTestsuite.addTest("hasTest_001", () =>
    {
        let concurrentHashMap: containers.ConcurrentHashMap<number, string> = new containers.ConcurrentHashMap<number, string>(map);
        for (let i: int = 0; i < 10 ; i++) {
            arktest.assertTrue(concurrentHashMap.has(i));
        }
    });
    concurrentHashMapHasTestsuite.addTest("hasTest_002", () =>
    {
        let concurrentHashMapNull: containers.ConcurrentHashMap<number, string> = new containers.ConcurrentHashMap<number, string>(map);
        arktest.assertFalse(concurrentHashMapNull.has(10));
    });
    concurrentHashMapHasTestsuite.addTest("hasTest_003", () =>
    {
        let concurrentHashMapNull: containers.ConcurrentHashMap<number, string> = new containers.ConcurrentHashMap<number, string>();
        arktest.assertFalse(concurrentHashMapNull.has(1));
    });
    concurrentHashMapHasTestsuite.addTest("hasTest_004", () =>
    {
        let concurrentHashMap: containers.ConcurrentHashMap<string, number> = new containers.ConcurrentHashMap<string, number>();
        for (let i: int = 0; i < 10; i++) {
            concurrentHashMap.set(arrValues[i], arrKeys[i]);
        }
        arktest.assertEQ(concurrentHashMap.size, 10);
        for (let i: int = 0; i < 10; i++) {
            arktest.assertTrue(concurrentHashMap.has(arrValues[i]));
        }
    });
    concurrentHashMapHasTestsuite.addTest("twiceTest_003", () =>
    {
        Atomics.store(flag, 0, 0);
        concurrentHashMapOne.clear();
        arktest.assertTrue(concurrentHashMapOne.isEmpty());
        setAll();
        let p1 = launch<void, () => void>(hasAll);
        let p2 = launch<void, () => void>(hasAll);
        p1.Await();
        p2.Await();
        arktest.assertTrue(Atomics.load(flag, 0) >= 10);
    });
    concurrentHashMapHasTestsuite.addTest("hasAndTest_001", () =>
    {
        Atomics.store(flag, 0, 0);
        concurrentHashMapOne.clear();
        arktest.assertTrue(concurrentHashMapOne.isEmpty());
        setAll();
        let p1 = launch<void, () => void>(hasAll);
        let p2 = launch<void, () => void>(deleteAllBack);
        p1.Await();
        p2.Await();
        arktest.assertTrue(Atomics.load(flag, 0) >= 10);
    });
    concurrentHashMapHasTestsuite.addTest("hasAndTest_002", () =>
    {
        Atomics.store(flag, 0, 0);
        concurrentHashMapOne.clear();
        arktest.assertTrue(concurrentHashMapOne.isEmpty());
        setAll();
        let p1 = launch<void, () => void>(hasAll);
        let p2 = launch<void, () => void>(replaceAll);
        p1.Await();
        p2.Await();
        arktest.assertEQ(Atomics.load(flag, 0), 10);
        for (let i = 0; i < 10; i++) {
            let newVal = arrValues[i] + "-re";
            arktest.assertEQ(concurrentHashMapOne.get(i), newVal);
        }
    });
    concurrentHashMapHasTestsuite.addTest("hasAndTest_003", () =>
    {
        Atomics.store(flag, 0, 0);
        concurrentHashMapOne.clear();
        arktest.assertTrue(concurrentHashMapOne.isEmpty());
        setAll();
        let p1 = launch<void, () => void>(hasAll);
        let p2 = launch<void, () => void>(isEmptyFun);
        p1.Await();
        p2.Await();
        arktest.assertEQ(Atomics.load(flag, 0), 10);
    });
    concurrentHashMapHasTestsuite.addTest("hasAndTest_004", () =>
    {
        Atomics.store(flag, 0, 0);
        concurrentHashMapOne.clear();
        arktest.assertTrue(concurrentHashMapOne.isEmpty());
        setAll();
        let p1 = launch<void, () => void>(hasAll);
        let p2 = launch<void, () => void>(clearAll);
        p1.Await();
        p2.Await();
        arktest.assertTrue(Atomics.load(flag, 0) >= 0);
    });
    concurrentHashMapHasTestsuite.addTest("hasAndTest_005", () =>
    {
        Atomics.store(flag, 0, 0);
        concurrentHashMapOne.clear();
        arktest.assertTrue(concurrentHashMapOne.isEmpty());
        setAll();
        let p1 = launch<void, () => void>(hasAll);
        let p2 = launch<void, () => void>(removeAllBack);
        p1.Await();
        p2.Await();
        arktest.assertTrue(Atomics.load(flag, 0) >= 0);
        for (let i = 0; i < 10; i++) {
            arktest.assertEQ(arrValuesTest[i], arrValues[i]);
        }
    });

    return concurrentHashMapHasTestsuite.run();
}

function setAll() {
    for (let i: int = 0; i < 10; i++) {
        concurrentHashMapOne.set(arrKeys[i], arrValues[i]);
    }
}

function replaceAll() {
    for (let i: int = 0; i < 10; i++) {
        let newVal = arrValues[i] + "-re";
        concurrentHashMapOne.replace(i, newVal);
    }
}

function hasAll() {
    for (let i: int = 0; i < 10; i++) {
        if (concurrentHashMapOne.has(i)) {
            Atomics.add(flag, 0, 1);
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

function removeAllBack() {
    for (let i = 9; i >= 0; i--) {
        if (concurrentHashMapOne.has(i)) {
            arrValuesTest[i] = concurrentHashMapOne.remove(i) as string;
        }
    }
}

function isEmptyFun() {
    let ie = concurrentHashMapOne.isEmpty();
}

function clearAll() {
    concurrentHashMapOne.clear();
}