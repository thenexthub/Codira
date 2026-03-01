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

let flag: int = 0;

let arrKeys: number[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9];
let arrValues: string[] = ["zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine"];
let arrKeysTest: number[] = [10, 10, 10, 10, 10, 10, 10, 10, 10, 10];
let arrValuesTest: string[] = ["ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten"];

let concurrentHashMapOne: containers.ConcurrentHashMap<number, string> = new containers.ConcurrentHashMap<number, string>();

function main(): int {
    let map: Map<number, string> = new Map<number, string>();
    for (let i: int = 0; i < 10 ; i++) {
        map.set(arrKeys[i], arrValues[i]);
    }

    let concurrentHashMapTestsuite = new arktest.ArkTestsuite("concurrentHashMapSetTest");

    concurrentHashMapTestsuite.addTest("setTest_001", () =>
    {
        let concurrentHashMap: containers.ConcurrentHashMap<number, string> = new containers.ConcurrentHashMap<number, string>();
        arktest.assertTrue(concurrentHashMap.isEmpty());
        for (let i = 0; i < 10; i++) {
            concurrentHashMap.set(arrKeys[i], arrValues[i]);
            arktest.assertTrue(concurrentHashMap.has(arrKeys[i]));
            arktest.assertEQ(concurrentHashMap.get(arrKeys[i]), arrValues[i]);
        }
        arktest.assertEQ(concurrentHashMap.size, 10);
    });
    concurrentHashMapTestsuite.addTest("setTest_002", () =>
    {
        let concurrentHashMap: containers.ConcurrentHashMap<number, string> = new containers.ConcurrentHashMap<number, string>(map);
        concurrentHashMap.set(10, "ten");
        arktest.assertEQ(concurrentHashMap.size, 11);
        flag = 0;
        for (let i: int = 0; i < 10 ; i++) {
            let value: string | undefined = concurrentHashMap.get(i);
            if (value != arrValues[i]) {
                flag++;
            }
        }
        arktest.assertEQ(flag, 0);
        arktest.assertEQ(concurrentHashMap.get(10), "ten");
    });
    concurrentHashMapTestsuite.addTest("setTest_003", () =>
    {
        let concurrentHashMap: containers.ConcurrentHashMap<number, string> = new containers.ConcurrentHashMap<number, string>(map);
        concurrentHashMap.set(1, "first");
        concurrentHashMap.set(9, "ninth");
        arktest.assertEQ(concurrentHashMap.size, 10);
        flag = 0;
        for (let i: int = 0; i < 10 ; i++) {
            let value: string | undefined = concurrentHashMap.get(i);
            if (value != arrValues[i]) {
                if ((i == 1 && value == "first") || (i == 9 && value == "ninth")) {
                    continue;
                }
                flag++;
            }
        }
        arktest.assertEQ(flag, 0);
    });
    concurrentHashMapTestsuite.addTest("setTest_004", () =>
    {
        let mapNull: Map<number | null, string> = new Map<number | null, string>();
        for (let i: int = 0; i < 10 ; i++) {
            mapNull.set(arrKeys[i], arrValues[i]);
        }
        let concurrentHashMap: containers.ConcurrentHashMap<number | null, string> =
            new containers.ConcurrentHashMap<number | null, string>(mapNull);
        arktest.assertEQ(concurrentHashMap.size, 10);
        concurrentHashMap.set(null, "string");
        arktest.assertEQ(concurrentHashMap.get(null), "string");
        concurrentHashMap.set(null, "string-2");
        arktest.assertEQ(concurrentHashMap.get(null), "string-2");
        flag = 0;
        for (let i: int = 0; i < 10 ; i++) {
            let value: string | undefined = concurrentHashMap.get(i);
            if (value != arrValues[i]) {
                flag++;
            }
        }
        arktest.assertEQ(flag, 0);
        arktest.assertEQ(concurrentHashMap.size, 11);
    });
    concurrentHashMapTestsuite.addTest("setTest_005", () =>
    {
        let mapNudefind: Map<number, string | undefined> = new Map<number, string | undefined>();
        for (let i: int = 0; i < 10 ; i++) {
            mapNudefind.set(arrKeys[i], arrValues[i]);
        }
        let concurrentHashMap: containers.ConcurrentHashMap<number, string | undefined> =
            new containers.ConcurrentHashMap<number, string | undefined>(mapNudefind);
        arktest.assertEQ(concurrentHashMap.size, 10);
        concurrentHashMap.set(10, undefined);
        arktest.assertEQ(concurrentHashMap.get(10), undefined);
        arktest.assertEQ(concurrentHashMap.size, 11);
    });
    concurrentHashMapTestsuite.addTest("setTest_006", () =>
    {
        let concurrentHashMap: containers.ConcurrentHashMap<number, number> = new containers.ConcurrentHashMap<number, number>();
        for (let i: int = 0; i < 10 ; i++) {
            concurrentHashMap.set(i, i * 2);
        }
        arktest.assertEQ(concurrentHashMap.size, 10);
        for (let i: int = 0; i < 10 ; i++) {
            arktest.assertEQ(concurrentHashMap.get(i), arrKeys[i] * 2);
        }
    });
    concurrentHashMapTestsuite.addTest("setTest_007", () =>
    {
        let concurrentHashMap: containers.ConcurrentHashMap<string, number> = new containers.ConcurrentHashMap<string, number>();
        for (let i: int = 0; i < 10 ; i++) {
            concurrentHashMap.set(arrValues[i], arrKeys[i]);
        }
        arktest.assertEQ(concurrentHashMap.size, 10);
        let setItor = concurrentHashMap.entries();
        let setItorVal = setItor.next().value;
        for (let i = 0; i < 10; i++) {
            let k = setItorVal![0];
            let v = setItorVal![1];
            arktest.assertEQ(k, arrValues[i]);
            arktest.assertEQ(v, arrKeys[i]);
            setItorVal = setItor.next().value;
        }
    });

    return concurrentHashMapTestsuite.run();
}