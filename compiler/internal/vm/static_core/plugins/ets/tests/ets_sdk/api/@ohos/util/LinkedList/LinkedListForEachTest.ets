/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import { LinkedList } from "@ohos.util.LinkedList";

class Temp {
    age: double = 0;
    name: string = "";
}

class Temp1 {
    age: double = 0;
    name: string = "";

    constructor(age: double, name: string) {
        this.age = age;
        this.name = name;
    }
}

function main() {
    const suite = new arktest.ArkTestsuite("LinkedList API tests");

    suite.addTest("Testing LinkedList set and update data", testLinkedListSetAndUpdate);
    suite.addTest("Testing LinkedList set with getIndexOf, set, and removeByIndex",
        testLinkedListSetWithIndexAndRemove);
    suite.addTest("Testing LinkedList set with get, set, and has", testLinkedListSetWithGetAndHas);
    suite.addTest("Testing LinkedList set with convertToArray and instanceof", testLinkedListSetWithConvertToArray);
    suite.addTest("Testing LinkedList set and getFirst", testLinkedListSetAndGetFirst);
    suite.addTest("Testing LinkedList set and getLast", testLinkedListSetAndGetLast);
    suite.addTest("Testing LinkedList Symbol.iterator with for-of loop", testLinkedListSymbolIteratorForOf);
    suite.addTest("Testing LinkedList Symbol.iterator with next method", testLinkedListSymbolIteratorNext);
    suite.addTest("Testing LinkedList forEach with value and index validation", testLinkedListForEachValueAndIndex);
    suite.addTest("Testing LinkedList forEach with additional linked list validation",
        testLinkedListForEachAdditionalValidation);
    suite.addTest("Testing LinkedList forEach with value-only validation", testLinkedListForEachValueOnly);
    suite.addTest("Testing LinkedList forEach with value, index, and linked list validation",
        testLinkedListForEachFullValidation);

    return suite.run();
}

// Test cases ported from ArkTS 1.0: LINKED_LIST_SET_0100
function testLinkedListSetAndUpdate() {
    let linkList = new LinkedList<double | string | boolean | undefined | null | Object>();
    linkList.add("zhang san");
    linkList.add(1);
    let param: Temp = {
        age: 10, name: "zhang"
    };
    linkList.add(param);
    linkList.add(undefined);
    linkList.add(null);
    linkList.add(true);
    linkList.add([0, 1, 2, 4]);
    linkList.forEach((value, index: int) => {
        arktest.assertNE(linkList.set(index, "li si"), undefined);
    });
}

// Test cases ported from ArkTS 1.0: LINKED_LIST_SET_0200
function testLinkedListSetWithIndexAndRemove() {
    let linkList = new LinkedList<double | string | boolean | undefined | null | Object>();
    linkList.add("zhang san");
    linkList.add(1);
    let param: Temp = {
        age: 10, name: "zhang"
    };
    linkList.add(param);
    linkList.add(undefined);
    linkList.add(null);
    linkList.add(true);
    linkList.add([0, 1, 2, 4]);
    let index = linkList.getIndexOf(param);
    let setParam = linkList.set(index, "li si");
    let removeParam = linkList.removeByIndex(index);
    arktest.assertEQ(setParam, removeParam);
}

// Test cases ported from ArkTS 1.0: LINKED_LIST_SET_0300
function testLinkedListSetWithGetAndHas() {
    let linkList = new LinkedList<double | string | boolean | undefined | null | Object>();
    linkList.add("zhang san");
    linkList.add(1);
    let param: Temp = {
        age: 10, name: "zhang"
    };
    linkList.add(param);
    linkList.add(undefined);
    linkList.add(null);
    linkList.add(true);
    linkList.add([0, 1, 2, 4]);
    let index = linkList.getIndexOf(param);
    let setParam = linkList.set(index, "li si");
    arktest.assertTrue(linkList.has(setParam), "The LinkedList should contain the element set by the set method");
}

// Test cases ported from ArkTS 1.0: LINKED_LIST_SET_0400
function testLinkedListSetWithConvertToArray() {
    let linkList = new LinkedList<double | string | boolean | undefined | null | Object>();
    linkList.add("zhang san");
    linkList.add(1);
    let param: Temp1 = new Temp1(10, "zhang");
    linkList.add(param);
    linkList.add(undefined);
    linkList.add(null);
    linkList.add(true);
    linkList.add([0, 1, 2, 4]);
    linkList.forEach((value, index) => {
        linkList.set(index, new Temp1(index, "zhang " + index));
    });
    let array = linkList.convertToArray();
    array.forEach((value) => {
        arktest.assertTrue(value instanceof Temp1,
            "Each element in the converted array should be an instance of Temp1");
    });
}

// Test cases ported from ArkTS 1.0: LINKED_LIST_SET_0500
function testLinkedListSetAndGetFirst() {
    let linkList = new LinkedList<double | string | boolean | undefined | null | Object>();

    linkList.add("zhang san");
    linkList.add(1);
    let param: Temp1 = new Temp1(10, "zhang");
    linkList.add(param);
    linkList.add(undefined);
    linkList.add(null);
    linkList.add(true);
    linkList.add([0, 1, 2, 4]);
    let setParam = linkList.set(0, "li si");
    let firstParam = linkList.getFirst();
    arktest.assertEQ(setParam, firstParam);
}

// Test cases ported from ArkTS 1.0: LINKED_LIST_SET_0600
function testLinkedListSetAndGetLast() {
    let linkList = new LinkedList<double | string | boolean | undefined | null | Object>();
    linkList.add("zhang san");
    linkList.add(1);
    let param: Temp1 = new Temp1(10, "zhang");
    linkList.add(param);
    linkList.add(undefined);
    linkList.add(null);
    linkList.add(true);
    linkList.add([0, 1, 2, 4]);
    let setParam = linkList.set(linkList.length - 1, "li si");
    let lastParam = linkList.getLast();
    arktest.assertEQ(setParam, lastParam);
}

// Test cases ported from ArkTS 1.0: LINKED_LIST_SYMBOL_ITERATOR_0100
function testLinkedListSymbolIteratorForOf() {
    let linkList = new LinkedList<double>();
    linkList.add(1);
    linkList.add(2);
    linkList.add(3);
    linkList.add(4);
    let iter = linkList.$_iterator();
    for (let tmp of iter) {
        arktest.assertTrue(linkList.has(tmp), `The LinkedList should contain the element '${tmp}'`);
    }
}

// Test cases ported from ArkTS 1.0: LINKED_LIST_SYMBOL_ITERATOR_0200
function testLinkedListSymbolIteratorNext() {
    let linkList = new LinkedList<double>();
    linkList.add(1);
    linkList.add(2);
    linkList.add(3);
    linkList.add(4);
    let iter = linkList.$_iterator();
    let tmp = iter.next();
    while (!tmp.done) {
        if (tmp.value) {
            arktest.assertTrue(linkList.has(tmp.value!), `The LinkedList should contain the element '${tmp.value}'`);
        }
        tmp = iter.next();
    }
}

// Test cases ported from ArkTS 1.0: LINKED_LIST_FOREACH_0100
function testLinkedListForEachValueAndIndex() {
    let linkedList = new LinkedList<double>();
    linkedList.add(2);
    linkedList.add(4);
    linkedList.add(5);
    linkedList.add(4);
    linkedList.forEach((value: double, index?: int) => {
        arktest.assertEQ(linkedList.get(index!), value);
    });
}

// Test cases ported from ArkTS 1.0: LINKED_LIST_FOREACH_0200
function testLinkedListForEachAdditionalValidation() {
    let linkedList = new LinkedList<double>();
    let linkedListNew = new LinkedList<double>();
    linkedList.add(2);
    linkedList.add(4);
    linkedList.add(5);
    linkedList.add(4);
    linkedListNew.add(0);
    linkedList.forEach((value: double, index?: int) => {
        arktest.assertEQ(linkedList.get(index!), value);
    });
    arktest.assertEQ(linkedListNew.length, 1);
}

// Test cases ported from ArkTS 1.0: LINKED_LIST_FOREACH_0300
function testLinkedListForEachValueOnly() {
    let linkedList = new LinkedList<double>();
    linkedList.add(2);
    linkedList.add(4);
    linkedList.add(5);
    linkedList.add(4);
    let index = 0;
    linkedList.forEach((value: double) => {
        arktest.assertEQ(linkedList.get(index), value);
        index++;
    });
    arktest.assertEQ(index, linkedList.length);
}

// Test cases ported from ArkTS 1.0: LINKED_LIST_FOREACH_0400
function testLinkedListForEachFullValidation() {
    let linkedList = new LinkedList<double>();
    linkedList.add(2);
    linkedList.add(4);
    linkedList.add(5);
    linkedList.add(4);
    linkedList.forEach((value: double, index: int, linkedList) => {
        arktest.assertEQ(linkedList.get(index), value);
    });
}
