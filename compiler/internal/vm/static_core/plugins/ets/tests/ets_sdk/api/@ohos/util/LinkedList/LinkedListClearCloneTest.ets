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

    suite.addTest("Testing LinkedList removeLastFound with multiple occurrences",
        testLinkedListRemoveLastFoundMultiple);
    suite.addTest("Testing LinkedList removeLastFound with non-existing element",
        testLinkedListRemoveLastFoundNonExisting);
    suite.addTest("Testing LinkedList clone and verify cloned values", testLinkedListCloneVerifyValues);
    suite.addTest("Testing LinkedList clone and insert into cloned list", testLinkedListCloneInsert);
    suite.addTest("Testing LinkedList clone and removeFirstFound from cloned list",
        testLinkedListCloneRemoveFirstFound);
    suite.addTest("Testing LinkedList clone and addFirst to cloned list", testLinkedListCloneAddFirst);
    suite.addTest("Testing LinkedList clone and iterate using forEach", testLinkedListCloneForEach);
    suite.addTest("Testing LinkedList clear cloned list and add new elements", testLinkedListClearClonedList);
    suite.addTest("Testing LinkedList clear original list and add new elements", testLinkedListClearOriginalList);

    return suite.run();
}

// Test cases ported from ArkTS 1.0: LINKED_LIST_REMOVELASTFOUND_0100
function testLinkedListRemoveLastFoundMultiple() {
    let linkList = new LinkedList<double>();
    linkList.add(2);
    let index1 = linkList.getLastIndexOf(2);
    linkList.add(2);
    let index2 = linkList.getLastIndexOf(2);
    linkList.add(2);
    let index3 = linkList.getLastIndexOf(2);
    linkList.add(3);
    arktest.assertNE(index1, index2);
    arktest.assertNE(index2, index3);
    linkList.removeLastFound(2);
    arktest.assertEQ(linkList.getLastIndexOf(2), index2);
}

// Test cases ported from ArkTS 1.0: LINKED_LIST_REMOVELASTFOUND_0200
function testLinkedListRemoveLastFoundNonExisting() {
    let linkList = new LinkedList<double>();
    arktest.assertFalse(linkList.removeLastFound(2),
        "The removeLastFound method should return false for a non-existing element");
}

// Test cases ported from ArkTS 1.0: LINKED_LIST_CLONE_0100
function testLinkedListCloneVerifyValues() {
    let linkList = new LinkedList<double>();
    linkList.add(2);
    linkList.add(3);
    linkList.add(4);
    let newLinkList = linkList.clone();
    linkList.forEach((value, index) => {
        arktest.assertEQ(value, newLinkList.get(index));
    });
}

// Test cases ported from ArkTS 1.0: LINKED_LIST_CLONE_0200
function testLinkedListCloneInsert() {
    let linkList = new LinkedList<double>();
    linkList.add(2);
    linkList.add(3);
    linkList.add(4);
    let length = linkList.length;
    let newLinkList = linkList.clone();
    let newLength = newLinkList.length;
    newLinkList.insert(0, 1);
    arktest.assertEQ(length, linkList.length);
    arktest.assertNE(newLength, newLinkList.length);
}

// Test cases ported from ArkTS 1.0: LINKED_LIST_CLONE_0300
function testLinkedListCloneRemoveFirstFound() {
    let linkList = new LinkedList<double>();
    linkList.add(2);
    linkList.add(3);
    linkList.add(4);
    let length = linkList.length;
    let newLinkList = linkList.clone();
    let newLength = newLinkList.length;
    newLinkList.removeFirstFound(3);
    arktest.assertEQ(length, linkList.length);
    arktest.assertNE(newLength, newLinkList.length);
}

// Test cases ported from ArkTS 1.0: LINKED_LIST_CLONE_0400
function testLinkedListCloneAddFirst() {
    let linkList = new LinkedList<double>();
    linkList.add(2);
    linkList.add(3);
    linkList.add(4);
    let length = linkList.length;
    let newLinkList = linkList.clone();
    let newLength = newLinkList.length;
    newLinkList.addFirst(3);
    arktest.assertEQ(length, linkList.length);
    arktest.assertNE(newLength, newLinkList.length);
}

// Test cases ported from ArkTS 1.0: LINKED_LIST_CLONE_0500
function testLinkedListCloneForEach() {
    let linkList = new LinkedList<double>();
    linkList.add(2);
    linkList.add(3);
    linkList.add(4);
    let newLinkList = linkList.clone();
    newLinkList.forEach((value, index) => {
        arktest.assertEQ(value, linkList.get(index));
    });
}

// Test cases ported from ArkTS 1.0: LINKED_LIST_CLONE_0600
function testLinkedListClearClonedList() {
    let linkList = new LinkedList<double>();
    linkList.add(2);
    linkList.add(3);
    linkList.add(4);
    let newLinkList = linkList.clone();
    newLinkList.clear();
    newLinkList.add(1);
    arktest.assertEQ(newLinkList.length, 1);
    arktest.assertEQ(linkList.length, 3);
}

// Test cases ported from ArkTS 1.0: LINKED_LIST_CLONE_0700
function testLinkedListClearOriginalList() {
    let linkList = new LinkedList<double>();
    linkList.add(2);
    linkList.add(3);
    linkList.add(4);
    let newLinkList = linkList.clone();
    linkList.clear();
    linkList.add(1);
    arktest.assertEQ(linkList.length, 1);
    arktest.assertEQ(newLinkList.length, 3);
}
