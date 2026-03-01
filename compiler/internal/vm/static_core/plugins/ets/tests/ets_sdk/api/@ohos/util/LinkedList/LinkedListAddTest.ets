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

function main() {
    const suite = new arktest.ArkTestsuite("LinkedList API tests");

    suite.addTest("Testing LinkedList add with various data types", testLinkedListAddVariousDataTypes);
    suite.addTest("Testing LinkedList add with special characters string", testLinkedListAddSpecialString);
    suite.addTest("Testing LinkedList addFirst with various data types", testLinkedListAddFirstVariousDataTypes);
    suite.addTest("Testing LinkedList addFirst with special characters string", testLinkedListAddFirstSpecialString);
    suite.addTest("Testing LinkedList insert, addFirst, getIndexOf, removeByIndex, and has",
        testLinkedListInsertAndOperations);
    suite.addTest("Testing LinkedList insert, add, getLastIndexOf, removeByIndex, and has",
        testLinkedListInsertAddAndOperations);
    suite.addTest("Testing LinkedList has method with various data types", testLinkedListHasVariousDataTypes);
    suite.addTest("Testing LinkedList get method with various data types", testLinkedListGetVariousDataTypes);
    suite.addTest("Testing LinkedList add -> JSON stringify", testLinkedListAddJsonStringify);
    suite.addTest("Testing LinkedList addFirst -> JSON stringify", testLinkedListAddFirstJsonStringify);

    return suite.run();
}

// Test cases ported from ArkTS 1.0: LINKED_LIST_ADD_0100
function testLinkedListAddVariousDataTypes() {
    let linkList = new LinkedList<double | string | boolean | undefined | null | Object>();
    linkList.add("zhang san");
    linkList.add(1);
    let temp: Temp = {
        age: 10, name: "zhang"
    };
    linkList.add(temp);
    linkList.add(undefined);
    linkList.add(null);
    linkList.add(true);
    linkList.add([0, 1, 2, 4]);
    arktest.assertEQ(linkList.get(0), "zhang san");
    arktest.assertEQ(linkList.get(1), 1);
    arktest.assertEQ(linkList.get(2), temp);
    arktest.assertEQ(linkList.get(3), undefined);
    arktest.assertEQ(linkList.get(4), null);
    arktest.assertEQ(linkList.get(5), true);
    for (let i: int = 0; i < 4; i++) {
        arktest.assertEQ((linkList.get(6)! as Array<int>)[i], [0, 1, 2, 4][i]);
    }
}

// Test cases ported from ArkTS 1.0: LINKED_LIST_ADD_0200
function testLinkedListAddSpecialString() {
    let linkList = new LinkedList<string>();
    let param = "!\"#$%'()+,-./:;<=>?@[\\]^_`{|}~";
    linkList.add(param);
    arktest.assertEQ(linkList.get(0), param);
}

// Test cases ported from ArkTS 1.0: LINKED_LIST_ADD_FIRST_0100
function testLinkedListAddFirstVariousDataTypes() {
    let linkList = new LinkedList<double | string | boolean | undefined | null | Object>();
    linkList.addFirst("zhang san");
    linkList.addFirst(1);
    let temp: Temp = {
        age: 10, name: "zhang"
    };
    linkList.addFirst(temp);
    linkList.addFirst(undefined);
    linkList.addFirst(null);
    linkList.addFirst(true);
    linkList.addFirst([0, 1, 2, 4]);
    for (let i: int = 0; i < 4; i++) {
        arktest.assertEQ((linkList.get(0)! as Array<int>)[i], [0, 1, 2, 4][i]);
    }
    arktest.assertEQ(linkList.get(1), true);
    arktest.assertEQ(linkList.get(2), null);
    arktest.assertEQ(linkList.get(3), undefined);
    arktest.assertEQ(linkList.get(4), temp);
    arktest.assertEQ(linkList.get(5), 1);
    arktest.assertEQ(linkList.get(6), "zhang san");
}

// Test cases ported from ArkTS 1.0: LINKED_LIST_ADD_0100
function testLinkedListAddFirstSpecialString() {
    let linkList = new LinkedList<string>();
    let param = "!\"#$%'()+,-./:;<=>?@[\\]^_`{|}~";
    linkList.addFirst(param);
    arktest.assertEQ(linkList.get(0), param);
}

// Test cases ported from ArkTS 1.0: LINKED_LIST_ADD_FIRST_0200
function testLinkedListInsertAndOperations() {
    let linkList = new LinkedList<double | string>();
    let param = "!\"#$%'()+,-./:;<=>?@[\\]^_`{|}~";
    linkList.insert(0, 1);
    linkList.insert(1, 2);
    linkList.addFirst(param);
    let index = linkList.getIndexOf(param);
    linkList.removeByIndex(index);
    arktest.assertEQ(index, 0);
    arktest.assertFalse(linkList.has(param), "The LinkedList should not contain the parameter after removal");
}

// Test cases ported from ArkTS 1.0: LINKED_LIST_INSERT_0100
function testLinkedListInsertAddAndOperations() {
    let linkList = new LinkedList<double | string>();
    let param = "!\"#$%'()+,-./:;<=>?@[\\]^_`{|}~";
    linkList.insert(0, 1);
    linkList.insert(1, 2);
    linkList.add(param);
    let index = linkList.getLastIndexOf(param);
    arktest.assertEQ(index, linkList.length - 1);
    linkList.removeByIndex(index);
    arktest.assertFalse(linkList.has(param), "The LinkedList should not contain the parameter after removal");
}

// Test cases ported from ArkTS 1.0: LINKED_LIST_HAS_0100
function testLinkedListHasVariousDataTypes() {
    let linkList = new LinkedList<double | string | boolean | undefined | null | Object>();
    linkList.add("zhang san");
    linkList.add(1);
    let temp: Temp = {
        age: 10, name: "zhang"
    };
    linkList.add(temp);
    linkList.add(undefined);
    linkList.add(null);
    linkList.add(true);
    let arr = [0, 1, 2, undefined];
    linkList.add(arr);
    let param = "!\"#$%'()+,-./:;<=>?@[\\]^_`{|}~";
    linkList.add(param);
    arktest.assertTrue(linkList.has("zhang san"), "The LinkedList should contain the string 'zhang san'");
    arktest.assertTrue(linkList.has(1), "The LinkedList should contain the double 1");
    arktest.assertTrue(linkList.has(temp), "The LinkedList should contain the Temp object");
    arktest.assertTrue(linkList.has(undefined), "The LinkedList should contain 'undefined'");
    arktest.assertTrue(linkList.has(null), "The LinkedList should contain 'null'");
    arktest.assertTrue(linkList.has(true), "The LinkedList should contain 'true'");
    arktest.assertTrue(linkList.has(arr), "The LinkedList should contain the array '[0, 1, 2, undefined]'");
    arktest.assertTrue(linkList.has(param), "The LinkedList should contain the special string parameter");
}

// Test cases ported from ArkTS 1.0: LINKED_LIST_GET_0100
function testLinkedListGetVariousDataTypes() {
    let linkList = new LinkedList<double | string | boolean | undefined | null | Object>();
    linkList.add("zhang san");
    linkList.add(1);
    let temp: Temp = {
        age: 10, name: "zhang"
    };
    linkList.add(temp);
    linkList.add(undefined);
    linkList.add(null);
    linkList.add(true);
    let arr = [0, 1, 2, undefined];
    linkList.add(arr);
    let param = "!\"#$%'()+,-./:;<=>?@[\\]^_`{|}~";
    linkList.add(param);
    linkList.forEach((value, index) => {
        arktest.assertEQ(linkList.get(index), value);
    });
}

function testLinkedListAddJsonStringify() {
    let linkList = new LinkedList<number | string | Array<number | string | boolean> | boolean | undefined | null |
    Object>();
    linkList.add("zhang san");
    linkList.add(1);
    let temp: Temp = {
        age: 10, name: "zhang"
    };
    linkList.add(temp);
    linkList.add(undefined);
    linkList.add(null);
    linkList.add(true);
    linkList.add([0, 1, 2, undefined]);
    let jsonResult = JSON.stringify(linkList);
    let result = `{"0":"zhang san","1":1,"2":{"age":10,"name":"zhang"},"4":null,"5":true,"6":[0,1,2,null]}`;
    arktest.assertEQ(jsonResult, result);
}

function testLinkedListAddFirstJsonStringify() {
    let linkList = new LinkedList<number | string | Array<number | string | boolean> | boolean | undefined | null |
    Object>();
    linkList.addFirst("zhang san");
    linkList.addFirst(1);
    let temp: Temp = {
        age: 10, name: "zhang"
    };
    linkList.addFirst(temp);
    linkList.addFirst(undefined);
    linkList.addFirst(null);
    linkList.addFirst(true);
    linkList.addFirst([0, 1, 2, undefined]);
    let jsonResult = JSON.stringify(linkList);
    let result = `{"0":[0,1,2,null],"1":true,"2":null,"4":{"age":10,"name":"zhang"},"5":1,"6":"zhang san"}`;
    arktest.assertEQ(jsonResult, result);
}
