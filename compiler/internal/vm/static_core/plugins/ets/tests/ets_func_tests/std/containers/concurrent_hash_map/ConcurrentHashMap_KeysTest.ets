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

let concurrentHashMapKeys: containers.ConcurrentHashMap<number, string> = new containers.ConcurrentHashMap<number, string>();

function main(): int {
    let map: Map<number, string> = new Map<number, string>();
    for (let i: int = 0; i < 10 ; i++) {
        map.set(arrKeys[i], arrValues[i]);
    }

    let concurrentHashMapKeysTestsuite = new arktest.ArkTestsuite("concurrentHashMapKeysTest");

    concurrentHashMapKeysTestsuite.addTest("keysTest_001", () =>
    {
        let concurrentHashMapNull: containers.ConcurrentHashMap<number, string> = new containers.ConcurrentHashMap<number, string>();
        let iterKey: IterableIterator<number> = concurrentHashMapNull.keys();
        arktest.assertEQ(iterKey.next().value, undefined);
        arktest.assertEQ(iterKey.next().value, undefined);
    });
    concurrentHashMapKeysTestsuite.addTest("keysTest_002", () =>
    {
        let concurrentHashMap: containers.ConcurrentHashMap<number, string> = new containers.ConcurrentHashMap<number, string>(map);
        let iterKeys: IterableIterator<number> = concurrentHashMap.keys();
        flag = 0;
        for (let i: int = 0; i < 10; i++) {
            if (iterKeys.next().value == arrKeys[i]) {
                flag++;
            }
        }
        arktest.assertEQ(flag, 10);
        arktest.assertEQ(iterKeys.next().value, undefined);
    });
    concurrentHashMapKeysTestsuite.addTest("keysTest_003", () =>
    {
        let concurrentHashMap: containers.ConcurrentHashMap<number, string> = new containers.ConcurrentHashMap<number, string>();
        let iterKeys: IterableIterator<number> = concurrentHashMap.keys();
        arktest.assertEQ(iterKeys.next().value, undefined);
        for (let i: int = 0; i < 10; i++) {
            concurrentHashMap.set(arrKeys[i], arrValues[i]);
        }
        flag = 0;
        iterKeys = concurrentHashMap.keys();
        for (let i: int = 0; i < 10; i++) {
            if (iterKeys.next().value == arrKeys[i]) {
                flag++;
            }
        }
        arktest.assertEQ(flag, 10);
        arktest.assertEQ(iterKeys.next().value, undefined);
    });
    concurrentHashMapKeysTestsuite.addTest("keysTest_004", () =>
    {
        let concurrentHashMap: containers.ConcurrentHashMap<string, number> = new containers.ConcurrentHashMap<String, number>();
        let iterKeys: IterableIterator<string> = concurrentHashMap.keys();
        arktest.assertEQ(iterKeys.next().value, undefined);
        for (let i: int = 0; i < 10; i++) {
            concurrentHashMap.set(arrValues[i], arrKeys[i]);
        }
        flag = 0;
        iterKeys = concurrentHashMap.keys();
        for (let i: int = 0; i < 10; i++) {
            if (iterKeys.next().value == arrValues[i]) {
                flag++;
            }
        }
        arktest.assertEQ(flag, 10);
        arktest.assertEQ(iterKeys.next().value, undefined);
    });
    concurrentHashMapKeysTestsuite.addTest("keysAndTest_001", () =>
    {
        concurrentHashMapKeys.clear();
        arktest.assertTrue(concurrentHashMapKeys.isEmpty());
        let p1 = launch<void, () => void>(setAll);
        let p2 = launch<void, () => void>(keysAll);
        p1.Await();
        p2.Await();
        arktest.assertTrue(concurrentHashMapKeys.size >= 0);
    });
    concurrentHashMapKeysTestsuite.addTest("keysAndTest_002", () =>
    {
        concurrentHashMapKeys.clear();
        arktest.assertTrue(concurrentHashMapKeys.isEmpty());
        setAll();
        let p1 = launch<void, () => void>(deleteAll);
        let p2 = launch<void, () => void>(keysAll);
        p1.Await();
        p2.Await();
        arktest.assertTrue(concurrentHashMapKeys.isEmpty());
    });
    concurrentHashMapKeysTestsuite.addTest("keysAndTest_003", () =>
    {
        concurrentHashMapKeys.clear();
        arktest.assertTrue(concurrentHashMapKeys.isEmpty());
        setAll();
        let p1 = launch<void, () => void>(clearAll);
        let p2 = launch<void, () => void>(keysAll);
        p1.Await();
        p2.Await();
        arktest.assertTrue(concurrentHashMapKeys.isEmpty());
    });

    return concurrentHashMapKeysTestsuite.run();
}

function setAll() {
    for (let i: int = 0; i < 10; i++) {
        concurrentHashMapKeys.set(arrKeys[i], arrValues[i]);
    }
}

function deleteAll() {
    for (let i: int = 0; i < 10; i++) {
        if (concurrentHashMapKeys.delete(i)) {
            flag++;
        }
    }
}

function clearAll() {
    concurrentHashMapKeys.clear();
}

function keysAll() {
    let iterKeys = concurrentHashMapKeys.keys();
    for (let i = 0; iterKeys.next().value == arrKeys[i]; i++) {
        if (iterKeys.next() == undefined) {
            break;
        }
    }
}