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

import HashMap from "@ohos.util.HashMap";

//Test cases ported from ArkTS 1.0:container_hashmap.js
function main() {
    const suite = new arktest.ArkTestsuite("HashMap API tests");

    suite.addTest("Testing HashMap basic operations", testBasicFuncAndStateCheck);
    suite.addTest("Testing HashMap collision handling", testCollisionAndForeachSummation);
    suite.addTest("Testing HashMap iteration and entries", testEntriesAndSymbolIterator);

    return suite.run();
}

function testBasicFuncAndStateCheck() {
    let map = new HashMap<string, string>();
    arktest.assertTrue(map.isEmpty(), "Expected the HashMap to be empty after initialization");

    map.set("a", "aa");
    map.set("b", "bb");
    arktest.assertFalse(map.isEmpty(), "Expected the HashMap to not be empty after adding elements");
    arktest.assertEQ(map.length, 2);
    arktest.assertEQ(map.get("a"), "aa");
    arktest.assertEQ(map.get("b"), "bb");
    arktest.assertTrue(map.hasKey("a"), "Expected map to have key 'a'");
    arktest.assertTrue(map.hasKey("b"), "Expected map to have key 'b'");
    arktest.assertTrue(map.hasValue("aa"), "Expected map to have value 'aa'");
    arktest.assertTrue(map.hasValue("bb"), "Expected map to have value 'bb'");
    arktest.assertTrue(!map.hasKey("c"), "Expected map to not have key 'c' before it is set");
    arktest.assertTrue(!map.hasValue("cc"), "Expected map to not have value 'cc' before it is set");

    map.set("c", "cc");

    let iteratorKey = map.keys();
    arktest.assertEQ(iteratorKey.next().value, "a");
    arktest.assertEQ(iteratorKey.next().value, "b");
    arktest.assertEQ(iteratorKey.next().value, "c");
    arktest.assertEQ(iteratorKey.next().value, undefined);
    let iteratorValues = map.values();
    arktest.assertEQ(iteratorValues.next().value, "aa");
    arktest.assertEQ(iteratorValues.next().value, "bb");
    arktest.assertEQ(iteratorValues.next().value, "cc");
    arktest.assertEQ(iteratorValues.next().value, undefined);

    map.forEach((valueHashMap, keyHashMap, newMap: HashMap<string, string>) => {
        arktest.assertEQ(newMap.get(keyHashMap), valueHashMap);
    });

    let doublemap = new HashMap<double, double>();
    for (let i: int = 0; i < 10; i++) {
        doublemap.set(i, i);
    }
    let ss = Math.floor(Math.random() * 10);
    arktest.assertTrue(doublemap.hasKey(ss), "Expected map to have key ss");

    let myTest = new HashMap<double, double>();
    myTest.set(Math.floor(1.4).toInt(), 2);
    arktest.assertTrue(myTest.hasKey(1), "Expected the HashMap to contain the key 1 after setting it");

    let dmap = new HashMap<string, string>();

    dmap.setAll(map);
    arktest.assertEQ(dmap.length, 3);
    arktest.assertEQ(dmap.remove("a"), "aa");
    arktest.assertEQ(dmap.length, 2);
    arktest.assertTrue(dmap.replace("b", "dd"), "Expected replace('b', 'dd') to return true");
    arktest.assertEQ(dmap.get("b"), "dd");
    dmap.clear();
    arktest.assertEQ(dmap.length, 0);
}

function testCollisionAndForeachSummation() {
    let collisionMap = new HashMap<double, double>();
    let count = 0;

    collisionMap.set(1224, 1);
    collisionMap.set(1285, 2);
    collisionMap.set(1463, 3);
    collisionMap.set(4307, 4);
    collisionMap.set(5135, 5);
    collisionMap.set(5903, 6);
    collisionMap.set(6603, 7);
    collisionMap.set(6780, 8);
    collisionMap.set(8416, 9);
    collisionMap.set(9401, 10);
    collisionMap.set(9740, 11);
    collisionMap.forEach((value, key, hashMap) => {
        if (hashMap.get(key) == value) {
            count += value;
        }
    });
    arktest.assertEQ(count, 66);
}

function testEntriesAndSymbolIterator() {
    let map = new HashMap<string, string>();
    map.set("a", "aa");
    map.set("b", "bb");
    map.set("c", "cc");
    let iteratorEntries = map.entries();
    iteratorEntries.next().value;
    iteratorEntries.next().value;
    arktest.assertNE(iteratorEntries.next().value, undefined, "Expected the third entry in the iterator to not be undefined");
    arktest.assertEQ(iteratorEntries.next().value, undefined);

    let arr = ["aa", "bb", "cc"];
    let i: int = 0;
    for (const item of map) {
        arktest.assertEQ(item[1], arr[i]);
        i++;
    }
}
