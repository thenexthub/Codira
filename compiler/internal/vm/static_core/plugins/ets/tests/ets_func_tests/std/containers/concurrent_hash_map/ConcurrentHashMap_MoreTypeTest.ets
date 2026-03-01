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

let concurrentHashMapOne: containers.ConcurrentHashMap<number, string> = new containers.ConcurrentHashMap<number, string>();

function main(): int {
    let map: Map<number, string> = new Map<number, string>();
    for (let i: int = 0; i < 10 ; i++) {
        map.set(arrKeys[i], arrValues[i]);
    }

    let concurrentHashMapTestsuite = new arktest.ArkTestsuite("concurrentHashMapMoreTypeTest");
    // K: number, V: boolean
    concurrentHashMapTestsuite.addTest("Number&Boolean_001", () =>
    {
        let concurrentHashMap: containers.ConcurrentHashMap<number, boolean> = new containers.ConcurrentHashMap<number, boolean>();
        arktest.assertEQ(concurrentHashMap.size, 0);
        for (let i = 0; i < 10; i++) {
            let tf = (i % 2) == 0;
            concurrentHashMap.set(i, tf);
        }
        arktest.assertEQ(concurrentHashMap.size, 10);
        for (let i = 0; i < 10; i++) {
            arktest.assertEQ(concurrentHashMap.get(i), true);
            i++;
        }
        for (let i = 1; i < 10; i++) {
            arktest.assertEQ(concurrentHashMap.get(i), false);
            i++;
        }
    });
    concurrentHashMapTestsuite.addTest("Number&Boolean_002", () =>
    {
        let concurrentHashMap: containers.ConcurrentHashMap<number, boolean> = new containers.ConcurrentHashMap<number, boolean>();
        arktest.assertEQ(concurrentHashMap.size, 0);
        for (let i = 0; i < 10; i++) {
            let tf = (i % 2) == 0;
            concurrentHashMap.set(i, tf);
        }
        arktest.assertEQ(concurrentHashMap.size, 10);
        for (let i = 0; i < 10; i++) {
            let val = concurrentHashMap.get(i) as boolean;
            if (val) {
                concurrentHashMap.replace(i, false);
            } else {
                concurrentHashMap.replace(i, true);
            }
        }
        arktest.assertEQ(concurrentHashMap.size, 10);
        for (let i = 0; i < 10; i++) {
            arktest.assertEQ(concurrentHashMap.get(i), false);
            i++;
        }
        for (let i = 1; i < 10; i++) {
            arktest.assertEQ(concurrentHashMap.get(i), true);
            i++;
        }
    });
    concurrentHashMapTestsuite.addTest("Number&Boolean_003", () =>
    {
        let concurrentHashMap: containers.ConcurrentHashMap<number, boolean> = new containers.ConcurrentHashMap<number, boolean>();
        arktest.assertEQ(concurrentHashMap.size, 0);
        for (let i = 0; i < 10; i++) {
            let tf = (i % 2) == 0;
            concurrentHashMap.set(i, tf);
        }
        arktest.assertEQ(concurrentHashMap.size, 10);
        for (let i = 0; i < 10; i++) {
            arktest.assertTrue(concurrentHashMap.delete(i));
            arktest.assertEQ(concurrentHashMap.size, 9 - i);
        }
    });
    concurrentHashMapTestsuite.addTest("Number&Boolean_004", () =>
    {
        let concurrentHashMap: containers.ConcurrentHashMap<number, boolean> = new containers.ConcurrentHashMap<number, boolean>();
        arktest.assertEQ(concurrentHashMap.size, 0);
        for (let i = 0; i < 10; i++) {
            let tf = (i % 2) == 0;
            concurrentHashMap.set(i, tf);
        }
        arktest.assertEQ(concurrentHashMap.size, 10);
        for (let i = 0; i < 10; i++) {
            let res = concurrentHashMap.remove(i) as boolean;
            arktest.assertEQ(res, (i % 2) == 0);
        }
        arktest.assertEQ(concurrentHashMap.size, 0);
    });
    concurrentHashMapTestsuite.addTest("Number&Boolean_005", () =>
    {
        let concurrentHashMap: containers.ConcurrentHashMap<number, boolean> = new containers.ConcurrentHashMap<number, boolean>();
        arktest.assertEQ(concurrentHashMap.size, 0);
        for (let i = 0; i < 10; i++) {
            let tf = (i % 2) == 0;
            concurrentHashMap.set(i, tf);
        }
        arktest.assertEQ(concurrentHashMap.size, 10);
        let iter_NB = concurrentHashMap.entries();
        for (let i = 0; i < 10; i++) {
            if ((i % 2) == 0) {
                arktest.assertTrue(iter_NB.next().value![1]);
            } else {
                arktest.assertFalse(iter_NB.next().value![1]);
            }
        }
    });
    concurrentHashMapTestsuite.addTest("Number&Boolean_006", () =>
    {
        let concurrentHashMap: containers.ConcurrentHashMap<number, boolean> = new containers.ConcurrentHashMap<number, boolean>();
        arktest.assertEQ(concurrentHashMap.size, 0);
        for (let i = 0; i < 10; i++) {
            let tf = (i % 2) == 0;
            concurrentHashMap.set(i, tf);
        }
        concurrentHashMap.forEach((val: boolean, key: number, map: containers.ConcurrentHashMap<number, boolean>) =>
        {
            let str = map.toString();
            arktest.assertEQ(str, "0,true,1,false,2,true,3,false,4,true,5,false,6,true,7,false,8,true,9,false");
            if (key % 2 == 0) {
                arktest.assertTrue(val);
            } else {
                arktest.assertFalse(val);
            }
        });
    });
    // K: number, V: Object
    concurrentHashMapTestsuite.addTest("Number&Object_001", () =>
    {
        let hasMapNO: containers.ConcurrentHashMap<number, Array<string>> = new containers.ConcurrentHashMap<number, Array<string>>();
        for (let i = 0; i < 5; i++) {
            let array: Array<string> = new Array<string>(5);
            for (let j = 0; j < i + 1; j++) {
                array[j] = arrValues[j];
            }
            hasMapNO.set(i, array);
            let arrGet = hasMapNO.get(i) as Array<string>;
            arktest.assertEQ(array, arrGet);
        }
    });
    concurrentHashMapTestsuite.addTest("Number&Object_002", () =>
    {
        let hasMapNO: containers.ConcurrentHashMap<number, Array<string>> = new containers.ConcurrentHashMap<number, Array<string>>();
        for (let i = 0; i < 5; i++) {
            let array: Array<string> = new Array<string>(5);
            for (let j = 0; j < i + 1; j++) {
                array[j] = arrValues[j];
            }
            hasMapNO.set(i, array);
        }
        for (let i = 0; i < 5; i++) {
            let array: Array<string> = new Array<string>(1);
            array[0] = "six";
            hasMapNO.replace(i, array);
            let arrGet = hasMapNO.get(i) as Array<string>;
            arktest.assertEQ(array, arrGet);
        }
    });
    concurrentHashMapTestsuite.addTest("Number&Object_003", () =>
    {
        let hasMapNO: containers.ConcurrentHashMap<number, Array<string>> = new containers.ConcurrentHashMap<number, Array<string>>();
        for (let i = 0; i < 5; i++) {
            let array: Array<string> = new Array<string>(5);
            for (let j = 0; j < i + 1; j++) {
                array[j] = arrValues[j];
            }
            hasMapNO.set(i, array);
        }
        for (let i = 0; i < 5; i++) {
            arktest.assertTrue(hasMapNO.delete(i));
            arktest.assertEQ(hasMapNO.size, 4 - i);
        }
    });
    concurrentHashMapTestsuite.addTest("Number&Object_004", () =>
    {
        let hasMapNO: containers.ConcurrentHashMap<number, Array<string>> = new containers.ConcurrentHashMap<number, Array<string>>();
        for (let i = 0; i < 5; i++) {
            let array: Array<string> = new Array<string>(5);
            for (let j = 0; j < i + 1; j++) {
                array[j] = arrValues[j];
            }
            hasMapNO.set(i, array);
        }
        for (let i = 0; i < 5; i++) {
            let arrRes = hasMapNO.remove(i) as Array<string>;
            for (let j = 0; j < i + 1; j++) {
                arktest.assertEQ(arrValues[j], arrRes[j]);
            }
            arktest.assertEQ(hasMapNO.size, 4 - i);
        }
    });
    concurrentHashMapTestsuite.addTest("Number&Object_005", () =>
    {
        let hasMapNO: containers.ConcurrentHashMap<number, Array<string>> = new containers.ConcurrentHashMap<number, Array<string>>();
        for (let i = 0; i < 5; i++) {
            let array: Array<string> = new Array<string>(5);
            for (let j = 0; j < i + 1; j++) {
                array[j] = arrValues[j];
            }
            hasMapNO.set(i, array);
        }
        let iter_NO = hasMapNO.entries();
        for (let i = 0; i < 5; i++) {
            let array: Array<string> = new Array<string>(5);
            for (let j = 0; j < i + 1; j++) {
                array[j] = arrValues[j];
            }
            let arrIter = iter_NO.next().value![1];
            for (let j = 0; j < i + 1; j++) {
                arktest.assertEQ(array[j], arrIter[j]);
            }
        }
        arktest.assertEQ(iter_NO.next().value, undefined);
    });
    concurrentHashMapTestsuite.addTest("Number&Object_006", () =>
    {
        let hasMapNO: containers.ConcurrentHashMap<number, Array<string>> = new containers.ConcurrentHashMap<number, Array<string>>();
        for (let i = 0; i < 5; i++) {
            let array: Array<string> = new Array<string>(5);
            for (let j = 0; j < i + 1; j++) {
                array[j] = arrValues[j];
            }
            hasMapNO.set(i, array);
        }
        hasMapNO.forEach((val: Array<string>, key: number, map: containers.ConcurrentHashMap<number, Array<string>>) =>
        {
            arktest.assertEQ(map.get(key), val);
        });
    });

    concurrentHashMapTestsuite.addTest("entriesOfNumber", () =>
    {
        let concurrentHashMap: containers.ConcurrentHashMap<number, number> = new containers.ConcurrentHashMap<number, number>();
        for (let i: number = 0; i < 10 ; i++) {
            concurrentHashMap.set(i, i);
        }
        let iterEntries = concurrentHashMap.entries();
        let str = "";
        for (let i: int = 0; i < 10; i++) {
            str += iterEntries.next().value!.toString();
            if (i < 9){
                str += ",";
            }
        }
        arktest.assertEQ(iterEntries.next().value, undefined);
    });
    concurrentHashMapTestsuite.addTest("entriesOfNumber", () =>
    {
        let concurrentHashMap: containers.ConcurrentHashMap<string, string> = new containers.ConcurrentHashMap<string, string>();
        for (let i: int = 0; i < 10 ; i++) {
            concurrentHashMap.set(arrValues[i], arrValues[i]);
        }
        let iterEntries = concurrentHashMap.entries();
        let str = "";
        for (let i: int = 0; i < 10; i++) {
            str += iterEntries.next().value!.toString();
            if (i < 9){
                str += ",";
            }
        }
        arktest.assertEQ(iterEntries.next().value, undefined);
    });


    return concurrentHashMapTestsuite.run();
}
