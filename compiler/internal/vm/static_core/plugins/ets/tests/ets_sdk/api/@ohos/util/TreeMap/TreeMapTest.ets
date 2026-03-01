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

import TreeMap from "@ohos.util.TreeMap";

//Test cases ported from ArkTS 1.0:container_treemap.js
function main() {
    const suite = new arktest.ArkTestsuite("TreeMap API tests");

    suite.addTest("Test TreeMap basic operations (isEmpty, set, get, hasKey, hasValue, iterators, clear, etc.)",
        testBasicOperations);
    suite.addTest("Test TreeMap with custom string comparator, clear, setAll (empty map), and remove",
        testCustomStringComparatorClearSetAllRemove);
    suite.addTest("Test TreeMap with custom object keys (Person), custom comparators, clear, and re-population",
        testCustomObjectKeysComparatorsClearRepopulate);
    suite.addTest("Test TreeMap set, clear, re-populate, and get operations", testSetClearRepopulateGet);
    suite.addTest("Test TreeMap multiple set/remove operations on the same key, and get",
        testMultipleSetRemoveSameKeyGet);
    suite.addTest("Test TreeMap entries iterator and for...of iteration", testEntriesIteratorAndForOfIteration);

    return suite.run();
}

class Person {
    id: double = 0;
    name: string = '';
    constructor(id: double, name: string) {
        this.id = id;
        this.name = name;
    }
}

function testBasicOperations(){
    let map = new TreeMap<string, string>();
    arktest.assertTrue(map.isEmpty(), "Expected map to be empty initially");
    map.set("a", "aa");
    map.set("b", "bb");
    arktest.assertFalse(map.isEmpty(), "Expected map not to be empty after adding elements");

    arktest.assertEQ(map.length, 2);
    arktest.assertEQ(map.get("a"), "aa");
    arktest.assertEQ(map.get("b"), "bb");
    arktest.assertTrue(map.hasKey("a"), "Expected map to have key 'a'");
    arktest.assertTrue(map.hasKey("b"), "Expected map to have key 'b'");
    arktest.assertTrue(map.hasValue("aa"), "Expected map to have value 'aa'");
    arktest.assertTrue(map.hasValue("bb"), "Expected map to have value 'bb'");
    arktest.assertFalse(map.hasKey("c"), "Expected map not to have key 'c'");
    arktest.assertFalse(map.hasValue("cc"), "Expected map not to have value 'cc'");

    map.set("c", "cc");
    arktest.assertEQ(map.getFirstKey(), "a");
    arktest.assertEQ(map.get(map.getFirstKey()!), "aa");
    arktest.assertEQ(map.getLastKey(), "c");
    arktest.assertEQ(map.get(map.getLastKey()!), "cc");
    arktest.assertEQ(map.getLowerKey("b"), "a");
    arktest.assertEQ(map.get(map.getLowerKey("b")!), "aa");
    arktest.assertEQ(map.getLowerKey("a"), undefined);
    arktest.assertEQ(map.getHigherKey("b"), "c");
    arktest.assertEQ(map.get(map.getHigherKey("b")!), "cc");
    arktest.assertEQ(map.getHigherKey("c"), undefined);

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

    let count = 0;
    map.forEach((value, key, newMap: TreeMap<string, string>) => {
        arktest.assertEQ(newMap.get(key), value);
        count++;
    });
    arktest.assertEQ(count, 3);

    let dmap = new TreeMap<string, string>();
    dmap.setAll(map);
    arktest.assertEQ(dmap.length, 3);
    arktest.assertEQ(dmap.remove("a"), "aa");
    arktest.assertEQ(dmap.length, 2);
    arktest.assertTrue(dmap.replace("b", "dd"), "Expected replace('b', 'dd') to return true");
    arktest.assertEQ(dmap.get("b"), "dd");
    dmap.clear();
    arktest.assertEQ(dmap.length, 0);
}

function testCustomStringComparatorClearSetAllRemove() {
    let cmmp: TreeMap<string, double> =
        new TreeMap<string, double>((firstValue: string, secondValue: string): double => {
            return firstValue.compareTo(secondValue);
        });
    cmmp.set("aa", 1);
    cmmp.set("bb", 1);
    cmmp.clear();
    let commap1 = new TreeMap<string, double>();
    commap1.setAll(cmmp);
    arktest.assertEQ(commap1.length, 0);

    let commap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): double => {
            return firstValue.compareTo(secondValue);
        });
    commap.set("c","1");
    commap.set("a","8");
    commap.set("b","2");
    commap.set("d","4");
    if (commap.length == 4) {
        commap.remove("a");
        commap.remove("b");
        commap.remove("c");
        commap.remove("d");
    }
    arktest.assertEQ(commap.length, 0);
}

function testCustomObjectKeysComparatorsClearRepopulate() {
    let commap: TreeMap<Person, string> =
        new TreeMap<Person, string>((firstValue: Person, secondValue: Person): double => {
            return firstValue.id - secondValue.id
        });
    let personone = new Person(1,'张三');
    let persontwo = new Person(3,'李四');
    let personsec = new Person(2,'王五');
    commap.set(personone,"1")
    commap.set(persontwo,"1")
    commap.set(personsec,"1")
    arktest.assertEQ(commap.getFirstKey()!.id, 3);
    arktest.assertEQ(commap.get(commap.getFirstKey()!), "1");
    commap.clear();
    let commap1: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): double => {
            return secondValue.compareTo(firstValue);
        });
    commap1.set("c","1");
    commap1.set("a","8");
    commap1.set("b","2");
    commap1.set("d","4");
    commap1.clear();
    commap1.set("c","1");
    commap1.set("a","8");
    commap1.set("b","2");
    commap1.set("d","4");
    arktest.assertEQ(commap1.getFirstKey(), "a");
    arktest.assertEQ(commap1.get(commap1.getFirstKey()!), "8");
}

function testSetClearRepopulateGet() {
    let map = new TreeMap<string, string>();
    map.set("a", "av");
    map.clear();
    map.set("b", "bv");
    map.set("c", "cv");
    arktest.assertEQ(map.get("a"), undefined);
    arktest.assertEQ(map.get("b"), "bv");
    arktest.assertEQ(map.get("c"), "cv");
}

function testMultipleSetRemoveSameKeyGet() {
    let growMap = new TreeMap<string, double>();
    const keyStr = 'KEY_TEST_MAP';
    growMap.set(keyStr,21)
    growMap.set(keyStr,68)
    growMap.remove(keyStr)
    growMap.set(keyStr,23)
    growMap.remove(keyStr)
    growMap.set(keyStr,86)
    growMap.remove(keyStr)
    growMap.set(keyStr,37)
    growMap.remove(keyStr)
    growMap.set(keyStr,33)
    growMap.remove(keyStr)
    growMap.set(keyStr,34)
    growMap.remove(keyStr)
    growMap.set(keyStr,64)
    growMap.set(keyStr,100)
    arktest.assertEQ(growMap.get(keyStr), 100);
}

function testEntriesIteratorAndForOfIteration() {
    let map = new TreeMap<string, string>();
    map.set("a", "aa");
    map.set("b", "bb");
    map.set("c", "cc");

    let iteratorEntries = map.entries();
    iteratorEntries.next().value;
    iteratorEntries.next().value;
    arktest.assertNE(iteratorEntries.next().value, undefined);
    arktest.assertEQ(iteratorEntries.next().value, undefined);

    let arr = ["aa", "bb", "cc"];
    let i: int = 0;
    for (const item of map) {
        arktest.assertEQ(item[1], arr[i]);
        i++;
    }
}
