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
let strAll: string = "0,zero,1,one,2,two,3,three,4,four,5,five,6,six,7,seven,8,eight,9,nine";

let concurrentHashMapOne: containers.ConcurrentHashMap<number, string> = new containers.ConcurrentHashMap<number, string>();

function main(): int {
    let map: Map<number, string> = new Map<number, string>();
    for (let i: int = 0; i < 10 ; i++) {
        map.set(arrKeys[i], arrValues[i]);
    }
    let concurrentHashMapIteratorTestsuite = new arktest.ArkTestsuite("concurrentHashMapIteratorTest");

    concurrentHashMapIteratorTestsuite.addTest("$_iteratorTest_001", () =>
    {
        let concurrentHashMap: containers.ConcurrentHashMap<number, string> = new containers.ConcurrentHashMap<number, string>();
        let iter_$: IterableIterator<[Number, string]> = concurrentHashMap.$_iterator();
        arktest.assertEQ(iter_$.next().value, undefined);
    });
    concurrentHashMapIteratorTestsuite.addTest("$_iteratorTest_002", () =>
    {
        let concurrentHashMap: containers.ConcurrentHashMap<number, string> = new containers.ConcurrentHashMap<number, string>(map);
        let iter_$: IterableIterator<[Number, string]> = concurrentHashMap.$_iterator();
        let str = "";
        for (let i: int = 0; i < 10; i++) {
            str += iter_$.next().value!.toString();
            if (i < 9){
                str += ",";
            }
        }
        arktest.assertEQ(str, strAll);
        arktest.assertEQ(iter_$.next().value, undefined);
    });
    concurrentHashMapIteratorTestsuite.addTest("$_iteratorTest_003", () =>
    {
        let concurrentHashMap: containers.ConcurrentHashMap<number, string> = new containers.ConcurrentHashMap<number, string>();
        let iter_$: IterableIterator<[Number, string]> = concurrentHashMap.$_iterator();
        arktest.assertEQ(iter_$.next().value, undefined);
        for (let i: int = 0; i < 10; i++) {
            concurrentHashMap.set(arrKeys[i], arrValues[i]);
        }
        iter_$ = concurrentHashMap.$_iterator();
        let str = "";
        for (let i: int = 0; i < 10; i++) {
            str += iter_$.next().value!.toString();
            if (i < 9){
                str += ",";
            }
        }
        arktest.assertEQ(str, strAll);
        arktest.assertEQ(iter_$.next().value, undefined);
    });
    concurrentHashMapIteratorTestsuite.addTest("$_iteratorTest_004", () =>
    {
        let concurrentHashMap: containers.ConcurrentHashMap<string, number> = new containers.ConcurrentHashMap<string, number>();
        for (let i = 0; i < 10; i++) {
            concurrentHashMap.set(arrValues[i], arrKeys[i]);
        }
        let iter = concurrentHashMap.$_iterator();
        for (let i = 0; i < 10; i++) {
            let iterNext = iter.next();
            let k = iterNext.value![0];
            let v = iterNext.value![1];
            arktest.assertEQ(k, arrValues[i]);
            arktest.assertEQ(v, arrKeys[i]);
        }
    });
    concurrentHashMapIteratorTestsuite.addTest("$_iteratorAndTest_001", () =>
    {
        concurrentHashMapOne.clear();
        arktest.assertTrue(concurrentHashMapOne.isEmpty());
        let p1 = launch<void, () => void>(setAll);
        let p2 = launch<void, () => void>($_iteratorAll);
        p1.Await();
        p2.Await();
        arktest.assertEQ(concurrentHashMapOne.size, 10);
    });
    concurrentHashMapIteratorTestsuite.addTest("$_iteratorAndTest_002", () =>
    {
        concurrentHashMapOne.clear();
        arktest.assertTrue(concurrentHashMapOne.isEmpty());
        setAll();
        let p1 = launch<void, () => void>(deleteAll);
        let p2 = launch<void, () => void>($_iteratorAll);
        p1.Await();
        p2.Await();
        arktest.assertTrue(concurrentHashMapOne.isEmpty());
    });
    concurrentHashMapIteratorTestsuite.addTest("$_iteratorAndTest_003", () =>
    {
        concurrentHashMapOne.clear();
        arktest.assertTrue(concurrentHashMapOne.isEmpty());
        setAll();
        let p1 = launch<void, () => void>(clearAll);
        let p2 = launch<void, () => void>($_iteratorAll);
        p1.Await();
        p2.Await();
        arktest.assertTrue(concurrentHashMapOne.isEmpty());
    });

    return concurrentHashMapIteratorTestsuite.run();
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

function $_iteratorAll() {
    let iterator$ = concurrentHashMapOne.$_iterator();
    let i = 0;
    while (iterator$.next().value != undefined) {
        i++;
    }
}
