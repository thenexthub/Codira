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

import {TreeSet} from "@ohos.util.TreeSet";

//Test cases ported from ArkTS 1.0:container_treeset.js
function main() {
    const suite = new arktest.ArkTestsuite("TreeSet API tests");

    suite.addTest("Test TreeSet basic operations", testBasicOperations);
    suite.addTest("Test TreeSet with custom string comparators, add, remove", testCustomStringComparatorsAddRemove);
    suite.addTest("Test TreeSet with custom object (Person) and comparator, clear, and re-populate with strings",
        testCustomObjectComparatorClearRepopulateStrings);
    suite.addTest("Test TreeSet with custom object (c) and comparator, getHigherValue, getLowerValue",
        testCustomObjectComparatorGetHigherLowerValue);
    suite.addTest("Test TreeSet entries() iterator and for...of loop", testEntriesIteratorAndForOfLoop);

    return suite.run();
}

class c {
    n: double = 0;
    constructor(a: double) {
        this.n = a;
    }
}

class Person {
    id: double = 0;
    name: string = '';
    constructor(id: double, name: string) {
        this.id = id;
        this.name = name;
    }
}

function testBasicOperations() {
    let treeSet = new TreeSet<string>();
    treeSet.add("aa");
    treeSet.add("bb");

    arktest.assertEQ(treeSet.length, 2);
    arktest.assertTrue(treeSet.has("aa"), "Expected has('aa') to be true");
    arktest.assertTrue(treeSet.has("bb"), "Expected has('bb') to be true");
    arktest.assertFalse(treeSet.has("cc"), "Expected has('cc') to be false initially");

    treeSet.add("cc");
    arktest.assertEQ(treeSet.getFirstValue(), "aa");
    arktest.assertEQ(treeSet.getLastValue(), "cc");
    arktest.assertEQ(treeSet.getLowerValue("bb"), "aa");
    arktest.assertEQ(treeSet.getLowerValue("aa"), undefined);
    arktest.assertEQ(treeSet.getHigherValue("bb"), "cc");
    arktest.assertEQ(treeSet.getHigherValue("cc"), undefined);

    let iteratorSetValues = treeSet.values();
    arktest.assertEQ(iteratorSetValues.next().value, "aa");
    arktest.assertEQ(iteratorSetValues.next().value, "bb");
    arktest.assertEQ(iteratorSetValues.next().value, "cc");
    arktest.assertEQ(iteratorSetValues.next().value, undefined);

    let itemArr = ["aa", "bb", "cc"]
    let item: string;
    let i: int = 0;
    for (item of treeSet) {
        arktest.assertEQ(item, itemArr[i]);
        i++;
    }

    treeSet.forEach((value, key, treeSet) => {
        arktest.assertTrue(treeSet.has(key), "Expected treeSet.has(key) to be true in forEach");
        arktest.assertTrue(treeSet.has(value), "Expected treeSet.has(value) to be true in forEach");
    });

    arktest.assertFalse(treeSet.isEmpty(), "Expected isEmpty() to be false when TreeSet is not empty");

    treeSet.add("ee");
    treeSet.add("dd");
    arktest.assertEQ(treeSet.length, 5);
    arktest.assertEQ(treeSet.popFirst(), "aa");
    arktest.assertEQ(treeSet.popLast(), "ee");
    arktest.assertFalse(treeSet.has("aa"), "Expected has('aa') to be false after popFirst()");

    arktest.assertTrue(treeSet.remove("bb"), "Expected remove('bb') to return true");
    arktest.assertEQ(treeSet.length, 2);
    arktest.assertFalse(treeSet.has("bb"), "Expected has('bb') to be false after removing it");
    treeSet.clear();
    arktest.assertEQ(treeSet.length, 0);
    arktest.assertFalse(treeSet.has("cc"), "Expected has('cc') to be false after clear()");
    arktest.assertTrue(treeSet.isEmpty(), "Expected isEmpty() to be true after clear()");
}

function testCustomStringComparatorsAddRemove() {
    let cmmp = new TreeSet<string>((firstValue: string,
        secondValue: string): double => { return firstValue.compareTo(secondValue) });
    cmmp.add("aa");
    cmmp.add("bb");

    let comtreeSet =  new TreeSet<string>((firstValue: string,
        secondValue: string): double => { return secondValue.compareTo(firstValue) });
    comtreeSet.add("c");
    comtreeSet.add("a");
    comtreeSet.add("b");
    comtreeSet.add("d");
    if (comtreeSet.length == 4) {
        comtreeSet.remove("a");
        comtreeSet.remove("b");
        comtreeSet.remove("c");
        comtreeSet.remove("d");
    }
    arktest.assertEQ(comtreeSet.length, 0);
}

function testCustomObjectComparatorClearRepopulateStrings() {
    let comtreeSet =  new TreeSet<Person>((firstValue: Person,
        secondValue: Person): double => {return secondValue.id - firstValue.id});
    let personone = new Person(3,'张三');
    let persontwo = new Person(1,'李四');
    let personsec = new Person(2,'王五');
    comtreeSet.add(personone);
    comtreeSet.add(persontwo);
    comtreeSet.add(personsec);
    arktest.assertEQ(comtreeSet.getFirstValue()!.id, 1);
    comtreeSet.clear();
    let comtreeSet1 =  new TreeSet<string>((firstValue: string,
        secondValue: string): double => { return firstValue.compareTo(secondValue) });
    comtreeSet1.add("c");
    comtreeSet1.add("a");
    comtreeSet1.add("b");
    comtreeSet1.add("d");
    comtreeSet1.clear();
    comtreeSet1.add("c");
    comtreeSet1.add("a");
    comtreeSet1.add("b");
    comtreeSet1.add("d");
    arktest.assertEQ(comtreeSet1.getFirstValue(), "d");
}

function testCustomObjectComparatorGetHigherLowerValue() {
    let newtreeSet = new TreeSet<c>((x: c, y: c): double => y.n - x.n);
    newtreeSet.add(new c(3));
    let tmp_c = new c(5);
    newtreeSet.add(tmp_c);
    newtreeSet.add(new c(1));
    arktest.assertEQ(newtreeSet.getHigherValue(new c(3))!.n, 5);
    arktest.assertEQ(newtreeSet.getHigherValue(new c(5)), undefined);
    arktest.assertEQ(newtreeSet.getLowerValue(new c(3))!.n, 1);
}

function testEntriesIteratorAndForOfLoop() {
    let treeSet = new TreeSet<string>();
    treeSet.add("aa");
    treeSet.add("bb");
    treeSet.add("cc");
    let iteratorSetEntries = treeSet.entries();
    iteratorSetEntries.next().value;
    iteratorSetEntries.next().value;
    arktest.assertNE(iteratorSetEntries.next().value, undefined);
    arktest.assertEQ(iteratorSetEntries.next().value, undefined);

    let arr = ["aa", "bb", "cc"];
    let i: int = 0;
    for (const item of treeSet) {
        arktest.assertEQ(item, arr[i]);
        i++;
    }
}
