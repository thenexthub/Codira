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

let concurrentHashMapOne: containers.ConcurrentHashMap<number, string> = new containers.ConcurrentHashMap<number, string>();

function main(): int {
    let map: Map<number, string> = new Map<number, string>();
    for (let i: int = 0; i < 10 ; i++) {
        map.set(arrKeys[i], arrValues[i]);
    }

    let concurrentHashMapTestsuite = new arktest.ArkTestsuite("concurrentHashMapValuesTest");

    concurrentHashMapTestsuite.addTest("valuesTest_001", () =>
    {
        let concurrentHashMap: containers.ConcurrentHashMap<number, string> = new containers.ConcurrentHashMap<number, string>();
        let iterValue: IterableIterator<string> = concurrentHashMap.values();
        arktest.assertEQ(iterValue.next().value, undefined);
        arktest.assertEQ(iterValue.next().value, undefined);
    });
    concurrentHashMapTestsuite.addTest("valuesTest_002", () =>
    {
        let concurrentHashMap: containers.ConcurrentHashMap<number, string> = new containers.ConcurrentHashMap<number, string>(map);
        let iterValues: IterableIterator<string> = concurrentHashMap.values();
        let flag: number = 0;
        for (let i: int = 0; i < 10; i++) {
            if (iterValues.next().value == arrValues[i]) {
                flag++;
            }
        }
        arktest.assertEQ(flag, 10);
        arktest.assertEQ(iterValues.next().value, undefined);
    });
    concurrentHashMapTestsuite.addTest("valuesTest_003", () =>
    {
        let concurrentHashMap: containers.ConcurrentHashMap<number, string> = new containers.ConcurrentHashMap<number, string>();
        let iterValues: IterableIterator<string> = concurrentHashMap.values();
        arktest.assertEQ(iterValues.next().value, undefined);
        for (let i: int = 0; i < 10; i++) {
            concurrentHashMap.set(arrKeys[i], arrValues[i]);
        }
        flag = 0;
        iterValues = concurrentHashMap.values();
        for (let i: int = 0; i < 10; i++) {
            if (iterValues.next().value == arrValues[i]) {
                flag++;
            }
        }
        arktest.assertEQ(flag, 10);
        arktest.assertEQ(iterValues.next().value, undefined);
    });
    concurrentHashMapTestsuite.addTest("valuesTest_004", () =>
    {
        let concurrentHashMap: containers.ConcurrentHashMap<string, number> = new containers.ConcurrentHashMap<String, number>();
        let iterValues: IterableIterator<number> = concurrentHashMap.values();
        arktest.assertEQ(iterValues.next().value, undefined);
        for (let i: int = 0; i < 10; i++) {
            concurrentHashMap.set(arrValues[i], arrKeys[i]);
        }
        flag = 0;
        iterValues = concurrentHashMap.values();
        for (let i: int = 0; i < 10; i++) {
            if (iterValues.next().value == arrKeys[i]) {
                flag++;
            }
        }
        arktest.assertEQ(flag, 10);
        arktest.assertEQ(iterValues.next().value, undefined);
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
    concurrentHashMapTestsuite.addTest("ValuesAndTest_002", () =>
    {
        concurrentHashMapOne.clear();
        arktest.assertTrue(concurrentHashMapOne.isEmpty());
        setAll();
        let p1 = launch<void, () => void>(deleteAll);
        let p2 = launch<void, () => void>(valuesAll);
        p1.Await();
        p2.Await();
        arktest.assertTrue(concurrentHashMapOne.isEmpty());
    });
    concurrentHashMapTestsuite.addTest("ValuesAndTest_003", () =>
    {
        concurrentHashMapOne.clear();
        arktest.assertTrue(concurrentHashMapOne.isEmpty());
        setAll();
        let p1 = launch<void, () => void>(clearAll);
        let p2 = launch<void, () => void>(valuesAll);
        p1.Await();
        p2.Await();
        arktest.assertTrue(concurrentHashMapOne.isEmpty());
    });

    return concurrentHashMapTestsuite.run();
}

function setAll() {
    for (let i: int = 0; i < 10; i++) {
        concurrentHashMapOne.set(arrKeys[i], arrValues[i]);
    }
}

function deleteAll() {
    for (let i: int = 0; i < 10; i++) {
        if (concurrentHashMapOne.delete(i)) {
            flag++;
        }
    }
}

function clearAll() {
    concurrentHashMapOne.clear();
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
