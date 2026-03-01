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
let arrKeysTest: number[] = [10, 10, 10, 10, 10, 10, 10, 10, 10, 10];
let arrValuesTest: string[] = ["ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten"];

let concurrentHashMapOne: containers.ConcurrentHashMap<number, string> = new containers.ConcurrentHashMap<number, string>();

function main(): int {

    let concurrentHashMapTestsuite = new arktest.ArkTestsuite("concurrentHashMapSetTest");

    concurrentHashMapTestsuite.addTest("keysAndTest_001", () =>
    {
        concurrentHashMapOne.clear();
        arktest.assertTrue(concurrentHashMapOne.isEmpty());
        let p1 = launch<void, () => void>(setAll);
        let p2 = launch<void, () => void>(keysAll);
        p1.Await();
        p2.Await();
        arktest.assertEQ(concurrentHashMapOne.size, 10);
    });
    concurrentHashMapTestsuite.addTest("ValuesAndTest_001", () =>
    {
        concurrentHashMapOne.clear();
        arktest.assertTrue(concurrentHashMapOne.isEmpty());
        let p1 = launch<void, () => void>(setAll);
        let p2 = launch<void, () => void>(valuesAll);
        p1.Await();
        p2.Await();
        arktest.assertEQ(concurrentHashMapOne.size, 10);
    });
    concurrentHashMapTestsuite.addTest("$_iteratorAndTest_001", () =>
    {
        concurrentHashMapOne.clear();
        arktest.assertTrue(concurrentHashMapOne.isEmpty());
        let p1 = launch<void, () => void>(setAll);
        let p2 = launch<void, () => void>($_iteratorAll);
        p1.Await();
        p2.Await();
        arktest.assertEQ(concurrentHashMapOne.size, 10);
    });
    concurrentHashMapTestsuite.addTest("EntriesAndTest_001", () =>
    {
        concurrentHashMapOne.clear();
        arktest.assertTrue(concurrentHashMapOne.isEmpty());
        let p1 = launch<void, () => void>(setAll);
        let p2 = launch<void, () => void>(entriesAll);
        p1.Await();
        p2.Await();
        arktest.assertEQ(concurrentHashMapOne.size, 10);
    });
    concurrentHashMapTestsuite.addTest("forEachWithoutAndTest_001", () =>
    {
        concurrentHashMapOne.clear();
        arktest.assertTrue(concurrentHashMapOne.isEmpty());
        flag = 0;
        let p1 = launch<void, () => void>(forEachZero);
        let p2 = launch<void, () => void>(setAll);
        p1.Await();
        p2.Await();
        arktest.assertEQ(concurrentHashMapOne.size, 10);
    });
    concurrentHashMapTestsuite.addTest("forEachWithOneAndTest_001", () =>
    {
        concurrentHashMapOne.clear();
        arktest.assertTrue(concurrentHashMapOne.isEmpty());
        flag = 0;
        let p1 = launch<void, () => void>(setAll);
        let p2 = launch<void, () => void>(forEachOne);
        p1.Await();
        p2.Await();
        arktest.assertEQ(concurrentHashMapOne.size, 10);
    });
    concurrentHashMapTestsuite.addTest("forEachWithTwoAndTest_001", () =>
    {
        concurrentHashMapOne.clear();
        arktest.assertTrue(concurrentHashMapOne.isEmpty());
        flag = 0;
        let p1 = launch<void, () => void>(setAll);
        let p2 = launch<void, () => void>(forEachTwo);
        p1.Await();
        p2.Await();
        arktest.assertEQ(concurrentHashMapOne.size, 10);
    });
    concurrentHashMapTestsuite.addTest("forEachWithThreeAndTest_001", () =>
    {
        concurrentHashMapOne.clear();
        arktest.assertTrue(concurrentHashMapOne.isEmpty());
        flag = 0;
        let p1 = launch<void, () => void>(setAll);
        let p2 = launch<void, () => void>(forEachThree);
        p1.Await();
        p2.Await();
        arktest.assertEQ(concurrentHashMapOne.size, 10);
    });

    return concurrentHashMapTestsuite.run();
}

function setAll() {
    for (let i: int = 0; i < 10; i++) {
        concurrentHashMapOne.set(arrKeys[i], arrValues[i]);
    }
}

function keysAll() {
    let iterKeys = concurrentHashMapOne.keys();
    for (let i = 0; iterKeys.next().value == arrKeys[i]; i++) {
        if (iterKeys.next().value == undefined) {
            break;
        }
    }
}

function valuesAll() {
    let iterValues = concurrentHashMapOne.values();
    let i = 0;
    while (iterValues.next().value == arrValues[i]) {
        i++;
        if (iterValues.next().value == undefined) {
            break;
        }
    }
}

function $_iteratorAll() {
    let iterator$ = concurrentHashMapOne.$_iterator();
    let i = 0;
    while (iterator$.next().value != undefined) {
        i++;
    }
}

function entriesAll() {
    let iterEntries = concurrentHashMapOne.entries();
    let i = 0;
    while (iterEntries.next().value != undefined) {
        i++;
    }
}

function forEachZero() {
    concurrentHashMapOne.forEach(() => {
        flag++;
    });
}

function forEachOne() {
    concurrentHashMapOne.forEach((val: string) => {
        arrValuesTest[flag++] = val;
    });
}

function forEachTwo() {
    concurrentHashMapOne.forEach((val: string, key: number) => {
        arrKeysTest[flag] = key;
        arrValuesTest[flag++] = val;
    });
}

function forEachThree() {
    concurrentHashMapOne.forEach((val: string, key: number, map: containers.ConcurrentHashMap<number, string>) => {
        arrKeysTest[flag] = key;
        arrValuesTest[flag++] = val;
    });
}
