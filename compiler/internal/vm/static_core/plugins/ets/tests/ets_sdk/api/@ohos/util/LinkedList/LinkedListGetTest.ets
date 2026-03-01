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

function main() {
    const suite = new arktest.ArkTestsuite("LinkedList API tests");

    suite.addTest("Testing LinkedList getIndexOf with duplicate data", testLinkedListGetIndexOfDuplicateData);
    suite.addTest("Testing LinkedList getIndexOf with add, getIndexOf, and removeByIndex",
        testLinkedListGetIndexOfAddRemove);
    suite.addTest("Testing LinkedList getIndexOf with add, removeFirst, and re-add",
        testLinkedListGetIndexOfRemoveFirstReAdd);
    suite.addTest("Testing LinkedList getIndexOf with add, removeLast, and re-add",
        testLinkedListGetIndexOfRemoveLastReAdd);
    suite.addTest("Testing LinkedList getLastIndexOf with duplicate data", testLinkedListGetLastIndexOfDuplicateData);
    suite.addTest("Testing LinkedList getLastIndexOf with add, getLastIndexOf, and removeByIndex",
        testLinkedListGetLastIndexOfAddRemove);
    suite.addTest("Testing LinkedList getLastIndexOf with add, removeFirst, and re-add",
        testLinkedListGetLastIndexOfRemoveFirstReAdd);
    suite.addTest("Testing LinkedList getLastIndexOf with add, removeLast, and re-add",
        testLinkedListGetLastIndexOfRemoveLastReAdd);
    suite.addTest("Testing LinkedList get and removeFirstFound", testLinkedListGetAndRemoveFirstFound);

    return suite.run();
}

// Test cases ported from ArkTS 1.0: LINKED_LIST_GETINDEXOF_0100
function testLinkedListGetIndexOfDuplicateData() {
    let linkList = new LinkedList<double>();
    linkList.add(1);
    linkList.add(2);
    let index1 = linkList.getIndexOf(2);
    linkList.add(2);
    let index2 = linkList.getIndexOf(2);
    linkList.add(2);
    let index3 = linkList.getIndexOf(2);
    arktest.assertEQ(index1, index2);
    arktest.assertEQ(index2, index3);
}

// Test cases ported from ArkTS 1.0: LINKED_LIST_GETINDEXOF_0200
function testLinkedListGetIndexOfAddRemove() {
    let linkList = new LinkedList<double>();
    linkList.add(1);
    linkList.add(2);
    linkList.add(2);
    linkList.add(3);
    linkList.add(3);
    let index101 = linkList.removeByIndex(linkList.getIndexOf(2));
    let index102 = linkList.removeByIndex(linkList.getIndexOf(2));
    arktest.assertEQ(index101, index102);
    let index103 = linkList.removeByIndex(linkList.getIndexOf(3));
    let index104 = linkList.removeByIndex(linkList.getIndexOf(3));
    arktest.assertEQ(index103, index104);
}

// Test cases ported from ArkTS 1.0: LINKED_LIST_GETINDEXOF_0300
function testLinkedListGetIndexOfRemoveFirstReAdd() {
    let linkList = new LinkedList<double>();
    linkList.add(1);
    linkList.add(2);
    linkList.add(3);
    linkList.add(4);
    linkList.add(5);
    let index = linkList.removeFirst();
    linkList.add(index!);
    arktest.assertEQ(linkList.getIndexOf(index!), linkList.length - 1);
}

// Test cases ported from ArkTS 1.0: LINKED_LIST_GETINDEXOF_0400
function testLinkedListGetIndexOfRemoveLastReAdd() {
    let linkList = new LinkedList<double>();
    linkList.add(1);
    linkList.add(2);
    linkList.add(3);
    linkList.add(4);
    linkList.add(5);
    let index = linkList.removeLast();
    linkList.add(index!);
    arktest.assertEQ(linkList.getIndexOf(index!), linkList.length - 1);
}

// Test cases ported from ArkTS 1.0: LINKED_LIST_GETLASTINDEXOF_0100
function testLinkedListGetLastIndexOfDuplicateData() {
    let linkList = new LinkedList<double>();
    linkList.add(1);
    linkList.add(2);
    let index1 = linkList.getLastIndexOf(2);
    linkList.add(2);
    let index2 = linkList.getLastIndexOf(2);
    linkList.add(2);
    let index3 = linkList.getLastIndexOf(2);
    arktest.assertNE(index1, index2);
    arktest.assertNE(index2, index3);
}

// Test cases ported from ArkTS 1.0: LINKED_LIST_GETLASTINDEXOF_0200
function testLinkedListGetLastIndexOfAddRemove() {
    let linkList = new LinkedList<double>();
    linkList.add(1);
    linkList.add(2);
    linkList.add(2);
    linkList.add(3);
    linkList.add(3);
    let index101 = linkList.removeByIndex(linkList.getLastIndexOf(2));
    let index102 = linkList.removeByIndex(linkList.getLastIndexOf(2));
    arktest.assertEQ(index101, index102);
    let index103 = linkList.removeByIndex(linkList.getLastIndexOf(3));
    let index104 = linkList.removeByIndex(linkList.getLastIndexOf(3));
    arktest.assertEQ(index103, index104);
}

// Test cases ported from ArkTS 1.0: LINKED_LIST_GETLASTINDEXOF_0300
function testLinkedListGetLastIndexOfRemoveFirstReAdd() {
    let linkList = new LinkedList<double>();
    linkList.add(1);
    linkList.add(2);
    linkList.add(3);
    linkList.add(4);
    linkList.add(5);
    let index = linkList.removeFirst();
    linkList.add(index!);
    arktest.assertEQ(linkList.getLastIndexOf(index!), linkList.length - 1);
}

// Test cases ported from ArkTS 1.0: LINKED_LIST_GETLASTINDEXOF_0400
function testLinkedListGetLastIndexOfRemoveLastReAdd() {
    let linkList = new LinkedList<double>();
    linkList.add(1);
    linkList.add(2);
    linkList.add(3);
    linkList.add(4);
    linkList.add(5);
    let index = linkList.removeLast();
    linkList.add(index!);
    arktest.assertEQ(linkList.getLastIndexOf(index!), linkList.length - 1);
}

// Test cases ported from ArkTS 1.0: LINKED_LIST_GETLASTINDEXOF_0500
function testLinkedListGetAndRemoveFirstFound() {
    let linkList = new LinkedList<double>();
    linkList.add(1);
    linkList.add(2);
    linkList.add(3);
    linkList.add(4);
    linkList.add(5);
    let old = linkList.get(0);
    linkList.removeFirstFound(old!);
    arktest.assertNE(old, linkList.get(0));
}
