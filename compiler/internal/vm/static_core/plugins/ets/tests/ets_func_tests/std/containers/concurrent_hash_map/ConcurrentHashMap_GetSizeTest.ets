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

    let concurrentHashMapSizeTestsuite = new arktest.ArkTestsuite("concurrentHashMapSizeTest");

    concurrentHashMapSizeTestsuite.addTest("sizeTest_001", () =>
    {
        let concurrentHashMap: containers.ConcurrentHashMap<number, string> = new containers.ConcurrentHashMap<number, string>();
        arktest.assertEQ(concurrentHashMap.size, 0);
    });
    concurrentHashMapSizeTestsuite.addTest("sizeTest_002", () =>
    {
        let concurrentHashMap: containers.ConcurrentHashMap<number, string> = new containers.ConcurrentHashMap<number, string>(map);
        arktest.assertEQ(concurrentHashMap.size, 10);
    });
    concurrentHashMapSizeTestsuite.addTest("sizeTest_003", () =>
    {
        let concurrentHashMap: containers.ConcurrentHashMap<number, string> = new containers.ConcurrentHashMap<number, string>();
        for (let i: int = 0; i < 10; i++) {
            concurrentHashMap.set(arrKeys[i], arrValues[i]);
            arktest.assertEQ(concurrentHashMap.size, i + 1);
        }
        for (let i: int = 0; i < 8; i++) {
            concurrentHashMap.delete(i);
            arktest.assertEQ(concurrentHashMap.size, 9 - i);
        }
        arktest.assertEQ(concurrentHashMap.size, 2);
    });
    concurrentHashMapSizeTestsuite.addTest("sizeTest_004", () =>
    {
        let concurrentHashMap: containers.ConcurrentHashMap<string, number> = new containers.ConcurrentHashMap<string, number>();
        for (let i: int = 0; i < 10; i++) {
            concurrentHashMap.set(arrValues[i], arrKeys[i]);
            arktest.assertEQ(concurrentHashMap.size, i + 1);
        }
        for (let i: int = 0; i < 8; i++) {
            concurrentHashMap.delete(arrValues[i]);
            arktest.assertEQ(concurrentHashMap.size, 9 - i);
        }
        arktest.assertEQ(concurrentHashMap.size, 2);
    });

    return concurrentHashMapSizeTestsuite.run();
}