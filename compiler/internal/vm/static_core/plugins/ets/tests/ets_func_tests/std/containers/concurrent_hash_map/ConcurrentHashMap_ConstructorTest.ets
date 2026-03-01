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

let arrKeys: number[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9];
let arrValues: string[] = ["zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine"];

function main(): int {
    let map: Map<number, string> = new Map<number, string>();
    for (let i: int = 0; i < 10 ; i++) {
        map.set(arrKeys[i], arrValues[i]);
    }
    let concurrentHashMapOneTestsuite = new arktest.ArkTestsuite("concurrentHashMapConstructorTest");

    concurrentHashMapOneTestsuite.addTest("constructorTest_001", () =>
    {
        let concurrentHashMap: containers.ConcurrentHashMap<number, string> = new containers.ConcurrentHashMap<number, string>();
        arktest.assertTrue(concurrentHashMap.isEmpty());
    });
    concurrentHashMapOneTestsuite.addTest("constructorTest_002", () =>
    {
        let array = Array<[Number, string]>(10);
        for (let i = 0; i < 10; i++) {
            let arrKV: [Number, string] = [arrKeys[i], arrValues[i]]
            array[i] = arrKV;
        }
        let arrayLike = array as ArrayLike<[Number, string]>;
        let concurrentHashMap: containers.ConcurrentHashMap<number, string> = new containers.ConcurrentHashMap<number, string>(arrayLike);
        arktest.assertEQ(concurrentHashMap.size, 10);
        for (let i = 0; i < 10; i++) {
            arktest.assertEQ(concurrentHashMap.get(arrKeys[i]), arrValues[i]);
        }
    });
    concurrentHashMapOneTestsuite.addTest("constructorTest_003", () =>
    {
        let arr: Array<[Number, string]> = new Array<[Number, string]>(10);
        for (let i: int = 0; i < 10 ; i++) {
            let arrKV: [Number, string] = [arrKeys[i], arrValues[i]]
            arr[i] = arrKV;
        }
        let concurrentHashMap: containers.ConcurrentHashMap<number, string> = new containers.ConcurrentHashMap<number, string>(arr);
        arktest.assertEQ(concurrentHashMap.size, 10);
        for (let i: int = 0; i < 10 ; i++) {
            arktest.assertEQ(concurrentHashMap.get(arrKeys[i]), arrValues[i]);
        }
    });
    concurrentHashMapOneTestsuite.addTest("constructorTest_004", () =>
    {
        let concurrentHashMap: containers.ConcurrentHashMap<number, string> = new containers.ConcurrentHashMap<number, string>(map);
        let iterable = concurrentHashMap.entries();
        for (let i = 0; i < 10; i++) {
            arktest.assertEQ(concurrentHashMap.get(arrKeys[i]), arrValues[i]);
        }
        let concurrentHashMapIter: containers.ConcurrentHashMap<number, string> = new containers.ConcurrentHashMap<number, string>(iterable);
        arktest.assertEQ(concurrentHashMapIter.size, 10);
        for (let i = 0; i < 10; i++) {
            arktest.assertEQ(concurrentHashMap.get(arrKeys[i]), arrValues[i]);
        }
    });
    concurrentHashMapOneTestsuite.addTest("constructorTest_005", () =>
    {
        let concurrentHashMap: containers.ConcurrentHashMap<number, string> = new containers.ConcurrentHashMap<number, string>(map);
        arktest.assertEQ(concurrentHashMap.size, 10);
        for (let i = 0; i < 10; i++) {
            arktest.assertEQ(concurrentHashMap.get(arrKeys[i]), arrValues[i]);
        }
    });

    return concurrentHashMapOneTestsuite.run();
}
